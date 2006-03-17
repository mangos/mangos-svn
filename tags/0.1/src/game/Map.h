/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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



#include "Platform/Define.h"
#include "Policies/ThreadingModel.h"
#include "zthread/Lockable.h"
#include "zthread/Mutex.h"
#include "zthread/FairReadWriteLock.h"
#include "GridDefines.h"
#include "Cell.h"


namespace ZThread
{
    class Lockable;
    class ReadWriteLock;
}


typedef ZThread::FairReadWriteLock GridRWLock;

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


typedef RGuard<GridRWLock, ZThread::Lockable> GridReadGuard;
typedef WGuard<GridRWLock, ZThread::Lockable> GridWriteGuard;
typedef MaNGOS::SingleThreaded<GridRWLock>::Lock NullGuard;

struct GridInfo
{
    GridInfo(time_t expiry) : i_timer(expiry) {}
    GridRWLock i_lock;
    TimeTracker i_timer;
};


class MANGOS_DLL_DECL Map : public MaNGOS::ObjectLevelLockable<Map, ZThread::Mutex>
{    
public:

    Map(uint32 id, time_t);

    void Add(Player *);
    void Remove(Player *, bool);
    template<class T> void Add(T *);
    template<class T> void Remove(T *, bool);

    void Update(const uint32&);    
    uint64 CalculateGridMask(const uint32 &y) const;

    
    void MessageBoardcast(Player *, WorldPacket *, bool to_self);

    
    void MessageBoardcast(Object *, WorldPacket *);

    
    void PlayerRelocation(Player *, const float &x, const float &y, const float &z, const float &angl);

    
    void CreatureRelocation(Creature *creature, const float &x, const float &y, const float &, const float &);

    
    template<class LOCK_TYPE, class T, class CONTAINER> void Visit(const CellLock<LOCK_TYPE> &cell, TypeContainerVisitor<T, CONTAINER> &visitor);

    
    void SetTimer(uint32 t) 
    { 
	i_gridExpiry = t < MIN_GRID_DELAY ? MIN_GRID_DELAY : t;
    }

    inline bool IsActiveGrid(Object *obj) const
    {
	GridPair p = MaNGOS::ComputeGridPair(obj->GetPositionX(), obj->GetPositionY());
	return( i_grids[p.x_coord][p.y_coord]->GetGridState() == GRID_STATE_ACTIVE );
    }

    
    bool UnloadGrid(const uint32 &x, const uint32 &y);
    
	void GetUnitList(const float &x, const float &y, std::list<Unit*> &unlist);

    void ResetGridExpiry(GridInfo &info) const
    {
	info.i_timer.Reset(i_gridExpiry);
    }

    time_t GetGridExpiry(void) const { return i_gridExpiry; }
    uint32 GetId(void) const { return i_id; }

    static void InitStateMachine(void);

private:
    bool loaded(const GridPair &) const;
    void EnsureGridLoadedForPlayer(const Cell&, Player*, bool add_player);
    void NotifyPlayerVisibility(const Cell &, const CellPair &, Player *);
    uint64  EnsureGridCreated(const GridPair &);

    template<class T> void AddType(T *obj);
    template<class T> void RemoveType(T *obj, bool);

    uint32 i_id;

    
    volatile uint64 i_gridMask[MAX_NUMBER_OF_GRIDS];
    volatile uint64 i_gridStatus[MAX_NUMBER_OF_GRIDS];
        
    typedef MaNGOS::ObjectLevelLockable<Map, ZThread::Mutex>::Lock Guard;
    typedef GridReadGuard ReadGuard;
    typedef GridWriteGuard WriteGuard;

    NGridType* i_grids[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
    GridInfo *i_info[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
    time_t i_gridExpiry;
};


inline
uint64
Map::CalculateGridMask(const uint32 &y) const
{
    uint64 mask = 1;
    mask <<= y;
    return mask;
}


template<class LOCK_TYPE, class T, class CONTAINER> 
inline void 
Map::Visit(const CellLock<LOCK_TYPE> &cell, TypeContainerVisitor<T, CONTAINER> &visitor)
{
    const uint32 x = cell->GridX();
    const uint32 y = cell->GridY();
    const uint32 cell_x = cell->CellX();
    const uint32 cell_y = cell->CellY();

    if( !cell->NoCreate() || loaded(GridPair(x,y)) )
    {
	EnsureGridLoadedForPlayer(cell, NULL, false);
	LOCK_TYPE guard(i_info[x][y]->i_lock);
	i_grids[x][y]->Visit(cell_x, cell_y, visitor);
    }
}

#endif

