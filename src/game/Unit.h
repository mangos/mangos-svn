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

#ifndef __UNIT_H
#define __UNIT_H

#include "Common.h"
#include "Object.h"
#include "Opcodes.h"
#include "Mthread.h"
#include "SpellAuraDefines.h"
#include "Util.h"
#include "UpdateFields.h"
#include "SharedDefines.h"
#include "ThreatManager.h"
#include "HostilRefManager.h"
#include "FollowerReference.h"
#include "FollowerRefManager.h"
#include <list>

// Passive Spell codes explicit used in code
#define SPELL_PASSIVE_BATTLE_STANCE            2457
#define SPELL_PASSIVE_RESURRECTION_SICKNESS   15007
/*#define SPELL_PASSIVE_ENDURENCE               20550
#define SPELL_PASSIVE_THROWING_SPECIALIZATION 20558
#define SPELL_PASSIVE_AXE_SPECIALIZATION      20574
#define SPELL_PASSIVE_SHADOW_RESISTANCE       20579
#define SPELL_PASSIVE_NATURE_RESISTANCE       20583
#define SPELL_PASSIVE_EXPANSIVE_MIND          20591
#define SPELL_PASSIVE_ARCANE_RESISTANCE       20592
#define SPELL_PASSIVE_GUN_SPECIALIZATION      20595
#define SPELL_PASSIVE_FROST_RESISTANCE        20596
#define SPELL_PASSIVE_SWORD_SPECIALIZATION    20597
#define SPELL_PASSIVE_HUMAN_SPIRIT            20598
#define SPELL_PASSIVE_DIPLOMACY               20599
#define SPELL_PASSIVE_MACE_SPECIALIZATION     20864
#define SPELL_PASSIVE_BOW_SPECIALIZATION      26290

// Horde Racial Passives
#define SPELL_HORDE_PASSIVE_NATURE_RESISTANCE 20551*/

#define BASE_MINDAMAGE 1.0f
#define BASE_MAXDAMAGE 2.0f
#define BASE_ATTACK_TIME 2000

enum ShapeshiftForm
{
    FORM_CAT                = 1,
    FORM_TREE               = 2,
    FORM_TRAVEL             = 3,
    FORM_AQUA               = 4,
    FORM_BEAR               = 5,
    FORM_AMBIENT            = 6,
    FORM_GHOUL              = 7,
    FORM_DIREBEAR           = 8,
    FORM_CREATUREBEAR       = 14,
    FORM_CREATURECAT        = 15,
    FORM_GHOSTWOLF          = 16,
    FORM_BATTLESTANCE       = 17,
    FORM_DEFENSIVESTANCE    = 18,
    FORM_BERSERKERSTANCE    = 19,
    FORM_SWIFT_FLIGHT       = 27,
    FORM_SHADOW             = 28,
    FORM_FLIGHT             = 29,
    FORM_STEALTH            = 30,
    FORM_MOONKIN            = 31,
    FORM_SPIRITOFREDEMPTION = 32
};

#define CREATURE_MAX_SPELLS     4
#define PLAYER_MAX_SKILLS       127

#define PLAYER_SKILL(x)         (PLAYER_SKILL_INFO_1_1 + ((x)*3))
// DWORD definitions gathered from windows api
#define SKILL_VALUE(x)          uint16(x)
#define SKILL_MAX(x)            uint16((uint32(x) >> 16))
#define MAKE_SKILL_VALUE(v, m) ((uint32)(((uint16)(v)) | (((uint32)((uint16)(m))) << 16)))

enum Swing
{
    NOSWING                    = 0,
    SINGLEHANDEDSWING          = 1,
    TWOHANDEDSWING             = 2
};

enum VictimState
{
    VICTIMSTATE_UNKNOWN1       = 0,
    VICTIMSTATE_NORMAL         = 1,
    VICTIMSTATE_DODGE          = 2,
    VICTIMSTATE_PARRY          = 3,
    VICTIMSTATE_UNKNOWN2       = 4,
    VICTIMSTATE_BLOCKS         = 5,
    VICTIMSTATE_EVADES         = 6,
    VICTIMSTATE_IS_IMMUNE      = 7,
    VICTIMSTATE_DEFLECTS       = 8
};

enum HitInfo
{
    HITINFO_NORMALSWING        = 0x00,
    HITINFO_NORMALSWING2       = 0x02,
    HITINFO_LEFTSWING          = 0x04,
    HITINFO_MISS               = 0x10,
    HITINFO_ABSORB             = 0x20,                      // plays absorb sound
    HITINFO_RESIST             = 0x40,                      // resisted atleast some damage
    HITINFO_CRITICALHIT        = 0x80,
    HITINFO_GLANCING           = 0x4000,
    HITINFO_CRUSHING           = 0x8000,
    HITINFO_NOACTION           = 0x10000,
    HITINFO_SWINGNOHITSOUND    = 0x80000
};

//i would like to remove this: (it is defined in item.h
enum InventorySlot
{
    NULL_BAG                   = 0,
    NULL_SLOT                  = 255
};

struct FactionTemplateEntry;
struct Modifier;
struct SpellEntry;
struct SpellEntryExt;

class Aura;
class Creature;
class Spell;
class DynamicObject;
class GameObject;
class Item;
class Pet;

struct DamageShield
{
    uint32 m_spellId;
    uint32 m_damage;
    uint64 m_caster_guid;
};

struct SpellImmune
{
    uint32 type;
    uint32 spellId;
};

typedef std::list<SpellImmune> SpellImmuneList;

enum UnitModifierType
{
    BASE_VALUE = 0,
    BASE_PCT = 1,
    TOTAL_VALUE = 2,
    TOTAL_PCT = 3,
    MODIFIER_TYPE_END = 4
};

enum WeaponDamageRange
{
    MINDAMAGE,
    MAXDAMAGE
};

enum DamageTypeToSchool
{
    RESISTANCE,
    DAMAGE_DEALT,
    DAMAGE_TAKEN
};

enum UnitMods
{
    UNIT_MOD_STAT_STRENGTH,                                 // UNIT_MOD_STAT_STRENGTH..UNIT_MOD_STAT_SPIRIT must be in existed order, it's accessed by index values of Stats enum.
    UNIT_MOD_STAT_AGILITY,
    UNIT_MOD_STAT_STAMINA,
    UNIT_MOD_STAT_INTELLECT,
    UNIT_MOD_STAT_SPIRIT,
    UNIT_MOD_HEALTH,
    UNIT_MOD_MANA,                                          // UNIT_MOD_MANA..UNIT_MOD_HAPPINESS must be in existed order, it's accessed by index values of Powers enum.
    UNIT_MOD_RAGE,
    UNIT_MOD_FOCUS,
    UNIT_MOD_ENERGY,
    UNIT_MOD_HAPPINESS,
    UNIT_MOD_ARMOR,                                         // UNIT_MOD_ARMOR..UNIT_MOD_RESISTANCE_ARCANE must be in existed order, it's accessed by index values of SpellSchools enum.
    UNIT_MOD_RESISTANCE_HOLY,
    UNIT_MOD_RESISTANCE_FIRE,
    UNIT_MOD_RESISTANCE_NATURE,
    UNIT_MOD_RESISTANCE_FROST,
    UNIT_MOD_RESISTANCE_SHADOW,
    UNIT_MOD_RESISTANCE_ARCANE,
    UNIT_MOD_ATTACK_POWER,
    UNIT_MOD_ATTACK_POWER_RANGED,
    UNIT_MOD_DAMAGE_MAINHAND,
    UNIT_MOD_DAMAGE_OFFHAND,
    UNIT_MOD_DAMAGE_RANGED,
    UNIT_MOD_END,
    // synonyms
    UNIT_MOD_STAT_START = UNIT_MOD_STAT_STRENGTH,
    UNIT_MOD_STAT_END = UNIT_MOD_STAT_SPIRIT + 1,
    UNIT_MOD_RESISTANCE_START = UNIT_MOD_ARMOR,
    UNIT_MOD_RESISTANCE_END = UNIT_MOD_RESISTANCE_ARCANE + 1,
    UNIT_MOD_POWER_START = UNIT_MOD_MANA,
    UNIT_MOD_POWER_END = UNIT_MOD_HAPPINESS + 1
};

enum BaseModGroup
{
    CRIT_PERCENTAGE,
    BLOCK_PERCENTAGE,
    DODGE_PERCENTAGE,
    PARRY_PERCENTAGE,
    RANGED_CRIT_PERCENTAGE,
    OFFHAND_CRIT_PERCENTAGE,
    SPELL_CRIT_PERCENTAGE,
    HOLY_SPELL_CRIT_PERCENTAGE,
    FIRE_SPELL_CRIT_PERCENTAGE,
    NATURE_SPELL_CRIT_PERCENTAGE,
    FROST_SPELL_CRIT_PERCENTAGE,
    SHADOW_SPELL_CRIT_PERCENTAGE,
    ARCANE_SPELL_CRIT_PERCENTAGE,
    SHIELD_BLOCK_VALUE,
    BASEMOD_END
};

enum BaseModType
{
    FLAT_MOD,
    PCT_MOD,
    MOD_END
};

enum DeathState
{
    ALIVE       = 0,
    JUST_DIED   = 1,
    CORPSE      = 2,
    DEAD        = 3
};

enum UnitState
{
    UNIT_STAT_STOPPED       = 0,
    UNIT_STAT_DIED          = 1,
    UNIT_STAT_ATTACKING     = 2,                            // player is attacking someone
    UNIT_STAT_ATTACK_BY     = 4,                            // player is attack by someone
                                                            // player is in combat mode
    UNIT_STAT_IN_COMBAT     = (UNIT_STAT_ATTACKING | UNIT_STAT_ATTACK_BY),
    UNIT_STAT_STUNDED       = 8,
    UNIT_STAT_ROAMING       = 16,
    UNIT_STAT_CHASE         = 32,
    UNIT_STAT_SEARCHING     = 64,
    UNIT_STAT_FLEEING       = 128,
    UNIT_STAT_MOVING        = (UNIT_STAT_ROAMING | UNIT_STAT_CHASE | UNIT_STAT_SEARCHING | UNIT_STAT_FLEEING),
    UNIT_STAT_IN_FLIGHT     = 256,                          // player is in flight mode
    UNIT_STAT_FOLLOW        = 512,
    UNIT_STAT_ROOT          = 1024,
    UNIT_STAT_CONFUSED      = 2048,
    UNIT_STAT_ALL_STATE     = 0xffff                        //(UNIT_STAT_STOPPED | UNIT_STAT_MOVING | UNIT_STAT_IN_COMBAT | UNIT_STAT_IN_FLIGHT)
};

enum UnitMoveType
{
    MOVE_WALK       = 0,
    MOVE_RUN        = 1,
    MOVE_WALKBACK   = 2,
    MOVE_SWIM       = 3,
    MOVE_SWIMBACK   = 4,
    MOVE_TURN       = 5,
    MOVE_FLY        = 6,
    MOVE_FLYBACK    = 7
};

#define MAX_MOVE_TYPE 8

extern float baseMoveSpeed[MAX_MOVE_TYPE];

enum WeaponAttackType
{
    BASE_ATTACK   = 0,
    OFF_ATTACK    = 1,
    RANGED_ATTACK = 2
};

enum DamageEffectType
{
    DIRECT_DAMAGE           = 0,
    SPELL_DIRECT_DAMAGE     = 1,
    DOT                     = 2,
    HEAL                    = 3,
    NODAMAGE                = 4,                            // used also in case when damage applied to health but not applied to spell channelInterruptFlags/etc
    SELF_DAMAGE             = 5
};

enum UnitVisibility
{
    VISIBILITY_OFF                = 0,                         // absolute, not detectable, GM-like, can see all other
    VISIBILITY_ON                 = 1,
    VISIBILITY_GROUP_STEALTH      = 2,                         // detect chance, seen and can see group members
    VISIBILITY_GROUP_INVISIBILITY = 3,                         // invisibility, can see and can be seen only another invisible unit or invisible detection unit
    VISIBILITY_GROUP_NO_DETECT    = 4                          // state just at stealth apply for update Grid state
};

// Value masks for UNIT_FIELD_FLAGS
enum UnitFlags
{
    UNIT_FLAG_UNKNOWN7       = 0x00000001,
    UNIT_FLAG_UNKNOWN8       = 0x00000002,                  // not attackable
    UNIT_FLAG_DISABLE_MOVE   = 0x00000004,
    UNIT_FLAG_UNKNOWN1       = 0x00000008,                  // for all units, make unit attackable even it's friendly in some cases...
    UNIT_FLAG_RENAME         = 0x00000010,
    UNIT_FLAG_RESTING        = 0x00000020,
    UNIT_FLAG_UNKNOWN9       = 0x00000040,
    UNIT_FLAG_UNKNOWN10      = 0x00000080,
    UNIT_FLAG_UNKNOWN2       = 0x00000100,                  // 2.0.8
    UNIT_FLAG_UNKNOWN11      = 0x00000200,
    UNIT_FLAG_UNKNOWN12      = 0x00000400,                  // loot animation
    UNIT_FLAG_UNKNOWN3       = 0x00000800,                  // in combat?, 2.0.8
    UNIT_FLAG_PVP            = 0x00001000,                  // ok
    UNIT_FLAG_MOUNT          = 0x00002000,                  // silenced, 2.1.1
    UNIT_FLAG_UNKNOWN4       = 0x00004000,                  // 2.0.8
    UNIT_FLAG_UNKNOWN13      = 0x00008000,
    UNIT_FLAG_UNKNOWN14      = 0x00010000,
    UNIT_FLAG_PACIFIED       = 0x00020000,
    UNIT_FLAG_DISABLE_ROTATE = 0x00040000,                  // stunned, 2.1.1
    UNIT_FLAG_IN_COMBAT      = 0x00080000,
    UNIT_FLAG_UNKNOWN15      = 0x00100000,
    UNIT_FLAG_DISARMED       = 0x00200000,                  // disable melee spells casting..., "Required melee weapon" added to melee spells tooltip.
    UNIT_FLAG_CONFUSED       = 0x00400000,
    UNIT_FLAG_FLEEING        = 0x00800000,
    UNIT_FLAG_UNKNOWN5       = 0x01000000,                  // used in spell Eyes of the Beast for pet...
    UNIT_FLAG_NOT_SELECTABLE = 0x02000000,                  // ok
    UNIT_FLAG_SKINNABLE      = 0x04000000,
    UNIT_FLAG_UNKNOWN16      = 0x08000000,
    UNIT_FLAG_UNKNOWN17      = 0x10000000,
    UNIT_FLAG_UNKNOWN6       = 0x20000000,                  // used in Feing Death spell
    UNIT_FLAG_SHEATHE        = 0x40000000
};

enum TempSummonType
{
    TEMPSUMMON_TIMED_OR_DEAD_DESPAWN       = 1,             // despawns after a specified time OR when the creature disappears
    TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN     = 2,             // despawns after a specified time OR when the creature dies
    TEMPSUMMON_TIMED_DESPAWN               = 3,             // despawns after a specified time
    TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT = 4,             // despawns after a specified time after the creature is out of combat
    TEMPSUMMON_CORPSE_DESPAWN              = 5,             // despawns instantly after death
    TEMPSUMMON_CORPSE_TIMED_DESPAWN        = 6,             // despawns after a specified time after death
    TEMPSUMMON_DEAD_DESPAWN                = 7,             // despawns when the creature disappears
    TEMPSUMMON_MANUAL_DESPAWN              = 8              // despawns when UnSummon() is called        
};

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
    PROC_FLAG_CAST_SPELL         = 0x00004000,              // On cast spell (and broken Aspect of Hawk)
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

enum AuraState
{
    AURA_STATE_DEFENSE        = 1,
    AURA_STATE_HEALTHLESS     = 2,
    AURA_STATE_RACE           = 3,
    AURA_STATE_UNKNOWN1       = 4,
    AURA_STATE_JUDGEMENT      = 5,
    AURA_STATE_UNKNOWN2       = 6,
    AURA_STATE_PARRY          = 7,                           // unsure
    AURA_STATE_UNKNOWN3       = 8,
    AURA_STATE_UNKNOWN4       = 9,
    AURA_STATE_UNKNOWN5       = 10,
    AURA_STATE_CRIT           = 11,
    AURA_STATE_UNKNOWN6       = 12,
    AURA_STATE_UNKNOWN7       = 13,
    AURA_STATE_IMMOLATE       = 14
};

enum Mechanics
{
    MECHANIC_CHARM            = 1,
    MECHANIC_CONFUSED         = 2,
    MECHANIC_DISARM           = 3,
    MECHANIC_ATTRACT          = 4,
    MECHANIC_FEAR             = 5,
    MECHANIC_STUPID           = 6,
    MECHANIC_ROOT             = 7,
    MECHANIC_PEACE            = 8,
    MECHANIC_SILENCE          = 9,
    MECHANIC_SLEEP            = 10,
    MECHANIC_CHASE            = 11,
    MECHANIC_STUNDED          = 12,
    MECHANIC_FREEZE           = 13,
    MECHANIC_KNOCKOUT         = 14,
    MECHANIC_BLEED            = 15,
    MECHANIC_HEAL             = 16,
    MECHANIC_POLYMORPH        = 17,
    MECHANIC_BANISH           = 18,
    MECHANIC_SHIELDED         = 19,
    MECHANIC_DURANCED         = 20,
    MECHANIC_MOUNT            = 21,
    MECHANIC_PERSUADED        = 22,
    MECHANIC_TURNED           = 23,
    MECHANIC_HORROR           = 24,
    MECHANIC_INVULNERABILITY  = 25,
    MECHANIC_INTERRUPTED      = 26,
    MECHANIC_DAZED            = 27,
    MECHANIC_DISCOVERY        = 28
};

//To all Immune system,if target has immunes,
//some spell that related to ImmuneToDispel or ImmuneToSchool or ImmuneToDamage type can't cast to it,
//some spell_effects that related to ImmuneToEffect<effect>(only this effect in the spell) can't cast to it,
//some aura(related to Mechanics or ImmuneToState<aura>) can't apply to it.
enum SpellImmunity
{
    IMMUNITY_EFFECT                = 0,
    IMMUNITY_STATE                 = 1,
    IMMUNITY_SCHOOL                = 2,
    IMMUNITY_DAMAGE                = 3,
    IMMUNITY_DISPEL                = 4,
    IMMUNITY_MECHANIC              = 5
};

enum DiminishingMechanics
{
    DIMINISHING_NONE                = 0,
    DIMINISHING_MECHANIC_CONFUSE    = 1,                    // incapacitate, confuse
    DIMINISHING_MECHANIC_CHARM      = 2,                    // fear, mind control, sleep
    DIMINISHING_MECHANIC_STUN       = 3,                    // stun
    DIMINISHING_MECHANIC_ROOT       = 4,                    // roots, freeze
    DIMINISHING_MECHANIC_SPEED      = 5                     // speed reduction
};

enum DiminishingLevels
{
    DIMINISHING_LEVEL_1             = 0,
    DIMINISHING_LEVEL_2             = 1,
    DIMINISHING_LEVEL_3             = 2,
    DIMINISHING_LEVEL_IMMUNE        = 3
};

struct DiminishingReturn
{
    DiminishingReturn(DiminishingMechanics mech, uint32 t, uint32 count) : Mechanic(mech), hitTime(t), hitCount(count) {}

    DiminishingMechanics    Mechanic;
    uint32                  hitTime;
    uint32                  hitCount;
};

enum ImmuneToDispel
{
    IMMUNE_DISPEL_MAGIC        = 1,
    IMMUNE_DISPEL_CURSE        = 2,
    IMMUNE_DISPEL_DISEASE      = 3,
    IMMUNE_DISPEL_POISON       = 4,
    IMMUNE_DISPEL_STEALTH      = 5,
    IMMUNE_DISPEL_INVISIBILITY = 6,
    IMMUNE_DISPEL_ALL          = 7,
    IMMUNE_DISPEL_SPE_NPC_ONLY = 8,
    IMMUNE_DISPEL_CRAZY        = 9,
    IMMUNE_DISPEL_ZG_TICKET    = 10
};

enum ImmuneToDamage
{
    IMMUNE_DAMAGE_PHYSICAL     = 1,
    IMMUNE_DAMAGE_MAGIC        = 126
};

enum ImmuneToSchool
{
    IMMUNE_SCHOOL_PHYSICAL     = 1,
    IMMUNE_SCHOOL_HOLY         = 2,
    IMMUNE_SCHOOL_FIRE         = 4,
    IMMUNE_SCHOOL_NATURE       = 8,
    IMMUNE_SCHOOL_FROST        = 16,
    IMMUNE_SCHOOL_SHADOW       = 32,
    IMMUNE_SCHOOL_ARCANE       = 64,
    IMMUNE_SCHOOL_MAGIC        = 126
};

inline SpellSchools immuneToSchool(ImmuneToSchool immune)
{
    switch(immune)
    {
        case IMMUNE_SCHOOL_PHYSICAL: return SPELL_SCHOOL_NORMAL;
        case IMMUNE_SCHOOL_HOLY:     return SPELL_SCHOOL_HOLY;
        case IMMUNE_SCHOOL_FIRE:     return SPELL_SCHOOL_FIRE;
        case IMMUNE_SCHOOL_NATURE:   return SPELL_SCHOOL_NATURE;
        case IMMUNE_SCHOOL_FROST:    return SPELL_SCHOOL_FROST;
        case IMMUNE_SCHOOL_SHADOW:   return SPELL_SCHOOL_SHADOW;
        case IMMUNE_SCHOOL_ARCANE:   return SPELL_SCHOOL_ARCANE;
        case IMMUNE_SCHOOL_MAGIC:
        default:
            break;
    }
    assert(false);
    return SPELL_SCHOOL_NORMAL;
}

enum MeleeHitOutcome
{
    MELEE_HIT_MISS, MELEE_HIT_DODGE, MELEE_HIT_BLOCK, MELEE_HIT_PARRY, MELEE_HIT_GLANCING,
    MELEE_HIT_CRIT, MELEE_HIT_CRUSHING, MELEE_HIT_NORMAL
};
struct CleanDamage
{
    CleanDamage(uint32 _damage, WeaponAttackType _attackType, MeleeHitOutcome _hitOutCome) : 
        damage(_damage), attackType(_attackType), hitOutCome(_hitOutCome) {}

    uint32 damage;
    WeaponAttackType attackType;
    MeleeHitOutcome hitOutCome; 
};

struct UnitActionBarEntry
{
	uint32 Type;
	uint32 SpellOrAction;
};

// delay time next attack to privent client attack animanation problems
#define ATTACK_DISPLAY_DELAY 200

class MANGOS_DLL_SPEC Unit : public WorldObject
{
    public:
        typedef std::set<Unit*> AttackerSet;
        typedef std::pair<uint32, uint32> spellEffectPair;
        typedef std::multimap< spellEffectPair, Aura*> AuraMap;
        typedef std::list<Aura *> AuraList;
        typedef std::list<DiminishingReturn> Diminishing;
        virtual ~Unit ( );

        static DiminishingMechanics Mechanic2DiminishingMechanics(uint32 mech);
        void AddDiminishing(DiminishingMechanics mech, uint32 hitTime, uint32 hitCount);
        DiminishingLevels GetDiminishing(DiminishingMechanics  mech);
        void IncrDiminishing(DiminishingMechanics  mech, uint32 duration);
        void UpdateDiminishingTime(DiminishingMechanics  mech);
        void ApplyDiminishingToDuration(DiminishingMechanics  mech, int32& duration);

        virtual void Update( uint32 time );

        void setAttackTimer(WeaponAttackType type, uint32 time) { m_attackTimer[type] = time; }
        void resetAttackTimer(WeaponAttackType type = BASE_ATTACK);
        uint32 getAttackTimer(WeaponAttackType type) const { return m_attackTimer[type]; }
        bool isAttackReady(WeaponAttackType type = BASE_ATTACK) const { return m_attackTimer[type] == 0; }
        bool haveOffhandWeapon() const;
        bool canReachWithAttack(Unit *pVictim) const;

        void _addAttacker(Unit *pAttacker)                  // must be called only from Unit::Attack(Unit*)
        {
            AttackerSet::iterator itr = m_attackers.find(pAttacker);
            if(itr == m_attackers.end())
                m_attackers.insert(pAttacker);
            addUnitState(UNIT_STAT_ATTACK_BY);
            SetInCombat();
        }
        void _removeAttacker(Unit *pAttacker)               // must be called only from Unit::AttackStop()
        {
            AttackerSet::iterator itr = m_attackers.find(pAttacker);
            if(itr != m_attackers.end())
                m_attackers.erase(itr);

            if (m_attackers.size() == 0)
            {
                clearUnitState(UNIT_STAT_ATTACK_BY);
                if(!m_attacking)
                    ClearInCombat();
            }
        }
        Unit * getAttackerForHelper()                       // If someone wants to help, who to give them
        {
            if (getVictim() != NULL)
                return getVictim();

            if (m_attackers.size() > 0)
                return *(m_attackers.begin());

            return NULL;
        }
        bool Attack(Unit *victim, bool playerMeleeAttack = false);
        bool AttackStop();
        void RemoveAllAttackers();
        AttackerSet const& getAttackers() const { return m_attackers; }
        bool isAttackingPlayer() const;
        Unit* getVictim() const { return m_attacking; }
        void CombatStop(bool force = false) { AttackStop(); RemoveAllAttackers(); ClearInCombat(force); }

        void addUnitState(uint32 f) { m_state |= f; };
        bool hasUnitState(const uint32 f) const { return (m_state & f); }
        void clearUnitState(uint32 f) { m_state &= ~f; };
        bool CanFreeMove() const
        {
            return !hasUnitState(UNIT_STAT_CONFUSED | UNIT_STAT_FLEEING | UNIT_STAT_IN_FLIGHT | 
                UNIT_STAT_ROOT | UNIT_STAT_STUNDED ) && GetOwnerGUID()==0;
        }

        uint32 getLevel() const { return GetUInt32Value(UNIT_FIELD_LEVEL); };
        void SetLevel(uint32 lvl) { SetUInt32Value(UNIT_FIELD_LEVEL,lvl); }
        uint8 getRace() const { return (uint8)(GetUInt32Value(UNIT_FIELD_BYTES_0) & 0xFF); };
        uint32 getRaceMask() const { return 1 << (getRace()-1); };
        uint8 getClass() const { return (uint8)((GetUInt32Value(UNIT_FIELD_BYTES_0) >> 8) & 0xFF); };
        uint32 getClassMask() const { return 1 << (getClass()-1); };
        uint8 getGender() const { return (uint8)((GetUInt32Value(UNIT_FIELD_BYTES_0) >> 16) & 0xFF); };

        float GetStat(Stats stat) const { return float(GetUInt32Value(UNIT_FIELD_STAT0+stat)); }
        void SetStat(Stats stat, int32 val) { SetStatInt32Value(UNIT_FIELD_STAT0+stat, val); }

        uint32 GetArmor() const { return GetResistance(SPELL_SCHOOL_NORMAL) ; }
        void SetArmor(int32 val) { SetResistance(SPELL_SCHOOL_NORMAL, val); }

        uint32 GetResistance(SpellSchools school) const { return GetUInt32Value(UNIT_FIELD_RESISTANCES+school); }
        void SetResistance(SpellSchools school, int32 val) { SetStatInt32Value(UNIT_FIELD_RESISTANCES+school,val); }

        uint32 GetHealth()    const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
        uint32 GetMaxHealth() const { return GetUInt32Value(UNIT_FIELD_MAXHEALTH); }
        void SetHealth(   uint32 val);
        void SetMaxHealth(uint32 val);
        int32 ModifyHealth(int32 val);

        Powers getPowerType() const { return Powers((GetUInt32Value(UNIT_FIELD_BYTES_0) >> 24) & 0xFF); };
        void setPowerType(Powers power);
        uint32 GetPower(   Powers power) const { return GetUInt32Value(UNIT_FIELD_POWER1   +power); }
        uint32 GetMaxPower(Powers power) const { return GetUInt32Value(UNIT_FIELD_MAXPOWER1+power); }
        void SetPower(   Powers power, uint32 val);
        void SetMaxPower(Powers power, uint32 val);
        int32 ModifyPower(Powers power, int32 val);
        void ApplyPowerMod(Powers power, uint32 val, bool apply) { ApplyModUInt32Value(UNIT_FIELD_POWER1+power, val, apply); }
        void ApplyMaxPowerMod(Powers power, uint32 val, bool apply) { ApplyModUInt32Value(UNIT_FIELD_MAXPOWER1+power, val, apply); }

        uint32 GetAttackTime(WeaponAttackType att) const { return (uint32)(GetFloatValue(UNIT_FIELD_BASEATTACKTIME+att)/m_modAttackSpeedPct[att]); } 
        void SetAttackTime(WeaponAttackType att, uint32 val) { SetFloatValue(UNIT_FIELD_BASEATTACKTIME+att,val*m_modAttackSpeedPct[att]); }
        void ApplyAttackTimePercentMod(WeaponAttackType att,float val, bool apply) { ApplyPercentModFloatVar(m_modAttackSpeedPct[att], val, !apply); ApplyPercentModFloatValue(UNIT_FIELD_BASEATTACKTIME+att,val,!apply);}

        // faction template id
        uint32 getFaction() const { return GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE); }
        void setFaction(uint32 faction) { SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, faction ); }
        FactionTemplateEntry const* getFactionTemplateEntry() const;
        bool IsHostileTo(Unit const* unit) const;
        bool IsHostileToPlayer() const;
        bool IsFriendlyTo(Unit const* unit) const;
        bool IsNeutralToAll() const;
        bool IsPvP() { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); }
        void SetPvP(bool state) { if(state) SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); else RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); }
        uint32 GetCreatureTypeMask() const;

        uint8 getStandState() const { return (uint8)(GetUInt32Value(UNIT_FIELD_BYTES_1) & 0xFF); };

        bool IsMounted() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT ); }
        uint32 GetMountID() const { return GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }
        void Mount(uint32 mount, bool taxi = false);
        void Unmount();

        uint16 GetMaxSkillValueForLevel() const { return getLevel()*5; }
        void DealDamage(Unit *pVictim, uint32 damage, CleanDamage const* cleanDamage, DamageEffectType damagetype, uint8 damageSchool, SpellEntry const *spellProto, uint32 procFlag, bool durabilityLoss);
        void DealDamageBySchool(Unit *pVictim, SpellEntry const *spellInfo, uint32 *damage, CleanDamage *cleanDamage, bool *crit = false, bool isTriggeredSpell = false);
        void DoAttackDamage(Unit *pVictim, uint32 *damage, CleanDamage *cleanDamage, uint32 *blocked_amount, uint32 *damageType, uint32 *hitInfo, uint32 *victimState, uint32 *absorbDamage, uint32 *resistDamage, WeaponAttackType attType, SpellEntry const *spellCasted = NULL, bool isTriggeredSpell = false);
        void ProcDamageAndSpell(Unit *pVictim, uint32 procAttacker, uint32 procVictim, uint32 damage = 0, SpellEntry const *procSpell = NULL, bool isTriggeredSpell = false, WeaponAttackType attType = BASE_ATTACK);
        void CastMeleeProcDamageAndSpell(Unit* pVictim, uint32 damage, WeaponAttackType attType, MeleeHitOutcome outcome, SpellEntry const *spellCasted = NULL, bool isTriggeredSpell = false);
        void HandleDummyAuraProc(Unit *pVictim, SpellEntry const *spellProto, uint32 effIndex, uint32 damage, Aura* triggredByAura, uint32 procFlag);
        void HandleProcTriggerSpell(Unit *pVictim,uint32 damage, Aura* triggredByAura, SpellEntry const *procSpell, uint32 procFlags);
        void HandleEmoteCommand(uint32 anim_id);
        void AttackerStateUpdate (Unit *pVictim, WeaponAttackType attType = BASE_ATTACK, bool isTriggered = false);
        uint32 SpellMissChanceCalc(Unit *pVictim) const;
        int32 MeleeMissChanceCalc(const Unit *pVictim) const;

        float GetUnitDodgeChance()    const;
        float GetUnitParryChance()    const;
        float GetUnitBlockChance()    const;
        float GetUnitCriticalChance() const { return GetTypeId() == TYPEID_PLAYER ? GetFloatValue( PLAYER_CRIT_PERCENTAGE ) : 5; }

        virtual uint32 GetShieldBlockValue() const =0;
        uint32 GetUnitMeleeSkill() const { return getLevel() * 5; }
        uint16 GetDefenseSkillValue() const;
        uint16 GetPureDefenseSkillValue() const;
        uint16 GetWeaponSkillValue(WeaponAttackType attType) const;
        uint16 GetPureWeaponSkillValue(WeaponAttackType attType) const;
        float GetWeaponProcChance() const;
        float GetPPMProcChance(uint32 WeaponSpeed, float PPM) const;
        MeleeHitOutcome RollPhysicalOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType, SpellEntry const *spellInfo);
        MeleeHitOutcome RollMeleeOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType) const;
        MeleeHitOutcome RollMeleeOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType, int32 crit_chance, int32 miss_chance, int32 hit_chance) const;

        bool isVendor()       const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR ); }
        bool isTrainer()      const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER ); }
        bool isQuestGiver()   const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER ); }
        bool isGossip()       const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP ); }
        bool isTaxi()         const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TAXIVENDOR ); }
        bool isGuildMaster()  const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PETITIONER ); }
        bool isBattleMaster() const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BATTLEFIELDPERSON ); }
        bool isBanker()       const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER ); }
        bool isInnkeeper()    const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER ); }
        bool isSpiritHealer() const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER ); }
        bool isSpiritGuide()  const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITGUIDE ); }
        bool isTabardDesigner()const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDDESIGNER ); }
        bool isAuctioner()    const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER ); }
        bool isArmorer()      const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_ARMORER ); }
        bool isServiceProvider() const
        {
            return HasFlag( UNIT_NPC_FLAGS,
                UNIT_NPC_FLAG_VENDOR | UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_TAXIVENDOR |
                UNIT_NPC_FLAG_PETITIONER | UNIT_NPC_FLAG_BATTLEFIELDPERSON | UNIT_NPC_FLAG_BANKER |
                UNIT_NPC_FLAG_INNKEEPER | UNIT_NPC_FLAG_GUARD | UNIT_NPC_FLAG_SPIRITHEALER |
                UNIT_NPC_FLAG_SPIRITGUIDE | UNIT_NPC_FLAG_TABARDDESIGNER | UNIT_NPC_FLAG_AUCTIONEER );
        }
        bool isSpiritService() const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER | UNIT_NPC_FLAG_SPIRITGUIDE ); }

        //Need fix or use this
        bool isGuard() const  { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GUARD); }

        bool isInFlight()  const { return hasUnitState(UNIT_STAT_IN_FLIGHT); }

        //bool isInCombat()  const { return hasUnitState(UNIT_STAT_IN_COMBAT); }
        bool isInCombat()  const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT); }
        void SetInCombat();
        void ClearInCombat(bool force = false);

        bool isAttacking() const { return hasUnitState(UNIT_STAT_ATTACKING); }
        bool isAttacked()  const { return hasUnitState(UNIT_STAT_ATTACK_BY); }

        bool HasAuraType(uint32 auraType) const;
        bool HasAura(uint32 spellId, uint32 effIndex) const
            { return m_Auras.find(spellEffectPair(spellId, effIndex)) != m_Auras.end(); }

        bool HasStealthAura() const                         // cache this in a bool someday
        {
            return HasAuraType(SPELL_AURA_MOD_STEALTH);
        }
        bool HasInvisibilityAura() const                    // cache this in a bool someday
        {
            return HasAuraType(SPELL_AURA_MOD_INVISIBILITY);
        }
        bool isFeared() const                               // cache this in a bool someday
        {
            return HasAuraType(SPELL_AURA_MOD_FEAR);
        }
        bool isInRoots() const                              // cache this in a bool someday
        {
            return HasAuraType(SPELL_AURA_MOD_ROOT);
        }
        void RemoveSpellbyDamageTaken(uint32 auraType, uint32 damage);

        bool isTargetableForAttack();
        virtual bool IsInWater() const;
        virtual bool IsUnderWater() const;
        bool isInAccessablePlaceFor(Creature* c) const;

        void SendHealSpellOnPlayer(Unit *pVictim, uint32 SpellID, uint32 Damage, bool critical = false);
        void SendHealSpellOnPlayerPet(Unit *pVictim, uint32 SpellID, uint32 Damage,Powers powertype, bool critical = false);
        void PeriodicAuraLog(Unit *pVictim, SpellEntry const *spellProto, Modifier *mod, uint8 effect_idx);
        void SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage, bool isTriggeredSpell = false, bool useSpellDamage = true);
        void CastSpell(Unit* Victim, uint32 spellId, bool triggered, Item *castItem = NULL, Aura* triggredByAura = NULL, uint64 originalCaster = 0);
        void CastSpell(Unit* Victim,SpellEntry const *spellInfo, bool triggered, Item *castItem= NULL, Aura* triggredByAura = NULL, uint64 originalCaster = 0);
        bool IsDamageToThreatSpell(SpellEntry const * spellInfo) const;

        void DeMorph();

        void SendAttackStateUpdate(uint32 HitInfo, Unit *target, uint8 SwingType, uint32 DamageType, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, uint32 TargetState, uint32 BlockedAmount);
        void SendSpellNonMeleeDamageLog(Unit *target,uint32 SpellID,uint32 Damage, uint8 DamageType,uint32 AbsorbedDamage, uint32 Resist,bool PhysicalDamage, uint32 Blocked, bool CriticalHit = false);

        void SendMonsterMove(float NewPosX, float NewPosY, float NewPosZ, uint8 type, bool Run, uint32 Time);

        virtual void MoveOutOfRange(Player &) {  };

        bool isAlive() const { return (m_deathState == ALIVE); };
        bool isDead() const { return ( m_deathState == DEAD || m_deathState == CORPSE ); };
        DeathState getDeathState() { return m_deathState; };
        virtual void setDeathState(DeathState s);           // overwrited in Creature/Player/Pet

        uint64 const& GetOwnerGUID() const { return  GetUInt64Value(UNIT_FIELD_SUMMONEDBY); }
        uint64 GetPetGUID() const { return  GetUInt64Value(UNIT_FIELD_SUMMON); }
        uint64 GetCharmerGUID() const { return GetUInt64Value(UNIT_FIELD_CHARMEDBY); }
        uint64 GetCharmGUID() const { return  GetUInt64Value(UNIT_FIELD_CHARM); }
        uint64 GetCharmerOrOwnerGUID() const { return GetCharmerGUID() ? GetCharmerGUID() : GetOwnerGUID(); }
        void SetCharmerGUID(uint64 owner) { SetUInt64Value(UNIT_FIELD_CHARMEDBY, owner); }

        Unit* GetOwner() const;
        Pet* GetPet() const;
        Unit* GetCharmer() const;
        Unit* GetCharm() const;
        Unit* GetCharmerOrOwner() const { return GetCharmerGUID() ? GetCharmer() : GetOwner(); }

        void SetPet(Pet* pet);
        void SetCharm(Unit* pet);
        bool isCharmed() const { return GetCharmerGUID() != 0; }

        bool AddAura(Aura *aur, bool uniq = false);

        void RemoveFirstAuraByDispel(uint32 dispel_type, Unit *pCaster);
        void RemoveAura(AuraMap::iterator &i, bool onDeath = false);
        void RemoveAura(uint32 spellId, uint32 effindex);
        void RemoveAurasDueToSpell(uint32 spellId);
        void RemoveAurasDueToItem(Item* castItem);
        void RemoveSpellsCausingAura(uint32 auraType);
        void RemoveRankAurasDueToSpell(uint32 spellId);
        bool RemoveNoStackAurasDueToAura(Aura *Aur);
        void RemoveAreaAurasByOthers(uint64 guid = 0);

        void RemoveAllAuras();
        void RemoveAllAurasOnDeath();
        void DelayAura(uint32 spellId, uint32 effindex, int32 delaytime);
        //void SetAura(Aura* Aur){ m_Auras = Aur; }
        bool SetAurDuration(uint32 spellId, uint32 effindex, uint32 duration);
        uint32 GetAurDuration(uint32 spellId, uint32 effindex);

        float GetResistanceBuffMods(SpellSchools school, bool positive) const { return GetFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school ); }
        void SetResistanceBuffMods(SpellSchools school, bool positive, float val) { SetFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school,val); }
        void ApplyResistanceBuffModsMod(SpellSchools school, bool positive, float val, bool apply) { ApplyModFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school, val, apply); }
        void ApplyResistanceBuffModsPercentMod(SpellSchools school, bool positive, float val, bool apply) { ApplyPercentModFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school, val, apply); }
        void SetPosStat(Stats stat, float val) { SetFloatValue(UNIT_FIELD_POSSTAT0+stat, val); }
        void ApplyPosStatMod(Stats stat, float val, bool apply) { ApplyModFloatValue(UNIT_FIELD_POSSTAT0+stat, val, apply); }
        void ApplyPosStatPercentMod(Stats stat, float val, bool apply) { ApplyPercentModFloatValue(UNIT_FIELD_POSSTAT0+stat, val, apply); }
        void SetNegStat(Stats stat, float val) { SetFloatValue(UNIT_FIELD_NEGSTAT0+stat, val); }
        void ApplyNegStatMod(Stats stat, float val, bool apply) { ApplyModFloatValue(UNIT_FIELD_NEGSTAT0+stat, val, apply); }
        void ApplyNegStatPercentMod(Stats stat, float val, bool apply) { ApplyPercentModFloatValue(UNIT_FIELD_NEGSTAT0+stat, val, apply); }
        void SetCreateStat(Stats stat, float val) { m_createStats[stat] = val; }
        void SetCreateHealth(float val) { m_createHealth = val; }
        float GetCreateHealth() const { return m_createHealth; }
        void SetCreatePowers(Powers power, float val) { m_createPowers[power] = val; }
        float GetCreatePowers(Powers power) const { return m_createPowers[power]; }
        float GetPosStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_POSSTAT0+stat); }
        float GetNegStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_NEGSTAT0+stat); }
        float GetCreateStat(Stats stat) const { return m_createStats[stat]; }

        void SetCurrentCastedSpell(Spell * pSpell);
        virtual void ProhibitSpellScholl(uint32 idSchool /* from SpellSchools */, uint32 unTimeMs ) { }
        void InterruptSpell();
        Spell * m_currentSpell;
        Spell * m_oldSpell;
        Spell * m_currentMeleeSpell;
        UnitActionBarEntry PetActionBar[10];
        uint32 m_addDmgOnce;
        uint64 m_TotemSlot[4];
        uint64 m_ObjectSlot[4];
        uint32 m_canMove;
        uint32 m_detectStealth;
        uint32 m_detectInvisibility;
        uint32 m_stealthvalue;
        uint32 m_invisibilityvalue;
        uint32 m_ShapeShiftForm;
        uint32 m_form;
        int32 m_modHitChance;
        int32 m_modSpellHitChance;
        int32 m_baseSpellCritChance;
        int32 m_modCastSpeedPct;
        float m_modResilience;
        float m_threatModifier[MAX_SPELL_SCHOOL];
        float m_modAttackSpeedPct[3];

        // stat system
        bool HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply);
        void SetModifierValue(UnitMods unitMod, UnitModifierType modifierType, float value) { m_auraModifiersGroup[unitMod][modifierType] = value; }
        float GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const;
        float GetTotalStatValue(Stats stat) const;
        float GetTotalAuraModValue(UnitMods unitMod) const;
        SpellSchools GetSpellSchoolByAuraGroup(UnitMods unitMod) const;
        Stats GetStatByAuraGroup(UnitMods unitMod) const;
        Powers GetPowerTypeByAuraGroup(UnitMods unitMod) const;
        bool CanModifyStats() const { return m_canModifyStats; }
        void SetCanModifyStats(bool modifyStats) { m_canModifyStats = modifyStats; }
        bool UpdateStats(Stats stat);
        bool UpdateAllStats();
        void UpdateResistances(uint32 school);
        void UpdateArmor();
        void UpdateMaxHealth();
        void UpdateMaxPower(Powers power);
        void UpdateAttackPowerAndDamage(bool ranged = false);
        void UpdateDamagePhysical(WeaponAttackType attType);
        float GetTotalAttackPowerValue(WeaponAttackType attType) const;
        float GetWeaponDamageRange(WeaponAttackType attType ,WeaponDamageRange type) const;
        void SetBaseWeaponDamage(WeaponAttackType attType ,WeaponDamageRange damageRange, float value) { m_weaponDamage[attType][damageRange] = value; }
        //

        bool isInFront(Unit const* target,float distance) const;
        void SetInFront(Unit const* target);

        // Visibility system
        UnitVisibility GetVisibility() const { return m_Visibility; }
        void SetVisibility(UnitVisibility x);

        // common function for visibility checks for player/creatures with detection code
        bool isVisibleForOrDetect(Unit const* u, bool detect, bool inVisibleList = false) const;
        
        // virtual functions for all world objects types
        bool isVisibleForInState(Player const* u, bool inVisibleList) const;
        // function for low level grid visibility checks in player/creature cases
        virtual bool IsVisibleInGridForPlayer(Player* pl) const = 0;

        bool m_silenced;
        bool waterbreath;
        AuraList      & GetSingleCastAuras()       { return m_scAuras; }
        AuraList const& GetSingleCastAuras() const { return m_scAuras; }
        SpellImmuneList m_spellImmune[6];

        // Threat related methodes
        bool CanHaveThreatList() const;
        void AddThreat(Unit* pVictim, float threat, uint8 school = 0, SpellEntry const *threatSpell = NULL);
        float ApplyTotalThreatModifier(float threat, uint8 school = 0);
        void DeleteThreatList();
        bool SelectHostilTarget();
        void TauntApply(Unit* pVictim);
        void TauntFadeOut(Unit *taunter);
        ThreatManager& getThreatManager() { return m_ThreatManager; }
        void addHatedBy(HostilReference* pHostilReference) { m_HostilRefManager.insertFirst(pHostilReference); };
        void removeHatedBy(HostilReference* pHostilReference) {};  // nothing to do yet
        HostilRefManager& getHostilRefManager() { return m_HostilRefManager; }

        Aura* GetAura(uint32 spellId, uint32 effindex);
        AuraMap& GetAuras( ) {return m_Auras;}
        AuraList const& GetAurasByType(uint8 type) const { return m_modAuras[type]; }
        void ApplyAuraProcTriggerDamage(Aura* aura, bool apply);
        long GetTotalAuraModifier(uint32 ModifierID);
        void SendMoveToPacket(float x, float y, float z, bool run, uint32 transitTime = 0);
        void setTransForm(uint32 spellid) { m_transform = spellid;}
        uint32 getTransForm() { return m_transform;}
        void AddDynObject(DynamicObject* dynObj);
        void RemoveDynObject(uint32 spellid);
        void RemoveDynObjectWithGUID(uint64 guid) { m_dynObjGUIDs.remove(guid); }
        void RemoveAllDynObjects();
        void AddGameObject(GameObject* gameObj);
        void RemoveGameObject(GameObject* gameObj, bool del);
        void RemoveGameObject(uint32 spellid, bool del);
        DynamicObject *GetDynObject(uint32 spellId, uint32 effIndex);
        uint32 CalculateDamage(WeaponAttackType attType);
        void ModifyAuraState(uint32 flag, bool apply);
        void UnsummonAllTotems();
        uint32 SpellDamageBonus(Unit *pVictim, SpellEntry const *spellProto, uint32 damage, DamageEffectType damagetype);
        uint32 SpellHealingBonus(SpellEntry const *spellProto, uint32 healamount, DamageEffectType damagetype, Unit *pVictim);
        bool SpellCriticalBonus(SpellEntry const *spellProto, int32 *peffect, Unit *pVictim);
        void MeleeDamageBonus(Unit *pVictim, uint32 *damage, WeaponAttackType attType);
        void ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply);
        virtual bool IsImmunedToSpell(SpellEntry const* spellInfo) const;
                                                            // redefined in Creature
        bool IsImmunedToPhysicalDamage() const;
        bool IsImmunedToSpellDamage(SpellEntry const* spellInfo) const;
        bool IsImmunedToSpellEffect(uint32 effect) const;

        uint32 CalcArmorReducedDamage(Unit* pVictim, const uint32 damage);
        void CalcAbsorbResist(Unit *pVictim, uint32 School, const uint32 damage, uint32 *absorb, uint32 *resist);

        float GetSpeed( UnitMoveType mtype ) const;
        float GetSpeedRate( UnitMoveType mtype ) const { return m_speed_rate[mtype]; }
        void SetSpeed(UnitMoveType mtype, float rate, bool forced = false);
        virtual void ApplySpeedMod(UnitMoveType mtype, float rate, bool forced, bool apply);

        void SetHover(bool on);
        bool isHover() const { return HasAuraType(SPELL_AURA_HOVER); }

        void _RemoveAllAuraMods();
        void _ApplyAllAuraMods();

        int32 CalculateSpellDamage(SpellEntry const* spellProto, uint8 effect_index);
        Creature* SummonCreature(uint32 id, uint32 mapid, float x, float y, float z, float ang,TempSummonType spwtype,uint32 despwtime);

        void addFollower(FollowerReference* pRef) { m_FollowingRefManager.insertFirst(pRef); }
        void removeFollower(FollowerReference* pRef) {  } // nothing to do yet
        static Unit* GetUnit(WorldObject& object, uint64 guid);
    protected:
        Unit ( WorldObject *instantiator );

        void _UpdateSpells(uint32 time);
        //void _UpdateAura();

        //Aura* m_aura;
        //uint32 m_auraCheck, m_removeAuraTimer;

        uint32 m_attackTimer[3];

        float m_createStats[5];
        float m_createHealth;
        float m_createPowers[MAX_POWERS];

        AttackerSet m_attackers;
        Unit* m_attacking;

        DeathState m_deathState;

        AuraMap m_Auras;

        std::list<Aura *> m_scAuras;                        // casted singlecast auras

        typedef std::list<uint64> DynObjectGUIDs;
        DynObjectGUIDs m_dynObjGUIDs;

        std::list<GameObject*> m_gameObj;
        bool m_isSorted;
        uint32 m_transform;
        uint32 m_removedAuras;

        AuraList m_modAuras[TOTAL_AURAS];
        long m_AuraModifiers[TOTAL_AURAS];
        float m_auraModifiersGroup[UNIT_MOD_END][MODIFIER_TYPE_END];
        float m_weaponDamage[3][2];
        bool m_canModifyStats;
        //std::list< spellEffectPair > AuraSpells[TOTAL_AURAS];  // TODO: use this if ok for mem

        float m_speed_rate[MAX_MOVE_TYPE];
    private:
        void SendAttackStop(Unit* victim);                  // only from AttackStop(Unit*)
        void SendAttackStart(Unit* pVictim);                // only from Unit::AttackStart(Unit*)

        uint32 m_state;                                     // Even derived shouldn't modify
        uint32 m_CombatTimer;

        UnitVisibility m_Visibility;

        Diminishing m_Diminishing;
        // Manage all Units threatening us
        ThreatManager m_ThreatManager;
        // Manage all Units that are threatened by us
        HostilRefManager m_HostilRefManager;

        FollowerRefManager m_FollowingRefManager;
};
#endif
