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

#include "ByteBuffer.h"
#include "TargetedMovementGenerator.h"
#include "Errors.h"
#include "Creature.h"
#include "MapManager.h"
#include "Spell.h"

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
    if(!&i_target || !&owner)
        return;
    owner.Relocate(owner.GetPositionX(), owner.GetPositionY(), owner.GetPositionZ(), owner.GetAngle( &i_target ));
    float x, y, z;
    i_target.GetClosePoint( &owner, x, y, z );
    Traveller<Creature> traveller(owner);
    i_destinationHolder.SetDestination(traveller, x, y, z, 0);
}

void
TargetedMovementGenerator::_setAttackRadius(Creature &owner)
{
    if(!&owner)
        return;
    float combat_reach = owner.GetFloatValue(UNIT_FIELD_COMBATREACH);
    if( combat_reach <= 0.0f )
        combat_reach = 1.0f;
    //float bounding_radius = owner.GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS);
    i_attackRadius = combat_reach;                          // - SMALL_ALPHA);
}

void
TargetedMovementGenerator::Initialize(Creature &owner)
{
    if(!&owner)
        return;
    owner.setMoveRunFlag(true);
    _setAttackRadius(owner);
    _setTargetLocation(owner);
    owner.addUnitState(UNIT_STAT_CHASE);
}

void
TargetedMovementGenerator::Reset(Creature &owner)
{
    Initialize(owner);
}

void
TargetedMovementGenerator::TargetedHome(Creature &owner)
{
    if(!&owner)
        return;
    DEBUG_LOG("Target home location %u", owner.GetGUIDLow());
    float x, y, z;
    owner.GetRespawnCoord(x, y, z);
    Traveller<Creature> traveller(owner);
    i_destinationHolder.SetDestination(traveller, x, y, z);
    i_targetedHome = true;
    owner.clearUnitState(UNIT_STAT_ALL_STATE);
    owner.addUnitState(UNIT_STAT_FLEEING);
}

void
TargetedMovementGenerator::Update(Creature &owner, const uint32 & time_diff)
{
    if(!&owner || !&i_target)
        return;
	if(owner.hasUnitState(UNIT_STAT_ROOT))
		return;

    SpellEntry* spellInfo;
    if( owner.IsStopped() && i_target.isAlive())
    {
        if(!owner.hasUnitState(UNIT_STAT_FOLLOW) && owner.hasUnitState(UNIT_STAT_IN_COMBAT))
        {
            if( spellInfo = owner.reachWithSpellAttack( &i_target))
            {
                _spellAtack(owner, spellInfo);
                return;
            }
        }
        if( !owner.canReachWithAttack( &i_target ) )
        {
            owner.addUnitState(UNIT_STAT_CHASE);
            _setTargetLocation(owner);
            DEBUG_LOG("restart to chase");
        }
    }
    else
    {
        Traveller<Creature> traveller(owner);
        bool reach = i_destinationHolder.UpdateTraveller(traveller, time_diff, false);
        if(i_targetedHome)
            return;
        else if(!owner.hasUnitState(UNIT_STAT_FOLLOW) && owner.hasUnitState(UNIT_STAT_IN_COMBAT) && (spellInfo = owner.reachWithSpellAttack(&i_target)) )
        {
            _spellAtack(owner, spellInfo);
            return;
        }
        if(reach)
        {
            if( owner.canReachWithAttack(&i_target) )
            {
                owner.Relocate(owner.GetPositionX(), owner.GetPositionY(), owner.GetPositionZ(), owner.GetAngle( &i_target ));
                owner.StopMoving();
                if(!owner.hasUnitState(UNIT_STAT_FOLLOW))
                    owner.addUnitState(UNIT_STAT_ATTACKING);
                owner.clearUnitState(UNIT_STAT_CHASE);
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

void TargetedMovementGenerator::_spellAtack(Creature &owner, SpellEntry* spellInfo)
{
    if(!spellInfo)
        return;
    owner.StopMoving();
    owner->Idle();
    if(owner.m_currentSpell)
    {
        if(owner.m_currentSpell->m_spellInfo->Id == spellInfo->Id )
            return;
        else
        {
            delete owner.m_currentSpell;
            owner.m_currentSpell = NULL;
        }
    }
    Spell *spell = new Spell(&owner, spellInfo, false, 0);
    spell->SetAutoRepeat(true);
    owner.addUnitState(UNIT_STAT_ATTACKING);
    owner.clearUnitState(UNIT_STAT_CHASE);
    SpellCastTargets targets;
    targets.setUnitTarget( &i_target );
    spell->prepare(&targets);
    owner.m_canMove = false;
    DEBUG_LOG("Spell Attack.");
}
