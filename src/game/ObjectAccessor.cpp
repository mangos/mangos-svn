
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


#define CLASS_LOCK MaNGOS::ClassLevelLockable<ObjectAccessor, ZThread::FastMutex>
INSTANTIATE_SINGLETON_2(ObjectAccessor, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(ObjectAccessor, ZThread::FastMutex);

#ifdef ENABLE_GRID_SYSTEM
Creature*
ObjectAccessor::GetCreature(Player &player, uint64 guid)
{
    return dynamic_cast<Creature *>(player.GetInRangeUnit(guid));
}

Corpse*
ObjectAccessor::GetCorpse(Player &player, uint64 guid)
{
    return dynamic_cast<Corpse *>(player.GetInRangeObject(guid));
}

Unit*
ObjectAccessor::GetUnit(Player &player, uint64 guid)
{
    //return player.GetInRangeUnit(guid);
	return dynamic_cast<Unit *>(player.GetInRangeUnit(guid)); // UQ1: ???
}

Player*
ObjectAccessor::GetPlayer(Player &player, uint64 guid)
{
    return dynamic_cast<Player *>(player.GetInRangeUnit(guid));
}

GameObject*
ObjectAccessor::GetGameObject(Player &player, uint64 guid)
{
    return dynamic_cast<GameObject *>(player.GetInRangeObject(guid));
}

DynamicObject*
ObjectAccessor::GetDynamicObject(Player &player, uint64 guid)
{
    return dynamic_cast<DynamicObject *>(player.GetInRangeObject(guid));
}
#endif

Player*
ObjectAccessor::FindPlayer(uint64 guid)
{
    for(PlayerMapType::iterator iter=i_players.begin(); iter != i_players.end(); ++iter)
    if( iter->second->GetGUID() == guid )
        return iter->second;
    return NULL;
}

Player*
ObjectAccessor::FindPlayerByName(const char *name) 
{
    for(PlayerMapType::iterator iter=i_players.begin(); iter != i_players.end(); ++iter)
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
    Guard guard(*this);
    PlayerMapType::iterator iter = i_players.find(pl->GetGUID());
    if( iter != i_players.end() )
    i_players.erase(iter);

    std::set<Object *>::iterator iter2 = std::find(i_objects.begin(), i_objects.end(), (Object *)pl);
    if( iter2 != i_objects.end() )
    i_objects.erase(iter2);
}

void
ObjectAccessor::Update()
{
    UpdateDataMapType update_players;
    {
    Guard guard(*this);    
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
    Guard guard(*this);
    i_objects.insert(obj);
}

void
ObjectAccessor::RemoveUpdateObject(Object *obj)
{
    Guard guard(*this);
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

    Guard guard(*this);
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
    _buildForInRangePlayer(obj, update_players);
    }

    if( pl == NULL )
    return;
#ifdef ENABLE_GRID_SYSTEM
    _buildPacket(pl, pl, update_players); // bulid myself for myself
    // update the in range players
    for(Player::InRangeUnitsMapType::iterator iter=pl->InRangeUnitsBegin(); iter != pl->InRangeUnitsEnd(); ++iter)
    {
    Player *for_pl = dynamic_cast<Player *>(iter->second);
    if( for_pl != NULL )
        _buildPacket(for_pl, pl, update_players); // build myself for my in range players
    }
#endif
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

void
ObjectAccessor::_buildForInRangePlayer(Object *obj, UpdateDataMapType &update_players)
{
#ifdef ENABLE_GRID_SYSTEM
    UpdateDataMapType::iterator pl_iter;
    for(PlayerMapType::iterator iter= i_players.begin(); iter != i_players.end(); ++ iter)
    {
    Unit *unit = dynamic_cast<Unit *>(obj);
    bool is_inrange = (unit == NULL ? iter->second->isInRange(obj) : iter->second->isInRange(unit));
    if( is_inrange )
    {
        pl_iter = update_players.find(iter->second);
        if( pl_iter == update_players.end() )
        {
        std::pair<UpdateDataMapType::iterator, bool> p = update_players.insert( UpdateDataValueType(iter->second, UpdateData()) );
        assert( p.second );
        pl_iter = p.first;
        }
        obj->BuildValuesUpdateBlockForPlayer(&pl_iter->second, pl_iter->first);        
    }
    }

    obj->ClearUpdateMask();
#endif
}


template void ObjectAccessor::RemoveUpdateObjects(std::map<OBJECT_HANDLE, GameObject *> &m);
template void ObjectAccessor::RemoveUpdateObjects(std::map<OBJECT_HANDLE, DynamicObject *> &m);
template void ObjectAccessor::RemoveUpdateObjects(std::map<OBJECT_HANDLE, Corpse *> &m);
template void ObjectAccessor::RemoveUpdateObjects(std::map<OBJECT_HANDLE, Creature *> &m);




