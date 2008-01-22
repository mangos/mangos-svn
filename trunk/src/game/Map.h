/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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
#include "Database/DBCStructure.h"
#include "GridDefines.h"
#include "Cell.h"
#include "Object.h"
#include "Timer.h"
#include "SharedDefines.h"
#include "GameSystem/GridRefManager.h"

#include <bitset>
#include <list>

class Unit;
class WorldPacket;
class InstanceData;

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

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

struct InstanceTemplate
{
    uint32 map;
    uint32 levelMin;
    uint32 levelMax;
    uint32 maxPlayers;
    uint32 reset_delay;
    float startLocX;
    float startLocY;
    float startLocZ;
    float startLocO;
    char const* script;
};

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

typedef HM_NAMESPACE::hash_map<Creature*, CreatureMover> CreatureMoveList;

#define MAX_HEIGHT            100000.0f                     // can be use for find ground height at surface
#define INVALID_HEIGHT       -100000.0f                     // for check, must be equal to VMAP_INVALID_HEIGHT, real value for unknown height is VMAP_INVALID_HEIGHT_VALUE

class MANGOS_DLL_DECL Map : public GridRefManager<NGridType>, public MaNGOS::ObjectLevelLockable<Map, ZThread::Mutex>
{
    public:
        typedef std::list<Player*> PlayerList;

        Map(uint32 id, time_t, uint32 aInstanceId);
        virtual ~Map();

        void Add(Player *);
        bool AddInstanced(Player *);
        void Remove(Player *, bool);
        void RemoveInstanced(Player *);
        template<class T> void Add(T *);
        template<class T> void Remove(T *, bool);

        virtual void Update(const uint32&);

        void MessageBroadcast(Player *, WorldPacket *, bool to_self, bool own_team_only = false);

        void MessageBroadcast(WorldObject *, WorldPacket *);

        void PlayerRelocation(Player *, float x, float y, float z, float angl);

        void CreatureRelocation(Creature *creature, float x, float y, float, float);

        template<class LOCK_TYPE, class T, class CONTAINER> void Visit(const CellLock<LOCK_TYPE> &cell, TypeContainerVisitor<T, CONTAINER> &visitor);

        inline bool IsRemovalGrid(float x, float y) const
        {
            GridPair p = MaNGOS::ComputeGridPair(x, y);
            return( !getNGrid(p.x_coord, p.y_coord) || getNGrid(p.x_coord, p.y_coord)->GetGridState() == GRID_STATE_REMOVAL );
        }

        bool GetUnloadFlag(const GridPair &p) const { return getNGrid(p.x_coord, p.y_coord)->getUnloadFlag(); }
        void SetUnloadFlag(const GridPair &p, bool unload) { getNGrid(p.x_coord, p.y_coord)->setUnloadFlag(unload); }
        void LoadGrid(const Cell& cell, bool no_unload = false);
        bool UnloadGrid(const uint32 &x, const uint32 &y);
        virtual void UnloadAll();

        void ResetGridExpiry(NGridType &grid, float factor = 1) const
        {
            grid.ResetTimeTracker((time_t)((float)i_gridExpiry*factor));
        }

        time_t GetGridExpiry(void) const { return i_gridExpiry; }
        uint32 GetId(void) const { return i_id; }

        static bool ExistMap(uint32 mapid, int x, int y);
        static bool ExistVMap(uint32 mapid, int x, int y);
        void LoadMapAndVMap(uint32 mapid, uint32 instanceid, int x, int y);

        static void InitStateMachine();
        static void DeleteStateMachine();

        // some calls like isInWater should not use vmaps due to processor power
        // can return INVALID_HEIGHT if under z+2 z coord not found height
        float GetHeight(float x, float y, float z, bool pCheckVMap=true) const;
        bool IsInWater(float x, float y, float z) const;    // does not use z pos. This is for future use

        uint16 GetAreaFlag(float x, float y ) const;
        uint8 GetTerrainType(float x, float y ) const;
        float GetWaterLevel(float x, float y ) const;
        bool IsUnderWater(float x, float y, float z) const;

        static uint32 GetAreaId(uint16 areaflag);
        static uint32 GetZoneId(uint16 areaflag);

        uint32 GetAreaId(float x, float y) const
        {
            return GetAreaId(GetAreaFlag(x,y));
        }

        uint32 GetZoneId(float x, float y) const
        {
            return GetZoneId(GetAreaFlag(x,y));
        }

        virtual void MoveAllCreaturesInMoveList();

        bool CreatureRespawnRelocation(Creature *c);        // used only in MoveAllCreaturesInMoveList and ObjectGridUnloader

        // assert print helper
        bool CheckGridIntegrity(Creature* c, bool moved) const;

        std::string GetScript() { return i_script; }
        InstanceData* GetInstanceData() { return i_data; }
        uint32 GetInstanceId() { return i_InstanceId; }
        bool NeedsReset() { return Instanceable() && ( i_resetTime == 0 || i_resetTime <= time(NULL)); }
        uint32 GetPlayersCount() const { return i_Players.size(); }
        void Reset();
        bool CanEnter(Player* player) const;
        const char* GetMapName() const;

        bool Instanceable() const { return(i_mapEntry && ((i_mapEntry->map_type == MAP_INSTANCE) || (i_mapEntry->map_type == MAP_RAID))); }
        // NOTE: this duplicate of Instanceable(), but Instanceable() can be changed when BG also will be instanceable
        bool IsDungeon() const { return(i_mapEntry && ((i_mapEntry->map_type == MAP_INSTANCE) || (i_mapEntry->map_type == MAP_RAID))); }
        bool IsRaid() const { return(i_mapEntry && (i_mapEntry->map_type == MAP_RAID)); }
        bool IsMountAllowed() const
        {
            return !IsDungeon() || i_mapEntry && (
                i_mapEntry->MapID==568 || i_mapEntry->MapID==309 || i_mapEntry->MapID==209 || i_mapEntry->MapID==534 ||
                i_mapEntry->MapID==560 || i_mapEntry->MapID==509 || i_mapEntry->MapID==269 );
        }

        virtual bool RemoveBones(uint64 guid, float x, float y);
        void InitResetTime();

        void UpdateObjectVisibility(WorldObject* obj, Cell cell, CellPair cellpair);
        void UpdatePlayerVisibility(Player* player, Cell cell, CellPair cellpair);
        void UpdateObjectsVisibilityFor(Player* player, Cell cell, CellPair cellpair);

        void resetMarkedCells() { marked_cells.reset(); }
        bool isCellMarked(uint32 pCellId) { return marked_cells.test(pCellId); }
        void markCell(uint32 pCellId) { marked_cells.set(pCellId); }
    private:
        void LoadVMap(int pX, int pY);
        void LoadMap(uint32 mapid, uint32 instanceid, int x,int y);

        void SetTimer(uint32 t) { i_gridExpiry = t < MIN_GRID_DELAY ? MIN_GRID_DELAY : t; }
        //uint64 CalculateGridMask(const uint32 &y) const;

        void SendInitSelf( Player * player );

        void SendInitTransports( Player * player );
        void SendRemoveTransports( Player * player );

        void PlayerRelocationNotify(Player* player, Cell cell, CellPair cellpair);
        void CreatureRelocationNotify(Creature *creature, Cell newcell, CellPair newval);

        bool CreatureCellRelocation(Creature *creature, Cell new_cell);

        void AddCreatureToMoveList(Creature *c, float x, float y, float z, float ang);
        CreatureMoveList i_creaturesToMove;

        bool loaded(const GridPair &) const;
        void EnsureGridLoadedForPlayer(const Cell&, Player*, bool add_player);
        void  EnsureGridCreated(const GridPair &);

        void buildNGridLinkage(NGridType* pNGridType) { pNGridType->link(this); }

        template<class T> void AddType(T *obj);
        template<class T> void RemoveType(T *obj, bool);


        NGridType* getNGrid(uint32 x, uint32 y) const
        {
            return i_grids[x][y];
        }

        bool isGridObjectDataLoaded(uint32 x, uint32 y) const { return getNGrid(x,y)->isGridObjectDataLoaded(); }
        void setGridObjectDataLoaded(bool pLoaded, uint32 x, uint32 y) { getNGrid(x,y)->setGridObjectDataLoaded(pLoaded); }


        inline void setNGrid(NGridType* grid, uint32 x, uint32 y)
        {
            if(x >= MAX_NUMBER_OF_GRIDS || y >= MAX_NUMBER_OF_GRIDS)
            {
                sLog.outError("map::setNGrid() Invalid grid coordinates found: %d, %d!",x,y);
                assert(false);
            }
            i_grids[x][y] = grid;
        }
    protected:
        typedef MaNGOS::ObjectLevelLockable<Map, ZThread::Mutex>::Lock Guard;

    private:
        typedef GridReadGuard ReadGuard;
        typedef GridWriteGuard WriteGuard;

        uint32 i_id;
        NGridType* i_grids[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
        GridMap *GridMaps[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
        std::bitset<TOTAL_NUMBER_OF_CELLS_PER_MAP*TOTAL_NUMBER_OF_CELLS_PER_MAP> marked_cells;

        time_t i_gridExpiry;

        MapEntry const* i_mapEntry;
        time_t i_resetTime;
        uint32 i_resetDelayTime;
        uint32 i_InstanceId;
        uint32 i_maxPlayers;
        InstanceData* i_data;
        std::string i_script;

        PlayerList i_Players;

        // Type specific code for add/remove to/from grid
        template<class T>
            void AddToGrid(T*, NGridType *, Cell const&);

        template<class T>
            void AddNotifier(T*, Cell const&, CellPair const&);

        template<class T>
            void RemoveFromGrid(T*, NGridType *, Cell const&);

        template<class T>
            void DeleteFromWorld(T*);
};

/*inline
uint64
Map::CalculateGridMask(const uint32 &y) const
{
    uint64 mask = 1;
    mask <<= y;
    return mask;
}
*/

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
        //LOCK_TYPE guard(i_info[x][y]->i_lock);
        getNGrid(x, y)->Visit(cell_x, cell_y, visitor);
    }
}
#endif
