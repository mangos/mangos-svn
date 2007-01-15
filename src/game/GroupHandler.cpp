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
#include "Database/DatabaseEnv.h"
#include "Opcodes.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Group.h"
#include "ObjectAccessor.h"

/* differeces from off:
    -you can uninvite yourself
    -you can accept invitation even if leader went offline
*/
/* todo:
    -group_destroyed msg is sent but not shown
    -reduce xp gaining when in raid group
    -quest sharing has to be corrected
*/
void WorldSession::SendPartyResult(uint32 unk, std::string member, uint32 state)
{
    WorldPacket data(SMSG_PARTY_COMMAND_RESULT, (8+member.size()+1));
    data << unk;
    data << member;
    data << state;

    SendPacket( &data );
}

void WorldSession::HandleGroupInviteOpcode( WorldPacket & recv_data )
{
    std::string membername;
    recv_data >> membername;
    normalizePlayerName(membername);

    Player *player = objmgr.GetPlayer(membername.c_str());
    Group  *group = GetPlayer()->groupInfo.group;
    bool newGroup=false;

    /** error handling **/
    if(!player)
    {
        SendPartyResult(0, membername, 1);
        return;
    }

    if(!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION) && GetPlayer()->GetTeam() != player->GetTeam())
    {
        SendPartyResult(0, membername, 7);
        return;
    }

    if(!group)
    {
        group = new Group;
        group->Create(GetPlayer()->GetGUID(), GetPlayer()->GetName());
        objmgr.AddGroup(group);
        newGroup = true;
    }
    else
    {
        if(!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
        {
            SendPartyResult(0, "", 6);
            return;
        }

        if(group->IsFull())
        {
            SendPartyResult(0, "", 3);
            return;
        }
    }

    if(player->groupInfo.group || player->groupInfo.invite || player->HasInIgnoreList(GetPlayer()->GetGUID()))
    {
        if(newGroup)
        {
            group->Disband(true);
            objmgr.RemoveGroup(group);
            delete group;
            group = NULL;
        }

        if(player->groupInfo.group || player->groupInfo.invite)
        {
            SendPartyResult(0, membername, 4);
        }
        else
        {
            SendPartyResult(0, membername, 0);
            SendPartyResult(0, membername, 8);
        }

        return;
    }
    /********************/

    // everything's fine, do it
    group->AddInvite(player);

    WorldPacket data(SMSG_GROUP_INVITE, 10);                // guess size
    data << GetPlayer()->GetName();
    player->GetSession()->SendPacket(&data);

    SendPartyResult(0, membername, 0);
}

void WorldSession::HandleGroupAcceptOpcode( WorldPacket & recv_data )
{
    if(!GetPlayer()->groupInfo.invite)
        return;

    Group *group = GetPlayer()->groupInfo.invite;

    /** error handling **/
    /********************/

    // everything's fine, do it
    group->RemoveInvite(GetPlayer());
    group->AddMember(GetPlayer()->GetGUID(), GetPlayer()->GetName());
    GetPlayer()->groupInfo.group  = group;
}

void WorldSession::HandleGroupDeclineOpcode( WorldPacket & recv_data )
{
    if (!GetPlayer()->groupInfo.invite)
        return;

    Group  *group  = GetPlayer()->groupInfo.invite;
    Player *leader = objmgr.GetPlayer(group->GetLeaderGUID());

    /** error handling **/
    if(!leader || !leader->GetSession())
        return;
    /********************/

    // everything's fine, do it
    if(group->GetMembersCount() <= 1)                       // group has just 1 member => disband
    {
        group->Disband(true);
        objmgr.RemoveGroup(group);
        delete group;
        group = NULL;
    }

    GetPlayer()->groupInfo.invite = NULL;

    WorldPacket data( SMSG_GROUP_DECLINE, 10 );             // guess size
    data << GetPlayer()->GetName();
    leader->GetSession()->SendPacket( &data );
}

void WorldSession::HandleGroupUninviteGuidOpcode(WorldPacket & recv_data)
{
    uint64 guid;
    recv_data >> guid;

    std::string membername;
    objmgr.GetPlayerNameByGUID(guid, membername);

    HandleGroupUninvite(guid, membername);
}

void WorldSession::HandleGroupUninviteNameOpcode(WorldPacket & recv_data)
{
    std::string membername;
    recv_data >> membername;
    if(membername.size() <= 0)
        return;
    normalizePlayerName(membername);

    uint64 guid = objmgr.GetPlayerGUIDByName(membername.c_str());

    HandleGroupUninvite(guid, membername);
}

void WorldSession::HandleGroupUninvite(uint64 guid, std::string name)
{
    if(!GetPlayer()->groupInfo.group)
        return;

    Group *group = GetPlayer()->groupInfo.group;
    Player *player = objmgr.GetPlayer(guid);

    /** error handling **/
    if(!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
    {
        SendPartyResult(0, "", 6);
        return;
    }

    if(!group->IsMember(guid) && (player && player->groupInfo.invite != group))
    {
        SendPartyResult(0, name, 2);
        return;
    }
    /********************/

    // everything's fine, do it
    if(player && player->groupInfo.invite)                  // uninvite invitee
    {
        group->RemoveInvite(player);

        if(group->GetMembersCount() <= 1)                   // group has just 1 member => disband
        {
            group->Disband(true);
            objmgr.RemoveGroup(group);
            delete group;
            group = NULL;
        }
    }
    else                                                    // uninvite member
    {
        if (group->RemoveMember(guid, 1) <= 1)
        {
            group->Disband();
            objmgr.RemoveGroup(group);
            delete group;
            group = NULL;
        }
    }
}

void WorldSession::HandleGroupSetLeaderOpcode( WorldPacket & recv_data )
{
    if(!GetPlayer()->groupInfo.group)
        return;

    uint64 guid;
    recv_data >> guid;

    Group *group = GetPlayer()->groupInfo.group;
    Player *player = objmgr.GetPlayer(guid);

    /** error handling **/
    if (!player || !group->IsLeader(GetPlayer()->GetGUID()) || player->groupInfo.group != group)
        return;
    /********************/

    // everything's fine, do it
    group->ChangeLeader(guid);
}

void WorldSession::HandleGroupDisbandOpcode( WorldPacket & recv_data )
{
    if(!GetPlayer()->groupInfo.group)
        return;

    /** error handling **/
    /********************/

    // everything's fine, do it
    SendPartyResult(2, GetPlayer()->GetName(), 0);

    Group *group = GetPlayer()->groupInfo.group;
    if(group->RemoveMember(GetPlayer()->GetGUID(), 0) <= 1)
    {
        group->Disband();
        objmgr.RemoveGroup(group);
        delete group;
    }
}

void WorldSession::HandleLootMethodOpcode( WorldPacket & recv_data )
{
    if(!GetPlayer()->groupInfo.group)
        return;

    uint32 lootMethod;
    uint64 lootMaster;
    recv_data >> lootMethod >> lootMaster;

    Group *group = GetPlayer()->groupInfo.group;

    /** error handling **/
    if(!group->IsLeader(GetPlayer()->GetGUID()))
        return;
    /********************/

    // everything's fine, do it
    group->SetLootMethod((LootMethod)lootMethod);
    group->SetLooterGuid(lootMaster);
    group->SendUpdate();
}

void WorldSession::HandleLootRoll( WorldPacket &recv_data )
{
    if(!GetPlayer()->groupInfo.group)
        return;

    if(recv_data.size() < 13)
    {
        //sLog.outDebug("TOO SHORT LOOTROLL");
        return;
    }

    uint64 Guid;
    uint32 NumberOfPlayers;
    uint8  Choise;
    recv_data >> Guid;                                      //guid of the item rolled
    recv_data >> NumberOfPlayers;
    recv_data >> Choise;                                    //0: pass, 1: need, 2: greed

    //sLog.outDebug("WORLD RECIEVE CMSG_LOOT_ROLL, From:%u, Numberofplayers:%u, Choise:%u", (uint32)Guid, NumberOfPlayers, Choise);

    /** error handling **/
    /********************/

    // everything's fine, do it
    GetPlayer()->groupInfo.group->CountTheRoll(GetPlayer()->GetGUID(), Guid, NumberOfPlayers, Choise);
}

void WorldSession::HandleMinimapPingOpcode(WorldPacket& recv_data)
{
    if(!GetPlayer()->groupInfo.group)
        return;

    float x, y;
    recv_data >> x;
    recv_data >> y;

    //sLog.outDebug("Received opcode MSG_MINIMAP_PING X: %f, Y: %f", x, y);

    /** error handling **/
    /********************/

    // everything's fine, do it
    WorldPacket data(MSG_MINIMAP_PING, (8+4+4));
    data << GetPlayer()->GetGUID();
    data << x;
    data << y;
    GetPlayer()->groupInfo.group->BroadcastPacket(&data, -1, GetPlayer()->GetGUID());
}

void WorldSession::HandleRandomRollOpcode(WorldPacket& recv_data)
{
    uint32 minimum, maximum, roll;
    recv_data >> minimum;
    recv_data >> maximum;

    /** error handling **/
    if(minimum > maximum || maximum > 10000)                // < 32768 for urand call
        return;
    /********************/

    // everything's fine, do it
    roll = urand(minimum, maximum);

    //sLog.outDebug("ROLL: MIN: %u, MAX: %u, ROLL: %u", minimum, maximum, roll);

    WorldPacket data(MSG_RANDOM_ROLL, 24);
    data << minimum;
    data << maximum;
    data << roll;
    data << GetPlayer()->GetGUID();
    if(GetPlayer()->groupInfo.group)
        GetPlayer()->groupInfo.group->BroadcastPacket(&data);
    else
        SendPacket(&data);
}

void WorldSession::HandleRaidIconTargetOpcode( WorldPacket & recv_data )
{
    if(!GetPlayer()->groupInfo.group)
        return;

    uint8  x;
    recv_data >> x;

    /** error handling **/
    /********************/

    // everything's fine, do it
    if(x == 0xFF)                                           // target icon request
    {
        GetPlayer()->groupInfo.group->SendTargetIconList(this);
    }
    else                                                    // target icon update
    {
        if(!GetPlayer()->groupInfo.group->IsLeader(GetPlayer()->GetGUID()) && !GetPlayer()->groupInfo.group->IsAssistant(GetPlayer()->GetGUID()))
            return;

        uint64 guid;
        recv_data >> guid;
        GetPlayer()->groupInfo.group->SetTargetIcon(x, guid);
    }
}

void WorldSession::HandleRaidConvertOpcode( WorldPacket & recv_data )
{
    if(!GetPlayer()->groupInfo.group)
        return;

    Group *group = GetPlayer()->groupInfo.group;

    /** error handling **/
    if(!group->IsLeader(GetPlayer()->GetGUID()) || group->GetMembersCount() < 2)
        return;
    /********************/

    // everything's fine, do it
    SendPartyResult(0, "", 0);
    GetPlayer()->groupInfo.group->ConvertToRaid();
}

void WorldSession::HandleGroupChangeSubGroupOpcode( WorldPacket & recv_data )
{
    if(!GetPlayer()->groupInfo.group)
        return;

    std::string name;
    uint8 groupNr;
    recv_data >> name;
    recv_data >> groupNr;

    Group *group = GetPlayer()->groupInfo.group;
    uint64 guid = objmgr.GetPlayerGUIDByName(name.c_str());

    /** error handling **/
    if(!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
        return;
    /********************/

    // everything's fine, do it
    group->ChangeMembersGroup(guid, groupNr);
}

void WorldSession::HandleAssistantOpcode( WorldPacket & recv_data )
{
    if(!GetPlayer()->groupInfo.group)
        return;

    uint64 guid;
    uint8 flag;
    recv_data >> guid;
    recv_data >> flag;

    Group *group = GetPlayer()->groupInfo.group;

    /** error handling **/
    if(!group->IsLeader(GetPlayer()->GetGUID()))
        return;
    /********************/

    // everything's fine, do it
    group->ChangeAssistantFlag(guid, (flag==0?false:true));
}

void WorldSession::HandleRaidReadyCheckOpcode( WorldPacket & recv_data )
{
    if(!GetPlayer()->groupInfo.group)
        return;

    if(recv_data.size() == 0)                               // request
    {
        /** error handling **/
        if(!GetPlayer()->groupInfo.group->IsLeader(GetPlayer()->GetGUID()))
            return;
        /********************/

        // everything's fine, do it
        WorldPacket data(MSG_RAID_READY_CHECK, 0);
        GetPlayer()->groupInfo.group->BroadcastPacket(&data, -1, GetPlayer()->GetGUID());
    }
    else                                                    // answer
    {
        uint8 state;
        recv_data >> state;

        /** error handling **/
        /********************/

        // everything's fine, do it
        Player *leader = objmgr.GetPlayer(GetPlayer()->groupInfo.group->GetLeaderGUID());
        if(leader && leader->GetSession())
        {
            WorldPacket data(MSG_RAID_READY_CHECK, 9);
            data << GetPlayer()->GetGUID();
            data << state;
            leader->GetSession()->SendPacket(&data);
        }
    }
}

/*?*/void WorldSession::HandleRequestPartyMemberStatsOpcode( WorldPacket &recv_data )
{
    //sLog.outDebug("WORLD RECIEVE CMSG_REQUEST_PARTY_MEMBER_STATS");
    uint64 Guid;
    recv_data >> Guid;
    return;
    Player *player = objmgr.GetPlayer(Guid);
    if(!player)
        return;

    WorldPacket data(SMSG_PARTY_MEMBER_STATS, 30);
    /*data << (uint16)0xFF << Guid;
    data << (uint8)0;
    data << (uint32)(player ? 1 : 0);*/

    /*0000: 7e 00 xx xx xx xx ef 17 00 00 0b xx xx xx xx 01 : ~....M.......P..
    0010: e8 03 01 00 6b 01 86 fd 38 ef 01 00 00 00 99 09 : ....k...8.......
    0020: 01 00 86 20 00 -- -- -- -- -- -- -- -- -- -- -- : ... .*/

    /*0000: 7e 00 xx xx xx xx 10 12 00 00 xx xx 01 00 00 00 : ~..D.N....;.....
      0010: xx xx 00 -- -- -- -- -- -- -- -- -- -- -- -- -- : ...*/

    /* mask: 0b0000 0000 - 0000 0000 - 0000 0000 - 0000 0000
                       \ 
                      cur_life*/

    data.append(player->GetPackGUID());
    //data << (uint8)mask1 << (uint8)mask2;

    data << (uint8)0x10 << (uint8)0x10 << (uint16)0 << (uint16) 21 << (uint32)1 << (uint16)24244 << (uint8)0;
    SendPacket(&data);
}

/*!*/void WorldSession::HandleRequestRaidInfoOpcode( WorldPacket & recv_data )
{
    //sLog.outDebug("Received opcode CMSG_REQUEST_RAID_INFO");

    WorldPacket data(SMSG_RAID_INSTANCE_INFO, 4);
    data << (uint32)0;

    /*data << (uint32)count;
    for(int i=0; i<count; i++)
    {
        data << (uint32)mapid;
        data << (uint32)time_left_in_seconds;
        data << (uint32)instanceid;
    }*/

    GetPlayer()->GetSession()->SendPacket(&data);
}

/*void WorldSession::HandleGroupCancelOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: got CMSG_GROUP_CANCEL." );
}*/
