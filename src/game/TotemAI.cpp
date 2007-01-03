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

#include "TotemAI.h"
#include "Totem.h"
#include "Creature.h"
#include "Player.h"
#include "Database/DBCStores.h"
#include "MapManager.h"

int
TotemAI::Permissible(const Creature *creature)
{
    if( creature->isTotem() )
        return PERMIT_BASE_PROACTIVE;

    return PERMIT_BASE_NO;
}

TotemAI::TotemAI(Creature &c) : i_totem(c), i_victimGuid(0), i_tracker(TIME_INTERVAL_LOOK)
{
}

void
TotemAI::MoveInLineOfSight(Unit *u)
{
    // would be a faster solution but it doesnt work
    /*if (i_totem.isTotem() && ((Totem*)&i_totem)->GetTotemType() == TOTEM_ACTIVE)
    {
        if (ObjectAccessor::Instance().GetUnit(*(Unit*)&i_totem, i_victimGuid))
            return;
        i_victimGuid = 0;

        FactionTemplateResolver its_faction = u->getFactionTemplateEntry();
        if(i_myFaction.IsFriendlyTo(its_faction) || i_myFaction.IsNeutralToAll() || its_faction.IsNeutralToAll())
            return;

        SpellEntry *spellInfo = sSpellStore.LookupEntry(((Totem*)&i_totem)->GetSpell());
        if (!spellInfo) return;
        SpellRange* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
        float max_range = GetMaxRange(srange);

        if (i_totem.GetDistanceSq(u) < max_range * max_range)
            i_victimGuid = u->GetGUID();
    }*/
}

void
TotemAI::HealBy(Unit *healer, uint32 amount_healed)
{
}

void
TotemAI::DamageInflict(Unit *healer, uint32 amount_healed)
{
}

void TotemAI::AttackStop(Unit *)
{
}

void
TotemAI::UpdateAI(const uint32 diff)
{
    if (!i_totem.isTotem() || !i_totem.isAlive() || i_totem.m_currentSpell)
        return;
    if (((Totem*)&i_totem)->GetTotemType() != TOTEM_ACTIVE)
        return;

    SpellEntry const *spellInfo = sSpellStore.LookupEntry(((Totem*)&i_totem)->GetSpell());
    if (!spellInfo) return;
    SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
    float max_range = GetMaxRange(srange);

    Unit *victim = ObjectAccessor::Instance().GetUnit(*(Unit*)&i_totem, i_victimGuid);
    // stop attacking dead or out of range victims
    if (victim && victim->isAlive() && i_totem.IsWithinDist(victim, max_range))
    {
        // if totem is not casting and it has a victim .. cast again
        AttackStart(victim);
        return;
    }
    else
        i_victimGuid = 0;

    // look for a new victim in range
    std::list<Unit*> UnitList;
    MapManager::Instance().GetMap(i_totem.GetMapId())->GetUnitList(i_totem.GetPositionX(), i_totem.GetPositionY(),UnitList);
    for(std::list<Unit*>::iterator iter=UnitList.begin();iter!=UnitList.end();iter++)
    {
        if((*iter) && (*iter)->isTargetableForAttack()&& IsVisible(*iter))
        {
            if(i_totem.IsFriendlyTo(*iter) || i_totem.IsNeutralToAll() || (*iter)->IsNeutralToAll())
                continue;
            if(i_totem.IsWithinDist(*iter, max_range) )
            {
                i_victimGuid = (*iter)->GetGUID();
                AttackStart(*iter);
                break;
            }
        }
    }
}

bool
TotemAI::IsVisible(Unit *pl) const
{
    return !pl->HasInvisibilityAura() && !pl->HasStealthAura();
}

void
TotemAI::AttackStart(Unit *u)
{
    if (!i_totem.isTotem() || !i_totem.isAlive()) return;
    if (i_totem.m_currentSpell) return;
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(((Totem*)&i_totem)->GetSpell());
    if (GetDuration(spellInfo) != -1)
        i_totem.CastSpell(u, ((Totem*)&i_totem)->GetSpell(), false);
}
