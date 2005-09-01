/* Affect.h
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
#define MAX_AURAS 56
#define MAX_POSITIVE_AURAS 32                     // ?

//! 4-bit flag
enum AURA_FLAGS
{
    AFLAG_EMPTY = 0x0,
    AFLAG_SET = 0x9
};

enum MOD_TYPES
{
    SPELL_AURA_NONE = 0,                          // None
    SPELL_AURA_BIND_SIGHT = 1,                    // Bind Sight
    SPELL_AURA_MOD_THREAT = 10,                   // Mod Threat
    SPELL_AURA_AURAS_VISIBLE = 100,               // Auras Visible
    SPELL_AURA_MOD_RESISTANCE_PCT = 101,          // Mod Resistance %
    SPELL_AURA_MOD_CREATURE_ATTACK_POWER = 102,   // Mod Creature Attack Power
    SPELL_AURA_MOD_TOTAL_THREAT = 103,            // Mod Total Threat (Fade)
    SPELL_AURA_WATER_WALK = 104,                  // Water Walk
    SPELL_AURA_FEATHER_FALL = 105,                // Feather Fall
    SPELL_AURA_HOVER = 106,                       // Hover
    SPELL_AURA_ADD_FLAT_MODIFIER = 107,           // Add Flat Modifier
    SPELL_AURA_ADD_PCT_MODIFIER = 108,            // Add % Modifier
    SPELL_AURA_ADD_TARGET_TRIGGER = 109,          // Add Class Target Trigger
    SPELL_AURA_MOD_TAUNT = 11,                    // Taunt
    SPELL_AURA_MOD_POWER_REGEN_PERCENT = 110,     // Mod Power Regen %
    SPELL_AURA_ADD_CASTER_HIT_TRIGGER = 111,      // Add Class Caster Hit Trigger
    SPELL_AURA_OVERRIDE_CLASS_SCRIPTS = 112,      // Override Class Scripts
    SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN = 113,     // Mod Ranged Dmg Taken
    SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT = 114, // Mod Ranged % Dmg Taken
    SPELL_AURA_MOD_HEALING = 115,                 // Mod Healing
    SPELL_AURA_IGNORE_REGEN_INTERRUPT = 116,      // Regen During Combat
    SPELL_AURA_MOD_MECHANIC_RESISTANCE = 117,     // Mod Mechanic Resistance
    SPELL_AURA_MOD_HEALING_PCT = 118,             // Mod Healing %
    SPELL_AURA_SHARE_PET_TRACKING = 119,          // Share Pet Tracking
    SPELL_AURA_MOD_STUN = 12,                     // Stun
    SPELL_AURA_UNTRACKABLE = 120,                 // Untrackable
    SPELL_AURA_EMPATHY = 121,                     // Empathy (Lore, whatever)
    SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT = 122,      // Mod Offhand Dmg %
    SPELL_AURA_MOD_POWER_COST_PCT = 123,          // Mod Power Cost %
    SPELL_AURA_MOD_RANGED_ATTACK_POWER = 124,     // Mod Ranged Attack Power
    SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN = 125,      // Mod Melee Dmg Taken
    SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT = 126,  // Mod Melee % Dmg Taken
// Rngd Atk Pwr Attckr Bonus
    SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS = 127,
    SPELL_AURA_MOD_POSSESS_PET = 128,             // Mod Possess Pet
    SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS = 129,   // Mod Speed Always
    SPELL_AURA_MOD_DAMAGE_DONE = 13,              // Mod Damage Done
    SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS = 130,    // Mod Mounted Speed Always
// Mod Creature Ranged Attack Power
    SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER = 131,
    SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT = 132, // Mod Increase Energy %
    SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT = 133, // Mod Max Health %
    SPELL_AURA_MOD_MANA_REGEN_INTERRUPT = 134,    // Mod Interrupted Mana Regen
    SPELL_AURA_MOD_HEALING_DONE = 135,            // Mod Healing Done
    SPELL_AURA_MOD_HEALING_DONE_PERCENT = 136,    // Mod Healing Done %
    SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE = 137,   // Mod Total Stat %
    SPELL_AURA_MOD_HASTE = 138,                   // Haste - Melee
    SPELL_AURA_FORCE_REACTION = 139,              // Force Reaction
    SPELL_AURA_MOD_DAMAGE_TAKEN = 14,             // Mod Damage Taken
    SPELL_AURA_MOD_RANGED_HASTE = 140,            // Haste - Ranged
    SPELL_AURA_MOD_RANGED_AMMO_HASTE = 141,       // Haste - Ranged (Ammo Only)
    SPELL_AURA_MOD_BASE_RESISTANCE_PCT = 142,     // Mod Base Resistance %
    SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE = 143,    // Mod Resistance Exclusive
    SPELL_AURA_SAFE_FALL = 144,                   // Safe Fall
    SPELL_AURA_CHARISMA = 145,                    // Charisma
    SPELL_AURA_PERSUADED = 146,                   // Persuaded
    SPELL_AURA_ADD_CREATURE_IMMUNITY = 147,       // Add Creature Immunity
    SPELL_AURA_RETAIN_COMBO_POINTS = 148,         // Retain Combo Points
    SPELL_AURA_DAMAGE_SHIELD = 15,                // Damage Shield
    SPELL_AURA_MOD_STEALTH = 16,                  // Mod Stealth
    SPELL_AURA_MOD_DETECT = 17,                   // Mod Detect
    SPELL_AURA_MOD_INVISIBILITY = 18,             // Mod Invisibility
    SPELL_AURA_MOD_INVISIBILITY_DETECTION = 19,   // Mod Invisibility Detection
    SPELL_AURA_MOD_POSSESS = 2,                   // Mod Possess
    SPELL_AURA_MOD_RESISTANCE = 22,               // Mod Resistance
    SPELL_AURA_PERIODIC_TRIGGER_SPELL = 23,       // Periodic Trigger
    SPELL_AURA_PERIODIC_ENERGIZE = 24,            // Periodic Energize
    SPELL_AURA_MOD_PACIFY = 25,                   // Pacify
    SPELL_AURA_MOD_ROOT = 26,                     // Root
    SPELL_AURA_MOD_SILENCE = 27,                  // Silence
    SPELL_AURA_REFLECT_SPELLS = 28,               // Reflect Spells %
    SPELL_AURA_MOD_STAT = 29,                     // Mod Stat
    SPELL_AURA_PERIODIC_DAMAGE = 3,               // Periodic Damage
    SPELL_AURA_MOD_SKILL = 30,                    // Mod Skill
    SPELL_AURA_MOD_INCREASE_SPEED = 31,           // Mod Speed
    SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED = 32,   // Mod Speed Mounted
    SPELL_AURA_MOD_DECREASE_SPEED = 33,           // Mod Speed Slow
    SPELL_AURA_MOD_INCREASE_HEALTH = 34,          // Mod Increase Health
    SPELL_AURA_MOD_INCREASE_ENERGY = 35,          // Mod Increase Energy
    SPELL_AURA_MOD_SHAPESHIFT = 36,               // Shapeshift
    SPELL_AURA_EFFECT_IMMUNITY = 37,              // Immune Effect
    SPELL_AURA_STATE_IMMUNITY = 38,               // Immune State
    SPELL_AURA_SCHOOL_IMMUNITY = 39,              // Immune School
    SPELL_AURA_DAMAGE_IMMUNITY = 40,              // Immune Damage
    SPELL_AURA_DISPEL_IMMUNITY = 41,              // Immune Dispel Type
    SPELL_AURA_PROC_TRIGGER_SPELL = 42,           // Proc Trigger Spell
    SPELL_AURA_PROC_TRIGGER_DAMAGE = 43,          // Proc Trigger Damage
    SPELL_AURA_TRACK_CREATURES = 44,              // Track Creatures
    SPELL_AURA_TRACK_RESOURCES = 45,              // Track Resources
    SPELL_AURA_MOD_PARRY_SKILL = 46,              // Mod Parry Skill
    SPELL_AURA_MOD_PARRY_PERCENT = 47,            // Mod Parry Percent
    SPELL_AURA_MOD_DODGE_SKILL = 48,              // Mod Dodge Skill
    SPELL_AURA_MOD_DODGE_PERCENT = 49,            // Mod Dodge Percent
    SPELL_AURA_MOD_CONFUSE = 5,                   // Mod Confuse
    SPELL_AURA_MOD_BLOCK_SKILL = 50,              // Mod Block Skill
    SPELL_AURA_MOD_BLOCK_PERCENT = 51,            // Mod Block Percent
    SPELL_AURA_MOD_CRIT_PERCENT = 52,             // Mod Crit Percent
    SPELL_AURA_PERIODIC_LEECH = 53,               // Periodic Leech
    SPELL_AURA_MOD_HIT_CHANCE = 54,               // Mod Hit Chance
    SPELL_AURA_MOD_SPELL_HIT_CHANCE = 55,         // Mod Spell Hit Chance
    SPELL_AURA_TRANSFORM = 56,                    // Transform
    SPELL_AURA_MOD_SPELL_CRIT_CHANCE = 57,        // Mod Spell Crit Chance
    SPELL_AURA_MOD_INCREASE_SWIM_SPEED = 58,      // Mod Speed Swim
    SPELL_AURA_MOD_DAMAGE_DONE_CREATURE = 59,     // Mod Creature Dmg Done
    SPELL_AURA_MOD_CHARM = 6,                     // Mod Charm
    SPELL_AURA_MOD_PACIFY_SILENCE = 60,           // Pacify & Silence
    SPELL_AURA_MOD_SCALE = 61,                    // Mod Scale
    SPELL_AURA_PERIODIC_HEALTH_FUNNEL = 62,       // Periodic Health Funnel
    SPELL_AURA_PERIODIC_MANA_FUNNEL = 63,         // Periodic Mana Funnel
    SPELL_AURA_PERIODIC_MANA_LEECH = 64,          // Periodic Mana Leech
    SPELL_AURA_MOD_CASTING_SPEED = 65,            // Haste - Spells
    SPELL_AURA_FEIGN_DEATH = 66,                  // Feign Death
    SPELL_AURA_MOD_DISARM = 67,                   // Disarm
    SPELL_AURA_MOD_STALKED = 68,                  // Mod Stalked
    SPELL_AURA_SCHOOL_ABSORB = 69,                // School Absorb
    SPELL_AURA_MOD_FEAR = 7,                      // Mod Fear
    SPELL_AURA_EXTRA_ATTACKS = 70,                // Extra Attacks
    SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL = 71, // Mod School Spell Crit Chance
    SPELL_AURA_MOD_POWER_COST = 72,               // Mod Power Cost
    SPELL_AURA_MOD_POWER_COST_SCHOOL = 73,        // Mod School Power Cost
    SPELL_AURA_REFLECT_SPELLS_SCHOOL = 74,        // Reflect School Spells %
    SPELL_AURA_MOD_LANGUAGE = 75,                 // Mod Language
    SPELL_AURA_FAR_SIGHT = 76,                    // Far Sight
    SPELL_AURA_MECHANIC_IMMUNITY = 77,            // Immune Mechanic
    SPELL_AURA_MOUNTED = 78,                      // Mounted
    SPELL_AURA_MOD_DAMAGE_PERCENT_DONE = 79,      // Mod Dmg %
    SPELL_AURA_PERIODIC_HEAL = 8,                 // Periodic Heal
    SPELL_AURA_MOD_PERCENT_STAT = 80,             // Mod Stat %
    SPELL_AURA_SPLIT_DAMAGE = 81,                 // Split Damage
    SPELL_AURA_WATER_BREATHING = 82,              // Water Breathing
    SPELL_AURA_MOD_BASE_RESISTANCE = 83,          // Mod Base Resistance
    SPELL_AURA_MOD_REGEN = 84,                    // Mod Health Regen
    SPELL_AURA_MOD_POWER_REGEN = 85,              // Mod Power Regen
    SPELL_AURA_CHANNEL_DEATH_ITEM = 86,           // Create Death Item
    SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN = 87,     // Mod Dmg % Taken
    SPELL_AURA_MOD_PERCENT_REGEN = 88,            // Mod Health Regen Percent
    SPELL_AURA_PERIODIC_DAMAGE_PERCENT = 89,      // Periodic Damage Percent
    SPELL_AURA_MOD_ATTACKSPEED = 9,               // Mod Attack Speed
    SPELL_AURA_MOD_RESIST_CHANCE = 90,            // Mod Resist Chance
    SPELL_AURA_MOD_DETECT_RANGE = 91,             // Mod Detect Range
    SPELL_AURA_PREVENTS_FLEEING = 92,             // Prevent Fleeing
    SPELL_AURA_MOD_UNATTACKABLE = 93,             // Mod Uninteractible
    SPELL_AURA_INTERRUPT_REGEN = 94,              // Interrupt Regen
    SPELL_AURA_GHOST = 95,                        // Ghost
    SPELL_AURA_SPELL_MAGNET = 96,                 // Spell Magnet
    SPELL_AURA_MANA_SHIELD = 97,                  // Mana Shield
    SPELL_AURA_MOD_SKILL_TALENT = 98,             // Mod Skill Talent
    SPELL_AURA_MOD_ATTACK_POWER = 99,             // Mod Attack Power
};

class Modifier
{
    public:
        Modifier()
            :   m_type( 0 ), m_amount( 0 ), m_miscValue( 0 ), m_miscValue2( 0 )
        {
        }

        Modifier( uint8 t, int32 a, uint32 miscValue, uint32 miscValue2 )
            :   m_type( t ), m_amount( a ), m_miscValue( miscValue ), m_miscValue2( miscValue2 )
        {
        }

        uint8 GetType() const { return m_type; }
        int32 GetAmount() const { return m_amount; }
        uint32 GetMiscValue() const { return m_miscValue; }
        uint32 GetMiscValue2() const { return m_miscValue2; }

    private:
        uint8 m_type;                             // What does it modify? (str,int,hp)
        int32 m_amount;                           // By how much does it mod?
        uint32 m_miscValue;                       // Misc Value
        uint32 m_miscValue2;                      // second Misc Value
};

struct DamageShield
{
    uint32 m_spellId;
    uint32 m_damage;
    uint64 m_caster;
};

struct ProcTriggerSpell
{
    uint32 trigger;
    uint32 spellId;
    uint64 caster;
    uint32 procChance;
    uint32 procFlags;
    uint32 procCharges;
};

//!
//! Casting spell should look time:
//!
//! Affect *af = new Affect( AFF_GIANTSTR, spellId, 30, caster->GetGUID())
//!
//! af->AddMod(MOD_STRENGTH, 5);
//!
//! victim->AddAffect(af);
//!

//!
//! TODO: should store something like spell strength or caster level.
//!       should we use affect id or spellId is enough? probably not
//!       because different spell ranks have differt ids (afaik)
//!

class Affect
{
    public:
        typedef std::list<Modifier> ModList;

        Affect() : m_spellProto(0), m_duration(0), m_casterGUID(0), m_damagePerTick(0), m_auraSlot(0), m_spellPerTick(0), m_coAffect(0) {}

        Affect(SpellEntry *proto, int32 duration, const uint64 &guid) :
        m_spellProto(proto), m_duration(duration), m_casterGUID(guid), m_damagePerTick(0), m_auraSlot(0), m_spellPerTick(0), m_coAffect(0) {}

        void AddMod(uint8 t, int32 a,uint32 miscValue, uint32 miscValue2)
        {
            m_modList.push_back(Modifier(t, a,miscValue,miscValue2));
        }

        void SetDamagePerTick(uint16 dmg, uint32 tick)
        {
            m_damagePerTick = dmg;
            m_tickInterval = tick;
            m_currentTick = tick;
        }

        void SetPeriodicTriggerSpell(uint32 spellId, uint32 tick)
        {
            m_spellPerTick = spellId;
            m_tickSpellInterval = tick;
            m_currentSpellTick = tick;
        }

        void SetHealPerTick(uint16 dmg, uint32 tick)
        {
            m_healPerTick = dmg;
            m_tickHealInterval = tick;
            m_currentHealTick = tick;
        }

        SpellEntry* GetSpellProto() const { return m_spellProto; }
        int32 GetSpellId() const
        {
            if(m_spellProto)
                return m_spellProto->field154;
            else
                return NULL;
        }
        uint32 GetId() const
        {
            if(m_spellProto)
                return m_spellProto->Id;
            else
                return NULL;
        }

        int32 GetDuration() const { return m_duration; }
        void SetDuration(int32 duration) { m_duration = duration; }

        const uint64& GetCasterGUID() const { return m_casterGUID; }
        void SetCasterGUID(const uint64& guid) { m_casterGUID = guid; }

        const ModList& GetModList() const { return m_modList; }

        uint32 GetDamagePerTick() const { return m_damagePerTick; }
        uint32 GetHealPerTick() const { return m_healPerTick; }
        uint32 GetSpellPerTick() const { return m_spellPerTick; }

        void SetCoAffect(Affect* coAff) { m_coAffect = coAff; }
        Affect* GetCoAffect() { return m_coAffect; }

        uint8 GetAuraSlot() const { return m_auraSlot; }
        void SetAuraSlot(uint8 slot) { m_auraSlot = slot; }

        bool IsPositive() { return m_positive; }
        void SetNegative() { m_positive = false; }
        void SetPositive() { m_positive = true; }

        uint8 Update(uint32 diff)
        {
            uint8 returnV = 0;
            if (m_duration > 0)
            {
                m_duration -= diff;
                if (m_duration < 0)
                {
                    m_duration = 0;
                }
            }

            if (m_damagePerTick)
            {
                if( m_currentTick <= diff )
                {
                    m_currentTick = m_tickInterval;
                    returnV += 2;
                }
                else
                    m_currentTick -= diff;
            }
            if (m_spellPerTick)
            {
                if( m_currentSpellTick <= diff )
                {
                    m_currentSpellTick = m_tickSpellInterval;
                    returnV += 4;
                }
                else
                    m_currentSpellTick -= diff;
            }
            if (m_healPerTick)
            {
                if( m_currentHealTick <= diff )
                {
                    m_currentHealTick = m_tickHealInterval;
                    returnV += 8;
                }
                else
                    m_currentHealTick -= diff;
            }
            return returnV;
        }

    private:
        SpellEntry *m_spellProto;
        uint64 m_casterGUID;
        int32 m_duration;                         // in msecs

        uint8 m_auraSlot;

        ModList m_modList;

        uint16 m_damagePerTick;
        uint32 m_currentTick;
        uint32 m_tickInterval;

        uint32 m_spellPerTick;
        uint32 m_tickSpellInterval;
        uint32 m_currentSpellTick;

        uint32 m_healPerTick;
        uint32 m_tickHealInterval;
        uint32 m_currentHealTick;

        Affect* m_coAffect;

        bool m_positive;
};
