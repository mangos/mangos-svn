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

#ifndef MANGOS_GRIDDEFINES_H
#define MANGOS_GRIDDEFINES_H

#include "GameSystem/NGrid.h"
#include "Player.h"
#include "GameObject.h"
#include "Creature.h"
#include "DynamicObject.h"
#include "Corpse.h"

#define MAX_NUMBER_OF_GRIDS      64

#define SIZE_OF_GRIDS            533.33333
#define CENTER_GRID_ID           (MAX_NUMBER_OF_GRIDS/2)

#define CENTER_GRID_OFFSET      (SIZE_OF_GRIDS/2)

#define MIN_GRID_DELAY          60*1000

#define MAX_NUMBER_OF_CELLS     4
#define SIZE_OF_GRID_CELL       (SIZE_OF_GRIDS/MAX_NUMBER_OF_CELLS)

#define CENTER_GRID_CELL_ID     128
#define CENTER_GRID_CELL_OFFSET (SIZE_OF_GRID_CELL/2)

#define TOTAL_NUMBER_OF_CELLS_PER_MAP    (MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_CELLS)

#define MAP_RESOLUTION 256

typedef TYPELIST_4(GameObject, Creature, DynamicObject, Corpse)    AllObjectTypes;
typedef Grid<Player, AllObjectTypes> GridType;
typedef std::map<OBJECT_HANDLE, Player* > PlayerMapType;
typedef std::map<OBJECT_HANDLE, Creature* > CreatureMapType;
typedef std::map<OBJECT_HANDLE, GameObject* > GameObjectMapType;
typedef std::map<OBJECT_HANDLE, DynamicObject* > DynamicObjectMapType;
typedef std::map<OBJECT_HANDLE, Corpse* > CorpseMapType;

typedef NGrid<4, Player, AllObjectTypes> NGridType;

template<const unsigned int LIMIT>
struct MANGOS_DLL_DECL CoordPair
{
    CoordPair(uint32 x=0, uint32 y=0) : x_coord(x), y_coord(y) {}
    CoordPair(const CoordPair<LIMIT> &obj) : x_coord(obj.x_coord), y_coord(obj.y_coord) {}
    bool operator==(const CoordPair<LIMIT> &obj) const { return (obj.x_coord == x_coord && obj.y_coord == y_coord); }
    bool operator!=(const CoordPair<LIMIT> &obj) const { return !operator==(obj); }
    CoordPair<LIMIT>& operator=(const CoordPair<LIMIT> &obj)
    {
        this->~CoordPair<LIMIT>();
        new (this) CoordPair<LIMIT>(obj);
        return *this;
    }

    void operator<<(const uint32 val)
    {
        if( x_coord >= val )
            x_coord -= val;
    }

    void operator>>(const uint32 val)
    {
        if( x_coord+val < LIMIT )
            x_coord += val;
    }

    void operator-=(const uint32 val)
    {
        if( y_coord >= val )
            y_coord -= val;
    }

    void operator+=(const uint32 val)
    {
        if( y_coord+val < LIMIT )
            y_coord += val;
    }

    uint32 x_coord;
    uint32 y_coord;
};

typedef CoordPair<MAX_NUMBER_OF_GRIDS> GridPair;
typedef CoordPair<TOTAL_NUMBER_OF_CELLS_PER_MAP> CellPair;

namespace MaNGOS
{
    template<class RET_TYPE, int CENTER_VAL>
        inline RET_TYPE Compute(const float &x, const float &y, const float center_offset, const float size)
    {
        float x_offset = (x - center_offset)/size;
        float y_offset = (y - center_offset)/size;

        int x_val = int(x_offset+CENTER_VAL + 0.5);
        int y_val = int(y_offset+CENTER_VAL + 0.5);
        return RET_TYPE(x_val, y_val);
    }

    inline GridPair ComputeGridPair(const float &x, const float &y)
    {
        return Compute<GridPair, CENTER_GRID_ID>(x, y, CENTER_GRID_OFFSET, SIZE_OF_GRIDS);
    }

    inline CellPair ComputeCellPair(const float &x, const float &y)
    {
        return Compute<CellPair, CENTER_GRID_CELL_ID>(x, y, CENTER_GRID_CELL_OFFSET, SIZE_OF_GRID_CELL);
    }
}
#endif
