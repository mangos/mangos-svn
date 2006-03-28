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

#include "LootMgr.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "ProgressBar.hpp"
#include "Policies/SingletonImp.h"
#include <stdlib.h>

bool Rand(float chance)
{
	uint32 val=rand()%100000;//in %	
	uint32 p = uint32(chance*1000);
	return p > val;
}



LootStore CreatureLoot;



void UnloadLoot()
{

  for(LootStore::iterator iter=CreatureLoot.begin(); iter != CreatureLoot.end(); ++iter)
  delete [] iter->second.items;

  CreatureLoot.clear();

}


void LoadCreaturesLootTables()
{

	uint32 curId=0;
	LootStore::iterator tab;
	uint32 ind;
	QueryResult *result = sDatabase.Query("SELECT * FROM `loottemplate`;");
    
    if( result )
    {      
    
		do 
		{     
			Field *fields = result->Fetch();
			
			uint32 entry_id=fields[0].GetUInt32();
		
			
			if( entry_id != curId )
			{
				curId = entry_id;
				ind=0;
				//new	
				
				
				QueryResult *result1 = 
				sDatabase.PQuery("SELECT COUNT(*) FROM loottemplate where entry = '%u';",entry_id);
				
				if(!result1) continue;
				
				Field *fields1 = result1->Fetch();

				CreatureLoot[entry_id].count=fields1[0].GetUInt32();
				
				
				CreatureLoot[entry_id].items=new StoreLootItem[CreatureLoot[entry_id].count];
				
				delete result1;
			}
		
			CreatureLoot[entry_id].items[ind].item.itemid=fields[1].GetUInt32();
			ItemPrototype*proto=objmgr.GetItemPrototype(fields[1].GetUInt32());
		
			if(proto)
			{
				CreatureLoot[entry_id].items[ind].item.displayid=proto->DisplayInfoID;

			}
				CreatureLoot[entry_id].items[ind].chance=fields[2].GetFloat();

			ind++;

		} while( result->NextRow() );
		delete result;
    }

}


void FillLoot(Loot * loot,uint32 loot_id)
{
	loot->items.clear ();
	loot->gold =0;
	
	LootStore::iterator tab =CreatureLoot.find(loot_id);
	if( CreatureLoot.end()==tab)return;

	for(uint32 x =0; x<tab->second.count;x++)
	if(Rand(tab->second.items[x].chance))
	{
		__LootItem itm;
		itm.item =tab->second.items[x].item;
		itm.isLooted =false;
		loot->items.push_back(itm);
						
	}
}



