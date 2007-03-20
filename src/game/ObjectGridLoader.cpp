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

#include "ObjectGridLoader.h"
#include "Database/DatabaseEnv.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "RedZoneDistrict.h"
#include "Creature.h"
#include "GameObject.h"
#include "DynamicObject.h"
#include "Corpse.h"

class MANGOS_DLL_DECL ObjectGridRespawnMover
{
    public:
        ObjectGridRespawnMover() {}

        void Move(GridType &grid);

        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m) {}
        void Visit(std::map<OBJECT_HANDLE, Creature *> &m);
};

void
ObjectGridRespawnMover::Move(GridType &grid)
{
    TypeContainerVisitor<ObjectGridRespawnMover, GridTypeMapContainer > mover(*this);
    grid.Visit(mover);
}

void
ObjectGridRespawnMover::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    if( m.size() == 0 )
        return;

    // creature in unloading grid can have respawn point in another grid
    // if it will be unloaded then it will not respawn in original grid until unload/load original grid
    // move to respwn point to prevent this case. For player view in respawn grid this wll be normal respawn.
    for(std::map<OBJECT_HANDLE, Creature* >::iterator iter=m.begin(), next; iter != m.end(); iter = next)
    {
        next = iter; ++next;

        Creature * c = iter->second;

        assert(!c->isPet() && "ObjectGridRespawnMover don't must be called for pets");

        Cell const& cur_cell  = c->GetCurrentCell();

        float resp_x, resp_y, resp_z;
        c->GetRespawnCoord(resp_x, resp_y, resp_z);
        CellPair resp_val = MaNGOS::ComputeCellPair(resp_x, resp_y);
        Cell resp_cell = RedZone::GetZone(resp_val);

        if(cur_cell.DiffGrid(resp_cell))
        {
            MapManager::Instance().GetMap(c->GetMapId())->CreatureRespawnRelocation(c);
            // false result ignored: will be unload with other creatures at grid
        }
    }
}

template<class T> void addUnitState(T *obj, CellPair const& cell_pair)
{
}

template<> void addUnitState(Creature *obj, CellPair const& cell_pair)
{
    Cell cell = RedZone::GetZone(cell_pair);

    obj->SetCurrentCell(cell);
    if( obj->isSpiritHealer())
        obj->setDeathState(DEAD);
}

template <class T>
void LoadHelper(QueryResult *result, CellPair &cell, std::map<OBJECT_HANDLE, T*> &m, uint32 &count)
{
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            T *obj = new T;
            uint32 guid = fields[result->GetFieldCount()-1].GetUInt32();
            //sLog.outString("DEBUG: LoadHelper from table: %s for (guid: %u) Loading",table,guid);
            if(!obj->LoadFromDB(guid, result))
            {
                delete obj;
                continue;
            }
            else
            {
                // Check loaded cell/grid integrity
                Cell old_cell = RedZone::GetZone(cell);

                CellPair pos_val = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
                Cell pos_cell = RedZone::GetZone(pos_val);

                if(old_cell != pos_cell)
                {
                    sLog.outError("Object (GUID: %u TypeId: %u Entry: %u) loaded (X: %f Y: %f) to grid[%u,%u]cell[%u,%u] instead grid[%u,%u]cell[%u,%u].", obj->GetGUIDLow(), obj->GetGUIDHigh(), obj->GetEntry(), obj->GetPositionX(), obj->GetPositionY(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), pos_cell.GridX(), pos_cell.GridY(), pos_cell.CellX(), pos_cell.CellY());
                    delete obj;
                    continue;
                }
            }

            m[obj->GetGUID()] = obj;

            addUnitState(obj,cell);
            obj->AddToWorld();
            ++count;

        }while( result->NextRow() );
        delete result;
    }
}

void
ObjectGridLoader::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
{
    uint32 x = (i_cell.GridX()*MAX_NUMBER_OF_CELLS) + i_cell.CellX();
    uint32 y = (i_cell.GridY()*MAX_NUMBER_OF_CELLS) + i_cell.CellY();
    CellPair cell_pair(x,y);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;
    QueryResult *result = sDatabase.PQuery(
    //      0    1                  2                         3                         4            5             6           7           8           9           10     11              12             13         14            15
        "SELECT `id`,`gameobject`.`map`,`gameobject`.`position_x`,`gameobject`.`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`loot`,`spawntimesecs`,`animprogress`,`dynflags`,`respawntime`,`gameobject`.`guid` "
        "FROM `gameobject` LEFT JOIN `gameobject_grid` ON `gameobject`.`guid` = `gameobject_grid`.`guid` "
        "LEFT JOIN `gameobject_respawn` ON `gameobject`.`guid`=`gameobject_respawn`.`guid` "
        "WHERE `grid` = '%u' AND `cell` = '%u' AND `gameobject_grid`.`map` = '%u'", i_grid.GetGridId(), cell_id, i_mapId);
    LoadHelper(result, cell_pair, m, i_gameObjects);
}

void
ObjectGridLoader::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    uint32 x = (i_cell.GridX()*MAX_NUMBER_OF_CELLS) + i_cell.CellX();
    uint32 y = (i_cell.GridY()*MAX_NUMBER_OF_CELLS) + i_cell.CellY();
    CellPair cell_pair(x,y);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;
    QueryResult *result = sDatabase.PQuery(
    //          0    1                2                       3                       4            5             6               7           8                  9                  10                 11          12        13            14      15             16      17
        "SELECT `id`,`creature`.`map`,`creature`.`position_x`,`creature`.`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawndist`,`spawn_position_x`,`spawn_position_y`,`spawn_position_z`,`curhealth`,`curmana`,`respawntime`,`state`,`MovementType`,`auras`,`creature`.`guid`"
        "FROM `creature` LEFT JOIN `creature_grid` ON `creature`.`guid` = `creature_grid`.`guid` "
        "LEFT JOIN `creature_respawn` ON `creature`.`guid`=`creature_respawn`.`guid`"
        "WHERE `grid` = '%u' AND `cell` = '%u' AND `creature_grid`.`map` = '%u'", i_grid.GetGridId(), cell_id, i_mapId);
    LoadHelper(result, cell_pair, m, i_creatures);
}

void
ObjectGridLoader::Visit(std::map<OBJECT_HANDLE, Corpse *> &m)
{
    uint32 x = (i_cell.GridX()*MAX_NUMBER_OF_CELLS) + i_cell.CellX();
    uint32 y = (i_cell.GridY()*MAX_NUMBER_OF_CELLS) + i_cell.CellY();
    CellPair cell_pair(x,y);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    // Load bones to grid store
    QueryResult *result = sDatabase.PQuery(
        "SELECT `corpse`.`position_x`,`corpse`.`position_y`,`position_z`,`orientation`,`corpse`.`map`,`data`,`bones_flag`,`corpse`.`guid` "
        "FROM `corpse` LEFT JOIN `corpse_grid` ON `corpse`.`guid` = `corpse_grid`.`guid` "
        "WHERE `grid` = '%u' AND `cell` = '%u' AND `corpse_grid`.`map` = '%u' AND `bones_flag` = 1", i_grid.GetGridId(), cell_id, i_mapId);
    LoadHelper(result, cell_pair, m, i_corpses);
}

void
ObjectGridLoader::Load(GridType &grid)
{
    TypeContainerVisitor<ObjectGridLoader, GridTypeMapContainer > loader(*this);
    grid.Visit(loader);
}

void ObjectGridLoader::LoadN(void)
{
    i_gameObjects = 0; i_creatures = 0; i_corpses = 0;
    i_cell.data.Part.cell_y = 0;
    for(unsigned int x=0; x < MAX_NUMBER_OF_CELLS; ++x)
    {
        i_cell.data.Part.cell_x = x;
        for(unsigned int y=0; y < MAX_NUMBER_OF_CELLS; ++y)
        {
            i_cell.data.Part.cell_y = y;
            GridLoader<Player, AllWorldObjectTypes, AllGridObjectTypes> loader;
            loader.Load(i_grid(x, y), *this);
        }
    }
    sLog.outDebug("%u GameObjects, %u Creatures, and %u Corpses/Bones loaded for grid %u on map %u", i_gameObjects, i_creatures, i_corpses,i_grid.GetGridId(), i_mapId);
}

void ObjectGridUnloader::MoveToRespawnN()
{
    for(unsigned int x=0; x < MAX_NUMBER_OF_CELLS; ++x)
    {
        for(unsigned int y=0; y < MAX_NUMBER_OF_CELLS; ++y)
        {
            ObjectGridRespawnMover mover;
            mover.Move(i_grid(x, y));
        }
    }
}

void
ObjectGridUnloader::Unload(GridType &grid)
{
    TypeContainerVisitor<ObjectGridUnloader, GridTypeMapContainer > unloader(*this);
    grid.Visit(unloader);
}

template<class T>
void
ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, T *> &m)
{
    if( m.size() == 0 )
        return;

    for(typename std::map<OBJECT_HANDLE, T* >::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        // if option set then object already saved at this moment
        if(!sWorld.getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATLY))
            iter->second->SaveRespawnTime();
        delete iter->second;
    }

    m.clear();
}

template<>
void
ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, Creature*> &m)
{
    if( m.size() == 0 )
        return;

    // remove all cross-reference before deleting
    for(std::map<OBJECT_HANDLE, Creature* >::iterator iter=m.begin(); iter != m.end(); ++iter)
        iter->second->CleanupCrossRefsBeforeDelete();

    for(std::map<OBJECT_HANDLE, Creature* >::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        // if option set then object already saved at this moment
        if(!sWorld.getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATLY))
            iter->second->SaveRespawnTime();
        delete iter->second;
    }

    m.clear();
}

void
ObjectGridStoper::Stop(GridType &grid)
{
    TypeContainerVisitor<ObjectGridStoper, GridTypeMapContainer > stoper(*this);
    grid.Visit(stoper);
}

void
ObjectGridStoper::Visit(std::map<OBJECT_HANDLE, Creature*> &m)
{
    if( m.size() == 0 )
        return;

    // stop any fights at grid de-activation
    for(std::map<OBJECT_HANDLE, Creature* >::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        iter->second->CombatStop(true);
        iter->second->DeleteThreatList();
    }
}

template void ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, GameObject *> &m);
template void ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m);
template void ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, Corpse *> &m);
