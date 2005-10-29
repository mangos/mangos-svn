/* ItemHandler.cpp
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
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Chat.h"
#include "Item.h"
#include "UpdateData.h"

#ifdef ENABLE_GRID_SYSTEM
#include "ObjectAccessor.h"
#endif

void WorldSession::HandleSwapInvItemOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    UpdateData upd;
    uint8 srcslot, dstslot;
    //uint32 srcslot, dstslot;

    recv_data >> srcslot >> dstslot;

    Log::getSingleton().outDetail("ITEM: swap, src slot: %u dst slot: %u", (uint32)srcslot, (uint32)dstslot);

    // these are the bags slots...ignore it for now
    //if (dstslot >= INVENTORY_SLOT_BAG_1 && dstslot <= INVENTORY_SLOT_BAG_4)
    //    dstslot = srcslot;

    // these are then bankbag slots...ignore them for now
    //if (dstslot >= 63 && dstslot <= 69)
    //    dstslot = srcslot;

    Item * dstitem = GetPlayer()->GetItemBySlot(dstslot);
    Item * srcitem = GetPlayer()->GetItemBySlot(srcslot);

    Log::getSingleton().outDetail(" #SKILL: %u", (uint32)srcitem->GetProto()->RequiredSkill );

    // check to make sure items are not being put in wrong spots
    if ( (srcslot >= INVENTORY_SLOT_BAG_START && srcslot < BANK_SLOT_BAG_END)&&
     (dstslot >= EQUIPMENT_SLOT_START && dstslot < EQUIPMENT_SLOT_END)
       )
    {
        uint8 error = GetPlayer()->CanEquipItemInSlot(dstslot, srcitem->GetProto());
        if (!srcitem || error)
        {
            data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
            data << (uint8)error;
            data << (uint64)srcitem->GetProto()->RequiredLevel;
            data << (uint64)0;
            data << (uint8)0;

            SendPacket( &data );
            //Atualiza os slots
            GetPlayer()->UpdateSlot(srcslot);
            GetPlayer()->UpdateSlot(dstslot);
            return;
        }
    }
    //Se existir um item no destino é preciso saber se ele pode trocar de lugar com o item da fonte        
    if(srcslot >= EQUIPMENT_SLOT_START && srcslot < EQUIPMENT_SLOT_END )
    {
        if(dstitem)
        {
            uint8 error = GetPlayer()->CanEquipItemInSlot(srcslot, dstitem->GetProto());
            if (error)
            {
                data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
                data << (uint8)error;
                data << (uint64)dstitem->GetProto()->RequiredLevel;
                data << (uint64)0;
                data << (uint8)0;

                SendPacket( &data );
                //Atualiza os slots
                GetPlayer()->UpdateSlot(srcslot);
                GetPlayer()->UpdateSlot(dstslot);
                return;
            }
        }
    }

    // error
    if (srcslot == dstslot)
    {
        data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
        data << uint8(0x16); //item is not fount
        data << (dstitem ? dstitem->GetGUID() : uint64(0));
        data << (srcitem ? srcitem->GetGUID() : uint64(0));
        data << uint8(0);

        SendPacket( &data );
        return;
    }

    // swap items
    GetPlayer()->SwapItemSlots(srcslot, dstslot);

//    GetPlayer( )->BuildUpdateObjectMsg( &data, &updateMask );
//    GetPlayer( )->SendMessageToSet( &data, false );
}


void WorldSession::HandleDestroyItemOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    uint8 srcslot, dstslot;

    recv_data >> srcslot >> dstslot;

    Log::getSingleton().outDetail("ITEM: destroy, src slot: %u dst slot: %u", (uint32)srcslot, (uint32)dstslot);

    Item *item = GetPlayer()->GetItemBySlot(dstslot);

    if(!item)
    {
        Log::getSingleton().outDetail("ITEM: tried to destroy non-existant item");
        return;
    }

    GetPlayer()->RemoveItemFromSlot(dstslot);

    item->DeleteFromDB();

    delete item;
}


void WorldSession::HandleAutoEquipItemOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    uint8 srcslot, dstslot;

    recv_data >> srcslot >> dstslot;

    Log::getSingleton().outDetail("ITEM: autoequip, src slot: %u dst slot: %u", (uint32)srcslot, (uint32)dstslot);

    Item *item  = GetPlayer()->GetItemBySlot(dstslot);

    //Se o item nao existe
    if(!item)
    {
        Log::getSingleton().outDetail("ITEM: tried to equip non-existant item");

        data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
        data << uint8(0x16); //item is not found
        data << uint64(0);
        data << uint64(0);
        data << uint8(0);

        SendPacket( &data );

        return;
    }

    //Se o nivel do item é compativel com o nivel do CHAR
    uint32 charLvl = GetPlayer()->getLevel();
    uint32 itemLvl = item->GetProto()->RequiredLevel;

    Log::getSingleton( ).outDetail("ITEM: CharLvl %d, ItemLvl %d", charLvl, itemLvl);

    if( charLvl < itemLvl)
    {
        data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
        data << (uint8)0x01; // You must reached on level ? to use that item
        data << (uint64)item->GetProto()->RequiredLevel;
        data << (uint64)0;
        data << (uint8)0;
        
        SendPacket( &data );
        GetPlayer()->UpdateSlot(dstslot);
        return;
    }

    //Acha um slot vazio
    uint8 slot = GetPlayer()->FindFreeItemSlot(item->GetProto()->InventoryType);
    //Se o slot retornado nao for fora da BAG
    if (slot == INVENTORY_SLOT_ITEM_END)
    {
        data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
        data << uint8(0x03); //that item do not go in this slot
        data << item->GetGUID();
        data << uint64(0);
        data << uint8(0);
        WPAssert(data.size() == 18);
        SendPacket( &data );
        GetPlayer()->UpdateSlot(dstslot);
        return;
    }

    //Se o item da bag pode equipar o CHAR
    uint8 error = GetPlayer()->CanEquipItemInSlot(slot, item->GetProto());  
    
    if ( error )
    {
        data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
        data << error;        
        data << (uint64)item->GetProto()->RequiredLevel;
        data << (uint64)0;
        data << (uint8)0;
        SendPacket( &data );
        GetPlayer()->UpdateSlot(dstslot);
        return;
    }

    GetPlayer()->SwapItemSlots(dstslot, slot);
}

void WorldSession::HandleItemQuerySingleOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    int i;
    uint32 itemid, guidlow, guidhigh;
    recv_data >> itemid >> guidlow >> guidhigh;   // guid is the guid of the ITEM OWNER - NO ITS NOT

    Log::getSingleton( ).outDetail( "WORLD: Recvd CMSG_ITEM_QUERY_SINGLE for item id 0x%.8X, guid 0x%.8X 0x%.8X",
        itemid, guidlow, guidhigh );

    ItemPrototype *itemProto = objmgr.GetItemPrototype(itemid);
    if(!itemProto)
    {
        Log::getSingleton( ).outError( "WORLD: Unknown item id 0x%.8X", itemid );
        return;
    }

    data.Initialize( SMSG_ITEM_QUERY_SINGLE_RESPONSE );

    data << itemProto->ItemId;
    data << itemProto->Class;
    data << itemProto->SubClass;
    data << itemProto->Name1.c_str();
    data << uint8(0) << uint8(0) << uint8(0);     // name 2,3,4
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
    data << itemProto->Field20;
    data << itemProto->Field21;
    data << itemProto->Field22;
    data << itemProto->Field23;
    data << itemProto->MaxCount;
    data << itemProto->ContainerSlots;;
    for(i = 0; i < 10; i++)
    {
        data << itemProto->ItemStatType[i];
        data << itemProto->ItemStatValue[i];
    }
    for(i = 0; i < 5; i++)
    {
        data << itemProto->DamageMin[i];
        data << itemProto->DamageMax[i];
        data << itemProto->DamageType[i];
    }
    data << itemProto->Armor;
    data << itemProto->HolyRes;
    data << itemProto->FireRes;
    data << itemProto->NatureRes;
    data << itemProto->FrostRes;
    data << itemProto->ShadowRes;
    data << itemProto->ArcaneRes;
    data << itemProto->Delay;
    data << itemProto->Field69;
    for(i = 0; i < 5; i++)
    {
        data << itemProto->SpellId[i];
        data << itemProto->SpellTrigger[i];
        data << itemProto->SpellCharges[i];
        data << itemProto->SpellCooldown[i];
        data << itemProto->SpellCategory[i];
        data << itemProto->SpellCategoryCooldown[i];
    }
    data << itemProto->Bonding;
    data << itemProto->Description.c_str();
    data << itemProto->Field102;
    data << itemProto->Field103;
    data << itemProto->Field104;
    data << itemProto->Field105;
    data << itemProto->Field106;
    data << itemProto->Field107;
    data << itemProto->Field108;
    data << itemProto->Field109;
    data << itemProto->Field110;
    data << itemProto->Field111;
    data << itemProto->MaxDurability;

    WPAssert(data.size() == 433 + itemProto->Name1.length() + itemProto->Description.length());
    SendPacket( &data );
}


void WorldSession::HandleSellItemOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDetail( "WORLD: Recieved CMSG_SELL_ITEM" );

    WorldPacket data;
    uint64 vendorguid, itemguid;
    uint8 amount;
    uint32 newmoney;
    uint8 slot = 0xFF;
    int check = 0;

    recv_data >> vendorguid;
    recv_data >> itemguid;
    recv_data >> amount;

    // Check if item exists
    if (itemguid == 0)
    {
        data.Initialize( SMSG_SELL_ITEM );
        data << vendorguid << itemguid << uint8(0x01);
        WPAssert(data.size() == 17);
        SendPacket( &data );
        return;
    }

#ifndef ENABLE_GRID_SYSTEM
    Creature *unit = objmgr.GetObject<Creature>(vendorguid);
#else
    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, vendorguid);
#endif
    // Check if Vendor exists
    if (unit == NULL)
    {
        data.Initialize( SMSG_SELL_ITEM );
        data << vendorguid << itemguid << uint8(0x03);
        WPAssert(data.size() == 17);
        SendPacket( &data );
        return;
    }

    Item *item;
    // Search the slot...
    for(uint8 i=0; i<39; i++)
    {
        item = GetPlayer()->GetItemBySlot(i);
        if (item != NULL)
        {
            if (item->GetGUID() == itemguid)
            {
                slot = i;
                break;
            }
        }
    }

    if (slot == 0xFF)
    {
        data.Initialize( SMSG_SELL_ITEM );
        data << vendorguid << itemguid << uint8(0x01);
        WPAssert(data.size() == 17);
        SendPacket( &data );
        return;                                   //our player doesn't have this item
    }

    /*
    // adding this item to the vendor's item list... not nessesary for unlimited items
    for(i=0; i<tempunit->getItemCount();i++)
    {
        if (tempunit->getItemId(i) == GetPlayer()->getItemIdBySlot(itemindex))
        {
            tempunit->setItemAmount(i, tempunit->getItemAmount(i) + amount);
            check = 1;
        }
    }

    if (check == 0)
    {
        if (tempunit->getItemCount() > 100)
        {
            data.Initialize( SMSG_SELL_ITEM );
            data << srcguid1 << srcguid2 << itemguid1 << itemguid2 << uint8(0x02);
            WPAssert(data.size() == 17);
            SendPacket( &data );
            return;
        }
        else
            tempunit->addItem(GetPlayer()->getItemIdBySlot(itemindex), amount);
    }
    */

    if (amount == 0) amount = 1;                  // ?!?
    newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) + (item->GetProto()->SellPrice) * amount);
    GetPlayer()->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);

    //removing the item from the char's inventory
    GetPlayer()->RemoveItemFromSlot(slot);

    item->DeleteFromDB();

    delete item;

    data.Initialize( SMSG_SELL_ITEM );
    data << vendorguid << itemguid
        << uint8(0x05);                           // Error Codes: 0x01 = Item not Found
                                                  //              0x02 = Vendor doesnt want that item
                                                  //              0x03 = Vendor doesnt like you ;P
                                                  //              0x04 = You dont own that item
                                                  //              0x05 = Ok
                                                  //              0x06 = Only with empty Bags !?

    WPAssert(data.size() == 17);
    SendPacket( &data );

    Log::getSingleton( ).outDetail( "WORLD: Sent SMSG_SELL_ITEM" );
}


// drag & drop
void WorldSession::HandleBuyItemInSlotOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDetail( "WORLD: Recieved CMSG_BUY_ITEM_IN_SLOT" );

    WorldPacket data;
    uint64 srcguid, dstguid;
    uint32 itemid;
    uint8 slot, amount;
    int vendorslot = -1;
    int32 newmoney;

    recv_data >> srcguid >> itemid;
    recv_data >> dstguid;                         // ??
    recv_data >> slot;
    recv_data >> amount;

#ifndef ENABLE_GRID_SYSTEM
    Creature *unit = objmgr.GetObject<Creature>(srcguid);
#else
    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, srcguid);
#endif
    if (unit == NULL)
        return;

    if (slot > 38)
        return;
    if (slot < 19)
        return;
    if ((slot <= 22) && (slot >=19))
        return;                                   //these are the bags slots...i'm not sure exactly how to use them
    if (GetPlayer()->GetItemBySlot(slot) != 0)
        return;                                   //slot is not empty...

    // Find item slot
    for(uint8 i = 0; i < unit->getItemCount(); i++)
    {
        if (unit->getItemId(i) == itemid)
        {
            vendorslot = i;
            break;
        }
    }

    if( vendorslot == -1 )
        return;

    // Check for vendor have the required amount of that item ....
    if (amount > unit->getItemAmount(vendorslot) && unit->getItemAmount(vendorslot)>=0)
        return;

    // Check for gold
    newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) - (objmgr.GetItemPrototype(itemid)->BuyPrice));
    if(newmoney < 0 )
    {
        data.Initialize( SMSG_BUY_FAILED );
        data << uint64(srcguid);
        data << uint32(itemid);
        data << uint8(2);                         //not enough money
        SendPacket( &data );
        return;
    }
    GetPlayer()->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);

    Item *item = new Item();
    item->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), itemid, GetPlayer());

    unit->setItemAmount( vendorslot, unit->getItemAmount(vendorslot)-amount );
    GetPlayer()->AddItemToSlot( slot, item );

    data.Initialize( SMSG_BUY_ITEM );
    data << uint64(srcguid);
    data << uint32(itemid) << uint32(amount);
    WPAssert(data.size() == 16);
    SendPacket( &data );
    Log::getSingleton( ).outDetail( "WORLD: Sent SMSG_BUY_ITEM" );
}


// right-click on item
void WorldSession::HandleBuyItemOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDetail( "WORLD: Recieved CMSG_BUY_ITEM" );

    WorldPacket data;
    uint64 srcguid;
    uint32 itemid;
    uint8 amount, slot;
    uint8 playerslot = 0;
    int vendorslot = -1;
    int32 newmoney;

    recv_data >> srcguid >> itemid;
    recv_data >> amount >> slot;

#ifndef ENABLE_GRID_SYSTEM
    Creature *unit = objmgr.GetObject<Creature>(srcguid);
#else
    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, srcguid);
#endif
    if (unit == NULL)
        return;

    // Find free slot and break if inv full
    for(uint8 i = 23; i <= 38; i++)
    {
        if (GetPlayer()->GetItemBySlot(i) == 0)
        {
            playerslot = i;
            break;
        }
    }
    if (playerslot == 0)
    {
        data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
        data << uint8(48);                        // Inventory Full
        data << uint64(0);
        data << uint64(0);
        data << uint8(0);
        SendPacket( &data );
        return;
    }

    // Find item slot
    for(uint8 i = 0; i < unit->getItemCount(); i++)
    {
        if (unit->getItemId(i) == itemid)
        {
            vendorslot = i;
            break;
        }
    }

    if( vendorslot == -1 )
        return;

    // Check for vendor have the required amount of that item ....
    if (amount > unit->getItemAmount(vendorslot) && unit->getItemAmount(vendorslot)>=0)
        return;

    // Check for gold
    newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) - (objmgr.GetItemPrototype(itemid)->BuyPrice));
    if(newmoney < 0 )
    {
        data.Initialize( SMSG_BUY_FAILED );
        data << uint64(srcguid);
        data << uint32(itemid);
        data << uint8(2);                         //not enough money
        SendPacket( &data );
        return;
    }
    GetPlayer()->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);

    // unit->setItemAmount( vendorslot, unit->getItemAmount(vendorslot)-amount ); // Unlimited Items

    Item *item = new Item();
    item->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), itemid, GetPlayer());

    GetPlayer()->AddItemToSlot( playerslot, item );

    data.Initialize( SMSG_BUY_ITEM );
    data << uint64(srcguid);
    data << uint32(itemid) << uint32(amount);
    WPAssert(data.size() == 16);
    SendPacket( &data );
    Log::getSingleton( ).outDetail( "WORLD: Sent SMSG_BUY_ITEM" );
}


void WorldSession::HandleListInventoryOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDetail( "WORLD: Recvd CMSG_LIST_INVENTORY" );

    WorldPacket data;
    uint64 guid;

    recv_data >> guid;
    Log::getSingleton( ).outDetail( "WORLD: Recvd CMSG_LIST_INVENTORY %u", guid );
#ifndef ENABLE_GRID_SYSTEM
    Creature *unit = objmgr.GetObject<Creature>(guid);
#else
    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);
#endif
    if (unit == NULL)
        return;

    uint8 numitems = (uint8)unit->getItemCount();
    uint8 actualnumitems = 0;
    uint8 i = 0;

    // get actual Item Count better then alot of spaces :D
    for(i = 0; i < numitems; i ++ )
    {
        if(unit->getItemId(i) != 0) actualnumitems++;
    }
    uint32 guidlow = GUID_LOPART(guid);

    data.Initialize( SMSG_LIST_INVENTORY );
    data << guid;
    data << uint8( actualnumitems );              // num items

    // each item has seven uint32's

    ItemPrototype * curItem;
    for(i = 0; i < numitems; i++ )
    {
        if(unit->getItemId(i) != 0)
        {
            curItem = objmgr.GetItemPrototype(unit->getItemId(i));
            if( !curItem )
            {
                Log::getSingleton( ).outError( "Unit %i has nonexistant item %i! the item will be removed next time", guid, unit->getItemId(i) );
                for( int a = 0; a < 7; a ++ )
                    data << uint32( 0 );

                std::stringstream ss;
                ss << "DELETE * FROM vendors WHERE vendorGuid=" << guidlow << " itemGuid=" << unit->getItemId(i) << '\0';
                QueryResult *result = sDatabase.Query( ss.str().c_str() );

                unit->setItemAmount(i,0);
                unit->setItemId(i,0);
            }
            else
            {
                data << uint32( i + 1 );          // index ? doesn't seem to affect anything
                // item id
                data << uint32( unit->getItemId(i) );
                // item icon
                data << uint32( curItem->DisplayInfoID );
                // number of items available, -1 works for infinity, although maybe just 'cause it's really big
                data << uint32( unit->getItemAmount(i) );
                // price
                data << uint32( curItem->BuyPrice );
                data << uint32( 0 );              // ?
                data << uint32( 0 );              // ?
            }
        }
    }

    if (!(data.size() == 8 + 1 + ((actualnumitems * 7) * 4)))
        return; // Let's just skip it if we can't use the vendor.. (Default system)

    WPAssert(data.size() == 8 + 1 + ((actualnumitems * 7) * 4));
    SendPacket( &data );
    Log::getSingleton( ).outDetail( "WORLD: Sent SMSG_LIST_INVENTORY" );
}

/*
void WorldSession::HandleAutoStoreBagItemOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDetail( "WORLD: Recvd CMSG_AUTO_STORE_BAG_ITEM" );

    WorldPacket data;
    uint64 guid;

    recv_data >> guid;
    Log::getSingleton( ).outDetail( "WORLD: Recvd CMSG_LIST_INVENTORY %u", guid );
#ifndef ENABLE_GRID_SYSTEM
    Creature *unit = objmgr.GetObject<Creature>(guid);
#else
    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);
#endif
    if (unit == NULL)
        return;

    uint8 numitems = (uint8)unit->getItemCount();
    uint8 actualnumitems = 0;
    uint8 i = 0;

    // get actual Item Count better then alot of spaces :D
    for(i = 0; i < numitems; i ++ )
    {
        if(unit->getItemId(i) != 0) actualnumitems++;
    }
    uint32 guidlow = GUID_LOPART(guid);

    data.Initialize( SMSG_AUTO_STORE_BAG_ITEM );
    data << guid;
    data << uint8( actualnumitems );              // num items

    // each item has seven uint32's

    ItemPrototype * curItem;
    for(i = 0; i < numitems; i++ )
    {
        if(unit->getItemId(i) != 0)
        {
            curItem = objmgr.GetItemPrototype(unit->getItemId(i));
            if( !curItem )
            {
                Log::getSingleton( ).outError( "Unit %i has nonexistant item %i! the item will be removed next time", guid, unit->getItemId(i) );
                for( int a = 0; a < 7; a ++ )
                    data << uint32( 0 );

                std::stringstream ss;
                ss << "DELETE * FROM vendors WHERE vendorGuid=" << guidlow << " itemGuid=" << unit->getItemId(i) << '\0';
                QueryResult *result = sDatabase.Query( ss.str().c_str() );

                unit->setItemAmount(i,0);
                unit->setItemId(i,0);
            }
            else
            {
                data << uint32( i + 1 );          // index ? doesn't seem to affect anything
                // item id
                data << uint32( unit->getItemId(i) );
                // item icon
                data << uint32( curItem->DisplayInfoID );
                // number of items available, -1 works for infinity, although maybe just 'cause it's really big
                data << uint32( unit->getItemAmount(i) );
                // price
                data << uint32( curItem->BuyPrice );
                data << uint32( 0 );              // ?
                data << uint32( 0 );              // ?
            }
        }
    }

    if (!(data.size() == 8 + 1 + ((actualnumitems * 7) * 4)))
        return; // Let's just skip it if we can't use the vendor.. (Default system)

    WPAssert(data.size() == 8 + 1 + ((actualnumitems * 7) * 4));
    SendPacket( &data );
    Log::getSingleton( ).outDetail( "WORLD: Sent SMSG_AUTO_STORE_BAG_ITEM" );
}
*/
