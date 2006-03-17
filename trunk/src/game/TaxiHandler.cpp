/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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


    sLog.outDebug( "WORLD: current location %u ",curloc);

    field = (uint8)((curloc - 1) / 32);
    submask = 1<<((curloc-1)%32);

    WorldPacket data;
    data.Initialize( SMSG_TAXINODE_STATUS );
    data << guid;

    
    if ( (GetPlayer( )->GetTaximask(field) & submask) != submask )
    {
        data << uint8( 0 );
    }
    else
    {
        data << uint8( 1 );
    }

    SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_TAXINODE_STATUS" );

}

void WorldSession::HandleTaxiQueryAviableNodesOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_TAXIQUERYAVAILABLENODES" );
    uint64 guid;
    uint32 curloc;
    uint8 field;
    uint32 TaxiMask[8];
    uint32 submask;

    recv_data >> guid;

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
        char buf[256];
        sprintf((char*)buf, "You discovered a new taxi vendor.");
        sChatHandler.FillSystemMessageData(&msg, GetPlayer()->GetSession(), buf);
        GetPlayer( )->GetSession( )->SendPacket( &msg );

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
        TaxiMask[i] &= GetPlayer( )->GetTaximask(i);
        data << TaxiMask[i];
    }
    SendPacket( &data );

    sLog.outDebug( "WORLD: Sent SMSG_SHOWTAXINODES" );
}


void WorldSession::HandleActivateTaxiOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_ACTIVATETAXI" );

    uint64 guid;
    uint32 sourcenode, destinationnode;
    uint32 path;
    uint32 cost;
    uint16 MountId;
    int32 newmoney;
    WorldPacket data;

    recv_data >> guid >> sourcenode >> destinationnode;

    if( GetPlayer( )->GetUInt32Value( UNIT_FIELD_FLAGS ) & 0x4 )
        return;

    objmgr.GetTaxiPath( sourcenode, destinationnode, path, cost);
    FlightPathMovementGenerator *flight(new FlightPathMovementGenerator(*_player, path));
    Path &pathnodes(flight->GetPath());
    assert( pathnodes.Size() > 0 );
    MountId = objmgr.GetTaxiMount(sourcenode);

    
    
    
    
    

    data.Initialize( SMSG_ACTIVATETAXIREPLY );

    
    if ( MountId == 0 || (path == 0 && cost == 0))
    {
        data << uint32( 1 );
        SendPacket( &data );
        return;
    }
    
    newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) - cost);
    if(newmoney < 0 )
    {
        data << uint32( 3 );
        SendPacket( &data );
        return;
    }
    GetPlayer( )->setDismountCost( newmoney );

    data << uint32( 0 );
    
    
    
    
    SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_ACTIVATETAXIREPLY" );

    GetPlayer( )->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID, MountId );
    GetPlayer( )->SetFlag( UNIT_FIELD_FLAGS ,0x000004 );
    GetPlayer( )->SetFlag( UNIT_FIELD_FLAGS, 0x002000 );
             
    uint32 traveltime = uint32(pathnodes.GetTotalLength( ) * 32);
    data.Initialize( SMSG_MONSTER_MOVE );
	data << uint8(0xFF);
    data << GetPlayer( )->GetGUID();  
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
