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

#ifndef MANGOS_LOOTMGR_H
#define MANGOS_LOOTMGR_H

#include <list>
#include <vector>
#include "Common.h"
#include "Policies/Singleton.h"
#include "ItemPrototype.h"
#include "ByteBuffer.h"
#include "Util.h"

using std::vector;
using std::list;

class Player;

struct LootItem
{
    uint32  itemid;
    uint32  displayid;
    float   chance;
    float   questchance;
    bool    is_looted;

    LootItem() 
        : itemid(0), displayid(0), chance(0), questchance(0), is_looted(true) {}

    LootItem(uint32 _itemid, uint32 _displayid, float _chance, float _questchance) 
        : itemid(_itemid), displayid(_displayid), chance(_chance), questchance(_questchance), is_looted(false) {}

    static bool looted(LootItem &itm) { return itm.is_looted; }
    static bool not_looted(LootItem &itm) { return !itm.is_looted; }
    static bool lootable(LootItem &itm, Player* player);
};

struct Loot
{
    vector<LootItem> items;
    uint32 gold;

    Loot(uint32 _gold = 0) : gold(_gold) {}

    ~Loot()
    {
        items.clear();
    }

    bool empty() const { return items.empty() && gold == 0; }
};

void FillLoot(Player* player,Loot *loot, uint32 loot_id);
void FillSkinLoot(Loot *Skinloot,uint32 itemid);
void LoadLootTables();

typedef list<LootItem> LootItemList;
typedef HM_NAMESPACE::hash_map<uint32, LootItemList > LootStore;

inline ByteBuffer& operator<<(ByteBuffer& b, LootItem& li)
{
    b << uint32(li.itemid);
    b << uint32(1);                                         // nr of items of this type
    b << uint32(li.displayid);
    b << uint64(0) << uint8(0);
    return b;
}

inline ByteBuffer& operator<<(ByteBuffer& b, Loot& l)
{
    b << l.gold;
    b << uint8(l.items.size());

    for (uint8 i = 0; i < l.items.size(); i++)
        b << uint8(i) << l.items[i];

    return b;
}
#endif
