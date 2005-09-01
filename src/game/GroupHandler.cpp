/* GroupHandler.cpp
 *
 * Copyright (C) 2004 Wow Daemon
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

//////////////////////////////////////////////////////////////
/// This function handles CMSG_GROUP_INVITE
//////////////////////////////////////////////////////////////
void WorldSession::HandleGroupInviteOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    std::string membername;
    Group *group;
    Player * player;

    recv_data >> membername;

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


///////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_CANCEL:
///////////////////////////////////////////////////////////////
void WorldSession::HandleGroupCancelOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: got CMSG_GROUP_CANCEL." );
}


////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_ACCEPT:
////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupAcceptOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    Player * player;

    player = objmgr.GetObject<Player>( GetPlayer()->GetGroupLeader() );
    if ( !player )
        return;

    if ( !GetPlayer()->IsInvited() )
        return;

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

// creating group
        Group * group = new Group;
        ASSERT(group);

        group->Create(player->GetGUID(), player->GetName());

// adding our client
        group->AddMember(GetPlayer()->GetGUID(), GetPlayer()->GetName());
        objmgr.AddGroup(group);

        group->SendUpdate();
    }
}


///////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_DECLINE:
//////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupDeclineOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    if (!GetPlayer()->IsInvited())
        return;

    GetPlayer()->UnSetInvited();

    data.Initialize( SMSG_GROUP_DECLINE );
    data << GetPlayer()->GetName();

    Player *player = objmgr.GetObject<Player>( GetPlayer()->GetGroupLeader() );
    if ( !player )
        return;

    player->GetSession()->SendPacket( &data );
}


//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_UNINVITE:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupUninviteOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    std::string membername;
    Group *group;
    Player * player;

    recv_data >> membername;

    player = objmgr.GetPlayer(membername.c_str());
    if ( player == NULL )
    {
        data.Initialize( SMSG_PARTY_COMMAND_RESULT );
        data << uint32( 0x0 );
        data << membername;
        data << uint32( 0x00000001 );

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

    if (group->RemoveMember(player->GetGUID()) < 1)
    {
        GetPlayer()->UnSetInGroup();
        objmgr.RemoveGroup(group);

        data.Initialize( SMSG_GROUP_DESTROYED );
        SendPacket( &data );

        delete group;
    }

    group->SendUpdate();
    player->UnSetInGroup();
    data.Initialize( SMSG_GROUP_UNINVITE );
    player->GetSession()->SendPacket( &data );
}


//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_UNINVITE_GUID:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupUninviteGuildOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: got CMSG_GROUP_UNINVITE_GUID." );
}


//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_SET_LEADER:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupSetLeaderOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    std::string membername;
    Group *group;
    Player * player;

    recv_data >> membername;

    player = objmgr.GetPlayer(membername.c_str());

    if ( player == NULL )
    {
        data.Initialize( SMSG_PARTY_COMMAND_RESULT );
        data << uint32( 0x0 );
        data << membername;
        data << uint32( 0x00000001 );

        SendPacket( &data );
        return;
    }

// error player is not leader
    if (!GetPlayer()->IsInGroup() ||
        (GetPlayer()->GetGroupLeader() != GetPlayer()->GetGUID()))
        return;

// error player not in group
    if (!player->IsInGroup() || (player->GetGroupLeader() != GetPlayer()->GetGUID()))
        return;

    group = objmgr.GetGroupByLeader(GetPlayer()->GetGroupLeader());
    ASSERT(group);

    group->ChangeLeader(player->GetGUID());
}


//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_DISBAND:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupDisbandOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

// error he's not in a group
    if (!GetPlayer()->IsInGroup())
        return;

    GetPlayer()->UnSetInGroup();

    Group *group;
    group = objmgr.GetGroupByLeader(GetPlayer()->GetGroupLeader());

    if(group==NULL)
    {
        printf("Not in a group\n");
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


//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_LOOT_METHOD:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleLootMethodOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    uint32 lootMethod;
    uint64 lootMaster;

    Group *group;

    recv_data >> lootMethod >> lootMaster;

    group = objmgr.GetGroupByLeader(GetPlayer()->GetGroupLeader());
    if (group == NULL)
        return;                                   // shouldn't get here

    group->SetLootMethod( lootMethod );
    group->SetLooterGuid( lootMaster );
    group->SendUpdate();
}
