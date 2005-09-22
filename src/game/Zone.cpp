/* ZoneDefine.h
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


#include "Zone.h"

void
Zone::AddPlayer(Player *player)
{
    if( i_grid == NULL )
    {
	// double check-Lock pattern
	Guard guard(*this);
	if( i_grid == NULL )
	{
	    GridType *grid = new GridType(i_coord1.x, i_coord1.y, i_coord2.x, i_coord2.y);
	    GridLoaderType loader;
	    loader.Load(*grid);
	    grid->AddObject(player);
	    i_grid = grid;
	}
    }
}

void
Zone::RemovePlayer(Player *player)
{
    if( i_grid != NULL )
    {
	i_grid->RemoveObject(player);
	
	// remove all objects in this grid from the player
	// in range....
    }
}
