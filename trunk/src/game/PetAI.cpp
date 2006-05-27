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

#include "PetAI.h"
#include "Errors.h"
#include "Pet.h"
#include "Player.h"
#include "TargetedMovementGenerator.h"
#include "Database/DBCStores.h"
#include "Spell.h"

int PetAI::Permissible(const Creature *creature)
{
    if( creature->isPet())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

PetAI::PetAI(Creature &c) : i_pet(c), i_tracker(TIME_INTERVAL_LOOK)
{
    i_owner = ObjectAccessor::Instance().GetCreature(c, c.GetUInt64Value(UNIT_FIELD_SUMMONEDBY));
    if(!i_owner)
        i_owner = ObjectAccessor::Instance().GetPlayer(c, c.GetUInt64Value(UNIT_FIELD_SUMMONEDBY));
}

void PetAI::MoveInLineOfSight(Unit *u)
{
}

void PetAI::AttackStart(Unit *u)
{
    if(!u)
        return;
    _taggedToKill(u);
}

void PetAI::AttackStop(Unit *)
{

}

void PetAI::HealBy(Unit *healer, uint32 amount_healed)
{
}

void PetAI::DamageInflict(Unit *healer, uint32 amount_healed)
{
}

bool PetAI::IsVisible(Unit *pl) const
{
    return _isVisible(pl);
}

bool PetAI::_needToStop() const
{
    return !i_pet.getVictim()->isTargetableForAttack() || !i_pet.isAlive();
}

void PetAI::_stopAttack()
{
    if(!i_pet.getVictim())
        return;
    
    if( !i_pet.isAlive() )
    {
        DEBUG_LOG("Creature stoped attacking cuz his dead [guid=%u]", i_pet.GetGUIDLow());
        i_pet.StopMoving();
        i_pet->Idle();
        return;
    }
    else if( !i_pet.getVictim()->isAlive() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is dead [guid=%u]", i_pet.GetGUIDLow());
    }
    else if( i_pet.getVictim()->isStealth() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is stealth [guid=%u]", i_pet.GetGUIDLow());
    }
    else if( i_pet.getVictim()->isInFlight() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is fly away [guid=%u]", i_pet.GetGUIDLow());
    }
    else
    {
        DEBUG_LOG("Creature stopped attacking due to target out run him [guid=%u]", i_pet.GetGUIDLow());
    }
    
    if(((Pet*)&i_pet)->HasActState(STATE_RA_FOLLOW))
    {
        i_pet.addUnitState(UNIT_STAT_FOLLOW);
        i_pet->Mutate(new TargetedMovementGenerator(*i_owner));
    }
    else
    {
        i_pet.clearUnitState(UNIT_STAT_FOLLOW);
        i_pet.addUnitState(UNIT_STAT_STOPPED);
        i_pet->Idle();
    }
    i_pet.AttackStop();
}

void PetAI::UpdateAI(const uint32 diff)
{
    if( i_pet.getVictim() != NULL && i_pet.isInCombat())
    {
        if( _needToStop() )
        {
            DEBUG_LOG("Pet AI stoped attacking [guid=%u]", i_pet.GetGUIDLow());
            _stopAttack();
            return;
        }
        else if( i_pet.IsStopped() )
        {
            SpellEntry *spellInfo;
            if ( i_pet.m_currentSpell )
            {
                if( i_pet.hasUnitState(UNIT_STAT_FOLLOW) )
                    i_pet.m_currentSpell = NULL;
                else
                    return;
            }
            else if( !i_pet.hasUnitState(UNIT_STAT_FOLLOW) && ((Pet*)&i_pet)->HasActState(STATE_RA_AUTOSPELL) && (spellInfo = i_pet.reachWithSpellAttack(i_pet.getVictim())))
            {
                Spell *spell = new Spell(&i_pet, spellInfo, false, 0);
                spell->SetAutoRepeat(true);
                SpellCastTargets targets;
                targets.setUnitTarget( i_pet.getVictim() );
                spell->prepare(&targets);
                i_pet.m_canMove = false;
                DEBUG_LOG("Spell Attack.");
            }
            else if( i_pet.isAttackReady() && i_pet.canReachWithAttack(i_pet.getVictim()) )
            {
                i_pet.AttackerStateUpdate(i_pet.getVictim(), 0);
                i_pet.setAttackTimer(0);

                if( _needToStop() )
                    _stopAttack();
            }
        }
    }
    else
    {
        if(i_owner->isInCombat()) {
            AttackStart(i_owner->getAttackerForHelper());
        }   
    }
}

bool PetAI::_isVisible(Unit *u) const
{
    return false;                                           //( ((Creature*)&i_pet)->GetDistanceSq(u) * 1.0<= sWorld.getConfig(CONFIG_SIGHT_GUARDER) && !u->m_stealth && u->isAlive());
}

void PetAI::_taggedToKill(Unit *u)
{
    if( i_pet.getVictim() || !u)
        return;
    i_pet.clearUnitState(UNIT_STAT_FOLLOW);
    i_pet.Attack(u);
    i_pet->Mutate(new TargetedMovementGenerator(*u));
    
    /*SpellEntry *spellInfo;
    if( ((Pet*)&i_pet)->HasActState(STATE_RA_AUTOSPELL) && (spellInfo = i_pet.reachWithSpellAttack( u )))
    {
        Spell *spell = new Spell(&i_pet, spellInfo, false, 0);
        spell->SetAutoRepeat(true);
        SpellCastTargets targets;
        targets.setUnitTarget( u );
        spell->prepare(&targets);
        i_pet.m_canMove = false;
        DEBUG_LOG("Spell Attack.");
    }
    else*/
}
