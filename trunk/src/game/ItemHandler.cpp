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
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Chat.h"
#include "Item.h"
#include "UpdateData.h"
#include "ObjectAccessor.h"

void WorldSession::HandleSplitItemOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_SPLIT_ITEM");
    uint8 srcbag, srcslot, dstbag, dstslot, count;

    recv_data >> srcbag >> srcslot >> dstbag >> dstslot >> count;
    sLog.outDebug("STORAGE: receive srcbag = %u, srcslot = %u, dstbag = %u, dstslot = %u, count = %u", srcbag, srcslot, dstbag, dstslot, count);

    uint16 src = ( (srcbag << 8) | srcslot );
    uint16 dst = ( (dstbag << 8) | dstslot );

    _player->SplitItem( src, dst, count );
}

void WorldSession::HandleSwapInvItemOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_SWAP_INV_ITEM");
    uint8 srcslot, dstslot;

    recv_data >> srcslot >> dstslot;
    sLog.outDebug("STORAGE: receive srcslot = %u, dstslot = %u", srcslot, dstslot);

    uint16 src = ( (INVENTORY_SLOT_BAG_0 << 8) | srcslot );
    uint16 dst = ( (INVENTORY_SLOT_BAG_0 << 8) | dstslot );

    _player->SwapItem( src, dst );
}

void WorldSession::HandleSwapItem( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_SWAP_ITEM");
    uint8 dstbag, dstslot, srcbag, srcslot;

    recv_data >> dstbag >> dstslot >> srcbag >> srcslot ;
    sLog.outDebug("STORAGE: receive srcbag = %u, srcslot = %u, dstbag = %u, dstslot = %u", srcbag, srcslot, dstbag, dstslot);

    uint16 src = ( (srcbag << 8) | srcslot );
    uint16 dst = ( (dstbag << 8) | dstslot );

    _player->SwapItem( src, dst );
}

void WorldSession::HandleAutoEquipItemOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_AUTOEQUIP_ITEM");
    uint8 srcbag, srcslot;

    recv_data >> srcbag >> srcslot;
    sLog.outDebug("STORAGE: receive srcbag = %u, srcslot = %u", srcbag, srcslot);

    Item *pItem  = _player->GetItemByPos( srcbag, srcslot );
    if( pItem )
    {
        uint16 dest;
        uint8 msg = _player->CanEquipItem( NULL_SLOT, dest, pItem, true );
        if( msg == EQUIP_ERR_OK )
        {
            Item *pItem2 = _player->GetItemByPos( dest );
            if( pItem2 )
            {
                uint16 src = ((srcbag << 8) | srcslot);
                uint8 bag = dest >> 8;
                uint8 slot = dest & 255;
                _player->RemoveItem( bag, slot, false );
                _player->RemoveItem( srcbag, srcslot, false );
                if( _player->IsInventoryPos( src ) )
                    _player->StoreItem( src, pItem2, true);
                else if( _player->IsBankPos ( src ) )
                    _player->BankItem( src, pItem2, true);
                else if( _player->IsEquipmentPos ( src ) )
                    _player->EquipItem( src, pItem2, true);
                _player->EquipItem( dest, pItem, true );
            }
            else
            {
                _player->RemoveItem( srcbag, srcslot, true );
                _player->EquipItem( dest, pItem, true );
            }
        }
        else
            _player->SendEquipError( msg, pItem, NULL, 0 );
    }
}

void WorldSession::HandleDestroyItemOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_DESTROYITEM");
    uint8 bag, slot, count, data1, data2, data3;

    recv_data >> bag >> slot >> count >> data1 >> data2 >> data3;
    sLog.outDebug("STORAGE: receive bag = %u, slot = %u, count = %u", bag, slot, count);

    _player->DestroyItem( bag, slot, true );
}

extern void CheckItemDamageValues ( ItemPrototype *itemProto );

void WorldSession::HandleItemQuerySingleOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_ITEM_QUERY_SINGLE");
    WorldPacket data;

    uint32 itemId, guidLow, guidHigh;
    recv_data >> itemId >> guidLow >> guidHigh;

    ItemPrototype *itemProto = objmgr.GetItemPrototype(itemId);

    if (!itemProto)
    {
        sLog.outError("ITEM: Unknown item, itemId = %u", itemId);
        data.Initialize( SMSG_ITEM_QUERY_SINGLE_RESPONSE );
        data << itemId;
        for(int a=0;a<11;a++)
            data << uint64(0);
        SendPacket( &data );
        return;
    }

    sLog.outDetail("ITEM: Item query, itemId = %u, guidLow = %u, guidHigh = %u", itemId, guidLow, guidHigh);

    data.Initialize( SMSG_ITEM_QUERY_SINGLE_RESPONSE );
    data << itemProto->ItemId;
    data << itemProto->Class;
    data << itemProto->SubClass;
    data << itemProto->Name1;
    data << itemProto->Name2;
    data << itemProto->Name3;
    data << itemProto->Name4;
    data << itemProto->DisplayInfoID;
    data << itemProto->Quality;
    data << itemProto->Flags;
    data << itemProto->BuyPrice;
    data << itemProto->SellPrice;
    data << itemProto->InventoryType;
    data << itemProto->AllowableClass;
    data << itemProto->AllowableRace;
    data << itemProto->ItemLevel;
    data << itemProto->RequiredLevel;
    data << itemProto->RequiredSkill;
    data << itemProto->RequiredSkillRank;
    data << itemProto->RequiredSpell;
    data << itemProto->RequiredHonorRank;
    data << itemProto->RequiredCityRank;
    data << itemProto->RequiredReputationFaction;
    data << itemProto->RequiredReputationRank;
    data << itemProto->MaxCount;
    data << itemProto->Stackable;
    data << itemProto->ContainerSlots;
    for(int i = 0; i < 10; i++)
    {
        data << itemProto->ItemStat[i].ItemStatType;
        data << itemProto->ItemStat[i].ItemStatValue;
    }
    for(int i = 0; i < 5; i++)
    {
        data << itemProto->Damage[i].DamageMin;
        data << itemProto->Damage[i].DamageMax;
        data << itemProto->Damage[i].DamageType;
    }
    data << itemProto->Armor;
    data << itemProto->HolyRes;
    data << itemProto->FireRes;
    data << itemProto->NatureRes;
    data << itemProto->FrostRes;
    data << itemProto->ShadowRes;
    data << itemProto->ArcaneRes;
    data << itemProto->Delay;
    data << itemProto->Ammo_type;

    data << (float)itemProto->RangedModRange;

    for(int s = 0; s < 5; s++)
    {
        data << itemProto->Spells[s].SpellId;
        data << itemProto->Spells[s].SpellTrigger;
        data << itemProto->Spells[s].SpellCharges;
        data << itemProto->Spells[s].SpellCooldown;
        data << itemProto->Spells[s].SpellCategory;
        data << itemProto->Spells[s].SpellCategoryCooldown;
    }
    data << itemProto->Bonding;
    data << itemProto->Description;
    data << itemProto->PageText;
    data << itemProto->LanguageID;
    data << itemProto->PageMaterial;
    data << itemProto->StartQuest;
    data << itemProto->LockID;
    data << itemProto->Material;
    data << itemProto->Sheath;
    data << itemProto->Extra;
    data << itemProto->Block;
    data << itemProto->ItemSet;
    data << itemProto->MaxDurability;
    data << itemProto->Area;
    data << itemProto->Unknown1;                            //unknown1

    //TODO FIX THIS
    //WPAssert(data.size() == 454 + strlen(itemProto->Name1) + strlen(itemProto->Name2) + strlen(itemProto->Name3) + strlen(itemProto->Name4) + strlen(itemProto->Description));
    SendPacket( &data );
}

extern char *fmtstring( char *format, ... );

extern char *GetInventoryImageFilefromObjectClass(uint32 classNum, uint32 subclassNum, uint32 type, uint32 DisplayID);

void WorldSession::HandleReadItem( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: CMSG_READ_ITEM");
    uint8 bagIndex, slot;
    WorldPacket data;
    recv_data >> bagIndex >> slot;

    sLog.outDetail("ITEM: Read, bagIndex = %u, slot = %u", bagIndex, slot);
    Item *pItem = _player->GetItemByPos( bagIndex, slot );

    if (pItem)
    {
        if ((!pItem->GetProto()->PageText) || (_player->isInCombat()) || (_player->isDead()))
        {
            data.Initialize( SMSG_READ_ITEM_FAILED );
            sLog.outDetail("ITEM: Unable to read item");
        }
        else
        {
            data.Initialize (SMSG_READ_ITEM_OK);
            sLog.outDetail("ITEM: Item page sent");
        }
        data << pItem->GetGUID();
        SendPacket(&data);
    }
}

void WorldSession::HandlePageQuerySkippedOpcode( WorldPacket & recv_data )
{
    sLog.outDetail( "WORLD: Received CMSG_PAGE_TEXT_QUERY" );

    WorldPacket data;
    uint32 itemid, guidlow, guidhigh;

    recv_data >> itemid >> guidlow >> guidhigh;

    sLog.outDetail( "Packet Info: itemid: %u guidlow: %u guidhigh: %u", itemid, guidlow, guidhigh );
}

void WorldSession::HandleSellItemOpcode( WorldPacket & recv_data )
{
    sLog.outDetail( "WORLD: Received CMSG_SELL_ITEM" );
    uint64 vendorguid, itemguid;

    recv_data >> vendorguid >> itemguid;

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, vendorguid);
    if( pCreature )
    {
        uint16 pos = _player->GetPosByGuid(itemguid);
        Item *pItem = _player->GetItemByPos( pos );
        if( pItem )
        {
            ItemPrototype *pProto = pItem->GetProto();
            if( pProto )
            {
                if( pProto->SellPrice > 0 )
                {
                    uint32 newmoney = _player->GetUInt32Value(PLAYER_FIELD_COINAGE) + pProto->SellPrice * pItem->GetCount();
                    _player->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney );
                    uint32 buyBackslot = _player->GetCurrentBuybackSlot();
                    _player->AddItemToBuyBackSlot( buyBackslot, pItem );
                    _player->SetCurrentBuybackSlot( buyBackslot + 1 );
                    _player->RemoveItem( (pos >> 8), (pos & 255), true);
                    return;
                }
                else
                    _player->SendSellError( SELL_ERR_CANT_SELL_ITEM, pCreature, itemguid, 0);
                return;
            }
        }
        _player->SendSellError( SELL_ERR_CANT_FIND_ITEM, pCreature, itemguid, 0);
        return;
    }
    _player->SendSellError( SELL_ERR_CANT_FIND_VENDOR, pCreature, itemguid, 0);
}

void WorldSession::HandleBuybackItem(WorldPacket & recv_data)
{
    sLog.outDetail( "WORLD: Received CMSG_BUYBACK_ITEM" );
    uint64 vendorguid;
    uint32 slot;

    recv_data >> vendorguid >> slot;

    Item *pItem = _player->GetItemFromBuyBackSlot( slot );
    if( pItem )
    {
        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, vendorguid);
        if( pCreature )
        {
            uint32 newmoney = _player->GetUInt32Value(PLAYER_FIELD_COINAGE) - _player->GetUInt32Value( PLAYER_FIELD_BUYBACK_PRICE_1 + slot - BUYBACK_SLOT_START );
            if( newmoney < 0 )
            {
                _player->SendBuyError( BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, pItem->GetEntry(), 0);
               return;
            }
            uint16 dest;
            uint8 msg = _player->CanStoreItem( NULL, NULL_SLOT, dest, pItem, false );
            if( msg == EQUIP_ERR_OK )
            {
                _player->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney );
                _player->RemoveItemFromBuyBackSlot( slot );
                _player->StoreItem( dest, pItem, true );
            }
            else
                _player->SendEquipError( msg, pItem, NULL, 0);
            return;
        }
        _player->SendBuyError( BUY_ERR_CANT_FIND_ITEM, pCreature, pItem->GetEntry(), 0);
    }
}

void WorldSession::HandleBuyItemInSlotOpcode( WorldPacket & recv_data )
{
    sLog.outDetail( "WORLD: Received CMSG_BUY_ITEM_IN_SLOT" );
    uint64 vendorguid, bagguid;
    uint32 item;
    uint8 bag, slot, count, vendorslot;

    recv_data >> vendorguid >> item >> bagguid >> slot >> count;
    recv_data.hexlike();

    ItemPrototype *pProto = objmgr.GetItemPrototype( item );
    if( pProto )
    {
        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, vendorguid);
        if( pCreature )
        {
            for(int i = 0; i < pCreature->GetItemCount(); i++)
            {
                if ( pCreature->GetItemId(i) == item )
                {
                    vendorslot = i + 1;
                    break;
                }
            }
            if( !vendorslot )
            {
                _player->SendBuyError( BUY_ERR_CANT_FIND_ITEM, pCreature, item, 0);
                return;
            }
            else
                vendorslot -= 1;
            if( pCreature->GetMaxItemCount( vendorslot ) != 0 && (pCreature->GetItemCount( vendorslot ) - count) < 0 )
            {
                _player->SendBuyError( BUY_ERR_ITEM_ALREADY_SOLD, pCreature, item, 0);
                return;
            }
            if( _player->getLevel() < pProto->RequiredLevel )
            {
                _player->SendBuyError( BUY_ERR_LEVEL_REQUIRE, pCreature, item, 0);
                return;
            }
            int newmoney = (int)_player->GetUInt32Value(PLAYER_FIELD_COINAGE) - (int)pProto->BuyPrice * count;
            if( newmoney < 0 )
            {
                _player->SendBuyError( BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, item, 0);
                return;
            }
            Bag *pBag;
            if( bagguid == _player->GetGUID() )
                bag = INVENTORY_SLOT_BAG_0;
            else
            {
                for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END;i++)
                {
                    pBag = (Bag*)_player->GetItemByPos(INVENTORY_SLOT_BAG_0,i);
                    if( pBag )
                    {
                        if( bagguid == pBag->GetGUID() )
                        {
                            bag = i;
                            break;
                        }
                    }
                }
            }
            uint16 dest = ((bag << 8) | slot);
            uint8 msg;
            if( _player->IsInventoryPos( dest ) )
            {
                msg = _player->CanStoreNewItem( bag, slot, dest, item, pCreature->GetItemBuyCount( vendorslot ) * count, false ); 
                if( msg == EQUIP_ERR_OK )
                {
                    _player->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney );
                    _player->StoreNewItem( dest, item, pCreature->GetItemBuyCount( vendorslot ) * count, true );
                    if( pCreature->GetMaxItemCount( vendorslot ) != 0 )
                        pCreature->SetItemCount( vendorslot, pCreature->GetItemCount( vendorslot ) - pCreature->GetItemBuyCount( vendorslot ) );
                }
                else
                    _player->SendEquipError( msg, NULL, NULL, 0);
            }
        }
        return;
    }
    _player->SendBuyError( BUY_ERR_CANT_FIND_ITEM, NULL, item, 0);
}

void WorldSession::HandleBuyItemOpcode( WorldPacket & recv_data )
{
    sLog.outDetail( "WORLD: Received CMSG_BUY_ITEM" );
    uint64 vendorguid;
    uint32 item;
    uint8 count, unk1, vendorslot;

    recv_data >> vendorguid >> item >> count >> unk1;
    recv_data.hexlike();

    ItemPrototype *pProto = objmgr.GetItemPrototype( item );
    if( pProto )
    {
        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, vendorguid);
        if( pCreature )
        {
            for(int i = 0; i < pCreature->GetItemCount(); i++)
            {
                if ( pCreature->GetItemId(i) == item )
                {
                    vendorslot = i + 1;
                    break;
                }
            }
            if( !vendorslot )
            {
                _player->SendBuyError( BUY_ERR_CANT_FIND_ITEM, pCreature, item, 0);
                return;
            }
            else
                vendorslot -= 1;
            if( pCreature->GetMaxItemCount( vendorslot ) != 0 && (pCreature->GetItemCount( vendorslot ) - count) < 0 )
            {
                _player->SendBuyError( BUY_ERR_ITEM_ALREADY_SOLD, pCreature, item, 0);
                return;
            }
            if( _player->getLevel() < pProto->RequiredLevel )
            {
                _player->SendBuyError( BUY_ERR_LEVEL_REQUIRE, pCreature, item, 0);
                return;
            }
            int newmoney = (int)_player->GetUInt32Value(PLAYER_FIELD_COINAGE) - (int)pProto->BuyPrice * count;
            if( newmoney < 0 )
            {
                _player->SendBuyError( BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, item, 0);
                return;
            }
            uint16 dest;
            uint8 msg = _player->CanStoreNewItem( NULL, NULL_SLOT, dest, item, pCreature->GetItemBuyCount( vendorslot ) * count, false );
            if( msg == EQUIP_ERR_OK )
            {
                _player->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney );
                _player->StoreNewItem( dest, item, pCreature->GetItemBuyCount( vendorslot ) * count, true );
                if( pCreature->GetMaxItemCount( vendorslot ) != 0 )
                    pCreature->SetItemCount( vendorslot, pCreature->GetItemCount( vendorslot ) - pCreature->GetItemBuyCount( vendorslot ) * count );
            }
            else
                _player->SendEquipError( msg, NULL, NULL, 0);
        }
        return;
    }
    _player->SendBuyError( BUY_ERR_CANT_FIND_ITEM, NULL, item, 0);
}

void WorldSession::HandleListInventoryOpcode( WorldPacket & recv_data )
{
    uint64 guid;

    recv_data >> guid;
    sLog.outDetail( "WORLD: Recvd CMSG_LIST_INVENTORY" );

    SendListInventory( guid );
}

void WorldSession::SendListInventory( uint64 guid )
{
    sLog.outDetail( "WORLD: Sent SMSG_LIST_INVENTORY" );
    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if( pCreature )
    {
        uint32 guidlow = GUID_LOPART(guid);
        uint8 numitems = pCreature->GetItemCount();
        uint8 count = 0;
        uint32 ptime = time(NULL);
        uint32 diff;

        WorldPacket data;
        data.Initialize( SMSG_LIST_INVENTORY );
        data << guid;
        data << numitems;

        ItemPrototype *pProto;
        for(int i = 0; i < numitems; i++ )
        {
            if( pCreature->GetItemId(i) != 0 )
            {
                pProto = objmgr.GetItemPrototype(pCreature->GetItemId(i));
                if( pProto )
                {
                    count++;
                    if( pCreature->GetItemIncrTime(i) != 0 && (pCreature->GetItemLastIncr(i) + pCreature->GetItemIncrTime(i) <= ptime) )
                    {
                        diff = uint32((ptime - pCreature->GetItemLastIncr(i))/pCreature->GetItemIncrTime(i));
                        if( (pCreature->GetItemCount(i) + diff * pCreature->GetItemBuyCount(i)) <= pCreature->GetMaxItemCount(i) )
                            pCreature->SetItemCount(i, pCreature->GetItemCount(i) + diff * pCreature->GetItemBuyCount(i));
                        else
                            pCreature->SetItemCount(i, pCreature->GetMaxItemCount(i));
                        pCreature->SetItemLastIncr(i, ptime);
                    }
                    data << uint32( i + 1 );
                    data << pCreature->GetItemId(i);
                    data << pProto->DisplayInfoID;
                    data << uint32(pCreature->GetMaxItemCount(i) <= 0 ? 0xFFFFFFFF : pCreature->GetItemCount(i));
                    data << pProto->BuyPrice;
                    data << uint32( 0xFFFFFFFF );
                    data << pCreature->GetItemBuyCount(i);
                }
            }
        }

        if ( !(data.size() == 8 + 1 + ((count * 7) * 4)) )
            return;
        data.put<uint32>(8, count);
        SendPacket( &data );
    }
}

void WorldSession::HandleAutoStoreBagItemOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_AUTOSTORE_BAG_ITEM");
    uint8 srcbag, srcslot, dstbag;

    recv_data >> srcbag >> srcslot >> dstbag;
    sLog.outDebug("STORAGE: receive srcbag = %u, srcslot = %u, dstbag = %u", srcbag, srcslot, dstbag);

    Item *pItem = _player->GetItemByPos( srcbag, srcslot );
    if( pItem )
    {
        uint16 dest;
        uint8 msg = _player->CanStoreItem( dstbag, NULL_SLOT, dest, pItem, false );
        if( msg == EQUIP_ERR_OK )
        {
            _player->RemoveItem(srcbag, srcslot, true);
            _player->StoreItem( dest, pItem, true );
        }
        else
            _player->SendEquipError( msg, pItem, NULL, 0);
    }
}

void WorldSession::HandleBuyBankSlotOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug("WORLD: CMSG_BUY_BANK_SLOT");
    uint32 bank, result, price, playerGold;

    bank = _player->GetUInt32Value(PLAYER_BYTES_2);
    result = (bank & 0x70000) >> 16;

    sLog.outDetail("PLAYER: Buy bank bag slot, slot number = %u", result);

    // Prices Hardcoded
    switch (result)
    {
        case 0:
            price = 1000;
            break;
        case 1:
            price = 10000;
            break;
        case 2:
            price = 100000;
            break;
        case 3:
            price = 250000;
            break;
        case 4:
            price = 500000;
            break;
        case 5:
            price = 1000000;
            break;
        default:
            return;
    }

    if (result < 6)
    {
        result++;
    }
    else
    {
        return;
    }
    bank = (bank & ~0x70000) + (result << 16);
    playerGold = _player->GetUInt32Value(PLAYER_FIELD_COINAGE);
    if (playerGold >= price)
    {
        _player->SetUInt32Value(PLAYER_BYTES_2, bank);
        _player->SetMoney(playerGold - price);

    }
}

void WorldSession::HandleAutoBankItemOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug("WORLD: CMSG_AUTOBANK_ITEM");
    uint8 srcbag, srcslot;

    recvPacket >> srcbag >> srcslot;
    sLog.outDebug("STORAGE: receive srcbag = %u, srcslot = %u", srcbag, srcslot);

    Item *pItem = _player->GetItemByPos( srcbag, srcslot );
    if( pItem )
    {
        uint16 dest;
        uint8 msg = _player->CanBankItem( NULL, NULL_SLOT, dest, pItem, false );
        if( msg == EQUIP_ERR_OK )
        {
            _player->RemoveItem(srcbag, srcslot, true);
            _player->BankItem( dest, pItem, true );
        }
        else
            _player->SendEquipError( msg, pItem, NULL, 0 );
    }
}

void WorldSession::HandleAutoStoreBankItemOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug("WORLD: CMSG_AUTOSTORE_BANK_ITEM");
    uint8 srcbag, srcslot;

    recvPacket >> srcbag >> srcslot;
    sLog.outDebug("STORAGE: receive srcbag = %u, srcslot = %u", srcbag, srcslot);

    Item *pItem = _player->GetItemByPos( srcbag, srcslot );
    if( pItem )
    {
        uint16 dest;
        uint8 msg = _player->CanBankItem( NULL, NULL_SLOT, dest, pItem, false );
        if( msg == EQUIP_ERR_OK )
        {
            _player->RemoveItem(srcbag, srcslot, true);
            _player->BankItem( dest, pItem, true );
        }
        else
            _player->SendEquipError( msg, pItem, NULL, 0 );
    }
}

void WorldSession::HandleSetAmmoOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("WORLD: CMSG_SET_AMMO");
    uint32 item;
    
    recv_data >> item;

    if( item == 0 )
        GetPlayer()->SetUInt32Value(PLAYER_AMMO_ID, 0);
    else
    {
        uint8 msg = GetPlayer()->CanUseAmmo( item );
        if( msg == EQUIP_ERR_OK )
            GetPlayer()->SetUInt32Value(PLAYER_AMMO_ID, item);
        else
            GetPlayer()->SendEquipError( msg, NULL, NULL, 0);
    }
}

void WorldSession::SendEnchantmentLog(uint64 Target, uint64 Caster,uint32 ItemID,uint32 SpellID)
{
    WorldPacket data;
    data.Initialize(SMSG_ENCHANTMENTLOG);
    data << Target;
    data << Caster;
    data << ItemID;
    data << SpellID;
    data << uint8(0);
    SendPacket(&data);
}
