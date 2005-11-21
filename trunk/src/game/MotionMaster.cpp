/* MotionMaster.cpp
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


#include "MotionMaster.h"

void
MotionMaster::UpdateMotion(const uint32 &diff)
{
}

void
MotionMaster::Clear()
{
    while( !empty() )
    {
	MovementGenerator *curr = top();
	pop();
	if( !isStatic( curr ) )
	    delete curr;
    }

    push(&si_idleMovement);
}

void
MotionMaster::MovementOverDue()
{
    MovementGenerator *curr = top();
    pop();

    if( !isStatic(curr) )
	delete curr;
    else if( empty() )
	push(curr);
    top()->Reset(*i_owner);
}

// static variable
IdleMovementGenerator MotionMaster::si_idleMovement;

