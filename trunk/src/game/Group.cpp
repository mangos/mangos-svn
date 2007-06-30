/* 
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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

bool Group::Create(const uint64 &guid, const char * name)
{
    m_leaderGuid = guid;
    m_leaderName = name;

    m_groupType  = GROUPTYPE_NORMAL;
    m_lootMethod = GROUP_LOOT;
    m_lootThreshold = UNCOMMON;

    if(!AddMember(guid, name))
        return false;

    // store group in database
    sDatabase.BeginTransaction();
    sDatabase.PExecute("DELETE FROM `group` WHERE `leaderGuid`='%u'", GUID_LOPART(m_leaderGuid));
    sDatabase.PExecute("DELETE FROM `group_member` WHERE `leaderGuid`='%u'", GUID_LOPART(m_leaderGuid));
    sDatabase.PExecute("INSERT INTO `group`(`leaderGuid`,`mainTank`,`mainAssistant`,`lootMethod`,`looterGuid`,`lootThreshold`,`icon1`,`icon2`,`icon3`,`icon4`,`icon5`,`icon6`,`icon7`,`icon8`,`isRaid`) VALUES('%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u',0)", GUID_LOPART(m_leaderGuid), GUID_LOPART(m_mainTank), GUID_LOPART(m_mainAssistant), m_lootMethod, GUID_LOPART(m_looterGuid), m_lootThreshold, m_targetIcons[0], m_targetIcons[1], m_targetIcons[2], m_targetIcons[3], m_targetIcons[4], m_targetIcons[5], m_targetIcons[6], m_targetIcons[7]);

    for(Group::member_citerator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        sDatabase.PExecute("INSERT INTO `group_member`(`leaderGuid`,`memberGuid`,`assistant`,`subgroup`) VALUES('%u','%u','%u','%u')", 
            GUID_LOPART(m_leaderGuid), GUID_LOPART(itr->guid), (itr->assistant==1)?1:0, itr->group);
    }
    sDatabase.CommitTransaction();

    return true;
}

bool Group::LoadGroupFromDB(const uint64 &leaderGuid)
{
    //                                             0          1               2            3            4               5       6       7       8       9       10      11      12      13
    QueryResult *result = sDatabase.PQuery("SELECT `mainTank`,`mainAssistant`,`lootMethod`,`looterGuid`,`lootThreshold`,`icon1`,`icon2`,`icon3`,`icon4`,`icon5`,`icon6`,`icon7`,`icon8`,`isRaid` FROM `group` WHERE `leaderGuid`='%u'", GUID_LOPART(leaderGuid));
    if(!result)
        return false;

    m_leaderGuid = leaderGuid;

    // group leader not exist
    if(!objmgr.GetPlayerNameByGUID(m_leaderGuid, m_leaderName))
        return false;

    m_groupType  = (*result)[13].GetBool() ? GROUPTYPE_RAID : GROUPTYPE_NORMAL;
    m_mainTank = (*result)[0].GetUInt64();
    m_mainAssistant = (*result)[1].GetUInt64();
    m_lootMethod = (LootMethod)(*result)[2].GetUInt8();
    m_looterGuid = MAKE_GUID((*result)[3].GetUInt32(),HIGHGUID_PLAYER);
    m_lootThreshold = (LootThreshold)(*result)[4].GetUInt16();

    for(int i=0; i<TARGETICONCOUNT; i++)
        m_targetIcons[i] = (*result)[5+i].GetUInt8();
    delete result;

    result = sDatabase.PQuery("SELECT `memberGuid`,`assistant`,`subgroup` FROM `group_member` WHERE `leaderGuid`='%u'", GUID_LOPART(leaderGuid));
    if(!result)
        return false;

    do
    {
        MemberSlot member;
        member.guid      = MAKE_GUID((*result)[0].GetUInt32(),HIGHGUID_PLAYER);

        // skip non-existed member
        if(!objmgr.GetPlayerNameByGUID(member.guid, member.name))
            continue;

        member.group     = (*result)[2].GetUInt8();
        member.assistant = (*result)[1].GetBool();
        m_members.push_back(member);
    } while( result->NextRow() );
    delete result;

    // group too small
    if(m_members.size() < 2)
        return false;

    return true;
}

bool Group::AddInvite(Player *player)
{
    if(!player || player->groupInfo.invite || player->groupInfo.group)
        return false;

    RemoveInvite(player);

    m_invitees.insert(player->GetGUID());

    player->groupInfo.invite = this;

    return true;
}

uint32 Group::RemoveInvite(Player *player)
{
    for(InvitesList::iterator itr=m_invitees.begin(); itr!=m_invitees.end(); ++itr)
    {
        if((*itr) == player->GetGUID())
        {
            m_invitees.erase(itr);
            break;
        }
    }

    player->groupInfo.invite = NULL;
    return m_members.size();
}

bool Group::AddMember(const uint64 &guid, const char* name)
{
    if(!_addMember(guid, name))
        return false;
    SendUpdate();

    return true;
}

uint32 Group::RemoveMember(const uint64 &guid, const uint8 &method)
{
    // remove member and change leader (if need) only if strong more 2 members _before_ member remove
    if(m_members.size() > 2)
    {
        bool leaderChanged = _removeMember(guid);

        Player *player = objmgr.GetPlayer( guid );
        if (player)
        {
            WorldPacket data;

            if(method == 1)
            {
                data.Initialize( SMSG_GROUP_UNINVITE, 0 );
                player->GetSession()->SendPacket( &data );
            }

            data.Initialize(SMSG_GROUP_LIST, 14);
            data<<(uint16)0<<(uint8)0<<(uint32)0<<(uint64)0;
            player->GetSession()->SendPacket(&data);
        }

        if(leaderChanged)
        {
            WorldPacket data(SMSG_GROUP_SET_LEADER, (m_members.front().name.size()+1));
            data << m_members.front().name;
            BroadcastPacket(&data);
        }

        SendUpdate();
    }
    // if group before remove <= 2 disband it
    else
        Disband(true);

    return m_members.size();
}

void Group::ChangeLeader(const uint64 &guid)
{
    member_citerator slot = _getMemberCSlot(guid);

    if(slot==m_members.end())
        return;

    _setLeader(guid);

    WorldPacket data(SMSG_GROUP_SET_LEADER, slot->name.size()+1);
    data << slot->name;
    BroadcastPacket(&data);
    SendUpdate();
}

void Group::Disband(bool hideDestroy)
{
    Player *player;

    for(member_citerator citr = m_members.begin(); citr != m_members.end(); ++citr)
    {
        player = objmgr.GetPlayer(citr->guid);
        if(!player)
            continue;

        player->RemoveAreaAurasByOthers();
        player->RemoveAreaAurasFromGroup();
        player->groupInfo.group = NULL;

        if(!player->GetSession())
            continue;

        WorldPacket data;
        if(!hideDestroy)
        {
            data.Initialize(SMSG_GROUP_DESTROYED, 0);
            player->GetSession()->SendPacket(&data);
        }

        data.Initialize(SMSG_GROUP_LIST, 14);
        data<<(uint16)0<<(uint8)0<<(uint32)0<<(uint64)0;
        player->GetSession()->SendPacket(&data);
    }
    RollId.clear();
    m_members.clear();

    for(InvitesList::iterator itr=m_invitees.begin(); itr!=m_invitees.end(); ++itr)
    {
        Player *invitee = objmgr.GetPlayer(*itr);
        if(invitee)
            invitee->groupInfo.invite = NULL;
    }
    m_invitees.clear();

    sDatabase.BeginTransaction();
    sDatabase.PExecute("DELETE FROM `group` WHERE `leaderGuid`='%u'", GUID_LOPART(m_leaderGuid));
    sDatabase.PExecute("DELETE FROM `group_member` WHERE `leaderGuid`='%u'", GUID_LOPART(m_leaderGuid));
    sDatabase.CommitTransaction();

    m_leaderGuid = 0;
    m_leaderName = "";
}

/*********************************************************/
/***                   LOOT SYSTEM                     ***/
/*********************************************************/

void Group::SendLootStartRoll(uint64 Guid, uint32 CountDown, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_START_ROLL, (8+4+4+4+4+4));
    data << uint64(Guid);                                   // guid of the creature/go that has the item which is being rolled for
    data << uint32(r.totalPlayersRolling);                  // maybe the number of players rolling for it???
    data << uint32(r.itemid);                               // the itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomSuffix);                     // randomSuffix
    data << uint32(r.itemRandomPropId);                     // item random property ID
    data << uint32(CountDown);                              // the countdown time to choose "need" or "greed"

    map<uint64, RollVote>::const_iterator itr;
    for (itr=r.playerVote.begin(); itr!=r.playerVote.end(); itr++)
    {
        Player *p = objmgr.GetPlayer(itr->first);
        if(!p || !p->GetSession())
            continue;

        if(itr->second != NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::SendLootRoll(uint64 SourceGuid, uint64 TargetGuid, uint8 RollNumber, uint8 RollType, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ROLL, (8+4+8+4+4+4+1+1));
    data << uint64(SourceGuid);                             // guid of the item rolled
    data << (uint32)0;                                      // unknown, maybe amount of players
    data << uint64(TargetGuid);
    data << uint32(r.itemid);                               // the itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomSuffix);                     // randomSuffix
    data << uint32(r.itemRandomPropId);                     // Item random property ID
    data << uint8(RollNumber);                              // 0: "Need for: [item name]" > 127: "you passed on: [item name]"      Roll number
    data << uint8(RollType);                                // 0: "Need for: [item name]" 0: "You have selected need for [item name] 1: need roll 2: greed roll

    map<uint64, RollVote>::const_iterator itr;
    for (itr=r.playerVote.begin(); itr!=r.playerVote.end(); itr++)
    {
        Player *p = objmgr.GetPlayer(itr->first);
        if(!p || !p->GetSession())
            continue;

        if(itr->second != NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::SendLootRollWon(uint64 SourceGuid, uint64 TargetGuid, uint8 RollNumber, uint8 RollType, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ROLL_WON, (8+4+4+4+4+1+1));
    data << uint64(SourceGuid);                             // guid of the item rolled
    data << (uint32)0;                                      // unknown, maybe amount of players
    data << uint32(r.itemid);                               // the itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomSuffix);                     // randomSuffix
    data << uint32(r.itemRandomPropId);                     // Item random property
    data << uint64(TargetGuid);                             // guid of the player who won.
    data << uint8(RollNumber);                              // rollnumber realted to SMSG_LOOT_ROLL
    data << uint8(RollType);                                // Rolltype related to SMSG_LOOT_ROLL

    map<uint64, RollVote>::const_iterator itr;
    for (itr=r.playerVote.begin(); itr!=r.playerVote.end(); itr++)
    {
        Player *p = objmgr.GetPlayer(itr->first);
        if(!p || !p->GetSession())
            continue;

        if(itr->second != NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::SendLootAllPassed(uint64 Guid, uint32 NumberOfPlayers, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ALL_PASSED, (8+4+4+4+4));
    data << uint64(Guid);                                   // Guid of the item rolled
    data << uint32(NumberOfPlayers);                        // The number of players rolling for it???
    data << uint32(r.itemid);                               // The itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomPropId);                     // Item random property ID
    data << uint32(r.itemRandomSuffix);                     // Item random suffix ID 

    map<uint64, RollVote>::const_iterator itr;
    for (itr=r.playerVote.begin(); itr!=r.playerVote.end(); itr++)
    {
        Player *p = objmgr.GetPlayer(itr->first);
        if(!p || !p->GetSession())
            continue;

        if(itr->second != NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::GroupLoot(uint64 playerGUID, Loot *loot, Creature *creature)
{
    vector<LootItem>::iterator i;
    ItemPrototype const *item;
    uint8 itemSlot = 0;
    Player *player = objmgr.GetPlayer(playerGUID);
    Group *group = player->groupInfo.group;

    for (i=loot->items.begin(); i != loot->items.end(); i++, itemSlot++)
    {
        item = objmgr.GetItemPrototype(i->itemid);
        if (!item)
        {
            //sLog.outDebug("Group::GroupLoot: missing item prototype for item with id: %d", i->itemid);
            continue;
        }
        if (item->Quality >= m_lootThreshold)
        {
            Roll r;
            uint32 newitemGUID = objmgr.GenerateLowGuid(HIGHGUID_ITEM);
            r.itemGUID = newitemGUID;
            r.itemid = i->itemid;
            r.itemRandomPropId = i->randomPropertyId;
            r.itemRandomSuffix = i->randomSuffix;

            //a vector is filled with only near party members
            for(member_citerator itr = m_members.begin(); itr != m_members.end(); ++itr)
            {
                Player *member = objmgr.GetPlayer(itr->guid);
                if(!member || !member->GetSession())
                    continue;

                if (member->GetDistance2dSq(creature) < sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE))
                {
                    r.playerVote[itr->guid] = NOT_EMITED_YET;
                    r.totalPlayersRolling++;
                }
            }

            r.loot = loot;
            r.itemSlot = itemSlot;

            group->SendLootStartRoll(newitemGUID, 60000, r);

            loot->items[itemSlot].is_blocked = true;
            creature->m_groupLootTimer = 60000;
            creature->lootingGroupLeaderGUID = GetLeaderGUID();

            RollId.push_back(r);
        }
    }
}

void Group::NeedBeforeGreed(uint64 playerGUID, Loot *loot, Creature *creature)
{
    vector<LootItem>::iterator i;
    ItemPrototype const *item;
    uint8 itemSlot = 0;
    Player *player = objmgr.GetPlayer(playerGUID);
    Group *group = player->groupInfo.group;

    for (i=loot->items.begin(); i != loot->items.end(); i++)
    {
        item = objmgr.GetItemPrototype(i->itemid);
        if (item->Quality >= m_lootThreshold)
        {
            Roll r;
            uint32 newitemGUID = objmgr.GenerateLowGuid(HIGHGUID_ITEM);
            r.itemGUID = newitemGUID;
            r.itemid = i->itemid;
            r.itemRandomPropId = i->randomPropertyId;
            r.itemRandomSuffix = i->randomSuffix;

            for(member_citerator itr = m_members.begin(); itr != m_members.end(); ++itr)
            {
                Player *playerToRoll = objmgr.GetPlayer(itr->guid);
                if(!playerToRoll || !playerToRoll->GetSession())
                    continue;

                if (playerToRoll->CanUseItem(item))
                {
                    if (playerToRoll->GetDistance2dSq(creature) < sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE))
                    {
                        r.playerVote[itr->guid] = NOT_EMITED_YET;
                        r.totalPlayersRolling++;
                    }
                }
            }

            if (r.totalPlayersRolling > 0)
            {
                r.loot = loot;
                r.itemSlot = itemSlot;

                group->SendLootStartRoll(newitemGUID, 60000, r);

                loot->items[itemSlot].is_blocked = true;

                RollId.push_back(r);
            }
        }

        itemSlot++;
    }
}

void Group::CountRollVote(uint64 playerGUID, uint64 Guid, uint32 NumberOfPlayers, uint8 Choise)
{
    vector<Roll>::iterator roll = GetRoll(Guid);
    if (roll == RollId.end())
        return;

    map<uint64, RollVote>::iterator itr = roll->playerVote.find(playerGUID);
    // this condition means that player joins to the party after roll begins
    if (itr == roll->playerVote.end())
        return;

    if (roll->loot)
        if (roll->loot->items.size() == 0)
            return;

    switch (Choise)
    {
        case 0:                                             //Player choose pass
        {
            SendLootRoll(0, playerGUID, 128, 128, *roll);
            roll->totalPass++;
            itr->second = PASS;
        }
        break;
        case 1:                                             //player choose Need
        {
            SendLootRoll(0, playerGUID, 1, 1, *roll);
            roll->totalNeed++;
            itr->second = NEED;
        }
        break;
        case 2:                                             //player choose Greed
        {
            SendLootRoll(0, playerGUID, 2, 2, *roll);
            roll->totalGreed++;
            itr->second = GREED;
        }
        break;
    }
    if (roll->totalPass + roll->totalGreed + roll->totalNeed >= roll->totalPlayersRolling)
    {
        CountTheRoll(roll, NumberOfPlayers);
    }
}

//called when roll timer expires
void Group::EndRoll()
{
    vector<Roll>::iterator itr;
    while(!RollId.empty())
    {
        //need more testing here, if rolls disappear
        itr = RollId.begin();
        CountTheRoll(itr, m_members.size());                //i don't have to edit player votes, who didn't vote ... he will pass
    }
}

void Group::CountTheRoll(vector<Roll>::iterator roll, uint32 NumberOfPlayers)
{
    //end of the roll
    if (roll->totalNeed > 0)
    {
        uint8 maxresul = 0;
        uint64 maxguid  = (*roll->playerVote.begin()).first;
        Player *player;

        map<uint64, RollVote>::iterator itr;
        for (itr=roll->playerVote.begin(); itr!=roll->playerVote.end(); ++itr)
        {
            if (itr->second != NEED)
                continue;

            uint8 randomN = urand(1, 99);
            SendLootRoll(0, itr->first, randomN, 1, *roll);
            if (maxresul < randomN)
            {
                maxguid  = itr->first;
                maxresul = randomN;
            }
        }
        SendLootRollWon(0, maxguid, maxresul, 1, *roll);
        player = objmgr.GetPlayer(maxguid);

        if(player && player->GetSession())
        {
            uint16 dest;
            LootItem *item = &(roll->loot->items[roll->itemSlot]);
            uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, roll->itemid, item->count, false );
            if ( msg == EQUIP_ERR_OK )
            {
                item->is_looted = true;
                roll->loot->NotifyItemRemoved(roll->itemSlot);
                player->StoreNewItem( dest, roll->itemid, item->count, true, item->randomPropertyId);
            }
            else
            {
                item->is_blocked = false;
                player->SendEquipError( msg, NULL, NULL );
            }
        }
    }
    else if (roll->totalGreed > 0)
    {
        uint8 maxresul = 0;
        uint64 maxguid = (*roll->playerVote.begin()).first;
        Player *player;

        map<uint64, RollVote>::iterator itr;
        for (itr=roll->playerVote.begin(); itr!=roll->playerVote.end(); ++itr)
        {
            if (itr->second != GREED)
                continue;

            uint8 randomN = urand(1, 99);
            SendLootRoll(0, itr->first, randomN, 2, *roll);
            if (maxresul < randomN)
            {
                maxguid  = itr->first;
                maxresul = randomN;
            }
        }
        SendLootRollWon(0, maxguid, maxresul, 2, *roll);
        player = objmgr.GetPlayer(maxguid);

        if(player && player->GetSession())
        {
            uint16 dest;
            LootItem *item = &(roll->loot->items[roll->itemSlot]);
            uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, roll->itemid, item->count, false );
            if ( msg == EQUIP_ERR_OK )
            {
                item->is_looted = true;
                roll->loot->NotifyItemRemoved(roll->itemSlot);
                player->StoreNewItem( dest, roll->itemid, item->count, true, item->randomPropertyId);
            }
            else
            {
                item->is_blocked = false;
                player->SendEquipError( msg, NULL, NULL );
            }
        }
    }
    else
    {
        SendLootAllPassed(roll->itemGUID, NumberOfPlayers, *roll);
        LootItem *item = &(roll->loot->items[roll->itemSlot]);
        item->is_blocked = false;
    }
    RollId.erase(roll);
}

void Group::SetTargetIcon(uint8 id, uint64 guid)
{
    if(id >= TARGETICONCOUNT)
        return;

    m_targetIcons[id] = guid;

    WorldPacket data(MSG_RAID_ICON_TARGET, (2+8));
    data << (uint8)0;
    data << id;
    data << guid;
    BroadcastPacket(&data);
}

Player* Group::GetMemberForXPAtKill(uint64 guid, Unit const* victim)
{
    Player* member = objmgr.GetPlayer(guid);
    if(!member || !member->isAlive())
        return NULL;
    if(victim->GetDistanceSq(member) > sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE))
        return NULL;
    if(uint32(abs((int)member->getLevel() - (int)victim->getLevel())) > sWorld.getConfig(CONFIG_GROUP_XP_LEVELDIFF))
        return NULL;

    return member;
}

uint32 Group::GetMemberCountForXPAtKill(Unit const* victim)
{
    uint32 count = 0;
    for(member_citerator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        Player* member = GetMemberForXPAtKill(itr->guid,victim);

        if(member)
            ++count;
    }
    return count;
}

/*void Group::SendInit(WorldSession *session)
{
    if(!session)
        return;

    int8   myIndex;
    uint8  myFlag;
    uint64 guid;

    guid = session->GetPlayer()->GetGUID();
    myIndex = _getMemberIndex(guid);
    myFlag  = m_members[myIndex].group | (m_members[myIndex].assistant?0x80:0);
    for(int i=1; i<=m_members.size(); i++)
    {
                                                            // guess size
        WorldPacket data(SMSG_GROUP_LIST, (2+4+8+8+1+2+m_members.size()*20));
        data << (uint8)m_groupType;
        data << (uint8)0;// 2.0.x, may be wrong position
        data << (uint8)myFlag;

        int count = 0;
        data << uint32(m_members.size()-1);
        for(vector<MemberSlot>::const_iterator citr=m_members.begin(); citr!=m_members.end(); citr++)
        {
            if(citr->guid == guid)
                continue;

            data << ((count<i) ? citr->name : "");
            data << citr->guid;
            data << (uint8)(objmgr.GetPlayer(citr->guid)?1:0);
            data << (uint8)(citr->group | (citr->assistant?0x80:0));
            count++;

            if(count >= i)
                break;
        }

        data << m_leaderGuid;
        data << (uint8)m_lootMethod;
        data << m_looterGuid;
        data << (uint16)2;
        data << m_mainTank;//2.0.x
        data << m_mainAssistant;//2.0.x
        session->SendPacket( &data );
    }
}*/

void Group::SendTargetIconList(WorldSession *session)
{
    if(!session)
        return;

    WorldPacket data(MSG_RAID_ICON_TARGET, (1+TARGETICONCOUNT*9));
    data << (uint8)1;

    for(int i=0; i<TARGETICONCOUNT; i++)
    {
        if(m_targetIcons[i] == 0)
            continue;

        data << (uint8)i;
        data << m_targetIcons[i];
    }

    session->SendPacket(&data);
}

void Group::SendUpdate()
{
    Player *player;

    for(member_citerator citr = m_members.begin(); citr != m_members.end(); ++citr)
    {
        player = objmgr.GetPlayer(citr->guid);
        if(!player || !player->GetSession())
            continue;

                                                            // guess size
        WorldPacket data(SMSG_GROUP_LIST, (6+8+8+1+2+m_members.size()*20));
        data << (uint8)m_groupType;
        data << (uint8)((m_bgGroup==true) ? 1 : 0);//2.0.x
                                                            // own flags (groupid | (assistant?0x80:0))
        data << (uint8)(citr->group | (citr->assistant?0x80:0));

        data << uint32(m_members.size()-1);
        for(member_citerator citr2 = m_members.begin(); citr2 != m_members.end(); ++citr2)
        {
            if(citr->guid == citr2->guid)
                continue;

            data << citr2->name;
            data << citr2->guid;
                                                            // online-state
            data << (uint8)(objmgr.GetPlayer(citr2->guid) ? 1 : 0);
                                                            // member flags
            data << (uint8)(citr2->group | (citr2->assistant?0x80:0));
        }

        data << m_leaderGuid;
        data << (uint8)m_lootMethod;
        data << m_looterGuid;
        data << (uint16)m_lootThreshold; // loot threshold
        data << m_mainTank;//2.0.x
        data << m_mainAssistant;//2.0.x

        player->GetSession()->SendPacket( &data );
    }
}

void Group::BroadcastPacket(WorldPacket *packet, int group, uint64 ignore)
{
    for(member_citerator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        if(ignore != 0 && itr->guid == ignore)
            continue;

        Player *pl = objmgr.GetPlayer(itr->guid);
        if (pl && pl->GetSession() && (group==-1 || itr->group==group))
            pl->GetSession()->SendPacket(packet);
    }
}

bool Group::_addMember(const uint64 &guid, const char* name, bool isAssistant)
{
    // get first not-full group
    uint8 groupid = 0;
    vector<uint8> temp(MAXRAIDSIZE/MAXGROUPSIZE);
    for(member_citerator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        if (itr->group >= temp.size()) continue;
        temp[itr->group]++;
        if(temp[groupid] >= MAXGROUPSIZE)
            groupid++;
    }

    return _addMember(guid, name, isAssistant, groupid);
}

bool Group::_addMember(const uint64 &guid, const char* name, bool isAssistant, uint8 group)
{
    if(IsFull())
        return false;

    if(!guid)
        return false;

    MemberSlot member;
    member.guid      = guid;
    member.name      = name;
    member.group     = group;
    member.assistant = isAssistant;
    m_members.push_back(member);

    Player *player = objmgr.GetPlayer(guid);
    if(player)
    {
        player->groupInfo.invite = NULL;
        player->groupInfo.group = this;
    }

    if(!isRaidGroup())                                      // reset targetIcons for non-raid-groups
    {
        for(int i=0; i<TARGETICONCOUNT; i++)
            m_targetIcons[i] = 0;
    }

    // insert into group table
    sDatabase.PExecute("INSERT INTO `group_member`(`leaderGuid`,`memberGuid`,`assistant`,`subgroup`) VALUES('%u','%u','%u','%u')", GUID_LOPART(m_leaderGuid), GUID_LOPART(member.guid), ((member.assistant==1)?1:0), member.group);

    return true;
}

bool Group::_removeMember(const uint64 &guid)
{
    Player *player = objmgr.GetPlayer(guid);
    if (player)
    {
        player->RemoveAreaAurasByOthers();
        player->RemoveAreaAurasFromGroup();
        player->groupInfo.group = NULL;
    }

    _removeRolls(guid);

    member_witerator slot = _getMemberWSlot(guid);
    if (slot != m_members.end())
        m_members.erase(slot);

    sDatabase.PExecute("DELETE FROM `group_member` WHERE `memberGuid`='%u'", GUID_LOPART(guid));

    if(m_leaderGuid == guid)                                // leader was removed
    {
        if(m_members.size() > 0)
            _setLeader(m_members.front().guid);
        return true;
    }

    return false;
}

void Group::_setLeader(const uint64 &guid)
{
    member_citerator slot = _getMemberCSlot(guid);
    if(slot==m_members.end())
        return;

    // instance system leader change process
    if (m_leaderGuid != slot->guid)
    {
        // here we must unbind all instances bound to that leader on group members from the
        // leader, and rebind them on the players
        uint32 old_guid = m_leaderGuid;
        uint32 new_guid = slot->guid;
        std::set< uint32 > changed_bindings;
        Player* player;
        BoundInstancesMap::iterator i_BoundInstances;

        // 1) rebind current associations in memory for group members
        // 2) get current group associations from database (for unbind)
        // combined because of query building in the iteration
        if (m_members.size() > 0)
        {
            std::ostringstream ss;
            ss << "SELECT DISTINCT(`map`) FROM `character_instance` WHERE (`guid` IN (";

            for(member_citerator citr = m_members.begin(); citr != m_members.end(); )
            {
                ss << GUID_LOPART(citr->guid);
                player = objmgr.GetPlayer(citr->guid);
                if(player && (citr->guid != old_guid))
                {
                    for(i_BoundInstances = player->m_BoundInstances.begin(); i_BoundInstances != player->m_BoundInstances.end(); i_BoundInstances++)
                    {
                        if (i_BoundInstances->second.second == GUID_LOPART(old_guid))
                        {
                            
                            i_BoundInstances->second.second = GUID_LOPART(new_guid);
                            changed_bindings.insert(i_BoundInstances->first);
                        }
                    }
                }
                ++citr;
                if (citr != m_members.end()) ss << ", ";
            }
            ss << ")) AND (`leader` = '" << GUID_LOPART(old_guid) << "')";
            QueryResult* result = sDatabase.Query(ss.str().c_str());
            if (result)
            {
                do
                {
                    Field* fields = result->Fetch();
                    changed_bindings.insert(fields[0].GetUInt32());
                } while(result->NextRow());
                delete result;
            }
        }

        // rebind changed associations in memory for old leader
        player = objmgr.GetPlayer(old_guid);
        if(player)
        {
            for (std::set< uint32 >::iterator i = changed_bindings.begin(); i != changed_bindings.end(); i++)
            {
                i_BoundInstances = player->m_BoundInstances.find(*i);
                if (i_BoundInstances != player->m_BoundInstances.end()) i_BoundInstances->second.second = GUID_LOPART(new_guid);
            }
        }

        // rebind changed associations in database
        if (changed_bindings.size() > 0)
        {
            std::ostringstream ss;
            ss << "UPDATE `character_instance` SET `leader` = '" << GUID_LOPART(new_guid) << "' WHERE (`map` IN (";
            {
                std::set< uint32 >::iterator i = changed_bindings.begin();
                while (i != changed_bindings.end())
                {
                    ss << "'" << *i << "'";
                    i++;
                    if (i != changed_bindings.end()) ss << ", ";
                }
            }
            ss << ")) AND (`leader` = '" << GUID_LOPART(old_guid) << "')";
            sDatabase.Execute(ss.str().c_str());
        }
    }
    
    sDatabase.BeginTransaction();
    sDatabase.PExecute("UPDATE `group` SET `leaderGuid`='%u' WHERE `leaderGuid`='%u'", GUID_LOPART(slot->guid), GUID_LOPART(m_leaderGuid));
    sDatabase.PExecute("UPDATE `group_member` SET `leaderGuid`='%u' WHERE `leaderGuid`='%u'", GUID_LOPART(slot->guid), GUID_LOPART(m_leaderGuid));
    sDatabase.CommitTransaction();

    m_leaderGuid = slot->guid;
    m_leaderName = slot->name;
}

void Group::_removeRolls(const uint64 &guid)
{
    vector<Roll>::iterator it;
    for (it = RollId.begin(); it < RollId.end(); it++)
    {
        map<uint64, RollVote>::iterator itr2 = it->playerVote.find(guid);
        if(itr2 == it->playerVote.end())
            continue;

        if (itr2->second == GREED) it->totalGreed--;
        if (itr2->second == NEED) it->totalNeed--;
        if (itr2->second == PASS) it->totalPass--;
        if (itr2->second != NOT_VALID) it->totalPlayersRolling--;

        it->playerVote.erase(itr2);

        CountRollVote(guid, it->itemGUID, m_members.size()-1, 3);
    }
}

void Group::_convertToRaid()
{
    m_groupType = GROUPTYPE_RAID;

    sDatabase.PExecute("UPDATE `group` SET `isRaid` = 1 WHERE `leaderGuid`='%u'", GUID_LOPART(m_leaderGuid));
}

bool Group::_setMembersGroup(const uint64 &guid, const uint8 &group)
{
    member_witerator slot = _getMemberWSlot(guid);
    if(slot==m_members.end())
        return false;

    slot->group = group;
    sDatabase.PExecute("UPDATE `group_member` SET `subgroup`='%u' WHERE `memberGuid`='%u'", group, GUID_LOPART(guid));
    return true;
}

bool Group::_setAssistantFlag(const uint64 &guid, const bool &state)
{
    member_witerator slot = _getMemberWSlot(guid);
    if(slot==m_members.end())
        return false;


    slot->assistant = state;
    sDatabase.PExecute("UPDATE `group_member` SET `assistant`='%u' WHERE `memberGuid`='%u'", (state==true)?1:0, GUID_LOPART(guid));
    return true;
}

bool Group::_setMainTank(const uint64 &guid)
{
    member_citerator slot = _getMemberCSlot(guid);
    if(slot==m_members.end())
        return false;

    if(m_mainAssistant == guid)
        _setMainAssistant(0);
    m_mainTank = guid;
    sDatabase.PExecute("UPDATE `group` SET `mainTank`='%u' WHERE `leaderGuid`='%u'", GUID_LOPART(m_mainTank), GUID_LOPART(m_leaderGuid));
    return true;
}

bool Group::_setMainAssistant(const uint64 &guid)
{
    member_witerator slot = _getMemberWSlot(guid);
    if(slot==m_members.end())
        return false;

    if(m_mainTank == guid)
        _setMainTank(0);
    m_mainAssistant = guid;
    sDatabase.PExecute("UPDATE `group` SET `mainAssistant`='%u' WHERE `leaderGuid`='%u'", GUID_LOPART(m_mainAssistant), GUID_LOPART(m_leaderGuid));
    return true;
}
