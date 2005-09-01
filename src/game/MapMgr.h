/* MapMgr.h
 *
 * Copyright (C) 2004 Wow Daemon
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

#ifndef __MAPMGR_H
#define __MAPMGR_H

class Object;
class MapCell;

// Distance a Player can "see" other objects and receive updates from them
#define UPDATE_DISTANCE 155.8

enum MapMgrTimers
{
    MMUPDATE_OBJECTS = 0,
    MMUPDATE_SESSIONS,
    MMUPDATE_FIELDS,
    MMUPDATE_COUNT
};

class MapMgr
{
    public:
        MapMgr(uint32 mapid);
        ~MapMgr();

        typedef HM_NAMESPACE::hash_map<uint64, Object*> ObjectMap;

        void AddObject(Object *obj);
        void RemoveObject(Object *obj);
        void ChangeObjectLocation(Object *obj);   // update inrange lists

        ObjectMap::iterator GetObjectsBegin() { return _objects.begin(); }
        ObjectMap::iterator GetObjectsEnd() { return _objects.end(); }

//! Mark object as updated
        void ObjectUpdated(Object *obj)
        {
            ASSERT(_updatedObjects.find(obj) == _updatedObjects.end());
            _updatedObjects.insert(obj);
        }

        void Update(time_t diff);

    protected:

//! Collect and send updates to clients
        void _UpdateObjects();

    private:
//! Objects that exist on map
        ObjectMap _objects;
        int32 _minX, _minY, _minZ, _maxX, _maxY, _maxZ;
        uint32 _cellSize, _sizeX, _sizeY;
        MapCell **_cells;

        uint32 _mapId;

        typedef std::set<Object*> ObjectSet;
        ObjectSet _updatedObjects;
        IntervalTimer _timers[MMUPDATE_COUNT];
};
#endif
