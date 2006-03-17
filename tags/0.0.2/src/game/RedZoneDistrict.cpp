/* RedZoneDistrict.cpp
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



#include "RedZoneDistrict.h"

// static initialization
/*   | Z2|   |
 * Z1|   | Z3|
 *-----------
 *   | X |   |
 * Z4|   |Z5 |
 *-----------
 *   | Z7|   |
 * Z6|   | Z8|
 *-----------
 *
 * The marked X is the current active grid that the player's in..
 * Z1 to Z8 is the affected zones represented by the mask...
 */
uint8 RedZoneDistrict::si_UpperLeftCorner = (DISTRICT_1 | DISTRICT_2 | DISTRICT_4);
uint8 RedZoneDistrict::si_UpperRightCorner = (DISTRICT_2 | DISTRICT_3 | DISTRICT_5);
uint8 RedZoneDistrict::si_LowerLeftCorner = (DISTRICT_4 | DISTRICT_6 | DISTRICT_7);
uint8 RedZoneDistrict::si_LowerRightCorner = (DISTRICT_7 | DISTRICT_8 | DISTRICT_5);
uint8 RedZoneDistrict::si_LeftCenter = DISTRICT_4;
uint8 RedZoneDistrict::si_RightCenter = DISTRICT_5;
uint8 RedZoneDistrict::si_UpperCenter = DISTRICT_2;
uint8 RedZoneDistrict::si_LowerCenter = DISTRICT_7;


void RedZone::Initialize()
{
    for(unsigned int x=0; x < MAX_NUMBER_OF_GRIDS; ++x)
    for(unsigned y=0; y < MAX_NUMBER_OF_GRIDS; ++y)
        si_RedZones[x][y].Initialize(x, y);
}

//=====================================//
//     RedZoneDistrict                //
void RedZoneDistrict::Enter(GridPair &p)
{
    uint8 mask = RedZone::si_RedZones[p.x_coord][p.y_coord].Enter(i_player.GetPositionX(), i_player.GetPositionY());
    if( mask != 0 )
    i_map.ZoneAlert(i_player, p, mask);
}

RedZone RedZone::si_RedZones[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];

