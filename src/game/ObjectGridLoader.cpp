/* ObjectGridLoader.h
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

#include "ObjectGridLoader.h"
#include "Database/DatabaseEnv.h"

// common method
template<class T> void LoadHelper(const char* table, GridType &grid, uint32 map_id, std::map<OBJECT_HANDLE, T*> &m)
{
    std::stringstream query;
    query << "SELECT guid from " << table << " WHERE grid_id=" << grid.GetGridId() << " and mapId=" << map_id;
    
    std::auto_ptr<QueryResult> result(sDatabase.Query(query.str().c_str()));
    unsigned int count = 0;
    if( result.get() != NULL )
    {
	do
	{
	    Field *fields = result->Fetch();
	    T *obj = new T;
	    uint32 guid = fields[0].GetUInt32();
	    obj->LoadFromDB(guid);
	    m[guid] = obj;
	    obj->AddToWorld();
	    ++count;

	}while( result->NextRow() );
    }

    sLog.outDebug("%d objects loaded for grid %d from table %s", count, grid.GetGridId(), table);
};

void
ObjectGridLoader::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
{
    LoadHelper<GameObject>("gameobjects_grid", i_grid, i_mapId, m);
}

void
ObjectGridLoader::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    LoadHelper<Creature>("creatures_grid", i_grid,i_mapId, m);
}


void
ObjectGridLoader::Load(GridType &grid)
{
    TypeContainerVisitor<ObjectGridLoader, TypeMapContainer<AllObjectTypes> > loader(*this);
    grid.VisitGridObjects(loader);
}

//==============================================//
//      ObjectGridUnloader
void 
ObjectGridUnloader::Unload(GridType &grid)
{
    TypeContainerVisitor<ObjectGridUnloader, TypeMapContainer<AllObjectTypes> > unloader(*this);
    grid.VisitGridObjects(unloader);
}

void
ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
{
    for(std::map<OBJECT_HANDLE, GameObject *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	delete iter->second;
    m.clear();
}

void
ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m)
{
    for(std::map<OBJECT_HANDLE, DynamicObject *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	delete iter->second;
    m.clear();
}

void
ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, Corpse *> &m)
{
    for(std::map<OBJECT_HANDLE, Corpse *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	delete iter->second;
    m.clear();
}

void
ObjectGridUnloader::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	delete iter->second;
    m.clear();
}
