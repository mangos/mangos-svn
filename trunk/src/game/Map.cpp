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

#include "Player.h"
#include "GridNotifiers.h"
#include "WorldSession.h"
#include "Log.h"
#include "GridStates.h"
#include "RedZoneDistrict.h"
#include "CellImpl.h"
#include "Map.h"
#include "GridNotifiersImpl.h"
#include "Config/ConfigEnv.h"
#include "Transports.h"

#define DEFAULT_GRID_EXPIRY     300
#define MAX_GRID_LOAD_TIME      50

// magic *.map header
const char MAP_MAGIC[] = "MAP_1.01";

static GridState* si_GridStates[MAX_GRID_STATE];

inline
bool FileExists(const char * fn)
{
    FILE *pf=fopen(fn,"rb");
    if(!pf)return false;
    fclose(pf);
    return true;
}

bool Map::ExistMAP(int mapid,int x,int y)
{
    std::string dataPath="./";

    if(sConfig.GetString("DataDir",&dataPath))
    {
        if(dataPath.at(dataPath.length()-1)!='/')
            dataPath.append("/");
    }

    int len = dataPath.length()+strlen("maps/%03u%02u%02u.map")+1;
    char* tmp = new char[len];
    snprintf(tmp, len, (char *)(dataPath+"maps/%03u%02u%02u.map").c_str(),mapid,x,y);

    FILE *pf=fopen(tmp,"rb");

    sLog.outDetail("Check existing of map file '%s': %s",tmp,(pf ? "exist." : "not exist!"));

    if(!pf)
    {
        delete[] tmp;
        return false;
    }

    char magic[8];
    fread(magic,1,8,pf);
    if(strncmp(MAP_MAGIC,magic,8))
    {
        sLog.outError("Map file '%s' is non-compatible version (outdated?). Please, create new using ad.exe program.",tmp);
        delete [] tmp;
        return false;
    }

    delete [] tmp;
    fclose(pf);

    return true;
}

GridMap * Map::LoadMAP(int mapid,int x,int y)
{
    char *tmp;
    static bool showcheckmapInfo=false;
    static int oldx=0,oldy=0;

    std::string dataPath="./";

    if(sConfig.GetString("DataDir",&dataPath))
    {
        if(dataPath.at(dataPath.length()-1)!='/')
            dataPath.append("/");
    }

    // Pihhan: dataPath length + "maps/" + 3+2+2+ ".map" length may be > 32 !
    int len = dataPath.length()+strlen("maps/%03u%02u%02u.map")+1;
    tmp = new char[len];
    snprintf(tmp, len, (char *)(dataPath+"maps/%03u%02u%02u.map").c_str(),mapid,x,y);

    if( (oldx!=x) || (oldy!=y) )
    {
        DEBUG_LOG("Loading map %s",tmp);
        oldx =x;
        oldy =y;
        showcheckmapInfo = true;
    }

    FILE *pf=fopen(tmp,"rb");

    if(!pf)
    {
        if( showcheckmapInfo )
        {
            DEBUG_LOG("Map file %s does not exist",tmp);
            showcheckmapInfo = false;
        }
        delete [] tmp;
        return NULL;
    }
    char magic[8];
    fread(magic,1,8,pf);
    if(strncmp(MAP_MAGIC,magic,8))
    {
        sLog.outError("Map file '%s' is non-compatible version (outdated?). Please, create new using ad.exe program.",tmp);
        delete [] tmp;
        return NULL;
    }

    // fseek(pf,0,2);
    // uint32 fs=ftell(pf);
    // fseek(pf,0,0);
    GridMap * buf= new GridMap;
    fread(buf,1,sizeof(GridMap),pf);
    fclose(pf);

    delete [] tmp;

    return buf;
}

void Map::InitStateMachine()
{
    si_GridStates[GRID_STATE_INVALID] = new InvalidState;
    si_GridStates[GRID_STATE_ACTIVE] = new ActiveState;
    si_GridStates[GRID_STATE_IDLE] = new IdleState;
    si_GridStates[GRID_STATE_REMOVAL] = new RemovalState;
}

void Map::DeleteStateMachine()
{
    delete si_GridStates[GRID_STATE_INVALID];
    delete si_GridStates[GRID_STATE_ACTIVE];
    delete si_GridStates[GRID_STATE_IDLE];
    delete si_GridStates[GRID_STATE_REMOVAL];
}

Map::Map(uint32 id, time_t expiry) : i_id(id), i_gridExpiry(expiry)
{
    //    char tmp[32];
    for(unsigned int idx=0; idx < MAX_NUMBER_OF_GRIDS; ++idx)
    {
        i_gridMask[idx] = 0;
        i_gridStatus[idx] = 0;
        for(unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
        {
            //z code
            GridMaps[idx][j] =NULL;

            //z code

            i_grids[idx][j] = NULL;
            i_info[idx][j] = NULL;
        }
    }
}

uint64
Map::EnsureGridCreated(const GridPair &p)
{
    //char tmp[128];
    uint64 mask = CalculateGridMask(p.y_coord);

    if( !(i_gridMask[p.x_coord] & mask) )
    {
        Guard guard(*this);
        if( !(i_gridMask[p.x_coord] & mask) )
        {
            i_grids[p.x_coord][p.y_coord] = new NGridType(p.x_coord*MAX_NUMBER_OF_GRIDS + p.y_coord);
            i_info[p.x_coord][p.y_coord] = new GridInfo(i_gridExpiry,sWorld.getConfig(CONFIG_GRID_UNLOAD));
            i_gridMask[p.x_coord] |= mask;
            //z coord

            int gx=63-p.x_coord;
            int gy=63-p.y_coord;

            if(!GridMaps[gx][gy])
                GridMaps[gx][gy]=Map::LoadMAP(i_id,gx,gy);

        }
    }

    return mask;
}

void
Map::EnsureGridLoadedForPlayer(const Cell &cell, Player *player, bool add_player)
{
    uint64 mask = EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];

    assert(grid != NULL);
    if( !(i_gridStatus[cell.GridX()] & mask) )
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        if( !(i_gridStatus[cell.GridX()] & mask) )
        {
            if( player != NULL )
            {
                player->SendDelayResponse(MAX_GRID_LOAD_TIME);
                DEBUG_LOG("Player %s enter cell[%u,%u] triggers of loading grid[%u,%u] on map %u", player->GetName(), cell.CellX(), cell.CellY(), cell.GridX(), cell.GridY(), i_id);
            }
            else
                DEBUG_LOG("Player nearby triggers of loading grid [%u,%u] on map %u", cell.GridX(), cell.GridY(), i_id);

            ObjectGridLoader loader(*grid, i_id, cell);
            loader.LoadN();
            grid->SetGridState(GRID_STATE_ACTIVE);

            if( add_player && player != NULL )
                (*grid)(cell.CellX(), cell.CellY()).AddObject(player, player->GetGUID());
            i_gridStatus[cell.GridX()] |= mask;
        }
    }
    else if( add_player )
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).AddObject(player, player->GetGUID());
    }
}

void
Map::LoadGrid(const Cell& cell, bool no_unload)
{
    uint64 mask = EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];

    assert(grid != NULL);
    if( !(i_gridStatus[cell.GridX()] & mask) )
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        if( !(i_gridStatus[cell.GridX()] & mask) )
        {
            ObjectGridLoader loader(*grid, i_id, cell);
            loader.LoadN();
            i_gridStatus[cell.GridX()] |= mask;
            if(no_unload)
                i_info[cell.GridX()][cell.GridY()]->i_unloadflag = false;
        }
    }
}

void
Map::NotifyPlayerVisibility(const Cell &cell, const CellPair &cell_pair, Player *player)
{
    MaNGOS::PlayerNotifier pl_notifier(*player);
    MaNGOS::VisibleNotifier obj_notifier(*player);
    TypeContainerVisitor<MaNGOS::PlayerNotifier, ContainerMapList<Player> > player_notifier(pl_notifier);
    TypeContainerVisitor<MaNGOS::VisibleNotifier, TypeMapContainer<AllObjectTypes> > object_notifier(obj_notifier);

    CellLock<ReadGuard> cell_lock(cell, cell_pair);
    cell_lock->Visit(cell_lock, player_notifier, *this);
    cell_lock->Visit(cell_lock, object_notifier, *this);
    obj_notifier.Notify();
}

void Map::Add(Player *player)
{
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());

    assert( player && p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
        p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );

    Cell cell = RedZone::GetZone(p);
    EnsureGridLoadedForPlayer(cell, player, true);
    cell.data.Part.reserved = ALL_DISTRICT;
    NotifyPlayerVisibility(cell, p, player);
}

template<class T>
void SetCurrentCell(T*, Cell const&)
{
}

template<>
void SetCurrentCell(Creature* c, Cell const& cell)
{
    c->SetCurrentCell(cell);
}

template<class T>
void
Map::Add(T *obj)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());

    assert(obj);

    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        sLog.outError("Map::Add: Object " I64FMTD " have invalide coordiated X:%u Y:%u grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell = RedZone::GetZone(p);
    EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );

    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).template AddGridObject<T>(obj, obj->GetGUID());
        SetCurrentCell(obj,cell);
    }

    DEBUG_LOG("Object %u enters grid[%u,%u]", GUID_LOPART(obj->GetGUID()), cell.GridX(), cell.GridY());
    cell.data.Part.reserved = ALL_DISTRICT;

    MaNGOS::ObjectVisibleNotifier notifier(*static_cast<WorldObject *>(obj));
    TypeContainerVisitor<MaNGOS::ObjectVisibleNotifier, ContainerMapList<Player> > player_notifier(notifier);

    CellLock<ReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, player_notifier, *this);
}

template<class T>
bool
Map::Find(T *obj) const
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        sLog.outError("Map::Add: Object " I64FMTD " have invalide coordiated X:%u Y:%u grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return false;
    }

    Cell cell = RedZone::GetZone(p);
    if(!i_grids[cell.GridX()][cell.GridY()])
        return false;

    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );
    return ((*grid)(cell.CellX(),cell.CellY())).template GetGridObject<T>(obj->GetGUID())!=0;
}

template <class T>
T* Map::GetObjectNear(WorldObject const &obj, OBJECT_HANDLE hdl)
{
    return GetObjectNear<T>(obj.GetPositionX(), obj.GetPositionY(), hdl);
}

template <class T>
T* Map::GetObjectNear(float x, float y, OBJECT_HANDLE hdl)
{
    CellPair p = MaNGOS::ComputeCellPair(x,y);
    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        sLog.outError("Map::GetObjectNear: invalid coordiates supplied X:%u Y:%u grid cell [%u:%u]", x, y, p.x_coord, p.y_coord);
        return false;
    }

    CellPair xcell(p), cell_iter;
    xcell << 1;
    xcell -= 1;
    for(; abs(int(p.x_coord - xcell.x_coord)) < 2; xcell >> 1)
    {
        for(cell_iter = xcell; abs(int(p.y_coord - cell_iter.y_coord)) < 2; cell_iter += 1)
        {
            Cell cell = RedZone::GetZone(cell_iter);
            if(!i_grids[cell.GridX()][cell.GridY()])
                continue;

            NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
            assert( grid != NULL );

            T *result = ((*grid)(cell.CellX(),cell.CellY())).template GetGridObject<T>(hdl);
            if (result) return result;

            if (cell_iter.y_coord == TOTAL_NUMBER_OF_CELLS_PER_MAP-1)
                break;
        }

        if (cell_iter.x_coord == TOTAL_NUMBER_OF_CELLS_PER_MAP-1)
            break;
    }
    return NULL;
}

void Map::MessageBoardcast(Player *player, WorldPacket *msg, bool to_self, bool own_team_only)
{
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
        p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );

    Cell cell = RedZone::GetZone(p);
    cell.data.Part.reserved = ALL_DISTRICT;

    if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
        return;

    MaNGOS::MessageDeliverer post_man(*player, msg, to_self, own_team_only);
    TypeContainerVisitor<MaNGOS::MessageDeliverer, ContainerMapList<Player> > message(post_man);
    CellLock<ReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, message, *this);
}

void Map::MessageBoardcast(WorldObject *obj, WorldPacket *msg)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());

    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        sLog.outError("Map::MessageBoardcast: Object " I64FMTD " have invalide coordinates X:%u Y:%u grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell = RedZone::GetZone(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
        return;

    MaNGOS::ObjectMessageDeliverer post_man(*obj, msg);
    TypeContainerVisitor<MaNGOS::ObjectMessageDeliverer, ContainerMapList<Player> > message(post_man);
    CellLock<ReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, message, *this);
}

bool Map::loaded(const GridPair &p) const
{
    uint64 mask = CalculateGridMask(p.y_coord);
    return ( (i_gridMask[p.x_coord] & mask) != 0  && (i_gridStatus[p.x_coord] & mask) != 0 );
}

void Map::Update(const uint32 &t_diff)
{
    for(unsigned int i=0; i < MAX_NUMBER_OF_GRIDS; ++i)
    {
        uint64 mask = 1;
        for(unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
        {
            if( i_gridMask[i] & mask )
            {
                // move all creatures with delayed move and remove in grid before new grid moves (and grid unload)
                //ObjectAccessor::Instance().DoDelayedMovesAndRemoves();

                assert(i_grids[i][j] != NULL && i_info[i][j] != NULL);
                NGridType &grid(*i_grids[i][j]);
                GridInfo &info(*i_info[i][j]);
                si_GridStates[grid.GetGridState()]->Update(*this, grid, info, i, j, t_diff);

            }
            mask <<= 1;
        }
    }
}

void Map::Remove(Player *player, bool remove)
{
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
        p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );

    Cell cell = RedZone::GetZone(p);
    uint64 mask = CalculateGridMask(cell.data.Part.grid_y);

    if( !(i_gridMask[cell.data.Part.grid_x] & mask) )
    {
        assert( false );
        return;
    }

    DEBUG_LOG("Remove player %s from grid[%u,%u]", player->GetName(), cell.GridX(), cell.GridY());
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert(grid != NULL);

    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        grid->RemoveObject(cell.CellX(), cell.CellY(), player, player->GetGUID());
        player->RemoveFromWorld();
    }

    cell.data.Part.reserved = ALL_DISTRICT;
    MaNGOS::NotVisibleNotifier notifier(*player);
    TypeContainerVisitor<MaNGOS::NotVisibleNotifier, ContainerMapList<Player> > player_notifier(notifier);
    CellLock<ReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, player_notifier, *this);
    notifier.Notify();

    if( remove )
        delete player;
}

template<class T>
void
Map::Remove(T *obj, bool remove)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        sLog.outError("Map::Remove: Object " I64FMTD " have invalide coordiated X:%u Y:%u grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell = RedZone::GetZone(p);
    if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
        return;

    DEBUG_LOG("Remove object " I64FMTD " from grid[%u,%u]", obj->GetGUID(), cell.data.Part.grid_x, cell.data.Part.grid_y);
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );

    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).template RemoveGridObject<T>(obj, obj->GetGUID());
    }

    {
        Cell cell = RedZone::GetZone(p);
        cell.data.Part.reserved = ALL_DISTRICT;
        MaNGOS::ObjectNotVisibleNotifier notifier(*static_cast<WorldObject *>(obj));
        TypeContainerVisitor<MaNGOS::ObjectNotVisibleNotifier, ContainerMapList<Player> > player_notifier(notifier);
        CellLock<ReadGuard> cell_lock(cell, p);
        cell_lock->Visit(cell_lock, player_notifier, *this);
    }

    if( remove )
    {
        obj->SaveRespawnTime();
        delete obj;
    }
}

void
Map::PlayerRelocation(Player *player, float x, float y, float z, float orientation, bool visibilityChanges )
{
    assert(player);

    CellPair old_val = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    CellPair new_val = MaNGOS::ComputeCellPair(x, y);

    Cell old_cell = RedZone::GetZone(old_val);
    Cell new_cell = RedZone::GetZone(new_val);
    new_cell |= old_cell;
    bool same_cell = (new_cell == old_cell);
    player->Relocate(x, y, z, orientation);

    if( old_cell.DiffGrid(new_cell) || old_cell.DiffCell(new_cell) )
    {
        DEBUG_LOG("Player %s relocation grid[%u,%u]cell[%u,%u]->grid[%u,%u]cell[%u,%u]", player->GetName(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());

        NGridType &grid(*i_grids[old_cell.GridX()][old_cell.GridY()]);

        {
            assert(i_info[old_cell.GridX()][old_cell.GridY()] != NULL);

            WriteGuard guard(i_info[old_cell.GridX()][old_cell.GridY()]->i_lock);
            grid(old_cell.CellX(),old_cell.CellY()).RemoveObject(player, player->GetGUID());

            if( !old_cell.DiffGrid(new_cell) )
                grid(new_cell.CellX(),new_cell.CellY()).AddObject(player, player->GetGUID());
        }

        if( old_cell.DiffGrid(new_cell) )
            EnsureGridLoadedForPlayer(new_cell, player, true);
    }

    CellLock<ReadGuard> cell_lock(new_cell, new_val);

    MaNGOS::VisibleNotifier notifier(*player);

    if( !same_cell )
    {
        TypeContainerVisitor<MaNGOS::VisibleNotifier, ContainerMapList<Player> > player_notifier(notifier);
        cell_lock->Visit(cell_lock, player_notifier, *this);
    }

    MaNGOS::PlayerRelocationNotifier relocationNotifier(*player);
    new_cell.data.Part.reserved = ALL_DISTRICT;
    TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, TypeMapContainer<AllObjectTypes> > p2c_relocation(relocationNotifier);
    cell_lock->Visit(cell_lock, p2c_relocation, *this);
    TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, ContainerMapList<Player> > p2p_relocation(relocationNotifier);
    cell_lock->Visit(cell_lock, p2p_relocation, *this);

    if (visibilityChanges)
    {
        MaNGOS::VisibleChangesNotifier visualChangesNotifier(*player);
        TypeContainerVisitor<MaNGOS::VisibleChangesNotifier, ContainerMapList<Player> > player_Vnotifier(visualChangesNotifier);
        cell_lock->Visit(cell_lock, player_Vnotifier, *this);
    }

    if( same_cell )
        return;

    TypeContainerVisitor<MaNGOS::VisibleNotifier, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
    cell_lock->Visit(cell_lock, object_notifier, *this);
    notifier.Notify();

    MaNGOS::NotVisibleNotifier notifier2(*player);
    TypeContainerVisitor<MaNGOS::NotVisibleNotifier, ContainerMapList<Player> > player_notifier2(notifier2);
    TypeContainerVisitor<MaNGOS::NotVisibleNotifier, TypeMapContainer<AllObjectTypes> > object_notifier2(notifier2);
    cell_lock = CellLock<ReadGuard>(old_cell, old_val);
    cell_lock->Visit(cell_lock, player_notifier2, *this);
    cell_lock->Visit(cell_lock, object_notifier2, *this);
    i_grids[new_cell.GridX()][new_cell.GridY()]->SetGridState(GRID_STATE_ACTIVE);
    notifier2.Notify();
}

void
Map::CreatureRelocation(Creature *creature, float x, float y, float z, float ang)
{
    assert(CheckGridIntegrity(creature,false));

    Cell old_cell = creature->GetCurrentCell();

    CellPair new_val = MaNGOS::ComputeCellPair(x, y);
    Cell new_cell = RedZone::GetZone(new_val);

    // delay creature move for grid/cell to grid/cell moves
    if( old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell) )
    {
        #ifdef MANGOS_DEBUG
        if((sWorld.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
            MaNGOS::Singleton<Log>::Instance().outDebug("Creature (GUID: %u Entry: %u) added to moving list from grid[%u,%u]cell[%u,%u] to grid[%u,%u]cell[%u,%u].", creature->GetGUIDLow(), creature->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif
        AddCreatureToMoveList(creature,x,y,z,ang);
        // in diffcell/diffgrid case notifiers called at finishing move creature in Map::MoveAllCreaturesInMoveList
    }
    else
    {
        creature->Relocate(x, y, z, ang);
        CreatureRelocationNotifying(creature,new_cell,new_val);
    }
    assert(CheckGridIntegrity(creature,true));
}

void Map::CreatureRelocationNotifying(Creature *creature, Cell new_cell, CellPair new_val)
{
    if( !creature->hasUnitState(UNIT_STAT_CHASE | UNIT_STAT_SEARCHING | UNIT_STAT_FLEEING))
    {
        CellLock<ReadGuard> cell_lock(new_cell, new_val);
        MaNGOS::CreatureRelocationNotifier relocationNotifier(*creature);
        new_cell.data.Part.reserved = ALL_DISTRICT;
        new_cell.SetNoCreate();                             // not trigger load unloaded grids at notifier call
        TypeContainerVisitor<MaNGOS::CreatureRelocationNotifier, ContainerMapList<Player> > c2p_relocation(relocationNotifier);
        cell_lock->Visit(cell_lock, c2p_relocation, *this);
    }
}

void Map::AddCreatureToMoveList(Creature *c, float x, float y, float z, float ang)
{
    if(!c) return;

    i_creaturesToMove[c] = CreatureMover(x,y,z,ang);
}

void Map::MoveAllCreaturesInMoveList()
{
    while(!i_creaturesToMove.empty())
    {
        // get data and remove element;
        CreatureMoveList::iterator iter = i_creaturesToMove.begin();
        Creature* c = iter->first;
        CreatureMover cm = iter->second;
        i_creaturesToMove.erase(iter);

        // calculate cells
        CellPair new_val = MaNGOS::ComputeCellPair(cm.x, cm.y);
        Cell new_cell = RedZone::GetZone(new_val);

        // do move or do move to respawn or remove creature if previous all fail
        if(CreatureCellRelocation(c,new_cell))
        {
            // update pos
            c->Relocate(cm.x, cm.y, cm.z, cm.ang);
            CreatureRelocationNotifying(c,new_cell,new_cell.cellPair());
        }
        else
        {
            // if creature can't be move in new cell/grid (not loaded) move it to repawn cell/grid
            // creature coordinates will be updated and notifiers send
            if(!CreatureRespawnRelocation(c))
            {
                // ... or unload (if respawn grid also not loaded)
                #ifdef MANGOS_DEBUG
                if((sWorld.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
                    MaNGOS::Singleton<Log>::Instance().outDebug("Creature (GUID: %u Entry: %u ) can't be move to unloaded respawn grid.",c->GetGUIDLow(),c->GetEntry());
                #endif
                ObjectAccessor::Instance().AddObjectToRemoveList(c);
            }
        }
    }
}

bool Map::CreatureCellRelocation(Creature *c, Cell new_cell)
{
    Cell const& old_cell = c->GetCurrentCell();
    if(!old_cell.DiffGrid(new_cell) )                       // in same grid
    {
        // if in same cell then none do
        if(old_cell.DiffCell(new_cell))
        {
            #ifdef MANGOS_DEBUG
            if((sWorld.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
                MaNGOS::Singleton<Log>::Instance().outDebug("Creature (GUID: %u Entry: %u) moved in grid[%u,%u] from cell[%u,%u] to cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.CellX(), new_cell.CellY());
            #endif

            assert(i_info[old_cell.GridX()][old_cell.GridY()] != NULL);
            WriteGuard guard(i_info[old_cell.GridX()][old_cell.GridY()]->i_lock);
            if( !old_cell.DiffGrid(new_cell) )
            {
                (*i_grids[old_cell.GridX()][old_cell.GridY()])(old_cell.CellX(), old_cell.CellY()).RemoveGridObject<Creature>(c, c->GetGUID());
                (*i_grids[new_cell.GridX()][new_cell.GridY()])(new_cell.CellX(), new_cell.CellY()).AddGridObject<Creature>(c, c->GetGUID());
                c->SetCurrentCell(new_cell);
            }
        }
        else
        {
            #ifdef MANGOS_DEBUG
            if((sWorld.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
                MaNGOS::Singleton<Log>::Instance().outDebug("Creature (GUID: %u Entry: %u) move in same grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY());
            #endif
        }
    }
    else                                                    // in diff. grids
    if(loaded(GridPair(new_cell.GridX(), new_cell.GridY())))
    {
        #ifdef MANGOS_DEBUG
        if((sWorld.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
            MaNGOS::Singleton<Log>::Instance().outDebug("Creature (GUID: %u Entry: %u) moved from grid[%u,%u]cell[%u,%u] to grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif

        {
            WriteGuard guard(i_info[old_cell.GridX()][old_cell.GridY()]->i_lock);
            (*i_grids[old_cell.GridX()][old_cell.GridY()])(old_cell.CellX(), old_cell.CellY()).RemoveGridObject<Creature>(c, c->GetGUID());
        }
        {
            EnsureGridCreated(GridPair(new_cell.GridX(), new_cell.GridY()));
            WriteGuard guard(i_info[new_cell.GridX()][new_cell.GridY()]->i_lock);
            (*i_grids[new_cell.GridX()][new_cell.GridY()])(new_cell.CellX(), new_cell.CellY()).AddGridObject<Creature>(c, c->GetGUID());
            c->SetCurrentCell(new_cell);
        }
    }
    else
    {
        #ifdef MANGOS_DEBUG
        if((sWorld.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
            MaNGOS::Singleton<Log>::Instance().outDebug("Creature (GUID: %u Entry: %u) attempt move from grid[%u,%u]cell[%u,%u] to unloaded grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif
        return false;
    }

    return true;
}

bool Map::CreatureRespawnRelocation(Creature *c)
{
    float resp_x, resp_y, resp_z;
    c->GetRespawnCoord(resp_x, resp_y, resp_z);

    CellPair resp_val = MaNGOS::ComputeCellPair(resp_x, resp_y);
    Cell resp_cell = RedZone::GetZone(resp_val);

    c->CombatStop();
    (*c)->Clear();

    #ifdef MANGOS_DEBUG
    if((sWorld.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
        MaNGOS::Singleton<Log>::Instance().outDebug("Creature (GUID: %u Entry: %u) will moved from grid[%u,%u]cell[%u,%u] to respawn grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), c->GetCurrentCell().GridX(), c->GetCurrentCell().GridY(), c->GetCurrentCell().CellX(), c->GetCurrentCell().CellY(), resp_cell.GridX(), resp_cell.GridY(), resp_cell.CellX(), resp_cell.CellY());
    #endif

    // teleport it to respawn point (like normal respawn if player see)
    if(CreatureCellRelocation(c,resp_cell))
    {
        c->Relocate(resp_x, resp_y, resp_z, c->GetOrientation());
        CreatureRelocationNotifying(c,resp_cell,resp_cell.cellPair());
        return true;
    }
    else
        return false;
}

bool Map::UnloadGrid(const uint32 &x, const uint32 &y)
{
    NGridType *grid = i_grids[x][y];
    assert( grid != NULL && i_info[x][y] != NULL );

    {
        if( ObjectAccessor::Instance().PlayersNearGrid(x, y, i_id) )
            return false;

        DEBUG_LOG("Unloading grid[%u,%u] for map %u", x,y, i_id);
        ObjectGridUnloader unloader(*grid);

        // Finish creature moves, remove and delete all creatures with delayed remove before moving to respawn grids
        // Must know real mob position before move
        ObjectAccessor::Instance().DoDelayedMovesAndRemoves();

        // move creatures to respawn grids if this is diff.grid or to remove list
        unloader.MoveToRespawnN();

        // Finish creature moves, remove and delete all creatures with delayed remove before unload
        ObjectAccessor::Instance().DoDelayedMovesAndRemoves();

        {
            WriteGuard guard(i_info[x][y]->i_lock);
            uint64 mask = CalculateGridMask(y);
            i_gridMask[x] &= ~mask;
            i_gridStatus[x] &= ~mask;
            unloader.UnloadN();
            delete i_grids[x][y];
            i_grids[x][y] = NULL;
        }
    }
    delete i_info[x][y];
    i_info[x][y] = NULL;

    //z coordinate

    int gx=63-x;
    int gy=63-y;
    if(GridMaps[gx][gy])
    {
        delete (GridMaps[gx][gy]);
        GridMaps[gx][gy]=NULL;

    }

    //z coordinate
    DEBUG_LOG("Unloading grid[%u,%u] for map %u finished", x,y, i_id);
    return true;
}

void Map::UnloadAll()
{
    for(unsigned int i=0; i < MAX_NUMBER_OF_GRIDS; ++i)
    {
        uint64 mask = 1;
        for(unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
        {
            if( i_gridMask[i] & mask )
                UnloadGrid(i, j);

            mask <<= 1;
        }
    }
}

void Map::GetUnitList(float x, float y, std::list<Unit*> &unlist)
{

    CellPair p = MaNGOS::ComputeCellPair(x, y);
    assert( p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
        p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );

    Cell cell = RedZone::GetZone(p);

    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();
    MaNGOS::GridUnitListNotifier notifier(unlist);
    TypeContainerVisitor<MaNGOS::GridUnitListNotifier, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
    CellLock<GridReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, object_notifier, *this);
}

float Map::GetHeight(float x, float y )
{
    //local x,y coords
    float lx,ly;
    int gx,gy;
    GridPair p = MaNGOS::ComputeGridPair(x, y);
    //Note: p.x_coord = 63-gx...
    /* method with no opt
        x=32* SIZE_OF_GRIDS-x;
        y=32* SIZE_OF_GRIDS-y;

        gx=x/SIZE_OF_GRIDS ;//grid x
        gy=y/SIZE_OF_GRIDS ;//grid y

        lx= x*(MAP_RESOLUTION/SIZE_OF_GRIDS) - gx*MAP_RESOLUTION;
        ly= y*(MAP_RESOLUTION/SIZE_OF_GRIDS) - gy*MAP_RESOLUTION;
    */

    // half opt method
    gx=(int)(32-x/SIZE_OF_GRIDS) ;                          //grid x
    gy=(int)(32-y/SIZE_OF_GRIDS);                           //grid y

    lx=MAP_RESOLUTION*(32 -x/SIZE_OF_GRIDS - gx);
    ly=MAP_RESOLUTION*(32 -y/SIZE_OF_GRIDS - gy);
    //DEBUG_LOG("my %d %d si %d %d",gx,gy,p.x_coord,p.y_coord);

    if(!GridMaps[gx][gy])                                   //this map is not loaded
        GridMaps[gx][gy]=Map::LoadMAP(i_id,gx,gy);

    if(GridMaps[gx][gy])
        return GridMaps[gx][gy]->Z[(int)(lx)][(int)(ly)];
    else
        return 0;
}

uint16 Map::GetAreaFlag(float x, float y )
{
    //local x,y coords
    float lx,ly;
    int gx,gy;
    GridPair p = MaNGOS::ComputeGridPair(x, y);
    //Note: p.x_coord = 63-gx...
    /* method with no opt
        x=32* SIZE_OF_GRIDS-x;
        y=32* SIZE_OF_GRIDS-y;

        gx=x/SIZE_OF_GRIDS ;//grid x
        gy=y/SIZE_OF_GRIDS ;//grid y

        lx= x*(MAP_RESOLUTION/SIZE_OF_GRIDS) - gx*MAP_RESOLUTION;
        ly= y*(MAP_RESOLUTION/SIZE_OF_GRIDS) - gy*MAP_RESOLUTION;
    */

    // half opt method
    gx=(int)(32-x/SIZE_OF_GRIDS) ;                          //grid x
    gy=(int)(32-y/SIZE_OF_GRIDS);                           //grid y

    lx=16*(32 -x/SIZE_OF_GRIDS - gx);
    ly=16*(32 -y/SIZE_OF_GRIDS - gy);
    //DEBUG_LOG("my %d %d si %d %d",gx,gy,p.x_coord,p.y_coord);

    if(!GridMaps[gx][gy])                                   //this map is not loaded
        GridMaps[gx][gy]=Map::LoadMAP(i_id,gx,gy);

    if(GridMaps[gx][gy])
        return GridMaps[gx][gy]->area_flag[(int)(lx)][(int)(ly)];
    else
        return 0;

}

uint8 Map::GetTerrainType(float x, float y )
{
    //local x,y coords
    float lx,ly;
    int gx,gy;
    //GridPair p = MaNGOS::ComputeGridPair(x, y);
    //Note: p.x_coord = 63-gx...
    /* method with no opt
        x=32* SIZE_OF_GRIDS-x;
        y=32* SIZE_OF_GRIDS-y;

        gx=x/SIZE_OF_GRIDS ;//grid x
        gy=y/SIZE_OF_GRIDS ;//grid y

        lx= x*(MAP_RESOLUTION/SIZE_OF_GRIDS) - gx*MAP_RESOLUTION;
        ly= y*(MAP_RESOLUTION/SIZE_OF_GRIDS) - gy*MAP_RESOLUTION;
    */

    // half opt method
    gx=(int)(32-x/SIZE_OF_GRIDS) ;                          //grid x
    gy=(int)(32-y/SIZE_OF_GRIDS);                           //grid y

    lx=16*(32 -x/SIZE_OF_GRIDS - gx);
    ly=16*(32 -y/SIZE_OF_GRIDS - gy);

    if(!GridMaps[gx][gy])                                   //this map is not loaded
        GridMaps[gx][gy]=Map::LoadMAP(i_id,gx,gy);

    if(GridMaps[gx][gy])
        return GridMaps[gx][gy]->terrain_type[(int)(lx)][(int)(ly)];
    else
        return 0;

}

float Map::GetWaterLevel(float x, float y )
{
    //local x,y coords
    float lx,ly;
    int gx,gy;
    //GridPair p = MaNGOS::ComputeGridPair(x, y);
    //Note: p.x_coord = 63-gx...
    /* method with no opt
        x=32* SIZE_OF_GRIDS-x;
        y=32* SIZE_OF_GRIDS-y;

        gx=x/SIZE_OF_GRIDS ;//grid x
        gy=y/SIZE_OF_GRIDS ;//grid y

        lx= x*(MAP_RESOLUTION/SIZE_OF_GRIDS) - gx*MAP_RESOLUTION;
        ly= y*(MAP_RESOLUTION/SIZE_OF_GRIDS) - gy*MAP_RESOLUTION;
    */

    // half opt method
    gx=(int)(32-x/SIZE_OF_GRIDS) ;                          //grid x
    gy=(int)(32-y/SIZE_OF_GRIDS);                           //grid y

    lx=128*(32 -x/SIZE_OF_GRIDS - gx);
    ly=128*(32 -y/SIZE_OF_GRIDS - gy);

    if(!GridMaps[gx][gy])                                   //this map is not loaded
        GridMaps[gx][gy]=Map::LoadMAP(i_id,gx,gy);

    if(GridMaps[gx][gy])
        return GridMaps[gx][gy]->liquid_level[(int)(lx)][(int)(ly)];
    else
        return 0;

}

uint32 Map::GetAreaId(uint16 areaflag)
{
    AreaTableEntry const *entry = GetAreaEntryByAreaFlag(areaflag);

    if (entry)
        return entry->ID;
    else
        return 0;
}

uint32 Map::GetZoneId(uint16 areaflag)
{
    AreaTableEntry const *entry = GetAreaEntryByAreaFlag(areaflag);

    if( entry )
        return ( entry->zone != 0 ) ? entry->zone : entry->ID;
    else
        return 0;
}

bool Map::IsInWater(float x, float y)
{
    float z = GetHeight(x,y);
    float water_z = GetWaterLevel(x,y);
    uint8 flag = GetTerrainType(x,y);
    return (z < (water_z-2)) && (flag & 0x01);
}

bool Map::IsUnderWater(float x, float y, float z)
{
    float water_z = GetWaterLevel(x,y);
    uint8 flag = GetTerrainType(x,y);
    return (z < (water_z-2)) && (flag & 0x01);
}

bool Map::CheckGridIntegrity(Creature* c, bool moved) const
{
    Cell const& cur_cell = c->GetCurrentCell();

    if(!i_grids[cur_cell.GridX()][cur_cell.GridY()] ||
        (*i_grids[cur_cell.GridX()][cur_cell.GridY()])(cur_cell.CellX(), cur_cell.CellY()).GetGridObject<Creature>(c->GetGUID())!=c)
    {
        sLog.outError("ERROR: %s (GUID: %u) not find in %s grid[%u,%u]cell[%u,%u]",
            (c->GetTypeId()==TYPEID_PLAYER ? "Player" : "Creature"),c->GetGUIDLow(), (moved ? "final" : "original"),
            cur_cell.GridX(), cur_cell.GridY(), cur_cell.CellX(), cur_cell.CellY());
        return true;                                        // not crash at error, just output error in debug mode
    }

    CellPair xy_val = MaNGOS::ComputeCellPair(c->GetPositionX(), c->GetPositionY());
    Cell xy_cell = RedZone::GetZone(xy_val);
    if(xy_cell != cur_cell)
    {
        sLog.outError("ERROR: %s (GUID: %u) X: %f Y: %f (%s) in grid[%u,%u]cell[%u,%u] instead grid[%u,%u]cell[%u,%u]",
            (c->GetTypeId()==TYPEID_PLAYER ? "Player" : "Creature"),c->GetGUIDLow(),
            c->GetPositionX(),c->GetPositionY(),(moved ? "final" : "original"),
            cur_cell.GridX(), cur_cell.GridY(), cur_cell.CellX(), cur_cell.CellY(),
            xy_cell.GridX(),  xy_cell.GridY(),  xy_cell.CellX(),  xy_cell.CellY());
        return true;                                        // not crash at error, just output error in debug mode
    }

    return true;
}

template void Map::Add(Creature *);
template void Map::Add(GameObject *);
template void Map::Add(DynamicObject *);
template void Map::Add(Corpse *);

template void Map::Remove(Creature *,bool);
template void Map::Remove(GameObject *, bool);
template void Map::Remove(DynamicObject *, bool);
template void Map::Remove(Corpse *, bool);

template bool Map::Find(Creature *) const;
template bool Map::Find(GameObject *) const;
template bool Map::Find(DynamicObject *) const;
template bool Map::Find(Corpse *) const;

template Creature* Map::GetObjectNear(WorldObject const &obj, OBJECT_HANDLE hdl);
template Creature* Map::GetObjectNear(float x, float y, OBJECT_HANDLE hdl);
template GameObject* Map::GetObjectNear(WorldObject const &obj, OBJECT_HANDLE hdl);
template GameObject* Map::GetObjectNear(float x, float y, OBJECT_HANDLE hdl);
template DynamicObject* Map::GetObjectNear(WorldObject const &obj, OBJECT_HANDLE hdl);
template DynamicObject* Map::GetObjectNear(float x, float y, OBJECT_HANDLE hdl);
template Corpse* Map::GetObjectNear(WorldObject const &obj, OBJECT_HANDLE hdl);
template Corpse* Map::GetObjectNear(float x, float y, OBJECT_HANDLE hdl);
