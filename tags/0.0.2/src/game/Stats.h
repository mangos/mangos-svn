/* Stats.h
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

/////////////////////////////////////////////////////////////////////////
//  CalculateXpToGive
//
//  Calculates XP given out by pVictim upon death.
//  XP=(MOB_LEVEL*5+45)*(1+0.05*(MOB_LEVEL-PLAYER_LEVEL))
//
//  old formula xp = (SUM(health,power1,power2,power3,power4) / 5) * (lvl_of_monster*2)
////////////////////////////////////////////////////////////////////////
inline uint32 CalculateXpToGive(Unit *pVictim, Unit *pAttacker)
{
    uint16 VictimLvl = pVictim->GetUInt32Value(UNIT_FIELD_LEVEL);
    uint16 AttackerLvl = pAttacker->GetUInt32Value(UNIT_FIELD_LEVEL);
    float xp = (VictimLvl*5+45)*(1+0.05*(VictimLvl-AttackerLvl));
    if( ( xp < 0 ) || ((VictimLvl<(AttackerLvl * 0.8)) && (AttackerLvl > 5)) )
        xp = 0;
    else
        xp *= sWorld.getRate(RATE_XP);

/*

        uint32 xp = 1;
        uint32 total =  pVictim->GetUInt32Value(UNIT_FIELD_MAXHEALTH) +
                        pVictim->GetUInt32Value(UNIT_FIELD_MAXPOWER1) +
                        pVictim->GetUInt32Value(UNIT_FIELD_MAXPOWER2) +
                        pVictim->GetUInt32Value(UNIT_FIELD_MAXPOWER3) +
                        pVictim->GetUInt32Value(UNIT_FIELD_MAXPOWER4);

        xp = total / 5;
        xp *= pVictim->GetUInt32Value(UNIT_FIELD_LEVEL)*2*sWorld.getRate(RATE_XP);
*/
/*
        // Maybe implement some modifier depending on difference of levels, but that might not be necessary
        // in theory a higher lvl mob will give higher xp thanks to having higher stats
        int lvl_diff_mod = (pVictim->GetUInt32Value(UNIT_FIELD_LEVEL) - pAttacker->GetUInt32Value(UNIT_FIELD_LEVEL)) / 3;
        // level difference multiplier
        if (lvl_diff_mod < 0)
        {
            // This monster is lower than the killer, reduce XP
            xp /= lvl_diff_mod;
        }
        else
        {
            // victim is higher level, increase XP
            xp *= lvl_diff_mod;
        }
*/
    return (uint32)xp;
}


// TODO: Some awesome formula to determine how much damage to deal
inline uint32 CalculateDamage(const Unit *pAttacker)
{
    uint32 attack_power = pAttacker->GetUInt32Value(UNIT_FIELD_ATTACKPOWER);

    /*
    if(pAttacker->GetTypeId() == TYPEID_PLAYER)
    {
       attack_power = pAttacker->GetUInt32Value(UNIT_FIELD_ATTACKPOWER);
       // attack_power += pAttacker->GetUInt32Value(PLAYER_FIELD_ATTACKPOWERMODPOS);
       // attack_power -= pAttacker->GetUInt32Value(PLAYER_FIELD_ATTACKPOWERMODNEG);
    }
*/

    float min_damage = pAttacker->GetFloatValue(UNIT_FIELD_MINDAMAGE)+pAttacker->GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE);
    float max_damage = pAttacker->GetFloatValue(UNIT_FIELD_MAXDAMAGE)+pAttacker->GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE);
    // Ehh, sometimes min is bigger than max!?!?
    if (min_damage > max_damage)
    {
        float temp = max_damage;
        max_damage = min_damage;
        min_damage = temp;
    }

    // Fix creatures that have no base attack damage.
    if(max_damage==0)
        max_damage=5;

    float diff = max_damage - min_damage + 1;
    float dmg = rand()%(uint32)diff + (uint32)min_damage;
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
