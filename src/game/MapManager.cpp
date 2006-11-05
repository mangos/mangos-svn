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
#include "RedZoneDistrict.h"

#define CLASS_LOCK MaNGOS::ClassLevelLockable<MapManager, ZThread::Mutex>
INSTANTIATE_SINGLETON_2(MapManager, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(MapManager, ZThread::Mutex);

static void grid_compression(const char *src_tbl, const char *dest_tbl)
{
    sDatabase.PExecute("TRUNCATE `%s`", dest_tbl);

    sDatabase.PExecute("DROP INDEX `idx_search` ON `%s`", dest_tbl);

    // used FLOOR instead ROUND: ROUND have different semmantic for yyy.5 case dependent from mySQL version/OS C-library
    sDatabase.PExecute(
        "INSERT INTO `%s` (`guid`,`map`,`position_x`,`position_y`,`cell_position_x`,`cell_position_y` ) "
        "SELECT `guid`,`map`,FLOOR(((`position_x`-'%f')/'%f') + '%u' + '0.5'),FLOOR(((`position_y`-'%f')/'%f') + '%u' + '0.5'),"
        "FLOOR(((`position_x`-'%f')/'%f') + '%u' + '0.5'),FLOOR(((`position_y`-'%f')/'%f') + '%u' + '0.5')  FROM `%s`", 
        dest_tbl, CENTER_GRID_OFFSET, SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_OFFSET,SIZE_OF_GRIDS, CENTER_GRID_ID, 
        CENTER_GRID_CELL_OFFSET,SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID, CENTER_GRID_CELL_OFFSET, SIZE_OF_GRID_CELL, 
        CENTER_GRID_CELL_ID, src_tbl);
    sDatabase.PExecute("UPDATE `%s` SET `grid`=(`position_x`*'%u') + `position_y`,`cell`=((`cell_position_y` * '%u') + `cell_position_x`)", dest_tbl, MAX_NUMBER_OF_GRIDS, TOTAL_NUMBER_OF_CELLS_PER_MAP);
    sDatabase.PExecute("CREATE INDEX `idx_search` ON `%s` (`grid`,`cell`,`map`)", dest_tbl);
}

MapManager::MapManager() : i_gridCleanUpDelay(sWorld.getConfig(CONFIG_INTERVAL_GRIDCLEAN))
{
    i_timer.SetInterval(sWorld.getConfig(CONFIG_INTERVAL_MAPUPDATE));
    Instances = 0;
    for(int i= 0;i<999;i++)
        for(int j=0;j<999;j++)
            m_instance[i][j] = 0;

}

MapManager::~MapManager()
{
    for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
        delete iter->second;

    sDatabase.PExecute("TRUNCATE table `creature_grid`");
    sDatabase.PExecute("TRUNCATE table `gameobject_grid`");
    sDatabase.PExecute("TRUNCATE table `corpse_grid`");

    Map::DeleteStateMachine();
}

void
MapManager::Initialize()
{
    Map::InitStateMachine();

    sLog.outDebug("Grid compression apply on creature(s) ...");
    grid_compression("creature", "creature_grid");
    sLog.outDebug("Grid compression apply on gameobject(s) ...");
    grid_compression("gameobject", "gameobject_grid");
    sLog.outDebug("Grid compression apply on corpse(s)/bone(s) ...");
    grid_compression("corpse", "corpse_grid");

    LoadCopys();
}

Map*
MapManager::GetMap(uint32 instanceid)
{
    Map *m = NULL;
    if( ( m=_getMap(instanceid) ) == NULL )
    {
        Guard guard(*this);
        if( (m = _getMap(instanceid)) == NULL )
        {
            m = new Map(instanceid, i_gridCleanUpDelay);
            i_maps[instanceid] = m;
        }

    }

    assert(m != NULL);
    return m;
}

bool MapManager::isInstanceMap( uint32 mapid )
{
    //TODO: check the mapid if its an instance from DB.
    if(mapid<=1)
        return false ;
    return true ;
}

void
MapManager::Update(time_t diff)
{
    i_timer.Update(diff);
    if( !i_timer.Passed() )
        return;

    for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
        iter->second->Update(i_timer.GetCurrent());

    ObjectAccessor::Instance().Update(i_timer.GetCurrent());
    FlightMaster::Instance().FlightReportUpdate(i_timer.GetCurrent());
    i_timer.SetCurrent(0);
    
    // Check copys state
    for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
    {
        if(iter->second == NULL)
            continue;
        uint64 playerguid = iter->second->GetCreater();
        uint32 mapid = iter->second->GetInstance()/1000;
        uint32 pinstance = iter->second->GetInstance();
        if( playerguid > 0 && iter->second->GetInstance()>1000)
        {
            if(iter->second->lefttime != -1)
                iter->second->lefttime -= diff;
            iter->second->savetime -= diff;

            Player* player = ObjectAccessor::Instance().FindPlayer(playerguid);
            if(!player)
            {
                continue;
            }
            if(!player->IsInWorld())
            {
                continue;
            }
            if(iter->second->lefttime <=1000 && m_instance[mapid][pinstance%1000] == 2)
            {
                DeleteCopy(pinstance);
                continue;
            }
			//if(iter->second->lefttime <=1000)
			//	SaveCopys(pinstance,playerguid,iter->second->lefttime);
            if(player->GetInstanceId() != pinstance && player->isAlive() 
                && m_instance[mapid][pinstance%1000] == 1)
            {
                m_instance[mapid][pinstance%1000] = 2;
                iter->second->lefttime = 60000;
            }
        }
    }
}

void MapManager::MoveAllCreaturesInMoveList()
{
    for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
        iter->second->MoveAllCreaturesInMoveList();
}

bool MapManager::ExistMAP(uint32 mapid, float x,float y)
{
    GridPair p = MaNGOS::ComputeGridPair(x,y);

    int gx=63-p.x_coord;
    int gy=63-p.y_coord;

    return Map::ExistMAP(mapid,gx,gy);
}

void MapManager::LoadGrid(uint32 instanceid, float x, float y, bool no_unload)
{
    CellPair p = MaNGOS::ComputeCellPair(x,y);
    Cell cell = RedZone::GetZone(p);
    GetMap(instanceid)->LoadGrid(cell,no_unload);
}

uint32 MapManager::CreateMapCopy(uint32 mapid,uint64 guid)
{
    int m_Instances = -1;
    uint32 instanceId = mapid*1000 + 1; // set the first copyid

    for(uint32 i=1;i<999;i++)
    {
        if(m_instance[mapid][i] == 0)
        {
            instanceId = mapid*1000 + i;
            m_instance[mapid][i] = 1;
            break;
        }
    }

    Map* newmap = new Map(instanceId,i_gridCleanUpDelay);
    newmap->SetCreater(guid);
    newmap->SetInstance(instanceId);
    newmap->lefttime = -1;
    newmap->savetime = 15*60*1000;
    i_maps[instanceId] = newmap;

    Instances++;

    // clean up and add spawns
    sDatabase.PExecute("DELETE FROM `creature` WHERE `map`=%u",instanceId);
    sDatabase.PExecute("DELETE FROM `creature_grid` WHERE `map`=%u",instanceId);
    sDatabase.PExecute("DELETE FROM `gameobject` WHERE `map`=%u",instanceId);
    sDatabase.PExecute("DELETE FROM `gameobject_grid` WHERE `map`=%u",instanceId);
    sDatabase.PExecute("DELETE FROM `corpse` WHERE `map`=%u",instanceId);
    sDatabase.PExecute("DELETE FROM `corpse_grid` WHERE `map`=%u",instanceId);
    sDatabase.PExecute("DELETE FROM `instance` WHERE `id` = '%u'",instanceId);

    sDatabase.PExecute(
        "INSERT INTO `creature` "
        "(`map`,`id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimemin`,`spawntimemax`,`spawndist`,`currentwaypoint`,`spawn_position_x`,`spawn_position_y`,`spawn_position_z`,`spawn_orientation`,`curhealth`,`curmana`,`respawntimer`,`state`,`npcflags`,`faction`,`MovementType`,`auras`) "
        "SELECT '%u',`id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimemin`,`spawntimemax`,`spawndist`,`currentwaypoint`,`spawn_position_x`,`spawn_position_y`,`spawn_position_z`,`spawn_orientation`,`curhealth`,`curmana`,`respawntimer`,`state`,`npcflags`,`faction`,`MovementType`,`auras` "
        "FROM `creature` WHERE `map` = '%u'",instanceId,mapid);
    sDatabase.PExecute("INSERT INTO `creature_grid` (`guid`,`map`,`position_x`,`position_y`,`cell_position_x`,`cell_position_y` ) SELECT `guid`,`map`,FLOOR(((`position_x`-%f)/%f) + %u.00001),FLOOR(((`position_y`-%f)/%f) + %u.00001),FLOOR(((`position_x`-%f)/%f) + %u.00001),FLOOR(((`position_y`-%f)/%f) + %u.00001)  FROM `creature`", CENTER_GRID_OFFSET, SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_OFFSET,SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_CELL_OFFSET,SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID, CENTER_GRID_CELL_OFFSET, SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID);
    sDatabase.PExecute("UPDATE `creature_grid` SET `grid`=(`position_x`*%u) + `position_y`,`cell`=((`cell_position_y` * %u) + `cell_position_x`)",MAX_NUMBER_OF_GRIDS, TOTAL_NUMBER_OF_CELLS_PER_MAP);

    sDatabase.PExecute(
        "INSERT INTO `gameobject` "
        "(`id`,`map`,`position_x`,`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`loot`,`respawntimer`) "
        "SELECT `id`,'%u',`position_x`,`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`loot`,`respawntimer` "
        "FROM `gameobject` WHERE `map` = '%u'",instanceId,mapid);
    sDatabase.PExecute("INSERT INTO `gameobject_grid` (`guid`,`map`,`position_x`,`position_y`,`cell_position_x`,`cell_position_y` ) SELECT `guid`,`map`,FLOOR(((`position_x`-%f)/%f) + %u.00001),FLOOR(((`position_y`-%f)/%f) + %u.00001),FLOOR(((`position_x`-%f)/%f) + %u.00001),FLOOR(((`position_y`-%f)/%f) + %u.00001)  FROM `gameobject`", CENTER_GRID_OFFSET, SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_OFFSET,SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_CELL_OFFSET,SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID, CENTER_GRID_CELL_OFFSET, SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID);
    sDatabase.PExecute("UPDATE `gameobject_grid` SET `grid`=(`position_x`*%u) + `position_y`,`cell`=((`cell_position_y` * %u) + `cell_position_x`)",MAX_NUMBER_OF_GRIDS, TOTAL_NUMBER_OF_CELLS_PER_MAP);

    return instanceId;
}

BOOL MapManager::DeleteCopy(uint32 instanceID)
{
    if(instanceID < 1000)
        return false;
    uint32 mapID = (instanceID / 1000);

    Map* map = GetMap(instanceID);
    map->SetCreater(0);
    map->SetInstance(0);

    for(uint32 i=1;i<999;i++)
    {
        if(i == instanceID%1000)
        {
            m_instance[instanceID/1000][i] = 0;
            break;
        }
    }

    sDatabase.PExecute("DELETE FROM `creature_grid` WHERE `map` = '%u'",instanceID);
    sDatabase.PExecute("DELETE FROM `creature` WHERE `map` = '%u'",instanceID);

    sDatabase.PExecute("DELETE FROM `gameobject_grid` WHERE `map` = '%u'",instanceID);
    sDatabase.PExecute("DELETE FROM `gameobject` WHERE `map` = '%u'",instanceID);

    sDatabase.PExecute("DELETE FROM `instance` WHERE `id` = '%u'",instanceID);

    char pAnnounce[256];
    sprintf((char*)pAnnounce, "[instance] Delete instance id: %u",instanceID);
    sWorld.SendWorldText(pAnnounce); 

    return true;
}

void MapManager::LoadCopys()
{
    QueryResult *result = sDatabase.PQuery("SELECT `id`,`map`,`lefttime`,`state`,`players` FROM `instance`");
    
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 m_Instancesid = fields[0].GetUInt32();
            uint32 mapid = fields[1].GetUInt32();
            uint64 guid = fields[4].GetUInt64();
            int32 lefttime = (int32)fields[2].GetUInt32();
            int state = fields[3].GetUInt32();
            sLog.outDebug("LoadCopys instanceID: %u mapid: %u Creater: %u LeftTime: %d",m_Instancesid,mapid,guid,lefttime);

            for(uint32 i=0;i<999;i++)
            {
                if(m_Instancesid%1000 == i)
                {
                    m_instance[m_Instancesid/1000][i] = state;
                    break;
                }
            }
            if(state != 1)
                continue;

            Map* newmap = new Map(m_Instancesid,i_gridCleanUpDelay);
            newmap->SetCreater(guid);
            newmap->lefttime = lefttime == -1 ? lefttime : lefttime*1000;
            newmap->SetInstance(m_Instancesid);
            newmap->savetime = 15*60*1000;
            i_maps[m_Instancesid] = newmap;
            // Add creatures
            Instances++;
            
            QueryResult *result = sDatabase.PQuery("SELECT `guid` FROM `creature` WHERE `map` = '%u'", m_Instancesid);

            if(!result)
                sDatabase.PExecute(
                    "INSERT INTO `creature` "
                    "(`map`,`id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimemin`,`spawntimemax`,`spawndist`,`currentwaypoint`,`spawn_position_x`,`spawn_position_y`,`spawn_position_z`,`spawn_orientation`,`curhealth`,`curmana`,`respawntimer`,`state`,`npcflags`,`faction`,`MovementType`,`auras`) "
                    "SELECT '%u',`id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimemin`,`spawntimemax`,`spawndist`,`currentwaypoint`,`spawn_position_x`,`spawn_position_y`,`spawn_position_z`,`spawn_orientation`,`curhealth`,`curmana`,`respawntimer`,`state`,`npcflags`,`faction`,`MovementType`,`auras` "
                    "FROM `creature` WHERE `map` = '%u'",m_Instancesid,m_Instancesid/1000);
            delete result;

            QueryResult *result1 = sDatabase.PQuery("SELECT `guid` FROM `gameobject` WHERE `map` = '%u'", m_Instancesid);

            if(!result1)
                sDatabase.PExecute(
                    "INSERT INTO `gameobject` "
                    "(`id`,`map`,`position_x`,`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`loot`,`respawntimer`) "
                    "SELECT `id`,'%u',`position_x`,`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`loot`,`respawntimer` "
                    "FROM `gameobject` WHERE `map` = '%u'",m_Instancesid,m_Instancesid/1000);
            delete result1;
            
            sDatabase.PExecute("INSERT INTO `creature_grid` (`guid`,`map`,`position_x`,`position_y`,`cell_position_x`,`cell_position_y` ) SELECT `guid`,`map`,FLOOR(((`position_x`-%f)/%f) + %u.00001),FLOOR(((`position_y`-%f)/%f) + %u.00001),FLOOR(((`position_x`-%f)/%f) + %u.00001),FLOOR(((`position_y`-%f)/%f) + %u.00001)  FROM `creature`", CENTER_GRID_OFFSET, SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_OFFSET,SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_CELL_OFFSET,SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID, CENTER_GRID_CELL_OFFSET, SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID);
            sDatabase.PExecute("UPDATE `creature_grid` SET `grid`=(`position_x`*%u) + `position_y`,`cell`=((`cell_position_y` * %u) + `cell_position_x`)",MAX_NUMBER_OF_GRIDS, TOTAL_NUMBER_OF_CELLS_PER_MAP);

            sDatabase.PExecute("INSERT INTO `gameobject_grid` (`guid`,`map`,`position_x`,`position_y`,`cell_position_x`,`cell_position_y` ) SELECT `guid`,`map`,FLOOR(((`position_x`-%f)/%f) + %u.00001),FLOOR(((`position_y`-%f)/%f) + %u.00001),FLOOR(((`position_x`-%f)/%f) + %u.00001),FLOOR(((`position_y`-%f)/%f) + %u.00001)  FROM `gameobject`", CENTER_GRID_OFFSET, SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_OFFSET,SIZE_OF_GRIDS, CENTER_GRID_ID, CENTER_GRID_CELL_OFFSET,SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID, CENTER_GRID_CELL_OFFSET, SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID);
            sDatabase.PExecute("UPDATE `gameobject_grid` SET `grid`=(`position_x`*%u) + `position_y`,`cell`=((`cell_position_y` * %u) + `cell_position_x`)",MAX_NUMBER_OF_GRIDS, TOTAL_NUMBER_OF_CELLS_PER_MAP);
        }while( result->NextRow() );
    }
    delete result;
}

void MapManager::SaveCopys(uint32 instanceID,uint64 guid,int32 lefttime)
{
    uint32 _state = m_instance[instanceID/1000][instanceID%1000];
    int32 left_time = lefttime == -1 ? -1 : lefttime/1000;
    sLog.outDebug("SaveCopys instanceID: %u Creater: %u LeftTime: %d",instanceID,guid,lefttime);
    sDatabase.PExecute("DELETE FROM `instance` WHERE `id` = '%u'",instanceID);
    sDatabase.PExecute("INSERT INTO `instance` (`id`,`map`,`players`,`state`,`lefttime`) VALUES('%u','%u','%u','%u','%d')",instanceID,instanceID/1000,uint32(guid),_state,left_time);
}
