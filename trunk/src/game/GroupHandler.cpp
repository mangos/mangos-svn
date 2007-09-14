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
    -you can uninvite yourself - is is useful
    -you can accept invitation even if leader went offline
*/
/* todo:
    -group_destroyed msg is sent but not shown
    -reduce xp gaining when in raid group
    -quest sharing has to be corrected
    -FIX sending PartyMemberStats
*/

void WorldSession::SendPartyResult(PartyOperation operation, std::string member, PartyResult res)
{
    WorldPacket data(SMSG_PARTY_COMMAND_RESULT, (8+member.size()+1));
    data << (uint32)operation;
    data << member;
    data << (uint32)res;

    SendPacket( &data );
}

void WorldSession::HandleGroupInviteOpcode( WorldPacket & recv_data )
{
    std::string membername;
    recv_data >> membername;

    // attempt add selected player

    // cheating
    if(membername.empty())
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_CANT_FIND_TARGET);
        return;
    }

    normalizePlayerName(membername);
    Player *player = objmgr.GetPlayer(membername.c_str());

    // no player
    if(!player)
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_CANT_FIND_TARGET);
        return;
    }

    // can't group with
    if(!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP) && GetPlayer()->GetTeam() != player->GetTeam())
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_TARGET_UNFRIENDLY);
        return;
    }

    // just ignore us
    if(player->HasInIgnoreList(GetPlayer()->GetGUID()))
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_TARGET_IGNORE_YOU);
        return;
    }

    // player already in another group or invited
    if(player->GetGroup() || player->GetGroupInvite() )
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_ALREADY_IN_GROUP);
        return;
    }

    Group *group = GetPlayer()->GetGroup();

    if(group)
    {
        // not have permissions for invite
        if(!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
        {
            SendPartyResult(PARTY_OP_INVITE, "", PARTY_RESULT_YOU_NOT_LEADER);
            return;
        }

        // not have place
        if(group->IsFull())
        {
            SendPartyResult(PARTY_OP_INVITE, "", PARTY_RESULT_PARTY_FULL);
            return;
        }
    }

    // ok, but group not exist, creating new
    if(!group)
    {
        group = new Group;
        if(!group->Create(GetPlayer()->GetGUID(), GetPlayer()->GetName()))
        {
            delete group;
            return;
        }

        objmgr.AddGroup(group);

        // new group: if can't add then delete
        if(!group->AddInvite(player))
        {
            group->Disband(true);
            objmgr.RemoveGroup(group);
            delete group;
            return;
        }
    }
    else
    {
        // already existed group: if can't add then just leave
        if(!group->AddInvite(player))
        {
            return;
        }
    }

    // ok, we do it
    WorldPacket data(SMSG_GROUP_INVITE, 10);                // guess size
    data << GetPlayer()->GetName();
    player->GetSession()->SendPacket(&data);

    SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_OK);
}

void WorldSession::HandleGroupAcceptOpcode( WorldPacket & /*recv_data*/ )
{
    Group *group = GetPlayer()->GetGroupInvite();
    if (!group) return;

    // remove in from ivites in any case
    group->RemoveInvite(GetPlayer());

    /** error handling **/
    /********************/

    // not have place
    if(group->IsFull())
    {
        SendPartyResult(PARTY_OP_INVITE, "", PARTY_RESULT_PARTY_FULL);
        return;
    }

    // everything's fine, do it
    if(!group->AddMember(GetPlayer()->GetGUID(), GetPlayer()->GetName()))
        return;

    uint8 subgroup = group->GetMemberGroup(GetPlayer()->GetGUID());

    GetPlayer()->SetGroup(group, subgroup);
}

void WorldSession::HandleGroupDeclineOpcode( WorldPacket & /*recv_data*/ )
{
    Group  *group  = GetPlayer()->GetGroupInvite();
    if (!group) return;

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
    }

    GetPlayer()->SetGroupInvite(NULL);

    WorldPacket data( SMSG_GROUP_DECLINE, 10 );             // guess size
    data << GetPlayer()->GetName();
    leader->GetSession()->SendPacket( &data );
}

void WorldSession::HandleGroupUninviteGuidOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;

    std::string membername;
    if(!objmgr.GetPlayerNameByGUID(guid, membername))
        return;                                             // not found

    HandleGroupUninvite(guid, membername);
}

void WorldSession::HandleGroupUninviteNameOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,1);

    std::string membername;
    recv_data >> membername;
    if(membername.empty())
        return;
    normalizePlayerName(membername);

    uint64 guid = objmgr.GetPlayerGUIDByName(membername);

    // player not found
    if(!guid)
        return;

    HandleGroupUninvite(guid, membername);
}

void WorldSession::HandleGroupUninvite(uint64 guid, std::string name)
{
    Group *group = GetPlayer()->GetGroup();
    if(!group) return;

    Player *player = objmgr.GetPlayer(guid);

    /** error handling **/
    if(!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
    {
        SendPartyResult(PARTY_OP_LEAVE, "", PARTY_RESULT_YOU_NOT_LEADER);
        return;
    }

    if(!group->IsMember(guid) && (player && player->GetGroupInvite() != group))
    {
        SendPartyResult(PARTY_OP_LEAVE, name, PARTY_RESULT_NOT_IN_YOUR_PARTY);
        return;
    }
    /********************/

    // everything's fine, do it

    if(player && player->GetGroupInvite())                  // uninvite invitee
        player->UninviteFromGroup();
    else                                                    // uninvite member
        Player::RemoveFromGroup(group,guid);
}

void WorldSession::HandleGroupSetLeaderOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    Group *group = GetPlayer()->GetGroup();
    if(!group) return;

    uint64 guid;
    recv_data >> guid;

    Player *player = objmgr.GetPlayer(guid);

    /** error handling **/
    if (!player || !group->IsLeader(GetPlayer()->GetGUID()) || player->GetGroup() != group)
        return;
    /********************/

    // everything's fine, do it
    group->ChangeLeader(guid);
}

void WorldSession::HandleGroupDisbandOpcode( WorldPacket & /*recv_data*/ )
{
    if(!GetPlayer()->GetGroup())
        return;

    /** error handling **/
    /********************/

    // everything's fine, do it
    SendPartyResult(PARTY_OP_LEAVE, GetPlayer()->GetName(), PARTY_RESULT_OK);

    GetPlayer()->RemoveFromGroup();
}

void WorldSession::HandleLootMethodOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4+8+4);

    Group *group = GetPlayer()->GetGroup();
    if(!group) return;

    uint32 lootMethod;
    uint64 lootMaster;
    uint32 lootThreshold;
    recv_data >> lootMethod >> lootMaster >> lootThreshold;

    /** error handling **/
    if(!group->IsLeader(GetPlayer()->GetGUID()))
        return;
    /********************/

    // everything's fine, do it
    group->SetLootMethod((LootMethod)lootMethod);
    group->SetLooterGuid(lootMaster);
    group->SetLootThreshold((ItemQualities)lootThreshold);
    group->SendUpdate();
}

void WorldSession::HandleLootRoll( WorldPacket &recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4+1);

    if(!GetPlayer()->GetGroup())
        return;

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
    GetPlayer()->GetGroup()->CountRollVote(GetPlayer()->GetGUID(), Guid, NumberOfPlayers, Choise);
}

void WorldSession::HandleMinimapPingOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4+4);

    if(!GetPlayer()->GetGroup())
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
    GetPlayer()->GetGroup()->BroadcastPacket(&data, -1, GetPlayer()->GetGUID());
}

void WorldSession::HandleRandomRollOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4+4);

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
    if(GetPlayer()->GetGroup())
        GetPlayer()->GetGroup()->BroadcastPacket(&data);
    else
        SendPacket(&data);
}

void WorldSession::HandleRaidIconTargetOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,1);

    Group *group = GetPlayer()->GetGroup();
    if(!group)
        return;

    uint8  x;
    recv_data >> x;

    /** error handling **/
    /********************/

    // everything's fine, do it
    if(x == 0xFF)                                           // target icon request
    {
        group->SendTargetIconList(this);
    }
    else                                                    // target icon update
    {
        // recheck
        CHECK_PACKET_SIZE(recv_data,1+8);

        if(!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
            return;

        uint64 guid;
        recv_data >> guid;
        group->SetTargetIcon(x, guid);
    }
}

void WorldSession::HandleRaidConvertOpcode( WorldPacket & /*recv_data*/ )
{
    Group *group = GetPlayer()->GetGroup();
    if(!group) return;

    /** error handling **/
    if(!group->IsLeader(GetPlayer()->GetGUID()) || group->GetMembersCount() < 2)
        return;
    /********************/

    // everything's fine, do it (is it 0 (PARTY_OP_INVITE) correct code)
    SendPartyResult(PARTY_OP_INVITE, "", PARTY_RESULT_OK);
    group->ConvertToRaid();
}

void WorldSession::HandleGroupChangeSubGroupOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,1+1);

    Group *group = GetPlayer()->GetGroup();
    if(!group)
        return;

    std::string name;
    uint8 groupNr;
    recv_data >> name;

    // recheck
    CHECK_PACKET_SIZE(recv_data,(name.size()+1)+1);

    recv_data >> groupNr;

    /** error handling **/
    if(!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
        return;
    /********************/

    // everything's fine, do it
    group->ChangeMembersGroup(objmgr.GetPlayer(name.c_str()), groupNr);
}

void WorldSession::HandleGroupAssistantOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+1);

    Group *group = GetPlayer()->GetGroup();
    if(!group) return;

    uint64 guid;
    uint8 flag;
    recv_data >> guid;
    recv_data >> flag;

    /** error handling **/
    if(!group->IsLeader(GetPlayer()->GetGUID()))
        return;
    /********************/

    // everything's fine, do it
    group->SetAssistant(guid, (flag==0?false:true));
}

void WorldSession::HandleGroupPromoteOpcode( WorldPacket & recv_data )
{
    Group *group = GetPlayer()->GetGroup();
    if(!group) return;

    uint8 flag;
    uint64 guid;
    recv_data >> flag;
    recv_data >> guid;

    /** error handling **/
    if(!group->IsLeader(GetPlayer()->GetGUID()))
        return;
    /********************/

    // everything's fine, do it
    if(flag == 0)
        group->SetMainTank(guid);
    else if(flag == 1)
        group->SetMainAssistant(guid);
}

void WorldSession::HandleRaidReadyCheckOpcode( WorldPacket & recv_data )
{
    Group *group = GetPlayer()->GetGroup();
    if(!group) return;

    if(recv_data.empty())                               // request
    {
        /** error handling **/
        if(!group->IsLeader(GetPlayer()->GetGUID()))
            return;
        /********************/

        // everything's fine, do it
        WorldPacket data(MSG_RAID_READY_CHECK, 0);
        group->BroadcastPacket(&data, -1, GetPlayer()->GetGUID());
    }
    else                                                    // answer
    {
        uint8 state;
        recv_data >> state;

        /** error handling **/
        /********************/

        // everything's fine, do it
        Player *leader = objmgr.GetPlayer(group->GetLeaderGUID());
        if(leader && leader->GetSession())
        {
            WorldPacket data(MSG_RAID_READY_CHECK, 9);
            data << GetPlayer()->GetGUID();
            data << state;
            leader->GetSession()->SendPacket(&data);
        }
    }
}

void WorldSession::SendPartyMemberStatsChanged(uint64 Guid, uint32 mask)
{
    if (mask == GROUP_UPDATE_FLAG_NONE)
        return;

    Player *player = objmgr.GetPlayer(Guid);
    if(!player)                                             //currently do not send update if player is offline
        return;
    /*if(!player && mask != GROUP_UPDATE_FLAG_ONLINE) //if player is offline - then send nothing, but OFFLINE status
        return;*/

    if(mask & GROUP_UPDATE_FLAG_POWER_TYPE)                 // if update power type, update current/max power also
        mask |= (GROUP_UPDATE_FLAG_CUR_POWER | GROUP_UPDATE_FLAG_MAX_POWER);

    uint32 byteCount = 0;
    for (int i=1;i<GROUP_UPDATE_FLAGS_COUNT;++i)
        if (mask & 1<<i)
            byteCount += GroupUpdateLength[i];

    WorldPacket data(SMSG_PARTY_MEMBER_STATS, 8+4+byteCount);
    if (player)
        data.append(player->GetPackGUID());
    else
        data.appendPackGUID(Guid);

    data << (uint32) mask;

    if (mask & GROUP_UPDATE_FLAG_ONLINE)
    {
        if (player)
        {
            if (player->IsPvP())
                data << uint8(MEMBER_STATUS_ONLINE | MEMBER_STATUS_PVP);
            else
                data << (uint8) MEMBER_STATUS_ONLINE;
        }
        else
            data << (uint8) MEMBER_STATUS_OFFLINE;
    }

    if (mask & GROUP_UPDATE_FLAG_CUR_HP)
        data << (uint16) player->GetHealth();

    if (mask & GROUP_UPDATE_FLAG_MAX_HP)
        data << (uint16) player->GetMaxHealth();

    Powers powerType = player->getPowerType();
    if (mask & GROUP_UPDATE_FLAG_POWER_TYPE)
        data << (uint8) powerType;

    if (mask & GROUP_UPDATE_FLAG_CUR_POWER)
        data << (uint16) player->GetPower(powerType);

    if (mask & GROUP_UPDATE_FLAG_MAX_POWER)
        data << (uint16) player->GetMaxPower(powerType);

    if (mask & GROUP_UPDATE_FLAG_LEVEL)
        data << (uint16) player->getLevel();

    if (mask & GROUP_UPDATE_FLAG_ZONE)
        data << (uint16) player->GetZoneId();

    if (mask & GROUP_UPDATE_FLAG_POSITION)
        data << (uint16) player->GetPositionX() << (uint16) player->GetPositionY();

    //TODO: add missing group update flags (pets?)

    SendPacket(&data);
}

/*this procedure handles clients CMSG_REQUEST_PARTY_MEMBER_STATS request*/
void WorldSession::HandleRequestPartyMemberStatsOpcode( WorldPacket &recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDebug("WORLD RECIEVED CMSG_REQUEST_PARTY_MEMBER_STATS");
    uint64 Guid;
    recv_data >> Guid;

    Player *player = objmgr.GetPlayer(Guid);
    if(!player)
        return;
    WorldPacket data(SMSG_PARTY_MEMBER_STATS_FULL, 4+1+2+2+1+2*6+8+1+8);

    data.append(player->GetPackGUID());
    uint32 mask1 = 0x7FFC0BFF;                              // real flags used 0x000040BFF
    //0x7FFC1BFF;                              // for hunters etc (GROUP_UPDATE_FLAG_PET_MODEL_ID)
    Powers powerType = player->getPowerType();
    data << (uint32) mask1;                                 // group update mask
    data << (uint8)  MEMBER_STATUS_ONLINE;                  // member's online status
    data << (uint16) player->GetHealth();                   // GROUP_UPDATE_FLAG_CUR_HP
    data << (uint16) player->GetMaxHealth();                // GROUP_UPDATE_FLAG_MAX_HP
    data << (uint8)  powerType;                             // GROUP_UPDATE_FLAG_POWER_TYPE
    data << (uint16) player->GetPower(powerType);           // GROUP_UPDATE_FLAG_CUR_POWER
    data << (uint16) player->GetMaxPower(powerType);        // GROUP_UPDATE_FLAG_MAX_POWER
    data << (uint16) player->getLevel();                    // GROUP_UPDATE_FLAG_LEVEL
    data << (uint16) player->GetZoneId();                   // GROUP_UPDATE_FLAG_ZONE
    data << (uint16) player->GetPositionX();                // GROUP_UPDATE_FLAG_POSITION
    data << (uint16) player->GetPositionY();                // GROUP_UPDATE_FLAG_POSITION
    data << (uint64) 0xFF00000000000000LL;                  // GROUP_UPDATE_FLAG_AURAS
    //data << (uint16) player->GetPet()->GetUInt32Value(UNIT_FIELD_DISPLAYID);    // GROUP_UPDATE_FLAG_PET_MODEL_ID, for hunters etc...
    data << (uint8)  0x00;                                  // GROUP_UPDATE_FLAG_PET_NAME
    data << (uint64) 0xFF00000000000000LL;                  // GROUP_UPDATE_FLAG_PET_AURAS
    SendPacket(&data);
}

/*!*/void WorldSession::HandleRequestRaidInfoOpcode( WorldPacket & /*recv_data*/ )
{
    //sLog.outDebug("Received opcode CMSG_REQUEST_RAID_INFO");

    WorldPacket data(SMSG_RAID_INSTANCE_INFO, 4);
    data << (uint32)0;                                      // count (max is 10)

    /*data << (uint32)count;
    for(int i=0; i<count; i++)
    {
        data << (uint32)mapid;
        data << (uint32)time_left_in_seconds; // time to reset
        data << (uint32)instanceid;
        data << (uint32)0;      // unknown
    }*/

    GetPlayer()->GetSession()->SendPacket(&data);
}

/*void WorldSession::HandleGroupCancelOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: got CMSG_GROUP_CANCEL." );
}*/
