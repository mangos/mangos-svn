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

#include "GuardAI.h"
#include "Errors.h"
#include "Creature.h"
#include "Player.h"
#include "TargetedMovementGenerator.h"
#include "Database/DBCStores.h"
#include "ObjectAccessor.h"

int GuardAI::Permissible(const Creature *creature)
{
    if( creature->isGuard())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

GuardAI::GuardAI(Creature &c) : i_creature(c), i_victimGuid(0), i_state(STATE_NORMAL), i_tracker(TIME_INTERVAL_LOOK)
{
}

void GuardAI::MoveInLineOfSight(Unit *u)
{
    if( i_creature.getVictim() == NULL && u->isTargetableForAttack() && IsVisible(u) && u->isInAccessablePlaceFor(&i_creature))
    {
        float attackRadius = i_creature.GetAttackDistance(u);
        if(i_creature.IsWithinDist(u,attackRadius) && i_creature.GetDistanceZ(u) <= CREATURE_Z_ATTACK_RANGE)
        {
            //Need add code to let guard suport player
            if( i_creature.IsHostileTo(u) || u->IsHostileToAll() )
            {
                AttackStart(u);
                if(u->HasStealthAura())
                    u->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
            }
        }
    }
}

void GuardAI::HealBy(Unit *healer, uint32 amount_healed)
{
}

void GuardAI::DamageInflict(Unit *healer, uint32 amount_healed)
{
}

bool GuardAI::_needToStop() const
{
    if( !i_creature.isAlive() || !i_creature.getVictim() || !i_creature.getVictim()->isTargetableForAttack() )
        return true;

    if(!i_creature.getVictim()->isInAccessablePlaceFor(&i_creature))
        return true;

    float rx,ry,rz;
    i_creature.GetRespawnCoord(rx, ry, rz);
    float spawndist=i_creature.GetDistanceSq(rx,ry,rz);
    float length = i_creature.GetDistanceSq(i_creature.getVictim());
    float hostillen=i_creature.GetHostilityDistance( i_creature.getVictim()->GetGUID());
    return (( length > (10.0f + hostillen) * (10.0f + hostillen) && spawndist > 6400.0f )
        || ( length > (20.0f + hostillen) * (20.0f + hostillen) && spawndist > 2500.0f )
        || ( length > (30.0f + hostillen) * (30.0f + hostillen) ));
}

void GuardAI::AttackStop(Unit *)
{
}

void GuardAI::_stopAttack()
{
    if( !i_creature.isAlive() )
    {
        DEBUG_LOG("Creature stopped attacking because he's dead [guid=%u]", i_creature.GetGUIDLow());
        i_creature.StopMoving();
        i_creature->Idle();

        i_state = STATE_NORMAL;

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
    else if( !victim ->isAlive() )
    {
        DEBUG_LOG("Creature stopped attacking because victim is dead [guid=%u]", i_creature.GetGUIDLow());
    }
    else if( victim ->HasStealthAura() )
    {
        DEBUG_LOG("Creature stopped attacking because victim is using stealth [guid=%u]", i_creature.GetGUIDLow());
    }
    else if( victim ->isInFlight() )
    {
        DEBUG_LOG("Creature stopped attacking because victim is flying away [guid=%u]", i_creature.GetGUIDLow());
    }
    else
    {
        DEBUG_LOG("Creature stopped attacking because victim outran him [guid=%u]", i_creature.GetGUIDLow());
    }

    i_state = STATE_NORMAL;

    i_victimGuid = 0;
    i_creature.AttackStop();

    // Remove TargetedMovementGenerator from MotionMaster stack list, and add HomeMovementGenerator instead
    if( i_creature->top()->GetMovementGeneratorType() == TARGETED_MOTION_TYPE )
        i_creature->TargetedHome();
}

void GuardAI::UpdateAI(const uint32 diff)
{
    // update i_victimGuid if i_creature.getVictim() !=0 and changed
    if(i_creature.getVictim())
        i_victimGuid = i_creature.getVictim()->GetGUID();

    // i_creature.getVictim() can't be used for check in case stop fighting, i_creature.getVictim() clearóâ at Unit death etc.
    if( i_victimGuid )
    {
        if( _needToStop() )
        {
            DEBUG_LOG("Guard AI stoped attacking [guid=%u]", i_creature.GetGUIDLow());
            _stopAttack();                                  // i_victimGuid == 0 && i_creature.getVictim() == NULL now
            return;
        }

        assert((i_victimGuid != 0) == (i_creature.getVictim() != NULL) && "i_victimGuid and i_creature.getVictim() not synchronized.");

        switch( i_state )
        {
            case STATE_LOOK_AT_VICTIM:
            {
                if( i_creature.getVictim() && IsVisible(i_creature.getVictim()) )
                {
                    DEBUG_LOG("Victim %u re-enters creature's aggro radius fater stop attacking", i_creature.getVictim()->GetGUIDLow());
                    i_state = STATE_NORMAL;
                    i_creature->MovementExpired();
                    break;                                  // move on
                    // back to the cat and mice game if you move back in range
                }

                i_tracker.Update(diff);
                if( i_tracker.Passed() )
                {
                    DEBUG_LOG("Creature running back home [guid=%u]", i_creature.GetGUIDLow());
                    i_state = STATE_NORMAL;
                    i_creature->TargetedHome();
                }
                /*else if( !i_creature.canReachWithAttack( i_pVictim ))
                {

                    float dx = i_pVictim->GetPositionX() - i_creature.GetPositionX();
                    float dy = i_pVictim->GetPositionY() - i_creature.GetPositionY();
                    float orientation = (float)atan2((double)dy, (double)dx);
                    i_creature.Relocate(i_pVictim->GetPositionX(), i_pVictim->GetPositionY(), i_pVictim->GetPositionZ(), orientation);
                }*/

                break;
            }
            case STATE_NORMAL:
            {
                if( i_creature.IsWithinDist(i_creature.getVictim(), ATTACK_DIST))
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

                        if( _needToStop() )
                            _stopAttack();
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    else
    {
        std::list<Unit*> unitlist;
        MapManager::Instance().GetMap(i_creature.GetMapId())->GetUnitList(i_creature.GetPositionX(), i_creature.GetPositionY(),unitlist);
        for(std::list<Unit*>::iterator iter=unitlist.begin();iter!=unitlist.end();iter++)
        {
            if((*iter) && (*iter)->isAlive() && !(*iter)->isInFlight() && IsVisible( *iter ) )
            {
                MoveInLineOfSight(*iter);
            }
        }
    }
}

bool GuardAI::IsVisible(Unit *pl) const
{
    return i_creature.GetDistanceSq(pl) < sWorld.getConfig(CONFIG_SIGHT_GUARDER)
        && pl->isVisibleFor(&i_creature,true);
}

void GuardAI::AttackStart(Unit *u)
{
    if( i_creature.getVictim() || !u )
        return;
    //    DEBUG_LOG("Creature %s tagged a victim to kill [guid=%u]", i_creature.GetName(), u->GetGUIDLow());
    if(i_creature.Attack(u))
    {
        i_victimGuid = u->GetGUID();
        i_creature->Mutate(new TargetedMovementGenerator(*u));
    }
}
