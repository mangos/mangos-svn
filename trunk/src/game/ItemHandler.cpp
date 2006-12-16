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

        ItemPrototype const *pProto = pItem->GetProto();
        bool not_swapable = pProto && pProto->InventoryType == INVTYPE_BAG;

        uint8 msg = _player->CanEquipItem( NULL_SLOT, dest, pItem, !not_swapable );

        if( msg == EQUIP_ERR_OK )
            msg = _player->CanUnequipItem( dest, !not_swapable );

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
            _player->SendEquipError( msg, pItem, NULL );
    }
}

void WorldSession::HandleDestroyItemOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_DESTROYITEM");
    uint8 bag, slot, count, data1, data2, data3;

    recv_data >> bag >> slot >> count >> data1 >> data2 >> data3;
    sLog.outDebug("STORAGE: receive bag = %u, slot = %u, count = %u", bag, slot, count);

    uint16 pos = (bag << 8) | slot;

    // prevent drop unequipable items (in combat, for example) and non-empty bags
    if(_player->IsEquipmentPos(pos) || _player->IsBagPos(pos))
    {
        uint8 msg = _player->CanUnequipItem( pos, false );
        if( msg != EQUIP_ERR_OK )
        {
            _player->SendEquipError( msg, _player->GetItemByPos(pos), NULL );
            return;
        }
    }

    _player->DestroyItem( bag, slot, true );
}

void WorldSession::HandleItemQuerySingleOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_ITEM_QUERY_SINGLE");

    WorldPacket data;
    data.Initialize( SMSG_ITEM_QUERY_SINGLE_RESPONSE );

    uint32 item, guidLow, guidHigh;
    recv_data >> item >> guidLow >> guidHigh;

    sLog.outDetail("STORAGE: Item Query = %u", item);

    ItemPrototype const *pProto = objmgr.GetItemPrototype( item );
    if( pProto )
    {
        data << pProto->ItemId;
        data << pProto->Class;
        // client known only 0 subclass (and 1-2 obsolute subclasses)
        data << (pProto->Class==ITEM_CLASS_CONSUMABLE ? uint32(0) : pProto->SubClass);
        data << pProto->Name1;
        data << pProto->Name2;
        data << pProto->Name3;
        data << pProto->Name4;
        data << pProto->DisplayInfoID;
        data << pProto->Quality;
        data << pProto->Flags;
        data << pProto->BuyPrice;
        data << pProto->SellPrice;
        data << pProto->InventoryType;
        data << pProto->AllowableClass;
        data << pProto->AllowableRace;
        data << pProto->ItemLevel;
        data << pProto->RequiredLevel;
        data << pProto->RequiredSkill;
        data << pProto->RequiredSkillRank;
        data << pProto->RequiredSpell;
        data << pProto->RequiredHonorRank;
        data << pProto->RequiredCityRank;
        data << pProto->RequiredReputationFaction;
        data << pProto->RequiredReputationRank;
        data << pProto->MaxCount;
        data << pProto->Stackable;
        data << pProto->ContainerSlots;
        for(int i = 0; i < 10; i++)
        {
            data << pProto->ItemStat[i].ItemStatType;
            data << pProto->ItemStat[i].ItemStatValue;
        }
        for(int i = 0; i < 5; i++)
        {
            data << pProto->Damage[i].DamageMin;
            data << pProto->Damage[i].DamageMax;
            data << pProto->Damage[i].DamageType;
        }
        data << pProto->Armor;
        data << pProto->HolyRes;
        data << pProto->FireRes;
        data << pProto->NatureRes;
        data << pProto->FrostRes;
        data << pProto->ShadowRes;
        data << pProto->ArcaneRes;
        data << pProto->Delay;
        data << pProto->Ammo_type;

        data << (float)pProto->RangedModRange;
        for(int s = 0; s < 5; s++)
        {
            // send DBC data for cooldowns in same way as it used in Spell::SendSpellCooldown
            // use `item_template` only if spell not have own cooldowns
            SpellEntry* spell = sSpellStore.LookupEntry(pProto->Spells[s].SpellId);
            bool dbc_data = spell && ( spell->RecoveryTime > 0 || spell->CategoryRecoveryTime > 0 );

            data << pProto->Spells[s].SpellId;
            data << pProto->Spells[s].SpellTrigger;
            data << uint32(pProto->Spells[s].SpellCharges);
            data << uint32(dbc_data ? spell->RecoveryTime         : pProto->Spells[s].SpellCooldown);
            data << uint32(dbc_data ? spell->Category             : pProto->Spells[s].SpellCategory);
            data << uint32(dbc_data ? spell->CategoryRecoveryTime : pProto->Spells[s].SpellCategoryCooldown);
        }
        data << pProto->Bonding;
        data << pProto->Description;
        data << pProto->PageText;
        data << pProto->LanguageID;
        data << pProto->PageMaterial;
        data << pProto->StartQuest;
        data << pProto->LockID;
        data << pProto->Material;
        data << pProto->Sheath;
        data << pProto->Extra;
        data << pProto->Block;
        data << pProto->ItemSet;
        data << pProto->MaxDurability;
        data << pProto->Area;
        data << pProto->Unknown1;
        data << pProto->Unknown1;                           // Added in 1.12.x client branch
    }
    else
    {
        data << item;
        for(int a = 0; a < 11; a++)
            data << uint64(0);
        data << uint32(0);                                  // Added in 1.12.x client branch
        SendPacket( &data );
        return;
    }
    SendPacket( &data );
}

void WorldSession::HandleReadItem( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: CMSG_READ_ITEM");

    WorldPacket data;
    uint8 bag, slot;
    recv_data >> bag >> slot;

    sLog.outDetail("STORAGE: Read bag = %u, slot = %u", bag, slot);
    Item *pItem = _player->GetItemByPos( bag, slot );

    if( pItem && pItem->GetProto()->PageText )
    {
        uint8 msg = _player->CanUseItem( pItem );
        if( msg == EQUIP_ERR_OK )
        {
            data.Initialize (SMSG_READ_ITEM_OK);
            sLog.outDetail("STORAGE: Item page sent");
        }
        else
        {
            data.Initialize( SMSG_READ_ITEM_FAILED );
            sLog.outDetail("STORAGE: Unable to read item");
            _player->SendEquipError( msg, pItem, NULL );
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
    uint8 count;

    recv_data >> vendorguid >> itemguid >> count;

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, vendorguid);
    if( pCreature )
    {
        uint16 pos = _player->GetPosByGuid(itemguid);
        Item *pItem = _player->GetItemByPos( pos );
        if( pItem )
        {
            // prevent sell non empty bag by drag-and-drop at vendor's item list
            if(pItem->IsBag() && !((Bag*)pItem)->IsEmpty())
            {
                _player->SendSellError( SELL_ERR_CANT_SELL_ITEM, pCreature, itemguid, 0);
                return;
            }

            // special case at auto sell (sell all)
            if(count==0)
            {
                count = pItem->GetCount();
            }
            else
            // prevent sell more items that exist in stack (possable only not from client)
            if(count > pItem->GetCount())
            {
                _player->SendSellError( SELL_ERR_CANT_SELL_ITEM, pCreature, itemguid, 0);
                return;
            }

            ItemPrototype const *pProto = pItem->GetProto();
            if( pProto )
            {
                if( pProto->SellPrice > 0 )
                {
                    _player->ModifyMoney( pProto->SellPrice * count );

                    if(count < pItem->GetCount())           // need split items
                    {
                        pItem->SetCount( pItem->GetCount() - count );
                        if( _player->IsInWorld() )
                            pItem->SendUpdateToPlayer( _player );
                        pItem->SetState(ITEM_CHANGED, _player);

                        Item *pNewItem = _player->CreateItem( pItem->GetEntry(), count );
                        _player->AddItemToBuyBackSlot( pNewItem );
                        if( _player->IsInWorld() )
                            pNewItem->SendUpdateToPlayer( _player );
                    }
                    else
                    {
                        _player->RemoveItem( (pos >> 8), (pos & 255), true);
                        pItem->RemoveFromUpdateQueueOf(_player);
                        _player->AddItemToBuyBackSlot( pItem );
                    }
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
            uint32 price = _player->GetUInt32Value( PLAYER_FIELD_BUYBACK_PRICE_1 + slot - BUYBACK_SLOT_START );
            if( _player->GetMoney() < price )
            {
                _player->SendBuyError( BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, pItem->GetEntry(), 0);
                return;
            }
            uint16 dest;
            uint8 msg = _player->CanStoreItem( NULL_BAG, NULL_SLOT, dest, pItem, false );
            if( msg == EQUIP_ERR_OK )
            {
                _player->ModifyMoney( -(int32)price );
                _player->RemoveItemFromBuyBackSlot( slot, false );
                _player->StoreItem( dest, pItem, true );
            }
            else
                _player->SendEquipError( msg, pItem, NULL );
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

    ItemPrototype const *pProto = objmgr.GetItemPrototype( item );
    if( pProto )
    {
        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, vendorguid);
        if( pCreature )
        {
            vendorslot = 0;
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
            uint32 price  = pProto->BuyPrice * count;
            if( _player->GetMoney() < price )
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
                msg = _player->CanStoreNewItem( bag, slot, dest, item, pProto->BuyCount * count, false );
                if( msg == EQUIP_ERR_OK )
                {
                    _player->ModifyMoney( -(int32)price );
                    _player->StoreNewItem( dest, item, pProto->BuyCount * count, true );
                    if( pCreature->GetMaxItemCount( vendorslot ) != 0 )
                        pCreature->SetItemCount( vendorslot, pCreature->GetItemCount( vendorslot ) - pProto->BuyCount );
                }
                else
                    _player->SendEquipError( msg, NULL, NULL );
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

    ItemPrototype const *pProto = objmgr.GetItemPrototype( item );
    if( pProto )
    {
        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, vendorguid);
        if( pCreature )
        {
            vendorslot = 0;
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

            uint32 price = pProto->BuyPrice * count;

            if( _player->GetMoney() < price )
            {
                _player->SendBuyError( BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, item, 0);
                return;
            }
            uint16 dest;
            uint8 msg = _player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, item, pProto->BuyCount * count, false );
            if( msg == EQUIP_ERR_OK )
            {
                _player->ModifyMoney( -(int32)price );
                _player->StoreNewItem( dest, item, pProto->BuyCount  * count, true );
                if( pCreature->GetMaxItemCount( vendorslot ) != 0 )
                    pCreature->SetItemCount( vendorslot, pCreature->GetItemCount( vendorslot ) - pProto->BuyCount * count );
            }
            else
                _player->SendEquipError( msg, NULL, NULL );
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
        uint8 numitems = pCreature->GetItemCount();
        uint32 count = 0;
        uint32 ptime = time(NULL);
        uint32 diff;

        WorldPacket data;
        data.Initialize( SMSG_LIST_INVENTORY );
        data << guid;
        data << numitems;

        ItemPrototype const *pProto;
        for(int i = 0; i < numitems; i++ )
        {
            if( pCreature->GetItemId(i) )
            {
                pProto = objmgr.GetItemPrototype(pCreature->GetItemId(i));
                if( pProto )
                {
                    count++;
                    if( pCreature->GetItemIncrTime(i) != 0 && (pCreature->GetItemLastIncr(i) + pCreature->GetItemIncrTime(i) <= ptime) )
                    {
                        diff = uint32((ptime - pCreature->GetItemLastIncr(i))/pCreature->GetItemIncrTime(i));
                        if( (pCreature->GetItemCount(i) + diff * pProto->BuyCount) <= pCreature->GetMaxItemCount(i) )
                            pCreature->SetItemCount(i, pCreature->GetItemCount(i) + diff * pProto->BuyCount);
                        else
                            pCreature->SetItemCount(i, pCreature->GetMaxItemCount(i));
                        pCreature->SetItemLastIncr(i, ptime);
                    }
                    data << uint32( 0xFFFFFFFF );           // unknown (not index item in sell list!) this value fix not selling first item in list
                    data << pCreature->GetItemId(i);
                    data << pProto->DisplayInfoID;
                    data << uint32(pCreature->GetMaxItemCount(i) <= 0 ? 0xFFFFFFFF : pCreature->GetItemCount(i));
                    data << pProto->BuyPrice;
                    data << uint32( 0xFFFFFFFF );
                    data << pProto->BuyCount;
                }
            }
        }

        if ( count == 0 || data.size() != 8 + 1 + count * 7 * 4 )
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
            _player->SendEquipError( msg, pItem, NULL );
    }
}

void WorldSession::HandleBuyBankSlotOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug("WORLD: CMSG_BUY_BANK_SLOT");

    uint32 bank = _player->GetUInt32Value(PLAYER_BYTES_2);
    uint32 slot = (bank & 0x70000) >> 16;

    // next slot
    ++slot;

    sLog.outDetail("PLAYER: Buy bank bag slot, slot number = %u", slot);

    BankBagSlotPricesEntry const* slotEntry = sBankBagSlotPricesStore.LookupEntry(slot);

    if(!slotEntry)
        return;

    uint32 price = slotEntry->price;

    if (_player->GetMoney() >= price)
    {
        bank = (bank & ~0x70000) + (slot << 16);

        _player->SetUInt32Value(PLAYER_BYTES_2, bank);
        _player->ModifyMoney(-int32(price));
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
        uint8 msg = _player->CanBankItem( NULL_BAG, NULL_SLOT, dest, pItem, false );
        if( msg == EQUIP_ERR_OK )
        {
            _player->RemoveItem(srcbag, srcslot, true);
            _player->BankItem( dest, pItem, true );
        }
        else
            _player->SendEquipError( msg, pItem, NULL );
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
        uint8 msg = _player->CanBankItem( NULL_BAG, NULL_SLOT, dest, pItem, false );
        if( msg == EQUIP_ERR_OK )
        {
            _player->RemoveItem(srcbag, srcslot, true);
            _player->BankItem( dest, pItem, true );
        }
        else
            _player->SendEquipError( msg, pItem, NULL );
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
            GetPlayer()->SendEquipError( msg, NULL, NULL );
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

void WorldSession::SendItemEnchantTimeUpdate(uint64 Itemguid,uint32 slot,uint32 Duration)
{
    WorldPacket data;
    data.Initialize(SMSG_ITEM_ENCHANT_TIME_UPDATE);
    data << Itemguid;
    data << slot;
    data << Duration;
    SendPacket(&data);
}

void WorldSession::HandleItemNameQueryOpcode(WorldPacket & recv_data)
{
    uint32 itemid;
    recv_data >> itemid;
    sLog.outDebug("WORLD: CMSG_ITEM_NAME_QUERY %u", itemid);
    ItemPrototype const *pProto = objmgr.GetItemPrototype( itemid );
    if( pProto )
    {
        WorldPacket data;
        data.Initialize(SMSG_ITEM_NAME_QUERY_RESPONSE);
        data << pProto->ItemId;
        data << pProto->Name1;
        SendPacket(&data);
        return;
    }
    else
        sLog.outDebug("WORLD: CMSG_ITEM_NAME_QUERY for item %u failed (unknown item)", itemid);
}
