/* CellImpl.h
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

#ifndef MANGOS_CELLIMPL_H
#define MANGOS_CELLIMPL_H

#include "Cell.h"
#include "Map.h"
#include "RedZoneDistrict.h"

template<class LOCK_TYPE, class T, class CONTAINER>
inline void 
Cell::VisitX(const CellLock<LOCK_TYPE> & l, CellPair &cell_pair, TypeContainerVisitor<T, CONTAINER> &visitor, Map &m, int &num, const int direction) const
{
    if( cell_pair.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP && num > 0 && cell_pair.x_coord >= 0 )
    {
	CellLock<LOCK_TYPE> lock(RedZone::GetZone(cell_pair), cell_pair);
	m.Visit(lock, visitor);
	cell_pair.x_coord += direction;
	--num;
	VisitX(l, cell_pair, visitor, m, num, direction);
    }
}

template<class LOCK_TYPE, class T, class CONTAINER>
inline void 
Cell::VisitY(const CellLock<LOCK_TYPE> &l, CellPair &cell_pair, TypeContainerVisitor<T, CONTAINER> &visitor, Map &m, int &num, const int direction) const
{
    if( cell_pair.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP  && num > 0 && cell_pair.y_coord >= 0 )
    {
	CellLock<LOCK_TYPE> lock(RedZone::GetZone(cell_pair), cell_pair);
	m.Visit(lock, visitor);
	cell_pair.y_coord += direction;
	--num;
	VisitY(l, cell_pair, visitor, m, num, direction);
    }
}


template<class LOCK_TYPE,class T, class CONTAINER> 
inline void 
Cell::Visit(const CellLock<LOCK_TYPE> &l, TypeContainerVisitor<T, CONTAINER> &visitor, Map &m) const
{
    CellPair cell = (const CellPair &)l;
    
    switch( (district_t)this->data.Part.reserved )
    {
    case ALL_DISTRICT:
	{
	    int num_x = 3;
	    int num_y = 3;

	    if( cell.x_coord > 0 ) 
		--cell.x_coord;
	    else
		--num_x;
	    
	    if( cell.y_coord > 0 )
		--cell.y_coord;
	    else
		--num_y;

	    for(unsigned int idx=0; idx < MAX_NUMBER_OF_CELLS; ++idx)
	    {
		int tmp_x = num_x;
		CellPair tmp_cell(cell);
		if( cell.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP )
		    VisitX(l, tmp_cell, visitor, m, tmp_x, 1);
		++cell.y_coord;
	    }
	    break;
	}
    case UPPER_LEFT_DISTRICT:
	{
	    int num_x = 3;
	    if( cell.x_coord > 0 ) 
	    {		
		--cell.x_coord;
		uint32 tmp = cell.y_coord;
		int num_y = 2;
		VisitY(l, cell, visitor, m, num_y, 1);
		cell.y_coord = tmp;
	    }
	    else
		--num_x;

	    if( cell.y_coord > 0 )
	    {
		--cell.y_coord;
		VisitX(l, cell, visitor, m, num_x, 1);
	    }
	    break;
	}
    case UPPER_RIGHT_DISTRICT:
	{
	    int num_x = 3;
	    if( cell.x_coord+1 < TOTAL_NUMBER_OF_CELLS_PER_MAP ) 
	    {
		++cell.x_coord;
		uint32 tmp = cell.y_coord;
		int num_y = 2;
		VisitY(l, cell, visitor, m, num_y, -1);
		cell.y_coord = tmp;
	    }
	    else
		--num_x;
	    
	    if( cell.y_coord >  0 )
	    {
		--cell.y_coord;
		VisitX(l, cell, visitor, m, num_x, -1);
	    }
	    break;
	}
    case LOWER_LEFT_DISTRICT:
	{
	    int num_x = 3;
	    if( cell.x_coord > 0 )
	    {
		--cell.x_coord;
		uint32 tmp = cell.y_coord;
		int num_y = 2;
		VisitY(l, cell, visitor, m, num_y, -1);
		cell.y_coord = tmp;
	    }
	    else
		--num_x;

	    if( ++cell.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP )
	    {
		VisitX(l, cell, visitor, m, num_x, 1);
	    }

	    break;
	}
    case LOWER_RIGHT_DISTRICT:
	{
	    int num_x = 3;
	    if( cell.x_coord+1 < TOTAL_NUMBER_OF_CELLS_PER_MAP )
	    {
		++cell.x_coord;
		uint32 tmp = cell.y_coord;
		int num_y = 2;
		VisitY(l, cell, visitor, m, num_y, -1);
		cell.y_coord = tmp;
	    }
	    else
		--num_x;

	    if( ++cell.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP )
	    {
		VisitX(l, cell, visitor, m, num_x, -1);
	    }
	    break;
	}
    case LEFT_DISTRICT:
	{
	    if( cell.x_coord > 0 )
	    {
		--cell.x_coord;
		int num_y = 3;
		if( cell.y_coord > 0 )
		{
		    --cell.y_coord;
		}
		else
		    --num_y;
		VisitY(l, cell, visitor, m, num_y, 1);
	    }
	    break;
	}
    case RIGHT_DISTRICT:
	{
	    if( ++cell.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP )
	    {
		int num_y = 3;
		if( cell.y_coord > 0 )
		    --cell.y_coord;
		else
		    --num_y;
		VisitY(l, cell, visitor, m, num_y, 1);
	    }
	    break;
	}
    case UPPER_DISTRICT:
	{
	    if( cell.y_coord > 0 )
	    {
		--cell.y_coord;
		int num_x = 3;
		if( cell.x_coord > 0 )
		    --cell.x_coord;
		else
		    --num_x;
		VisitX(l, cell, visitor, m, num_x, 1);
	    }
	    break;
	}
    case LOWER_DISTRICT:
	{
	    if( ++cell.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP )
	    {
		int num_x = 3;
		if( cell.x_coord > 0 )
		    --cell.x_coord;
		else
		    --num_x;
		VisitX(l, cell, visitor, m, num_x, 1);
	    }
	    break;
	}
    default:
	{
	    assert( false );
	    break;
	}
    }    
}

#endif
