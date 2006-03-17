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
#include "Item.h"
#include "ObjectMgr.h"
#include "Database/DatabaseEnv.h"

Item::Item( )
{
    m_objectType |= TYPE_ITEM;
    m_objectTypeId = TYPEID_ITEM;

    m_valuesCount = ITEM_END;
}

uint32 GetRandPropertiesSeedfromDisplayInfoDBC(uint32 DisplayID)
{
	ItemDisplayTemplateEntry *itemDisplayTemplateEntry = sItemDisplayTemplateStore.LookupEntry( DisplayID );

	if (itemDisplayTemplateEntry)
		return itemDisplayTemplateEntry->seed;
	else
		return 0;
}

uint32 GetRandPropertiesIDfromDisplayInfoDBC(uint32 DisplayID)
{
	ItemDisplayTemplateEntry *itemDisplayTemplateEntry = sItemDisplayTemplateStore.LookupEntry( DisplayID );

	if (itemDisplayTemplateEntry)
		return itemDisplayTemplateEntry->randomPropertyID;
	else
		return 0;
}

extern char *fmtstring( char *format, ... );

char *GetImageFilefromDisplayInfoDBC(uint32 DisplayID)
{
	ItemDisplayTemplateEntry *itemDisplayTemplateEntry = sItemDisplayTemplateStore.LookupEntry( DisplayID );

	if (itemDisplayTemplateEntry)
		return fmtstring("%s", itemDisplayTemplateEntry->imageFile);
	else
		return fmtstring("");
}

char *GetInventoryImageFilefromDisplayInfoDBC(uint32 DisplayID)
{
	ItemDisplayTemplateEntry *itemDisplayTemplateEntry = sItemDisplayTemplateStore.LookupEntry( DisplayID );

	if (itemDisplayTemplateEntry && itemDisplayTemplateEntry->invImageFile)
	{
		Log::getSingleton( ).outDebug( "Image file is [%s].", itemDisplayTemplateEntry->invImageFile );
		return fmtstring("%s", itemDisplayTemplateEntry->invImageFile);
	}
	else
	{
		
		
		
		

		return fmtstring("");
	}
}


#define ITEM_SUBCLASS_FOOD					1 
#define ITEM_SUBCLASS_LIQUID				2 

char *GetInventoryImageFilefromObjectClass(uint32 classNum, uint32 subclassNum, uint32 type, uint32 DisplayID)
{
	

	char *itemIcon = fmtstring("INV_Misc_QuestionMark");

	switch (classNum)
	{
		case ITEM_CLASS_CONSUMABLE:
			switch (subclassNum)
			{
				case ITEM_SUBCLASS_FOOD:
					itemIcon = fmtstring("INV_Misc_Food_01");
					break;
				case ITEM_SUBCLASS_LIQUID:
					itemIcon = fmtstring("INV_Potion_01");
					break;
				default:
					
					
					itemIcon = fmtstring("INV_Scroll_02");
					
					break;
			}
			break;
		case ITEM_CLASS_CONTAINER:
			itemIcon = fmtstring("INV_Box_02");
			
			break;
		case ITEM_CLASS_WEAPON:
			switch (subclassNum)
			{
				case ITEM_SUBCLASS_WEAPON_AXE:
				case ITEM_SUBCLASS_WEAPON_AXE2:
					itemIcon = fmtstring("INV_Axe_04");
					break;
				case ITEM_SUBCLASS_WEAPON_BOW:
					itemIcon = fmtstring("INV_Weapon_Bow_05");
					break;
				case ITEM_SUBCLASS_WEAPON_GUN:
					itemIcon = fmtstring("INV_Weapon_Rifle_04");
					break;
				case ITEM_SUBCLASS_WEAPON_MACE:
				case ITEM_SUBCLASS_WEAPON_MACE2:
					itemIcon = fmtstring("INV_Mace_01");
					break;
				case ITEM_SUBCLASS_WEAPON_POLEARM:
					itemIcon = fmtstring("INV_Spear_04");
					break;
				case ITEM_SUBCLASS_WEAPON_SWORD2:
					itemIcon = fmtstring("INV_Sword_04");
					break;
				case ITEM_SUBCLASS_WEAPON_STAFF:
					itemIcon = fmtstring("INV_Staff_08");
					break;
				case ITEM_SUBCLASS_WEAPON_EXOTIC:
				case ITEM_SUBCLASS_WEAPON_EXOTIC2:
					itemIcon = fmtstring("INV_Weapon_ShortBlade_02");
					break;
				case ITEM_SUBCLASS_WEAPON_UNARMED:
					itemIcon = fmtstring("INV_Gauntlets_04");
					break;
				case ITEM_SUBCLASS_WEAPON_DAGGER:
					itemIcon = fmtstring("INV_Weapon_ShortBlade_05");
					break;
				case ITEM_SUBCLASS_WEAPON_THROWN:
					itemIcon = fmtstring("INV_Misc_QuestionMark");
					break;
				case ITEM_SUBCLASS_WEAPON_SPEAR:
					itemIcon = fmtstring("INV_Spear_05");
					break;
				case ITEM_SUBCLASS_WEAPON_CROSSBOW:
					itemIcon = fmtstring("INV_Weapon_Crossbow_01");
					break;
				case ITEM_SUBCLASS_WEAPON_WAND:
					itemIcon = fmtstring("INV_Wand_07");
					break;
				case ITEM_SUBCLASS_WEAPON_FISHING_POLE:
					itemIcon = fmtstring("INV_Fishingpole_02");
					break;
				case ITEM_SUBCLASS_WEAPON_GENERIC:
				default:
					itemIcon = fmtstring("INV_Misc_QuestionMark");
					
					
					break;
			}
			break;
		case ITEM_CLASS_JEWELRY:
			
			itemIcon = fmtstring("INV_Misc_Gem_Variety_01");
			break;
		case ITEM_CLASS_ARMOR:
			switch (type)
			{
				case INVTYPE_HEAD:
					itemIcon = fmtstring("INV_Helmet_04");
					break;
				case INVTYPE_NECK:
					itemIcon = fmtstring("INV_Jewelry_Amulet_05");
					break;
				case INVTYPE_SHOULDERS:
					itemIcon = fmtstring("INV_Shoulder_06");
					break;
				case INVTYPE_BODY:
					itemIcon = fmtstring("INV_Shirt_02");
					break;
				case INVTYPE_CHEST:
					itemIcon = fmtstring("INV_Chest_Chain");
					break;
				case INVTYPE_WAIST:
					itemIcon = fmtstring("INV_Belt_01");
					break;
				case INVTYPE_LEGS:
					
					itemIcon = fmtstring("INV_Pants_02");
					break;
				case INVTYPE_FEET:
					itemIcon = fmtstring("INV_Boots_09");
					break;
				case INVTYPE_WRISTS:
					itemIcon = fmtstring("INV_Bracer_07");
					break;
				case INVTYPE_HANDS:
					itemIcon = fmtstring("INV_Gauntlets_05");
					break;
				case INVTYPE_FINGER:
					itemIcon = fmtstring("INV_Jewelry_Ring_01");
					break;
				case INVTYPE_TRINKET:
					itemIcon = fmtstring("INV_Misc_PocketWatch_02");
					break;
				case INVTYPE_SHIELD:
					itemIcon = fmtstring("INV_Shield_09");
					break;
				case INVTYPE_CLOAK:
					
					
					itemIcon = fmtstring("INV_Misc_Cape_11");
					break;
				case INVTYPE_TABARD:
					itemIcon = fmtstring("INV_Banner_02");
					break;
				case INVTYPE_ROBE:
					
					itemIcon = fmtstring("INV_Shirt_01");
					break;
				case INVTYPE_RANGEDRIGHT:
					itemIcon = fmtstring("INV_Weapon_Bow_10");
					break;
				default:
					itemIcon = fmtstring("INV_Shield_09");
					break;
			}
			break;
		case ITEM_CLASS_REAGENT:
			itemIcon = fmtstring("INV_Misc_QuestionMark");
			break;
		case ITEM_CLASS_PROJECTILE:
			itemIcon = fmtstring("INV_Ammo_Arrow_01");
			break;
		case ITEM_CLASS_TRADE_GOODS:
			itemIcon = fmtstring("INV_TradeskillItem_03");
			break;
		case ITEM_CLASS_GENERIC:
			itemIcon = fmtstring("INV_Misc_QuestionMark");
			break;
		case ITEM_CLASS_BOOK:
			itemIcon = fmtstring("INV_Misc_Book_09");
			break;
		case ITEM_CLASS_MONEY:
			itemIcon = fmtstring("INV_Misc_Gem_Variety_01");
			break;
		case ITEM_CLASS_QUIVER:
			itemIcon = fmtstring("INV_Misc_Quiver_04");
			break;
		case ITEM_CLASS_QUEST:
			itemIcon = fmtstring("INV_Misc_QuestionMark");
			break;
		case ITEM_CLASS_KEY:
			itemIcon = fmtstring("INV_Misc_Key_04");
			break;
		case ITEM_CLASS_PERMANENT:
			itemIcon = fmtstring("INV_Misc_QuestionMark");
			break;
		case ITEM_CLASS_JUNK:
			itemIcon = fmtstring("INV_Misc_QuestionMark");
			break;
		default:
			
			itemIcon = fmtstring("INV_Misc_QuestionMark");
			break;
	}

	return itemIcon;
}

void Item::UpdateStats()
{
	



}

void Item::Create( uint32 guidlow, uint32 itemid, Player *owner )
{
    Object::_Create( guidlow, HIGHGUID_ITEM );

    SetUInt32Value( OBJECT_FIELD_ENTRY, itemid );
    SetFloatValue( OBJECT_FIELD_SCALE_X, 1.0f );

    SetUInt64Value( ITEM_FIELD_OWNER, owner->GetGUID() );
    SetUInt64Value( ITEM_FIELD_CONTAINED, owner->GetGUID() );
    SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );

    m_itemProto = objmgr.GetItemPrototype( itemid );
    ASSERT(m_itemProto);

    
    

    SetUInt32Value( ITEM_FIELD_MAXDURABILITY, m_itemProto->MaxDurability);
    SetUInt32Value( ITEM_FIELD_DURABILITY, m_itemProto->MaxDurability);


    SetUInt32Value( ITEM_FIELD_SPELL_CHARGES, m_itemProto->SpellCharges[0]);
    SetUInt32Value( ITEM_FIELD_SPELL_CHARGES+1, m_itemProto->SpellCharges[1]);
    SetUInt32Value( ITEM_FIELD_SPELL_CHARGES+2, m_itemProto->SpellCharges[2]);
    SetUInt32Value( ITEM_FIELD_SPELL_CHARGES+3, m_itemProto->SpellCharges[3]);
    SetUInt32Value( ITEM_FIELD_SPELL_CHARGES+4, m_itemProto->SpellCharges[4]);
    SetUInt32Value( ITEM_FIELD_FLAGS, m_itemProto->Flags);
    SetUInt32Value( ITEM_FIELD_ITEM_TEXT_ID, m_itemProto->DisplayInfoID);
    m_owner = owner;

	
	
	
    
	
	
	

	SetUInt32Value( ITEM_FIELD_DURATION, m_itemProto->Delay);
    
	
    
	SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, m_itemProto->DisplayInfoID);
	

	

	


	
	




	
	

}


void Item::SaveToDB()
{
    std::stringstream ss;
    ss << "DELETE FROM item_instances WHERE guid = " << GetGUIDLow();
    sDatabase.Execute( ss.str( ).c_str( ) );

    ss.rdbuf()->str("");
    ss << "INSERT INTO item_instances (guid, data) VALUES ("
        << GetGUIDLow() << ", '";                 

    for(uint16 i = 0; i < m_valuesCount; i++ )
        ss << GetUInt32Value(i) << " ";

    ss << "' )";

    sDatabase.Execute( ss.str().c_str() );
}


void Item::LoadFromDB(uint32 guid, uint32 auctioncheck)
{
    std::stringstream ss;
    if (auctioncheck == 1)
    {
        ss << "SELECT data FROM item_instances WHERE guid = " << guid;
    }
    else if (auctioncheck == 2)
    {
        ss << "SELECT data FROM auctioned_items WHERE guid = " << guid;
    }
    else
    {
        ss << "SELECT data FROM mailed_items WHERE guid = " << guid;
    }

    QueryResult *result = sDatabase.Query( ss.str().c_str() );
    if(result==NULL)
        return;
    

    Field *fields = result->Fetch();

    LoadValues( fields[0].GetString() );

    delete result;

    m_itemProto = objmgr.GetItemPrototype( GetUInt32Value(OBJECT_FIELD_ENTRY) );
    ASSERT(m_itemProto);

}


void Item::DeleteFromDB()
{
    std::stringstream ss;
    ss << "DELETE FROM item_instances WHERE guid = " << GetGUIDLow();
    sDatabase.Execute( ss.str( ).c_str( ) );
}
