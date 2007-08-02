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
#include "ObjectAccessor.h"

#define REACTOR_VISIBLE_RANGE (26.46f)

int
ReactorAI::Permissible(const Creature *creature)
{
    if( creature->isCivilian() || creature->IsNeutralToAll() )
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
    if(!p)
        return;

    if(i_creature.Attack(p))
    {
        DEBUG_LOG("Tag unit GUID: %u (TypeId: %u) as a victim", p->GetGUIDLow(), p->GetTypeId());
        i_creature.AddThreat(p, 0.0f);
        i_victimGuid = p->GetGUID();
        i_creature->Mutate(new TargetedMovementGenerator(*p));
        if (p->GetTypeId() == TYPEID_PLAYER)
            i_creature.SetLootRecipient((Player*)p);
    }
}

void
ReactorAI::stopAttack()
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
    if(!i_creature.SelectHostilTarget())
        return;

    i_victimGuid = i_creature.getVictim()->GetGUID();

    // i_creature.getVictim() can't be used for check in case stop fighting, i_creature.getVictim() cleared at Unit death etc.
    if( i_victimGuid )
    {
        if( needToStop() )
        {
            DEBUG_LOG("Reactor AI stoped attacking [guid=%u]", i_creature.GetGUIDLow());
            EnterEvadeMode();                               // i_victimGuid == 0 && i_creature.getVictim() == NULL now
            return;
        }

        if( i_creature.IsWithinDistInMap(i_creature.getVictim(), ATTACK_DISTANCE))
        {
            if( i_creature.isAttackReady() )
            {
                i_creature.AttackerStateUpdate(i_creature.getVictim());
                i_creature.resetAttackTimer();
            }
        }
    }
}

bool
ReactorAI::needToStop() const
{
    if( !i_creature.isAlive() || !i_creature.getVictim())
        return true;

    /*if(!i_creature.getVictim()->isTargetableForAttack() || !i_creature.getVictim()->isInAccessablePlaceFor(&i_creature))
        return true;*/
    //no need for this checks because mob changes its victim only when
    //1) victim is dead (check is in SelectHostilTarget() func)
    //2) victim is out of threat radius

    float rx,ry,rz;
    i_creature.GetRespawnCoord(rx, ry, rz);
    float length = i_creature.getVictim()->GetDistanceSq(rx,ry,rz);
    return ( length > CREATURE_THREAT_RADIUS );
}

void
ReactorAI::EnterEvadeMode()
{
    if( !i_creature.isAlive() )
    {
        DEBUG_LOG("Creature stoped attacking cuz his dead [guid=%u]", i_creature.GetGUIDLow());
        i_creature->MovementExpired();
        i_creature->Idle();
        i_victimGuid = 0;
        i_creature.CombatStop(true);
        i_creature.DeleteThreatList();
        return;
    }

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

    i_creature.RemoveAllAuras();
    i_creature.DeleteThreatList();
    i_victimGuid = 0;
    i_creature.CombatStop(true);

    // Remove TargetedMovementGenerator from MotionMaster stack list, and add HomeMovementGenerator instead
    if( i_creature->top()->GetMovementGeneratorType() == TARGETED_MOTION_TYPE )
        i_creature->TargetedHome();
}
