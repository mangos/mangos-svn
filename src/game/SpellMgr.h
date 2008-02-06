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

#ifndef _SPELLMGR_H
#define _SPELLMGR_H

// For static or at-server-startup loaded spell data
// For more high level function for sSpellStore data

#include "SharedDefines.h"
#include "Database/DBCStructure.h"
#include "Database/SQLStorage.h"

#include "Utilities/HashMap.h"
#include <map>

class Player;

extern SQLStorage sSpellThreatStore;

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

//Soem SpellFamilyFlags
#define SPELLFAMILYFLAG_ROGUE_VANISH     0x00000800
#define SPELLFAMILYFLAG_ROGUE_STEALTH    0x00400000
#define SPELLFAMILYFLAG_ROGUE_BACKSTAB   0x00800004
#define SPELLFAMILYFLAG_ROGUE_SAP        0x00000080
#define SPELLFAMILYFLAG_ROGUE_FEINT      0x08000000
#define SPELLFAMILYFLAG_ROGUE_KIDNEYSHOT 0x00200000

// Spell clasification
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
    SPELL_MAGE_POLYMORPH = 11,
    SPELL_POSITIVE_SHOUT = 12
};

SpellSpecific GetSpellSpecific(uint32 spellId);

// Different spell properties
float GetRadius(SpellRadiusEntry const *radius);
uint32 GetCastTime(SpellCastTimesEntry const*time);
float GetMinRange(SpellRangeEntry const *range);
float GetMaxRange(SpellRangeEntry const *range);
int32 GetDuration(SpellEntry const *spellInfo);
int32 GetMaxDuration(SpellEntry const *spellInfo);
inline uint32 GetRecoveryTime(SpellEntry const *spellInfo) { return spellInfo->RecoveryTime > spellInfo->CategoryRecoveryTime ? spellInfo->RecoveryTime : spellInfo->CategoryRecoveryTime; }

bool IsNoStackAuraDueToAura(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2);
bool IsSealSpell(uint32 spellId);
int32 CompareAuraRanks(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2);
bool IsSpellSingleEffectPerCaster(uint32 spellId);
bool IsPassiveSpell(uint32 spellId);
bool IsNonCombatSpell(uint32 spellId);

bool IsPositiveSpell(uint32 spellId);
bool IsPositiveEffect(uint32 spellId, uint32 effIndex);
bool IsPositiveTarget(uint32 targetA, uint32 targetB);

bool IsSingleTargetSpell(uint32 spellId);
bool IsSingleTargetSpells(SpellEntry const *spellInfo1, SpellEntry const *spellInfo2);

bool CanUsedWhileStealthed(uint32 spellId);
bool IsMechanicInvulnerabilityImmunityToSpell(SpellEntry const* spellInfo);

// Spell affects related declarations (accessed using SpellMgr functions)
struct SpellAffection
{
    uint16 SpellId;
    uint8 SchoolMask;
    uint16 Category;
    uint16 SkillId;
    uint8 SpellFamily;
    uint64 SpellFamilyMask;
    uint16 Charges;
};

typedef HM_NAMESPACE::hash_map<uint32, SpellAffection> SpellAffectMap;

// Spell proc event related declarations (accessed using SpellMgr functions)
enum ProcFlags
{
    PROC_FLAG_NONE               = 0x00000000,              // None
    PROC_FLAG_HIT_MELEE          = 0x00000001,              // On melee hit
    PROC_FLAG_STRUCK_MELEE       = 0x00000002,              // On being struck melee
    PROC_FLAG_KILL_XP_GIVER      = 0x00000004,              // On kill target giving XP or honor
    PROC_FLAG_SPECIAL_DROP       = 0x00000008,              //
    PROC_FLAG_DODGE              = 0x00000010,              // On dodge melee attack
    PROC_FLAG_PARRY              = 0x00000020,              // On parry melee attack
    PROC_FLAG_BLOCK              = 0x00000040,              // On block attack
    PROC_FLAG_TOUCH              = 0x00000080,              // On being touched (for bombs, probably?)
    PROC_FLAG_TARGET_LOW_HEALTH  = 0x00000100,              // On deal damage to enemy with 20% or less health
    PROC_FLAG_LOW_HEALTH         = 0x00000200,              // On health dropped below 20%
    PROC_FLAG_STRUCK_RANGED      = 0x00000400,              // On being struck ranged
    PROC_FLAG_HIT_SPECIAL        = 0x00000800,              // (!)Removed, may be reassigned in future
    PROC_FLAG_CRIT_MELEE         = 0x00001000,              // On crit melee
    PROC_FLAG_STRUCK_CRIT_MELEE  = 0x00002000,              // On being critically struck in melee
    PROC_FLAG_CAST_SPELL         = 0x00004000,              // On cast spell
    PROC_FLAG_TAKE_DAMAGE        = 0x00008000,              // On take damage
    PROC_FLAG_CRIT_SPELL         = 0x00010000,              // On crit spell
    PROC_FLAG_HIT_SPELL          = 0x00020000,              // On hit spell
    PROC_FLAG_STRUCK_CRIT_SPELL  = 0x00040000,              // On being critically struck by a spell
    PROC_FLAG_HIT_RANGED         = 0x00080000,              // On getting ranged hit
    PROC_FLAG_STRUCK_SPELL       = 0x00100000,              // On being struck by a spell
    PROC_FLAG_TRAP               = 0x00200000,              // On trap activation (?)
    PROC_FLAG_CRIT_RANGED        = 0x00400000,              // On getting ranged crit
    PROC_FLAG_STRUCK_CRIT_RANGED = 0x00800000,              // On being critically struck by a ranged attack
    PROC_FLAG_RESIST_SPELL       = 0x01000000,              // On resist enemy spell
    PROC_FLAG_TARGET_RESISTS     = 0x02000000,              // On enemy resisted spell
    PROC_FLAG_TARGET_DODGE_OR_PARRY= 0x04000000,            // On enemy dodges/parries
    PROC_FLAG_HEAL               = 0x08000000,              // On heal
    PROC_FLAG_CRIT_HEAL          = 0x10000000,              // On critical healing effect
    PROC_FLAG_HEALED             = 0x20000000,              // On healing
    PROC_FLAG_TARGET_BLOCK       = 0x40000000,              // On enemy blocks
    PROC_FLAG_MISS               = 0x80000000               // On miss melee attack
};

struct SpellProcEventEntry
{
    uint32      schoolMask;                                 // if nonzero - bit mask for matching proc condition based on spell candidate's school: Fire=2, Mask=1<<(2-1)=2
    uint32      category;                                   // if nonzero - match proc condition based on candidate spell's category
    uint32      skillId;                                    // if nonzero - for matching proc condition based on candidate spell's skillId from SkillLineAbility.dbc (Shadow Bolt = Destruction)
    uint32      spellFamilyName;                            // if nonzero - for matching proc condition based on candidate spell's SpellFamilyNamer value
    uint64      spellFamilyMask;                            // if nonzero - for matching proc condition based on candidate spell's SpellFamilyFlags (like auras 107 and 108 do)
    uint32      procFlags;                                  // bitmask for matching proc event
    float       ppmRate;                                    // for melee (ranged?) damage spells - proc rate per minute. if zero, falls back to flat chance from Spell.dbc
};

typedef HM_NAMESPACE::hash_map<uint32, SpellProcEventEntry> SpellProcEventMap;

// Spell script target related declarations (accessed using SpellMgr functions)
enum SpellTargetType
{
    SPELL_TARGET_TYPE_GAMEOBJECT = 0,
    SPELL_TARGET_TYPE_CREATURE   = 1,
    SPELL_TARGET_TYPE_DEAD       = 2
};

#define MAX_SPELL_TARGET_TYPE 3

struct SpellTargetEntry
{
    SpellTargetEntry(SpellTargetType type_,uint32 targetEntry_) : type(type_), targetEntry(targetEntry_) {}
    SpellTargetType type;
    uint32 targetEntry;
};

typedef std::multimap<uint32,SpellTargetEntry> SpellScriptTarget;

// Spell teleports (accessed using SpellMgr functions)
struct SpellTeleport
{
    uint32 target_mapId;
    float  target_X;
    float  target_Y;
    float  target_Z;
    float  target_Orientation;
};

typedef HM_NAMESPACE::hash_map<uint32, SpellTeleport> SpellTeleportMap;

// Spell rank chain  (accessed using SpellMgr functions)
struct SpellChainNode
{
    uint32 prev;
    uint32 first;
    uint8  rank;
};

typedef HM_NAMESPACE::hash_map<uint32, SpellChainNode> SpellChainMap;

// Spell learning properties (accessed using SpellMgr functions)
struct SpellLearnSkillNode
{
    uint32 skill;
    uint32 value;                                           // 0  - max skill value for player level
    uint32 maxvalue;                                        // 0  - max skill value for player level
};

typedef std::map<uint32, SpellLearnSkillNode> SpellLearnSkillMap;

struct SpellLearnSpellNode
{
    uint32 spell;
    uint32 ifNoSpell;
    uint32 autoLearned;
};

typedef std::multimap<uint32, SpellLearnSpellNode> SpellLearnSpellMap;

class SpellMgr
{
    // Constructors
    public:
        SpellMgr();
        ~SpellMgr();

    // Accessors (const or static functions)
    public:
        // Spell affects
        SpellAffection const* GetSpellAffection(uint16 spellId, uint8 effectId) const
        {
            SpellAffectMap::const_iterator itr = mSpellAffectMap.find((spellId<<8) + effectId);
            if( itr != mSpellAffectMap.end( ) )
                return &itr->second;
            return NULL;
        }

        bool IsAffectedBySpell(SpellEntry const *spellInfo, uint32 spellId, uint8 effectId, uint64 const& familyFlags) const;

        // Spell proc events
        SpellProcEventEntry const* GetSpellProcEvent(uint32 spellId) const
        {
            SpellProcEventMap::const_iterator itr = mSpellProcEventMap.find(spellId);
            if( itr != mSpellProcEventMap.end( ) )
                return &itr->second;
            return NULL;
        }

        static bool IsSpellProcEventCanTriggeredBy( SpellProcEventEntry const * spellProcEvent, SpellEntry const * procSpell, uint32 procFlags );

        // Spell teleports
        SpellTeleport const* GetSpellTeleport(uint32 spell_id) const
        {
            SpellTeleportMap::const_iterator itr = mSpellTeleports.find( spell_id );
            if( itr != mSpellTeleports.end( ) )
                return &itr->second;
            return NULL;
        }

        // Spell ranks chains
        uint32 GetFirstSpellInChain(uint32 spell_id) const
        {
            SpellChainMap::const_iterator itr = mSpellChains.find(spell_id);
            if(itr == mSpellChains.end())
                return spell_id;

            return itr->second.first;
        }

        uint32 GetPrevSpellInChain(uint32 spell_id) const
        {
            SpellChainMap::const_iterator itr = mSpellChains.find(spell_id);
            if(itr == mSpellChains.end())
                return 0;

            return itr->second.prev;
        }

        uint8 GetSpellRank(uint32 spell_id) const
        {
            SpellChainMap::const_iterator itr = mSpellChains.find(spell_id);
            if(itr == mSpellChains.end())
                return 0;

            return itr->second.rank;
        }

        uint32 GetLastSpellInChain(uint32 spell_id) const
        {
            // fast check non ranked spell
            SpellChainMap::const_iterator spell_itr = mSpellChains.find(spell_id);
            if(spell_itr == mSpellChains.end())
                return 0;

            for(SpellChainMap::const_iterator itr = mSpellChains.begin(); itr != mSpellChains.end(); ++itr)
            {
                if(itr->second.first==spell_itr->second.first && itr->second.rank > spell_itr->second.rank)
                    spell_itr = itr;
            }

            return spell_itr->first;
        }

        bool IsRankSpellDueToSpell(SpellEntry const *spellInfo_1,uint32 spellId_2) const;
        static bool canStackSpellRanks(SpellEntry const *spellInfo,SpellEntry const *spellInfo2);
        bool IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2) const;

        SpellEntry const* SelectAuraRankForPlayerLevel(SpellEntry const* spellInfo, uint32 playerLevel) const;

        // Spell learning
        SpellLearnSkillNode const* GetSpellLearnSkill(uint32 spell_id) const
        {
            SpellLearnSkillMap::const_iterator itr = mSpellLearnSkills.find(spell_id);
            if(itr != mSpellLearnSkills.end())
                return &itr->second;
            else
                return NULL;
        }

        bool IsSpellLearnSpell(uint32 spell_id) const
        {
            return mSpellLearnSpells.count(spell_id)!=0;
        }

        SpellLearnSpellMap::const_iterator GetBeginSpellLearnSpell(uint32 spell_id) const
        {
            return mSpellLearnSpells.lower_bound(spell_id);
        }

        SpellLearnSpellMap::const_iterator GetEndSpellLearnSpell(uint32 spell_id) const
        {
            return mSpellLearnSpells.upper_bound(spell_id);
        }

        bool IsSpellLearnToSpell(uint32 spell_id1,uint32 spell_id2) const
        {
            SpellLearnSpellMap::const_iterator b = GetBeginSpellLearnSpell(spell_id1);
            SpellLearnSpellMap::const_iterator e = GetEndSpellLearnSpell(spell_id1);
            for(SpellLearnSpellMap::const_iterator i = b; i != e; ++i)
                if(i->second.spell==spell_id2)
                    return true;
            return false;
        }

        static bool IsProfessionSpell(uint32 spellId);
        static bool IsPrimaryProfessionSpell(uint32 spellId);
        bool IsPrimaryProfessionFirstRankSpell(uint32 spellId) const;

        // Spell script targets
        SpellScriptTarget::const_iterator GetBeginSpellScriptTarget(uint32 spell_id) const
        {
            return mSpellScriptTarget.lower_bound(spell_id);
        }

        SpellScriptTarget::const_iterator GetEndSpellScriptTraget(uint32 spell_id) const
        {
            return mSpellScriptTarget.upper_bound(spell_id);
        }


        // Spell correctess for client using
        static bool IsSpellValid(SpellEntry const * spellInfo, Player* pl = NULL, bool msg = true);

    // Modifiers
    public:
        static SpellMgr& Instance();

        // Loading data at server startup
        void LoadSpellChains();
        void LoadSpellLearnSkills();
        void LoadSpellLearnSpells();
        void LoadSpellScriptTarget();
        void LoadSpellAffects();
        void LoadSpellProcEvents();
        void LoadSpellTeleports();
        void LoadSpellThreats();

    private:
        SpellScriptTarget  mSpellScriptTarget;
        SpellChainMap      mSpellChains;
        SpellLearnSkillMap mSpellLearnSkills;
        SpellLearnSpellMap mSpellLearnSpells;
        SpellTeleportMap   mSpellTeleports;
        SpellAffectMap     mSpellAffectMap;
        SpellProcEventMap  mSpellProcEventMap;
};

#define spellmgr SpellMgr::Instance()

#endif
