/* ObjectAccessor.h
 *
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

/** ObjectAccessor is a singleton that helps to access either player or 
 * objects or units in the line-of-sight of the player
 */

#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "zthread/FastMutex.h"
//#include "Common.h"

#include "ByteBuffer.h"
#include "UpdateData.h"

#include <set>

class Creature;
class Player;
class Corpse;
class Unit;
class GameObject;
class DynamicObject;
class Corpse;
class Object;


class MANGOS_DLL_DECL ObjectAccessor : public MaNGOS::Singleton<ObjectAccessor, MaNGOS::ClassLevelLockable<ObjectAccessor, ZThread::FastMutex> >
{

    friend class MaNGOS::OperatorNew<ObjectAccessor>;
    ObjectAccessor() {}
    ObjectAccessor(const ObjectAccessor &);
    ObjectAccessor& operator=(const ObjectAccessor &);

public:

    typedef HM_NAMESPACE::hash_map<uint64, Player* > PlayerMapType;  
    typedef HM_NAMESPACE::hash_map<uint64, Corpse* > CorpseMapType;  
    typedef HM_NAMESPACE::hash_map<Player*, UpdateData> UpdateDataMapType;  
    typedef HM_NAMESPACE::hash_map<Player*, UpdateData>::value_type UpdateDataValueType;  


    Creature* GetCreature(Player &, uint64);
    Corpse* GetCorpse(Player &, uint64);
    Unit* GetUnit(Player &, uint64);
    Player* GetPlayer(Player &, uint64);
    GameObject* GetGameObject(Player &, uint64);
    DynamicObject* GetDynamicObject(Player &, uint64);

    // find functions are expensive
    Player* FindPlayer(uint64);
    Player* FindPlayerByName(const char *name) ;

    inline PlayerMapType& GetPlayers(void) { return i_players; }
    void InsertPlayer(Player *);
    void RemovePlayer(Player *);

    void AddUpdateObject(Object *obj);
    void RemoveUpdateObject(Object *obj);

    /* This method updates the players and anything within its visibility
     */
    void Update(const uint32 &diff);

    // retrieves a corpse for the player
    Corpse* GetCorpseForPlayer(Player &);
    void RemoveCorpse(uint64);
    void AddCorpse(Corpse *corpse);

    // Check to see if player is located in a range of cells
    bool PlayersNearGrid(const uint32 &x, const uint32 &y, const uint32 &) const;

    template<class T> void RemoveUpdateObjects(std::map<OBJECT_HANDLE, T *> &);
private:

    struct ObjectChangeAccumulator
    {
	UpdateDataMapType &i_updateDatas;
	Object &i_object;
	ObjectChangeAccumulator(Object &obj, UpdateDataMapType &d) : i_object(obj), i_updateDatas(d) {}
	void Visit(std::map<OBJECT_HANDLE, Player *> &);
    };

    PlayerMapType i_players;
    CorpseMapType i_corpse;

    typedef ZThread::FastMutex LockType;
    typedef MaNGOS::GeneralLock<LockType > Guard;
    
    void _buildChangeObjectForPlayer(Object *, UpdateDataMapType &);
    void _buildUpdateObject(Object *, UpdateDataMapType &);
    void _buildPacket(Player *, Player *, UpdateDataMapType &);
    void _update(void);
    std::set<Object *> i_objects;
    LockType i_playerGuard;
    LockType i_updateGuard;
    LockType i_corpseGuard;
};


#endif
