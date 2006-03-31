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

#ifndef MANGOS_OBJECTACCESSOR_H
#define MANGOS_OBJECTACCESSOR_H

#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "zthread/FastMutex.h"

#include "ByteBuffer.h"
#include "UpdateData.h"

#include <set>

class Creature;
class Player;
class Corpse;
class Unit;
class GameObject;
class DynamicObject;
class Object;
	    

class MANGOS_DLL_DECL ObjectAccessor : public MaNGOS::Singleton<ObjectAccessor, MaNGOS::ClassLevelLockable<ObjectAccessor, ZThread::FastMutex> >
{

    friend class MaNGOS::OperatorNew<ObjectAccessor>;
    ObjectAccessor() {}
    ObjectAccessor(const ObjectAccessor &);
    ObjectAccessor& operator=(const ObjectAccessor &);

public:

    typedef HM_NAMESPACE::hash_map<uint64, Player* > PlayersMapType;  
    typedef HM_NAMESPACE::hash_map<uint64, Corpse* > CorpsesMapType;  
    typedef HM_NAMESPACE::hash_map<Player*, UpdateData> UpdateDataMapType;  
    typedef HM_NAMESPACE::hash_map<Player*, UpdateData>::value_type UpdateDataValueType;  


    Creature* GetCreature(Unit &, uint64);
    Corpse* GetCorpse(Unit &, uint64);
    Unit* GetUnit(Unit &, uint64);
    Player* GetPlayer(Unit &, uint64);
    GameObject* GetGameObject(Unit &, uint64);
    DynamicObject* GetDynamicObject(Unit &, uint64);

    
    Player* FindPlayer(uint64);
    Player* FindPlayerByName(const char *name) ;
    void BuildCreateForSameMapPlayer(Player *pl);

    inline PlayersMapType& GetPlayers(void) { return i_players; }
    void InsertPlayer(Player *);
    void RemovePlayer(Player *);

    void AddUpdateObject(Object *obj);
    void RemoveUpdateObject(Object *obj);
    void RemoveCreatureCorpseFromPlayerView(Creature *);

    
    void Update(const uint32 &diff);

    
    Corpse* GetCorpseForPlayer(Player &);
    void RemoveCorpse(uint64);
    void AddCorpse(Corpse *corpse);

    
    bool PlayersNearGrid(const uint32 &x, const uint32 &y, const uint32 &) const;

    template<class T> void RemoveUpdateObjects(std::map<OBJECT_HANDLE, T *> &);

#ifdef WIN32
	
	template<> void RemoveUpdateObjects(std::map<OBJECT_HANDLE, Corpse *> &);
#endif
private:

    struct ObjectChangeAccumulator
    {
	UpdateDataMapType &i_updateDatas;
	Object &i_object;
	ObjectAccessor &i_accessor;
	ObjectChangeAccumulator(Object &obj, UpdateDataMapType &d, ObjectAccessor &a) : i_updateDatas(d), i_object(obj), i_accessor(a) {}
	void Visit(std::map<OBJECT_HANDLE, Player *> &);
    };

    friend struct ObjectChangeAccumulator; 
    PlayersMapType i_players;
    CorpsesMapType i_corpse;

    typedef ZThread::FastMutex LockType;
    typedef MaNGOS::GeneralLock<LockType > Guard;
    
    void _buildChangeObjectForPlayer(Object *, UpdateDataMapType &);
    void _buildUpdateObject(Object *, UpdateDataMapType &);
    void _buildPacket(Player *, Object *, UpdateDataMapType &);
    void _update(void);
    std::set<Object *> i_objects;
    LockType i_playerGuard;
    LockType i_updateGuard;
    LockType i_corpseGuard;
};

#ifndef WIN32
template<> void ObjectAccessor::RemoveUpdateObjects(std::map<OBJECT_HANDLE, Corpse *> &);
#endif

namespace MaNGOS
{
    
    struct MANGOS_DLL_DECL BuildUpdateForPlayer
    {
		Player &i_player;
		ObjectAccessor::UpdateDataMapType &i_updatePlayers;
		BuildUpdateForPlayer(Player &player, ObjectAccessor::UpdateDataMapType &data_map) : i_player(player), i_updatePlayers(data_map) {}
		void Visit(std::map<OBJECT_HANDLE, Player *> &);
    };

    
    struct MANGOS_DLL_DECL CreatureCorpseViewRemover
    {
	Creature &i_creature;
	CreatureCorpseViewRemover(Creature &c) : i_creature(c) {}
	void Visit(std::map<OBJECT_HANDLE, Player *>  &);
    };
}



#endif
