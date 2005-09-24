/* TaxiHandler.cpp
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

void WorldSession::HandleTaxiNodeStatusQueryOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_TAXINODE_STATUS_QUERY" );

    uint64 guid;
    uint32 curloc;
    uint8 field;
    uint32 submask;

    recv_data >> guid;

    curloc = objmgr.GetNearestTaxiNode(
        GetPlayer( )->GetPositionX( ),
        GetPlayer( )->GetPositionY( ),
        GetPlayer( )->GetPositionZ( ),
        GetPlayer( )->GetMapId( ) );


	Log::getSingleton( ).outDebug( "WORLD: CMSG_TAXINODE_STATUS_QUERY %u ",curloc);

    field = (uint8)((curloc - 1) / 32);
    submask = 1<<((curloc-1)%32);

    WorldPacket data;
    data.Initialize( SMSG_TAXINODE_STATUS );
    data << guid;

    // Check for known nodes
    if ( (GetPlayer( )->GetTaximask(field) & submask) != submask )
    {
        data << uint8( 0 );
    }
    else
    {
        data << uint8( 1 );
    }

    SendPacket( &data );
    Log::getSingleton( ).outDebug( "WORLD: Sent SMSG_TAXINODE_STATUS" );
}


void WorldSession::HandleTaxiQueryAviableNodesOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_TAXIQUERYAVAILABLENODES" );
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

	Log::getSingleton( ).outDebug( "WORLD: CMSG_TAXINODE_STATUS_QUERY %u ",curloc);

    if ( curloc == 0 )
        return;

    field = (uint8)((curloc - 1) / 32);
    submask = 1<<((curloc-1)%32);

    // Check for known nodes
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

    // A 256bit bitmask representing taxi nodes ... position of the bit = taxinodeID
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

    Log::getSingleton( ).outDebug( "WORLD: Sent SMSG_SHOWTAXINODES" );
}


void WorldSession::HandleActivateTaxiOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_ACTIVATETAXI" );

    uint64 guid;
    uint32 sourcenode, destinationnode;
    Path pathnodes;
    uint32 path;
    uint32 cost;
    uint16 MountId;
    int32 newmoney;
    WorldPacket data;

    recv_data >> guid >> sourcenode >> destinationnode;

    if( GetPlayer( )->GetUInt32Value( UNIT_FIELD_FLAGS ) & 0x4 )
        return;

    objmgr.GetTaxiPath( sourcenode, destinationnode, path, cost);
	objmgr.GetTaxiPathNodes( path, &pathnodes );
    MountId = objmgr.GetTaxiMount(sourcenode);

    // MOUNTDISPLAYID
    // bat: 1566
    // gryph: 1147
    // wyvern: 295
    // hippogryph: 479

    data.Initialize( SMSG_ACTIVATETAXIREPLY );

    // Check for valid node
    if ( MountId == 0 || (path == 0 && cost == 0))
    {
        data << uint32( 1 );
        SendPacket( &data );
        return;
    }
    // Check for gold
    newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) - cost);
    if(newmoney < 0 )
    {
        data << uint32( 3 );
        SendPacket( &data );
        return;
    }
    GetPlayer( )->setDismountCost( newmoney );

    data << uint32( 0 );
    // 0 Ok
    // 1 Unspecified Server Taxi Error
    // 2.There is no direct path to that direction
    // 3 Not enough Money
    SendPacket( &data );
    Log::getSingleton( ).outDebug( "WORLD: Sent SMSG_ACTIVATETAXIREPLY" );

    GetPlayer( )->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID, MountId );
    GetPlayer( )->SetFlag( UNIT_FIELD_FLAGS ,0x000004 );
    GetPlayer( )->SetFlag( UNIT_FIELD_FLAGS, 0x002000 );

    // 0x001000 seems to make a mount visible
    // 0x002000 seems to make you sit on the mount, and the mount move with you
    // 0x000004 locks you so you can't move, no msg_move updates are sent to the server
    // 0x000008 seems to enable detailed collision checking

    // 36.7407
    uint32 traveltime = uint32(pathnodes.getTotalLength( ) * 32);
	//uint32 traveltime = uint32(pathnodes.getTotalLength( ) * 16);

    GetPlayer()->setMountPos( pathnodes.getNodes( )[ pathnodes.getLength( ) - 1 ].x,
        pathnodes.getNodes( )[ pathnodes.getLength( ) - 1 ].y,
        pathnodes.getNodes( )[ pathnodes.getLength( ) - 1 ].z );

    data.Initialize( SMSG_MONSTER_MOVE );
    data << GetPlayer( )->GetGUID();
    data << GetPlayer( )->GetPositionX( )
        << GetPlayer( )->GetPositionY( )
        << GetPlayer( )->GetPositionZ( );
    data << GetPlayer( )->GetOrientation( );
    data << uint8( 0 );
    data << uint32( 0x00000300 );
    data << uint32( traveltime );
    data << uint32( pathnodes.getLength( ) );
    data.append( (char*)pathnodes.getNodes( ), pathnodes.getLength( ) * 4 * 3 );

    WPAssert( data.size() == 37 + pathnodes.getLength( ) * 4 * 3 );

    GetPlayer()->SendMessageToSet(&data, true);
    GetPlayer()->setDismountTimer((uint32)(traveltime*(0.53)));
}
