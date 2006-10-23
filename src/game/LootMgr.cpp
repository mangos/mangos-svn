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

LootStore LootTemplates_Creature;
LootStore LootTemplates_Fishing;
LootStore LootTemplates_Gameobject;
LootStore LootTemplates_Pickpocketing;
LootStore LootTemplates_Skinning;
LootSkinnigAlternative sLootSkinnigAlternative;

void UnloadLoot()
{
    LootTemplates_Creature.clear();
    LootTemplates_Fishing.clear();
    LootTemplates_Gameobject.clear();
    LootTemplates_Pickpocketing.clear();
    LootTemplates_Skinning.clear();
    sLootSkinnigAlternative.clear();
}

void LoadLootTable(LootStore& lootstore,char const* tablename)
{
    LootStore::iterator tab;
    uint32 item, displayid, entry;
    uint32 maxcount = 0;
    float chance;
    float questchance;

    uint32 count = 0;

    sLog.outString( "%s :", tablename);

    QueryResult *result = sDatabase.PQuery("SELECT `entry`, `item`, `chance`, `questchance`, `maxcount` FROM `%s`",tablename);

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
            maxcount = fields[4].GetUInt32();

            ItemPrototype const *proto = objmgr.GetItemPrototype(item);

            displayid = (proto != NULL) ? proto->DisplayInfoID : 0;

            if( chance < 0.000001 && questchance < 0.000001 )
                ssNonLootableItems << "loot entry = " << entry << " item = " << item << " maxcount = " << maxcount << "\n";

            lootstore[entry].push_back( LootStoreItem(item, displayid, chance, questchance,maxcount) );

            count++;
        } while (result->NextRow());

        delete result;

        sLog.outString( ">> Loaded %u loot definitions", count );
        if(ssNonLootableItems.str().size() > 0)
            sLog.outError("Some items can't be succesfully looted: have in chance and questchance fields value < 0.000001 in `%s` DB table . List:\n%s",tablename,ssNonLootableItems.str().c_str());
    }
    else
        sLog.outError(">> Loaded 0 loot definitions. DB table `%s` is empty.",tablename);
}

void LoadSkinnigAlternativeTable()
{
    LootStore::iterator tab;
    uint32 item, item2, displayid2;
    uint32 count = 0;

    QueryResult *result = sDatabase.Query("SELECT `item`, `item2` FROM `skinning_loot_template_alternative`");

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            item  = fields[0].GetUInt32();;
            item2 = fields[1].GetUInt32();;

            if(!item2)
                continue;

            ItemPrototype const *proto2 = objmgr.GetItemPrototype(item2);

            displayid2 = (proto2 != NULL) ? proto2->DisplayInfoID : 0;

            sLootSkinnigAlternative[item] = LootSkinningAltItem(item2, displayid2);

            count++;
        } while (result->NextRow());

        delete result;
    }
}

void LoadLootTables()
{
    LoadLootTable(LootTemplates_Creature,     "creature_loot_template");
    LoadLootTable(LootTemplates_Fishing,      "fishing_loot_template");
    LoadLootTable(LootTemplates_Gameobject,   "gameobject_loot_template");
    LoadLootTable(LootTemplates_Pickpocketing,"pickpocketing_loot_template");
    LoadLootTable(LootTemplates_Skinning,     "skinning_loot_template");
    LoadSkinnigAlternativeTable();
}

struct NotChanceFor
{
    Player* m_player;

    explicit NotChanceFor(Player* _player) : m_player(_player) {}

    bool operator() ( LootStoreItem &itm )
    {
        if(m_player && itm.questchance > 0 && m_player->HaveQuestForItem(itm.itemid))
            return itm.questchance <= rand_chance();
        else if(itm.chance > 0)
        {
            if(itm.chance >= 100)
                return false;
            else
                return itm.chance * sWorld.getRate(RATE_DROP_ITEMS) <= rand_chance();
        }
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
        sLog.outError("Loot id #%u used in `creature_template` or `gameobject` or fishing but it doesn't have records in appropriate *_loot_template table.",loot_id);
        return;
    }

    loot->items.resize(tab->second.size());

    // fill loot with items that have a chance
    NotChanceFor not_chance_for(player);

    size_t pos = 0;
    for(LootStoreItemList::iterator item_iter = tab->second.begin(); item_iter != tab->second.end(); ++item_iter)
    {
        uint8 lcount = 0;
        for(uint8 i = 0; i < item_iter->maxcount; ++i)
            if(!not_chance_for(*item_iter)) ++lcount;

        if(lcount > 0)
            loot->items[pos++] = LootItem(*item_iter,lcount);
    }

    loot->items.erase(loot->items.begin()+pos, loot->items.end());
}

void Loot::NotifyItemRemoved(uint8 lootIndex)
{
    // notifiy all players that are looting this that the item was removed
    // convert the index to the slot the player sees
    for(std::set<Player*>::iterator i = PlayersLooting.begin(); i != PlayersLooting.end(); ++i)
        (*i)->SendNotifyLootItemRemoved(lootIndex);
}

void Loot::NotifyMoneyRemoved()
{
    // notifiy all players that are looting this that the money was removed
    for(std::set<Player*>::iterator i = PlayersLooting.begin(); i != PlayersLooting.end(); ++i)
        (*i)->SendNotifyLootMoneyRemoved();
}
