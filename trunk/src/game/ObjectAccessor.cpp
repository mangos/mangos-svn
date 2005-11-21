
#include "ObjectAccessor.h"
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
#include "RedZoneDistrict.h"
#include "GridNotifiers.h"
#include "MapManager.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Opcodes.h"

#include <cmath>
#include <bitset>

#define CLASS_LOCK MaNGOS::ClassLevelLockable<ObjectAccessor, ZThread::FastMutex>
INSTANTIATE_SINGLETON_2(ObjectAccessor, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(ObjectAccessor, ZThread::FastMutex);

Creature*
ObjectAccessor::GetCreature(Player &player, uint64 guid)
{
    CellPair p(MaNGOS::ComputeCellPair(player.GetPositionX(), player.GetPositionY()));
    Cell cell = RedZone::GetZone(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    Creature *obj=NULL; 
    MaNGOS::ObjectAccessorNotifier<Creature> searcher(obj, guid);
    TypeContainerVisitor<MaNGOS::ObjectAccessorNotifier<Creature>, TypeMapContainer<AllObjectTypes> > object_notifier(searcher);
    CellLock<GridReadGuard> cell_lock(cell, p); 
    cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(player.GetMapId()));
    return obj;
}

Corpse*
ObjectAccessor::GetCorpse(Player &player, uint64 guid)
{
    CellPair p(MaNGOS::ComputeCellPair(player.GetPositionX(), player.GetPositionY()));
    Cell cell = RedZone::GetZone(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    Corpse *obj=NULL; 
    MaNGOS::ObjectAccessorNotifier<Corpse> searcher(obj, guid);
    TypeContainerVisitor<MaNGOS::ObjectAccessorNotifier<Corpse>, TypeMapContainer<AllObjectTypes> > object_notifier(searcher);
    CellLock<GridReadGuard> cell_lock(cell, p); 
    cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(player.GetMapId()));
	return obj; 
}

Unit*
ObjectAccessor::GetUnit(Player &player, uint64 guid)
{
    Unit *unit = FindPlayer(guid);
    if( unit != NULL )
	return unit;

    return GetCreature(player, guid);
}

Player*
ObjectAccessor::GetPlayer(Player &player, uint64 guid)
{
    return FindPlayer(guid);
}

GameObject*
ObjectAccessor::GetGameObject(Player &player, uint64 guid)
{
    CellPair p(MaNGOS::ComputeCellPair(player.GetPositionX(), player.GetPositionY()));
    Cell cell = RedZone::GetZone(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    GameObject *obj=NULL;
    MaNGOS::ObjectAccessorNotifier<GameObject> searcher(obj, guid);
    TypeContainerVisitor<MaNGOS::ObjectAccessorNotifier<GameObject>, TypeMapContainer<AllObjectTypes> > object_notifier(searcher);
    CellLock<GridReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(player.GetMapId()));
    return obj;
}

DynamicObject*
ObjectAccessor::GetDynamicObject(Player &player, uint64 guid)
{
    CellPair p(MaNGOS::ComputeCellPair(player.GetPositionX(), player.GetPositionY()));
    Cell cell = RedZone::GetZone(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    DynamicObject *obj=NULL;
    MaNGOS::ObjectAccessorNotifier<DynamicObject> searcher(obj, guid);
    TypeContainerVisitor<MaNGOS::ObjectAccessorNotifier<DynamicObject>, TypeMapContainer<AllObjectTypes> > object_notifier(searcher);
    CellLock<GridReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(player.GetMapId()));
    return obj;
}

Player*
ObjectAccessor::FindPlayer(uint64 guid)
{
    for(PlayersMapType::iterator iter=i_players.begin(); iter != i_players.end(); ++iter)
    if( iter->second->GetGUID() == guid )
        return iter->second;
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
ObjectAccessor::_update()
{
    UpdateDataMapType update_players;
    {
	Guard guard(i_updateGuard);    
	for(std::set<Object *>::iterator iter=i_objects.begin(); iter != i_objects.end(); ++iter)
	{
	    _buildUpdateObject(*iter, update_players);
	    (*iter)->ClearUpdateMask();
	}
	i_objects.clear();
    }
    
    for(UpdateDataMapType::iterator iter = update_players.begin(); iter != update_players.end(); ++iter)
    {
	WorldPacket packet;
	iter->second.BuildPacket(&packet);
	iter->first->GetSession()->SendPacket(&packet);
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


template<class T>
void
ObjectAccessor::RemoveUpdateObjects(std::map<OBJECT_HANDLE, T *> &m)
{
    if( m.size() == 0 )
    return;

    Guard guard(i_updateGuard);
    for(typename std::map<OBJECT_HANDLE, T *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
    std::set<Object *>::iterator obj = i_objects.find(iter->second);
    if( obj != i_objects.end() )
        i_objects.erase( obj );
    }
}

void
ObjectAccessor::_buildUpdateObject(Object *obj, UpdateDataMapType &update_players)
{
    bool build_for_all = true;
    Player *pl = NULL;
    if( obj->isType(TYPE_ITEM ))
    {
	Item *item = static_cast<Item *>(obj);
	item->UpdateStats();
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

    /* else then the object is either a creature or player.. now build for all
     */
    if( pl != NULL )
	_buildPacket(pl, obj, update_players);

    // build this object for its surrounding players
    if( build_for_all )
	_buildChangeObjectForPlayer(obj, update_players);
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

Corpse*
ObjectAccessor::GetCorpseForPlayer(Player &player)
{
    Guard guard(i_corpseGuard);
    const uint64 guid(player.GetGUID());
    for(CorpsesMapType::iterator iter=i_corpse.begin(); iter != i_corpse.end(); ++iter)
	if(iter->second->GetUInt64Value(CORPSE_FIELD_OWNER) == guid)
	    return iter->second;
    return NULL;
}


void
ObjectAccessor::_buildChangeObjectForPlayer(Object *obj, UpdateDataMapType &update_players)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    Cell cell = RedZone::GetZone(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();
    ObjectChangeAccumulator notifier(*obj, update_players, *this);
    TypeContainerVisitor<ObjectChangeAccumulator, ContainerMapList<Player> > player_notifier(notifier);
    CellLock<GridReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(obj->GetMapId()));
    obj->ClearUpdateMask();
}


void
ObjectAccessor::RemoveCorpse(uint64 guid)
{
    Guard guard(i_corpseGuard);
    CorpsesMapType::iterator iter = i_corpse.find(guid);
    if( iter != i_corpse.end() )
	i_corpse.erase(iter);
}

void
ObjectAccessor::AddCorpse(Corpse *corpse)
{
    Guard guard(i_corpseGuard);
    assert(i_corpse.find(corpse->GetGUID()) == i_corpse.end());
    i_corpse[corpse->GetGUID()] = corpse;
}

void
ObjectAccessor::Update(const uint32  &diff)
{
    { // don't remove scope braces
	typedef std::multimap<uint32, Player *> CreatureLocationHolderType;
	CreatureLocationHolderType creature_locations;
	Guard guard(i_playerGuard);
	for(PlayersMapType::iterator iter=i_players.begin(); iter != i_players.end(); ++iter)
	{    
	    iter->second->Update(diff);
	    creature_locations.insert( CreatureLocationHolderType::value_type(iter->second->GetMapId(), iter->second) );
	}

	
	/* now update the creatures near the player. Note,
	 * we only update the creatures on and adjecent cells
	 * to the player's standing.  Due to the size of the cell
	 * division, any creature beyon that position will
	 * not be visually visible on the player's monitor.
	 */
	uint32 map_id = 0;
	MaNGOS::ObjectUpdater updater(diff);
	TypeContainerVisitor<MaNGOS::ObjectUpdater, TypeMapContainer<AllObjectTypes> > object_update(updater);	
	std::bitset<TOTAL_NUMBER_OF_CELLS_PER_MAP*TOTAL_NUMBER_OF_CELLS_PER_MAP> marked_cell(0);
	for(CreatureLocationHolderType::iterator iter=creature_locations.begin(); iter != creature_locations.end(); ++iter)
	{
	    if( map_id != (*iter).first )
	    {
		map_id = (*iter).first;
		marked_cell.reset(); // new map
	    }

	    Player *player = (*iter).second;
	    CellPair standing_cell(MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY()));
	    CellPair update_cell(standing_cell);
	    update_cell << 1;
	    update_cell -= 1;

	    for(; abs(int(standing_cell.x_coord - update_cell.x_coord)) < 2; update_cell >> 1)
	    {
		for(CellPair cell_iter=update_cell; abs(int(standing_cell.y_coord - cell_iter.y_coord)) < 2; cell_iter += 1)
		{		    
		    uint32 cell_id = (cell_iter.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_iter.x_coord;
		    if( !marked_cell.test(cell_id) )
		    {
			marked_cell.set(cell_id);
			Cell cell = RedZone::GetZone(cell_iter);
			cell.data.Part.reserved = CENTER_DISTRICT;
			cell.SetNoCreate();
			CellLock<NullGuard> cell_lock(cell, cell_iter);
			cell_lock->Visit(cell_lock, object_update, *MapManager::Instance().GetMap(map_id));
		    }
		}
	    }	    
	}
	
    }

    _update();
}

bool 
ObjectAccessor::PlayersNearGrid(const uint32 &x, const uint32 &y, const uint32 &m_id) const
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
	if( m_id != iter->second->GetMapId() )
	    continue;

	CellPair p = MaNGOS::ComputeCellPair(iter->second->GetPositionX(), iter->second->GetPositionY());
	if( (cell_min.x_coord <= p.x_coord && p.x_coord <= cell_max.x_coord) &&
	    (cell_min.y_coord <= p.y_coord && p.y_coord <= cell_max.y_coord) )
	    return true;
    }

    return false;
}

// last the notifier
void
ObjectAccessor::ObjectChangeAccumulator::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    for(std::map<OBJECT_HANDLE, Player *>::iterator iter = m.begin(); iter != m.end(); ++iter)
	i_accessor._buildPacket(iter->second, &i_object, i_updateDatas);
}

namespace MaNGOS
{
    //====================================//
    // BuildUpdateForPlayer
    void
    BuildUpdateForPlayer::Visit(PlayerMapType &m)
    {
	for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
	{
	    if( iter->second == &i_player )
		continue;
	    
	    ObjectAccessor::UpdateDataMapType::iterator iter2 = i_updatePlayers.find(iter->second);
	    if( iter2 == i_updatePlayers.end() )
	    {
		std::pair<ObjectAccessor::UpdateDataMapType::iterator, bool> p = i_updatePlayers.insert( ObjectAccessor::UpdateDataValueType(iter->second, UpdateData()) );
		assert(p.second);
		iter2 = p.first;
	    }
	    
	    // build myself for other player
	    i_player.BuildValuesUpdateBlockForPlayer(&iter2->second, iter2->first);
	}
    }
}

template void ObjectAccessor::RemoveUpdateObjects(std::map<OBJECT_HANDLE, GameObject *> &m);
template void ObjectAccessor::RemoveUpdateObjects(std::map<OBJECT_HANDLE, DynamicObject *> &m);
template void ObjectAccessor::RemoveUpdateObjects(std::map<OBJECT_HANDLE, Creature *> &m);







