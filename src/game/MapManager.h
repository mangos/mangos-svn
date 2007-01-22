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

#ifndef MANGOS_MAPMANAGER_H
#define MANGOS_MAPMANAGER_H

#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "zthread/Mutex.h"
#include "Common.h"
#include "Map.h"
class Transport;

class MANGOS_DLL_DECL MapManager : public MaNGOS::Singleton<MapManager, MaNGOS::ClassLevelLockable<MapManager, ZThread::Mutex> >
{

    friend class MaNGOS::OperatorNew<MapManager>;
    typedef HM_NAMESPACE::hash_map<uint32, Map*> MapMapType;
    typedef std::pair<HM_NAMESPACE::hash_map<uint32, Map*>::iterator, bool>  MapMapPair;

    public:

        Map* GetMap(uint32);
        void Initialize(void);
        void Update(time_t);

        inline void SetGridCleanUpDelay(uint32 t)
        {
            if( t < MIN_GRID_DELAY )
                i_gridCleanUpDelay = MIN_GRID_DELAY;
            else
                i_gridCleanUpDelay = t;
        }

        inline void SetMapUpdateInterval(uint32 t)
        {

            if( t > 50 )
            {
                i_timer.SetInterval(t);
                i_timer.Reset();
            }
        }

        void LoadGrid(int mapid, float x, float y, bool no_unload = false);
        void UnloadAll();

        static bool ExistMAP(int mapid, float x, float y);

        void MoveAllCreaturesInMoveList();

        void LoadTransports();

        vector<Transport *> m_Transports;
        map<uint32, vector< Transport * > > m_TransportsByMap;

    private:
        MapManager();
        ~MapManager();

        MapManager(const MapManager &);
        MapManager& operator=(const MapManager &);

        inline Map* _getMap(uint32 id)
        {
            MapMapType::iterator iter = i_maps.find(id);
            return (iter == i_maps.end() ? NULL : iter->second);
        }

        typedef MaNGOS::ClassLevelLockable<MapManager, ZThread::Mutex>::Lock Guard;
        uint32 i_gridCleanUpDelay;
        MapMapType i_maps;
        IntervalTimer i_timer;
};
#endif
