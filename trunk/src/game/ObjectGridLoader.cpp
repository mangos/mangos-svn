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

#include "ObjectGridLoader.h"
#include "Database/DatabaseEnv.h"
#include "ObjectAccessor.h"
#include "Utilities.h"
#include "MapManager.h"
#include "RedZoneDistrict.h"

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
    TypeContainerVisitor<ObjectGridRespawnMover, TypeMapContainer<AllObjectTypes> > mover(*this);
    grid.VisitGridObjects(mover);
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
    if( MaNGOS::Utilities::IsSpiritHealer(obj) )
        obj->setDeathState(DEAD);
}

template<class T> void LoadHelper(const char* table, const uint32 &grid_id, const uint32 map_id, const CellPair &cell, std::map<OBJECT_HANDLE, T*> &m, uint32 &count)
{
    uint32 cell_id = (cell.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell.x_coord;

    QueryResult *result = sDatabase.PQuery("SELECT `guid` FROM `%s` WHERE `grid` = '%u' AND `cell` = '%u' AND `map` = '%u'", table, grid_id, cell_id, map_id);

    if( result )

    {
        do
        {
            Field *fields = result->Fetch();
            T *obj = new T;
            uint32 guid = fields[0].GetUInt32();
            if(!obj->LoadFromDB(guid))
            {
                delete obj;
                continue;
            }

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
    CellPair cell_pair(x, y);
    LoadHelper<GameObject>("gameobject_grid", i_grid.GetGridId(), i_mapId, cell_pair, m, i_gameObjects);
}

void
ObjectGridLoader::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    uint32 x = (i_cell.GridX()*MAX_NUMBER_OF_CELLS) + i_cell.CellX();
    uint32 y = (i_cell.GridY()*MAX_NUMBER_OF_CELLS) + i_cell.CellY();
    CellPair cell_pair(x,y);
    LoadHelper<Creature>("creature_grid", i_grid.GetGridId(), i_mapId, cell_pair, m, i_creatures);
}

void
ObjectGridLoader::Visit(std::map<OBJECT_HANDLE, Corpse *> &m)
{
    uint32 x = (i_cell.GridX()*MAX_NUMBER_OF_CELLS) + i_cell.CellX();
    uint32 y = (i_cell.GridY()*MAX_NUMBER_OF_CELLS) + i_cell.CellY();
    CellPair cell_pair(x,y);
    LoadHelper<Corpse>("corpse_grid", i_grid.GetGridId(), i_mapId, cell_pair, m, i_corpses);
}

void
ObjectGridLoader::Load(GridType &grid)
{
    TypeContainerVisitor<ObjectGridLoader, TypeMapContainer<AllObjectTypes> > loader(*this);
    grid.VisitGridObjects(loader);
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
            GridLoader<Player, AllObjectTypes> loader;
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
    TypeContainerVisitor<ObjectGridUnloader, TypeMapContainer<AllObjectTypes> > unloader(*this);
    grid.VisitGridObjects(unloader);
}

template<class T>
void
ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, T *> &m)
{
    if( m.size() == 0 )
        return;

    for(typename std::map<OBJECT_HANDLE, T* >::iterator iter=m.begin(); iter != m.end(); ++iter)
        delete iter->second;

    m.clear();
}

template void ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, GameObject *> &m);
template void ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m);
template void ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, Corpse *> &m);
template void ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, Creature *> &m);
