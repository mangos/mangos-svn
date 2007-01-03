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

#ifndef __UNIT_H
#define __UNIT_H

#include "Common.h"
#include "Object.h"
#include "ObjectAccessor.h"
#include "Opcodes.h"
#include "Mthread.h"
#include "SpellAuraDefines.h"

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

#define CREATURE_MAX_SPELLS     4
#define PLAYER_MAX_SKILLS       127
#define PLAYER_SKILL(x)         (PLAYER_SKILL_INFO_START + ((x)*3))
// DWORD definitions gathered from windows api
#define SKILL_VALUE(x)          uint16(x)
#define SKILL_MAX(x)            uint16((uint32(x) >> 16))
#define MAKE_SKILL_VALUE(v, m) ((uint32)(((uint16)(v)) | (((uint32)((uint16)(m))) << 16)))

#define NOSWING                     0
#define SINGLEHANDEDSWING           1
#define TWOHANDEDSWING              2

#define VICTIMSTATE_UNKNOWN1        0
#define VICTIMSTATE_NORMAL          1
#define VICTIMSTATE_DODGE           2
#define VICTIMSTATE_PARRY           3
#define VICTIMSTATE_UNKNOWN2        4
#define VICTIMSTATE_BLOCKS          5
#define VICTIMSTATE_EVADES          6
#define VICTIMSTATE_IS_IMMUNE       7
#define VICTIMSTATE_DEFLECTS        8

#define HITINFO_NORMALSWING         0x00
#define HITINFO_NORMALSWING2        0x02
#define HITINFO_LEFTSWING           0x04
#define HITINFO_MISS                0x10
#define HITINFO_ABSORB              0x20                    // plays absorb sound
#define HITINFO_RESIST              0x40                    // resisted atleast some damage
#define HITINFO_CRITICALHIT         0x80
#define HITINFO_GLANCING            0x4000
#define HITINFO_CRUSHING            0x8000
#define HITINFO_NOACTION            0x10000
#define HITINFO_SWINGNOHITSOUND     0x80000

#define NULL_BAG                    0
#define NULL_SLOT                   255

#define MAX_DIST_INVISIBLE_UNIT     20                      // Max distance to be able to detect an invisible unit
#define ATTACK_DIST                 5

struct FactionTemplateEntry;
struct Modifier;
struct SpellEntry;

class Aura;
class Creature;
class Spell;
class DynamicObject;
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

typedef std::list<SpellImmune*> SpellImmuneList;

enum DeathState
{
    ALIVE = 0,
    JUST_DIED,
    CORPSE,
    DEAD
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
    UNIT_STAT_IN_FLIGHT     = 256,                          // player is i n flight mode
    UNIT_STAT_FOLLOW        = 512,
    UNIT_STAT_ROOT          = 1024,
    UNIT_STAT_CONFUSED      = 2048,
    UNIT_STAT_ALL_STATE     = 0xffff                        //(UNIT_STAT_STOPPED | UNIT_STAT_MOVING | UNIT_STAT_IN_COMBAT | UNIT_STAT_IN_FLIGHT)
};

enum UnitMoveType
{
    MOVE_WALK       =0,
    MOVE_RUN        =1,
    MOVE_WALKBACK   =2,
    MOVE_SWIM       =3,
    MOVE_SWIMBACK   =4,
    MOVE_TURN       =5
};

#define MAX_MOVE_TYPE 6

extern float baseMoveSpeed[MAX_MOVE_TYPE];

enum WeaponAttackType
{
    BASE_ATTACK   = 0,
    OFF_ATTACK    = 1,
    RANGED_ATTACK = 2
};

enum DamageEffectType
{
    DIRECT_DAMAGE = 0,
    SPELL_DIRECT_DAMAGE = 1,
    DOT = 2,
    HEAL = 3,
    NODAMAGE = 4,
    SELF_DAMAGE = 5
};

enum UnitVisibilityUpdate
{
    VISIBLE_NOCHANGES                 = 0,
    VISIBLE_SET_VISIBLE               = 1,
    VISIBLE_SET_INVISIBLE             = 2,
    VISIBLE_SET_INVISIBLE_FOR_GROUP   = 3
};

enum UnitVisibility
{
    VISIBILITY_OFF         = 0,
    VISIBILITY_ON          = 1,
    VISIBILITY_GROUP       = 2
};

// Value masks for UNIT_FIELD_FLAGS
enum UnitFlags
{
    UNIT_FLAG_NONE           = 0x00000000,
    UNIT_FLAG_DISABLE_MOVE   = 0x00000004,
    UNIT_FLAG_UNKNOWN1       = 0x00000008,                  // essential for all units..
    UNIT_FLAG_RENAME         = 0x00000010,                  // rename creature
    UNIT_FLAG_RESTING        = 0x00000020,
    UNIT_FLAG_PVP            = 0x00001000,
    UNIT_FLAG_MOUNT          = 0x00002000,
    UNIT_FLAG_DISABLE_ROTATE = 0x00040000,
    UNIT_FLAG_IN_COMBAT      = 0x00080000,
    UNIT_FLAG_SKINNABLE      = 0x04000000,
    UNIT_FLAG_SHEATHE        = 0x40000000
};
enum ProcFlags
{
    PROC_FLAG_NONE            = 0x00000000,
    PROC_FLAG_UNKOWN1         = 0x00000001,
    PROC_FLAG_DIE             = 0x00000002,
    PROC_FLAG_SHORT_ATTACK    = 0x00000004,
    PROC_FLAG_BE_SHORT_ATTACK = 0x00000008,
    PROC_FLAG_SHORT_HIT       = 0x00000010,
    PROC_FLAG_BE_SHORT_HIT    = 0x00000020,
    PROC_FLAG_LONG_ATTACK     = 0x00000040,
    PROC_FLAG_BE_LONG_ATTACK  = 0x00000080,
    PROC_FLAG_LONG_HIT        = 0x00000100,
    PROC_FLAG_BE_LONG_HIT     = 0x00000200,
    PROC_FLAG_HEAL_SPELL      = 0x00004000,
    PROC_FLAG_SPELL_DAMAGE    = 0x00010000,
    PROC_FLAG_BE_SPELL_DAMAGED= 0x00020000,
};
enum AuraState
{
    AURA_STATE_DODGE          = 1,
    AURA_STATE_HEALTHLESS     = 2,
    AURA_STATE_RACE           = 3,
    AURA_STATE_UNKNOWN1       = 4,
    AURA_STATE_JUDGEMENT      = 5,
    AURA_STATE_UNKNOWN2       = 6,
    AURA_STATE_PARRY          = 7                           // unsure.
};
//To all Immune system,if target has immunes,
//some spell that related to ImmuneToDispel or ImmuneToSchool or ImmuneToDamage type can't cast to it,
//some spell_effects that related to ImmuneToEffect<effect>(only this effect in the spell) can't cast to it,
//some aura(related to ImmuneToMechanic or ImmuneToState<aura>) can't apply to it.
enum SpellImmunity
{
    IMMUNITY_EFFECT                = 0,
    IMMUNITY_STATE                 = 1,
    IMMUNITY_SCHOOL                = 2,
    IMMUNITY_DAMAGE                = 3,
    IMMUNITY_DISPEL                = 4,
    IMMUNITY_MECHANIC              = 5

};

enum ImmuneToMechanic
{
    IMMUNE_MECHANIC_CHARM            =1,
    IMMUNE_MECHANIC_CONFUSED         =2,
    IMMUNE_MECHANIC_DISARM           =3,
    IMMUNE_MECHANIC_ATTRACT          =4,
    IMMUNE_MECHANIC_FEAR             =5,
    IMMUNE_MECHANIC_STUPID           =6,
    IMMUNE_MECHANIC_ROOT             =7,
    IMMUNE_MECHANIC_PEACE            =8,
    IMMUNE_MECHANIC_SILENCE          =9,
    IMMUNE_MECHANIC_SLEEP            =10,
    IMMUNE_MECHANIC_CHASE            =11,
    IMMUNE_MECHANIC_STUNDED          =12,
    IMMUNE_MECHANIC_FREEZE           =13,
    IMMUNE_MECHANIC_KNOCKOUT         =14,
    IMMUNE_MECHANIC_BLEED            =15,
    IMMUNE_MECHANIC_HEAL             =16,
    IMMUNE_MECHANIC_POLYMORPH        =17,
    IMMUNE_MECHANIC_BANISH           =18,
    IMMUNE_MECHANIC_SHIELDED         =19,
    IMMUNE_MECHANIC_DURANCED         =20,
    IMMUNE_MECHANIC_MOUNT            =21,
    IMMUNE_MECHANIC_PERSUADED        =22,
    IMMUNE_MECHANIC_TURNED           =23,
    IMMUNE_MECHANIC_HORROR           =24,
    IMMUNE_MECHANIC_INVULNERABILITY  =25,
    IMMUNE_MECHANIC_INTERRUPTED      =26,
    IMMUNE_MECHANIC_DAZED            =27
};

enum ImmuneToDispel
{
    IMMUNE_DISPEL_MAGIC        =1,
    IMMUNE_DISPEL_CURSE        =2,
    IMMUNE_DISPEL_DISEASE      =3,
    IMMUNE_DISPEL_POISON       =4,
    IMMUNE_DISPEL_STEALTH      =5,
    IMMUNE_DISPEL_INVISIBILITY =6,
    IMMUNE_DISPEL_ALL          =7,
    IMMUNE_DISPEL_SPE_NPC_ONLY =8,
    IMMUNE_DISPEL_CRAZY        =9,
    IMMUNE_DISPEL_ZG_TICKET    =10
};

enum ImmuneToDamage
{
    IMMUNE_DAMAGE_PHYSICAL     =1,
    IMMUNE_DAMAGE_MAGIC        =126
};

enum ImmuneToSchool
{
    IMMUNE_SCHOOL_PHYSICAL     =1,
    IMMUNE_SCHOOL_HOLY         =2,
    IMMUNE_SCHOOL_FIRE         =4,
    IMMUNE_SCHOOL_NATURE       =8,
    IMMUNE_SCHOOL_FROST        =16,
    IMMUNE_SCHOOL_SHADOW       =32,
    IMMUNE_SCHOOL_ARCANE       =64,
    IMMUNE_SCHOOL_MAGIC        =126
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

struct Hostil
{
    Hostil(uint64 _UnitGuid, float _Hostility) : UnitGuid(_UnitGuid), Hostility(_Hostility) {}

    uint64 UnitGuid;
    float Hostility;
    bool operator <(Hostil item)
    {
        if(Hostility < item.Hostility)
            return true;
        else
            return false;
    };
};

typedef std::list<Hostil> HostilList;

enum MeleeHitOutcome
{
    MELEE_HIT_MISS, MELEE_HIT_DODGE, MELEE_HIT_BLOCK, MELEE_HIT_PARRY, MELEE_HIT_GLANCING,
    MELEE_HIT_CRIT, MELEE_HIT_CRUSHING, MELEE_HIT_NORMAL
};

// delay time next attack to privent client attack animanation problems
#define ATTACK_DISPLAY_DELAY 200

class MANGOS_DLL_SPEC Unit : public Object
{
    public:
        typedef std::set<Unit*> AttackerSet;
        typedef std::pair<uint32, uint32> spellEffectPair;
        typedef std::multimap< spellEffectPair, Aura*> AuraMap;
        typedef std::list<Aura *> AuraList;
        virtual ~Unit ( );

        virtual void Update( uint32 time );

        char const* GetName() const { return m_name.c_str(); }
        void SetName(std::string newname) { m_name=newname; }

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
        bool Attack(Unit *victim);
        bool AttackStop();
        void RemoveAllAttackers();
        bool isInCombatWithPlayer() const;
        Unit* getVictim() const { return m_attacking; }
        void CombatStop() { AttackStop(); RemoveAllAttackers(); ClearInCombat(); }

        void addUnitState(uint32 f) { m_state |= f; };
        bool hasUnitState(const uint32 f) const { return (m_state & f); }
        void clearUnitState(uint32 f) { m_state &= ~f; };

        uint32 getLevel() const { return GetUInt32Value(UNIT_FIELD_LEVEL); };
        void SetLevel(uint32 lvl) { SetUInt32Value(UNIT_FIELD_LEVEL,lvl); }
        uint8 getRace() const { return (uint8)(GetUInt32Value(UNIT_FIELD_BYTES_0) & 0xFF); };
        uint32 getRaceMask() const { return 1 << (getRace()-1); };
        uint8 getClass() const { return (uint8)((GetUInt32Value(UNIT_FIELD_BYTES_0) >> 8) & 0xFF); };
        uint32 getClassMask() const { return 1 << (getClass()-1); };
        uint8 getGender() const { return (uint8)((GetUInt32Value(UNIT_FIELD_BYTES_0) >> 16) & 0xFF); };

        float GetStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_STATS+stat); }
        void SetStat(Stats stat, float val) { SetFloatValue(UNIT_FIELD_STATS+stat, val); }
        void ApplyStatMod(Stats stat, float val, bool apply) { ApplyModFloatValue(UNIT_FIELD_STATS+stat, val, apply); }
        void ApplyStatPercentMod(Stats stat, float val, bool apply) { ApplyPercentModFloatValue(UNIT_FIELD_STATS+stat, val, apply); }

        float GetArmor() const { return GetResistance(SPELL_SCHOOL_NORMAL) ; }
        void SetArmor(float val) { SetResistance(SPELL_SCHOOL_NORMAL, val); }
        void ApplyArmorMod(float val, bool apply) { ApplyResistanceMod(SPELL_SCHOOL_NORMAL, val, apply); }
        void ApplyArmorPercentMod(float val, bool apply) { ApplyResistancePercentMod(SPELL_SCHOOL_NORMAL, val, apply); }

        float GetResistance(SpellSchools school) const { return GetFloatValue(UNIT_FIELD_RESISTANCES+school); }
        void SetResistance(SpellSchools school, float val) { SetFloatValue(UNIT_FIELD_RESISTANCES+school,val); }
        void ApplyResistanceMod(SpellSchools school, float val, bool apply) { ApplyModFloatValue(UNIT_FIELD_RESISTANCES+school, val, apply); }
        void ApplyResistancePercentMod(SpellSchools school, float val, bool apply) { ApplyPercentModFloatValue(UNIT_FIELD_RESISTANCES+school, val, apply); }

        uint32 GetHealth()    const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
        uint32 GetMaxHealth() const { return (uint32)GetFloatValue(UNIT_FIELD_MAXHEALTH); }
        void SetHealth(   uint32 val) { SetUInt32Value(UNIT_FIELD_HEALTH,val); }
        void SetMaxHealth(uint32 val) { SetFloatValue(UNIT_FIELD_MAXHEALTH,val); }
        void ModifyHealth(int32 val);
        void ApplyMaxHealthMod(uint32 val, bool apply) { ApplyModFloatValue(UNIT_FIELD_MAXHEALTH, val, apply); }
        void ApplyMaxHealthPercentMod(float val, bool apply) { ApplyPercentModFloatValue(UNIT_FIELD_MAXHEALTH, val, apply); }

        Powers getPowerType() const { return Powers((GetUInt32Value(UNIT_FIELD_BYTES_0) >> 24) & 0xFF); };
        void setPowerType(Powers power);
        uint32 GetPower(   Powers power) const { return (uint32)GetFloatValue(UNIT_FIELD_POWER1   +power); }
        uint32 GetMaxPower(Powers power) const { return (uint32)GetFloatValue(UNIT_FIELD_MAXPOWER1+power); }
        void SetPower(   Powers power, uint32 val) { SetFloatValue(UNIT_FIELD_POWER1   +power,val); }
        void SetMaxPower(Powers power, uint32 val) { SetFloatValue(UNIT_FIELD_MAXPOWER1+power,val); }
        void ModifyPower(Powers power, int32 val);
        void ApplyPowerMod(Powers power, uint32 val, bool apply) { ApplyModFloatValue(UNIT_FIELD_POWER1+power, val, apply); }
        void ApplyPowerPercentMod(Powers power, float val, bool apply) { ApplyPercentModFloatValue(UNIT_FIELD_POWER1+power, val, apply); }
        void ApplyMaxPowerMod(Powers power, uint32 val, bool apply) { ApplyModFloatValue(UNIT_FIELD_MAXPOWER1+power, val, apply); }
        void ApplyMaxPowerPercentMod(Powers power, float val, bool apply) { ApplyPercentModFloatValue(UNIT_FIELD_MAXPOWER1+power, val, apply); }

        uint32 GetAttackTime(WeaponAttackType att) const { return (uint32)GetFloatValue(UNIT_FIELD_BASEATTACKTIME+att); }
        void SetAttackTime(WeaponAttackType att, uint32 val) { SetFloatValue(UNIT_FIELD_BASEATTACKTIME+att,val); }
        void ApplyAttackTimePercentMod(WeaponAttackType att,float val, bool apply) { ApplyPercentModFloatValue(UNIT_FIELD_BASEATTACKTIME+att, val, !apply); }

        // faction template id
        uint32 getFaction() const { return GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE); }
        void setFaction(uint32 faction) { SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, faction ); }
        FactionTemplateEntry const* getFactionTemplateEntry() const;
        bool IsHostileTo(Unit const* unit) const;
        bool IsHostileToAll() const;
        bool IsFriendlyTo(Unit const* unit) const;
        bool IsNeutralToAll() const;
        bool IsPvP() { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); }
        void SetPvP(bool state) { if(state) SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); else RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); }

        uint8 getStandState() const { return (uint8)(GetUInt32Value(UNIT_FIELD_BYTES_1) & 0xFF); };

        bool IsMounted() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT ); }
        uint32 GetMountID() const { return GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }
        void Mount(uint32 mount, bool taxi = false);
        void Unmount();

        void DealDamage(Unit *pVictim, uint32 damage, DamageEffectType damagetype, uint32 procFlag, bool durabilityLoss);
        void DoAttackDamage(Unit *pVictim, uint32 *damage, uint32 *blocked_amount, uint32 *damageType, uint32 *hitInfo, uint32 *victimState, uint32 *absorbDamage, uint32 *resistDamage, WeaponAttackType attType);
        void ProcDamageAndSpell(Unit *pVictim, uint32 procflag1, uint32 procflag2);
        void HandleEmoteCommand(uint32 anim_id);
        void AttackerStateUpdate (Unit *pVictim, WeaponAttackType attType = BASE_ATTACK);
        uint32 SpellMissChanceCalc(Unit *pVictim) const;
        int32 MeleeMissChanceCalc(const Unit *pVictim) const;

        float GetUnitDodgeChance()    const;
        float GetUnitParryChance()    const;
        float GetUnitBlockChance()    const;
        float GetUnitCriticalChance() const { return GetTypeId() == TYPEID_PLAYER ? GetFloatValue( PLAYER_CRIT_PERCENTAGE ) : 5; }

        virtual uint32 GetBlockValue() const =0;
        uint32 GetUnitMeleeSkill() const { return getLevel() * 5; }
        uint16 GetDefenceSkillValue() const;
        uint16 GetPureDefenceSkillValue() const;
        uint16 GetWeaponSkillValue(WeaponAttackType attType) const;
        uint16 GetPureWeaponSkillValue(WeaponAttackType attType) const;
        uint32 GetWeaponProcChance() const;
        MeleeHitOutcome RollMeleeOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType) const;

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
        bool isTabardVendor() const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDVENDOR ); }
        bool isAuctioner()    const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER ); }
        bool isArmorer()      const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_ARMORER ); }
        bool isServiceProvider() const
        {
            return HasFlag( UNIT_NPC_FLAGS,
                UNIT_NPC_FLAG_VENDOR | UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_TAXIVENDOR |
                UNIT_NPC_FLAG_PETITIONER | UNIT_NPC_FLAG_BATTLEFIELDPERSON | UNIT_NPC_FLAG_BANKER |
                UNIT_NPC_FLAG_INNKEEPER | UNIT_NPC_FLAG_GUARD | UNIT_NPC_FLAG_SPIRITHEALER | UNIT_NPC_FLAG_TABARDVENDOR |
                UNIT_NPC_FLAG_AUCTIONEER );
        }
        //Need fix or use this
        bool isGuard() const  { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GUARD); }

        bool isInFlight()  const { return hasUnitState(UNIT_STAT_IN_FLIGHT); }

        //bool isInCombat()  const { return hasUnitState(UNIT_STAT_IN_COMBAT); }
        bool isInCombat()  const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT); }
        void SetInCombat();
        void ClearInCombat();

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
        bool isTargetableForAttack();
        bool IsInWater() const;
        bool IsUnderWater() const;
        bool isInAccessablePlaceFor(Creature* c) const;

        void SendHealSpellOnPlayer(Unit *pVictim, uint32 SpellID, uint32 Damage, bool critical = false);
        void SendHealSpellOnPlayerPet(Unit *pVictim, uint32 SpellID, uint32 Damage,Powers powertype, bool critical = false);
        void PeriodicAuraLog(Unit *pVictim, SpellEntry const *spellProto, Modifier *mod);
        void SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage);
        void CastSpell(Unit* Victim, uint32 spellId, bool triggered, Item *castItem = NULL);
        void CastSpell(Unit* Victim,SpellEntry const *spellInfo, bool triggered, Item *castItem= NULL);

        void DeMorph();

        void SendAttackStateUpdate(uint32 HitInfo, Unit *target, uint8 SwingType, uint32 DamageType, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, uint32 TargetState, uint32 BlockedAmount);
        void SendSpellNonMeleeDamageLog(Unit *target,uint32 SpellID,uint32 Damage, uint8 DamageType,uint32 AbsorbedDamage, uint32 Resist,bool PhysicalDamage, uint32 Blocked, bool CriticalHit = false);

        void SendMonsterMove(float NewPosX, float NewPosY, float NewPosZ, bool Walkback, bool Run, uint32 Time);

        virtual void MoveOutOfRange(Player &) {  };

        bool isAlive() const { return (m_deathState == ALIVE); };
        bool isDead() const { return ( m_deathState == DEAD || m_deathState == CORPSE ); };
        DeathState getDeathState() { return m_deathState; };
        virtual void setDeathState(DeathState s);           // overwrited in Creature/Player/Pet

        uint64 const& GetOwnerGUID() const { return  GetUInt64Value(UNIT_FIELD_SUMMONEDBY); }
        uint64 GetPetGUID() const { return  GetUInt64Value(UNIT_FIELD_SUMMON); }
        uint64 GetCharmGUID() const { return  GetUInt64Value(UNIT_FIELD_CHARM); }

        Unit* GetOwner() const;
        Pet* GetPet() const;
        Creature* GetCharm() const;
        void SetPet(Pet* pet);
        void SetCharm(Creature* pet);

        bool AddAura(Aura *aur, bool uniq = false);

        void RemoveFirstAuraByDispel(uint32 dispel_type);
        void RemoveAura(AuraMap::iterator &i, bool onDeath = false);
        void RemoveAura(uint32 spellId, uint32 effindex);
        void RemoveAurasDueToSpell(uint32 spellId);
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

        void castSpell(Spell * pSpell);
        void InterruptSpell();
        Spell * m_currentSpell;
        Spell * m_oldSpell;
        Spell * m_currentMeleeSpell;
        uint32 m_addDmgOnce;
        uint64 m_TotemSlot[4];
        uint64 m_ObjectSlot[4];
        uint32 m_canMove;
        uint32 m_detectStealth;
        uint32 m_stealthvalue;
        uint32 m_ShapeShiftForm;
        uint32 m_form;
        int32 m_modDamagePCT;
        int32 m_modHitChance;
        int32 m_modSpellHitChance;
        int32 m_baseSpellCritChance;
        int32 m_modCastSpeedPct;

        bool isInFront(Unit const* target,float distance);
        void SetInFront(Unit const* target);

        // Invisibility and detection system
        UnitVisibility GetVisibility() {return m_Visibility;}
        UnitVisibilityUpdate GetUpdateVisibility() { return m_UpdateVisibility; }
        void SetVisibility(UnitVisibility x);
        void SetUpdateVisibility(UnitVisibilityUpdate x) { m_UpdateVisibility = x; }
        bool isVisibleFor(Unit* u, bool detect);

        bool m_silenced;
        bool waterbreath;
        std::list<Aura *> *GetSingleCastAuras() { return &m_scAuras; }
        SpellImmuneList m_spellImmune[6];

        float GetHostility(uint64 guid) const;
        float GetHostilityDistance(uint64 guid) const { return GetHostility( guid )/(3.5f * getLevel()+1.0f); }
        HostilList& GetHostilList() { return m_hostilList; }
        void AddHostil(uint64 guid, float hostility);
        Unit* SelectHostilTarget();

        Aura* GetAura(uint32 spellId, uint32 effindex);
        AuraMap& GetAuras( ) {return m_Auras;}
        AuraList& GetAurasByType(uint8 type) {return m_modAuras[type];}
        long GetTotalAuraModifier(uint32 ModifierID);
        void SendMoveToPacket(float x, float y, float z, bool run, uint32 transitTime = 0);
        void AddItemEnchant(Item *item,uint32 enchant_id,bool apply);
        void setTransForm(uint32 spellid) { m_transform = spellid;}
        uint32 getTransForm() { return m_transform;}
        void AddDynObject(DynamicObject* dynObj);
        void RemoveDynObject(uint32 spellid);
        void AddGameObject(GameObject* gameObj);
        void RemoveGameObject(GameObject* gameObj, bool del);
        void RemoveGameObject(uint32 spellid, bool del);
        DynamicObject *GetDynObject(uint32 spellId, uint32 effIndex);
        uint32 CalculateDamage(WeaponAttackType attType);
        void SetStateFlag(uint32 index, uint32 newFlag );
        void RemoveStateFlag(uint32 index, uint32 oldFlag );
        void ApplyStats(bool apply);
        void UnsummonTotem(int8 slot = -1);
        uint32 SpellDamageBonus(Unit *pVictim, SpellEntry const *spellProto, uint32 damage);
        uint32 SpellHealingBonus(SpellEntry const *spellProto, uint32 healamount);
        bool SpellCriticalBonus(SpellEntry const *spellProto, int32 *peffect);
        void MeleeDamageBonus(Unit *pVictim, uint32 *damage);
        void ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply);

        uint32 CalcArmorReducedDamage(Unit* pVictim, const uint32 damage);
        void CalcAbsorbResist(Unit *pVictim, uint32 School, const uint32 damage, uint32 *absorb, uint32 *resist);
        
        float GetSpeed( UnitMoveType mtype ) const;
        float GetSpeedRate( UnitMoveType mtype ) const { return m_speed_rate[mtype]; }
        void SetSpeed(UnitMoveType mtype, float rate, bool forced = false);
        void ApplySpeedMod(UnitMoveType mtype, float rate, bool forced, bool aplly);

        void SetHover(bool on);
        bool isHover() const { return HasAuraType(SPELL_AURA_HOVER); }
    protected:
        Unit ( );

        void _RemoveStatsMods();
        void _ApplyStatsMods();

        void _RemoveAllAuraMods();
        void _ApplyAllAuraMods();

        void _UpdateSpells(uint32 time);
        void _UpdateHostil( uint32 time );
        //void _UpdateAura();

        //Aura* m_aura;
        //uint32 m_auraCheck, m_removeAuraTimer;

        uint32 m_attackTimer[3];

        AttackerSet m_attackers;
        Unit* m_attacking;

        DeathState m_deathState;

        AuraMap m_Auras;

        std::list<Aura *> m_scAuras;                        // casted singlecast auras
        std::list<DynamicObject*> m_dynObj;
        std::list<GameObject*> m_gameObj;
        HostilList m_hostilList;
        uint32 m_transform;
        uint32 m_removedAuras;

        AuraList m_modAuras[TOTAL_AURAS];
        long m_AuraModifiers[TOTAL_AURAS];
        //std::list< spellEffectPair > AuraSpells[TOTAL_AURAS];  // TODO: use this if ok for mem

        std::string m_name;
        float m_speed_rate[MAX_MOVE_TYPE];
    private:
        void SendAttackStop(Unit* victim);                  // only from AttackStop(Unit*)
        void SendAttackStart(Unit* pVictim);                // only from Unit::AttackStart(Unit*)

        uint32 m_state;                                     // Even derived shouldn't modify
        uint32 m_CombatTimer;

        UnitVisibilityUpdate m_UpdateVisibility;
        UnitVisibility m_Visibility;
};
#endif
