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

#include "AggressorAI.h"
#include "Errors.h"
#include "Creature.h"
#include "Player.h"
#include "TargetedMovementGenerator.h"
#include "Database/DBCStores.h"
#include "ObjectAccessor.h"
#include "VMapFactory.h"
#include "World.h"

#include <list>

int
AggressorAI::Permissible(const Creature *creature)
{
    // have some hostile factions, it will be selected by IsHostileTo check at MoveInLineOfSight
    if( !creature->IsNeutralToAll() && !creature->isCivilian() )
        return PERMIT_BASE_PROACTIVE;

    return PERMIT_BASE_NO;
}

AggressorAI::AggressorAI(Creature &c) : i_creature(c), i_victimGuid(0), i_state(STATE_NORMAL), i_tracker(TIME_INTERVAL_LOOK)
{
}

void
AggressorAI::MoveInLineOfSight(Unit *u)
{
    if( !i_creature.getVictim() && !i_creature.hasUnitState(UNIT_STAT_STUNDED) && u->isTargetableForAttack() &&
        ( i_creature.IsHostileTo( u ) /*|| u->getVictim() && i_creature.IsFriendlyTo( u->getVictim() )*/ ) &&
        u->isInAccessablePlaceFor(&i_creature) )
    {
        float attackRadius = i_creature.GetAttackDistance(u);
        if(i_creature.IsWithinDistInMap(u, attackRadius) && i_creature.GetDistanceZ(u) <= CREATURE_Z_ATTACK_RANGE)
        {
            if(!i_creature.IsWithinLOSInMap(u)) return;
            AttackStart(u);
            if(u->HasStealthAura())
                u->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
        }
    }
}

bool
AggressorAI::_needToStop() const
{
    if( !i_creature.isAlive() || !i_creature.getVictim())
        return true;

    //if(!i_creature.getVictim()->isTargetableForAttack() || !i_creature.getVictim()->isInAccessablePlaceFor(&i_creature))
    //return true;
    //no need for this checks because mob changes its victim only when
    //1) victim is dead (check is in SelectHostilTarget() func)
    //2) victim is out of threat radius

    // chaise only at same map
    if(!i_creature.IsInMap(i_creature.getVictim()))
        return true;

    // instance not have threat radius for stop
    Map* map = MapManager::Instance().GetMap(i_creature.GetMapId(),&i_creature);
    if(map->Instanceable())
        return false;

    float rx,ry,rz;
    i_creature.GetRespawnCoord(rx, ry, rz);
    float length = i_creature.getVictim()->GetDistanceSq(rx,ry,rz);
    return ( length > CREATURE_THREAT_RADIUS );
}

void AggressorAI::_stopAttack()
{
    DEBUG_LOG("What we do HERE ?");
    return;
}

void AggressorAI::EnterEvadeMode()
{
    if( !i_creature.isAlive() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his dead [guid=%u]", i_creature.GetGUIDLow());
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
    else if( !victim->isAlive() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is dead [guid=%u]", i_creature.GetGUIDLow());
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
        DEBUG_LOG("Creature stopped attacking due to target out run him [guid=%u]", i_creature.GetGUIDLow());
        //i_state = STATE_LOOK_AT_VICTIM;
        //i_tracker.Reset(TIME_INTERVAL_LOOK);
    }

    if(!i_creature.isCharmed())
    {
        i_creature.RemoveAllAuras();

        // Remove TargetedMovementGenerator from MotionMaster stack list, and add HomeMovementGenerator instead
        if( i_creature->top()->GetMovementGeneratorType() == TARGETED_MOTION_TYPE )
            i_creature->TargetedHome();
    }

    i_creature.DeleteThreatList();
    i_victimGuid = 0;
    i_creature.CombatStop(true);

}

void
AggressorAI::UpdateAI(const uint32 diff)
{
    // update i_victimGuid if i_creature.getVictim() !=0 and changed
    if(!i_creature.SelectHostilTarget() || !i_creature.getVictim())
        return;

    i_victimGuid = i_creature.getVictim()->GetGUID();

    // i_creature.getVictim() can't be used for check in case stop fighting, i_creature.getVictim() clearóâ at Unit death etc.
    if( i_victimGuid )
    {
        if( _needToStop() )
        {
            DEBUG_LOG("Aggressor AI stoped attacking [guid=%u]", i_creature.GetGUIDLow());
            EnterEvadeMode();                               // i_victimGuid == 0 && i_creature.getVictim() == NULL now
            return;
        }

        assert((i_victimGuid != 0) == (i_creature.getVictim() != NULL) && "i_victimGuid and i_creature.getVictim() not synchronized.");

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
AggressorAI::IsVisible(Unit *pl) const
{
    return i_creature.GetDistanceSq(pl) < sWorld.getConfig(CONFIG_SIGHT_MONSTER)
        && pl->isVisibleForOrDetect(&i_creature,true);
}

void
AggressorAI::AttackStart(Unit *u)
{
    if( !u )
        return;

    if(i_creature.Attack(u))
    {
        i_creature.AddThreat(u, 0.0f);
        //    DEBUG_LOG("Creature %s tagged a victim to kill [guid=%u]", i_creature.GetName(), u->GetGUIDLow());
        i_victimGuid = u->GetGUID();

        i_creature.resetAttackTimer();
        i_creature->Mutate(new TargetedMovementGenerator(*u));
        if (u->GetTypeId() == TYPEID_PLAYER)
            i_creature.SetLootRecipient((Player*)u);
    }
}
