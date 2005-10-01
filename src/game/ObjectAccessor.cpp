
#include "ObjectAccessor.h"
#include "Policies/SingletonImp.h"
#include "Player.h"
#include "Creature.h"
#include "GameObject.h"
#include "DynamicObject.h"
#include "Corpse.h"

#ifdef ENABLE_GRID_SYSTEM

#define CLASS_LOCK MaNGOS::ClassLevelLockable<ObjectAccessor, ZThread::FastMutex>
INSTANTIATE_SINGLETON_2(ObjectAccessor, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(ObjectAccessor, ZThread::FastMutex);

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
    return player.GetInRangeUnit(guid);
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
    PlayerMapType::iterator iter = i_players.find(pl->GetGUID());
    if( iter != i_players.end() )
	i_players.erase(iter);
}

void
ObjectAccessor::Update()
{
}

#endif
