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
	 0: Target is busy
	 1: Begin Trade
	 2: Open trade window
	 3:	Trade canceled
	 4:	Accept trade
	 5:	Target is busy
	 6:	I dont have a target
	 7:	Unaccept trade
	 8:	Trade complete
	 9: ???
	10:	Trade target is too far away
	11:	Target is not party of your alliance
	12:	Close trade window
	13:	???
	14:	Target is ignoring you
	15:	You are stunned
	16:	Target is stunned
	17:	You cant do that when you are dead
	18:	You cant trade with dead players
	19:	You are loging out
	20:	Target is loging out
	21: Trial accounts cannot...

 SMSG_TRADE_STATUS_EXTENDED
	1: Send itens and gold?

*/

void WorldSession::HandleAcceptTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Accept Trade %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();
}

void WorldSession::HandleBeginTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Begin Trade %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();

	WorldPacket data;

	//Opens trade window to my partner
	data.Initialize(SMSG_TRADE_STATUS);
	data << (uint32)2; //Open trade window
	GetPlayer()->pTrader->GetSession()->SendPacket(&data);
	
	//Opens trade window to me
	data.Initialize(SMSG_TRADE_STATUS);
	data << (uint32)2; //Open trade window 
	SendPacket(&data);
}

void WorldSession::HandleBusyTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Busy Trade %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();
}

void WorldSession::HandleCancelTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Cancel Trade %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();

	WorldPacket data;

	//GetPlayer()->SetTarget( GetPlayer()->pTrader->GetGUID() );

	data.Initialize(SMSG_TRADE_STATUS);
	data << (uint32)3; //Trade Canceled
	GetPlayer()->pTrader->GetSession()->SendPacket(&data);

	//Set the trader as NULL
	GetPlayer()->pTrader->pTrader = NULL;
	GetPlayer()->pTrader = NULL;
}

void WorldSession::HandleClearTradeItemOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Clear Trade Item %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();
}

void WorldSession::HandleIgnoreTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Ignore Trade %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();
}

void WorldSession::HandleInitiateTradeOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	uint64 ID;
	uint32 type;

	Log::getSingleton( ).outDebug( "\nWORLD: Initiate Trade %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();

	if( !GetPlayer()->isAlive() )
	{
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)17; //You cant trade dead
		SendPacket(&data);
		return;        
	}

	if( isLogingOut() )
	{
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)19; //LogingOut
		SendPacket(&data);
		return;        
	}

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
	//Check if the trader is busy
	if( pOther->pTrader )
	{
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)0; //Target is busy...
		SendPacket(&data);
		return;    
	}
	//You cant trade with dead players
	if( !pOther->isAlive() )
	{
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)18; //You cant trade with dead players
		SendPacket(&data);
		return;        
	}
	//Player is LogingOut
	if( pOther->GetSession()->isLogingOut() )
	{
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)20; //Player is LogingOut
		SendPacket(&data);
		return;        
	}
	//Check the distance
/*	if( pOther->getdistance( ??? TODO ) > ??? )
	{
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)10; //Trade target is too far away
		SendPacket(&data);
		return;    
	}
	//Check factions
	if( pOther->faction != GetPlayer()->faction ??? )
	{
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)11; //Target is not party of your alliance
		SendPacket(&data);
		return;    
	}
*/

	GetPlayer()->pTrader = pOther; 
	pOther->pTrader = GetPlayer();    

	//Send a MSG to player
	data.Initialize(SMSG_TRADE_STATUS);
	data << (uint32) 1; //Confirmation message
	data << (uint64) GetPlayer()->GetGUID();
	GetPlayer()->pTrader->GetSession()->SendPacket(&data);
}

void WorldSession::HandleSetTradeGoldOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Set Trade Gold %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();

	WorldPacket data;
	uint32 gold;

	recvPacket >> gold;
	
	static unsigned int T = 0;

	data.Initialize(SMSG_TRADE_STATUS);
	data << (uint32)T; 
	data << (uint32) gold;
	SendPacket(&data);
	
	T++;
}

void WorldSession::HandleSetTradeItemOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Set Trade Item %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();
}

void WorldSession::HandleUnacceptTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Unaccept Trade %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();
}