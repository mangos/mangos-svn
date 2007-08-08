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

#ifndef MANGOS_GAMEEVENT_H
#define MANGOS_GAMEEVENT_H

#include "Platform/Define.h"
#include "Creature.h"
#include "GameObject.h"

#define max_ge_check_delay 86400 // 1 day in secondes

struct GameEventData
{
    time_t start;
    time_t end;
    uint32 occurence;
    uint32 length;
    std::string description;
};

class MANGOS_DLL_DECL GameEvent
{
    public:
        GameEvent();
        ~GameEvent() {};
        typedef std::set<uint16> ActiveEvents;
        ActiveEvents const* GetActiveEventList() const { return &m_ActiveEvents; }
        bool CheckOneGameEvent(uint16 entry);
        void LoadFromDB();
        uint32 Update();
        bool IsActiveEvent(uint16 event_id) { return ( m_ActiveEvents.find(event_id)!=m_ActiveEvents.end()); }
        uint32 Initialize();
    private:
        void AddActiveEvent(uint16 event_id) { m_ActiveEvents.insert(event_id); }
        void RemoveActiveEvent(uint16 event_id) { m_ActiveEvents.erase(event_id); }
        void ApplyNewEvent(uint16 event_id);
        void UnApplyEvent(uint16 event_id);
        void GameEventSpawn(int16 event_id);
        void GameEventUnspawn(int16 event_id);
        uint32 NextCheck(uint16 entry);
    protected:
		typedef std::vector<GameEventData> GameEventDataMap;
        typedef std::list<uint32> GuidList;
        typedef std::vector<GuidList> GameEventGuidMap;
        GameEventGuidMap  mGameEventCreatureGuids;
        GameEventGuidMap  mGameEventGameobjectGuids;
        GameEventDataMap  mGameEvent;
        ActiveEvents m_ActiveEvents;
        bool isSystemInit;
        uint16 max_event_id;
};

#define gameeventmgr MaNGOS::Singleton<GameEvent>::Instance()
#endif
