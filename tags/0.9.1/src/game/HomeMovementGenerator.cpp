/* 
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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

#include "HomeMovementGenerator.h"
#include "Creature.h"
#include "Traveller.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "DestinationHolderImp.h"

void
HomeMovementGenerator<Creature>::Initialize(Creature & owner)
{
    owner.setMoveRunFlag(true);
    _setTargetLocation(owner);
}

void
HomeMovementGenerator<Creature>::Reset(Creature &)
{
}

void
HomeMovementGenerator<Creature>::_setTargetLocation(Creature & owner)
{
    if( !&owner )
        return;

    if( owner.hasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNDED) )
        return;

    float x, y, z;
    owner.GetRespawnCoord(x, y, z);

    CreatureTraveller traveller(owner);

    uint32 travel_time = i_destinationHolder.SetDestination(traveller, x, y, z);
    modifyTravelTime(travel_time);
    owner.clearUnitState(UNIT_STAT_ALL_STATE);
}

bool
HomeMovementGenerator<Creature>::Update(Creature &owner, const uint32& time_diff)
{
    CreatureTraveller traveller( owner);
    i_destinationHolder.UpdateTraveller(traveller, time_diff, false);

    if (time_diff > i_travel_timer)
    {
        owner.setMoveRunFlag(false);
        return false;
    }

    i_travel_timer -= time_diff;

    return true;
}
