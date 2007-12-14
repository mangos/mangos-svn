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

#ifndef TRANSPORTS_H
#define TRANSPORTS_H

#include "GameObject.h"

#include <map>
#include <set>
#include <string>

class TransportPath
{
    public:
        struct PathNode
        {
            uint32 mapid;
            float x,y,z;
            uint32 actionFlag;
            uint32 delay;
        };

        inline void SetLength(const unsigned int sz)
        {
            i_nodes.resize( sz );
        }

        inline unsigned int Size(void) const { return i_nodes.size(); }
        inline bool Empty(void) const { return i_nodes.empty(); }
        inline void Resize(unsigned int sz) { i_nodes.resize(sz); }
        inline void Clear(void) { i_nodes.clear(); }
        inline PathNode* GetNodes(void) { return static_cast<PathNode *>(&i_nodes[0]); }
        float GetTotalLength(void)
        {
            float len = 0, xd, yd, zd;
            for(unsigned int idx=1; idx < i_nodes.size(); ++idx)
            {
                xd = i_nodes[ idx ].x - i_nodes[ idx-1 ].x;
                yd = i_nodes[ idx ].y - i_nodes[ idx-1 ].y;
                zd = i_nodes[ idx ].z - i_nodes[ idx-1 ].z;
                len += (float)sqrt( xd * xd + yd*yd + zd*zd );
            }
            return len;
        }

        PathNode& operator[](const unsigned int idx) { return i_nodes[idx]; }
        const PathNode& operator()(const unsigned int idx) const { return i_nodes[idx]; }

    protected:
        std::vector<PathNode> i_nodes;
};

class Transport : public GameObject
{
    public:
        explicit Transport( WorldObject *instantiator );

        bool Create(uint32 guidlow, uint32 displayId, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress, uint32 dynflags);
        bool GenerateWaypoints(uint32 pathid, std::set<uint32> &mapids);
        void Update(uint32 p_time);
        bool AddPassenger(Player* passenger);
        bool RemovePassenger(Player* passenger);

        typedef std::set<Player*> PlayerSet;
        PlayerSet const& GetPassengers() const { return m_passengers; }

        std::string m_name;
    private:
        struct WayPoint
        {
            WayPoint() : mapid(0), x(0), y(0), z(0), teleport(false) {}
            WayPoint(uint32 _mapid, float _x, float _y, float _z, bool _teleport) :
            mapid(_mapid), x(_x), y(_y), z(_z), teleport(_teleport) {}
            uint32 mapid;
            float x;
            float y;
            float z;
            bool teleport;
        };

        typedef std::map<uint32, WayPoint> WayPointMap;

        WayPointMap::iterator m_curr;
        WayPointMap::iterator m_next;
        uint32 m_pathTime;
        uint32 m_timer;

        PlayerSet m_passengers;

    public:
        WayPointMap m_WayPoints;
        uint32 m_lastMovement;
        uint32 m_nextNodeTime;
        uint32 m_period;

    private:
        void TeleportTransport(uint32 oldMapid, uint32 newMapid, float x, float y, float z);
        WayPointMap::iterator GetNextWayPoint();
};
#endif
