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

#include <map>
#include <string>
#include <vector>
#include "Common.h"
#include "Policies/Singleton.h"
#include "ItemPrototype.h"

typedef struct
{
	uint32 itemid;
	uint32 displayid;
}_LootItem;

typedef struct
{
	_LootItem item;
	bool	isLooted;
}__LootItem;


typedef struct
{
	float chance;
	_LootItem item;
	
}StoreLootItem;


typedef struct 
{
	uint32 count;
	StoreLootItem*items;
}StoreLootList;

typedef struct
{
	std::vector<__LootItem> items;
	uint32 gold;
}Loot;

void FillLoot(Loot * loot,uint32 loot_id);
void LoadCreaturesLootTables();
//////////////////////////////////////////////////////////////////////////////////////////


typedef HM_NAMESPACE::hash_map<uint32, StoreLootList > LootStore;  



#endif
