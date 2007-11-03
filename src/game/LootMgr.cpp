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
// #include <ext/functional>

#include "LootMgr.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "ProgressBar.h"
#include "Policies/SingletonImp.h"
#include "ObjectAccessor.h"
#include "World.h"

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

using std::remove_copy_if;

LootStore LootTemplates_Creature;
LootStore LootTemplates_Fishing;
LootStore LootTemplates_Gameobject;
LootStore LootTemplates_Item;
LootStore LootTemplates_Pickpocketing;
LootStore LootTemplates_Skinning;
LootStore LootTemplates_Disenchant;
LootStore LootTemplates_Prospecting;

void UnloadLoot()
{
    LootTemplates_Creature.clear();
    LootTemplates_Fishing.clear();
    LootTemplates_Gameobject.clear();
    LootTemplates_Item.clear();
    LootTemplates_Pickpocketing.clear();
    LootTemplates_Skinning.clear();
    LootTemplates_Disenchant.clear();
    LootTemplates_Prospecting.clear();
}

void LoadLootTable(LootStore& lootstore,char const* tablename)
{
    LootStore::iterator tab;
    uint32 item, displayid, entry;
    uint32 mincount = 0;
    uint32 maxcount = 0;
    float chanceOrRef;
    int32 questchance;
    uint32 count = 0;
    int32 ffa_or_condition = 0;
    uint32 cond_value1 = 0;
    uint32 cond_value2 = 0;

    lootstore.clear();                                      // need for reload

    sLog.outString( "%s :", tablename);

    //                                                 0        1       2              3                     4           5           6                          7                   8
    QueryResult *result = WorldDatabase.PQuery("SELECT `entry`, `item`, `ChanceOrRef`, `QuestChanceOrGroup`, `mincount`, `maxcount`, `QuestFFAorLootCondition`, `condition_value1`, `condition_value2` FROM `%s`",tablename);

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
            chanceOrRef = fields[2].GetFloat();
            questchance = int32(fields[3].GetUInt32());
            mincount = fields[4].GetUInt32();
            maxcount = fields[5].GetUInt32();
            ffa_or_condition = int32(fields[6].GetUInt32());
            cond_value1 = fields[7].GetUInt32();
            cond_value2 = fields[8].GetUInt32();

            if( chanceOrRef >= 0 )                          // chance
            {
                ItemPrototype const *proto = objmgr.GetItemPrototype(item);

                if(!proto)
                {
                    ssNonLootableItems << "loot entry = " << entry << " item = " << item << " mincount = " << mincount << " maxcount = " << maxcount << " (not exist)\n";
                    continue;
                }

                displayid = proto->DisplayInfoID;

                // non-quest (maybe group) loot with low chance
                if( chanceOrRef < 0.000001f && questchance <= 0.0f )
                {
                    ssNonLootableItems << "loot entry = " << entry << " item = " << item << " mincount = " << mincount << " maxcount = " << maxcount << " (no chance)\n";
                    continue;
                }
            }
            // in case chanceOrRef < 0 item is loot ref slot index in fact that allow have more one refs in loot

            switch (abs(ffa_or_condition))
            {
                case CONDITION_AURA:
                {
                    // TODO: add some check for aura
                    break;
                }
                case CONDITION_ITEM:
                {
                    ItemPrototype const *proto = objmgr.GetItemPrototype(cond_value1);
                    if(!proto)
                    {
                        sLog.outErrorDb("Table: %s Dropped item (entry: %d) from creature (entry: %d) requires to have non existing item to be dropped (entry: %d)!", tablename, item, entry, cond_value1);
                        continue;
                    }
                    break;
                }
                case CONDITION_ITEM_EQUIPPED:
                {
                    ItemPrototype const *proto = objmgr.GetItemPrototype(cond_value1);
                    if(!proto)
                    {
                        sLog.outErrorDb("Table: %s Dropped item (entry: %d) from creature (entry: %d) requires non existing item equipped to be dropped! (entry: %d)", tablename, item, entry, cond_value1);
                        continue;
                    }
                    if (cond_value2)
                    {
                        sLog.outErrorDb("Table: %s Dropped item (entry: %d) from creature (entry: %d) requires item equipped, but item count is set (setting it to 0)", tablename, item, entry);
                        cond_value2 = 0;
                    }
                    break;
                }
                case CONDITION_ZONEID:
                {
                    // TODO: add some check for zone
                    break;
                }
                case CONDITION_REPUTATION_RANK:
                {
                    FactionEntry const* factionEntry = sFactionStore.LookupEntry(cond_value1);
                    if(!factionEntry)
                    {
                        sLog.outErrorDb("Table: %s Dropped item (entry: %d) from creature (entry: %d) has set non existing faction reputation requirement!", tablename, item, entry);
                        continue;
                    }
                    break;
                }
                case CONDITION_DUNGEON_DIFFICULTY:
                {
                    if (cond_value1 > 1)
                        sLog.outErrorDb("Table: %s Dropped item (entry: %d) from creature (entry: %d) has set non existing dungeon difficulty!", tablename, item, entry);
                    break;
                }
                case CONDITION_TEAM:
                {
                    if (cond_value1 != ALLIANCE && cond_value1 != HORDE)
                    {
                        sLog.outErrorDb("Table: %s Dropped item (entry: %d) from creature (entry: %d) has set team drop condition to nonexisting team!", tablename, item, entry);
                        continue;
                    }
                    break;
                }
            }

            lootstore[entry].push_back( LootStoreItem(item, displayid, chanceOrRef, questchance, ffa_or_condition, cond_value1, cond_value2, mincount, maxcount) );


            count++;
        } while (result->NextRow());

        delete result;

        sLog.outString();
        sLog.outString( ">> Loaded %u loot definitions", count );
        if(!ssNonLootableItems.str().empty())
            sLog.outErrorDb("Some items can't be succesfully looted: not exist or have in chance field value < 0.000001 with quest chance ==0 in `%s` DB table . List:\n%s",tablename,ssNonLootableItems.str().c_str());
    }
    else
    {
        sLog.outString();
        sLog.outErrorDb( ">> Loaded 0 loot definitions. DB table `%s` is empty.",tablename );
    }
}

void LoadLootTables()
{
    LoadLootTable(LootTemplates_Creature,     "creature_loot_template");
    LoadLootTable(LootTemplates_Disenchant,   "disenchant_loot_template");
    LoadLootTable(LootTemplates_Fishing,      "fishing_loot_template");
    LoadLootTable(LootTemplates_Gameobject,   "gameobject_loot_template");
    LoadLootTable(LootTemplates_Item,         "item_loot_template");
    LoadLootTable(LootTemplates_Pickpocketing,"pickpocketing_loot_template");
    LoadLootTable(LootTemplates_Skinning,     "skinning_loot_template");
    LoadLootTable(LootTemplates_Prospecting,  "prospecting_loot_template");
}

#define MaxLootGroups 8

struct HasChance
{
    LootStore* m_store;
    Player* owner;
    float RolledChance[MaxLootGroups];
    float CumulativeChance[MaxLootGroups];

    explicit HasChance(LootStore* _store, Player* _owner) : m_store(_store), owner(_owner)
    {
        for (int i=0; i < MaxLootGroups; i++)
            CumulativeChance[i] = 0.0f;
    }

    LootStoreItem* operator() ( LootStoreItem& itm )
    {
        // Quest loot handled separately
        if (itm.questChanceOrGroup > 0)
            return NULL;

        // Non-grouped loot
        if (itm.questChanceOrGroup == 0)
        {
            // reference non group loot ( itm.chanceOrRef < 0) handled separatly
            if ( itm.chanceOrRef > 0 && roll_chance_f(itm.chanceOrRef * sWorld.getRate(RATE_DROP_ITEMS)) )
            {
                //(QuestFFAorLootCondition < 0) -> item has aditional loot condition
                if( owner && itm.ffa_or_condition < 0 )
                {
                    switch (abs(itm.ffa_or_condition))
                    {
                    case CONDITION_AURA:
                    {
                        if(owner->HasAura(itm.cond_value1, itm.cond_value2))
                            return &itm;
                        else
                            return NULL;
                        break;
                    }
                    case CONDITION_ITEM:
                    {
                        if(owner->HasItemCount(itm.cond_value1,itm.cond_value2))
                            return &itm;
                        else
                            return NULL;
                        break;
                    }
                    case CONDITION_ITEM_EQUIPPED:
                    {
                        if(owner->HasItemEquipped(itm.cond_value1))
                            return &itm;
                        else
                            return NULL;
                        break;
                    }
                    case CONDITION_ZONEID:
                    {
                        if(owner->GetZoneId() == itm.cond_value1)
                            return &itm;
                        else
                            return NULL;
                        break;
                    }
                    case CONDITION_REPUTATION_RANK:
                    {
                        if(owner->GetBaseReputationRank(sFactionStore.LookupEntry(itm.cond_value1)) >= itm.cond_value2)
                            return &itm;
                        else
                            return NULL;
                        break;
                    }
                    case CONDITION_DUNGEON_DIFFICULTY:
                    {
                        if(owner->GetDungeonDifficulty() == itm.cond_value1)
                            return &itm;
                        else
                            return NULL;
                        break;
                    }
                    case CONDITION_TEAM:
                    {
                        if(owner->GetTeam() == itm.cond_value1)
                            return &itm;
                        else
                            return NULL;
                        break;
                    }
                    default:
                        return NULL;
                    }
                }
                else
                {
                    return &itm;
                }
             }
             return NULL;
        }

        // Grouped loot
        int32 GroupId = itm.GetGroupId();

        if (GroupId < 0 || GroupId >=  MaxLootGroups)
        {
            sLog.outErrorDb("HasChance: wrong loot group in DB (%i) for item %u", itm.questChanceOrGroup,itm.itemid);
            return NULL;
        }

        if (itm.chanceOrRef >= 0)
        {
            // Group of current loot - check for item chance in the group
            if (CumulativeChance[GroupId] == 0.0f)
                RolledChance[GroupId] = rand_chance();
            if (CumulativeChance[GroupId] >= RolledChance[GroupId])
                // An item from the group already accepted
                return NULL;

            CumulativeChance[GroupId] += itm.chanceOrRef;
            if (CumulativeChance[GroupId] >= RolledChance[GroupId])
                return &itm;
            return NULL;
        }

        // Reference to a group of another loot
        int LootId = -int(itm.chanceOrRef);
        float Chance = rand_chance();
        float CumulChance = 0.0f;

        LootStore::iterator tab = m_store->find(LootId);
        if(tab==m_store->end())
        {
            sLog.outErrorDb("HasChance: wrong loot reference in DB (%i) for item %u", itm.chanceOrRef,itm.itemid);
            return NULL;
        }

        for(LootStoreItemList::iterator item_iter = tab->second.begin(); item_iter != tab->second.end(); ++item_iter)
        {
            if ( item_iter->GetGroupId() == GroupId &&  item_iter->chanceOrRef > 0 )
            {
                CumulChance += item_iter->chanceOrRef;
                if ( CumulChance >= Chance )
                    return &*item_iter;
            }
        }
        return NULL;
    }
};

struct HasQuestChance
{
    // explicit HasQuestChanceFor() : {}
    inline bool operator() ( LootStoreItem &itm )   { return roll_chance_i(itm.questChanceOrGroup); }
};

void FillLoot(Loot* loot, uint32 loot_id, LootStore& store, Player* loot_owner)
{
    LootStore::iterator tab = store.find(loot_id);

    if (tab == store.end())
    {
        sLog.outErrorDb("Loot id #%u used in `creature_template` or `gameobject` or fishing but it doesn't have records in appropriate *_loot_template table.",loot_id);
        return;
    }

    loot->items.reserve(min(tab->second.size(),MAX_NR_LOOT_ITEMS));
    loot->quest_items.reserve(min(tab->second.size(),MAX_NR_QUEST_ITEMS));

    // fill loot with all normal and quest items that have a chance
    HasChance      hasChance(&store, loot_owner);
    HasQuestChance hasQuestChance;

    for(LootStoreItemList::iterator item_iter = tab->second.begin(); item_iter != tab->second.end(); ++item_iter)
    {
        // There are stats of count variations for 100% drop - so urand used
        if ( loot->quest_items.size() < MAX_NR_QUEST_ITEMS && hasQuestChance(*item_iter) )
            loot->quest_items.push_back(
                LootItem(*item_iter, urand(item_iter->mincount, item_iter->maxcount),
                GenerateEnchSuffixFactor(item_iter->itemid),
                Item::GenerateItemRandomPropertyId(item_iter->itemid))
                );
        else if ( loot->items.size() < MAX_NR_LOOT_ITEMS )
        {
            // non-group and non-quest loot reference
            if(item_iter->chanceOrRef < 0 && item_iter->questChanceOrGroup == 0)
            {
                // Reference to another loot
                int LootId = -int(item_iter->chanceOrRef);

                FillLoot(loot,LootId,store,loot_owner);
                continue;
            }

            LootStoreItem* LootedItem = hasChance(*item_iter);
            if ( LootedItem )
                loot->items.push_back(
                    LootItem(*LootedItem, urand(LootedItem->mincount, LootedItem->maxcount),
                    GenerateEnchSuffixFactor(LootedItem->itemid),
                    Item::GenerateItemRandomPropertyId(LootedItem->itemid))
                    );
        }
    }
    loot->unlootedCount = loot->items.size();
}

QuestItemList* FillQuestLoot(Player* player, Loot *loot)
{
    if (loot->items.size() == MAX_NR_LOOT_ITEMS) return NULL;
    QuestItemList *ql = new QuestItemList();

    for(uint8 i = 0; i < loot->quest_items.size(); i++)
    {
        LootItem &item = loot->quest_items[i];
        if(!item.is_looted && player->HasQuestForItem(item.itemid))
        {
            ql->push_back(QuestItem(i));
            // questitems get blocked when they first apper in a
            // player's quest vector
            if (!item.ffa_or_condition || !item.is_blocked) loot->unlootedCount++;
            item.is_blocked = true;
            if (loot->items.size() + ql->size() == MAX_NR_LOOT_ITEMS) break;
        }
    }

    if (ql->empty())
    {
        delete ql;
        return NULL;
    }

    loot->PlayerQuestItems[player] = ql;
    return ql;
}

void Loot::NotifyItemRemoved(uint8 lootIndex)
{
    // notify all players that are looting this that the item was removed
    // convert the index to the slot the player sees
    std::set<uint64>::iterator i_next; 
    for(std::set<uint64>::iterator i = PlayersLooting.begin(); i != PlayersLooting.end(); i = i_next)
    {
        i_next = i;
        ++i_next;
        if(Player* pl = ObjectAccessor::Instance().FindPlayer(*i))
            pl->SendNotifyLootItemRemoved(lootIndex);
        else
            PlayersLooting.erase(i);
    }
}

void Loot::NotifyMoneyRemoved()
{
    // notify all players that are looting this that the money was removed
    std::set<uint64>::iterator i_next; 
    for(std::set<uint64>::iterator i = PlayersLooting.begin(); i != PlayersLooting.end(); i = i_next)
    {
        i_next = i;
        ++i_next;
        if(Player* pl = ObjectAccessor::Instance().FindPlayer(*i))
            pl->SendNotifyLootMoneyRemoved();
        else
            PlayersLooting.erase(i);
    }
}

void Loot::NotifyQuestItemRemoved(uint8 questIndex)
{
    // when a free for all questitem is looted
    // all players will get notified of it being removed
    // (other questitems can be looted by each group member)
    // bit inefficient but isnt called often

    std::set<uint64>::iterator i_next; 
    for(std::set<uint64>::iterator i = PlayersLooting.begin(); i != PlayersLooting.end(); i = i_next)
    {
        i_next = i;
        ++i_next;
        if(Player* pl = ObjectAccessor::Instance().FindPlayer(*i))
        {
            QuestItemMap::iterator pq = PlayerQuestItems.find(pl);
            if (pq != PlayerQuestItems.end() && pq->second)
            {
                // find where/if the player has the given item in it's vector
                QuestItemList& pql = *pq->second;

                uint8 j;
                for (j = 0; j < pql.size(); ++j)
                    if (pql[j].index == questIndex)
                        break;

                if (j < pql.size())
                    pl->SendNotifyLootItemRemoved(items.size()+j);
            }
        }
        else
            PlayersLooting.erase(i);
    }
}

bool Loot::isLooted()
{
    return gold == 0 && unlootedCount == 0;
}

void Loot::generateMoneyLoot( uint32 minAmount, uint32 maxAmount )
{
    if (maxAmount > 0)
    {
        if (maxAmount <= minAmount)
            gold = uint32(maxAmount * sWorld.getRate(RATE_DROP_MONEY));
        else if ((maxAmount - minAmount) < 32700)
            gold = uint32(urand(minAmount, maxAmount) * sWorld.getRate(RATE_DROP_MONEY));
        else
            gold = uint32(urand(minAmount >> 8, maxAmount >> 8) * sWorld.getRate(RATE_DROP_MONEY)) << 8;
    }
}

ByteBuffer& operator<<(ByteBuffer& b, LootItem const& li)
{
    b << uint32(li.itemid);
    b << uint32(li.count);                                  // nr of items of this type
    b << uint32(li.displayid);
    b << uint32(li.randomSuffix);
    b << uint32(li.randomPropertyId);
    b << uint8(0);
    return b;
}

ByteBuffer& operator<<(ByteBuffer& b, LootView const& lv)
{
    Loot &l = lv.loot;
    uint8 itemsShown = 0;

    if (lv.qlist)
    {
        for (uint8 i = 0; i < lv.qlist->size(); i++)
            if (!lv.qlist->at(i).is_looted) itemsShown++;
    }

    switch (lv.permission)
    {
        case NONE_PERMISSION:
        {
            b << uint32(0);                                 //gold
            b << uint8(0);                                  //itemsShown
            return b;
        }
        case GROUP_PERMISSION:
        {
            // You are not the items propietary, so you can only see
            // blocked rolled items and questitems
            b << uint32(0);                                 //gold
            for (uint8 i = 0; i < l.items.size(); i++)
                if (!l.items[i].is_looted && l.items[i].is_blocked) itemsShown++;
            b << itemsShown;

            for (uint8 i = 0; i < l.items.size(); i++)
                if (!l.items[i].is_looted && l.items[i].is_blocked)
                    b << uint8(i) << l.items[i];
        }
        break;
        default:
        {
            b << l.gold;
            for (uint8 i = 0; i < l.items.size(); i++)
                if (!l.items[i].is_looted) itemsShown++;
            b << itemsShown;

            for (uint8 i = 0; i < l.items.size(); i++)
                if (!l.items[i].is_looted)
                    b << uint8(i) << l.items[i];
        }
    }

    if (lv.qlist)
    {
        for (QuestItemList::iterator i = lv.qlist->begin() ; i != lv.qlist->end(); ++i)
        {
            LootItem &item = l.quest_items[i->index];
            if (!i->is_looted && !item.is_looted)
                b << uint8(l.items.size() + (i - lv.qlist->begin())) << item;
        }
    }

    return b;
}
