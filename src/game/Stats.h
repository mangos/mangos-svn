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

#ifndef __STATS_H
#define __STATS_H

#include "Unit.h"
#include "UpdateMask.h"

inline uint32 CalculateXpToGive(Unit *pVictim, Unit *pAttacker)
{
    uint32 VictimLvl = pVictim->GetUInt32Value(UNIT_FIELD_LEVEL);
    uint32 AttackerLvl = pAttacker->GetUInt32Value(UNIT_FIELD_LEVEL);
    double xp = (VictimLvl*5+45)*(1+0.05*(VictimLvl-AttackerLvl));
    if( ( xp < 0 ) || ((VictimLvl<(AttackerLvl * 0.8)) && (AttackerLvl > 5)) )
        xp = 0;
    else
        xp *= sWorld.getRate(RATE_XP);

    return (uint32)xp;
}

inline uint32 CalculateDamage(const Unit *pAttacker)
{
    uint32 attack_power = pAttacker->GetUInt32Value(UNIT_FIELD_ATTACKPOWER);

    float min_damage = pAttacker->GetFloatValue(UNIT_FIELD_MINDAMAGE)+pAttacker->GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE);
    float max_damage = pAttacker->GetFloatValue(UNIT_FIELD_MAXDAMAGE)+pAttacker->GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE);

    if (min_damage > max_damage)
    {
        float temp = max_damage;
        max_damage = min_damage;
        min_damage = temp;
    }

    if(max_damage==0)
        max_damage=5;

    float diff = max_damage - min_damage + 1;
    float dmg = float (rand()%(uint32)diff + (uint32)min_damage);
    return (uint32)dmg;
}

inline bool isEven (int num)
{
    if ((num%2)==0)
    {
        return true;
    }
    return false;
}
#endif
