/* Map.h
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

#ifndef MANGOS_MAP_H
#define MANGOS_MAP_H

/** Map identified a map in the world.  Each map is further divided into
 * grids (64 by 64) grids or a map can have no grids.  In the later
 * case, MaNGOS represent the map with 1 grid.
 */

#include "Platform/Define.h"
#include "Policies/ThreadingModel.h"
#include "ObjectGridLoader.h"
#include "zthread/Lockable.h"
#include "zthread/Mutex.h"
#include "zthread/FairReadWriteLock.h"

#define MAX_NUMBER_OF_GRIDS      64

// the size of grids is
#define SIZE_OF_GRIDS            533.33333
#define CENTER_GRID_ID           32

// this offset is computed base on SIZE_OF_GRIDS/2
#define CENTER_GRID_OFFSET      266.66666

// minimum time to delay is 60 seconds
#define MIN_GRID_DELAY          60*1000

// foward declaration
namespace ZThread
{
    class Lockable;
    class ReadWriteLock;
}


struct MANGOS_DLL_DECL GridPair
{
    GridPair(uint32 x=0, uint32 y=0) : x_coord(x), y_coord(y) {}
    GridPair(const GridPair &obj) : x_coord(obj.x_coord), y_coord(obj.y_coord) {}
    bool operator==(const GridPair &obj) const { return (obj.x_coord == x_coord && obj.y_coord == y_coord); }
    bool operator!=(const GridPair &obj) const { return !operator==(obj); }
    GridPair& operator=(const GridPair &obj) 
    {
	this->~GridPair();
	new (this) GridPair(obj);
	return *this;
    }

    uint32 x_coord;
    uint32 y_coord;
};

class MANGOS_DLL_DECL Map : public MaNGOS::ObjectLevelLockable<Map, ZThread::Mutex>
{    
public:

    Map(uint32 id, time_t);

    void Add(Player *);
    void Add(Creature *);
    void Add(GameObject *);
    void Add(DynamicObject *);
    void Add(Corpse *);

    void Remove(Player *, bool);
    void Remove(Creature *, bool);
    void Remove(GameObject *, bool);
    void Remove(DynamicObject *, bool);
    void Remove(Corpse *, bool);

    void RemoveFromMap(Player *);
    void RemoveFromMap(Creature *obj) { Remove(obj, false); }
    void RemoveFromMap(GameObject *obj) { Remove(obj, false); }
    void RemoveFromMap(DynamicObject *obj) { Remove(obj, false); }
    void RemoveFromMap(Corpse *obj) { Remove(obj, false); }

    void Update(uint32);
    
    GridPair CalculateGrid(const float &x, const float &y) const;
    uint64 CalculateGridMask(const uint32 &y) const;

    /** MessageBoardcast is a player sending messages to all near by
     * players in his/her grid
     */
    void MessageBoardcast(Player *, WorldPacket *, bool to_self);

    /** MessageBoardcast a message to all players within the range of the object
     */
    void MessageBoardcast(Object *, WorldPacket *);

    /** Relocation of a player means a player has moved on the map
     */
    void PlayerRelocation(Player *, const float &x, const float &y, const float &z, const float &angl);

    /** Relocation of an object means an object moved such as
     * creatures running after you
     */
  template<class T> void ObjectRelocation(T *obj, const float &x, const float &y, const float &, const float &);

    /** Sets the timer interval
     */
    void SetTimer(uint32 t) 
    { 
	i_gridExpiry = t < MIN_GRID_DELAY ? MIN_GRID_DELAY : t;
    }

    inline bool IsActiveGrid(Object *obj) const
    {
	GridPair p = CalculateGrid(obj->GetPositionX(), obj->GetPositionY());
	return( i_grids[p.x_coord][p.y_coord]->GetGridStatus() == GRID_STATUS_ACTIVE );
    }

private:
    bool loaded(const GridPair &) const;
    void EnsurePlayerInGrid(const GridPair&, Player*);
    uint64  EnsureGridCreated(const GridPair &);

    template<class T> void AddType(T *obj);
    template<class T> void RemoveType(T *obj, bool);

    uint32 i_id;

    // no register cache.. need to use it to sync stuff
    volatile uint64 i_gridMask[MAX_NUMBER_OF_GRIDS];
    volatile uint64 i_gridStatus[MAX_NUMBER_OF_GRIDS];
        
    typedef MaNGOS::ObjectLevelLockable<Map, ZThread::Mutex>::Lock Guard;
    //    typedef ZThread::Lockable GridLockType;
    //    typedef MaNGOS::GeneralLock<ZThread::Lockable> GridGuard;
    typedef ZThread::FairReadWriteLock GridRWLock;

    struct GridInfo
    {
	GridInfo(time_t expiry) : i_timer(expiry) {}
	GridRWLock i_lock;
	TimeTracker i_timer;
    };

    template<class MUTEX, class LOCK_TYPE>
    struct RGuard
    {
	RGuard(MUTEX &l) : i_lock(l.getReadLock()) {}
	MaNGOS::GeneralLock<LOCK_TYPE> i_lock;
    };

    template<class MUTEX, class LOCK_TYPE>
    struct WGuard
    {
	WGuard(MUTEX &l) : i_lock(l.getWriteLock()) {}
	MaNGOS::GeneralLock<LOCK_TYPE> i_lock;
    };

    
    typedef RGuard<GridRWLock, ZThread::Lockable> ReadGuard;
    typedef WGuard<GridRWLock, ZThread::Lockable> WriteGuard;

    GridType* i_grids[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
    GridInfo *i_info[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
    time_t i_gridExpiry;

	time_t m_nextThinkTime;
};

inline
GridPair
Map::CalculateGrid(const float &x, const float &y) const
{
    float x_offset = (x - (float)CENTER_GRID_OFFSET)/(float)SIZE_OF_GRIDS;
    float y_offset = (y - (float)CENTER_GRID_OFFSET)/(float)SIZE_OF_GRIDS;

    // avoid round off errors
    int x_val = int(x_offset+CENTER_GRID_ID + 0.5);
    int y_val = int(y_offset+CENTER_GRID_ID + 0.5); 
    return GridPair(x_val, y_val);
}

inline
uint64
Map::CalculateGridMask(const uint32 &y) const
{
    uint64 mask = 1;
    mask <<= y;
    return mask;
}


#endif
