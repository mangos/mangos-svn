/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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

INSTANTIATE_SINGLETON_1(LootMgr);

LootMgr::LootMgr()
{
}

void LootMgr::LoadLootTables()
{   
    DEBUG_LOG("Initialize creature loot tables...");
    _populateLootTemplate("creatureloot", i_creaturesLoot);
    DEBUG_LOG("Initialize game object loot tables...");
    _populateLootTemplate("gameobj_loot", i_gameObjectsLoot);
}

LootMgr::~LootMgr()
{
  for(LootTable::iterator iter=i_creaturesLoot.begin(); iter != i_creaturesLoot.end(); ++iter)
    delete iter->second;

  for(LootTable::iterator iter=i_gameObjectsLoot.begin(); iter != i_gameObjectsLoot.end(); ++iter)
    delete iter->second;

  i_creaturesLoot.clear();
  i_gameObjectsLoot.clear();
}


void LootMgr::_populateLootTemplate(const char *loot_table_name, LootTable &loot_table)
{
  
    std::stringstream id_query;
    unsigned int count = 0;
    id_query << "SELECT * FROM " << loot_table_name;
    std::auto_ptr<QueryResult> result( sDatabase.Query(id_query.str().c_str()) );
    
    if( result.get() != NULL )
    {      
    int curId = -1;
    LootList *table = NULL;
    barGoLink bar( result->GetRowCount() );
    do 
    {     
        bar.step();
        Field *fields = result->Fetch();

		int entry_id = fields[0].GetUInt32(); 

		if (stricmp(loot_table_name, "gameobj_loot") == 0)
			entry_id = fields[1].GetUInt32(); 
        
        if( entry_id != curId )
        {
        LootTable::iterator next_table = loot_table.find(entry_id);
        curId = entry_id;

        
        
        
        
        
        if( next_table == loot_table.end() )
        {
            ++count;
            table = new LootList;
            loot_table[curId] = table; 
        }
        else
        {
            table = next_table->second;
        }
        }
        
        table->push_back(LootItem(fields[1].GetUInt32(), fields[2].GetFloat()*100));
        
    } while( result->NextRow() );
    }
    
    sLog.outString(">> Loaded %d loot templates for table %s", count, loot_table_name);
	sLog.outString( "" );
}

LootMgr::LootList LootMgr::si_noLoot;
