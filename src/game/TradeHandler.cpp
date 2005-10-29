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
#include "Item.h"
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


void WorldSession::HandleIgnoreTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Ignore Trade %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();
}

void WorldSession::HandleBusyTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Busy Trade %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();
}

void WorldSession::ClearTrade()
{
	GetPlayer()->tradeGold = 0;
	GetPlayer()->acceptTrade = false;
	for(int i=0; i<7; i++)
		GetPlayer()->tradeItems[i] = -1;
}

void WorldSession::UpdateTrade()
{
	WorldPacket data;
	Player *pThis = GetPlayer();
	Item *item = NULL;

	if( !pThis->pTrader ) return;

	data.Initialize(SMSG_TRADE_STATUS_EXTENDED);
	data << (uint8 ) 1;
	data << (uint32) 7;
	data << (uint32) 0;
	data << (uint32) GetPlayer()->tradeGold;
	data << (uint32) 0;
	for(int i=0; i<7; i++)
	{
		item = (pThis->tradeItems[i] >= 0 ? pThis->GetItemBySlot( (uint8) pThis->tradeItems[i] ) : NULL);
		
		data << (uint8) i;
		if(item)
		{
			data << (uint32) item->GetProto()->ItemId;
			data << (uint32) 0;
			data << (uint32) item->GetProto()->MaxCount;
		}
		else
		{
			data << (uint32) 0;
			data << (uint32) 0;
			data << (uint32) 0;
		}		
		for(int j=0; j<12; j++)
			data << (uint32) 0;
	}
	pThis->pTrader->GetSession()->SendPacket(&data);

}

void WorldSession::HandleAcceptTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Accept Trade %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();

	WorldPacket data;

	if ( !GetPlayer()->pTrader ) return;

	GetPlayer()->acceptTrade = true;

	if ( GetPlayer()->pTrader->acceptTrade )
	{
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)4;
		GetPlayer()->pTrader->GetSession()->SendPacket(&data);
		
		//Conditions
		
		if(false)
		{
			return;
		}

		//Do trade
			GetPlayer()->setGold( -((int) GetPlayer()->tradeGold) );			
			GetPlayer()->setGold( GetPlayer()->pTrader->tradeGold );

			GetPlayer()->pTrader->setGold( -((int) GetPlayer()->pTrader->tradeGold) );			
			GetPlayer()->pTrader->setGold( GetPlayer()->tradeGold );

		//Clear
		ClearTrade();

		//Trade Complete
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)8;
		GetPlayer()->pTrader->GetSession()->SendPacket(&data);
	
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)8;
		SendPacket(&data);

		GetPlayer()->pTrader->pTrader = NULL;
		GetPlayer()->pTrader = NULL;

	}
	else
	{
		//Accept trade
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)4;
		GetPlayer()->pTrader->GetSession()->SendPacket(&data);
	}

}

void WorldSession::HandleUnacceptTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Unaccept Trade %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();

	WorldPacket data;

	//Unaccept trade
	data.Initialize(SMSG_TRADE_STATUS);
	data << (uint32)7;
	GetPlayer()->pTrader->GetSession()->SendPacket(&data);

	GetPlayer()->acceptTrade = false;

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
	GetPlayer()->pTrader->GetSession()->ClearTrade();

	//Opens trade window to me
	data.Initialize(SMSG_TRADE_STATUS);
	data << (uint32)2; //Open trade window 
	SendPacket(&data);
	ClearTrade();
}

void WorldSession::HandleCancelTradeOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Cancel Trade %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();

	WorldPacket data;

	if( GetPlayer()->pTrader )
	{
		data.Initialize(SMSG_TRADE_STATUS);
		data << (uint32)3; //Trade Canceled
		GetPlayer()->pTrader->GetSession()->SendPacket(&data);
	}
	//Set the trader as NULL
	GetPlayer()->pTrader = NULL;
	ClearTrade();
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
	data << (uint32) 1; //Begin Trade request
	data << (uint64) GetPlayer()->GetGUID();
	GetPlayer()->pTrader->GetSession()->SendPacket(&data);
}

void WorldSession::HandleSetTradeGoldOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Set Trade Gold %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();

	uint32 gold;

	recvPacket >> gold;

	GetPlayer()->tradeGold = gold;

	UpdateTrade();

}

void WorldSession::HandleSetTradeItemOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Set Trade Item %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();
	
	uint8 tradeSlot;
	uint8 trash;
	uint8 bagSlot;

	recvPacket >> tradeSlot;
	recvPacket >> trash;
	recvPacket >> bagSlot;

	GetPlayer()->tradeItems[tradeSlot] = (int) bagSlot;

	UpdateTrade();
}

void WorldSession::HandleClearTradeItemOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "\nWORLD: Clear Trade Item %u", GetPlayer()->GetGUID());
	recvPacket.print_storage();

	uint8 slot;
	recvPacket >> slot;
	
	GetPlayer()->tradeItems[slot] = -1;

	UpdateTrade();
}