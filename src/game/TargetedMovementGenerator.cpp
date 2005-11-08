/* TargetedMovementGenerator.h
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

#include "TargetedMovementGenerator.h"
#include "Errors.h"
#include "Creature.h"

void
TargetedMovementGenerator::Initialize(const Creature &creature)
{
}

void
TargetedMovementGenerator::Reset(const Creature &)
{
}

bool
TargetedMovementGenerator::GetNext(const Creature &owner, float &x, float &y, float &z, float &orientation)
{
    float target_distance = i_target.GetDistanceSq(&owner);
    float reach = owner.GetFloatValue(UNIT_FIELD_COMBATREACH);
    float radius = owner.GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS);
    
    if( target_distance > (reach + radius) )
    {
	float q = (target_distance - radius)/target_distance;
	x = (owner.GetPositionX() + i_target.GetPositionX()*q)/(1+q);
	y = (owner.GetPositionY() + i_target.GetPositionY()*q)/(1+q);
	z = (owner.GetPositionZ() + i_target.GetPositionZ()*q)/(1+q);
	return true;
    }

    return false; /* already there */
}

void
TargetedMovementGenerator::Update(Creature &, const uint32 & time_diff)
{
}
