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

#include "Creature.h"
#include "MapManager.h"
#include "Opcodes.h"
#include "RandomMovementGenerator.h"
#include "DestinationHolderImp.h"

/*
Here interpolation is disabled by default due to it causing crashing
in some compile environments.
If your server can handle the small amount of lag this may cause
and you can build without experiencing a significant increase in crashes
then you may uncomment the following line to have correct random motion.
*/
// uncomment now for wide testing purpose
#define USE_INTERPOLATION

template<>
void
RandomMovementGenerator<Creature>::Initialize(Creature &creature)
{
    float x,y,z,z2, wander_distance;
    creature.GetRespawnCoord(x, y, z);
    creature.GetRespawnDist(wander_distance);
    uint32 mapid=creature.GetMapId();

    Map const* map = MapManager::Instance().GetBaseMap(mapid);
    // Initialization is done in bulk. Don’t use vamps for that (4. parameter = false). It is too costly when entering a new map grid
    z2 = map->GetHeight(x,y,z, false);                      // use .map base surface height
    if( fabs( z2 - z ) < 5 )
        z = z2;

    i_nextMove = 1;
    i_waypoints[0][0] = x;
    i_waypoints[0][1] = y;
    i_waypoints[0][2] = z;

    bool is_water_ok = creature.isCanSwimOrFly();
    bool is_land_ok  = creature.isCanWalkOrFly();

    for(unsigned int idx=1; idx < MAX_RAND_WAYPOINTS+1; ++idx)
    {
        const float angle = 2*M_PI*rand_norm();
        const float range = wander_distance*rand_norm();

        i_waypoints[idx][0] = x+ range * cos(angle);
        i_waypoints[idx][1] = y+ range * sin(angle);

        // prevent invalid coordinates generation
        MaNGOS::NormalizeMapCoord(i_waypoints[idx][0]);
        MaNGOS::NormalizeMapCoord(i_waypoints[idx][1]);

        bool is_water = map->IsInWater(i_waypoints[idx][0],i_waypoints[idx][1],z);
        // if generated wrong path just ignore
        if( is_water && !is_water_ok || !is_water && !is_land_ok )
        {
            i_waypoints[idx][0] = i_waypoints[idx-1][0];
            i_waypoints[idx][1] = i_waypoints[idx-1][1];
            i_waypoints[idx][2] = i_waypoints[idx-1][2];
            continue;
        }

        // Initialization is done in bulk. Don’t use vamps for that (4. parameter = false).
        // It is too costly when entering a new map grid, use .map base surface height
        z2 = map->GetHeight(i_waypoints[idx][0],i_waypoints[idx][1],z, false);
        if( fabs( z2 - z ) < 5 )
            z = z2;
        i_waypoints[idx][2] =  z;
    }
    i_nextMoveTime.Reset(urand(0, 10000-1));                // TODO: check the lower bound (it is probably too small)
    creature.StopMoving();
}

template<>
void
RandomMovementGenerator<Creature>::Reset(Creature &creature)
{
    i_nextMove = 1;
    i_nextMoveTime.Reset(urand(0,10000-1));                 // TODO: check the lower bound (it is probably too small)
    creature.StopMoving();
}

template<>
bool
RandomMovementGenerator<Creature>::Update(Creature &creature, const uint32 &diff)
{
    if(!&creature)
        return true;
    if(creature.hasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNDED))
        return true;
    i_nextMoveTime.Update(diff);
    #ifdef USE_INTERPOLATION
    CreatureTraveller traveller(creature);
    i_destinationHolder.UpdateTraveller(traveller, diff, false);
    #endif
    if( i_nextMoveTime.Passed() )
    {
        if( creature.IsStopped() )
        {
            assert( i_nextMove <= MAX_RAND_WAYPOINTS );
            const float x = i_waypoints[i_nextMove][0];
            const float y = i_waypoints[i_nextMove][1];
            const float z = i_waypoints[i_nextMove][2];
            creature.addUnitState(UNIT_STAT_ROAMING);
            CreatureTraveller traveller(creature);
            i_destinationHolder.SetDestination(traveller, x, y, z);
            #ifndef USE_INTERPOLATION
            traveller.Relocation(x,y,z);
            #endif
            i_nextMoveTime.Reset( i_destinationHolder.GetTotalTravelTime() );
        }
        else
        {
            creature.StopMoving();
            creature.setMoveRunFlag(!urand(0,10));

            ++i_nextMove;
            if( i_nextMove == MAX_RAND_WAYPOINTS )
            {
                i_nextMove = 0;
                i_nextMoveTime.Reset(urand(0, 3000-1));     // TODO: check the lower bound (it is probably too small)
            }
            else
            {
                i_nextMoveTime.Reset(urand(0, 7000-1));     // TODO: check the lower bound (it is probably too small)
            }
        }
    }
    return true;
}
