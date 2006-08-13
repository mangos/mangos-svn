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

#define DEFAULT_GRID_EXPIRY     300
#define MAX_GRID_LOAD_TIME      50

static GridState* si_GridStates[MAX_GRID_STATE];

inline
bool FileExists(const char * fn)
{
    FILE *pf=fopen(fn,"rb");
    if(!pf)return false;
    fclose(pf);
    return true;
}

GridMap * Map::LoadMAP(int mapid,int x,int y)
{
    char *tmp;
    static bool showcheckmapInfo=false;
    static int oldx=0,oldy=0;

    std::string dataPath="./";

    if(!sConfig.GetString("DataDir",&dataPath))
        dataPath="./";
    else
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
            i_info[p.x_coord][p.y_coord] = new GridInfo(i_gridExpiry);
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

    assert( p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
        p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );

    Cell cell = RedZone::GetZone(p);
    EnsureGridLoadedForPlayer(cell, player, true);
    cell.data.Part.reserved = ALL_DISTRICT;
    NotifyPlayerVisibility(cell, p, player);
}

template<class T>
void
Map::Add(T *obj)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        DEBUG_LOG("Map::Add: Object %lu have invalide coordiated X:%u Y:%u grid cell [%u:%u]", (unsigned long)obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell = RedZone::GetZone(p);
    EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );

    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).template AddGridObject<T>(obj, obj->GetGUID());
    }

    DEBUG_LOG("Object %lu enters grid[%u,%u]", (unsigned long)obj->GetGUID(), cell.GridX(), cell.GridY());
    cell.data.Part.reserved = ALL_DISTRICT;

    MaNGOS::ObjectVisibleNotifier notifier(*static_cast<Object *>(obj));
    TypeContainerVisitor<MaNGOS::ObjectVisibleNotifier, ContainerMapList<Player> > player_notifier(notifier);

    CellLock<ReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, player_notifier, *this);
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

void Map::MessageBoardcast(Object *obj, WorldPacket *msg)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());

    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        DEBUG_LOG("Map::MessageBoardcast: Object %lu have invalide coordiated X:%u Y:%u grid cell [%u:%u]", (unsigned long)obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
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
    return ( (i_gridMask[p.x_coord] & mask)  && (i_gridStatus[p.x_coord] & mask) );
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
        DEBUG_LOG("Map::Remove: Object %lu have invalide coordiated X:%u Y:%u grid cell [%u:%u]", (unsigned long)obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell = RedZone::GetZone(p);
    if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
        return;

    DEBUG_LOG("Remove object %lu from grid[%u,%u]", (unsigned long)obj->GetGUID(), cell.data.Part.grid_x, cell.data.Part.grid_y);
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );

    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).template RemoveGridObject<T>(obj, obj->GetGUID());
    }

    {
        Cell cell = RedZone::GetZone(p);
        CellLock<ReadGuard> cell_lock(cell, p);
        MaNGOS::ObjectNotVisibleNotifier notifier(*obj);
        TypeContainerVisitor<MaNGOS::ObjectNotVisibleNotifier, ContainerMapList<Player> > player_notifier(notifier);

    }

    if( remove )
        delete obj;
}

void
Map::PlayerRelocation(Player *player, float x, float y, float z, float orientation)
{
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
    CellPair old_val = MaNGOS::ComputeCellPair(creature->GetPositionX(), creature->GetPositionY());
    CellPair new_val = MaNGOS::ComputeCellPair(x, y);

    Cell old_cell = RedZone::GetZone(old_val);
    Cell new_cell = RedZone::GetZone(new_val);
    creature->Relocate(x, y, z, ang);

    if( creature->hasUnitState(UNIT_STAT_CHASE | UNIT_STAT_SEARCHING | UNIT_STAT_FLEEING) )
    {
        if( old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell) )
        {
            DEBUG_LOG("Creature "I64FMT" moved from grid[%u,%u]cell[%u,%u] to grid[%u,%u]cell[%u,%u].", creature->GetGUID(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());

            {
                assert(i_info[old_cell.GridX()][old_cell.GridY()] != NULL);
                WriteGuard guard(i_info[old_cell.GridX()][old_cell.GridY()]->i_lock);
                (*i_grids[old_cell.GridX()][old_cell.GridY()])(old_cell.CellX(), old_cell.CellY()).RemoveGridObject<Creature>(creature, creature->GetGUID());
                if( !old_cell.DiffGrid(new_cell) )
                    (*i_grids[new_cell.GridX()][new_cell.GridY()])(new_cell.CellX(), new_cell.CellY()).AddGridObject<Creature>(creature, creature->GetGUID());
            }

            if( old_cell.DiffGrid(new_cell) )
            {
                EnsureGridCreated(GridPair(new_cell.GridX(), new_cell.GridY()));
                WriteGuard guard(i_info[new_cell.GridX()][new_cell.GridY()]->i_lock);
                (*i_grids[new_cell.GridX()][new_cell.GridY()])(new_cell.CellX(), new_cell.CellY()).AddGridObject<Creature>(creature, creature->GetGUID());
            }
        }
    }
    else
    {
        CellLock<ReadGuard> cell_lock(new_cell, new_val);
        MaNGOS::CreatureRelocationNotifier relocationNotifier(*creature);
        new_cell.data.Part.reserved = ALL_DISTRICT;
        TypeContainerVisitor<MaNGOS::CreatureRelocationNotifier, ContainerMapList<Player> > c2p_relocation(relocationNotifier);
        cell_lock->Visit(cell_lock, c2p_relocation, *this);
    }

}

bool Map::UnloadGrid(const uint32 &x, const uint32 &y)
{
    NGridType *grid = i_grids[x][y];
    assert( grid != NULL && i_info[x][y] != NULL );

    {
        if( ObjectAccessor::Instance().PlayersNearGrid(x, y, i_id) )
            return false;

        WriteGuard guard(i_info[x][y]->i_lock);
        DEBUG_LOG("Unloading grid[%u,%u] for map %u", x,y, i_id);
        ObjectGridUnloader unloader(*grid);
        uint64 mask = CalculateGridMask(y);
        i_gridMask[x] &= ~mask;
        i_gridStatus[x] &= ~mask;
        unloader.UnloadN();
        delete i_grids[x][y];
        i_grids[x][y] = NULL;

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
    return true;
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

    lx=16*(32 -x/SIZE_OF_GRIDS - gx);
    ly=16*(32 -y/SIZE_OF_GRIDS - gy);

    if(!GridMaps[gx][gy])                                   //this map is not loaded
        GridMaps[gx][gy]=Map::LoadMAP(i_id,gx,gy);

    if(GridMaps[gx][gy])
        return GridMaps[gx][gy]->liquid_level[(int)(lx)][(int)(ly)];
    else
        return 0;

}

uint32 Map::GetAreaId(uint16 areaflag)
{
    AreaTableEntry *entry = sAreaStore.LookupEntry(areaflag);

    if (entry)
        return entry->ID;
    else
        return 0;
}

uint32 Map::GetZoneId(uint16 areaflag)
{
    AreaTableEntry *entry = sAreaStore.LookupEntry(areaflag);

    if( entry )
        return ( entry->zone != 0 ) ? entry->zone : entry->ID;
    else
        return 0;
}

template void Map::Add(Creature *);
template void Map::Add(GameObject *);
template void Map::Add(DynamicObject *);
template void Map::Add(Corpse *);

template void Map::Remove(Creature *,bool);
template void Map::Remove(GameObject *, bool);
template void Map::Remove(DynamicObject *, bool);
template void Map::Remove(Corpse *, bool);
