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

#ifndef MANGOS_MAP_H
#define MANGOS_MAP_H

#include "Platform/Define.h"
#include "Policies/ThreadingModel.h"
#include "zthread/Lockable.h"
#include "zthread/Mutex.h"
#include "zthread/FairReadWriteLock.h"
#include "GridDefines.h"
#include "Cell.h"
#include "Object.h"
#include "Timer.h"

class Unit;
class WorldPacket;

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
    GridInfo(time_t expiry, bool unload = true ) : i_timer(expiry), i_unloadflag(unload) {}
    GridRWLock i_lock;
    TimeTracker i_timer;
    bool i_unloadflag;
};

typedef struct
{
    uint16 area_flag[16][16];
    uint8 terrain_type[16][16];
    float liquid_level[128][128];
    float Z[MAP_RESOLUTION][MAP_RESOLUTION];
}GridMap;

struct CreatureMover
{
    CreatureMover() : x(0), y(0), z(0), ang(0) {}
    CreatureMover(float _x, float _y, float _z, float _ang) : x(_x), y(_y), z(_z), ang(_ang) {}

    float x, y, z, ang;
};

typedef HM_NAMESPACE::hash_map<Creature*, CreatureMover> CreatureMoveList;

class MANGOS_DLL_DECL Map : public MaNGOS::ObjectLevelLockable<Map, ZThread::Mutex>
{
    public:

        Map(uint32 id, time_t);

        void Add(Player *);
        void Remove(Player *, bool);
        void Add(Corpse *);
        void Remove(Corpse *, bool);
        template<class T> void Add(T *);
        template<class T> void Remove(T *, bool);
        template<class T> bool Find(T *) const;

        template<class T> T* GetObjectNear(WorldObject const &obj, OBJECT_HANDLE hdl);
        template<class T> T* GetObjectNear(float x, float y, OBJECT_HANDLE hdl);

        void Update(const uint32&);
        uint64 CalculateGridMask(const uint32 &y) const;

        void MessageBoardcast(Player *, WorldPacket *, bool to_self, bool own_team_only = false);

        void MessageBoardcast(WorldObject *, WorldPacket *);

        void PlayerRelocation(Player *, float x, float y, float z, float angl, bool visibilityChanges = false);

        void CreatureRelocation(Creature *creature, float x, float y, float, float);

        template<class LOCK_TYPE, class T, class CONTAINER> void Visit(const CellLock<LOCK_TYPE> &cell, TypeContainerVisitor<T, CONTAINER> &visitor);

        void SetTimer(uint32 t)
        {
            i_gridExpiry = t < MIN_GRID_DELAY ? MIN_GRID_DELAY : t;
        }

        inline bool IsActiveGrid(WorldObject *obj) const
        {
            return IsActiveGrid(obj->GetPositionX(),obj->GetPositionY());
        }

        inline bool IsActiveGrid(float x, float y) const
        {
            GridPair p = MaNGOS::ComputeGridPair(x, y);
            return( i_grids[p.x_coord][p.y_coord]->GetGridState() == GRID_STATE_ACTIVE );
        }

        inline bool IsRemovalGrid(float x, float y) const
        {
            GridPair p = MaNGOS::ComputeGridPair(x, y);
            return( !i_grids[p.x_coord][p.y_coord] || i_grids[p.x_coord][p.y_coord]->GetGridState() == GRID_STATE_REMOVAL );
        }

        bool GetUnloadFlag(const GridPair &p) const { return i_info[p.x_coord][p.y_coord]->i_unloadflag; }
        void SetUnloadFlag(const GridPair &p, bool unload) { i_info[p.x_coord][p.y_coord]->i_unloadflag = unload; }
        void LoadGrid(const Cell& cell, bool no_unload = false);
        bool UnloadGrid(const uint32 &x, const uint32 &y);
        void UnloadAll();

        void ResetGridExpiry(GridInfo &info) const
        {
            info.i_timer.Reset(i_gridExpiry);
        }

        time_t GetGridExpiry(void) const { return i_gridExpiry; }
        uint32 GetId(void) const { return i_id; }

        static bool ExistMAP(uint32 mapid, int x, int y);
        GridMap * LoadMAP(uint32 mapid, int x, int y);

        static void InitStateMachine();
        static void DeleteStateMachine();

        float GetHeight(float x, float y);
        uint16 GetAreaFlag(float x, float y );
        uint8 GetTerrainType(float x, float y );
        float GetWaterLevel(float x, float y );
        bool IsInWater(float x, float y);
        bool IsUnderWater(float x, float y, float z);

        uint32 GetAreaId(uint16 areaflag);
        uint32 GetZoneId(uint16 areaflag);

        uint32 GetAreaId(float x, float y)
        {
            return GetAreaId(GetAreaFlag(x,y));
        }

        uint32 GetZoneId(float x, float y)
        {
            return GetZoneId(GetAreaFlag(x,y));
        }

        void MoveAllCreaturesInMoveList();

        bool CreatureRespawnRelocation(Creature *c);        // used only in MoveAllCreaturesInMoveList and ObjectGridUnloader

        // assert print helper
        bool CheckGridIntegrity(Creature* c, bool moved) const;
    private:
        bool CreatureCellRelocation(Creature *creature, Cell new_cell);
        void CreatureRelocationNotifying(Creature *creature, Cell newcell, CellPair newval);

        void AddCreatureToMoveList(Creature *c, float x, float y, float z, float ang);
        CreatureMoveList i_creaturesToMove;

        bool loaded(const GridPair &) const;
        void EnsureGridLoadedForPlayer(const Cell&, Player*, bool add_player);
        void NotifyPlayerVisibility(const Cell &, const CellPair &, Player *);
        uint64  EnsureGridCreated(const GridPair &);

        template<class T> void AddType(T *obj);
        template<class T> void RemoveType(T *obj, bool);

        uint32 i_id;

        GridMap *GridMaps[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
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
