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

#include "MapManager.h"
#include "Policies/SingletonImp.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "FlightMaster.h"

#define CLASS_LOCK MaNGOS::ClassLevelLockable<MapManager, ZThread::Mutex>
INSTANTIATE_SINGLETON_2(MapManager, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(MapManager, ZThread::Mutex);

static void grid_compression(const char *src_tbl, const char *dest_tbl)
{
    sDatabase.PExecute("DROP TABLE IF EXISTS %s;", dest_tbl);

    sDatabase.PExecute("CREATE TABLE IF NOT EXISTS %s (`guid` bigint(20) unsigned NOT NULL default '0', `x` int(11) NOT NULL default '0', `y` int(11) NOT NULL default '0', `cell_x` int(11) NOT NULL default '0', `cell_y` int(11) NOT NULL default '0', `grid_id` int(11) NOT NULL default '0', `cell_id` int(11) NOT NULL default '0', `mapId` int(11) NOT NULL default '0', KEY srch_grid(grid_id, cell_id, mapId) ) TYPE=MyISAM;", dest_tbl);

    sDatabase.PExecute("INSERT INTO %s (guid, mapId, x, y, cell_x, cell_y) SELECT guid,map, (( positionX-'%f')/'%f') + '%d' ,((positionY-'%f')/'%f') + '%d', ((positionX-'%f')/'%f') + '%d', ((positionY-'%f')/'%f') + '%d' FROM %s;", dest_tbl, CENTER_GRID_OFFSET, SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_OFFSET,SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_CELL_OFFSET,SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID, CENTER_GRID_CELL_OFFSET, SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID, src_tbl);
    
    sDatabase.PExecute("UPDATE %s SET grid_id=(x*'%d') + y,cell_id=((cell_y * '%u') + cell_x);", dest_tbl, MAX_NUMBER_OF_GRIDS, TOTAL_NUMBER_OF_CELLS_PER_MAP);
}

MapManager::MapManager() : i_gridCleanUpDelay(1000*300)
{
    i_timer.SetInterval(100);
}

MapManager::~MapManager()
{    
    for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
    delete iter->second;   

    sDatabase.PExecute("TRUNCATE table creatures_grid;");
    sDatabase.PExecute("TRUNCATE table gameobjects_grid;");
}

void
MapManager::Initialize()
{
    sLog.outDebug("Grid compression apply on creatures....");
    grid_compression("creatures", "creatures_grid");
    sLog.outDebug("Grid compression apply on gameobjects....");
    grid_compression("gameobjects", "gameobjects_grid");
}

Map*
MapManager::GetMap(uint32 id)
{
    Map *m = NULL;
    if( ( m=_getMap(id) ) == NULL )
    {
    Guard guard(*this);
    if( (m = _getMap(id)) == NULL )
    {
        m = new Map(id, i_gridCleanUpDelay);
        i_maps[id] = m;        
    }
    
    }

    assert(m != NULL);
    return m;
}

void
MapManager::Update(time_t diff)
{
    i_timer.Update(diff);
    if( !i_timer.Passed() )
	return;

    i_timer.Reset();
    Guard guard(*this);
    for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
	iter->second->Update(diff);
    
    ObjectAccessor::Instance().Update(diff);
    FlightMaster::Instance().FlightReportUpdate(diff);
}


