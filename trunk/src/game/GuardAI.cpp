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
    if( ((Creature*)&creature)->isGuard())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

GuardAI::GuardAI(Creature &c) : i_creature(c), i_pVictim(NULL), i_myFaction(sFactionTemplateStore.LookupEntry(c.GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE))), i_state(STATE_NORMAL), i_tracker(TIME_INTERVAL_LOOK)
{
}

void GuardAI::MoveInLineOfSight(Unit *u)
{
    if( i_pVictim == NULL )
    {
        FactionTemplateEntry *your_faction = sFactionTemplateStore.LookupEntry(u->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
        //Need add code to let guard suport player
        if(( i_myFaction->hostile & 1 )>0 || (i_myFaction->hostile & your_faction->friendly)>0 || (your_faction->hostile & 1)>0 )
            _taggedToKill(u);
    }
}

void GuardAI::AttackStart(Unit *u)
{
    if( i_pVictim == NULL )
        _taggedToKill(u);
}

void GuardAI::AttackStop(Unit *)
{

}

void GuardAI::HealBy(Unit *healer, uint32 amount_healed)
{
}

void GuardAI::DamageInflict(Unit *healer, uint32 amount_healed)
{
}

bool GuardAI::IsVisible(Unit *pl) const
{
    return _isVisible(pl);
}

bool GuardAI::_needToStop() const
{
    if( !i_pVictim->isAlive() || !i_creature.isAlive() || i_pVictim->m_stealth)
        return true;

    float rx,ry,rz;
    i_creature.GetRespawnCoord(rx, ry, rz);
    float spawndist=i_creature.GetDistance(rx,ry,rz);
    float length = i_creature.GetDistance(i_pVictim);
    float hostillen=i_creature.GetHostility( i_pVictim->GetGUID())/(3.0f * i_creature.getLevel()+1.0f);
    return (( length > 14.0f + hostillen && spawndist > 70.0f ) ||
        ( length > 24.0f + hostillen && spawndist > 40.0f ) || ( length > 34.0f + hostillen ));
}

void GuardAI::_stopAttack()
{
    assert( i_pVictim != NULL );
    i_creature.ClearState(UNIT_STAT_ATTACKING);
    i_creature.RemoveFlag(UNIT_FIELD_FLAGS, 0x80000 );

    if( !i_creature.isAlive() )
    {
        DEBUG_LOG("Creature stoped attacking cuz his dead [guid=%d]", i_creature.GetGUIDLow());
        i_creature.StopMoving();
        i_creature->Idle();
    }
    else if( !i_pVictim->isAlive() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is dead [guid=%d]", i_creature.GetGUIDLow());
        static_cast<TargetedMovementGenerator *>(i_creature->top())->TargetedHome(i_creature);
    }
    else if( i_pVictim->m_stealth )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is stealth [guid=%d]", i_creature.GetGUIDLow());
        static_cast<TargetedMovementGenerator *>(i_creature->top())->TargetedHome(i_creature);
    }
    else
    {
        DEBUG_LOG("Creature stopped attacking due to target out run him [guid=%d]", i_creature.GetGUIDLow());
        static_cast<TargetedMovementGenerator *>(i_creature->top())->TargetedHome(i_creature);
    }
    i_state = STATE_NORMAL;
    i_pVictim = NULL;
}

void GuardAI::UpdateAI(const uint32 diff)
{
    if( i_pVictim != NULL )
    {
        switch( i_state )
        {
            case STATE_LOOK_AT_VICTIM:
            {
                if( _isVisible(i_pVictim) )
                {
                    DEBUG_LOG("Victim %d re-enters creature's aggro radius fater stop attacking", i_pVictim->GetGUIDLow());
                    i_state = STATE_NORMAL;
                    i_creature->MovementExpired();
                    break;                                  // move on
                    // back to the cat and mice game if you move back in range
                }

                i_tracker.Update(diff);
                if( i_tracker.Passed() )
                {
                    i_creature->MovementExpired();
                    DEBUG_LOG("Creature running back home [guid=%d]", i_creature.GetGUIDLow());
                    static_cast<TargetedMovementGenerator *>(i_creature->top())->TargetedHome(i_creature);
                    i_state = STATE_NORMAL;
                    i_pVictim = NULL;
                }
                else if( !i_creature.canReachWithAttack( i_pVictim ))
                {

                    float dx = i_pVictim->GetPositionX() - i_creature.GetPositionX();
                    float dy = i_pVictim->GetPositionY() - i_creature.GetPositionY();
                    float orientation = (float)atan2((double)dy, (double)dx);
                    i_creature.Relocate(i_creature.GetPositionX(), i_creature.GetPositionY(), i_creature.GetPositionZ(), orientation);
                }

                break;
            }
            case STATE_NORMAL:
            {
                if( _needToStop() )
                {
                    DEBUG_LOG("Guard AI stoped attacking [guid=%d]", i_creature.GetGUIDLow());
                    _stopAttack();
                }
                else if( i_creature.IsStopped() )
                {
                    if( i_creature.isAttackReady() )
                    {
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
            if((*iter) && (*iter)->isAlive() && !(*iter)->testStateFlag(UNIT_STAT_IN_FLIGHT) && IsVisible( *iter ) )
            {
                MoveInLineOfSight(*iter);
            }
        }
    }
}

bool GuardAI::_isVisible(Unit *u) const
{
    return ( ((Creature*)&i_creature)->GetDistance(u) * 1.0<= IN_LINE_OF_SIGHT && !u->m_stealth );
}

void GuardAI::_taggedToKill(Unit *u)
{
    assert( i_pVictim == NULL );
    //    DEBUG_LOG("Creature %s tagged a victim to kill [guid=%d]", i_creature.GetName(), u->GetGUIDLow());
    i_creature.SetState(UNIT_STAT_ATTACKING);
    i_creature.SetFlag(UNIT_FIELD_FLAGS, 0x80000);
    i_creature->Mutate(new TargetedMovementGenerator(*u));
    i_pVictim = u;
}
