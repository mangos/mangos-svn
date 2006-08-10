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
#include "ProgressBar.h"
#include "Policies/SingletonImp.h"

using std::remove_copy_if;
using std::ptr_fun;

LootStore LootTemplates_Creature;
LootStore LootTemplates_Fishing;
LootStore LootTemplates_Gameobject;
LootStore LootTemplates_Pickpocketing;

void UnloadLoot()
{
    LootTemplates_Creature.clear();
    LootTemplates_Fishing.clear();
    LootTemplates_Gameobject.clear();
    LootTemplates_Pickpocketing.clear();
}

void LoadLootTable(LootStore& lootstore,char const* tablename)
{
    LootStore::iterator tab;
    uint32 item, displayid, entry;
    uint32 count = 0;
    float chance;
    float questchance;

    sLog.outString( "%s :", tablename);

    QueryResult *result = sDatabase.PQuery("SELECT `entry`, `item`, `chance`, `questchance` FROM `%s`",tablename);

    if (result)
    {
        barGoLink bar(result->GetRowCount());

        std::ostringstream ssNonLootableItems;

        do
        {
            Field *fields = result->Fetch();
            bar.step();

            entry = fields[0].GetUInt32();
            item = fields[1].GetUInt32();;
            chance = fields[2].GetFloat();
            questchance = fields[3].GetFloat();

            ItemPrototype const *proto = objmgr.GetItemPrototype(item);

            displayid = (proto != NULL) ? proto->DisplayInfoID : 0;

            if( chance < 0.000001 && questchance < 0.000001 )
                ssNonLootableItems << "loot entry = " << entry << " item = " << item << "\n";

            lootstore[entry].push_back( LootItem(item, displayid, chance, questchance) );

            count++;
        } while (result->NextRow());

        delete result;

        sLog.outString( "\n>> Loaded %u loot definitions", count );
        if(ssNonLootableItems.str().size() > 0)
            sLog.outError("\nSome items can't be succesfully looted: have in chance and questchance fields value < 0.000001 in `%s` DB table . List:\n%s",tablename,ssNonLootableItems.str().c_str());
    }
    else
        sLog.outError("\n>> Loaded 0 loot definitions. DB table `%s` is empty.",tablename);
}

void LoadLootTables()
{
    LoadLootTable(LootTemplates_Creature,     "creature_loot_template");
    LoadLootTable(LootTemplates_Fishing,      "fishing_loot_template");
    LoadLootTable(LootTemplates_Gameobject,   "gameobject_loot_template");
    LoadLootTable(LootTemplates_Pickpocketing,"pickpocketing_loot_template");
}

// Result: true  - have chance for non quest items or active quest items (loot can be empty or non empty in this case)
//         false - only quest loot for non active quests (loot empty)
bool LootItem::lootable(LootItem &itm, Player* player)
{
    if(player && itm.questchance > 0 && player->HaveQuestForItem(itm.itemid))
        return true;
    else
        return itm.chance > 0;
};

struct NotChanceFor
{
    Player* m_player;

    explicit NotChanceFor(Player* _player) : m_player(_player) {}

    bool operator() ( LootItem &itm )
    {
        if(m_player && itm.questchance > 0 && m_player->HaveQuestForItem(itm.itemid))
            return itm.questchance <= rand_chance();
        else if(itm.chance > 0)
            return itm.chance <= rand_chance();
        else
            return true;
    }
};

void FillLoot(Player* player, Loot *loot, uint32 loot_id, LootStore& store)
{
    loot->items.clear();
    loot->gold = 0;

    LootStore::iterator tab = store.find(loot_id);

    if (tab == store.end())
    {
        sLog.outError("Loot id #%u used in `creature_template` or `gameobject` or fishing but it doesn't have records in appropriate loot_template_* table.",loot_id);
        return;
    }

    vector <LootItem>::iterator new_end;
    loot->items.resize(tab->second.size());

    // fill loot with items that have a chance
    NotChanceFor not_chance_for(player);

    new_end = remove_copy_if(tab->second.begin(), tab->second.end(), loot->items.begin(),not_chance_for);
    loot->items.erase(new_end, loot->items.end());
}

void FillSkinLoot(Loot *Skinloot,uint32 itemid)
{
    //Skinloot->items.clear();
    ItemPrototype const *proto = objmgr.GetItemPrototype(itemid);
    uint32 displayid = (proto != NULL) ? proto->DisplayInfoID : 0;
    Skinloot->items.push_back(LootItem(itemid,displayid,99,0));
}
