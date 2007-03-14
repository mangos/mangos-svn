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

#include "TemporarySummon.h"
#include "WorldPacket.h"
#include "MapManager.h"
#include "Database/DBCStores.h"
#include "Log.h"

TemporarySummon::TemporarySummon() :
m_type(TEMPSUMMON_REMOVE_DEAD), m_timer(0), m_lifetime(0)
{
}

void TemporarySummon::Update( uint32 diff )
{
    if(isAlive())
    {
        if (!isInCombat())
        {
            if(m_timer <= diff )
                UnSummon();
            else
                m_timer -= diff;
        } else
        m_timer = m_lifetime;
    }

    Creature::Update( diff );
}

void TemporarySummon::Summon(TempSummonType type, uint32 lifetime)
{
    m_type = type;
    m_timer = lifetime;
    m_lifetime = lifetime;

    AddToWorld();
    MapManager::Instance().GetMap(GetMapId())->Add((Creature*)this);

    AIM_Initialize();
}

void TemporarySummon::UnSummon()
{
    CombatStop(true);

    CleanupCrossRefsBeforeDelete();
    ObjectAccessor::Instance().AddObjectToRemoveList(this);
}

void TemporarySummon::setDeathState(DeathState s)
{
    Creature::setDeathState( s );

    if( ( m_deathState == DEAD   && m_type == TEMPSUMMON_REMOVE_DEAD ) ||
        ( m_deathState == CORPSE && m_type == TEMPSUMMON_REMOVE_CORPSE ) )
    {
        CleanupCrossRefsBeforeDelete();
        ObjectAccessor::Instance().AddObjectToRemoveList(this);
    }
}

void TemporarySummon::SaveToDB()
{
}
