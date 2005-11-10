
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
    Player *pl = NULL;
    if( obj->isType(TYPE_PLAYER) )
    {
	pl = dynamic_cast<Player *>(obj);
    }
    else if( obj->isType(TYPE_ITEM ))
    {
	Item *item = dynamic_cast<Item *>(obj);
	assert( item != NULL );
	pl = item->GetOwner();
    }
    else if( obj->isType(TYPE_CONTAINER) )
    {
	Container *c = dynamic_cast<Container *>(obj);
	assert( c != NULL );
	pl = c->GetOwner();
    }
    else
    {
	_buildChangeObjectForPlayer(obj, update_players);
    }

    if( pl == NULL )
	return;

    _buildPacket(pl, pl, update_players); // bulid myself for myself

    CellPair p(MaNGOS::ComputeCellPair(pl->GetPositionX(), pl->GetPositionY()));
    Cell cell = RedZone::GetZone(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    MaNGOS::BuildUpdateForPlayer notifier(*pl, update_players); 
    TypeContainerVisitor<MaNGOS::BuildUpdateForPlayer, ContainerMapList<Player> > player_update(notifier);
    CellLock<GridReadGuard> cell_lock(cell, p); 
    cell_lock->Visit(cell_lock, player_update, *MapManager::Instance().GetMap(pl->GetMapId()));  
}

void
ObjectAccessor::_buildPacket(Player *pl, Player *bpl, UpdateDataMapType &update_players)
{
    UpdateDataMapType::iterator iter = update_players.find(pl);

    if( iter == update_players.end() )
    {
	std::pair<UpdateDataMapType::iterator, bool> p = update_players.insert( UpdateDataValueType(pl, UpdateData()) );
	assert(p.second);
	iter = p.first;
    }
    
    bpl->BuildValuesUpdateBlockForPlayer(&iter->second, iter->first);

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
    ObjectChangeAccumulator notifier(*obj, update_players);
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
	Guard guard(i_playerGuard);
	for(PlayersMapType::iterator iter=i_players.begin(); iter != i_players.end(); ++iter)
	{    
	    iter->second->Update(diff);
	    CellPair p(MaNGOS::ComputeCellPair(iter->second->GetPositionX(), iter->second->GetPositionY()));
	    Cell cell = RedZone::GetZone(p);
	    cell.data.Part.reserved = ALL_DISTRICT;
	    MaNGOS::ObjectUpdater updater(diff);
	    TypeContainerVisitor<MaNGOS::ObjectUpdater, TypeMapContainer<AllObjectTypes> > object_update(updater);	
	    CellLock<NullGuard> cell_lock(cell, p);
	    cell_lock->Visit(cell_lock, object_update, *MapManager::Instance().GetMap(iter->second->GetMapId()));	
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
    {
	UpdateDataMapType::iterator pl_iter = i_updateDatas.find(iter->second);
	if( pl_iter == i_updateDatas.end() )
	{
	    std::pair<UpdateDataMapType::iterator, bool> p = i_updateDatas.insert( UpdateDataValueType(iter->second, UpdateData()) );
	    assert( p.second );
	    pl_iter = p.first;
	}

	i_object.BuildValuesUpdateBlockForPlayer(&pl_iter->second, pl_iter->first);
    }
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







