/* RedZoneDistrict.h
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

#ifndef MANGOS_REDZONEDISTRICT_H
#define MANGOS_REDZONEDISTRICT_H

/** The RedZoneDistrict is a series of smaller cell that
 * alerts the map when the user enters one of its red zones.
 * Once alerted, this will triggers the loading of the
 * affected grids due to player are approaching extremely close.
 */

#include "Map.h"

typedef enum
    {
	DISTRICT_1 = 1,
	DISTRICT_2 = 1 << 1,
	DISTRICT_3 = 1 << 2,
	DISTRICT_4 = 1 << 3,
	DISTRICT_5 = 1 << 4,
	DISTRICT_6 = 1 << 5,
	DISTRICT_7 = 1 << 6,
	DISTRICT_8 = 1 << 7
    } district_t;


struct MANGOS_DLL_DECL RedZoneDistrict
{    
    Map &i_map;
    Player &i_player;
    RedZoneDistrict(Map &m, Player &player) : i_map(m), i_player(player) {}
    void Enter(GridPair &);

    static uint8 si_UpperLeftCorner;
    static uint8 si_UpperRightCorner;
    static uint8 si_LowerLeftCorner;
    static uint8 si_LowerRightCorner;
    static uint8 si_LeftCenter;
    static uint8 si_RightCenter;
    static uint8 si_UpperCenter;
    static uint8 si_LowerCenter;
};

// SIZE_OF_GRIDS / 4
#define CORNER_GRID_OFFSET 133.333333

struct MANGOS_DLL_DECL RedZone
{
    struct Coordinate
    {
	float x;
	float y;
    };

    void Initialize(const uint32 &x, const uint32 &y)
    {
	const float x_top = x * SIZE_OF_GRIDS;
	const float y_top = y * SIZE_OF_GRIDS;
	i_upperLeft.x = x_top + CORNER_GRID_OFFSET;
	i_upperLeft.y = y_top + CORNER_GRID_OFFSET;
	i_upperRight.x = x_top + (3.0*CORNER_GRID_OFFSET);
	i_upperRight.y = y_top + CORNER_GRID_OFFSET;
	i_lowerLeft.x = x_top + CORNER_GRID_OFFSET;
	i_lowerLeft.y = y_top + (3.0*CORNER_GRID_OFFSET);
	i_lowerRight.x = x_top + (3.0*CORNER_GRID_OFFSET);
	i_lowerRight.y = y_top + (3.0*CORNER_GRID_OFFSET);
    }

    uint8 Enter(const float &x, const float &y)
    {
	if( x < i_upperLeft.x )
	{
	    // left strip
	    if( y < i_upperLeft.y )
		return RedZoneDistrict::si_UpperLeftCorner;
	    else if( y < i_lowerLeft.y )
		return RedZoneDistrict::si_LeftCenter;
	    else
		return RedZoneDistrict::si_LowerLeftCorner;
	}
	else if( x > i_upperRight.x )
	{
	    // right strip
	    if( y < i_upperRight.y )
		return RedZoneDistrict::si_UpperRightCorner;
	    else if( y < i_lowerRight.y )
		return RedZoneDistrict::si_RightCenter;
	    else
		return RedZoneDistrict::si_LowerRightCorner;
	}
	else if( y < i_upperLeft.y )
	{
	    // upper strip
	    return RedZoneDistrict::si_UpperCenter;
	}
	else if( y > i_lowerLeft.y )
	{
	    return RedZoneDistrict::si_LowerCenter;
	}
	
	return 0; // not in red zone
    }

    Coordinate i_upperLeft;
    Coordinate i_upperRight;
    Coordinate i_lowerLeft;
    Coordinate i_lowerRight;

    static void Initialize(void);
    static RedZone si_RedZones[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
};

#endif
