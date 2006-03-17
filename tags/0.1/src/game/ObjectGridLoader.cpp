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

#include "ObjectGridLoader.h"
#include "Database/DatabaseEnv.h"
#include "ObjectAccessor.h"
#include "Utilities.h"


template<>
inline void
ObjectAccessor::RemoveUpdateObjects(std::map<OBJECT_HANDLE, Corpse *> &m)
{
    if( m.size() == 0 )
	return;

    Guard guard(i_updateGuard);
    for(std::map<OBJECT_HANDLE, Corpse *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	std::set<Object *>::iterator obj = i_objects.find(iter->second);
	if( obj != i_objects.end() )
	    i_objects.erase( obj );
	RemoveCorpse(iter->second->GetGUID());
    }
}

template<class T> void SetState(T *obj)
{
    obj->AddToWorld();
}

template<> void SetState(Creature *obj)
{
    if( MaNGOS::Utilities::IsSpiritHealer(obj) )
	obj->setDeathState(DEAD);
}


template<class T> void LoadHelper(const char* table, const uint32 &grid_id, const uint32 map_id, const CellPair &cell, std::map<OBJECT_HANDLE, T*> &m, uint32 &count)
{
    uint32 cell_id = (cell.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell.x_coord;
    std::stringstream query;
    query << "SELECT guid from " << table << " WHERE grid_id=" << grid_id << " and mapId=" << map_id << " and cell_id=" << cell_id;
    std::auto_ptr<QueryResult> result(sDatabase.Query(query.str().c_str()));
    if( result.get() != NULL )
    {
	do
	{
	    Field *fields = result->Fetch();
	    T *obj = new T;
	    uint32 guid = fields[0].GetUInt32();
	    obj->LoadFromDB(guid);
	    m[obj->GetGUID()] = obj;
	    
	    
	    SetState(obj);
	    obj->AddToWorld();
	    ++count; 
	    
	}while( result->NextRow() );
    }
}

void
ObjectGridLoader::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
{
    uint32 x = (i_cell.GridX()*MAX_NUMBER_OF_CELLS) + i_cell.CellX();
    uint32 y = (i_cell.GridY()*MAX_NUMBER_OF_CELLS) + i_cell.CellY();
    CellPair cell_pair(x, y);
    LoadHelper<GameObject>("gameobjects_grid", i_grid.GetGridId(), i_mapId, cell_pair, m, i_gameObjects);    
}

void
ObjectGridLoader::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    uint32 x = (i_cell.GridX()*MAX_NUMBER_OF_CELLS) + i_cell.CellX();
    uint32 y = (i_cell.GridY()*MAX_NUMBER_OF_CELLS) + i_cell.CellY();
    CellPair cell_pair(x,y);
    LoadHelper<Creature>("creatures_grid", i_grid.GetGridId(), i_mapId, cell_pair, m, i_creatures);
}


void
ObjectGridLoader::Load(GridType &grid)
{
    TypeContainerVisitor<ObjectGridLoader, TypeMapContainer<AllObjectTypes> > loader(*this);
    grid.VisitGridObjects(loader);
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
    ObjectAccessor::Instance().RemoveUpdateObjects(m);
    for(typename std::map<OBJECT_HANDLE, T* >::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	delete iter->second;
    }

    m.clear();
}


template void ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, GameObject *> &m);
template void ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m);
template void ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, Corpse *> &m);
template void ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, Creature *> &m);
