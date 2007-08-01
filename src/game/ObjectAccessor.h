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

#ifndef MANGOS_OBJECTACCESSOR_H
#define MANGOS_OBJECTACCESSOR_H

#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "zthread/FastMutex.h"
#include "Utilities/HashMap.h"
#include "Policies/ThreadingModel.h"

#include "ByteBuffer.h"
#include "UpdateData.h"

#include "GridDefines.h"
#include "Object.h"
#include "Player.h"

#include <set>

class Creature;
class Corpse;
class Unit;
class GameObject;
class DynamicObject;
class WorldObject;
class Map;

template <class T>
class HashMapHolder
{
    public:
        
        typedef HM_NAMESPACE::hash_map< uint64, T* >   MapType;
        typedef ZThread::FastMutex LockType;
        typedef MaNGOS::GeneralLock<LockType > Guard;

        static void Insert(T* o) { m_objectMap[o->GetGUID()] = o; }
        static void Remove(T* o)
        {
            Guard guard(i_lock);
            typename MapType::iterator itr = m_objectMap.find(o->GetGUID());
            if (itr != m_objectMap.end())
                m_objectMap.erase(itr);
        }
        static T* Find(uint64 guid)
        {
            typename MapType::iterator itr = m_objectMap.find(guid);
            return (itr != m_objectMap.end()) ? itr->second : NULL;
        }

        static MapType& GetContainer() { return m_objectMap; }

        static LockType* GetLock() { return &i_lock; }
    private:

        //Non instanciable only static
        HashMapHolder() {}

        static LockType i_lock;
        static MapType  m_objectMap;
};

class MANGOS_DLL_DECL ObjectAccessor : public MaNGOS::Singleton<ObjectAccessor, MaNGOS::ClassLevelLockable<ObjectAccessor, ZThread::FastMutex> >
{

    friend class MaNGOS::OperatorNew<ObjectAccessor>;
    ObjectAccessor();
    ~ObjectAccessor();
    ObjectAccessor(const ObjectAccessor &);
    ObjectAccessor& operator=(const ObjectAccessor &);

    public:
        typedef HM_NAMESPACE::hash_map<uint64, Corpse* >      Player2CorpsesMapType;
        typedef HM_NAMESPACE::hash_map<Player*, UpdateData>::value_type UpdateDataValueType;

        template<class T> T* GetObjectInWorld(uint64 guid, T* /*fake*/)
        {
            return HashMapHolder<T>::Find(guid);
        }
        
        template<class T> T* GetObjectInWorld(uint32 mapid, float x, float y, uint64 guid, T* /*fake*/)
        {
            T* obj = HashMapHolder<T>::Find(guid);
            if(!obj || obj->GetMapId() != mapid) return NULL;

            CellPair p = MaNGOS::ComputeCellPair(x,y);
            if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
            {
                sLog.outError("ObjectAccessor::GetObjectInWorld: invalid coordiates supplied X:%u Y:%u grid cell [%u:%u]", x, y, p.x_coord, p.y_coord);
                return NULL;
            }

            CellPair q = MaNGOS::ComputeCellPair(obj->GetPositionX(),obj->GetPositionY());
            if(q.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || q.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
            {
                sLog.outError("ObjectAccessor::GetObjecInWorld: object "I64FMTD" has invalid coordinates X:%u Y:%u grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), q.x_coord, q.y_coord);
                return NULL;
            }
            
            uint32 dx = p.x_coord - q.x_coord;
            uint32 dy = p.y_coord - q.y_coord;

            if (dx > -2 && dx < 2 && dy > -2 && dy < 2) return obj;
            else return NULL;                
        }

        Object*   GetObjectByTypeMask(Player const &, uint64, uint32 typemask);
        Creature* GetNPCIfCanInteractWith(Player const &player, uint64 guid, uint32 npcflagmask);
        Creature* GetCreature(WorldObject const &, uint64);
        Creature* GetCreatureOrPet(WorldObject const &, uint64);
        Unit* GetUnit(WorldObject const &, uint64);
        Pet* GetPet(Unit const &, uint64 guid) { return GetPet(guid); }
        Player* GetPlayer(Unit const &, uint64 guid) { return FindPlayer(guid); }
        GameObject* GetGameObject(Unit const &, uint64);
        DynamicObject* GetDynamicObject(Unit const &, uint64);
        Corpse* GetCorpse(WorldObject const &u, uint64 guid);
        Pet* GetPet(uint64 guid);
        Player* FindPlayer(uint64);

        Player* FindPlayerByName(const char *name) ;

        HashMapHolder<Player>::MapType& GetPlayers()
        {
            return HashMapHolder<Player>::GetContainer();
        }

        template<class T> void AddObject(T *object)
        {
            HashMapHolder<T>::Insert(object);
        }

        template<class T> void RemoveObject(T *object)
        {
            HashMapHolder<T>::Remove(object);
        }

        void RemoveObject(Player *pl)
        {
            HashMapHolder<Player>::Remove(pl);

            Guard guard(i_updateGuard);

            std::set<Object *>::iterator iter2 = std::find(i_objects.begin(), i_objects.end(), (Object *)pl);
            if( iter2 != i_objects.end() )
                i_objects.erase(iter2);
        }

        void SaveAllPlayers();

        void AddUpdateObject(Object *obj);
        void RemoveUpdateObject(Object *obj);

        void AddObjectToRemoveList(WorldObject   *obj);

        void DoDelayedMovesAndRemoves();

        void Update(const uint32 &diff);

        Corpse* GetCorpseForPlayerGUID(uint64 guid);
        void RemoveCorpse(Corpse *corpse);
        void AddCorpse(Corpse* corpse);
        void AddCorpsesToGrid(GridPair const& gridpair,GridType& grid,Map* map);
        bool ConvertCorpseForPlayer(uint64 player_guid);

        bool PlayersNearGrid(const uint32 &x, const uint32 &y, const uint32 &m_id, const uint32 &i_id) const;

        static void UpdateObject(Object* obj, Player* exceptPlayer);
        static void _buildUpdateObject(Object* obj, UpdateDataMapType &);

        static void UpdateObjectVisibility(WorldObject* obj);
        static void UpdateVisibilityForPlayer(Player* player);
    private:
        void RemoveAllObjectsInRemoveList();

        struct WorldObjectChangeAccumulator
        {
            UpdateDataMapType &i_updateDatas;
            WorldObject &i_object;
            WorldObjectChangeAccumulator(WorldObject &obj, UpdateDataMapType &d) : i_updateDatas(d), i_object(obj) {}
            void Visit(PlayerMapType &);
            template<class SKIP> void Visit(std::map<OBJECT_HANDLE, SKIP *> &) {}
        };

        friend struct WorldObjectChangeAccumulator;
        Player2CorpsesMapType   i_player2corpse;

        typedef ZThread::FastMutex LockType;
        typedef MaNGOS::GeneralLock<LockType > Guard;

        static void _buildChangeObjectForPlayer(WorldObject *, UpdateDataMapType &);
        static void _buildPacket(Player *, Object *, UpdateDataMapType &);
        void _update(void);
        std::set<Object *> i_objects;
        std::set<WorldObject *> i_objectsToRemove;
        LockType i_playerGuard;
        LockType i_updateGuard;
        LockType i_removeGuard;
        LockType i_corpseGuard;
        LockType i_petGuard;
};
#endif
