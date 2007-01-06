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

#define TRADE_STATUS_BUSY           0
#define TRADE_STATUS_BEGIN_TRADE    1
#define TRADE_STATUS_OPEN_WINDOW    2
#define TRADE_STATUS_TRADE_CANCELED 3
#define TRADE_STATUS_TRADE_ACCEPT   4
#define TRADE_STATUS_BUSY_2         5
#define TRADE_STATUS_NO_TARGET      6
#define TRADE_STATUS_BACK_TO_TRADE  7
#define TRADE_STATUS_TRADE_COMPLETE 8
#define TRADE_STATUS_TARGET_TO_FAR  10
#define TRADE_STATUS_WRONG_FACTION  11
#define TRADE_STATUS_CLOSE_WINDOW   12
#define TRADE_STATUS_IGNORE_YOU     14
#define TRADE_STATUS_YOU_STUNNED    15
#define TRADE_STATUS_TARGET_STUNNED 16
#define TRADE_STATUS_YOU_DEAD       17
#define TRADE_STATUS_TARGET_DEAD    18
#define TRADE_STATUS_YOU_LOGOUT     19
#define TRADE_STATUS_TARGET_LOGOUT  20

void WorldSession::SendTradeStatus(uint32 status)
{
    WorldPacket data(SMSG_TRADE_STATUS,4);
    data << (uint32)status;
    SendPacket(&data);
}


void WorldSession::HandleIgnoreTradeOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug( "WORLD: Ignore Trade %u",_player->GetGUIDLow());
    // recvPacket.print_storage();
}

void WorldSession::HandleBusyTradeOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug( "WORLD: Busy Trade %u",_player->GetGUIDLow());
    // recvPacket.print_storage();
}

void WorldSession::SendUpdateTrade()
{
    Player *pThis =_player;
    Item *item = NULL;

    if( !pThis->pTrader ) return;

    WorldPacket data(SMSG_TRADE_STATUS_EXTENDED, (100));    // guess size
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
        Item *myItems[TRADE_SLOT_TRADED_COUNT]  = { NULL, NULL, NULL, NULL, NULL, NULL };
        Item *hisItems[TRADE_SLOT_TRADED_COUNT] = { NULL, NULL, NULL, NULL, NULL, NULL };
        bool myCanStoreItem=false,hisCanStoreItem=false,myCanCompleteTrade=true,hisCanCompleteTrade=true;
        uint16 dst;

        if ( !GetPlayer()->pTrader )
            return;

        // not accept case incorrect money amount
        if( _player->tradeGold > _player->GetMoney() )
        {
            SendNotification( "You do not have enough gold" );
            _player->pTrader->GetSession()->SendTradeStatus(TRADE_STATUS_BACK_TO_TRADE);
            _player->acceptTrade = false;
            return;
        }

        // not accept case incorrect money amount
        if( _player->pTrader->tradeGold > _player->pTrader->GetMoney() )
        {
            _player->pTrader->GetSession( )->SendNotification( "You do not have enough gold" );
            SendTradeStatus(TRADE_STATUS_BACK_TO_TRADE);
            _player->pTrader->acceptTrade = false;
            return;
        }

        _player->acceptTrade = true;
        if (_player->pTrader->acceptTrade )
        {

            _player->pTrader->GetSession()->SendTradeStatus(TRADE_STATUS_TRADE_ACCEPT);

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
                SendNotification("You do not have enough free slots");
                GetPlayer( )->pTrader->GetSession( )->SendNotification("Your partner does not have enough free bag slots");
                SendTradeStatus(TRADE_STATUS_BACK_TO_TRADE);
                _player->pTrader->GetSession()->SendTradeStatus(TRADE_STATUS_BACK_TO_TRADE);
                return;
            }
            else if (!hisCanCompleteTrade)
            {
                SendNotification("Your partner does not have enough free bag slots");
                GetPlayer()->pTrader->GetSession()->SendNotification("You do not have enough free slots");
                SendTradeStatus(TRADE_STATUS_BACK_TO_TRADE);
                _player->pTrader->GetSession()->SendTradeStatus(TRADE_STATUS_BACK_TO_TRADE);
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

            _player->pTrader->GetSession()->SendTradeStatus(TRADE_STATUS_TRADE_COMPLETE);
            SendTradeStatus(TRADE_STATUS_TRADE_COMPLETE);

            _player->pTrader->pTrader = NULL;
            _player->pTrader = NULL;
        }
        else
        {
            _player->pTrader->GetSession()->SendTradeStatus(TRADE_STATUS_TRADE_ACCEPT);
        }
    }
}

void WorldSession::HandleUnacceptTradeOpcode(WorldPacket& recvPacket)
{
    if ( !GetPlayer()->pTrader )
        return;

    _player->pTrader->GetSession()->SendTradeStatus(TRADE_STATUS_BACK_TO_TRADE);
    _player->acceptTrade = false;
}

void WorldSession::HandleBeginTradeOpcode(WorldPacket& recvPacket)
{
    if(!_player->pTrader)
        return;

    _player->pTrader->GetSession()->SendTradeStatus(TRADE_STATUS_OPEN_WINDOW);
    _player->pTrader->ClearTrade();

    SendTradeStatus(TRADE_STATUS_OPEN_WINDOW);
    _player->ClearTrade();
}

void WorldSession::SendCancelTrade()
{
    SendTradeStatus(TRADE_STATUS_TRADE_CANCELED);
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

    uint64 ID;

    if( !GetPlayer()->isAlive() )
    {
        SendTradeStatus(TRADE_STATUS_YOU_DEAD);
        return;
    }

    if( GetPlayer()->hasUnitState(UNIT_STAT_STUNDED) )
    {
        SendTradeStatus(TRADE_STATUS_YOU_STUNNED);
        return;
    }

    if( isLogingOut() )
    {
        SendTradeStatus(TRADE_STATUS_YOU_LOGOUT);
        return;
    }

    recvPacket >> ID;

    Player* pOther = ObjectAccessor::Instance().FindPlayer( ID );

    if(!pOther)
    {
        SendTradeStatus(TRADE_STATUS_NO_TARGET);
        return;
    }

    if( pOther->pTrader )
    {
        SendTradeStatus(TRADE_STATUS_BUSY);
        return;
    }

    if( !pOther->isAlive() )
    {
        SendTradeStatus(TRADE_STATUS_TARGET_DEAD);
        return;
    }

    if( pOther->hasUnitState(UNIT_STAT_STUNDED) )
    {
        SendTradeStatus(TRADE_STATUS_TARGET_STUNNED);
        return;
    }

    if( pOther->GetSession()->isLogingOut() )
    {
        SendTradeStatus(TRADE_STATUS_TARGET_LOGOUT);
        return;
    }

    if( pOther->HasInIgnoreList(GetPlayer()->GetGUID()) )
    {
        SendTradeStatus(TRADE_STATUS_IGNORE_YOU);
        return;
    }

    if(!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION) && pOther->GetTeam() !=_player->GetTeam() )
    {
        SendTradeStatus(TRADE_STATUS_WRONG_FACTION);
        return;
    }

    if( pOther->GetDistance2dSq( (Object*)_player ) > 100.00 )
    {
        SendTradeStatus(TRADE_STATUS_TARGET_TO_FAR);
        return;
    }

    // OK start trade
    _player->pTrader = pOther;
    pOther->pTrader =_player;

    WorldPacket data(SMSG_TRADE_STATUS, 12);
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

    // gold can be incorrect, but this is checked at trade fihished.
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

    // invalide slot number
    if(tradeSlot >= TRADE_SLOT_COUNT)
    {
        SendTradeStatus(TRADE_STATUS_TRADE_CANCELED);
        return;
    }

    // check cheating, can't fail with correct client operations
    Item* item = _player->GetItemByPos(bag,slot);
    if(!item || tradeSlot!=TRADE_SLOT_NONTRADED && !item->CanBeTraded())
    {
        SendTradeStatus(TRADE_STATUS_TRADE_CANCELED);
        return;
    }

    _player->tradeItems[tradeSlot] = (bag << 8) | slot;

    _player->pTrader->GetSession()->SendUpdateTrade();
}

void WorldSession::HandleClearTradeItemOpcode(WorldPacket& recvPacket)
{
    if(!_player->pTrader)
        return;

    uint8 tradeSlot;
    recvPacket >> tradeSlot;

    // invalide slot number
    if(tradeSlot >= TRADE_SLOT_COUNT)
        return;

    _player->tradeItems[tradeSlot] = NULL_SLOT;

    _player->pTrader->GetSession()->SendUpdateTrade();
}
