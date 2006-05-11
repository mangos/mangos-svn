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

#include "DBCStores.h"
//#include "DataStore.h"
#include "Policies/SingletonImp.h"

#include "DBCfmt.cpp"

DBCStorage <AreaTableEntry> sAreaStore(AreaTableEntryfmt);

DBCStorage <TalentEntry> sTalentStore(TalentEntryfmt);
DBCStorage <SpellRadius> sSpellRadius(SpellRadiusfmt);

DBCStorage <FactionTemplateEntry> sFactionTemplateStore(FactionTemplateEntryfmt);

DBCStorage <SpellRange> sSpellRange(SpellRangefmt);
DBCStorage <emoteentry> sEmoteStore(emoteentryfmt);

DBCStorage <SpellEntry> sSpellStore(SpellEntryfmt);
DBCStorage <SpellCastTime> sCastTime(SpellCastTimefmt);
DBCStorage <SpellDuration> sSpellDuration(SpellDurationfmt);
DBCStorage <FactionEntry> sFactionStore(FactionEntryfmt);

DBCStorage <ItemSetEntry> sItemSetStore(ItemSetEntryfmt);
DBCStorage <ItemDisplayTemplateEntry> sItemDisplayTemplateStore(ItemDisplayTemplateEntryfmt);

DBCStorage <SkillLineAbility> sSkillLineAbilityStore(SkillLineAbilityfmt);
DBCStorage <SpellItemEnchantment> sSpellItemEnchantmentStore(SpellItemEnchantmentfmt);

float GetRadius(SpellRadius *radius)
{
    if(radius)
        return radius->Radius;
    else
        return 0;
}

uint32 GetCastTime(SpellCastTime *time)
{
    if(time)
        return time->CastTime;
    else
        return 0;
}

float GetMaxRange(SpellRange *range)
{
    if(range)
        return range->maxRange;
    else
        return 0;
}

float GetMinRange(SpellRange *range)
{
    if(range)
        return range->minRange;
    else
        return 0;
}

int32 GetDuration(SpellEntry *spellInfo, uint8 effindex)
{
    if(!spellInfo || effindex >= 3)
        return 0;
    SpellDuration *du = sSpellDuration.LookupEntry(spellInfo->DurationIndex);
    if(!du)
        return 0;
    return (du->Duration[effindex] == -1) ? -1 : abs(du->Duration[effindex]);
}

int32 GetMaxDuration(SpellEntry *spellInfo)
{
    if(!spellInfo)
        return 0;
    SpellDuration *du = sSpellDuration.LookupEntry(spellInfo->DurationIndex);
    if(!du)
        return 0;
    int32 maxdu = -2147483646;
    for(uint8 i = 0; i < 3; i++)
    {
        if(du->Duration[i] > maxdu)
            maxdu = du->Duration[i];
    }
    return maxdu;
}
