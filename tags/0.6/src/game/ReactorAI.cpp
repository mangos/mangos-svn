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

#include "ByteBuffer.h"
#include "ReactorAI.h"
#include "Errors.h"
#include "Creature.h"
#include "TargetedMovementGenerator.h"
#include "Log.h"

#define REACTOR_VISIBLE_RANGE (26.46f)

int
ReactorAI::Permissible(const Creature *creature)
{
    if( creature->IsNeutralToAll() )
        return PERMIT_BASE_REACTIVE;

    return PERMIT_BASE_NO;
}

void
ReactorAI::MoveInLineOfSight(Unit *)
{
}

void
ReactorAI::AttackStart(Unit *p)
{
    if( i_creature.getVictim() == NULL )
    {
        if(i_creature.Attack(p))
        {
            DEBUG_LOG("Tag unit LowGUID(%u) HighGUID(%u) as a victim", p->GetGUIDLow(), p->GetGUIDHigh());
            i_victimGuid = p->GetGUID();
            i_creature->Mutate(new TargetedMovementGenerator(*p));
            if (p->GetTypeId() == TYPEID_PLAYER)
                i_creature.SetLootRecipient((Player*)p);
        }
    }
}

void
ReactorAI::AttackStop(Unit *)
{

}

void
ReactorAI::HealBy(Unit *healer, uint32 amount_healed)
{
}

void
ReactorAI::DamageInflict(Unit *healer, uint32 amount_healed)
{
}

bool
ReactorAI::IsVisible(Unit *pl) const
{
    return false;
}

void
ReactorAI::UpdateAI(const uint32 time_diff)
{
    // update i_victimGuid if i_creature.getVictim() !=0 and changed
    if(i_creature.getVictim())
        i_victimGuid = i_creature.getVictim()->GetGUID();

    // i_creature.getVictim() can't be used for check in case stop fighting, i_creature.getVictim() cleared at Unit death etc.
    if( i_victimGuid )
    {
        if( needToStop() )
        {
            DEBUG_LOG("Creature %u stopped attacking.", i_creature.GetGUIDLow());
            stopAttack();                                   // i_victimGuid == 0 && i_creature.getVictim() == NULL now
            return;
        }
        else if( i_creature.IsWithinDist(i_creature.getVictim(), ATTACK_DIST))
        {
            if( i_creature.isAttackReady() )
            {
                Unit* newtarget = i_creature.SelectHostilTarget();
                if(newtarget)
                    AttackStart(newtarget);

                i_creature.AttackerStateUpdate(i_creature.getVictim());
                i_creature.resetAttackTimer();

                if ( !i_creature.getVictim() )
                    return;

                if( needToStop() )
                    stopAttack();
            }
        }
    }
}

bool
ReactorAI::needToStop() const
{
    if( !i_creature.isAlive() || !i_creature.getVictim() || !i_creature.getVictim()->isTargetableForAttack() )
        return true;

    if(!i_creature.getVictim()->isInAccessablePlaceFor(&i_creature))
        return true;

    float rx,ry,rz;
    i_creature.GetRespawnCoord(rx, ry, rz);
    float spawndist=i_creature.GetDistanceSq(rx,ry,rz);
    float length = i_creature.GetDistanceSq(i_creature.getVictim());
    float hostillen=i_creature.GetHostility( i_creature.getVictim()->GetGUID())/(2.5f * i_creature.getLevel()+1.0f);
    return (( length > (10.0f + hostillen) * (10.0f + hostillen) && spawndist > 6400.0f )
        || ( length > (20.0f + hostillen) * (20.0f + hostillen) && spawndist > 2500.0f )
        || ( length > (30.0f + hostillen) * (30.0f + hostillen) ));
}

void
ReactorAI::stopAttack()
{
    if( !i_creature.isAlive() )
    {
        DEBUG_LOG("Creature stoped attacking cuz his dead [guid=%u]", i_creature.GetGUIDLow());
        i_creature->MovementExpired();
        i_creature->Idle();
        i_victimGuid = 0;
        i_creature.CombatStop();
        return;
    }

    assert( i_victimGuid );

    Unit* victim = ObjectAccessor::Instance().GetUnit(i_creature, i_victimGuid );

    if( !victim  )
    {
        DEBUG_LOG("Creature stopped attacking because victim is non exist [guid=%u]", i_creature.GetGUIDLow());
    }
    else if( victim->HasStealthAura() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is stealth [guid=%u]", i_creature.GetGUIDLow());
    }
    else if( victim->isInFlight() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is fly away [guid=%u]", i_creature.GetGUIDLow());
    }
    else
    {
        DEBUG_LOG("Creature stopped attacking due to target %s [guid=%u]", victim->isAlive() ? "out run him" : "is dead", i_creature.GetGUIDLow());
    }

    i_victimGuid = 0;
    i_creature.AttackStop();

    // Remove TargetedMovementGenerator from MotionMaster stack list, and add HomeMovementGenerator instead
    if( i_creature->top()->GetMovementGeneratorType() == TARGETED_MOTION_TYPE )
        i_creature->TargetedHome();
}
