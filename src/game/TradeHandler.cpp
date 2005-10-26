/* TradeHandler.cpp
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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
//#include "Spell.h"
#include "Chat.h"

/*
TRADE OPCODES
 CMSG_ACCEPT_TRADE
 CMSG_BEGIN_TRADE
 CMSG_BUSY_TRADE
 CMSG_CANCEL_TRADE
 CMSG_CLEAR_TRADE_ITEM
 CMSG_IGNORE_TRADE
 CMSG_INITIATE_TRADE
 CMSG_SET_TRADE_GOLD
 CMSG_SET_TRADE_ITEM
 CMSG_UNACCEPT_TRADE

TRADE RESPONSE OPCODES
 SMSG_TRADE_STATUS
 SMSG_TRADE_STATUS_EXTENDED

void WorldSession::Handle XXX Opcode(WorldPacket& recvPacket)
{

}
*/

void WorldSession::HandleAcceptTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Accept Trade");
	recvPacket.print_storage();
}

void WorldSession::HandleBeginTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Begin Trade");
	recvPacket.print_storage();
}

void WorldSession::HandleBusyTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Busy Trade");
	recvPacket.print_storage();
}

void WorldSession::HandleCancelTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Cancel Trade");
	recvPacket.print_storage();
}

void WorldSession::HandleClearTradeItemOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Clear Trade Item");
	recvPacket.print_storage();
}

void WorldSession::HandleIgnoreTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Ignore Trade");
	recvPacket.print_storage();
}

void WorldSession::HandleInitiateTradeOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	uint64 ID;
	uint32 type;

	Log::getSingleton( ).outDebug( "WORLD: Initiate Trade");

	recvPacket >> ID;

#ifndef ENABLE_GRID_SYSTEM
    Player* pOther = objmgr.GetObject<Player>( ID );
#else
    Player* pOther = ObjectAccessor::Instance().FindPlayer( ID );
#endif

	if(!pOther)
	{
		//Player does not exists
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)6; //I dont have a target
		SendPacket(&data);
		return;        
	}
	
	//Send a MSG to player
	data.Initialize(SMSG_TRADE_STATUS);
	data << (uint32) 1; //Confirm
	data << (uint64) GetPlayer()->GetGUID();
	pOther->GetSession()->SendPacket(&data);

}

void WorldSession::HandleSetTradeGoldOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Set Trade Gold");
	recvPacket.print_storage();
}

void WorldSession::HandleSetTradeItemOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Set Trade Item");
	recvPacket.print_storage();
}

void WorldSession::HandleUnacceptTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Unaccept Trade");
	recvPacket.print_storage();
}