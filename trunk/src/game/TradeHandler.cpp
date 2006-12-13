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
    sLog.outDebug( "WORLD: Ignore Trade %u",_player->GetGUIDLow());
    recvPacket.print_storage();
}

void WorldSession::HandleBusyTradeOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug( "WORLD: Busy Trade %u",_player->GetGUIDLow());
    recvPacket.print_storage();
}

void WorldSession::SendUpdateTrade()
{
    WorldPacket data;
    Player *pThis =_player;
    Item *item = NULL;

    if( !pThis->pTrader ) return;

    data.Initialize(SMSG_TRADE_STATUS_EXTENDED);
    data << (uint8 ) 1;
    data << (uint32) 7;
    data << (uint32) 0;
    data << (uint32)pThis->pTrader->tradeGold;
    data << (uint32) 0;
    for(int i=0; i<TRADE_SLOT_COUNT; i++)
    {
        item = (pThis->pTrader->tradeItems[i] != NULL_SLOT ? pThis->pTrader->GetItemByPos( pThis->pTrader->tradeItems[i] ) : NULL);

        data << (uint8) i;
        if(item)
        {
            data << (uint32) item->GetProto()->ItemId;
            data << (uint32) 0;
            data << (uint32) item->GetUInt32Value(ITEM_FIELD_STACK_COUNT);

            data << (uint32) 0;                             // gift here ???
            data << (uint32) 0;                             // gift here ???
            data << (uint32) 0;                             // gift here ??? or enchants count ?
            data << (uint32) item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT);
            data << (uint32) item->GetUInt32Value(ITEM_FIELD_CREATOR);
            data << (uint32) HIGHGUID_PLAYER;
            data << (uint32) 0;
            data << (uint32) 0;
            data << (uint32) item->GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID);
            data << (uint32) item->GetUInt32Value(ITEM_FIELD_FLAGS);
            data << (uint32) item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
            data << (uint32) item->GetUInt32Value(ITEM_FIELD_DURABILITY);
        }
        else
        {
            for(int j=0; j<15; j++)
                data << (uint32) 0;
        }
    }

    SendPacket(&data);
}

void WorldSession::HandleAcceptTradeOpcode(WorldPacket& recvPacket)
{
    {
        WorldPacket data;
        Item *myItems[TRADE_SLOT_TRADED_COUNT]  = { NULL, NULL, NULL, NULL, NULL, NULL };
        Item *hisItems[TRADE_SLOT_TRADED_COUNT] = { NULL, NULL, NULL, NULL, NULL, NULL };
        bool myCanStoreItem=false,hisCanStoreItem=false,myCanCompleteTrade=true,hisCanCompleteTrade=true;
        uint16 dst;

        if ( !GetPlayer()->pTrader )
            return;

        _player->acceptTrade = true;
        if (_player->pTrader->acceptTrade )
        {
            data.Initialize(SMSG_TRADE_STATUS);
            data << (uint32)4;
            _player->pTrader->GetSession()->SendPacket(&data);
            for(int i=0; i<TRADE_SLOT_TRADED_COUNT; i++)
            {
                if(_player->tradeItems[i] != NULL_SLOT )
                {
                    sLog.outDebug("player trade item bag: %u slot: %u",_player->tradeItems[i] >> 8, _player->tradeItems[i] & 255 );
                    myItems[i]=_player->GetItemByPos( _player->tradeItems[i] );
                }
                if(_player->pTrader->tradeItems[i] != NULL_SLOT)
                {
                    sLog.outDebug("partner trade item bag: %u slot: %u",_player->pTrader->tradeItems[i] >> 8,_player->pTrader->tradeItems[i] & 255);
                    hisItems[i]=_player->pTrader->GetItemByPos( _player->pTrader->tradeItems[i]);
                }
            }
            for(int i=0; i<TRADE_SLOT_TRADED_COUNT; i++)
            {
                if(myItems[i])
                {
                    myItems[i]->SetUInt64Value( ITEM_FIELD_GIFTCREATOR,_player->GetGUID());
                    if(_player->pTrader->CanStoreItem( NULL_BAG, NULL_SLOT, dst, myItems[i], false )== EQUIP_ERR_OK)
                    {
                        hisCanStoreItem = true;
                        sLog.outDebug("partner can accept item: %u",myItems[i]->GetGUIDLow());
                    }
                }
                else
                {
                    hisCanStoreItem = true;
                }
                sLog.outDebug("hisCanStoreItem: %u",hisCanStoreItem);
                if(hisItems[i])
                {
                    hisItems[i]->SetUInt64Value( ITEM_FIELD_GIFTCREATOR,_player->pTrader->GetGUID());
                    if(_player->CanStoreItem( NULL_BAG, NULL_SLOT, dst, hisItems[i], false ) == EQUIP_ERR_OK)
                    {
                        myCanStoreItem = true;
                        sLog.outDebug("you can accept item %u ",hisItems[i]->GetGUIDLow());
                    }
                }
                else
                {
                    myCanStoreItem = true;
                }
                sLog.outDebug("myCanStoreItem: %u",myCanStoreItem);
                if (!myCanStoreItem || !hisCanStoreItem)
                {
                    myCanCompleteTrade = myCanStoreItem;
                    hisCanCompleteTrade = hisCanStoreItem;
                    break;
                }
            }
            if(!myCanCompleteTrade)
            {
                sChatHandler.FillSystemMessageData(&data,_player->GetSession(), "You do not have enough free slots");
                GetPlayer( )->GetSession( )->SendPacket( &data );
                sChatHandler.FillSystemMessageData(&data,_player->pTrader->GetSession(), "Your partner does not have enough free bag slots");
                GetPlayer( )->pTrader->GetSession( )->SendPacket( &data );
                _player->GetSession()->HandleUnacceptTradeOpcode(recvPacket);
                _player->pTrader->GetSession()->HandleUnacceptTradeOpcode(recvPacket);
                return;
            }
            else if (!hisCanCompleteTrade)
            {
                sChatHandler.FillSystemMessageData(&data,_player->GetSession(), "Your partner does not have enough free bag slots");
                GetPlayer()->GetSession()->SendPacket( &data );
                sChatHandler.FillSystemMessageData(&data,_player->pTrader->GetSession(), "You do not have enough free slots");
                GetPlayer()->pTrader->GetSession()->SendPacket( &data );
                _player->GetSession()->HandleUnacceptTradeOpcode(recvPacket);
                _player->pTrader->GetSession()->HandleUnacceptTradeOpcode(recvPacket);
                return;
            }
            for(int i=0; i<TRADE_SLOT_TRADED_COUNT; i++)
            {
                if(myItems[i])
                {
                    myItems[i]->SetUInt64Value( ITEM_FIELD_GIFTCREATOR,_player->GetGUID());
                    if(_player->pTrader->CanStoreItem( NULL_BAG, NULL_SLOT, dst, myItems[i], false ) == EQUIP_ERR_OK)
                    {
                        sLog.outDebug("partner storing: %u",myItems[i]->GetGUIDLow());
                        _player->ItemRemovedQuestCheck(myItems[i]->GetEntry(),myItems[i]->GetCount());
                        _player->RemoveItem(_player->tradeItems[i] >> 8, _player->tradeItems[i] & 255, true);
                        myItems[i]->RemoveFromUpdateQueueOf(_player);
                        _player->pTrader->ItemAddedQuestCheck(myItems[i]->GetEntry(),myItems[i]->GetCount());
                        _player->pTrader->StoreItem( dst, myItems[i], true);
                    }
                }
                if(hisItems[i])
                {
                    hisItems[i]->SetUInt64Value( ITEM_FIELD_GIFTCREATOR,_player->pTrader->GetGUID());
                    if(_player->CanStoreItem( NULL_BAG, NULL_SLOT, dst, hisItems[i], false ) == EQUIP_ERR_OK)
                    {
                        sLog.outDebug("player storing: %u",hisItems[i]->GetGUIDLow());
                        _player->pTrader->ItemRemovedQuestCheck(hisItems[i]->GetEntry(),hisItems[i]->GetCount());
                        _player->pTrader->RemoveItem(_player->pTrader->tradeItems[i] >> 8, _player->pTrader->tradeItems[i] & 255, true);
                        hisItems[i]->RemoveFromUpdateQueueOf(_player->pTrader);
                        _player->ItemAddedQuestCheck(hisItems[i]->GetEntry(),hisItems[i]->GetCount());
                        _player->StoreItem( dst, hisItems[i], true);
                    }
                }
            }
            // desynced with the other saves here
            _player->_SaveInventory();
            _player->pTrader->_SaveInventory();
            _player->ModifyMoney( -((int32)_player->tradeGold) );
            _player->ModifyMoney(_player->pTrader->tradeGold );
            _player->pTrader->ModifyMoney( -((int32)_player->pTrader->tradeGold) );
            _player->pTrader->ModifyMoney(_player->tradeGold );
            _player->ClearTrade();
            _player->pTrader->ClearTrade();
            data.Initialize(SMSG_TRADE_STATUS);
            data << (uint32)8;
            _player->pTrader->GetSession()->SendPacket(&data);
            data.Initialize(SMSG_TRADE_STATUS);
            data << (uint32)8;
            SendPacket(&data);
            _player->pTrader->pTrader = NULL;
            _player->pTrader = NULL;
        }
        else
        {
            data.Initialize(SMSG_TRADE_STATUS);
            data << (uint32)4;
            _player->pTrader->GetSession()->SendPacket(&data);
        }
    }
}

void WorldSession::HandleUnacceptTradeOpcode(WorldPacket& recvPacket)
{
    WorldPacket data;

    if ( !GetPlayer()->pTrader )
        return;

    data.Initialize(SMSG_TRADE_STATUS);
    data << (uint32)7;
    _player->pTrader->GetSession()->SendPacket(&data);

    _player->acceptTrade = false;
}

void WorldSession::HandleBeginTradeOpcode(WorldPacket& recvPacket)
{
    WorldPacket data;

    if(!_player->pTrader)
        return;

    data.Initialize(SMSG_TRADE_STATUS);
    data << (uint32)2;
    _player->pTrader->GetSession()->SendPacket(&data);
    _player->pTrader->ClearTrade();

    data.Initialize(SMSG_TRADE_STATUS);
    data << (uint32)2;
    SendPacket(&data);
    _player->ClearTrade();
}

void WorldSession::SendCancelTrade()
{
    WorldPacket data;

    data.Initialize(SMSG_TRADE_STATUS);
    data << (uint32)3;
    SendPacket(&data);
}

void WorldSession::HandleCancelTradeOpcode(WorldPacket& recvPacket)
{
    // sended also after LOGOUT COMPLETE
    if(_player)
        _player->TradeCancel(true);
}

void WorldSession::HandleInitiateTradeOpcode(WorldPacket& recvPacket)
{
    if( GetPlayer()->pTrader )
        return;

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

    if( pOther->HasInIgnoreList(GetPlayer()->GetGUID()) )
    {
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)14;
        SendPacket(&data);
        return;
    }

    if(!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION) && pOther->GetTeam() !=_player->GetTeam() )
    {
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)11;
        SendPacket(&data);
        return;
    }

    if( pOther->GetDistance2dSq( (Object*)_player ) > 100.00 )
    {
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)10;
        SendPacket(&data);
        return;
    }

    _player->pTrader = pOther;
    pOther->pTrader =_player;

    data.Initialize(SMSG_TRADE_STATUS);
    data << (uint32) 1;
    data << (uint64)_player->GetGUID();
    _player->pTrader->GetSession()->SendPacket(&data);
}

void WorldSession::HandleSetTradeGoldOpcode(WorldPacket& recvPacket)
{
    if(!_player->pTrader)
        return;

    uint32 gold;

    recvPacket >> gold;

    _player->tradeGold = gold;

    _player->pTrader->GetSession()->SendUpdateTrade();

}

void WorldSession::HandleSetTradeItemOpcode(WorldPacket& recvPacket)
{
    if(!_player->pTrader)
        return;

    uint8 tradeSlot;
    uint8 bag;
    uint8 slot;

    recvPacket >> tradeSlot;
    recvPacket >> bag;
    recvPacket >> slot;

    // check cheating, can't fail with correct client operations
    Item* item = _player->GetItemByPos(bag,slot);
    if(!item || tradeSlot!=TRADE_SLOT_NONTRADED && !item->CanBeTraded())
    {
        // send to self (cancel trade at cheating attempt)
        WorldPacket data;
        data.Initialize(SMSG_TRADE_STATUS);
        data << (uint32)3;
        SendPacket(&data);
        return;
    }

    _player->tradeItems[tradeSlot] = (bag << 8) | slot;

    _player->pTrader->GetSession()->SendUpdateTrade();
}

void WorldSession::HandleClearTradeItemOpcode(WorldPacket& recvPacket)
{
    if(!_player->pTrader)
        return;

    uint8 slot;
    recvPacket >> slot;

    _player->tradeItems[slot] = NULL_SLOT;

    _player->pTrader->GetSession()->SendUpdateTrade();
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
