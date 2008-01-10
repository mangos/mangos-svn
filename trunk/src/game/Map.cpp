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
#include "CellImpl.h"
#include "InstanceData.h"
#include "Map.h"
#include "GridNotifiersImpl.h"
#include "Config/ConfigEnv.h"
#include "Transports.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "World.h"
#include "ScriptCalls.h"

#include "MapManager.h"
#include "MapInstanced.h"
#include "VMapFactory.h"

#define DEFAULT_GRID_EXPIRY     300
#define MAX_GRID_LOAD_TIME      50

// magic *.map header
const char MAP_MAGIC[] = "MAP_1.02";

static GridState* si_GridStates[MAX_GRID_STATE];

Map::~Map()
{
    if(i_data)
    {
        delete i_data;
        i_data = NULL;
    }
}

bool Map::ExistMap(uint32 mapid,int x,int y)
{
    int len = sWorld.GetDataPath().length()+strlen("maps/%03u%02u%02u.map")+1;
    char* tmp = new char[len];
    snprintf(tmp, len, (char *)(sWorld.GetDataPath()+"maps/%03u%02u%02u.map").c_str(),mapid,x,y);

    FILE *pf=fopen(tmp,"rb");

    if(!pf)
    {
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

bool Map::ExistVMap(uint32 mapid,int x,int y)
{
    if(VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager())
    {
        if(vmgr->isMapLoadingEnabled())
        {
                                                            // x and y are swaped !!
            bool exists = vmgr->existsMap((sWorld.GetDataPath()+ "vmaps").c_str(),  mapid, y,x);
            if(!exists)
            {
                std::string name = vmgr->getDirFileName(mapid,x,y);
                sLog.outError("Could not find vmap file '%s'", (sWorld.GetDataPath()+"vmaps/"+name).c_str());
                return false;
            }
        }
    }

    return true;
}

void Map::LoadVMap(int x,int y)
{
                                                            // x and y are swaped !!
    int vmapLoadResult = VMAP::VMapFactory::createOrGetVMapManager()->loadMap((sWorld.GetDataPath()+ "vmaps").c_str(),  GetId(), y,x);
    switch(vmapLoadResult)
    {
        case VMAP::VMAP_LOAD_RESULT_OK:
            sLog.outDetail("VMAP loaded name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", GetMapName(), GetId(), x,y, y,x);
            break;
        case VMAP::VMAP_LOAD_RESULT_ERROR:
            sLog.outDetail("Could not load VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", GetMapName(), GetId(), x,y, y,x);
            break;
        case VMAP::VMAP_LOAD_RESULT_IGNORED:
            DEBUG_LOG("Ignored VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", GetMapName(), GetId(), x,y, y,x);
            break;
    }
}

void Map::LoadMap(uint32 mapid, uint32 instanceid, int x,int y)
{
    if( instanceid != 0 )
    {
        if(GridMaps[x][y])
            return;

        Map* baseMap = const_cast<Map*>(MapManager::Instance().GetBaseMap(mapid));

        // load gridmap for base map
        if (!baseMap->GridMaps[x][y])
            baseMap->EnsureGridCreated(GridPair(63-x,63-y));

        if (!baseMap->GridMaps[x][y])
            return;

        ((MapInstanced*)(baseMap))->AddGridMapReference(GridPair(x,y));
        baseMap->SetUnloadFlag(GridPair(63-x,63-y), false);
        GridMaps[x][y] = baseMap->GridMaps[x][y];
        return;
    }

    //map already load, delete it before reloading (Is it neccessary? Do we really need the abilty the reload maps during runtime?)
    if(GridMaps[x][y])
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
    sLog.outDetail("Loading map %s",tmp);
    // loading data
    FILE *pf=fopen(tmp,"rb");
    if(!pf)
    {
        delete [] tmp;
        return;
    }

    char magic[8];
    fread(magic,1,8,pf);
    if(strncmp(MAP_MAGIC,magic,8))
    {
        sLog.outError("Map file '%s' is non-compatible version (outdated?). Please, create new using ad.exe program.",tmp);
        delete [] tmp;
        fclose(pf);                                         //close file before return
        return;
    }
    delete []  tmp;

    GridMap * buf= new GridMap;
    fread(buf,1,sizeof(GridMap),pf);
    fclose(pf);

    GridMaps[x][y] = buf;
}

void Map::LoadMapAndVMap(uint32 mapid, uint32 instanceid, int x,int y)
{
    LoadMap(mapid,instanceid,x,y);
    LoadVMap(x, y);
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

Map::Map(uint32 id, time_t expiry, uint32 ainstanceId)
: i_id(id), i_gridExpiry(expiry), i_mapEntry (sMapStore.LookupEntry(id)),
i_resetTime(0), i_resetDelayTime(0), i_InstanceId(ainstanceId), i_maxPlayers(0), i_data(NULL)
{
    for(unsigned int idx=0; idx < MAX_NUMBER_OF_GRIDS; ++idx)
    {
        i_gridMask[idx] = 0;
        i_gridStatus[idx] = 0;
        for(unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
        {
            //z code
            GridMaps[idx][j] =NULL;

            i_grids[idx][j] = NULL;
            i_info[idx][j] = NULL;
        }
    }

    if (Instanceable())
    {
        sLog.outDetail("INSTANCEMAP: Loading instance template");

        InstanceTemplate const* mInstance = objmgr.GetInstanceTemplate(id);
        if (mInstance)
        {
            i_maxPlayers = mInstance->maxPlayers;
            i_resetDelayTime = mInstance->reset_delay;
            i_script = mInstance->script;
            i_data = Script->CreateInstanceData(this);
            if(i_data)
            {
                sLog.outDebug("New instance data, \"%s\" ,initialized!",i_script.c_str());
                i_data->Initialize();
            }
        }
        else
        {
            i_maxPlayers = 0;
            i_resetDelayTime = 0;
            sLog.outErrorDb("Instance (Map: %u) not have records in `instance_template` in DB. Using default settings.",id);
        }

        if(i_InstanceId!=0)
        {
            QueryResult* result = CharacterDatabase.PQuery("SELECT `resettime`, `data` FROM `instance` WHERE `map` = '%u' AND `id` = '%u'", id, i_InstanceId);
            if (result)
            {
                Field* fields = result->Fetch();
                i_resetTime = (time_t) fields[0].GetUInt64();

                const char* data = fields[1].GetString();
                if(data && i_data)
                {
                    sLog.outDebug("Loading instance data for `%s` with id %u", i_script.c_str(), i_InstanceId);
                    i_data->Load(data);
                }

                delete result;
            }
        }

        if (i_resetTime == 0) InitResetTime();
    }
    else
        i_data = NULL;

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
void Map::AddToGrid(Corpse *obj, NGridType *grid, Cell const& cell)
{
    // add to world object registry in grid
    if(obj->GetType()==CORPSE_RESURRECTABLE)
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).AddWorldObject(obj, obj->GetGUID());
    }
    // add to grid object store
    else
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).AddGridObject(obj, obj->GetGUID());
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
void Map::RemoveFromGrid(Corpse *obj, NGridType *grid, Cell const& cell)
{
    // remove from world object registry in grid
    if(obj->GetType()==CORPSE_RESURRECTABLE)
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).RemoveWorldObject(obj, obj->GetGUID());
    }
    // remove from grid object store
    else
    {
        WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
        (*grid)(cell.CellX(), cell.CellY()).RemoveGridObject(obj, obj->GetGUID());
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
void Map::AddNotifier(T* , Cell const& , CellPair const& )
{
}

template<>
void Map::AddNotifier(Player* obj, Cell const& cell, CellPair const& cellpair)
{
    PlayerRelocationNotify(obj,cell,cellpair);
}

template<>
void Map::AddNotifier(Creature* obj, Cell const& cell, CellPair const& cellpair)
{
    CreatureRelocationNotify(obj,cell,cellpair);
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

            i_grids[p.x_coord][p.y_coord]->SetGridState(GRID_STATE_IDLE);

            //z coord
            int gx=63-p.x_coord;
            int gy=63-p.y_coord;

            if(!GridMaps[gx][gy])
                Map::LoadMapAndVMap(i_id,i_InstanceId,gx,gy);
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

            ResetGridExpiry(*i_info[cell.GridX()][cell.GridY()], 0.1f);
            grid->SetGridState(GRID_STATE_ACTIVE);

            if( add_player && player != NULL )
                (*grid)(cell.CellX(), cell.CellY()).AddWorldObject(player, player->GetGUID());
            i_gridStatus[cell.GridX()] |= mask;
            LoadVMap(63-cell.GridX(),63-cell.GridY());
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

            // Add resurrectable corpses to world object list in grid
            ObjectAccessor::Instance().AddCorpsesToGrid(GridPair(cell.GridX(),cell.GridY()),(*grid)(cell.CellX(), cell.CellY()), this);

            i_gridStatus[cell.GridX()] |= mask;
            if(no_unload)
                i_info[cell.GridX()][cell.GridY()]->i_unloadflag = false;

        }
    }
    LoadVMap(63-cell.GridX(),63-cell.GridY());
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

    for (PlayerList::iterator i = i_Players.begin(); i != i_Players.end(); i++)
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
        if (i_maxPlayers && (i_Players.size() >= i_maxPlayers) && (!player->isGameMaster()))
        {
            sLog.outDetail("MAP: Instance '%u' of map '%s' cannot have more than '%u' players. Player '%s' rejected", GetInstanceId(), GetMapName(), i_maxPlayers, player->GetName());
            player->SendTransferAborted(GetId(), TRANSFER_ABORT_MAX_PLAYERS);
            return(false);
        }

        if(i_data)
            i_data->OnPlayerEnter(player);
        i_Players.push_back(player);
    }

    if (player->GetPet()) player->GetPet()->SetInstanceId(this->GetInstanceId());
    player->SetInstanceId(this->GetInstanceId());

    player->SendInitWorldStates();

    sLog.outDetail("MAP: Player '%s' entered the instance '%u' of map '%s'", player->GetName(), GetInstanceId(), GetMapName());

    // reinitialize reset time
    InitResetTime();

    if (IsRaid())
        player->SendRaidInstanceResetWarning(RAID_INSTANCE_WELCOME, GetId(), i_resetTime-time(NULL));

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
    // for i_resetDelayTime==0 call single time for i_resetTime==0
    if (Instanceable() && (i_resetDelayTime != 0 || i_resetTime == 0) )
    {
        i_resetTime = time(NULL) + i_resetDelayTime;        // only used for Instanceable() case

        if(GetInstanceId())
        {
            const char* instance_data = i_data ? i_data->Save() : "";
            CharacterDatabase.BeginTransaction();
            CharacterDatabase.PExecute("DELETE FROM `instance` WHERE `id` = '%u'", GetInstanceId());
            CharacterDatabase.PExecute("INSERT INTO `instance` VALUES ('%u', '%u', '" I64FMTD "','%s')", GetInstanceId(), i_id, (uint64)this->i_resetTime, instance_data);
            CharacterDatabase.CommitTransaction();
        }
    }
}

void Map::Reset()
{
    objmgr.DeleteRespawnTimeForInstance(GetInstanceId());

    UnloadAll();

    // reinitialize reset time
    InitResetTime();
}

bool Map::CanEnter(Player* player) const
{
    if (!Instanceable()) return(true);

    // GMs can avoid raid limitations
    if (IsRaid() && (!player->isGameMaster() && !sWorld.getConfig(CONFIG_INSTANCE_IGNORE_RAID)))
    {
        Group* group = player->GetGroup();
        if (!group || !group->isRaidGroup())
        {
            // probably there must be special opcode, because client has this string constant in GlobalStrings.lua
            player->GetSession()->SendAreaTriggerMessage("You must be in a raid group to enter %s instance", GetMapName());
            sLog.outDebug("MAP: Player '%s' must be in a raid group to enter instance of '%s'", player->GetName(), GetMapName());
            return(false);
        }
    }

    if (!player->isAlive())
    {
        if(Corpse *corpse = player->GetCorpse())
        {
            if (corpse->GetMapId() != GetId())
            {
                player->GetSession()->SendAreaTriggerMessage("You cannot enter %s while in a ghost mode", GetMapName());
                sLog.outDebug("MAP: Player '%s' doesn't has a corpse in instance '%s' and can't enter", player->GetName(), GetMapName());
                return(false);
            }
            sLog.outDebug("MAP: Player '%s' has corpse in instance '%s' and can enter", player->GetName(), GetMapName());
        }
        else
        {
            sLog.outDebug("Map::CanEnter - player '%s' is dead but doesn't have a corpse!", player->GetName());
        }
    }

    if(i_data && i_data->IsEncounterInProgress())
    {
        sLog.outDebug("MAP: Player '%s' can't enter instance '%s' while an encounter is in progress.", player->GetName(), GetMapName());
        player->SendTransferAborted(GetId(), TRANSFER_ABORT_ZONE_IN_COMBAT);
        return(false);
    }

    return(true);
}

void Map::Add(Player *player)
{
    if (player->GetPet()) player->GetPet()->SetInstanceId(this->GetInstanceId());
    player->SetInstanceId(this->GetInstanceId());

    player->AddToWorld();

    SendInitSelf(player);
    SendInitTransports(player);

    // update player state for other player and visa-versa
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    Cell cell(p);
    EnsureGridLoadedForPlayer(cell, player, true);

    UpdatePlayerVisibility(player,cell,p);
    UpdateObjectsVisibilityFor(player,cell,p);

    AddNotifier(player,cell,p);

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

    Cell cell(p);
    EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );

    AddToGrid(obj,grid,cell);
    obj->AddToWorld();

    DEBUG_LOG("Object %u enters grid[%u,%u]", GUID_LOPART(obj->GetGUID()), cell.GridX(), cell.GridY());

    UpdateObjectVisibility(obj,cell,p);

    AddNotifier(obj,cell,p);
}

/*template<class T>
bool
Map::Find(T *obj) const
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        sLog.outError("Map::Find: Object " I64FMTD " have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return false;
    }

    Cell cell(p);
    if(!i_grids[cell.GridX()][cell.GridY()])
        return false;

    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );
    return this->FindInGrid<T>(obj->GetGUID(),grid,cell, (T*)NULL)!=0;
}

template <class T>
T* Map::GetObjectNear(float x, float y, OBJECT_HANDLE hdl, T *fake)
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
            Cell cell(cell_iter);
            if(!i_grids[cell.GridX()][cell.GridY()])
                continue;

            NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
            assert( grid != NULL );

            T *result = FindInGrid<T>(hdl,grid,cell, fake);
            if (result) return result;

            if (cell_iter.y_coord == TOTAL_NUMBER_OF_CELLS_PER_MAP-1)
                break;
        }

        if (cell_iter.x_coord == TOTAL_NUMBER_OF_CELLS_PER_MAP-1)
            break;
    }
    return NULL;
}*/

void Map::MessageBroadcast(Player *player, WorldPacket *msg, bool to_self, bool own_team_only)
{
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());

    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        sLog.outError("Map::MessageBroadcast: Player (GUID: %u) have invalid coordinates X:%f Y:%f grid cell [%u:%u]", player->GetGUIDLow(), player->GetPositionX(), player->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;

    if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
        return;

    MaNGOS::MessageDeliverer post_man(*player, msg, to_self, own_team_only);
    TypeContainerVisitor<MaNGOS::MessageDeliverer, WorldTypeMapContainer > message(post_man);
    CellLock<ReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, message, *this);
}

void Map::MessageBroadcast(WorldObject *obj, WorldPacket *msg)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());

    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        sLog.outError("Map::MessageBroadcast: Object " I64FMTD " have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
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
    // Don't unload grids if it's battleground, since we may have manually added GOs,creatures, those doesn't load from DB at grid re-load !
    // This isn't really bother us, since as soon as we have instanced BG-s, the whole map unloads as the BG gets ended
    if ((i_id == 489) || (i_id == 529))                     // WS, AB
        return;

    for(unsigned int i=0; i < MAX_NUMBER_OF_GRIDS; ++i)
    {
        if(!i_gridMask[i])
            continue;

        for(unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
        {
            if( i_gridMask[i] & (uint64(1) << j) )
            {
                // move all creatures with delayed move and remove in grid before new grid moves (and grid unload)
                //ObjectAccessor::Instance().DoDelayedMovesAndRemoves();

                assert(i_grids[i][j] != NULL && i_info[i][j] != NULL);
                NGridType &grid(*i_grids[i][j]);
                GridInfo &info(*i_info[i][j]);
                si_GridStates[grid.GetGridState()]->Update(*this, grid, info, i, j, t_diff);
            }
        }
    }

    if(i_data)
        i_data->Update(t_diff);
}

void Map::Remove(Player *player, bool remove)
{
    if (Instanceable())
    {
        sLog.outDetail("MAP: Removing player '%s' from instance '%u' of map '%s' before relocating to other map", player->GetName(), GetInstanceId(), GetMapName());
        RemoveInstanced(player);                            // remove from instance player list, etc.
    }

    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        // invalid coordinates
        player->RemoveFromWorld();

        if( remove )
            DeleteFromWorld(player);

        return;
    }

    Cell cell(p);
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

    SendRemoveTransports(player);

    UpdateObjectsVisibilityFor(player,cell,p);

    if( remove )
        DeleteFromWorld(player);

    // reinitialize reset time
    InitResetTime();
}

bool Map::RemoveBones(uint64 guid, float x, float y)
{
    if (IsRemovalGrid(x, y))
    {
        Corpse * corpse = ObjectAccessor::Instance().GetObjectInWorld(GetId(), x, y, guid, (Corpse*)NULL);
        if(corpse && corpse->GetTypeId() == TYPEID_CORPSE && corpse->GetType() == CORPSE_BONES)
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

    Cell cell(p);
    if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
        return;

    DEBUG_LOG("Remove object " I64FMTD " from grid[%u,%u]", obj->GetGUID(), cell.data.Part.grid_x, cell.data.Part.grid_y);
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );

    RemoveFromGrid(obj,grid,cell);
    obj->RemoveFromWorld();

    UpdateObjectVisibility(obj,cell,p);

    if( remove )
    {
        // if option set then object already saved at this moment
        if(!sWorld.getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATLY))
            obj->SaveRespawnTime();
        DeleteFromWorld(obj);
    }
}

void
Map::PlayerRelocation(Player *player, float x, float y, float z, float orientation)
{
    assert(player);

    CellPair old_val = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    CellPair new_val = MaNGOS::ComputeCellPair(x, y);

    Cell old_cell(old_val);
    Cell new_cell(new_val);
    new_cell |= old_cell;
    bool same_cell = (new_cell == old_cell);

    player->Relocate(x, y, z, orientation);

    if( old_cell.DiffGrid(new_cell) || old_cell.DiffCell(new_cell) )
    {
        DEBUG_LOG("Player %s relocation grid[%u,%u]cell[%u,%u]->grid[%u,%u]cell[%u,%u]", player->GetName(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());

        // update player position for group at taxi flight
        if(player->GetGroup() && player->isInFlight())
            player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POSITION);

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

    // if move then update what player see and who seen
    UpdatePlayerVisibility(player,new_cell,new_val);
    UpdateObjectsVisibilityFor(player,new_cell,new_val);
    PlayerRelocationNotify(player,new_cell,new_val);

    if( !same_cell && i_grids[new_cell.GridX()][new_cell.GridY()]->GetGridState()!=GRID_STATE_ACTIVE )
    {
        ResetGridExpiry(*i_info[new_cell.GridX()][new_cell.GridY()], 0.1f);
        i_grids[new_cell.GridX()][new_cell.GridY()]->SetGridState(GRID_STATE_ACTIVE);
    }
}

void
Map::CreatureRelocation(Creature *creature, float x, float y, float z, float ang)
{
    assert(CheckGridIntegrity(creature,false));

    Cell old_cell = creature->GetCurrentCell();

    CellPair new_val = MaNGOS::ComputeCellPair(x, y);
    Cell new_cell(new_val);

    // delay creature move for grid/cell to grid/cell moves
    if( old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell) )
    {
        #ifdef MANGOS_DEBUG
        if((sLog.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
            sLog.outDebug("Creature (GUID: %u Entry: %u) added to moving list from grid[%u,%u]cell[%u,%u] to grid[%u,%u]cell[%u,%u].", creature->GetGUIDLow(), creature->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        #endif
        AddCreatureToMoveList(creature,x,y,z,ang);
        // in diffcell/diffgrid case notifiers called at finishing move creature in Map::MoveAllCreaturesInMoveList
    }
    else
    {
        creature->Relocate(x, y, z, ang);
        CreatureRelocationNotify(creature,new_cell,new_val);
    }
    assert(CheckGridIntegrity(creature,true));
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
        Cell new_cell(new_val);

        // do move or do move to respawn or remove creature if previous all fail
        if(CreatureCellRelocation(c,new_cell))
        {
            // update pos
            c->Relocate(cm.x, cm.y, cm.z, cm.ang);
            CreatureRelocationNotify(c,new_cell,new_cell.cellPair());
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
                    sLog.outDebug("Creature (GUID: %u Entry: %u ) can't be move to unloaded respawn grid.",c->GetGUIDLow(),c->GetEntry());
                #endif
                c->CleanupsBeforeDelete();
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
                sLog.outDebug("Creature (GUID: %u Entry: %u) moved in grid[%u,%u] from cell[%u,%u] to cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.CellX(), new_cell.CellY());
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
                sLog.outDebug("Creature (GUID: %u Entry: %u) move in same grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY());
            #endif
        }
    }
    else                                                    // in diff. grids
    if(loaded(GridPair(new_cell.GridX(), new_cell.GridY())))
    {
        #ifdef MANGOS_DEBUG
        if((sLog.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
            sLog.outDebug("Creature (GUID: %u Entry: %u) moved from grid[%u,%u]cell[%u,%u] to grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
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
            sLog.outDebug("Creature (GUID: %u Entry: %u) attempt move from grid[%u,%u]cell[%u,%u] to unloaded grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
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
    Cell resp_cell(resp_val);

    c->CombatStop();
    c->GetMotionMaster()->Clear();

    #ifdef MANGOS_DEBUG
    if((sLog.getLogFilter() & LOG_FILTER_CREATURE_MOVES)==0)
        sLog.outDebug("Creature (GUID: %u Entry: %u) will moved from grid[%u,%u]cell[%u,%u] to respawn grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), c->GetCurrentCell().GridX(), c->GetCurrentCell().GridY(), c->GetCurrentCell().CellX(), c->GetCurrentCell().CellY(), resp_cell.GridX(), resp_cell.GridY(), resp_cell.CellX(), resp_cell.CellY());
    #endif

    // teleport it to respawn point (like normal respawn if player see)
    if(CreatureCellRelocation(c,resp_cell))
    {
        c->Relocate(resp_x, resp_y, resp_z, c->GetOrientation());
        c->GetMotionMaster()->Initialize();                 // prevent possible problems with default move generators
        CreatureRelocationNotify(c,resp_cell,resp_cell.cellPair());
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
        if( ObjectAccessor::Instance().PlayersNearGrid(x, y, i_id, i_InstanceId) )
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

    // delete grid map, but don't delete if it is from parent map (and thus only reference)
    if (GridMaps[gx][gy])
    {
        if (i_InstanceId == 0)
            delete (GridMaps[gx][gy]);
        else
            ((MapInstanced*)(MapManager::Instance().GetBaseMap(i_id)))->RemoveGridMapReference(GridPair(gx, gy));
        GridMaps[gx][gy] = NULL;
    }
                                                            // x and y are swaped
    VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(GetId(), gy, gx);

    DEBUG_LOG("Unloading grid[%u,%u] for map %u finished", x,y, i_id);
    return true;
}

void Map::UnloadAll()
{
    // clear all delayed moves, useless anyway do this moves before map unload.
    i_creaturesToMove.clear();

    for(unsigned int i=0; i < MAX_NUMBER_OF_GRIDS; ++i)
    {
        if(!i_gridMask[i])
            continue;

        for(unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
            if( i_gridMask[i] & (uint64(1) << j) )
                UnloadGrid(i, j);
    }
}

float Map::GetVMapHeight(float x, float y, float z) const
{
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();

    float height=VMAP_INVALID_HEIGHT;
    if(vmgr->isHeightCalcEnabled())
    {
        height = vmgr->getHeight(GetId(), x, y, z + 2);     // look from a bit higher pos to find the floor
        if(height > VMAP_INVALID_HEIGHT)
        {
            // we have a vmap height for that place, but now we have to test
            // if the normal map height has higher priority here
            // If normal map height has priority, we return VMAP_INVALID_HEIGHT
            int gx,gy;
            GridPair p = MaNGOS::ComputeGridPair(x, y);
            gx=(int)(32-x/SIZE_OF_GRIDS) ;                  //grid x
            gy=(int)(32-y/SIZE_OF_GRIDS);                   //grid y
            if(GridMaps[gx][gy])
            {
                float lx,ly;
                lx=MAP_RESOLUTION*(32 -x/SIZE_OF_GRIDS - gx);
                ly=MAP_RESOLUTION*(32 -y/SIZE_OF_GRIDS - gy);
                float normheight = GridMaps[gx][gy]->Z[(int)(lx)][(int)(ly)];
                if((z>=(normheight-0.2f)) && fabs(normheight-z) < fabs(height-z))
                    height=VMAP_INVALID_HEIGHT;             // normal map height has pri
            }
        }
    }
    return height;

}

float Map::GetHeight(float x, float y, float z, bool pUseVmaps) const
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

    // ensure GridMap is loaded
    const_cast<Map*>(this)->EnsureGridCreated(GridPair(63-gx,63-gy));

    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    if(vmgr->isHeightCalcEnabled())
    {
        float height = 0;
        bool mapHeightFound = false;
        if(GridMaps[gx][gy])
        {
            height = GridMaps[gx][gy]->Z[(int)(lx)][(int)(ly)];
            mapHeightFound = true;
        }
        if(pUseVmaps)
        {
                                                            // look from a bit higher pos to find the floor
            float vmapheight = vmgr->getHeight(GetId(), x, y, z + 2);
            // if the land map did not find the height or if we are already under the surface and vmap found a height
            // or if the distance of the vmap height is less the land height distance
            if(!mapHeightFound || (z<height && vmapheight > VMAP_INVALID_HEIGHT) || fabs(height-z) > fabs(vmapheight-z))
            {
                height = vmapheight;
                mapHeightFound = true;
            }
        }
        else if(!mapHeightFound)
            height = z;

        return height;
    }
    else
    {
        if(GridMaps[gx][gy])
            return GridMaps[gx][gy]->Z[(int)(lx)][(int)(ly)];
        else
            return z;
    }
}

uint16 Map::GetAreaFlag(float x, float y ) const
{
    //local x,y coords
    float lx,ly;
    int gx,gy;
    GridPair p = MaNGOS::ComputeGridPair(x, y);

    // half opt method
    gx=(int)(32-x/SIZE_OF_GRIDS) ;                          //grid x
    gy=(int)(32-y/SIZE_OF_GRIDS);                           //grid y

    lx=16*(32 -x/SIZE_OF_GRIDS - gx);
    ly=16*(32 -y/SIZE_OF_GRIDS - gy);
    //DEBUG_LOG("my %d %d si %d %d",gx,gy,p.x_coord,p.y_coord);

    // ensure GridMap is loaded
    const_cast<Map*>(this)->EnsureGridCreated(GridPair(63-gx,63-gy));

    if(GridMaps[gx][gy])
        return GridMaps[gx][gy]->area_flag[(int)(lx)][(int)(ly)];
    // this used while not all *.map files generated (instances)
    else
        return GetAreaFlagByMapId(i_id);
}

uint8 Map::GetTerrainType(float x, float y ) const
{
    //local x,y coords
    float lx,ly;
    int gx,gy;

    // half opt method
    gx=(int)(32-x/SIZE_OF_GRIDS) ;                          //grid x
    gy=(int)(32-y/SIZE_OF_GRIDS);                           //grid y

    lx=16*(32 -x/SIZE_OF_GRIDS - gx);
    ly=16*(32 -y/SIZE_OF_GRIDS - gy);

    // ensure GridMap is loaded
    const_cast<Map*>(this)->EnsureGridCreated(GridPair(63-gx,63-gy));

    if(GridMaps[gx][gy])
        return GridMaps[gx][gy]->terrain_type[(int)(lx)][(int)(ly)];
    else
        return 0;

}

float Map::GetWaterLevel(float x, float y ) const
{
    //local x,y coords
    float lx,ly;
    int gx,gy;

    // half opt method
    gx=(int)(32-x/SIZE_OF_GRIDS) ;                          //grid x
    gy=(int)(32-y/SIZE_OF_GRIDS);                           //grid y

    lx=128*(32 -x/SIZE_OF_GRIDS - gx);
    ly=128*(32 -y/SIZE_OF_GRIDS - gy);

    // ensure GridMap is loaded
    const_cast<Map*>(this)->EnsureGridCreated(GridPair(63-gx,63-gy));

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

bool Map::IsInWater(float x, float y, float pZ) const
{
    // This method is called too often to use vamps for that (4. parameter = false).
    // The z pos is taken anyway for future use
    float z = GetHeight(x,y,pZ,false);
    float water_z = GetWaterLevel(x,y);
    uint8 flag = GetTerrainType(x,y);
    return (z < (water_z-2)) && (flag & 0x01);
}

bool Map::IsUnderWater(float x, float y, float z) const
{
    float water_z = GetWaterLevel(x,y);
    uint8 flag = GetTerrainType(x,y);
    return (z < (water_z-2)) && (flag & 0x01);
}

bool Map::CheckGridIntegrity(Creature* c, bool moved) const
{
    Cell const& cur_cell = c->GetCurrentCell();

    CellPair xy_val = MaNGOS::ComputeCellPair(c->GetPositionX(), c->GetPositionY());
    Cell xy_cell(xy_val);
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

const char* Map::GetMapName() const
{
    return i_mapEntry ? i_mapEntry->name[sWorld.GetDBClang()] : "UNNAMEDMAP\x0";
}

void Map::UpdateObjectVisibility( WorldObject* obj, Cell cell, CellPair cellpair)
{
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();
    MaNGOS::VisibleChangesNotifier notifier(*obj);
    TypeContainerVisitor<MaNGOS::VisibleChangesNotifier, WorldTypeMapContainer > player_notifier(notifier);
    CellLock<GridReadGuard> cell_lock(cell, cellpair);
    cell_lock->Visit(cell_lock, player_notifier, *this);
}

void Map::UpdatePlayerVisibility( Player* player, Cell cell, CellPair cellpair )
{
    cell.data.Part.reserved = ALL_DISTRICT;

    MaNGOS::PlayerNotifier pl_notifier(*player);
    TypeContainerVisitor<MaNGOS::PlayerNotifier, WorldTypeMapContainer > player_notifier(pl_notifier);

    CellLock<ReadGuard> cell_lock(cell, cellpair);
    cell_lock->Visit(cell_lock, player_notifier, *this);
}

void Map::UpdateObjectsVisibilityFor( Player* player, Cell cell, CellPair cellpair )
{
    MaNGOS::VisibleNotifier notifier(*player);

    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();
    TypeContainerVisitor<MaNGOS::VisibleNotifier, WorldTypeMapContainer > world_notifier(notifier);
    TypeContainerVisitor<MaNGOS::VisibleNotifier, GridTypeMapContainer  > grid_notifier(notifier);
    CellLock<GridReadGuard> cell_lock(cell, cellpair);
    cell_lock->Visit(cell_lock, world_notifier, *this);
    cell_lock->Visit(cell_lock, grid_notifier,  *this);

    // send data
    notifier.Notify();
}

void Map::PlayerRelocationNotify( Player* player, Cell cell, CellPair cellpair )
{
    CellLock<ReadGuard> cell_lock(cell, cellpair);
    MaNGOS::PlayerRelocationNotifier relocationNotifier(*player);
    cell.data.Part.reserved = ALL_DISTRICT;

    TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, GridTypeMapContainer >  p2grid_relocation(relocationNotifier);
    TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, WorldTypeMapContainer > p2world_relocation(relocationNotifier);

    cell_lock->Visit(cell_lock, p2grid_relocation, *this);
    cell_lock->Visit(cell_lock, p2world_relocation, *this);
}

void Map::CreatureRelocationNotify(Creature *creature, Cell cell, CellPair cellpair)
{
    CellLock<ReadGuard> cell_lock(cell, cellpair);
    MaNGOS::CreatureRelocationNotifier relocationNotifier(*creature);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();                                     // not trigger load unloaded grids at notifier call

    TypeContainerVisitor<MaNGOS::CreatureRelocationNotifier, WorldTypeMapContainer > c2world_relocation(relocationNotifier);
    TypeContainerVisitor<MaNGOS::CreatureRelocationNotifier, GridTypeMapContainer >  c2grid_relocation(relocationNotifier);

    cell_lock->Visit(cell_lock, c2world_relocation, *this);
    cell_lock->Visit(cell_lock, c2grid_relocation, *this);
}

void Map::SendInitSelf( Player * player )
{
    sLog.outDetail("Creating player data for himself %u", player->GetGUIDLow());

    UpdateData data;

    bool hasTransport = false;

    // attach to player data current transport data
    if(Transport* transport = player->GetTransport())
    {
        hasTransport = true;
        transport->BuildCreateUpdateBlockForPlayer(&data, player);
    }

    // build data for self presence in world at own client (one time for map)
    player->BuildCreateUpdateBlockForPlayer(&data, player);

    // build other passengers at transport also (they always visible and marked as visible and will not send at visibility update at add to map
    if(Transport* transport = player->GetTransport())
    {
        for(Transport::PlayerSet::const_iterator itr = transport->GetPassengers().begin();itr!=transport->GetPassengers().end();++itr)
        {
            if(player!=(*itr) && player->HaveAtClient(*itr))
            {
                hasTransport = true;
                (*itr)->BuildCreateUpdateBlockForPlayer(&data, player);
            }
        }
    }

    WorldPacket packet;
    data.BuildPacket(&packet, hasTransport);
    player->GetSession()->SendPacket(&packet);
}

void Map::SendInitTransports( Player * player )
{
    // Hack to send out transports
    MapManager::TransportMap& tmap = MapManager::Instance().m_TransportsByMap;

    // no transports at map
    if (tmap.find(player->GetMapId()) == tmap.end())
        return;

    UpdateData transData;

    MapManager::TransportSet& tset = tmap[player->GetMapId()];

    bool hasTransport = false;

    for (MapManager::TransportSet::iterator i = tset.begin(); i != tset.end(); ++i)
    {
        if((*i) != player->GetTransport())                  // send data for current transport in other place
        {
            hasTransport = true;
            (*i)->BuildCreateUpdateBlockForPlayer(&transData, player);
        }
    }

    WorldPacket packet;
    transData.BuildPacket(&packet, hasTransport);
    player->GetSession()->SendPacket(&packet);
}

void Map::SendRemoveTransports( Player * player )
{
    // Hack to send out transports
    MapManager::TransportMap& tmap = MapManager::Instance().m_TransportsByMap;

    // no transports at map
    if (tmap.find(player->GetMapId()) == tmap.end())
        return;

    UpdateData transData;

    MapManager::TransportSet& tset = tmap[player->GetMapId()];

    // except used transport
    for (MapManager::TransportSet::iterator i = tset.begin(); i != tset.end(); ++i)
        if(player->GetTransport() != (*i))
            (*i)->BuildOutOfRangeUpdateBlock(&transData);

    WorldPacket packet;
    transData.BuildPacket(&packet);
    player->GetSession()->SendPacket(&packet);
}

template void Map::Add(Corpse *);
template void Map::Add(Creature *);
template void Map::Add(GameObject *);
template void Map::Add(DynamicObject *);

template void Map::Remove(Corpse *,bool);
template void Map::Remove(Creature *,bool);
template void Map::Remove(GameObject *, bool);
template void Map::Remove(DynamicObject *, bool);
