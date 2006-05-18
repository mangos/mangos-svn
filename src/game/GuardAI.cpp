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

#include "GuardAI.h"
#include "Errors.h"
#include "Creature.h"
#include "Player.h"
#include "FactionTemplateResolver.h"
#include "TargetedMovementGenerator.h"
#include "Database/DBCStores.h"

int GuardAI::Permissible(const Creature *creature)
{
    if( creature->isGuard())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

GuardAI::GuardAI(Creature &c) : i_creature(c), i_pVictim(NULL), i_myFaction(sFactionTemplateStore.LookupEntry(c.GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE))), i_state(STATE_NORMAL), i_tracker(TIME_INTERVAL_LOOK)
{
}

void GuardAI::MoveInLineOfSight(Unit *u)
{
    if( i_pVictim == NULL && !u->m_stealth && u->isAlive())
    {
        FactionTemplateEntry *your_faction = sFactionTemplateStore.LookupEntry(u->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
        //Need add code to let guard suport player
        if(( i_myFaction->hostile & 1 )>0 || (i_myFaction->hostile & your_faction->friendly)>0 || (your_faction->hostile & 1)>0 )
            AttackStart(u);
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
    if( !i_pVictim->isAlive() || !i_creature.isAlive() || i_pVictim->m_stealth)
        return true;

    float rx,ry,rz;
    i_creature.GetRespawnCoord(rx, ry, rz);
    float spawndist=i_creature.GetDistanceSq(rx,ry,rz);
    float length = i_creature.GetDistanceSq(i_pVictim);
    float hostillen=i_creature.GetHostility( i_pVictim->GetGUID())/(3.0f * i_creature.getLevel()+1.0f);
    return (( length > (10.0f + hostillen) * (10.0f + hostillen) && spawndist > 6400.0f )
        || ( length > (20.0f + hostillen) * (20.0f + hostillen) && spawndist > 2500.0f )
        || ( length > (30.0f + hostillen) * (30.0f + hostillen) ));
}

void GuardAI::AttackStop(Unit *)
{
}

void GuardAI::_stopAttack()
{
    assert( i_pVictim != NULL );
    i_creature.clearUnitState(UNIT_STAT_ATTACKING);
    i_creature.RemoveFlag(UNIT_FIELD_FLAGS, 0x80000 );

    if( !i_creature.isAlive() )
    {
        DEBUG_LOG("Creature stoped attacking cuz his dead [guid=%u]", i_creature.GetGUIDLow());
        i_creature.StopMoving();
        i_creature->Idle();
    }
    else if( !i_pVictim->isAlive() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is dead [guid=%u]", i_creature.GetGUIDLow());
        static_cast<TargetedMovementGenerator *>(i_creature->top())->TargetedHome(i_creature);
    }
    else if( i_pVictim->m_stealth )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is stealth [guid=%u]", i_creature.GetGUIDLow());
        static_cast<TargetedMovementGenerator *>(i_creature->top())->TargetedHome(i_creature);
    }
    else
    {
        DEBUG_LOG("Creature stopped attacking due to target out run him [guid=%u]", i_creature.GetGUIDLow());
        static_cast<TargetedMovementGenerator *>(i_creature->top())->TargetedHome(i_creature);
    }
    i_state = STATE_NORMAL;
    i_pVictim = NULL;
}

void GuardAI::UpdateAI(const uint32 diff)
{
    if( i_pVictim != NULL )
    {
        if( _needToStop() )
        {
            DEBUG_LOG("Guard AI stoped attacking [guid=%u]", i_creature.GetGUIDLow());
            _stopAttack();
        }
        switch( i_state )
        {
            case STATE_LOOK_AT_VICTIM:
            {
                if( IsVisible(i_pVictim) )
                {
                    DEBUG_LOG("Victim %u re-enters creature's aggro radius fater stop attacking", i_pVictim->GetGUIDLow());
                    i_state = STATE_NORMAL;
                    i_creature->MovementExpired();
                    break;                                  // move on
                    // back to the cat and mice game if you move back in range
                }

                i_tracker.Update(diff);
                if( i_tracker.Passed() )
                {
                    i_creature->MovementExpired();
                    DEBUG_LOG("Creature running back home [guid=%u]", i_creature.GetGUIDLow());
                    static_cast<TargetedMovementGenerator *>(i_creature->top())->TargetedHome(i_creature);
                    i_state = STATE_NORMAL;
                    i_pVictim = NULL;
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
                if( i_creature.IsStopped() )
                {
                    if( i_creature.isAttackReady() )
                    {
                        std::list<Hostil*> hostillist = i_creature.GetHostilList();
                        if(hostillist.size())
                        {
                            hostillist.sort();
                            hostillist.reverse();
                            uint64 guid;
                            if((guid = (*hostillist.begin())->UnitGuid) != i_pVictim->GetGUID())
                            {
                                Unit* newtarget = ObjectAccessor::Instance().GetUnit(i_creature, guid);
                                if(newtarget)
                                {
                                    i_pVictim = NULL;
                                    AttackStart(newtarget);
                                }
                            }
                        }
                        if(!i_creature.canReachWithAttack(i_pVictim))
                            return;
                        i_creature.AttackerStateUpdate(i_pVictim, 0);
                        i_creature.setAttackTimer(0);

                        if( !i_creature.isAlive() || !i_pVictim->isAlive() )
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
            if((*iter) && (*iter)->isAlive() && !(*iter)->hasUnitState(UNIT_STAT_IN_FLIGHT) && IsVisible( *iter ) )
            {
                MoveInLineOfSight(*iter);
            }
        }
    }
}

bool GuardAI::IsVisible(Unit *pl) const
{
    return ( ((Creature*)&i_creature)->GetDistanceSq(pl) * 1.0 <= sWorld.getConfig(CONFIG_SIGHT_GUARDER) && !pl->m_stealth && pl->isAlive() );
}

void GuardAI::AttackStart(Unit *u)
{
    if( i_pVictim || !u )
        return;
    //    DEBUG_LOG("Creature %s tagged a victim to kill [guid=%u]", i_creature.GetName(), u->GetGUIDLow());
    i_creature.addUnitState(UNIT_STAT_ATTACKING);
    i_creature.SetFlag(UNIT_FIELD_FLAGS, 0x80000);
    i_creature->Mutate(new TargetedMovementGenerator(*u));
    i_pVictim = u;
}
