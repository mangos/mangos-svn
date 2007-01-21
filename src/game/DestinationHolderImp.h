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

#ifndef MANGOS_DESTINATIONHOLDERIMP_H
#define MANGOS_DESTINATIONHOLDERIMP_H

#include "Creature.h"
#include "MapManager.h"
#include "DestinationHolder.h"

#include <cmath>

template<typename TRAVELLER>
void
DestinationHolder<TRAVELLER>::_findOffSetPoint(float x1, float y1, float x2, float y2, float offset, float &x, float &y)
{
    /* given the point (x1, y1) and (x2, y2).. need to find the point (x,y) on the same line
     * such that the distance from (x, y) to (x2, y2) is offset.
     * Let the distance of p1 to p2 = d.. then the ratio of offset/d = (x2-x)/(x2-x1)
     * hence x = x2 - (offset/d)*(x2-x1)
     * like wise offset/d = (y2-y)/(y2-y1);
     */
    if( offset == 0 )
    {
        x = x2;
        y = y2;
    }
    else
    {
        double x_diff = double(x2 - x1);
        double y_diff = double(y2 - y1);
        double distance_d = (double)((x_diff*x_diff) + (y_diff * y_diff));
        if(distance_d == 0)
        {
            x = x2;
            y = y2;
        }
        else
        {
            distance_d = ::sqrt(distance_d);                // starting distance
            double distance_ratio = (double)(distance_d - offset)/(double)distance_d;
            // line above has revised formula which is more correct, I think
            x = (float)(x1 + (distance_ratio*x_diff));
            y = (float)(y1 + (distance_ratio*y_diff));
        }
    }
}

template<typename TRAVELLER>
uint32
DestinationHolder<TRAVELLER>::SetDestination(TRAVELLER &traveller, float dest_x, float dest_y, float dest_z, float offset)
{
    if (i_destX == dest_x && i_destY == dest_y && i_destZ == dest_z)
        return 0;

    i_fromX = traveller.GetPositionX();
    i_fromY = traveller.GetPositionY();
    i_fromZ = traveller.GetPositionZ();

    UpdateLocation(traveller, dest_x, dest_y, dest_z);

    float dx = dest_x - i_fromX;
    float dy = dest_y - i_fromY;
    float dz = dest_z - i_fromZ;
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz));
    double speed = traveller.Speed();
    if( speed <= 0 )
        speed = 2.5f;
    speed *= 0.001f;
    uint32 travel_time = static_cast<uint32>(dist / speed + 0.5);
    traveller.MoveTo(dest_x, dest_y, dest_z, travel_time);
    return travel_time;
}

template<typename TRAVELLER>
void
DestinationHolder<TRAVELLER>::UpdateLocation(TRAVELLER &traveller, float dest_x, float dest_y, float dest_z)
{
    i_destX = dest_x;
    i_destY = dest_y;
    i_destZ = dest_z;

    float dx = i_destX - i_fromX;
    float dy = i_destY - i_fromY;
    float dz = i_destZ - i_fromZ;
    double dist = ::sqrt((dx*dx) + (dy*dy) + (dz*dz));
    double speed = traveller.Speed();
    if(speed<=0)
        speed = 2.5f;
    speed *=  0.001f;                                       // speed is in seconds so convert from second to millisecond
    i_totalTravelTime = static_cast<uint32>( dist/speed + 0.5 );
    i_timeStarted = getMSTime();
}

template<typename TRAVELLER>
bool
DestinationHolder<TRAVELLER>::UpdateTraveller(TRAVELLER &traveller, uint32 diff, bool force_update)
{
    i_tracker.Update(diff);
    if( i_tracker.Passed() || force_update )
    {
        ResetUpdate();
        float x,y,z;
        GetLocationNow(x, y, z);
        if( x == -431602080 )
            return false;
        if( traveller.GetTraveller().GetPositionX() != x || traveller.GetTraveller().GetPositionY() != y )
        {
            float ori = traveller.GetTraveller().GetAngle(x, y);
            traveller.Relocation(x, y, z, ori);
        }
        return true;
    }
    return false;
}

template<typename TRAVELLER>
void
DestinationHolder<TRAVELLER>::GetLocationNow(float &x, float &y, float &z) const
{
    uint32 time_elapsed = getMSTime() - i_timeStarted;

    if( i_totalTravelTime == 0 || time_elapsed >= i_totalTravelTime )
    {
        x = i_destX;
        y = i_destY;
        z = i_destZ;
    }
    else
    {
        double percent_passed = (double)time_elapsed / (double)i_totalTravelTime;
        x = i_fromX + ((i_destX - i_fromX) * percent_passed);
        y = i_fromY + ((i_destY - i_fromY) * percent_passed);
        z = i_fromZ + ((i_destZ - i_fromZ) * percent_passed);
    }
}

template<typename TRAVELLER>
float
DestinationHolder<TRAVELLER>::GetDistanceFromDestSq(const WorldObject &obj) const
{
    float x,y,z;
    obj.GetPosition(x,y,z);
    return (i_destX-x)*(i_destX-x)+(i_destY-y)*(i_destY-y)+(i_destZ-z)*(i_destZ-z);
}
#endif
