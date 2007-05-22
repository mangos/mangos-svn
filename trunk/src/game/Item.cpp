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
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_ALL);

    m_valuesCount = ITEM_END;
    m_slot = 0;
    uState = ITEM_NEW;
    uQueuePos = -1;
    m_container = NULL;
    m_lootGenerated = false;
    mb_in_trade = false;
}

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

    return true;
}

void Item::SaveToDB(bool first_save)
{
    uint32 guid = GetGUIDLow();
    switch (uState)
    {
        case ITEM_NEW:
        {
            // it's better than rebuilding indexes multiple times
            QueryResult *result = sDatabase.Query("select count(*) as r from `item_instance` where `guid` = '%u'", guid);
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

                //sDatabase.Execute( ss.str().c_str() );
                if(first_save)
                    sDatabase.Execute( ss.str().c_str() );
                else
                    sDatabase.WaitExecute(ss.str().c_str());
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
            ss << "', `owner_guid` = '" << GUID_LOPART(GetOwnerGUID()) << "' WHERE `guid` = '" << guid << "'";

            sDatabase.Execute( ss.str().c_str() );

            if(HasFlag(ITEM_FIELD_FLAGS, 8))
                sDatabase.Execute("UPDATE `character_gifts` SET `guid` = '%u' WHERE `item_guid` = '%u'", GUID_LOPART(GetOwnerGUID()),GetGUIDLow());
        } break;
        case ITEM_REMOVED:
        {
            if (GetUInt32Value(ITEM_FIELD_ITEM_TEXT_ID) > 0 )
                sDatabase.Execute("DELETE FROM `item_text` WHERE `id` = '%u'", GetUInt32Value(ITEM_FIELD_ITEM_TEXT_ID));
            sDatabase.Execute("DELETE FROM `item_instance` WHERE `guid` = '%u'", guid);
            if(HasFlag(ITEM_FIELD_FLAGS, 8))
                sDatabase.Execute("DELETE FROM `character_gifts` WHERE `item_guid` = '%u'", GetGUIDLow());
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
    QueryResult *result = sDatabase.Query("SELECT `data` FROM `item_instance` WHERE `guid` = '%u'", guid);

    if (!result)
    {
        sLog.outError("ERROR: Item (GUID: %u owner: %u) not found in table `item_instance`, can't load. ",guid,owner_guid);
        return false;
    }

    Field *fields = result->Fetch();

    _Create(guid, HIGHGUID_ITEM);

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

    return true;
}

void Item::DeleteFromDB()
{
    sDatabase.Execute("DELETE FROM `item_instance` WHERE `guid` = '%u'",GetGUIDLow());
}

void Item::DeleteFromInventoryDB()
{
    sDatabase.Execute("DELETE FROM `character_inventory` WHERE `item` = '%u'",GetGUIDLow());
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
        SKILL_THROWN, SKILL_ASSASSINATION, SKILL_CROSSBOWS, SKILL_WANDS, SKILL_FISHING
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

uint32 Item::GenerateItemRandomPropertyId(uint32 item_id)
{
    ItemPrototype const *itemProto = sItemStorage.LookupEntry<ItemPrototype>(item_id);

    if(!itemProto)
        return 0;

    // only for bounded item
    if(itemProto->Bonding != BIND_WHEN_PICKED_UP && itemProto->Bonding != BIND_WHEN_EQUIPED)
        return 0;

    // only white or green item
    if(itemProto->Quality != ITEM_QUALITY_NORMAL && itemProto->Quality != ITEM_QUALITY_UNCOMMON)
        return 0;

    // only for specific item classes
    if(itemProto->Class != ITEM_CLASS_WEAPON && itemProto->Class != ITEM_CLASS_JEWELRY && itemProto->Class != ITEM_CLASS_ARMOR)
        return 0;

    // only if not other stats bonuses
    for(uint8 i = 0; i < 10; i++)
    {
        if(itemProto->ItemStat[i].ItemStatValue != 0)
            return 0;
    }

    for(uint8 i = 0; i < 5; i++)
    {
        if(itemProto->Spells[i].SpellId > 0)
            return 0;
    }

    if(itemProto->HolyRes > 0 || itemProto->FireRes > 0 || itemProto->NatureRes > 0 || itemProto->FrostRes > 0 || itemProto->ShadowRes > 0 || itemProto->ArcaneRes > 0)
        return 0;

    // maybe just hack here,we should find out the correct random_id in DBC
    uint32 random_id,enchant_id_1;
    enchant_id_1 = random_id = 0;

    std::vector<unsigned int> enchantlist;
    ItemRandomPropertiesEntry const *cur;
    float ItemValue = 0, tempItemValue = 0;
    float ItemSlotMod = 0;
    int32 wsc = -1;
    switch(itemProto->InventoryType)
    {
        case INVTYPE_HEAD:
            ItemSlotMod = ITEM_SLOT_HEAD_MOD; break;
        case INVTYPE_NECK:
            ItemSlotMod = ITEM_SLOT_NECK_MOD; break;
        case INVTYPE_SHOULDERS:
            ItemSlotMod = ITEM_SLOT_SHOULDERS_MOD; break;
        case INVTYPE_CHEST:
        case INVTYPE_ROBE:
            ItemSlotMod = ITEM_SLOT_CHEST_MOD; break;
        case INVTYPE_WAIST:
            ItemSlotMod = ITEM_SLOT_WAIST_MOD; break;
        case INVTYPE_LEGS:
            ItemSlotMod = ITEM_SLOT_LEGS_MOD; break;
        case INVTYPE_FEET:
            ItemSlotMod = ITEM_SLOT_FEET_MOD; break;
        case INVTYPE_WRISTS:
            ItemSlotMod = ITEM_SLOT_WRISTS_MOD; break;
        case INVTYPE_HANDS:
            ItemSlotMod = ITEM_SLOT_HANDS_MOD; break;
        case INVTYPE_FINGER:
            ItemSlotMod = ITEM_SLOT_FINGER_MOD; break;
        case INVTYPE_TRINKET:
            ItemSlotMod = ITEM_SLOT_TRINKET_MOD; break;
        case INVTYPE_SHIELD:
            ItemSlotMod = ITEM_SLOT_SHIELD_MOD; break;
        case INVTYPE_RANGED:
        case INVTYPE_RANGEDRIGHT:
            ItemSlotMod = ITEM_SLOT_RANGED_MOD; break;
        case INVTYPE_CLOAK:
            ItemSlotMod = ITEM_SLOT_BACK_MOD; break;
        case INVTYPE_2HWEAPON:
            ItemSlotMod = ITEM_SLOT_2HAND_MOD; break;
        case INVTYPE_WEAPONMAINHAND:
        case INVTYPE_WEAPON:
            ItemSlotMod = ITEM_SLOT_MAIN_HAND_MOD; break;
        case INVTYPE_WEAPONOFFHAND:
        case INVTYPE_HOLDABLE:
            ItemSlotMod = ITEM_SLOT_OFF_HAND_MOD; break;
        default:
            ItemSlotMod = 0; break;
    }

    if(!ItemSlotMod)
        return 0;

    for(unsigned int i = 1; i <= sItemRandomPropertiesStore.GetNumRows(); i++)
    {
        if(cur = sItemRandomPropertiesStore.LookupEntry(i))
        {
            ItemValue = GetEnchantMod(cur->enchant_id[0], itemProto);
            if(!ItemValue)
                continue;
            if(cur->enchant_id[1])
            {
                tempItemValue = GetEnchantMod(cur->enchant_id[1], itemProto);
                if(!tempItemValue)
                    continue;
                ItemValue += tempItemValue;
            }
            if(cur->enchant_id[2])
            {
                tempItemValue = GetEnchantMod(cur->enchant_id[2], itemProto);
                if(!tempItemValue)
                    continue;
                ItemValue += tempItemValue;
            }
            ItemValue = pow(ItemValue, 2.0f/3.0f )*ItemSlotMod;
            switch(itemProto->Quality)
            {
                case ITEM_QUALITY_UNCOMMON:
                    ItemValue = ItemValue*2.0 + 4.0; break;
                case ITEM_QUALITY_RARE:
                    ItemValue = ItemValue*1.6 + 1.84; break;
                case ITEM_QUALITY_EPIC:
                    ItemValue = ItemValue*1.3 + 1.3; break;
            }
            if((uint32)ItemValue > itemProto->RequiredLevel && (uint32)ItemValue <= (itemProto->RequiredLevel + 5))
            {
                enchantlist.insert(enchantlist.end(),i);
            }
        }
    }
    if(!enchantlist.size())
        return 0;
    random_id = enchantlist.at(irand(1,enchantlist.size())-1);
    return random_id;
}

void Item::SetItemRandomProperties(uint32 randomPropId)
{
    if(!randomPropId)
        return;

    ItemRandomPropertiesEntry const *item_rand = sItemRandomPropertiesStore.LookupEntry(randomPropId);
    if(item_rand)
    {
        SetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID,item_rand->ID);
        for(uint32 i = PROP_ENCHANTMENT_SLOT; i < PROP_ENCHANTMENT_SLOT + 3; ++i)
            SetEnchantment(EnchantmentSlot(i),item_rand->enchant_id[i],0,0);
    }
}

float Item::GetEnchantMod(uint32 enchant_id, ItemPrototype const * itemProto)
{
    if(!enchant_id)
        return 0;
    if(!itemProto)
        return 0;

    SpellItemEnchantmentEntry const *entry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if(!entry)
        return 0;
    int32 weapon_subclass;
    if(itemProto->Class == ITEM_CLASS_WEAPON)
        weapon_subclass = itemProto->SubClass;
    else
        weapon_subclass = -1;
    int32 inv_type = itemProto->InventoryType;
    float tempmod, mod = 0;
    for (int s=0;s<3;s++)
    {
        uint32 en_display = entry->display_type[s];
        uint32 en_value1 = entry->amount[s];
        uint32 en_spellid = entry->spellid[s];
        if(en_display == 4)
        {
            mod += pow((float)(en_value1 * ITEM_STAT_ARMOR_MOD), 1.50f);
        }
        else if(en_display == 2)
        {
            mod += pow((float)(en_value1 * ITEM_STAT_ATTACK_POWER_MOD * 3), 1.50f);
        }
        else if(en_display == 3 && en_spellid)
        {
            SpellEntry const *en_spellinfo = sSpellStore.LookupEntry(en_spellid);
            if(!en_spellinfo)
                return 0;
            for(uint8 i = 0; i<3; i++)
            {
                if(!en_spellinfo->EffectApplyAuraName[i])
                    break;
                int32 points = en_spellinfo->EffectBasePoints[i]+1;
                int32 misc = en_spellinfo->EffectMiscValue[i];
                switch(en_spellinfo->EffectApplyAuraName[i])
                {
                    case SPELL_AURA_MOD_CREATURE_ATTACK_POWER:
                        switch(misc)
                        {
                            case (0x0001 << (CREATURE_TYPE_BEAST-1)):
                            case (0x0001 << (CREATURE_TYPE_DEMON-1)):
                            case (0x0001 << (CREATURE_TYPE_UNDEAD-1)):
                                mod += pow((float)(points * ITEM_STAT_DBU_ATTACK_POWER_MOD), 1.50f); break;
                        }
                        break;
                    case SPELL_AURA_MOD_DAMAGE_DONE_CREATURE:
                        switch(misc)
                        {
                            case (0x0001 << (CREATURE_TYPE_BEAST-1)):
                            case (0x0001 << (CREATURE_TYPE_DEMON-1)):
                            case (0x0001 << (CREATURE_TYPE_UNDEAD-1)):
                                mod += pow((float)(points * ITEM_STAT_DBU_SPELL_DAMAGE_MOD), 1.50f); break;
                        }
                        break;
                    case SPELL_AURA_MOD_RANGED_ATTACK_POWER:
                        mod += pow((float)(points * ITEM_STAT_RANGED_ATTACK_POWER_MOD), 1.50f); break;
                    case SPELL_AURA_MOD_HEALING_DONE:
                        mod += pow((float)(points * ITEM_STAT_HEALING_MOD), 1.50f); break;
                    case SPELL_AURA_MOD_ATTACK_POWER:
                        mod += pow((float)(points * ITEM_STAT_ATTACK_POWER_MOD), 1.50f); break;
                    case SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT:
                        if(inv_type == INVTYPE_2HWEAPON || inv_type == INVTYPE_HOLDABLE || inv_type == INVTYPE_WEAPONOFFHAND || weapon_subclass == ITEM_SUBCLASS_WEAPON_WAND)
                            return 0;

                        if(inv_type == INVTYPE_SHIELD)
                            mod += pow((float)(points * ITEM_STAT_BLOCK_SHIELD_MOD), 1.50f);
                        else
                            mod += pow((float)(points * ITEM_STAT_BLOCK_MOD), 1.50f);
                        break;
                    case SPELL_AURA_MOD_DAMAGE_DONE:
                        if(misc == 126)
                        {
                            mod += pow((float)(points * ITEM_STAT_ALL_SPELL_DAMAGE_MOD), 1.50f);
                        }
                        else
                        {
                            if(misc & (0x0001 << SPELL_SCHOOL_HOLY))
                            {
                                if(itemProto->Class == ITEM_CLASS_WEAPON)
                                    if(itemProto->SubClass == ITEM_SUBCLASS_WEAPON_CROSSBOW || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_BOW || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_GUN)
                                        return 0;
                                mod += pow((float)(points * ITEM_STAT_SCHOOL_SPELL_DAMAGE_MOD), 1.50f);
                            }
                            if(misc & (0x0001 << SPELL_SCHOOL_FIRE) && !(itemProto->Class == ITEM_CLASS_ARMOR && itemProto->SubClass == ITEM_SUBCLASS_ARMOR_PLATE)) mod += pow((float)(points * ITEM_STAT_SCHOOL_SPELL_DAMAGE_MOD), 1.50f);
                            if(misc & (0x0001 << SPELL_SCHOOL_NATURE) && !(itemProto->Class == ITEM_CLASS_ARMOR && itemProto->SubClass == ITEM_SUBCLASS_ARMOR_PLATE)) mod += pow((float)(points * ITEM_STAT_SCHOOL_SPELL_DAMAGE_MOD), 1.50f);
                            if(misc & (0x0001 << SPELL_SCHOOL_FROST) && !(itemProto->Class == ITEM_CLASS_ARMOR && itemProto->SubClass == ITEM_SUBCLASS_ARMOR_PLATE)) mod += pow((float)(points * ITEM_STAT_SCHOOL_SPELL_DAMAGE_MOD), 1.50f);
                            if(misc & (0x0001 << SPELL_SCHOOL_SHADOW) && !(itemProto->Class == ITEM_CLASS_ARMOR && itemProto->SubClass == ITEM_SUBCLASS_ARMOR_PLATE))
                            {
                                if(itemProto->Class == ITEM_CLASS_WEAPON)
                                    if(itemProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD2 || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE2 || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_CROSSBOW || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_BOW || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_GUN)
                                        return 0;
                                if(inv_type == INVTYPE_SHIELD)
                                    return 0;
                                mod += pow((float)(points * ITEM_STAT_SCHOOL_SPELL_DAMAGE_MOD), 1.50f);
                            }
                            if(misc & (0x0001 << SPELL_SCHOOL_ARCANE) && !(itemProto->Class == ITEM_CLASS_ARMOR && itemProto->SubClass == ITEM_SUBCLASS_ARMOR_PLATE))
                            {
                                if(itemProto->Class == ITEM_CLASS_WEAPON)
                                    if(itemProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD2 || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE2 || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_CROSSBOW || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_BOW || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_GUN)
                                        return 0;
                                if(inv_type == INVTYPE_SHIELD)
                                    return 0;
                                mod += pow((float)(points * ITEM_STAT_SCHOOL_SPELL_DAMAGE_MOD), 1.50f);
                            }
                        }
                        break;
                    case SPELL_AURA_MOD_RESISTANCE:
                        if(misc == 126)
                        {
                            if(inv_type == INVTYPE_FINGER)
                                mod += pow((float)(points * ITEM_STAT_ALL_SPELL_RESIST_RING_MOD), 1.50f);
                            else
                                mod += pow((float)(points * ITEM_STAT_ALL_SPELL_RESIST_MOD), 1.50f);
                        }
                        else
                        {
                            if(inv_type == INVTYPE_FINGER)
                                tempmod = ITEM_STAT_SCHOOL_RESIST_RING_MOD;
                            else
                                tempmod = ITEM_STAT_SCHOOL_RESIST_MOD;
                            if(misc & (0x0001 << SPELL_SCHOOL_FIRE))   mod += pow((float)(points * tempmod), 1.50f);
                            if(misc & (0x0001 << SPELL_SCHOOL_NATURE)) mod += pow((float)(points * tempmod), 1.50f);
                            if(misc & (0x0001 << SPELL_SCHOOL_FROST))  mod += pow((float)(points * tempmod), 1.50f);
                            if(misc & (0x0001 << SPELL_SCHOOL_SHADOW)) mod += pow((float)(points * tempmod), 1.50f);
                            if(misc & (0x0001 << SPELL_SCHOOL_ARCANE)) mod += pow((float)(points * tempmod), 1.50f);
                        }
                        break;
                    case SPELL_AURA_MOD_STAT:
                        mod += pow((float)(points * ITEM_STAT_ATTRIBUTE_MOD), 1.50f); break;
                    case SPELL_AURA_MOD_SKILL:
                        switch(misc)
                        {
                            case SKILL_DEFENSE:
                                if(inv_type == INVTYPE_SHIELD)
                                    mod += pow((float)(points * ITEM_STAT_DEFENSE_SHIELD_MOD), 1.50f);
                                else
                                    mod += pow((float)(points * ITEM_STAT_DEFENSE_MOD), 1.50f);
                                break;
                            case SKILL_DAGGERS:
                                if(weapon_subclass != -1 && inv_type != INVTYPE_WEAPON && inv_type != INVTYPE_WEAPONOFFHAND && inv_type != INVTYPE_WEAPONMAINHAND && inv_type != INVTYPE_RANGED && inv_type != INVTYPE_RANGEDRIGHT) return 0;
                                mod += pow((float)(points * ITEM_STAT_DAGGER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_SWORDS:
                                if(weapon_subclass != -1 && inv_type != INVTYPE_WEAPON && inv_type != INVTYPE_WEAPONOFFHAND && inv_type != INVTYPE_WEAPONMAINHAND && inv_type != INVTYPE_RANGED && inv_type != INVTYPE_RANGEDRIGHT) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_AXES:
                                if(weapon_subclass != -1 && inv_type != INVTYPE_WEAPON && inv_type != INVTYPE_WEAPONOFFHAND && inv_type != INVTYPE_WEAPONMAINHAND && inv_type != INVTYPE_RANGED && inv_type != INVTYPE_RANGEDRIGHT || weapon_subclass == ITEM_SUBCLASS_WEAPON_WAND) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_BOWS:
                                if(weapon_subclass != -1 && weapon_subclass != ITEM_SUBCLASS_WEAPON_BOW && inv_type != INVTYPE_WEAPON && inv_type != INVTYPE_WEAPONMAINHAND && inv_type != INVTYPE_2HWEAPON) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_GUNS:
                                if(weapon_subclass != -1 && weapon_subclass != ITEM_SUBCLASS_WEAPON_GUN && inv_type != INVTYPE_WEAPON && inv_type != INVTYPE_WEAPONMAINHAND && inv_type != INVTYPE_2HWEAPON) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_MACES:
                                if(weapon_subclass != -1 && inv_type != INVTYPE_WEAPON && inv_type != INVTYPE_WEAPONOFFHAND && inv_type != INVTYPE_WEAPONMAINHAND && inv_type != INVTYPE_RANGED && inv_type != INVTYPE_RANGEDRIGHT) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_2H_SWORDS:
                                if((weapon_subclass != -1 || inv_type == INVTYPE_SHIELD || inv_type == INVTYPE_HOLDABLE) && weapon_subclass != ITEM_SUBCLASS_WEAPON_SWORD2 && inv_type != INVTYPE_RANGED && inv_type != INVTYPE_RANGEDRIGHT || weapon_subclass == ITEM_SUBCLASS_WEAPON_WAND) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_STAVES:
                                if((weapon_subclass != -1 || inv_type == INVTYPE_SHIELD || inv_type == INVTYPE_HOLDABLE) && weapon_subclass != ITEM_SUBCLASS_WEAPON_STAFF && inv_type != INVTYPE_RANGED && inv_type != INVTYPE_RANGEDRIGHT) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_2H_MACES:
                                if((weapon_subclass != -1 || inv_type == INVTYPE_SHIELD || inv_type == INVTYPE_HOLDABLE) && weapon_subclass != ITEM_SUBCLASS_WEAPON_MACE2 && inv_type != INVTYPE_RANGED && inv_type != INVTYPE_RANGEDRIGHT || weapon_subclass == ITEM_SUBCLASS_WEAPON_WAND) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_2H_AXES:
                                if((weapon_subclass != -1 || inv_type == INVTYPE_SHIELD || inv_type == INVTYPE_HOLDABLE) && weapon_subclass != ITEM_SUBCLASS_WEAPON_AXE2 && inv_type != INVTYPE_RANGED && inv_type != INVTYPE_RANGEDRIGHT || weapon_subclass == ITEM_SUBCLASS_WEAPON_WAND) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_CROSSBOWS:
                                if(weapon_subclass != -1 && weapon_subclass != ITEM_SUBCLASS_WEAPON_CROSSBOW && inv_type != INVTYPE_WEAPON && inv_type != INVTYPE_WEAPONMAINHAND && inv_type != INVTYPE_2HWEAPON) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_SPEARS:
                                if((weapon_subclass != -1 || inv_type == INVTYPE_SHIELD || inv_type == INVTYPE_HOLDABLE) && weapon_subclass != ITEM_SUBCLASS_WEAPON_SPEAR && inv_type != INVTYPE_RANGED && inv_type != INVTYPE_RANGEDRIGHT || weapon_subclass == ITEM_SUBCLASS_WEAPON_WAND) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_WANDS:
                                if((weapon_subclass != -1 || inv_type == INVTYPE_SHIELD) && weapon_subclass != ITEM_SUBCLASS_WEAPON_WAND && inv_type != INVTYPE_WEAPON && inv_type != INVTYPE_WEAPONMAINHAND && inv_type != INVTYPE_2HWEAPON) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                            case SKILL_POLEARMS:
                                if((weapon_subclass != -1 || inv_type == INVTYPE_SHIELD || inv_type == INVTYPE_HOLDABLE) && weapon_subclass != ITEM_SUBCLASS_WEAPON_POLEARM && inv_type != INVTYPE_RANGED && inv_type != INVTYPE_RANGEDRIGHT || weapon_subclass == ITEM_SUBCLASS_WEAPON_WAND) return 0;
                                mod += pow((float)(points * ITEM_STAT_OTHER_WEAPON_SKILL_MOD), 1.50f); break;
                        }
                        break;
                    case SPELL_AURA_MOD_POWER_REGEN:
                        mod += pow((float)(points * ITEM_STAT_REGEN_IN_5_SEC_MOD), 1.50f); break;
                    case SPELL_AURA_MOD_REGEN:
                    case SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT:
                        if(inv_type == INVTYPE_FINGER)
                            mod += pow((float)(points * ITEM_STAT_HP_REGEN_IN_5_SEC_RING_MOD), 1.50f);
                        else if(inv_type == INVTYPE_NECK)
                            mod += pow((float)(points * ITEM_STAT_HP_REGEN_IN_5_SEC_NECK_MOD), 1.50f);
                        else
                            mod += pow((float)(points * ITEM_STAT_REGEN_IN_5_SEC_MOD), 1.50f);
                        break;
                    case SPELL_AURA_PROC_TRIGGER_DAMAGE:
                        mod += pow((float)(points * ITEM_STAT_MAGIC_PENETRATION_MOD), 1.50f); break;
                    case SPELL_AURA_DAMAGE_SHIELD:
                        if(inv_type == INVTYPE_2HWEAPON || inv_type == INVTYPE_HOLDABLE || inv_type == INVTYPE_WEAPONOFFHAND || weapon_subclass == ITEM_SUBCLASS_WEAPON_WAND) return 0;
                        mod += pow((float)(points * ITEM_STAT_DAMAGE_SHIELD_MOD), 1.50f); break;
                    case SPELL_AURA_MOD_BLOCK_PERCENT:
                        if(inv_type == INVTYPE_2HWEAPON || inv_type == INVTYPE_HOLDABLE || inv_type == INVTYPE_WEAPONOFFHAND || weapon_subclass == ITEM_SUBCLASS_WEAPON_WAND) return 0;
                        mod += pow((float)(points * ITEM_STAT_BLOCK_PCT_MOD), 1.50f); break;
                    case SPELL_AURA_MOD_HIT_CHANCE:
                        mod += pow((float)(points * ITEM_STAT_HIT_PCT_MOD), 1.50f); break;
                    case SPELL_AURA_MOD_SPELL_HIT_CHANCE:
                        mod += pow((float)(points * ITEM_STAT_SPELL_HIT_PCT_MOD), 1.50f); break;
                    case SPELL_AURA_MOD_DODGE_PERCENT:
                        mod += pow((float)(points * ITEM_STAT_DODGE_PCT_MOD), 1.50f); break;
                    case SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL:
                        mod += pow((float)(points * ITEM_STAT_SPELL_CRIT_PCT_MOD), 1.50f); break;
                    case SPELL_AURA_MOD_CRIT_PERCENT:
                        mod += pow((float)(points * ITEM_STAT_CRIT_PCT_MOD), 1.50f); break;
                    case SPELL_AURA_MOD_PARRY_PERCENT:
                        mod += pow((float)(points * ITEM_STAT_PARRY_PCT_MOD), 1.50f); break;
                }
            }
        }
    }
    return mod;
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
                case ITEM_SUBCLASS_MINING_CONTAINER:
                    if(pProto->BagFamily != BAG_FAMILY_MINING_SUPP)
                        return false;
                    return true;
                case ITEM_SUBCLASS_ENGINEERING_CONTAINER:
                    if(pProto->BagFamily != BAG_FAMILY_ENGINEERING_SUPP)
                        return false;
                    return true;
                case ITEM_SUBCLASS_GEM_CONTAINER:
                    if(pProto->BagFamily != BAG_FAMILY_GEMS)
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

void Item::SetEnchantment(EnchantmentSlot slot, uint32 id, uint32 duration, uint32 charges)
{
    SetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_ID_OFFSET,id);
    SetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_DURATION_OFFSET,duration);
    SetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_CHARGES_OFFSET,charges);
    SetState(ITEM_CHANGED);
}

void Item::SetEnchantmentDuration(EnchantmentSlot slot, uint32 duration)
{
    SetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_DURATION_OFFSET,duration);
    SetState(ITEM_CHANGED);
}
void Item::SetEnchantmentCharges(EnchantmentSlot slot, uint32 charges)
{
    SetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_CHARGES_OFFSET,charges);
    SetState(ITEM_CHANGED);
}

void Item::ClearEnchantment(EnchantmentSlot slot)
{
    for(int x=0;x<3;x++)
        SetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+x,0);
    SetState(ITEM_CHANGED);
}

bool Item::GemsFitSockets() const
{
    bool fits = true;
    for(uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
    {
        uint8 SocketColor = GetProto()->Socket[enchant_slot-SOCK_ENCHANTMENT_SLOT].Color;

        uint32 enchant_id = GetEnchantmentId(EnchantmentSlot(enchant_slot));
        if(!enchant_id)
        {
            if(SocketColor) fits &= false;
            continue;
        }

        SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if(!enchantEntry)
        {
            if(SocketColor) fits &= false;
            continue;
        }

        uint8 GemColor = 0;

        uint32 gemid = enchantEntry->GemID;
        if(gemid)
        {
            ItemPrototype const* gemProto = sItemStorage.LookupEntry<ItemPrototype>(gemid);
            if(gemProto)
            {
                GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemProto->GemProperties);
                if(gemProperty)
                    GemColor = gemProperty->color;
            }
        }

        fits &= (GemColor & SocketColor) ? true : false;
    }
    return fits;
}

uint8 Item::GetGemCountWithID(uint32 GemID) const
{
    uint8 count = 0;
    for(uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
    {
        uint32 enchant_id = GetEnchantmentId(EnchantmentSlot(enchant_slot));
        if(!enchant_id)
            continue;

        SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if(!enchantEntry)
            continue;

        if(GemID == enchantEntry->GemID)
            count++;
    }
    return count;
}
