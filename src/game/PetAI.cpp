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
#include "FactionTemplateResolver.h"
#include "TargetedMovementGenerator.h"
#include "Database/DBCStores.h"

#define TIME_INTERVAL_LOOK   5000

int PetAI::Permissible(const Creature *creature)
{
    if( ((Pet*)&creature)->isPet())
        return SPEICAL_PERMIT_BASE;

    return NO_PERMIT;
}

PetAI::PetAI(Creature &c) : i_pet(c), i_pVictim(NULL), i_owner(ObjectAccessor::Instance().GetCreature(c, c.GetUInt64Value(UNIT_FIELD_SUMMONEDBY))), i_state( ((Pet*)&c)->GetActState() ), i_tracker(TIME_INTERVAL_LOOK)
{
}

void PetAI::MoveInLineOfSight(Unit *u)
{
}

void PetAI::AttackStart(Unit *u)
{
    if( i_pVictim == NULL && u )
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
    if( !i_pVictim->isAlive() || !i_pet.isAlive() || i_pVictim->m_stealth)
        return true;
    return false;
}

void PetAI::_stopAttack()
{
    assert( i_pVictim != NULL );
    i_pet.ClearState(UNIT_STAT_ATTACKING);
    i_pet.RemoveFlag(UNIT_FIELD_FLAGS, 0x80000 );

    if( !i_pet.isAlive() )
    {
        DEBUG_LOG("Creature stoped attacking cuz his dead [guid=%d]", i_pet.GetGUIDLow());
        i_pet.StopMoving();
        i_pet->Idle();
    }
    else if( !i_pVictim->isAlive() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is dead [guid=%d]", i_pet.GetGUIDLow());
        if((i_state & STATE_RA_FOLLOW)>0)
            i_pet->Mutate(new TargetedMovementGenerator(*i_owner));
        else
            i_pet->Idle();
    }
    else if( i_pVictim->m_stealth )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is stealth [guid=%d]", i_pet.GetGUIDLow());
        if((i_state & STATE_RA_FOLLOW)>0)
            i_pet->Mutate(new TargetedMovementGenerator(*i_owner));
        else
            i_pet->Idle();
    }
    else
    {
        DEBUG_LOG("Creature stopped attacking due to target out run him [guid=%d]", i_pet.GetGUIDLow());
        if((i_state & STATE_RA_FOLLOW)>0)
            i_pet->Mutate(new TargetedMovementGenerator(*i_owner));
        else
            i_pet->Idle();
    }
    i_pVictim = NULL;
}

void PetAI::UpdateAI(const uint32 diff)
{
    if( i_pVictim != NULL )
    {
        if( _isVisible(i_pVictim) )
        {
            DEBUG_LOG("Victim %d re-enters creature's aggro radius fater stop attacking", i_pVictim->GetGUIDLow());
            i_state = UNIT_STAT_STOPPED;
            i_pet->MovementExpired();
            // back to the cat and mice game if you move back in range
        }
        i_tracker.Update(diff);
        if( i_tracker.Passed() )
        {
            i_pet->MovementExpired();
            DEBUG_LOG("Creature running back home [guid=%d]", i_pet.GetGUIDLow());
            if((i_state & STATE_RA_FOLLOW)>0)
                i_pet->Mutate(new TargetedMovementGenerator(*i_owner));
            else
                i_pet->Idle();
            i_pVictim = NULL;
        }
        else if( !i_pet.canReachWithAttack( i_pVictim ))
        {

            float dx = i_pVictim->GetPositionX() - i_pet.GetPositionX();
            float dy = i_pVictim->GetPositionY() - i_pet.GetPositionY();
            float orientation = (float)atan2((double)dy, (double)dx);
            i_pet.Relocate(i_pet.GetPositionX(), i_pet.GetPositionY(), i_pet.GetPositionZ(), orientation);
        }

        if( _needToStop() )
        {
            DEBUG_LOG("Guard AI stoped attacking [guid=%d]", i_pet.GetGUIDLow());
            _stopAttack();
        }
        else if( i_pet.IsStopped() )
        {
            if( i_pet.isAttackReady() )
            {
                i_pet.AttackerStateUpdate(i_pVictim, 0);
                i_pet.setAttackTimer(0);
                if( !i_pet.isAlive() || !i_pVictim->isAlive() )
                    _stopAttack();
            }
        }
    }
    else
    {
        if(i_owner->testStateFlag(UNIT_STAT_IN_COMBAT) && i_owner->getAttackerSet().size())
            AttackStart(*(i_owner->getAttackerSet().begin()));
    }
}

bool PetAI::_isVisible(Unit *u) const
{
    return ( ((Creature*)&i_pet)->GetDistance(u) * 1.0<= IN_LINE_OF_SIGHT && !u->m_stealth );
}

void PetAI::_taggedToKill(Unit *u)
{
    assert( i_pVictim == NULL );
    i_pet.SetState(UNIT_STAT_ATTACKING);
    i_pet.SetFlag(UNIT_FIELD_FLAGS, 0x80000);
    i_pet->Mutate(new TargetedMovementGenerator(*u));
    i_pVictim = u;
}
