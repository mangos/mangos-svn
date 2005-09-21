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

// common method
template<class T> void loadHelper(GridType &grid, Player &pl, std::map<OBJECT_HANDLE, T *> &m)
{
    const GridCoord &coord1(grid.GetLowerLeftCoord());
    const GridCoord &coord2(grid.GetUpperRightCoord());

    std::stringstream query;
    query << "SELECT id from creatures WHERE positionX <=" << coord1.x << " and positionX <=" << coord2.x << " and positionY <=" << coord1.y << " and positionY <=" << coord2.y;

    std::auto_ptr<QueryResult> result(sDatabase.Query(ss.str().c_str()));

    if( result.get() != NULL )
    {
	do
	{
	    Fields *fields = result->Fetch();
	    Creature *obj = new T;
	    unit32 guid = fields[0].GetUInt32();
	    obj->LoadFromDB(guid);
	    m[guid] = obj;
	    
	}while( result->NextRow() );
    }
}

void
ObjectGridLoader::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
{
    LoadHelper<GameObject>(i_grid, i_player, m);
}

void
ObjectGridLoader::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    LoadHelper<Creature>(i_grid, i_player, m);
}


void
ObjectGridUnLoader::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
{
    for(std::map<OBJECT_HANDLE, GameObject *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	delete iter->second;
    m.clear();
}

void
ObjectGridUnLoader::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	delete iter->second;
    m.clear();
}
