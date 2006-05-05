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
#include "ReactorAI.h"
#include "Errors.h"
#include "Creature.h"
#include "TargetedMovementGenerator.h"
#include "FactionTemplateResolver.h"
#include "Log.h"

#define REACTOR_VISIBLE_RANGE (26.46f)

int
ReactorAI::Permissible(const Creature *creature)
{
    FactionTemplateEntry *fact = sFactionTemplateStore.LookupEntry(creature->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    FactionTemplateResolver fact_source(fact);
    if( fact_source.IsNeutralToAll() )
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
    if( i_pVictim == NULL )
    {
        DEBUG_LOG("Tag unit LowGUID(%u) HighGUID(%u) as a victim", p->GetGUIDLow(), p->GetGUIDHigh());
        i_creature.addUnitState(UNIT_STAT_ATTACKING);
        i_creature.SetFlag(UNIT_FIELD_FLAGS, 0x80000);
        i_creature->Mutate(new TargetedMovementGenerator(*p));
        i_pVictim = p;
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

    if( i_pVictim != NULL )
    {
        if( needToStop() )
        {
            DEBUG_LOG("Creature %u stopped attacking.", i_creature.GetGUIDLow());
            stopAttack();
        }
        else if( i_creature.IsStopped() )
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
                i_creature.AttackerStateUpdate(i_pVictim, 0);
                i_creature.setAttackTimer(0);

                if( !i_creature.isAlive() || !i_pVictim->isAlive() )
                    stopAttack();
            }
        }
    }
}

bool
ReactorAI::needToStop() const
{
    if( !i_pVictim->isAlive() || !i_creature.isAlive()  || i_pVictim->m_stealth)
        return true;

    float rx,ry,rz;
    i_creature.GetRespawnCoord(rx, ry, rz);
    float spawndist=i_creature.GetDistanceSq(rx,ry,rz);
    float length = i_creature.GetDistanceSq(i_pVictim);
    float hostillen=i_creature.GetHostility( i_pVictim->GetGUID())/(2.5f * i_creature.getLevel()+1.0f);
    return (( length > (10.0f + hostillen) * (10.0f + hostillen) && spawndist > 6400.0f )
        || ( length > (20.0f + hostillen) * (20.0f + hostillen) && spawndist > 2500.0f )
        || ( length > (30.0f + hostillen) * (30.0f + hostillen) ));
}

void
ReactorAI::stopAttack()
{
    if( i_pVictim != NULL )
    {
        i_creature.clearUnitState(UNIT_STAT_IN_COMBAT);
        i_creature.RemoveFlag(UNIT_FIELD_FLAGS, 0x80000 );

        if( !i_creature.isAlive() )
        {
            DEBUG_LOG("Creature stoped attacking cuz his dead [guid=%u]", i_creature.GetGUIDLow());
            i_creature->Idle();
        }
        else if( i_pVictim->m_stealth )
        {
            DEBUG_LOG("Creature stopped attacking cuz his victim is stealth [guid=%u]", i_creature.GetGUIDLow());
            i_pVictim = NULL;
            static_cast<TargetedMovementGenerator *>(i_creature->top())->TargetedHome(i_creature);
        }
        else
        {
            DEBUG_LOG("Creature stopped attacking due to target %s [guid=%u]", i_pVictim->isAlive() ? "out run him" : "is dead", i_creature.GetGUIDLow());
            static_cast<TargetedMovementGenerator *>(i_creature->top())->TargetedHome(i_creature);
        }

        i_pVictim = NULL;
    }
}
