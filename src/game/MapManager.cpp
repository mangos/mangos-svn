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
    sDatabase.PExecute("TRUNCATE `%s`;", dest_tbl);

    sDatabase.PExecute("DROP INDEX `idx_search` ON `%s`", dest_tbl);
    sDatabase.PExecute("INSERT INTO `%s` (`guid`,`map`,`position_x`,`position_y`,`cell_position_x`,`cell_position_y` ) SELECT `guid`,`map`,((`position_x`-%f)/%f) + %u,((`position_y`-%f)/%f) + %u,((`position_x`-%f)/%f) + %u,((`position_y`-%f)/%f) + %u  FROM `%s`;", dest_tbl, CENTER_GRID_OFFSET, SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_OFFSET,SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_CELL_OFFSET,SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID, CENTER_GRID_CELL_OFFSET, SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID, src_tbl);
    sDatabase.PExecute("UPDATE `%s` SET `grid`=(`position_x`*%u) + `position_y`,`cell`=((`cell_position_y` * %u) + `cell_position_x`);", dest_tbl, MAX_NUMBER_OF_GRIDS, TOTAL_NUMBER_OF_CELLS_PER_MAP);
    sDatabase.PExecute("CREATE INDEX `idx_search` ON `%s` (`grid`,`cell`,`map`)", dest_tbl);
}

MapManager::MapManager() : i_gridCleanUpDelay(sWorld.getConfig(CONFIG_INTERVAL_GRIDCLEAN))
{
    i_timer.SetInterval(sWorld.getConfig(CONFIG_INTERVAL_MAPUPDATE));
}

MapManager::~MapManager()
{
    for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
        delete iter->second;

    sDatabase.PExecute("TRUNCATE table `creature_grid`;");
    sDatabase.PExecute("TRUNCATE table `gameobject_grid`;");
    sDatabase.PExecute("TRUNCATE table `corpse_grid`;");
}

void
MapManager::Initialize()
{
    sLog.outDebug("Grid compression apply on creature(s) ...");
    grid_compression("creature", "creature_grid");
    sLog.outDebug("Grid compression apply on gameobject(s) ...");
    grid_compression("gameobject", "gameobject_grid");
    sLog.outDebug("Grid compression apply on corpse(s)/bone(s) ...");
    grid_compression("corpse", "corpse_grid");
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
