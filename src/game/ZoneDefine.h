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

#ifndef MANGOS_ZONEDEFINE_H
#define MANGOS_ZONEDEFINE_H

/*
 * Perphaps this is more quicker than loading in at run time
 */

enum zone_t
    {
	ZONE_UNKNOWN = 0,
	ZONE_141 = 141,
	ZONE_12 = 12,
	ZONE_406 = 406
    };


template<zone_t T> 
struct ZoneDefinition
{
};

template<> struct ZoneDefinition<ZONE_141>
{
    static const float y2=3814.58325195312;
    static const float y1=-1277.08325195312;
    static const float x2=11831.25;
    static const float x1=8437.5;
};

template<> struct ZoneDefinition<ZONE_12>
{
    static const float y2=1535.41662597656;
    static const float y1=-1935.41662597656;
    static const float x2=-7939.5830078125;
    static const float x1=-10254.166015625;
};

#endif
