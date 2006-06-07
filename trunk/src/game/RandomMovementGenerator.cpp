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

#include "Creature.h"
#include "MapManager.h"
#include "Opcodes.h"
#include "RandomMovementGenerator.h"
#include "DestinationHolderImp.h"

void
RandomMovementGenerator::Initialize(Creature &creature)
{
    const float wander_distance=16;
    float x,y,z;
    creature.GetRespawnCoord(x, y, z);
    int mapid=creature.GetMapId();
    z=MapManager::Instance ().GetMap(mapid)->GetHeight(x,y);

    i_nextMove = 1;
    i_waypoints[0][0] = x;
    i_waypoints[0][1] = y;
    i_waypoints[0][2] = z;

    for(unsigned int idx=1; idx < MAX_RAND_WAYPOINTS+1; ++idx)
    {
        const float wanderX=((wander_distance*rand())/RAND_MAX)-wander_distance/2;
        const float wanderY=((wander_distance*rand())/RAND_MAX)-wander_distance/2;

        if( idx == 1 )
        {
            i_waypoints[idx][0] = x + wanderX;
            i_waypoints[idx][1] = y + wanderY;
            i_waypoints[idx][2] = MapManager::Instance ().GetMap(mapid)->GetHeight(i_waypoints[idx][0],i_waypoints[idx][1]);
        }
        else
        {
            i_waypoints[idx][0] = i_waypoints[idx-1][0]+wanderX;
            i_waypoints[idx][1] = i_waypoints[idx-1][1]+wanderY;
            i_waypoints[idx][2] =  MapManager::Instance ().GetMap(mapid)->GetHeight(i_waypoints[idx][0],
                i_waypoints[idx][1]);
        }
    }

    i_nextMoveTime.Reset((rand() % 10000));
    creature.StopMoving();
}

void
RandomMovementGenerator::Reset(Creature &creature)
{
    i_nextMove = 1;
    i_nextMoveTime.Reset((rand() % 10000));
    creature.StopMoving();
}

void
RandomMovementGenerator::Update(Creature &creature, const uint32 &diff)
{
    if(!&creature)
        return;
    if(creature.hasUnitState(UNIT_STAT_ROOT) || creature.hasUnitState(UNIT_STAT_STUNDED))
        return;
    i_nextMoveTime.Update(diff);
    Traveller<Creature> traveller(creature);
    i_destinationHolder.UpdateTraveller(traveller, diff,false);
    if( i_nextMoveTime.Passed() )
    {
        if( creature.IsStopped() )
        {
            assert( i_nextMove <= MAX_RAND_WAYPOINTS );
            const float &x = i_waypoints[i_nextMove][0];
            const float &y = i_waypoints[i_nextMove][1];
            const float &z = i_waypoints[i_nextMove][2];

            creature.addUnitState(UNIT_STAT_ROAMING);
            Traveller<Creature> traveller(creature);
            i_destinationHolder.SetDestination(traveller, x, y, z);
            i_nextMoveTime.Reset( i_destinationHolder.GetTotalTravelTime() );
        }
        else
        {
            creature.StopMoving();
            creature.setMoveRunFlag(!urand(0,10));

            if( creature.getMoveRandomFlag() )
            {
                i_nextMove = (rand() % MAX_RAND_WAYPOINTS) + 1;
                i_nextMoveTime.Reset((rand() % 7000));
            }
            else
            {
                ++i_nextMove;
                if( i_nextMove == MAX_RAND_WAYPOINTS )
                {
                    i_nextMove = 0;
                    i_nextMoveTime.Reset(rand() % 3000);
                }
                else
                {
                    i_nextMoveTime.Reset((rand() % 7000));
                }
            }
        }
    }
}

int
RandomMovementGenerator::Permissible(const Creature *creature)
{
    if( creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR)
        || creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP)
        || creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER)
        || creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TAXIVENDOR)
        || creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER)
        || creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER)
        || creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER)
        || creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PETITIONER)
        || creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDVENDOR)
        || creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_STABLE))
        return CANNOT_HANDLE_TYPE;

    return RANDOM_MOTION_TYPE;
}
