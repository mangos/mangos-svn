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

pAuraHandler AuraHandler[TOTAL_AURAS]=
{
    &Aura::HandleNULL,                                      //SPELL_AURA_NONE
    &Aura::HandleNULL,                                      //SPELL_AURA_BIND_SIGHT
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_POSSESS = 2,
    &Aura::HandlePeriodicDamage,                            //SPELL_AURA_PERIODIC_DAMAGE = 3,
    &Aura::HandleNULL,                                      //SPELL_AURA_DUMMY    //missing 4
    &Aura::HandleModConfuse,                                //SPELL_AURA_MOD_CONFUSE = 5,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_CHARM = 6,
    &Aura::HandleModFear,                                   //SPELL_AURA_MOD_FEAR = 7,
    &Aura::HandlePeriodicHeal,                              //SPELL_AURA_PERIODIC_HEAL = 8,
    &Aura::HandleModAttackSpeed,                            //SPELL_AURA_MOD_ATTACKSPEED = 9,
    &Aura::HandleModThreat,                                 //SPELL_AURA_MOD_THREAT = 10,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_TAUNT = 11,
    &Aura::HandleAuraModStun,                               //SPELL_AURA_MOD_STUN = 12,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_DAMAGE_DONE = 13,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_DAMAGE_TAKEN = 14,
    &Aura::HandleAuraDamageShield,                          //SPELL_AURA_DAMAGE_SHIELD = 15,
    &Aura::HandleModStealth,                                //SPELL_AURA_MOD_STEALTH = 16,
    &Aura::HandleModDetect,                                 //SPELL_AURA_MOD_DETECT = 17,
    &Aura::HandleInvisibility,                              //SPELL_AURA_MOD_INVISIBILITY = 18,
    &Aura::HandleInvisibilityDetect,                        //SPELL_AURA_MOD_INVISIBILITY_DETECTION = 19,
    &Aura::HandleNULL,                                      //missing 20,
    &Aura::HandleNULL,                                      //missing 21
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
    &Aura::HandleAuraTracCreatures,                         //SPELL_AURA_TRACK_CREATURES = 44,
    &Aura::HandleAuraTracResources,                         //SPELL_AURA_TRACK_RESOURCES = 45,
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
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_CASTING_SPEED = 65,
    &Aura::HandleNULL,                                      //SPELL_AURA_FEIGN_DEATH = 66,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_DISARM = 67,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_STALKED = 68,
    &Aura::HandleAuraSchoolAbsorb,                          //SPELL_AURA_SCHOOL_ABSORB = 69,
    &Aura::HandleNULL,                                      //SPELL_AURA_EXTRA_ATTACKS = 70,
    &Aura::HandleModSpellCritChanceShool,                   //SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL = 71,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_POWER_COST = 72,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_POWER_COST_SCHOOL = 73,
    &Aura::HandleReflectSpellsSchool,                       //SPELL_AURA_REFLECT_SPELLS_SCHOOL = 74,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_LANGUAGE = 75,
    &Aura::HandleNULL,                                      //SPELL_AURA_FAR_SIGHT = 76,
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
    &Aura::HandleModPCTRegen,                               //SPELL_AURA_MOD_PERCENT_REGEN = 88,
    &Aura::HandlePeriodicDamagePCT,                         //SPELL_AURA_PERIODIC_DAMAGE_PERCENT = 89,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_RESIST_CHANCE = 90,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_DETECT_RANGE = 91,
    &Aura::HandleNULL,                                      //SPELL_AURA_PREVENTS_FLEEING = 92,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_UNATTACKABLE = 93,
    &Aura::HandleNULL,                                      //SPELL_AURA_INTERRUPT_REGEN = 94,
    &Aura::HandleNULL,                                      //SPELL_AURA_GHOST = 95,
    &Aura::HandleNULL,                                      //SPELL_AURA_SPELL_MAGNET = 96,
    &Aura::HandleAuraManaShield,                            //SPELL_AURA_MANA_SHIELD = 97,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_SKILL_TALENT = 98,
    &Aura::HandleAuraModAttackPower,                        //SPELL_AURA_MOD_ATTACK_POWER = 99,
    &Aura::HandleNULL,                                      //SPELL_AURA_AURAS_VISIBLE = 100,
    &Aura::HandleModResistancePercent,                      //SPELL_AURA_MOD_RESISTANCE_PCT = 101,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_CREATURE_ATTACK_POWER = 102,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_TOTAL_THREAT = 103,
    &Aura::HandleAuraWaterWalk,                             //SPELL_AURA_WATER_WALK = 104,
    &Aura::HandleAuraFeatherFall,                           //SPELL_AURA_FEATHER_FALL = 105,
    &Aura::HandleNULL,                                      //SPELL_AURA_HOVER = 106,
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
    &Aura::HandleNULL,                                      //SPELL_AURA_EMPATHY = 121,
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
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE = 137,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_HASTE = 138,
    &Aura::HandleForceReaction,                             //SPELL_AURA_FORCE_REACTION = 139,
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_RANGED_HASTE = 140,
    &Aura::HandleRangedAmmoHaste,                           //SPELL_AURA_MOD_RANGED_AMMO_HASTE = 141,
    &Aura::HandleAuraModBaseResistancePCT,                  //SPELL_AURA_MOD_BASE_RESISTANCE_PCT = 142,
    &Aura::HandleAuraModResistanceExclusive,                //SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE = 143,
    &Aura::HandleAuraSafeFall,                              //SPELL_AURA_SAFE_FALL = 144,
    &Aura::HandleNULL,                                      //SPELL_AURA_CHARISMA = 145,
    &Aura::HandleNULL,                                      //SPELL_AURA_PERSUADED = 146,
    &Aura::HandleNULL,                                      //SPELL_AURA_ADD_CREATURE_IMMUNITY = 147,
    &Aura::HandleNULL,                                      //SPELL_AURA_RETAIN_COMBO_POINTS = 148,
    &Aura::HandleNULL,                                      //SPELL_AURA_RESIST_PUSHBACK    =    149    ,//    Resist Pushback
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_SHIELD_BLOCK    =    150    ,//    Mod Shield Block %
    &Aura::HandleNULL,                                      //SPELL_AURA_TRACK_STEALTHED    =    151    ,//    Track Stealthed
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_DETECTED_RANGE    =    152    ,//    Mod Detected Range
    &Aura::HandleNULL,                                      //SPELL_AURA_SPLIT_DAMAGE_FLAT    =    153    ,//    Split Damage Flat
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_STEALTH_LEVEL    =    154    ,//    Stealth Level Modifier
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_WATER_BREATHING    =    155    ,//    Mod Water Breathing
    &Aura::HandleNULL,                                      //SPELL_AURA_MOD_REPUTATION_ADJUST    =    156    ,//    Mod Reputation Gain
    &Aura::HandleNULL                                       //SPELL_AURA_PET_DAMAGE_MULTI    =    157    ,//    Mod Pet Damage
};

Aura::Aura(SpellEntry* spellproto, uint32 eff, Unit *caster, Unit *target) :
    m_procSpell(NULL),m_procdamage(NULL), m_spellId(spellproto->Id), m_effIndex(eff), 
    m_caster(caster), m_target(target), m_auraSlot(0),m_positive(false), m_permanent(false),  
    m_isPeriodic(false), m_isTrigger(false), m_periodicTimer(0), m_PeriodicEventId(0)
{
    assert(target);
    sLog.outDebug("Aura construct spellid is: %u, auraname is: %u.", spellproto->Id, spellproto->EffectApplyAuraName[eff]);
    m_duration = GetDuration(spellproto);
    if(m_duration == -1)
        m_permanent = true;

    switch(spellproto->EffectImplicitTargetA[eff])
    {
        case TARGET_S_E:
        case TARGET_AE_E:
        case TARGET_AE_E_INSTANT:
        case TARGET_AC_E:
        case TARGET_INFRONT:
        case TARGET_DUELVSPLAYER:
        case TARGET_AE_E_CHANNEL:
        case TARGET_AE_SELECTED:
            m_positive = false;
            break;

        default:
            m_positive = (spellproto->AttributesEx & (1<<7)) ? false : true;
    }
    
    uint32 type = 0;
    if(!m_positive)
        type = 1;
    uint32 damage;
    if(!caster)
    {
        m_caster = target;
        damage = spellproto->EffectBasePoints[eff];
    }
    else
        damage = CalculateDamage();

    m_effIndex = eff;
    SetModifier(spellproto->EffectApplyAuraName[eff], damage, spellproto->EffectAmplitude[eff], spellproto->EffectMiscValue[eff], type);
}

uint32 Aura::CalculateDamage()
{
    SpellEntry* spellproto = GetSpellProto();
    uint32 value = 0;
    uint32 level;
    if(!m_target)
        return 0;
    Unit* caster = m_caster;
    if(!m_caster)
        caster = m_target;
    level= caster->getLevel();

    float basePointsPerLevel = spellproto->EffectRealPointsPerLevel[m_effIndex];
    float randomPointsPerLevel = spellproto->EffectDicePerLevel[m_effIndex];
    uint32 basePoints = uint32(spellproto->EffectBasePoints[m_effIndex] + level * basePointsPerLevel);
    uint32 randomPoints = uint32(spellproto->EffectDieSides[m_effIndex] + level * randomPointsPerLevel);
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
        value += (uint32)(comboDamage * comboPoints);
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
}

void Aura::Update(uint32 diff)
{
    if (m_duration > 0)
    {
        m_duration -= diff;
        if (m_duration < 0)
            m_duration = 0;
        if(m_target->isAlive() && m_target->hasUnitState(UNIT_STAT_FLEEING))
        {
            float x,y,z,angle,speed,pos_x,pos_y,pos_z;
            uint32 time;
            m_target->AttackStop();
            m_target->RemoveAllAttackers();
            angle = m_target->GetAngle( m_caster->GetPositionX(), m_caster->GetPositionY() );
            speed = m_target->GetSpeed();
            pos_x = m_target->GetPositionX()+speed*diff* cos(-angle)/1000;
            pos_y = m_target->GetPositionY()+speed*diff* sin(-angle)/1000;
            uint32 mapid = m_target->GetMapId();
            pos_z = MapManager::Instance().GetMap(mapid)->GetHeight(pos_x,pos_y);
            m_target->Relocate(pos_x,pos_y,pos_z,-angle);

            x = m_target->GetPositionX() + speed*m_duration * cos(-angle)/1000;
            y = m_target->GetPositionY() + speed*m_duration * sin(-angle)/1000;
            mapid = m_target->GetMapId();
            z = MapManager::Instance().GetMap(mapid)->GetHeight(x,y);
            time = uint32(::sqrt(x*x+y*y+z*z)/speed);
            m_target->SendMonsterMove(x,y,z,false,true,time);
        }
    }
    if(m_isPeriodic && m_duration > 0)
    {
        if(m_periodicTimer > 0)
        {
            if(m_periodicTimer <= diff)
                m_periodicTimer = 0;
            else
                m_periodicTimer -= diff;
        }
        if(m_periodicTimer == 0)
        {
            // update before applying (aura can be removed in TriggerSpell or PeriodicAuraLog calls)
            m_periodicTimer = m_modifier.periodictime;

            if(m_isTrigger)
            {
                TriggerSpell();
            }
            else
            {
                if(!m_caster)
                    m_target->PeriodicAuraLog(m_target, GetSpellProto(), &m_modifier);
                else
                    m_caster->PeriodicAuraLog(m_target, GetSpellProto(), &m_modifier);
            }
        }
    }
}

void Aura::ApplyModifier(bool apply)
{
    uint8 aura = 0;
    aura = m_modifier.m_auraname;
    if(aura<TOTAL_AURAS)
        (*this.*AuraHandler [aura])(apply);
}

void Aura::UpdateAuraDuration()
{

    if(m_target->GetTypeId() != TYPEID_PLAYER) 
        return;

    WorldPacket data;
    data.Initialize(SMSG_UPDATE_AURA_DURATION);
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
            if (i != m_effIndex)
            {
                samespell = true;
                slot = aura->GetAuraSlot();
            }
        }
    }

    //m_target->RemoveRankAurasDueToSpell(m_spellId);
    m_target->ApplyStats(false);
    ApplyModifier(true);
    m_target->ApplyStats(true);
    sLog.outDebug("Aura %u now is in use", m_modifier.m_auraname);

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

    SetAuraSlot( slot );
    if( m_target->GetTypeId() == TYPEID_PLAYER )
        UpdateAuraDuration();
}

void Aura::_RemoveAura()
{
    m_target->ApplyStats(false);
    sLog.outDebug("Aura %u now is remove", m_modifier.m_auraname);
    ApplyModifier(false);
    m_target->ApplyStats(true);

    uint8 slot = GetAuraSlot();
    Aura* aura = m_target->GetAura(m_spellId, m_effIndex);
    if(!aura)
        return;

    if(m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURA + slot)) == 0)
        return;

    // only remove icon when the last aura of the spell is removed
    for(uint32 i = 0; i < 3; i++)
    {
        aura = m_target->GetAura(m_spellId, i);
        if(aura && i != m_effIndex)
            return;
    }

    m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURA + slot), 0);

    uint8 flagslot = slot >> 3;

    uint32 value = m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot));

    uint8 aurapos = (slot & 7) << 2;
    uint32 value1 = ~( AFLAG_SET << aurapos );
    value &= value1;

    m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot), value);
}

void Aura::HandleNULL(bool apply)
{
}

void HandleDOTEvent(void *obj)
{
    Aura *Aur = ((Aura*)obj);
    //Aur->GetCaster()->AddPeriodicAura(Aur);
    Aur->GetCaster()->PeriodicAuraLog(Aur->GetTarget(), Aur->GetSpellProto(), Aur->GetModifier());
}

void Aura::HandlePeriodicDamage(bool apply)
{
    if( apply )
    {
        //m_PeriodicEventId = AddEvent(&HandleDOTEvent,(void*)this,m_modifier.periodictime,false,true);
        m_isPeriodic = true;
        m_periodicTimer = m_modifier.periodictime;
    }
    else
    {
        //RemovePeriodicEvent(m_PeriodicEventId);
        m_isPeriodic = false;
        m_duration = 0;
    }
}

void Aura::HandleModConfuse(bool apply)
{
    uint32 apply_stat = UNIT_STAT_CONFUSED;
    if( apply )
    {
        m_target->addUnitState(UNIT_STAT_CONFUSED);
        m_target->SetFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));
    }
    else
    {
        m_target->clearUnitState(UNIT_STAT_CONFUSED);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));
    }
}

void Aura::HandleModFear(bool Apply)
{
    uint32 apply_stat = UNIT_STAT_FLEEING;
    WorldPacket data;
    data.Initialize(SMSG_DEATH_NOTIFY_OBSOLETE);
    if( Apply )
    {
        m_target->addUnitState(UNIT_STAT_FLEEING);
        m_target->SendAttackStop(m_caster->GetGUID());
        m_caster->SendAttackStop(m_target->GetGUID());
        m_target->AttackStop();
        m_target->SetFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));

        data<<m_target->GetGUIDLow();
        data<<uint8(0);
    }
    else
    {
        data<<m_target->GetGUIDLow();
        data<<uint8(1);
        m_target->clearUnitState(UNIT_STAT_FLEEING);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));
    }
    m_target->SendMessageToSet(&data,true);
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        m_target->SendUpdateToPlayer((Player*)m_target);
}

void HandleHealEvent(void *obj)
{
    Aura *Aur = ((Aura*)obj);
    Aur->GetTarget()->PeriodicAuraLog(Aur->GetCaster(), Aur->GetSpellProto(), Aur->GetModifier());
}

void Aura::HandlePeriodicHeal(bool apply)
{
                                                            //Can't heal
    if(!m_target || (m_target->m_immuneToMechanic & IMMUNE_MECHANIC_HEAL))
        return;
    if(apply)
    {
        //m_PeriodicEventId = AddEvent(&HandleHealEvent,(void*)this,m_modifier.periodictime,false,true);
        m_isPeriodic = true;
        m_periodicTimer = m_modifier.periodictime;
    }
    else
    {
        //RemovePeriodicEvent(m_PeriodicEventId);
        m_isPeriodic = false;
        m_duration = 0;
    }
}

void Aura::HandleModAttackSpeed(bool apply)
{
    if(!m_target || !m_target->isAlive() || !m_caster->isAlive())
        return;
    
    m_target->ApplyPercentModUInt32Value(UNIT_FIELD_BASEATTACKTIME,m_modifier.m_amount,apply);
}

void Aura::HandleModThreat(bool apply)
{
    if(!m_target || !m_target->isAlive() || !m_caster->isAlive())
        return;
    m_target->AddHostil(m_caster->GetGUID(),apply ? float(m_modifier.m_amount) : -float(m_modifier.m_amount));
}

void Aura::HandleAuraWaterWalk(bool apply)
{
    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_WATER_WALK);
    else
        data.Initialize(SMSG_MOVE_LAND_WALK);
    data << uint8(0xFF) << m_target->GetGUID();
    m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraFeatherFall(bool apply)
{
    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_FEATHER_FALL);
    else
        data.Initialize(SMSG_MOVE_NORMAL_FALL);
    data << uint8(0xFF) << m_target->GetGUID();
    m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAddModifier(bool apply)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data;

    SpellEntry *spellInfo = GetSpellProto();
    if(!spellInfo) return;
    uint8 op;
    uint16 val=0;
    int16 tmpval=0;
    uint16 mark=0;
    uint32 shiftdata=0x01;
    uint8  FlatId=0;
    uint32 EffectVal;
    uint32 Opcode=SMSG_SET_FLAT_SPELL_MODIFIER;

    if(spellInfo->EffectItemType[m_effIndex])
    {
        EffectVal=spellInfo->EffectItemType[m_effIndex];
        op=spellInfo->EffectMiscValue[m_effIndex];
        tmpval = spellInfo->EffectBasePoints[m_effIndex];

        if(tmpval != 0)
        {
            if(tmpval > 0)
            {
                val =  tmpval+1;
                mark = 0x0;
            }
            else
            {
                val  = 0xFFFF + (tmpval+2);
                mark = 0xFFFF;
            }
        }

        switch(spellInfo->EffectApplyAuraName[m_effIndex])
        {
            case 107:
                Opcode=SMSG_SET_FLAT_SPELL_MODIFIER;
                break;
            case 108:
                Opcode=SMSG_SET_PCT_SPELL_MODIFIER;
                break;
        }

        for(int m_effIndex=0;m_effIndex<32;m_effIndex++)
        {
            if ( EffectVal&shiftdata )
            {
                FlatId=m_effIndex;
                data.Initialize(Opcode);
                data << uint8(FlatId);
                data << uint8(op);
                data << uint16(val);
                data << uint16(mark);
                //m_target->SendMessageToSet(&data,true);
                ((Player *)m_target)->SendDirectMessage(&data);
            }
            shiftdata=shiftdata<<1;
        }
    }
}

void Aura::HandleAuraModStun(bool apply)
{
    uint32 apply_stat = UNIT_STAT_STUNDED;
    if (apply)
    {
        m_target->addUnitState(UNIT_STAT_STUNDED);
        m_target->SetUInt64Value (UNIT_FIELD_TARGET, 0);
        m_target->SetFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));
        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            WorldPacket data;
            data.Initialize(SMSG_FORCE_MOVE_ROOT);
            data << uint8(0xFF) << m_target->GetGUID();
            m_target->SendMessageToSet(&data,true);
            m_target->SetFlag(UNIT_FIELD_FLAGS, 0x40000);
        }
        else
            ((Creature *)m_target)->StopMoving();
    }
    else
    {
        m_target->clearUnitState(UNIT_STAT_STUNDED);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));
        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            WorldPacket data;
            data.Initialize(SMSG_FORCE_MOVE_UNROOT);
            data << uint8(0xFF) << m_target->GetGUID();
            m_target->SendMessageToSet(&data,true);
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, 0x40000);
        }
    }
}

void Aura::HandleAuraModRangedAttackPower(bool apply)
{
    m_target->ApplyModUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS,m_modifier.m_amount,apply);
}

void Aura::HandleAuraModIncreaseEnergyPercent(bool apply)
{
    m_target->ApplyPercentModUInt32Value(UNIT_FIELD_POWER4,m_modifier.m_amount,apply);
}

void Aura::HandleAuraModIncreaseHealthPercent(bool apply)
{
    m_target->ApplyPercentModUInt32Value(UNIT_FIELD_MAXHEALTH,m_modifier.m_amount,apply);
}

// FIX-ME!!
void HandleTriggerSpellEvent(void *obj)
{
    Aura *Aur = ((Aura*)obj);
    if(!Aur)
        return;
    SpellEntry *spellInfo = sSpellStore.LookupEntry(Aur->GetSpellProto()->EffectTriggerSpell[Aur->GetEffIndex()]);

    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i\n", Aur->GetSpellProto()->EffectTriggerSpell[Aur->GetEffIndex()]);
        return;
    }

    Spell *spell = new Spell(Aur->GetCaster(), spellInfo, true, Aur);
    SpellCastTargets targets;
    targets.setUnitTarget(Aur->GetTarget());
    //WorldPacket dump;
    //dump.Initialize(0);
    //dump << uint16(2) << GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT) << GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT+1);
    //targets.read(&dump,this);
    spell->prepare(&targets);

    /*else if(m_spellProto->EffectApplyAuraName[i] == 23)
    {
        unitTarget->tmpAura->SetPeriodicTriggerSpell(m_spellProto->EffectTriggerSpell[i],m_spellProto->EffectAmplitude[i]);
    }*/
}

void Aura::TriggerSpell()
{
    SpellEntry *spellInfo = sSpellStore.LookupEntry( GetSpellProto()->EffectTriggerSpell[m_effIndex] );

    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i\n",  GetSpellProto()->EffectTriggerSpell[m_effIndex]);
        return;
    }

    Spell *spell = new Spell(m_caster, spellInfo, true, this);
    Unit* target = m_target;
    if(!target && m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        target = ObjectAccessor::Instance().GetUnit(*m_caster, ((Player*)m_caster)->GetSelection());
    }
    if(!target)
        return;
    SpellCastTargets targets;
    targets.setUnitTarget(target);
    spell->prepare(&targets);
}

void Aura::HandlePeriodicTriggerSpell(bool apply)
{
    if(apply)
    {
        //m_PeriodicEventId = AddEvent(&HandleTriggerSpellEvent,(void*)this,m_modifier.periodictime,false,true);
        m_isPeriodic = true;
        m_isTrigger = true;
        m_periodicTimer = m_modifier.periodictime;
    }
    else
    {
        //RemovePeriodicEvent(m_PeriodicEventId);
        m_isPeriodic = false;
        m_isTrigger = false;
        m_duration = 0;
        //probably it's temporary for taming creature..
        if(GetSpellProto()->Id == 1515 && m_caster->isAlive())
        {
            SpellEntry *spell_proto = sSpellStore.LookupEntry(13481);
            Spell *spell = new Spell(m_caster, spell_proto, false, 0);
            Unit* target = NULL;
            if(m_caster->GetTypeId() == TYPEID_PLAYER)
            {
                target = ObjectAccessor::Instance().GetUnit(*m_caster, ((Player*)m_caster)->GetSelection());
            }
            else target = m_target;
            if(!target || !target->isAlive())
                return;
            SpellCastTargets targets;
            targets.setUnitTarget(target);
            spell->prepare(&targets);
        }
    }
}

void Aura::HandlePeriodicEnergize(bool apply)
{
    if(apply)
    {
        //m_PeriodicEventId = AddEvent(&HandleTriggerSpellEvent,(void*)this,m_modifier.periodictime,false,true);
        m_isPeriodic = true;
        m_periodicTimer = m_modifier.periodictime;
    }
    else
    {
        //RemovePeriodicEvent(m_PeriodicEventId);
        m_isPeriodic = false;
        m_duration = 0;
    }
}

void Aura::HandleAuraModResistanceExclusive(bool apply)
{
    uint16 index = 0;
    uint16 index2 = 0;
    switch(m_modifier.m_miscvalue)
    {
        case 1:
        {
            index = UNIT_FIELD_ARMOR;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE;
        }break;
        case IMMUNE_SCHOOL_HOLY:
        {
            index = UNIT_FIELD_RESISTANCES_01;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01;
        }break;
        case IMMUNE_SCHOOL_FIRE:
        {
            index = UNIT_FIELD_RESISTANCES_02;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_02 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_02;
        }break;
        case IMMUNE_SCHOOL_NATURE:
        {
            index = UNIT_FIELD_RESISTANCES_03;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_03 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_03;
        }break;
        case IMMUNE_SCHOOL_FROST:
        {
            index = UNIT_FIELD_RESISTANCES_04;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_04 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_04;
        }break;
        case IMMUNE_SCHOOL_SHADOW:
        {
            index = UNIT_FIELD_RESISTANCES_05;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_05 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_05;
        }break;
        case IMMUNE_SCHOOL_ARCANE:
        {
            index = UNIT_FIELD_RESISTANCES_06;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06;
        }
        break;
        case IMMUNE_SCHOOL_MAGIC:
        {
            for(int8 x=0;x < 6;x++)
            {
                index  = UNIT_FIELD_RESISTANCES_01 + x;
                index2 = m_modifier.m_miscvalue2 == 0 ? PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01 + x : PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01 + x;

                m_target->ApplyModUInt32Value(index,m_modifier.m_amount,apply);
                if(m_target->GetTypeId() == TYPEID_PLAYER)
                    m_target->ApplyModUInt32Value(index2,m_modifier.m_amount,apply);
            }
            return;
        }break;
        default:
        {
            sLog.outString("WARNING: Misc Value for SPELL_AURA_MOD_STAT not valid");
            return;
        }break;
    }

    m_target->ApplyModUInt32Value(index,m_modifier.m_amount,apply);
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        m_target->ApplyModUInt32Value(index2,m_modifier.m_amount,apply);
}

void Aura::HandleAuraSafeFall(bool apply)
{
    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_FEATHER_FALL);
    else
        data.Initialize(SMSG_MOVE_NORMAL_FALL);
    data << uint8(0xFF) << m_target->GetGUID();
    m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraDamageShield(bool apply)
{
    if(apply)
    {
        for(std::list<struct DamageShield>::iterator i = m_target->m_damageShields.begin();i != m_target->m_damageShields.end();i++)
            if(i->m_spellId == GetId() && i->m_caster == GetCaster())
        {
            m_target->m_damageShields.erase(i);
            break;
        }
        DamageShield* ds = new DamageShield();
        ds->m_caster = GetCaster();
        ds->m_damage = m_modifier.m_amount;
        ds->m_spellId = GetId();
        m_target->m_damageShields.push_back((*ds));
    }
    else
    {
        for(std::list<struct DamageShield>::iterator i = m_target->m_damageShields.begin();i != m_target->m_damageShields.end();i++)
            if(i->m_spellId == GetId() && i->m_caster == GetCaster())
        {
            m_target->m_damageShields.erase(i);
            break;
        }
    }
}

void Aura::HandleModStealth(bool apply)
{
    if(apply)
    {
        m_target->m_stealthvalue = CalculateDamage();
        m_target->SetFlag(UNIT_FIELD_BYTES_1, (0x2000000) );
    }
    else
    {
        SendCoolDownEvent();
        m_target->m_stealthvalue = 0;
        m_target->RemoveFlag(UNIT_FIELD_BYTES_1, (0x2000000) );
    }
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        m_target->SendUpdateToPlayer((Player*)m_target);
}

void Aura::HandleModDetect(bool apply)
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

void Aura::HandleInvisibility(bool Apply)
{
    if(Apply)
    {
        m_target->m_stealthvalue = CalculateDamage();
        m_target->SetFlag(UNIT_FIELD_BYTES_1, (0x2000000) );
    }
    else
    {
        SendCoolDownEvent();
        m_target->m_stealthvalue = 0;
        m_target->RemoveFlag(UNIT_FIELD_BYTES_1, (0x2000000) );
    }
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        m_target->SendUpdateToPlayer((Player*)m_target);
}

void Aura::HandleInvisibilityDetect(bool Apply)
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

void Aura::HandleAuraModResistance(bool apply)
{
    uint16 index = 0;
    uint16 index2 = 0;
    switch(m_modifier.m_miscvalue)
    {
        case 1:
        {
            index = UNIT_FIELD_ARMOR;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE;
        }break;
        case IMMUNE_SCHOOL_HOLY:
        {
            index = UNIT_FIELD_RESISTANCES_01;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01;
        }break;
        case IMMUNE_SCHOOL_FIRE:
        {
            index = UNIT_FIELD_RESISTANCES_02;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_02 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_02;
        }break;
        case IMMUNE_SCHOOL_NATURE:
        {
            index = UNIT_FIELD_RESISTANCES_03;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_03 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_03;
        }break;
        case IMMUNE_SCHOOL_FROST:
        {
            index = UNIT_FIELD_RESISTANCES_04;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_04 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_04;
        }break;
        case IMMUNE_SCHOOL_SHADOW:
        {
            index = UNIT_FIELD_RESISTANCES_05;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_05 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_05;
        }break;
        case IMMUNE_SCHOOL_ARCANE:
        {
            index = UNIT_FIELD_RESISTANCES_06;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06;
        }
        break;
        case IMMUNE_SCHOOL_MAGIC:
        {
            for(int8 x=0;x < 6;x++)
            {
                index  = UNIT_FIELD_RESISTANCES_01 + x;
                index2 = m_modifier.m_miscvalue2 == 0 ? PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01 + x : PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01 + x;

                m_target->ApplyModUInt32Value(index,m_modifier.m_amount,apply);
                if(m_target->GetTypeId() == TYPEID_PLAYER)
                    m_target->ApplyModUInt32Value(index2,m_modifier.m_amount,apply);
            }
            return;
        }break;
        default:
        {
            sLog.outString("WARNING: Misc Value for SPELL_AURA_MOD_STAT not valid");
            return;
        }break;
    }

    m_target->ApplyModUInt32Value(index,m_modifier.m_amount,apply);
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        m_target->ApplyModUInt32Value(index2,m_modifier.m_amount,apply);
}

void Aura::HandleAuraModRoot(bool apply)
{
    uint32 apply_stat = UNIT_STAT_ROOT;
    if (apply)
    {
        m_target->addUnitState(UNIT_STAT_ROOT);
        m_target->SetUInt64Value (UNIT_FIELD_TARGET, 0);
        m_target->SetFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));
        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            WorldPacket data;
            data.Initialize(SMSG_FORCE_MOVE_ROOT);
            data << uint8(0xFF) << m_target->GetGUID() << (uint32)2;
            m_target->SendMessageToSet(&data,true);
        }
        else
            ((Creature *)m_target)->StopMoving();
    }
    else
    {
        m_target->clearUnitState(UNIT_STAT_ROOT);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));
        WorldPacket data;
        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            WorldPacket data;
            data.Initialize(SMSG_FORCE_MOVE_UNROOT);
            data << uint8(0xFF) << m_target->GetGUID() << (uint32)2;
            m_target->SendMessageToSet(&data,true);
        }
    }
}

void Aura::HandleAuraModSilence(bool apply)
{
    apply ? m_target->m_silenced = true : m_target->m_silenced = false;
}

void Aura::HandleReflectSpells(bool apply)
{
    if(apply)
    {
        for(std::list<struct ReflectSpellSchool*>::iterator i = m_target->m_reflectSpellSchool.begin();i != m_target->m_reflectSpellSchool.end();i++)
        {
            if(GetId() == (*i)->spellId)
            {
                m_target->m_reflectSpellSchool.erase(i);
            }
        }
        ReflectSpellSchool *rss = new ReflectSpellSchool();

        rss->chance = m_modifier.m_amount;
        rss->spellId = GetId();
        rss->school = -1;
        m_target->m_reflectSpellSchool.push_back(rss);
    }
    else
    {
        for(std::list<struct ReflectSpellSchool*>::iterator i = m_target->m_reflectSpellSchool.begin();i != m_target->m_reflectSpellSchool.end();i++)
        {
            if(GetId() == (*i)->spellId)
            {
                m_target->m_reflectSpellSchool.erase(i);
                break;
            }
        }
    }
}

void Aura::HandleAuraModStat(bool apply)
{
    uint16 index = 0;
    uint16 index2 = 0;
    uint16 index3 = 0;
    switch(m_modifier.m_miscvalue)
    {
        case 0:
            index = UNIT_FIELD_STR;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_POSSTAT0 : index2 = PLAYER_FIELD_NEGSTAT0;
            break;
        case 1:
            index = UNIT_FIELD_AGILITY;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_POSSTAT1 : index2 = PLAYER_FIELD_NEGSTAT1;
            break;
        case 2:
            index = UNIT_FIELD_STAMINA;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_POSSTAT2 : index2 = PLAYER_FIELD_NEGSTAT2;
            index3 = UNIT_FIELD_MAXHEALTH;
            break;
        case 3:
            index = UNIT_FIELD_IQ;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_POSSTAT3 : index2 = PLAYER_FIELD_NEGSTAT3;
            index3 = UNIT_FIELD_MAXPOWER1;
            break;
        case 4:
            index = UNIT_FIELD_SPIRIT;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_POSSTAT4 : index2 = PLAYER_FIELD_NEGSTAT4;
            break;
        case -1:
        {
            index = UNIT_FIELD_STR;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_POSSTAT0 : index2 = PLAYER_FIELD_NEGSTAT0;
            for(int x=0;x<5;x++)
            {
                m_target->ApplyModUInt32Value(index+x,m_modifier.m_amount,apply);
                if(m_target->GetTypeId() == TYPEID_PLAYER)
                    m_target->ApplyModUInt32Value(index2+x,m_modifier.m_amount,apply);
            }
            return;
        }break;
        default:
            sLog.outString("WARNING: Misc Value for SPELL_AURA_MOD_STAT not valid");
            return;
            break;
    }

    m_target->ApplyModUInt32Value(index,m_modifier.m_amount,apply);
    //if(index3)
    //    m_target->ApplyModUInt32Value(index3, (m_modifier.m_miscvalue2 == 0 ? m_modifier.m_amount:-m_modifier.m_amount)*(m_modifier.m_miscvalue==2?10:15),apply);
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        m_target->ApplyModUInt32Value(index2,m_modifier.m_amount,apply);
}

void Aura::HandleAuraModIncreaseSpeedAlways(bool apply)
{
    sLog.outDebug("Current Speed:%f \tmodify:%f", m_target->GetSpeed(MOVE_RUN),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;
    WorldPacket data;
    if(apply)
        m_target->SetSpeed( m_target->GetSpeed() * (100.0f + m_modifier.m_amount)/100.0f );
    else
        m_target->SetSpeed( m_target->GetSpeed() * 100.0f/(100.0f + m_modifier.m_amount) );
    data.Initialize(MSG_MOVE_SET_RUN_SPEED);
    data << m_target->GetGUID();
    data << m_target->GetSpeed( MOVE_RUN );
    m_target->SendMessageToSet(&data,true);
    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_RUN));
}

void Aura::HandleAuraModIncreaseSpeed(bool apply)
{
    sLog.outDebug("Current Speed:%f \tmodify:%f", m_target->GetSpeed(MOVE_RUN),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;
    WorldPacket data;
    if(apply)
        m_target->SetSpeed( m_target->GetSpeed() * (100.0f + m_modifier.m_amount)/100.0f );
    else
        m_target->SetSpeed( m_target->GetSpeed() * 100.0f/(100.0f + m_modifier.m_amount) );
    data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
    data << uint8(0xFF);
    data << m_target->GetGUID();
    data << (uint32)0;
    data << m_target->GetSpeed( MOVE_RUN );

    m_target->SendMessageToSet(&data,true);
    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_RUN));
}

void Aura::HandleAuraModIncreaseMountedSpeed(bool apply)
{
    sLog.outDebug("Current Speed:%f \tmodify:%f", m_target->GetSpeed(MOVE_RUN),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;
    WorldPacket data;
    if(apply)
        m_target->SetSpeed( m_target->GetSpeed() * ( m_modifier.m_amount + 100.0f ) / 100.0f );
    else
        m_target->SetSpeed( m_target->GetSpeed() * 100.0f / ( m_modifier.m_amount + 100.0f ) );
    data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
    data << uint8(0xFF);
    data << m_target->GetGUID();
    data << (uint32)0;
    data << m_target->GetSpeed( MOVE_RUN );
    m_target->SendMessageToSet(&data,true);
    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_RUN));
}

void Aura::HandleAuraModDecreaseSpeed(bool apply)
{
    sLog.outDebug("Current Speed:%f \tmodify:%f", m_target->GetSpeed(MOVE_RUN),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;
    WorldPacket data;
    if(apply)
        m_target->SetSpeed( m_target->GetSpeed() * m_modifier.m_amount/100.0f );
    else
        m_target->SetSpeed( m_target->GetSpeed() * 100.0f/m_modifier.m_amount );
    data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
    data << uint8(0xFF);
    data << m_target->GetGUID();
    data << (uint32)0;
    data << m_target->GetSpeed( MOVE_RUN );
    m_target->SendMessageToSet(&data,true);
    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_RUN));
}

void Aura::HandleAuraModIncreaseHealth(bool apply)
{
    uint32 newValue;
    newValue = m_target->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    apply ? newValue += m_modifier.m_amount : newValue -= m_modifier.m_amount;
    m_target->SetUInt32Value(UNIT_FIELD_MAXHEALTH,newValue);
}

void Aura::HandleAuraModIncreaseEnergy(bool apply)
{
    uint16 powerField = UNIT_FIELD_POWER1;
    uint8 powerType = m_target->getPowerType();
    if(powerType != m_modifier.m_miscvalue)
        return;
    if(powerType == 0)
        powerField = UNIT_FIELD_POWER1;
    else if(powerType == 1)
        powerField = UNIT_FIELD_POWER2;
    else if(powerType == 3)
        powerField = UNIT_FIELD_POWER4;
    else
    {
        powerField = UNIT_FIELD_POWER1;
        sLog.outError("AURA: unknown power type %i spell id %u\n",(int)powerType, m_spellId);
    }

    uint32 newValue = m_target->GetUInt32Value(powerField);
    apply ? newValue += m_modifier.m_amount : newValue -= m_modifier.m_amount;
    m_target->SetUInt32Value(powerField,newValue);
}

void Aura::HandleAuraModShapeshift(bool apply)
{
    if(!m_target)
        return;
    Unit *unit_target = m_target;
    uint32 spellId = 0;
    uint32 modelid = 0;
    uint8 PowerType = 0;
    uint32 new_bytes_1 = m_modifier.m_miscvalue;
    switch(m_modifier.m_miscvalue)
    {
        case FORM_CAT:
            if(unit_target->getRace() == RACE_NIGHT_ELF)
                modelid = 892;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 8571;
            PowerType = 3;
            spellId = 3025;
            break;
        case FORM_TREE:
            spellId = 3122;
            break;
        case FORM_TRAVEL:
            modelid = 632;
            spellId = 5419;
            break;
        case FORM_AQUA:
            if(unit_target->getRace() == RACE_NIGHT_ELF)
                modelid = 2428;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 2428;
            spellId = 5421;
            break;
        case FORM_BEAR:
            if(unit_target->getRace() == RACE_NIGHT_ELF)
                modelid = 2281;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 2289;
            PowerType = 1;
            spellId = 1178;
            break;
        case FORM_AMBIENT:
            spellId = 0;
            break;
        case FORM_GHOUL:
            if(unit_target->getRace() == RACE_NIGHT_ELF)
                modelid = 10045;
            spellId = 0;
            break;
        case FORM_DIREBEAR:
            if(unit_target->getRace() == RACE_NIGHT_ELF)
                modelid = 2281;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 2289;
            PowerType = 1;
            spellId = 9635;
            break;
        case FORM_CREATUREBEAR:
            modelid = 902;
            spellId = 2882;
            break;
        case FORM_GHOSTWOLF:
            modelid = 1236;
            spellId = 0;
            break;
        case FORM_BATTLESTANCE:
            spellId = 0;
            break;
        case FORM_DEFENSIVESTANCE:
            spellId = 7376;
            break;
        case FORM_BERSERKERSTANCE:
            spellId = 7381;
            break;
        case FORM_SHADOW:
            spellId = 0;
            break;
        case FORM_STEALTH:
            spellId = 0;
            break;
        case FORM_MOONKIN:
            if(unit_target->getRace() == RACE_NIGHT_ELF)
                modelid = 15374;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 15375;
            spellId = 24907;
            break;
        default:
            sLog.outString("Unknown Shapeshift Type: %u", m_modifier.m_miscvalue);
    }

    SpellEntry *spellInfo = sSpellStore.LookupEntry( spellId );

    if(apply)
    {
        if(unit_target->GetTypeId() == TYPEID_PLAYER)
        {
            if(((Player*)unit_target)->IsInWater())
            {
                if(m_modifier.m_miscvalue != FORM_AQUA )
                    return;
            }
        }
        if(unit_target->m_ShapeShiftForm)
            unit_target->RemoveAurasDueToSpell(unit_target->m_ShapeShiftForm);

        unit_target->SetFlag(UNIT_FIELD_BYTES_1, (new_bytes_1<<16) );
        if(modelid > 0)
        {
            unit_target->SetUInt32Value(UNIT_FIELD_DISPLAYID,modelid);
        }
        if(PowerType > 0)
        {
            unit_target->setPowerType(PowerType);
        }
        unit_target->m_ShapeShiftForm = m_spellId;
        unit_target->m_form = m_modifier.m_miscvalue;
        if(unit_target->m_form == FORM_DIREBEAR)
			if (m_target->getRace() == TAUREN)
			{
				m_target->SetFloatValue(OBJECT_FIELD_SCALE_X,1.35f);
			}
			else
            m_target->SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);

        if(spellInfo)
        {
            Spell *p_spell = new Spell(m_caster,spellInfo,true,0);
            WPAssert(p_spell);
            SpellCastTargets targets;
            targets.setUnitTarget(unit_target);
            p_spell->prepare(&targets);
        }
    }
    else
    {
		if (m_target->getRace() == TAUREN)
				unit_target->SetFloatValue(OBJECT_FIELD_SCALE_X,1.35f);
		else
        unit_target->SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);
        unit_target->SetUInt32Value(UNIT_FIELD_DISPLAYID,unit_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
        unit_target->RemoveFlag(UNIT_FIELD_BYTES_1, (new_bytes_1<<16) );
        if(unit_target->getClass() == CLASS_DRUID)
            unit_target->setPowerType(0);
        unit_target->m_ShapeShiftForm = 0;
        unit_target->m_form = 0;
        unit_target->RemoveAurasDueToSpell(spellId);
    }
    if(unit_target->GetTypeId() == TYPEID_PLAYER)
        unit_target->SendUpdateToPlayer((Player*)unit_target);
}

void Aura::HandleModMechanicImmunity(bool apply)
{
    apply ? m_target->SetStateFlag(m_target->m_immuneToMechanic,m_modifier.m_miscvalue) : m_target->RemoveStateFlag(m_target->m_immuneToMechanic,m_modifier.m_miscvalue);
}

void Aura::HandleAuraModEffectImmunity(bool apply)
{
    apply ? m_target->SetStateFlag(m_target->m_immuneToEffect,m_modifier.m_miscvalue) : m_target->RemoveStateFlag(m_target->m_immuneToEffect,m_modifier.m_miscvalue);
}

void Aura::HandleAuraModStateImmunity(bool apply)
{
    apply ? m_target->SetStateFlag(m_target->m_immuneToState,m_modifier.m_miscvalue) : m_target->RemoveStateFlag(m_target->m_immuneToState,m_modifier.m_miscvalue);
}

void Aura::HandleAuraModSchoolImmunity(bool apply)
{
    apply ? m_target->SetStateFlag(m_target->m_immuneToSchool,m_modifier.m_miscvalue) : m_target->RemoveStateFlag(m_target->m_immuneToSchool,m_modifier.m_miscvalue);
}

void Aura::HandleAuraModDmgImmunity(bool apply)
{
    apply ? m_target->SetStateFlag(m_target->m_immuneToDmg,m_modifier.m_miscvalue) : m_target->RemoveStateFlag(m_target->m_immuneToDmg,m_modifier.m_miscvalue);
}

void Aura::HandleAuraModDispelImmunity(bool apply)
{
    apply ? m_target->SetStateFlag(m_target->m_immuneToDispel,m_modifier.m_miscvalue) : m_target->RemoveStateFlag(m_target->m_immuneToDispel,m_modifier.m_miscvalue);
}

void Aura::HandleAuraProcTriggerSpell(bool apply)
{
    if(apply)
    {
        m_procSpell = new ProcTriggerSpell();
        m_procSpell->caster = m_caster->GetGUID();
        m_procSpell->spellId = GetSpellProto()->EffectTriggerSpell[GetEffIndex()];
        m_procSpell->procChance = GetSpellProto()->procChance;
        m_procSpell->procFlags = GetSpellProto()->procFlags;
        m_procSpell->procCharges = GetSpellProto()->procCharges;
    }
    else
    {
        delete m_procSpell;
        m_procSpell = NULL;
    }
}

void Aura::HandleAuraProcTriggerDamage(bool apply)
{
    if(apply)
    {
        m_procdamage->caster = m_caster->GetGUID();
        m_procdamage->procDamage = m_modifier.m_amount;
        m_procdamage->procChance = GetSpellProto()->procChance;
        m_procdamage->procFlags = GetSpellProto()->procFlags;
        m_procdamage->procCharges = GetSpellProto()->procCharges;
    }
    else
    {
        m_procdamage = NULL;
    }    
}

void Aura::HandleAuraTracCreatures(bool apply)
{
    m_target->SetUInt32Value(PLAYER_TRACK_CREATURES, apply ? m_modifier.m_miscvalue : 0 );
}

void Aura::HandleAuraTracResources(bool apply)
{
    m_target->SetUInt32Value(PLAYER_TRACK_RESOURCES, apply ? ((uint32)1)<<(m_modifier.m_miscvalue-1): 0 );
}

void Aura::HandleAuraModParryPercent(bool apply)
{
    m_target->ApplyModFloatValue(PLAYER_PARRY_PERCENTAGE,m_modifier.m_amount,apply);
}

void Aura::HandleAuraModDodgePercent(bool apply)
{
    m_target->ApplyModFloatValue(PLAYER_DODGE_PERCENTAGE,m_modifier.m_amount,apply);
}

void Aura::HandleAuraModBlockPercent(bool apply)
{
    m_target->ApplyModFloatValue(PLAYER_BLOCK_PERCENTAGE,m_modifier.m_amount,apply);
}

void Aura::HandleAuraModCritPercent(bool apply)
{
    m_target->ApplyModFloatValue(PLAYER_CRIT_PERCENTAGE,m_modifier.m_amount,apply);
}

void Aura::HandlePeriodicLeech(bool apply)
{
    if(apply)
    {
        //m_PeriodicEventId = AddEvent(&HandleTriggerSpellEvent,(void*)this,m_modifier.periodictime,false,true);
        m_isPeriodic = true;
        m_periodicTimer = m_modifier.periodictime;
    }
    else
    {
        //RemovePeriodicEvent(m_PeriodicEventId);
        m_isPeriodic = false;
        m_duration = 0;
    }
}

void Aura::HandleModHitChance(bool Apply)
{
    m_target->m_modHitChance = Apply?m_modifier.m_amount:0;
}

void Aura::HandleModSpellHitChance(bool Apply)
{
    m_target->m_modSpellHitChance = Apply?m_modifier.m_amount:0;
}

void Aura::HandleAuraModScale(bool apply)
{
    m_target->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE_X,10,apply);
}

void Aura::HandlePeriodicManaLeech(bool Apply)
{
    if(Apply)
    {
        //m_PeriodicEventId = AddEvent(&HandleTriggerSpellEvent,(void*)this,m_modifier.periodictime,false,true);
        m_isPeriodic = true;
        m_periodicTimer = m_modifier.periodictime;
    }
    else
    {
        //RemovePeriodicEvent(m_PeriodicEventId);
        m_isPeriodic = false;
        m_duration = 0;
    }
}

void Aura::HandleAuraMounted(bool apply)
{
    if(apply)
    {
        CreatureInfo* ci = objmgr.GetCreatureTemplate(m_modifier.m_miscvalue);
        if(!ci)return;
        uint32 displayId = ci->DisplayID;
        if(displayId != 0)
        {
            m_target->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID , displayId);
            //m_target->SetUInt32Value( UNIT_FIELD_FLAGS , 0x002000 );
            //m_target->SetFlag( UNIT_FIELD_FLAGS ,0x000004 );
            m_target->SetFlag( UNIT_FIELD_FLAGS, 0x002000 );
        }
    }else
    {
        m_target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
        m_target->RemoveFlag( UNIT_FIELD_FLAGS, 0x002000 );

        if (m_target->GetUInt32Value(UNIT_FIELD_FLAGS) & 0x000004 )
            m_target->RemoveFlag( UNIT_FIELD_FLAGS, 0x000004 );
    }
}

void Aura::HandleWaterBreathing(bool apply)
{
    m_target->waterbreath = apply;
}

void Aura::HandleModBaseResistance(bool apply)
{
    if(m_modifier.m_miscvalue == 1 || m_modifier.m_miscvalue == 127)
        m_target->ApplyModUInt32Value(UNIT_FIELD_RESISTANCES, m_modifier.m_amount, apply);
    if(m_modifier.m_miscvalue == 126 || m_modifier.m_miscvalue == 127)
    {
        m_target->ApplyModUInt32Value(UNIT_FIELD_RESISTANCES_01, m_modifier.m_amount, apply);
        m_target->ApplyModUInt32Value(UNIT_FIELD_RESISTANCES_02, m_modifier.m_amount, apply);
        m_target->ApplyModUInt32Value(UNIT_FIELD_RESISTANCES_03, m_modifier.m_amount, apply);
        m_target->ApplyModUInt32Value(UNIT_FIELD_RESISTANCES_04, m_modifier.m_amount, apply);
        m_target->ApplyModUInt32Value(UNIT_FIELD_RESISTANCES_05, m_modifier.m_amount, apply);
        m_target->ApplyModUInt32Value(UNIT_FIELD_RESISTANCES_06, m_modifier.m_amount, apply);
    }
}

void Aura::HandleModRegen(bool apply)                       // eating
{
    m_target->ApplyModFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT,apply);
    // add event
}

void Aura::HandleModPowerRegen(bool apply)                  // drinking
{
    m_target->ApplyModFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT,apply);
}

void Aura::HandleChannelDeathItem(bool apply)
{
    if(!apply)
    {
        if(m_caster->GetTypeId() != TYPEID_PLAYER || m_target->isAlive())
            return;
        SpellEntry *spellInfo = GetSpellProto();
        if(spellInfo->EffectItemType[m_effIndex] == 0)
            return;
        uint16 dest;
        uint8 msg = ((Player*)m_caster)->CanStoreNewItem( 0, NULL_SLOT, dest, spellInfo->EffectItemType[m_effIndex], 1, false);
        if( msg == EQUIP_ERR_OK )
            ((Player*)m_caster)->StoreNewItem(dest, spellInfo->EffectItemType[m_effIndex], 1, true);
        else
            ((Player*)m_caster)->SendEquipError( msg, NULL, NULL );
    }
}

void Aura::HandleModDamagePCTTaken(bool apply)
{
    m_target->m_modDamagePCT = apply ? m_modifier.m_amount : 0;
}

void Aura::HandleModPCTRegen(bool apply)
{
    m_target->m_RegenPCT = apply ? m_modifier.m_amount : 0;
}

void Aura::HandlePeriodicDamagePCT(bool apply)
{
    if(apply)
    {
        //m_PeriodicEventId = AddEvent(&HandleTriggerSpellEvent,(void*)this,m_modifier.periodictime,false,true);
        m_isPeriodic = true;
        m_periodicTimer = m_modifier.periodictime;
    }
    else
    {
        //RemovePeriodicEvent(m_PeriodicEventId);
        m_isPeriodic = false;
        m_duration = 0;
    }
}

void Aura::HandleAuraModAttackPower(bool apply)
{
    m_target->ApplyModUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS, m_modifier.m_amount, apply);
}

void Aura::HandleAuraTransform(bool apply)
{
    if(!m_target)
        return;
    if (m_target->m_immuneToMechanic & IMMUNE_MECHANIC_POLYMORPH)   //Can't transform
        return;

    if (apply)
    {
        CreatureInfo* ci = objmgr.GetCreatureTemplate(m_modifier.m_miscvalue);
        m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, ci->DisplayID);
        m_target->setTransForm(GetSpellProto()->Id);
    }
    else
    {
        m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
        m_target->setTransForm(0);
    }
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        m_target->SendUpdateToPlayer((Player*)m_caster);

    /*uint32 id=GetId();
    switch (id)
    {
        case 118:
        case 851:
        case 5254:
        case 12824:
        case 12825:
        case 12826:
        case 13323:
            if (apply)
                m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, 856);
            else
                m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
            break;
        case 228:
            if (apply)
                m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, 304);
            else
                m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
            break;

        case 4060:
            if (apply)
                m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, 131);
            else
                m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
            break;

        case 15534:
            if (apply)
                m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, 1141);
            else
                m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
            break;

    }*/
}

void Aura::HandleModSpellCritChance(bool Apply)
{
    m_target->m_baseSpellCritChance += Apply?m_modifier.m_amount:(-m_modifier.m_amount);
}

void Aura::HandleAuraModIncreaseSwimSpeed(bool Apply)
{
    sLog.outDebug("Current Speed:%f \tmodify:%f", m_target->GetSpeed(MOVE_SWIM),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;
    WorldPacket data;
    if(Apply)
        m_target->SetSpeed( m_target->GetSpeed() * ( m_modifier.m_amount + 100.0f ) / 100.0f );
    else
        m_target->SetSpeed( m_target->GetSpeed() * 100.0f / ( m_modifier.m_amount + 100.0f ) );
    data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
    data << uint8(0xFF);
    data << m_target->GetGUID();
    data << (uint32)0;
    data << m_target->GetSpeed( MOVE_SWIM );
    m_target->SendMessageToSet(&data,true);
    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_SWIM));
}

void Aura::HandleModDamageDoneCreature(bool Apply)
{
    if(Apply)
    {

        for(std::list<struct DamageDoneCreature*>::iterator i = m_target->m_damageDoneCreature.begin();i != m_target->m_damageDoneCreature.end();i++)
        {
            if(GetId() == (*i)->spellId)
            {
                m_target->m_damageDoneCreature.erase(i);
            }
        }

        DamageDoneCreature *ddc = new DamageDoneCreature();

        ddc->spellId = GetId();
        ddc->damage = m_modifier.m_amount;
        ddc->creaturetype = m_modifier.m_miscvalue;
        m_target->m_damageDoneCreature.push_back(ddc);
    }
    else
    {
        for(std::list<struct DamageDoneCreature*>::iterator i = m_target->m_damageDoneCreature.begin();i != m_target->m_damageDoneCreature.end();i++)
        {
            if(GetId() == (*i)->spellId)
            {
                m_target->m_damageDoneCreature.erase(i);
                break;
            }
        }
    }
}

void Aura::HandleAuraManaShield(bool apply)
{
    if(apply)
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
    }
}

void Aura::HandleAuraSchoolAbsorb(bool apply)
{
    if(apply)
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
    }
}

void Aura::HandleModSpellCritChanceShool(bool Apply)
{
    if(Apply)
    {
        for(std::list<struct SpellCritSchool*>::iterator i = m_target->m_spellCritSchool.begin();i != m_target->m_spellCritSchool.end();i++)
        {
            if(GetId() == (*i)->spellId)
            {
                m_target->m_spellCritSchool.erase(i);
            }
        }
        SpellCritSchool *scs = new SpellCritSchool();

        scs->chance = m_modifier.m_amount;
        scs->spellId = GetId();
        scs->school = m_modifier.m_miscvalue;
        m_target->m_spellCritSchool.push_back(scs);
    }
    else
    {
        for(std::list<struct SpellCritSchool*>::iterator i = m_target->m_spellCritSchool.begin();i != m_target->m_spellCritSchool.end();i++)
        {
            if(GetId() == (*i)->spellId)
            {
                m_target->m_spellCritSchool.erase(i);
                break;
            }
        }
    }
}

void Aura::HandleReflectSpellsSchool(bool apply)
{
    if(apply)
    {
        for(std::list<struct ReflectSpellSchool*>::iterator i = m_target->m_reflectSpellSchool.begin();i != m_target->m_reflectSpellSchool.end();i++)
        {
            if(GetId() == (*i)->spellId)
            {
                m_target->m_reflectSpellSchool.erase(i);
            }
        }
        ReflectSpellSchool *rss = new ReflectSpellSchool();

        rss->chance = m_modifier.m_amount;
        rss->spellId = GetId();
        rss->school = m_modifier.m_miscvalue;
        m_target->m_reflectSpellSchool.push_back(rss);
    }
    else
    {
        for(std::list<struct ReflectSpellSchool*>::iterator i = m_target->m_reflectSpellSchool.begin();i != m_target->m_reflectSpellSchool.end();i++)
        {
            if(GetId() == (*i)->spellId)
            {
                m_target->m_reflectSpellSchool.erase(i);
                break;
            }
        }
    }
}

void Aura::HandleAuraModSkill(bool apply)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    SpellEntry* prot=GetSpellProto();

    ((Player*)m_target)->ModifySkillBonus(prot->EffectMiscValue[0],
        (apply ? (prot->EffectBasePoints[0]+1): (-(prot->EffectBasePoints[0]+1))));
}

void Aura::HandleModDamagePercentDone(bool apply)
{
    sLog.outDebug("AURA MOD DAMAGE type:%u type2:%u", m_modifier.m_miscvalue, m_modifier.m_miscvalue2);

    if(m_modifier.m_miscvalue == 1)
    {
        m_target->ApplyPercentModFloatValue(UNIT_FIELD_MINDAMAGE, m_modifier.m_amount, apply );
        m_target->ApplyPercentModFloatValue(UNIT_FIELD_MAXDAMAGE, m_modifier.m_amount, apply );
    }
    if(m_modifier.m_miscvalue == 126)
    {
        m_target->ApplyPercentModFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, m_modifier.m_amount, apply );
        m_target->ApplyPercentModFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, m_modifier.m_amount, apply );
    }
    if(m_modifier.m_miscvalue == 127)
    {
        m_target->ApplyPercentModFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, m_modifier.m_amount, apply );
        m_target->ApplyPercentModFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, m_modifier.m_amount, apply );
    }
}

void Aura::HandleModPercentStat(bool apply)
{
    m_target->ApplyPercentModUInt32Value(UNIT_FIELD_MAXHEALTH, m_modifier.m_amount, apply );
    m_target->ApplyPercentModUInt32Value(UNIT_FIELD_MAXPOWER1, m_modifier.m_amount, apply );
    m_target->ApplyPercentModUInt32Value(UNIT_FIELD_MAXPOWER2, m_modifier.m_amount, apply );
    m_target->ApplyPercentModUInt32Value(UNIT_FIELD_MAXPOWER3, m_modifier.m_amount, apply );
    m_target->ApplyPercentModUInt32Value(UNIT_FIELD_MAXPOWER4, m_modifier.m_amount, apply );
    m_target->ApplyPercentModUInt32Value(UNIT_FIELD_MAXPOWER5, m_modifier.m_amount, apply );
    if(m_modifier.m_miscvalue == 0 || m_modifier.m_miscvalue == -1)
        m_target->ApplyPercentModUInt32Value(UNIT_FIELD_STR,     m_modifier.m_amount, apply );
    if(m_modifier.m_miscvalue == 1 || m_modifier.m_miscvalue == -1)
        m_target->ApplyPercentModUInt32Value(UNIT_FIELD_AGILITY, m_modifier.m_amount, apply );
    if(m_modifier.m_miscvalue == 2 || m_modifier.m_miscvalue == -1)
        m_target->ApplyPercentModUInt32Value(UNIT_FIELD_STAMINA, m_modifier.m_amount, apply );
    if(m_modifier.m_miscvalue == 3 || m_modifier.m_miscvalue == -1)
        m_target->ApplyPercentModUInt32Value(UNIT_FIELD_IQ,      m_modifier.m_amount, apply );
    if(m_modifier.m_miscvalue == 4 || m_modifier.m_miscvalue == -1)
        m_target->ApplyPercentModUInt32Value(UNIT_FIELD_SPIRIT,  m_modifier.m_amount, apply );
}

void Aura::HandleModResistancePercent(bool apply)
{
    if(m_modifier.m_miscvalue == 1 || m_modifier.m_miscvalue == 127)
        m_target->ApplyPercentModUInt32Value(UNIT_FIELD_RESISTANCES, m_modifier.m_amount, apply );
    if(m_modifier.m_miscvalue == 127 || m_modifier.m_miscvalue == 126)
    {
        m_target->ApplyPercentModUInt32Value(UNIT_FIELD_RESISTANCES_01, m_modifier.m_amount, apply );
        m_target->ApplyPercentModUInt32Value(UNIT_FIELD_RESISTANCES_02, m_modifier.m_amount, apply );
        m_target->ApplyPercentModUInt32Value(UNIT_FIELD_RESISTANCES_03, m_modifier.m_amount, apply );
        m_target->ApplyPercentModUInt32Value(UNIT_FIELD_RESISTANCES_04, m_modifier.m_amount, apply );
        m_target->ApplyPercentModUInt32Value(UNIT_FIELD_RESISTANCES_05, m_modifier.m_amount, apply );
        m_target->ApplyPercentModUInt32Value(UNIT_FIELD_RESISTANCES_06, m_modifier.m_amount, apply );
    }
}

void Aura::HandleAuraModBaseResistancePCT(bool apply)
{
    uint16 index = 0;
    uint16 index2 = 0;
    switch(m_modifier.m_miscvalue)
    {
        case 1:
        {
            index = UNIT_FIELD_ARMOR;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE;
        }break;
        case IMMUNE_SCHOOL_HOLY:
        {
            index = UNIT_FIELD_RESISTANCES_01;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01;
        }break;
        case IMMUNE_SCHOOL_FIRE:
        {
            index = UNIT_FIELD_RESISTANCES_02;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_02 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_02;
        }break;
        case IMMUNE_SCHOOL_NATURE:
        {
            index = UNIT_FIELD_RESISTANCES_03;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_03 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_03;
        }break;
        case IMMUNE_SCHOOL_FROST:
        {
            index = UNIT_FIELD_RESISTANCES_04;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_04 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_04;
        }break;
        case IMMUNE_SCHOOL_SHADOW:
        {
            index = UNIT_FIELD_RESISTANCES_05;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_05 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_05;
        }break;
        case IMMUNE_SCHOOL_ARCANE:
        {
            index = UNIT_FIELD_RESISTANCES_06;
            m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06;
        }
        break;
        case IMMUNE_SCHOOL_MAGIC:
        {
            for(int8 x=0;x < 6;x++)
            {
                index = UNIT_FIELD_RESISTANCES_01 + x;
                m_modifier.m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01 + x : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01 + x;

                m_target->ApplyPercentModUInt32Value(index,m_modifier.m_amount, apply);
                if(m_target->GetTypeId() == TYPEID_PLAYER)
                    m_target->ApplyPercentModUInt32Value(index2,m_modifier.m_amount, apply);
            }
            return;
        }break;

        default:
        {
            sLog.outString("WARNING: Misc Value for SPELL_AURA_MOD_STAT not valid");
            return;
        }break;
    }

    m_target->ApplyPercentModUInt32Value(index,m_modifier.m_amount,apply);
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        m_target->ApplyPercentModUInt32Value(index2,m_modifier.m_amount,apply);
}

void Aura::HandleForceReaction(bool Apply)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(Apply)
    {
        uint32 faction_id = m_modifier.m_miscvalue;

        FactionTemplateEntry *factionTemplateEntry;

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


void Aura::HandleRangedAmmoHaste(bool apply)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    m_target->ApplyPercentModUInt32Value(UNIT_FIELD_BASEATTACKTIME+1,m_modifier.m_amount, apply);
}

void Aura::SendCoolDownEvent()
{
    WorldPacket data;
    data.Initialize(SMSG_COOLDOWN_EVENT);
    data << uint32(m_spellId) << m_caster->GetGUID();
    data << uint32(0);                                      //CoolDown Time ?
    m_caster->SendMessageToSet(&data,true);
}

bool Aura::IsSingleTarget()
{
    SpellEntry *spellInfo = GetSpellProto();
    if (!spellInfo) return false;

    // cheap shot is an exception
    if ( m_spellId == 1833 || m_spellId == 14902 ) return false;

    // cannot be cast on another target while not cooled down anyway
    if ( GetAuraDuration() < spellInfo->RecoveryTime) return false;
    if ( spellInfo->RecoveryTime == 0 && GetAuraDuration() < spellInfo->CategoryRecoveryTime) return false;

    // banish is not covered
    if ( m_spellId == 710 || m_spellId == 18647 || m_spellId == 27565) return true;

    // all other single target spells have
    if ( spellInfo->AttributesEx & (1<<18) ) return true;
    return false;
}