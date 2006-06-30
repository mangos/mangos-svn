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
        void Notify(void);

        #ifdef WIN32

        template<> void VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &);
        template<> void VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &);
        #endif
    };

    struct MANGOS_DLL_DECL NotVisibleNotifier
    {
        Player &i_player;
        UpdateData i_data;
        NotVisibleNotifier(Player &player) : i_player(player) {}
        void Notify(void);
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m);

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
            std::map<OBJECT_HANDLE, T *> tmp(m);
            for(typename std::map<OBJECT_HANDLE, T*>::iterator iter=tmp.begin(); iter != tmp.end(); ++iter)
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
        MessageDeliverer(Player &pl, WorldPacket *msg, bool to_self) : i_player(pl), i_message(msg), i_toSelf(to_self) {}
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

    template<class T>
    struct MANGOS_DLL_DECL ObjectSetIn2DRangeChecker
    {
        bool& i_result;
        Unit* i_unit;
        std::set<uint32> const& i_ids;
        float  i_dist2;

        ObjectSetIn2DRangeChecker(bool& result, Unit* unit, std::set<uint32> const& ids, float radius) 
            : i_result(result), i_unit(unit), i_ids(ids), i_dist2(radius*radius) {}

        void Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
        {
            for(std::map<OBJECT_HANDLE, GameObject *>::iterator itr=m.begin(); itr != m.end(); ++itr)
            {
                            
                if(i_ids.find(itr->second->GetGOInfo()->id) != i_ids.end())
                {
                    if(itr->second->GetDistance2dSq(i_unit) <= i_dist2)
                    {
                        i_result = true;
                        return;
                    }
                }
            }
        }

        void Visit(std::map<OBJECT_HANDLE, Creature *> &m)
        {
            for(std::map<OBJECT_HANDLE, Creature *>::iterator itr=m.begin(); itr != m.end(); ++itr)
            {
                if(i_ids.find(itr->second->GetCreatureInfo()->Entry) != i_ids.end())
                {
                    if(itr->second->GetDistance2dSq(i_unit) <= i_dist2)
                    {
                        i_result = true;
                        return;
                    }
                }
            }
        }

        template<class NOT_INTERESTED> void Visit(std::map<OBJECT_HANDLE, NOT_INTERESTED *> &m) {}
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

    struct MANGOS_DLL_DECL PlayerConfrontationNotifier
    {
        Player &i_player;
        PlayerConfrontationNotifier(Player &pl) : i_player(pl) {}
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m) {}
        #ifdef WIN32
        template<> void Visit(std::map<OBJECT_HANDLE, Creature *> &);
        #endif
    };

    #ifndef WIN32

    template<> void VisibleNotifier::Visit<Creature>(std::map<OBJECT_HANDLE, Creature *> &);
    template<> void VisibleNotifier::Visit<Player>(std::map<OBJECT_HANDLE, Player *> &);
    template<> void NotVisibleNotifier::Visit<Creature>(std::map<OBJECT_HANDLE, Creature *> &);
    template<> void NotVisibleNotifier::Visit<Player>(std::map<OBJECT_HANDLE, Player *> &);
    template<> void ObjectUpdater::Visit<Creature>(std::map<OBJECT_HANDLE, Creature *> &);
    template<> void PlayerConfrontationNotifier::Visit<Creature>(std::map<OBJECT_HANDLE, Creature *> &);
    template<> inline void GridUnitListNotifier::Visit(std::map<OBJECT_HANDLE, Corpse *> &m ) {}
    template<> inline void GridUnitListNotifier::Visit(std::map<OBJECT_HANDLE, GameObject *> &m ) {}
    template<> inline void GridUnitListNotifier::Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m ) {}
    #endif
}
#endif
