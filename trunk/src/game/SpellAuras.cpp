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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "UpdateMask.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "Unit.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "DynamicObject.h"
#include "Group.h"
#include "UpdateData.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Policies/SingletonImp.h"
#include "Totem.h"
#include "Creature.h"
#include "ConfusedMovementGenerator.h"
#include "TargetedMovementGenerator.h"
#include "Formulas.h"
#include "BattleGround.h"
#include "BattleGroundAV.h"
#include "BattleGroundAB.h"
#include "BattleGroundEY.h"
#include "BattleGroundWS.h"
#include "CreatureAI.h"

pAuraHandler AuraHandler[TOTAL_AURAS]=
{
    &Aura::HandleNULL,                                      //  0 SPELL_AURA_NONE
    &Aura::HandleBindSight,                                 //  1 SPELL_AURA_BIND_SIGHT
    &Aura::HandleModPossess,                                //  2 SPELL_AURA_MOD_POSSESS
    &Aura::HandlePeriodicDamage,                            //  3 SPELL_AURA_PERIODIC_DAMAGE
    &Aura::HandleAuraDummy,                                 //  4 SPELL_AURA_DUMMY
    &Aura::HandleModConfuse,                                //  5 SPELL_AURA_MOD_CONFUSE
    &Aura::HandleModCharm,                                  //  6 SPELL_AURA_MOD_CHARM
    &Aura::HandleModFear,                                   //  7 SPELL_AURA_MOD_FEAR
    &Aura::HandlePeriodicHeal,                              //  8 SPELL_AURA_PERIODIC_HEAL
    &Aura::HandleModAttackSpeed,                            //  9 SPELL_AURA_MOD_ATTACKSPEED
    &Aura::HandleModThreat,                                 // 10 SPELL_AURA_MOD_THREAT
    &Aura::HandleModTaunt,                                  // 11 SPELL_AURA_MOD_TAUNT
    &Aura::HandleAuraModStun,                               // 12 SPELL_AURA_MOD_STUN
    &Aura::HandleModDamageDone,                             // 13 SPELL_AURA_MOD_DAMAGE_DONE
    &Aura::HandleNoImmediateEffect,                         // 14 SPELL_AURA_MOD_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &Aura::HandleAuraDamageShield,                          // 15 SPELL_AURA_DAMAGE_SHIELD
    &Aura::HandleModStealth,                                // 16 SPELL_AURA_MOD_STEALTH
    &Aura::HandleNoImmediateEffect,                         // 17 SPELL_AURA_MOD_STEALTH_DETECT
    &Aura::HandleInvisibility,                              // 18 SPELL_AURA_MOD_INVISIBILITY
    &Aura::HandleInvisibilityDetect,                        // 19 SPELL_AURA_MOD_INVISIBILITY_DETECTION
    &Aura::HandleAuraModTotalHealthPercentRegen,            // 20 SPELL_AURA_OBS_MOD_HEALTH
    &Aura::HandleAuraModTotalManaPercentRegen,              // 21 SPELL_AURA_OBS_MOD_MANA
    &Aura::HandleAuraModResistance,                         // 22 SPELL_AURA_MOD_RESISTANCE
    &Aura::HandlePeriodicTriggerSpell,                      // 23 SPELL_AURA_PERIODIC_TRIGGER_SPELL
    &Aura::HandlePeriodicEnergize,                          // 24 SPELL_AURA_PERIODIC_ENERGIZE
    &Aura::HandleAuraModPacify,                             // 25 SPELL_AURA_MOD_PACIFY
    &Aura::HandleAuraModRoot,                               // 26 SPELL_AURA_MOD_ROOT
    &Aura::HandleAuraModSilence,                            // 27 SPELL_AURA_MOD_SILENCE
    &Aura::HandleNoImmediateEffect,                         // 28 SPELL_AURA_REFLECT_SPELLS implemented in Spell::reflect
    &Aura::HandleAuraModStat,                               // 29 SPELL_AURA_MOD_STAT
    &Aura::HandleAuraModSkill,                              // 30 SPELL_AURA_MOD_SKILL
    &Aura::HandleAuraModIncreaseSpeed,                      // 31 SPELL_AURA_MOD_INCREASE_SPEED
    &Aura::HandleAuraModIncreaseMountedSpeed,               // 32 SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED
    &Aura::HandleAuraModDecreaseSpeed,                      // 33 SPELL_AURA_MOD_DECREASE_SPEED
    &Aura::HandleAuraModIncreaseHealth,                     // 34 SPELL_AURA_MOD_INCREASE_HEALTH
    &Aura::HandleAuraModIncreaseEnergy,                     // 35 SPELL_AURA_MOD_INCREASE_ENERGY
    &Aura::HandleAuraModShapeshift,                         // 36 SPELL_AURA_MOD_SHAPESHIFT
    &Aura::HandleAuraModEffectImmunity,                     // 37 SPELL_AURA_EFFECT_IMMUNITY
    &Aura::HandleAuraModStateImmunity,                      // 38 SPELL_AURA_STATE_IMMUNITY
    &Aura::HandleAuraModSchoolImmunity,                     // 39 SPELL_AURA_SCHOOL_IMMUNITY
    &Aura::HandleAuraModDmgImmunity,                        // 40 SPELL_AURA_DAMAGE_IMMUNITY
    &Aura::HandleAuraModDispelImmunity,                     // 41 SPELL_AURA_DISPEL_IMMUNITY
    &Aura::HandleAuraProcTriggerSpell,                      // 42 SPELL_AURA_PROC_TRIGGER_SPELL  implemented in Unit::ProcDamageAndSpellFor and Unit::HandleProcTriggerSpell
    &Aura::HandleNoImmediateEffect,                         // 43 SPELL_AURA_PROC_TRIGGER_DAMAGE implemented in Unit::ProcDamageAndSpellFor
    &Aura::HandleAuraTrackCreatures,                        // 44 SPELL_AURA_TRACK_CREATURES
    &Aura::HandleAuraTrackResources,                        // 45 SPELL_AURA_TRACK_RESOURCES
    &Aura::HandleNULL,                                      // 46 SPELL_AURA_MOD_PARRY_SKILL    obsolete?
    &Aura::HandleAuraModParryPercent,                       // 47 SPELL_AURA_MOD_PARRY_PERCENT
    &Aura::HandleNULL,                                      // 48 SPELL_AURA_MOD_DODGE_SKILL    obsolete?
    &Aura::HandleAuraModDodgePercent,                       // 49 SPELL_AURA_MOD_DODGE_PERCENT
    &Aura::HandleNULL,                                      // 50 SPELL_AURA_MOD_BLOCK_SKILL    obsolete?
    &Aura::HandleAuraModBlockPercent,                       // 51 SPELL_AURA_MOD_BLOCK_PERCENT
    &Aura::HandleAuraModCritPercent,                        // 52 SPELL_AURA_MOD_CRIT_PERCENT
    &Aura::HandlePeriodicLeech,                             // 53 SPELL_AURA_PERIODIC_LEECH
    &Aura::HandleModHitChance,                              // 54 SPELL_AURA_MOD_HIT_CHANCE
    &Aura::HandleModSpellHitChance,                         // 55 SPELL_AURA_MOD_SPELL_HIT_CHANCE
    &Aura::HandleAuraTransform,                             // 56 SPELL_AURA_TRANSFORM
    &Aura::HandleModSpellCritChance,                        // 57 SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    &Aura::HandleAuraModIncreaseSwimSpeed,                  // 58 SPELL_AURA_MOD_INCREASE_SWIM_SPEED
    &Aura::HandleNoImmediateEffect,                         // 59 SPELL_AURA_MOD_DAMAGE_DONE_CREATURE implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &Aura::HandleAuraModPacifyAndSilence,                   // 60 SPELL_AURA_MOD_PACIFY_SILENCE
    &Aura::HandleAuraModScale,                              // 61 SPELL_AURA_MOD_SCALE
    &Aura::HandleNULL,                                      // 62 SPELL_AURA_PERIODIC_HEALTH_FUNNEL
    &Aura::HandleNULL,                                      // 63 SPELL_AURA_PERIODIC_MANA_FUNNEL
    &Aura::HandlePeriodicManaLeech,                         // 64 SPELL_AURA_PERIODIC_MANA_LEECH
    &Aura::HandleModCastingSpeed,                           // 65 SPELL_AURA_MOD_CASTING_SPEED
    &Aura::HandleFeignDeath,                                // 66 SPELL_AURA_FEIGN_DEATH
    &Aura::HandleAuraModDisarm,                             // 67 SPELL_AURA_MOD_DISARM
    &Aura::HandleAuraModStalked,                            // 68 SPELL_AURA_MOD_STALKED
    &Aura::HandleNoImmediateEffect,                         // 69 SPELL_AURA_SCHOOL_ABSORB implemented in Unit::CalcAbsorbResist
    &Aura::HandleNULL,                                      // 70 SPELL_AURA_EXTRA_ATTACKS      Useless
    &Aura::HandleModSpellCritChanceShool,                   // 71 SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    &Aura::HandleModPowerCostPCT,                           // 72 SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT
    &Aura::HandleModPowerCost,                              // 73 SPELL_AURA_MOD_POWER_COST_SCHOOL
    &Aura::HandleNoImmediateEffect,                         // 74 SPELL_AURA_REFLECT_SPELLS_SCHOOL implemented in Spell::reflect
    &Aura::HandleNoImmediateEffect,                         // 75 SPELL_AURA_MOD_LANGUAGE
    &Aura::HandleFarSight,                                  // 76 SPELL_AURA_FAR_SIGHT
    &Aura::HandleModMechanicImmunity,                       // 77 SPELL_AURA_MECHANIC_IMMUNITY
    &Aura::HandleAuraMounted,                               // 78 SPELL_AURA_MOUNTED
    &Aura::HandleModDamagePercentDone,                      // 79 SPELL_AURA_MOD_DAMAGE_PERCENT_DONE
    &Aura::HandleModPercentStat,                            // 80 SPELL_AURA_MOD_PERCENT_STAT
    &Aura::HandleNoImmediateEffect,                         // 81 SPELL_AURA_SPLIT_DAMAGE_PCT
    &Aura::HandleWaterBreathing,                            // 82 SPELL_AURA_WATER_BREATHING
    &Aura::HandleModBaseResistance,                         // 83 SPELL_AURA_MOD_BASE_RESISTANCE
    &Aura::HandleModRegen,                                  // 84 SPELL_AURA_MOD_REGEN
    &Aura::HandleModPowerRegen,                             // 85 SPELL_AURA_MOD_POWER_REGEN
    &Aura::HandleChannelDeathItem,                          // 86 SPELL_AURA_CHANNEL_DEATH_ITEM
    &Aura::HandleNoImmediateEffect,                         // 87 SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &Aura::HandleNoImmediateEffect,                         // 88 SPELL_AURA_MOD_HEALTH_REGEN_PERCENT
    &Aura::HandlePeriodicDamagePCT,                         // 89 SPELL_AURA_PERIODIC_DAMAGE_PERCENT
    &Aura::HandleNULL,                                      // 90 SPELL_AURA_MOD_RESIST_CHANCE  Useless
    &Aura::HandleNoImmediateEffect,                         // 91 SPELL_AURA_MOD_DETECT_RANGE
    &Aura::HandleNULL,                                      // 92 SPELL_AURA_PREVENTS_FLEEING
    &Aura::HandleModUnattackable,                           // 93 SPELL_AURA_MOD_UNATTACKABLE
    &Aura::HandleNoImmediateEffect,                         // 94 SPELL_AURA_INTERRUPT_REGEN implemented in Player::RegenerateAll
    &Aura::HandleAuraGhost,                                 // 95 SPELL_AURA_GHOST
    &Aura::HandleNoImmediateEffect,                         // 96 SPELL_AURA_SPELL_MAGNET implemented in Spell::SelectMagnetTarget
    &Aura::HandleNoImmediateEffect,                         // 97 SPELL_AURA_MANA_SHIELD implemented in Unit::CalcAbsorbResist
    &Aura::HandleAuraModSkill,                              // 98 SPELL_AURA_MOD_SKILL_TALENT
    &Aura::HandleAuraModAttackPower,                        // 99 SPELL_AURA_MOD_ATTACK_POWER
    &Aura::HandleNULL,                                      //100 SPELL_AURA_AURAS_VISIBLE
    &Aura::HandleModResistancePercent,                      //101 SPELL_AURA_MOD_RESISTANCE_PCT
    &Aura::HandleNoImmediateEffect,                         //102 SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonus
    &Aura::HandleAuraModTotalThreat,                        //103 SPELL_AURA_MOD_TOTAL_THREAT
    &Aura::HandleAuraWaterWalk,                             //104 SPELL_AURA_WATER_WALK
    &Aura::HandleAuraFeatherFall,                           //105 SPELL_AURA_FEATHER_FALL
    &Aura::HandleAuraHover,                                 //106 SPELL_AURA_HOVER
    &Aura::HandleAddModifier,                               //107 SPELL_AURA_ADD_FLAT_MODIFIER
    &Aura::HandleAddModifier,                               //108 SPELL_AURA_ADD_PCT_MODIFIER
    &Aura::HandleNoImmediateEffect,                         //109 SPELL_AURA_ADD_TARGET_TRIGGER
    &Aura::HandleNoImmediateEffect,                         //110 SPELL_AURA_MOD_POWER_REGEN_PERCENT
    &Aura::HandleNULL,                                      //111 SPELL_AURA_ADD_CASTER_HIT_TRIGGER
    &Aura::HandleNoImmediateEffect,                         //112 SPELL_AURA_OVERRIDE_CLASS_SCRIPTS
    &Aura::HandleNoImmediateEffect,                         //113 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus
    &Aura::HandleNoImmediateEffect,                         //114 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonus
    &Aura::HandleAuraHealing,                               //115 SPELL_AURA_MOD_HEALING
    &Aura::HandleNoImmediateEffect,                         //116 SPELL_AURA_MOD_REGEN_DURING_COMBAT
    &Aura::HandleNoImmediateEffect,                         //117 SPELL_AURA_MOD_MECHANIC_RESISTANCE
    &Aura::HandleAuraHealingPct,                            //118 SPELL_AURA_MOD_HEALING_PCT
    &Aura::HandleNULL,                                      //119 SPELL_AURA_SHARE_PET_TRACKING useless
    &Aura::HandleAuraUntrackable,                           //120 SPELL_AURA_UNTRACKABLE
    &Aura::HandleAuraEmpathy,                               //121 SPELL_AURA_EMPATHY
    &Aura::HandleModOffhandDamagePercent,                   //122 SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT
    &Aura::HandleModTargetResistance,                       //123 SPELL_AURA_MOD_TARGET_RESISTANCE
    &Aura::HandleAuraModRangedAttackPower,                  //124 SPELL_AURA_MOD_RANGED_ATTACK_POWER
    &Aura::HandleNoImmediateEffect,                         //125 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus
    &Aura::HandleNoImmediateEffect,                         //126 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonus
    &Aura::HandleNoImmediateEffect,                         //127 SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonus
    &Aura::HandleModPossessPet,                             //128 SPELL_AURA_MOD_POSSESS_PET
    &Aura::HandleAuraModIncreaseSpeedAlways,                //129 SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS
    &Aura::HandleAuraModIncreaseMountedSpeedAlways,         //130 SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS
    &Aura::HandleNoImmediateEffect,                         //131 SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonus
    &Aura::HandleAuraModIncreaseEnergyPercent,              //132 SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT
    &Aura::HandleAuraModIncreaseHealthPercent,              //133 SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT
    &Aura::HandleAuraModRegenInterrupt,                     //134 SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
    &Aura::HandleModHealingDone,                            //135 SPELL_AURA_MOD_HEALING_DONE
    &Aura::HandleAuraHealingPct,                            //136 SPELL_AURA_MOD_HEALING_DONE_PERCENT   implemented in Unit::SpellHealingBonus
    &Aura::HandleModTotalPercentStat,                       //137 SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE
    &Aura::HandleHaste,                                     //138 SPELL_AURA_MOD_HASTE
    &Aura::HandleForceReaction,                             //139 SPELL_AURA_FORCE_REACTION
    &Aura::HandleAuraModRangedHaste,                        //140 SPELL_AURA_MOD_RANGED_HASTE
    &Aura::HandleRangedAmmoHaste,                           //141 SPELL_AURA_MOD_RANGED_AMMO_HASTE
    &Aura::HandleAuraModBaseResistancePCT,                  //142 SPELL_AURA_MOD_BASE_RESISTANCE_PCT
    &Aura::HandleAuraModResistanceExclusive,                //143 SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE
    &Aura::HandleNoImmediateEffect,                         //144 SPELL_AURA_SAFE_FALL                  implemented in WorldSession::HandleMovementOpcodes
    &Aura::HandleNULL,                                      //145 SPELL_AURA_CHARISMA
    &Aura::HandleNULL,                                      //146 SPELL_AURA_PERSUADED
    &Aura::HandleNULL,                                      //147 SPELL_AURA_ADD_CREATURE_IMMUNITY
    &Aura::HandleAuraRetainComboPoints,                     //148 SPELL_AURA_RETAIN_COMBO_POINTS
    &Aura::HandleNoImmediateEffect,                         //149 SPELL_AURA_RESIST_PUSHBACK
    &Aura::HandleShieldBlockValue,                          //150 SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT
    &Aura::HandleAuraTrackStealthed,                        //151 SPELL_AURA_TRACK_STEALTHED
    &Aura::HandleNULL,                                      //152 SPELL_AURA_MOD_DETECTED_RANGE
    &Aura::HandleNoImmediateEffect,                         //153 SPELL_AURA_SPLIT_DAMAGE_FLAT
    &Aura::HandleNoImmediateEffect,                         //154 SPELL_AURA_MOD_STEALTH_LEVEL
    &Aura::HandleNoImmediateEffect,                         //155 SPELL_AURA_MOD_WATER_BREATHING
    &Aura::HandleNoImmediateEffect,                         //156 SPELL_AURA_MOD_REPUTATION_GAIN
    &Aura::HandleNULL,                                      //157 SPELL_AURA_PET_DAMAGE_MULTI
    &Aura::HandleShieldBlockValue,                          //158 SPELL_AURA_MOD_SHIELD_BLOCKVALUE
    &Aura::HandleNULL,                                      //159 SPELL_AURA_NO_PVP_CREDIT      only for Honorless Target spell
    &Aura::HandleNULL,                                      //160 SPELL_AURA_MOD_AOE_AVOIDANCE
    &Aura::HandleNoImmediateEffect,                         //161 SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT
    &Aura::HandleNULL,                                      //162 SPELL_AURA_POWER_BURN_MANA
    &Aura::HandleNULL,                                      //163 SPELL_AURA_MOD_CRIT_DAMAGE_BONUS_MELEE
    &Aura::HandleNULL,                                      //164
    &Aura::HandleNoImmediateEffect,                         //165 SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonus
    &Aura::HandleAuraModAttackPowerPercent,                 //166 SPELL_AURA_MOD_ATTACK_POWER_PCT
    &Aura::HandleAuraModRangedAttackPowerPercent,           //167 SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT
    &Aura::HandleNULL,                                      //168 SPELL_AURA_MOD_DAMAGE_DONE_VERSUS
    &Aura::HandleNULL,                                      //169 SPELL_AURA_MOD_CRIT_PERCENT_VERSUS
    &Aura::HandleNULL,                                      //170 SPELL_AURA_DETECT_AMORE       only for Detect Amore spell
    &Aura::HandleNULL,                                      //171 SPELL_AURA_MOD_PARTY_SPEED    unused
    &Aura::HandleAuraModIncreaseMountedSpeed,               //172 SPELL_AURA_MOD_PARTY_SPEED_MOUNTED
    &Aura::HandleNULL,                                      //173 SPELL_AURA_ALLOW_CHAMPION_SPELLS  only for Proclaim Champion spell
    &Aura::HandleModSpellDamagePercentFromStat,             //174 SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT  implemented in Unit::SpellBaseDamageBonus (by defeult intelect, dependent from SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT)
    &Aura::HandleModSpellHealingPercentFromStat,            //175 SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT implemented in Unit::SpellBaseHealingBonus
    &Aura::HandleSpiritOfRedemption,                        //176 SPELL_AURA_SPIRIT_OF_REDEMPTION   only for Spirit of Redemption spell, die at aura end
    &Aura::HandleNULL,                                      //177 SPELL_AURA_AOE_CHARM
    &Aura::HandleNULL,                                      //178 SPELL_AURA_MOD_DEBUFF_RESISTANCE
    &Aura::HandleNoImmediateEffect,                         //179 SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE implemented in Unit::SpellCriticalBonus
    &Aura::HandleNULL,                                      //180 SPELL_AURA_MOD_SPELL_DAMAGE_VS_UNDEAD,
    &Aura::HandleNULL,                                      //181 unused
    &Aura::HandleAuraModResistenceOfIntellectPercent,       //182 SPELL_AURA_MOD_RESISTANCE_OF_INTELLECT_PERCENT
    &Aura::HandleNULL,                                      //183 SPELL_AURA_MOD_CRITICAL_THREAT
    &Aura::HandleNoImmediateEffect,                         //184 SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE  implemented in Unit::RollMeleeOutcomeAgainst
    &Aura::HandleNoImmediateEffect,                         //185 SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE implemented in Unit::RollMeleeOutcomeAgainst
    &Aura::HandleNoImmediateEffect,                         //186 SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE  implemented in Unit::SpellMissChanceCalc
    &Aura::HandleNoImmediateEffect,                         //187 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE  implemended in Unit::GetUnitCriticalChance
    &Aura::HandleNoImmediateEffect,                         //188 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE implemented in Unit::GetUnitCriticalChance
    &Aura::HandleModRating,                                 //189 SPELL_AURA_MOD_RATING
    &Aura::HandleNULL,                                      //190 SPELL_AURA_MOD_FACTION_REPUTATION_GAIN
    &Aura::HandleNULL,                                      //191 SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED
    &Aura::HandleNULL,                                      //192 SPELL_AURA_HASTE_MELEE
    &Aura::HandleModCombatSpeedPct,                         //193 SPELL_AURA_MELEE_SLOW (in fact combat (any type attack) speed pct)
    &Aura::HandleNULL,                                      //194 SPELL_AURA_MOD_DEPRICATED_1 not used now (old SPELL_AURA_MOD_SPELL_DAMAGE_OF_INTELLECT)
    &Aura::HandleNULL,                                      //195 SPELL_AURA_MOD_DEPRICATED_1 not used now (old SPELL_AURA_MOD_SPELL_HEALING_OF_INTELLECT)
    &Aura::HandleNULL,                                      //196
    &Aura::HandleNoImmediateEffect,                         //197 SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE implemented in Unit::SpellCriticalBonus Unit::GetUnitCriticalChance
    &Aura::HandleNULL,                                      //198 SPELL_AURA_MOD_ALL_WEAPON_SKILLS
    &Aura::HandleNULL,                                      //199 SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT
    &Aura::HandleNoImmediateEffect,                         //200 SPELL_AURA_MOD_XP_PCT implemented in Player::GiveXP
    &Aura::HandleAuraAllowFlight,                           //201 SPELL_AURA_FLY                        this aura probably must enable flight mode...
    &Aura::HandleNULL,                                      //202 SPELL_AURA_CANNOT_BE_DODGED
    &Aura::HandleNoImmediateEffect,                         //203 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE  implemented in Unit::DoAttackDamage
    &Aura::HandleNoImmediateEffect,                         //204 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE implemented in Unit::DoAttackDamage
    &Aura::HandleNULL,                                      //205
    &Aura::HandleNULL,                                      //206 SPELL_AURA_MOD_SPEED_MOUNTED
    &Aura::HandleAuraModSpeedMountedFlight,                 //207 SPELL_AURA_MOD_SPEED_MOUNTED_FLIGHT
    &Aura::HandleAuraModSpeedFlight,                        //208 SPELL_AURA_MOD_SPEED_FLIGHT, used only in spell: Flight Form (Passive)
    &Aura::HandleAuraModSpeedFlightAlways,                  //209 SPELL_AURA_MOD_SPEED_FLIGHT_ALWAYS
    &Aura::HandleNULL,                                      //210
    &Aura::HandleNULL,                                      //211
    &Aura::HandleAuraModRangedAttackPowerOfStatPercent,     //212 SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT
    &Aura::HandleNoImmediateEffect,                         //213 SPELL_AURA_MOD_RAGE_FROM_DAMAGE_DEALT implemented in Player::RewardRage
    &Aura::HandleNULL,                                      //214
    &Aura::HandleNULL,                                      //215
    &Aura::HandleModCastingSpeed,                           //216 SPELL_AURA_HASTE_SPELLS
    &Aura::HandleNULL,                                      //217                                   unused
    &Aura::HandleNULL,                                      //218 ranged attack haste?
    &Aura::HandleModManaRegen,                              //219 SPELL_AURA_MOD_MANA_REGEN
    &Aura::HandleNoImmediateEffect,                         //220 SPELL_AURA_MOD_SPELL_HEALING_OF_STRENGTH implemented in Unit::SpellHealingBonus
    &Aura::HandleNULL,                                      //221 ignored
    &Aura::HandleNULL,                                      //222 unused
    &Aura::HandleNULL,                                      //223 unused
    &Aura::HandleNULL,                                      //224 unused
    &Aura::HandleNULL,                                      //225
    &Aura::HandleNoImmediateEffect,                         //226 SPELL_AURA_DUMMY_2 dummy like aura, but at this moment only no immediate effect cases
    &Aura::HandleNULL,                                      //227
    &Aura::HandleNULL,                                      //228 detection
    &Aura::HandleNULL,                                      //228 avoidance
    &Aura::HandleAuraModIncreaseHealth,                     //230 Commanding Shout
    &Aura::HandleNULL,                                      //231
    &Aura::HandleNULL,                                      //232
    &Aura::HandleNULL,                                      //233
    &Aura::HandleNoImmediateEffect,                         //234 SPELL_AURA_MECHANIC_DURATION_MOD imlement in Unit::CalculateSpellDamageAndDuration
    &Aura::HandleNULL,                                      //235
    &Aura::HandleNULL,                                      //236
    &Aura::HandleModSpellDamagePercentFromAttackPower,      //237 SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER  implemented in Unit::SpellBaseDamageBonus
    &Aura::HandleModSpellHealingPercentFromAttackPower,     //238 SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER implemented in Unit::SpellBaseHealingBonus
    &Aura::HandleAuraModScale,                              //239 SPELL_AURA_MOD_SCALE_2 only in Noggenfogger Elixir (16595) before 2.3.0 aura 61
    &Aura::HandleAuraModExpertise,                          //240 SPELL_AURA_MOD_EXPERTISE
    &Aura::HandleNULL,                                      //241
    &Aura::HandleNULL,                                      //242
    &Aura::HandleNULL,                                      //243
    &Aura::HandleNULL,                                      //244
    &Aura::HandleNULL,                                      //245
    &Aura::HandleNULL,                                      //246
    &Aura::HandleNULL,                                      //247
    &Aura::HandleNULL                                       //248
};

Aura::Aura(SpellEntry const* spellproto, uint32 eff, int32 *currentBasePoints, Unit *target, Unit *caster, Item* castItem) :
m_procCharges(0), m_spellmod(NULL), m_spellId(spellproto->Id), m_effIndex(eff), m_caster_guid(0), m_target(target),
m_timeCla(1000), m_castItemGuid(castItem?castItem->GetGUID():0), m_auraSlot(MAX_AURAS),
m_positive(false), m_permanent(false), m_isPeriodic(false), m_isTrigger(false), m_isAreaAura(false), m_isPersistent(false),
m_periodicTimer(0), m_PeriodicEventId(0), m_updated(false), m_removeOnDeath(false),m_fearMoveAngle(0), m_isRemovedOnShapeLost(true)
{
    assert(target);

    assert(spellproto && spellproto == sSpellStore.LookupEntry( spellproto->Id ) && "`info` must be pointer to sSpellStore element");

    m_spellProto = spellproto;

    m_currentBasePoints = currentBasePoints ? *currentBasePoints : m_spellProto->EffectBasePoints[eff];

    m_isPassive = IsPassiveSpell(m_spellId);
    m_positive = IsPositiveEffect(m_spellId, m_effIndex);

    m_applyTime = time(NULL);

    uint32 type = 0;
    if(!m_positive)
        type = 1;

    int32 damage;
    if(!caster)
    {
        m_caster_guid = target->GetGUID();
        damage = m_currentBasePoints+1;                     // stored value-1
        m_maxduration = target->CalculateSpellDuration(m_spellProto);
    }
    else
    {
        m_caster_guid = caster->GetGUID();

        damage        = caster->CalculateSpellDamage(m_spellProto,m_effIndex,m_currentBasePoints,target);
        m_maxduration = caster->CalculateSpellDuration(m_spellProto);

        if (!damage && castItem && castItem->GetItemSuffixFactor())
        {
            ItemRandomSuffixEntry const *item_rand_suffix = sItemRandomSuffixStore.LookupEntry(abs(castItem->GetItemRandomPropertyId()));
            if(item_rand_suffix)
            {
                for (int k=0; k<3; k++)
                {
                    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(item_rand_suffix->enchant_id[k]);
                    if(pEnchant)
                    {
                        for (int t=0; t<3; t++)
                            if(pEnchant->spellid[t] == m_spellProto->Id)
                        {
                            damage = uint32((item_rand_suffix->prefix[k]*castItem->GetItemSuffixFactor()) / 10000 );
                            break;
                        }
                    }

                    if(damage)
                        break;
                }
            }
        }
    }

    if(m_maxduration == -1 || m_isPassive && m_spellProto->DurationIndex == 0)
        m_permanent = true;

    Player* modOwner = caster ? caster->GetSpellModOwner() : NULL;

    if(!m_permanent && modOwner)
        modOwner->ApplySpellMod(m_spellId, SPELLMOD_DURATION, m_maxduration);

    m_duration = m_maxduration;

    if(modOwner)
        modOwner->ApplySpellMod(m_spellId, SPELLMOD_ACTIVATION_TIME, m_periodicTimer);

    sLog.outDebug("Aura: construct Spellid : %u, Aura : %u Duration : %d Target : %d Damage : %d", m_spellProto->Id, m_spellProto->EffectApplyAuraName[eff], m_maxduration, m_spellProto->EffectImplicitTargetA[eff],damage);

    m_effIndex = eff;
    SetModifier(AuraType(m_spellProto->EffectApplyAuraName[eff]), damage, m_spellProto->EffectAmplitude[eff], m_spellProto->EffectMiscValue[eff], type);

    //(spellproto->AttributesEx3 & 0x100000) all death persistent spells have this flag
    m_isDeathPersist = (m_spellProto->AttributesEx3 & 0x100000) != 0;

    if(m_spellProto->procCharges)
    {
        m_procCharges = m_spellProto->procCharges;

        if(modOwner)
            modOwner->ApplySpellMod(m_spellId, SPELLMOD_CHARGES, m_procCharges);
    }
    else
        m_procCharges = -1;

    m_isRemovedOnShapeLost = (m_caster_guid==m_target->GetGUID() && m_spellProto->Stances &&
                            !(m_spellProto->AttributesEx2 & 0x80000) && !(m_spellProto->Attributes & 0x10000));
}

Aura::~Aura()
{
}

AreaAura::AreaAura(SpellEntry const* spellproto, uint32 eff, int32 *currentBasePoints, Unit *target,
Unit *caster, Item* castItem) : Aura(spellproto, eff, currentBasePoints, target, caster, castItem)
{
    m_isAreaAura = true;

    // caster==NULL in constructor args if target==caster in fact
    Unit* caster_ptr = caster ? caster : target;

    m_radius = GetRadius(sSpellRadiusStore.LookupEntry(GetSpellProto()->EffectRadiusIndex[m_effIndex]));
    if(Player* modOwner = caster_ptr->GetSpellModOwner())
        modOwner->ApplySpellMod(GetSpellProto()->Id, SPELLMOD_RADIUS, m_radius);
}

AreaAura::~AreaAura()
{
}

PersistentAreaAura::PersistentAreaAura(SpellEntry const* spellproto, uint32 eff, int32 *currentBasePoints, Unit *target,
Unit *caster, Item* castItem) : Aura(spellproto, eff, currentBasePoints, target, caster, castItem)
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

    //return ObjectAccessor::GetUnit(*m_target,m_caster_guid);
    //must return caster even if it's in another grid/map
    return ObjectAccessor::GetObjectInWorld(m_caster_guid, (Unit*)NULL);
}

void Aura::SetModifier(AuraType t, int32 a, uint32 pt, int32 miscValue, uint32 miscValue2)
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
            int q = rand() % 80;
            if(q == 8) m_fearMoveAngle += (float)(urand(45, 90));
            else if(q == 23) m_fearMoveAngle -= (float)(urand(45, 90));

            // If the m_target is player,and if the speed is too slow,change it :P
            float speed = m_target->GetSpeed(MOVE_RUN);
            // Speed modifier, may need to find correct one
            float mod = m_target->GetTypeId() != TYPEID_PLAYER ? 10 : 6;
            float pos_x = m_target->GetPositionX();
            float pos_y = m_target->GetPositionY();
            float pos_z = m_target->GetPositionZ();
            uint32 mapid = m_target->GetMapId();
            // Control the max Distance; 28 for temp.
            if(m_target->IsWithinDistInMap(caster, 28))
            {
                float x = m_target->GetPositionX() - (speed*cosf(m_fearMoveAngle))/mod;
                float y = m_target->GetPositionY() - (speed*sinf(m_fearMoveAngle))/mod;
                float z = pos_z;
                m_target->UpdateGroundPositionZ(x,y,z);

                // Control the target to not climb or drop when dz > |x|,x = 1.3 for temp.
                // fixed me if it needs checking when the position will be in water?
                                                            //+vmaps
                if((z<=pos_z+1.3 && z>=pos_z-1.3) && m_target->IsWithinLOS(x,y,z))
                {
                    m_target->SendMonsterMove(x,y,z,0,true,(diff*2));
                    if(m_target->GetTypeId() != TYPEID_PLAYER)
                        MapManager::Instance().GetMap(m_target->GetMapId(), m_target)->CreatureRelocation((Creature*)m_target,x,y,z,m_target->GetOrientation());
                }
                else
                {
                    //Complete the move only if z coord is now correct
                    m_fearMoveAngle += 120;
                    x = m_target->GetPositionX() + (speed*sinf(m_fearMoveAngle))/mod;
                    y = m_target->GetPositionY() + (speed*cosf(m_fearMoveAngle))/mod;
                    float z = pos_z;
                    m_target->UpdateGroundPositionZ(x,y,z);

                    if((z<=pos_z+1.3 && z>=pos_z-1.3) && m_target->IsWithinLOS(x,y,z))
                    {
                        m_target->SendMonsterMove(x,y,z,0,true,(diff*2));
                        if(m_target->GetTypeId() != TYPEID_PLAYER)
                            MapManager::Instance().GetMap(m_target->GetMapId(), m_target)->CreatureRelocation((Creature*)m_target,x,y,z,m_target->GetOrientation());
                    }
                }
            }
        }
    }

    if(m_isPeriodic && (m_duration >= 0 || m_isPassive || m_permanent))
    {
        m_periodicTimer -= diff;
        if(m_periodicTimer <= 0)                            // tick also at m_periodicTimer==0 to prevent lost last tick in case max m_duration == (max m_periodicTimer)*N
        {
            if( m_modifier.m_auraname == SPELL_AURA_MOD_REGEN ||
                m_modifier.m_auraname == SPELL_AURA_MOD_POWER_REGEN ||
                                                            // Cannibalize, eating items and other spells
                m_modifier.m_auraname == SPELL_AURA_OBS_MOD_HEALTH ||
                                                            // Eating items and other spells
                m_modifier.m_auraname == SPELL_AURA_OBS_MOD_MANA ||
                m_modifier.m_auraname == SPELL_AURA_MOD_MANA_REGEN )
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
                    caster->PeriodicAuraLog(m_target, GetSpellProto(), &m_modifier,GetEffIndex(),m_castItemGuid);
                else
                    m_target->PeriodicAuraLog(m_target, GetSpellProto(), &m_modifier,GetEffIndex(),m_castItemGuid);
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
            pGroup = ((Player*)caster)->GetGroup();
        else if(caster->GetCharmerOrOwnerGUID() != 0)
        {
            Unit *owner = caster->GetCharmerOrOwner();
            if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                pGroup = ((Player*)owner)->GetGroup();
        }

        if(pGroup)
        {
            for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* Target = itr->getSource();
                if(!Target)
                    continue;

                if (caster->GetTypeId() == TYPEID_PLAYER)
                {
                    if(Target->GetGUID() == m_caster_guid || !Target->isAlive() || !pGroup->SameSubGroup((Player*)caster, Target))
                        continue;
                }
                else if(caster->GetCharmerOrOwnerGUID() != 0)
                {
                    Unit *owner = caster->GetCharmerOrOwner();
                    if(!Target->isAlive() || (owner->GetTypeId() == TYPEID_PLAYER && !pGroup->SameSubGroup((Player*)owner, Target)))
                        continue;
                }

                Aura *t_aura = Target->GetAura(m_spellId, m_effIndex);

                if(caster->IsWithinDistInMap(Target, m_radius) )
                {
                    // apply aura to players in range that dont have it yet
                    if (!t_aura)
                    {
                        // if rank not found 
                        if(SpellEntry const *actualSpellInfo = spellmgr.SelectAuraRankForPlayerLevel(GetSpellProto(),Target->getLevel()))
                        {
                            int32 actualBasePoints = m_currentBasePoints;

                            // recalculate basepoints for lower rank (all AreaAura spell not use custom basepoints?)
                            if(actualSpellInfo != GetSpellProto())
                                actualBasePoints = actualSpellInfo->EffectBasePoints[m_effIndex];

                            AreaAura *aur = new AreaAura(actualSpellInfo, m_effIndex, &actualBasePoints, Target, caster);
                            Target->AddAura(aur);
                        }
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
        else if (caster->GetCharmerOrOwnerGUID() != 0)
        {
            // add / remove auras from the totem's owner
            Unit *owner = caster->GetCharmerOrOwner();
            if (owner)
            {
                Aura *o_aura = owner->GetAura(m_spellId, m_effIndex);
                if(caster->IsWithinDistInMap(owner, m_radius))
                {
                    if (!o_aura)
                    {
                        AreaAura *aur = new AreaAura(GetSpellProto(), m_effIndex, &m_currentBasePoints, owner, caster);
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

    if(m_caster_guid != m_target->GetGUID())                // aura at non-caster
    {
        Unit * tmp_target = m_target;
        Unit* caster = GetCaster();
        uint32 tmp_spellId = m_spellId, tmp_effIndex = m_effIndex;

        // WARNING: the aura may get deleted during the update
        // DO NOT access its members after update!
        Aura::Update(diff);

        // remove aura if out-of-range from caster (after teleport for example)
        if(!caster || !caster->IsWithinDistInMap(tmp_target, m_radius) )
            tmp_target->RemoveAura(tmp_spellId, tmp_effIndex);
    }
    else Aura::Update(diff);
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
            if (!m_target->IsWithinDistInMap(dynObj, dynObj->GetRadius()))
                remove = true;
        }
        else
            remove = true;
    }
    else
        remove = true;

    Unit *tmp_target = m_target;
    uint32 tmp_id = GetId(), tmp_index = GetEffIndex();

    // WARNING: the aura may get deleted during the update
    // DO NOT access its members after update!
    Aura::Update(diff);

    if(remove)
        tmp_target->RemoveAura(tmp_id, tmp_index);
}

void Aura::ApplyModifier(bool apply, bool Real)
{
    AuraType aura = m_modifier.m_auraname;

    if(aura<TOTAL_AURAS)
        (*this.*AuraHandler [aura])(apply,Real);
}

void Aura::UpdateAuraDuration()
{
    if(m_auraSlot >= MAX_AURAS || m_isPassive)
        return;

    if( m_target->GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data(SMSG_UPDATE_AURA_DURATION, 5);
        data << (uint8)m_auraSlot << (uint32)m_duration;
        ((Player*)m_target)->SendDirectMessage(&data);
    }

    // not send in case player loading (will not work anyway until player not added to map), sent in visibility change code
    if(m_target->GetTypeId() == TYPEID_PLAYER && ((Player*)m_target)->GetSession()->PlayerLoading())
        return;

    Unit* caster = GetCaster();

    if(caster && caster->GetTypeId() == TYPEID_PLAYER && caster != m_target)
        SendAuraDurationForCaster((Player*)caster);
}

void Aura::SendAuraDurationForCaster(Player* caster)
{
    WorldPacket data(SMSG_SET_AURA_SINGLE, (8+2+4+4+4));
    data.append(m_target->GetPackGUID());
    data << uint8(m_auraSlot);
    data << uint32(GetSpellProto()->Id);
    data << uint32(GetAuraMaxDuration());                   // full
    data << uint32(GetAuraDuration());                      // remain
    caster->GetSession()->SendPacket(&data);
}

void Aura::_AddAura()
{
    if (!m_spellId)
        return;
    if(!m_target)
        return;

    bool samespell = false;
    uint8 slot = 0xFF;

    for(uint8 i = 0; i < 3; i++)
    {
        Aura* aura = m_target->GetAura(m_spellId, i);
        if(aura)
        {
            samespell = true;
            slot = aura->GetAuraSlot();
            break;
        }
    }

    //if(m_spellId == 23333 || m_spellId == 23335)            // for BG
    //    m_positive = true;

    // not call total regen auras at adding
    if( m_modifier.m_auraname==SPELL_AURA_OBS_MOD_HEALTH || m_modifier.m_auraname==SPELL_AURA_OBS_MOD_MANA )
        m_periodicTimer = m_modifier.periodictime;
    else
    if( m_modifier.m_auraname==SPELL_AURA_MOD_REGEN       ||
        m_modifier.m_auraname==SPELL_AURA_MOD_POWER_REGEN ||
        m_modifier.m_auraname == SPELL_AURA_MOD_MANA_REGEN )
        m_periodicTimer = 5000;

    ApplyModifier(true,true);

    sLog.outDebug("Aura %u now is in use", m_modifier.m_auraname);

    Unit* caster = GetCaster();

    // passive auras (except totem auras) do not get placed in the slots
    if(!m_isPassive || (caster && caster->GetTypeId() == TYPEID_UNIT && ((Creature*)caster)->isTotem()))
    {
        if(!samespell)
        {
            if (IsPositive())
            {
                for (uint8 i = 0; i < MAX_POSITIVE_AURAS; i++)
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
                for (uint8 i = MAX_POSITIVE_AURAS; i < MAX_AURAS; i++)
                {
                    if (m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURA + i)) == 0)
                    {
                        slot = i;
                        break;
                    }
                }
            }

            if(slot < MAX_AURAS)
            {
                m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURA + slot), GetId());

                uint8 flagslot = slot >> 2;
                uint32 value = m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot));

                uint8 value1 = (slot & 3) << 3;

                value &= ~((uint32)AFLAG_MASK << value1);
                if (IsPositive())
                    value |= ((uint32)AFLAG_POSITIVE << value1);
                else
                    value |= ((uint32)AFLAG_NEGATIVE << value1);
                m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot), value);
            }

            SetAuraSlot( slot );
            UpdateAuraDuration();
        }
        else
        {
            SetAuraSlot( slot );
            UpdateSlotCounterAndDuration(true);
        }

        // Update Seals information
        if( IsSealSpell(GetId()) )
            m_target->ModifyAuraState(AURA_STATE_JUDGEMENT,true);

        // Conflagrate aura state
        if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_WARLOCK && (GetSpellProto()->SpellFamilyFlags & 4))
            m_target->ModifyAuraState(AURA_STATE_IMMOLATE,true);

        if(GetSpellProto()->SpellFamilyName == SPELLFAMILY_DRUID
            && (GetSpellProto()->SpellFamilyFlags == 0x40 || GetSpellProto()->SpellFamilyFlags == 0x10))
        {
            m_target->ModifyAuraState(AURA_STATE_SWIFTMEND, true);
        }
    }
}

void Aura::_RemoveAura()
{
    // Remove all triggered by aura spells vs unlimited duration
    CleanupTriggeredSpells();

    sLog.outDebug("Aura %u now is remove", m_modifier.m_auraname);
    ApplyModifier(false,true);

    Unit* caster = GetCaster();

    if(caster && IsPersistent())
    {
        DynamicObject *dynObj = caster->GetDynObject(GetId(), GetEffIndex());
        if (dynObj)
            dynObj->RemoveAffected(m_target);
    }

    //passive auras do not get put in slots
    // Note: but totem can be not accessible for aura target in time remove (to far for find in grid)
    //if(m_isPassive && !(caster && caster->GetTypeId() == TYPEID_UNIT && ((Creature*)caster)->isTotem()))
    //    return;

    uint8 slot = GetAuraSlot();

    // Aura added always to m_target
    //Aura* aura = m_target->GetAura(m_spellId, m_effIndex);
    //if(!aura)
    //    return;

    if(slot >= MAX_AURAS)                                   // slot not set
        return;

    if(m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURA + slot)) == 0)
        return;

    bool samespell = false;

    for(uint8 i = 0; i < 3; i++)
    {
        Aura* aura = m_target->GetAura(m_spellId, i);
        if(aura)
        {
            samespell = true;
            break;
        }
    }

    // only remove icon when the last aura of the spell is removed (current aura already removed from list)
    if (!samespell)
    {
        m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURA + slot), 0);

        uint8 flagslot = slot >> 2;

        uint32 value = m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot));

        uint8 aurapos = (slot & 3) << 3;
        value &= ~((uint32)AFLAG_MASK << aurapos );

        m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot), value);
        if( IsSealSpell(GetId()) )
            m_target->ModifyAuraState(AURA_STATE_JUDGEMENT,false);

        // Conflagrate aura state
        if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_WARLOCK && (GetSpellProto()->SpellFamilyFlags & 4))
            m_target->ModifyAuraState(AURA_STATE_IMMOLATE, false);

        // Swiftmend aura state
        if(GetSpellProto()->SpellFamilyName == SPELLFAMILY_DRUID
            && (GetSpellProto()->SpellFamilyFlags == 0x40 || GetSpellProto()->SpellFamilyFlags == 0x10))
        {
            bool found = false;
            Unit::AuraList const& RejorRegr = m_target->GetAurasByType(SPELL_AURA_PERIODIC_HEAL);
            for(Unit::AuraList::const_iterator i = RejorRegr.begin(); i != RejorRegr.end(); ++i)
            {
                if((*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_DRUID
                    && ((*i)->GetSpellProto()->SpellFamilyFlags == 0x40 || (*i)->GetSpellProto()->SpellFamilyFlags == 0x10) )
                {
                    found = true;
                    break;
                }
            }
            if(!found)
                m_target->ModifyAuraState(AURA_STATE_SWIFTMEND, false);
        }

        // reset cooldown state for spells infinity/long aura (it's all self applied (?))
        int32 duration = GetDuration(GetSpellProto());
        if( caster==m_target && ( duration < 0 || uint32(duration) > GetRecoveryTime(GetSpellProto()) ))
            SendCoolDownEvent();
    }
    else                                                    // decrease count for spell
        UpdateSlotCounterAndDuration(false);
}

void Aura::UpdateSlotCounterAndDuration(bool add)
{
    uint8 slot = GetAuraSlot();
    if(slot >= MAX_AURAS)
        return;

    // calculate amount of similar auras by same effect index (similar different spells)
    int8 count = 0;

    // calculate auras and update durations in case aura adding
    Unit::AuraList const& aura_list = m_target->GetAurasByType(GetModifier()->m_auraname);
    for(Unit::AuraList::const_iterator i = aura_list.begin();i != aura_list.end(); ++i)
    {
        if((*i)->m_spellId==m_spellId && (*i)->m_effIndex==m_effIndex)
        {
            ++count;

            if(add)
                (*i)->SetAuraDuration(GetAuraDuration());
        }
    }

    // at aura add aura not added yet, at aura remove aura already removed
    // in field stored (count-1)
    if(!add)
        --count;

    uint32 index = slot / 4;
    uint32 byte  = slot % 4;
    uint32 val   = m_target->GetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+index);

    uint32 byte_bitpos  = byte * 8;
    uint32 byte_mask = 0xFF << (byte * 8);

    val = (val & ~byte_mask) | (count << byte_bitpos);

    m_target->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+index, val);

    UpdateAuraDuration();
}

/*********************************************************/
/***               BASIC AURA FUNCTION                 ***/
/*********************************************************/
void Aura::HandleAddModifier(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER || !Real)
        return;

    SpellEntry const *spellInfo = GetSpellProto();
    if(!spellInfo) return;

    if(m_modifier.m_miscvalue >= MAX_SPELLMOD)
        return;

    if (apply)
    {
        SpellModifier *mod = new SpellModifier;
        mod->op = SpellModOp(m_modifier.m_miscvalue);
        mod->value = m_modifier.m_amount;
        mod->type = SpellModType(m_modifier.m_auraname);    // SpellModType value == spell aura types
        mod->spellId = m_spellId;
        mod->effectId = m_effIndex;
        mod->lastAffected = NULL;

        SpellAffection const *spellAffect = spellmgr.GetSpellAffection(m_spellId, m_effIndex);

        if (spellAffect && spellAffect->SpellFamilyMask)
            mod->mask = spellAffect->SpellFamilyMask;
        else
            mod->mask = spellInfo->EffectItemType[m_effIndex];

        if(spellAffect && spellAffect->Charges)
            mod->charges = spellAffect->Charges;
        else
            mod->charges = spellInfo->procCharges;

        m_spellmod = mod;
    }

    uint64 spellFamilyMask = m_spellmod->mask;

    ((Player*)m_target)->AddSpellMod(m_spellmod, apply);

    // reapply some passive spells after add/remove related spellmods
    if(spellInfo->SpellFamilyName==SPELLFAMILY_WARRIOR && (spellFamilyMask & 0x0000100000000000LL))
    {
        m_target->RemoveAurasDueToSpell(45471);

        if(apply)
            m_target->CastSpell(m_target,45471,true);
    }
}

void Aura::TriggerSpell()
{
    // custom triggering code (triggered spell not set or not exist

    // Frenzied Regeneration
    if (GetSpellProto()->Category == 1011)
    {
        int32 LifePerRage = GetModifier()->m_amount;

        int32 lRage = m_target->GetPower(POWER_RAGE);
        if(lRage > 100)                                     // rage stored as rage*10
            lRage = 100;

        m_target->ModifyPower(POWER_RAGE, -lRage);

        int32 FRTriggerBasePoints = int32(lRage*LifePerRage/10)-1;
        m_target->CastCustomSpell(m_target,22845,&FRTriggerBasePoints,NULL,NULL,true,NULL,this);
        return;
    }

    // generic casting code with custom spells and target/caster customs
    uint32 trigger_spell_id = GetSpellProto()->EffectTriggerSpell[m_effIndex];
    Unit* caster = GetCaster();
    Unit* target = m_target;
    uint64 originalCasterGUID = GetCasterGUID();

    // specific code for cases with no trigger spell provided in field
    switch(GetId())
    {
        // trigger_spell_id not set and unknown effect triggered in this case, ignoring for while
        case   768:                                         // Cat Form (passive)
        case 17947:                                         // Firestone Passive
            return;

        case 66:
        {
            // Invisibility - overwrite non existing triggered spell call in spell.dbc
            if(!m_duration)
                m_target->CastSpell(m_target, 32612, true, NULL, this);
            return;
        }

        case 29528: trigger_spell_id = 28713; break;        // Inoculation

        case 29917: trigger_spell_id = 29916; break;        // Feed Captured Animal
        case  1515:                                         // Tame Beast
        {
            // TODO: currently this used as hack for Tame beast triggered spell,
            // BUT this can be correct way to provide target for ALL this function calls
            // in case m_target==caster (or GetSpellProto()->EffectImplicitTargetA[m_effIndex]==TARGET_SELF )
            target = ObjectAccessor::GetUnit(*m_target, m_target->GetUInt64Value(UNIT_FIELD_TARGET));
            break;
        }
        case 1010:                                          // Curse of Idiocy
        {
            // TODO: spell casted by result in correct way mostly
            // BUT:
            // 1) target show casting at each triggered cast: target don't must show casting animation for any triggered spell
            //      but must show affect apply like item casting
            // 2) maybe aura must be replace by new with accumulative stat mods insteed stacking

            // prevent cast by triggered auras
            if(m_caster_guid == m_target->GetGUID())
                return;

            // stop triggering after each affected stats lost > 90
            int32 intelectLoss = 0;
            int32 spiritLoss = 0;

            Unit::AuraList const& mModStat = m_target->GetAurasByType(SPELL_AURA_MOD_STAT);
            for(Unit::AuraList::const_iterator i = mModStat.begin(); i != mModStat.end(); ++i)
            {
                if ((*i)->GetSpellProto()->Id == 1010)
                {
                    switch((*i)->GetModifier()->m_miscvalue)
                    {
                        case STAT_INTELLECT: intelectLoss += (*i)->GetModifier()->m_amount; break;
                        case STAT_SPIRIT:    spiritLoss   += (*i)->GetModifier()->m_amount; break;
                        default: break;
                    }
                }
            }

            if(intelectLoss <= -90 && spiritLoss <= -90)
                return;

            caster = target;
            originalCasterGUID = 0;
            break;
        }
    }

    if(!trigger_spell_id)
    {
        sLog.outError("Aura::TriggerSpell: Spell %u have 0 in EffectTriggered[%d], not handled custom case?",GetSpellProto()->Id,GetEffIndex());
        return;
    }

    if(!caster || !target)
        return;


    // custom points code
    switch(GetId())
    {
        // Mana Tide
        case 16191:
        {
            int32 MTBasePoints = GetModifier()->m_amount-1;
            caster->CastCustomSpell(target,trigger_spell_id,&MTBasePoints,NULL,NULL,true,NULL,this,originalCasterGUID);
            return;
        }
    }

    SpellEntry const *triggredSpellInfo = sSpellStore.LookupEntry(trigger_spell_id);

    if(!triggredSpellInfo)
    {
        sLog.outError("WORLD: unknown aura triggered spell id %i\n", trigger_spell_id);
        return;
    }

    Spell *spell = new Spell(caster, triggredSpellInfo, true, originalCasterGUID );

    SpellCastTargets targets;
    targets.setUnitTarget( target );

    // if spell create dynamic object extract area from it
    if(DynamicObject* dynObj = caster->GetDynObject(GetId()))
        targets.setDestination(dynObj->GetPositionX(),dynObj->GetPositionY(),dynObj->GetPositionZ());

    spell->prepare(&targets, this);
}

/*********************************************************/
/***                  AURA EFFECTS                     ***/
/*********************************************************/

void Aura::HandleAuraDummy(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    Unit* caster = GetCaster();

    // Mangle (Cat) combo && damage
    if( m_spellProto->SpellFamilyName==SPELLFAMILY_DRUID && m_spellProto->SpellFamilyFlags == 0x40000000000LL &&
        caster && caster->GetTypeId()==TYPEID_PLAYER )
    {
        if(apply)
        {
            // 1 combo
            ((Player*)caster)->AddComboPoints(m_target,1);

            // damage%
            SpellModifier *mod = new SpellModifier;
            mod->op = SPELLMOD_DAMAGE;
            mod->value = m_modifier.m_amount;
            mod->type = SPELLMOD_PCT;
            mod->spellId = m_spellId;
            mod->effectId = m_effIndex;
            mod->lastAffected = NULL;
            mod->mask = 0x00008000 | 0x00001000;
            mod->charges = 0;

            m_spellmod = mod;
        }

        ((Player*)caster)->AddSpellMod(m_spellmod, apply);
    }

    // Improved Aspect of the Viper
    if( GetId()==38390 && m_target->GetTypeId()==TYPEID_PLAYER )
    {
        if(apply)
        {
            // + effect value for Aspect of the Viper
            SpellModifier *mod = new SpellModifier;
            mod->op = SPELLMOD_DAMAGE;
            mod->value = m_modifier.m_amount;
            mod->type = SPELLMOD_FLAT;
            mod->spellId = m_spellId;
            mod->effectId = m_effIndex;
            mod->lastAffected = NULL;
            mod->mask = 0x4000000000000LL;
            mod->charges = 0;

            m_spellmod = mod;
        }

        ((Player*)m_target)->AddSpellMod(m_spellmod, apply);

        // update active aura
        Unit::AuraList const& mDummy2Auras = m_target->GetAurasByType(SPELL_AURA_DUMMY_2);
        for(Unit::AuraList::const_iterator i = mDummy2Auras.begin();i != mDummy2Auras.end(); ++i)
            if((*i)->GetId() == 34074)                      // Aspect of the Viper
                const_cast<int32&>((*i)->GetModifier()->m_amount) += (apply ? m_modifier.m_amount : -m_modifier.m_amount);
    }

    // Tame beast
    if( GetId()==1515 && apply && caster && m_target->CanHaveThreatList())
    {
        // FIX_ME: this is 2.0.12 threat effect relaced in 2.1.x by dummy aura, must be checked for correctness
        m_target->AddThreat(caster, 10.0f);
    }

    if( m_target->GetTypeId() == TYPEID_PLAYER && !apply &&
        ( GetSpellProto()->Effect[0]==72 || GetSpellProto()->Effect[0]==6 &&
        ( GetSpellProto()->EffectApplyAuraName[0]==1 || GetSpellProto()->EffectApplyAuraName[0]==128 ) ) )
    {
        // spells with SpellEffect=72 and aura=4: 6196, 6197, 21171, 21425
        m_target->SetUInt64Value(PLAYER_FARSIGHT, 0);
        WorldPacket data(SMSG_CLEAR_FAR_SIGHT_IMMEDIATE, 0);
        ((Player*)m_target)->GetSession()->SendPacket(&data);
    }

    // net-o-matic
    if (GetId() == 13139 && apply && caster)
    {
        // root to self part of (root_target->charge->root_self sequence
        caster->CastSpell(caster,13138,true,NULL,this);
    }

    // seal of righteousness
    if(GetSpellProto()->SpellVisual == 7986 && caster && caster->GetTypeId() == TYPEID_PLAYER)
    {
        if(GetEffIndex() == 0)
            m_target->ApplyAuraProcTriggerDamage(this,apply);
    }

    if(GetSpellProto()->SpellVisual == 7395 && GetSpellProto()->SpellIconID == 278 && caster->GetTypeId() == TYPEID_PLAYER)
        m_target->ApplyAuraProcTriggerDamage(this,apply);

    // soulstone resurrection (only at real add/remove and can overwrite reincarnation spell setting)
    if(GetSpellProto()->SpellVisual == 99 && GetSpellProto()->SpellIconID == 92 &&
        caster && caster->GetTypeId() == TYPEID_PLAYER && m_target && m_target->GetTypeId() == TYPEID_PLAYER)
    {
        Player * player = (Player*)m_target;

        uint32 spellid = 0;
        switch(GetId())
        {
            case 20707:spellid =  3026;break;
            case 20762:spellid = 20758;break;
            case 20763:spellid = 20759;break;
            case 20764:spellid = 20760;break;
            case 20765:spellid = 20761;break;
            case 27239:spellid = 27240;break;
            default: return;
        }

        if(apply)
        {
            // overwrite any
            player->SetUInt32Value(PLAYER_SELF_RES_SPELL, spellid);
        }
        else
        {
            // remove only own spell
            if(player->GetUInt32Value(PLAYER_SELF_RES_SPELL)==spellid)
                player->SetUInt32Value(PLAYER_SELF_RES_SPELL, 0);
        }
    }

    // Victorious
    if(GetId()==32216 && m_target->getClass()==CLASS_WARRIOR)
        m_target->ModifyAuraState(AURA_STATE_WARRIOR_VICTORY_RUSH, apply);

    if(!apply)
    {
        if( (IsQuestTameSpell(GetId())) && caster && caster->isAlive() && m_target && m_target->isAlive())
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
                case 30646: finalSpelId = 30647; break;
                case 30653: finalSpelId = 30648; break;
                case 30654: finalSpelId = 30652; break;
                case 30099: finalSpelId = 30100; break;
                case 30102: finalSpelId = 30103; break;
                case 30105: finalSpelId = 30104; break;
            }

            if(finalSpelId)
                caster->CastSpell(m_target,finalSpelId,true,NULL,this);
        }
    }
}

void Aura::HandleAuraMounted(bool apply, bool Real)
{
    if(apply)
    {
        CreatureInfo const* ci = objmgr.GetCreatureTemplate(m_modifier.m_miscvalue);
        if(!ci)
        {
            sLog.outErrorDb("AuraMounted: `creature_template`='%u' not found in database (only need it modelid)", m_modifier.m_miscvalue);
            return;
        }

        // drop flag at mount in bg
        if(Real && m_target->GetTypeId()==TYPEID_PLAYER && ((Player*)m_target)->InBattleGround())
            if(BattleGround *bg = ((Player*)m_target)->GetBattleGround())
                bg->HandleDropFlag((Player*)m_target);

        uint32 team = 0;
        if (m_target->GetTypeId()==TYPEID_PLAYER)
            team = ((Player*)m_target)->GetTeam();

        uint32 displayId = (team==HORDE) ? ci->DisplayID_H : ci->DisplayID_A;

        CreatureModelInfo const *minfo = objmgr.GetCreatureModelRandomGender(displayId);
        if(!minfo)
        {
            sLog.outErrorDb("Mount (Entry: %u) has model %u not found in table `creature_model_info`, can't load. ", 
                m_modifier.m_miscvalue, displayId);
            return;
        }
        else
            displayId = minfo->modelid;                     // can change for other gender

        m_target->Mount(displayId);
    }
    else
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
        data.Initialize(SMSG_MOVE_WATER_WALK, 8+4);
    else
        data.Initialize(SMSG_MOVE_LAND_WALK, 8+4);
    data.append(m_target->GetPackGUID());
    data << uint32(0);
    m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraFeatherFall(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_FEATHER_FALL, 8+4);
    else
        data.Initialize(SMSG_MOVE_NORMAL_FALL, 8+4);
    data.append(m_target->GetPackGUID());
    data << (uint32)0;
    m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraHover(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_SET_HOVER, 8+4);
    else
        data.Initialize(SMSG_MOVE_UNSET_HOVER, 8+4);
    data.append(m_target->GetPackGUID());
    data << uint32(0);
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
        case FORM_FLIGHT:
            if(unit_target->getRace() == RACE_NIGHTELF)
                modelid = 20857;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 20872;
            break;
        case FORM_MOONKIN:
            if(unit_target->getRace() == RACE_NIGHTELF)
                modelid = 15374;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 15375;
            break;
        case FORM_SWIFT_FLIGHT:
            if(unit_target->getRace() == RACE_NIGHTELF)
                modelid = 21243;
            else if(unit_target->getRace() == RACE_TAUREN)
                modelid = 21244;
            break;
        case FORM_AMBIENT:
        case FORM_SHADOW:
        case FORM_STEALTH:
            break;
        case FORM_TREE:
            modelid = 864;
            break;
        case FORM_BATTLESTANCE:
        case FORM_BERSERKERSTANCE:
        case FORM_DEFENSIVESTANCE:
            PowerType = POWER_RAGE;
            break;
        case FORM_SPIRITOFREDEMPTION:
            modelid = 16031;
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
            // reset power to default values only at power change
            if(unit_target->getPowerType()!=PowerType)
                unit_target->setPowerType(PowerType);

            switch(m_modifier.m_miscvalue)
            {
                case FORM_CAT:
                case FORM_BEAR:
                case FORM_DIREBEAR:
                {
                    // get furor proc chance
                    uint32 FurorChance = 0;
                    Unit::AuraList const& mDummy = m_target->GetAurasByType(SPELL_AURA_DUMMY);
                    for(Unit::AuraList::const_iterator i = mDummy.begin(); i != mDummy.end(); ++i)
                    {
                        if ((*i)->GetSpellProto()->SpellIconID == 238)
                        {
                            FurorChance = (*i)->GetModifier()->m_amount;
                            break;
                        }
                    }

                    if (m_modifier.m_miscvalue == FORM_CAT)
                    {
                        unit_target->SetPower(POWER_ENERGY,0);
                        if(urand(1,100) <= FurorChance)
                        {
                            unit_target->CastSpell(unit_target,17099,true,NULL,this);
                        }
                    }
                    else
                    {
                        unit_target->SetPower(POWER_RAGE,0);
                        if(urand(1,100) <= FurorChance)
                        {
                            unit_target->CastSpell(unit_target,17057,true,NULL,this);
                        }
                    }
                    break;
                }
                case FORM_BATTLESTANCE:
                case FORM_DEFENSIVESTANCE:
                case FORM_BERSERKERSTANCE:
                {
                    uint32 Rage_val = 0;
                    // Stance mastery + Tactical mastery (both passive, and last have aura only in defence stance, but need apply at any stance switch)
                    if(m_target->GetTypeId() == TYPEID_PLAYER)
                    {
                        PlayerSpellMap const& sp_list = ((Player *)m_target)->GetSpellMap();
                        for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
                        {
                            if(itr->second->state == PLAYERSPELL_REMOVED) continue;
                            SpellEntry const *spellInfo = sSpellStore.LookupEntry(itr->first);
                            if (spellInfo && spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR && spellInfo->SpellIconID == 139)
                                Rage_val += m_target->CalculateSpellDamage(spellInfo,0,spellInfo->EffectBasePoints[0],m_target) * 10;
                        }
                    }

                    if (unit_target->GetPower(POWER_RAGE) > Rage_val)
                        unit_target->SetPower(POWER_RAGE,Rage_val);
                    break;
                }
                default:
                    break;
            }
        }

        unit_target->m_ShapeShiftForm = m_spellId;
        unit_target->m_form = m_modifier.m_miscvalue;
    }
    else
    {
        unit_target->SetUInt32Value(UNIT_FIELD_DISPLAYID,unit_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
        unit_target->RemoveFlag(UNIT_FIELD_BYTES_1, (new_bytes_1<<16) );
        if(unit_target->getClass() == CLASS_DRUID)
            unit_target->setPowerType(POWER_MANA);
        unit_target->m_ShapeShiftForm = 0;
        unit_target->m_form = 0;
    }

    if(player)
        player->InitDataForForm();
}

void Aura::HandleAuraTransform(bool apply, bool Real)
{
    // skip if player not added to map at loading or far teleport (to prevent client crash)
    // it will applied in Player::SendInitialPacketsAfterAddToMap after adding to map
    if(m_target->GetTypeId()==TYPEID_PLAYER && !((Player*)m_target)->IsInWorld())
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
                // BloodElf Female
                case 15475: m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 17830); break;
                // BloodElf Male
                case 15476: m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 17829); break;
                // Dranei Female
                case 16126: m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 17828); break;
                // Dranei Male
                case 16125: m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 17827); break;
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
                                                            // Will use the default model here
                m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, ci->DisplayID_A);
            }
            m_target->setTransForm(GetSpellProto()->Id);
        }
    }
    else
    {
        m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
        m_target->setTransForm(0);
    }

    // for players, start regeneration after 1s (in polymorph fast regeneration case)
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        if (m_target->IsPolymorphed())
            ((Player*)m_target)->setRegenTimer(1000);

    Unit* caster = GetCaster();

    if(caster && caster->GetTypeId() == TYPEID_PLAYER)
        m_target->SendUpdateToPlayer((Player*)caster);
}

void Aura::HandleForceReaction(bool Apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(!Real)
        return;

    Player* player = (Player*)m_target;

    uint32 faction_id = m_modifier.m_miscvalue;
    uint32 faction_rank = m_modifier.m_amount;

    if(Apply)
        player->m_forcedReactions[faction_id] = ReputationRank(faction_rank);
    else
        player->m_forcedReactions.erase(faction_id);

    WorldPacket data;
    data.Initialize(SMSG_SET_FORCED_REACTIONS, 4+player->m_forcedReactions.size()*(4+4));
    data << uint32(player->m_forcedReactions.size());
    for(ForcedReactions::const_iterator itr = player->m_forcedReactions.begin(); itr != player->m_forcedReactions.end(); ++itr)
    {
        data << uint32(itr->first);                         // faction_id (Faction.dbc)
        data << uint32(itr->second);                        // reputation rank
    }
    player->SendDirectMessage(&data);
}

void Aura::HandleAuraModSkill(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 prot=GetSpellProto()->EffectMiscValue[m_effIndex];
    int32 points = GetModifier()->m_amount;

    ((Player*)m_target)->ModifySkillBonus(prot,(apply ? points: -points),m_modifier.m_auraname==SPELL_AURA_MOD_SKILL_TALENT);
    if(prot == SKILL_DEFENSE)
        ((Player*)m_target)->UpdateDefenseBonusesMod();
}

void Aura::HandleChannelDeathItem(bool apply, bool Real)
{
    if(Real && !apply)
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
        {
            Item* newitem = ((Player*)caster)->StoreNewItem(dest, spellInfo->EffectItemType[m_effIndex], 1, true);
            ((Player*)caster)->SendNewItem(newitem, 1, true, false);
        }
        else
            ((Player*)caster)->SendEquipError( msg, NULL, NULL );
    }
}

void Aura::HandleBindSight(bool apply, bool Real)
{
    if(!m_target)
        return;

    Unit* caster = GetCaster();
    if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    caster->SetUInt64Value(PLAYER_FARSIGHT,apply ? m_target->GetGUID() : 0);
}

void Aura::HandleFarSight(bool apply, bool Real)
{
    if(!m_target)
        return;

    Unit* caster = GetCaster();
    if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    caster->SetUInt64Value(PLAYER_FARSIGHT,apply ? m_modifier.m_miscvalue : 0);
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

    m_target->ApplyModFlag(PLAYER_FIELD_BYTES,0x02,apply);
}

void Aura::HandleAuraModScale(bool apply, bool Real)
{
    m_target->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE_X,m_modifier.m_amount,apply);
}

void Aura::HandleModPossess(bool apply, bool Real)
{
    if(!m_target)
        return;
    if(!Real)
        return;

    if(m_target->GetCreatureType() != CREATURE_TYPE_HUMANOID)
        return;

    Unit* caster = GetCaster();
    if(!caster)
        return;

    if(int32(m_target->getLevel()) <= m_modifier.m_amount)
    {
        if( apply )
        {
            m_target->SetCharmerGUID(GetCasterGUID());
            m_target->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,caster->getFaction());
            caster->SetCharm(m_target);

            if(caster->getVictim()==m_target)
                caster->AttackStop();
            m_target->CombatStop();
            m_target->DeleteThreatList();
            if(m_target->GetTypeId() == TYPEID_UNIT)
            {
                m_target->StopMoving();
                m_target->GetMotionMaster()->Clear();
                m_target->GetMotionMaster()->Idle();
                CharmInfo *charmInfo = ((Creature*)m_target)->InitCharmInfo(m_target);
                charmInfo->InitPossessCreateSpells();
            }

            if(caster->GetTypeId() == TYPEID_PLAYER)
            {
                ((Player*)caster)->PossessSpellInitialize();
            }
        }
        else
        {
            //remove area auras from charms
            Unit::AuraMap& tAuras = m_target->GetAuras();
            for (Unit::AuraMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
            {
                if (itr->second && itr->second->IsAreaAura())
                    m_target->RemoveAura(itr);
                else
                    ++itr;
            }

            m_target->SetCharmerGUID(0);

            if(m_target->GetTypeId() == TYPEID_PLAYER)
            {
                ((Player*)m_target)->setFactionForRace(m_target->getRace());
            }
            else if(m_target->GetTypeId() == TYPEID_UNIT)
            {
                CreatureInfo const *cinfo = ((Creature*)m_target)->GetCreatureInfo();
                m_target->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,cinfo->faction_A);
            }

            caster->SetCharm(0);

            if(caster->GetTypeId() == TYPEID_PLAYER)
            {
                WorldPacket data(SMSG_PET_SPELLS, 8);
                data << uint64(0);
                ((Player*)caster)->GetSession()->SendPacket(&data);
            }
            if(m_target->GetTypeId() == TYPEID_UNIT)
            {
                ((Creature*)m_target)->AIM_Initialize();

                if (((Creature*)m_target)->AI())
                    ((Creature*)m_target)->AI()->AttackStart(caster);
            }
        }
        if(caster->GetTypeId() == TYPEID_PLAYER)
            caster->SetUInt64Value(PLAYER_FARSIGHT,apply ? m_target->GetGUID() : 0);
    }
}

void Aura::HandleModPossessPet(bool apply, bool Real)
{
    if(!m_target)
        return;
    if(!Real)
        return;

    Unit* caster = GetCaster();
    if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;
    if(caster->GetPet() != m_target)
        return;

    if(apply)
    {
        caster->SetUInt64Value(PLAYER_FARSIGHT, m_target->GetGUID());
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN5);
    }
    else
    {
        caster->SetUInt64Value(PLAYER_FARSIGHT, 0);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN5);
    }
}

void Aura::HandleModCharm(bool apply, bool Real)
{
    if(!m_target)
        return;
    if(!Real)
        return;

    Unit* caster = GetCaster();
    if(!caster)
        return;

    if(int32(m_target->getLevel()) <= m_modifier.m_amount)
    {
        if( apply )
        {
            m_target->SetCharmerGUID(GetCasterGUID());
            m_target->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,caster->getFaction());

            caster->SetCharm(m_target);

            if(caster->getVictim()==m_target)
                caster->AttackStop();
            m_target->CombatStop();
            m_target->DeleteThreatList();

            if(m_target->GetTypeId() == TYPEID_UNIT)
            {
                ((Creature*)m_target)->AIM_Initialize();
                CharmInfo *charmInfo = ((Creature*)m_target)->InitCharmInfo(m_target);
                charmInfo->InitCharmCreateSpells();
                charmInfo->SetReactState( REACT_DEFENSIVE );
                charmInfo->SetCommandState( COMMAND_FOLLOW );

                if(caster->GetTypeId() == TYPEID_PLAYER && caster->getClass() == CLASS_WARLOCK)
                {
                    CreatureInfo const *cinfo = ((Creature*)m_target)->GetCreatureInfo();
                    if(cinfo && cinfo->type == CREATURE_TYPE_DEMON)
                    {
                        //to prevent client crash
                        m_target->SetFlag(UNIT_FIELD_BYTES_0, 2048);
                        //just to enable stat window
                        charmInfo->SetPetNumber(objmgr.GeneratePetNumber(), true);
                        //if charmed two demons the same session, the 2nd gets the 1st one's name
                        m_target->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, time(NULL));
                    }
                }
            }

            if(caster->GetTypeId() == TYPEID_PLAYER)
            {
                ((Player*)caster)->CharmSpellInitialize();
            }
        }
        else
        {
            //remove area auras from charms
            Unit::AuraMap& tAuras = m_target->GetAuras();
            for (Unit::AuraMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
            {
                if (itr->second && itr->second->IsAreaAura())
                    m_target->RemoveAura(itr);
                else
                    ++itr;
            }

            m_target->SetCharmerGUID(0);

            if(m_target->GetTypeId() == TYPEID_PLAYER)
            {
                ((Player*)m_target)->setFactionForRace(m_target->getRace());
            }
            else
            {
                CreatureInfo const *cinfo = ((Creature*)m_target)->GetCreatureInfo();

                // restore faction
                if(((Creature*)m_target)->isPet())
                {
                    if(Unit* owner = m_target->GetOwner())
                        m_target->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,owner->getFaction());
                    else if(cinfo)
                        m_target->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,cinfo->faction_A);
                }
                else if(cinfo)                              // normal creature
                    m_target->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,cinfo->faction_A);

                // restore UNIT_FIELD_BYTES_0
                if(cinfo && caster->GetTypeId() == TYPEID_PLAYER && caster->getClass() == CLASS_WARLOCK && cinfo->type == CREATURE_TYPE_DEMON)
                {
                    CreatureDataAddon const *cainfo = ((Creature*)m_target)->GetCreatureAddon();
                    if(cainfo && cainfo->bytes0 != 0)
                        m_target->SetUInt32Value(UNIT_FIELD_BYTES_0, cainfo->bytes0);
                    else
                        m_target->RemoveFlag(UNIT_FIELD_BYTES_0, 2048);

                    if(m_target->GetCharmInfo())
                        m_target->GetCharmInfo()->SetPetNumber(0, true);
                    else
                        sLog.outError("Aura::HandleModCharm: target="I64FMTD" with typeid=%d has a charm aura but no charm info!", m_target->GetGUID(), m_target->GetTypeId());
                }
            }

            caster->SetCharm(0);

            if(caster->GetTypeId() == TYPEID_PLAYER)
            {
                WorldPacket data(SMSG_PET_SPELLS, 8);
                data << uint64(0);
                ((Player*)caster)->GetSession()->SendPacket(&data);
            }
            if(m_target->GetTypeId() == TYPEID_UNIT)
            {
                ((Creature*)m_target)->AIM_Initialize();
                if (((Creature*)m_target)->AI())
                    ((Creature*)m_target)->AI()->AttackStart(caster);
            }
        }
    }
}

void Aura::HandleModConfuse(bool apply, bool Real)
{
    Unit* caster = GetCaster();

    if( apply )
    {
        m_target->addUnitState(UNIT_STAT_CONFUSED);
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);

        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            Player *player = (Player*)m_target;
            WorldPacket data(SMSG_DEATH_NOTIFY_OBSOLETE, 10);
            data.append(player->GetPackGUID());             // target's viewpoint
            data << uint8(0x00);                            // movement disabled
            player->GetSession()->SendPacket(&data);
        }

        // Rogue/Blind ability stops attack
        // TODO: does all confuses ?
        if (caster && GetSpellProto()->SpellFamilyName == SPELLFAMILY_ROGUE &&
            (GetSpellProto()->Mechanic == MECHANIC_CONFUSED || GetSpellProto()->EffectMechanic[GetEffIndex()] == MECHANIC_CONFUSED))
            caster->AttackStop();

        // only at real add aura
        if(Real)
        {
            //This fixes blind so it doesn't continue to attack
            // TODO: may other spells casted confuse aura (but not all) stop attack
            if(caster && caster->GetTypeId() == TYPEID_PLAYER && 
                GetSpellProto()->SpellFamilyName == SPELLFAMILY_ROGUE && 
                (GetSpellProto()->Mechanic == MECHANIC_CONFUSED || GetSpellProto()->EffectMechanic[GetEffIndex()] == MECHANIC_CONFUSED))
                caster->AttackStop();

            if (m_target->GetTypeId() == TYPEID_UNIT)
                m_target->GetMotionMaster()->Mutate(new ConfusedMovementGenerator<Creature>(*((Creature*)m_target)));
            else
                m_target->GetMotionMaster()->Mutate(new ConfusedMovementGenerator<Player>(*((Player*)m_target)));
        }
    }
    else
    {
        m_target->clearUnitState(UNIT_STAT_CONFUSED);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);

        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            Player *player = (Player*)m_target;
            WorldPacket data(SMSG_DEATH_NOTIFY_OBSOLETE, 10);
            data.append(player->GetPackGUID());             // target's viewpoint
            data << uint8(0x01);                            // movement enabled
            player->GetSession()->SendPacket(&data);
        }

        // only at real remove aura
        if(Real)
        {
            m_target->GetMotionMaster()->MovementExpired(false);

            if (m_target->GetTypeId() == TYPEID_UNIT)
            {
                Creature* c = (Creature*)m_target;
                // if in combat restore movement generator
                if(c->getVictim())
                    c->GetMotionMaster()->Mutate(new TargetedMovementGenerator<Creature>(*c->getVictim()));
            }
        }
    }
}

void Aura::HandleModFear(bool Apply, bool Real)
{
    if( Apply )
    {
        m_target->addUnitState(UNIT_STAT_FLEEING);
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);

        // only at real add aura
        if(Real)
        {
            //FIX ME: we need Mutate to now not existed FleeMovementGenerator (see Aura::Update hack code)
            // at this moment Idle instead
            m_target->GetMotionMaster()->MovementExpired(false);
            m_target->GetMotionMaster()->Idle();

            // what is this for ? (doesn't work anyway)
            /*WorldPacket data(SMSG_DEATH_NOTIFY_OBSOLETE, 9);
            data.append(m_target->GetPackGUID());
            data << uint8(0x00);
            m_target->SendMessageToSet(&data,true);*/
        }
    }
    else
    {
        m_target->clearUnitState(UNIT_STAT_FLEEING);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);

        // only at real remove aura
        if(Real)
        {
            m_target->GetMotionMaster()->MovementExpired(false);

            if( m_target->GetTypeId() != TYPEID_PLAYER && m_target->isAlive() )
            {
                // restore appropriate movement generator
                if(m_target->getVictim())
                    m_target->GetMotionMaster()->Mutate(new TargetedMovementGenerator<Creature>(*m_target->getVictim()));
                else
                    m_target->GetMotionMaster()->Initialize();

                // attack caster if can
                Unit* caster = GetCaster();
                if(caster && caster != m_target->getVictim())
                    if (((Creature*)m_target)->AI())
                        ((Creature*)m_target)->AI()->AttackStart(caster);
            }

            // what is this for ? (doesn't work anyway)
            /*WorldPacket data(SMSG_DEATH_NOTIFY_OBSOLETE, 9);
            data.append(m_target->GetPackGUID());
            data << uint8(0x01);
            m_target->SendMessageToSet(&data,true);*/
        }
    }
}

void Aura::HandleFeignDeath(bool Apply, bool Real)
{
    if(!Real)
        return;

    if(!m_target || m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if( Apply )
    {
        /*
        WorldPacket data(SMSG_FEIGN_DEATH_RESISTED, 9);
        data<<m_target->GetGUID();
        data<<uint8(0);
        m_target->SendMessageToSet(&data,true);
        */
                                                            // blizz like 2.0.x
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6);
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, 0x00000001);  // blizz like 2.0.x
                                                            // blizz like 2.0.x
        m_target->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);

        m_target->addUnitState(UNIT_STAT_DIED);
        m_target->CombatStop();

        // prevent interrupt message
        if(m_caster_guid==m_target->GetGUID() && m_target->m_currentSpells[CURRENT_GENERIC_SPELL])
            m_target->m_currentSpells[CURRENT_GENERIC_SPELL]->finish();
        m_target->InterruptNonMeleeSpells(true);
        ((Player*)m_target)->SendAttackSwingCancelAttack(); // melee and ranged forced attack cancel
        m_target->getHostilRefManager().deleteReferences();

        // send cooldown explicitly
        WorldPacket data(SMSG_SPELL_COOLDOWN, 8+8);
        data << m_target->GetGUID();
        data << uint8(0x0);
        data << uint32(GetSpellProto()->Id);
        data << uint32(GetRecoveryTime(GetSpellProto()));
        ((Player*)m_target)->GetSession()->SendPacket(&data);

    }
    else
    {
        /*
        WorldPacket data(SMSG_FEIGN_DEATH_RESISTED, 9);
        data<<m_target->GetGUID();
        data<<uint8(1);
        m_target->SendMessageToSet(&data,true);
        */
                                                            // blizz like 2.0.x
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6);
                                                            // blizz like 2.0.x
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, 0x00000001);
                                                            // blizz like 2.0.x
        m_target->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);

        m_target->clearUnitState(UNIT_STAT_DIED);
    }
}

void Aura::HandleAuraModDisarm(bool Apply, bool Real)
{
    if(!Real)
        return;

    // not sure for it's correctness
    if(Apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);

    m_target->UpdateDamagePhysical(BASE_ATTACK);
}

void Aura::HandleAuraModStun(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    Unit* caster = GetCaster();

    if (apply)
    {
        m_target->addUnitState(UNIT_STAT_STUNDED);
        m_target->SetUInt64Value (UNIT_FIELD_TARGET, 0);

        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);

        if (caster)
        {
            // Stop attack if spell has knockout effect
            if(GetSpellProto()->Mechanic == MECHANIC_KNOCKOUT || GetSpellProto()->EffectMechanic[GetEffIndex()] == MECHANIC_KNOCKOUT)
                caster->AttackStop();
        }

        //Save last orientation
        if( m_target->getVictim() )
            m_target->SetOrientation(m_target->GetAngle(m_target->getVictim()));

        // Creature specific
        if(m_target->GetTypeId() != TYPEID_PLAYER)
        {
            ((Creature *)m_target)->StopMoving();
        }

        WorldPacket data(SMSG_FORCE_MOVE_ROOT, 8);

        data.append(m_target->GetPackGUID());
        data << uint32(0);
        m_target->SendMessageToSet(&data,true);
    }
    else
    {
        // Real remove called after current aura remove from lists, check if other similar auras active
        if(m_target->HasAuraType(SPELL_AURA_MOD_STUN))
            return;

        m_target->clearUnitState(UNIT_STAT_STUNDED);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);

        if(m_target->getVictim() && m_target->isAlive())
            m_target->SetUInt64Value (UNIT_FIELD_TARGET,m_target->getVictim()->GetGUID() );

        WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 8+4);
        data.append(m_target->GetPackGUID());
        data << uint32(0);
        m_target->SendMessageToSet(&data,true);

        // Wyvern Sting
        if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_HUNTER && GetSpellProto()->SpellIconID == 1721)
        {
            if( !caster || caster->GetTypeId()!=TYPEID_PLAYER )
                return;

            uint32 spell_id = 0;

            switch(GetSpellProto()->Id)
            {
                case 19386: spell_id = 24131; break;
                case 24132: spell_id = 24134; break;
                case 24133: spell_id = 24135; break;
                case 27068: spell_id = 27069; break;
                default:
                    sLog.outError("Spell selection called for unexpected original spell %u, new spell for this spell family?",GetSpellProto()->Id);
                    return;
            }

            SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell_id);

            if(!spellInfo)
                return;

            caster->CastSpell(m_target,spellInfo,true,NULL,this);
            return;
        }
    }
}

void Aura::HandleModStealth(bool apply, bool Real)
{
    if(apply)
    {
        // drop flag at stealth in bg
        if(Real && m_target->GetTypeId()==TYPEID_PLAYER && ((Player*)m_target)->InBattleGround())
            if(BattleGround *bg = ((Player*)m_target)->GetBattleGround())
                bg->HandleDropFlag((Player*)m_target);

        // not apply flag for RACE_NIGHTELF stealth
        if(GetId()!=20580)
            m_target->SetFlag(UNIT_FIELD_BYTES_1, PLAYER_STATE_FLAG_CREEP);

        // only at real aura add
        if(Real)
        {
            // apply only if not in GM invisibility
            if(m_target->GetVisibility()!=VISIBILITY_OFF)
            {
                m_target->SetVisibility(VISIBILITY_GROUP_NO_DETECT);
                if(m_target->GetTypeId() == TYPEID_PLAYER)
                    m_target->SendUpdateToPlayer((Player*)m_target);
                m_target->SetVisibility(VISIBILITY_GROUP_STEALTH);
            }

            // for RACE_NIGHTELF stealth
            if(m_target->GetTypeId()==TYPEID_PLAYER && GetId()==20580)
            {
                m_target->CastSpell(m_target, 21009, true, NULL, this);
            }
        }
    }
    else
    {
        // only at real aura remove
        if(Real)
        {
            bool reallyRemove =true;
            SpellEntry const *spellInfo = GetSpellProto();
            if( spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE &&
                (spellInfo->SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE_VANISH) &&
                (m_target->HasStealthAura() || m_target->HasInvisibilityAura()) )
                reallyRemove = false;                       // vanish it timed out, but we have stealth active as well
            if(reallyRemove)
            {
                m_target->RemoveFlag(UNIT_FIELD_BYTES_1, PLAYER_STATE_FLAG_CREEP);

                // apply only if not in GM invisibility
                if(m_target->GetVisibility()!=VISIBILITY_OFF)
                {
                    m_target->SetVisibility(VISIBILITY_ON);
                    if(m_target->GetTypeId() == TYPEID_PLAYER)
                        m_target->SendUpdateToPlayer((Player*)m_target);
                }
                // for RACE_NIGHTELF stealth
                if(m_target->GetTypeId()==TYPEID_PLAYER && GetId()==20580)
                {
                    m_target->RemoveAurasDueToSpell(21009);
                }
            }
        }
    }
}

void Aura::HandleInvisibility(bool Apply, bool Real)
{
    if(Apply)
    {
        m_target->m_invisibilityMask |= (1 << m_modifier.m_miscvalue);

        // drop flag at invisible in bg
        if(Real && m_target->GetTypeId()==TYPEID_PLAYER && ((Player*)m_target)->InBattleGround())
            if(BattleGround *bg = ((Player*)m_target)->GetBattleGround())
                bg->HandleDropFlag((Player*)m_target);

        // Real visibility update in Unit::_AddAura
    }
    else
    {
        // recalculate value at modifier remove (current aura already removed)
        m_target->m_invisibilityMask &= ~(1 << m_modifier.m_miscvalue);
        Unit::AuraList const& auras = m_target->GetAurasByType(SPELL_AURA_MOD_INVISIBILITY);
        for(Unit::AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
        {
            if((*itr)->GetModifier()->m_miscvalue==m_modifier.m_miscvalue)
            {
                m_target->m_invisibilityMask |= (1 << m_modifier.m_miscvalue);
                break;
            }
        }

        // only at real aura remove and if not have different invisibility auras.
        if(Real && m_target->m_invisibilityMask==0)
        {
            // apply only if not in GM invisibility
            if(m_target->GetVisibility()!=VISIBILITY_OFF)
            {
                m_target->SetVisibility(VISIBILITY_ON);
                if(m_target->GetTypeId() == TYPEID_PLAYER)
                    m_target->SendUpdateToPlayer((Player*)m_target);
            }
        }
    }
}

void Aura::HandleInvisibilityDetect(bool Apply, bool Real)
{
    if(Apply)
    {
        m_target->m_detectInvisibilityMask |= (1 << m_modifier.m_miscvalue);

        // Real visibility update in Unit::_AddAura
    }
    else
    {
        // recalculate value at modifier remove (current aura already removed)
        m_target->m_detectInvisibilityMask &= ~(1 << m_modifier.m_miscvalue);
        Unit::AuraList const& auras = m_target->GetAurasByType(SPELL_AURA_MOD_INVISIBILITY_DETECTION);
        for(Unit::AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
        {
            if((*itr)->GetModifier()->m_miscvalue==m_modifier.m_miscvalue)
            {
                m_target->m_detectInvisibilityMask |= (1 << m_modifier.m_miscvalue);
                break;
            }
        }

        if(Real && m_target->GetTypeId()==TYPEID_PLAYER)
            ObjectAccessor::UpdateVisibilityForPlayer((Player*)m_target);
    }
}

void Aura::HandleAuraModRoot(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    uint32 apply_stat = UNIT_STAT_ROOT;
    if (apply)
    {
        m_target->addUnitState(UNIT_STAT_ROOT);
        m_target->SetUInt64Value (UNIT_FIELD_TARGET, 0);
                                                            // probably wrong
        m_target->SetFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));

        //Save last orientation
        if( m_target->getVictim() )
            m_target->SetOrientation(m_target->GetAngle(m_target->getVictim()));

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
    else
    {
        // Real remove called after current aura remove from lists, check if other similar auras active
        if(m_target->HasAuraType(SPELL_AURA_MOD_ROOT))
            return;

        m_target->clearUnitState(UNIT_STAT_ROOT);
                                                            // probably wrong
        m_target->RemoveFlag(UNIT_FIELD_FLAGS,(apply_stat<<16));

        if(m_target->getVictim() && m_target->isAlive())
            m_target->SetUInt64Value (UNIT_FIELD_TARGET,m_target->getVictim()->GetGUID() );

        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 10);
            data.append(m_target->GetPackGUID());
            data << (uint32)2;
            m_target->SendMessageToSet(&data,true);
        }
    }
}

void Aura::HandleAuraModSilence(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    if(apply)
    {
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
        m_target->InterruptNonMeleeSpells(false);
    }
    else
    {
        // Real remove called after current aura remove from lists, check if other similar auras active
        if(m_target->HasAuraType(SPELL_AURA_MOD_SILENCE))
            return;

        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
    }
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

    if(m_modifier.m_miscvalue < SPELL_SCHOOL_NORMAL || m_modifier.m_miscvalue >= (1<<MAX_SPELL_SCHOOL))
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_THREAT not valid");
        return;
    }

    bool positive = m_modifier.m_miscvalue2 == 0;

    for(int8 x=0;x < MAX_SPELL_SCHOOL;x++)
    {
        if(m_modifier.m_miscvalue & int32(1<<x))
        {
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                ApplyPercentModFloatVar(m_target->m_threatModifier[x], positive ? m_modifier.m_amount : -m_modifier.m_amount, apply);
        }
    }
}

void Aura::HandleAuraModTotalThreat(bool Apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    if(!m_target || !m_target->isAlive() || m_target->GetTypeId()!= TYPEID_PLAYER)
        return;

    Unit* caster = GetCaster();

    if(!caster || !caster->isAlive())
        return;

    float threatMod = 0.0f;
    if(Apply)
        threatMod = float(m_modifier.m_amount);
    else
        threatMod =  float(-m_modifier.m_amount);

    m_target->getHostilRefManager().threatAssist(caster, threatMod);
}

void Aura::HandleModTaunt(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    if(!m_target || !m_target->isAlive() || !m_target->CanHaveThreatList())
        return;

    Unit* caster = GetCaster();

    if(!caster || !caster->isAlive() || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if(apply)
    {
        m_target->TauntApply(caster);
    }
    else
    {
        // When taunt aura fades out, mob will switch to previous target if current has less than 1.1 * secondthreat
        m_target->TauntFadeOut(caster);
    }
}

/*********************************************************/
/***                  MODIFY SPEED                     ***/
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

void Aura::HandleAuraModIncreaseMountedSpeedAlways(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    sLog.outDebug("HandleAuraModIncreaseMountedSpeedAlways: Current Speed:%f \tmodify percent:%f", m_target->GetSpeed(MOVE_MOUNTED),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;

    float rate = (100.0f + m_modifier.m_amount)/100.0f;

    m_target->ApplySpeedMod(MOVE_MOUNTED, rate, false, apply );

    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_MOUNTED));
}

void Aura::HandleAuraModIncreaseMountedSpeed(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    sLog.outDebug("HandleAuraModIncreaseMountedSpeed: Current Speed:%f \tmodify percent:%f", m_target->GetSpeed(MOVE_MOUNTED),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;

    float rate = (100.0f + m_modifier.m_amount)/100.0f;

    m_target->ApplySpeedMod(MOVE_MOUNTED, rate, true, apply );

    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_MOUNTED));
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
        m_target->ApplySpeedMod(MOVE_MOUNTED, rate, true, apply );
    }
    else
    {                                                       //for old spell dbc
        float rate = m_modifier.m_amount / 100.0f;

        m_target->ApplySpeedMod(MOVE_RUN, rate, true, apply );
        m_target->ApplySpeedMod(MOVE_MOUNTED, rate, true, apply );
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
    uint32 mechanic = 1 << m_modifier.m_miscvalue;
    
    //immune movement impairement and loss of control
    if(GetId()==42292)
        mechanic=0x9967da6;

    if(apply)
    {
        Unit::AuraMap& Auras = m_target->GetAuras();
        for(Unit::AuraMap::iterator iter = Auras.begin(), next; iter != Auras.end(); iter = next)
        {
            next = iter;
            next++;
            SpellEntry const *spell = iter->second->GetSpellProto();
            if (!( spell->Attributes & 0x20000000)	//spells unaffected by invulnerability
                && !iter->second->IsPositive()		//only remove negative spells
                && spell->Id != GetId())
            {
                //check for mechanic mask
                if( ((1<<spell->Mechanic) & mechanic) || ((1<<spell->EffectMechanic[iter->second->GetEffIndex()]) & mechanic))
                {
                    m_target->RemoveAurasDueToSpell(spell->Id);
                    if(Auras.empty())
                        break;
                    else
                        next = Auras.begin();
                } 
            }
        }
    }

    m_target->ApplySpellImmune(GetId(),IMMUNITY_MECHANIC,m_modifier.m_miscvalue,apply);
}

void Aura::HandleAuraModEffectImmunity(bool apply, bool Real)
{
    if(!apply)
    {
        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            if(((Player*)m_target)->InBattleGround())
            {
                BattleGround *bg = ((Player*)m_target)->GetBattleGround();
                if(bg)
                {
                    switch(bg->GetTypeID())
                    {
                        case BATTLEGROUND_AV:
                        {
                            break;
                        }
                        case BATTLEGROUND_WS:
                        {
                            if(((BattleGroundWS*)bg)->IsHordeFlagPickedup())
                                // Warsong Flag, horde
                                if(GetSpellProto()->Id == 23333)
                                    // Horde Flag Drop
                                    ((BattleGroundWS*)bg)->EventPlayerDroppedFlag(((Player*)m_target));
                            if(((BattleGroundWS*)bg)->IsAllianceFlagPickedup())
                                // Silverwing Flag, alliance
                                if(GetSpellProto()->Id == 23335)
                                    // Alliance Flag Drop
                                    ((BattleGroundWS*)bg)->EventPlayerDroppedFlag(((Player*)m_target));
                            break;
                        }
                        case BATTLEGROUND_AB:
                        {
                            break;
                        }
                        case BATTLEGROUND_EY:
                        {
                            break;
                        }
                    }
                }
            }
        }
    }

    m_target->ApplySpellImmune(GetId(),IMMUNITY_EFFECT,m_modifier.m_miscvalue,apply);
}

void Aura::HandleAuraModStateImmunity(bool apply, bool Real)
{
    if(apply && Real)
    {
        Unit::AuraList const& auraList = m_target->GetAurasByType(AuraType(m_modifier.m_miscvalue));
        while(!auraList.empty())
            m_target->RemoveAura(auraList.front()->GetId(),auraList.front()->GetEffIndex());
    }

    m_target->ApplySpellImmune(GetId(),IMMUNITY_STATE,m_modifier.m_miscvalue,apply);
}

void Aura::HandleAuraModSchoolImmunity(bool apply, bool Real)
{
    m_target->ApplySpellImmune(GetId(),IMMUNITY_SCHOOL,m_modifier.m_miscvalue,apply);

    if(Real && apply)
    {
        if(IsPositiveSpell(GetId()))                        //Only positive immunity removes auras
        {
            uint32 school_mask = m_modifier.m_miscvalue;
            Unit::AuraMap& Auras = m_target->GetAuras();
            for(Unit::AuraMap::iterator iter = Auras.begin(), next; iter != Auras.end(); iter = next)
            {
                next = iter;
                next++;
                SpellEntry const *spell = iter->second->GetSpellProto();
                if( ( (1 << spell->School) & school_mask)   //Check for school mask
                    && !( spell->Attributes & 0x20000000)   //Spells unaffected by invulnerability
                    && !iter->second->IsPositive()          //Don't remove positive spells
                    && spell->Id != GetId())                //Don't remove self
                {
                    m_target->RemoveAurasDueToSpell(spell->Id);
                    if(Auras.empty())
                        break;
                    else
                        next = Auras.begin();
                }
            }
        }
    }
}

void Aura::HandleAuraModDmgImmunity(bool apply, bool Real)
{
    m_target->ApplySpellImmune(GetId(),IMMUNITY_DAMAGE,m_modifier.m_miscvalue,apply);
}

void Aura::HandleAuraModDispelImmunity(bool apply, bool Real)
{
    Unit* caster = GetCaster();
    if (!caster)
        return;

    m_target->ApplySpellImmune(GetId(),IMMUNITY_DISPEL,m_modifier.m_miscvalue,apply);

    if(m_target->IsHostileTo(caster) && (m_target->GetTypeId()!=TYPEID_PLAYER || !((Player*)m_target)->isGameMaster()))
    {
        if (m_target->HasStealthAura() && m_modifier.m_miscvalue == 5)
        {
            m_target->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
        }

        if (m_target->HasInvisibilityAura() && m_modifier.m_miscvalue == 6)
        {
            m_target->RemoveSpellsCausingAura(SPELL_AURA_MOD_INVISIBILITY);
        }

        if( caster->GetTypeId()==TYPEID_PLAYER && !caster->IsPvP() && m_target->IsPvP())
            ((Player*)caster)->UpdatePvP(true, true);
    }
}

void Aura::HandleAuraProcTriggerSpell(bool Apply, bool Real)
{
    if(!Real)
        return;

    if(Apply)
    {
        // some spell have charges by functionality not have its in spell data
        switch (m_spellId)
        {
            case 28200: m_procCharges = 6; break;           // Ascendance (Talisman of Ascendance trinket
            default: break;
        }
    }
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

void Aura::HandleAuraModStalked(bool apply, bool Real)
{
    // used by spells: Hunter's Mark, Mind Vision, Syndicate Tracker (MURP) DND
    if(apply)
        m_target->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
    else
        m_target->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
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

    // only at real apply
    if (Real && apply && GetSpellProto()->Mechanic == MECHANIC_BANDAGE)
    {
        m_target->CastSpell(m_target,11196,true,NULL,this);
    }
}

void Aura::HandlePeriodicDamage(bool apply, bool Real)
{
    if (m_periodicTimer <= 0)
        m_periodicTimer += m_modifier.periodictime;

    m_isPeriodic = apply;

    // Deadly poison aura state
    if(Real && m_spellProto->SpellFamilyName==SPELLFAMILY_ROGUE && (m_spellProto->SpellFamilyFlags & 0x10000) && m_spellProto->SpellVisual==5100)
    {
        if(apply)
            m_target->ModifyAuraState(AURA_STATE_DEADLY_POISON,true);
        else
        {
            // current aura already removed, search present of another
            bool found = false;
            Unit::AuraList const& auras = m_target->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
            for(Unit::AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
            {
                SpellEntry const* itr_spell = (*itr)->GetSpellProto();
                if(itr_spell && itr_spell->SpellFamilyName==SPELLFAMILY_ROGUE && (itr_spell->SpellFamilyFlags & 0x10000) && itr_spell->SpellVisual==5100)
                {
                    found = true;
                    break;
                }
            }

            // this has been last deadly poison aura
            if(!found)
                m_target->ModifyAuraState(AURA_STATE_DEADLY_POISON,false);
        }
    }
}

void Aura::HandlePeriodicDamagePCT(bool apply, bool Real)
{
    if (m_periodicTimer <= 0)
        m_periodicTimer += m_modifier.periodictime;

    m_isPeriodic = apply;
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
/***                  MODIFY STATS                     ***/
/*********************************************************/

/********************************/
/***        RESISTANCE        ***/
/********************************/

void Aura::HandleAuraModResistanceExclusive(bool apply, bool Real)
{
    if(m_modifier.m_miscvalue < IMMUNE_SCHOOL_PHYSICAL || m_modifier.m_miscvalue > 127)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_RESISTANCE_EXCLUS not valid");
        return;
    }

    bool positive = m_modifier.m_miscvalue2 == 0;

    for(int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL;x++)
    {
        if(m_modifier.m_miscvalue & int32(1<<x))
        {
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_VALUE, float(m_modifier.m_amount), apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                m_target->ApplyResistanceBuffModsMod(SpellSchools(x),positive,m_modifier.m_amount, apply);
        }
    }
}

void Aura::HandleAuraModResistance(bool apply, bool Real)
{
    if(m_modifier.m_miscvalue < IMMUNE_SCHOOL_PHYSICAL || m_modifier.m_miscvalue > 127)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_RESISTANCE not valid");
        return;
    }

    bool positive = m_modifier.m_miscvalue2 == 0;

    for(int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL;x++)
    {
        if(m_modifier.m_miscvalue & int32(1<<x))
        {
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), TOTAL_VALUE, float(m_modifier.m_amount), apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER || ((Creature*)m_target)->isPet())
                m_target->ApplyResistanceBuffModsMod(SpellSchools(x),positive,m_modifier.m_amount, apply);
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
    {
        //pets only have base armor
        if(((Creature*)m_target)->isPet() && (m_modifier.m_miscvalue & IMMUNE_SCHOOL_PHYSICAL))
        {
            m_target->HandleStatModifier(UNIT_MOD_ARMOR, BASE_PCT, float(m_modifier.m_amount), apply);
        }
    }
    else
    {
        for(int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL;x++)
        {
            if(m_modifier.m_miscvalue & int32(1<<x))
            {
                m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_PCT, float(m_modifier.m_amount), apply);
            }
        }
    }
}

void Aura::HandleModResistancePercent(bool apply, bool Real)
{
    for(int8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
    {
        if(m_modifier.m_miscvalue & int32(1<<i))
        {
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_PCT, float(m_modifier.m_amount), apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER || ((Creature*)m_target)->isPet())
            {
                m_target->ApplyResistanceBuffModsPercentMod(SpellSchools(i),true,m_modifier.m_amount, apply);
                m_target->ApplyResistanceBuffModsPercentMod(SpellSchools(i),false,m_modifier.m_amount, apply);
            }
        }
    }
}

void Aura::HandleModBaseResistance(bool apply, bool Real)
{
    // only players have base stats
    if(m_target->GetTypeId() != TYPEID_PLAYER)
    {
        //only pets have base stats
        if(((Creature*)m_target)->isPet() && m_modifier.m_miscvalue & IMMUNE_SCHOOL_PHYSICAL)
            m_target->HandleStatModifier(UNIT_MOD_ARMOR, TOTAL_VALUE, float(m_modifier.m_amount), apply);
    }
    else
    {
        for(int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
            if(m_modifier.m_miscvalue & (1<<i))
                m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_VALUE, float(m_modifier.m_amount), apply);
    }
}

/********************************/
/***           STAT           ***/
/********************************/

void Aura::HandleAuraModStat(bool apply, bool Real)
{
    if (m_modifier.m_miscvalue < -2 || m_modifier.m_miscvalue > 4)
    {
        sLog.outError("WARNING: Spell %u effect %u have unsupported misc value (%i) for SPELL_AURA_MOD_STAT ",GetId(),GetEffIndex(),m_modifier.m_miscvalue);
        return;
    }

    for(int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        // -1 or -2 is all stats ( misc < -2 checked in function beggining )
        if (m_modifier.m_miscvalue < 0 || m_modifier.m_miscvalue == i)
        {
            //m_target->ApplyStatMod(Stats(i), m_modifier.m_amount,apply);
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, float(m_modifier.m_amount), apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER || ((Creature*)m_target)->isPet())
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

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        if(m_modifier.m_miscvalue == i || m_modifier.m_miscvalue == -1)
        {
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), BASE_PCT, float(m_modifier.m_amount), apply);
        }
    }
}

void Aura::HandleModSpellDamagePercentFromStat(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // stat used dependent from next effect aura SPELL_AURA_MOD_SPELL_HEALING presence and misc value (stat index)
    Stats usedStat = STAT_INTELLECT;
    if(m_effIndex < 2 && m_spellProto->EffectApplyAuraName[m_effIndex+1]==SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT)
        usedStat = Stats(m_spellProto->EffectMiscValue[m_effIndex+1]);

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only

    // Recalculate bonus
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();

    // On apply aura isn`t on Player so need add manually
    if (apply)
    {
        int32 DamageBonus = int32(m_target->GetStat(usedStat) * m_modifier.m_amount / 100.0f);

        for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; i++)
        {
            if((m_modifier.m_miscvalue & (1<<i)) != 0)
                m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+i,DamageBonus,apply);
        }
    }

}

void Aura::HandleModSpellHealingPercentFromStat(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    Stats usedStat = Stats(m_spellProto->EffectMiscValue[m_effIndex]);

    // Recalculate bonus
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();

    // On apply aura isn`t on Player so need add manually
    int32 HealingBonus = 0;
    if (apply)
    {
        int32 HealingBonus = int32(m_target->GetStat(usedStat) * m_modifier.m_amount / 100.0f);
        m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS,HealingBonus,apply);
    }
}

void Aura::HandleModSpellDamagePercentFromAttackPower(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only

    // Recalculate bonus
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();

    // On apply aura isn`t on Player so need add manually
    if (apply)
    {
        int32 DamageBonus = int32(m_target->GetTotalAttackPowerValue(BASE_ATTACK) * m_modifier.m_amount/100.0f);

        for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; i++)
        {
            if((m_modifier.m_miscvalue & (1<<i)) != 0)
                m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+i,DamageBonus,apply);
        }
    }

}

void Aura::HandleModSpellHealingPercentFromAttackPower(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate bonus
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();

    // On apply aura isn`t on Player so need add manually
    if (apply)
    {
        int32 HealingBonus = int32(m_target->GetTotalAttackPowerValue(BASE_ATTACK) * m_modifier.m_amount/100.0f);
        m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS,HealingBonus,apply);
    }
}

void Aura::HandleModHealingDone(bool apply, bool Real)
{
    // implemented in Unit::SpellHealingBonus
    // this information is for client side only
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS,m_modifier.m_amount,apply);
}

void Aura::HandleModTotalPercentStat(bool apply, bool Real)
{
    if (m_modifier.m_miscvalue < -1 || m_modifier.m_miscvalue > 4)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_PERCENT_STAT not valid");
        return;
    }

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        if(m_modifier.m_miscvalue == i || m_modifier.m_miscvalue == -1)
        {
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT, float(m_modifier.m_amount), apply);
            if (m_target->GetTypeId() == TYPEID_PLAYER)
            {
                ((Player*)m_target)->ApplyPosStatPercentMod(Stats(i), m_modifier.m_amount, apply );
                ((Player*)m_target)->ApplyNegStatPercentMod(Stats(i), m_modifier.m_amount, apply );
            }
        }
    }
}

void Aura::HandleAuraModResistenceOfIntellectPercent(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER) 
        return;

    if(m_modifier.m_miscvalue != (1<<SPELL_SCHOOL_NORMAL))
    {
        // support required adding replace UpdateArmor by loop by UpdateResistence at intelect update
        // and include in UpdateResistence same code as in UpdateArmor for aura mod apply.
        sLog.outError("Aura SPELL_AURA_MOD_RESISTANCE_OF_INTELLECT_PERCENT(182) need adding support for non-armor resistences!"); 
        return;
    }
    
    // Recalculate Armor 
    m_target->UpdateArmor();
    
    // On apply aura isn`t on Player so need add manually 
     if (apply) 
     { 
         int32 ArmorBonus = int32(m_target->GetStat(STAT_INTELLECT) * m_modifier.m_amount/100.0f);
         m_target->ApplyModUInt32Value(UNIT_FIELD_RESISTANCES + SPELL_SCHOOL_NORMAL,ArmorBonus,apply); 
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
    if(apply)
    {
        if(!m_target->isAlive())
            return;

        if((GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED))
        {
            m_target->SetFlag(UNIT_FIELD_BYTES_1, PLAYER_STATE_SIT);
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                ((Player*)m_target)->SetStandState(PLAYER_STATE_SIT);
        }

        if(m_periodicTimer <= 0)
        {
            m_periodicTimer += m_modifier.periodictime;

            if(m_target->GetHealth() < m_target->GetMaxHealth())
            {
                // PeriodicAuraLog can cast triggered spells with stats changes
                m_target->PeriodicAuraLog(m_target, GetSpellProto(), &m_modifier,GetEffIndex(),m_castItemGuid);
            }
        }
    }

    m_isPeriodic = apply;
}

void Aura::HandleAuraModTotalManaPercentRegen(bool apply, bool Real)
{
    if((GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED) && apply)
    {
        m_target->SetFlag(UNIT_FIELD_BYTES_1, PLAYER_STATE_SIT);
        if(m_target->GetTypeId() == TYPEID_PLAYER)
            ((Player*)m_target)->SetStandState(PLAYER_STATE_SIT);
    }

    if(apply && m_periodicTimer <= 0 && m_target->getPowerType() == POWER_MANA)
    {
        m_periodicTimer += m_modifier.periodictime;

        if(m_target->GetPower(POWER_MANA) < m_target->GetMaxPower(POWER_MANA))
        {
            // PeriodicAuraLog can cast triggered spells with stats changes
            m_target->PeriodicAuraLog(m_target, GetSpellProto(), &m_modifier,GetEffIndex(),m_castItemGuid);
        }
    }

    m_isPeriodic = apply;
}

void Aura::HandleModRegen(bool apply, bool Real)            // eating
{
    if(apply)
    {
        if(!m_target->isAlive())
            return;

        if ((GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED))
        {
            m_target->SetFlag(UNIT_FIELD_BYTES_1, PLAYER_STATE_SIT);
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                ((Player*)m_target)->SetStandState(PLAYER_STATE_SIT);
        }

        if(m_periodicTimer <= 0)
        {
            m_periodicTimer += 5000;
            int32 gain = m_target->ModifyHealth(m_modifier.m_amount);
            Unit *caster = GetCaster();
            if (caster)
            {
                SpellEntry const *spellProto = GetSpellProto();
                if (spellProto)
                    m_target->getHostilRefManager().threatAssist(caster, float(gain) * 0.5f, spellProto);
            }
        }
    }

    m_isPeriodic = apply;
}

void Aura::HandleModPowerRegen(bool apply, bool Real)       // drinking
{
    if (m_target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_target)->UpdateManaRegen();
    if ((GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED) && apply)
    {
        m_target->SetFlag(UNIT_FIELD_BYTES_1, PLAYER_STATE_SIT);
        if(m_target->GetTypeId() == TYPEID_PLAYER)
            ((Player*)m_target)->SetStandState(PLAYER_STATE_SIT);
    }

    if(apply && m_periodicTimer <= 0)
    {
        m_periodicTimer += 2000;

        Powers pt = m_target->getPowerType();
        if(int32(pt) != m_modifier.m_miscvalue)
            return;

        if ( GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED )
        {
            // eating anim
            m_target->HandleEmoteCommand(EMOTE_ONESHOT_EAT);
        }
        else if( GetSpellProto()->Id == 20577 )
        {
            // cannibalize anim
            m_target->HandleEmoteCommand(398);
        }

        // Prevent rage regeneration in combat with rage loss slowdown warrior talent and 0<->1 switching range out combat.
        if( !(pt == POWER_RAGE && (m_target->isInCombat() || m_target->GetPower(POWER_RAGE) == 0)) )
        {
            if(pt != POWER_MANA)
                m_target->ModifyPower(pt, m_modifier.m_amount*2/5);
        }
    }
    m_isPeriodic = apply;
}

void Aura::HandleModManaRegen(bool apply, bool Real)
{
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    //Already calculated in Player::UpdateManaRegen()
    //Note: an increase in regen does NOT cause threat.
    ((Player*)m_target)->UpdateManaRegen();

}

void Aura::HandleAuraModIncreaseHealth(bool apply, bool Real)
{
    // Special case with temporary increase max/current health
    if( GetId() == 12976 ||                                 //Warrior Last Stand triggered spell
        GetId() == 44055 )                                  //Item spell
    {
        if(Real)
        {
            if(apply)
            {
                m_target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_modifier.m_amount), apply);
                m_target->ModifyHealth(m_modifier.m_amount);
            }
            else
            {
                if (int32(m_target->GetHealth()) > m_modifier.m_amount)
                    m_target->ModifyHealth(-m_modifier.m_amount);
                else
                    m_target->SetHealth(1);
                m_target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_modifier.m_amount), apply);
            }
        }

        return;
    }

    // generic case
    m_target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModIncreaseEnergy(bool apply, bool Real)
{
    Powers powerType = m_target->getPowerType();
    if(int32(powerType) != m_modifier.m_miscvalue)
        return;

    m_target->HandleStatModifier(UnitMods(UNIT_MOD_POWER_START + powerType), TOTAL_VALUE, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModIncreaseEnergyPercent(bool apply, bool Real)
{
    Powers powerType = m_target->getPowerType();
    if(int32(powerType) != m_modifier.m_miscvalue)
        return;

    m_target->HandleStatModifier(UnitMods(UNIT_MOD_POWER_START + powerType), TOTAL_PCT, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModIncreaseHealthPercent(bool apply, bool Real)
{
    //m_target->ApplyMaxHealthPercentMod(m_modifier.m_amount,apply);
    m_target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_PCT, float(m_modifier.m_amount), apply);
}

/********************************/
/***          FIGHT           ***/
/********************************/

void Aura::HandleAuraModParryPercent(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    ((Player*)m_target)->HandleBaseModValue(PARRY_PERCENTAGE, FLAT_MOD, float (m_modifier.m_amount), apply);
}

void Aura::HandleAuraModDodgePercent(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    ((Player*)m_target)->HandleBaseModValue(DODGE_PERCENTAGE, FLAT_MOD, float (m_modifier.m_amount), apply);
    //sLog.outError("BONUS DODGE CHANCE: + %f", float(m_modifier.m_amount));
}

void Aura::HandleAuraModBlockPercent(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    ((Player*)m_target)->HandleBaseModValue(BLOCK_PERCENTAGE, FLAT_MOD, float (m_modifier.m_amount), apply);
    //sLog.outError("BONUS BLOCK CHANCE: + %f", float(m_modifier.m_amount));
}

void Aura::HandleAuraModRegenInterrupt(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;
    ((Player*)m_target)->UpdateManaRegen();
}

void Aura::HandleAuraModCritPercent(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    // apply item specific bonuses for already equipped weapon
    if(Real)
    {
        if(Item* pItem = ((Player*)m_target)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
            ((Player*)m_target)->_ApplyWeaponDependentAuraCritMod(pItem,EQUIPMENT_SLOT_MAINHAND,this,apply);

        if(Item* pItem = ((Player*)m_target)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
            ((Player*)m_target)->_ApplyWeaponDependentAuraCritMod(pItem,EQUIPMENT_SLOT_OFFHAND,this,apply);

        if(Item* pItem = ((Player*)m_target)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
            ((Player*)m_target)->_ApplyWeaponDependentAuraCritMod(pItem,EQUIPMENT_SLOT_RANGED,this,apply);
    }

    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // m_modifier.m_miscvalue comparison with item generated damage types

    if (GetSpellProto()->EquippedItemClass == -1)
    {
        ((Player*)m_target)->HandleBaseModValue(CRIT_PERCENTAGE,         FLAT_MOD, float (m_modifier.m_amount), apply);
        ((Player*)m_target)->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, float (m_modifier.m_amount), apply);
        ((Player*)m_target)->HandleBaseModValue(RANGED_CRIT_PERCENTAGE,  FLAT_MOD, float (m_modifier.m_amount), apply);
    }
    else
    {
        // done in Player::_ApplyWeaponDependentAuraMods
    }
}

void Aura::HandleModHitChance(bool Apply, bool Real)
{
    m_target->m_modMeleeHitChance += Apply ? m_modifier.m_amount : (-m_modifier.m_amount);
    m_target->m_modRangedHitChance += Apply ? m_modifier.m_amount : (-m_modifier.m_amount);
}

void Aura::HandleModSpellHitChance(bool Apply, bool Real)
{
    m_target->m_modSpellHitChance += Apply ? m_modifier.m_amount: (-m_modifier.m_amount);
}

void Aura::HandleModSpellCritChance(bool Apply, bool Real)
{
    if(m_target->GetTypeId() == TYPEID_PLAYER)
    {
        for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            ((Player*)m_target)->HandleBaseModValue( BaseModGroup(SPELL_CRIT_PERCENTAGE + i), FLAT_MOD, float(m_modifier.m_amount), Apply);
    }
    else
    {
        m_target->m_baseSpellCritChance += Apply?m_modifier.m_amount:(-m_modifier.m_amount);
    }
}

void Aura::HandleModSpellCritChanceShool(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
    {
        if(m_modifier.m_miscvalue == -2 || (m_modifier.m_miscvalue & (1<<i)) != 0)
            ((Player*)m_target)->HandleBaseModValue( BaseModGroup(SPELL_CRIT_PERCENTAGE + i), FLAT_MOD, float(m_modifier.m_amount), apply);
    }
}

/********************************/
/***         ATTACK SPEED     ***/
/********************************/

void Aura::HandleModCastingSpeed(bool apply, bool Real)
{
    m_target->ApplyCastTimePercentMod(m_modifier.m_amount,apply);
}

void Aura::HandleModCombatSpeedPct(bool apply, bool Real)
{
    m_target->ApplyCastTimePercentMod(m_modifier.m_amount,apply);
    m_target->ApplyAttackTimePercentMod(BASE_ATTACK,m_modifier.m_amount,apply);
    m_target->ApplyAttackTimePercentMod(OFF_ATTACK,m_modifier.m_amount,apply);
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK, m_modifier.m_amount, apply);
}

void Aura::HandleModAttackSpeed(bool apply, bool Real)
{
    if(!m_target || !m_target->isAlive() )
        return;

    m_target->ApplyAttackTimePercentMod(BASE_ATTACK,m_modifier.m_amount,apply);
}

void Aura::HandleHaste(bool apply, bool Real)
{
    m_target->ApplyAttackTimePercentMod(BASE_ATTACK,  m_modifier.m_amount,apply);
    m_target->ApplyAttackTimePercentMod(OFF_ATTACK,   m_modifier.m_amount,apply);
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK,m_modifier.m_amount,apply);
}

void Aura::HandleAuraModRangedHaste(bool apply, bool Real)
{
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK, m_modifier.m_amount, apply);
}

void Aura::HandleRangedAmmoHaste(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK,m_modifier.m_amount, apply);
}

/********************************/
/***        ATTACK POWER      ***/
/********************************/

void Aura::HandleAuraModAttackPower(bool apply, bool Real)
{
    m_target->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModRangedAttackPower(bool apply, bool Real)
{
    if((m_target->getClassMask() & CLASSMASK_WAND_USERS)!=0)
        return;

    m_target->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModAttackPowerPercent(bool apply, bool Real)
{
    //UNIT_FIELD_ATTACK_POWER_MULTIPLIER = multiplier - 1
    m_target->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_PCT, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModRangedAttackPowerPercent(bool apply, bool Real)
{
    if((m_target->getClassMask() & CLASSMASK_WAND_USERS)!=0)
        return;

    //UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER = multiplier - 1
    m_target->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_PCT, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModRangedAttackPowerOfStatPercent(bool apply, bool Real)
{
    if(m_target->GetTypeId() == TYPEID_PLAYER && (m_target->getClassMask() & CLASSMASK_WAND_USERS)!=0)
        return;

    if(m_modifier.m_miscvalue != STAT_INTELLECT)
    {
        // support required adding UpdateAttackPowerAndDamage calls at stat update
        sLog.outError("Aura SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT (212) need support non-intelect stats!");
        return;
    }
    
    // Recalculate bonus 
    ((Player*)m_target)->UpdateAttackPowerAndDamage(true); 
      
     // On apply aura isn`t on Player so need add manually 
     if (apply) 
     { 
         int32 RAPBonus = int32(m_target->GetStat(Stats(m_modifier.m_miscvalue)) * m_modifier.m_amount / 100.0f); 
         m_target->ApplyModUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS,RAPBonus,apply); 
    }
}

/********************************/
/***        DAMAGE BONUS      ***/
/********************************/
void Aura::HandleModDamageDone(bool apply, bool Real)
{
    // apply item specific bonuses for already equipped weapon
    if(Real && m_target->GetTypeId()==TYPEID_PLAYER)
    {
        if(Item* pItem = ((Player*)m_target)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
            ((Player*)m_target)->_ApplyWeaponDependentAuraDamageMod(pItem,EQUIPMENT_SLOT_MAINHAND,this,apply);

        if(Item* pItem = ((Player*)m_target)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
            ((Player*)m_target)->_ApplyWeaponDependentAuraDamageMod(pItem,EQUIPMENT_SLOT_OFFHAND,this,apply);

        if(Item* pItem = ((Player*)m_target)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
            ((Player*)m_target)->_ApplyWeaponDependentAuraDamageMod(pItem,EQUIPMENT_SLOT_RANGED,this,apply);
    }

    // m_modifier.m_miscvalue is bitmask of spell schools
    // 1 ( 0-bit ) - normal school damage (IMMUNE_SCHOOL_PHYSICAL)
    // 126 - full bitmask all magic damages (IMMUNE_SCHOOL_MAGIC) including wands
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // m_modifier.m_miscvalue comparison with item generated damage types

    if((m_modifier.m_miscvalue & IMMUNE_SCHOOL_PHYSICAL) != 0)
    {
        // apply generic physical damage bonuses including wand case
        if (GetSpellProto()->EquippedItemClass == -1 || m_target->GetTypeId() != TYPEID_PLAYER)
        {
            m_target->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE, float(m_modifier.m_amount), apply);
            m_target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE, float(m_modifier.m_amount), apply);
            m_target->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_VALUE, float(m_modifier.m_amount), apply);
        }
        else
        {
            // done in Player::_ApplyWeaponDependentAuraMods
        }

        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            if(m_modifier.m_miscvalue2)
                m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG,m_modifier.m_amount,apply);
            else
                m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS,m_modifier.m_amount,apply);
        }
    }

    // Skip non magic case for speedup
    if((m_modifier.m_miscvalue & IMMUNE_SCHOOL_MAGIC) == 0)
        return;

    if( GetSpellProto()->EquippedItemClass != -1 || GetSpellProto()->EquippedItemInventoryTypeMask != 0 )
    {
        // wand magic case (skip generic to all item spell bonuses)
        // done in Player::_ApplyWeaponDependentAuraMods

        // Skip item specific requirements for not wand magic damage
        return;
    }

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only
    if(m_target->GetTypeId() == TYPEID_PLAYER)
    {
        if(m_modifier.m_miscvalue2)
        {
            for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; i++)
            {
                if((m_modifier.m_miscvalue & (1<<i)) != 0)
                    m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG+i,m_modifier.m_amount,apply);
            }
        }
        else
        {
            for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; i++)
            {
                if((m_modifier.m_miscvalue & (1<<i)) != 0)
                    m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+i,m_modifier.m_amount,apply);
            }
        }
        Pet* pet = m_target->GetPet();
        if(pet)
            pet->UpdateAttackPowerAndDamage();
    }
}

void Aura::HandleModDamagePercentDone(bool apply, bool Real)
{
    sLog.outDebug("AURA MOD DAMAGE type:%u type2:%u", m_modifier.m_miscvalue, m_modifier.m_miscvalue2);

    // apply item specific bonuses for already equipped weapon
    if(Real && m_target->GetTypeId()==TYPEID_PLAYER)
    {
        if(Item* pItem = ((Player*)m_target)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
            ((Player*)m_target)->_ApplyWeaponDependentAuraDamageMod(pItem,EQUIPMENT_SLOT_MAINHAND,this,apply);

        if(Item* pItem = ((Player*)m_target)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
            ((Player*)m_target)->_ApplyWeaponDependentAuraDamageMod(pItem,EQUIPMENT_SLOT_OFFHAND,this,apply);

        if(Item* pItem = ((Player*)m_target)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
            ((Player*)m_target)->_ApplyWeaponDependentAuraDamageMod(pItem,EQUIPMENT_SLOT_RANGED,this,apply);
    }

    // m_modifier.m_miscvalue is bitmask of spell schools
    // 1 ( 0-bit ) - normal school damage (IMMUNE_SCHOOL_PHYSICAL)
    // 126 - full bitmask all magic damages (IMMUNE_SCHOOL_MAGIC) including wand
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // m_modifier.m_miscvalue comparison with item generated damage types

    if((m_modifier.m_miscvalue & IMMUNE_SCHOOL_PHYSICAL) != 0)
    {
        // apply generic physical damage bonuses including wand case
        if (GetSpellProto()->EquippedItemClass == -1 || m_target->GetTypeId() != TYPEID_PLAYER)
        {
            m_target->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, float(m_modifier.m_amount), apply);
            m_target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT, float(m_modifier.m_amount), apply);
            m_target->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_PCT, float(m_modifier.m_amount), apply);
        }
        else
        {
            // done in Player::_ApplyWeaponDependentAuraMods
        }
    }

    // Skip non magic case for speedup
    if((m_modifier.m_miscvalue & IMMUNE_SCHOOL_MAGIC) == 0)
        return;

    if( GetSpellProto()->EquippedItemClass != -1 || GetSpellProto()->EquippedItemInventoryTypeMask != 0 )
    {
        // wand magic case (skip generic to all item spell bonuses)
        // done in Player::_ApplyWeaponDependentAuraMods

        // Skip item specific requirements for not wand magic damage
        return;
    }

    // Magic damage percent modifiers implemented in Unit::SpellDamageBonus
    // Client does not update visual spell damages when percentage aura is applied
}

void Aura::HandleModOffhandDamagePercent(bool apply, bool Real)
{
    sLog.outDebug("AURA MOD OFFHAND DAMAGE");

    if (!m_target )
        return;

    m_target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT, float(m_modifier.m_amount), apply);
}

/********************************/
/***        POWER COST        ***/
/********************************/

void Aura::HandleModPowerCostPCT(bool apply, bool Real)
{
    float amount = m_modifier.m_amount/100.0f;
    for(int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if(m_modifier.m_miscvalue & (1<<i))
            m_target->ApplyModSignedFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER+i,amount,apply);
}

void Aura::HandleModPowerCost(bool apply, bool Real)
{
    for(int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if(m_modifier.m_miscvalue & (1<<i))
            m_target->ApplyModInt32Value(UNIT_FIELD_POWER_COST_MODIFIER+i,m_modifier.m_amount,apply);
}

/*********************************************************/
/***                    OTHERS                         ***/
/*********************************************************/

void Aura::SendCoolDownEvent()
{
    Unit* caster = GetCaster();
    if(caster)
    {
        WorldPacket data(SMSG_COOLDOWN_EVENT, (4+8+4));     // last check 2.0.10
        data << uint32(m_spellId) << m_caster_guid;
        //data << uint32(0); // removed
        caster->SendMessageToSet(&data,true);               // WTF? why we send cooldown message to set?
    }
}

void Aura::HandleShapeshiftBoosts(bool apply)
{
    if(!m_target)
        return;

    uint32 spellId = 0;
    uint32 spellId2 = 0;
    uint32 HotWSpellId = 0;

    switch(GetModifier()->m_miscvalue)
    {
        case FORM_CAT:
            spellId = 3025;
            HotWSpellId = 24900;
            break;
        case FORM_TREE:
            spellId = 5420;
            break;
        case FORM_TRAVEL:
            spellId = 5419;
            break;
        case FORM_AQUA:
            spellId = 5421;
            break;
        case FORM_BEAR:
            spellId = 1178;
            spellId2 = 21178;
            HotWSpellId = 24899;
            break;
        case FORM_DIREBEAR:
            spellId = 9635;
            spellId2 = 21178;
            HotWSpellId = 24899;
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
        case FORM_FLIGHT:
            spellId = 33948;
            break;
        case FORM_SWIFT_FLIGHT:
            spellId  = 40122;
            spellId2 = 40121;
            break;
        case FORM_SPIRITOFREDEMPTION:
            spellId  = 27792;
            spellId2 = 27795;                               // must be second, this important at aura remove to prevent to early iterator invalidation.
            break;
        case FORM_GHOSTWOLF:
        case FORM_AMBIENT:
        case FORM_GHOUL:
        case FORM_SHADOW:
        case FORM_STEALTH:
        case FORM_CREATURECAT:
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

        if (spellId) m_target->CastSpell(m_target, spellId, true, NULL, this );
        if (spellId2) m_target->CastSpell(m_target, spellId2, true, NULL, this);

        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            const PlayerSpellMap& sp_list = ((Player *)m_target)->GetSpellMap();
            for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
            {
                if(itr->second->state == PLAYERSPELL_REMOVED) continue;
                if(itr->first==spellId || itr->first==spellId2) continue;
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(itr->first);
                if (!spellInfo || !(spellInfo->Attributes & ((1<<6) | (1<<7)))) continue;
                if (spellInfo->Stances & (1<<form))
                    m_target->CastSpell(m_target, itr->first, true, NULL, this);
            }
            //LotP
            if (((Player*)m_target)->HasSpell(17007))
            {
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(24932);
                if (spellInfo && spellInfo->Stances & (1<<form))
                    m_target->CastSpell(m_target, 24932, true, NULL, this);
            }
            // HotW
            if (HotWSpellId)
            {
                int32 HotWMod = 0;
                Unit::AuraList const& mModTotalStatPct = m_target->GetAurasByType(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
                for(Unit::AuraList::const_iterator i = mModTotalStatPct.begin(); i != mModTotalStatPct.end(); ++i)
                    if ((*i)->GetSpellProto()->SpellIconID == 240 && (*i)->GetModifier()->m_miscvalue == 3)
                        HotWMod = (*i)->GetModifier()->m_amount;
                if (HotWMod)
                    m_target->CastCustomSpell(m_target, HotWSpellId, &HotWMod, NULL, NULL, true, NULL, this);
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
            if (itr->second->IsRemovedOnShapeLost())
            {
                m_target->RemoveAura(itr);
            }
            else
            {
                ++itr;
            }
        }
    }

    /*double healthPercentage = (double)m_target->GetHealth() / (double)m_target->GetMaxHealth();
    m_target->SetHealth(uint32(ceil((double)m_target->GetMaxHealth() * healthPercentage)));*/
}

void Aura::HandleAuraEmpathy(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_UNIT)
        return;

    CreatureInfo const * ci = objmgr.GetCreatureTemplate(m_target->GetEntry());
    if(ci && ci->type == CREATURE_TYPE_BEAST)
    {
        m_target->ApplyModUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO, apply);
    }
}

void Aura::HandleAuraUntrackable(bool apply, bool Real)
{
    // value 100% blizz like (2.0.10)
    m_target->ApplyModUInt32Value(UNIT_FIELD_BYTES_1, 0x4000000, apply);
    /*
    Packet offset 00
    Packet number: 1
    Opcode: 00A9
    Object count: 1
    Unk: 0
    Update block for object 1:
    Block offset 07
    Updatetype: UPDATETYPE_VALUES
    Object guid: 00000000004765CE
    === values_update_block_start ===
    Bit mask blocks count: 45
    UNIT_FIELD_POWER1 (23): 1105
    UNIT_FIELD_AURA1 (48): 13161
    UNIT_FIELD_AURAFLAGS1 (104): 9
    UNIT_FIELD_BYTES_1 (152): 67108864
    === values_update_block_end ===

    Packet offset 00
    Packet number: 1
    Opcode: 00A9
    Object count: 1
    Unk: 0
    Update block for object 1:
    Block offset 07
    Updatetype: UPDATETYPE_VALUES
    Object guid: 00000000004765CE
    === values_update_block_start ===
    Bit mask blocks count: 45
    UNIT_FIELD_AURA1 (48): 0
    UNIT_FIELD_AURAFLAGS1 (104): 0
    UNIT_FIELD_BYTES_1 (152): 0
    === values_update_block_end ===
    */
}

void Aura::HandleAuraModPacify(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
}

void Aura::HandleAuraModPacifyAndSilence(bool apply, bool Real)
{
    HandleAuraModPacify(apply,Real);
    HandleAuraModSilence(apply,Real);
}

void Aura::HandleAuraGhost(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(apply)
    {
        m_target->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
    }
    else
    {
        m_target->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
    }
}

void Aura::HandleAuraAllowFlight(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    // allow fly
    sLog.outDebug("%u %u %u %u %u", m_modifier.m_amount, m_modifier.m_auraname, m_modifier.m_miscvalue, m_modifier.m_miscvalue2, m_modifier.periodictime);

    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_FLY_MODE_START, 12);
    else
        data.Initialize(SMSG_FLY_MODE_STOP, 12);
    data.append(m_target->GetPackGUID());
    data << uint32(0);                                      // unk
    m_target->SendMessageToSet(&data, true);
}

void Aura::HandleAuraModSpeedMountedFlight(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    sLog.outDebug("HandleAuraModSpeedMountedFlight: Current Speed:%f \tmodify percent:%f", m_target->GetSpeed(MOVE_FLY),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;

    float rate = (100.0f + m_modifier.m_amount)/100.0f;

    m_target->ApplySpeedMod(MOVE_FLY, rate, true, apply );

    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_FLY));

    // allow fly
    sLog.outDebug("%u %u %u %u %u", m_modifier.m_amount, m_modifier.m_auraname, m_modifier.m_miscvalue, m_modifier.m_miscvalue2, m_modifier.periodictime);

    // FIXME: is later code need?
    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_FLY_MODE_START, 12);
    else
        data.Initialize(SMSG_FLY_MODE_STOP, 12);
    data.append(m_target->GetPackGUID());
    data << uint32(0);                                      // unk
    m_target->SendMessageToSet(&data, true);
}

void Aura::HandleAuraModSpeedFlight(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    sLog.outDebug("HandleAuraModSpeedFlight: Current Speed:%f \tmodify percent:%f", m_target->GetSpeed(MOVE_FLY),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;

    float rate = (100.0f + m_modifier.m_amount)/100.0f;

    m_target->ApplySpeedMod(MOVE_FLY, rate, true, apply );

    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_FLY));
}

void Aura::HandleAuraModSpeedFlightAlways(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    sLog.outDebug("HandleAuraModSpeedFlightAlways: Current Speed:%f \tmodify percent:%f", m_target->GetSpeed(MOVE_FLY),(float)m_modifier.m_amount);
    if(m_modifier.m_amount<=1)
        return;

    float rate = (100.0f + m_modifier.m_amount)/100.0f;

    m_target->ApplySpeedMod(MOVE_FLY, rate, true, apply );

    sLog.outDebug("ChangeSpeedTo:%f", m_target->GetSpeed(MOVE_FLY));
}

void Aura::HandleModRating(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    for (uint32 rating = 0; rating < MAX_RATING; ++rating)
        if (m_modifier.m_miscvalue & (1 << rating))
            ((Player*)m_target)->ApplyRatingMod(PLAYER_FIELD_COMBAT_RATING_1 + rating, m_modifier.m_amount, apply);
}

void Aura::HandleAuraModExpertise(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)m_target)->UpdateExpertise();
    // On apply rating aura not exist - need add it manually
    if (apply)
        ((Player*)m_target)->ApplyModInt32Value(PLAYER_EXPERTISE, m_modifier.m_amount, apply);
}

void Aura::HandleModTargetResistance(bool apply, bool Real)
{
    // applied to damage as HandleNoImmediateEffect in Unit::CalcAbsorbResist and Unit::CalcArmorReducedDamage

    // show as spell penetration only full spell penetration bonuses (all resistances except armor and holy
    if (m_target->GetTypeId() == TYPEID_PLAYER && (m_modifier.m_miscvalue & 124)==124)
        m_target->ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE,m_modifier.m_amount, apply);
}

//HandleNoImmediateEffect auras implementation to support new stat system
void Aura::HandleAuraHealing(bool apply, bool Real)
{
    //m_target->HandleStatModifier(UNIT_MOD_HEALING, TOTAL_VALUE, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraHealingPct(bool apply, bool Real)
{
    //m_target->HandleStatModifier(UNIT_MOD_HEALING, TOTAL_PCT, float(m_modifier.m_amount), apply);
}

void Aura::HandleShieldBlockValue(bool apply, bool Real)
{
    BaseModType modType = FLAT_MOD;
    if(m_modifier.m_auraname == SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT)
        modType = PCT_MOD;

    if(m_target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_target)->HandleBaseModValue(SHIELD_BLOCK_VALUE, modType, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraRetainComboPoints(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *target = (Player*)m_target;

    // combo points was added in SPELL_EFFECT_ADD_COMBO_POINTS handler
    // remove only if aura expire by time (in case combo points amount change aura removed without combo points lost)
    if( !apply && Real && m_duration==0 && target->GetComboTarget())
        if(Unit* unit = ObjectAccessor::GetUnit(*m_target,target->GetComboTarget()))
            target->AddComboPoints(unit, -m_modifier.m_amount);
}

void Aura::HandleModUnattackable( bool Apply, bool Real )
{
    if(Real && Apply)
        m_target->CombatStop();

    m_target->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE,Apply);
}

void Aura::HandleSpiritOfRedemption( bool apply, bool Real )
{
    if(!Real)
        return;

    // prepare spirit state
    if(apply)
    {
        if(m_target->GetTypeId()==TYPEID_PLAYER)
        {
            // disable breath/etc timers
            ((Player*)m_target)->StopMirrorTimers();

            // set stand state (expected in this form)
            if(!m_target->IsStandState())
            {
                m_target->RemoveFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT);
                ((Player*)m_target)->SetStandState(PLAYER_STATE_NONE);
            }

        }

        m_target->SetHealth(1);
    }
    // die at aura end
    else
        m_target->DealDamage(m_target, m_target->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_NORMAL, GetSpellProto(), false);
}

void Aura::CleanupTriggeredSpells()
{
    uint32 tSpellId = m_spellProto->EffectTriggerSpell[GetEffIndex()];
    if(!tSpellId)
        return;

    SpellEntry const* tProto = sSpellStore.LookupEntry(tSpellId);
    if(!tProto)
        return;

    if(GetDuration(tProto) != -1)
        return;

    m_target->RemoveAurasDueToSpell(tSpellId);
}
