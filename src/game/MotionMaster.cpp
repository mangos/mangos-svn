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

#include "MotionMaster.h"
#include "CreatureAISelector.h"
#include "Creature.h"
#include "Traveller.h"
#include "HomeMovementGenerator.h"
#include "IdleMovementGenerator.h"
#include <cassert>

inline bool isStatic(MovementGenerator *mv)
{
    return (mv == &si_idleMovement);
}

void
MotionMaster::Initialize()
{
    // clear ALL movement generators (including default)
    while(!empty())
    {
        MovementGenerator *curr = top();
        pop();
        if( !isStatic( curr ) )
            delete curr;
    }

    // set new default movement generator
    if(i_owner->GetTypeId() == TYPEID_UNIT)
    {
        MovementGenerator* movement = FactorySelector::selectMovementGenerator((Creature*)i_owner);
        push(  movement == NULL ? &si_idleMovement : movement );
        top()->Initialize(*i_owner);
    }
    else
        push(&si_idleMovement);
}

MotionMaster::~MotionMaster()
{
    // clear ALL movement generators (including default)
    while(!empty())
    {
        MovementGenerator *curr = top();
        pop();
        if( !isStatic( curr ) )
            delete curr;
    }
}

void
MotionMaster::UpdateMotion(const uint32 &diff)
{
    if(i_owner->hasUnitState(UNIT_STAT_ROOT) || i_owner->hasUnitState(UNIT_STAT_STUNDED))
        return;
    assert( !empty() );
    if (!top()->Update(*i_owner, diff))
        MovementExpired();
}

void
MotionMaster::Clear(bool reset)
{
    while( !empty() && size() > 1 )
    {
        MovementGenerator *curr = top();
        pop();
        if( !isStatic( curr ) )
            delete curr;
    }

    assert( !empty() );
    if (reset) top()->Reset(*i_owner);
}

void
MotionMaster::MovementExpired(bool reset)
{
    if( empty() || size() == 1 )
        return;

    MovementGenerator *curr = top();
    pop();

    if( !isStatic(curr) )
        delete curr;

    assert( !empty() );
    while( !empty() && top()->GetMovementGeneratorType() == TARGETED_MOTION_TYPE )
    {
        // Should check if target is still valid? If not valid it will crash.
        curr = top();
        pop();
        delete curr;
    }
    if( empty() )
        Initialize();
    if (reset) top()->Reset(*i_owner);
}

void
MotionMaster::TargetedHome()
{
    if(i_owner->hasUnitState(UNIT_STAT_FLEEING))
        return;

    DEBUG_LOG("Target home location %u", i_owner->GetGUIDLow());

    Clear(false);
    Mutate(new HomeMovementGenerator<Creature>());
}

void MotionMaster::Idle(void)
{
    if( !isStatic( top() ) )
        push( &si_idleMovement );
}

void MotionMaster::Mutate(MovementGenerator *m)
{
    // HomeMovement is not that important, delete it if meanwhile a new comes
    if (!empty() && top()->GetMovementGeneratorType() == HOME_MOTION_TYPE)
        MovementExpired(false);
    m->Initialize(*i_owner);
    push(m);
}
