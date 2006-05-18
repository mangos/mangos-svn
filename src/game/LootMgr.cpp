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

#include <stdlib.h>
#include <functional>
// #include <ext/functional>

#include "LootMgr.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "ProgressBar.hpp"
#include "Policies/SingletonImp.h"

using std::remove_copy_if;
using std::ptr_fun;

LootStore CreatureLoot;

void UnloadLoot()
{
    CreatureLoot.clear();
}

void LoadCreaturesLootTables()
{
    uint32 curId = 0;
    LootStore::iterator tab;
    uint32 itemid, displayid, entry_id;
    uint32 count = 0;
    float chance;

    QueryResult *result = sDatabase.Query("SELECT * FROM `loot_template`;");

    if (result)
    {
        barGoLink bar(result->GetRowCount());
        do
        {
            Field *fields = result->Fetch();
            bar.step();

            entry_id = fields[0].GetUInt32();
            itemid = fields[1].GetUInt32();;
            chance = fields[2].GetFloat();

            ItemPrototype *proto = objmgr.GetItemPrototype(itemid);

            displayid = (proto != NULL) ? proto->DisplayInfoID : 0;

            CreatureLoot[entry_id].push_back(
                LootItem(itemid, displayid, chance));

            count++;
        } while (result->NextRow());

        delete result;

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u loot definitions", count );
    }
}

void FillLoot(Loot *loot, uint32 loot_id)
{
    LootStore::iterator tab;

    loot->items.clear();
    loot->gold = 0;

    if ((tab = CreatureLoot.find(loot_id)) == CreatureLoot.end())
        return;

    vector <LootItem>::iterator new_end;
    loot->items.resize(tab->second.size());
    // fill loot with items that have a chance
    new_end = remove_copy_if(tab->second.begin(), tab->second.end(), loot->items.begin(),
        LootItem::not_chance_for);
    loot->items.erase(new_end, loot->items.end());
}

void FillSkinLoot(Loot *Skinloot,uint32 itemid)
{
    //Skinloot->items.clear();
    ItemPrototype *proto = objmgr.GetItemPrototype(itemid);
    uint32 displayid = (proto != NULL) ? proto->DisplayInfoID : 0;
    Skinloot->items.push_back(LootItem(itemid,displayid,99));
}

void ChangeLoot(Loot * loot,uint32 loot_id,uint32 itemid, float chance)
{
    LootStore::iterator tab =CreatureLoot.find(loot_id);
    if( CreatureLoot.end()==tab)return;

    for(list<LootItem>::iterator iter = tab->second.begin(); iter!=tab->second.end();iter++)
    {
        if((*iter).itemid==itemid && (rand()%100000 < chance*1000))
        {
            LootItem itm;
            itm.itemid = itemid;
            itm.displayid = (*iter).displayid;
            itm.is_looted =false;
            itm.chance=chance;
            loot->items.push_back(itm);
        }
    }
}
