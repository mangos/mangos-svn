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
        PlayerNotifier(Player &pl) : i_player(pl) {}
        void Visit(PlayerMapType &);
        void BuildForMySelf(void);
        Player &i_player;
    };

    struct MANGOS_DLL_DECL VisibleNotifier
    {
        Player &i_player;
        UpdateData i_data;
        VisibleNotifier(Player &player) : i_player(player) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m);
        void Visit(std::map<OBJECT_HANDLE, GameObject *> &);
        void Notify(void);

        #ifdef WIN32

        template<> void VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &);
        template<> void VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &);
        #endif
    };

    struct MANGOS_DLL_DECL VisibleChangesNotifier
    {
        Player &i_player;
        VisibleChangesNotifier(Player &player) : i_player(player) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m);

        #ifdef WIN32
        template<> void VisibleChangesNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &);
        #endif
    };

    struct MANGOS_DLL_DECL NotVisibleNotifier
    {
        Player &i_player;
        UpdateData i_data;
        NotVisibleNotifier(Player &player) : i_player(player) {}
        void Notify(void);
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m);
        void Visit(std::map<OBJECT_HANDLE, GameObject *> &);

        #ifdef WIN32
        template<> void NotVisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &);
        template<> void NotVisibleNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &);
        #endif
    };

    struct MANGOS_DLL_DECL ObjectVisibleNotifier
    {
        Object &i_object;
        ObjectVisibleNotifier(Object &obj) : i_object(obj) {}
        void Visit(PlayerMapType &);
    };

    struct MANGOS_DLL_DECL ObjectNotVisibleNotifier
    {
        Object &i_object;
        ObjectNotVisibleNotifier(Object &obj) : i_object(obj) {}
        void Visit(PlayerMapType &);
    };

    struct MANGOS_DLL_DECL GridUpdater
    {
        GridType &i_grid;
        uint32 i_timeDiff;
        GridUpdater(GridType &grid, uint32 diff) : i_grid(grid), i_timeDiff(diff) {}

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
    };

    struct MANGOS_DLL_DECL ObjectMessageDeliverer
    {
        Object &i_object;
        WorldPacket *i_message;
        ObjectMessageDeliverer(Object &obj, WorldPacket *msg) : i_object(obj), i_message(msg) {}
        void Visit(PlayerMapType &m);
    };

    struct MANGOS_DLL_DECL CreatureVisibleMovementNotifier
    {
        Creature &i_creature;
        CreatureVisibleMovementNotifier(Creature &creature) : i_creature(creature) {}
        void Visit(PlayerMapType &m);
    };

    struct MANGOS_DLL_DECL CreatureNotVisibleMovementNotifier
    {
        Creature &i_creature;
        CreatureNotVisibleMovementNotifier(Creature &creature) : i_creature(creature) {}
        void Visit(PlayerMapType &m);
    };

    struct MANGOS_DLL_DECL ObjectUpdater
    {
        uint32 i_timeDiff;
        ObjectUpdater(const uint32 &diff) : i_timeDiff(diff) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m);
        #ifdef WIN32
        template<> void Visit(std::map<OBJECT_HANDLE, Creature *> &);
        #endif
    };

    template<class T>
        struct MANGOS_DLL_DECL ObjectAccessorNotifier
    {
        T *& i_object;
        uint64 i_id;
        ObjectAccessorNotifier(T * &obj, uint64 id) : i_object(obj), i_id(id)
        {
            i_object = NULL;
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

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
    };

    template<class Check>
        struct MANGOS_DLL_DECL GameObjectSearcher
    {
        GameObject* &i_object;
        Check const& i_check;

        GameObjectSearcher(GameObject* & result, Check const& check) : i_object(result),i_check(check) {}

        void Visit(std::map<OBJECT_HANDLE, GameObject *> &m);

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
    };

    template<class Check>
        void GameObjectSearcher<Check>::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
    {
        // already found
        if(i_object) return;

        for(std::map<OBJECT_HANDLE, GameObject *>::iterator itr=m.begin(); itr != m.end(); ++itr)
        {
            if(i_check(itr->second))
            {
                i_object = itr->second;
                return;
            }
        }
    }

    class GameObjectFocusCheck
    {
        public:
            GameObjectFocusCheck(Unit* unit,uint32 focusId) : i_unit(unit), i_focusId(focusId) {}
            bool operator()(GameObject* go) const
            {
                if(go->GetGOInfo()->type != GAMEOBJECT_TYPE_SPELL_FOCUS || go->GetGOInfo()->sound0 != i_focusId)
                    return false;

                float dist = go->GetGOInfo()->sound1;

                return go->IsWithinDist(i_unit, dist);
            }
        private:
            Unit* i_unit;
            uint32 i_focusId;

    };

    template<class Check>
        struct MANGOS_DLL_DECL UnitSearcher
    {
        Unit* &i_object;
        Check & i_check;

        UnitSearcher(Unit* & result, Check & check) : i_object(result),i_check(check) {}

        void Visit(std::map<OBJECT_HANDLE, Creature *> &m);
        void Visit(std::map<OBJECT_HANDLE, Player *> &m);

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
    };

    template<class Check>
    struct MANGOS_DLL_DECL UnitListSearcher
    {
        std::list<Unit*> &i_objects;
        Check& i_check;

        UnitListSearcher(std::list<Unit*> &objects, Check & check) : i_objects(objects),i_check(check) {}

        void Visit(std::map<OBJECT_HANDLE, Player *> &m)
        {
            for(std::map<OBJECT_HANDLE, Player *>::iterator itr=m.begin(); itr != m.end(); ++itr)
            {
                if(i_check(itr->second))
                    i_objects.push_back(itr->second);
            }
        }

        void Visit(std::map<OBJECT_HANDLE, Creature *> &m)
        {
            for(std::map<OBJECT_HANDLE, Creature *>::iterator itr=m.begin(); itr != m.end(); ++itr)
            {
                if(i_check(itr->second))
                    i_objects.push_back(itr->second);
            }
        }

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
    };


    class AnyUnfriendlyUnitInObjectRangeCheck
    {
        public:
            AnyUnfriendlyUnitInObjectRangeCheck(Object* obj, Unit* funit, float range) : i_obj(obj), i_funit(funit), i_range(range), i_result(NULL) {}
            bool operator()(Unit* u)
            {
                if(!i_funit->IsFriendlyTo(u) && i_obj->IsWithinDist(u, i_range))
                {
                    i_result = u;
                    return true;
                }
                return false;
            }
            Unit *GetResult() const { return i_result; }
        private:
            Object* const i_obj;
            Unit* const i_funit;
            float i_range;
            Unit* i_result;
    };

    struct MANGOS_DLL_DECL GridUnitListNotifier
    {
        std::list<Unit*> &i_data;
        GridUnitListNotifier(std::list<Unit*> &data) : i_data(data) {}

        template<class T> inline void Visit(std::map<OBJECT_HANDLE, T *>  &m)
        {
            for(typename std::map<OBJECT_HANDLE, T*>::iterator itr=m.begin(); itr != m.end(); ++itr)
            {
                i_data.push_back(itr->second);
            }
        }

        #ifdef WIN32
        template<> inline void Visit(std::map<OBJECT_HANDLE, Corpse *> &m ) {}
        template<> inline void Visit(std::map<OBJECT_HANDLE, GameObject *> &m ) {}
        template<> inline void Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m ) {}
        #endif
    };

    struct MANGOS_DLL_DECL PlayerRelocationNotifier
    {
        Player &i_player;
        PlayerRelocationNotifier(Player &pl) : i_player(pl) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m) {}
        #ifdef WIN32
        template<> void Visit(std::map<OBJECT_HANDLE, Corpse   *> &);
        template<> void Visit(std::map<OBJECT_HANDLE, Player   *> &);
        template<> void Visit(std::map<OBJECT_HANDLE, Creature *> &);
        #endif
    };

    struct MANGOS_DLL_DECL CreatureRelocationNotifier
    {
        Creature &i_creature;
        CreatureRelocationNotifier(Creature &c) : i_creature(c) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m) {}
        #ifdef WIN32
        template<> void Visit(std::map<OBJECT_HANDLE, Player *> &);
        #endif
    };

    struct MANGOS_DLL_DECL DynamicObjectUpdater
    {
        DynamicObject &owner;
        DynamicObjectUpdater(DynamicObject &owner) : owner(owner) {}

        template<class T> inline void Visit(std::map<OBJECT_HANDLE, T *>  &m) {}
        #ifdef WIN32
        template<> inline void Visit<Player>(std::map<OBJECT_HANDLE, Player *> &);
        template<> inline void Visit<Creature>(std::map<OBJECT_HANDLE, Creature *> &);
        #endif
    };

    #ifndef WIN32

    template<> void VisibleNotifier::Visit<Creature>(std::map<OBJECT_HANDLE, Creature *> &);
    template<> void VisibleNotifier::Visit<Player>(std::map<OBJECT_HANDLE, Player *> &);
    template<> void VisibleChangesNotifier::Visit<Player>(std::map<OBJECT_HANDLE, Player *> &);
    template<> void NotVisibleNotifier::Visit<Creature>(std::map<OBJECT_HANDLE, Creature *> &);
    template<> void NotVisibleNotifier::Visit<Player>(std::map<OBJECT_HANDLE, Player *> &);
    template<> void ObjectUpdater::Visit<Creature>(std::map<OBJECT_HANDLE, Creature *> &);
    template<> void PlayerRelocationNotifier::Visit<Corpse>(std::map<OBJECT_HANDLE, Corpse *> &);
    template<> void PlayerRelocationNotifier::Visit<Creature>(std::map<OBJECT_HANDLE, Creature *> &);
    template<> void PlayerRelocationNotifier::Visit<Player>(std::map<OBJECT_HANDLE, Player *> &);
    template<> void CreatureRelocationNotifier::Visit<Player>(std::map<OBJECT_HANDLE, Player *> &);
    template<> inline void DynamicObjectUpdater::Visit<Creature>(std::map<OBJECT_HANDLE, Creature *> &);
    template<> inline void DynamicObjectUpdater::Visit<Player>(std::map<OBJECT_HANDLE, Player *> &);
    template<> inline void GridUnitListNotifier::Visit(std::map<OBJECT_HANDLE, Corpse *> &m ) {}
    template<> inline void GridUnitListNotifier::Visit(std::map<OBJECT_HANDLE, GameObject *> &m ) {}
    template<> inline void GridUnitListNotifier::Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m ) {}
    #endif
}
#endif
