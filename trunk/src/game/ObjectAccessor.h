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
class WorldObject;

class MANGOS_DLL_DECL ObjectAccessor : public MaNGOS::Singleton<ObjectAccessor, MaNGOS::ClassLevelLockable<ObjectAccessor, ZThread::FastMutex> >
{

    friend class MaNGOS::OperatorNew<ObjectAccessor>;
    ObjectAccessor() {}
    ObjectAccessor(const ObjectAccessor &);
    ObjectAccessor& operator=(const ObjectAccessor &);

    public:

        typedef HM_NAMESPACE::hash_map<uint64, Player* > PlayersMapType;
        typedef HM_NAMESPACE::hash_map<uint64, Corpse* > Player2CorpsesMapType;
        typedef HM_NAMESPACE::hash_map<Player*, UpdateData> UpdateDataMapType;
        typedef HM_NAMESPACE::hash_map<Player*, UpdateData>::value_type UpdateDataValueType;

        Object*   GetObjectByTypeMask(Player const &, uint64, uint32 typemask);
        Creature* GetCreature(WorldObject const &, uint64);
        Corpse*   GetCorpse(Unit const &u, uint64 guid);
        Corpse*   GetCorpse(float x, float y, uint32 mapid, uint64 guid);
        Unit* GetUnit(WorldObject const &, uint64);
        Player* GetPlayer(Unit const &, uint64);
        GameObject* GetGameObject(Unit const &, uint64);
        DynamicObject* GetDynamicObject(Unit const &, uint64);

        Player* FindPlayer(uint64);
        Player* FindPlayerByName(const char *name) ;

        PlayersMapType& GetPlayers() { return i_players; }
        void InsertPlayer(Player *);
        void RemovePlayer(Player *);
        void SaveAllPlayers();

        void AddUpdateObject(Object *obj);
        void RemoveUpdateObject(Object *obj);

        void AddObjectToRemoveList(WorldObject   *obj);

        void DoDelayedMovesAndRemoves();

        void RemoveCreatureCorpseFromPlayerView(Creature *);
        void RemoveBonesFromPlayerView(Corpse *);
        void RemovePlayerFromPlayerView(Player *, Player *);
        void RemoveInvisiblePlayerFromPlayerView(Player *, Player *);
        void RemoveCreatureFromPlayerView(Player *pl, Creature *c);

        void Update(const uint32 &diff);

        Corpse* GetCorpseForPlayer(Player const &);
        void RemoveCorpse(Corpse *corpse);
        void AddCorpse(Corpse *corpse);

        bool PlayersNearGrid(const uint32 &x, const uint32 &y, const uint32 &) const;

    private:
        void RemoveAllObjectsInRemoveList();

        struct ObjectChangeAccumulator
        {
            UpdateDataMapType &i_updateDatas;
            Object &i_object;
            ObjectAccessor &i_accessor;
            ObjectChangeAccumulator(Object &obj, UpdateDataMapType &d, ObjectAccessor &a) : i_updateDatas(d), i_object(obj), i_accessor(a) {}
            void Visit(std::map<OBJECT_HANDLE, Player *> &);
        };

        friend struct ObjectChangeAccumulator;
        PlayersMapType        i_players;
        Player2CorpsesMapType i_corpse;

        typedef ZThread::FastMutex LockType;
        typedef MaNGOS::GeneralLock<LockType > Guard;

        void _buildChangeObjectForPlayer(WorldObject *, UpdateDataMapType &);
        void _buildUpdateObject(Object *, UpdateDataMapType &);
        void _buildPacket(Player *, Object *, UpdateDataMapType &);
        void _update(void);
        std::set<Object *> i_objects;
        std::set<WorldObject *> i_objectsToRemove;
        LockType i_playerGuard;
        LockType i_updateGuard;
        LockType i_removeGuard;
        LockType i_corpseGuard;
};

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

    struct MANGOS_DLL_DECL BonesViewRemover
    {
        Object &i_objects;
        BonesViewRemover(Object &o) : i_objects(o) {}
        void Visit(std::map<OBJECT_HANDLE, Player *>  &);
    };

    struct MANGOS_DLL_DECL PlayerDeadViewRemover
    {
        Player &i_player;
        Player &i_player2;
        PlayerDeadViewRemover(Player &pl, Player &pl2) : i_player(pl), i_player2(pl2) {}
        void Visit(std::map<OBJECT_HANDLE, Player *>  &);
    };

    struct MANGOS_DLL_DECL PlayerInvisibilityRemover
    {
        Player &i_player;
        Player &i_player2;
        PlayerInvisibilityRemover(Player &pl, Player &pl2) : i_player(pl), i_player2(pl2) {}
        void Visit(std::map<OBJECT_HANDLE, Player *>  &);
    };

    struct MANGOS_DLL_DECL CreatureViewRemover
    {
        Player &i_player;
        Creature &i_creature;
        CreatureViewRemover(Player &pl, Creature &c) : i_player(pl), i_creature(c) {}
        void Visit(std::map<OBJECT_HANDLE, Player *>  &);
    };
}
#endif
