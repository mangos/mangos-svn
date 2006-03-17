/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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

#include "Creature.h"
#include "MapManager.h"
#include "DestinationHolder.h"


#include <cmath>

void
DestinationHolder::SetDestination(Creature &traveller, const float &dest_x, const float &dest_y, const float &dest_z)
{
    i_fromX = traveller.GetPositionX();
    i_fromY = traveller.GetPositionY();
    i_fromZ = traveller.GetPositionZ();
    UpdateLocation(traveller, dest_x, dest_y, dest_z);

    
    float dx = i_destX - i_fromX;
    float dy = i_destY - i_fromY;
    float dz = i_destZ - i_fromZ;    
    float d_square = (dx*dx) + (dy*dy) + (dz*dz);
    double speed = traveller.GetMobSpeed() * (traveller.getMoveRunFlag() ? UNIT_RUN_SPEED : UNIT_WALK_SPEED);
    speed *= 0.001;
    uint32 travel_time = static_cast<int>((::sqrt(d_square) / speed ) + 0.5);
    float orientation = (float)atan2((double)dy, (double)dx);
    traveller.Relocate(i_fromX, i_fromY, i_fromZ, orientation);

    
    traveller.AI_SendMoveToPacket(i_destX, i_destY, i_destZ, travel_time, traveller.getMoveRunFlag());
}

void
DestinationHolder::UpdateLocation(Creature &traveller, const float &dest_x, const float &dest_y, const float &dest_z)
{
    i_destX = dest_x;
    i_destY = dest_y;
    i_destZ = dest_z;

    float dx = i_destX - i_fromX;
    float dy = i_destY - i_fromY;
    double dist = ::sqrt((dx*dx) + (dy*dy));
    double speed = traveller.GetMobSpeed() * (traveller.getMoveRunFlag() ? UNIT_RUN_SPEED : UNIT_WALK_SPEED);
    speed *=  0.001f; 
    i_totalTravelTime = static_cast<uint32>( dist/speed + 0.5 );
    i_timeStarted = getMSTime();
}

bool
DestinationHolder::UpdateTraveller(Creature &traveller, const uint32 &diff, bool force_update)
{
    i_tracker.Update(diff);
    if( i_tracker.Passed() || force_update )
    {
	float x,y,z;
	GetLocationNow(x, y, z);
	MapManager::Instance().GetMap(traveller.GetMapId())->CreatureRelocation(&traveller, x, y, z, traveller.GetOrientation());
	ResetUpdate();
	return true;
    }

    return false;
}

void
DestinationHolder::GetLocationNow(float &x, float &y, float &z) const
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
	
	double percent_passed = (double)((double)time_elapsed / (double)i_totalTravelTime);
	x = i_fromX + ((i_destX - i_fromX) * percent_passed);
	y = i_fromY + ((i_destY - i_fromY) * percent_passed);
	z = i_fromZ + ((i_destZ - i_fromZ) * percent_passed);
    }
}
