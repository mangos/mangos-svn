/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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

#include "MapInstanced.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "BattleGround.h"
#include "VMapFactory.h"
#include "InstanceSaveMgr.h"

MapInstanced::MapInstanced(uint32 id, time_t expiry, uint32 aInstanceId) : Map(id, expiry, 0, 0)
{
    // initialize instanced maps list
    m_InstancedMaps.clear();
    // fill with zero
    memset(&GridMapReference, 0, MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_GRIDS*sizeof(uint16));
}

void MapInstanced::Update(const uint32& t)
{
    // take care of loaded GridMaps (when unused, unload it!)
    Map::Update(t);

    // update the instanced maps
    InstancedMaps::iterator i = m_InstancedMaps.begin();

    while (i != m_InstancedMaps.end())
    {
        if(i->second->CanUnload(t))
        {
            DestroyInstance(i);                             // iterator incremented
        }
        else
        {
            // update only here, because it may schedule some bad things before delete
            i->second->Update(t);
            ++i;
        }
    }
}

void MapInstanced::MoveAllCreaturesInMoveList()
{
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); i++)
    {
        i->second->MoveAllCreaturesInMoveList();
    }
}

bool MapInstanced::RemoveBones(uint64 guid, float x, float y)
{
    bool remove_result = false;

    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); i++)
    {
        remove_result = remove_result || i->second->RemoveBones(guid, x, y);
    }

    return remove_result;
}

void MapInstanced::UnloadAll(bool pForce)
{
    // Unload instanced maps
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); i++)
        i->second->UnloadAll(pForce);

    // Delete the maps only after everything is unloaded to prevent crashes
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); i++)
        delete i->second;

    m_InstancedMaps.clear();

    // Unload own grids (just dummy(placeholder) grids, neccesary to unload GridMaps!)
    Map::UnloadAll(pForce);
}

/*
- return the right instance for the object, based on its InstanceId
- create the instance if it's not created already
- the player is not actually added to the instance (only in InstanceMap::Add)
*/
Map* MapInstanced::GetInstance(const WorldObject* obj)
{
    uint32 CurInstanceId = obj->GetInstanceId();
    Map* map = NULL;

    if (obj->GetMapId() == GetId() && CurInstanceId != 0)
    {
        // the object wants to be put in a certain instance of this map
        map = _FindMap(CurInstanceId);
        if (map)
        {
            // it's already loaded
            return(map);
        }
        else
        {
            // not loaded then try to load it
            if(obj->GetTypeId() != TYPEID_PLAYER)
            {
                // creatures/gameobjects are not allowed to load new instances for themselves
                sLog.outError("MAPINSTANCED: WorldObject '%u' (Entry: %u Type: %u) is requesting instance '%u' of map '%u', instantiating", obj->GetGUIDLow(), obj->GetEntry(), obj->GetTypeId(), CurInstanceId, GetId());
                assert(false);
                return NULL;
            }
            else
            {
                //If the player has left game, being in instance, and the server has crashed
                //at the next login of the player we should create new instance with the same ID
                //and add this player in it.

                Player *player = (Player*)obj;
                InstanceSave *save = NULL;

                if(!player->IsInWorld())
                    assert(false);

                const MapEntry *entry = sMapStore.LookupEntry(GetId());
                switch(entry->map_type)
                {
                    case MAP_INSTANCE:
                    case MAP_RAID:
                        if(!(save = sInstanceSaveManager.GetInstanceSave(CurInstanceId)))
                        {
                            sLog.outError("MapInstanced::GetInstance: player %d has %d instanceId but no instance save can be found!", player->GetGUIDLow(), CurInstanceId, player->GetMapId());
                            assert(false);
                        }
                        else
                            return CreateInstance(CurInstanceId, save, player->GetDifficulty());
                    default:
                        sLog.outError("MapInstanced::GetInstance: unhandled map type %d!", entry->map_type);
                        assert(false);
                        return NULL;
                }
            }
        }
    }
    else
    {
        // instance not specified, find an existing or create a new one
        if(obj->GetTypeId() != TYPEID_PLAYER)
        {
            sLog.outError("MAPINSTANCED: WorldObject '%u' (Entry: %u Type: %u) requested base map instance of map '%u', this must not happen", obj->GetGUIDLow(), obj->GetEntry(), obj->GetTypeId(), GetId());
            assert(false);
            return NULL;
        }
        else
        {
            uint32 NewInstanceId = 0;                       // instanceId of the resulting map
            Player* player = (Player*)obj;

            // TODO: battlegrounds and arenas

            InstancePlayerBind *pBind = player->GetBoundInstance(GetId(), player->GetDifficulty());
            InstanceSave *pSave = pBind ? pBind->save : NULL;

            // the player's permanet player bind is taken into consideration first
            // then the player's group bind and finally the solo bind.
            if(!pBind || !pBind->perm)
            {
                InstanceGroupBind *groupBind = NULL;
                Group *group = player->GetGroup();
                // use the player's difficulty setting (it may not be the same as the group's)
                if(group && (groupBind = group->GetBoundInstance(GetId(), player->GetDifficulty())))
                    pSave = groupBind->save;
            }

            if(pSave)
            {
                // solo/perm/group
                NewInstanceId = pSave->GetInstanceId();
                map = _FindMap(NewInstanceId);
                // it is possible that the save exists but the map doesn't
                if(!map)
                    map = CreateInstance(NewInstanceId, pSave, pSave->GetDifficulty());
                return map;
            }
            else
            {
                // if no instanceId via group members or instance saves is found
                // the instance will be created for the first time
                NewInstanceId = MapManager::Instance().GenerateInstanceId();
                return CreateInstance(NewInstanceId, NULL, player->GetDifficulty());
            }
        }
    }
}

InstanceMap* MapInstanced::CreateInstance(uint32 InstanceId, InstanceSave *save, uint8 difficulty)
{
    // load/create a map
    Guard guard(*this);

    // some instances only have one difficulty
    const MapEntry* entry = sMapStore.LookupEntry(GetId());
    if(!entry || !entry->SupportsHeroicMode()) difficulty = DIFFICULTY_NORMAL;

    sLog.outDebug("MapInstanced::CreateInstance: %smap instance %d for %d created with difficulty %s", save?"":"new ", InstanceId, GetId(), difficulty?"heroic":"normal");
 
    InstanceMap *map = new InstanceMap(GetId(), GetGridExpiry(), InstanceId, difficulty);
    assert(map->IsDungeon());

    bool load_data = save != NULL;
    map->CreateInstanceData(load_data);

    m_InstancedMaps[InstanceId] = map;
    return map;
}

void MapInstanced::DestroyInstance(uint32 InstanceId)
{
    InstancedMaps::iterator itr = m_InstancedMaps.find(InstanceId);
    if(itr != m_InstancedMaps.end())
        DestroyInstance(itr);
}

// increments the iterator after erase
void MapInstanced::DestroyInstance(InstancedMaps::iterator &itr)
{
    itr->second->UnloadAll(true);
    VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(itr->second->GetId());
    // erase map
    delete itr->second;
    m_InstancedMaps.erase(itr++);
}

