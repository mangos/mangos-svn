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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "Path.h"
#include "Chat.h"
#include "WaypointMovementGenerator.h"
#include "DestinationHolderImp.h"

#include <cassert>

void WorldSession::HandleTaxiNodeStatusQueryOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_TAXINODE_STATUS_QUERY" );

    uint64 guid;

    recv_data >> guid;
    SendTaxiStatus( guid );
}

void WorldSession::SendTaxiStatus( uint64 guid )
{
    uint32 curloc;
    uint8 field;
    uint32 submask;

    curloc = objmgr.GetNearestTaxiNode(
        GetPlayer( )->GetPositionX( ),
        GetPlayer( )->GetPositionY( ),
        GetPlayer( )->GetPositionZ( ),
        GetPlayer( )->GetMapId( ) );

    // not found nearest
    if(curloc == 0)
        return;

    sLog.outDebug( "WORLD: current location %u ",curloc);

    field = (uint8)((curloc - 1) / 32);
    submask = 1<<((curloc-1)%32);

    WorldPacket data;
    data.Initialize( SMSG_TAXINODE_STATUS );
    data << guid;

    if ( (GetPlayer( )->GetTaximask(field) & submask) != submask )
        data << uint8( 0 );
    else
        data << uint8( 1 );

    SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_TAXINODE_STATUS" );
}

void WorldSession::HandleTaxiQueryAviableNodesOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_TAXIQUERYAVAILABLENODES" );

    uint64 guid;
    recv_data >> guid;
    SendTaxiMenu( guid );
}

void WorldSession::SendTaxiMenu( uint64 guid )
{
    uint32 curloc;
    uint8 field;
    uint32 TaxiMask[8];
    uint32 submask;

    if(_player->IsMounted())
    {
        WorldPacket data;
        data.Initialize(SMSG_CAST_RESULT);
        data << uint32(0);
        data << uint8(2);
        data << uint8(CAST_FAIL_CANT_USE_WHEN_MOUNTED);
        SendPacket(&data);
        return;
    }

    Creature *unit = ObjectAccessor::Instance().GetCreature(*GetPlayer(), guid);

    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleTaxiQueryAviableNodesOpcode - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    if(!unit->isTaxi())
        return;

    if(!unit->IsWithinDistInMap(_player,OBJECT_ITERACTION_DISTANCE))
        return;

    curloc = objmgr.GetNearestTaxiNode(
        GetPlayer( )->GetPositionX( ),
        GetPlayer( )->GetPositionY( ),
        GetPlayer( )->GetPositionZ( ),
        GetPlayer( )->GetMapId( ) );

    sLog.outDebug( "WORLD: CMSG_TAXINODE_STATUS_QUERY %u ",curloc);

    if ( curloc == 0 )
        return;

    field = (uint8)((curloc - 1) / 32);
    submask = 1<<((curloc-1)%32);

    if ( (GetPlayer( )->GetTaximask(field) & submask)
        != submask )
    {
        GetPlayer()->SetTaximask(field, (submask | GetPlayer( )->GetTaximask(field)) );

        WorldPacket msg;
        msg.Initialize(SMSG_NEW_TAXI_PATH);
        _player->GetSession()->SendPacket( &msg );

        WorldPacket update;
        update.Initialize( SMSG_TAXINODE_STATUS );
        update << guid;
        update << uint8( 1 );
        SendPacket( &update );
    }

    memset(TaxiMask, 0, sizeof(TaxiMask));
    if ( !objmgr.GetGlobalTaxiNodeMask( curloc, TaxiMask ) )
        return;
    TaxiMask[field] |= 1 << ((curloc-1)%32);

    WorldPacket data;
    data.Initialize( SMSG_SHOWTAXINODES );
    data << uint32( 1 ) << guid;
    data << uint32( curloc );
    for (uint8 i=0; i<8; i++)
    {
        TaxiMask[i] = 0xFFFFFFFF;

        // remove unknown nodes
        if(!GetPlayer()->isTaxiCheater())
            TaxiMask[i] &= GetPlayer()->GetTaximask(i);

        data << TaxiMask[i];
    }
    SendPacket( &data );

    sLog.outDebug( "WORLD: Sent SMSG_SHOWTAXINODES" );
}

void WorldSession::SendDoFlight( uint16 MountId, uint32 path )
{
    WorldPacket data;

    GetPlayer( )->Mount( MountId, true );
    FlightPathMovementGenerator *flight(new FlightPathMovementGenerator(*_player, path));
    Path &pathnodes(flight->GetPath());
    assert( pathnodes.Size() > 0 );

    uint32 traveltime = uint32(pathnodes.GetTotalLength( ) * 32);
    data.Initialize( SMSG_MONSTER_MOVE );
    data.append(GetPlayer()->GetPackGUID());
    data << GetPlayer( )->GetPositionX( )
        << GetPlayer( )->GetPositionY( )
        << GetPlayer( )->GetPositionZ( );
    data << GetPlayer( )->GetOrientation( );
    data << uint8( 0 );
    data << uint32( 0x00000300 );
    data << uint32( traveltime );
    data << uint32( pathnodes.Size( ) );
    data.append( (char*)pathnodes.GetNodes( ), pathnodes.Size( ) * 4 * 3 );

    //WPAssert( data.size() == 37 + pathnodes.Size( ) * 4 * 3 );
    GetPlayer()->SendMessageToSet(&data, true);
}

void WorldSession::HandleActivateTaxiFarOpcode ( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_ACTIVATETAXI_FAR" );

    uint64 guid;
    uint32 node_count, _totalcost;
    uint16 MountId;
    WorldPacket data;

    recv_data >> guid >> _totalcost >> node_count;

    // node_count = 1 (source node) + destination nodes

    uint32 sourcenode;
    recv_data >> sourcenode;

    // not let cheating with start flight mounted
    if(_player->IsMounted())
    {
        WorldPacket data;
        data.Initialize(SMSG_CAST_RESULT);
        data << uint32(0);
        data << uint8(2);
        data << uint8(CAST_FAIL_CANT_USE_WHEN_MOUNTED);
        SendPacket(&data);
        return;
    }

    uint32 curloc = objmgr.GetNearestTaxiNode(
        GetPlayer( )->GetPositionX( ),
        GetPlayer( )->GetPositionY( ),
        GetPlayer( )->GetPositionZ( ),
        GetPlayer( )->GetMapId( ) );

    // starting node != nearest node (cheat?)
    if(curloc != sourcenode)
    {
        data.Initialize( SMSG_ACTIVATETAXIREPLY );
        data << uint32( 4 );
        SendPacket( &data );
        return;
    }

    uint32 sourcepath = 0;
    uint32 totalcost = 0;

    uint32 prevnode = sourcenode;
    uint32 lastnode = 0;

    GetPlayer()->ClearTaxiDestinations();

    for(uint32 i = 1; i < node_count; ++i)
    {
        uint32 path, cost;

        recv_data >> lastnode;
        objmgr.GetTaxiPath( prevnode, lastnode, path, cost);

        if(!path)
            break;

        totalcost += cost;

        if(prevnode == sourcenode)
            sourcepath = path;

        GetPlayer()->AddTaxiDestination(lastnode);

        prevnode = lastnode;
    }

    sLog.outDebug( "WORLD: Received CMSG_ACTIVATETAXI_FAR from %d to %d" ,sourcenode,lastnode);

    if( GetPlayer( )->HasFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE ))
        return;

    MountId = objmgr.GetTaxiMount(sourcenode, GetPlayer()->GetTeam());

    data.Initialize( SMSG_ACTIVATETAXIREPLY );

    if ( MountId == 0 || sourcepath == 0)
    {
        data << uint32( 1 );
        SendPacket( &data );
        return;
    }

    uint32 money = GetPlayer()->GetMoney();
    if(money < totalcost )
    {
        data << uint32( 3 );
        SendPacket( &data );
        return;
    }

    // unsommon pet, it will be lost anyway
    GetPlayer( )->AbandonPet();

    //CHECK DONE, DO FLIGHT

    GetPlayer( )->SaveToDB();                               //For temporary avoid save player on air

    GetPlayer( )->setDismountCost( money - totalcost);

    data << uint32( 0 );

    SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_ACTIVATETAXIREPLY" );

    SendDoFlight( MountId, sourcepath );
}

void WorldSession::HandleTaxiNextDestinationOpcode(WorldPacket& recvPacket)
{
    uint32 sourcenode,destinationnode;
    uint16 MountId;
    uint32 path, cost;
    WorldPacket data;

    sLog.outDebug( "WORLD: Received CMSG_MOVE_SPLINE_DONE" );

    sourcenode      = GetPlayer()->GetTaxiSource();
    if ( sourcenode > 0 )                                   // if more destinations to go
    {
        // Add to taximask middle hubs in taxicheat mode (to prevent having player with disabled taxicheat and not having back flight path)
        if (GetPlayer()->isTaxiCheater())
        {
            uint8 field = (uint8)((sourcenode - 1) / 32);
            uint32 submask = 1<<((sourcenode-1)%32);
            if((GetPlayer( )->GetTaximask(field) & submask) != submask )
            {
                GetPlayer()->SetTaximask(field, (submask | GetPlayer( )->GetTaximask(field)) );

                data.Initialize(SMSG_NEW_TAXI_PATH);
                _player->GetSession()->SendPacket( &data );
            }
        }

        destinationnode = GetPlayer()->NextTaxiDestination();
        if ( destinationnode > 0 )
        {
            sLog.outDebug( "WORLD: Taxi has to go from %u to %u", sourcenode, destinationnode );

            MountId = objmgr.GetTaxiMount(sourcenode, GetPlayer()->GetTeam());
            objmgr.GetTaxiPath( sourcenode, destinationnode, path, cost);

            SendDoFlight( MountId, path );
        }
    }
}

void WorldSession::HandleActivateTaxiOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_ACTIVATETAXI" );

    uint64 guid;
    uint32 sourcenode, destinationnode;
    uint32 path;
    uint32 cost;
    uint16 MountId;
    WorldPacket data;

    recv_data >> guid >> sourcenode >> destinationnode;
    sLog.outDebug( "WORLD: Received CMSG_ACTIVATETAXI from %d to %d" ,sourcenode ,destinationnode);

    // not let cheating with start flight mounted
    if(_player->IsMounted())
    {
        WorldPacket data;
        data.Initialize(SMSG_CAST_RESULT);
        data << uint32(0);
        data << uint8(2);
        data << uint8(CAST_FAIL_CANT_USE_WHEN_MOUNTED);
        SendPacket(&data);
        return;
    }

    uint32 curloc = objmgr.GetNearestTaxiNode(
        GetPlayer( )->GetPositionX( ),
        GetPlayer( )->GetPositionY( ),
        GetPlayer( )->GetPositionZ( ),
        GetPlayer( )->GetMapId( ) );

    // starting node != nearest node (cheat?)
    if(curloc != sourcenode)
    {
        data.Initialize( SMSG_ACTIVATETAXIREPLY );
        data << uint32( 4 );
        SendPacket( &data );
        return;
    }

    if( GetPlayer( )->HasFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE ))
        return;

    // not let flight if casting not finished
    if(GetPlayer( )->m_currentSpell)
    {
        data.Initialize( SMSG_ACTIVATETAXIREPLY );
        data << uint32( 7 );
        SendPacket( &data );
        return;
    }

    objmgr.GetTaxiPath( sourcenode, destinationnode, path, cost);
    MountId = objmgr.GetTaxiMount(sourcenode, GetPlayer()->GetTeam());

    if ( MountId == 0 || path == 0 )
    {
        data.Initialize( SMSG_ACTIVATETAXIREPLY );
        data << uint32( 1 );
        SendPacket( &data );
        return;
    }

    uint32 money = GetPlayer()->GetMoney();
    if(money < cost )
    {
        data.Initialize( SMSG_ACTIVATETAXIREPLY );
        data << uint32( 3 );
        SendPacket( &data );
        return;
    }

    GetPlayer()->ClearTaxiDestinations();
    GetPlayer()->AddTaxiDestination(destinationnode);

    // unsommon pet, it will be lost anyway
    GetPlayer( )->AbandonPet();

    //CHECK DONE, DO FLIGHT

    GetPlayer( )->SaveToDB();                               //For temporary avoid save player on air

    GetPlayer( )->setDismountCost( money - cost);

    data.Initialize( SMSG_ACTIVATETAXIREPLY );
    data << uint32( 0 );

    SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_ACTIVATETAXIREPLY" );

    SendDoFlight( MountId, path );
}
