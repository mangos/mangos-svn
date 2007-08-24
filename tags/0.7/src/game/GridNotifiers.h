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

#ifndef MANGOS_GRIDNOTIFIERS_H
#define MANGOS_GRIDNOTIFIERS_H

#include "ObjectGridLoader.h"
#include "ByteBuffer.h"
#include "UpdateData.h"
#include <iostream>

#include "Corpse.h"
#include "Object.h"
#include "DynamicObject.h"
#include "GameObject.h"
#include "Player.h"
#include "Unit.h"

class Player;
class Map;

namespace MaNGOS
{

    struct MANGOS_DLL_DECL PlayerNotifier
    {
        explicit PlayerNotifier(Player &pl) : i_player(pl) {}
        void Visit(PlayerMapType &);
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, SKIP *> &) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &m) {}
        Player &i_player;
    };

    struct MANGOS_DLL_DECL VisibleNotifier
    {
        Player &i_player;
        UpdateData i_data;
        UpdateDataMapType i_data_updates;
        Player::ClientGUIDs i_clientGUIDs;

        explicit VisibleNotifier(Player &player) : i_player(player),i_clientGUIDs(player.m_clientGUIDs) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m);
        template<class T> void Visit(std::map<OBJECT_HANDLE, CountedPtr<T> > &m);
        void Visit(PlayerMapType &);
        void Notify(void);
    };

    struct MANGOS_DLL_DECL VisibleChangesNotifier
    {
        WorldObject &i_object;

        explicit VisibleChangesNotifier(WorldObject &object) : i_object(object) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, CountedPtr<T> > &m) {}
        void Visit(PlayerMapType &);
    };

    struct MANGOS_DLL_DECL GridUpdater
    {
        GridType &i_grid;
        uint32 i_timeDiff;
        GridUpdater(GridType &grid, uint32 diff) : i_grid(grid), i_timeDiff(diff) {}

        template<class T> void updateObjects(std::map<OBJECT_HANDLE, CountedPtr<T> >  &m)
        {
            for(typename std::map<OBJECT_HANDLE, CountedPtr<T> >::iterator iter=m.begin(); iter != m.end(); ++iter)
                iter->second->Update(i_timeDiff);
        }
        template<class T> void updateObjects(std::map<OBJECT_HANDLE, T *> &m)
        {
            for(typename std::map<OBJECT_HANDLE, T*>::iterator iter=m.begin(); iter != m.end(); ++iter)
                iter->second->Update(i_timeDiff);
        }

        void Visit(PlayerMapType &m) { updateObjects<Player>(m); }
        void Visit(CreatureMapType &m){ updateObjects<Creature>(m); }
        void Visit(GameObjectMapType &m) { updateObjects<GameObject>(m); }
        void Visit(DynamicObjectMapType &m) { updateObjects<DynamicObject>(m); }
        void Visit(CorpseMapType &m) { updateObjects<Corpse>(m); }
    };

    struct MANGOS_DLL_DECL MessageDeliverer
    {
        Player &i_player;
        WorldPacket *i_message;
        bool i_toSelf;
        bool i_ownTeamOnly;
        MessageDeliverer(Player &pl, WorldPacket *msg, bool to_self, bool ownTeamOnly) : i_player(pl), i_message(msg), i_toSelf(to_self), i_ownTeamOnly(ownTeamOnly) {}
        void Visit(PlayerMapType &m);
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, SKIP *> &) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &m) {}
    };

    struct MANGOS_DLL_DECL ObjectMessageDeliverer
    {
        Object &i_object;
        WorldPacket *i_message;
        ObjectMessageDeliverer(Object &obj, WorldPacket *msg) : i_object(obj), i_message(msg) {}
        void Visit(PlayerMapType &m);
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, SKIP *> &) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &m) {}
    };

    struct MANGOS_DLL_DECL ObjectUpdater
    {
        uint32 i_timeDiff;
        explicit ObjectUpdater(const uint32 &diff) : i_timeDiff(diff) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m);
        template<class T> void Visit(std::map<OBJECT_HANDLE, CountedPtr<T> > &m);
        void Visit(PlayerMapType &) {}
        void Visit(CorpseMapType &) {}
        void Visit(CreatureMapType &);
    };

    template<class T>
        struct MANGOS_DLL_DECL ObjectAccessorNotifier
    {
        T *& i_object;
        CountedPtr<T>& i_objectptr;

        uint64 i_id;
        ObjectAccessorNotifier(T * &obj, uint64 id) : i_object(obj), i_id(id)
        {
            i_object = NULL;
        }

        ObjectAccessorNotifier(CountedPtr<T> &obj, uint64 id) : i_objectptr(obj), i_id(id)
        {
            i_objectptr.reset();
        }

        void Visit(std::map<OBJECT_HANDLE, T *> &m )
        {
            if( i_object == NULL )
            {
                typename std::map<OBJECT_HANDLE, T *>::iterator iter = m.find(i_id);
                if( iter != m.end() )
                {
                    assert( iter->second != NULL );
                    i_object = iter->second;
                }
            }
        }

        void Visit(std::map<OBJECT_HANDLE, CountedPtr<T> > &m )
        {
            if( !i_objectptr )
            {
                typename std::map<OBJECT_HANDLE, CountedPtr<T> >::iterator iter = m.find(i_id);
                if( iter != m.end() )
                {
                    assert( iter->second );
                    i_objectptr = iter->second;
                }
            }
        }

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &m) {}
    };

    struct MANGOS_DLL_DECL PlayerRelocationNotifier
    {
        Player &i_player;
        PlayerRelocationNotifier(Player &pl) : i_player(pl) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, CountedPtr<T> > &m) {}
        void Visit(PlayerMapType &);
        void Visit(CreatureMapType &);
    };

    struct MANGOS_DLL_DECL CreatureRelocationNotifier
    {
        Creature &i_creature;
        CreatureRelocationNotifier(Creature &c) : i_creature(c) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, CountedPtr<T> > &m) {}
        #ifdef WIN32
        template<> void Visit(PlayerMapType &);
        #endif
    };

    struct MANGOS_DLL_DECL DynamicObjectUpdater
    {
        DynamicObject &i_dynobject;
        Unit* i_check;
        DynamicObjectUpdater(DynamicObject &dynobject, Unit* caster) : i_dynobject(dynobject)
        {
            i_check = caster;
            Unit* owner = i_check->GetOwner();
            if(owner)
                i_check = owner;
        }

        template<class T> inline void Visit(std::map<OBJECT_HANDLE, T *>  &m) {}
        template<class T> inline void Visit(std::map<OBJECT_HANDLE, CountedPtr<T> > &m) {}
        #ifdef WIN32
        template<> inline void Visit<Player>(PlayerMapType &);
        template<> inline void Visit<Creature>(CreatureMapType &);
        #endif

        void VisitHelper(Unit* target);
    };

    // SEARCHERS & LIST SEARCHERS & WORKERS

    // WorldObject searchers & workers

    template<class Check>
        struct MANGOS_DLL_DECL WorldObjectSearcher
    {
        WorldObject* &i_object;
        CountedPtr<WorldObject> &i_objectptr;
        Check const& i_check;

        WorldObjectSearcher(WorldObject* & result, Check const& check) : i_object(result),i_check(check) {}
        WorldObjectSearcher(CountedPtr<WorldObject> & result, Check const& check) : i_objectptr(result),i_check(check) { i_objectptr.reset(); }

        void Visit(GameObjectMapType &m);
        void Visit(PlayerMapType &m);
        void Visit(CreatureMapType &m);
        void Visit(CorpseMapType &m);
        void Visit(DynamicObjectMapType &m);

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &m) {}
    };

    template<class Check>
        struct MANGOS_DLL_DECL WorldObjectListSearcher
    {
        std::list<WorldObject*> &i_objects;
        Check& i_check;

        WorldObjectListSearcher(std::list<WorldObject*> &objects, Check & check) : i_objects(objects),i_check(check) {}

        void Visit(PlayerMapType &m);
        void Visit(CreatureMapType &m);
        void Visit(CorpseMapType &m);
        void Visit(GameObjectMapType &m);
        void Visit(DynamicObjectMapType &m);

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &m) {}
    };

    template<class Do>
        struct MANGOS_DLL_DECL WorldObjectWorker
    {
        Do const& i_do;

        explicit WorldObjectWorker(Do const& _do) : i_do(_do) {}

        void Visit(GameObjectMapType &m)
        {
            for(GameObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
                i_do(itr->second);
        }

        void Visit(PlayerMapType &m)
        {
            for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
                i_do(itr->second);
        }
        void Visit(CreatureMapType &m)
        {
            for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
                i_do(itr->second);
        }

        void Visit(CorpseMapType &m)
        {
            for(CorpseMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
                i_do(itr->second);
        }

        void Visit(DynamicObjectMapType &m)
        {
            for(DynamicObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
                i_do(itr->second);
        }

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
    };

    // Gameobject searchers

    template<class Check>
        struct MANGOS_DLL_DECL GameObjectSearcher
    {
        GameObject* &i_object;
        Check const& i_check;

        GameObjectSearcher(GameObject* & result, Check const& check) : i_object(result),i_check(check) {}

        void Visit(GameObjectMapType &m);

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &m) {}
    };

    template<class Check>
        struct MANGOS_DLL_DECL GameObjectListSearcher
    {
        std::list<GameObject*> &i_objects;
        Check& i_check;

        GameObjectListSearcher(std::list<GameObject*> &objects, Check & check) : i_objects(objects),i_check(check) {}

        void Visit(GameObjectMapType &m);

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &m) {}
    };

    // Unit searchers

    template<class Check>
        struct MANGOS_DLL_DECL UnitSearcher
    {
        Unit* &i_object;
        Check & i_check;

        UnitSearcher(Unit* & result, Check & check) : i_object(result),i_check(check) {}

        void Visit(CreatureMapType &m);
        void Visit(PlayerMapType &m);

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &m) {}
    };

    template<class Check>
        struct MANGOS_DLL_DECL UnitListSearcher
    {
        std::list<Unit*> &i_objects;
        Check& i_check;

        UnitListSearcher(std::list<Unit*> &objects, Check & check) : i_objects(objects),i_check(check) {}

        void Visit(PlayerMapType &m);
        void Visit(CreatureMapType &m);

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &m) {}
    };

    // Creature searchers

    template<class Check>
        struct MANGOS_DLL_DECL CreatureSearcher
    {
        Creature* &i_object;
        Check & i_check;

        CreatureSearcher(Creature* & result, Check & check) : i_object(result),i_check(check) {}

        void Visit(CreatureMapType &m);

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &m) {}
    };

    template<class Check>
        struct MANGOS_DLL_DECL CreatureListSearcher
    {
        std::list<Creature*> &i_objects;
        Check& i_check;

        CreatureListSearcher(std::list<Creature*> &objects, Check & check) : i_objects(objects),i_check(check) {}

        void Visit(CreatureMapType &m);

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &m) {}
    };

    // CHECKS && DO classes

    // WorldObject do classes

    class RespawnDo
    {
        public:
            RespawnDo() {}
            void operator()(Creature* u) const { u->Respawn(); }
            void operator()(GameObject* u) const { u->Respawn(); }
            void operator()(WorldObject* u) const {}
            void operator()(CountedPtr<Corpse> &u) const {}
    };

    // GameObject checks

    class GameObjectFocusCheck
    {
        public:
            GameObjectFocusCheck(Unit* unit,uint32 focusId) : i_unit(unit), i_focusId(focusId) {}
            bool operator()(GameObject* go) const
            {
                if(go->GetGOInfo()->type != GAMEOBJECT_TYPE_SPELL_FOCUS || go->GetGOInfo()->sound0 != i_focusId)
                    return false;

                float dist = go->GetGOInfo()->sound1;

                return go->IsWithinDistInMap(i_unit, dist);
            }
        private:
            Unit* i_unit;
            uint32 i_focusId;

    };

    // Unit checks

    class AnyUnfriendlyUnitInObjectRangeCheck
    {
        public:
            AnyUnfriendlyUnitInObjectRangeCheck(WorldObject* obj, Unit* funit, float range) : i_obj(obj), i_funit(funit), i_range(range) {}
            bool operator()(Unit* u)
            {
                if(u->isAlive() && !i_funit->IsFriendlyTo(u) && i_obj->IsWithinDistInMap(u, i_range))
                    return true;

                return false;
            }
        private:
            WorldObject* const i_obj;
            Unit* const i_funit;
            float i_range;
    };

    class AnyAoETargetUnitInObjectRangeCheck
    {
        public:
            AnyAoETargetUnitInObjectRangeCheck(WorldObject* obj, Unit* funit, float range)
                : i_obj(obj), i_funit(funit), i_range(range)
            {
                Unit* check = i_funit;
                Unit* owner = i_funit->GetOwner();
                if(owner)
                    check = owner;
                i_targetForPlayer = ( check->GetTypeId()==TYPEID_PLAYER );
            }
            bool operator()(Unit* u)
            {
                if(u->isAlive() && (i_targetForPlayer ? !i_funit->IsFriendlyTo(u) : i_funit->IsHostileTo(u) )&& i_obj->IsWithinDistInMap(u, i_range))
                    return true;

                return false;
            }
        private:
            bool i_targetForPlayer;
            WorldObject* const i_obj;
            Unit* const i_funit;
            float i_range;
    };

    struct AnyDeadUnitCheck
    {
        bool operator()(Unit* u) { return !u->isAlive(); }
    };

    struct AnyStealthedCheck
    {
        bool operator()(Unit* u) { return u->GetVisibility()==VISIBILITY_GROUP_STEALTH; }
    };

    class CannibalizeUnitCheck
    {
        public:
            CannibalizeUnitCheck(Unit* funit, float range) : i_funit(funit), i_range(range) {}
            bool operator()(Player* u)
            {
                if( i_funit->IsFriendlyTo(u) || u->isAlive() || u->isInFlight() )
                    return false;

                if(i_funit->IsWithinDistInMap(u, i_range) )
                    return true;

                return false;
            }
            bool operator()(Creature* u)
            {
                if( i_funit->IsFriendlyTo(u) || u->isAlive() || u->isInFlight() ||
                    ((Creature*)u)->GetCreatureInfo()->type != CREATURE_TYPE_HUMANOID &&
                    ((Creature*)u)->GetCreatureInfo()->type != CREATURE_TYPE_UNDEAD)
                    return false;

                if(i_funit->IsWithinDistInMap(u, i_range) )
                    return true;

                return false;
            }
        private:
            Unit* const i_funit;
            float i_range;
    };

    // Creature checks

    class InAttackDistanceFromAnyHostileCreatureCheck
    {
        public:
            explicit InAttackDistanceFromAnyHostileCreatureCheck(Unit* funit) : i_funit(funit) {}
            bool operator()(Creature* u)
            {
                if(u->isAlive() && u->IsHostileTo(i_funit) && i_funit->IsWithinDistInMap(u, u->GetAttackDistance(i_funit)))
                    return true;

                return false;
            }
        private:
            Unit* const i_funit;
    };

    #ifndef WIN32
    template<> void PlayerRelocationNotifier::Visit<Creature>(CreatureMapType &);
    template<> void PlayerRelocationNotifier::Visit<Player>(PlayerMapType &);
    template<> void CreatureRelocationNotifier::Visit<Player>(PlayerMapType &);
    template<> void CreatureRelocationNotifier::Visit<Creature>(CreatureMapType &);
    template<> inline void DynamicObjectUpdater::Visit<Creature>(CreatureMapType &);
    template<> inline void DynamicObjectUpdater::Visit<Player>(PlayerMapType &);
    #endif
}
#endif
