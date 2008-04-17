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

#include "RedZoneDistrict.h"

void RedZone::Initialize()
{
    for(unsigned int x=0; x < MAX_NUMBER_OF_GRIDS; ++x)
    {
        for(unsigned y=0; y < MAX_NUMBER_OF_GRIDS; ++y)
        {
            for(unsigned xoffset=0; xoffset < MAX_NUMBER_OF_CELLS; ++xoffset)
            {
                for(unsigned yoffset=0; yoffset < MAX_NUMBER_OF_CELLS; ++yoffset)
                {
                    uint32 cell_x = (x*MAX_NUMBER_OF_CELLS) + xoffset;
                    uint32 cell_y = (y*MAX_NUMBER_OF_CELLS) + yoffset;
                    si_RedZones[cell_x][cell_y].initialize(x, y, xoffset, yoffset);
                }
            }
        }
    }
}

RedZone RedZone::si_RedZones[MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_CELLS][MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_CELLS];
