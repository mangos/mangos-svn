/* AggressorAI.cpp
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

#include "AggressorAI.h"
#include "Errors.h"
#include "Creature.h"
#include "Player.h"
#include "Utilities.h"

int
AggressorAI::Permissible(const Creature *creature)
{
    //    if( creature->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) == Beast )
    //	return 1;
    return -1;
}


void 
AggressorAI::MoveInLineOfSight(Creature *) 
{
}

void 
AggressorAI::AttackStart(Creature *) 
{
}

void 
AggressorAI::AttackStop(Creature *) 
{
}

void 
AggressorAI::HealBy(Creature *healer, uint32 amount_healed) 
{
}

void 
AggressorAI::DamageInflict(Creature *healer, uint32 amount_healed) 
{
}

bool
AggressorAI::IsVisible(Creature *creature) const 
{
    return MaNGOS::Utilities::is_in_line_of_sight(i_creature.GetPositionX(), i_creature.GetPositionY(), i_creature.GetPositionZ(),
						  creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), 1.0);
}


void 
AggressorAI::MoveInLineOfSight(Player *) 
{
}

void 
AggressorAI::AttackStart(Player *) 
{
}

void 
AggressorAI::AttackStop(Player *) 
{
}

void 
AggressorAI::HealBy(Player *healer, uint32 amount_healed) 
{
}

void 
AggressorAI::DamageInflict(Player *healer, uint32 amount_healed) 
{
}

bool
AggressorAI::IsVisible(Player *pl) const 
{
    return MaNGOS::Utilities::is_in_line_of_sight(i_creature.GetPositionX(), i_creature.GetPositionY(), i_creature.GetPositionZ(),
						 pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), 1.0);
}

void
AggressorAI::UpdateAI(const uint32)
{
}
