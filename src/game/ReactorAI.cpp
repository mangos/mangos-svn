/* ReactorAI.cpp
 *
 * Copyright (C) 2004 Wow Daemon
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

/* Windows nightmare.. dependency resolution pain!!! Althgough
* some of the files are NOT well define but hey, gcc resolves it.
* Windows VC seems to be very painful so let's do this for now...
* the right way.. make files like Creature.h and Player.h well define.
*/
#include "ByteBuffer.h" // yupe has nothing to do here but Windows nightmare
#include "ReactorAI.h"
#include "Errors.h"
#include "Creature.h"
#include "Player.h"

#define MAX_RANGE_OF_SPELLS (30.0f*30.0f)

int
ReactorAI::Permissible(const Creature *creature)
{
    return -1;
}


void 
ReactorAI::MoveInLineOfSight(Creature *) 
{
}

void 
ReactorAI::AttackStart(Creature *c) 
{
    if( i_pVictim == NULL )
    {
	i_pVictim = c;
	i_creature.AI_AttackReaction(c, 0);
    }
}

void 
ReactorAI::AttackStop(Creature *c) 
{
    /* if he stops.. I shall continue until he outrun me */
}

void 
ReactorAI::HealBy(Creature *healer, uint32 amount_healed) 
{
}

void 
ReactorAI::DamageInflict(Creature *healer, uint32 amount_healed) 
{
}

bool
ReactorAI::IsVisible(Creature *creature) const 
{
    return false; /* reactor is not proactive, so we don't care if he sees it or not */
}


void 
ReactorAI::MoveInLineOfSight(Player *) 
{
}

void 
ReactorAI::AttackStart(Player *p) 
{
    if( i_pVictim != NULL )
    {
	i_pVictim = p;
	i_creature.AI_AttackReaction(p, 0);
    }
}

void 
ReactorAI::AttackStop(Player *) 
{
}

void 
ReactorAI::HealBy(Player *healer, uint32 amount_healed) 
{
}

void 
ReactorAI::DamageInflict(Player *healer, uint32 amount_healed) 
{
}

bool
ReactorAI::IsVisible(Player *pl) const 
{
    return false; /* reactor is not proactive, so we don't care he sees it or not */
}

void
ReactorAI::UpdateAI(const uint32 time_diff)
{
    /* this is where I decide what to do */
    if( i_pVictim != NULL )
    {
	if( needToStop() )
	    stopAttack();
    }
}

bool
ReactorAI::needToStop() const
{
    if( !i_pVictim->isAlive() )
	return true;

    float length_square = i_creature.GetDistanceSq(i_pVictim);
    if( length_square > MAX_RANGE_OF_SPELLS )
	return true;
    return false;
}

void
ReactorAI::stopAttack()
{
    if( i_pVictim )
	;
	// not exist yet i_creature.AI_StopAttack();
}
