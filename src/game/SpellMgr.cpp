/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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

#include "SpellMgr.h"
#include "ObjectMgr.h"
#include "SpellAuraDefines.h"
#include "ProgressBar.h"
#include "Database/DBCStores.h"
#include "World.h"
#include "Chat.h"

SpellMgr::SpellMgr()
{
}

SpellMgr::~SpellMgr()
{
}

SpellMgr& SpellMgr::Instance()
{
    static SpellMgr spellMgr;
    return spellMgr;
}

float GetRadius(SpellRadiusEntry const *radius)
{
    if(radius)
        return radius->Radius;
    else
        return 0;
}

uint32 GetCastTime(SpellCastTimesEntry const *time)
{
    if(time)
        return time->CastTime;
    else
        return 0;
}

float GetMaxRange(SpellRangeEntry const *range)
{
    if(range)
        return range->maxRange;
    else
        return 0;
}

float GetMinRange(SpellRangeEntry const *range)
{
    if(range)
        return range->minRange;
    else
        return 0;
}

int32 GetDuration(SpellEntry const *spellInfo)
{
    if(!spellInfo)
        return 0;
    SpellDurationEntry const *du = sSpellDurationStore.LookupEntry(spellInfo->DurationIndex);
    if(!du)
        return 0;
    return (du->Duration[0] == -1) ? -1 : abs(du->Duration[0]);
}

int32 GetMaxDuration(SpellEntry const *spellInfo)
{
    if(!spellInfo)
        return 0;
    SpellDurationEntry const *du = sSpellDurationStore.LookupEntry(spellInfo->DurationIndex);
    if(!du)
        return 0;
    return (du->Duration[2] == -1) ? -1 : abs(du->Duration[2]);
}

bool IsPassiveSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo)
        return false;
    return (spellInfo->Attributes & (1<<6)) != 0;
}

bool IsNonCombatSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo)
        return false;
    return (spellInfo->Attributes & (1<<28)) != 0;
}

bool IsNoStackAuraDueToAura(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2)
{
    SpellEntry const *spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    if (spellInfo_1->Effect[effIndex_1] != spellInfo_2->Effect[effIndex_2] ||
        spellInfo_1->EffectItemType[effIndex_1] != spellInfo_2->EffectItemType[effIndex_2] ||
        spellInfo_1->EffectMiscValue[effIndex_1] != spellInfo_2->EffectMiscValue[effIndex_2] ||
        spellInfo_1->EffectApplyAuraName[effIndex_1] != spellInfo_2->EffectApplyAuraName[effIndex_2])
        return false;

    return true;
}

int32 CompareAuraRanks(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2)
{
    SpellEntry const*spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry const*spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return 0;
    if (spellId_1 == spellId_2) return 0;

    int32 diff = spellInfo_1->EffectBasePoints[effIndex_1] - spellInfo_2->EffectBasePoints[effIndex_2];
    if (spellInfo_1->EffectBasePoints[effIndex_1]+1 < 0 && spellInfo_2->EffectBasePoints[effIndex_2]+1 < 0) return -diff;
    else return diff;
}

bool IsSealSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);

    //Collection of all the seal family flags. No other paladin spell has any of those.
    return spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN &&
        ( spellInfo->SpellFamilyFlags & 0x4000A000200LL );
}

SpellSpecific GetSpellSpecific(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo) return SPELL_NORMAL;

    switch(spellInfo->SpellFamilyName)
    {
    case SPELLFAMILY_MAGE:
        {
            // family flags 18(Molten), 25(Frost/Ice), 28(Mage)
            if (spellInfo->SpellFamilyFlags & 0x12040000)
                return SPELL_MAGE_ARMOR;

            if ((spellInfo->SpellFamilyFlags & 0x1000000) && spellInfo->EffectApplyAuraName[0]==SPELL_AURA_MOD_CONFUSE)
                return SPELL_MAGE_POLYMORPH;

            break;
        }
    case SPELLFAMILY_WARRIOR:
        {
            if (spellInfo->SpellFamilyFlags & 0x00008000010000LL)
                return SPELL_POSITIVE_SHOUT;

            break;
        }
    case SPELLFAMILY_WARLOCK:
        {
            // only warlock curses have this
            if (spellInfo->Dispel == IMMUNE_DISPEL_CURSE)
                return SPELL_CURSE;

            // family flag 37 (only part spells have family name)
            if (spellInfo->SpellFamilyFlags & 0x2000000000LL)
                return SPELL_WARLOCK_ARMOR;

            break;
        }
    case SPELLFAMILY_HUNTER:
        {
            // only hunter stings have this
            if (spellInfo->Dispel == IMMUNE_DISPEL_POISON)
                return SPELL_STING;

            break;
        }
    case SPELLFAMILY_PALADIN:
        {
            if (IsSealSpell(spellId))
                return SPELL_SEAL;

            if (spellInfo->SpellFamilyFlags & 0x10000000)
                return SPELL_BLESSING;

            if ((spellInfo->SpellFamilyFlags & 0x00000820180400LL) && (spellInfo->AttributesEx3 & 0x200))
                return SPELL_JUDGEMENT;

            for (int i = 0; i < 3; i++)
            {
                // only paladin auras have this
                if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA)
                    return SPELL_AURA;
            }
            break;
        }
    case SPELLFAMILY_SHAMAN:
        {
            // family flags 10 (Lightning), 42 (Earth), 37 (Water)
            if (spellInfo->SpellFamilyFlags & 0x42000000400LL)
                return SPELL_ELEMENTAL_SHIELD;

            break;
        }
    }

    // only warlock armor/skin have this (in additional to family cases)
    if( spellInfo->SpellVisual == 130 && spellInfo->SpellIconID == 89)
    {
        return SPELL_WARLOCK_ARMOR;
    }

    // only hunter aspects have this (but not all aspects in hunter family)
    if( spellInfo->activeIconID == 122 && spellInfo->School == SPELL_SCHOOL_NATURE &&
        (spellInfo->Attributes & 0x50000) != 0 && (spellInfo->Attributes & 0x9000010) == 0)
    {
        return SPELL_ASPECT;
    }

    for(int i = 0; i < 3; i++)
        if( spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA && (
            spellInfo->EffectApplyAuraName[i] == SPELL_AURA_TRACK_CREATURES ||
            spellInfo->EffectApplyAuraName[i] == SPELL_AURA_TRACK_RESOURCES ||
            spellInfo->EffectApplyAuraName[i] == SPELL_AURA_TRACK_STEALTHED ) )
            return SPELL_TRACKER;

    return SPELL_NORMAL;
}

bool IsSpellSingleEffectPerCaster(uint32 spellId)
{
    switch(GetSpellSpecific(spellId))
    {
    case SPELL_SEAL:
    case SPELL_BLESSING:
    case SPELL_AURA:
    case SPELL_STING:
    case SPELL_CURSE:
    case SPELL_ASPECT:
    case SPELL_TRACKER:
    case SPELL_WARLOCK_ARMOR:
    case SPELL_MAGE_ARMOR:
    case SPELL_ELEMENTAL_SHIELD:
    case SPELL_MAGE_POLYMORPH:
    case SPELL_POSITIVE_SHOUT:
    case SPELL_JUDGEMENT:
        return true;
    default:
        return false;
    }
}

bool IsPositiveTarget(uint32 targetA, uint32 targetB)
{
    // non-positive targets
    switch(targetA)
    {
    case TARGET_CHAIN_DAMAGE:
    case TARGET_ALL_ENEMY_IN_AREA:
    case TARGET_ALL_ENEMY_IN_AREA_INSTANT:
    case TARGET_IN_FRONT_OF_CASTER:
    case TARGET_DUELVSPLAYER:
    case TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
    case TARGET_CURRENT_SELECTED_ENEMY:
        return false;
    case TARGET_ALL_AROUND_CASTER:
        return (targetB == TARGET_ALL_PARTY || targetB == TARGET_ALL_FRIENDLY_UNITS_AROUND_CASTER);
    default:
        break;
    }
    return true;
}

bool IsPositiveEffect(uint32 spellId, uint32 effIndex)
{
    SpellEntry const *spellproto = sSpellStore.LookupEntry(spellId);
    if (!spellproto) return false;

    switch(spellId)
    {
    case 23333:                                         // BG spell
    case 23335:                                         // BG spell
        return true;
    case 28441:                                         // not possitive dummy spell
        return false;
    }

    switch(spellproto->Effect[effIndex])
    {
        // always positive effects (check before target checks that provided non-positive result in some case for positive effects)
    case SPELL_EFFECT_LEARN_SPELL:
    case SPELL_EFFECT_SKILL_STEP:
        return true;

        // non-positive aura use
    case SPELL_EFFECT_APPLY_AURA:
    case SPELL_EFFECT_APPLY_AURA_NEW2:
        {
            switch(spellproto->EffectApplyAuraName[effIndex])
            {
            case SPELL_AURA_DUMMY:
                {
                    // dummy aura can be positive or negative dependent from casted spell
                    switch(spellproto->Id)
                    {
                    case 13139:                             // net-o-matic special effect
                        return false;
                    default:
                        break;
                    }
                }   break;
            case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                if(spellId != spellproto->EffectTriggerSpell[effIndex])
                {
                    uint32 spellTriggeredId = spellproto->EffectTriggerSpell[effIndex];
                    SpellEntry const *spellTriggeredProto = sSpellStore.LookupEntry(spellTriggeredId);

                    if(spellTriggeredProto)
                    {
                        // non-positive targets of main spell return early
                        for(int i = 0; i < 3; ++i)
                        {
                            // if non-positive trigger cast targeted to positive target this main cast is non-positive
                            // this will place this spell auras as debuffs
                            if(IsPositiveTarget(spellTriggeredProto->EffectImplicitTargetA[effIndex],spellTriggeredProto->EffectImplicitTargetB[effIndex]) && !IsPositiveEffect(spellTriggeredId,i))
                                return false;
                        }
                    }
                }
                break;
            case SPELL_AURA_PROC_TRIGGER_SPELL:
                // many positive auras have negative triggered spells at damage for example and this not make it negative (it can be canceled for example)
                break;
            case SPELL_AURA_MOD_ROOT:
            case SPELL_AURA_MOD_SILENCE:
            case SPELL_AURA_GHOST:
                return false;
            case SPELL_AURA_MOD_DECREASE_SPEED:      // used in positive spells also
                // part of positive spell if casted at self
                if(spellproto->EffectImplicitTargetA[effIndex] != TARGET_SELF)
                    return false;
                // but not this if this first effect (don't found batter check)
                if(spellproto->Attributes & 0x4000000 && effIndex==0)
                    return false;
                break;
            case SPELL_AURA_MECHANIC_IMMUNITY:
                {
                    // non-positive immunities
                    switch(spellproto->EffectMiscValue[effIndex])
                    {
                    case MECHANIC_BANDAGE:
                    case MECHANIC_SHIELD:
                    case MECHANIC_MOUNT:
                    case MECHANIC_INVULNERABILITY:
                        return false;
                    default:
                        break;
                    }
                }   break;
            default:
                break;
            }
            break;
        }
    default:
        break;
    }

    // non-positive targets
    if(!IsPositiveTarget(spellproto->EffectImplicitTargetA[effIndex],spellproto->EffectImplicitTargetB[effIndex]))
        return false;

    // AttributesEx check
    if(spellproto->AttributesEx & (1<<7))
        return false;

    // ok, positive
    return true;
}

bool IsPositiveSpell(uint32 spellId)
{
    SpellEntry const *spellproto = sSpellStore.LookupEntry(spellId);
    if (!spellproto) return false;

    // spells with atleast one negative effect are considered negative
    // some self-applied spells have negative effects but in self casting case negative check ignored.
    for (int i = 0; i < 3; i++)
        if (!IsPositiveEffect(spellId, i))
            return false;
    return true;
}

bool IsSingleTargetSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo)
        return false;

    // cheap shot is an exception
    if ( spellId == 1833 || spellId == 14902 )
        return false;

    // hunter's mark and similar
    if(spellInfo->SpellVisual == 3239)
        return true;

    // cannot be cast on another target while not cooled down anyway
    int32 duration = GetDuration(spellInfo);
    if ( duration >= 0 && duration < int32(GetRecoveryTime(spellInfo)))
        return false;

    // all other single target spells have if it has AttributesEx
    if ( spellInfo->AttributesEx & (1<<18) )
        return true;

    // other single target
    //Fear
    if ((spellInfo->SpellIconID == 98 && spellInfo->SpellVisual == 336)
        //Banish
        || (spellInfo->SpellIconID == 96 && spellInfo->SpellVisual == 1305) )
        return true;

    // spell with single target specific types
    switch(GetSpellSpecific(spellInfo->Id))
    {
        case SPELL_TRACKER:
        case SPELL_ELEMENTAL_SHIELD:
        case SPELL_MAGE_POLYMORPH:
        case SPELL_JUDGEMENT:
            return true;
    }

    // all other single target spells have if it has Attributes
    //if ( spellInfo->Attributes & (1<<30) ) return true;
    return false;
}

bool IsSingleTargetSpells(SpellEntry const *spellInfo1, SpellEntry const *spellInfo2)
{

    // similar spell
    // FIX ME: this is not very good check for this
    if( spellInfo1->Category       == spellInfo2->Category     &&
        spellInfo1->SpellIconID    == spellInfo2->SpellIconID  &&
        spellInfo1->SpellVisual    == spellInfo2->SpellVisual  &&
        spellInfo1->Attributes     == spellInfo2->Attributes   &&
        spellInfo1->AttributesEx   == spellInfo2->AttributesEx &&
        spellInfo1->AttributesExEx == spellInfo2->AttributesExEx )
        return true;

    // base at spell specific
    SpellSpecific spec1 = GetSpellSpecific(spellInfo1->Id);
    // spell with single target specific types
    switch(spec1)
    {
        case SPELL_TRACKER:
        case SPELL_ELEMENTAL_SHIELD:
        case SPELL_MAGE_POLYMORPH:
        case SPELL_JUDGEMENT:
            if(GetSpellSpecific(spellInfo2->Id) == spec1)
                return true;
    }

    return false;
}

bool CanUsedWhileStealthed(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if ( (spellInfo->AttributesEx & 32) == 32 || spellInfo->AttributesEx2 == 0x200000)
        return true;
    return false;
}

void SpellMgr::LoadSpellTeleports()
{
    mSpellTeleports.clear();                                // need for reload case

    uint32 count = 0;

    //                                            0    1            2                   3                   4                   5
    QueryResult *result = WorldDatabase.Query("SELECT `id`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation` FROM `spell_teleport`");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell teleport coordinates", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        count++;

        uint32 Spell_ID = fields[0].GetUInt32();

        SpellTeleport st;

        st.target_mapId       = fields[1].GetUInt32();
        st.target_X           = fields[2].GetFloat();
        st.target_Y           = fields[3].GetFloat();
        st.target_Z           = fields[4].GetFloat();
        st.target_Orientation = fields[5].GetFloat();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(Spell_ID);
        if(!spellInfo)
        {
            sLog.outErrorDb("Spell (ID:%u) listed in `spell_teleport` does not exist`.",Spell_ID);
            continue;
        }

        bool found = false;
        for(int i = 0; i < 3; ++i)
        {
            if( spellInfo->Effect[i]==SPELL_EFFECT_TELEPORT_UNITS )
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            sLog.outErrorDb("Spell (Id: %u) listed in `spell_teleport` does not have effect SPELL_EFFECT_TELEPORT_UNITS (5).",Spell_ID);
            continue;
        }

        MapEntry const* mapEntry = sMapStore.LookupEntry(st.target_mapId);
        if(!mapEntry)
        {
            sLog.outErrorDb("Spell (ID:%u) teleport target map (ID: %u) does not exist in `Map.dbc`.",Spell_ID,st.target_mapId);
            continue;
        }

        if(st.target_X==0 && st.target_Y==0 && st.target_Z==0)
        {
            sLog.outErrorDb("Spell (ID:%u) teleport target coordinates not provided.",Spell_ID);
            continue;
        }

        mSpellTeleports[Spell_ID] = st;

    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell teleport coordinates", count );
}

void SpellMgr::LoadSpellAffects()
{
    mSpellAffectMap.clear();                                // need for reload case

    uint32 count = 0;

    //                                            0        1          2         3            4          5         6             7                 8
    QueryResult *result = WorldDatabase.Query("SELECT `entry`,`effectId`,`SpellId`,`SchoolMask`,`Category`,`SkillID`,`SpellFamily`,`SpellFamilyMask`,`Charges` FROM `spell_affect`");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell affect definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 entry = fields[0].GetUInt16();
        uint8 effectId = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(entry);

        if (!spellInfo)
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` does not exist", entry);
            continue;
        }

        if (effectId >= 3)
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` have invalid effect index (%u)", entry,effectId);
            continue;
        }

        if( spellInfo->Effect[effectId] != SPELL_EFFECT_APPLY_AURA ||
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_FLAT_MODIFIER &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_PCT_MODIFIER  &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_TARGET_TRIGGER )
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` have not SPELL_AURA_ADD_FLAT_MODIFIER (%u) or SPELL_AURA_ADD_PCT_MODIFIER (%u) or SPELL_AURA_ADD_TARGET_TRIGGER (%u) for effect index (%u)", entry,SPELL_AURA_ADD_FLAT_MODIFIER,SPELL_AURA_ADD_PCT_MODIFIER,SPELL_AURA_ADD_TARGET_TRIGGER,effectId);
            continue;
        }

        SpellAffection sa;

        sa.SpellId = fields[2].GetUInt16();
        sa.SchoolMask = fields[3].GetUInt8();
        sa.Category = fields[4].GetUInt16();
        sa.SkillId = fields[5].GetUInt16();
        sa.SpellFamily = fields[6].GetUInt8();
        sa.SpellFamilyMask = fields[7].GetUInt64();
        sa.Charges = fields[8].GetUInt16();

        mSpellAffectMap.insert(SpellAffectMap::value_type((entry<<8) + effectId,sa));

        count++;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell affect definitions", count );
}

bool SpellMgr::IsAffectedBySpell(SpellEntry const *spellInfo, uint32 spellId, uint8 effectId, uint64 const& familyFlags) const
{
    if (!spellInfo)
        return false;

    SpellAffection const *spellAffect = GetSpellAffection(spellId,effectId);

    if (spellAffect )
    {
        if (spellAffect->SpellId && (spellAffect->SpellId == spellInfo->Id))
            return true;
        if (spellAffect->SchoolMask && (spellAffect->SchoolMask & (1 << spellInfo->School)))
            return true;
        if (spellAffect->Category && (spellAffect->Category == spellInfo->Category))
            return true;
        if (spellAffect->SkillId)
        {
            SkillLineAbilityEntry const *skillLineEntry = sSkillLineAbilityStore.LookupEntry(spellInfo->Id);
            if(skillLineEntry && skillLineEntry->skillId == spellAffect->SkillId)
                return true;
        }
        if (spellAffect->SpellFamily && spellAffect->SpellFamily == spellInfo->SpellFamilyName)
            return true;

        if (spellAffect->SpellFamilyMask && (spellAffect->SpellFamilyMask & spellInfo->SpellFamilyFlags))
            return true;
    }
    else if (familyFlags & spellInfo->SpellFamilyFlags)
        return true;

    return false;
}

void SpellMgr::LoadSpellProcEvents()
{
    mSpellProcEventMap.clear();                             // need for reload case

    uint32 count = 0;

    //                                            0       1            2          3         4          5      6                 7           8
    QueryResult *result = WorldDatabase.Query("SELECT `entry`,`SchoolMask`,`Category`,`SkillID`,`SpellFamilyName`,`SpellFamilyMask`,`procFlags`,`ppmRate` FROM `spell_proc_event`");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell proc event conditions", count  );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 entry = fields[0].GetUInt16();

        if (!sSpellStore.LookupEntry(entry))
        {
            sLog.outErrorDb("Spell %u listed in `spell_proc_event` does not exist", entry);
            continue;
        }

        SpellProcEventEntry spe;

        spe.schoolMask      = fields[1].GetUInt32();
        spe.category        = fields[2].GetUInt32();
        spe.skillId         = fields[3].GetUInt32();
        spe.spellFamilyName = fields[4].GetUInt32();
        spe.spellFamilyMask = fields[5].GetUInt64();
        spe.procFlags       = fields[6].GetUInt32();
        spe.ppmRate         = fields[7].GetFloat();

        mSpellProcEventMap[entry] = spe;

        count++;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell proc event conditions", count  );
}

bool SpellMgr::IsSpellProcEventCanTriggeredBy( SpellProcEventEntry const * spellProcEvent, SpellEntry const * procSpell, uint32 procFlags )
{
    if((procFlags & spellProcEvent->procFlags) == 0)
        return false;

    // Additional checks in case spell cast/hit/crit is the event
    // Check (if set) school, category, skill line, spell talent mask
    if(spellProcEvent->schoolMask && (!procSpell || !procSpell->School || ((1<<procSpell->School) & spellProcEvent->schoolMask) == 0))
        return false;
    if(spellProcEvent->category && (!procSpell || procSpell->Category != spellProcEvent->category))
        return false;
    if(spellProcEvent->skillId)
    {
        if (!procSpell) return false;
        SkillLineAbilityEntry const *skillLineEntry = sSkillLineAbilityStore.LookupEntry(procSpell->Id);
        if(!skillLineEntry || skillLineEntry->skillId != spellProcEvent->skillId)
            return false;
    }
    if(spellProcEvent->spellFamilyName && (!procSpell || spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
        return false;
    if(spellProcEvent->spellFamilyMask && (!procSpell || (spellProcEvent->spellFamilyMask & procSpell->SpellFamilyFlags) == 0))
        return false;

    return true;
}

void SpellMgr::LoadSpellThreats()
{
    sSpellThreatStore.Load();

    sLog.outString( ">> Loaded %u aggro generating spells", sSpellThreatStore.RecordCount );
    sLog.outString();
}

bool SpellMgr::IsRankSpellDueToSpell(SpellEntry const *spellInfo_1,uint32 spellId_2) const
{
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    return GetFirstSpellInChain(spellInfo_1->Id)==GetFirstSpellInChain(spellId_2);
}

bool SpellMgr::canStackSpellRanks(SpellEntry const *spellInfo,SpellEntry const *spellInfo2)
{
    // Riding not listed in spellbook and not stacked but have ranks and don't must replace ranks at learning
    if(spellInfo->Attributes == 0x10000D0)
        return true;

    if(spellInfo->powerType == 0)
    {
        if(spellInfo->manaCost > 0 && spellInfo->manaCost != spellInfo2->manaCost)
            return true;
        if(spellInfo->ManaCostPercentage > 0 && spellInfo->ManaCostPercentage != spellInfo2->ManaCostPercentage)
            return true;
        if(spellInfo->manaCostPerlevel > 0 && spellInfo->manaCostPerlevel != spellInfo2->manaCostPerlevel)
            return true;
        if(spellInfo->manaPerSecond > 0 && spellInfo->manaPerSecond != spellInfo2->manaPerSecond)
            return true;
        if(spellInfo->manaPerSecondPerLevel > 0 && spellInfo->manaPerSecondPerLevel != spellInfo2->manaPerSecondPerLevel)
            return true;
        if(spellInfo->Reagent[0] > 0 && spellInfo->Reagent[0] != spellInfo2->Reagent[0])
            return true;
    }
    return false;
}

bool SpellMgr::IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2) const
{
    SpellEntry const *spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);

    if(!spellInfo_1 || !spellInfo_2)
        return false;

    if(spellInfo_1->Id == spellId_2)
        return false;

    //I think we don't check this correctly because i need a exception for spell:
    //72,11327,18461...(called from 1856,1857...) Call Aura 16,31, after trigger another spell who call aura 77 and 77 remove 16 and 31, this should not happen.
    if(spellInfo_2->SpellFamilyFlags == 2048)
        return false;

    // Resurrection sickness
    if((spellInfo_1->Id == 15007) != (spellInfo_2->Id==15007))
        return false;

    // Paladin Seals
    if( IsSealSpell(spellId_1) && IsSealSpell(spellId_2) )
        return true;

    // Specific spell family spells
    switch(spellInfo_1->SpellFamilyName)
    {
        case SPELLFAMILY_WARLOCK:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_WARLOCK )
            {
                //Corruption & Seed of corruption
                if( spellInfo_1->SpellIconID == 313 && spellInfo_2->SpellIconID == 1932 ||
                    spellInfo_2->SpellIconID == 313 && spellInfo_1->SpellIconID == 1932 )
                    if(spellInfo_1->SpellVisual != 0 && spellInfo_2->SpellVisual != 0)
                        return true;                        // can't be stacked

                // Siphon Life and Drain Life
                if( spellInfo_1->SpellIconID == 152 && spellInfo_2->SpellIconID == 546 ||
                    spellInfo_2->SpellIconID == 152 && spellInfo_1->SpellIconID == 546 )
                    return false;

                // Corruption and Unstable Affliction
                if( spellInfo_1->SpellIconID == 313 && spellInfo_2->SpellIconID == 2039 ||
                    spellInfo_2->SpellIconID == 313 && spellInfo_1->SpellIconID == 2039 )
                    return false;
            }
            break;
        case SPELLFAMILY_PRIEST:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_PRIEST )
            {
                //Devouring Plague and Shadow Vulnerability
                if( (spellInfo_1->SpellFamilyFlags & 0x2000000) && (spellInfo_2->SpellFamilyFlags & 0x800000000LL) ||
                    (spellInfo_2->SpellFamilyFlags & 0x2000000) && (spellInfo_1->SpellFamilyFlags & 0x800000000LL) )
                    return false;
            }
            break;
        case SPELLFAMILY_WARRIOR:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_WARRIOR )
            {
                // Rend and Deep Wound
                if( (spellInfo_1->SpellFamilyFlags & 0x20) && (spellInfo_2->SpellFamilyFlags & 0x1000000000LL) ||
                    (spellInfo_2->SpellFamilyFlags & 0x20) && (spellInfo_1->SpellFamilyFlags & 0x1000000000LL) )
                    return false;
            }
            break;
        case SPELLFAMILY_SHAMAN:
            // Windfury weapon 
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_SHAMAN )
            {
                if( spellInfo_1->SpellIconID==220 && spellInfo_2->SpellIconID==220 &&
                    spellInfo_1->SpellFamilyFlags != spellInfo_2->SpellFamilyFlags )
                    return false;
            }
            break;
        default:
            break;
    }

    // Garrote and Garrote-Silence
    if( spellInfo_1->SpellIconID == 498 && spellInfo_2->SpellIconID == 498 && (
        spellInfo_1->SpellVisual == 0 && spellInfo_2->SpellVisual == 757 ||
        spellInfo_2->SpellVisual == 0 && spellInfo_1->SpellVisual == 757 ) )
        return false;

    // Thunderfury
    if( spellInfo_1->Id == 21992 && spellInfo_2->Id == 27648 || spellInfo_2->Id == 21992 && spellInfo_1->Id == 27648 )
        return false;

    // more generic checks
    if (spellInfo_1->SpellIconID == spellInfo_2->SpellIconID &&
        spellInfo_1->SpellIconID != 0 && spellInfo_2->SpellIconID != 0)
    {
        bool isModifier = false;
        for (int i = 0; i < 3; i++)
        {
            if (spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER ||
                spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER  ||
                spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER ||
                spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER )
                isModifier = true;
        }

        if (!isModifier)
            return true;
    }

    if (IsRankSpellDueToSpell(spellInfo_1, spellId_2))
        return true;

    if (spellInfo_1->SpellFamilyName == 0 || spellInfo_2->SpellFamilyName == 0)
        return false;

    if (spellInfo_1->SpellFamilyName != spellInfo_2->SpellFamilyName)
        return false;

    for (int i = 0; i < 3; ++i)
        if (spellInfo_1->Effect[i] != spellInfo_2->Effect[i] ||
            spellInfo_1->EffectItemType[i] != spellInfo_2->EffectItemType[i] ||
            spellInfo_1->EffectMiscValue[i] != spellInfo_2->EffectMiscValue[i] ||
            spellInfo_1->EffectApplyAuraName[i] != spellInfo_2->EffectApplyAuraName[i])
            return false;

    return true;
}

bool SpellMgr::IsProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return false;

    if(spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return IsProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return false;

    if(spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return IsPrimaryProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionFirstRankSpell(uint32 spellId) const
{
    return IsPrimaryProfessionSpell(spellId) && GetSpellRank(spellId)==1;
}

SpellEntry const* SpellMgr::SelectAuraRankForPlayerLevel(SpellEntry const* spellInfo, uint32 playerLevel) const
{
    // ignore passive spells
    if(IsPassiveSpell(spellInfo->Id))
        return spellInfo;

    bool needRankSelection = false;
    for(int i=0;i<3;i++)
    {
        if( IsPositiveEffect(spellInfo->Id, i) && (
            spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA ||
            spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA
            ) )
        {
            needRankSelection = true;
            break;
        }
    }

    // not required
    if(!needRankSelection)
        return spellInfo;

    for(uint32 nextSpellId = spellInfo->Id; nextSpellId != 0; nextSpellId = GetPrevSpellInChain(nextSpellId))
    {
        SpellEntry const *nextSpellInfo = sSpellStore.LookupEntry(nextSpellId);
        if(!nextSpellInfo)
            break;

        // if found appropriate level
        if(playerLevel + 10 >= nextSpellInfo->spellLevel)
            return nextSpellInfo;

        // one rank less then
    }

    // not found
    return NULL;
}

void SpellMgr::LoadSpellChains()
{
    mSpellChains.clear();                                   // need for reload case

    QueryResult *result = WorldDatabase.PQuery("SELECT `spell_id`, `prev_spell`, `first_spell`, `rank` FROM `spell_chain`");
    if(result == NULL)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 spell chain records" );
        sLog.outErrorDb("`spell_chains` table is empty!");
        return;
    }

    uint32 count = 0;

    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();

        SpellChainNode node;
        node.prev  = fields[1].GetUInt32();
        node.first = fields[2].GetUInt32();
        node.rank  = fields[3].GetUInt8();

        if(!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` does not exist",spell_id);
            continue;
        }

        if(node.prev!=0 && !sSpellStore.LookupEntry(node.prev))
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` has not existing previous rank spell: %u",spell_id,node.prev);
            continue;
        }

        if(!sSpellStore.LookupEntry(node.first))
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` has not existing first rank spell: %u",spell_id,node.first);
            continue;
        }

        // check basic spell chain data integrity (note: rank can be equal 0 or 1 for first/single spell)
        if( (spell_id == node.first) != (node.rank <= 1) ||
            (spell_id == node.first) != (node.prev == 0) ||
            (node.rank <= 1) != (node.prev == 0) )
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` has not compatible chain data (prev: %u, first: %u, rank: %d)",spell_id,node.prev,node.first,node.rank);
            continue;
        }

        mSpellChains[spell_id] = node;

        ++count;
    } while( result->NextRow() );

    // additional integrity checks
    for(SpellChainMap::iterator i = mSpellChains.begin(); i != mSpellChains.end(); ++i)
    {
        if(i->second.prev)
        {
            SpellChainMap::iterator i_prev = mSpellChains.find(i->second.prev);
            if(i_prev == mSpellChains.end())
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d) listed in `spell_chain` has not found previous rank spell in table.",
                    i->first,i->second.prev,i->second.first,i->second.rank);
            }
            else if( i_prev->second.first != i->second.first )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d) listed in `spell_chain` has different first spell in chain compared to previous rank spell (prev: %u, first: %u, rank: %d).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i_prev->second.prev,i_prev->second.first,i_prev->second.rank);
            }
            else if( i_prev->second.rank+1 != i->second.rank )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d) listed in `spell_chain` has different rank compared to previous rank spell (prev: %u, first: %u, rank: %d).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i_prev->second.prev,i_prev->second.first,i_prev->second.rank);
            }
        }
    }

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell chain records", count );
}

void SpellMgr::LoadSpellLearnSkills()
{
    mSpellLearnSkills.clear();                              // need for reload case

    QueryResult *result = WorldDatabase.PQuery("SELECT `entry`, `SkillID`, `Value`, `MaxValue` FROM `spell_learn_skill`");
    if(!result)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 spell learn skills" );
        sLog.outErrorDb("`spell_learn_skill` table is empty!");
        return;
    }

    uint32 count = 0;

    uint16 maxconfskill = sWorld.GetConfigMaxSkillValue();

    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();
        int32 skill_val = fields[2].GetInt32();
        int32 skill_max = fields[3].GetInt32();

        SpellLearnSkillNode node;
        node.skill    = fields[1].GetUInt32();
        node.value    = skill_val < 0 ? maxconfskill : skill_val;
        node.maxvalue = skill_max < 0 ? maxconfskill : skill_max;

        if(!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_skill` does not exist",spell_id);
            continue;
        }

        if(!sSkillLineStore.LookupEntry(node.skill))
        {
            sLog.outErrorDb("Skill %u listed in `spell_learn_skill` does not exist",node.skill);
            continue;
        }

        mSpellLearnSkills[spell_id] = node;

        ++count;
    } while( result->NextRow() );

    delete result;

    // search auto-learned skills and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for(uint32 spell = 0; spell < sSpellStore.nCount; ++spell)
    {
        SpellEntry const* entry = sSpellStore.LookupEntry(spell);

        if(!entry)
            continue;

        for(int i = 0; i < 3; ++i)
        {
            if(entry->Effect[i]==SPELL_EFFECT_SKILL)
            {
                SpellLearnSkillNode dbc_node;
                dbc_node.skill    = entry->EffectMiscValue[i];
                dbc_node.value    = 1;
                dbc_node.maxvalue = (entry->EffectBasePoints[i]+1)*75;

                SpellLearnSkillNode const* db_node = GetSpellLearnSkill(spell);

                if(db_node)
                {
                    if(db_node->skill != dbc_node.skill)
                        sLog.outErrorDb("Spell %u auto-learn skill %u in spell.dbc but learn skill %u in `spell_learn_skill`, please fix DB.",
                        spell,dbc_node.skill,db_node->skill);

                    continue;                               // skip already added spell-skill pair
                }

                mSpellLearnSkills[spell] = dbc_node;
                ++dbc_count;
                break;
            }
        }
    }

    sLog.outString();
    sLog.outString( ">> Loaded %u spell learn skills + %u found in DBC", count, dbc_count );
}

void SpellMgr::LoadSpellLearnSpells()
{
    mSpellLearnSpells.clear();                              // need for reload case

    QueryResult *result = WorldDatabase.PQuery("SELECT `entry`, `SpellID`,`IfNoSpell` FROM `spell_learn_spell`");
    if(!result)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 spell learn spells" );
        sLog.outErrorDb("`spell_learn_spell` table is empty!");
        return;
    }

    uint32 count = 0;

    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id    = fields[0].GetUInt32();

        SpellLearnSpellNode node;
        node.spell      = fields[1].GetUInt32();
        node.ifNoSpell  = fields[2].GetUInt32();
        node.autoLearned= false;

        if(!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` does not exist",spell_id);
            continue;
        }

        if(!sSpellStore.LookupEntry(node.spell))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` does not exist",node.spell);
            continue;
        }

        if(node.ifNoSpell && !sSpellStore.LookupEntry(node.ifNoSpell))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` does not exist",node.ifNoSpell);
            continue;
        }

        mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell_id,node));

        ++count;
    } while( result->NextRow() );

    delete result;

    // search auto-learned spells and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for(uint32 spell = 0; spell < sSpellStore.nCount; ++spell)
    {
        SpellEntry const* entry = sSpellStore.LookupEntry(spell);

        if(!entry)
            continue;

        for(int i = 0; i < 3; ++i)
        {
            if(entry->Effect[i]==SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell       = entry->EffectTriggerSpell[i];
                dbc_node.ifNoSpell   = 0;
                dbc_node.autoLearned = true;

                SpellLearnSpellMap::const_iterator db_node_begin = GetBeginSpellLearnSpell(spell);
                SpellLearnSpellMap::const_iterator db_node_end   = GetEndSpellLearnSpell(spell);

                bool found = false;
                for(SpellLearnSpellMap::const_iterator itr = db_node_begin; itr != db_node_end; ++itr)
                {
                    if(itr->second.spell == dbc_node.spell)
                    {
                        sLog.outErrorDb("Spell %u auto-learn spell %u in spell.dbc then the record in `spell_learn_spell` is redundant, please fix DB.",
                            spell,dbc_node.spell);
                        found = true;
                        break;
                    }
                }

                if(!found)                                  // add new spell-spell pair if not found
                {
                    mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell,dbc_node));
                    ++dbc_count;
                }
            }
        }
    }

    sLog.outString();
    sLog.outString( ">> Loaded %u spell learn spells + %u found in DBC", count, dbc_count );
}

void SpellMgr::LoadSpellScriptTarget()
{
    mSpellScriptTarget.clear();                             // need for reload case

    uint32 count = 0;

    QueryResult *result = WorldDatabase.Query("SELECT `entry`,`type`,`targetEntry` FROM `spell_script_target`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outErrorDb(">> Loaded 0 SpellScriptTarget. DB table `spell_script_target` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 spellId     = fields[0].GetUInt32();
        uint32 type        = fields[1].GetUInt32();
        uint32 targetEntry = fields[2].GetUInt32();

        SpellEntry const* spellProto = sSpellStore.LookupEntry(spellId);

        if(!spellProto)
        {
            sLog.outErrorDb("Table `spell_script_target`: spellId %u listed for TargetEntry %u does not exist.",spellId,targetEntry);
            continue;
        }

        bool targetfound = false;
        for(int i = 0; i <3; ++i)
        {
            if(spellProto->EffectImplicitTargetA[i]==TARGET_SCRIPT||spellProto->EffectImplicitTargetB[i]==TARGET_SCRIPT)
            {
                targetfound = true;
                break;
            }
        }
        if(!targetfound)
        {
            sLog.outErrorDb("Table `spell_script_target`: spellId %u listed for TargetEntry %u does not have any implecit target TARGET_SCRIPT(38).",spellId,targetEntry);
            continue;
        }

        if( type >= MAX_SPELL_TARGET_TYPE )
        {
            sLog.outErrorDb("Table `spell_script_target`: target type %u for TargetEntry %u is incorrect.",type,targetEntry);
            continue;
        }

        switch(type)
        {
        case SPELL_TARGET_TYPE_GAMEOBJECT:
            {
                if( targetEntry==0 )
                    break;

                if(!sGOStorage.LookupEntry<GameObjectInfo>(targetEntry))
                {
                    sLog.outErrorDb("Table `spell_script_target`: gameobject template entry %u does not exist.",targetEntry);
                    continue;
                }
                break;
            }
        default:
            {
                if( targetEntry==0 )
                {
                    sLog.outErrorDb("Table `spell_script_target`: target entry == 0 for not GO target type (%u).",type);
                    continue;
                }
                if(!sCreatureStorage.LookupEntry<CreatureInfo>(targetEntry))
                {
                    sLog.outErrorDb("Table `spell_script_target`: creature template entry %u does not exist.",targetEntry);
                    continue;
                }
                break;
            }
        }

        mSpellScriptTarget.insert(SpellScriptTarget::value_type(spellId,SpellTargetEntry(SpellTargetType(type),targetEntry)));

        count++;
    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString(">> Loaded %u Spell Script Targets", count);
}

/// Some checks for spells, to prevent adding depricated/broken spells for trainers, spell book, etc
bool SpellMgr::IsSpellValid(SpellEntry const* spellInfo, Player* pl, bool msg)
{
    // not exist
    if(!spellInfo)
        return false;

    bool need_check_reagents = false;

    // check effects
    for(int i=0; i<3; ++i)
    {
        switch(spellInfo->Effect[i])
        {
        case 0:
            continue;

            // craft spell for crafting non-existed item (break client recipes list show)
        case SPELL_EFFECT_CREATE_ITEM:
            {
                if(!ObjectMgr::GetItemPrototype( spellInfo->EffectItemType[i] ))
                {
                    if(msg)
                    {
                        if(pl)
                            ChatHandler(pl).PSendSysMessage("Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->EffectItemType[i]);
                        else
                            sLog.outErrorDb("Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->EffectItemType[i]);
                    }
                    return false;
                }

                need_check_reagents = true;
                break;
            }
        case SPELL_EFFECT_LEARN_SPELL:
            {
                SpellEntry const* spellInfo2 = sSpellStore.LookupEntry(spellInfo->EffectTriggerSpell[0]);
                if( !IsSpellValid(spellInfo2,pl,msg) )
                {
                    if(msg)
                    {
                        if(pl)
                            ChatHandler(pl).PSendSysMessage("Spell %u learn to broken spell %u, and then...",spellInfo->Id,spellInfo->EffectTriggerSpell[0]);
                        else
                            sLog.outErrorDb("Spell %u learn to invalid spell %u, and then...",spellInfo->Id,spellInfo->EffectTriggerSpell[0]);
                    }
                    return false;
                }
                break;
            }
        }
    }

    if(need_check_reagents)
    {
        for(int j = 0; j < 8; ++j)
        {
            if(spellInfo->Reagent[j] > 0 && !ObjectMgr::GetItemPrototype( spellInfo->Reagent[j] ))
            {
                if(msg)
                {
                    if(pl)
                        ChatHandler(pl).PSendSysMessage("Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Reagent[j]);
                    else
                        sLog.outErrorDb("Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Reagent[j]);
                }
                return false;
            }
        }
    }

    return true;
}

// Spell that must be affected by MECHANIC_INVULNERABILITY not have this mechanic, check its by another data
bool IsMechanicInvulnerabilityImmunityToSpell(SpellEntry const* spellInfo)
{
    if(spellInfo->SpellFamilyName != SPELLFAMILY_PALADIN)
        return false;
    
    //Divine Protection
    if((spellInfo->SpellFamilyFlags & 0x400000000000LL) && spellInfo->SpellIconID==73)
        return true;

    //Divine Shield or Blessing of Protection or Avenging Wrath
    if(spellInfo->SpellFamilyFlags & 0x200000400080LL)
        return true;

    return false;
}
