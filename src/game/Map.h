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

// foward declaration
namespace ZThread
{
    class Lockable;
    class ReadWriteLock;
}

struct MANGOS_DLL_DECL GridPair
{
    GridPair(uint32 x=0, uint32 y=0) : x_coord(x), y_coord(y) {}
    uint32 x_coord;
    uint32 y_coord;
};

class MANGOS_DLL_DECL Map : public MaNGOS::ObjectLevelLockable<Map, ZThread::Mutex>
{    
public:

    Map(uint32 id);

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

    void Update(uint32);
    
    GridPair CalculateGrid(const float &x, const float &y) const;
    uint64 CalculateGridMask(const uint32 &x, const uint32 &y) const;

private:
    bool loaded(const GridPair &) const;
    template<class T> GridType* AddType(T *obj);
    template<class T> GridType* RemoveType(T *obj, bool);

    uint32 i_id;
    bool i_hasGrids;

    // no register cache.. need to use it to sync stuff
    volatile uint64 i_gridMask;
    volatile uint64 i_gridStatus;
        
    typedef MaNGOS::ObjectLevelLockable<Map, ZThread::Mutex>::Lock Guard;
    typedef ZThread::Lockable GridLockType;
    typedef MaNGOS::GeneralLock<ZThread::Lockable> GridGuard;
    typedef ZThread::FairReadWriteLock GridRWLock;

    GridType* i_grids[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
    GridRWLock *i_guards[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];

};

inline
GridPair
Map::CalculateGrid(const float &x, const float &y) const
{
    int x_offset = int((x - CENTER_GRID_OFFSET)/SIZE_OF_GRIDS);
    int y_offset = int((y - CENTER_GRID_OFFSET)/SIZE_OF_GRIDS);
    return GridPair(x_offset+CENTER_GRID_ID, y_offset+CENTER_GRID_ID);
}

inline
uint64
Map::CalculateGridMask(const uint32 &x, const uint32 &y) const
{
    uint32 idx = MAX_NUMBER_OF_GRIDS*x + y;
    uint64 mask = 1 << idx;
    return mask;
}

#endif
