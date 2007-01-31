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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "UpdateMask.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Unit.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "EventSystem.h"
#include "DynamicObject.h"
#include "Group.h"
#include "UpdateData.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "RedZoneDistrict.h"
#include "CellImpl.h"
#include "Policies/SingletonImp.h"
#include "Totem.h"
#include "Creature.h"
#include "ConfusedMovementGenerator.h"
#include "Formulas.h"

pAuraHandler AuraHandler[TOTAL_AURAS]=
{
    &Aura::HandleNULL,                                      //SPELL_AURA_NONE
    &Aura::HandleBindSight,                                 //SPELL_AURA_BIND_SIGHT
    &Aura::HandleModPossess,                                //SPELL_AURA_MOD_POSSESS = 2,
    &Aura::HandlePeriodicDamage,                            //SPELL_AURA_PERIODIC_DAMAGE = 3,
    &Aura::HandleAuraDummy,                                 //SPELL_AURA_DUMMY    //missing 4
    &Aura::HandleModConfuse,                                //SPELL_AURA_MOD_CONFUSE = 5,
    &Aura::HandleModCharm,                                  //SPELL_AURA_MOD_CHARM = 6,
    &Aura::HandleModFear,                                   //SPELL_AURA_MOD_FEAR = 7,
    &Aura::HandlePeriodicHeal,                              //SPELL_AURA_PERIODIC_HEAL = 8,
    &Aura::HandleModAttackSpeed,                            //SPELL_AURA_MOD_ATTACKSPEED = 9,
    &Aura::HandleModThreat,                                 //SPELL_AURA_MOD_THREAT = 10,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_TAUNT = 11,
    &Aura::HandleAuraModStun,                               //SPELL_AURA_MOD_STUN = 12,
    &Aura::HandleModDamageDone,                             //SPELL_AURA_MOD_DAMAGE_DONE = 13,
    &Aura::HandleModDamageTaken,                            //SPELL_AURA_MOD_DAMAGE_TAKEN = 14,
    &Aura::HandleAuraDamageShield,                          //SPELL_AURA_DAMAGE_SHIELD = 15,
    &Aura::HandleModStealth,                                //SPELL_AURA_MOD_STEALTH = 16,
    &Aura::HandleModDetect,                                 //SPELL_AURA_MOD_DETECT = 17,
    &Aura::HandleInvisibility,                              //SPELL_AURA_MOD_INVISIBILITY = 18,
    &Aura::HandleInvisibilityDetect,                        //SPELL_AURA_MOD_INVISIBILITY_DETECTION = 19,
    &Aura::HandleAuraModTotalHealthPercentRegen,            //SPELL_AURA_OBS_MOD_HEALTH = 20
    &Aura::HandleAuraModTotalManaPercentRegen,              //SPELL_AURA_OBS_MOD_MANA = 21
    &Aura::HandleAuraModResistance,                         //SPELL_AURA_MOD_RESISTANCE = 22,
    &Aura::HandlePeriodicTriggerSpell,                      //SPELL_AURA_PERIODIC_TRIGGER_SPELL = 23,
    &Aura::HandlePeriodicEnergize,                          //SPELL_AURA_PERIODIC_ENERGIZE = 24,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_PACIFY = 25,
    &Aura::HandleAuraModRoot,                               //SPELL_AURA_MOD_ROOT = 26,
    &Aura::HandleAuraModSilence,                            //SPELL_AURA_MOD_SILENCE = 27,
    &Aura::HandleReflectSpells,                             //SPELL_AURA_REFLECT_SPELLS = 28,
    &Aura::HandleAuraModStat,                               //SPELL_AURA_MOD_STAT = 29,
    &Aura::HandleAuraModSkill,                              //SPELL_AURA_MOD_SKILL = 30,
    &Aura::HandleAuraModIncreaseSpeed,                      //SPELL_AURA_MOD_INCREASE_SPEED = 31,
    &Aura::HandleAuraModIncreaseMountedSpeed,               //SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED = 32,
    &Aura::HandleAuraModDecreaseSpeed,                      //SPELL_AURA_MOD_DECREASE_SPEED = 33,
    &Aura::HandleAuraModIncreaseHealth,                     //SPELL_AURA_MOD_INCREASE_HEALTH = 34,
    &Aura::HandleAuraModIncreaseEnergy,                     //SPELL_AURA_MOD_INCREASE_ENERGY = 35,
    &Aura::HandleAuraModShapeshift,                         //SPELL_AURA_MOD_SHAPESHIFT = 36,
    &Aura::HandleAuraModEffectImmunity,                     //SPELL_AURA_EFFECT_IMMUNITY = 37,
    &Aura::HandleAuraModStateImmunity,                      //SPELL_AURA_STATE_IMMUNITY = 38,
    &Aura::HandleAuraModSchoolImmunity,                     //SPELL_AURA_SCHOOL_IMMUNITY = 39,
    &Aura::HandleAuraModDmgImmunity,                        //SPELL_AURA_DAMAGE_IMMUNITY = 40,
    &Aura::HandleAuraModDispelImmunity,                     //SPELL_AURA_DISPEL_IMMUNITY = 41,
    &Aura::HandleAuraProcTriggerSpell,                      //SPELL_AURA_PROC_TRIGGER_SPELL = 42,
    &Aura::HandleAuraProcTriggerDamage,                     //SPELL_AURA_PROC_TRIGGER_DAMAGE = 43,
    &Aura::HandleAuraTrackCreatures,                        //SPELL_AURA_TRACK_CREATURES = 44,
    &Aura::HandleAuraTrackResources,                        //SPELL_AURA_TRACK_RESOURCES = 45,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_PARRY_SKILL = 46, obsolete?
    &Aura::HandleAuraModParryPercent,                       //SPELL_AURA_MOD_PARRY_PERCENT = 47,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_DODGE_SKILL = 48, obsolete?
    &Aura::HandleAuraModDodgePercent,                       //SPELL_AURA_MOD_DODGE_PERCENT = 49,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_BLOCK_SKILL = 50, obsolete?
    &Aura::HandleAuraModBlockPercent,                       //SPELL_AURA_MOD_BLOCK_PERCENT = 51,
    &Aura::HandleAuraModCritPercent,                        //SPELL_AURA_MOD_CRIT_PERCENT = 52,
    &Aura::HandlePeriodicLeech,                             //SPELL_AURA_PERIODIC_LEECH = 53,
    &Aura::HandleModHitChance,                              //SPELL_AURA_MOD_HIT_CHANCE = 54,
    &Aura::HandleModSpellHitChance,                         //SPELL_AURA_MOD_SPELL_HIT_CHANCE = 55,
    &Aura::HandleAuraTransform,                             //SPELL_AURA_TRANSFORM = 56,
    &Aura::HandleModSpellCritChance,                        //SPELL_AURA_MOD_SPELL_CRIT_CHANCE = 57,
    &Aura::HandleAuraModIncreaseSwimSpeed,                  //SPELL_AURA_MOD_INCREASE_SWIM_SPEED = 58,
    &Aura::HandleModDamageDoneCreature,                     //SPELL_AURA_MOD_DAMAGE_DONE_CREATURE = 59,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_PACIFY_SILENCE = 60,
    &Aura::HandleAuraModScale,                              //SPELL_AURA_MOD_SCALE = 61,
    &Aura::HandleNULL,                                      //SPELL_AURA_PERIODIC_HEALTH_FUNNEL = 62,
    &Aura::HandleNULL,                                      //SPELL_AURA_PERIODIC_MANA_FUNNEL = 63,
    &Aura::HandlePeriodicManaLeech,                         //SPELL_AURA_PERIODIC_MANA_LEECH = 64,
    &Aura::HandleModCastingSpeed,                           //SPELL_AURA_MOD_CASTING_SPEED = 65,
    &Aura::HandleNULL,                                      //SPELL_AURA_FEIGN_DEATH = 66,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_DISARM = 67,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_STALKED = 68,
    &Aura::HandleAuraSchoolAbsorb,                          //SPELL_AURA_SCHOOL_ABSORB = 69,
    &Aura::HandleNULL,                                      //SPELL_AURA_EXTRA_ATTACKS = 70,// Useless
    &Aura::HandleModSpellCritChanceShool,                   //SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL = 71,
    &Aura::HandleModPowerCost,                              //SPELL_AURA_MOD_POWER_COST = 72,
    &Aura::HandleModPowerCostSchool,                        //SPELL_AURA_MOD_POWER_COST_SCHOOL = 73,
    &Aura::HandleReflectSpellsSchool,                       //SPELL_AURA_REFLECT_SPELLS_SCHOOL = 74,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_LANGUAGE = 75,
    &Aura::HandleFarSight,                                  //SPELL_AURA_FAR_SIGHT = 76,
    &Aura::HandleModMechanicImmunity,                       //SPELL_AURA_MECHANIC_IMMUNITY = 77,
    &Aura::HandleAuraMounted,                               //SPELL_AURA_MOUNTED = 78,
    &Aura::HandleModDamagePercentDone,                      //SPELL_AURA_MOD_DAMAGE_PERCENT_DONE = 79,
    &Aura::HandleModPercentStat,                            //SPELL_AURA_MOD_PERCENT_STAT = 80,
    &Aura::HandleNULL,                                      //SPELL_AURA_SPLIT_DAMAGE = 81,
    &Aura::HandleWaterBreathing,                            //SPELL_AURA_WATER_BREATHING = 82,
    &Aura::HandleModBaseResistance,                         //SPELL_AURA_MOD_BASE_RESISTANCE = 83,
    &Aura::HandleModRegen,                                  //SPELL_AURA_MOD_REGEN = 84,
    &Aura::HandleModPowerRegen,                             //SPELL_AURA_MOD_POWER_REGEN = 85,
    &Aura::HandleChannelDeathItem,                          //SPELL_AURA_CHANNEL_DEATH_ITEM = 86,
    &Aura::HandleModDamagePCTTaken,                         //SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN = 87,
    &Aura::HandleModPCTRegen,                               //SPELL_AURA_MOD_HEALTH_REGEN_PERCENT = 88,
    &Aura::HandlePeriodicDamagePCT,                         //SPELL_AURA_PERIODIC_DAMAGE_PERCENT = 89,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_RESIST_CHANCE = 90,// Useless
    &Aura::HandleModDetectRange,                            //SPELL_AURA_MOD_DETECT_RANGE = 91,
    &Aura::HandleNULL,                                      //SPELL_AURA_PREVENTS_FLEEING = 92,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_UNATTACKABLE = 93,
    &Aura::HandleNULL,                                      //SPELL_AURA_INTERRUPT_REGEN = 94,
    &Aura::HandleNULL,                                      //SPELL_AURA_GHOST = 95,
    &Aura::HandleNULL,                                      //SPELL_AURA_SPELL_MAGNET = 96,
    &Aura::HandleAuraManaShield,                            //SPELL_AURA_MANA_SHIELD = 97,
    &Aura::HandleAuraModSkill,                              //SPELL_AURA_MOD_SKILL_TALENT = 98,
    &Aura::HandleAuraModAttackPower,                        //SPELL_AURA_MOD_ATTACK_POWER = 99,
    &Aura::HandleNULL,                                      //SPELL_AURA_AURAS_VISIBLE = 100,
    &Aura::HandleModResistancePercent,                      //SPELL_AURA_MOD_RESISTANCE_PCT = 101,
    &Aura::HandleModCreatureAttackPower,                    //SPELL_AURA_MOD_CREATURE_ATTACK_POWER = 102,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_TOTAL_THREAT = 103,
    &Aura::HandleAuraWaterWalk,                             //SPELL_AURA_WATER_WALK = 104,
    &Aura::HandleAuraFeatherFall,                           //SPELL_AURA_FEATHER_FALL = 105,
    &Aura::HandleAuraHover,                                 //SPELL_AURA_HOVER = 106,
    &Aura::HandleAddModifier,                               //SPELL_AURA_ADD_FLAT_MODIFIER = 107,
    &Aura::HandleAddModifier,                               //SPELL_AURA_ADD_PCT_MODIFIER = 108,
    &Aura::HandleNULL,                                      //SPELL_AURA_ADD_TARGET_TRIGGER = 109,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_POWER_REGEN_PERCENT = 110,
    &Aura::HandleNULL,                                      //SPELL_AURA_ADD_CASTER_HIT_TRIGGER = 111,
    &Aura::HandleNULL,                                      //SPELL_AURA_OVERRIDE_CLASS_SCRIPTS = 112,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN = 113,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT = 114,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_HEALING = 115,
    &Aura::HandleNULL,                                      //SPELL_AURA_IGNORE_REGEN_INTERRUPT = 116,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_MECHANIC_RESISTANCE = 117,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_HEALING_PCT = 118,
    &Aura::HandleNULL,                                      //SPELL_AURA_SHARE_PET_TRACKING = 119,
    &Aura::HandleNULL,                                      //SPELL_AURA_UNTRACKABLE = 120,
    &Aura::HandleAuraEmpathy,                               //SPELL_AURA_EMPATHY = 121,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT = 122,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_POWER_COST_PCT = 123,
    &Aura::HandleAuraModRangedAttackPower,                  //SPELL_AURA_MOD_RANGED_ATTACK_POWER = 124,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN = 125,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT = 126,
    &Aura::HandleNULL,                                      //SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS = 127,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_POSSESS_PET = 128,
    &Aura::HandleAuraModIncreaseSpeedAlways,                //SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS = 129,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS = 130,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER = 131,
    &Aura::HandleAuraModIncreaseEnergyPercent,              //SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT = 132,
    &Aura::HandleAuraModIncreaseHealthPercent,              //SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT = 133,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_MANA_REGEN_INTERRUPT = 134,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_HEALING_DONE = 135,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_HEALING_DONE_PERCENT = 136,
    &Aura::HandleModTotalPercentStat,                       //SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE = 137,
    &Aura::HandleHaste,                                     //SPELL_AURA_MOD_HASTE = 138,
    &Aura::HandleForceReaction,                             //SPELL_AURA_FORCE_REACTION = 139,
    &Aura::HandleAuraModRangedHaste,                        //SPELL_AURA_MOD_RANGED_HASTE = 140,
    &Aura::HandleRangedAmmoHaste,                           //SPELL_AURA_MOD_RANGED_AMMO_HASTE = 141,
    &Aura::HandleAuraModBaseResistancePCT,                  //SPELL_AURA_MOD_BASE_RESISTANCE_PCT = 142,
    &Aura::HandleAuraModResistanceExclusive,                //SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE = 143,
    &Aura::HandleAuraSafeFall,                              //SPELL_AURA_SAFE_FALL = 144,
    &Aura::HandleNULL,                                      //SPELL_AURA_CHARISMA = 145,
    &Aura::HandleNULL,                                      //SPELL_AURA_PERSUADED = 146,
    &Aura::HandleNULL,                                      //SPELL_AURA_ADD_CREATURE_IMMUNITY = 147,
    &Aura::HandleNULL,                                      //SPELL_AURA_RETAIN_COMBO_POINTS = 148,
    &Aura::HandleNULL,                                      //SPELL_AURA_RESIST_PUSHBACK    =    149    ,//    Resist Pushback
    &Aura::HandleModShieldBlock,                            //SPELL_AURA_MOD_SHIELD_BLOCK_PCT   = 150   ,//    Mod Shield Block %
    &Aura::HandleAuraTrackStealthed,                        //SPELL_AURA_TRACK_STEALTHED    =    151    ,//    Track Stealthed
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_DETECTED_RANGE    =    152    ,//    Mod Detected Range
    &Aura::HandleNULL,                                      //SPELL_AURA_SPLIT_DAMAGE_FLAT    =    153    ,//    Split Damage Flat
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_STEALTH_LEVEL    =    154    ,//    Stealth Level Modifier
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_WATER_BREATHING    =    155    ,//    Mod Water Breathing
    &Aura::HandleModReputationGain,                         //SPELL_AURA_MOD_REPUTATION_GAIN    =    156    ,//    Mod Reputation Gain
    &Aura::HandleNULL,                                      //SPELL_AURA_PET_DAMAGE_MULTI       = 157   ,//    Mod Pet Damage
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_SHIELD_BLOCK            = 158 ,//
    &Aura::HandleNULL,                                      //SPELL_AURA_NO_PVP_CREDIT               = 159 ,//
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_AOE_AVOIDANCE           = 160 ,//
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT  = 161 ,//
    &Aura::HandleNULL                                       //SPELL_AURA_POWER_BURN_MANA             = 162 ,//
};

Aura::Aura(SpellEntry const* spellproto, uint32 eff, Unit *target, Unit *caster, Item* castItem) :
m_spellId(spellproto->Id), m_effIndex(eff), m_caster_guid(0), m_target(target),
m_timeCla(1000), m_castItem(castItem), m_auraSlot(0),m_positive(false), m_permanent(false),
m_isPeriodic(false), m_isTrigger(false), m_periodicTimer(0), m_PeriodicEventId(0),
m_procCharges(0), m_absorbDmg(0), m_isPersistent(false), m_removeOnDeath(false),
m_isAreaAura(false)
{
    assert(target);

    // make own copy of custom `spellproto` (`spellproto` will be deleted at spell cast finished)
    // copy custom SpellEntry in m_spellProto will be delete at Aura delete
    if(spellproto != sSpellStore.LookupEntry( spellproto->Id ))
    {
        SpellEntry* sInfo = new SpellEntry;
        *sInfo = *spellproto;
        m_spellProto = sInfo;
    }
    else
        m_spellProto = spellproto;

    m_duration = GetDuration(spellproto);
    int32 maxduration = GetMaxDuration(spellproto);
    if(m_duration == -1)
        m_permanent = true;
    if( m_duration != maxduration )
    {
        uint8 comboPoints=0;
        if(caster->GetTypeId() == TYPEID_PLAYER)
        {
            comboPoints = (uint8)((caster->GetUInt32Value(PLAYER_FIELD_BYTES) & 0xFF00) >> 8);
            caster->SetUInt32Value(PLAYER_FIELD_BYTES,((caster->GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (0x00 << 8)));
        }
        comboPoints = comboPoints < 5 ? comboPoints : 5;
        m_duration += int32((maxduration - m_duration) * comboPoints / 5);
    }
    m_isPassive = IsPassiveSpell(m_spellId);
    m_positive = IsPositiveEffect(m_spellId, m_effIndex);
    m_applyTime = time(NULL);

    sLog.outDebug("Aura: construct Spellid : %u, Aura : %u Duration : %d Target : %d.", spellproto->Id, spellproto->EffectApplyAuraName[eff], m_duration, spellproto->EffectImplicitTargetA[eff]);

    uint32 type = 0;
    if(!m_positive)
        type = 1;
    int32 damage;
    if(!caster)
    {
        m_caster_guid = target->GetGUID();
        damage = spellproto->EffectBasePoints[eff]+1;       // stored value-1
    }
    else
    {
        m_caster_guid = caster->GetGUID();
        damage = CalculateDamage();
    }

    m_effIndex = eff;
    SetModifier(spellproto->EffectApplyAuraName[eff], damage, spellproto->EffectAmplitude[eff], spellproto->EffectMiscValue[eff], type);
}

Aura::~Aura()
{
    // free custom m_spellProto
    if(m_spellProto != sSpellStore.LookupEntry(m_spellProto->Id))
        delete m_spellProto;
}

AreaAura::AreaAura(SpellEntry const* spellproto, uint32 eff, Unit *target,
Unit *caster, Item* castItem) : Aura(spellproto, eff, target, caster, castItem)
{
    m_isAreaAura = true;
}

AreaAura::~AreaAura()
{
}

PersistentAreaAura::PersistentAreaAura(SpellEntry const* spellproto, uint32 eff, Unit *target,
Unit *caster, Item* castItem) : Aura(spellproto, eff, target, caster, castItem)
{
    m_isPersistent = true;
}

PersistentAreaAura::~PersistentAreaAura()
{
}

Unit* Aura::GetCaster() const
{
    if(m_caster_guid==m_target->GetGUID())
        return m_target;

    return ObjectAccessor::Instance().GetUnit(*m_target,m_caster_guid);
}

int32 Aura::CalculateDamage()
{
    SpellEntry const* spellproto = GetSpellProto();
    int32 value = 0;
    uint32 level = 0;
    if(!m_target)
        return 0;
    Unit* caster = GetCaster();
    if(!caster)
        caster = m_target;
    /*level= caster->getLevel();
    if( level > spellproto->maxLevel && spellproto->maxLevel > 0)
        level = spellproto->maxLevel;*/

    float basePointsPerLevel = spellproto->EffectRealPointsPerLevel[m_effIndex];
    float randomPointsPerLevel = spellproto->EffectDicePerLevel[m_effIndex];
    int32 basePoints = int32(spellproto->EffectBasePoints[m_effIndex] + level * basePointsPerLevel);
    int32 randomPoints = int32(spellproto->EffectDieSides[m_effIndex] + level * randomPointsPerLevel);
    float comboDamage = spellproto->EffectPointsPerComboPoint[m_effIndex];
    uint8 comboPoints=0;
    if(caster->GetTypeId() == TYPEID_PLAYER)
        comboPoints = (uint8)((caster->GetUInt32Value(PLAYER_FIELD_BYTES) & 0xFF00) >> 8);

    value += spellproto->EffectBaseDice[m_effIndex];
    if(randomPoints <= 1)
        value = basePoints+1;
    else
        value = basePoints+rand()%randomPoints;

    if(comboDamage > 0)
    {
        value += (int32)(comboDamage * comboPoints);
        if(caster->GetTypeId() == TYPEID_PLAYER)
            caster->SetUInt32Value(PLAYER_FIELD_BYTES,((caster->GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (0x00 << 8)));
    }

    return value;
}

void Aura::SetModifier(uint8 t, int32 a, uint32 pt, int32 miscValue, uint32 miscValue2)
{
    m_modifier.m_auraname = t;
    m_modifier.m_amount   = a;
    m_modifier.m_miscvalue = miscValue;
    m_modifier.m_miscvalue2 = miscValue2;
    m_modifier.periodictime = pt;

    Unit* caster = GetCaster();
    if (caster && caster->GetTypeId() == TYPEID_PLAYER)
        ((Player *)caster)->ApplySpellMod(m_spellId,SPELLMOD_ALL_EFFECTS, m_modifier.m_amount);
}

void Aura::Update(uint32 diff)
{
    if (m_duration > 0)
    {
        m_duration -= diff;
        if (m_duration < 0)
            m_duration = 0;
        m_timeCla -= diff;

        Unit* caster = GetCaster();
        if(caster && m_timeCla <= 0)
        {
            Powers powertype = caster->getPowerType();
            int32 manaPerSecond = GetSpellProto()->manaPerSecond;
            int32 manaPerSecondPerLevel = uint32(GetSpellProto()->manaPerSecondPerLevel*caster->getLevel());
            m_timeCla = 1000;
            caster->ModifyPower(powertype,-manaPerSecond);
            caster->ModifyPower(powertype,-manaPerSecondPerLevel);
        }
        if(caster && m_target->isAlive() && m_target->HasFlag(UNIT_FIELD_FLAGS,(UNIT_STAT_FLEEING<<16)))
        {
            float x,y,z,angle,speed,pos_x,pos_y,pos_z;
            angle = m_target->GetAngle( caster->GetPositionX(), caster->GetPositionY() );
            // If the m_target is player,and if the speed is too slow,change it :P
            speed = m_target->GetSpeed(MOVE_RUN);
            pos_x = m_target->GetPositionX();
            pos_y = m_target->GetPositionY();
            uint32 mapid = m_target->GetMapId();
            pos_z = MapManager::Instance().GetMap(mapid)->GetHeight(pos_x,pos_y);
            // Control the max Distance; 20 for temp.
            if(m_target->IsWithinDist(caster, 20))
            {
                if( m_target->GetPositionX() < caster->GetPositionX() || m_target->GetPositionY() > caster->GetPositionY() )
                    x = m_target->GetPositionX() + speed*diff * sin(angle)/1000;
                else
                    x = m_target->GetPositionX() - speed*diff * sin(angle)/1000;
                y = m_target->GetPositionY() - speed*diff * cos(angle)/1000;
                mapid = m_target->GetMapId();
                z = MapManager::Instance().GetMap(mapid)->GetHeight(x,y);
                // Control the target to not climb or drop when dz > |x|,x = 1.3 for temp.
                // fixed me if it needs checking when the position will be in water?
                if(z<=pos_z+1.3 && z>=pos_z-1.3)
                {
                    m_target->SendMonsterMove(x,y,z,false,true,diff);
                    if(m_target->GetTypeId() != TYPEID_PLAYER)
                        m_target->Relocate(x,y,z,m_target->GetOrientation());
                }
            }
        }
    }

    if(m_isPeriodic && (m_duration >= 0 || m_isPassive))
    {
        m_periodicTimer -= diff;
        if(m_periodicTimer < 0)
        {
            if( m_modifier.m_auraname == SPELL_AURA_MOD_REGEN ||
                m_modifier.m_auraname == SPELL_AURA_MOD_POWER_REGEN ||
                                                            // Cannibalize, eating items and other spells
                m_modifier.m_auraname == SPELL_AURA_OBS_MOD_HEALTH ||
                                                            // Eating items and other spells
                m_modifier.m_auraname == SPELL_AURA_OBS_MOD_MANA )
            {
                ApplyModifier(true);
                return;
            }
            // update before applying (aura can be removed in TriggerSpell or PeriodicAuraLog calls)
            m_periodicTimer += m_modifier.periodictime;

            if(m_isTrigger)
            {
                TriggerSpell();
            }
            else
            {
                if(Unit* caster = GetCaster())
                    caster->PeriodicAuraLog(m_target, GetSpellProto(), &m_modifier);
                else
                    m_target->PeriodicAuraLog(m_target, GetSpellProto(), &m_modifier);
            }
        }
    }
}

void AreaAura::Update(uint32 diff)
{
    // update for the caster of the aura
    if(m_caster_guid == m_target->GetGUID())
    {
        Unit* caster = m_target;

        Group *pGroup = NULL;
        if (caster->GetTypeId() == TYPEID_PLAYER)
            pGroup = ((Player*)caster)->groupInfo.group;
        else if(((Creature*)caster)->isTotem())
        {
            Unit *owner = ((Totem*)caster)->GetOwner();
            if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                pGroup = ((Player*)owner)->groupInfo.group;
        }

        float radius =  GetRadius(sSpellRadiusStore.LookupEntry(GetSpellProto()->EffectRadiusIndex[m_effIndex]));
        if(pGroup)
        {
            for(uint32 p=0;p<pGroup->GetMembersCount();p++)
            {
                Unit* Target = objmgr.GetPlayer(pGroup->GetMemberGUID(p));

                if (caster->GetTypeId() == TYPEID_PLAYER)
                {
                    if(!Target || Target->GetGUID() == m_caster_guid || !Target->isAlive() || !pGroup->SameSubGroup(m_caster_guid, Target->GetGUID()))
                        continue;
                }
                else if(((Creature*)caster)->isTotem())
                {
                    Unit *owner = ((Totem*)caster)->GetOwner();
                    if(!Target || !Target->isAlive() || !pGroup->SameSubGroup(owner->GetGUID(), Target->GetGUID()))
                        continue;
                }

                Aura *t_aura = Target->GetAura(m_spellId, m_effIndex);

                if(caster->IsWithinDist(Target, radius) )
                {
                    // apply aura to players in range that dont have it yet
                    if (!t_aura)
                    {
                        AreaAura *aur = new AreaAura(GetSpellProto(), m_effIndex, Target, caster);
                        Target->AddAura(aur);
                    }
                }
                else
                {
                    // remove auras of the same caster from out of range players
                    if (t_aura)
                        if (t_aura->GetCasterGUID() == m_caster_guid)
                            Target->RemoveAura(m_spellId, m_effIndex);
                }
            }
        }
        else if (caster->GetTypeId() != TYPEID_PLAYER && ((Creature*)caster)->isTotem())
        {
            // add / remove auras from the totem's owner
            Unit *owner = ((Totem*)caster)->GetOwner();
            if (owner)
            {
                Aura *o_aura = owner->GetAura(m_spellId, m_effIndex);
                if(caster->IsWithinDist(owner, radius))
                {
                    if (!o_aura)
                    {
			AreaAura *aur = new AreaAura(GetSpellProto(), m_effIndex, owner, caster);
                        owner->AddAura(aur);
                    }
                }
                else
                {
                    if (o_aura)
                        if (o_aura->GetCasterGUID() == m_caster_guid)
                            owner->RemoveAura(m_spellId, m_effIndex);
                }
            }
        }
    }

    Aura::Update(diff);
}

void PersistentAreaAura::Update(uint32 diff)
{
    bool remove = false;

    // remove the aura if its caster or the dynamic object causing it was removed
    // or if the target moves too far from the dynamic object
    Unit *caster = GetCaster();
    if (caster)
    {
        DynamicObject *dynObj = caster->GetDynObject(GetId(), GetEffIndex());
        if (dynObj)
        {
            if (!m_target->IsWithinDist(dynObj, dynObj->GetRadius()))
                remove = true;
        }
        else
            remove = true;
    }
    else
        remove = true;

    Aura::Update(diff);

    if(remove)
        m_target->RemoveAura(GetId(), GetEffIndex());
}

void Aura::ApplyModifier(bool apply, bool Real)
{
    uint8 aura = 0;
    aura = m_modifier.m_auraname;

    if(aura<TOTAL_AURAS)
        (*this.*AuraHandler [aura])(apply,Real);
}

void Aura::UpdateAuraDuration()
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    if(m_isPassive)
        return;

    WorldPacket data(SMSG_UPDATE_AURA_DURATION, 5);
    data << (uint8)m_auraSlot << (uint32)m_duration;
    //((Player*)m_target)->SendMessageToSet(&data, true); //GetSession()->SendPacket(&data);
    ((Player*)m_target)->SendDirectMessage(&data);
}

void Aura::_AddAura()
{
    if (!m_spellId)
        return;
    if(!m_target)
        return;

    bool samespell = false;
    uint8 slot = 0xFF, i;
    Aura* aura = NULL;

    for(i = 0; i < 3; i++)
    {
        aura = m_target->GetAura(m_spellId, i);
        if(aura)
        {
            //if (i != m_effIndex)
            {
                samespell = true;
                slot = aura->GetAuraSlot();
            }
        }
    }

    // not call total regen auras at adding
    if( m_modifier.m_auraname==SPELL_AURA_OBS_MOD_HEALTH || m_modifier.m_auraname==SPELL_AURA_OBS_MOD_MANA )
        m_periodicTimer = m_modifier.periodictime;
    else
    if( m_modifier.m_auraname==SPELL_AURA_MOD_REGEN || m_modifier.m_auraname==SPELL_AURA_MOD_POWER_REGEN )
        m_periodicTimer = 5000;

    m_target->ApplyStats(false);
    ApplyModifier(true,true);
    m_target->ApplyStats(true);

    sLog.outDebug("Aura %u now is in use", m_modifier.m_auraname);

    Unit* caster = GetCaster();

    // passive auras (except totem auras) do not get placed in the slots
    if(!m_isPassive || (caster && caster->GetTypeId() == TYPEID_UNIT && ((Creature*)caster)->isTotem()))
    {
        if(!samespell)
        {
            if (IsPositive())
            {
                for (i = 0; i < MAX_POSITIVE_AURAS; i++)
                {
                    if (m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURA + i)) == 0)
                    {
                        slot = i;
                        break;
                    }
                }
            }
            else
            {
                for (i = MAX_POSITIVE_AURAS; i < MAX_AURAS; i++)
                {
                    if (m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURA + i)) == 0)
                    {
                        slot = i;
                        break;
                    }
                }
            }

            m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURA + slot), GetId());

            uint8 flagslot = slot >> 3;
            uint32 value = m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot));

            uint8 value1 = (slot & 7) << 2;
            value |= ((uint32)AFLAG_SET << value1);

            m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot), value);
        }
        else
        {
            /* TODO: increase count */
        }

        if(GetSpellProto()->SpellVisual == 5622)
            m_target->SetFlag(UNIT_FIELD_AURASTATE, uint32(1<<(AURA_STATE_JUDGEMENT-1)));
        SetAuraSlot( slot );
        if( m_target->GetTypeId() == TYPEID_PLAYER )
            UpdateAuraDuration();
    }

}

void Aura::_RemoveAura()
{
    m_target->ApplyStats(false);
    sLog.outDebug("Aura %u now is remove", m_modifier.m_auraname);
    ApplyModifier(false,true);
    m_target->ApplyStats(true);

    Unit* caster = GetCaster();

    if(caster && IsPersistent())
    {
        DynamicObject *dynObj = caster->GetDynObject(GetId(), GetEffIndex());
        if (dynObj)
            dynObj->RemoveAffected(m_target);
    }
                                                            //passive auras do not get put in slots
    if(m_isPassive && !(caster && caster->GetTypeId() == TYPEID_UNIT && ((Creature*)caster)->isTotem()))
        return;

    uint8 slot = GetAuraSlot();

    // Aura added always to m_target
    //Aura* aura = m_target->GetAura(m_spellId, m_effIndex);
    //if(!aura)
    //    return;

    if(m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURA + slot)) == 0)
        return;

    // count all auras from each effect of the spell
    Unit::AuraMap& t_Auras = m_target->GetAuras();
    uint8 count[3], totalcount = 0;
    for(uint8 i = 0; i < 3; i++)
    {
        count[i] = t_Auras.count(Unit::spellEffectPair(m_spellId, i));
        totalcount += count[i];
    }

    /*
    count[m_effIndex]--;
    // all counts should be the same after the last effect of a spell was taken out
    for(uint8 i = 0; i < 3; i++)
        if (count[i] > count[m_effIndex])
            break;
    if (i == 3)
        TODO: decrease count for spell
    */

    // only remove icon when the last aura of the spell is removed (current aura already removed from list)
    if (totalcount > 0)
        return;

    m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURA + slot), 0);

    uint8 flagslot = slot >> 3;

    uint32 value = m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot));

    uint8 aurapos = (slot & 7) << 2;
    uint32 value1 = ~( AFLAG_SET << aurapos );
    value &= value1;

    m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot), value);
    if(GetSpellProto()->SpellVisual == 5622)
        m_target->RemoveFlag(UNIT_FIELD_AURASTATE, uint32(1<<(AURA_STATE_JUDGEMENT-1)));
}

/*********************************************************/
/***               BASIC AURA FUNCTION                 ***/
/*********************************************************/

void Aura::HandleNULL(bool apply, bool Real)
{
}

void Aura::HandleAddModifier(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *p_target = (Player *)m_target;

    SpellEntry const *spellInfo = GetSpellProto();
    if(!spellInfo) return;

    uint8 op = spellInfo->EffectMiscValue[m_effIndex];
    int32 value = spellInfo->EffectBasePoints[m_effIndex]+1;
    uint8 type = spellInfo->EffectApplyAuraName[m_effIndex];
    uint32 mask = spellInfo->EffectItemType[m_effIndex];
    if (!op) return;
    SpellModList *p_mods = p_target->getSpellModList(op);
    if (!p_mods) return;

    if (apply)
    {
        SpellModifier *mod = new SpellModifier;
        mod->op = op;
        mod->value = value;
        mod->type = type;
        mod->mask = mask;
        mod->spellId = m_spellId;
        mod->charges = spellInfo->procCharges;
        p_mods->push_back(mod);
        m_spellmod = mod;

        uint16 send_val=0, send_mark=0;
        int16 tmpval=spellInfo->EffectBasePoints[m_effIndex];
        uint32 shiftdata=0x01, Opcode=SMSG_SET_FLAT_SPELL_MODIFIER;

        if(tmpval != 0)
        {
            if(tmpval > 0)
            {
                send_val =  tmpval+1;
                send_mark = 0x0;
            }
            else
            {
                send_val  = 0xFFFF + (tmpval+2);
                send_mark = 0xFFFF;
            }
        }

        if (mod->type == SPELLMOD_FLAT) Opcode = SMSG_SET_FLAT_SPELL_MODIFIER;
        else if (mod->type == SPELLMOD_PCT) Opcode = SMSG_SET_PCT_SPELL_MODIFIER;

        for(int eff=0;eff<32;eff++)
        {
            if ( mask & shiftdata )
            {
                WorldPacket data(Opcode, (1+1+2+2));
                data << uint8(eff);
                data << uint8(mod->op);
                data << uint16(send_val);
                data << uint16(send_mark);
                p_target->SendDirectMessage(&data);
            }
            shiftdata=shiftdata<<1;
        }
    }
    else
    {
        SpellModList *p_mods = p_target->getSpellModList(spellInfo->EffectMiscValue[m_effIndex]);
        p_mods->remove(this->m_spellmod);
    }
}

void Aura::TriggerSpell()
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry( GetSpellProto()->EffectTriggerSpell[m_effIndex] );

    if(!spellInfo)
    {
        sLog.outError("Auras: unknown TriggerSpell:%i From spell: %i",  GetSpellProto()->EffectTriggerSpell[m_effIndex],GetSpellProto()->Id);
        return;
    }
    if(GetSpellProto()->Id == 1515)
        spellInfo = sSpellStore.LookupEntry( 13481 );

    Unit* caster = GetCaster();

    Spell spell(caster, spellInfo, true, this);
    Unit* target = m_target;
    if(!target && caster && caster->GetTypeId() == TYPEID_PLAYER)
    {
        target = ObjectAccessor::Instance().GetUnit(*caster, ((Player*)caster)->GetSelection());
    }
    if(!target)
        return;
    SpellCastTargets targets;
    targets.setUnitTarget(target);
    spell.prepare(&targets);
}

/*********************************************************/
/***                  AURA EFFECTS                     ***/
/*********************************************************/

void Aura::HandleAuraDummy(bool apply, bool Real)
{
    Unit* caster = GetCaster();

    // currently all dummy auras applyed/un-applied only at real add/remove
    if(!Real)
        return;

    if(apply && !m_procCharges)
    {
        m_procCharges = GetSpellProto()->procCharges;
        if (!m_procCharges)
            m_procCharges = -1;
    }

    if(GetSpellProto()->SpellVisual == 5622 && caster && caster->GetTypeId() == TYPEID_PLAYER)
    {
        if(GetSpellProto()->SpellIconID == 25 && GetEffIndex() == 0)
        {
            Unit::AuraList& tAuraProcTriggerDamage = m_target->GetAurasByType(SPELL_AURA_PROC_TRIGGER_DAMAGE);
            if(apply)
                tAuraProcTriggerDamage.push_back(this);
            else
                tAuraProcTriggerDamage.remove(this);
        }
    }

    if(GetSpellProto()->SpellVisual == 7395 && GetSpellProto()->SpellIconID == 278 && caster->GetTypeId() == TYPEID_PLAYER)
    {
        Unit::AuraList& tAuraProcTriggerDamage = m_target->GetAurasByType(SPELL_AURA_PROC_TRIGGER_DAMAGE);
        if(apply)
            tAuraProcTriggerDamage.push_back(this);
        else
            tAuraProcTriggerDamage.remove(this);
    }

    // only at real add/remove
    if(GetSpellProto()->SpellVisual == 99 && GetSpellProto()->SpellIconID == 92 &&
        caster && caster->GetTypeId() == TYPEID_PLAYER && m_target && m_target->GetTypeId() == TYPEID_PLAYER)
    {
        Player * player = (Player*)m_target;
        if(apply)
        {
            uint32 spellid = 0;
            switch(GetId())
            {
                case 20707:spellid = 3026;break;
                case 20762:spellid = 20758;break;
                case 20763:spellid = 20759;break;
                case 20764:spellid = 20760;break;
                case 20765:spellid = 20761;break;
                default:break;
            }
            if(!spellid)
                return;
            player->SetSoulStoneSpell(spellid);
        }
        else
            player->SetSoulStoneSpell(0);
    }

    if(!apply)
    {
        if( IsQuestTameSpell(GetId()) && caster && caster->isAlive() && m_target && m_target->isAlive())
        {
            uint32 finalSpelId = 0;
            switch(GetId())
            {
                case 19548: finalSpelId = 19597; break;
                case 19674: finalSpelId = 19677; break;
                case 19687: finalSpelId = 19676; break;
                case 19688: finalSpelId = 19678; break;
                case 19689: finalSpelId = 19679; break;
                case 19692: finalSpelId = 19680; break;
                case 19693: finalSpelId = 19684; break;
                case 19694: finalSpelId = 19681; break;
                case 19696: finalSpelId = 19682; break;
                case 19697: finalSpelId = 19683; break;
                case 19699: finalSpelId = 19685; break;
                case 19700: finalSpelId = 19686; break;
            }

            if(finalSpelId)
            {
                SpellEntry const *spell_proto = sSpellStore.LookupEntry(finalSpelId);
                if(!spell_proto)
                    return;

                Spell spell(caster, spell_proto, true, 0);
                SpellCastTargets targets;
                targets.setUnitTarget(m_target);
                // prevent double stat apply for triggered auras
                m_target->ApplyStats(true);
                spell.prepare(&targets);
                m_target->ApplyStats(false);
            }
        }
    }
}

void Aura::HandleAuraMounted(bool apply, bool Real)
{
    if(apply)
    {
        CreatureInfo const* ci = objmgr.GetCreatureTemplate(m_modifier.m_miscvalue);
        if(!ci)return;
        uint32 displayId = ci->randomDisplayID();
        if(displayId != 0)
            m_target->Mount(displayId);
    }else
    {
        m_target->Unmount();
    }
}

void Aura::HandleAuraWaterWalk(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_WATER_WALK, 8);
    else
        data.Initialize(SMSG_MOVE_LAND_WALK, 8);
    data.append(m_target->GetPackGUID());
    m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraFeatherFall(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_FEATHER_FALL, 8);
    else
        data.Initialize(SMSG_MOVE_NORMAL_FALL, 8);
    data.append(m_target->GetPackGUID());
    m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraHover(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_SET_HOVER, 8);
    else
        data.Initialize(SMSG_MOVE_UNSET_HOVER, 8);
    data.append(m_target->GetPackGUID());
    m_target->SendMessageToSet(&data,true);
}

void Aura::HandleWaterBreathing(bool apply, bool Real)
{
    m_target->waterbreath = apply;
}

void Aura::HandleAuraModShapeshift(bool apply, bool Real)
{
    if(!m_target)
        return;
    if(!Real)
        return;

    Player* player = m_target->GetTypeId()==TYPEID_PLAYER ? ((Player*)m_target) : NULL;

    // remove for old form
    if(player)
    {
        player->_ApplyStatsMods();
        player->_RemoveAllItemMods();
        player->_RemoveAllAuraMods();
        player->_RemoveStatsMods();
    }

    Unit *unit_target = m_target;
    uint32 modelid = 0;
    Powers PowerType = POWER_MANA;
    uint32 new_bytes_1 = m_modifier.m_miscvalue;
    switch(m_modifier.m_miscvalue)
    {
        case FORM_CAT:
            if(unit_target->getRace() == RACE_NIGHTELF)
                modelid = 892;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 8571;
            PowerType = POWER_ENERGY;
            break;
        case FORM_TRAVEL:
            modelid = 632;
            break;
        case FORM_AQUA:
            if(unit_target->getRace() == RACE_NIGHTELF)
                modelid = 2428;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 2428;
            break;
        case FORM_BEAR:
            if(unit_target->getRace() == RACE_NIGHTELF)
                modelid = 2281;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 2289;
            PowerType = POWER_RAGE;
            break;
        case FORM_GHOUL:
            if(unit_target->getRace() == RACE_NIGHTELF)
                modelid = 10045;
            break;
        case FORM_DIREBEAR:
            if(unit_target->getRace() == RACE_NIGHTELF)
                modelid = 2281;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 2289;
            PowerType = POWER_RAGE;
            break;
        case FORM_CREATUREBEAR:
            modelid = 902;
            break;
        case FORM_GHOSTWOLF:
            modelid = 4613;
            break;
        case FORM_MOONKIN:
            if(unit_target->getRace() == RACE_NIGHTELF)
                modelid = 15374;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 15375;
            break;
        case FORM_AMBIENT:
        case FORM_BATTLESTANCE:
        case FORM_BERSERKERSTANCE:
        case FORM_DEFENSIVESTANCE:
        case FORM_SHADOW:
        case FORM_STEALTH:
        case FORM_TREE:
            break;
        default:
            sLog.outError("Auras: Unknown Shapeshift Type: %u", m_modifier.m_miscvalue);
    }

    if(apply)
    {
        unit_target->SetFlag(UNIT_FIELD_BYTES_1, (new_bytes_1<<16) );
        if(modelid > 0)
        {
            unit_target->SetUInt32Value(UNIT_FIELD_DISPLAYID,modelid);
        }

        if(PowerType != POWER_MANA)
        {
            unit_target->setPowerType(PowerType);

            // energy in cat start with 0.
            //TODO: implement SPELL_AURA_ADD_TARGET_TRIGGER that used for receiving with some chance non-0 energy at transformation
            if(m_modifier.m_miscvalue == FORM_CAT)
                unit_target->SetPower(POWER_ENERGY,0);
        }

        unit_target->m_ShapeShiftForm = m_spellId;
        unit_target->m_form = m_modifier.m_miscvalue;
        if(unit_target->m_form == FORM_DIREBEAR)
            if (m_target->getRace() == RACE_TAUREN)
        {
            m_target->SetFloatValue(OBJECT_FIELD_SCALE_X,1.35f);
        }
        else
            m_target->SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);
    }
    else
    {
        if (m_target->getRace() == RACE_TAUREN)
            unit_target->SetFloatValue(OBJECT_FIELD_SCALE_X,1.35f);
        else
            unit_target->SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);
        unit_target->SetUInt32Value(UNIT_FIELD_DISPLAYID,unit_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
        unit_target->RemoveFlag(UNIT_FIELD_BYTES_1, (new_bytes_1<<16) );
        if(unit_target->getClass() == CLASS_DRUID)
            unit_target->setPowerType(POWER_MANA);
        unit_target->m_ShapeShiftForm = 0;
        unit_target->m_form = 0;
    }

    if(player)
        player->InitDataForForm();

    // apply for new form
    if(player)
    {
        player->_ApplyStatsMods();
        player->_ApplyAllAuraMods();
        player->_ApplyAllItemMods();
        player->_RemoveStatsMods();
    }
}

void Aura::HandleAuraTransform(bool apply, bool Real)
{
    if(!m_target)
        return;

    if (apply)
    {
        // special case
        if(m_modifier.m_miscvalue==0)
        {
            if(m_target->GetTypeId()!=TYPEID_PLAYER)
                return;

            uint32 orb_model = m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
            switch(orb_model)
            {
                // Troll Female
                case 1479: m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10134); break;
                // Troll Male
                case 1478: m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10135); break;
                // Tauren Male
                case 59:   m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10136); break;
                // Human Male
                case 49:   m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10137); break;
                // Human Female
                case 50:   m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10138); break;
                // Orc Male
                case 51:   m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10139); break;
                // Orc Female
                case 52:   m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10140); break;
                // Dwarf Male
                case 53:   m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10141); break;
                // Dwarf Female
                case 54:   m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10142); break;
                // NightElf Male
                case 55:   m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10143); break;
                // NightElf Female
                case 56:   m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10144); break;
                // Undead Female
                case 58:   m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10145); break;
                // Undead Male
                case 57:   m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10146); break;
                // Tauren Female
                case 60:   m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10147); break;
                // Gnome Male
                case 1563: m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10148); break;
                // Gnome Female
                case 1564: m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10149); break;
                default: break;
            }
        }
        else
        {
            CreatureInfo const * ci = objmgr.GetCreatureTemplate(m_modifier.m_miscvalue);
            if(!ci)
            {
                                                            //pig pink ^_^
                m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, 16358);
                sLog.outError("Auras: unknown creature id = %d (only need its modelid) Form Spell Aura Transform in Spell ID = %d", m_modifier.m_miscvalue, GetSpellProto()->Id);
            }
            else
            {
                m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, ci->randomDisplayID());
            }
            m_target->setTransForm(GetSpellProto()->Id);
        }
    }
    else
    {
        m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
        m_target->setTransForm(0);
    }

    Unit* caster = GetCaster();

    if(caster && caster->GetTypeId() == TYPEID_PLAYER)
        m_target->SendUpdateToPlayer((Player*)caster);
}

void Aura::HandleForceReaction(bool Apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(Apply)
    {
        uint32 faction_id = m_modifier.m_miscvalue;

        FactionTemplateEntry const *factionTemplateEntry = NULL;

        for(uint32 i = 0; i <  sFactionTemplateStore.GetNumRows(); ++i)
        {
            factionTemplateEntry = sFactionTemplateStore.LookupEntry(i);
            if(!factionTemplateEntry)
                continue;

            if(factionTemplateEntry->faction == faction_id)
                break;
        }

        if(!factionTemplateEntry)
            return;

        m_target->setFaction(factionTemplateEntry->ID);
    }
    else
        ((Player*)m_target)->setFactionForRace(((Player*)m_target)->getRace());
}

void Aura::HandleAuraModSkill(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 prot=GetSpellProto()->EffectMiscValue[m_effIndex];
    int32 points = GetSpellProto()->EffectBasePoints[m_effIndex]+1;

    ((Player*)m_target)->ModifySkillBonus(prot,(apply ? points: -points));
    if(prot == SKILL_DEFENSE)
        ((Player*)m_target)->ApplyDefenseBonusesMod(points, apply);
}

void Aura::HandleChannelDeathItem(bool apply, bool Real)
{
    if(!apply)
    {
        Unit* caster = GetCaster();
        Unit* victim = GetTarget();
        if(!caster || caster->GetTypeId() != TYPEID_PLAYER || !victim || !m_removeOnDeath)
            return;

        SpellEntry const *spellInfo = GetSpellProto();
        if(spellInfo->EffectItemType[m_effIndex] == 0)
            return;

        // Soul Shard only from non-grey units
        if( victim->getLevel() <= MaNGOS::XP::GetGrayLevel(caster->getLevel()) &&  spellInfo->EffectItemType[m_effIndex] == 6265)
            return;

        uint16 dest;
        uint8 msg = ((Player*)caster)->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, spellInfo->EffectItemType[m_effIndex], 1, false);
        if( msg == EQUIP_ERR_OK )
            ((Player*)caster)->StoreNewItem(dest, spellInfo->EffectItemType[m_effIndex], 1, true);
        else
            ((Player*)caster)->SendEquipError( msg, NULL, NULL );
    }
}

void Aura::HandleAuraSafeFall(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_FEATHER_FALL, 8);
    else
        data.Initialize(SMSG_MOVE_NORMAL_FALL, 8);
    data.append(m_target->GetPackGUID());
    m_target->SendMessageToSet(&data,true);
}

void Aura::HandleBindSight(bool apply, bool Real)
{
    if(!m_target)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    m_target->SetUInt64Value(PLAYER_FARSIGHT,apply ? m_target->GetGUID() : 0);
}

void Aura::HandleFarSight(bool apply, bool Real)
{
    if(!m_target)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    m_target->SetUInt64Value(PLAYER_FARSIGHT,apply ? m_modifier.m_miscvalue : 0);
}

void Aura::HandleAuraTrackCreatures(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    if(apply)
        m_target->RemoveNoStackAurasDueToAura(this);
    m_target->SetUInt32Value(PLAYER_TRACK_CREATURES, apply ? ((uint32)1)<<(m_modifier.m_miscvalue-1) : 0 );
}

void Aura::HandleAuraTrackResources(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    if(apply)
        m_target->RemoveNoStackAurasDueToAura(this);
    m_target->SetUInt32Value(PLAYER_TRACK_RESOURCES, apply ? ((uint32)1)<<(m_modifier.m_miscvalue-1): 0 );
}

void Aura::HandleAuraTrackStealthed(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    if(apply)
        m_target->RemoveNoStackAurasDueToAura(this);

    //TODO: add stealthed tracking effect
}

void Aura::HandleAuraModScale(bool apply, bool Real)
{
    m_target->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE_X,m_modifier.m_amount,apply);
}

void Aura::HandleModPossess(bool apply, bool Real)
{
    if(!m_target)
        return;

    if(m_target->GetTypeId() != TYPEID_UNIT)
        return;

    if(!Real)
        return;

    Creature* creatureTarget = (Creature*)m_target;

    if(int32(m_target->getLevel()) <= m_modifier.m_amount)
    {
        CreatureInfo const *cinfo = ((Creature*)m_target)->GetCreatureInfo();
        if( apply )
        {
            Unit* caster = GetCaster();
            if(caster)
            {
                creatureTarget->AttackStop();
                if(caster->getVictim()==creatureTarget)
                    caster->AttackStop();

                creatureTarget->CombatStop();

                creatureTarget->SetUInt64Value(UNIT_FIELD_CHARMEDBY,caster->GetGUID());
                creatureTarget->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,caster->getFaction());
                caster->SetCharm(creatureTarget);
                creatureTarget->AIM_Initialize();

                if(caster->GetTypeId() == TYPEID_PLAYER)
                {
                    ((Player*)caster)->PetSpellInitialize();
                }
            }
        }
        else
        {
            creatureTarget->SetUInt64Value(UNIT_FIELD_CHARMEDBY,0);
            creatureTarget->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,cinfo->faction);

            if(Unit* caster = GetCaster())
            {
                caster->SetCharm(0);

                if(caster->GetTypeId() == TYPEID_PLAYER)
                {
                    WorldPacket data(SMSG_PET_SPELLS, 8);
                    data << uint64(0);
                    ((Player*)caster)->GetSession()->SendPacket(&data);
                }
            }
            creatureTarget->AIM_Initialize();
        }
    }
}

void Aura::HandleModCharm(bool apply, bool Real)
{
    if(!m_target)
        return;

    if(m_target->GetTypeId() != TYPEID_UNIT)
        return;

    if(!Real)
        return;

    Creature* creatureTarget = (Creature*)m_target;

    if(int32(m_target->getLevel()) <= m_modifier.m_amount)
    {
        CreatureInfo const *cinfo = ((Creature*)m_target)->GetCreatureInfo();
        if( apply )
        {
            Unit* caster = GetCaster();
            if(caster)
            {
                creatureTarget->AttackStop();
                if(caster->getVictim()==creatureTarget)
                    caster->AttackStop();

                creatureTarget->CombatStop();

                creatureTarget->SetUInt64Value(UNIT_FIELD_CHARMEDBY,caster->GetGUID());
                creatureTarget->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,caster->getFaction());
                caster->SetCharm(creatureTarget);
                creatureTarget->AIM_Initialize();
                if(caster->GetTypeId() == TYPEID_PLAYER)
                {
                    ((Player*)caster)->PetSpellInitialize();
                }
            }
        }
        else
        {
            creatureTarget->SetUInt64Value(UNIT_FIELD_CHARMEDBY,0);
            creatureTarget->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,cinfo->faction);
            Unit* caster = GetCaster();
            if(caster)
            {
                caster->SetCharm(0);
                if(caster->GetTypeId() == TYPEID_PLAYER)
                {
                    WorldPacket data(SMSG_PET_SPELLS);
                    data << uint64(0);
                    ((Player*)caster)->GetSession()->SendPacket(&data);
                }
            }
            creatureTarget->AIM_Initialize();
        }
    }
}

void Aura::HandleModConfuse(bool apply, bool Real)
{
    uint32 apply_stat = UNIT_STAT_CONFUSED;
    if( apply )
    {
        m_target->addUnitState(UNIT_STAT_CONFUSED);
        m_target->SetFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));

        // only at real add aura
        if(Real)
        {
            if (m_target->GetTypeId() == TYPEID_UNIT)
                (*((Creature*)m_target))->Mutate(new ConfusedMovementGenerator(*((Creature*)m_target)));
        }
    }
    else
    {
        m_target->clearUnitState(UNIT_STAT_CONFUSED);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));

        // only at real remove aura
        if(Real)
        {
            if (m_target->GetTypeId() == TYPEID_UNIT)
                (*((Creature*)m_target))->MovementExpired();
        }
    }
}

void Aura::HandleModFear(bool Apply, bool Real)
{
    uint32 apply_stat = UNIT_STAT_FLEEING;
    if( Apply )
    {
        m_target->addUnitState(UNIT_STAT_FLEEING);
        // m_target->AttackStop();

        m_target->SetFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));

        // only at real add aura
        if(Real)
        {
            WorldPacket data(SMSG_DEATH_NOTIFY_OBSOLETE, 9);
            data<<m_target->GetGUID();
            data<<uint8(0);
            m_target->SendMessageToSet(&data,true);
        }
    }
    else
    {
        m_target->clearUnitState(UNIT_STAT_FLEEING);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));

        // only at real remove aura
        if(Real)
        {
            if(m_target->GetTypeId() != TYPEID_PLAYER)
                m_target->Attack(GetCaster());
            WorldPacket data(SMSG_DEATH_NOTIFY_OBSOLETE, 9);
            data<<m_target->GetGUID();
            data<<uint8(1);
            m_target->SendMessageToSet(&data,true);
        }
    }
}

void Aura::HandleAuraModStun(bool apply, bool Real)
{
    if (apply)
    {
        m_target->addUnitState(UNIT_STAT_STUNDED);
        m_target->SetUInt64Value (UNIT_FIELD_TARGET, 0);
        m_target->SetFlag(UNIT_FIELD_FLAGS, 0x40000);

        // only at real add aura
        if(Real)
        {
            if(m_target->GetTypeId() != TYPEID_PLAYER)
                ((Creature *)m_target)->StopMoving();

            WorldPacket data(SMSG_FORCE_MOVE_ROOT, 8);
            data.append(m_target->GetPackGUID());
            m_target->SendMessageToSet(&data,true);
        }
    }
    else
    {
        m_target->clearUnitState(UNIT_STAT_STUNDED);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, 0x40000);
        Unit* caster = GetCaster();
        if(caster)                                          // set creature facing on root effect
        {
            m_target->SetUInt64Value (UNIT_FIELD_TARGET,caster->GetGUIDLow());
        }

        // only at real remove aura
        if(Real)
        {
            WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 8);
            data.append(m_target->GetPackGUID());
            m_target->SendMessageToSet(&data,true);
        }
    }
}

void Aura::HandleModStealth(bool apply, bool Real)
{
    if(apply)
    {
        m_target->m_stealthvalue = CalculateDamage();
        m_target->SetFlag(UNIT_FIELD_BYTES_1, PLAYER_STATE_FLAG_STEALTH);

        // only at real aura add
        if(Real)
        {
            m_target->SetVisibility(VISIBILITY_GROUP);
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                m_target->SendUpdateToPlayer((Player*)m_target);
        }
    }
    else
    {
        m_target->m_stealthvalue = 0;
        m_target->RemoveFlag(UNIT_FIELD_BYTES_1, PLAYER_STATE_FLAG_STEALTH);

        // only at real aura remove
        if(Real)
        {
            SendCoolDownEvent();
            m_target->SetVisibility(VISIBILITY_ON);
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                m_target->SendUpdateToPlayer((Player*)m_target);
        }
    }
}

void Aura::HandleModDetect(bool apply, bool Real)
{
    if(apply)
    {
        m_target->m_detectStealth = CalculateDamage();
    }
    else
    {
        m_target->m_detectStealth = 0;
    }
}

void Aura::HandleInvisibility(bool Apply, bool Real)
{
    if(Apply)
    {
        m_target->m_stealthvalue = CalculateDamage();
        m_target->SetFlag(UNIT_FIELD_BYTES_1, PLAYER_STATE_FLAG_STEALTH );

        // only at real aura add
        if(Real)
        {
            m_target->SetVisibility(VISIBILITY_GROUP);
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                m_target->SendUpdateToPlayer((Player*)m_target);
        }
    }
    else
    {
        m_target->m_stealthvalue = 0;
        m_target->RemoveFlag(UNIT_FIELD_BYTES_1, PLAYER_STATE_FLAG_STEALTH );

        // only at real aura remove
        if(Real)
        {
            SendCoolDownEvent();
            m_target->SetVisibility(VISIBILITY_ON);
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                m_target->SendUpdateToPlayer((Player*)m_target);
        }
    }
}

void Aura::HandleInvisibilityDetect(bool Apply, bool Real)
{
    if(Apply)
    {
        m_target->m_detectStealth = CalculateDamage();
    }
    else
    {
        m_target->m_detectStealth = 0;
    }
}

void Aura::HandleAuraModRoot(bool apply, bool Real)
{
    uint32 apply_stat = UNIT_STAT_ROOT;
    if (apply)
    {
        m_target->addUnitState(UNIT_STAT_ROOT);
        m_target->SetUInt64Value (UNIT_FIELD_TARGET, 0);
        m_target->SetFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));

        // only at real add aura
        if(Real)
        {
            if(m_target->GetTypeId() == TYPEID_PLAYER)
            {
                WorldPacket data(SMSG_FORCE_MOVE_ROOT, 10);
                data.append(m_target->GetPackGUID());
                data << (uint32)2;
                m_target->SendMessageToSet(&data,true);
            }
            else
                ((Creature *)m_target)->StopMoving();
        }
    }
    else
    {
        m_target->clearUnitState(UNIT_STAT_ROOT);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));
        Unit* caster = GetCaster();
        if(caster)                                          // set creature facing on root effect
        {
            m_target->SetUInt64Value (UNIT_FIELD_TARGET,caster->GetGUIDLow());
        }

        // only at real remove aura
        if(Real)
        {
            if(m_target->GetTypeId() == TYPEID_PLAYER)
            {
                WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 10);
                data.append(m_target->GetPackGUID());
                data << (uint32)2;
                m_target->SendMessageToSet(&data,true);
            }
        }
    }
}

void Aura::HandleAuraModSilence(bool apply, bool Real)
{
    apply ? m_target->m_silenced = true : m_target->m_silenced = false;
}

void Aura::HandleModThreat(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    if(!m_target || !m_target->isAlive())
        return;

    Unit* caster = GetCaster();

    if(!caster || !caster->isAlive())
        return;

    m_target->AddHostil(m_caster_guid,apply ? float(m_modifier.m_amount) : -float(m_modifier.m_amount));
}

/*********************************************************/
/***                  MODIDY SPEED                     ***/
/*********************************************************/

void Aura::HandleAuraModIncreaseSpeedAlways(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    sLog.outDebug("HandleAuraModIncreaseSpeedAlways: Current Speed:%f \tmodify percent:%f", m_target->GetSpeed(MOVE_RUN),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;

    float rate = (100.0f + m_modifier.m_amount)/100.0f;

    m_target->ApplySpeedMod(MOVE_RUN, rate, false, apply );
    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_RUN));
}

void Aura::HandleAuraModIncreaseSpeed(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    sLog.outDebug("HandleAuraModIncreaseSpeed: Current Speed:%f \tmodify percent:%f", m_target->GetSpeed(MOVE_RUN),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;

    float rate = (100.0f + m_modifier.m_amount)/100.0f;

    m_target->ApplySpeedMod(MOVE_RUN, rate, true, apply );

    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_RUN));
}

void Aura::HandleAuraModIncreaseMountedSpeed(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    sLog.outDebug("HandleAuraModIncreaseMountedSpeed: Current Speed:%f \tmodify percent:%f", m_target->GetSpeed(MOVE_RUN),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;

    float rate = (100.0f + m_modifier.m_amount)/100.0f;

    m_target->ApplySpeedMod(MOVE_RUN, rate, true, apply );

    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_RUN));
}

void Aura::HandleAuraModDecreaseSpeed(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    sLog.outDebug("HandleAuraModDecreaseSpeed: Current Speed:%f \tmodify percent:%f", m_target->GetSpeed(MOVE_RUN),(float)m_modifier.m_amount);
    if(m_modifier.m_amount <= 0)
    {                                                       //for new spell dbc
        float rate = (100.0f + m_modifier.m_amount)/100.0f;

        m_target->ApplySpeedMod(MOVE_RUN, rate, true, apply );
    }
    else
    {                                                       //for old spell dbc
        float rate = m_modifier.m_amount / 100.0f;

        m_target->ApplySpeedMod(MOVE_RUN, rate, true, apply );
    }
    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_RUN));
}

void Aura::HandleAuraModIncreaseSwimSpeed(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    sLog.outDebug("HandleAuraModIncreaseSwimSpeed: Current Speed:%f \tmodify percent:%f", m_target->GetSpeed(MOVE_SWIM),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;

    float rate = (100.0f + m_modifier.m_amount)/100.0f;

    m_target->ApplySpeedMod(MOVE_SWIM, rate, true, apply );

    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_SWIM));
}

/*********************************************************/
/***                     IMMUNITY                      ***/
/*********************************************************/

void Aura::HandleModMechanicImmunity(bool apply, bool Real)
{
    m_target->ApplySpellImmune(GetId(),IMMUNITY_MECHANIC,m_modifier.m_miscvalue,apply);
}

void Aura::HandleAuraModEffectImmunity(bool apply, bool Real)
{
    m_target->ApplySpellImmune(GetId(),IMMUNITY_EFFECT,m_modifier.m_miscvalue,apply);
}

void Aura::HandleAuraModStateImmunity(bool apply, bool Real)
{
    m_target->ApplySpellImmune(GetId(),IMMUNITY_STATE,m_modifier.m_miscvalue,apply);
}

void Aura::HandleAuraModSchoolImmunity(bool apply, bool Real)
{
    m_target->ApplySpellImmune(GetId(),IMMUNITY_SCHOOL,m_modifier.m_miscvalue,apply);
}

void Aura::HandleAuraModDmgImmunity(bool apply, bool Real)
{
    m_target->ApplySpellImmune(GetId(),IMMUNITY_DAMAGE,m_modifier.m_miscvalue,apply);
}

void Aura::HandleAuraModDispelImmunity(bool apply, bool Real)
{
    m_target->ApplySpellImmune(GetId(),IMMUNITY_DISPEL,m_modifier.m_miscvalue,apply);
}

/*********************************************************/
/***                  MANA SHIELD                      ***/
/*********************************************************/

void Aura::HandleAuraDamageShield(bool apply, bool Real)
{
    /*if(apply)
    {
        for(std::list<struct DamageShield>::iterator i = m_target->m_damageShields.begin();i != m_target->m_damageShields.end();i++)
            if(i->m_spellId == GetId() && i->m_caster_guid == m_caster_guid)
            {
                m_target->m_damageShields.erase(i);
                break;
            }
        DamageShield* ds = new DamageShield();
        ds->m_caster_guid = m_caster_guid;
        ds->m_damage = m_modifier.m_amount;
        ds->m_spellId = GetId();
        m_target->m_damageShields.push_back((*ds));
    }
    else
    {
        for(std::list<struct DamageShield>::iterator i = m_target->m_damageShields.begin();i != m_target->m_damageShields.end();i++)
            if(i->m_spellId == GetId() && i->m_caster_guid == m_caster_guid)
        {
            m_target->m_damageShields.erase(i);
            break;
        }
    }*/
}

void Aura::HandleAuraManaShield(bool apply, bool Real)
{
    if (apply && !m_absorbDmg)
        m_absorbDmg = m_modifier.m_amount;

    /*if(apply)
    {

        for(std::list<struct DamageManaShield*>::iterator i = m_target->m_damageManaShield.begin();i != m_target->m_damageManaShield.end();i++)
        {
            if(GetId() == (*i)->m_spellId)
            {
                delete *i;
                m_target->m_damageManaShield.erase(i);
            }
        }

        DamageManaShield *dms = new DamageManaShield();

        dms->m_spellId = GetId();
        dms->m_modType = m_modifier.m_auraname;
        dms->m_totalAbsorb = m_modifier.m_amount;
        dms->m_currAbsorb = 0;
        dms->m_schoolType = m_modifier.m_miscvalue;
        m_target->m_damageManaShield.push_back(dms);
    }
    else
    {
        for(std::list<struct DamageManaShield*>::iterator i = m_target->m_damageManaShield.begin();i != m_target->m_damageManaShield.end();i++)
        {
            if(GetId() == (*i)->m_spellId)
            {
                delete *i;
                m_target->m_damageManaShield.erase(i);
                break;
            }
        }
    }*/
}

void Aura::HandleAuraSchoolAbsorb(bool apply, bool Real)
{
    if (apply && !m_absorbDmg)
        m_absorbDmg = m_modifier.m_amount;
    /*if(apply)
    {

        for(std::list<struct DamageManaShield*>::iterator i = m_target->m_damageManaShield.begin();i != m_target->m_damageManaShield.end();i++)
        {
            if(GetId() == (*i)->m_spellId)
            {
                m_target->m_damageManaShield.erase(i);
            }
        }

        DamageManaShield *dms = new DamageManaShield();

        dms->m_spellId = GetId();
        dms->m_modType = m_modifier.m_auraname;
        dms->m_totalAbsorb = m_modifier.m_amount;
        dms->m_currAbsorb = 0;
        dms->m_schoolType = m_modifier.m_miscvalue;
        m_target->m_damageManaShield.push_back(dms);
    }
    else
    {
        for(std::list<struct DamageManaShield*>::iterator i = m_target->m_damageManaShield.begin();i != m_target->m_damageManaShield.end();i++)
        {
            if(GetId() == (*i)->m_spellId)
            {
                m_target->m_damageManaShield.erase(i);
                break;
            }
        }
    }*/
}

/*********************************************************/
/***               REFLECT SPELLS                      ***/
/*********************************************************/

void Aura::HandleReflectSpells(bool apply, bool Real)
{
    // has no immediate effect when adding / removing
}

void Aura::HandleReflectSpellsSchool(bool apply, bool Real)
{
    // has no immediate effect when adding / removing
}

/*********************************************************/
/***                 PROC TRIGGER                      ***/
/*********************************************************/

void Aura::HandleAuraProcTriggerSpell(bool apply, bool Real)
{
    if(apply && !m_procCharges)
    {
        m_procCharges = GetSpellProto()->procCharges;
        if (!m_procCharges)
            m_procCharges = -1;
    }
}

void Aura::HandleAuraProcTriggerDamage(bool apply, bool Real)
{
    if(apply && !m_procCharges)
    {
        m_procCharges = GetSpellProto()->procCharges;
        if (!m_procCharges)
            m_procCharges = -1;
    }
}

/*********************************************************/
/***                   PERIODIC                        ***/
/*********************************************************/

void Aura::HandlePeriodicTriggerSpell(bool apply, bool Real)
{
    if (m_periodicTimer <= 0)
        m_periodicTimer += m_modifier.periodictime;

    m_isPeriodic = apply;
    m_isTrigger = apply;
}

void Aura::HandlePeriodicEnergize(bool apply, bool Real)
{
    if (m_periodicTimer <= 0)
        m_periodicTimer += m_modifier.periodictime;

    m_isPeriodic = apply;
}

void Aura::HandlePeriodicHeal(bool apply, bool Real)
{
    if (m_periodicTimer <= 0)
        m_periodicTimer += m_modifier.periodictime;

    m_isPeriodic = apply;
}

void Aura::HandlePeriodicDamage(bool apply, bool Real)
{
    if (m_periodicTimer <= 0)
        m_periodicTimer += m_modifier.periodictime;

    m_isPeriodic = apply;
}

void Aura::HandlePeriodicDamagePCT(bool apply, bool Real)
{
    if (m_periodicTimer <= 0)
        m_periodicTimer += m_modifier.periodictime;

    m_isPeriodic = apply;
}

void Aura::HandleModDetectRange(bool apply, bool Real)
{
    // has no immediate effect when adding / removing
}

void Aura::HandlePeriodicLeech(bool apply, bool Real)
{
    if (m_periodicTimer <= 0)
        m_periodicTimer += m_modifier.periodictime;

    m_isPeriodic = apply;
}

void Aura::HandlePeriodicManaLeech(bool apply, bool Real)
{
    if (m_periodicTimer <= 0)
        m_periodicTimer += m_modifier.periodictime;

    m_isPeriodic = apply;
}

/*********************************************************/
/***                  MODITY STATES                    ***/
/*********************************************************/

/********************************/
/***        RESISTANCE        ***/
/********************************/

void Aura::HandleAuraModResistanceExclusive(bool apply, bool Real)
{
    if(m_modifier.m_miscvalue < IMMUNE_SCHOOL_PHYSICAL || m_modifier.m_miscvalue > 127)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_BASE_RESISTANCE_PCT not valid");
        return;
    }

    bool positive = m_modifier.m_miscvalue2 == 0;

    for(int8 x=0;x < MAX_SPELL_SCHOOOL;x++)
    {
        if(m_modifier.m_miscvalue & int32(1<<x))
        {
            SpellSchools school = SpellSchools(SPELL_SCHOOL_NORMAL + x);

            m_target->ApplyResistanceMod(school,m_modifier.m_amount, apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                ((Player*)m_target)->ApplyResistanceBuffModsMod(school,positive,m_modifier.m_amount, apply);
        }
    }
}

void Aura::HandleAuraModResistance(bool apply, bool Real)
{
    if(m_modifier.m_miscvalue < IMMUNE_SCHOOL_PHYSICAL || m_modifier.m_miscvalue > 127)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_BASE_RESISTANCE_PCT not valid");
        return;
    }

    bool positive = m_modifier.m_miscvalue2 == 0;

    for(int8 x=0;x < MAX_SPELL_SCHOOOL;x++)
    {
        if(m_modifier.m_miscvalue & int32(1<<x))
        {
            SpellSchools school = SpellSchools(SPELL_SCHOOL_NORMAL + x);

            m_target->ApplyResistanceMod(school,m_modifier.m_amount, apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                ((Player*)m_target)->ApplyResistanceBuffModsMod(school,positive,m_modifier.m_amount, apply);
        }
    }
}

void Aura::HandleAuraModBaseResistancePCT(bool apply, bool Real)
{
    if(m_modifier.m_miscvalue < IMMUNE_SCHOOL_PHYSICAL || m_modifier.m_miscvalue > 127)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_BASE_RESISTANCE_PCT not valid");
        return;
    }

    // only players have base stats
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    Player *p_target = (Player*)m_target;

    for(int8 x=0;x < MAX_SPELL_SCHOOOL;x++)
    {
        if(m_modifier.m_miscvalue & int32(1<<x))
        {
            SpellSchools school = SpellSchools(SPELL_SCHOOL_NORMAL + x);
            float curRes = p_target->GetResistance(school);
            float baseRes = curRes + p_target->GetResistanceBuffMods(school, false) - p_target->GetResistanceBuffMods(school, true);
            float baseRes_new = baseRes * (apply?(100.0f+m_modifier.m_amount)/100.0f : 100.0f / (100.0f+m_modifier.m_amount));
            p_target->SetResistance(school, curRes + baseRes_new - baseRes);
        }
    }
}

void Aura::HandleModResistancePercent(bool apply, bool Real)
{
    for(int8 i = 0; i < MAX_SPELL_SCHOOOL; i++)
    {
        if(m_modifier.m_miscvalue & int32(1<<i))
        {
            m_target->ApplyResistancePercentMod(SpellSchools(i), m_modifier.m_amount, apply );
            if(m_target->GetTypeId() == TYPEID_PLAYER)
            {
                ((Player*)m_target)->ApplyResistanceBuffModsPercentMod(SpellSchools(i),true,m_modifier.m_amount, apply);
                ((Player*)m_target)->ApplyResistanceBuffModsPercentMod(SpellSchools(i),false,m_modifier.m_amount, apply);
            }
        }
    }
}

void Aura::HandleModBaseResistance(bool apply, bool Real)
{
    // only players have base stats
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    Player *p_target = (Player*)m_target;

    for(int i = 0; i < MAX_SPELL_SCHOOOL; i++)
        if(m_modifier.m_miscvalue & (1<<i))
            p_target->ApplyResistanceMod(SpellSchools(SPELL_SCHOOL_NORMAL + i),m_modifier.m_amount, apply);
}

/********************************/
/***           STAT           ***/
/********************************/

void Aura::HandleAuraModStat(bool apply, bool Real)
{
    if (m_modifier.m_miscvalue < -1 || m_modifier.m_miscvalue > 4)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_STAT not valid");
        return;
    }

    for(int32 i = 0; i < 5; i++)
    {
        if (m_modifier.m_miscvalue == -1 || m_modifier.m_miscvalue == i)
        {
            m_target->ApplyStatMod(Stats(i), m_modifier.m_amount,apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER)
            {
                if (m_modifier.m_miscvalue2 == 0)
                    ((Player*)m_target)->ApplyPosStatMod(Stats(i),m_modifier.m_amount,apply);
                else
                    ((Player*)m_target)->ApplyNegStatMod(Stats(i),m_modifier.m_amount,apply);
            }
        }
    }
}

void Aura::HandleModPercentStat(bool apply, bool Real)
{
    if (m_modifier.m_miscvalue < -1 || m_modifier.m_miscvalue > 4)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_PERCENT_STAT not valid");
        return;
    }

    // only players have base stats
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    Player *p_target = (Player*)m_target;

    for (int32 i = 0; i < 5; i++)
    {
        if(m_modifier.m_miscvalue == i || m_modifier.m_miscvalue == -1)
        {
            float curStat = p_target->GetStat(Stats(i));
            float baseStat = curStat + p_target->GetNegStat(Stats(i)) - p_target->GetPosStat(Stats(i));
            float baseStat_new = baseStat * (apply?(100.0f+m_modifier.m_amount)/100.0f : 100.0f / (100.0f+m_modifier.m_amount));
            p_target->SetStat(Stats(i), curStat + baseStat_new - baseStat);
            p_target->ApplyCreateStatPercentMod(Stats(i), m_modifier.m_amount, apply );
        }
    }
}

void Aura::HandleModTotalPercentStat(bool apply, bool Real)
{
    if (m_modifier.m_miscvalue < -1 || m_modifier.m_miscvalue > 4)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_PERCENT_STAT not valid");
        return;
    }

    for (int32 i = 0; i < 5; i++)
    {
        if(m_modifier.m_miscvalue == i || m_modifier.m_miscvalue == -1)
        {
            m_target->ApplyStatPercentMod(Stats(i), m_modifier.m_amount, apply );
            if (m_target->GetTypeId() == TYPEID_PLAYER)
            {
                ((Player*)m_target)->ApplyPosStatPercentMod(Stats(i), m_modifier.m_amount, apply );
                ((Player*)m_target)->ApplyNegStatPercentMod(Stats(i), m_modifier.m_amount, apply );
                ((Player*)m_target)->ApplyCreateStatPercentMod(Stats(i), m_modifier.m_amount, apply );
            }
        }
    }
}

/********************************/
/***      HEAL & ENERGIZE     ***/
/********************************/
void Aura::HandleAuraModTotalHealthPercentRegen(bool apply, bool Real)
{
    /*
    Need additional checking for auras who reduce or increase healing, magic effect like Dumpen Magic,
    so this aura not fully working.
    */
    if ((GetSpellProto()->AuraInterruptFlags & (1 << 18)) != 0)
    {
        m_target->ApplyModFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT,apply);
    }

    if(apply && m_periodicTimer <= 0)
    {
        m_periodicTimer += m_modifier.periodictime;
        float modifier = GetSpellProto()->EffectBasePoints[m_effIndex]+1;
        m_modifier.m_amount = uint32(m_target->GetMaxHealth() * modifier/100);

        if(m_target->GetHealth() < m_target->GetMaxHealth())
            m_target->PeriodicAuraLog(m_target, GetSpellProto(), &m_modifier);
    }

    m_isPeriodic = apply;
}

void Aura::HandleAuraModTotalManaPercentRegen(bool apply, bool Real)
{
    if ((GetSpellProto()->AuraInterruptFlags & (1 << 18)) != 0)
        m_target->ApplyModFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT,apply);

    if(apply && m_periodicTimer <= 0 && m_target->getPowerType() == POWER_MANA)
    {
        m_periodicTimer += m_modifier.periodictime;
        if (m_modifier.m_amount)
        {
            float modifier = GetSpellProto()->EffectBasePoints[m_effIndex]+1;
                                                            // take percent (m_modifier.m_amount) max mana
            m_modifier.m_amount = uint32((m_target->GetMaxPower(POWER_MANA) * modifier)/100);
        }

        if(m_target->GetPower(POWER_MANA) < m_target->GetMaxPower(POWER_MANA))
            m_target->PeriodicAuraLog(m_target, GetSpellProto(), &m_modifier);
    }

    m_isPeriodic = apply;
}

void Aura::HandleModRegen(bool apply, bool Real)            // eating
{
    if ((GetSpellProto()->AuraInterruptFlags & (1 << 18)) != 0)
        m_target->ApplyModFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT,apply);

    if(apply && m_periodicTimer <= 0)
    {
        m_periodicTimer += 5000;
        m_target->ModifyHealth(m_modifier.m_amount);
    }

    m_isPeriodic = apply;
}

void Aura::HandleModPowerRegen(bool apply, bool Real)       // drinking
{
    if ((GetSpellProto()->AuraInterruptFlags & (1 << 18)) != 0)
        m_target->ApplyModFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT,apply);

    if(apply && m_periodicTimer <= 0)
    {
        m_periodicTimer += 5000;
        Powers pt = m_target->getPowerType();
        // Prevent rage regeneration in combat with rage loss slowdown warrior talant and 0<->1 switching range out combat.
        if( !(pt == POWER_RAGE && (m_target->isInCombat() || m_target->GetPower(POWER_RAGE) == 0)) )
        {
            m_target->ModifyPower(pt, m_modifier.m_amount);
        }
    }

    m_isPeriodic = apply;
}

void Aura::HandleAuraModIncreaseHealth(bool apply, bool Real)
{
    m_target->ApplyMaxHealthMod(m_modifier.m_amount,apply);
}

void Aura::HandleAuraModIncreaseEnergy(bool apply, bool Real)
{
    Powers powerType = m_target->getPowerType();
    if(int32(powerType) != m_modifier.m_miscvalue)
        return;

    m_target->ApplyMaxPowerMod(powerType, m_modifier.m_amount,apply);
}

void Aura::HandleAuraModIncreaseEnergyPercent(bool apply, bool Real)
{
    Powers powerType = m_target->getPowerType();
    if(int32(powerType) != m_modifier.m_miscvalue)
        return;

    m_target->ApplyMaxPowerPercentMod(powerType,m_modifier.m_amount,apply);
    sLog.outDetail("MaxPowerPercentMod %s %d",(apply ? "+" : "-"), m_modifier.m_amount);
}

void Aura::HandleAuraModIncreaseHealthPercent(bool apply, bool Real)
{
    m_target->ApplyMaxHealthPercentMod(m_modifier.m_amount,apply);
}

/********************************/
/***          FIGHT           ***/
/********************************/

void Aura::HandleAuraModParryPercent(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    m_target->ApplyModFloatValue(PLAYER_PARRY_PERCENTAGE,m_modifier.m_amount,apply);
}

void Aura::HandleAuraModDodgePercent(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    m_target->ApplyModFloatValue(PLAYER_DODGE_PERCENTAGE,m_modifier.m_amount,apply);
}

void Aura::HandleAuraModBlockPercent(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    m_target->ApplyModFloatValue(PLAYER_BLOCK_PERCENTAGE,m_modifier.m_amount,apply);
}

void Aura::HandleAuraModCritPercent(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    m_target->ApplyModFloatValue(PLAYER_CRIT_PERCENTAGE,m_modifier.m_amount,apply);
}

void Aura::HandleModShieldBlock(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)m_target)->ApplyBlockValueMod(m_modifier.m_amount,apply);
}

void Aura::HandleModHitChance(bool Apply, bool Real)
{
    m_target->m_modHitChance = Apply?m_modifier.m_amount:0;
}

void Aura::HandleModSpellHitChance(bool Apply, bool Real)
{
    m_target->m_modSpellHitChance = Apply?m_modifier.m_amount:0;
}

void Aura::HandleModSpellCritChance(bool Apply, bool Real)
{
    m_target->m_baseSpellCritChance += Apply?m_modifier.m_amount:(-m_modifier.m_amount);
}

void Aura::HandleModSpellCritChanceShool(bool apply, bool Real)
{
    // has no immediate effect when adding / removing
}

/********************************/
/***         ATTACK SPEED     ***/
/********************************/

void Aura::HandleModCastingSpeed(bool apply, bool Real)
{
    m_target->m_modCastSpeedPct += apply ? m_modifier.m_amount : (-m_modifier.m_amount);
}

void Aura::HandleModAttackSpeed(bool apply, bool Real)
{
    if(!m_target || !m_target->isAlive() )
        return;

    m_target->ApplyAttackTimePercentMod(BASE_ATTACK,m_modifier.m_amount,apply);
}

void Aura::HandleHaste(bool apply, bool Real)
{

    if(m_modifier.m_amount >= 0)
    {
        // v*(1+percent/100)
        m_target->ApplyAttackTimePercentMod(BASE_ATTACK,  m_modifier.m_amount,apply);
        m_target->ApplyAttackTimePercentMod(OFF_ATTACK,   m_modifier.m_amount,apply);

        if(m_target->GetTypeId()==TYPEID_PLAYER)
            ((Player*)m_target)->_ApplyAmmoBonuses(false);

        m_target->ApplyAttackTimePercentMod(RANGED_ATTACK,m_modifier.m_amount,apply);

        if(m_target->GetTypeId()==TYPEID_PLAYER)
            ((Player*)m_target)->_ApplyAmmoBonuses(true);
    }
    else
    {
        // v/(1+abs(percent)/100)
        m_target->ApplyAttackTimePercentMod(BASE_ATTACK,  -m_modifier.m_amount,!apply);
        m_target->ApplyAttackTimePercentMod(OFF_ATTACK,   -m_modifier.m_amount,!apply);

        if(m_target->GetTypeId()==TYPEID_PLAYER)
            ((Player*)m_target)->_ApplyAmmoBonuses(false);

        m_target->ApplyAttackTimePercentMod(RANGED_ATTACK,-m_modifier.m_amount,!apply);

        if(m_target->GetTypeId()==TYPEID_PLAYER)
            ((Player*)m_target)->_ApplyAmmoBonuses(true);
    }
}

void Aura::HandleAuraModRangedHaste(bool apply, bool Real)
{
    if(m_target->GetTypeId()==TYPEID_PLAYER)
        ((Player*)m_target)->_ApplyAmmoBonuses(false);

    if(m_modifier.m_amount >= 0)
        m_target->ApplyAttackTimePercentMod(RANGED_ATTACK, m_modifier.m_amount, apply);
    else
        m_target->ApplyAttackTimePercentMod(RANGED_ATTACK, -m_modifier.m_amount, !apply);

    if(m_target->GetTypeId()==TYPEID_PLAYER)
        ((Player*)m_target)->_ApplyAmmoBonuses(true);
}

void Aura::HandleRangedAmmoHaste(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    ((Player*)m_target)->_ApplyAmmoBonuses(false);
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK,m_modifier.m_amount, apply);
    ((Player*)m_target)->_ApplyAmmoBonuses(true);
}

/********************************/
/***        ATTACK POWER      ***/
/********************************/

void Aura::HandleAuraModAttackPower(bool apply, bool Real)
{
    m_target->ApplyModUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS, m_modifier.m_amount, apply);
}

void Aura::HandleAuraModRangedAttackPower(bool apply, bool Real)
{
    m_target->ApplyModUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS,m_modifier.m_amount,apply);
}

/********************************/
/***        DAMAGE BONUS      ***/
/********************************/

void Aura::HandleModDamagePCTTaken(bool apply, bool Real)
{
    m_target->m_modDamagePCT = apply ? m_modifier.m_amount : 0;
}

void Aura::HandleModPCTRegen(bool apply, bool Real)
{
    // has no immediate effect when adding / removing
}

void Aura::HandleModCreatureAttackPower(bool apply, bool Real)
{
    // has no immediate effect when adding / removing
}

void Aura::HandleModDamageDoneCreature(bool apply, bool Real)
{
    // has no immediate effect when adding / removing
}

void Aura::HandleModDamageDone(bool apply, bool Real)
{
    // physical damage modifier is applied to damage fields
    if(m_modifier.m_miscvalue & (int32)(1<<SPELL_SCHOOL_NORMAL))
    {
        m_target->ApplyModFloatValue(UNIT_FIELD_MINDAMAGE,m_modifier.m_amount,apply);
        m_target->ApplyModFloatValue(UNIT_FIELD_MAXDAMAGE,m_modifier.m_amount,apply);
        // TODO: add ranged support and maybe offhand ?
        // not completely sure how this should work
        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            if(m_modifier.m_miscvalue2)
                m_target->ApplyModFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG,m_modifier.m_amount,apply);
            else
                m_target->ApplyModFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS,m_modifier.m_amount,apply);
        }
    }
}

void Aura::HandleModDamageTaken(bool apply, bool Real)
{
    // has no immediate effect when adding / removing
}

void Aura::HandleModDamagePercentDone(bool apply, bool Real)
{
    sLog.outDebug("AURA MOD DAMAGE type:%u type2:%u", m_modifier.m_miscvalue, m_modifier.m_miscvalue2);

    // FIX ME: This is wrong code.
    // It not work with 20218 18791 spells

    // m_modifier.m_miscvalue is bitmask of spell schools
    // 1 ( 0-bit ) - normal school damage
    // 126 - full bitmask all magic damages
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equiped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // m_modifier.m_miscvalue comparison with item generated damage types
    if (!m_target)
        return;

    if((m_modifier.m_miscvalue & 1) != 0)
    {
        if (GetSpellProto()->EquippedItemClass == -1 || m_target->GetTypeId() != TYPEID_PLAYER)
        {
            m_target->ApplyPercentModFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, m_modifier.m_amount, apply );
            m_target->ApplyPercentModFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, m_modifier.m_amount, apply );
            m_target->ApplyPercentModFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, m_modifier.m_amount, apply );
            m_target->ApplyPercentModFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, m_modifier.m_amount, apply );
            m_target->ApplyPercentModFloatValue(UNIT_FIELD_MINDAMAGE, m_modifier.m_amount, apply );
            m_target->ApplyPercentModFloatValue(UNIT_FIELD_MAXDAMAGE, m_modifier.m_amount, apply );
        }
        else
        {
            Item* pItem = ((Player*)m_target)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            if (pItem)
            {
                if (pItem->IsFitToSpellRequirements(GetSpellProto()))
                {
                    m_target->ApplyPercentModFloatValue(UNIT_FIELD_MINDAMAGE, m_modifier.m_amount, apply );
                    m_target->ApplyPercentModFloatValue(UNIT_FIELD_MAXDAMAGE, m_modifier.m_amount, apply );
                }
            }
            pItem = ((Player*)m_target)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            if (pItem)
            {
                if (pItem->IsFitToSpellRequirements(GetSpellProto()))
                {
                    m_target->ApplyPercentModFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, m_modifier.m_amount, apply );
                    m_target->ApplyPercentModFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, m_modifier.m_amount, apply );
                }
            }
            pItem = ((Player*)m_target)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
            if (pItem)
            {
                if (pItem->IsFitToSpellRequirements(GetSpellProto()))
                {
                    m_target->ApplyPercentModFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, m_modifier.m_amount, apply );
                    m_target->ApplyPercentModFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, m_modifier.m_amount, apply );
                }
            }
        }
    }

    // Magic damage percent modifiers implemented in Unit::SpellDamageBonus
}

/********************************/
/***        POWER COST        ***/
/********************************/

void Aura::HandleModPowerCost(bool apply, bool Real)
{
    m_target->ApplyModUInt32Value(UNIT_FIELD_POWER_COST_MODIFIER, m_modifier.m_amount, apply);
}

void Aura::HandleModPowerCostSchool(bool apply, bool Real)
{
    // has no immediate effect when adding / removing
}

/*********************************************************/
/***                    OTHERS                         ***/
/*********************************************************/

void Aura::HandleModReputationGain(bool apply, bool Real)
{
    // has no immediate effect when adding / removing
}

void Aura::SendCoolDownEvent()
{
    Unit* caster = GetCaster();
    if(caster)
    {
        WorldPacket data(SMSG_COOLDOWN_EVENT, (4+8+4));
        data << uint32(m_spellId) << m_caster_guid;
        data << uint32(0);                                  //CoolDown Time ?
        caster->SendMessageToSet(&data,true);
    }
}

// FIX-ME!!
void HandleTriggerSpellEvent(void *obj)
{
    Aura *Aur = ((Aura*)obj);
    if(!Aur)
        return;
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(Aur->GetSpellProto()->EffectTriggerSpell[Aur->GetEffIndex()]);

    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i\n", Aur->GetSpellProto()->EffectTriggerSpell[Aur->GetEffIndex()]);
        return;
    }

    Spell spell(Aur->GetCaster(), spellInfo, true, Aur);
    SpellCastTargets targets;
    targets.setUnitTarget(Aur->GetTarget());
    //WorldPacket dump;
    //dump.Initialize(0);
    //dump << uint16(2) << GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT) << GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT+1);
    //targets.read(&dump,this);
    spell.prepare(&targets);

    /*else if(m_spellProto->EffectApplyAuraName[i] == 23)
    {
        unitTarget->tmpAura->SetPeriodicTriggerSpell(m_spellProto->EffectTriggerSpell[i],m_spellProto->EffectAmplitude[i]);
    }*/
}

void HandleDOTEvent(void *obj)
{
    Aura *Aur = ((Aura*)obj);
    //Aur->GetCaster()->AddPeriodicAura(Aur);
    if(Unit* caster = Aur->GetCaster())
        caster->PeriodicAuraLog(Aur->GetTarget(), Aur->GetSpellProto(), Aur->GetModifier());
}

void HandleHealEvent(void *obj)
{
    Aura *Aur = ((Aura*)obj);
    if(Unit* caster = Aur->GetCaster())
        Aur->GetTarget()->PeriodicAuraLog(caster, Aur->GetSpellProto(), Aur->GetModifier());
}

void Aura::HandleShapeshiftBoosts(bool apply)
{
    if(!m_target)
        return;

    uint32 spellId = 0;
    uint32 spellId2 = 0;

    switch(GetModifier()->m_miscvalue)
    {
        case FORM_CAT:
            spellId = 3025;
            break;
        case FORM_TREE:
            spellId = 3122;
            break;
        case FORM_TRAVEL:
            spellId = 5419;
            break;
        case FORM_AQUA:
            spellId = 5421;
            break;
        case FORM_BEAR:
            spellId = 1178;
            break;
        case FORM_DIREBEAR:
            spellId = 9635;
            break;
        case FORM_CREATUREBEAR:
            spellId = 2882;
            break;
        case FORM_BATTLESTANCE:
            spellId = 21156;
            break;
        case FORM_DEFENSIVESTANCE:
            spellId = 7376;
            break;
        case FORM_BERSERKERSTANCE:
            spellId = 7381;
            break;
        case FORM_MOONKIN:
            spellId = 24905;
            // aura from effect trigger spell
            spellId2 = 24907;
            break;
        case FORM_GHOSTWOLF:
        case FORM_AMBIENT:
        case FORM_GHOUL:
        case FORM_SHADOW:
        case FORM_STEALTH:
            spellId = 0;
            break;
    }

    uint32 form = GetModifier()->m_miscvalue-1;

    if(apply)
    {
        if(m_target->m_ShapeShiftForm)
        {
            m_target->RemoveAurasDueToSpell(m_target->m_ShapeShiftForm);
        }

        if (spellId) m_target->CastSpell(m_target, spellId, true);
        if (spellId2) m_target->CastSpell(m_target, spellId2, true);

        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            const PlayerSpellMap& sp_list = ((Player *)m_target)->GetSpellMap();
            for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
            {
                if(itr->second->state == PLAYERSPELL_REMOVED) continue;
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(itr->first);
                if (!spellInfo || !IsPassiveSpell(itr->first)) continue;
                if (spellInfo->Stances & (1<<form))
                    m_target->CastSpell(m_target, itr->first, true);
            }
        }
    }
    else
    {
        m_target->RemoveAurasDueToSpell(spellId);
        m_target->RemoveAurasDueToSpell(spellId2);

        Unit::AuraMap& tAuras = m_target->GetAuras();
        for (Unit::AuraMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
        {
            if ((*itr).second->GetSpellProto()->Stances & uint32(1<<form))
                m_target->RemoveAura(itr);
            else
                ++itr;
        }
    }

    double healthPercentage = (double)m_target->GetHealth() / (double)m_target->GetMaxHealth();
    m_target->SetHealth(uint32(ceil((double)m_target->GetMaxHealth() * healthPercentage)));
}

void Aura::HandleAuraEmpathy(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_UNIT)
        return;

    CreatureInfo const * ci = objmgr.GetCreatureTemplate(m_target->GetEntry());
    if(ci && ci->type == CREATURE_TYPE_BEAST)
    {
        m_target->ApplyModUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO, apply);
    }
}
