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

#include "ReactorAI.h"
#include "Errors.h"

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
ReactorAI::AttackStart(Creature *) 
{
}

void 
ReactorAI::AttackStop(Creature *) 
{
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
ReactorAI::AttackStart(Player *) 
{
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
ReactorAI::UpdateAI(const uint32 )
{
}
