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
#include "Item.h"
#include "ObjectMgr.h"
#include "Database/DatabaseEnv.h"

SpellEntry const* Cast(Player*player,Item* item, uint32 spellId)
{

    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i", spellId);
        return NULL;
    }
    Spell spell(player, spellInfo, true, 0);

    SpellCastTargets targets;
    targets.setUnitTarget( player );
    spell.m_CastItem = item;
    spell.prepare(&targets);
    return spellInfo;

}

void AddItemsSetItem(Player*player,Item *item)
{
    ItemPrototype const *proto = item->GetProto();
    uint32 setid = proto->ItemSet;

    ItemSetEntry const *set = sItemSetStore.LookupEntry(setid);

    if(!set)
    {
        sLog.outErrorDb("Item set %u for item (id %u) not found, mods not applied.",setid,proto->ItemId);
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

    ++eff->item_count;

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

void RemoveItemsSetItem(Player*player,ItemPrototype const *proto)
{
    uint32 setid = proto->ItemSet;

    ItemSetEntry const *set = sItemSetStore.LookupEntry(setid);

    if(!set)
    {
        sLog.outErrorDb("Item set #%u for item #%u not found, mods not removed.",setid,proto->ItemId);
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

    // can be in case now enough skill requirement for set appling but set has been appliend when skill requirement not enough
    if(!eff)
        return;

    --eff->item_count;

    for(uint32 x=0;x<8;x++)
        if(set->spells[x])
                                                            //not enough for spell
            if(set->items_to_triggerspell[x] > eff->item_count)
            {
                for(uint32 z=0;z<8;z++)
                    if(eff->spells[z])
                        if(eff->spells[z]->Id==set->spells[x])
                        {
                            player->RemoveAurasDueToSpell(set->spells[x]);
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
    uState = ITEM_NEW;
    uQueuePos = -1;
    m_container = NULL;
    m_lootGenerated = false;
}

uint32 GetRandPropertiesSeedfromDisplayInfoDBC(uint32 DisplayID)
{
    /*
    ItemDisplayInfoEntry *itemDisplayInfoEntry = sItemDisplayInfoStore.LookupEntry( DisplayID );

    if (itemDisplayInfoEntry)
        return itemDisplayInfoEntry->randomPropertyChance;
    else
        return 0;
    */
    return 0;
}

uint32 GetRandPropertiesIDfromDisplayInfoDBC(uint32 DisplayID)
{
    /*
    ItemDisplayInfoEntry *itemDisplayInfoEntry = sItemDisplayInfoStore.LookupEntry( DisplayID );

    if (itemDisplayInfoEntry)
        return itemDisplayInfoEntry->unknown;
    else
        return 0;
    */
    return 0;
}

/*
extern char *fmtstring( char *format, ... );

char *GetImageFilefromDisplayInfoDBC(uint32 DisplayID)
{
    ItemDisplayInfoEntry *itemDisplayInfoEntry = sItemDisplayInfoStore.LookupEntry( DisplayID );

    if (itemDisplayInfoEntry)
        return fmtstring("%s", itemDisplayInfoEntry->imageFile);
    else
        return fmtstring("");
}

char *GetInventoryImageFilefromDisplayInfoDBC(uint32 DisplayID)
{
    ItemDisplayInfoEntry *itemDisplayInfoEntry = sItemDisplayInfoStore.LookupEntry( DisplayID );

    if (itemDisplayInfoEntry && itemDisplayInfoEntry->invImageFile)
    {
        sLog.outDebug( "Image file is [%s].", itemDisplayInfoEntry->invImageFile );
        return fmtstring("%s", itemDisplayInfoEntry->invImageFile);
    }
    else
    {
        return fmtstring("");
    }
}

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

    ItemPrototype const *itemProto = objmgr.GetItemPrototype(itemid);
    if(!itemProto)
        return false;

    SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);
    SetUInt32Value(ITEM_FIELD_MAXDURABILITY, itemProto->MaxDurability);
    SetUInt32Value(ITEM_FIELD_DURABILITY, itemProto->MaxDurability);

    SetUInt32Value(ITEM_FIELD_SPELL_CHARGES,  uint32(abs(itemProto->Spells[0].SpellCharges)));
    SetUInt32Value(ITEM_FIELD_SPELL_CHARGES+1,uint32(abs(itemProto->Spells[1].SpellCharges)));
    SetUInt32Value(ITEM_FIELD_SPELL_CHARGES+2,uint32(abs(itemProto->Spells[2].SpellCharges)));
    SetUInt32Value(ITEM_FIELD_SPELL_CHARGES+3,uint32(abs(itemProto->Spells[3].SpellCharges)));
    SetUInt32Value(ITEM_FIELD_SPELL_CHARGES+4,uint32(abs(itemProto->Spells[4].SpellCharges)));
    SetUInt32Value(ITEM_FIELD_FLAGS, itemProto->Flags);
    //SetUInt32Value(ITEM_FIELD_DURATION, itemProto->Delay); ITEM_FIELD_DURATION is time until item expires, not speed

    _LoadQuests();

    return true;
}

void Item::SaveToDB()
{
    uint32 guid = GetGUIDLow();
    switch (uState)
    {
        case ITEM_NEW:
        {
            // it's better than rebuilding indexes multiple times
            QueryResult *result = sDatabase.PQuery("select count(*) as r from `item_instance` where `guid` = '%u'", guid);
            Field *fields = result->Fetch();
            uint32 Rows = fields[0].GetUInt32();
            delete result;
            // guess - instance exists ?
            if (!Rows)
            {
                // no - we must insert new rec
                std::ostringstream ss;
                ss << "INSERT INTO `item_instance` (`guid`,`owner_guid`,`data`) VALUES (" << guid << "," << GUID_LOPART(GetOwnerGUID()) << ",'";
                for(uint16 i = 0; i < m_valuesCount; i++ )
                    ss << GetUInt32Value(i) << " ";
                ss << "' )";

                sDatabase.Execute( ss.str().c_str() );
            } else
            {
                std::ostringstream ss;
                ss << "UPDATE `item_instance` SET `data` = '";
                for(uint16 i = 0; i < m_valuesCount; i++ )
                    ss << GetUInt32Value(i) << " ";
                ss << "' WHERE `guid` = '" << guid << "'";
                sDatabase.Execute( ss.str().c_str() );
            };
        } break;
        case ITEM_CHANGED:
        {
            std::ostringstream ss;
            ss << "UPDATE `item_instance` SET `data` = '";
            for(uint16 i = 0; i < m_valuesCount; i++ )
                ss << GetUInt32Value(i) << " ";
            ss << "' WHERE `guid` = '" << guid << "'";

            sDatabase.Execute( ss.str().c_str() );
        } break;
        case ITEM_REMOVED:
        {
            if (GetUInt32Value(ITEM_FIELD_ITEM_TEXT_ID) > 2282 )
                sDatabase.PExecute("DELETE FROM `item_page` WHERE `id` = '%u'", GetUInt32Value(ITEM_FIELD_ITEM_TEXT_ID));
            sDatabase.PExecute("DELETE FROM `item_instance` WHERE `guid` = '%u'", guid);
            delete this;
            return;
        }
        case ITEM_UNCHANGED:
            break;
    }
    SetState(ITEM_UNCHANGED);
}

bool Item::LoadFromDB(uint32 guid, uint64 owner_guid)
{
    QueryResult *result = sDatabase.PQuery("SELECT `data` FROM `item_instance` WHERE `guid` = '%u'", guid);

    if (!result)
    {
        sLog.outError("ERROR: Item (GUID: %u owner: %u) not found in table `item_instance`, can't load. ",guid,owner_guid);
        return false;
    }

    Field *fields = result->Fetch();

    if(!LoadValues(fields[0].GetString()))
    {
        sLog.outError("ERROR: Item #%d have broken data in `data` field. Can't be loaded.",guid);
        delete result;
        return false;
    }

    if(owner_guid != 0 && GetOwnerGUID()!=owner_guid)
    {
        sLog.outError("Item::LoadFromDB: item: %u have in DB owner guid: %u. Updated to correct: %u",GetGUIDLow(),GUID_LOPART(GetOwnerGUID()), GUID_LOPART(owner_guid));
        SetOwnerGUID(owner_guid);
    }

    delete result;

    _LoadQuests();
    return true;
}

void Item::DeleteFromDB()
{
    sDatabase.PExecute("DELETE FROM `item_instance` WHERE `guid` = '%u'",GetGUIDLow());
}

void Item::DeleteFromInventoryDB()
{
    sDatabase.PExecute("DELETE FROM `character_inventory` WHERE `item` = '%u'",GetGUIDLow());
}

void Item::_LoadQuests()
{
    mQuests.clear();

    ItemPrototype const *itemProto = GetProto();
    if(itemProto && itemProto->StartQuest)
    {
        Quest * quest = objmgr.QuestTemplates[itemProto->StartQuest];
        if(quest)
            addQuest(itemProto->StartQuest);
        else
            sLog.outErrorDb("Item (Entry: %u) can start quest %u but quest not exist, ignoring.",GetEntry(),itemProto->StartQuest);
    }
}

ItemPrototype const *Item::GetProto() const
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

void Item::SetItemRandomProperties()
{
    ItemPrototype const *itemProto = GetProto();

    // only for bounded item
    if(itemProto->Bonding != BIND_WHEN_PICKED_UP && itemProto->Bonding != BIND_WHEN_EQUIPED)
        return;

    // only white or green item
    if(itemProto->Quality != ITEM_QUALITY_NORMAL && itemProto->Quality != ITEM_QUALITY_UNCOMMON)
        return;

    // only for specific item classes
    if(itemProto->Class != ITEM_CLASS_WEAPON && itemProto->Class != ITEM_CLASS_JEWELRY && itemProto->Class != ITEM_CLASS_ARMOR)
        return;

    // only if not other stats bonuses
    for(uint8 i = 0; i < 10; i++)
    {
        if(itemProto->ItemStat[i].ItemStatValue > 0)
            return;
    }

    // maybe just hack here,we should find out the correct random_id in DBC
    uint32 random_id,enchant_id_1;
    enchant_id_1 = random_id = 0;

    if(irand(1,100) <= 60 )
    {
        switch(irand(1,100)%14)
        {
                                                            // of the Wolf
            case 1:random_id = 501 + uint32((583-501)/150.0f*(float)itemProto->ItemLevel);break;
                                                            // of the Monkey
            case 2:random_id = 584 + uint32((668-584)/150.0f*(float)itemProto->ItemLevel);break;
                                                            // of the Tiger
            case 3:random_id = 669 + uint32((753-669)/150.0f*(float)itemProto->ItemLevel);break;
                                                            // of the Owl
            case 4:random_id = 754 + uint32((838-754)/150.0f*(float)itemProto->ItemLevel);break;
                                                            // of the Eagle
            case 5:random_id = 839 + uint32((923-839)/150.0f*(float)itemProto->ItemLevel);break;
                                                            // of the Gorilla
            case 6:random_id = 924 + uint32((1008-924)/150.0f*(float)itemProto->ItemLevel);break;
                                                            // of the Whale
            case 7:random_id = 1009 + uint32((1093-1009)/150.0f*(float)itemProto->ItemLevel);break;
                                                            // of the Boar
            case 8:random_id = 1094 + uint32((1178-1094)/150.0f*(float)itemProto->ItemLevel);break;
                                                            // of the Bear
            case 9:random_id = 1179 + uint32((1263-1179)/150.0f*(float)itemProto->ItemLevel);break;
                                                            // of Arcane Resistance
            case 10:random_id = 1307 + uint32((1352-1307)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Frost Resistance
            case 11:random_id = 1353 + uint32((1398-1353)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Fire Resistance
            case 12:random_id = 1399 + uint32((1444-1399)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Shadow Resistance
            case 13:random_id = 1445 + uint32((1490-1445)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Nature Resistance
            case 0:random_id = 1491 + uint32((1536-1491)/100.0f*(float)itemProto->ItemLevel);break;
            default:break;
        }
    }

    if(irand(1,100) <= 50 && !random_id)
    {
        if(itemProto->Class == ITEM_CLASS_ARMOR)
        {
            if(itemProto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
                                                            // of Blocking
                random_id = 1647 + uint32((1703-1647)/100.0f*(float)itemProto->ItemLevel);
            else if(irand(1,100) <= 30)
                                                            // of Defense
                random_id = 1607 + uint32((1636-1607)/100.0f*(float)itemProto->ItemLevel);
        }
        else if(itemProto->Class == ITEM_CLASS_JEWELRY)
        {
            switch(irand(1,3))
            {
                                                            // of Healing
                case 1:random_id = 2026 + uint32((2064-2026)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Concentration
                case 2:random_id = 2067 + uint32((2104-2067)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Regeneration
                case 3:random_id = 2105 + uint32((2142-2105)/100.0f*(float)itemProto->ItemLevel);break;
                default:break;
            }
        }
        else if(itemProto->Class == ITEM_CLASS_WEAPON)
        {
            if((itemProto->SubClass == ITEM_SUBCLASS_WEAPON_GUN || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_BOW
                || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_THROWN) && irand(1,100) <= 50)
            {
                                                            // of Marksmanship
                random_id = 1704 + uint32((1741-1704)/100.0f*(float)itemProto->ItemLevel);
            }
            else
            {
                switch(irand(1,8))
                {
                                                            // of Attack Power
                    case 1:random_id = 1547 + uint32((1592-1547)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Eluding
                    case 2:random_id = 1742 + uint32((1798-1742)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Arcane Wrath
                    case 3:random_id = 1799 + uint32((1836-1799)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Shadow Wrath
                    case 4:random_id = 1837 + uint32((1874-1837)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Fiery Wrath
                    case 5:random_id = 1875 + uint32((1912-1875)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Holy Wrath
                    case 6:random_id = 1913 + uint32((1950-1913)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Frozen Wrath
                    case 7:random_id = 1951 + uint32((1988-1951)/100.0f*(float)itemProto->ItemLevel);break;
                                                            // of Nature's Wrath
                    case 8:random_id = 1989 + uint32((2026-1989)/100.0f*(float)itemProto->ItemLevel);break;
                    default:break;
                }
            }
        }
    }

    if(itemProto->ItemLevel <= 15 && !random_id)
    {
        random_id = irand(1,34);
    }
    else if(itemProto->ItemLevel <= 30 && !random_id)
    {
        switch(irand(1,4))
        {
            case 1:random_id = irand(93,99);break;
            case 2:random_id = irand(111,119);break;
            case 3:random_id = irand(130,138);break;
            case 4:random_id = irand(151,155);break;
            default:break;
        }
    }
    else if(itemProto->ItemLevel <= 45 && !random_id)
    {
        random_id = irand(167,220);
    }
    else if(itemProto->ItemLevel <= 60 && !random_id)
    {
        random_id = irand(308,434);
    }
    //else if(itemProto->ItemLevel <= 60 && !random_id)
    //{
    //    random_id = irand(1267,1296);
    //}

    ItemRandomPropertiesEntry const *item_rand = sItemRandomPropertiesStore.LookupEntry(random_id);

    if(item_rand)
    {
        SetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID,item_rand->ID);
        SetUInt32Value(ITEM_FIELD_ENCHANTMENT+9,item_rand->enchant_id_1);
        SetUInt32Value(ITEM_FIELD_ENCHANTMENT+12,item_rand->enchant_id_2);
        SetUInt32Value(ITEM_FIELD_ENCHANTMENT+15,item_rand->enchant_id_3);
    }
}

void Item::SetState(ItemUpdateState state, Player *forplayer)
{
    if (uState == ITEM_NEW && state == ITEM_REMOVED)
    {
        // pretend the item never existed
        RemoveFromUpdateQueueOf(forplayer);
        delete this;
        return;
    }

    if (state != ITEM_UNCHANGED)
    {
        // new items must stay in new state until saved
        if (uState != ITEM_NEW) uState = state;
        AddToUpdateQueueOf(forplayer);
    }
    else
    {
        // unset in queue
        // the item must be removed from the queue manually
        uQueuePos = -1;
        uState = ITEM_UNCHANGED;
    }
}

void Item::AddToUpdateQueueOf(Player *player)
{
    if (IsInUpdateQueue()) return;

    if (!player)
    {
        player = GetOwner();
        if (!player)
        {
            sLog.outError("Item::AddToUpdateQueueOf - GetPlayer didn't find a player matching owner's guid (%u)!", GUID_LOPART(GetOwnerGUID()));
            return;
        }
    }

    if (player->GetGUID() != GetOwnerGUID())
    {
        sLog.outError("Item::AddToUpdateQueueOf - Owner's guid (%u) and player's guid (%u) don't match!", GUID_LOPART(GetOwnerGUID()), player->GetGUIDLow());
        return;
    }

    if (player->m_itemUpdateQueueBlocked) return;

    player->m_itemUpdateQueue.push_back(this);
    uQueuePos = player->m_itemUpdateQueue.size()-1;
}

void Item::RemoveFromUpdateQueueOf(Player *player)
{
    if (!IsInUpdateQueue()) return;

    if (!player)
    {
        player = GetOwner();
        if (!player)
        {
            sLog.outError("Item::RemoveFromUpdateQueueOf - GetPlayer didn't find a player matching owner's guid (%u)!", GUID_LOPART(GetOwnerGUID()));
            return;
        }
    }

    if (player->GetGUID() != GetOwnerGUID())
    {
        sLog.outError("Item::RemoveFromUpdateQueueOf - Owner's guid (%u) and player's guid (%u) don't match!", GUID_LOPART(GetOwnerGUID()), player->GetGUIDLow());
        return;
    }

    if (player->m_itemUpdateQueueBlocked) return;

    player->m_itemUpdateQueue[uQueuePos] = NULL;
    uQueuePos = -1;
}

uint8 Item::GetBagSlot() const
{
    return m_container ? m_container->GetSlot() : INVENTORY_SLOT_BAG_0;
}

bool Item::IsEquipped() const
{
    return !IsInBag() && m_slot < EQUIPMENT_SLOT_END;
}

bool Item::CanBeTraded() const
{
    if(HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAGS_BINDED) || GetProto()->Class == ITEM_CLASS_QUEST)
        return false;
    if(IsBag() && !((Bag*)this)->IsEmpty())
        return false;
    return true;
}

bool Item::CanGoIntoBag(ItemPrototype const *pBagProto)
{
    ItemPrototype const *pProto = GetProto();

    if(!pProto || !pBagProto)
        return false;

    switch(pBagProto->Class)
    {
        case ITEM_CLASS_CONTAINER:
            switch(pBagProto->SubClass)
            {
                case ITEM_SUBCLASS_CONTAINER:
                    return true;
                case ITEM_SUBCLASS_SOUL_CONTAINER:
                    if(pProto->BagFamily != BAG_FAMILY_SOUL_SHARDS)
                        return false;
                    return true;
                case ITEM_SUBCLASS_HERB_CONTAINER:
                    if(pProto->BagFamily != BAG_FAMILY_HERBS)
                        return false;
                    return true;
                case ITEM_SUBCLASS_ENCHANTING_CONTAINER:
                    if(pProto->BagFamily != BAG_FAMILY_ENCHANTING_SUPP)
                        return false;
                    return true;
                case ITEM_SUBCLASS_ENGINEERING_CONTAINER:
                    if(pProto->BagFamily != BAG_FAMILY_ENGINEERING_SUPP)
                        return false;
                    return true;
                default:
                    return false;
            }
        case ITEM_CLASS_QUIVER:
            switch(pBagProto->SubClass)
            {
                case ITEM_SUBCLASS_QUIVER:
                    if(pProto->BagFamily != BAG_FAMILY_ARROWS)
                        return false;
                    return true;
                case ITEM_SUBCLASS_AMMO_POUCH:
                    if(pProto->BagFamily != BAG_FAMILY_BULLETS)
                        return false;
                    return true;
                default:
                    return false;
            }
    }
    return false;
}

bool Item::IsFitToSpellRequirements(SpellEntry const* spellInfo) const
{
    ItemPrototype const* proto = GetProto();

    if (spellInfo->EquippedItemClass != -1)                 // -1 == any item class
    {
        if(spellInfo->EquippedItemClass != int32(proto->Class))
            return false;                                   //  wrong item class

        if(spellInfo->EquippedItemSubClassMask != 0)        // 0 == any subclass
        {
            if((spellInfo->EquippedItemSubClassMask & (1 << proto->SubClass)) == 0)
                return false;                               // subclass not present in mask
        }
    }

    if(spellInfo->EquippedItemInventoryTypeMask != 0)       // 0 == any inventory type
    {
        if((spellInfo->EquippedItemInventoryTypeMask  & (1 << proto->InventoryType)) == 0)
            return false;                                   // inventory type not present in mask
    }

    return true;
}
