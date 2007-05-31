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

#include "PetAI.h"
#include "Errors.h"
#include "Pet.h"
#include "Player.h"
#include "TargetedMovementGenerator.h"
#include "Database/DBCStores.h"
#include "Spell.h"
#include "ObjectAccessor.h"

int PetAI::Permissible(const Creature *creature)
{
    if( creature->isPet())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

PetAI::PetAI(Creature &c) : i_pet(c), i_victimGuid(0), i_tracker(TIME_INTERVAL_LOOK)
{

}

void PetAI::MoveInLineOfSight(Unit *u)
{
    if( !i_pet.getVictim() && (i_pet.isPet() && ((Pet&)i_pet).HasActState(STATE_RA_PROACTIVE) || i_pet.isCharmed()) &&
        u->isTargetableForAttack() && i_pet.IsHostileTo( u )  &&
        u->isInAccessablePlaceFor(&i_pet))
    {
        float attackRadius = i_pet.GetAttackDistance(u);
        if(i_pet.IsWithinDistInMap(u, attackRadius) && i_pet.GetDistanceZ(u) <= CREATURE_Z_ATTACK_RANGE)
        {
            AttackStart(u);
            if(u->HasStealthAura())
                u->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
        }
    }
}

void PetAI::AttackStart(Unit *u)
{
    if( i_pet.getVictim() || !u || i_pet.isPet() && ((Pet&)i_pet).getPetType()==MINI_PET )
        return;

    DEBUG_LOG("Start to attack");
    if(i_pet.Attack(u))
    {
        i_pet.clearUnitState(UNIT_STAT_FOLLOW);
        i_pet->Clear();
        i_victimGuid = u->GetGUID();
        i_pet->Mutate(new TargetedMovementGenerator(*u));
    }
}

void PetAI::EnterEvadeMode()
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
    if(!i_pet.getVictim() || !i_pet.isAlive())
        return true;

    // This is needed for charmed creatures, as once their target was reset other effects can trigger threat
    if(i_pet.isCharmed() && i_pet.getVictim() == i_pet.GetCharmer())
        return true;

    return !i_pet.getVictim()->isTargetableForAttack();
}

void PetAI::_stopAttack()
{
    if( !i_victimGuid )
        return;

    Unit* victim = ObjectAccessor::Instance().GetUnit(i_pet, i_victimGuid );

    if ( !victim )
        return;

    assert(!i_pet.getVictim() || i_pet.getVictim() == victim);

    if( !i_pet.isAlive() )
    {
        DEBUG_LOG("Creature stoped attacking cuz his dead [guid=%u]", i_pet.GetGUIDLow());
        i_pet.StopMoving();
        i_pet->Clear();
        i_pet->Idle();
        i_victimGuid = 0;
        i_pet.CombatStop(true);
        i_pet.DeleteInHateListOf();
        return;
    }
    else if( !victim  )
    {
        DEBUG_LOG("Creature stopped attacking because victim is non exist [guid=%u]", i_pet.GetGUIDLow());
    }
    else if( !victim->isAlive() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is dead [guid=%u]", i_pet.GetGUIDLow());
    }
    else if( victim->HasStealthAura() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is stealth [guid=%u]", i_pet.GetGUIDLow());
    }
    else if( victim->isInFlight() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is fly away [guid=%u]", i_pet.GetGUIDLow());
    }
    else
    {
        DEBUG_LOG("Creature stopped attacking due to target out run him [guid=%u]", i_pet.GetGUIDLow());
    }

    Unit* owner = i_pet.GetCharmerOrOwner();

    if(((Pet*)&i_pet)->HasActState(STATE_RA_FOLLOW) && owner)
    {
        i_pet.addUnitState(UNIT_STAT_FOLLOW);
        i_pet->Clear();
        i_pet->Mutate(new TargetedMovementGenerator(*owner,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE));
    }
    else
    {
        i_pet.clearUnitState(UNIT_STAT_FOLLOW);
        i_pet.addUnitState(UNIT_STAT_STOPPED);
        i_pet->Clear();
        i_pet->Idle();
    }
    i_victimGuid = 0;
    i_pet.AttackStop();
}

void PetAI::UpdateAI(const uint32 diff)
{
    // update i_victimGuid if i_pet.getVictim() !=0 and changed
    if(i_pet.getVictim())
        i_victimGuid = i_pet.getVictim()->GetGUID();

    Unit* owner = i_pet.GetCharmerOrOwner();

    // i_pet.getVictim() can't be used for check in case stop fighting, i_pet.getVictim() clearóâ at Unit death etc.
    if( i_victimGuid )
    {
        if( _needToStop() )
        {
            DEBUG_LOG("Pet AI stoped attacking [guid=%u]", i_pet.GetGUIDLow());
            _stopAttack();                                  // i_victimGuid == 0 && i_pet.getVictim() == NULL now
            return;
        }
        else if( i_pet.IsStopped() || i_pet.IsWithinDistInMap(i_pet.getVictim(), ATTACK_DIST))
        {
            SpellEntry const* spellInfo;
            // required to be stopped cases
            if ( i_pet.IsStopped() && i_pet.m_currentSpell )
            {
                if( i_pet.hasUnitState(UNIT_STAT_FOLLOW) )
                    i_pet.m_currentSpell->cancel();
                else
                    return;
            }
            else if( i_pet.IsStopped() && !i_pet.hasUnitState(UNIT_STAT_FOLLOW) &&
                ((Pet*)&i_pet)->HasActState(STATE_RA_AUTOSPELL) && (spellInfo = i_pet.reachWithSpellAttack(i_pet.getVictim())))
            {
                Spell *spell = new Spell(&i_pet, spellInfo, false, 0);
                spell->SetAutoRepeat(true);
                SpellCastTargets targets;
                targets.setUnitTarget( i_pet.getVictim() );
                spell->prepare(&targets);
                i_pet.m_canMove = false;
                DEBUG_LOG("Spell Attack.");
            }
            // not required to be stopped case
            else if( i_pet.isAttackReady() && i_pet.canReachWithAttack(i_pet.getVictim()) )
            {
                i_pet.AttackerStateUpdate(i_pet.getVictim());

                i_pet.resetAttackTimer();

                if ( !i_pet.getVictim() )
                    return;

                //if pet misses its target, it will also be the first in threat list
                i_pet.getVictim()->AddThreat(&i_pet,0.0f);

                if( _needToStop() )
                    _stopAttack();
            }
        }
    }
    else if(owner)
    {
        if(owner->isInCombat() && (!((Pet*)&i_pet)->HasActState(STATE_RA_PASSIVE)))
        {
            AttackStart(owner->getAttackerForHelper());
        }
        else if(((Pet*)&i_pet)->HasActState(STATE_RA_FOLLOW))
        {
            if (!i_pet.hasUnitState(UNIT_STAT_FOLLOW) )
            {
                i_pet.addUnitState(UNIT_STAT_FOLLOW);
                i_pet->Clear();
                i_pet->Mutate(new TargetedMovementGenerator(*owner,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE));
            }
        }
    }
}

bool PetAI::_isVisible(Unit *u) const
{
    return false;                                           //( ((Creature*)&i_pet)->GetDistanceSq(u) * 1.0<= sWorld.getConfig(CONFIG_SIGHT_GUARDER) && !u->m_stealth && u->isAlive());
}
