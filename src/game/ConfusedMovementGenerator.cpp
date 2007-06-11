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

#include "Creature.h"
#include "MapManager.h"
#include "Opcodes.h"
#include "ConfusedMovementGenerator.h"
#include "DestinationHolderImp.h"

void
ConfusedMovementGenerator::Initialize(Creature &creature)
{
    const float wander_distance=11;
    float x,y,z,z2;
    x = creature.GetPositionX();
    y = creature.GetPositionY();
    z = creature.GetPositionZ();
    uint32 mapid=creature.GetMapId();

    Map* map = MapManager::Instance().GetMap(mapid, &creature);
    z2 = map->GetHeight(x,y);
    if( abs( z2 - z ) < 5 )
        z = z2;

    i_nextMove = 1;

    bool is_water_ok = creature.isCanSwimOrFly();
    bool is_land_ok  = creature.isCanWalkOrFly();

    for(unsigned int idx=0; idx < MAX_CONF_WAYPOINTS+1; ++idx)
    {
        const float wanderX=wander_distance*rand_norm() - wander_distance/2;
        const float wanderY=wander_distance*rand_norm() - wander_distance/2;

        i_waypoints[idx][0] = x + wanderX;
        i_waypoints[idx][1] = y + wanderY;

        // prevent invalid coordinates generation
        MaNGOS::NormalizeMapCoord(i_waypoints[idx][0]);
        MaNGOS::NormalizeMapCoord(i_waypoints[idx][1]);

        bool is_water = map->IsInWater(i_waypoints[idx][0],i_waypoints[idx][1]);

        // if generated wrong path just ignore
        if( is_water && !is_water_ok || !is_water && !is_land_ok )
        {
            i_waypoints[idx][0] = idx > 0 ? i_waypoints[idx-1][0] : x;
            i_waypoints[idx][1] = idx > 0 ? i_waypoints[idx-1][1] : y;
        }

        z2 = map->GetHeight(i_waypoints[idx][0],i_waypoints[idx][1]);
        if( abs( z2 - z ) < 5 )
            z = z2;
        i_waypoints[idx][2] =  z;
    }
    creature.StopMoving();
}

void
ConfusedMovementGenerator::Reset(Creature &creature)
{
    i_nextMove = 1;
    i_nextMoveTime.Reset(0);
    creature.StopMoving();
}

bool
ConfusedMovementGenerator::Update(Creature &creature, const uint32 &diff)
{
    if(!&creature)
        return true;
    if(creature.hasUnitState(UNIT_STAT_ROOT) || creature.hasUnitState(UNIT_STAT_STUNDED))
        return true;
    i_nextMoveTime.Update(diff);
    i_destinationHolder.ResetUpdate();
    if( i_nextMoveTime.Passed() )
    {
        if( creature.IsStopped() )
        {
            assert( i_nextMove <= MAX_CONF_WAYPOINTS );
            const float x = i_waypoints[i_nextMove][0];
            const float y = i_waypoints[i_nextMove][1];
            const float z = i_waypoints[i_nextMove][2];
            creature.addUnitState(UNIT_STAT_ROAMING);
            CreatureTraveller traveller(creature);
            i_destinationHolder.SetDestination(traveller, x, y, z);
            traveller.Relocation(x,y,z);
            i_nextMoveTime.Reset( i_destinationHolder.GetTotalTravelTime() );
        }
        else
        {
            creature.StopMoving();
            creature.setMoveRunFlag(true);

            i_nextMove = urand(1,MAX_CONF_WAYPOINTS);
            i_nextMoveTime.Reset(urand(0, 1500-1));         // TODO: check the minimum reset time, should be probably higher
        }
    }
    return true;
}

int
ConfusedMovementGenerator::Permissible(const Creature *creature)
{
    if( creature->HasFlag(UNIT_NPC_FLAGS, 
            UNIT_NPC_FLAG_VENDOR | UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_TAXIVENDOR |
            UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_SPIRITHEALER | UNIT_NPC_FLAG_SPIRITGUIDE | UNIT_NPC_FLAG_BANKER |
            UNIT_NPC_FLAG_PETITIONER | UNIT_NPC_FLAG_TABARDDESIGNER | UNIT_NPC_FLAG_STABLE) )
        return CANNOT_HANDLE_TYPE;

    return CONFUSED_MOTION_TYPE;
}
