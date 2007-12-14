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
    bool freeforall = true;
    uint32 condition = 0;
    uint32 cond_value1 = 0;
    uint32 cond_value2 = 0;

    lootstore.clear();                                      // need for reload

    sLog.outString( "%s :", tablename);

    //                                                 0        1       2              3                     4           5           6                          7                   8
    QueryResult *result = WorldDatabase.PQuery("SELECT `entry`, `item`, `ChanceOrRef`, `QuestChanceOrGroup`, `mincount`, `maxcount`, `freeforall`, `lootcondition`, `condition_value1`, `condition_value2` FROM `%s`",tablename);

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
            freeforall = fields[6].GetBool();
            condition = fields[7].GetUInt32();
            cond_value1 = fields[8].GetUInt32();
            cond_value2 = fields[9].GetUInt32();

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

            switch (condition)
            {
                case CONDITION_AURA:
                    {
                        if(!sSpellStore.LookupEntry(cond_value1))
                        {
                            sLog.outErrorDb("Table: %s Dropped item (entry: %d) from loot (entry: %d) requires to have non existing spell (Id: %d) in aura condition!", tablename, item, entry, cond_value1);
                            continue;
                        }
                        if(cond_value2 > 2)
                        {
                            sLog.outErrorDb("Table: %s Dropped item (entry: %d) from loot (entry: %d) requires to have non existing effect index (%u) in aura condition (must be 0..2)!", tablename, item, entry, cond_value2);
                            continue;
                        }
                        break;
                    }
                case CONDITION_ITEM:
                    {
                        ItemPrototype const *proto = objmgr.GetItemPrototype(cond_value1);
                        if(!proto)
                        {
                            sLog.outErrorDb("Table: %s Dropped item (entry: %d) from loot (entry: %d) requires to have non existing item to be dropped (entry: %d)!", tablename, item, entry, cond_value1);
                            continue;
                        }
                        break;
                    }
                case CONDITION_ITEM_EQUIPPED:
                    {
                        ItemPrototype const *proto = objmgr.GetItemPrototype(cond_value1);
                        if(!proto)
                        {
                            sLog.outErrorDb("Table: %s Dropped item (entry: %d) from loot (entry: %d) requires non existing item (entry: %d) equipped to be dropped!", tablename, item, entry, cond_value1);
                            continue;
                        }
                        if (cond_value2)
                        {
                            sLog.outErrorDb("Table: %s Dropped item (entry: %d) from loot (entry: %d) requires item equipped, but item count is set (setting it to 0)", tablename, item, entry);
                            cond_value2 = 0;
                        }
                        break;
                    }
                case CONDITION_ZONEID:
                    {
                        AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(cond_value1);
                        if(!areaEntry)
                        {
                            sLog.outErrorDb("Table: %s Dropped item (entry: %d) from loot (entry: %d) has set non existing area (%u) requirement!", tablename, item, entry,cond_value1);
                            continue;
                        }

                        if(areaEntry->zone!=0)
                        {
                            sLog.outErrorDb("Table: %s Dropped item (entry: %d) from loot (entry: %d) has set to subzone (%u) instead zone requirement!", tablename, item, entry,cond_value1);
                            continue;
                        }
                        break;
                    }
                case CONDITION_REPUTATION_RANK:
                    {
                        FactionEntry const* factionEntry = sFactionStore.LookupEntry(cond_value1);
                        if(!factionEntry)
                        {
                            sLog.outErrorDb("Table: %s Dropped item (entry: %d) from loot (entry: %d) has set non existing faction (%u), reputation requirement!", tablename, item, entry, cond_value1);
                            continue;
                        }
                        break;
                    }
                case CONDITION_TEAM:
                    {
                        if (cond_value1 != ALLIANCE && cond_value1 != HORDE)
                        {
                            sLog.outErrorDb("Table: %s Dropped item (entry: %d) from loot (entry: %d) has set team drop condition to non-existing team!", tablename, item, entry);
                            continue;
                        }
                        break;
                    }
            }

            lootstore[entry].push_back( LootStoreItem(item, displayid, chanceOrRef, questchance, freeforall, condition, cond_value1, cond_value2, mincount, maxcount) );


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

struct HasChance
{
    struct GroupChance
    {
        GroupChance() : rolled(0.0f), cumulative(0.0f) {}

        float rolled;
        float cumulative;
    };

    typedef std::vector<GroupChance> GroupChances;

    LootStore* m_store;

    GroupChances groupChance;

    explicit HasChance(LootStore* _store) : m_store(_store)
    {
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
                return &itm;
            return NULL;
        }

        // Grouped loot
        int32 GroupId = itm.GetGroupId();

        if (GroupId < 0)
        {
            sLog.outErrorDb("HasChance: wrong loot group in DB (%i) for item %u", itm.questChanceOrGroup,itm.itemid);
            return NULL;
        }

        if (itm.chanceOrRef >= 0)
        {
            // Group of current loot - check for item chance in the group
            if(groupChance.size() <= GroupId)
                groupChance.resize(GroupId+1);

            GroupChance & gChance = groupChance[GroupId];

            if (gChance.cumulative == 0.0f)
                gChance.rolled = rand_chance();
            if (gChance.cumulative >= gChance.rolled)
                // An item from the group already accepted
                return NULL;

            gChance.cumulative += itm.chanceOrRef;
            if (gChance.cumulative >= gChance.rolled)
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

void FillLoot(Loot* loot, uint32 loot_id, LootStore& store, Player* loot_owner, bool referenced)
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
    HasChance      hasChance(&store);
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

                FillLoot(loot,LootId,store,loot_owner,true);
                continue;
            }

            LootStoreItem* LootedItem = hasChance(*item_iter);
            if ( LootedItem )
            {
                loot->items.push_back(
                    LootItem(*LootedItem, urand(LootedItem->mincount, LootedItem->maxcount),
                    GenerateEnchSuffixFactor(LootedItem->itemid),
                    Item::GenerateItemRandomPropertyId(LootedItem->itemid))
                    );

                // non-conditional one-player only items are counted here, 
                // free for all items are counted in FillFFALoot(), 
                // non-ffa conditionals are counted in FillNonQuestNonFFAConditionalLoot()
                if( ! LootedItem->freeforall && ! LootedItem->condition)
                    loot->unlootedCount++;
            }
        }
    }

    if(referenced)
        return;
    if(!loot_owner)
        return;
    Group * pGroup=loot_owner->GetGroup();
    if(!pGroup)
        return;
    for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        //fill the quest item map for every player in the recipient's group
        Player* pl = itr->getSource();
        if(!pl)
            continue;
        uint32 plguid = pl->GetGUIDLow();
        QuestItemMap::iterator qmapitr = loot->PlayerQuestItems.find(plguid);
        if (qmapitr == loot->PlayerQuestItems.end())
        {
            FillQuestLoot(pl, loot);
        }
        qmapitr = loot->PlayerFFAItems.find(plguid);
        if (qmapitr == loot->PlayerFFAItems.end())
        {
            FillFFALoot(pl, loot);
        }
        qmapitr = loot->PlayerNonQuestNonFFAConditionalItems.find(plguid);
        if (qmapitr == loot->PlayerNonQuestNonFFAConditionalItems.end())
        {
            FillNonQuestNonFFAConditionalLoot(pl, loot);
        }
    }
}

bool MeetsConditions(Player * owner, LootItem * itm)
{
    if( owner && itm->condition > 0 )
    {
        switch (itm->condition)
        {
            case CONDITION_AURA:
                return owner->HasAura(itm->cond_value1, itm->cond_value2);
            case CONDITION_ITEM:
                return owner->HasItemCount(itm->cond_value1,itm->cond_value2);
            case CONDITION_ITEM_EQUIPPED:
                return owner->HasItemEquipped(itm->cond_value1);
            case CONDITION_ZONEID:
                return owner->GetZoneId() == itm->cond_value1;
            case CONDITION_REPUTATION_RANK:
            {
                FactionEntry const* faction = sFactionStore.LookupEntry(itm->cond_value1);
                return faction && owner->GetReputationRank(faction) >= itm->cond_value2;
            }
            case CONDITION_TEAM:
                return owner->GetTeam() == itm->cond_value1;
            default:
                return false;
        }
    }
    else if (owner)
        return true;    //empty condition, return true
    return false;       //player not present, return false
}

QuestItemList* FillFFALoot(Player* player, Loot *loot)
{
    QuestItemList *ql = new QuestItemList();

    for(uint8 i = 0; i < loot->items.size(); i++)
    {
        LootItem &item = loot->items[i];
        if(!item.is_looted && item.freeforall && MeetsConditions(player, &item))
        {
            ql->push_back(QuestItem(i));
            loot->unlootedCount++;
        }
    }
    if (ql->empty())
    {
        delete ql;
        return NULL;
    }

    loot->PlayerFFAItems[player->GetGUIDLow()] = ql;
    return ql;
}

QuestItemList* FillQuestLoot(Player* player, Loot *loot)
{
    if (loot->items.size() == MAX_NR_LOOT_ITEMS) return NULL;
    QuestItemList *ql = new QuestItemList();

    for(uint8 i = 0; i < loot->quest_items.size(); i++)
    {
        LootItem &item = loot->quest_items[i];
        if(!item.is_looted && player->HasQuestForItem(item.itemid) && MeetsConditions(player, &item))
        {
            ql->push_back(QuestItem(i));

            // questitems get blocked when they first apper in a
            // player's quest vector
            //
            // increase once if one looter only, looter-times if free for all
            if (item.freeforall || !item.is_blocked)
                loot->unlootedCount++;

            item.is_blocked = true;

            if (loot->items.size() + ql->size() == MAX_NR_LOOT_ITEMS)
                break;
        }
    }
    if (ql->empty())
    {
        delete ql;
        return NULL;
    }

    loot->PlayerQuestItems[player->GetGUIDLow()] = ql;
    return ql;
}

QuestItemList* FillNonQuestNonFFAConditionalLoot(Player* player, Loot *loot)
{
    QuestItemList *ql = new QuestItemList();

    for(uint8 i = 0; i < loot->items.size(); i++)
    {
        LootItem &item = loot->items[i];
        if(!item.is_looted && !item.freeforall && item.condition && MeetsConditions(player, &item))
        {
            ql->push_back(QuestItem(i));
            if(!item.is_counted)
            {
                loot->unlootedCount++;
                item.is_counted=true;
            }
        }
    }
    if (ql->empty())
    {
        delete ql;
        return NULL;
    }

    loot->PlayerNonQuestNonFFAConditionalItems[player->GetGUIDLow()] = ql;
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
        if(Player* pl = ObjectAccessor::FindPlayer(*i))
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
        if(Player* pl = ObjectAccessor::FindPlayer(*i))
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
        if(Player* pl = ObjectAccessor::FindPlayer(*i))
        {
            QuestItemMap::iterator pq = PlayerQuestItems.find(pl->GetGUIDLow());
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
        for (uint8 i = 0; i < lv.qlist->size(); ++i)
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
            // You are not the items proprietary, so you can only see
            // blocked rolled items and quest items, and !ffa items (still needs work I'm afraid) /
            //group loot fix: you can see gold, and items set to underthreshold
            b << l.gold;                                    //gold
            for (uint8 i = 0; i < l.items.size(); ++i)
                if (!l.items[i].is_looted && (l.items[i].is_blocked || l.items[i].is_underthreshold || l.items[i].freeforall) && (!l.items[i].condition || MeetsConditions(lv.viewer, &l.items[i]))) 
                    itemsShown++;
            b << itemsShown;                                //send the number of items shown

            for (uint8 i = 0; i < l.items.size(); ++i)
                if (!l.items[i].is_looted && (l.items[i].is_blocked || l.items[i].is_underthreshold) && !l.items[i].freeforall && (!l.items[i].condition))
                    b << uint8(i) << l.items[i];            //send the index and the item if it's not looted, and blocked or under threshold, free for all items will be sent later, only one-player loots here
        }
        break;
        default:
        {
            b << l.gold;
            for (uint8 i = 0; i < l.items.size(); ++i)
                if (!l.items[i].is_looted && (!l.items[i].condition || MeetsConditions(lv.viewer, &l.items[i])))
                    itemsShown++;
            b << itemsShown;

            for (uint8 i = 0; i < l.items.size(); ++i)
                if (!l.items[i].is_looted && !l.items[i].freeforall && !l.items[i].condition)
                    b << uint8(i) << l.items[i];            //only send one-player loot items now, free for all will be sent later
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

    if (lv.ffalist)
    {
        for (QuestItemList::iterator i = lv.ffalist->begin() ; i != lv.ffalist->end(); ++i)
        {
            LootItem &item = l.items[i->index];
            if (!i->is_looted && !item.is_looted)
                b << uint8(i->index) << item;
        }
    }

    if (lv.conditionallist)
    {
        for (QuestItemList::iterator i = lv.conditionallist->begin() ; i != lv.conditionallist->end(); ++i)
        {
            LootItem &item = l.items[i->index];
            if (!i->is_looted && !item.is_looted)
                b << uint8(i->index) << item;
        }
    }

    return b;
}
