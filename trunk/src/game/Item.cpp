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
#include "ItemEnchantmentMgr.h"

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

    ItemSetEffect *eff=NULL;

    for(size_t x = 0; x < player->ItemSetEff.size(); x++)
        if(player->ItemSetEff[x] && player->ItemSetEff[x]->setid == setid)
    {
        eff = player->ItemSetEff[x];
        break;
    }

    if(!eff)
    {
        eff = new ItemSetEffect;
        memset(eff,0,sizeof(ItemSetEffect));
        eff->setid = setid;

        size_t x = 0;
        for(; x < player->ItemSetEff.size(); x++)
            if(!player->ItemSetEff[x])
                break;

        if(x < player->ItemSetEff.size())
            player->ItemSetEff[x]=eff;
        else
            player->ItemSetEff.push_back(eff);
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
                            SpellEntry const *spellInfo = sSpellStore.LookupEntry(set->spells[x]);
                            if(!spellInfo)
                            {
                                sLog.outError("WORLD: unknown spell id %u in items set %u effects", set->spells[x],setid);
                                break;
                            }

                            player->CastSpell(player,set->spells[x],true,item);
                            eff->spells[y] = spellInfo;
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

    ItemSetEffect *eff = NULL;
    size_t setindex = 0;
    for(;setindex < player->ItemSetEff.size(); setindex++)
        if(player->ItemSetEff[setindex] && player->ItemSetEff[setindex]->setid == setid)
    {
        eff = player->ItemSetEff[setindex];
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
        assert(eff == player->ItemSetEff[setindex]);
        delete eff;
        player->ItemSetEff[setindex] = NULL;
    }
}

Item::Item( )
{
    m_objectType |= TYPE_ITEM;
    m_objectTypeId = TYPEID_ITEM;
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_ALL);  // 2.1.2 - 0x18

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

    for(int i = 0; i < 5; ++i)
        SetSpellCharges(i,itemProto->Spells[i].SpellCharges);

    SetUInt32Value(ITEM_FIELD_FLAGS, itemProto->Flags);
    //SetUInt32Value(ITEM_FIELD_DURATION, itemProto->Delay); ITEM_FIELD_DURATION is time until item expires, not speed

    return true;
}

void Item::SaveToDB()
{
    uint32 guid = GetGUIDLow();
    switch (uState)
    {
        case ITEM_NEW:
        {
            uint32 Rows=0;

            // it's better than rebuilding indexes multiple times
            QueryResult *result = CharacterDatabase.PQuery("select count(*) as r from `item_instance` where `guid` = '%u'", guid);
            if(result)
            {
                Rows = result->Fetch()[0].GetUInt32();
                delete result;
            }

            // guess - instance exists ?
            if (!Rows)
            {
                // no - we must insert new rec
                std::ostringstream ss;
                ss << "INSERT INTO `item_instance` (`guid`,`owner_guid`,`data`) VALUES (" << guid << "," << GUID_LOPART(GetOwnerGUID()) << ",'";
                for(uint16 i = 0; i < m_valuesCount; i++ )
                    ss << GetUInt32Value(i) << " ";
                ss << "' )";

                CharacterDatabase.Execute( ss.str().c_str() );
            } else
            {
                std::ostringstream ss;
                ss << "UPDATE `item_instance` SET `data` = '";
                for(uint16 i = 0; i < m_valuesCount; i++ )
                    ss << GetUInt32Value(i) << " ";
                ss << "' WHERE `guid` = '" << guid << "'";
                CharacterDatabase.Execute( ss.str().c_str() );
            };
        } break;
        case ITEM_CHANGED:
        {
            std::ostringstream ss;
            ss << "UPDATE `item_instance` SET `data` = '";
            for(uint16 i = 0; i < m_valuesCount; i++ )
                ss << GetUInt32Value(i) << " ";
            ss << "', `owner_guid` = '" << GUID_LOPART(GetOwnerGUID()) << "' WHERE `guid` = '" << guid << "'";

            CharacterDatabase.Execute( ss.str().c_str() );

            if(HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAGS_WRAPPED))
                CharacterDatabase.PExecute("UPDATE `character_gifts` SET `guid` = '%u' WHERE `item_guid` = '%u'", GUID_LOPART(GetOwnerGUID()),GetGUIDLow());
        } break;
        case ITEM_REMOVED:
        {
            if (GetUInt32Value(ITEM_FIELD_ITEM_TEXT_ID) > 0 )
                CharacterDatabase.PExecute("DELETE FROM `item_text` WHERE `id` = '%u'", GetUInt32Value(ITEM_FIELD_ITEM_TEXT_ID));
            CharacterDatabase.PExecute("DELETE FROM `item_instance` WHERE `guid` = '%u'", guid);
            if(HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAGS_WRAPPED))
                CharacterDatabase.PExecute("DELETE FROM `character_gifts` WHERE `item_guid` = '%u'", GetGUIDLow());
            delete this;
            return;
        }
        case ITEM_UNCHANGED:
            break;
    }
    SetState(ITEM_UNCHANGED);
}

bool Item::LoadFromDB(uint32 guid, uint64 owner_guid, QueryResult *result)
{
    bool delete_result = false;
    if(!result)
    {
        result = CharacterDatabase.PQuery("SELECT `data` FROM `item_instance` WHERE `guid` = '%u'", guid);
        delete_result = true;
    }

    if (!result)
    {
        sLog.outError("ERROR: Item (GUID: %u owner: %u) not found in table `item_instance`, can't load. ",guid,GUID_LOPART(owner_guid));
        return false;
    }

    Field *fields = result->Fetch();

    _Create(guid, HIGHGUID_ITEM);

    if(!LoadValues(fields[0].GetString()))
    {
        sLog.outError("ERROR: Item #%d have broken data in `data` field. Can't be loaded.",guid);
        if (delete_result) delete result;
        return false;
    }

    if (delete_result) delete result;

    if(owner_guid != 0)
        SetOwnerGUID(owner_guid);

    return true;
}

void Item::DeleteFromDB()
{
    CharacterDatabase.PExecute("DELETE FROM `item_instance` WHERE `guid` = '%u'",GetGUIDLow());
}

void Item::DeleteFromInventoryDB()
{
    CharacterDatabase.PExecute("DELETE FROM `character_inventory` WHERE `item` = '%u'",GetGUIDLow());
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
    const static uint32 item_weapon_skills[MAX_ITEM_SUBCLASS_WEAPON] =
    {
        SKILL_AXES,     SKILL_2H_AXES,  SKILL_BOWS,          SKILL_GUNS,      SKILL_MACES,
        SKILL_2H_MACES, SKILL_POLEARMS, SKILL_SWORDS,        SKILL_2H_SWORDS, 0,
        SKILL_STAVES,   0,              0,                   SKILL_UNARMED,   0,
        SKILL_DAGGERS,  SKILL_THROWN,   SKILL_ASSASSINATION, SKILL_CROSSBOWS, SKILL_WANDS,
        SKILL_FISHING
    };

    const static uint32 item_armor_skills[MAX_ITEM_SUBCLASS_ARMOR] =
    {
        0,SKILL_CLOTH,SKILL_LEATHER,SKILL_MAIL,SKILL_PLATE_MAIL,0,SKILL_SHIELD,0,0,0
    };

    switch (GetProto()->Class)
    {
        case ITEM_CLASS_WEAPON:
            if( GetProto()->SubClass >= MAX_ITEM_SUBCLASS_WEAPON )
                return 0;
            else
                return item_weapon_skills[GetProto()->SubClass];

        case ITEM_CLASS_ARMOR:
            if( GetProto()->SubClass >= MAX_ITEM_SUBCLASS_ARMOR )
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
                case ITEM_SUBCLASS_WEAPON_AXE:     return  196;
                case ITEM_SUBCLASS_WEAPON_AXE2:    return  197;
                case ITEM_SUBCLASS_WEAPON_BOW:     return  264;
                case ITEM_SUBCLASS_WEAPON_GUN:     return  266;
                case ITEM_SUBCLASS_WEAPON_MACE:    return  198;
                case ITEM_SUBCLASS_WEAPON_MACE2:   return  199;
                case ITEM_SUBCLASS_WEAPON_POLEARM: return  200;
                case ITEM_SUBCLASS_WEAPON_SWORD:   return  201;
                case ITEM_SUBCLASS_WEAPON_SWORD2:  return  202;
                case ITEM_SUBCLASS_WEAPON_STAFF:   return  227;
                case ITEM_SUBCLASS_WEAPON_DAGGER:  return 1180;
                case ITEM_SUBCLASS_WEAPON_THROWN:  return 2567;
                case ITEM_SUBCLASS_WEAPON_SPEAR:   return 3386;
                case ITEM_SUBCLASS_WEAPON_CROSSBOW:return 5011;
                case ITEM_SUBCLASS_WEAPON_WAND:    return 5009;
                default: return 0;
            }
        case ITEM_CLASS_ARMOR:
            switch(GetProto()->SubClass)
            {
                case ITEM_SUBCLASS_ARMOR_CLOTH:    return 9078;
                case ITEM_SUBCLASS_ARMOR_LEATHER:  return 9077;
                case ITEM_SUBCLASS_ARMOR_MAIL:     return 8737;
                case ITEM_SUBCLASS_ARMOR_PLATE:    return  750;
                case ITEM_SUBCLASS_ARMOR_SHIELD:   return 9116;
                default: return 0;
            }
    }
    return 0;
}

int32 Item::GenerateItemRandomPropertyId(uint32 item_id)
{
    ItemPrototype const *itemProto = sItemStorage.LookupEntry<ItemPrototype>(item_id);

    if(!itemProto)
        return 0;

    // item must have one from this field values not null if it can have random enchantments
    if((!itemProto->RandomProperty) && (!itemProto->RandomSuffix))
        return 0;

    // item can have not null only one from field values
    if((itemProto->RandomProperty) && (itemProto->RandomSuffix))
    {
        sLog.outErrorDb("Item template %u have `RandomProperty`==%u and `RandomSuffix`==%u, but must have one from field =0",itemProto->ItemId,itemProto->RandomProperty,itemProto->RandomSuffix);
        return 0;
    }

    // RandomProperty case
    if(itemProto->RandomProperty)
    {
        uint32 randomPropId = GetItemEnchantMod(itemProto->RandomProperty);
        ItemRandomPropertiesEntry const *random_id = sItemRandomPropertiesStore.LookupEntry(randomPropId);
        if(!random_id)
        {
            sLog.outErrorDb("Enchantment id #%u used but it doesn't have records in 'ItemRandomProperties.dbc'",randomPropId);
            return 0;
        }

        return random_id->ID;
    }
    // RandomSuffix case
    else
    {
        uint32 randomPropId = GetItemEnchantMod(itemProto->RandomSuffix);
        ItemRandomSuffixEntry const *random_id = sItemRandomSuffixStore.LookupEntry(randomPropId);
        if(!random_id)
        {
            sLog.outErrorDb("Enchantment id #%u used but it doesn't have records in sItemRandomSuffixStore.",randomPropId);
            return 0;
        }

        return -int32(random_id->ID);
    }
    return 0;
}

void Item::SetItemRandomProperties(int32 randomPropId)
{
    if(!randomPropId)
        return;

    if(randomPropId > 0)
    {
        ItemRandomPropertiesEntry const *item_rand = sItemRandomPropertiesStore.LookupEntry(randomPropId);
        if(item_rand)
        {
            if(GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID) != item_rand->ID)
            {
                SetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID,item_rand->ID);
                SetState(ITEM_CHANGED);
            }
            for(uint32 i = PROP_ENCHANTMENT_SLOT_2; i < PROP_ENCHANTMENT_SLOT_2 + 3; ++i)
                SetEnchantment(EnchantmentSlot(i),item_rand->enchant_id[i - PROP_ENCHANTMENT_SLOT_2],0,0);
        }
    }
    else
    {
        ItemRandomSuffixEntry const *item_rand = sItemRandomSuffixStore.LookupEntry(-randomPropId);
        if(item_rand)
        {
            if( GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID)!=(uint32)(-int32(item_rand->ID)) ||
                !GetUInt32Value(ITEM_FIELD_SUFFIX_FACTOR))
            {
                SetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID,(uint32)(-int32(item_rand->ID)));
                SetUInt32Value(ITEM_FIELD_SUFFIX_FACTOR,GenerateEnchSuffixFactor(GetEntry()));
                SetState(ITEM_CHANGED);
            }

            for(uint32 i = PROP_ENCHANTMENT_SLOT_0; i < PROP_ENCHANTMENT_SLOT_0 + 3; ++i)
                SetEnchantment(EnchantmentSlot(i),item_rand->enchant_id[i - PROP_ENCHANTMENT_SLOT_0],0,0);
        }
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
    return m_container ? m_container->GetSlot() : uint8(INVENTORY_SLOT_BAG_0);
}

bool Item::IsEquipped() const
{
    return !IsInBag() && m_slot < EQUIPMENT_SLOT_END;
}

bool Item::CanBeTraded() const
{
    if(HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAGS_BINDED) || GetProto()->Class == ITEM_CLASS_QUEST)
        return false;
    if(IsBag() && !((Bag const*)this)->IsEmpty())
        return false;

    if(Player* owner = GetOwner())
    {
        if(owner->CanUnequipItem(uint16(GetBagSlot()) << 8 | GetSlot(),false) !=  EQUIP_ERR_OK )
            return false;
        if(owner->GetLootGUID()==GetGUID())
            return false;
    }

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
    // Better lost small time at check in comparison lost time at item save to DB.
    if( GetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_ID_OFFSET)==id &&
        GetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_DURATION_OFFSET)==duration &&
        GetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_CHARGES_OFFSET)==charges )
        return;

    SetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_ID_OFFSET,id);
    SetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_DURATION_OFFSET,duration);
    SetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_CHARGES_OFFSET,charges);
    SetState(ITEM_CHANGED);
}

void Item::SetEnchantmentDuration(EnchantmentSlot slot, uint32 duration)
{
    if(GetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_DURATION_OFFSET)==duration)
        return;

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
    if(!GetUInt32Value(ITEM_FIELD_ENCHANTMENT+slot*3+ENCHANTMENT_ID_OFFSET))
        return;

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
