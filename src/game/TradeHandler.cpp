/* 
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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

enum TradeStatus
{
    TRADE_STATUS_BUSY           = 0,
    TRADE_STATUS_BEGIN_TRADE    = 1,
    TRADE_STATUS_OPEN_WINDOW    = 2,
    TRADE_STATUS_TRADE_CANCELED = 3,
    TRADE_STATUS_TRADE_ACCEPT   = 4,
    TRADE_STATUS_BUSY_2         = 5,
    TRADE_STATUS_NO_TARGET      = 6,
    TRADE_STATUS_BACK_TO_TRADE  = 7,
    TRADE_STATUS_TRADE_COMPLETE = 8,
    // 9?
    TRADE_STATUS_TARGET_TO_FAR  = 10,
    TRADE_STATUS_WRONG_FACTION  = 11,
    TRADE_STATUS_CLOSE_WINDOW   = 12,
    // 13?
    TRADE_STATUS_IGNORE_YOU     = 14,
    TRADE_STATUS_YOU_STUNNED    = 15,
    TRADE_STATUS_TARGET_STUNNED = 16,
    TRADE_STATUS_YOU_DEAD       = 17,
    TRADE_STATUS_TARGET_DEAD    = 18,
    TRADE_STATUS_YOU_LOGOUT     = 19,
    TRADE_STATUS_TARGET_LOGOUT  = 20,
    TRADE_STATUS_TRIAL_ACCOUNT  = 21,                       // Trial accounts can not perform that action
    TRADE_STATUS_ONLY_CONJURED  = 22                        // You can only trade conjured items... (cross realm BG related).
};

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
    Item *item = NULL;

    if( !_player || !_player->pTrader ) return;

    // reset trade status
    if (_player->acceptTrade)
    {
        _player->acceptTrade = false;
        SendTradeStatus(TRADE_STATUS_BACK_TO_TRADE);
    }

    if (_player->pTrader->acceptTrade)
    {
        _player->pTrader->acceptTrade = false;
        _player->pTrader->GetSession()->SendTradeStatus(TRADE_STATUS_BACK_TO_TRADE);
    }

    WorldPacket data(SMSG_TRADE_STATUS_EXTENDED, (100));    // guess size
    data << (uint8 ) 1;                                     // can be different (only seen 0 and 1)
    data << (uint32) TRADE_SLOT_COUNT;                      // trade slots count/number?, = next field in most cases
    data << (uint32) TRADE_SLOT_COUNT;                      // trade slots count/number?, = prev field in most cases
    data << (uint32) _player->pTrader->tradeGold;           // trader gold
    data << (uint32) 0;                                     // unknown

    for(uint8 i = 0; i < TRADE_SLOT_COUNT; i++)
    {
        item = (_player->pTrader->tradeItems[i] != NULL_SLOT ? _player->pTrader->GetItemByPos( _player->pTrader->tradeItems[i] ) : NULL);

        data << (uint8) i;                                  // trade slot number, if not specified, then end of packet

        if(item)
        {
            data << (uint32) item->GetProto()->ItemId;      // entry
                                                            // display id
            data << (uint32) item->GetProto()->DisplayInfoID;
                                                            // stack count
            data << (uint32) item->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
            data << (uint32) 0;                             // probably gift=1, created_by=0?
                                                            // gift creator
            data << (uint32) item->GetUInt32Value(ITEM_FIELD_GIFTCREATOR);
            data << (uint32) 0;                             // ITEM_FIELD_GIFTCREATOR high_guid
            data << (uint32) item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT);
            data << (uint32) 0;                             //item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+1);    // enchantment id (permanent/gems?)
            data << (uint32) 0;                             //item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+2);    // enchantment id (permanent/gems?)
            data << (uint32) 0;                             //item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+3);    // enchantment id (permanent/gems?)
                                                            // creator
            data << (uint32) item->GetUInt32Value(ITEM_FIELD_CREATOR);
            data << (uint32) 0;                             // ITEM_FIELD_CREATOR high_guid
            data << (uint32) item->GetSpellCharges();       // charges
            data << (uint32) item->GetItemSuffixFactor();   // SuffixFactor
                                                            // random properties id
            data << (uint32) item->GetItemRandomPropertyId();
            data << (uint32) item->GetProto()->LockID;      // lock id
                                                            // max durability
            data << (uint32) item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
                                                            // durability
            data << (uint32) item->GetUInt32Value(ITEM_FIELD_DURABILITY);
        }
        else
        {
            for(uint8 j = 0; j < 18; j++)
                data << uint32(0);
        }
    }
    SendPacket(&data);
}

void WorldSession::HandleAcceptTradeOpcode(WorldPacket& recvPacket)
{
    Item *myItems[TRADE_SLOT_TRADED_COUNT]  = { NULL, NULL, NULL, NULL, NULL, NULL };
    Item *hisItems[TRADE_SLOT_TRADED_COUNT] = { NULL, NULL, NULL, NULL, NULL, NULL };
    bool myCanCompleteTrade=true,hisCanCompleteTrade=true;
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
        // inform partner client
        _player->pTrader->GetSession()->SendTradeStatus(TRADE_STATUS_TRADE_ACCEPT);

        // store items in local list and set 'in-trade' flag
        for(int i=0; i<TRADE_SLOT_TRADED_COUNT; i++)
        {
            if(_player->tradeItems[i] != NULL_SLOT )
            {
                sLog.outDebug("player trade item bag: %u slot: %u",_player->tradeItems[i] >> 8, _player->tradeItems[i] & 255 );
                                                            //Can return NULL
                myItems[i]=_player->GetItemByPos( _player->tradeItems[i] );
                if (myItems[i])
                    myItems[i]->SetInTrade();
            }
            if(_player->pTrader->tradeItems[i] != NULL_SLOT)
            {
                sLog.outDebug("partner trade item bag: %u slot: %u",_player->pTrader->tradeItems[i] >> 8,_player->pTrader->tradeItems[i] & 255);
                                                            //Can return NULL
                hisItems[i]=_player->pTrader->GetItemByPos( _player->pTrader->tradeItems[i]);
                if(hisItems[i])
                    hisItems[i]->SetInTrade();
            }
        }

        // test if item will fit in each inventory
        hisCanCompleteTrade =  (_player->pTrader->CanStoreItems( myItems,TRADE_SLOT_TRADED_COUNT )== EQUIP_ERR_OK);
        myCanCompleteTrade = (_player->CanStoreItems( hisItems,TRADE_SLOT_TRADED_COUNT ) == EQUIP_ERR_OK);

        // clear 'in-trade' flag
        for(int i=0; i<TRADE_SLOT_TRADED_COUNT; i++)
        {
            if(myItems[i])  myItems[i]->SetInTrade(false);
            if(hisItems[i]) hisItems[i]->SetInTrade(false);
        }

        // in case of missing space report error
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

        // execute trade: 1. remove
        for(int i=0; i<TRADE_SLOT_TRADED_COUNT; i++)
        {
            if(myItems[i])
            {
                myItems[i]->SetUInt64Value( ITEM_FIELD_GIFTCREATOR,_player->GetGUID());
                _player->ItemRemovedQuestCheck(myItems[i]->GetEntry(),myItems[i]->GetCount());
                _player->RemoveItem(_player->tradeItems[i] >> 8, _player->tradeItems[i] & 255, true);
                myItems[i]->RemoveFromUpdateQueueOf(_player);
            }
            if(hisItems[i])
            {
                hisItems[i]->SetUInt64Value( ITEM_FIELD_GIFTCREATOR,_player->pTrader->GetGUID());
                _player->pTrader->ItemRemovedQuestCheck(hisItems[i]->GetEntry(),hisItems[i]->GetCount());
                _player->pTrader->RemoveItem(_player->pTrader->tradeItems[i] >> 8, _player->pTrader->tradeItems[i] & 255, true);
                hisItems[i]->RemoveFromUpdateQueueOf(_player->pTrader);
            }
        }

        // execute trade: 2. store
        for(int i=0; i<TRADE_SLOT_TRADED_COUNT; i++)
        {
            if(myItems[i])
            {
                if(_player->pTrader->CanStoreItem( NULL_BAG, NULL_SLOT, dst, myItems[i], false ) == EQUIP_ERR_OK)
                {
                    // logging
                    sLog.outDebug("partner storing: %u",myItems[i]->GetGUIDLow());
                    if( _player->GetSession()->GetSecurity() > SEC_PLAYER && sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
                        sLog.outCommand("GM %s (Account: %u) trade: %s (Entry: %d Count: %u) to player: %s (Account: %u)",
                            _player->GetName(),_player->GetSession()->GetAccountId(),
                            myItems[i]->GetProto()->Name1,myItems[i]->GetEntry(),myItems[i]->GetCount(),
                            _player->pTrader->GetName(),_player->pTrader->GetSession()->GetAccountId());

                    // store
                    _player->pTrader->ItemAddedQuestCheck(myItems[i]->GetEntry(),myItems[i]->GetCount());
                    _player->pTrader->StoreItem( dst, myItems[i], true);
                }
                else
                {
                    // in case of fatal error move items back
                    sLog.outError("player can't store item: %u",hisItems[i]->GetGUIDLow());
                    _player->ItemAddedQuestCheck(hisItems[i]->GetEntry(),hisItems[i]->GetCount());
                    _player->StoreItem( dst, hisItems[i], true);
                }
            }
            if(hisItems[i])
            {
                if(_player->CanStoreItem( NULL_BAG, NULL_SLOT, dst, hisItems[i], false ) == EQUIP_ERR_OK)
                {
                    // logging
                    sLog.outDebug("player storing: %u",hisItems[i]->GetGUIDLow());
                    if( _player->pTrader->GetSession()->GetSecurity() > SEC_PLAYER && sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
                        sLog.outCommand("GM %s (Account: %u) trade: %s (Entry: %d Count: %u) to player: %s (Account: %u)",
                            _player->pTrader->GetName(),_player->pTrader->GetSession()->GetAccountId(),
                            hisItems[i]->GetProto()->Name1,hisItems[i]->GetEntry(),hisItems[i]->GetCount(),
                            _player->GetName(),_player->GetSession()->GetAccountId());

                    // store
                    _player->ItemAddedQuestCheck(hisItems[i]->GetEntry(),hisItems[i]->GetCount());
                    _player->StoreItem( dst, hisItems[i], true);
                }
                else
                {
                    // in case of fatal error move items back
                    sLog.outError("player can't store item: %u",hisItems[i]->GetGUIDLow());
                    _player->pTrader->ItemAddedQuestCheck(myItems[i]->GetEntry(),myItems[i]->GetCount());
                    _player->pTrader->StoreItem( dst, myItems[i], true);

                }
            }
        }

        // logging money
        if(sWorld.getConfig(CONFIG_GM_LOG_TRADE))
        {
            if( _player->GetSession()->GetSecurity() > SEC_PLAYER && _player->tradeGold > 0)
                sLog.outCommand("GM %s (Account: %u) give money (Amount: %u) to player: %s (Account: %u)",
                    _player->GetName(),_player->GetSession()->GetAccountId(),
                    _player->tradeGold,
                    _player->pTrader->GetName(),_player->pTrader->GetSession()->GetAccountId());
            if( _player->pTrader->GetSession()->GetSecurity() > SEC_PLAYER && _player->pTrader->tradeGold > 0)
                sLog.outCommand("GM %s (Account: %u) give money (Amount: %u) to player: %s (Account: %u)",
                    _player->pTrader->GetName(),_player->pTrader->GetSession()->GetAccountId(),
                    _player->pTrader->tradeGold,
                    _player->GetName(),_player->GetSession()->GetAccountId());
        }

        // update money
        _player->ModifyMoney( -int32(_player->tradeGold) );
        _player->ModifyMoney(_player->pTrader->tradeGold );
        _player->pTrader->ModifyMoney( -int32(_player->pTrader->tradeGold) );
        _player->pTrader->ModifyMoney(_player->tradeGold );

        _player->ClearTrade();
        _player->pTrader->ClearTrade();

        // desynchronized with the other saves here (SaveInventoryAndGoldToDB() not have own transaction guards)
        CharacterDatabase.BeginTransaction();
        _player->SaveInventoryAndGoldToDB();
        _player->pTrader->SaveInventoryAndGoldToDB();
        CharacterDatabase.CommitTransaction();

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
    CHECK_PACKET_SIZE(recvPacket,8);

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

    Player* pOther = ObjectAccessor::FindPlayer( ID );

    if( !pOther )
    {
        SendTradeStatus(TRADE_STATUS_NO_TARGET);
        return;
    }

    if( pOther == GetPlayer() || pOther->pTrader )
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

    if(!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_TRADE) && pOther->GetTeam() !=_player->GetTeam() )
    {
        SendTradeStatus(TRADE_STATUS_WRONG_FACTION);
        return;
    }

    if( pOther->GetDistance2dSq( _player ) > 100.0f )
    {
        SendTradeStatus(TRADE_STATUS_TARGET_TO_FAR);
        return;
    }

    // OK start trade
    _player->pTrader = pOther;
    pOther->pTrader =_player;

    WorldPacket data(SMSG_TRADE_STATUS, 12);
    data << (uint32) TRADE_STATUS_BEGIN_TRADE;
    data << (uint64)_player->GetGUID();
    _player->pTrader->GetSession()->SendPacket(&data);
}

void WorldSession::HandleSetTradeGoldOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket,4);

    if(!_player->pTrader)
        return;

    uint32 gold;

    recvPacket >> gold;

    // gold can be incorrect, but this is checked at trade finished.
    _player->tradeGold = gold;

    _player->pTrader->GetSession()->SendUpdateTrade();
}

void WorldSession::HandleSetTradeItemOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket,1+1+1);

    if(!_player->pTrader)
        return;

    // send update
    uint8 tradeSlot;
    uint8 bag;
    uint8 slot;

    recvPacket >> tradeSlot;
    recvPacket >> bag;
    recvPacket >> slot;

    // invalid slot number
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
    CHECK_PACKET_SIZE(recvPacket,1);

    if(!_player->pTrader)
        return;

    uint8 tradeSlot;
    recvPacket >> tradeSlot;

    // invalid slot number
    if(tradeSlot >= TRADE_SLOT_COUNT)
        return;

    _player->tradeItems[tradeSlot] = NULL_SLOT;

    _player->pTrader->GetSession()->SendUpdateTrade();
}
