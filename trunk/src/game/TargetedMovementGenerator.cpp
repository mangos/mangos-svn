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

#include "ByteBuffer.h"
#include "TargetedMovementGenerator.h"
#include "Errors.h"
#include "Creature.h"
#include "MapManager.h"

#define SMALL_ALPHA 0.05

#include <cmath>

struct StackCleaner
{
    Creature &i_creature;
    StackCleaner(Creature &creature) : i_creature(creature) {}
    void Done(void) { i_creature.StopMoving(); }
    ~StackCleaner()
    {
	i_creature->Clear();
    }
};

void
TargetedMovementGenerator::_setTargetLocation(Creature &owner)
{
    float x = i_target.GetPositionX();
    float y = i_target.GetPositionY();
    float z = i_target.GetPositionZ();
    Traveller<Creature> traveller(owner);
    i_destinationHolder.SetDestination(traveller, x, y, z, i_attackRadius);
}

void
TargetedMovementGenerator::_setAttackRadius(Creature &owner)
{
    float combat_reach = owner.GetFloatValue(UNIT_FIELD_COMBATREACH);
    float bounding_radius = owner.GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS);    
    i_attackRadius =  (combat_reach + bounding_radius - SMALL_ALPHA);
}

void
TargetedMovementGenerator::Initialize(Creature &owner)
{
    owner.setMoveRunFlag(true);
    _setAttackRadius(owner);
    _setTargetLocation(owner);
    owner.SetState(CHASE);
}

void
TargetedMovementGenerator::Reset(Creature &owner)
{
    Initialize(owner);
}

void
TargetedMovementGenerator::TargetedHome(Creature &owner)
{
    DEBUG_LOG("Target home location %d", owner.GetGUIDLow());
    float x, y, z;
    owner.GetRespawnCoord(x, y, z);
    Traveller<Creature> traveller(owner);
    i_destinationHolder.SetDestination(traveller, x, y, z);
    i_targetedHome = true;
    owner.ClearState(ALL_STATE);
    owner.SetState(FLEEING);
}

void
TargetedMovementGenerator::Update(Creature &owner, const uint32 & time_diff)
{
        
    if( owner.IsStopped() )
    {
	assert( i_target.isAlive() );
	if( !owner.canReachWithAttack( &i_target ) )
	{
	    owner.SetState(CHASE);
	    _setTargetLocation(owner);
	    DEBUG_LOG("restart to chase");
	}
    }
    else
    {
	Traveller<Creature> traveller(owner);
	if( i_destinationHolder.UpdateTraveller(traveller, time_diff, false) )
	{
	    if( i_targetedHome )
	    {
		
		DEBUG_LOG("Target %d ran home", owner.GetGUIDLow());
		float x, y, z, orientation;
		owner.GetRespawnCoord(x, y, z);
		orientation = owner.GetOrientation();		
		owner.Relocate(x, y, z, orientation);
		StackCleaner stack_cleaner(owner);
		stack_cleaner.Done();
	    }
	    else if( owner.canReachWithAttack(&i_target) )
	    {
		owner.StopMoving();
		owner.SetState(ATTACKING);
		DEBUG_LOG("UNIT IS THERE");
	    }
	    else
	    {
		_setTargetLocation(owner); 
		DEBUG_LOG("Continue to chase");
	    }	    
	}
    }
}
