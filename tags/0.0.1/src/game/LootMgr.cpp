/* LootMgr.cpp
 *
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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


createFileSingleton( LootMgr );

LootMgr::LootMgr()
{
}

void
LootMgr::LoadLootTables()
{    
    _populateLootTemplate("creatureloot", i_creaturesLoot);
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


void
LootMgr::_populateLootTemplate(const char *loot_table_name, LootTable &loot_table)
{
  // Note the item should be entryid,itemid,percentchance..
    std::stringstream id_query;
    unsigned int count = 0;
    id_query << "SELECT * FROM " << loot_table_name;
    std::auto_ptr<QueryResult> result( sDatabase.Query(id_query.str().c_str()) );
    
    if( result.get() != NULL )
    {      
	int curId = -1;
	LootList *table = NULL;
	
	do 
	{	 
	    Field *fields = result->Fetch();
	    int entry_id = fields[0].GetUInt32(); 
	    
	    if( entry_id != curId )
	    {
		LootTable::iterator next_table = loot_table.find(entry_id);
		curId = entry_id;

		// Note, doing this is faster than each select by a where clause
		// or order the selection.  The penalty is one hash search per 
		// key change which is faster than searching or
		// a where clause in each key.  I'll make the loading faster
		// by indexing
		if( next_table == loot_table.end() )
		{
		    ++count;
		    table = new LootList;
		    loot_table[curId] = table; // next table
		}
		else
		{
		    table = next_table->second;
		}
	    }
	    
	    table->push_back(LootItem(fields[1].GetUInt32(), fields[2].GetFloat()*100));
	    
	} while( result->NextRow() );
    }
    
    sLog.outDebug("Loaded %d loot templates for table %s", count, loot_table_name);
}

LootMgr::LootList LootMgr::si_noLoot;
