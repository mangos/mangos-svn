/* MotionMaster.h
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

#ifndef MANGOS_MOTIONMASTER_H
#define MANGOS_MOTIONMASTER_H

/** MotionMaster controls the movement of the creatures.
 * It also allows the creature to mutate its state
 */

#include "IdleMovementGenerator.h"
#include <stack>

class MANGOS_DLL_DECL MotionMaster : public std::stack<MovementGenerator *>
{
    static IdleMovementGenerator si_idleMovement;
public:
    
    MotionMaster() : i_lastMotionTime(0) 
    {
	push(&si_idleMovement);
    }

    // operator override.
    MovementGenerator* operator->(void) { return top(); }

    /// Update the creatures motion
    void UpdateMotion(const uint32 &diff);
    
    /// Clear the motion queue
    void Clear(void);

    /// Current movement is overdue
    void MovementOverDue(void);

    /// Sets the movement to idle
    void Idle(void)
    {
	if( !isStatic( top() ) )
	    push( &si_idleMovement );
    }

    /// Mutate to a new movement generator due to changes in behaviour
    template<class NEW_MOVEMENT> void Mutate(void) 
    {
	NEW_MOVEMENT *m = new NEW_MOVEMENT;
	m->Reset(*i_owner);
	push(m);
    }

private:

    inline bool isStatic(MovementGenerator *mv) const { return (mv == & si_idleMovement); }
    uint32 i_lastMotionTime;    // Timer creature moves    
    Creature *i_owner; // this is be resolve.. but for now just for compilation
};

#endif
