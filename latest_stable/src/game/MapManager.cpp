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

#include "MapManager.h"
#include "Policies/SingletonImp.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "FlightMaster.h"
#include "RedZoneDistrict.h"
#include "Transports.h"
#include "GridDefines.h"
#include "MapInstanced.h"
#include "DestinationHolderImp.h"

#define CLASS_LOCK MaNGOS::ClassLevelLockable<MapManager, ZThread::Mutex>
INSTANTIATE_SINGLETON_2(MapManager, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(MapManager, ZThread::Mutex);

MapManager::MapManager() : i_gridCleanUpDelay(sWorld.getConfig(CONFIG_INTERVAL_GRIDCLEAN))
{
    i_timer.SetInterval(sWorld.getConfig(CONFIG_INTERVAL_MAPUPDATE));
}

MapManager::~MapManager()
{
    for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
        delete iter->second;

    for(size_t i = 0; i < m_Transports.size(); i++)
        delete m_Transports[i];

    Map::DeleteStateMachine();
}

void
MapManager::Initialize()
{
    Map::InitStateMachine();

    InitMaxInstanceId();
}

Map*
MapManager::GetBaseMap(uint32 id)
{
    Map *m = _findMap(id);

    if( m == NULL )
    {
        Guard guard(*this);

        const MapEntry* entry = sMapStore.LookupEntry(id);
        if (entry && ((entry->map_type == MAP_INSTANCE) || (entry->map_type == MAP_RAID)))
        {
            m = new MapInstanced(id, i_gridCleanUpDelay, 0);
        }
        else
        {
            m = new Map(id, i_gridCleanUpDelay, 0);
        }
        i_maps[id] = m;
    }

    assert(m != NULL);
    return m;
}

Map* MapManager::GetMap(uint32 id, const WorldObject* obj)
{
    Map *m = NULL;
    m = GetBaseMap(id);

    if (m && obj && m->Instanceable()) m = ((MapInstanced*)m)->GetInstance(obj);

    return m;
}

bool MapManager::CanPlayerEnter(uint32 mapid, Player* player)
{
    return GetBaseMap(mapid)->CanEnter(player);
}

void MapManager::RemoveBonesFromMap(uint32 mapid, uint64 guid, float x, float y)
{
    bool remove_result = GetBaseMap(mapid)->RemoveBones(guid, x, y);

    if (!remove_result)
    {
        sLog.outDebug("Bones %u not found in world. Delete from DB also.", GUID_LOPART(guid));
    }
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
    for (vector<Transport*>::iterator iter = m_Transports.begin(); iter != m_Transports.end(); iter++)
        (*iter)->Update(i_timer.GetCurrent());

    i_timer.SetCurrent(0);
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

bool MapManager::IsValidMAP(uint32 mapid)
{
    return sMapStore.LookupEntry(mapid);
}

bool MapManager::IsValidMapCoord(uint32 mapid, float x,float y)
{
    return IsValidMAP(mapid) && MaNGOS::IsValidMapCoord(x,y);
}

void MapManager::LoadGrid(int mapid, float x, float y, const WorldObject* obj, bool no_unload)
{
    CellPair p = MaNGOS::ComputeCellPair(x,y);
    Cell cell = RedZone::GetZone(p);
    GetMap(mapid, obj)->LoadGrid(cell,no_unload);
}

void MapManager::UnloadAll()
{
    for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
        iter->second->UnloadAll();
}

void MapManager::InitMaxInstanceId()
{
    i_MaxInstanceId = 0;

    QueryResult *result = sDatabase.Query( "SELECT MAX(`id`) FROM `instance`" );
    if( result )
    {
        i_MaxInstanceId = result->Fetch()[0].GetUInt32();
        delete result;
    }
}
