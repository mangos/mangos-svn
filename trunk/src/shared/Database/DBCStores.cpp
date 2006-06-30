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

DBCStorage <emoteentry> sEmoteStore(emoteentryfmt);

DBCStorage <FactionEntry> sFactionStore(FactionEntryfmt);
DBCStorage <FactionTemplateEntry> sFactionTemplateStore(FactionTemplateEntryfmt);

DBCStorage <ItemSetEntry> sItemSetStore(ItemSetEntryfmt);
DBCStorage <ItemDisplayTemplateEntry> sItemDisplayTemplateStore(ItemDisplayTemplateEntryfmt);

DBCStorage <SkillLineAbility> sSkillLineAbilityStore(SkillLineAbilityfmt);

DBCStorage <SpellItemEnchantment> sSpellItemEnchantmentStore(SpellItemEnchantmentfmt);
DBCStorage <SpellEntry> sSpellStore(SpellEntryfmt);
DBCStorage <SpellCastTime> sCastTime(SpellCastTimefmt);
DBCStorage <SpellDuration> sSpellDuration(SpellDurationfmt);
DBCStorage <SpellFocusObject> sSpellFocusObject(SpellFocusObjectfmt);
DBCStorage <SpellRadius> sSpellRadius(SpellRadiusfmt);
DBCStorage <SpellRange> sSpellRange(SpellRangefmt);

DBCStorage <TalentEntry> sTalentStore(TalentEntryfmt);






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

int32 GetDuration(SpellEntry *spellInfo)
{
    if(!spellInfo)
        return 0;
    SpellDuration *du = sSpellDuration.LookupEntry(spellInfo->DurationIndex);
    if(!du)
        return 0;
    return (du->Duration[0] == -1) ? -1 : abs(du->Duration[0]);
}

bool IsRankSpellDueToSpell(SpellEntry *spellInfo_1,uint32 spellId_2)
{
    SpellEntry *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2 || spellInfo_1->Id == spellId_2)
        return false;
    //not sure the code below can separate all spell which is not the rank spell to another.
    if(spellInfo_1->SpellIconID != spellInfo_2->SpellIconID
        || spellInfo_1->SpellVisual != spellInfo_2->SpellVisual
        || spellInfo_1->Category != spellInfo_2->Category
        || spellInfo_1->Attributes != spellInfo_2->Attributes
        || spellInfo_1->AttributesEx !=spellInfo_2->AttributesEx
        || spellInfo_1->EffectApplyAuraName[0] != spellInfo_2->EffectApplyAuraName[0]
        || spellInfo_1->EffectApplyAuraName[1] != spellInfo_2->EffectApplyAuraName[1]
        || spellInfo_1->EffectApplyAuraName[2] != spellInfo_2->EffectApplyAuraName[2])
        return false;
    return true;
}