/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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

#ifndef MANGOS_REDZONEDISTRICT_H
#define MANGOS_REDZONEDISTRICT_H

#include "GridDefines.h"
#include "Cell.h"

struct MANGOS_DLL_DECL RedZone
{
    void initialize(const uint32 &grid_x, const uint32 &grid_y, const uint32 &cell_x, const uint32 &cell_y)
    {
        i_cell.data.Part.grid_x = (unsigned)(grid_x);
        i_cell.data.Part.grid_y = (unsigned)(grid_y);
        i_cell.data.Part.cell_x = (unsigned)(cell_x);
        i_cell.data.Part.cell_y = (unsigned)(cell_y);
        i_cell.data.Part.reserved = 0;
    }

    operator const Cell &(void) const { return i_cell; }
    operator Cell &(void) { return i_cell; }
    Cell i_cell;

    static const Cell& GetZone(const CellPair &p) { return si_RedZones[p.x_coord][p.y_coord]; }
    static RedZone si_RedZones[MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_CELLS][MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_CELLS];
    static void Initialize(void);
};
#endif
