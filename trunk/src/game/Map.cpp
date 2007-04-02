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
#include "ObjectAccessor.h"

#include "MapManager.h"
#include "MapInstanced.h"

#define DEFAULT_GRID_EXPIRY     300
#define MAX_GRID_LOAD_TIME      50

// magic *.map header
const char MAP_MAGIC[] = "MAP_1.01";

static GridState* si_GridStates[MAX_GRID_STATE];

bool Map::ExistMAP(uint32 mapid,int x,int y, bool output)
{
    int len = sWorld.GetDataPath().length()+strlen("maps/%03u%02u%02u.map")+1;
    char* tmp = new char[len];
    snprintf(tmp, len, (char *)(sWorld.GetDataPath()+"maps/%03u%02u%02u.map").c_str(),mapid,x,y);

    FILE *pf=fopen(tmp,"rb");

    if(!pf)
    {
        if(output)
            sLog.outError("Check existing of map file '%s': not exist!",tmp);
        delete[] tmp;
        return false;
    }

    char magic[8];
    fread(magic,1,8,pf);
    if(strncmp(MAP_MAGIC,magic,8))
    {
        sLog.outError("Map file '%s' is non-compatible version (outdated?). Please, create new using ad.exe program.",tmp);
        delete [] tmp;
        fclose(pf);                                         //close file before return
        return false;
    }

    delete [] tmp;
    fclose(pf);

    return true;
}

GridMap * Map::LoadMAP(uint32 mapid, uint32 instanceid, int x,int y)
{
    //check map file existance (but not output error)
    if(!Map::ExistMAP(mapid,x,y,false))
        return NULL;

    if (instanceid != 0)
    {
        // load from base map
        if (GridMaps[x][y]) GridMaps[x][y]=NULL;

        if (!MapManager::Instance().GetBaseMap(mapid)->GridMaps[x][y])
        {
            // load grids for base map
            MapManager::Instance().GetBaseMap(mapid)->GridMaps[x][y] = Map::LoadMAP(mapid, 0, x, y);
        }

        return(MapManager::Instance().GetBaseMap(mapid)->GridMaps[x][y]);
    }

    if(GridMaps[x][y])                                      //map already load, delete it before reloading
    {
        sLog.outDetail("Unloading already loaded map %u before reloading.",mapid);
        delete (GridMaps[x][y]);
        GridMaps[x][y]=NULL;
    }

    // map file name
    char *tmp=NULL;
    // Pihhan: dataPath length + "maps/" + 3+2+2+ ".map" length may be > 32 !
    int len = sWorld.GetDataPath().length()+strlen("maps/%03u%02u%02u.map")+1;
    tmp = new char[len];
    snprintf(tmp, len, (char *)(sWorld.GetDataPath()+"maps/%03u%02u%02u.map").c_str(),mapid,x,y);
    sLog.outBasic("Loading map %s",tmp);

    // loading data
    FILE *pf=fopen(tmp,"rb");
    char magic[8];
    fread(magic,1,8,pf);
    delete []  tmp;
    GridMap * buf= new GridMap;
    if(!buf)                                                //unexpected error
    {
        fclose(pf);
        return NULL;
    }
    fread(buf,1,sizeof(GridMap),pf);
    fclose(pf);
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

Map::Map(uint32 id, time_t expiry, uint32 ainstanceId) : i_id(id), i_gridExpiry(expiry), i_mapEntry (sMapStore.LookupEntry(id)), i_InstanceId(ainstanceId)
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

    if (Instanceable())
    {
        sLog.outDetail("INSTANCEMAP: Loading instance template");
        QueryResult* result = sDatabase.PQuery("SELECT `instance_template`.`maxplayers`, `instance_template`.`reset_delay`, `instance`.`resettime` FROM `instance_template` LEFT JOIN `instance` ON ((`instance_template`.`map` = `instance`.`map`) AND (`instance`.`id` = '%u')) WHERE `instance_template`.`map` = '%u'", i_InstanceId, id);
        if (result)
        {
            Field* fields = result->Fetch();
            i_maxPlayers = fields[0].GetUInt32();
            i_resetDelayTime = fields[1].GetUInt32();
            i_resetTime = (time_t) fields[2].GetUInt64();
            if (i_resetTime == 0) InitResetTime();
        }
        delete result;
    }

    i_Players.clear();
}

// Template specialization of utility methods
template<class T>
void Map::AddToGrid(T* obj, NGridType *grid, Cell const& cell)
{
    WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
    (*grid)(cell.CellX(), cell.CellY()).template AddGridObject<T>(obj, obj->GetGUID());
}

template<>
void Map::AddToGrid(Player* obj, NGridType *grid, Cell const& cell)
{
    WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
    (*grid)(cell.CellX(), cell.CellY()).AddWorldObject(obj, obj->GetGUID());
}

template<>
void Map::AddToGrid(Corpse* obj, NGridType *grid, Cell const& cell)
{
    // add to world object registry in grid
    if(obj->GetType()==CORPSE_RESURRECTABLE)
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).AddWorldObject<Corpse>(obj, obj->GetGUID());
    }
    // add to grid object store
    else
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).AddGridObject<Corpse>(obj, obj->GetGUID());
    }
}

template<>
void Map::AddToGrid(Creature* obj, NGridType *grid, Cell const& cell)
{
    // add to world object registry in grid
    if(obj->isPet())
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).AddWorldObject<Creature>(obj, obj->GetGUID());
        obj->SetCurrentCell(cell);
    }
    // add to grid object store
    else
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).AddGridObject<Creature>(obj, obj->GetGUID());
        obj->SetCurrentCell(cell);
    }
}

template<class T>
void Map::RemoveFromGrid(T* obj, NGridType *grid, Cell const& cell)
{
    WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
    (*grid)(cell.CellX(), cell.CellY()).template RemoveGridObject<T>(obj, obj->GetGUID());
}

template<>
void Map::RemoveFromGrid(Player* obj, NGridType *grid, Cell const& cell)
{
    WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
    (*grid)(cell.CellX(), cell.CellY()).RemoveWorldObject(obj, obj->GetGUID());
}

template<>
void Map::RemoveFromGrid(Corpse* obj, NGridType *grid, Cell const& cell)
{
    // remove from world object registry in grid
    if(obj->GetType()==CORPSE_RESURRECTABLE)
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).RemoveWorldObject<Corpse>(obj, obj->GetGUID());
    }
    // remove from grid object store
    else
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).RemoveGridObject<Corpse>(obj, obj->GetGUID());
    }
}

template<>
void Map::RemoveFromGrid(Creature* obj, NGridType *grid, Cell const& cell)
{
    // remove from world object registry in grid
    if(obj->isPet())
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).RemoveWorldObject<Creature>(obj, obj->GetGUID());
    }
    // remove from grid object store
    else
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).RemoveGridObject<Creature>(obj, obj->GetGUID());
    }
}

template<class T>
void Map::DeleteFromWorld(T* obj)
{
    // Note: In case resurrectable corpse and pet its removed from gloabal lists in own destructors
    delete obj;
}

template<class T>
T* Map::FindInGrid(uint64 guid, NGridType *grid, Cell const& cell) const
{
    return ((*grid)(cell.CellX(),cell.CellY())).template GetGridObject<T>(guid);
}

template<>
Player* Map::FindInGrid(uint64 guid, NGridType *grid, Cell const& cell) const
{
    return ((*grid)(cell.CellX(),cell.CellY())).GetWorldObject<Player>(guid);
}

template<>
Corpse* Map::FindInGrid(uint64 guid, NGridType *grid, Cell const& cell) const
{
    Corpse* obj = ((*grid)(cell.CellX(),cell.CellY())).GetWorldObject<Corpse>(guid);
    if(obj)
        return obj;
    return ((*grid)(cell.CellX(),cell.CellY())).GetGridObject<Corpse>(guid);
}

template<>
Creature* Map::FindInGrid(uint64 guid, NGridType *grid, Cell const& cell) const
{
    Creature* obj = ((*grid)(cell.CellX(),cell.CellY())).GetWorldObject<Creature>(guid);
    if(obj)
        return obj;
    return ((*grid)(cell.CellX(),cell.CellY())).GetGridObject<Creature>(guid);
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
                GridMaps[gx][gy]=Map::LoadMAP(i_id,i_InstanceId,gx,gy);

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
            {
                DEBUG_LOG("Player nearby triggers of loading grid [%u,%u] on map %u", cell.GridX(), cell.GridY(), i_id);
            }

            ObjectGridLoader loader(*grid, this, cell);
            loader.LoadN();

            // Add resurrectable corpses to world object list in grid
            ObjectAccessor::Instance().AddCorpsesToGrid(GridPair(cell.GridX(),cell.GridY()),(*grid)(cell.CellX(), cell.CellY()), this);

            grid->SetGridState(GRID_STATE_ACTIVE);

            if( add_player && player != NULL )
                (*grid)(cell.CellX(), cell.CellY()).AddWorldObject(player, player->GetGUID());
            i_gridStatus[cell.GridX()] |= mask;

        }
    }
    else if( player && add_player )
        AddToGrid(player,grid,cell);
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
            ObjectGridLoader loader(*grid, this, cell);
            loader.LoadN();

            // Add resurrectable corpses to world obect list in grid
            ObjectAccessor::Instance().AddCorpsesToGrid(GridPair(cell.GridX(),cell.GridY()),(*grid)(cell.CellX(), cell.CellY()), this);

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
    TypeContainerVisitor<MaNGOS::PlayerNotifier, WorldTypeMapContainer > player_notifier(pl_notifier);
    TypeContainerVisitor<MaNGOS::VisibleNotifier, WorldTypeMapContainer > world_object_notifier(obj_notifier);
    TypeContainerVisitor<MaNGOS::VisibleNotifier, GridTypeMapContainer > grid_object_notifier(obj_notifier);

    CellLock<ReadGuard> cell_lock(cell, cell_pair);
    cell_lock->Visit(cell_lock, player_notifier, *this);
    cell_lock->Visit(cell_lock, world_object_notifier, *this);
    cell_lock->Visit(cell_lock, grid_object_notifier, *this);
    obj_notifier.Notify();
}

bool Map::AddInstanced(Player *player)
{
    if (!Instanceable())
    {
        sLog.outDetail("MAP: Player '%s' entered the non-instanceable map '%s'", player->GetName(), GetMapName());
        if (player->GetPet()) player->GetPet()->SetInstanceId(0);
        player->SetInstanceId(0);
        return(true);
    }

    for (list< Player* >::iterator i = i_Players.begin(); i != i_Players.end(); i++)
    {
        if (*i == player)
        {
            sLog.outDetail("MAP: Player '%s' already in instance '%u' of map '%s'", player->GetName(), GetInstanceId(), GetMapName());
            return(true);
        }
    }

    // TODO: Not sure about checking player level: already done in HandleAreaTriggerOpcode
    // GM's still can teleport player in instance. 
    // Is it needed?

    {
        Guard guard(*this);

        // GMs can avoid player limits
        if ((i_Players.size() >= i_maxPlayers) && (!player->isGameMaster()))
        {
            sLog.outDetail("MAP: Instance '%u' of map '%s' cannot have more than '%u' players. Player '%s' rejected", GetInstanceId(), GetMapName(), i_maxPlayers, player->GetName() );
            player->GetSession()->SendAreaTriggerMessage("Sorry, the instance is full (%u players maximum)", i_maxPlayers);
            return(false);
        }

        i_Players.push_back(player);
    }

    if (player->GetPet()) player->GetPet()->SetInstanceId(this->GetInstanceId());
    player->SetInstanceId(this->GetInstanceId());

    player->SendInitWorldStates(player->GetMapId());

    sLog.outDetail("MAP: Player '%s' entered the instance '%u' of map '%s'", player->GetName(), GetInstanceId(), GetMapName());

    // reinitialize reset time
    InitResetTime();

    return(true);
}

void Map::RemoveInstanced(Player *player)
{
    i_Players.remove(player);

    // reinitialize reset time
    InitResetTime();
}

void Map::InitResetTime()
{
    i_resetTime = time(NULL) + i_resetDelayTime;

    if (Instanceable() && GetInstanceId())
    {
        sDatabase.BeginTransaction();
        sDatabase.PExecute("DELETE FROM `instance` WHERE `id` = '%u'", GetInstanceId());
        sDatabase.PExecute("INSERT INTO `instance` VALUES ('%u', '%u', '" I64FMTD "')", GetInstanceId(), i_id, (uint64)this->i_resetTime);
        sDatabase.CommitTransaction();
    }
}

void Map::Reset()
{
    sDatabase.PExecute("DELETE FROM `creature_respawn` WHERE `instance` = '%u'", GetInstanceId());
    sDatabase.PExecute("DELETE FROM `gameobject_respawn` WHERE `instance` = '%u'", GetInstanceId());

    UnloadAll();

    // reinitialize reset time
    InitResetTime();
}

bool Map::CanEnter(Player* player)
{
    if (!Instanceable()) return(true);
    
    // GMs can avoid raid limitations
    if (IsRaid() && (!player->isGameMaster()))
    {
        Group* group = player->groupInfo.group;
        if (!group || !group->isRaidGroup())
        {
            player->GetSession()->SendAreaTriggerMessage("You must be in a raid group to enter %s instance", GetMapName());
            sLog.outDebug("MAP: Player '%s' must be in a raid group to enter instance of '%s'", GetMapName());
            return(false);
        }
    }
    
    if (!player->isAlive())
    {
        player->GetCorpse()->GetMapId();
        if (player->GetCorpse()->GetMapId() != GetId())
        {
            player->GetSession()->SendAreaTriggerMessage("You cannot enter %s while in a ghost mode", GetMapName());
            sLog.outDebug("MAP: Player '%s' doesn't has a corpse in instance '%s' and can't enter", player->GetName(), GetMapName());
            return(false);
        }
        sLog.outDebug("MAP: Player '%s' has corpse in instance '%s' and can enter", player->GetName(), GetMapName());
    }

    return(true);
}

void Map::Add(Player *player)
{
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());

    assert( player && p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
        p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );

    if (player->GetPet()) player->GetPet()->SetInstanceId(this->GetInstanceId());
    player->SetInstanceId(this->GetInstanceId());

    Cell cell = RedZone::GetZone(p);
    EnsureGridLoadedForPlayer(cell, player, true);
    cell.data.Part.reserved = ALL_DISTRICT;
    NotifyPlayerVisibility(cell, p, player);

    // reinitialize reset time
    InitResetTime();
}

template<class T>
void
Map::Add(T *obj)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());

    assert(obj);

    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        sLog.outError("Map::Add: Object " I64FMTD " have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell = RedZone::GetZone(p);
    EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );

    AddToGrid(obj,grid,cell);

    DEBUG_LOG("Object %u enters grid[%u,%u]", GUID_LOPART(obj->GetGUID()), cell.GridX(), cell.GridY());
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();                                 // not create cells at object add notifier (if player near then cell will already loaded)

    MaNGOS::ObjectVisibleNotifier notifier(*static_cast<WorldObject *>(obj));
    TypeContainerVisitor<MaNGOS::ObjectVisibleNotifier, WorldTypeMapContainer > player_notifier(notifier);

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
        sLog.outError("Map::Add: Object " I64FMTD " have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return false;
    }

    Cell cell = RedZone::GetZone(p);
    if(!i_grids[cell.GridX()][cell.GridY()])
        return false;

    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );
    return this->FindInGrid<T>(obj->GetGUID(),grid,cell)!=0;
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

            T *result = FindInGrid<T>(hdl,grid,cell);
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
    TypeContainerVisitor<MaNGOS::MessageDeliverer, WorldTypeMapContainer > message(post_man);
    CellLock<ReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, message, *this);
}

void Map::MessageBoardcast(WorldObject *obj, WorldPacket *msg)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());

    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        sLog.outError("Map::MessageBoardcast: Object " I64FMTD " have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell = RedZone::GetZone(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
        return;

    MaNGOS::ObjectMessageDeliverer post_man(*obj, msg);
    TypeContainerVisitor<MaNGOS::ObjectMessageDeliverer, WorldTypeMapContainer > message(post_man);
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
    if (Instanceable())
    {
        sLog.outDetail("MAP: Removing player '%s' from instance '%u' of map '%s' before relocating to other map", player->GetName(), GetInstanceId(), GetMapName());
        RemoveInstanced(player); // remove from instance player list, etc.
    }

    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
        p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );

    Cell cell = RedZone::GetZone(p);
    uint64 mask = CalculateGridMask(cell.data.Part.grid_y);

    if( !(i_gridMask[cell.data.Part.grid_x] & mask) )
    {
        // assert( false );
        return;
    }

    DEBUG_LOG("Remove player %s from grid[%u,%u]", player->GetName(), cell.GridX(), cell.GridY());
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert(grid != NULL);

    RemoveFromGrid(player,grid,cell);
    player->RemoveFromWorld();

    cell.data.Part.reserved = ALL_DISTRICT;
    MaNGOS::NotVisibleNotifier notifier(*player);
    TypeContainerVisitor<MaNGOS::NotVisibleNotifier, WorldTypeMapContainer > player_notifier(notifier);
    CellLock<ReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, player_notifier, *this);
    notifier.Notify();

    if( remove )
        DeleteFromWorld(player);

    // reinitialize reset time
    InitResetTime();
}

bool Map::RemoveBones(uint64 guid, float x, float y)
{
    if (IsRemovalGrid(x, y))
    {
        Corpse *corpse = GetObjectNear<Corpse>(x, y, guid);
        if(corpse)
            corpse->DeleteBonesFromWorld();
        else
            return false;
    }
    return true;
}

template<class T>
void
Map::Remove(T *obj, bool remove)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        sLog.outError("Map::Remove: Object " I64FMTD " have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell = RedZone::GetZone(p);
    if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
        return;

    DEBUG_LOG("Remove object " I64FMTD " from grid[%u,%u]", obj->GetGUID(), cell.data.Part.grid_x, cell.data.Part.grid_y);
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );

    RemoveFromGrid(obj,grid,cell);

    {
        Cell cell = RedZone::GetZone(p);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();                                 // not create cells at object remove
        MaNGOS::ObjectNotVisibleNotifier notifier(*static_cast<WorldObject *>(obj));
        TypeContainerVisitor<MaNGOS::ObjectNotVisibleNotifier, WorldTypeMapContainer > player_notifier(notifier);
        CellLock<ReadGuard> cell_lock(cell, p);
        cell_lock->Visit(cell_lock, player_notifier, *this);
    }

    if( remove )
    {
        // if option set then object already saved at this moment
        if(!sWorld.getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATLY))
            obj->SaveRespawnTime();
        DeleteFromWorld(obj);
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

            RemoveFromGrid(player,&grid,old_cell);

            if( !old_cell.DiffGrid(new_cell) )
                AddToGrid(player,&grid,new_cell);
        }

        if( old_cell.DiffGrid(new_cell) )
            EnsureGridLoadedForPlayer(new_cell, player, true);
    }

    CellLock<ReadGuard> cell_lock(new_cell, new_val);

    MaNGOS::VisibleNotifier notifier(*player);

    if( !same_cell || player->IsBeingTeleported() )
    {
        if( player->IsBeingTeleported() )
            new_cell.data.Part.reserved = ALL_DISTRICT;

        TypeContainerVisitor<MaNGOS::VisibleNotifier, WorldTypeMapContainer > world_object_notifier(notifier);
        TypeContainerVisitor<MaNGOS::VisibleNotifier, GridTypeMapContainer >  grid_object_notifier(notifier);

        cell_lock->Visit(cell_lock, world_object_notifier, *this);
        cell_lock->Visit(cell_lock, grid_object_notifier, *this);

        notifier.Notify();
    }

    MaNGOS::PlayerRelocationNotifier relocationNotifier(*player);
    new_cell.data.Part.reserved = ALL_DISTRICT;

    TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, GridTypeMapContainer >  p2grid_relocation(relocationNotifier);
    TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, WorldTypeMapContainer > p2world_relocation(relocationNotifier);

    cell_lock->Visit(cell_lock, p2grid_relocation, *this);
    cell_lock->Visit(cell_lock, p2world_relocation, *this);

    if (visibilityChanges)
    {
        MaNGOS::VisibleChangesNotifier visualChangesNotifier(*player);
        TypeContainerVisitor<MaNGOS::VisibleChangesNotifier, WorldTypeMapContainer > player_Vnotifier(visualChangesNotifier);
        cell_lock->Visit(cell_lock, player_Vnotifier, *this);
    }

    if( same_cell )
        return;

    MaNGOS::NotVisibleNotifier notifier2(*player);
    TypeContainerVisitor<MaNGOS::NotVisibleNotifier, WorldTypeMapContainer > world_object_notifier2(notifier2);
    TypeContainerVisitor<MaNGOS::NotVisibleNotifier, GridTypeMapContainer >  grid_object_notifier2(notifier2);

    cell_lock = CellLock<ReadGuard>(old_cell, old_val);

    cell_lock->Visit(cell_lock, world_object_notifier2, *this);
    cell_lock->Visit(cell_lock, grid_object_notifier2, *this);

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
        if((sLog.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
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

        TypeContainerVisitor<MaNGOS::CreatureRelocationNotifier, WorldTypeMapContainer > c2world_relocation(relocationNotifier);
        TypeContainerVisitor<MaNGOS::CreatureRelocationNotifier, GridTypeMapContainer >  c2grid_relocation(relocationNotifier);

        cell_lock->Visit(cell_lock, c2world_relocation, *this);
        cell_lock->Visit(cell_lock, c2grid_relocation, *this);
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
                if((sLog.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
                    MaNGOS::Singleton<Log>::Instance().outDebug("Creature (GUID: %u Entry: %u ) can't be move to unloaded respawn grid.",c->GetGUIDLow(),c->GetEntry());
                #endif
                c->CleanupCrossRefsBeforeDelete();
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
            if((sLog.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
                MaNGOS::Singleton<Log>::Instance().outDebug("Creature (GUID: %u Entry: %u) moved in grid[%u,%u] from cell[%u,%u] to cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.CellX(), new_cell.CellY());
            #endif

            assert(i_info[old_cell.GridX()][old_cell.GridY()] != NULL);
            if( !old_cell.DiffGrid(new_cell) )
            {
                RemoveFromGrid(c,i_grids[old_cell.GridX()][old_cell.GridY()],old_cell);
                AddToGrid(c,i_grids[new_cell.GridX()][new_cell.GridY()],new_cell);
                c->SetCurrentCell(new_cell);
            }
        }
        else
        {
            #ifdef MANGOS_DEBUG
            if((sLog.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
                MaNGOS::Singleton<Log>::Instance().outDebug("Creature (GUID: %u Entry: %u) move in same grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY());
            #endif
        }
    }
    else                                                    // in diff. grids
    if(loaded(GridPair(new_cell.GridX(), new_cell.GridY())))
    {
        #ifdef MANGOS_DEBUG
        if((sLog.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
            MaNGOS::Singleton<Log>::Instance().outDebug("Creature (GUID: %u Entry: %u) moved from grid[%u,%u]cell[%u,%u] to grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif

        RemoveFromGrid(c,i_grids[old_cell.GridX()][old_cell.GridY()],old_cell);
        {
            EnsureGridCreated(GridPair(new_cell.GridX(), new_cell.GridY()));
            AddToGrid(c,i_grids[new_cell.GridX()][new_cell.GridY()],new_cell);
        }
    }
    else
    {
        #ifdef MANGOS_DEBUG
        if((sLog.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
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

    c->CombatStop(true);
    (*c)->Clear();

    #ifdef MANGOS_DEBUG
    if((sLog.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
        MaNGOS::Singleton<Log>::Instance().outDebug("Creature (GUID: %u Entry: %u) will moved from grid[%u,%u]cell[%u,%u] to respawn grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), c->GetCurrentCell().GridX(), c->GetCurrentCell().GridY(), c->GetCurrentCell().CellX(), c->GetCurrentCell().CellY(), resp_cell.GridX(), resp_cell.GridY(), resp_cell.CellX(), resp_cell.CellY());
    #endif

    // teleport it to respawn point (like normal respawn if player see)
    if(CreatureCellRelocation(c,resp_cell))
    {
        c->Relocate(resp_x, resp_y, resp_z, c->GetOrientation());
        (*c)->Initialize();                                 // prevent possible problems with default move generators
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

    // delete grid map, but don't delete grid map if it is from parent map
    // FIXME: we must delete also in case single loaded instance or none loaded instances
    if(GridMaps[gx][gy])
    {
        if (i_InstanceId == 0) delete (GridMaps[gx][gy]);
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
        GridMaps[gx][gy]=Map::LoadMAP(i_id,i_InstanceId,gx,gy);

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
        GridMaps[gx][gy]=Map::LoadMAP(i_id,i_InstanceId,gx,gy);

    if(GridMaps[gx][gy])
        return GridMaps[gx][gy]->area_flag[(int)(lx)][(int)(ly)];
    // this used while not all *.map files generated (instances)
    else
        return GetAreaFlagByMapId(i_id);
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
        GridMaps[gx][gy]=Map::LoadMAP(i_id,i_InstanceId,gx,gy);

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
        GridMaps[gx][gy]=Map::LoadMAP(i_id,i_InstanceId,gx,gy);

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
        FindInGrid<Creature>(c->GetGUID(),i_grids[cur_cell.GridX()][cur_cell.GridY()],cur_cell)!=c)
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

template void Map::Add(Corpse *);
template void Map::Add(Creature *);
template void Map::Add(GameObject *);
template void Map::Add(DynamicObject *);

template void Map::Remove(Corpse *,bool);
template void Map::Remove(Creature *,bool);
template void Map::Remove(GameObject *, bool);
template void Map::Remove(DynamicObject *, bool);

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
