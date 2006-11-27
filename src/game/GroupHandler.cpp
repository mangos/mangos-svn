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

void WorldSession::SendToGroup(WorldPacket* data, bool to_self)
{
    Group *group = objmgr.GetGroupByLeader( GetPlayer()->GetGroupLeader() );

    if (group == NULL)
    {
        if(to_self)
            SendPacket(data);
        return;
    }

    for (uint32 i = 0; i < group->GetMembersCount(); i++)
    {
        Player* player = ObjectAccessor::Instance().FindPlayer( group->GetMemberGUID(i) );
        if (!player)
            continue;

        if(player == GetPlayer() && !to_self)
            continue;

        player->GetSession()->SendPacket( data );
    }
}

void WorldSession::HandleGroupInviteOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    std::string membername;
    Group *group;
    Player * player;

    recv_data >> membername;

    if(membername.size() == 0)
        return;

    normalizePlayerName(membername);

    player = objmgr.GetPlayer(membername.c_str());

    if ( player == NULL )
    {
        data.Initialize(SMSG_PARTY_COMMAND_RESULT);
        data << uint32( 0 );
        data << membername;
        data << uint32( 0x00000001 );

        SendPacket( &data );
        return;
    }

    if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION))
    {
        uint32 sidea = GetPlayer()->GetTeam();
        uint32 sideb = player->GetTeam();
        //This may be the right respons. It is the same as for if(player == null)
        if ( sidea != sideb )
        {
            data.Initialize(SMSG_PARTY_COMMAND_RESULT);
            data << uint32( 0 );
            data << membername;
            data << uint32( 0x00000001 );
            SendPacket( &data );
            return;
        }
    }

    if ( GetPlayer()->IsInGroup() && (GetPlayer()->GetGroupLeader() != GetPlayer()->GetGUID() ))
    {
        data.Initialize(SMSG_PARTY_COMMAND_RESULT);
        data << uint32( 0 );
        data << uint8( 0 );
        data << uint32( 0x00000006 );

        SendPacket( &data );
        return;
    }

    group = objmgr.GetGroupByLeader( GetPlayer()->GetGroupLeader() );
    if ( group != NULL )
    {
        if (group->IsFull())
        {
            data.Initialize(SMSG_PARTY_COMMAND_RESULT);
            data << uint32( 0 );
            data << uint8( 0 );
            data << uint32( 0x00000003 );

            SendPacket( &data );
            return;
        }
    }

    if ( player->IsInGroup() )
    {
        data.Initialize(SMSG_PARTY_COMMAND_RESULT);
        data << uint32( 0x0 );
        data << membername;
        data << uint32( 0x00000004 );

        SendPacket( &data );
        return;
    }

    if ( player->IsInvited() )
        return;

    data.Initialize(SMSG_GROUP_INVITE);
    data << GetPlayer()->GetName();

    player->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_PARTY_COMMAND_RESULT);
    data << uint32( 0x0 );
    data << membername;
    data << uint32( 0x00000000 );

    SendPacket( &data );

    player->SetLeader(GetPlayer()->GetGUID());
    player->SetInvited();

}

void WorldSession::HandleGroupCancelOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: got CMSG_GROUP_CANCEL." );
}

void WorldSession::HandleGroupAcceptOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    Player * player;

    player = ObjectAccessor::Instance().FindPlayer(GetPlayer()->GetGroupLeader());

    if ( !player )
        return;

    if ( !GetPlayer()->IsInvited() )
        return;

    if ( GetPlayer()->IsInGroup() )
    {
        data.Initialize(SMSG_PARTY_COMMAND_RESULT);
        data << uint32( 0x0 );
        data << GetPlayer()->GetName();
        data << uint32( 0x00000004 );
        SendPacket( &data );
        return;
    }

    GetPlayer()->UnSetInvited();

    if ( player->IsInGroup() && (player->GetGroupLeader() == player->GetGUID()) )
    {
        GetPlayer()->SetInGroup();

        Group *group = objmgr.GetGroupByLeader( GetPlayer()->GetGroupLeader() );
        ASSERT(group);

        group->AddMember( GetPlayer()->GetGUID(), GetPlayer()->GetName() );
        group->SendUpdate();

        return;
    }
    else if ( !player->IsInGroup() )
    {
        player->SetInGroup();
        player->SetLeader( player->GetGUID() );
        GetPlayer()->SetInGroup();
        GetPlayer()->SetLeader( player->GetGUID() );

        Group * group = new Group;
        ASSERT(group);

        if(!group->Create(player->GetGUID(), player->GetName()))
        {
            delete group;
            return;
        }

        group->AddMember(GetPlayer()->GetGUID(), GetPlayer()->GetName());
        objmgr.AddGroup(group);

        group->SendUpdate();
    }
}

void WorldSession::HandleGroupDeclineOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    if (!GetPlayer()->IsInvited())
        return;

    GetPlayer()->UnSetInvited();

    data.Initialize( SMSG_GROUP_DECLINE );
    data << GetPlayer()->GetName();
    Player *player = ObjectAccessor::Instance().FindPlayer(_player->GetGroupLeader());

    if ( !player )
        return;

    player->GetSession()->SendPacket( &data );
}

void WorldSession::HandleGroupUninviteOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    std::string membername;
    Group *group;
    Player * player;

    sLog.outDebug("WORLD: UNINVITE");

    recv_data >> membername;

    if(membername.size() == 0)
        return;

    normalizePlayerName(membername);

    player = objmgr.GetPlayer(membername.c_str());
    if ( player == NULL )
    {
        data.Initialize( SMSG_PARTY_COMMAND_RESULT );
        data << uint32( 0x0 );
        data << membername;
        data << uint32( 0x00000001 );

        sLog.outDebug("WORLD: UNINVITE: No player");

        SendPacket( &data );
        return;
    }

    if ( !GetPlayer()->IsInGroup() || (GetPlayer()->GetGroupLeader() != GetPlayer()->GetGUID()) )
    {
        data.Initialize( SMSG_PARTY_COMMAND_RESULT );
        data << uint32( 0x0 );
        data << uint8( 0 );
        data << uint32( 0x00000006 );

        SendPacket( &data );
        return;
    }

    if ( !player->IsInGroup() || (player->GetGroupLeader() != GetPlayer()->GetGroupLeader()) )
    {
        data.Initialize( SMSG_PARTY_COMMAND_RESULT );
        data << uint32( 0x0 );
        data << membername;
        data << uint32( 0x00000002 );

        SendPacket( &data );
        return;
    }

    group = objmgr.GetGroupByLeader(GetPlayer()->GetGroupLeader());

    if(group==NULL)
    {
        data.Initialize( SMSG_PARTY_COMMAND_RESULT );
        data << uint32( 0x0 );
        data << uint8(0);
        data << uint32( 0x00000006 );

        SendPacket( &data );
        return;
    }

    if (group->RemoveMember(player->GetGUID()) <= 1)
    {
        GetPlayer()->UnSetInGroup();

        group->Disband();
        objmgr.RemoveGroup(group);

        data.Initialize( SMSG_GROUP_DESTROYED );
        SendPacket( &data );

        group->SendUpdate();
        player->UnSetInGroup();
        data.Initialize( SMSG_GROUP_UNINVITE );
        player->GetSession()->SendPacket( &data );
        delete group;
        return;
    }

    group->SendUpdate();
    player->UnSetInGroup();
    data.Initialize( SMSG_GROUP_UNINVITE );
    player->GetSession()->SendPacket( &data );
}

void WorldSession::HandleGroupUninviteGuildOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: got CMSG_GROUP_UNINVITE_GUID." );
}

void WorldSession::HandleGroupSetLeaderOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    Group *group;
    Player * player;

    uint64 guid;
    recv_data >> guid;

    player = objmgr.GetPlayer(guid);

    if ( player == NULL )
    {
        data.Initialize( SMSG_PARTY_COMMAND_RESULT );
        data << uint32( 0x0 );
        data << uint8(0);
        data << uint32( 0x00000006 );

        SendPacket( &data );
        return;
    }

    if (!GetPlayer()->IsInGroup() ||
        (GetPlayer()->GetGroupLeader() != GetPlayer()->GetGUID()))
        return;

    if (!player->IsInGroup() || (player->GetGroupLeader() != GetPlayer()->GetGUID()))
        return;

    group = objmgr.GetGroupByLeader(GetPlayer()->GetGroupLeader());
    ASSERT(group);

    group->ChangeLeader(player->GetGUID());
}

void WorldSession::HandleGroupDisbandOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    sLog.outDebug("WORLD: GROUPDISBAND");

    if (!GetPlayer()->IsInGroup())
        return;

    Group *group;
    group = objmgr.GetGroupByLeader(GetPlayer()->GetGroupLeader());

    GetPlayer()->UnSetInGroup();

    sLog.outDebug( "GROUP: is in group?:%u",GetPlayer()->m_isInGroup);

    if(group==NULL)
    {
        sLog.outDetail("Not in a group");
        return;
    }

    if (group->RemoveMember(GetPlayer()->GetGUID()) > 1)
        group->SendUpdate();
    else
    {
        group->Disband();
        objmgr.RemoveGroup(group);
        data.Initialize( SMSG_GROUP_DESTROYED );
        SendPacket( &data );

        delete group;
    }

    data.Initialize( SMSG_GROUP_UNINVITE );
    SendPacket( &data );
}

void WorldSession::HandleLootMethodOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    uint32 lootMethod;
    uint64 lootMaster;

    Group *group;

    recv_data >> lootMethod >> lootMaster;

    group = objmgr.GetGroupByLeader(GetPlayer()->GetGroupLeader());
    if (group == NULL)
        return;

    group->SetLootMethod( LootMethod(lootMethod) );
    group->SetLooterGuid( lootMaster );
    group->SendUpdate();
}

void WorldSession::HandleLootRoll( WorldPacket &recv_data )
{
    uint64 Guid;
    uint32 NumberOfPlayers;
    uint8 Choise;
    recv_data >> Guid;                                      //guid of the item rolled
    recv_data >> NumberOfPlayers;
    recv_data >> Choise;                                    //0: pass, 1: need, 2: greed

    sLog.outDebug("WORLD RECIEVE CMSG_LOOT_ROLL, From:%u, Numberofplayers:%u, Choise:%u", (uint32)Guid, NumberOfPlayers, Choise);

    if (GetPlayer()->IsInGroup())
    {
        Group *group;
        group = objmgr.GetGroupByLeader(GetPlayer()->GetGroupLeader());
        group->CountTheRoll(GetPlayer()->GetGUID(), Guid, NumberOfPlayers, Choise);
    }
}

void WorldSession::HandleRequestPartyMemberStatsOpcode( WorldPacket &recv_data )
{
    sLog.outDebug("WORLD RECIEVE CMSG_REQUEST_PARTY_MEMBER_STATS");
    uint64 Guid;
    recv_data >> Guid;
    //TODO: send SMSG_PARTY_MEMBER_STATS
    //WorldPacket data;
    //data.Initialize(SMSG_PARTY_MEMBER_STATS);
    //data << uint8(0x0F);
    //data << Guid;
    //data << uint8(0);    //data << uint8(0x10);    //data << uint16(0x00);
    //SendPacket( &data );
    //sLog.outDebug("WORLD SEND SMSG_PARTY_MEMBER_STATS");
}

void WorldSession::HandleMinimapPingOpcode(WorldPacket& recv_data)
{
    float x, y;
    WorldPacket data;

    recv_data >> x;
    recv_data >> y;

    sLog.outDebug("Received opcode MSG_MINIMAP_PING X: %f, Y: %f", x, y);

    data.Initialize(MSG_MINIMAP_PING);
    data << _player->GetGUID();
    data << x;
    data << y;
    SendToGroup(&data, false);
}

void WorldSession::HandleRandomRollOpcode(WorldPacket& recv_data)
{
    sLog.outDebug("Received opcode MSG_RANDOM_ROLL");
    uint32 minimum, maximum, roll;
    WorldPacket data;

    recv_data >> minimum;
    recv_data >> maximum;

    if(minimum > maximum)
        return;

    if(maximum > 10000)                                     // < 32768 for urand call
        return;

    roll = urand(minimum, maximum);

    sLog.outDebug("ROLL: MIN: %u, MAX: %u, ROLL: %u", minimum, maximum, roll);

    data.Initialize(MSG_RANDOM_ROLL);
    data << minimum;
    data << maximum;
    data << roll;
    data << _player->GetGUID();
    SendToGroup( &data , true );
}

void WorldSession::HandleRaidIconTargetOpcode( WorldPacket & recv_data )
{
    /*
    receive from client: uint8 icon, uint64 guid
    server send to client:
    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 00 00 00 00 00 00 00 00 00 -- -- -- -- : !........... delete icon 0

    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 01 40 bb e0 01 00 00 00 00 -- -- -- -- : !...@....... set icon 1

    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 01 00 00 00 00 00 00 00 00 -- -- -- -- : !........... delete icon 1
    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 02 40 bb e0 01 00 00 00 00 -- -- -- -- : !...@....... set icon 2

    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 02 00 00 00 00 00 00 00 00 -- -- -- -- : !........... delete icon 2
    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 03 40 bb e0 01 00 00 00 00 -- -- -- -- : !...@....... set icon 3

    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 03 00 00 00 00 00 00 00 00 -- -- -- -- : !........... delete icon 3
    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 04 40 bb e0 01 00 00 00 00 -- -- -- -- : !...@....... set icon 4

    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 04 00 00 00 00 00 00 00 00 -- -- -- -- : !........... delete icon 4
    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 05 40 bb e0 01 00 00 00 00 -- -- -- -- : !...@....... set icon 5

    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 05 00 00 00 00 00 00 00 00 -- -- -- -- : !........... delete icon 5
    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 06 40 bb e0 01 00 00 00 00 -- -- -- -- : !...@....... set icon 6

    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 06 00 00 00 00 00 00 00 00 -- -- -- -- : !........... delete icon 6
    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 07 40 bb e0 01 00 00 00 00 -- -- -- -- : !...@....... set icon 7

    Packet SMSG.(null) (801), len: 12
    0000: 21 03 00 07 00 00 00 00 00 00 00 00 -- -- -- -- : !........... delete icon 7
    */
    sLog.outDebug("Received opcode MSG_RAID_ICON_TARGET");
    WorldPacket data;
    uint8 icon;
    uint64 guid;
    recv_data >> icon;                                      // icon
    recv_data >> guid;                                      // guid
    sLog.outDebug("Raid group icon %u for guid %u", icon, guid);
    if(guid == 0)                                           // none case
    {
        data.Initialize(MSG_RAID_ICON_TARGET);
        data << (uint8)0;                                   // may be used when in raid group?
        data << icon;                                       // icon to delete
        data << guid;                                       // 0
        SendToGroup(&data, true);
    }
    else
    {
        // FIXME: before adding new icon we must remove old if any
        //if(unit(get_by_guid)->GetRaidIcon()) (unit can be player, NPC, creature...)
        //{
        //    data.Initialize(MSG_RAID_ICON_TARGET);
        //    data << (uint8)0; // may be used when in raid group?
        //    data << unit(get_by_guid)->GetRaidIcon(); // delete current icon (if it already set), but how to get it?, need fix
        //    data << (uint64)0;
        //    SendToGroup(&data, true);
        //}
        data.Initialize(MSG_RAID_ICON_TARGET);
        data << (uint8)0;                                   // may be used when in raid group?
        data << icon;                                       // set new icon
        data << guid;
        SendToGroup(&data, true);
    }
}
