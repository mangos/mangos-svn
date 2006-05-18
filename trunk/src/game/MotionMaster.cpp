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

#include "MotionMaster.h"
#include "CreatureAISelector.h"
#include "Creature.h"
#include <cassert>

void
MotionMaster::Initialize(Creature *creature)
{
    i_owner = creature;
    MovementGenerator* movement = FactorySelector::selectMovementGenerator(i_owner);
    push(  movement == NULL ? &si_idleMovement : movement );
    top()->Initialize(*i_owner);
}

void
MotionMaster::UpdateMotion(const uint32 &diff)
{
    if(i_owner->hasUnitState(UNIT_STAT_ROOT))
        return;
    assert( !empty() );
    top()->Update(*i_owner, diff);
}

void
MotionMaster::Clear()
{
    while( !empty() && size() > 1 )
    {
        MovementGenerator *curr = top();
        pop();
        if( !isStatic( curr ) )
            delete curr;
    }

    assert( !empty() );
    top()->Reset(*i_owner);
}

void
MotionMaster::MovementExpired()
{
    if( empty() || size() == 1 )
        return;

    MovementGenerator *curr = top();
    pop();

    if( !isStatic(curr) )
        delete curr;

    assert( !empty() );
    top()->Reset(*i_owner);
}

IdleMovementGenerator MotionMaster::si_idleMovement;
