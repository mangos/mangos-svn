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

#ifndef _ENCHANTMENTROTOTYPE_H
#define _ENCHANTMENTROTOTYPE_H

#include <list>
#include <vector>
#include "Common.h"
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

	extern EnchantmentStore RandomItemEnch;

//	void LoadRandomEnchantmentsTable(EnchantmentStore& enchstore,char const* tablename);
	void LoadRandomEnchantmentsTable();
	uint32 GetItemEnchantMod(uint32 entry);
	uint32 GenerateEnchSuffixFactor(uint32 item_id);
#endif
