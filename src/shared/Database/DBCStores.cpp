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
#include "Log.h"
#include "ProgressBar.h"

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

void LoadDBCStores(std::string dataPath)
{
    std::string tmpPath="";

    const uint32 DBCFilesCount = 15;

    barGoLink bar( DBCFilesCount );

    std::list<std::string> not_found_dbc_files;

    tmpPath=dataPath;
    tmpPath.append("dbc/AreaTable.dbc");
    if(sAreaStore.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/AreaTable.dbc");

    tmpPath=dataPath;
    tmpPath.append("dbc/EmotesText.dbc");
    if(sEmoteStore.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/EmotesText.dbc");

    tmpPath=dataPath;
    tmpPath.append("dbc/Faction.dbc");
    if(sFactionStore.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/Faction.dbc");

    tmpPath=dataPath;
    tmpPath.append("dbc/FactionTemplate.dbc");
    if(sFactionTemplateStore.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/FactionTemplate.dbc");

    tmpPath=dataPath;
    tmpPath.append("dbc/ItemDisplayInfo.dbc");
    if(sItemDisplayTemplateStore.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/ItemDisplayInfo.dbc");

    tmpPath=dataPath;
    tmpPath.append("dbc/ItemSet.dbc");
    if(sItemSetStore.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/ItemSet.dbc");

    tmpPath=dataPath;
    tmpPath.append("dbc/SkillLineAbility.dbc");
    if(sSkillLineAbilityStore.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/SkillLineAbility.dbc");

    tmpPath=dataPath;
    tmpPath.append("dbc/Spell.dbc");
    if(sSpellStore.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/Spell.dbc");

    tmpPath=dataPath;
    tmpPath.append("dbc/SpellCastTimes.dbc");
    if(sCastTime.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/SpellCastTimes.dbc");

    tmpPath=dataPath;
    tmpPath.append("dbc/SpellDuration.dbc");
    if(sSpellDuration.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/SpellDuration.dbc");


    tmpPath=dataPath;
    tmpPath.append("dbc/SpellFocusObject.dbc");
    if(sSpellFocusObject.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("SpellFocusObject.dbc"); 

    tmpPath=dataPath;
    tmpPath.append("dbc/SpellItemEnchantment.dbc");
    if(sSpellItemEnchantmentStore.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/SpellItemEnchantment.dbc");

    tmpPath=dataPath;
    tmpPath.append("dbc/SpellRadius.dbc");
    if(sSpellRadius.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/SpellRadius.dbc");

    tmpPath=dataPath;
    tmpPath.append("dbc/SpellRange.dbc");
    if(sSpellRange.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/SpellRange.dbc");

    tmpPath=dataPath;
    tmpPath.append("dbc/Talent.dbc");
    if(sTalentStore.Load(tmpPath.c_str()))
        bar.step();
    else
        not_found_dbc_files.push_back("dbc/Talent.dbc");

    if(not_found_dbc_files.size() >= DBCFilesCount )
    {
        sLog.outError("\n\nIncorrect DataDir value in mangosd.conf or ALL required *.dbc files (%d) not found by path: %sdbc",DBCFilesCount,dataPath.c_str());
        exit(1);
    }
    else if(not_found_dbc_files.size() > 0 )
    {

        std::string str;
        for(std::list<std::string>::iterator i = not_found_dbc_files.begin(); i != not_found_dbc_files.end(); ++i)
            str += dataPath + *i + "\n";

        sLog.outError("\n\nSome required *.dbc files (%u from %d) not found:\n%s",not_found_dbc_files.size(),DBCFilesCount,str.c_str());
        exit(1);
    }

    sLog.outString( "" );
    sLog.outString( ">> Loaded %d data stores", DBCFilesCount );
    sLog.outString( "" );
}




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
uint32 FindSpellRank(uint32 spellId)
{
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo) return false;
    int rankfield = -1;
    uint32 rank = 0;
    for(int i=0;i<8;i++)
    if(spellInfo->Rank[i] > 0)
    {
        rankfield = i;
        break;
    }
    if(rankfield < 0)
        return 0;
    if(rankfield == 0)//english client
    {
        switch(spellInfo->Rank[rankfield])
        {
            case 172:rank = 1;break;
            case 2438:rank = 2;break;
            case 5271:rank = 3;break;
            case 16790:rank = 4;break;
            case 18149:rank = 5;break;
            case 13419:rank = 6;break;
            case 11757:rank = 7;break;
            case 70968:rank = 8;break;
            case 134222:rank = 9;break;
            case 134229:rank = 10;break;
            case 134237:rank = 11;break;
            case 134245:rank = 12;break;
            case 134274:rank = 13;break;
            case 134282:rank = 14;break;
            case 134290:rank = 15;break;
            case 146442:rank = 16;break;
            case 59695:rank = 51;break;//51-54 is rank of profession skill.
            case 46478:rank = 52;break;
            case 84128:rank = 53;break;
            case 274133:rank = 54;break;
            case 822:rank = 0;break;//spell from creating related to race 
            default:rank = 0;break;
        }
    }
    if(rankfield == 4)//chinese client
    {
        switch(spellInfo->Rank[rankfield])
        {
            case 134:rank = 1;break;
            case 2286:rank = 2;break;
            case 5283:rank = 3;break;
            case 17645:rank = 4;break;
            case 19250:rank = 5;break;
            case 13924:rank = 6;break;
            case 12034:rank = 7;break;
            case 70787:rank = 8;break;
            case 129076:rank = 9;break;
            case 129085:rank = 10;break;
            case 129095:rank = 11;break;
            case 129105:rank = 12;break;
            case 129136:rank = 13;break;
            case 129146:rank = 14;break;
            case 129156:rank = 15;break;
            case 140190:rank = 16;break;
            case 49393:rank = 51;break;//51-54 is rank of profession skill.
            case 49400:rank = 52;break;
            case 83240:rank = 53;break;
            case 256953:rank = 54;break;
            case 797:rank = 0;break;//spell from creating related to race 
            default:rank = 0;break;
        }
    }
    return rank;
}
bool IsRankSpellDueToSpell(SpellEntry *spellInfo_1,uint32 spellId_2)
{
    SpellEntry *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    for(int i=0;i<8;i++) 
    if (spellInfo_1->SpellNameIndex[i] != spellInfo_2->SpellNameIndex[i]) 
        return false;
    return true;
}

bool IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2)
{
    SpellEntry *spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    if (spellInfo_1->SpellFamilyName == 0 ||
        spellInfo_2->SpellFamilyName == 0)
         return false;

    if (spellInfo_1->SpellIconID == spellInfo_2->SpellIconID ||
        IsRankSpellDueToSpell(spellInfo_1, spellId_2)) 
        return true;

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

bool IsNoStackAuraDueToAura(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2)
{
    SpellEntry *spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
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
    SpellEntry *spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return 0;
    if (spellId_1 == spellId_2) return 0;

    int32 diff = (int32)spellInfo_1->EffectBasePoints[effIndex_1] - (int32)spellInfo_2->EffectBasePoints[effIndex_2];
    if (spellInfo_1->EffectBasePoints[effIndex_1] < 0 && spellInfo_2->EffectBasePoints[effIndex_2] < 0) return -diff;
    else return diff;
}

SpellSpecific GetSpellSpecific(uint32 spellId)
{
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo) return SPELL_NORMAL;

    if(spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN)
    {
        // only paladin seals have this
        if (spellInfo->SpellVisual == 5622)
            return SPELL_SEAL;

        for (int i = 0; i < 3; i++)
        {
            // only paladin auras have this
            if (spellInfo->Effect[i] == 35)//SPELL_EFFECT_APPLY_AREA_AURA 
                return SPELL_AURA;
            // only paladin blessings / greater blessings have this
            if (spellInfo->EffectImplicitTargetA[i] == 21//TARGET_S_F
                ||spellInfo->EffectImplicitTargetA[i] == 57//TARGET_S_F_2
                ||spellInfo->EffectImplicitTargetA[i] == 61)//TARGET_AF_PC
                return SPELL_BLESSING;
        }
    }

    // only warlock curses have this
    if(spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK)
        if (spellInfo->Dispel == 2)//IMMUNE_DISPEL_CURSE
            return SPELL_CURSE;

    if(spellInfo->SpellFamilyName == SPELLFAMILY_HUNTER)
    {
        // only hunter stings have this
        if (spellInfo->Dispel == 4)//IMMUNE_DISPEL_POISON
            return SPELL_STING;
        // only hunter aspects have this
        if (spellInfo->School == 3/*SPELL_SCHOOL_NATURE*/ && spellInfo->activeIconID == 122)
            return SPELL_ASPECT;
    }

    return SPELL_NORMAL;
}

bool IsSpellSingleEffectPerCaster(uint32 spellId)
{
    switch(GetSpellSpecific(spellId)) {
        case SPELL_SEAL:
        case SPELL_BLESSING:
        case SPELL_AURA:
        case SPELL_STING:
        case SPELL_CURSE:
        case SPELL_ASPECT:
            return true;
        default:
            return false;
    }
}
