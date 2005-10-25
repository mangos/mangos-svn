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
#include "Spell.h"

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

}

void WorldSession::HandleBeginTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Begin Trade");

}

void WorldSession::HandleBusyTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Busy Trade");

}

void WorldSession::HandleCancelTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Cancel Trade");

}

void WorldSession::HandleClearTradeItemOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Clear Trade Item");

}

void WorldSession::HandleIgnoreTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Ignore Trade");

}

void WorldSession::HandleInitiateTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Initiate Trade");

}

void WorldSession::HandleSetTradeGoldOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Set Trade Gold");

}

void WorldSession::HandleSetTradeItemOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Set Trade Item");

}

void WorldSession::HandleUnacceptTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Unaccept Trade");

}