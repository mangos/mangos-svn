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

#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Policies/SingletonImp.h"
#include "Player.h"
#include "Creature.h"
#include "GameObject.h"
#include "DynamicObject.h"
#include "Corpse.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "Item.h"
#include "Container.h"
#include "Corpse.h"
#include "RedZoneDistrict.h"
#include "GridNotifiers.h"
#include "MapManager.h"
#include "Map.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Opcodes.h"
#include "ObjectDefines.h"

#include <cmath>

#define CLASS_LOCK MaNGOS::ClassLevelLockable<ObjectAccessor, ZThread::FastMutex>
INSTANTIATE_SINGLETON_2(ObjectAccessor, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(ObjectAccessor, ZThread::FastMutex);

namespace MaNGOS
{

    struct MANGOS_DLL_DECL BuildUpdateForPlayer
    {
        Player &i_player;
        UpdateDataMapType &i_updatePlayers;

        BuildUpdateForPlayer(Player &player, UpdateDataMapType &data_map) : i_player(player), i_updatePlayers(data_map) {}

        void Visit(PlayerMapType &m)
        {
            for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
            {
                if( iter->second == &i_player )
                    continue;

                UpdateDataMapType::iterator iter2 = i_updatePlayers.find(iter->second);
                if( iter2 == i_updatePlayers.end() )
                {
                    std::pair<UpdateDataMapType::iterator, bool> p = i_updatePlayers.insert( ObjectAccessor::UpdateDataValueType(iter->second, UpdateData()) );
                    assert(p.second);
                    iter2 = p.first;
                }

                i_player.BuildValuesUpdateBlockForPlayer(&iter2->second, iter2->first);
            }
        }

        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, SKIP *> &) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &) {}
    };
}

ObjectAccessor::ObjectAccessor() {}
ObjectAccessor::~ObjectAccessor() {}

Creature*
ObjectAccessor::GetNPCIfCanInteractWith(Player const &player, uint64 guid, uint32 npcflagmask)
{
    // unit checks
    if (!guid)
        return NULL;

    // exist
    Creature *unit = GetCreature(player, guid);
    if (!unit)
        return NULL;

    // player check
    if(!player.CanInteractWithNPCs(!unit->isSpiritService()))
        return NULL;

    // appropriate npc type
    if(npcflagmask && !unit->HasFlag( UNIT_NPC_FLAGS, npcflagmask ))
        return NULL;

    // alive or spirit healer
    if(!unit->isAlive() && (!unit->isSpiritService() || player.isAlive() ))
        return NULL;

    // not enemy
    if( unit->IsHostileTo(&player))
        return NULL;

    // not too far
    if(!unit->IsWithinDistInMap(&player,INTERACTION_DISTANCE))
        return NULL;

    return unit;
}

Creature*
ObjectAccessor::GetCreatureOrPet(WorldObject const &u, uint64 guid)
{
    if(Creature *unit = GetPet(guid))
        return unit;

    return GetCreature(u, guid);
}

Creature*
ObjectAccessor::GetCreature(WorldObject const &u, uint64 guid)
{
    return MapManager::Instance().GetMap(u.GetMapId(), &u)->GetObjectNear<Creature>(u, guid, (Creature*)NULL);
}

Unit*
ObjectAccessor::GetUnit(WorldObject const &u, uint64 guid)
{
    if(GUID_HIPART(guid)==HIGHGUID_PLAYER)
        return FindPlayer(guid);

    return GetCreature(u, guid);
}

CorpsePtr&
ObjectAccessor::GetCorpse(WorldObject const &u, uint64 guid)
{
    return MapManager::Instance().GetMap(u.GetMapId(), &u)->GetObjectNear<Corpse>(u, guid);
}

Object* ObjectAccessor::GetObjectByTypeMask(Player const &p, uint64 guid, uint32 typemask)
{
    Object *obj = NULL;

    if(typemask & TYPE_PLAYER)
    {
        obj = FindPlayer(guid);
        if(obj) return obj;
    }

    if(typemask & TYPE_UNIT)
    {
        obj = GetCreatureOrPet(p,guid);
        if(obj) return obj;
    }

    if(typemask & TYPE_GAMEOBJECT)
    {
        obj = GetGameObject(p,guid);
        if(obj) return obj;
    }

    if(typemask & TYPE_DYNAMICOBJECT)
    {
        obj = GetDynamicObject(p,guid);
        if(obj) return obj;
    }

    if(typemask & TYPE_ITEM)
    {
        obj = p.GetItemByGuid( guid );
        if(obj) return obj;
    }

    return NULL;
}

GameObject*
ObjectAccessor::GetGameObject(Unit const &u, uint64 guid)
{
    return MapManager::Instance().GetMap(u.GetMapId(), &u)->GetObjectNear<GameObject>(u, guid, (GameObject*)NULL);
}

DynamicObject*
ObjectAccessor::GetDynamicObject(Unit const &u, uint64 guid)
{
    return MapManager::Instance().GetMap(u.GetMapId(), &u)->GetObjectNear<DynamicObject>(u, guid, (DynamicObject*)NULL);
}

Player*
ObjectAccessor::FindPlayer(uint64 guid)
{
    PlayersMapType::iterator itr = i_players.find(guid);
    if (itr != i_players.end())
        return itr->second;
    return NULL;
}

Player*
ObjectAccessor::FindPlayerByName(const char *name)
{
    for(PlayersMapType::iterator iter=i_players.begin(); iter != i_players.end(); ++iter)
        if( ::strcmp(name, iter->second->GetName()) == 0 )
            return iter->second;
    return NULL;
}

void
ObjectAccessor::InsertPlayer(Player *pl)
{
    i_players[pl->GetGUID()] = pl;
}

void
ObjectAccessor::RemovePlayer(Player *pl)
{
    Guard guard(i_playerGuard);
    PlayersMapType::iterator iter = i_players.find(pl->GetGUID());
    if( iter != i_players.end() )
        i_players.erase(iter);

    std::set<Object *>::iterator iter2 = std::find(i_objects.begin(), i_objects.end(), (Object *)pl);
    if( iter2 != i_objects.end() )
        i_objects.erase(iter2);
}

void
ObjectAccessor::SaveAllPlayers()
{
    for(PlayersMapType::iterator itr = i_players.begin(); itr != i_players.end(); ++itr)
        itr->second->SaveToDB();
}

void
ObjectAccessor::_update()
{
    UpdateDataMapType update_players;
    {
        Guard guard(i_updateGuard);
        for(std::set<Object *>::iterator iter=i_objects.begin(); iter != i_objects.end(); ++iter)
        {
            // check for valid pointer
            if (!*iter)
                continue;
            _buildUpdateObject(*iter, update_players);
            (*iter)->ClearUpdateMask(false);
        }
        i_objects.clear();
    }

    WorldPacket packet; // here we allocate a std::vector with a size of 0x10000
    for(UpdateDataMapType::iterator iter = update_players.begin(); iter != update_players.end(); ++iter)
    {
        iter->second.BuildPacket(&packet);
        iter->first->GetSession()->SendPacket(&packet);
        packet.clear(); // clean the string
    }
}

void
ObjectAccessor::UpdateObject(Object* obj, Player* exceptPlayer)
{
    UpdateDataMapType update_players;
    obj->BuildUpdate(update_players);

    WorldPacket packet;
    for(UpdateDataMapType::iterator iter = update_players.begin(); iter != update_players.end(); ++iter)
    {
        if(iter->first == exceptPlayer)
            continue;

        iter->second.BuildPacket(&packet);
        iter->first->GetSession()->SendPacket(&packet);
        packet.clear();
    }
}

void
ObjectAccessor::AddUpdateObject(Object *obj)
{
    Guard guard(i_updateGuard);
    i_objects.insert(obj);
}

void
ObjectAccessor::RemoveUpdateObject(Object *obj)
{
    Guard guard(i_updateGuard);
    std::set<Object *>::iterator iter = i_objects.find(obj);
    if( iter != i_objects.end() )
        i_objects.erase( iter );
}

void ObjectAccessor::AddObjectToRemoveList(WorldObject *obj)
{
    if(!obj) return;

    Guard guard(i_removeGuard);
    i_objectsToRemove.insert(obj);
    //sLog.outDebug("Object (GUID: %u TypeId: %u ) added to removing list.",obj->GetGUIDLow(),obj->GetTypeId());
}

void ObjectAccessor::DoDelayedMovesAndRemoves()
{
    MapManager::Instance().MoveAllCreaturesInMoveList();
    RemoveAllObjectsInRemoveList();
}

void ObjectAccessor::RemoveAllObjectsInRemoveList()
{
    if(i_objectsToRemove.empty())
        return;

    Guard guard(i_removeGuard);
    //sLog.outDebug("Object remover 1 check.");
    while(!i_objectsToRemove.empty())
    {
        WorldObject* obj = *i_objectsToRemove.begin();
        i_objectsToRemove.erase(i_objectsToRemove.begin());
        switch(obj->GetTypeId())
        {
            case TYPEID_CORPSE:
            {
                CorpsePtr corpse = MapManager::Instance().GetMap(obj->GetMapId(), obj)->GetObjectNear<Corpse>(*obj, obj->GetGUID());
                if (!corpse)
                {
                    sLog.outError("ERROR: Try delete corpse/bones %u that not in map", obj->GetGUIDLow());
                }
                else
                {
                    MapManager::Instance().GetMap(obj->GetMapId(), obj)->Remove(corpse,true);
                }
                break;
            }
            case TYPEID_DYNAMICOBJECT:
                MapManager::Instance().GetMap(obj->GetMapId(), obj)->Remove((DynamicObject*)obj,true);
                break;
            case TYPEID_GAMEOBJECT:
                MapManager::Instance().GetMap(obj->GetMapId(), obj)->Remove((GameObject*)obj,true);
                break;
            case TYPEID_UNIT:
                MapManager::Instance().GetMap(obj->GetMapId(), obj)->Remove((Creature*)obj,true);
                break;
            default:
                sLog.outError("Non-grid object (TypeId: %u) in grid object removing list, ignored.",obj->GetTypeId());
                break;
        }
    }
    //sLog.outDebug("Object remover 2 check.");
}

void
ObjectAccessor::_buildUpdateObject(Object *obj, UpdateDataMapType &update_players)
{
    bool build_for_all = true;
    Player *pl = NULL;
    if( obj->isType(TYPE_ITEM ))
    {
        Item *item = static_cast<Item *>(obj);
        pl = item->GetOwner();
        build_for_all = false;
    }
    else if( obj->isType(TYPE_CONTAINER) )
    {
        Container *c = static_cast<Container *>(obj);
        assert( c != NULL );
        pl = c->GetOwner();
        build_for_all = false;
    }

    if( pl != NULL )
        _buildPacket(pl, obj, update_players);

    // Capt: okey for all those fools who think its a real fix
    //       THIS IS A TEMP FIX
    if( build_for_all )
    {
        WorldObject * temp = dynamic_cast<WorldObject*>(obj);

        //assert(dynamic_cast<WorldObject*>(obj)!=NULL);
        if (temp)
            _buildChangeObjectForPlayer(temp, update_players);
        else
            sLog.outDebug("ObjectAccessor: Ln 405 Temp bug fix");
    }
}

void
ObjectAccessor::_buildPacket(Player *pl, Object *obj, UpdateDataMapType &update_players)
{
    UpdateDataMapType::iterator iter = update_players.find(pl);

    if( iter == update_players.end() )
    {
        std::pair<UpdateDataMapType::iterator, bool> p = update_players.insert( UpdateDataValueType(pl, UpdateData()) );
        assert(p.second);
        iter = p.first;
    }

    obj->BuildValuesUpdateBlockForPlayer(&iter->second, iter->first);
}

void
ObjectAccessor::_buildChangeObjectForPlayer(WorldObject *obj, UpdateDataMapType &update_players)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    Cell cell = RedZone::GetZone(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();
    WorldObjectChangeAccumulator notifier(*obj, update_players);
    TypeContainerVisitor<WorldObjectChangeAccumulator, WorldTypeMapContainer > player_notifier(notifier);
    CellLock<GridReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(obj->GetMapId(), obj));
}

Pet*
ObjectAccessor::GetPet(uint64 guid)
{
    Guard guard(i_petGuard);

    PetsMapType::iterator iter = i_pets.find(guid);
    if( iter == i_pets.end() ) return NULL;

    return iter->second;
}

void
ObjectAccessor::RemovePet(Pet *pet)
{
    Guard guard(i_petGuard);
    PetsMapType::iterator iter = i_pets.find(pet->GetGUID());
    if( iter != i_pets.end() )
        i_pets.erase(iter);
}

void
ObjectAccessor::AddPet(Pet *pet)
{
    Guard guard(i_petGuard);
    assert(i_pets.find(pet->GetGUID()) == i_pets.end());
    i_pets[pet->GetGUID()] = pet;
}

CorpsePtr&
ObjectAccessor::GetCorpseForPlayerGUID(uint64 guid)
{
    Guard guard(i_corpseGuard);

    Player2CorpsesMapType::iterator iter = i_player2corpse.find(guid);
    if( iter == i_player2corpse.end() ) return NullPtr<Corpse>((Corpse*)NULL);

    assert(iter->second->GetType() == CORPSE_RESURRECTABLE);

    return iter->second;
}

void
ObjectAccessor::RemoveCorpse(Corpse *corpse)
{
    assert(corpse && corpse->GetType() == CORPSE_RESURRECTABLE);

    Guard guard(i_corpseGuard);
    Player2CorpsesMapType::iterator iter = i_player2corpse.find(corpse->GetOwnerGUID());
    if( iter == i_player2corpse.end() )
        return;

    // build mapid*cellid -> guid_set map
    CellPair cell_pair = MaNGOS::ComputeCellPair(corpse->GetPositionX(), corpse->GetPositionY());
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    objmgr.DeleteCorpseCellData(corpse->GetMapId(),cell_id,corpse->GetOwnerGUID());

    i_player2corpse.erase(iter);
}

void
ObjectAccessor::AddCorpse(CorpsePtr &corpse)
{
    assert(corpse && corpse->GetType() == CORPSE_RESURRECTABLE);

    Guard guard(i_corpseGuard);
    assert(i_player2corpse.find(corpse->GetOwnerGUID()) == i_player2corpse.end());
    i_player2corpse[corpse->GetOwnerGUID()] = corpse;

    // build mapid*cellid -> guid_set map
    CellPair cell_pair = MaNGOS::ComputeCellPair(corpse->GetPositionX(), corpse->GetPositionY());
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    objmgr.AddCorpseCellData(corpse->GetMapId(),cell_id,corpse->GetOwnerGUID(),corpse->GetInstanceId());
}

void
ObjectAccessor::AddCorpsesToGrid(GridPair const& gridpair,GridType& grid,Map* map)
{
    Guard guard(i_corpseGuard);
    for(Player2CorpsesMapType::iterator iter = i_player2corpse.begin(); iter != i_player2corpse.end(); ++iter)
        if(iter->second->GetGrid()==gridpair)
        {
            // verify, if the corpse in our instance (add only corpses which are)
            if (map->Instanceable())
            {
                if (iter->second->GetInstanceId() == map->GetInstanceId())
                {
                    grid.AddWorldObject(iter->second,iter->second->GetGUID());
                }
            }
            else
            {
                grid.AddWorldObject(iter->second,iter->second->GetGUID());
            }
        }
}

bool
ObjectAccessor::ConvertCorpseForPlayer(uint64 player_guid)
{
//    Guard guard(i_corpseGuard);

    Player2CorpsesMapType::iterator iter = i_player2corpse.find(player_guid);

    // corpse can be converted in another thread already
    if( iter == i_player2corpse.end() )
        return false;

    CorpsePtr corpse = iter->second;

    // remove corpse from player_guid -> corpse map
    // i_player2corpse.erase(iter);

    corpse->_ConvertCorpseToBones();

    return true;
}

void
ObjectAccessor::Update(const uint32  &diff)
{
    {
        typedef std::multimap<uint32, Player *> CreatureLocationHolderType;
        CreatureLocationHolderType creature_locations;
        Guard guard(i_playerGuard);
        for(PlayersMapType::iterator iter=i_players.begin(); iter != i_players.end(); ++iter)
        {
            iter->second->Update(diff);
            creature_locations.insert( CreatureLocationHolderType::value_type(iter->second->GetMapId(), iter->second) );
        }

        Map *map;
        uint32 instance_id = 0;
        MaNGOS::ObjectUpdater updater(diff);
        // for creature
        TypeContainerVisitor<MaNGOS::ObjectUpdater, GridTypeMapContainer  > grid_object_update(updater);
        // for pets
        TypeContainerVisitor<MaNGOS::ObjectUpdater, WorldTypeMapContainer > world_object_update(updater);

        for(CreatureLocationHolderType::iterator iter=creature_locations.begin(); iter != creature_locations.end(); ++iter)
        {
            MapManager::Instance().GetMap((*iter).first, (*iter).second)->marked_cells.reset();
        }

        for(CreatureLocationHolderType::iterator iter=creature_locations.begin(); iter != creature_locations.end(); ++iter)
        {
            Player *player = (*iter).second;
            map = MapManager::Instance().GetMap((*iter).first, player);

            CellPair standing_cell(MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY()));
            CellPair update_cell(standing_cell);
            update_cell << 1;
            update_cell -= 1;

            for(; abs(int(standing_cell.x_coord - update_cell.x_coord)) < 2; update_cell >> 1)
            {
                for(CellPair cell_iter=update_cell; abs(int(standing_cell.y_coord - cell_iter.y_coord)) < 2; cell_iter += 1)
                {
                    uint32 cell_id = (cell_iter.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_iter.x_coord;
                    if( !map->marked_cells.test(cell_id) )
                    {
                        map->marked_cells.set(cell_id);
                        Cell cell = RedZone::GetZone(cell_iter);
                        cell.data.Part.reserved = CENTER_DISTRICT;
                        cell.SetNoCreate();
                        CellLock<NullGuard> cell_lock(cell, cell_iter);
                        cell_lock->Visit(cell_lock, grid_object_update,  *map);
                        cell_lock->Visit(cell_lock, world_object_update, *map);
                    }

                    if (cell_iter.y_coord == TOTAL_NUMBER_OF_CELLS_PER_MAP-1)
                        break;
                }

                if (update_cell.x_coord == TOTAL_NUMBER_OF_CELLS_PER_MAP-1)
                    break;
            }
        }

    }

    _update();
}

bool
ObjectAccessor::PlayersNearGrid(const uint32 &x, const uint32 &y, const uint32 &m_id, const uint32 &i_id) const
{
    CellPair cell_min(x*MAX_NUMBER_OF_CELLS, y*MAX_NUMBER_OF_CELLS);
    CellPair cell_max(cell_min.x_coord + MAX_NUMBER_OF_CELLS, cell_min.y_coord+MAX_NUMBER_OF_CELLS);
    cell_min << 2;
    cell_min -= 2;
    cell_max >> 2;
    cell_max += 2;

    Guard guard(const_cast<ObjectAccessor *>(this)->i_playerGuard);
    for(PlayersMapType::const_iterator iter=i_players.begin(); iter != i_players.end(); ++iter)
    {
        if( m_id != iter->second->GetMapId() || i_id != iter->second->GetInstanceId() )
            continue;

        CellPair p = MaNGOS::ComputeCellPair(iter->second->GetPositionX(), iter->second->GetPositionY());
        if( (cell_min.x_coord <= p.x_coord && p.x_coord <= cell_max.x_coord) &&
            (cell_min.y_coord <= p.y_coord && p.y_coord <= cell_max.y_coord) )
            return true;
    }

    return false;
}

void
ObjectAccessor::WorldObjectChangeAccumulator::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        if(iter->second->HaveAtClient(&i_object))
            ObjectAccessor::_buildPacket(iter->second, &i_object, i_updateDatas);
}

void
ObjectAccessor::UpdateObjectVisibility(WorldObject *obj)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    Cell cell = RedZone::GetZone(p);

    MapManager::Instance().GetMap(obj->GetMapId(), obj)->UpdateObjectVisibility(obj,cell,p);
}

void ObjectAccessor::UpdateVisibilityForPlayer( Player* player )
{
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    Cell cell = RedZone::GetZone(p);
    Map* m = MapManager::Instance().GetMap(player->GetMapId(),player);

    m->UpdatePlayerVisibility(player,cell,p);
    m->UpdateObjectsVisibilityFor(player,cell,p);
}
