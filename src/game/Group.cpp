/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Common.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Group.h"
#include "Chat.h"
#include "ObjectAccessor.h"

void Group::ChangeLeader(const uint64 &guid)
{
    uint32 i;
    WorldPacket data;

    Player *player;

    for( i = 0; i < m_count; i++ )
    {
        if( m_members[i].guid == guid )
            break;
    }

    ASSERT( i <= MAXGROUPSIZE );

    m_leaderGuid=guid;
    data.Initialize( SMSG_GROUP_SET_LEADER );
    data << m_members[i].name;

    for( i = 0; i < m_count; i++ )
    {
        player = ObjectAccessor::Instance().FindPlayer( m_members[i].guid );
        ASSERT( player );

        player->SetLeader(guid );
        player->GetSession()->SendPacket( &data );
    }

    //delete all outstanding items rolled
    vector<Roll>::iterator it;
    for (it=RollId.begin(); it != RollId.end(); it++)
    {
        it->totalGreed.clear();
        it->totalNeed.clear();
        it->playersRolling.clear();
    }
    RollId.clear();

    SendUpdate();
}

void Group::Disband()
{
    uint32 i;
    WorldPacket data;
    Player *player;

    data.Initialize( SMSG_GROUP_DESTROYED );

    for( i = 0; i < m_count; i++ )
    {
        player = ObjectAccessor::Instance().FindPlayer( m_members[i].guid );
        ASSERT( player );
        player->RemoveAreaAurasByOthers();

        player->UnSetInGroup();
        player->GetSession()->SendPacket( &data );
    }

}

void Group::SendUpdate()
{
    uint32 i ,j;
    Player *player;
    WorldPacket data;

    for( i = 0; i < m_count; i ++ )
    {
        player = ObjectAccessor::Instance().FindPlayer( m_members[i].guid );
        ASSERT( player );

        data.Initialize(SMSG_GROUP_LIST);
        data << (uint16)m_grouptype;
        data << uint32(m_count - 1);

        for( j = 0; j < m_count; j++ )
        {
            if (m_members[j].guid != m_members[i].guid)
            {
                data << m_members[j].name;
                data << (uint32)m_members[j].guid;
                data << uint32(0) << uint16(1);
            }
        }
        data << (uint64)m_leaderGuid;
        data << (uint8)m_lootMethod;
        data << (uint32)m_looterGuid;
        data << uint32(0);
        data << uint8(2);

        player->GetSession()->SendPacket( &data );
    }
}

uint32 Group::RemoveMember(const uint64 &guid)
{
    uint32 i, j;
    bool leaderFlag = false;

    Player *player = ObjectAccessor::Instance().FindPlayer( guid );
    if (player)
    {
        player->RemoveAreaAurasByOthers();
        player->RemoveAreaAurasFromGroup();
    }

    for( i = 0; i < m_count; i++ )
    {
        if (m_members[i].guid == guid)
        {
            leaderFlag = m_members[i].guid == m_leaderGuid;

            for( j = i + 1; j < m_count; j++ )
            {
                m_members[j-1].guid = m_members[j].guid;
                m_members[j-1].name = m_members[j].name;
            }

            break;
        }
    }

    m_count--;

    if (m_count > 1 && leaderFlag)
        ChangeLeader(m_members[0].guid);

    //if player was rolling for a item, he choose pass
    vector<Roll>::iterator it;
    vector<uint64>::iterator it2;
    Player *p=objmgr.GetPlayer(guid);
    for (it = RollId.begin(); it < RollId.end(); it++)
    {
        for (it2 = it->playersRolling.begin(); it2 < it->playersRolling.end(); it2++)
        {
            if (*it2 == guid)
                CountTheRoll(guid, it->itemGUID, it->playersRolling.size(), 0);
        }
    }

    return m_count;
}

void Group::BroadcastToGroup(WorldSession *session, std::string msg)
{
    if (session && session->GetPlayer())
    {
        for (uint32 i = 0; i < m_count; i++)
        {
            WorldPacket data;
            sChatHandler.FillMessageData(&data, session, CHAT_MSG_PARTY, LANG_UNIVERSAL, NULL, 0, msg.c_str());
            Player *pl = ObjectAccessor::Instance().FindPlayer(m_members[i].guid);
            if (pl && pl->GetSession())
                pl->GetSession()->SendPacket(&data);
        }
    }
}

void Group::SendLootStartRoll(uint64 Guid, uint32 NumberinGroup, uint32 ItemEntry, uint32 ItemInfo, uint32 CountDown, const Roll &r)
{
    WorldPacket data;
    data.Initialize(SMSG_LOOT_START_ROLL);
    data << Guid;                                           // guid of the creature/go that has the item which is being rolled for
    data << NumberinGroup;                                  // maybe the number of players rolling for it???
    data << ItemEntry;                                      // the itemEntryId for the item that shall be rolled for
    data << uint32(0);                                      // unknown
    data << ItemInfo;                                       // ItemInfo this is related to special additionals to a item
    data << CountDown;                                      // the countdown time to choose "need" or "greed"

    vector<uint64>::const_iterator it;
    for (it = r.playersRolling.begin(); it < r.playersRolling.end(); it++)
    {
        Player *p = objmgr.GetPlayer(*it); 
        p->GetSession()->SendPacket( &data );
    }
}

void Group::SendLootRoll(uint64 SourceGuid, uint64 TargetGuid, uint32 ItemEntry, uint32 ItemInfo, uint8 RollNumber, uint8 RollType, const Roll &r)
{
    WorldPacket data;
    data.Initialize(SMSG_LOOT_ROLL);
    data << SourceGuid;                                     // guid of the item rolled
    data << (uint32)0;                                      // unknown, maybe amount of players
    data << TargetGuid;
    data << ItemEntry;                                      // the itemEntryId for the item that shall be rolled for
    data << uint32(0);                                      // unknown
    data << ItemInfo;                                       // ItemInfo this is related to special additionals to a item
    data << RollNumber;                                     // 0: "Need for: [item name]" > 127: "you passed on: [item name]"      Roll number
    data << RollType;                                       // 0: "Need for: [item name]" 0: "You have selected need for [item name] 1: need roll 2: greed roll 

    vector<uint64>::const_iterator it;
    for (it = r.playersRolling.begin(); it < r.playersRolling.end(); it++)
    {
        Player *p = objmgr.GetPlayer(*it); 
        p->GetSession()->SendPacket( &data );
    }
}

void Group::SendLootRollWon(uint64 SourceGuid, uint64 TargetGuid, uint32 ItemEntry, uint32 ItemInfo, uint8 RollNumber, uint8 RollType, const Roll &r)
{
    WorldPacket data;
    data.Initialize(SMSG_LOOT_ROLL_WON);
    data << SourceGuid;                                     // guid of the item rolled
    data << (uint32)0;                                      // unknown, maybe amount of players
    data << ItemEntry;                                      // the itemEntryId for the item that shall be rolled for
    data << uint32(0);                                      // unknown
    data << ItemInfo;                                       // ItemInfo this is related to special additionals to a item
    data << TargetGuid;                                     // guid of the player who won.
    data << RollNumber;                                     // rollnumber realted to SMSG_LOOT_ROLL
    data << RollType;                                       // Rolltype related to SMSG_LOOT_ROLL

    vector<uint64>::const_iterator it;
    for (it = r.playersRolling.begin(); it < r.playersRolling.end(); it++)
    {
        Player *p = objmgr.GetPlayer(*it); 
        p->GetSession()->SendPacket( &data );
    }
}

void Group::SendLootAllPassed(uint64 Guid, uint32 NumberOfPlayers, uint32 ItemEntry, uint32 ItemInfo, const Roll &r)
{
    WorldPacket data;
    data.Initialize(SMSG_LOOT_ALL_PASSED);
    data << Guid;                                           // Guid of the item rolled
    data << NumberOfPlayers;                                // The number of players rolling for it???
    data << ItemEntry;                                      // The itemEntryId for the item that shall be rolled for
    data << ItemInfo;                                       // ItemInfo
    data << uint32(0x3F3);                                  // unknown, I think it can be number of roll

    vector<uint64>::const_iterator it;
    for (it = r.playersRolling.begin(); it < r.playersRolling.end(); it++)
    {
        Player *p = objmgr.GetPlayer(*it); 
        p->GetSession()->SendPacket( &data );
    }
}

void Group::GroupLoot(uint64 playerGUID, Loot *loot, Creature *creature)
{

    vector<LootItem>::iterator i;
    ItemPrototype const *item;
    uint8 THRESHOLD = 1;
    Player *player = objmgr.GetPlayer(playerGUID);
    Group *group = objmgr.GetGroupByLeader(player->GetGroupLeader());

    for (i=loot->items.begin(); i != loot->items.end(); i++)
    {
        item = objmgr.GetItemPrototype(i->itemid);
        if (item->Quality > THRESHOLD)
        {
            Roll r;
            uint32 newitemGUID = objmgr.GenerateLowGuid(HIGHGUID_ITEM);
            r.itemGUID = newitemGUID;
            r.itemid = i->itemid;
            
            //a vector is filled with only near party members
            uint32 maxdist = sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE);
            for (int j = 0; j < group->GetMembersCount(); j++)
            {
                if (objmgr.GetPlayer(GetMemberGUID(j))->GetDistance2dSq(creature) < maxdist*maxdist)
                    r.playersRolling.push_back(GetMemberGUID(j));
            }
            
            group->SendLootStartRoll(newitemGUID, r.playersRolling.size(), i->itemid, 0, 60000, r);
            
            RollId.push_back(r);
            loot->remove(*i);
            i--;
        }
    }
}

void Group::NeedBeforeGreed(uint64 playerGUID, Loot *loot, Creature *creature)
{
    vector<LootItem>::iterator i;
    ItemPrototype const *item;
    uint8 THRESHOLD = 1;
    Player *player = objmgr.GetPlayer(playerGUID);
    Group *group = objmgr.GetGroupByLeader(player->GetGroupLeader());

    for (i=loot->items.begin(); i != loot->items.end(); i++)
    {
        item = objmgr.GetItemPrototype(i->itemid);
        if (item->Quality > THRESHOLD)
        {
            Roll r;
            uint32 newitemGUID = objmgr.GenerateLowGuid(HIGHGUID_ITEM);
            r.itemGUID = newitemGUID;
            r.itemid = i->itemid;
            
            //a vector is filled with only near party members
            uint32 maxdist = sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE);
            for (int j = 0; j < group->GetMembersCount(); j++)
            {
                Player *playerToRoll = objmgr.GetPlayer(GetMemberGUID(j));
                if (playerToRoll->CanUseItem(item))
                {
                    if (playerToRoll->GetDistance2dSq(creature) < maxdist*maxdist)
                        r.playersRolling.push_back(GetMemberGUID(j));
                }
            }
            
            if (r.playersRolling.size() > 0)
            {
                group->SendLootStartRoll(newitemGUID, r.playersRolling.size(), i->itemid, 0, 60000, r);
            
                RollId.push_back(r);
                loot->remove(*i);
                i--;
            }            
        }
    }
}

void Group::CountTheRoll(uint64 playerGUID, uint64 Guid, uint32 NumberOfPlayers, uint8 Choise)
{
	
    vector<Roll>::iterator i;
    for (i=RollId.begin(); i != RollId.end(); i++)
    {
        if (i->itemGUID == Guid)
        {
            if (Choise == 2) //player choose Greed
                i->totalGreed.push_back(playerGUID);
            else
            {
                if (Choise == 1)  //player choose Need
                {
                    SendLootRoll(0, playerGUID, i->itemid, 0, 1, 0, *i);
                    i->totalNeed.push_back(playerGUID);
                }
                else  //Player choose pass
                {
                    SendLootRoll(0, playerGUID, i->itemid, 0, 128, 0, *i);
                    i->totalPass++;
                }
            }
            if (i->totalPass + i->totalGreed.size() + i->totalNeed.size() >= i->playersRolling.size())  //end of the roll
            {
               if (i->totalNeed.size() > 0)
                {
                    vector<uint8> rollresul;
                    uint8 maxresul = 0;
                    uint8 maxindex = 0;
                    Player *player;

                    for (uint8 j = 0; j < i->totalNeed.size(); j++)
                    {
                        uint8 randomN = rand() % 126;
                        rollresul.push_back(randomN);
                        SendLootRoll(0, i->totalNeed[j], i->itemid, 0, rollresul[j], 1, *i);
                        if (maxresul < rollresul[j]) 
                        {
                            maxindex = j;
                            maxresul = rollresul[j];
                        }
                    }
                    SendLootRollWon(0, i->totalNeed[maxindex], i->itemid, 0, rollresul[maxindex], 1, *i);
                    player = objmgr.GetPlayer(i->totalNeed[maxindex]);
                    uint16 dest;
                    uint8 msg = player->CanStoreNewItem( 0, NULL_SLOT, dest, i->itemid, 1, false );
                    if ( msg == EQUIP_ERR_OK )
                        player->StoreNewItem( dest, i->itemid, 1, true);
                    else
                        player->SendEquipError( msg, NULL, NULL );
                }
                else
                {
                    if (i->totalGreed.size() > 0)
                    {
                        vector<uint8> rollresul;
                        uint8 maxresul = 0;
                        uint8 maxindex = 0;
                        Player *player;

                        for (uint8 j = 0; j < i->totalGreed.size(); j++)
                        {
                            uint8 randomN = rand() % 126;
                            rollresul.push_back(randomN);
                            SendLootRoll(0, i->totalGreed[j], i->itemid, 0, rollresul[j], 2, *i);
                            if (maxresul < rollresul[j]) 
                            {
                                maxindex = j;
                                maxresul = rollresul[j];
                            }
                        }
                        SendLootRollWon(0, i->totalGreed[maxindex], i->itemid, 0, rollresul[maxindex], 2, *i);
                        player = objmgr.GetPlayer(i->totalGreed[maxindex]);
						
                        uint16 dest;
                        uint8 msg = player->CanStoreNewItem( 0, NULL_SLOT, dest, i->itemid, 1, false );
                        if ( msg == EQUIP_ERR_OK )
                           player->StoreNewItem( dest, i->itemid, 1, true);
                        else
                            player->SendEquipError( msg, NULL, NULL );
                    }
                    else
                        SendLootAllPassed(i->itemGUID, NumberOfPlayers, i->itemid, 0, *i);
                    //TODO: if all players choose pass, re-add item to loot so everyone can take it
                }
            }
        }
    }
} 