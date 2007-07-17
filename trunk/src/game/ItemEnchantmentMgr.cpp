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

#include <stdlib.h>
#include <functional>
#include "ItemEnchantmentMgr.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "ProgressBar.h"
#include <list>
#include <vector>
#include "Util.h"

struct EnchStoreItem
{
    uint32  ench;
    float   chance;

    EnchStoreItem()
        : ench(0), chance(0) {}

    EnchStoreItem(uint32 _ench, float _chance)
        : ench(_ench), chance(_chance) {}
};

typedef std::vector<EnchStoreItem> EnchStoreList;
typedef HM_NAMESPACE::hash_map<uint32, EnchStoreList> EnchantmentStore;

static EnchantmentStore RandomItemEnch;

void LoadRandomEnchantmentsTable()
{
    EnchantmentStore::iterator tab;
    uint32 entry, ench;
    float chance;
    uint32 count = 0;

    QueryResult *result = sDatabase.Query("SELECT `entry`, `ench`, `chance` FROM `item_enchantment_template`");

    if (result)
    {
        barGoLink bar(result->GetRowCount());

        do
        {
            Field *fields = result->Fetch();
            bar.step();

            entry = fields[0].GetUInt32();
            ench = fields[1].GetUInt32();
            chance = fields[2].GetFloat();

            if (chance > 0.000001 && chance <= 100)
                RandomItemEnch[entry].push_back( EnchStoreItem(ench, chance) );

            count++;
        } while (result->NextRow());

        delete result;

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u Item Enchantment definitions", count );
    }
    else
    {
        sLog.outString( "" );
        sLog.outErrorDb( ">> Loaded 0 Item Enchantment definitions. DB table `item_enchantment_template` is empty.");
    }
}

uint32 GetItemEnchantMod(uint32 entry)
{
    if (!entry) return 0;

    EnchantmentStore::iterator tab = RandomItemEnch.find(entry);

    if (tab == RandomItemEnch.end())
    {
        sLog.outErrorDb("Item RandomProperty / RandomSuffix id #%u used in `item_template` but it doesn't have records in `item_enchantment_template` table.",entry);
        return 0;
    }

    double dRoll = rand_chance();
    float fCount = 0;

    for(EnchStoreList::iterator ench_iter = tab->second.begin(); ench_iter != tab->second.end(); ++ench_iter)
    {
        fCount += ench_iter->chance;

        if (fCount > dRoll) return ench_iter->ench;
    }

    //we could get here only if sum of all enchantment chances is lower than 100%
    dRoll =  (irand(0, (int)floor(fCount * 100) + 1)) / 100;
    fCount = 0;

    for(EnchStoreList::iterator ench_iter = tab->second.begin(); ench_iter != tab->second.end(); ++ench_iter)
    {
        fCount += ench_iter->chance;

        if (fCount > dRoll) return ench_iter->ench;
    }

    return 0;
}

uint32 GenerateEnchSuffixFactor(uint32 item_id)
{
    float suffixFactor = 0;

    ItemPrototype const *itemProto = sItemStorage.LookupEntry<ItemPrototype>(item_id);

    if(!itemProto)
        return 0;

    if(itemProto->RandomSuffix)
    {
        switch(itemProto->InventoryType)
        {
            case INVTYPE_HEAD:
                { suffixFactor = ITEM_SUFFIXFACTOR_HEAD_MOD; }  break;
            case INVTYPE_NECK:
                { suffixFactor = ITEM_SUFFIXFACTOR_NECK_MOD; }  break;
            case INVTYPE_SHOULDERS:
                { suffixFactor = ITEM_SUFFIXFACTOR_SHOULDERS_MOD; }  break;
            case INVTYPE_CHEST:
            case INVTYPE_ROBE:
            case INVTYPE_BODY:
                { suffixFactor = ITEM_SUFFIXFACTOR_CHEST_MOD; }  break;
            case INVTYPE_WAIST:
                { suffixFactor = ITEM_SUFFIXFACTOR_WAIST_MOD; }  break;
            case INVTYPE_LEGS:
                { suffixFactor = ITEM_SUFFIXFACTOR_LEGS_MOD; }  break;
            case INVTYPE_FEET:
                { suffixFactor = ITEM_SUFFIXFACTOR_FEET_MOD; }  break;
            case INVTYPE_WRISTS:
                { suffixFactor = ITEM_SUFFIXFACTOR_WRISTS_MOD; }  break;
            case INVTYPE_HANDS:
                { suffixFactor = ITEM_SUFFIXFACTOR_HANDS_MOD; }  break;
            case INVTYPE_FINGER:
                { suffixFactor = ITEM_SUFFIXFACTOR_FINGER_MOD; }  break;
            case INVTYPE_SHIELD:
                { suffixFactor = ITEM_SUFFIXFACTOR_SHIELD_MOD; }  break;
            case INVTYPE_RANGED:
            case INVTYPE_RANGEDRIGHT:
                { suffixFactor = ITEM_SUFFIXFACTOR_RANGED_MOD; }  break;
            case INVTYPE_CLOAK:
                { suffixFactor = ITEM_SUFFIXFACTOR_BACK_MOD; }  break;
            case INVTYPE_2HWEAPON:
                { suffixFactor = ITEM_SUFFIXFACTOR_2HAND_MOD; }  break;
            case INVTYPE_WEAPONMAINHAND:
                { suffixFactor = ITEM_SUFFIXFACTOR_MAIN_HAND_MOD; }  break;
            case INVTYPE_WEAPONOFFHAND:
            case INVTYPE_HOLDABLE:
                { suffixFactor = ITEM_SUFFIXFACTOR_OFF_HAND_MOD; }  break;
            case INVTYPE_THROWN:
                { suffixFactor = ITEM_SUFFIXFACTOR_THROWN_MOD; }  break;
            case INVTYPE_WEAPON:
                { suffixFactor = ITEM_SUFFIXFACTOR_ONE_HAND_MOD; }  break;
            default:  return 0;
        }

        //apply rare/epic armor modifier
        if( (itemProto->Class == ITEM_CLASS_ARMOR) && (itemProto->Quality > ITEM_QUALITY_UNCOMMON))
        {
            suffixFactor *= ITEM_SUFFIXFACTOR_RARE_MOD;
        }
    }

    return uint32(floor((suffixFactor*itemProto->ItemLevel) + 0.5 ));
}
