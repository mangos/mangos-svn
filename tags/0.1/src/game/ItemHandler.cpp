/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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

void WorldSession::HandleSwapInvItemOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    UpdateData upd;
    uint8 srcslot, dstslot;
    

    recv_data >> srcslot >> dstslot;

    Log::getSingleton().outDetail("ITEM: swap, src slot: %u dst slot: %u", (uint32)srcslot, (uint32)dstslot);

    
    
    

    
    
    

    Item * dstitem = GetPlayer()->GetItemBySlot(dstslot);
    Item * srcitem = GetPlayer()->GetItemBySlot(srcslot);

	if (dstitem != NULL)
	{
		if (srcitem->GetProto()->DisplayInfoID == dstitem->GetProto()->DisplayInfoID 
			&& dstitem->GetProto()->MaxCount > 0 && dstitem->GetCount() < dstitem->GetProto()->MaxCount )
		{
			uint32 free_slots = dstitem->GetProto()->MaxCount - dstitem->GetCount();
			uint32 leftover_count = free_slots - srcitem->GetCount();

			if (leftover_count <= 0)
			{
				dstitem->SetCount(dstitem->GetCount()+srcitem->GetCount());
				GetPlayer()->RemoveItemFromSlot(srcslot);
				GetPlayer()->UpdateSlot(srcslot);
				GetPlayer()->UpdateSlot(dstslot);
				dstitem->UpdateStats();
				srcitem->UpdateStats();
				return;
			}
			else
			{
				dstitem->SetCount(dstitem->GetProto()->MaxCount);
				srcitem->SetCount(leftover_count);
				GetPlayer()->UpdateSlot(srcslot);
				GetPlayer()->UpdateSlot(dstslot);
				dstitem->UpdateStats();
				srcitem->UpdateStats();
				return;
			}
		}
	}

	dstitem->UpdateStats();
	srcitem->UpdateStats();

    
    if ( (srcslot >= INVENTORY_SLOT_BAG_START && srcslot < BANK_SLOT_BAG_END)&&
         (dstslot >= EQUIPMENT_SLOT_START && dstslot < EQUIPMENT_SLOT_END)
       )
    {
		bool error = GetPlayer()->CanEquipItem(srcitem->GetProto());
		if (error) 
		{
			GetPlayer()->UpdateSlot(srcslot);
            GetPlayer()->UpdateSlot(dstslot);
			return;
		}
        
    }
    
    if(srcslot >= EQUIPMENT_SLOT_START && srcslot < EQUIPMENT_SLOT_END )
    {
        if(dstitem)
        {
			bool error = GetPlayer()->CanEquipItem(srcitem->GetProto());
			{
				GetPlayer()->UpdateSlot(srcslot);
				GetPlayer()->UpdateSlot(dstslot);
				return;
			}

            
        }
    }

    
    if (srcslot == dstslot)
    {
        data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
        data << uint8(0x16); 
        data << (dstitem ? dstitem->GetGUID() : uint64(0));
        data << (srcitem ? srcitem->GetGUID() : uint64(0));
        data << uint8(0);

        SendPacket( &data );
        return;
    }

    
    GetPlayer()->SwapItemSlots(srcslot, dstslot);



}

void WorldSession::HandleDestroyItemOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    uint8 srcslot, dstslot;

    recv_data >> srcslot >> dstslot;

    Log::getSingleton().outDetail("ITEM: destroy, src slot: %u dst slot: %u", (uint32)srcslot, (uint32)dstslot);

    Item *item = GetPlayer()->GetItemBySlot(dstslot);

	item->UpdateStats();

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

    
    if(!item)
    {
        Log::getSingleton().outDetail("ITEM: tried to equip non-existant item");

        data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
        data << uint8(0x16); 
        data << uint64(0);
        data << uint64(0);
        data << uint8(0);

        SendPacket( &data );

        return;
    }

	item->UpdateStats();

    
    uint32 charLvl = GetPlayer()->getLevel();
    uint32 itemLvl = item->GetProto()->RequiredLevel;

    Log::getSingleton( ).outDetail("ITEM: CharLvl %d, ItemLvl %d", charLvl, itemLvl);

    if( charLvl < itemLvl)
    {
        data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
        data << (uint8)0x01; 
        data << (uint64)item->GetProto()->RequiredLevel;
        data << (uint64)0;
        data << (uint8)0;
        
        SendPacket( &data );
        GetPlayer()->UpdateSlot(dstslot);
        return;
    }

    
    uint8 slot = GetPlayer()->FindFreeItemSlot(item->GetProto()->InventoryType);
    
    if (slot == INVENTORY_SLOT_ITEM_END)
    {
        data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
        data << uint8(0x03); 
        data << item->GetGUID();
        data << uint64(0);
        data << uint8(0);
        WPAssert(data.size() == 18);
        SendPacket( &data );
        GetPlayer()->UpdateSlot(dstslot);
        return;
    }

    
    

	bool error = GetPlayer()->CanEquipItem(item->GetProto());
	if (error)
	{
        GetPlayer()->UpdateSlot(dstslot);
		return;
	}

    GetPlayer()->SwapItemSlots(dstslot, slot);
}

extern void CheckItemDamageValues ( ItemPrototype *itemProto );

void WorldSession::HandleItemQuerySingleOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    int i;
    uint32 itemid, guidlow, guidhigh;
    recv_data >> itemid >> guidlow >> guidhigh;   

    ItemPrototype *itemProto = objmgr.GetItemPrototype(itemid);
    if(!itemProto)
    {
        
		Log::getSingleton( ).outError( "WORLD: Unknown item id %u", itemid );
		
		data.Initialize( SMSG_ITEM_QUERY_SINGLE_RESPONSE );
		data << itemid;
		data << uint32(0);
		data << uint32(0);
		data << uint32();
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		data << uint32(0);
		SendPacket( &data );
        return;
    }




	Log::getSingleton( ).outDetail( "WORLD: Recvd CMSG_ITEM_QUERY_SINGLE for item id %u, guidlow %u, guidhigh %u",
        itemid, guidlow, guidhigh );

    data.Initialize( SMSG_ITEM_QUERY_SINGLE_RESPONSE );

    data << itemProto->ItemId;
    data << itemProto->Class;
    data << itemProto->SubClass;
    data << itemProto->Name1.c_str();
    

    if (stricmp(itemProto->Name2.c_str(), ""))
    {
        data << itemProto->Name2.c_str();
    }
    else
    {
        
        data << uint8(0);
    }
    

    if (stricmp(itemProto->Name3.c_str(), ""))
    {
        data << itemProto->Name3.c_str();
    }
    else
    {
        
        data << uint8(0);
    }
    

    if (stricmp(itemProto->Name4.c_str(), ""))
    {
        data << itemProto->Name4.c_str();
    }
    else
    {
        
        data << uint8(0);
    }
    

    

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
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);

	if (itemProto->MaxCount > 1)
		data << itemProto->MaxCount; 
	else
		data << uint32(0);

	data << itemProto->MaxCount; 
	data << itemProto->ContainerSlots; 

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


														

		data << uint32(0);

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

	if (stricmp(itemProto->Description.c_str(), ""))
    {
        data << itemProto->Description.c_str();
		
    }
    else
    {
		if (itemProto->Quality == ITEM_QUALITY_NORMAL)
			data << std::string("This is a normal item.");
		else if (itemProto->Quality == ITEM_QUALITY_UNCOMMON)
			data << std::string("This is an uncommon item.");
		else if (itemProto->Quality == ITEM_QUALITY_RARE)
			data << std::string("This is a rare item.");
		else if (itemProto->Quality == ITEM_QUALITY_EPIC)
			data << std::string("This is an epic item.");
		else if (itemProto->Quality == ITEM_QUALITY_LEGENDARY)
			data << std::string("This is a legendary item.");

        
        
    }
    data << itemProto->Field102; 
    data << itemProto->Field103; 
    data << itemProto->Field104; 
    data << itemProto->Field105; 
    
	
	if (itemProto->Class == ITEM_CLASS_PERMANENT)
		data << uint32(1);       
	else
		data << uint32(0);

    data << itemProto->Field106; 
	data << itemProto->Sheath;
    data << itemProto->Field108; 
	data << itemProto->Block;    
    data << itemProto->Field110; 
    data << itemProto->MaxDurability;

	data << uint32(0); 

    
    SendPacket( &data );
}

extern char *fmtstring( char *format, ... );


extern char *GetInventoryImageFilefromObjectClass(uint32 classNum, uint32 subclassNum, uint32 type, uint32 DisplayID);

void WorldSession::SendItemPageInfo( uint32 realID, uint32 itemid )
{
    int i;
	Player* pl = GetPlayer();
	char *itemInfo;
	bool resist_added = false;
	bool names_added = false;
	WorldPacket data;

	if (realID < 0)
	{
        
        return;
    }

    ItemPrototype *itemProto = objmgr.GetItemPrototype(realID);
    
	if(!itemProto)
    {
        
        return;
    }

	Log::getSingleton( ).outDebug( "WORLD: Real item id is %u. Name %s.", realID, itemProto->Name1.c_str() );

	data.Initialize(SMSG_ITEM_TEXT_QUERY_RESPONSE);
    data << itemid;

	itemInfo = (fmtstring("<HTML>\n<BODY>\n"));

	itemInfo = (fmtstring("%s<H1 align=\"left\">Name: %s</H1><BR/>", itemInfo, itemProto->Name1.c_str()));

	itemInfo = (fmtstring("%s<IMG src=\"Interface\\Icons\\%s\" align=\"right\"/><P align=\"left\">", itemInfo, GetInventoryImageFilefromObjectClass(itemProto->Class, itemProto->SubClass, itemProto->InventoryType, itemProto->DisplayInfoID)));
	itemInfo = (fmtstring("%s<BR/>", itemInfo));

    if (stricmp(itemProto->Name2.c_str(), "") && stricmp(itemProto->Name2.c_str(), " ") && stricmp(itemProto->Name2.c_str(), itemProto->Name1.c_str()))
    {
        itemInfo = (fmtstring("%s%s<BR/>", itemInfo, itemProto->Name2.c_str()));
		names_added = true;
    }

    if (stricmp(itemProto->Name3.c_str(), "") && stricmp(itemProto->Name2.c_str(), " ") && stricmp(itemProto->Name3.c_str(), itemProto->Name2.c_str()))
    {
        itemInfo = (fmtstring("%s%s<BR/>", itemInfo, itemProto->Name3.c_str()));
		names_added = true;
    }

    if (stricmp(itemProto->Name4.c_str(), "") && stricmp(itemProto->Name2.c_str(), " ") && stricmp(itemProto->Name4.c_str(), itemProto->Name3.c_str()))
    {
        itemInfo = (fmtstring("%s%s<BR/>", itemInfo, itemProto->Name4.c_str()));
		names_added = true;
    }

	if (names_added)
		itemInfo = (fmtstring("%s<BR/>", itemInfo)); 

    if (stricmp(itemProto->Description.c_str(), ""))
    {
		itemInfo = (fmtstring("%sDescription: %s<BR/>", itemInfo, itemProto->Description.c_str()));
		itemInfo = (fmtstring("%s<BR/>", itemInfo)); 
    }

	
	if (itemProto->Quality == ITEM_QUALITY_NORMAL)
		itemInfo = (fmtstring("%sThis is a normal item.<BR/><BR/>", itemInfo));
	else if (itemProto->Quality == ITEM_QUALITY_UNCOMMON)
		itemInfo = (fmtstring("%sThis is an uncommon item.<BR/><BR/>", itemInfo));
	else if (itemProto->Quality == ITEM_QUALITY_RARE)
		itemInfo = (fmtstring("%sThis is a rare item.<BR/><BR/>", itemInfo));
	else if (itemProto->Quality == ITEM_QUALITY_EPIC)
		itemInfo = (fmtstring("%sThis is an epic item.<BR/><BR/>", itemInfo));
	else if (itemProto->Quality == ITEM_QUALITY_LEGENDARY)
		itemInfo = (fmtstring("%sThis is a legendary item.<BR/><BR/>", itemInfo));

	if (itemProto->Bonding)
		itemInfo = (fmtstring("%sThis is a bonding item.<BR/>", itemInfo));

	itemInfo = (fmtstring("%sMaximum Durability: %u.<BR/>", itemInfo, itemProto->MaxDurability));


	for(i = 0; i < 5; i++)
	{
		if ( (itemProto->DamageMax[i] <= 0 || itemProto->DamageMax[i] <= 0)
			|| (itemProto->DamageMax[i] > 99999 || itemProto->DamageMax[i] > 99999) )
			continue; 
		
		switch (itemProto->DamageType[i])
		{
		case NORMAL_DAMAGE:
			itemInfo = (fmtstring("%sDoes %u to %u of normal damage.<BR/>", itemInfo, (uint32)itemProto->DamageMin[i], (uint32)itemProto->DamageMax[i]));
			break;
		case HOLY_DAMAGE:
			itemInfo = (fmtstring("%sadds %u to %u Holy damage.<BR/>", itemInfo, (uint32)itemProto->DamageMin[i], (uint32)itemProto->DamageMax[i]));
			break;
		case FIRE_DAMAGE:
			itemInfo = (fmtstring("%sadds %u to %u Fire damage.<BR/>", itemInfo, (uint32)itemProto->DamageMin[i], (uint32)itemProto->DamageMax[i]));
			break;
		case NATURE_DAMAGE:
			itemInfo = (fmtstring("%sadds %u to %u Nature damage.<BR/>", itemInfo, (uint32)itemProto->DamageMin[i], (uint32)itemProto->DamageMax[i]));
			break;
		case FROST_DAMAGE:
			itemInfo = (fmtstring("%sadds %u to %u Frost damage.<BR/>", itemInfo, (uint32)itemProto->DamageMin[i], (uint32)itemProto->DamageMax[i]));
			break;
		case SHADOW_DAMAGE:
			itemInfo = (fmtstring("%sadds %u to %u Shadow damage.<BR/>", itemInfo, (uint32)itemProto->DamageMin[i], (uint32)itemProto->DamageMax[i]));
			break;
		case ARCANE_DAMAGE:
			itemInfo = (fmtstring("%sadds %u to %u Arcane damage.<BR/>", itemInfo, (uint32)itemProto->DamageMin[i], (uint32)itemProto->DamageMax[i]));
			break;
		default: 
			break;
		}
	}

    itemInfo = (fmtstring("%sSell Price: %u.<BR/>", itemInfo, itemProto->SellPrice));
	itemInfo = (fmtstring("%s<BR/>", itemInfo)); 

    itemInfo = (fmtstring("%sLevel: %u.<BR/>", itemInfo, itemProto->ItemLevel));
	itemInfo = (fmtstring("%sRequired Character Level: %u.<BR/>", itemInfo, itemProto->RequiredLevel));
	itemInfo = (fmtstring("%s<BR/>", itemInfo)); 

    if (itemProto->ContainerSlots)
	{
		itemInfo = (fmtstring("%sThis item is a container, and will hold %u items.<BR/>", itemInfo, itemProto->ContainerSlots));
		itemInfo = (fmtstring("%s<BR/>", itemInfo)); 
	}

	
    
	if (itemProto->Armor > 0)
	{
		itemInfo = (fmtstring("%sArmor Bonus: %u.<BR/>", itemInfo, itemProto->Armor));
		itemInfo = (fmtstring("%s<BR/>", itemInfo)); 
	}

	if (itemProto->HolyRes > 0)
	{
		itemInfo = (fmtstring("%sHoly Resistance Bonus: %u.<BR/>", itemInfo, itemProto->HolyRes));
		resist_added = true;
	}

	if (itemProto->FireRes > 0)
	{
		itemInfo = (fmtstring("%sFire Resistance Bonus: %u.<BR/>", itemInfo, itemProto->FireRes));
		resist_added = true;
	}

	if (itemProto->NatureRes > 0)
	{
		itemInfo = (fmtstring("%sNature Resistance Bonus: %u.<BR/>", itemInfo, itemProto->NatureRes));
		resist_added = true;
	}

	if (itemProto->FrostRes > 0)
	{
		itemInfo = (fmtstring("%sFrost Resistance Bonus: %u.<BR/>", itemInfo, itemProto->FrostRes));
		resist_added = true;
	}

	if (itemProto->ShadowRes > 0)
	{
		itemInfo = (fmtstring("%sShadow Resistance Bonus: %u.<BR/>", itemInfo, itemProto->ShadowRes));
		resist_added = true;
	}

	if (itemProto->ArcaneRes > 0)
	{
		itemInfo = (fmtstring("%sArcane Resistance Bonus: %u.<BR/>", itemInfo, itemProto->ArcaneRes));
		resist_added = true;
	}

	if (resist_added)
		itemInfo = (fmtstring("%s<BR/>", itemInfo)); 
    
	itemInfo = (fmtstring("%sAttack Delay: %u.<BR/><BR/>", itemInfo, itemProto->Delay));

	itemInfo = (fmtstring("%s</P></BODY>\n</HTML>\n", itemInfo));

	data << itemInfo;
	data << uint32(0);
    SendPacket(&data);  

	
}

void WorldSession::SendAllItemPageInfos( void )
{
	uint8 i = 0;
	Item * srcitem;
	Player* pl = GetPlayer();

	for (i = EQUIPMENT_SLOT_START; i < BANK_SLOT_BAG_END; i++)
	{
		srcitem = pl->GetItemBySlot(i);

		if (srcitem)
		{
			SendItemPageInfo(srcitem->GetItemProto()->ItemId, srcitem->GetItemProto()->DisplayInfoID);
		}
	}
}

void WorldSession::HandlePageQuerySkippedOpcode( WorldPacket & recv_data )
{
	Log::getSingleton( ).outDetail( "WORLD: Recieved CMSG_PAGE_TEXT_QUERY" );

    WorldPacket data;
	uint32 itemid, guidlow, guidhigh;

	recv_data >> itemid >> guidlow >> guidhigh;

	Log::getSingleton( ).outDetail( "Packet Info: itemid: %u guidlow: %u guidhigh: %u", itemid, guidlow, guidhigh );
	
	
	

	
	if (itemid >= 415 && itemid <= 416) 
		SendAllItemPageInfos();
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

    Item *item;
    
    for(uint8 i=0; i<39; i++)
    {
        item = GetPlayer()->GetItemBySlot(i);
        if (item != NULL)
        {
			item->UpdateStats();

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
        return;                                   
    }

    

    if (amount == 0) amount = 1;                  
    newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) + (item->GetProto()->SellPrice) * amount);
    GetPlayer()->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);

    
    GetPlayer()->RemoveItemFromSlot(slot);

    item->DeleteFromDB();

    delete item;

    data.Initialize( SMSG_SELL_ITEM );
    data << vendorguid << itemguid
        << uint8(0x05);                           
                                                  
                                                  
                                                  
                                                  
                                                  

    WPAssert(data.size() == 17);
    SendPacket( &data );

    Log::getSingleton( ).outDetail( "WORLD: Sent SMSG_SELL_ITEM" );
}



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
    recv_data >> dstguid;                         
    recv_data >> slot;
    recv_data >> amount;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, srcguid);

    if (unit == NULL)
        return;

    if (slot > 38)
        return;
    if (slot < 19)
        return;
    if ((slot <= 22) && (slot >=19))
        return;                                   

	
	for(uint8 i = 23; i <= 38; i++)
    {
		Item *item = new Item();
		item->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), itemid, GetPlayer());
		
		if (GetPlayer()->GetItemBySlot(i) != 0)
        {
            if (GetPlayer()->GetItemBySlot(i)->GetProto()->DisplayInfoID == item->GetProto()->DisplayInfoID
				&& GetPlayer()->GetItemBySlot(i)->GetProto()->MaxCount > 0 && GetPlayer()->GetItemBySlot(i)->GetCount() < GetPlayer()->GetItemBySlot(i)->GetProto()->MaxCount )
			{
				if (GetPlayer()->GetItemBySlot(i)->GetCount()+amount <= GetPlayer()->GetItemBySlot(i)->GetProto()->MaxCount)
				{
					GetPlayer()->GetItemBySlot(i)->SetCount(GetPlayer()->GetItemBySlot(i)->GetCount()+amount);
					delete item;
					return;
				}
				else 
				{
					uint32 free_slot_count = GetPlayer()->GetItemBySlot(i)->GetProto()->MaxCount - GetPlayer()->GetItemBySlot(i)->GetCount();
					uint32 max_add = amount - free_slot_count;

					GetPlayer()->GetItemBySlot(i)->SetCount(GetPlayer()->GetItemBySlot(i)->GetProto()->MaxCount);
					amount = (amount - max_add); 
					
				}
			}
        }

		delete item;
    }

    if (GetPlayer()->GetItemBySlot(slot) != 0)
        return;                                   

    
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

    
    if (amount > unit->getItemAmount(vendorslot) && unit->getItemAmount(vendorslot)>=0)
        return;

    
    newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) - (objmgr.GetItemPrototype(itemid)->BuyPrice));
    if(newmoney < 0 )
    {
        data.Initialize( SMSG_BUY_FAILED );
        data << uint64(srcguid);
        data << uint32(itemid);
        data << uint8(2);                         
        SendPacket( &data );
        return;
    }
    GetPlayer()->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);

    Item *item = new Item();
    item->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), itemid, GetPlayer());

	
	

	item->UpdateStats();

    unit->setItemAmount( vendorslot, unit->getItemAmount(vendorslot)-amount );
    GetPlayer()->AddItemToSlot( slot, item );

    data.Initialize( SMSG_BUY_ITEM );
    data << uint64(srcguid);
    data << uint32(itemid) << uint32(amount);
    WPAssert(data.size() == 16);
    SendPacket( &data );
    Log::getSingleton( ).outDetail( "WORLD: Sent SMSG_BUY_ITEM" );

	item->UpdateStats();
}



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

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, srcguid);

    if (unit == NULL)
        return;

	
	for(uint8 i = 23; i <= 38; i++)
    {
		Item *item = new Item();
		item->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), itemid, GetPlayer());
		
		if (GetPlayer()->GetItemBySlot(i) != 0)
        {
            if (GetPlayer()->GetItemBySlot(i)->GetProto()->DisplayInfoID == item->GetProto()->DisplayInfoID
				&& GetPlayer()->GetItemBySlot(i)->GetProto()->MaxCount > 0 && GetPlayer()->GetItemBySlot(i)->GetCount() < GetPlayer()->GetItemBySlot(i)->GetProto()->MaxCount )
			{
				if (GetPlayer()->GetItemBySlot(i)->GetCount()+amount <= GetPlayer()->GetItemBySlot(i)->GetProto()->MaxCount)
				{
					GetPlayer()->GetItemBySlot(i)->SetCount(GetPlayer()->GetItemBySlot(i)->GetCount()+amount);
					delete item;
					return;
				}
				else 
				{
					uint32 free_slot_count = GetPlayer()->GetItemBySlot(i)->GetProto()->MaxCount - GetPlayer()->GetItemBySlot(i)->GetCount();
					uint32 max_add = amount - free_slot_count;

					GetPlayer()->GetItemBySlot(i)->SetCount(GetPlayer()->GetItemBySlot(i)->GetProto()->MaxCount);
					amount = (amount - max_add); 
					
				}
			}
        }

		delete item;
    }

    
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
        data << uint8(48);                        
        data << uint64(0);
        data << uint64(0);
        data << uint8(0);
        SendPacket( &data );
        return;
    }

    
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

    
    if (amount > unit->getItemAmount(vendorslot) && unit->getItemAmount(vendorslot)>=0)
        return;

    
    newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) - (objmgr.GetItemPrototype(itemid)->BuyPrice));
    if(newmoney < 0 )
    {
        data.Initialize( SMSG_BUY_FAILED );
        data << uint64(srcguid);
        data << uint32(itemid);
        data << uint8(2);                         
        SendPacket( &data );
        return;
    }
    GetPlayer()->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);

    

    Item *item = new Item();
    item->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), itemid, GetPlayer());

	
	

	item->UpdateStats();

    GetPlayer()->AddItemToSlot( playerslot, item );

    data.Initialize( SMSG_BUY_ITEM );
    data << uint64(srcguid);
    data << uint32(itemid) << uint32(amount);
    WPAssert(data.size() == 16);
    SendPacket( &data );
    Log::getSingleton( ).outDetail( "WORLD: Sent SMSG_BUY_ITEM" );

	item->UpdateStats();
}


void WorldSession::HandleListInventoryOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDetail( "WORLD: Recvd CMSG_LIST_INVENTORY" );

    WorldPacket data;
    uint64 guid;

    recv_data >> guid;
    Log::getSingleton( ).outDetail( "WORLD: Recvd CMSG_LIST_INVENTORY %u", guid );
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
    Log::getSingleton( ).outDetail( "WORLD: Sent SMSG_LIST_INVENTORY" );
}

void WorldSession::SendListInventory( uint64 guid )
{
    Log::getSingleton( ).outDetail( "WORLD: Recvd CMSG_LIST_INVENTORY" );

    WorldPacket data;

    Log::getSingleton( ).outDetail( "WORLD: Recvd CMSG_LIST_INVENTORY %u", guid );
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
    Log::getSingleton( ).outDetail( "WORLD: Sent SMSG_LIST_INVENTORY" );
}

void WorldSession::HandleAutoStoreBagItemOpcode( WorldPacket & recv_data )
{




	Log::getSingleton( ).outDebug( "WORLD: CMSG_AUTOSTORE_BAG_ITEM");
	
	WorldPacket data;

    uint8 unk1, srcslot, dstslot;

    recv_data >> unk1 >> srcslot >> dstslot;

	Log::getSingleton().outDetail("INVENTORY: AUTOSTORE ITEM: SLOTS -> SRC: %d DST: %d UNK: %d", srcslot, dstslot, unk1 );
}

