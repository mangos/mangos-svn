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

void Group::Create(const uint64 &guid, const char * name)
{
    m_leaderGuid = guid;
    m_leaderName = name;

    m_groupType  = GROUPTYPE_NORMAL;
    m_lootMethod = GROUP_LOOT;

    AddMember(guid, name);
}

void Group::LoadRaidGroupFromDB(const uint64 &leaderGuid)
{
    QueryResult *result = sDatabase.PQuery("SELECT `lootMethod`,`looterGuid`,`icon1`,`icon2`,`icon3`,`icon4`,`icon5`,`icon6`,`icon7`,`icon8` FROM `raidgroup` WHERE `leaderGuid`='%u'", GUID_LOPART(leaderGuid));
    if(!result)
        return;

    m_leaderGuid = leaderGuid;
    objmgr.GetPlayerNameByGUID(m_leaderGuid, m_leaderName);
    m_groupType  = GROUPTYPE_RAID;
    m_lootMethod = (LootMethod)(*result)[0].GetUInt8();
    m_looterGuid = (*result)[1].GetUInt64();

    for(int i=0; i<TARGETICONCOUNT; i++)
        m_targetIcons[i] = (*result)[2+i].GetUInt8();
    delete result;


    result = sDatabase.PQuery("SELECT `memberGuid`,`assistant`,`subgroup` FROM `raidgroup_member` WHERE `leaderGuid`='%u'", GUID_LOPART(leaderGuid));
    if(!result)
        return;    

    do
    {
        MemberSlot member;
        member.guid      = (*result)[0].GetUInt64();
        objmgr.GetPlayerNameByGUID(member.guid, member.name);
        member.group     = (*result)[2].GetUInt8();
        member.assistant = (*result)[1].GetBool();
        m_members.push_back(member);
    } while( result->NextRow() );
    delete result;
}

bool Group::AddInvite(Player *player)
{
    if(!player || player->groupInfo.invite || player->groupInfo.group)
        return false;

    RemoveInvite(player->GetGUID());

    m_invitees.push_back(player->GetGUID());

    player->groupInfo.invite = this;

    return true;
}

void Group::RemoveInvite(const uint64 &guid)
{
    for(vector<uint64>::iterator itr=m_invitees.begin(); itr!=m_invitees.end(); itr++)
    {
        if((*itr) == guid)
        {
            m_invitees.erase(itr);
            break;
        }
    }

    Player *player = objmgr.GetPlayer(guid);
    if(player)
        player->groupInfo.invite = NULL;
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
    if(m_members.size() > 1)
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
            data<<(uint16)0<<(uint32)0<<(uint64)0;
            player->GetSession()->SendPacket(&data);
        }

        if(leaderChanged)
        {
            WorldPacket data(SMSG_GROUP_SET_LEADER, (m_members[0].name.size()+1));
            data << m_members[0].name;
            BroadcastPacket(&data);  
        }

        SendUpdate();
    }
    else
        Disband(true);

    return m_members.size();
}

void Group::ChangeLeader(const uint64 &guid)
{    
    _setLeader(guid);

    WorldPacket data(SMSG_GROUP_SET_LEADER, (m_members[_getMemberIndex(guid)].name.size()+1));
    data << m_members[_getMemberIndex(guid)].name;
    BroadcastPacket(&data);  
    SendUpdate();
}

void Group::Disband(bool hideDestroy)
{
    Player *player;
    
    for(vector<MemberSlot>::const_iterator citr=m_members.begin(); citr!=m_members.end(); citr++)
    {
        player = objmgr.GetPlayer(citr->guid);
        if(!player || !player->GetSession())
            continue;
        
        player->RemoveAreaAurasByOthers();
        player->RemoveAreaAurasFromGroup();
        player->groupInfo.group = NULL;

        WorldPacket data;
        if(!hideDestroy)
        {
            data.Initialize(SMSG_GROUP_DESTROYED, 0);
            player->GetSession()->SendPacket(&data);
        }

        data.Initialize(SMSG_GROUP_LIST, 14);
        data<<(uint16)0<<(uint32)0<<(uint64)0;
        player->GetSession()->SendPacket(&data);
    }
    RollId.clear();
    m_members.clear();

    for(vector<uint64>::iterator itr=m_invitees.begin(); itr!=m_invitees.end(); itr++)
    {
        Player *invitee = objmgr.GetPlayer((*itr));
        if(invitee)
            invitee->groupInfo.invite = NULL;
    }
    m_invitees.clear();


    if(isRaidGroup())
    {
        sDatabase.BeginTransaction();
        sDatabase.PExecute("DELETE FROM `raidgroup` WHERE `leaderGuid`='%u'", GUID_LOPART(m_leaderGuid));
        sDatabase.PExecute("DELETE FROM `raidgroup_member` WHERE `leaderGuid`='%u'", GUID_LOPART(m_leaderGuid));
        sDatabase.CommitTransaction();
    }

    m_leaderGuid = 0;
    m_leaderName = "";
}

void Group::SendLootStartRoll(uint64 Guid, uint32 NumberinGroup, uint32 ItemEntry, uint32 ItemInfo, uint32 CountDown, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_START_ROLL, (8+4+4+4+4+4));
    data << Guid;                                           // guid of the creature/go that has the item which is being rolled for
    data << NumberinGroup;                                  // maybe the number of players rolling for it???
    data << ItemEntry;                                      // the itemEntryId for the item that shall be rolled for
    data << uint32(0);                                      // unknown
    data << ItemInfo;                                       // ItemInfo this is related to special additionals to a item
    data << CountDown;                                      // the countdown time to choose "need" or "greed"

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

void Group::SendLootRoll(uint64 SourceGuid, uint64 TargetGuid, uint32 ItemEntry, uint32 ItemInfo, uint8 RollNumber, uint8 RollType, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ROLL, (8+4+8+4+4+4+1+1));
    data << SourceGuid;                                     // guid of the item rolled
    data << (uint32)0;                                      // unknown, maybe amount of players
    data << TargetGuid;
    data << ItemEntry;                                      // the itemEntryId for the item that shall be rolled for
    data << uint32(0);                                      // unknown
    data << ItemInfo;                                       // ItemInfo this is related to special additionals to a item
    data << RollNumber;                                     // 0: "Need for: [item name]" > 127: "you passed on: [item name]"      Roll number
    data << RollType;                                       // 0: "Need for: [item name]" 0: "You have selected need for [item name] 1: need roll 2: greed roll

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

void Group::SendLootRollWon(uint64 SourceGuid, uint64 TargetGuid, uint32 ItemEntry, uint32 ItemInfo, uint8 RollNumber, uint8 RollType, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ROLL_WON, (8+4+4+4+4+1+1));
    data << SourceGuid;                                     // guid of the item rolled
    data << (uint32)0;                                      // unknown, maybe amount of players
    data << ItemEntry;                                      // the itemEntryId for the item that shall be rolled for
    data << uint32(0);                                      // unknown
    data << ItemInfo;                                       // ItemInfo this is related to special additionals to a item
    data << TargetGuid;                                     // guid of the player who won.
    data << RollNumber;                                     // rollnumber realted to SMSG_LOOT_ROLL
    data << RollType;                                       // Rolltype related to SMSG_LOOT_ROLL

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

void Group::SendLootAllPassed(uint64 Guid, uint32 NumberOfPlayers, uint32 ItemEntry, uint32 ItemInfo, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ALL_PASSED, (8+4+4+4+4));
    data << Guid;                                           // Guid of the item rolled
    data << NumberOfPlayers;                                // The number of players rolling for it???
    data << ItemEntry;                                      // The itemEntryId for the item that shall be rolled for
    data << ItemInfo;                                       // ItemInfo
    data << uint32(0x3F3);                                  // unknown, I think it can be number of roll

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
    uint8 THRESHOLD = 1;
    Player *player = objmgr.GetPlayer(playerGUID);
    Group *group = player->groupInfo.group;

    for (i=loot->items.begin(); i != loot->items.end(); i++, itemSlot++)
    {
        item = objmgr.GetItemPrototype(i->itemid);
        if (!item)
        {
            sLog.outDebug("Group::GroupLoot: missing item prototype for item with id: %d", i->itemid);
            continue;
        }
        if (item->Quality > THRESHOLD)
        {
            Roll r;
            uint32 newitemGUID = objmgr.GenerateLowGuid(HIGHGUID_ITEM);
            r.itemGUID = newitemGUID;
            r.itemid = i->itemid;

            //a vector is filled with only near party members
            for (int j = 0; j < m_members.size(); j++)
            {
                Player *member = objmgr.GetPlayer(m_members[j].guid);
                if(!member || !member->GetSession())
                    continue;

                if (member->GetDistance2dSq(creature) < sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE))
                {
                    r.playerVote[m_members[j].guid] = NOT_EMITED_YET;
                    r.totalPlayersRolling++;
                }
            }

            group->SendLootStartRoll(newitemGUID, r.totalPlayersRolling, i->itemid, 0, 60000, r);

            r.loot = loot;
            r.itemSlot = itemSlot;
            loot->items[itemSlot].is_blocked = true;

            RollId.push_back(r);
        }
    }
}

void Group::NeedBeforeGreed(uint64 playerGUID, Loot *loot, Creature *creature)
{
    vector<LootItem>::iterator i;
    ItemPrototype const *item;
    uint8 itemSlot = 0;
    uint8 THRESHOLD = 1;
    Player *player = objmgr.GetPlayer(playerGUID);
    Group *group = player->groupInfo.group;

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
            for (int j = 0; j < m_members.size(); j++)
            {
                Player *playerToRoll = objmgr.GetPlayer(m_members[j].guid);
                if(!playerToRoll || !playerToRoll->GetSession())
                    continue;

                if (playerToRoll->CanUseItem(item))
                {
                    if (playerToRoll->GetDistance2dSq(creature) < sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE))
                    {
                        r.playerVote[m_members[j].guid] = NOT_EMITED_YET;
                        r.totalPlayersRolling++;
                    }
                }
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
            map<uint64, RollVote>::iterator itr2 = i->playerVote.find(playerGUID);
            // this condition means that player joins to the party after roll begins
            if (itr2 == i->playerVote.end())
                return;

            if (i->loot)
                if (i->loot->items.size() == 0)
                    return;

            switch (Choise)
            {
                case 0:                                     //Player choose pass
                {
                    SendLootRoll(0, playerGUID, i->itemid, 0, 128, 0, *i);
                    i->totalPass++;
                    itr2->second = PASS;
                }
                break;
                case 1:                                     //player choose Need
                {
                    SendLootRoll(0, playerGUID, i->itemid, 0, 1, 0, *i);
                    i->totalNeed++;
                    itr2->second = NEED;
                }
                break;
                case 2:                                     //player choose Greed
                {
                    i->totalGreed++;
                    itr2->second = GREED;
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
                    uint64 maxguid  = (*i->playerVote.begin()).first;
                    Player *player;

                    map<uint64, RollVote>::iterator itr2;
                    for (itr2=i->playerVote.begin(); itr2!=i->playerVote.end(); itr2++)
                    {
                        if (itr2->second != NEED)
                            continue;

                        uint8 randomN = rand() % 100;
                        SendLootRoll(0, itr2->first, i->itemid, 0, randomN, 1, *i);
                        if (maxresul < randomN)
                        {
                            maxguid  = itr2->first;
                            maxresul = randomN;
                        }
                    }
                    SendLootRollWon(0, maxguid, i->itemid, 0, maxresul, 1, *i);
                    player = objmgr.GetPlayer(maxguid);

                    if(player && player->GetSession())
                    {
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
                }
                else
                {
                    if (i->totalGreed > 0)
                    {
                        uint8 maxresul = 0;
                        uint64 maxguid = (*i->playerVote.begin()).first;
                        Player *player;

                        map<uint64, RollVote>::iterator itr2;
                        for (itr2=i->playerVote.begin(); itr2!=i->playerVote.end(); itr2++)
                        {
                            if (itr2->second != GREED)
                                continue;

                            uint8 randomN = rand() % 100;
                            SendLootRoll(0, itr2->first, i->itemid, 0, randomN, 1, *i);
                            if (maxresul < randomN)
                            {
                                maxguid  = itr2->first;
                                maxresul = randomN;
                            }
                        }
                        SendLootRollWon(0, maxguid, i->itemid, 0, maxresul, 2, *i);
                        player = objmgr.GetPlayer(maxguid);

                        if(player && player->GetSession())
                        {
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

bool Group::IsMember(uint64 guid)
{
    Player *player = objmgr.GetPlayer(guid);

    if(player)
        return (player->groupInfo.group == this);
    else
    {
        if(!isRaidGroup())
            return false;

        QueryResult *result = sDatabase.PQuery("SELECT `leaderGuid` FROM `raidgroup_member` WHERE `memberGuid`='%u' AND `leaderGuid`='%u'", GUID_LOPART(guid), GUID_LOPART(GetLeaderGUID()));
        if(result)
        {
            delete result;
            return true;
        }
        else
            return false;
    }
}

void Group::SendInit(WorldSession *session)
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
        WorldPacket data(SMSG_GROUP_LIST, (2+4+8+8+1+2+m_members.size()*20)); // guess size
        data << (uint8)m_groupType;
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

        session->SendPacket( &data );
    }
}

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
    WorldPacket data;

    for(vector<MemberSlot>::const_iterator citr=m_members.begin(); citr!=m_members.end(); citr++)
    {
        player = objmgr.GetPlayer(citr->guid);
        if(!player || !player->GetSession())
            continue;

        data.Initialize(SMSG_GROUP_LIST, (6+8+8+1+2+m_members.size()*20)); // guess size
        data << (uint8)m_groupType;
        data << (uint8)(citr->group | (citr->assistant?0x80:0));        // own flags (groupid | (assistant?0x80:0))

        data << uint32(m_members.size()-1);
        for(vector<MemberSlot>::const_iterator citr2=m_members.begin(); citr2!=m_members.end(); citr2++)
        {
            if(citr == citr2)
                continue;

            data << citr2->name;
            data << citr2->guid;
            data << (uint8)(objmgr.GetPlayer(citr2->guid) ? 1 : 0);         // online-state
            data << (uint8)(citr2->group | (citr2->assistant?0x80:0));      // member flags
        }

        data << m_leaderGuid;
        data << (uint8)m_lootMethod;
        data << m_looterGuid;
        data << (uint16)2;

        player->GetSession()->SendPacket( &data );
    }
}

void Group::BroadcastPacket(WorldPacket *packet, int group, uint64 ignore)
{
    for (uint32 i = 0; i < m_members.size(); i++)
    {
        if(ignore != 0 && m_members[i].guid == ignore)
            continue;

        Player *pl = objmgr.GetPlayer(m_members[i].guid);
        if (pl && pl->GetSession() && (group==-1 || m_members[i].group==group))
            pl->GetSession()->SendPacket(packet);
    }
}


bool Group::_addMember(const uint64 &guid, const char* name, bool isAssistant)
{
    // get first not-full group
    uint8 groupid = 0;
    vector<uint8> temp(MAXRAIDSIZE/MAXGROUPSIZE);
    for(int i=0; i<m_members.size(); i++)
    {
        temp[m_members[i].group]++;
        if(temp[groupid] >= MAXGROUPSIZE)
            groupid++;
    }

    return _addMember(guid, name, isAssistant, groupid);
}

bool Group::_addMember(const uint64 &guid, const char* name, bool isAssistant, uint8 group)
{
    if(IsFull())
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

    if(!isRaidGroup())  // reset targetIcons for non-raid-groups
    {
        for(int i=0; i<TARGETICONCOUNT; i++)
            m_targetIcons[i] = 0;
    }
    else                // insert into raid table..
        sDatabase.PExecute("INSERT INTO `raidgroup_member`(`leaderGuid`,`memberGuid`,`assistant`,`subgroup`) VALUES('%u','%u','%u','%u')", GUID_LOPART(m_leaderGuid), GUID_LOPART(member.guid), ((member.assistant==1)?1:0), member.group);

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
    m_members.erase(m_members.begin()+_getMemberIndex(guid));
    if(isRaidGroup())
        sDatabase.PExecute("DELETE FROM `raidgroup_member` WHERE `memberGuid`='%u'", GUID_LOPART(guid));

    if(m_leaderGuid == guid) // leader was removed
    {
        if(m_members.size() > 0)
            _setLeader(m_members[0].guid);
        return true;
    }

    return false;
}

void Group::_setLeader(const uint64 &guid)
{
    int8 id = _getMemberIndex(guid);
    if(id < 0)
        return;

    if(isRaidGroup())
    {            
        sDatabase.BeginTransaction();
        sDatabase.PExecute("UPDATE `raidgroup` SET `leaderGuid`='%u' WHERE `leaderGuid`='%u'", GUID_LOPART(m_members[id].guid), GUID_LOPART(m_leaderGuid));
        sDatabase.PExecute("UPDATE `raidgroup_member` SET `leaderGuid`='%u' WHERE `leaderGuid`='%u'", GUID_LOPART(m_members[id].guid), GUID_LOPART(m_leaderGuid));
        sDatabase.CommitTransaction();
    }
    m_leaderGuid = m_members[id].guid;
    m_leaderName = m_members[id].name;
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

        CountTheRoll(guid, it->itemGUID, m_members.size()-1, 3);
    }
}

void Group::_convertToRaid()
{
    m_groupType = GROUPTYPE_RAID;

    sDatabase.BeginTransaction();
    sDatabase.PExecute("DELETE FROM `raidgroup` WHERE `leaderGuid`='%u'", GUID_LOPART(m_leaderGuid));
    sDatabase.PExecute("DELETE FROM `raidgroup_member` WHERE `leaderGuid`='%u'", GUID_LOPART(m_leaderGuid));
    sDatabase.PExecute("INSERT INTO `raidgroup`(`leaderGuid`,`lootMethod`,`looterGuid`,`icon1`,`icon2`,`icon3`,`icon4`,`icon5`,`icon6`,`icon7`,`icon8`) VALUES('%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u')", GUID_LOPART(m_leaderGuid), m_lootMethod, GUID_LOPART(m_looterGuid), m_targetIcons[0], m_targetIcons[1], m_targetIcons[2], m_targetIcons[3], m_targetIcons[4], m_targetIcons[5], m_targetIcons[6], m_targetIcons[7]);

    for(vector<MemberSlot>::const_iterator citr=m_members.begin(); citr!=m_members.end(); citr++)
        sDatabase.PExecute("INSERT INTO `raidgroup_member`(`leaderGuid`,`memberGuid`,`assistant`,`subgroup`) VALUES('%u','%u','%u','%u')", GUID_LOPART(m_leaderGuid), GUID_LOPART(citr->guid), (citr->assistant==1)?1:0, citr->group);
    sDatabase.CommitTransaction();
}

bool Group::_setMembersGroup(const uint64 &guid, const uint8 &group)
{
    int8 i = _getMemberIndex(guid);
    if(i < 0)
        return false;

    m_members[i].group = group; 
    sDatabase.PExecute("UPDATE `raidgroup_member` SET `subgroup`='%u' WHERE `memberGuid`='%u'", group, GUID_LOPART(guid));
    return true;
}

bool Group::_setAssistantFlag(const uint64 &guid, const bool &state)
{
    int8 i = _getMemberIndex(guid);
    if(i < 0)
        return false;

    m_members[i].assistant = state; 
    sDatabase.PExecute("UPDATE `raidgroup_member` SET `assistant`='%u' WHERE `memberGuid`='%u'", (state==true)?1:0, GUID_LOPART(guid));
    return true;
}

int8 Group::_getMemberIndex(uint64 Guid)
{
    for (int i=0; i<m_members.size(); i++)
    {
        if (m_members[i].guid == Guid)
            return i;
    }
    return -1;
}