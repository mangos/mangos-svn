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
    uint8 srcBag, srcSlot, dstBag, dstSlot, count;

    recv_data >> srcBag >> srcSlot >> dstBag >> dstSlot >> count;

    sLog.outDetail("ITEM: Split item, srcBag = %u, srcSlot = %u, dstBag = %u, dstSlot = %u, count = %u",srcBag, srcSlot, dstBag, dstSlot, count);

    _player->SplitItem(srcBag, srcSlot, dstBag, dstSlot, count);
}

void WorldSession::HandleSwapInvItemOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_SWAP_INV_ITEM");
    WorldPacket data;
    UpdateData upd;
    uint8 srcSlot, dstSlot;

    recv_data >> srcSlot >> dstSlot;

    sLog.outDetail("ITEM: Swap inventory, srcSlot = %u, dstSlot = %u", (uint32)srcSlot, (uint32)dstSlot);

    _player->SwapItem(0, dstSlot, 0, srcSlot);
}

void WorldSession::HandleSwapItem( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_SWAP_ITEM");
    WorldPacket data;
    uint8 dstBag, dstSlot, srcBag, srcSlot;

    recv_data >> dstBag >> dstSlot >> srcBag >> srcSlot ;
    sLog.outDetail("ITEM: Swap, srcBag = %u, srcSlot = %u, dstBag = %u, dstSlot = %u", srcBag, srcSlot, dstBag, dstSlot);

    _player->SwapItem(dstBag, dstSlot, srcBag, srcSlot);
}

void WorldSession::HandleAutoEquipItemOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_AUTOEQUIP_ITEM");
    WorldPacket data;
    uint8 srcBag, srcSlot, dstSlot, error_msg = 0;

    recv_data >> srcBag >> srcSlot;
    sLog.outDetail("ITEM: Auto equip, srcBag = %u, srcSlot = %u", srcBag, srcSlot);

    Item *item  = _player->GetItemBySlot(srcBag, srcSlot);

    if (!item) error_msg = EQUIP_ERR_ITEM_NOT_FOUND;

    if (!error_msg)
    {
        dstSlot = _player->FindEquipSlot(item->GetProto()->InventoryType);
        if (dstSlot == INVENTORY_SLOT_ITEM_END) error_msg = EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;
    }

    if (error_msg)
    {
        data.Initialize(SMSG_INVENTORY_CHANGE_FAILURE);
        data << error_msg;
        data << (item ? item->GetGUID() : uint64(0));
        data << uint64(0);
        data << uint8(0);
        SendPacket( &data );
        return;
    }

    _player->SwapItem(CLIENT_SLOT_BACK,dstSlot,srcBag,srcSlot);
}

void WorldSession::HandleDestroyItemOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_DESTROYITEM");
    WorldPacket data;
    uint8 bagIndex, slot, count, data1, data2, data3;

    recv_data >> bagIndex >> slot >> count >> data1 >> data2 >> data3;

    sLog.outDetail("ITEM: Destroy, bagIndex = %u, slot = %u, count = %u (Uknown data: %u %u %u)", bagIndex, slot, count, data1, data2, data3);

    Item *item = _player->GetItemBySlot(bagIndex,slot);

    if (!item)
    {
        sLog.outDetail("ITEM: Tried to destroy a non-existant item");
        return;
    }

    if ((!count) || (count >= item->GetCount()))
    {
        _player->RemoveItemFromSlot(bagIndex,slot);
        //item->DeleteFromDB();
        delete item;
    }
    else
    {
        item->SetCount(item->GetCount() - count);
    }
    //_player->_SaveInventory();
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
    data << uint32(0);                                      //unknown1

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
    Item *pItem = _player->GetItemBySlot(bagIndex, slot);

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

    WorldPacket data;
    uint64 vendorguid, itemguid;
    uint8 amount;
    uint32 newmoney;
    //uint8 slot = 0xFF;
    int check = 0;

    recv_data >> vendorguid;
    recv_data >> itemguid;
    recv_data >> amount;

    if (itemguid == 0)
    {
        data.Initialize( SMSG_SELL_ITEM );
        data << vendorguid << itemguid << uint8(0x01);
        WPAssert(data.size() == 17);
        SendPacket( &data );
        return;
    }

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, vendorguid);

    if (unit == NULL)
    {
        data.Initialize( SMSG_SELL_ITEM );
        data << vendorguid << itemguid << uint8(0x03);
        WPAssert(data.size() == 17);
        SendPacket( &data );
        return;
    }

    Item *item=NULL;
    uint8 bag,slot;
    if (_player->GetSlotByItemGUID(itemguid,bag,slot))
    {
        item = _player->GetItemBySlot(bag,slot);
        if (item)
        {
            if(item->GetProto()->SellPrice !=0)
            {
                data.Initialize( SMSG_SELL_ITEM );
                data << vendorguid << itemguid << uint8(0x0);
                WPAssert(data.size() == 17);
                SendPacket( &data );

                //if (amount == 0) amount = 1;
                newmoney = ((_player->GetUInt32Value(PLAYER_FIELD_COINAGE)) + (item->GetProto()->SellPrice) * item->GetCount());
                _player->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);

                uint32 buyBackslot=_player->GetCurrentBuybackSlot();
                _player->AddItemToBuyBackSlot(buyBackslot,item);
                _player->SetCurrentBuybackSlot(buyBackslot+1);
                _player->RemoveItemFromSlot(bag,slot,false);
            }
            else
            {
                data.Initialize( SMSG_SELL_ITEM );
                data << vendorguid << itemguid << uint8(0x02);
                WPAssert(data.size() == 17);
                SendPacket( &data );
            }
        }
    }
    else
    {
        data.Initialize( SMSG_SELL_ITEM );
        data << vendorguid << itemguid << uint8(0x01);
        WPAssert(data.size() == 17);
        SendPacket( &data );
        return;
    }
    sLog.outDetail( "WORLD: Sent SMSG_SELL_ITEM" );
}

void WorldSession::HandleBuybackItem(WorldPacket & recv_data)
{
    sLog.outDetail( "WORLD: Received CMSG_BUYBACK_ITEM" );
    uint64 vendorguid;
    uint32 buybackslot;
    WorldPacket data;

    recv_data >> vendorguid;
    recv_data >> buybackslot;                               //start slot is (69 0x45) end slot is 0x45+12

    //sLog.outDetail( "Packet Info: vendorguid: %u buybackslot: %u ", vendorguid, buybackslot);
    Item *buybackItem=NULL;
    uint32 slot=buybackslot-0x45;
    buybackItem = _player->GetItemFromBuyBackSlot(slot);
    if (buybackItem)
    {
        int32 newmoney;
        newmoney = ((_player->GetUInt32Value(PLAYER_FIELD_COINAGE)) - (_player->GetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1+slot)));
        if(newmoney < 0 )
        {
            data.Initialize( SMSG_BUY_FAILED );
            data << uint64(buybackItem->GetGUID());
            data << uint32(buybackItem->GetEntry());
            data << uint8(2);
            SendPacket( &data );
            return;
        }
        if(_player->CanAddItemCount(buybackItem, 1) >= buybackItem->GetCount())
        {
            _player->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);
            _player->AddItemToInventory(buybackItem, true);
            _player->RemoveItemFromBuyBackSlot(slot);
        }
    }
}

void WorldSession::HandleBuyItemInSlotOpcode( WorldPacket & recv_data )
{
    sLog.outDetail( "WORLD: Received CMSG_BUY_ITEM_IN_SLOT" );

    WorldPacket data;
    uint64 srcguid, dstguid;
    uint32 itemid;
    uint8 slot, amount;
    int vendorslot = -1;
    int32 newmoney;
    Bag *bag;
    uint8 slotType=0;
    bool buyOk = false;
    UpdateData upd;
    WorldPacket packet;

    /*
    SMSG_BUY_FAILED error
    0: cant find item
    1: item already selled
    2: not enought money
    4: seller dont like u
    5: distance too far
    8: cant carry more
    11: level require
    12: reputation require
    */

    recv_data >> srcguid >> itemid >> dstguid >> slot >> amount;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, srcguid);
    Player *player = _player;

    if (unit == NULL)
        return;

    if(dstguid == _player->GetGUID())
    {
        slotType=CLIENT_SLOT_BACK;
    }
    else
    {
        for (uint8 i=CLIENT_SLOT_01;i<=CLIENT_SLOT_04;i++)
        {
            bag=_player->GetBagBySlot(i);
            if(bag)
            {
                if( dstguid == bag->GetGUID() )
                {
                    slotType = i;
                    break;
                }
            }
        }
    }

    ItemPrototype *ItemPro = objmgr.GetItemPrototype(itemid);
    if(ItemPro)
    {
        //check money
        if( slotType>0 )
        {
            newmoney = ((_player->GetUInt32Value(PLAYER_FIELD_COINAGE)) - (ItemPro->BuyPrice));
            if(newmoney < 0 )
            {
                data.Initialize( SMSG_BUY_FAILED );
                data << uint64(srcguid);
                data << uint32(itemid);
                data << uint8(2);
                SendPacket( &data );
                return;
            }
            //check level
            if(ItemPro->RequiredLevel> _player->getLevel())
            {
                data.Initialize( SMSG_BUY_FAILED );
                data << uint64(srcguid);
                data << uint32(itemid);
                data << uint8(11);
                SendPacket( &data );
                return;
            }
            // Check if item is usable
            if( (slot < EQUIPMENT_SLOT_END) && (slotType==CLIENT_SLOT_BACK) )
            {
                uint8 error =0;
                //	            uint8 error = _player->CanEquipItem(ItemPro);
                if (error)
                {
                    data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
                    data << uint8(error);
                    if (error == EQUIP_ERR_YOU_MUST_REACH_LEVEL_N)
                    {
                        data << (uint32)ItemPro->RequiredLevel;
                        data << uint64(srcguid);
                        data << uint64(0);
                        data << uint8(0);
                        SendPacket( &data );
                        return;
                    }
                }
            }
            //}
            //check seller
            for(uint8 i = 0; i < unit->getItemCount(); i++)
            {
                if (unit->getItemId(i) == itemid)
                {
                    amount = unit->getItemAmount(i);
                    vendorslot = i;
                    break;
                }
            }
            if(( vendorslot == -1 )||((amount > unit->getItemAmount(vendorslot)) &&
                (unit->getItemAmount(vendorslot)>=0)))
            {
                data.Initialize( SMSG_BUY_FAILED );
                data << uint64(srcguid);
                data << uint32(itemid);
                data << uint8(1);
                SendPacket( &data );
                return;
            }
            //add to slot
            if(_player->AddNewItem (itemid,amount,false))
            {
                _player->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);
                data.Initialize( SMSG_BUY_ITEM );
                data << uint64(srcguid);
                data << uint32(itemid) << uint32(amount);
                SendPacket( &data );
                sLog.outDetail( "WORLD: Sent SMSG_BUY_ITEM" );
            }
            else
            {
                data.Initialize( SMSG_BUY_FAILED );
                data << uint64(srcguid);
                data<< uint32(itemid);
                data << uint8(8);
                SendPacket( &data );
            }
        }
    }                                                       //if(!ItemPro)
    else
    {
        data.Initialize( SMSG_BUY_FAILED );
        data << uint64(srcguid);
        data<< uint32(itemid);
        data << uint8(0);
        SendPacket( &data );
    }
}

void WorldSession::HandleBuyItemOpcode( WorldPacket & recv_data )
{
    sLog.outDetail( "WORLD: Received CMSG_BUY_ITEM" );

    WorldPacket data;
    uint64 srcguid;
    uint32 itemid;
    uint8 amount, slot;
    uint8 playerslot = 0;
    int vendorslot = -1;
    int32 newmoney;

    recv_data >> srcguid >> itemid;
    recv_data >> amount >> slot;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, srcguid);

    if (unit == NULL)
        return;
    ItemPrototype *ItemPro = objmgr.GetItemPrototype(itemid);
    if(ItemPro)
    {
        for(uint8 i = 0; i < unit->getItemCount(); i++)
        {
            if (unit->getItemId(i) == itemid)
            {
                vendorslot = i;
                amount = unit->getItemAmount(i);
                break;
            }
        }

        if(( vendorslot == -1 )||((amount > unit->getItemAmount(vendorslot)) &&
            (unit->getItemAmount(vendorslot)>=0)))
        {
            data.Initialize( SMSG_BUY_FAILED );
            data << uint64(srcguid);
            data << uint32(itemid);
            data << uint8(1);
            SendPacket( &data );
            return;
        }

        newmoney = ((_player->GetUInt32Value(PLAYER_FIELD_COINAGE)) - (objmgr.GetItemPrototype(itemid)->BuyPrice));
        if(newmoney < 0 )
        {
            data.Initialize( SMSG_BUY_FAILED );
            data << uint64(srcguid);
            data << uint32(itemid);
            data << uint8(2);
            SendPacket( &data );
            return;
        }

        if (_player->AddNewItem(itemid,amount,false))
        {
            _player->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);
            data.Initialize( SMSG_BUY_ITEM );
            data << uint64(srcguid);
            data << uint32(itemid) << uint32(amount);
            SendPacket( &data );
            sLog.outDetail( "WORLD: Sent SMSG_BUY_ITEM" );
            return;
        }
        else
        {
            data.Initialize( SMSG_BUY_FAILED );
            data << uint64(srcguid);
            data<< uint32(itemid);
            data << uint8(8);
            SendPacket( &data );
        }
    }
    else
    {
        data.Initialize( SMSG_BUY_FAILED );
        data << uint64(srcguid);
        data<< uint32(itemid);
        data << uint8(0);
        SendPacket( &data );
    }
}

void WorldSession::HandleListInventoryOpcode( WorldPacket & recv_data )
{

    WorldPacket data;
    uint64 guid;

    recv_data >> guid;
    sLog.outDetail( "WORLD: Recvd CMSG_LIST_INVENTORY %u", guid );
    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (unit == NULL)
        return;

    uint8 numitems = (uint8)unit->getItemCount();
    uint8 actualnumitems = 0;
    uint8 i = 0;

    for(i = 0; i < numitems; i ++ )
    {
        if(unit->getItemId(i) != 0) actualnumitems++;
    }
    uint32 guidlow = GUID_LOPART(guid);

    data.Initialize( SMSG_LIST_INVENTORY );
    data << guid;
    data << uint8( actualnumitems );

    ItemPrototype * curItem;
    for(i = 0; i < numitems; i++ )
    {
        if(unit->getItemId(i) != 0)
        {
            curItem = unit->getProtoByslot(i);

            if( !curItem )
            {
                sLog.outError( "Unit %i has nonexistant item %i! the item will be removed next time", guid, unit->getItemId(i) );
                for( int a = 0; a < 7; a ++ )
                    data << uint32( 0 );

                // That should be OR or AND ?
                sDatabase.PExecute("DELETE * FROM `npc_vendor` WHERE `entry` = '%u' AND `itemguid` = '%u'", unit->GetEntry(),unit->getItemId(i));

                unit->setItemAmount(i,0);
                unit->setItemId(i,0);

            }
            else
            {
                data << uint32( i + 1 );

                data << uint32( unit->getItemId(i) );

                data << uint32( curItem->DisplayInfoID );

                data << uint32( unit->getItemAmount(i) );

                data << uint32( curItem->BuyPrice );
                data << uint32( 0 );
                data << uint32( 0 );
            }
        }
    }

    if (!(data.size() == 8 + 1 + ((actualnumitems * 7) * 4)))
        return;

    WPAssert(data.size() == 8 + 1 + ((actualnumitems * 7) * 4));
    SendPacket( &data );
    sLog.outDetail( "WORLD: Sent SMSG_LIST_INVENTORY" );
}

void WorldSession::SendListInventory( uint64 guid )
{
    WorldPacket data;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (unit == NULL)
        return;

    uint8 numitems = (uint8)unit->getItemCount();
    uint8 actualnumitems = 0;
    uint8 i = 0;

    for(i = 0; i < numitems; i ++ )
    {
        if(unit->getItemId(i) != 0) actualnumitems++;
    }
    uint32 guidlow = GUID_LOPART(guid);

    data.Initialize( SMSG_LIST_INVENTORY );
    data << guid;
    data << uint8( actualnumitems );

    ItemPrototype * curItem;
    for(i = 0; i < numitems; i++ )
    {
        if(unit->getItemId(i) != 0)
        {
            curItem = unit->getProtoByslot(i);
            if( !curItem )
            {
                sLog.outError( "Unit %i has nonexistant item %i! the item will be removed next time", guid, unit->getItemId(i) );
                for( int a = 0; a < 7; a ++ )
                    data << uint32( 0 );

                sDatabase.PExecute("DELETE * FROM `npc_vendor` WHERE `entry` = '%u' AND `itemguid` = '%u'", unit->GetEntry(),unit->getItemId(i));

                unit->setItemAmount(i,0);
                unit->setItemId(i,0);
            }
            else
            {
                data << uint32( i + 1 );

                data << uint32( unit->getItemId(i) );

                data << uint32( curItem->DisplayInfoID );

                data << uint32( unit->getItemAmount(i) );

                data << uint32( curItem->BuyPrice );
                data << uint32( 0 );
                data << uint32( 0 );
            }
        }
    }

    if (!(data.size() == 8 + 1 + ((actualnumitems * 7) * 4)))
        return;

    WPAssert(data.size() == 8 + 1 + ((actualnumitems * 7) * 4));
    SendPacket( &data );
    sLog.outDetail( "WORLD: Sent SMSG_LIST_INVENTORY" );
}

void WorldSession::HandleAutoStoreBagItemOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_AUTOSTORE_BAG_ITEM");
    WorldPacket data;
    uint8 srcBag, srcSlot, dstBag, result;

    recv_data >> srcBag >> srcSlot >> dstBag;

    sLog.outDetail("ITEM: Autostore item, srcBag = %u, srcSlot = %u, dstBag = %u", srcBag, srcSlot, dstBag);

    Item *pItem = _player->GetItemBySlot(srcBag, srcSlot);

    if (!pItem)
    {
        sLog.outDetail("ITEM: The item doesn't exist");
        return;
    }

    result = (_player->CanAddItemCount(pItem, 1) >= pItem->GetCount()) ? 1 : 0;

    if (!result)
    {
        data.Initialize(SMSG_INVENTORY_CHANGE_FAILURE);
        data << uint8(EQUIP_ERR_BAG_FULL);
        data << (pItem ? pItem->GetGUID() : uint64(0));
        data << uint64(0);
        data << uint8(0);
        SendPacket(&data);
        return;
    }

    _player->RemoveItemFromSlot(srcBag, srcSlot);
    result = _player->AddItemToInventory(pItem, false);
    if (result == 2)
    {
        //pItem->DeleteFromDB();
        delete pItem;
    }
    //_player->_SaveInventory();
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
    WorldPacket data;
    uint8 bagIndex, slot, result;
    Item *pItem;

    recvPacket >> bagIndex >> slot;

    pItem = _player->GetItemBySlot(bagIndex, slot);

    if (!pItem)
    {
        sLog.outDetail("ITEM: Unknown item");
        return;
    }

    result = (_player->CanAddItemCount(pItem, 2) >= pItem->GetCount()) ? 1 : 0;

    if (!result)
    {
        data.Initialize(SMSG_INVENTORY_CHANGE_FAILURE);
        data << uint8(EQUIP_ERR_BANK_FULL);
        data << (pItem ? pItem->GetGUID() : uint64(0));
        data << uint64(0);
        data << uint8(0);
        SendPacket(&data);
        return;
    }

    _player->RemoveItemFromSlot(bagIndex, slot);
    result = _player->AddItemToBank(pItem, true);
    if (result == 2)
    {
        //pItem->DeleteFromDB();
        delete pItem;
    }
    //_player->_SaveInventory();
}

void WorldSession::HandleAutoStoreBankItemOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug("WORLD: CMSG_AUTOSTORE_BANK_ITEM");
    WorldPacket data;
    uint8 bagIndex, slot, result;
    Item *pItem;

    recvPacket >> bagIndex >> slot;

    pItem = _player->GetItemBySlot(bagIndex, slot);

    if (!pItem)
    {
        sLog.outDetail("ITEM: Unknown item");
        return;
    }

    result = (_player->CanAddItemCount(pItem, 2) >= pItem->GetCount())? 1 : 0;

    if (!result)
    {
        data.Initialize(SMSG_INVENTORY_CHANGE_FAILURE);
        data << uint8(EQUIP_ERR_INVENTORY_FULL);
        data << (pItem ? pItem->GetGUID() : uint64(0));
        data << uint64(0);
        data << uint8(0);
        SendPacket(&data);
        return;
    }

    _player->RemoveItemFromSlot(bagIndex, slot);
    result = _player->AddItemToBank(pItem, true);
    if (result == 2)
    {
        //pItem->DeleteFromDB();
        delete pItem;
    }
    //_player->_SaveInventory();
}
