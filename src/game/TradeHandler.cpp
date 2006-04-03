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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
#include "Item.h"
#include "Chat.h"

void WorldSession::HandleIgnoreTradeOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug( "\nWORLD: Ignore Trade %u", GetPlayer()->GetGUID());
    recvPacket.print_storage();
}

void WorldSession::HandleBusyTradeOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug( "\nWORLD: Busy Trade %u", GetPlayer()->GetGUID());
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
    WorldPacket data;
    Item *myItems[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
    Item *hisItems[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
    int i, myCount = 0, hisCount = 0, myFreeSlots = 0, hisFreeSlots = 0;
    

    if ( !GetPlayer()->pTrader ) return;

    GetPlayer()->acceptTrade = true;

    if ( GetPlayer()->pTrader->acceptTrade )
    {
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)4;
        GetPlayer()->pTrader->GetSession()->SendPacket(&data);
        
        
        for(i=0; i<6; i++)
        {
            
            if( GetPlayer()->tradeItems[i] >= INVENTORY_SLOT_ITEM_START ) myCount++;
            if( GetPlayer()->pTrader->tradeItems[i] >= INVENTORY_SLOT_ITEM_START ) hisCount++;
        }
        
        myFreeSlots = GetPlayer()->CountFreeBagSlot();
        hisFreeSlots = GetPlayer()->pTrader->CountFreeBagSlot();        

        

        
        if( (myCount + myFreeSlots) < hisCount )
        {    
            sChatHandler.FillSystemMessageData(&data, GetPlayer()->GetSession(), "You do not have enough free slots");
            GetPlayer( )->GetSession( )->SendPacket( &data );

            sChatHandler.FillSystemMessageData(&data, GetPlayer()->pTrader->GetSession(), "Your partner do not have enough free bag slots");
            GetPlayer( )->pTrader->GetSession( )->SendPacket( &data );

            GetPlayer()->GetSession()->HandleUnacceptTradeOpcode(recvPacket);
            GetPlayer()->pTrader->GetSession()->HandleUnacceptTradeOpcode(recvPacket);
            return;
        }
        
        if( (hisCount + hisFreeSlots) < myCount )
        {
            sChatHandler.FillSystemMessageData(&data, GetPlayer()->GetSession(), "Your partner do not have enough free bag slots");
            GetPlayer( )->GetSession( )->SendPacket( &data );

            sChatHandler.FillSystemMessageData(&data, GetPlayer()->pTrader->GetSession(), "You do not have enough free slots");
            GetPlayer( )->pTrader->GetSession( )->SendPacket( &data );

            GetPlayer()->GetSession()->HandleUnacceptTradeOpcode(recvPacket);
            GetPlayer()->pTrader->GetSession()->HandleUnacceptTradeOpcode(recvPacket);
            return;
        }
        

        
            GetPlayer()->setGold( -((int) GetPlayer()->tradeGold) );            
            GetPlayer()->setGold( GetPlayer()->pTrader->tradeGold );

            GetPlayer()->pTrader->setGold( -((int) GetPlayer()->pTrader->tradeGold) );            
            GetPlayer()->pTrader->setGold( GetPlayer()->tradeGold );

            
            for(i=0; i<6; i++)
            {
                if( GetPlayer()->tradeItems[i] >= 0 )
                    myItems[i] = GetPlayer()->RemoveItemFromSlot(0, (uint8) GetPlayer()->tradeItems[i],true );
                if( GetPlayer()->pTrader->tradeItems[i] >= 0)
                    hisItems[i] = GetPlayer()->pTrader->RemoveItemFromSlot(0, (uint8) GetPlayer()->pTrader->tradeItems[i], true );
            }
            
            for(i=0; i<6; i++)
            {
                if(hisItems[i])
				{
					
					hisItems[i]->SetUInt64Value( ITEM_FIELD_GIFTCREATOR, GetPlayer()->pTrader->GetGUID());

                    GetPlayer()->AddItemToInventory(0, NULL_SLOT, hisItems[i], false, false, false);
				}
                if(myItems[i])
				{
					
					myItems[i]->SetUInt64Value( ITEM_FIELD_GIFTCREATOR, GetPlayer()->GetGUID());

                    GetPlayer()->pTrader->AddItemToInventory(0, NULL_SLOT, myItems[i], false, false, false);
				}
            }

        

        
        ClearTrade();

        
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
        
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)4;
        GetPlayer()->pTrader->GetSession()->SendPacket(&data);
    }

}

void WorldSession::HandleUnacceptTradeOpcode(WorldPacket& recvPacket)
{
    WorldPacket data;

    
    data.Initialize(SMSG_TRADE_STATUS);
    data << (uint32)7;
    GetPlayer()->pTrader->GetSession()->SendPacket(&data);

    GetPlayer()->acceptTrade = false;
}

void WorldSession::HandleBeginTradeOpcode(WorldPacket& recvPacket)
{
    WorldPacket data;

    
    data.Initialize(SMSG_TRADE_STATUS);
    data << (uint32)2; 
    GetPlayer()->pTrader->GetSession()->SendPacket(&data);
    GetPlayer()->pTrader->GetSession()->ClearTrade();

    
    data.Initialize(SMSG_TRADE_STATUS);
    data << (uint32)2; 
    SendPacket(&data);
    ClearTrade();
}

void WorldSession::HandleCancelTradeOpcode(WorldPacket& recvPacket)
{
    WorldPacket data;

    if( GetPlayer()->pTrader )
    {
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)3; 
        GetPlayer()->pTrader->GetSession()->SendPacket(&data);
    }
    
    GetPlayer()->pTrader = NULL;
    ClearTrade();
}

void WorldSession::HandleInitiateTradeOpcode(WorldPacket& recvPacket)
{
    WorldPacket data;
    uint64 ID;


    if( !GetPlayer()->isAlive() )
    {
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)17; 
        SendPacket(&data);
        return;        
    }

    if( isLogingOut() )
    {
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)19; 
        SendPacket(&data);
        return;        
    }

    recvPacket >> ID;

    Player* pOther = ObjectAccessor::Instance().FindPlayer( ID );

    if(!pOther)
    {
        
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)6; 
        SendPacket(&data);
        return;        
    }
    
    if( pOther->pTrader )
    {
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)0; 
        SendPacket(&data);
        return;    
    }
    
    if( !pOther->isAlive() )
    {
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)18; 
        SendPacket(&data);
        return;        
    }
    
    if( pOther->GetSession()->isLogingOut() )
    {
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)20; 
        SendPacket(&data);
        return;        
    }

	if( pOther->GetTeam() != GetPlayer()->GetTeam() )
    {
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)11; 
        SendPacket(&data);
        return;        
    }
	
	if( pOther->GetDistance2dSq( (Object*) GetPlayer() ) > 100.00 )
    {
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)10; 
        SendPacket(&data);
        return;        
    }

    GetPlayer()->pTrader = pOther; 
    pOther->pTrader = GetPlayer();  

    
    data.Initialize(SMSG_TRADE_STATUS);
    data << (uint32) 1; 
    data << (uint64) GetPlayer()->GetGUID();
    GetPlayer()->pTrader->GetSession()->SendPacket(&data);
}

void WorldSession::HandleSetTradeGoldOpcode(WorldPacket& recvPacket)
{
    uint32 gold;

    recvPacket >> gold;

    GetPlayer()->tradeGold = gold;

    UpdateTrade();

}

void WorldSession::HandleSetTradeItemOpcode(WorldPacket& recvPacket)
{
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
    uint8 slot;
    recvPacket >> slot;
    
    GetPlayer()->tradeItems[slot] = -1;

    UpdateTrade();
}

/*
OPCODE: TRADE_STATUS

0  - "[NAME] is busy"
1  - BEGIN TRADE
2  - OPEN TRADE WINDOW
3  - "Trade canceled"
4  - TRADE COMPLETE
5  - "[NAME] is busy"
6  - SOUND: I dont have a target
7  - BACK TRADE
8  - "Trade Complete" (FECHA A JANELA)
9  - ?
10 - "Trade target is too far away"
11 - "Trade is not party of your alliance"
12 - CLOSE TRADE WINDOW
13 - ?
14 - "[NAME] is ignoring you"
15 - "You are stunned"
16 - "Target is stunned"
17 - "You cannot do that when you are dead"
18 - "You cannot trade with dead players"
19 - "You are loging out"
*/