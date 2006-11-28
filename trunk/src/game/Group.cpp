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
    WorldPacket data;
    Player *player;
    int32 slot = GetPlayerGroupSlot(guid);
    ASSERT( slot >= 0 );

    m_leaderGuid=guid;
    data.Initialize( SMSG_GROUP_SET_LEADER );
    data << m_members[slot].name;

    for( uint32 i = 0; i < m_count; i++ )
    {
        player = ObjectAccessor::Instance().FindPlayer( m_members[i].guid );
        ASSERT( player );

        player->SetLeader(guid );
        player->GetSession()->SendPacket( &data );
    }

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

    //delete all outstanding items rolled
    RollId.clear();

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
                data << (uint64)m_members[j].guid;
                data << uint16(1);
            }
        }
        data << (uint64)m_leaderGuid;
        data << (uint8)m_lootMethod;
        data << (uint64)m_looterGuid;
        data << (uint16)2;

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

    RemoveRollsFromMember( guid);

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

    return m_count;
}

void Group::RemoveRollsFromMember(const uint64 &guid)
{
    //If a player was rolling for an item, all his votes has to be reseted to "pass"
    vector<Roll>::iterator it;
    int8 pos;
    for (it = RollId.begin(); it < RollId.end(); it++)
    {
        pos = GetPlayerGroupSlot(guid);
        assert(pos >= 0);

        if (it->playerVote[pos] == GREED) it->totalGreed--;
        if (it->playerVote[pos] == NEED) it->totalNeed--;
        if (it->playerVote[pos] == PASS) it->totalPass--;
        if (it->playerVote[pos] != NOT_VALID) it->totalPlayersRolling--;

        for( uint32 j = pos + 1; j < m_count; j++ )
            it->playerVote[j-1] = it->playerVote[j];

        CountTheRoll(guid, it->itemGUID, m_count-1, 3);
    }
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

    for (uint32 i = 0; i < m_count; i++)
    {
        Player *p = objmgr.GetPlayer(m_members[i].guid);
        if (r.playerVote[i] != NOT_VALID)
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

    for (uint32 i = 0; i < m_count; i++)
    {
        Player *p = objmgr.GetPlayer(m_members[i].guid);
        if (r.playerVote[i] != NOT_VALID)
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

    for (uint32 i = 0; i < m_count; i++)
    {
        Player *p = objmgr.GetPlayer(m_members[i].guid);
        if (r.playerVote[i] != NOT_VALID)
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

    for (uint32 i = 0; i < m_count; i++)
    {
        Player *p = objmgr.GetPlayer(m_members[i].guid);
        if (r.playerVote[i] != NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::GroupLoot(uint64 playerGUID, Loot *loot, Creature *creature)
{

    vector<LootItem>::iterator i;
    ItemPrototype const *item;
    uint8 itemSlot = 0;
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
            for (uint32 j = 0; j < m_count; j++)
            {
                if (objmgr.GetPlayer(m_members[j].guid)->GetDistance2dSq(creature) < sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE))
                {
                    r.playerVote[j] = NOT_EMITED_YET;
                    r.totalPlayersRolling++;
                }
                else
                    r.playerVote[j] = NOT_VALID;
            }

            group->SendLootStartRoll(newitemGUID, r.totalPlayersRolling, i->itemid, 0, 60000, r);

            r.loot = loot;
            r.itemSlot = itemSlot;
            loot->items[itemSlot].is_blocked = true;

            RollId.push_back(r);
        }

        itemSlot++;
    }
}

void Group::NeedBeforeGreed(uint64 playerGUID, Loot *loot, Creature *creature)
{
    vector<LootItem>::iterator i;
    ItemPrototype const *item;
    uint8 itemSlot = 0;
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
            for (uint32 j = 0; j < m_count; j++)
            {
                Player *playerToRoll = objmgr.GetPlayer(m_members[j].guid);
                if (playerToRoll->CanUseItem(item))
                {
                    if (playerToRoll->GetDistance2dSq(creature) < sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE))
                    {
                        r.playerVote[j] = NOT_EMITED_YET;
                        r.totalPlayersRolling++;
                    }
                    else
                        r.playerVote[j] = NOT_VALID;
                }
                else
                    r.playerVote[j] = NOT_VALID;
            }

            if (r.totalPlayersRolling > 0)
            {
                group->SendLootStartRoll(newitemGUID, r.totalPlayersRolling, i->itemid, 0, 60000, r);

                r.loot = loot;
                r.itemSlot = itemSlot;
                loot->items[itemSlot].is_blocked = true;

                RollId.push_back(r);
            }
        }

        itemSlot++;
    }
}

void Group::CountTheRoll(uint64 playerGUID, uint64 Guid, uint32 NumberOfPlayers, uint8 Choise)
{

    vector<Roll>::iterator i;
    for (i=RollId.begin(); i != RollId.end(); i++)
    {
        if (i->itemGUID == Guid)
        {
            // this condition means that player joins to the party after roll begins
            if (GetPlayerGroupSlot(playerGUID) >= i->totalPlayersRolling)
                return;

            if (i->loot)
                if (i->loot->items.size() == 0)
                    return;

            int8 pos = GetPlayerGroupSlot(playerGUID);
            if (pos == -1 || pos >= MAXGROUPSIZE)           //error pos
                return;

            switch (Choise)
            {
                case 0:                                     //Player choose pass
                {
                    SendLootRoll(0, playerGUID, i->itemid, 0, 128, 0, *i);
                    i->playerVote[pos] = PASS;
                    i->totalPass++;
                }
                break;
                case 1:                                     //player choose Need
                {
                    SendLootRoll(0, playerGUID, i->itemid, 0, 1, 0, *i);
                    i->totalNeed++;
                    i->playerVote[pos] = NEED;
                }
                break;
                case 2:                                     //player choose Greed
                {
                    i->totalGreed++;
                    i->playerVote[pos] = GREED;
                }
                break;
            }

            //end of the roll
            if (i->totalPass + i->totalGreed + i->totalNeed >= i->totalPlayersRolling)
            {
                sLog.outDebug("Group::CountTheRoll - Finished item roll. itemSlot:%u;  total players rolling:%u; id of item:%u;", i->itemSlot, i->totalPlayersRolling, i->itemid);

                if (i->totalNeed > 0)
                {
                    uint8 maxresul = 0;
                    uint8 maxindex = 0;
                    Player *player;

                    for (uint8 j = 0; j < m_count; j++)
                    {
                        if (i->playerVote[j] != NEED)
                            continue;
                        uint8 randomN = rand() % 100;
                        SendLootRoll(0, m_members[j].guid, i->itemid, 0, randomN, 1, *i);
                        if (maxresul < randomN)
                        {
                            maxindex = j;
                            maxresul = randomN;
                        }
                    }
                    SendLootRollWon(0, m_members[maxindex].guid, i->itemid, 0, maxresul, 1, *i);
                    player = objmgr.GetPlayer(m_members[maxindex].guid);
                    uint16 dest;
                    LootItem *item = &(i->loot->items[i->itemSlot]);
                    uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, i->itemid, item->count, false );
                    if ( msg == EQUIP_ERR_OK )
                    {
                        item->is_looted = true;
                        i->loot->NotifyItemRemoved(i->itemSlot);
                        player->StoreNewItem( dest, i->itemid, item->count, true, true);
                    }
                    else
                    {
                        item->is_blocked = false;
                        player->SendEquipError( msg, NULL, NULL );
                    }
                }
                else
                {
                    if (i->totalGreed > 0)
                    {
                        uint8 maxresul = 0;
                        uint8 maxindex = 0;
                        Player *player;

                        for (uint8 j = 0; j < m_count; j++)
                        {
                            if (i->playerVote[j] != GREED)
                                continue;
                            uint8 randomN = rand() % 100;
                            SendLootRoll(0, m_members[j].guid, i->itemid, 0, randomN, 2, *i);
                            if (maxresul < randomN)
                            {
                                maxindex = j;
                                maxresul = randomN;
                            }
                        }
                        SendLootRollWon(0, m_members[maxindex].guid, i->itemid, 0, maxresul, 2, *i);
                        player = objmgr.GetPlayer(m_members[maxindex].guid);

                        uint16 dest;
                        LootItem *item = &(i->loot->items[i->itemSlot]);
                        uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, i->itemid, item->count, false );
                        if ( msg == EQUIP_ERR_OK )
                        {
                            item->is_looted = true;
                            i->loot->NotifyItemRemoved(i->itemSlot);
                            player->StoreNewItem( dest, i->itemid, item->count, true, true);
                        }
                        else
                        {
                            item->is_blocked = false;
                            player->SendEquipError( msg, NULL, NULL );
                        }
                    }
                    else
                    {
                        SendLootAllPassed(i->itemGUID, NumberOfPlayers, i->itemid, 0, *i);
                        LootItem *item = &(i->loot->items[i->itemSlot]);
                        item->is_blocked = false;
                    }
                }

                RollId.erase(i);
            }
            break;
        }
    }
}

int8 Group::GetPlayerGroupSlot(uint64 Guid)
{
    for (uint32 i = 0; i < m_count; i++)
    {
        if (m_members[i].guid == Guid)
            return i;
    }
    return -1;
}

void Group::BroadcastPacket(WorldPacket *packet)
{
    for (uint32 i = 0; i < m_count; i++)
    {
        Player *pl = ObjectAccessor::Instance().FindPlayer(m_members[i].guid);
        if (pl && pl->GetSession())
            pl->GetSession()->SendPacket(packet);
    }
}
