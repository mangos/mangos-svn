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

    WorldPacket data( SMSG_TAXINODE_STATUS, 9 );
    data << guid;

    if ( (GetPlayer( )->GetTaximask(field) & submask) != submask )
        data << uint8( 0 );
    else
        data << uint8( 1 );

    SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_TAXINODE_STATUS" );
}

void WorldSession::HandleTaxiQueryAvailableNodesOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_TAXIQUERYAVAILABLENODES" );

    uint64 guid;
    recv_data >> guid;

    // cheating checks
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

    // unknown taxi node case
    if( SendLearnNewTaxiNode(guid) )
        return;

    // known taxo node case
    SendTaxiMenu( guid );
}

void WorldSession::SendTaxiMenu( uint64 guid )
{
    // find current node
    uint32 curloc = objmgr.GetNearestTaxiNode(
        GetPlayer( )->GetPositionX( ),
        GetPlayer( )->GetPositionY( ),
        GetPlayer( )->GetPositionZ( ),
        GetPlayer( )->GetMapId( ) );

    if ( curloc == 0 )
        return;

    sLog.outDebug( "WORLD: CMSG_TAXINODE_STATUS_QUERY %u ",curloc);

    // check additinal requirements
    if(_player->IsMounted())
    {
        WorldPacket data(SMSG_CAST_RESULT, (4+1+1));
        data << uint32(0);
        data << uint8(2);
        data << uint8(CAST_FAIL_CANT_USE_WHEN_MOUNTED);
        SendPacket(&data);
        return;
    }

    WorldPacket data( SMSG_SHOWTAXINODES, (4+8+4+8*4) );
    data << uint32( 1 ) << guid;
    data << uint32( curloc );

    if(GetPlayer()->isTaxiCheater())
    {
        for (uint8 i=0; i<8; i++)
            data << uint32(0xFFFF);                         // all nodes
    }
    else
    {
        for (uint8 i=0; i<8; i++)
            data << uint32(GetPlayer()->GetTaximask(i));    // known nodes
    }
    SendPacket( &data );

    sLog.outDebug( "WORLD: Sent SMSG_SHOWTAXINODES" );
}

void WorldSession::SendDoFlight( uint16 MountId, uint32 path )
{
    GetPlayer( )->Mount( MountId, true );
    FlightPathMovementGenerator *flight(new FlightPathMovementGenerator(*_player, path));
    Path &pathnodes(flight->GetPath());
    assert( pathnodes.Size() > 0 );

    uint32 traveltime = uint32(pathnodes.GetTotalLength( ) * 32);
    WorldPacket data( SMSG_MONSTER_MOVE, (8+4+4+4+4+1+4+4+4+pathnodes.Size()*4*3) );
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

bool WorldSession::SendLearnNewTaxiNode( uint64 guid )
{
    // find current node
    uint32 curloc = objmgr.GetNearestTaxiNode(
        GetPlayer( )->GetPositionX( ),
        GetPlayer( )->GetPositionY( ),
        GetPlayer( )->GetPositionZ( ),
        GetPlayer( )->GetMapId( ) );

    if ( curloc == 0 )
        return true;                                        // `true` send to avoid WorldSession::SendTaxiMenu call with one more curlock seartch with same false result.

    uint8  field   = (uint8)((curloc - 1) / 32);
    uint32 submask = 1<<((curloc-1)%32);

    if ( (GetPlayer( )->GetTaximask(field) & submask) != submask )
    {
        GetPlayer()->SetTaximask(field, (submask | GetPlayer( )->GetTaximask(field)) );

        WorldPacket msg(SMSG_NEW_TAXI_PATH, 0);
        _player->GetSession()->SendPacket( &msg );

        WorldPacket update( SMSG_TAXINODE_STATUS, 9 );
        update << guid;
        update << uint8( 1 );
        SendPacket( &update );

        return true;
    } else
    return false;
}

void WorldSession::HandleActivateTaxiFarOpcode ( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_ACTIVATETAXI_FAR" );

    uint64 guid;
    uint32 node_count, _totalcost;

    recv_data >> guid >> _totalcost >> node_count;

    std::vector<uint32> nodes;

    for(uint32 i = 0; i < node_count; ++i)
    {
        uint32 node;
        recv_data >> node;
        nodes.push_back(node);
    }

    if(nodes.size() < 1)
        return;

    sLog.outDebug( "WORLD: Received CMSG_ACTIVATETAXI_FAR from %d to %d" ,nodes.front(),nodes.back());

    GetPlayer()->ActivateTaxiPathTo(nodes);
}

void WorldSession::HandleTaxiNextDestinationOpcode(WorldPacket& recvPacket)
{
    uint32 sourcenode,destinationnode;
    uint16 MountId;
    uint32 path, cost;

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

                WorldPacket data(SMSG_NEW_TAXI_PATH, 0);
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
    std::vector<uint32> nodes;
    nodes.resize(2);

    recv_data >> guid >> nodes[0] >> nodes[1];
    sLog.outDebug( "WORLD: Received CMSG_ACTIVATETAXI from %d to %d" ,nodes[0],nodes[1]);

    GetPlayer()->ActivateTaxiPathTo(nodes);
}
