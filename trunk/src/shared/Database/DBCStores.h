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

#ifndef DBCSTORES_H
#define DBCSTORES_H

#include "Common.h"
//#include "DataStore.h"
#include "dbcfile.h"
#include "DBCStructure.h"

enum SpellSpecific
{
    SPELL_NORMAL = 0,
    SPELL_SEAL = 1,
    SPELL_BLESSING = 2,
    SPELL_AURA = 3,
    SPELL_STING = 4,
    SPELL_CURSE = 5,
    SPELL_ASPECT = 6,
    SPELL_TRACKER = 7,
    SPELL_WARLOCK_ARMOR = 8,
    SPELL_MAGE_ARMOR = 9,
    SPELL_ELEMENTAL_SHIELD = 10,
    SPELL_MAGE_POLYMORPH = 11
};

enum SpellFamilyNames
{
    SPELLFAMILY_GENERIC = 0,
    SPELLFAMILY_MAGE = 3,
    SPELLFAMILY_WARRIOR = 4,
    SPELLFAMILY_WARLOCK = 5,
    SPELLFAMILY_PRIEST = 6,
    SPELLFAMILY_DRUID = 7,
    SPELLFAMILY_ROGUE = 8,
    SPELLFAMILY_HUNTER = 9,
    SPELLFAMILY_PALADIN = 10,
    SPELLFAMILY_SHAMAN = 11,
    SPELLFAMILY_POTION = 13
};

//SpellFamilyFlags
#define SPELLFAMILYFLAG_ROGUE_VANISH     0x00000800
#define SPELLFAMILYFLAG_ROGUE_STEALTH    0x00400000
#define SPELLFAMILYFLAG_ROGUE_BACKSTAB   0x00800004
#define SPELLFAMILYFLAG_ROGUE_SAP        0x00000080
#define SPELLFAMILYFLAG_ROGUE_FEINT      0x08000000
#define SPELLFAMILYFLAG_ROGUE_KIDNEYSHOT 0x00200000

typedef std::list<uint32> SimpleFactionsList;

SimpleFactionsList const* GetFactionTeamList(uint32 faction);
float GetRadius(SpellRadiusEntry const *radius);
uint32 GetCastTime(SpellCastTimesEntry const*time);
float GetMinRange(SpellRangeEntry const *range);
float GetMaxRange(SpellRangeEntry const *range);
int32 GetDuration(SpellEntry const *spellInfo);
int32 GetMaxDuration(SpellEntry const *spellInfo);
char* GetPetName(uint32 petfamily, uint32 dbclang);
bool IsNoStackAuraDueToAura(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2);
bool IsSealSpell(uint32 spellId);
bool CanCastWhileMounted(uint32 spellId);
int32 CompareAuraRanks(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2);
SpellSpecific GetSpellSpecific(uint32 spellId);
bool IsSpellSingleEffectPerCaster(uint32 spellId);
bool IsPassiveSpell(uint32 spellId);
bool IsNonCombatSpell(uint32 spellId);
uint32 GetTalentSpellCost(uint32 spellId);

bool IsPositiveSpell(uint32 spellId);
bool IsPositiveEffect(uint32 spellId, uint32 effIndex);
bool IsPositiveTarget(uint32 targetA, uint32 targetB);

bool IsSingleTargetSpell(uint32 spellId);
bool IsSingleTargetSpells(SpellEntry const *spellInfo1, SpellEntry const *spellInfo2);

AreaTableEntry const* GetAreaEntryByAreaID(uint32 area_id);
AreaTableEntry const* GetAreaEntryByAreaFlag(uint32 area_flag);
uint32 GetAreaFlagByMapId(uint32 mapid);
inline bool IsNoDamageXPArea(uint32 area_id) { return area_id==3712 || area_id==3803; }
bool CanUsedWhileStealthed(uint32 spellId);
ChatChannelsEntry const* GetChannelEntryFor(uint32 channel_id);

bool IsTotemCategoryCompatiableWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId);

template<class T>
class DBCStorage
{
    public:
        DBCStorage(const char *f){indexTable = NULL;fmt=f;fieldCount = 0; nCount =0; data = NULL; }
        ~DBCStorage() { Clear(); }

        inline
            T const* LookupEntry(uint32 id) const
        {
            return (id>=nCount)?NULL:indexTable[id];
        }
        inline
            unsigned int GetNumRows() const
        {
            return nCount;
        }

        bool Load(char const* fn)
        {

            dbc = new DBCFile;
            // Check if load was sucessful, only then continue
            bool res = dbc->Load(fn, fmt);
            if (res)
            {
                fieldCount = dbc->GetCols();
                indexTable=(T **) dbc->AutoProduceData(fmt,&nCount,data);
            }
            delete dbc;

            // error in dbc file at loading
            if(!indexTable)
                res = false;

            return res;
        }

        void Clear()
        {
            if (!indexTable) return;
            delete[] ((char*)indexTable); indexTable = NULL;
            delete[] data;
        }

        T** indexTable;
        char * data;
        uint32 nCount;
        uint32 fieldCount;
        const char * fmt;

    private:
        DBCFile * dbc;
};

//extern DBCStorage <AreaTableEntry>            sAreaStore; -- accessed using 2 functions
extern DBCStorage <AreaTriggerEntry>          sAreaTriggerStore;
extern DBCStorage <BankBagSlotPricesEntry>    sBankBagSlotPricesStore;
extern DBCStorage <BattlemasterListEntry>     sBattlemasterListStore;
//extern DBCStorage <ChatChannelsEntry>         sChatChannelsStore; -- accessed using function, no usable index
extern DBCStorage <ChrClassesEntry>           sChrClassesStore;
extern DBCStorage <ChrRacesEntry>             sChrRacesStore;
extern DBCStorage <CreatureDisplayInfoEntry>  sCreatureDisplayInfoStore;
extern DBCStorage <CreatureFamilyEntry>       sCreatureFamilyStore;
extern DBCStorage <DurabilityCostsEntry>      sDurabilityCostsStore;
extern DBCStorage <DurabilityQualityEntry>    sDurabilityQualityStore;
extern DBCStorage <SpellCastTimesEntry>       sCastTimesStore;
extern DBCStorage <EmotesTextEntry>           sEmotesTextStore;
extern DBCStorage <FactionEntry>              sFactionStore;
extern DBCStorage <FactionTemplateEntry>      sFactionTemplateStore;
extern DBCStorage <GemPropertiesEntry>        sGemPropertiesStore;

extern DBCStorage <GtCombatRatingsEntry>         sGtCombatRatingsStore;
extern DBCStorage <GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore;
extern DBCStorage <GtChanceToMeleeCritEntry>     sGtChanceToMeleeCritStore;
extern DBCStorage <GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore;
extern DBCStorage <GtChanceToSpellCritEntry>     sGtChanceToSpellCritStore;
extern DBCStorage <GtOCTRegenHPEntry>            sGtOCTRegenHPStore;
extern DBCStorage <GtOCTRegenMPEntry>            sGtOCTRegenMPStore;
extern DBCStorage <GtRegenHPPerSptEntry>         sGtRegenHPPerSptStore;
extern DBCStorage <GtRegenMPPerSptEntry>         sGtRegenMPPerSptStore;

//extern DBCStorage <ItemDisplayInfoEntry>      sItemDisplayInfoStore; -- not used currently
extern DBCStorage <ItemExtendedCostEntry>     sItemExtendedCostStore;
extern DBCStorage <ItemRandomPropertiesEntry> sItemRandomPropertiesStore;
extern DBCStorage <ItemRandomSuffixEntry>     sItemRandomSuffixStore;
extern DBCStorage <ItemSetEntry>              sItemSetStore;
extern DBCStorage <LockEntry>                 sLockStore;
extern DBCStorage <MapEntry>                  sMapStore;
extern DBCStorage <SkillLineEntry>            sSkillLineStore;
extern DBCStorage <SkillLineAbilityEntry>     sSkillLineAbilityStore;
extern DBCStorage <SpellDurationEntry>        sSpellDurationStore;
//extern DBCStorage <SpellFocusObjectEntry>     sSpellFocusObjectStore;
extern DBCStorage <SpellItemEnchantmentEntry> sSpellItemEnchantmentStore;
extern DBCStorage <SpellItemEnchantmentConditionEntry> sSpellItemEnchantmentConditionStore;
extern SpellCategoryStore                     sSpellCategoryStore;
extern DBCStorage <SpellRadiusEntry>          sSpellRadiusStore;
extern DBCStorage <SpellRangeEntry>           sSpellRangeStore;
extern DBCStorage <SpellEntry>                sSpellStore;
extern DBCStorage <StableSlotPricesEntry>     sStableSlotPricesStore;
extern DBCStorage <TalentEntry>               sTalentStore;
extern DBCStorage <TalentTabEntry>            sTalentTabStore;
extern DBCStorage <TaxiNodesEntry>            sTaxiNodesStore;
extern TaxiMask                               sTaxiNodesMask;
extern TaxiPathSetBySource                    sTaxiPathSetBySource;
extern TaxiPathNodesByPath                    sTaxiPathNodesByPath;
extern DBCStorage <TotemCategoryEntry>        sTotemCategoryStore;
extern DBCStorage <WorldSafeLocsEntry>        sWorldSafeLocsStore;

void LoadDBCStores(std::string dataPath);

// script support functions
MANGOS_DLL_SPEC DBCStorage <SpellEntry>      const* GetSpellStore()     ;
MANGOS_DLL_SPEC DBCStorage <SpellRangeEntry> const* GetSpellRangeStore() ;
#endif
