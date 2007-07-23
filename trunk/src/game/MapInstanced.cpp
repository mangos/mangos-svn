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

#include "MapInstanced.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "BattleGround.h"
#include "VMapFactory.h"

MapInstanced::MapInstanced(uint32 id, time_t expiry, uint32 aInstanceId) : Map(id, expiry, 0)
{
    // initialize instanced maps list
    InstancedMaps.clear();
    // fill with zero
    memset(&GridMapReference, 0, MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_GRIDS*sizeof(uint16));
}

void MapInstanced::Update(const uint32& t)
{
    // take care of loaded GridMaps (when unused, unload it!)
    Map::Update(t);

    // update the instanced maps
    HM_NAMESPACE::hash_map< uint32, Map* >::iterator i = InstancedMaps.begin();

    while (i != InstancedMaps.end())
    {
        if (i->second->NeedsReset())
        {
            if (i->second->GetPlayersCount() == 0)
            {
                i->second->Reset();
                // avoid doing ++ on invalid data
                HM_NAMESPACE::hash_map< uint32, Map* >::iterator i_old = i;
                ++i;
                VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(i_old->second->GetId());
                // erase map
                delete i_old->second;
                InstancedMaps.erase(i_old);
            }
            else
            {
                // shift reset time of the map
                i->second->InitResetTime();
            }
        }
        else
        {
            // update only here, because it may schedule some bad things before delete
            // in the other case
            i->second->Update(t);
            ++i;
        }
    }
}

void MapInstanced::MoveAllCreaturesInMoveList()
{
    for (HM_NAMESPACE::hash_map< uint32, Map* >::iterator i = InstancedMaps.begin(); i != InstancedMaps.end(); i++)
    {
        i->second->MoveAllCreaturesInMoveList();
    }
}

bool MapInstanced::RemoveBones(uint64 guid, float x, float y)
{
    bool remove_result = false;

    for (HM_NAMESPACE::hash_map< uint32, Map* >::iterator i = InstancedMaps.begin(); i != InstancedMaps.end(); i++)
    {
        remove_result = remove_result || i->second->RemoveBones(guid, x, y);
    }

    return remove_result;
}

void MapInstanced::UnloadAll()
{
    // Unload instanced maps
    for (HM_NAMESPACE::hash_map< uint32, Map* >::iterator i = InstancedMaps.begin(); i != InstancedMaps.end(); i++)
        i->second->UnloadAll();

    // Delete the maps only after everything is unloaded to prevent crashes
    for (HM_NAMESPACE::hash_map< uint32, Map* >::iterator i = InstancedMaps.begin(); i != InstancedMaps.end(); i++)
        delete i->second;

    InstancedMaps.clear();

    // Unload own grids (just dummy(placeholder) grids, neccesary to unload GridMaps!)
    Map::UnloadAll();
}

Map* MapInstanced::GetInstance(const WorldObject* obj)
{
    uint32 InstanceId = obj->GetInstanceId();
    Map* map = NULL;

    if (InstanceId != 0) map = _FindMap(InstanceId);

    if (map && obj->GetTypeId() != TYPEID_PLAYER) return(map); // return map for non-player objects

    if (obj->GetTypeId() != TYPEID_PLAYER)
    {
        sLog.outDebug("MAPINSTANCED: WorldObject '%u' (Entry: %u Type: %u) is requesting instance '%u' of map '%u', instantiating", obj->GetGUIDLow(), obj->GetEntry(), obj->GetTypeId(), InstanceId, GetId());

        if (InstanceId == 0)
        {
            sLog.outError("MAPINSTANCED: WorldObject '%u' (Entry: %u Type: %u) requested base map instance of map '%u', this must not happen", obj->GetGUIDLow(), obj->GetEntry(), obj->GetTypeId(), GetId());
            return(this);
        }

        // short instantiate process for world object
        CreateInstance(InstanceId, map);
        return(map);
    }

    // here we do additionally verify, if the player is allowed in instance by leader
    // and if it has current instance bound correctly
    Player* player = (Player*)obj;

    // reset instance validation flag
    player->m_InstanceValid = true;

    BoundInstancesMap::iterator i = player->m_BoundInstances.find(GetId());
    if (i != player->m_BoundInstances.end())
    {
        // well, we have an instance bound, really, verify self or group leader
        if (!((player->GetGUIDLow() == i->second.second) || 
            (player->GetGroup() && 
             (GUID_LOPART(player->GetGroup()->GetLeaderGUID()) == i->second.second))))
        {
            // well, we are bound to instance, but are not a leader and are not in the correct group
            // we must not rebind us or the instantiator (which can surely be the same)
            // will remain bound if accepted into group or will be unbound, if we go to homebind
            InstanceId = i->second.first; // restore the instance bound
            player->m_InstanceValid = false; // player instance is invalid
            if (InstanceId != 0) map = _FindMap(InstanceId); // restore the map bound
        }
    }
    else
    {
        // the player has no map bindings, we can proceed safely creating a new one
        map = NULL;
    }

    if (map) return(map); // here we go, the map is found and we are correctly bound

    Player* instantiator = NULL;
    uint32 instantiator_id = 0;
    bool instantiator_online = true;
    bool instantiator_bound = false;

    // we do need to scan for the instantiator, if the instance we scan for is valid, not temporary
    if (player->m_InstanceValid)
    {
        // either we are not bound to the instance, or we have to create new instance, do it
        InstanceId = 0;
    
        // determine the instantiator which designates the instance id
        if (player->GetGroup())
        {
            // instantiate map for group leader (possibly got from the database)
            sLog.outDebug("MAPINSTANCED: Player '%s' is in group, instantiating map for group leader", player->GetName());
            instantiator = objmgr.GetPlayer(player->GetGroup()->GetLeaderGUID());
            if (!instantiator)
            {
                // the very special case: leader is not online, read instance map from DB
                instantiator_online = false;
            }
        }

        if (!instantiator && instantiator_online)
        {
            sLog.outDebug("MAPINSTANCED: Player '%s' is not in group, instantiating map for player", player->GetName());
            instantiator = player;
        }
    
        // now, get the real instance id from the instantiator
        if (instantiator_online)
        {
            // player online, normal instantianting
            sLog.outDebug("MAPINSTANCED: Instantiating map for player '%s' (group leader '%s')", player->GetName(), instantiator->GetName());
            instantiator_id = instantiator->GetGUIDLow();
            BoundInstancesMap::iterator i = instantiator->m_BoundInstances.find(GetId());
            if (i != instantiator->m_BoundInstances.end())
            {
                // the instantiator has his instance bound
                InstanceId = i->second.first;
                // this check is to avoid the case where remote instantiator has his instance
                // bound to another remote instantiator (e.g. exited from group recently)
                // if that is the case, the instance for him will be regenerated and rebound
                if ((instantiator == player) || (i->second.second == instantiator_id))
                {
                    // player is instantiator or instantiator has his instance bound to himself
                    instantiator_bound = true;
                }
            }
        }
        else
        {
            // the aforementioned "very special" case of leader being not online
            sLog.outDebug("MAPINSTANCED: Instantiating map for player '%s' (group leader is not online, querying DB)", player->GetName());
            instantiator_id = GUID_LOPART(player->GetGroup()->GetLeaderGUID());
            QueryResult* result = sDatabase.PQuery("SELECT `instance` FROM `character_instance` WHERE (`guid` = '%u') AND (`map` = '%u') AND (`leader` = '%u')", instantiator_id, GetId(), instantiator_id);
            if (result)
            {
                // the instantiator has his instance bound
                InstanceId = result->Fetch()[0].GetUInt32();
                instantiator_bound = true;
                delete result;
            }
        }

        // check, if we have to generate new instance
        if (!instantiator_bound || (InstanceId == 0))
        {
            // yes, new instance id has to be generated
            InstanceId = MapManager::Instance().GenerateInstanceId();
        }
        else
        {
            // find map of the designated instance and verify it
            map = _FindMap(InstanceId);
        }
    }

    // now create the instance
    CreateInstance(InstanceId, map);

    // we do need to bind instance, if the instance we use is valid, not temporary
    if (player->m_InstanceValid)
    {
        // bind instance to instantiator (only if needed)
        if (!instantiator_bound)
        {
            sDatabase.BeginTransaction();
            if (instantiator_online)
            {
                // player online, normal bind
                instantiator->m_BoundInstances[GetId()] = std::pair< uint32, uint32 >(InstanceId, instantiator_id);
                sDatabase.PExecute("DELETE FROM `character_instance` WHERE (`guid` = '%u') AND (`map` = '%u')", instantiator_id, GetId());
                sDatabase.PExecute("INSERT INTO `character_instance` VALUES ('%u', '%u', '%u', '%u')", instantiator_id, GetId(), InstanceId, instantiator_id);
            }
            else
            {
                // the aforementioned "very special" case of leader being not online
                sDatabase.BeginTransaction();
                sDatabase.PExecute("DELETE FROM `character_instance` WHERE (`guid` = '%u') AND (`map` = '%u')", GUID_LOPART(player->GetGroup()->GetLeaderGUID()), GetId());
                sDatabase.PExecute("INSERT INTO `character_instance` VALUES ('%u', '%u', '%u', '%u')", GUID_LOPART(player->GetGroup()->GetLeaderGUID()), GetId(), InstanceId, GUID_LOPART(player->GetGroup()->GetLeaderGUID()));
            }
            sDatabase.CommitTransaction();
        }

        // bind instance to player (avoid duplicate binding)
        if (instantiator != player)
        {
            sDatabase.BeginTransaction();
            if (instantiator_online)
            {
                player->m_BoundInstances[GetId()] = std::pair< uint32, uint32 >(InstanceId, instantiator_id);
                sDatabase.PExecute("DELETE FROM `character_instance` WHERE (`guid` = '%u') AND (`map` = '%u')", player->GetGUIDLow(), GetId());
                sDatabase.PExecute("INSERT INTO `character_instance` VALUES ('%u', '%u', '%u', '%u')", player->GetGUIDLow(), GetId(), InstanceId, instantiator_id);
            }
            else
            {
                // the aforementioned "very special" case of leader being not online
                player->m_BoundInstances[GetId()] = std::pair< uint32, uint32 >(InstanceId, GUID_LOPART(player->GetGroup()->GetLeaderGUID()));
                sDatabase.PExecute("DELETE FROM `character_instance` WHERE (`guid` = '%u') AND (`map` = '%u')", player->GetGUIDLow(), GetId());
                sDatabase.PExecute("INSERT INTO `character_instance` VALUES ('%u', '%u', '%u', '%u')", player->GetGUIDLow(), GetId(), InstanceId, GUID_LOPART(player->GetGroup()->GetLeaderGUID()));
            }
            sDatabase.CommitTransaction();
        }
    }

    // set instance id for instantiating player
    if (player->GetPet()) player->GetPet()->SetInstanceId(InstanceId);
    player->SetInstanceId(InstanceId);

    return(map);
}

void MapInstanced::CreateInstance(uint32 InstanceId, Map* &map)
{
    if (!IsValidInstance(InstanceId))
    {
        // instance is invalid, need to verify map presence
        if (map)
        {
            // map for the instance is present, but the instance itself is no more valid, destroy map
            Guard guard(*this);

            map->Reset();
            InstancedMaps.erase(InstanceId);
            VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(GetId());
            delete map;
            map = NULL;
        }
    }

    // verify, if we have the map created, and create it if needed
    if (!map)
    {
        // map does not exist, create it
        Guard guard(*this);

        map = new Map(GetId(), GetGridExpiry(), InstanceId);
        InstancedMaps[InstanceId] = map;
    }
}

bool MapInstanced::IsValidInstance(uint32 InstanceId)
{
    // verify, if the map exists and needs to be reset
    Map* m = _FindMap(InstanceId);

    if (m && m->NeedsReset())
    {
        // check for real reset need (can only reset if no players)
        if (m->GetPlayersCount() == 0) return(false); // map exists, but needs reset
        // shift reset time of the map to a bit later
        m->InitResetTime();
    }

    // verify, if the map theoretically exists but not loaded
    QueryResult* result = sDatabase.PQuery("SELECT '1' FROM `instance` WHERE (`id` = '%u') AND (`map` = '%u') AND (`resettime` > " I64FMTD ")", InstanceId, GetId(), (uint64)time(NULL));
    if (result)
    {
        delete result;
        return(true);
    }

    // no map exists at all
    return(false);
}
