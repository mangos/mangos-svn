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
#include "Item.h"
#include "ObjectMgr.h"
#include "Database/DatabaseEnv.h"

SpellEntry* Cast(Player*player,Item* item, uint32 spellId)
{

    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i", spellId);
        return NULL;
    }
    Spell *spell = new Spell(player, spellInfo, true, 0);
    WPAssert(spell);

    SpellCastTargets targets;
    targets.setUnitTarget( player );
    spell->m_CastItem = item;
    spell->prepare(&targets);
    return spellInfo;

}

void AddItemsSetItem(Player*player,Item *item)
{
    ItemPrototype *proto = item->GetProto();
    uint32 setid = proto->ItemSet;

    ItemSetEntry *set=sItemSetStore.LookupEntry(setid);

    if(!set)
    {
        sLog.outError("Item set %u for item (id %u) not found, mods not applied.",setid,proto->ItemId);
        return;
    }

    if(set->required_skill_id )
        if(player->GetSkillValue(set->required_skill_id) < set->required_skill_value) return;

    ItemsSetEffect *eff=NULL;

    for(uint32 x =0;x<3;x++)
        if(player->ItemsSetEff[x])
            if(player->ItemsSetEff[x]->setid==setid)
            {
                eff=player->ItemsSetEff[x];
                break;
            }

    if(!eff)
    {
        eff=new ItemsSetEffect;
        memset(eff,0,sizeof(ItemsSetEffect));
        eff->setid=setid;

        for(uint32 x =0;x<3;x++)
            if(!player->ItemsSetEff[x])
        {
            player->ItemsSetEff[x]=eff;
            break;
        }
    }

    eff->item_count++;

    for(uint32 x=0;x<8;x++)
        if(set->spells [x])
                                                            //enough for  spell
            if(set->items_to_triggerspell[x] <= eff->item_count)
            {
                uint32 z=0;
                for(;z<8;z++)
                    if(eff->spells[z])
                        if(eff->spells[z]->Id==set->spells[x])break;

                if(z==8)                                    //new spell
                    for(uint32 y=0;y<8;y++)
                        if(!eff->spells[y])
                        {
                            eff->spells[y]=Cast(player,item,set->spells[x]);
                            break;
                        }
            }

}

void RemoveItemsSetItem(Player*player,ItemPrototype *proto)
{
    uint32 setid = proto->ItemSet;

    ItemSetEntry *set=sItemSetStore.LookupEntry(setid);

    if(!set)
    {
        sLog.outError("Item set #%u for item #%u not found, mods not removed.",setid,proto->ItemId);
        return;
    }

    ItemsSetEffect *eff=NULL;
    uint32 setindex=0;
    for(;setindex<3;setindex++)
        if(player->ItemsSetEff[setindex])
            if(player->ItemsSetEff[setindex]->setid==setid)
            {
                eff=player->ItemsSetEff[setindex];
                break;
            }

    if(!eff)
    {
        sLog.outError("Item set effect for equiped item #%u not found, mods not removed.",proto->ItemId);
        return;
    }

    eff->item_count--;

    for(uint32 x=0;x<8;x++)
        if(set->spells[x])
                                                            //not enough for spell
            if(set->items_to_triggerspell[x] > eff->item_count)
            {
                for(uint32 z=0;z<8;z++)
                    if(eff->spells[z])
                        if(eff->spells[z]->Id==set->spells[x])
                        {
                            for(uint32 i =0;x<3;++x)
                                player->RemoveAura(eff->spells[z]->Id,i);
                            eff->spells[z]=NULL;
                            break;
                        }

            }

    if(!eff->item_count)                                    //all items of a set were removed
    {
        assert(eff==player->ItemsSetEff[setindex]);
        delete eff;
        player->ItemsSetEff[setindex]=NULL;

    }
}

Item::Item( )
{
    m_objectType |= TYPE_ITEM;
    m_objectTypeId = TYPEID_ITEM;

    m_valuesCount = ITEM_END;
    m_slot = 0;
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

/*
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
        sLog.outDebug( "Image file is [%s].", itemDisplayTemplateEntry->invImageFile );
        return fmtstring("%s", itemDisplayTemplateEntry->invImageFile);
    }
    else
    {
        return fmtstring("");
    }
}

#define ITEM_SUBCLASS_FOOD                    1
#define ITEM_SUBCLASS_LIQUID                2

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
*/
bool Item::Create( uint32 guidlow, uint32 itemid, Player* owner)
{
    Object::_Create( guidlow, HIGHGUID_ITEM );

    SetUInt32Value(OBJECT_FIELD_ENTRY, itemid);
    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);

    SetUInt64Value(ITEM_FIELD_OWNER, owner->GetGUID());
    SetUInt64Value(ITEM_FIELD_CONTAINED, owner->GetGUID());

    ItemPrototype *m_itemProto = objmgr.GetItemPrototype(itemid);
    if(!m_itemProto)
        return false;

    SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);
    SetUInt32Value(ITEM_FIELD_MAXDURABILITY, m_itemProto->MaxDurability);
    SetUInt32Value(ITEM_FIELD_DURABILITY, m_itemProto->MaxDurability);

    SetUInt32Value(ITEM_FIELD_SPELL_CHARGES, m_itemProto->Spells[0].SpellCharges );
    SetUInt32Value(ITEM_FIELD_SPELL_CHARGES+1,m_itemProto->Spells[1].SpellCharges);
    SetUInt32Value(ITEM_FIELD_SPELL_CHARGES+2, m_itemProto->Spells[2].SpellCharges);
    SetUInt32Value(ITEM_FIELD_SPELL_CHARGES+3,m_itemProto->Spells[3].SpellCharges);
    SetUInt32Value(ITEM_FIELD_SPELL_CHARGES+4, m_itemProto->Spells[4].SpellCharges);
    SetUInt32Value(ITEM_FIELD_FLAGS, m_itemProto->Flags);
    SetUInt32Value(ITEM_FIELD_DURATION, m_itemProto->Delay);
    return true;
}

void Item::SaveToDB()
{
    uint32 guid = GetGUIDLow();
    sDatabase.PExecute("DELETE FROM `item_instance` WHERE `guid` = '%u'", guid);
    std::stringstream ss;
    ss.rdbuf()->str("");
    ss << "INSERT INTO `item_instance` (`guid`,`data`) VALUES ("
        << guid << ",'";

    for(uint16 i = 0; i < m_valuesCount; i++ )
        ss << GetUInt32Value(i) << " ";

    ss << "' )";

    sDatabase.Execute( ss.str().c_str() );
}

bool Item::LoadFromDB(uint32 guid, uint64 owner_guid, uint32 auctioncheck)
{
    QueryResult *result;

    if (auctioncheck == 1)
    {
        result = sDatabase.PQuery("SELECT `data` FROM `item_instance` WHERE `guid` = '%u';", guid);
    }
    else if (auctioncheck == 2)
    {
        result = sDatabase.PQuery("SELECT `data` FROM `auctionhouse_item` WHERE `guid` = '%u';", guid);
    }
    else
    {
        result = sDatabase.PQuery("SELECT `data` FROM `mail_item` WHERE `guid` = '%u';", guid);
    }

    if (!result) return false;

    Field *fields = result->Fetch();

    LoadValues(fields[0].GetString());

    if(GetOwnerGUID()!=owner_guid) 
    {
        sLog.outError("Item::LoadFromDB: item: %u have in DB owner guid: %lu. Updated to correct: %lu",GetOwnerGUID(),owner_guid);
        SetOwnerGUID(owner_guid);
    }


    delete result;

    return true;
}

void Item::DeleteFromDB()
{
    sDatabase.PExecute("DELETE FROM `item_instance` WHERE `guid` = '%u'",GetGUIDLow());
}

ItemPrototype *Item::GetProto() const
{
    return objmgr.GetItemPrototype(GetUInt32Value(OBJECT_FIELD_ENTRY));
}

Player* Item::GetOwner()const
{
    return objmgr.GetPlayer(GetOwnerGUID());
}


uint32 Item::GetSkill()
{
    const static uint32 item_weapon_skills[]=
    {
        SKILL_AXES, SKILL_2H_AXES,SKILL_BOWS, SKILL_GUNS,SKILL_MACES, SKILL_2H_MACES,
        SKILL_POLEARMS, SKILL_SWORDS,SKILL_2H_SWORDS,0, SKILL_STAVES,0,0,0,0, SKILL_DAGGERS,
        SKILL_THROWN, SKILL_SPEARS, SKILL_CROSSBOWS, SKILL_WANDS, SKILL_FISHING
    };

    const static uint32 item_armor_skills[]=
    {
        0,SKILL_CLOTH,SKILL_LEATHER,SKILL_MAIL,SKILL_PLATE_MAIL,0,SKILL_SHIELD
    };

    switch (GetProto()->Class)
    {
        case ITEM_CLASS_WEAPON:
            if( GetProto()->SubClass >= sizeof(item_weapon_skills)/4 )
                return 0;
            else
                return item_weapon_skills[GetProto()->SubClass];

        case ITEM_CLASS_ARMOR:
            if( GetProto()->SubClass >= sizeof(item_armor_skills)/4 )
                return 0;
            else
                return item_armor_skills[GetProto()->SubClass];

        default:
            return 0;
    }
}

uint32 Item::GetSpell()
{
    switch (GetProto()->Class)
    {
        case ITEM_CLASS_WEAPON:
            switch (GetProto()->SubClass)
            {
                case ITEM_SUBCLASS_WEAPON_AXE: return 196;
                case ITEM_SUBCLASS_WEAPON_AXE2: return 197;
                case ITEM_SUBCLASS_WEAPON_BOW: return 264;
                case ITEM_SUBCLASS_WEAPON_GUN: return 266;
                case ITEM_SUBCLASS_WEAPON_MACE: return 198;
                case ITEM_SUBCLASS_WEAPON_MACE2: return 199;
                case ITEM_SUBCLASS_WEAPON_POLEARM: return 200;
                case ITEM_SUBCLASS_WEAPON_SWORD: return 201;
                case ITEM_SUBCLASS_WEAPON_SWORD2: return 202;
                case ITEM_SUBCLASS_WEAPON_STAFF: return 227;
                case ITEM_SUBCLASS_WEAPON_DAGGER: return 1180;
                case ITEM_SUBCLASS_WEAPON_THROWN: return 2567;
                case ITEM_SUBCLASS_WEAPON_SPEAR: return 3386;
                case ITEM_SUBCLASS_WEAPON_CROSSBOW: return 5011;
                case ITEM_SUBCLASS_WEAPON_WAND: return 5009;
                default: return 0;
            }
        case ITEM_CLASS_ARMOR:
            switch(GetProto()->SubClass)
            {
                case ITEM_SUBCLASS_ARMOR_CLOTH: return 9078;
                case ITEM_SUBCLASS_ARMOR_LEATHER: return 9077;
                case ITEM_SUBCLASS_ARMOR_MAIL: return 8737;
                case ITEM_SUBCLASS_ARMOR_PLATE: return 750;
                case ITEM_SUBCLASS_ARMOR_SHIELD: return 9116;
                default: return 0;
            }
    }
    return 0;
}
