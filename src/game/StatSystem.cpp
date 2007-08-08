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

#include "Unit.h"
#include "Player.h"
#include "Pet.h"
#include "Creature.h"
#include "SharedDefines.h"


/*#######################################
########                         ########
########   PLAYERS STAT SYSTEM   ########
########                         ########
#######################################*/

bool Player::UpdateStats(Stats stat)
{
    if(stat > STAT_SPIRIT)
        return false;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = GetTotalStatValue(stat);

    SetStat(stat, int32(value));

    if(stat == STAT_STAMINA || stat == STAT_INTELLECT)
    {
        Pet *pet = GetPet();
        if(pet)
            pet->UpdateStats(stat);
    }

    switch(stat)
    {
    case STAT_STRENGTH:
        UpdateAttackPowerAndDamage();
        break;
    case STAT_AGILITY:
        UpdateArmor();
        UpdateAttackPowerAndDamage(true);
        if(getClass() == CLASS_ROGUE || getClass() == CLASS_HUNTER)
            UpdateAttackPowerAndDamage();

        UpdateAllCritPercentages();
        UpdateDodgePercentage();
        break;

    case STAT_STAMINA:   UpdateMaxHealth(); break;
    case STAT_INTELLECT: 
        UpdateMaxPower(POWER_MANA);
        UpdateAllSpellCritChances();
        break;

    case STAT_SPIRIT:                       break;

    default:
        break;
    }

     return true;
}

bool Player::UpdateAllStats()
{
    for (int i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        float value = GetTotalStatValue(Stats(i));
        SetStat(Stats(i), (int32)value);
    }

    UpdateAttackPowerAndDamage();
    UpdateAttackPowerAndDamage(true);
    UpdateArmor();
    UpdateMaxHealth();

    for(int i = POWER_MANA; i < MAX_POWERS; i++)
        UpdateMaxPower(Powers(i));

    UpdateAllCritPercentages();
    UpdateAllSpellCritChances();
    UpdateDefenseBonusesMod();

    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
        UpdateResistances(i);

    return true;
}

void Player::UpdateResistances(uint32 school)
{
    if(school > SPELL_SCHOOL_NORMAL)
    {
         float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));
         SetResistance(SpellSchools(school), int32(value));

         Pet *pet = GetPet();
         if(pet)
             pet->UpdateResistances(school);
    }
    else
        UpdateArmor();
}

void Player::UpdateArmor()
{
    float value = 0.0f;
    UnitMods unitMod = UNIT_MOD_ARMOR;

    value  = GetModifierValue(unitMod, BASE_VALUE) + GetStat(STAT_AGILITY) * 2.0f;
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE);
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetArmor(int32(value));

    Pet *pet = GetPet();
    if(pet)
        pet->UpdateArmor();
}

void Player::UpdateMaxHealth()
{
    UnitMods unitMod = UNIT_MOD_HEALTH;
    float stamina = GetStat(STAT_STAMINA) - GetCreateStat(STAT_STAMINA);

    float value   = GetModifierValue(unitMod, BASE_VALUE) + GetCreateHealth();
    value  *= GetModifierValue(unitMod, BASE_PCT);
    value  += GetModifierValue(unitMod, TOTAL_VALUE) + stamina * 10.0f;
    value  *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxHealth((uint32)value);
}

void Player::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);
    float addValue = (power == POWER_MANA) ? GetStat(STAT_INTELLECT) - GetCreateStat(STAT_INTELLECT) : 0.0f;

    float value  = GetModifierValue(unitMod, BASE_VALUE) + GetCreatePowers(power);
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE) +  addValue * 15.0f;
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxPower(power, uint32(value));
}

void Player::UpdateAttackPowerAndDamage(bool ranged )
{
    float val2 = 0.0f;
    float level = float(getLevel());

    UnitMods unitMod = ranged ? UNIT_MOD_ATTACK_POWER_RANGED : UNIT_MOD_ATTACK_POWER;

    uint16 index = UNIT_FIELD_ATTACK_POWER;
    uint16 index_mod = UNIT_FIELD_ATTACK_POWER_MODS;
    uint16 index_mult = UNIT_FIELD_ATTACK_POWER_MULTIPLIER;

    if(ranged)
    {
        index = UNIT_FIELD_RANGED_ATTACK_POWER;
        index_mod = UNIT_FIELD_RANGED_ATTACK_POWER_MODS;
        index_mult = UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER;

        switch(getClass())
        {
        case CLASS_HUNTER: val2 = level * 2.0f + GetStat(STAT_AGILITY) - 10.0f;    break;
        case CLASS_ROGUE:  val2 = level        + GetStat(STAT_AGILITY) - 10.0f;    break;
        case CLASS_WARRIOR:val2 = level        + GetStat(STAT_AGILITY) - 20.0f;    break;
        default:           val2 = 0.0f;                                            break;
        }
    }
    else
    {
        switch(getClass())
        {
        case CLASS_WARRIOR: val2 = level*3.0f + GetStat(STAT_STRENGTH)*2.0f - 20.0f;        break;
        case CLASS_PALADIN: val2 = GetStat(STAT_STRENGTH)*2.0f - 20.0f;                     break;
        case CLASS_ROGUE:   val2 = GetStat(STAT_STRENGTH) + GetStat(STAT_AGILITY) - 20.0f;  break;
        case CLASS_HUNTER:  val2 = GetStat(STAT_STRENGTH) + GetStat(STAT_AGILITY) - 20.0f;  break;
        case CLASS_SHAMAN:  val2 = level*2.0f + GetStat(STAT_STRENGTH)*2.0f - 20.0f;        break;
        case CLASS_DRUID:
            switch(m_form)
            {
                case FORM_CAT:
                    val2 = level*2 + GetStat(STAT_STRENGTH)*2 - 20 + GetStat(STAT_AGILITY); break;
                case FORM_BEAR:
                case FORM_DIREBEAR:
                    val2 = level*3 + GetStat(STAT_STRENGTH)*2 - 20; break;
                default:
                    val2 = GetStat(STAT_STRENGTH)*2 - 20; break;
            }
            break;
        case CLASS_MAGE:    val2 = GetStat(STAT_STRENGTH) - 10.0f; break;
        case CLASS_PRIEST:  val2 = GetStat(STAT_STRENGTH) - 10.0f; break;
        case CLASS_WARLOCK: val2 = GetStat(STAT_STRENGTH) - 10.0f; break;
        }
    }

    SetModifierValue(unitMod, BASE_VALUE, val2);

    float base_attPower  = GetModifierValue(unitMod, BASE_VALUE) * GetModifierValue(unitMod, BASE_PCT);
    float attPowerMod = GetModifierValue(unitMod, TOTAL_VALUE);
    float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    SetUInt32Value(index, (uint32)base_attPower);            //UNIT_FIELD_(RANGED)_ATTACK_POWER field
    SetUInt32Value(index_mod, (uint32)attPowerMod);          //UNIT_FIELD_(RANGED)_ATTACK_POWER_MODS field
    SetFloatValue(index_mult, attPowerMultiplier);   //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field

    //automatically update weapon damage after attack power modification
    if(ranged)
    {
        UpdateDamagePhysical(RANGED_ATTACK);

        Pet *pet = GetPet(); //update pet's AP
        if(pet)
            pet->UpdateAttackPowerAndDamage();
    }
    else
    {
        UpdateDamagePhysical(BASE_ATTACK);
        if(CanDualWield() && haveOffhandWeapon()) //allow update offhand damage only if player knows DualWield Spec and has equipped offhand weapon 
            UpdateDamagePhysical(OFF_ATTACK);
    } 
}

void Player::UpdateDamagePhysical(WeaponAttackType attType)
{
    UnitMods unitMod = UNIT_MOD_DAMAGE_MAINHAND;
    UnitMods attPower = UNIT_MOD_ATTACK_POWER;

    uint16 index1 = UNIT_FIELD_MINDAMAGE;
    uint16 index2 = UNIT_FIELD_MAXDAMAGE;

    switch(attType)
    {
    case BASE_ATTACK: break;
    case OFF_ATTACK:
        unitMod = UNIT_MOD_DAMAGE_OFFHAND;
        index1 = UNIT_FIELD_MINOFFHANDDAMAGE;
        index2 = UNIT_FIELD_MAXOFFHANDDAMAGE;
        break;
    case RANGED_ATTACK:
        unitMod = UNIT_MOD_DAMAGE_RANGED;
        attPower = UNIT_MOD_ATTACK_POWER_RANGED;
        index1 = UNIT_FIELD_MINRANGEDDAMAGE;
        index2 = UNIT_FIELD_MAXRANGEDDAMAGE;
        break;
    }

    float att_speed = float(GetAttackTime(attType))/1000.0f;

    float base_value  = GetModifierValue(unitMod, BASE_VALUE) + GetTotalAttackPowerValue(attType)/ 14.0f * att_speed;
    float base_pct    = GetModifierValue(unitMod, BASE_PCT);
    float total_value = GetModifierValue(unitMod, TOTAL_VALUE);
    float total_pct   = GetModifierValue(unitMod, TOTAL_PCT);
    
    float weapon_mindamage = GetWeaponDamageRange(attType, MINDAMAGE);
    float weapon_maxdamage = GetWeaponDamageRange(attType, MAXDAMAGE);
    
    if(!IsUseEquipedWeapon()) //check if player is druid and in cat or bear forms
    {
        weapon_mindamage = 0.9 * GetTotalAuraModValue(attPower)/ 14.0f * att_speed;
        weapon_maxdamage = 1.1 * GetTotalAuraModValue(attPower)/ 14.0f * att_speed;
    }
    else if(attType == RANGED_ATTACK) //add ammo DPS to ranged damage
    {
        weapon_mindamage += GetAmmoDPS() * att_speed;
        weapon_maxdamage += GetAmmoDPS() * att_speed;
    }

    float mindamage = ((base_value + weapon_mindamage) * base_pct + total_value) * total_pct ;
    float maxdamage = ((base_value + weapon_maxdamage) * base_pct + total_value) * total_pct ;

    SetStatFloatValue(index1, mindamage);
    SetStatFloatValue(index2, maxdamage);
}

void Player::UpdateDefenseBonusesMod()
{
    UpdateBlockPercentage();
    UpdateParryPercentage();
    UpdateDodgePercentage();
}

void Player::UpdateBlockPercentage()
{
    BaseModGroup modGroup = BLOCK_PERCENTAGE;

    float chance = 5 - (getLevel()*5 - GetPureDefenseSkillValue()) * 0.04;
    chance = chance < 0 ? 0 : chance;

    SetBaseModValue(BLOCK_PERCENTAGE, PCT_MOD, chance);

    float value  = GetBaseModValue(modGroup, FLAT_MOD) + chance;
    value += float((GetDefenseSkillBonusValue())*0.04) + GetRatingBonusValue(PLAYER_FIELD_BLOCK_RATING);

    SetStatFloatValue(PLAYER_BLOCK_PERCENTAGE, value);
}

void Player::UpdateCritPercentage(WeaponAttackType attType)
{
    BaseModGroup modGroup = CRIT_PERCENTAGE;
    uint16 index = PLAYER_CRIT_PERCENTAGE;
    uint16 ratingIndex = PLAYER_FIELD_MELEE_CRIT_RATING;

    switch(attType)
    {
    case OFF_ATTACK:
        modGroup = OFFHAND_CRIT_PERCENTAGE;
        index = PLAYER_OFFHAND_CRIT_PERCENTAGE; 
        break;
    case RANGED_ATTACK: 
        modGroup = RANGED_CRIT_PERCENTAGE; 
        index = PLAYER_RANGED_CRIT_PERCENTAGE;
        ratingIndex = PLAYER_FIELD_RANGED_CRIT_RATING;
        break;
    case BASE_ATTACK:
    default:
        break;
    }

    float value = GetTotalPercentageModValue(modGroup) + GetRatingBonusValue(ratingIndex);

    SetStatFloatValue(index, value);
}

void Player::UpdateAllCritPercentages()
{
    GtChanceToMeleeCritBaseEntry const * gtCritBase = sGtChanceToMeleeCritBaseStore.LookupEntry(getClass() - 1);
    float base_crit = gtCritBase ? gtCritBase->base * 100 : 0;

    /*
    GtChanceToMeleeCritEntry     const * gtCritRate = sGtChanceToMeleeCritStore.LookupEntry((getClass() - 1) * 100 + getLevel() - 1);
    
    // values in sGtChanceToMeleeCritStore only until 100 level with 0 values - better use last level (70) at 2.1.3 for overflow level
    float rate_value = gtCritRate ? gtCritRate->ratio * 100 : sGtChanceToMeleeCritStore.LookupEntry((getClass() - 1) * 100 + 70 - 1)->ratio * 100;

    float value = base_crit + GetStat(STAT_AGILITY) * (0.04 + rate_value);
    */

    // temporary used old code until finding correct formula for GtChanceToMeleeCritEntry
    float classrate = 20.0f;

    switch(getClass())
    {
        case CLASS_WARRIOR: classrate = getLevel() > 60 ? 25 : 20; break;
        case CLASS_PALADIN: classrate = getLevel() > 60 ? 25 : 20; break;
        case CLASS_HUNTER:  classrate = getLevel() > 60 ? 40 : 33; break;
        case CLASS_ROGUE:   classrate = getLevel() > 60 ? 40 : 29; break;
        case CLASS_PRIEST:  classrate = getLevel() > 60 ? 25 : 20; break;
        case CLASS_SHAMAN:  classrate = getLevel() > 60 ? 25 : 20; break;
        case CLASS_MAGE:    classrate = getLevel() > 60 ? 25 : 20; break;
        case CLASS_WARLOCK: classrate = getLevel() > 60 ? 25 : 20; break;
        case CLASS_DRUID:   classrate = getLevel() > 60 ? 24.46f : 20; break;
    }

    float value = base_crit + GetStat(STAT_AGILITY) / classrate;

    SetBaseModValue(CRIT_PERCENTAGE, PCT_MOD, value);
    SetBaseModValue(OFFHAND_CRIT_PERCENTAGE, PCT_MOD, value);
    SetBaseModValue(RANGED_CRIT_PERCENTAGE, PCT_MOD, value);

    UpdateCritPercentage(BASE_ATTACK);
    UpdateCritPercentage(OFF_ATTACK);
    UpdateCritPercentage(RANGED_ATTACK);
}

void Player::UpdateParryPercentage()
{
    BaseModGroup modGroup = PARRY_PERCENTAGE;

    //pct mods for pct fields act like flat mods
    float value  = 5.0f + GetBaseModValue(modGroup, FLAT_MOD);
    value += float(GetDefenseSkillBonusValue()*0.04) + GetRatingBonusValue(PLAYER_FIELD_PARRY_RATING);

    SetStatFloatValue(PLAYER_PARRY_PERCENTAGE, value);
}

void Player::UpdateDodgePercentage()
{
    BaseModGroup modGroup = DODGE_PERCENTAGE;
    float classrate = 20.0f;
    float base_dodge = 0.0f;

    switch(getClass())
    {
    case CLASS_DRUID:   base_dodge = 0.75f; classrate = getLevel() > 60 ? 14.7 : 10.0f; break; //
    case CLASS_HUNTER:  base_dodge = 0.64f; classrate = getLevel() > 60 ? 40 : 26.5f; break;   // dunno exact values for case lvl > 60 :/
    case CLASS_ROGUE:   classrate = getLevel() > 60 ? 25 : 14.5f; break;                        //
    case CLASS_PALADIN: base_dodge = 0.75f; classrate = getLevel() > 60 ? 30 : 20; break;
    case CLASS_SHAMAN:  base_dodge = 1.75f; classrate = getLevel() > 60 ? 30 : 20; break;
    case CLASS_MAGE:    base_dodge = 3.25f; classrate = getLevel() > 60 ? 30 : 20; break;
    case CLASS_PRIEST:  base_dodge = 3.0f;  classrate = getLevel() > 60 ? 30 : 20; break;
    case CLASS_WARLOCK: base_dodge = 2.0f;  classrate = getLevel() > 60 ? 30 : 20; break;
    case CLASS_WARRIOR: 
    default:            classrate = getLevel() > 60 ? 30 : 20; break;
    }

    //pct mods for pct fields act like flat mods
    float value  = base_dodge + GetStat(STAT_AGILITY)/classrate; 
    value += float(GetDefenseSkillBonusValue()*0.04)+ GetBaseModValue(modGroup, FLAT_MOD);
    value += GetRatingBonusValue(PLAYER_FIELD_DODGE_RATING);

    SetStatFloatValue(PLAYER_DODGE_PERCENTAGE, value);
}

void Player::UpdateSpellCritChance(uint32 school)
{
    if(school == 0)
        return;

    BaseModGroup modGroup = BaseModGroup(SPELL_CRIT_PERCENTAGE + school);

    //Spell crit
    float base_value = GetBaseModValue(SPELL_CRIT_PERCENTAGE, PCT_MOD); //here we store basic % crit chance 
    base_value += GetRatingBonusValue(PLAYER_FIELD_SPELL_CRIT_RATING);  //for ALL spell schools

    float total_value = base_value + GetBaseModValue(modGroup, FLAT_MOD); 

    SetStatFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + school, total_value);
}

void Player::UpdateAllSpellCritChances()
{
    uint32 playerClass = getClass();

    static const struct
    {
        float base;
        float rate0, rate1;
    }
    crit_data[MAX_CLASSES] =
    {
        {0,0,10},                       //  0: unused
        {0,0,10},                       //  1: warrior
        {3.70,14.77,0.65},              //  2: paladin
        {0,0,10},                       //  3: hunter
        {0,0,10},                       //  4: rogue
        {2.97,10.03,0.82},              //  5: priest
        {0,0,10},                       //  6: unused
        {3.54,11.51,0.80},              //  7: shaman
        {3.70,14.77,0.65},              //  8: mage
        {3.18,11.30,0.82},              //  9: warlock
        {0,0,10},                       // 10: unused
        {3.33,12.41,0.79}               // 11: druid
    };

    float crit_ratio = crit_data[playerClass].rate0 + crit_data[playerClass].rate1 * getLevel();
    float base_value = 5 + GetStat(STAT_INTELLECT) / crit_ratio;

    SetBaseModValue(SPELL_CRIT_PERCENTAGE, PCT_MOD, base_value);

    SetStatFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1, base_value);

    for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; i++)
        UpdateSpellCritChance(i);
}

void Player::_ApplyAllStatBonuses()
{
    SetCanModifyStats(false);

    _ApplyAllAuraMods();
    _ApplyAllItemMods();

    SetCanModifyStats(true);

    UpdateAllStats();
}

void Player::_RemoveAllStatBonuses()
{
    SetCanModifyStats(false);

    _RemoveAllItemMods();
    _RemoveAllAuraMods();

    SetCanModifyStats(true);

    UpdateAllStats();
}

/*#######################################
########                         ########
########    MOBS STAT SYSTEM     ########
########                         ########
#######################################*/

bool Creature::UpdateStats(Stats stat)
{
     return true;
}

bool Creature::UpdateAllStats()
{
    UpdateMaxHealth();
    UpdateAttackPowerAndDamage();

    for(int i = POWER_MANA; i < MAX_POWERS; ++i)
        UpdateMaxPower(Powers(i));

    for(int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateResistances(i);

    return true;
}

void Creature::UpdateResistances(uint32 school)
{
    if(school > SPELL_SCHOOL_NORMAL)
    {
        float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));
        SetResistance(SpellSchools(school), int32(value));
    }
    else
        UpdateArmor();
}

void Creature::UpdateArmor()
{
    float value = GetTotalAuraModValue(UNIT_MOD_ARMOR);
    SetArmor(int32(value));
}

void Creature::UpdateMaxHealth()
{
    float value = GetTotalAuraModValue(UNIT_MOD_HEALTH);
    SetMaxHealth((uint32)value);
}

void Creature::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);

    float value  = GetTotalAuraModValue(unitMod);
    SetMaxPower(power, uint32(value));
}

void Creature::UpdateAttackPowerAndDamage(bool ranged)
{
    if(ranged)
        return;

    //automatically update weapon damage after attack power modification
    UpdateDamagePhysical(BASE_ATTACK); 
}

void Creature::UpdateDamagePhysical(WeaponAttackType attType)
{
    if(attType > BASE_ATTACK)
        return;

    UnitMods unitMod = UNIT_MOD_DAMAGE_MAINHAND;

    float att_speed = float(GetAttackTime(BASE_ATTACK))/1000.0f;

    float base_value  = GetModifierValue(unitMod, BASE_VALUE) + GetTotalAttackPowerValue(attType)/ 14.0f * att_speed;
    float base_pct    = GetModifierValue(unitMod, BASE_PCT);
    float total_value = GetModifierValue(unitMod, TOTAL_VALUE);
    float total_pct   = GetModifierValue(unitMod, TOTAL_PCT);
    
    float weapon_mindamage = GetWeaponDamageRange(BASE_ATTACK, MINDAMAGE);
    float weapon_maxdamage = GetWeaponDamageRange(BASE_ATTACK, MAXDAMAGE);

    float mindamage = ((base_value + weapon_mindamage) * base_pct + total_value) * total_pct ;
    float maxdamage = ((base_value + weapon_maxdamage) * base_pct + total_value) * total_pct ;

    SetStatFloatValue(UNIT_FIELD_MINDAMAGE, mindamage);
    SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, maxdamage);
}

/*#######################################
########                         ########
########    PETS STAT SYSTEM     ########
########                         ########
#######################################*/

bool Pet::UpdateStats(Stats stat)
{
    if(stat > STAT_SPIRIT)
        return false;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = GetTotalStatValue(stat);

    Unit *owner = GetOwner();
    if ( stat == STAT_STAMINA )
    {
        if(owner)
            value += float(owner->GetStat(stat)) * 0.3f;
    }
    else if ( stat == STAT_INTELLECT && getPetType() == SUMMON_PET ) //warlock's and mage's pets gain 30% of owner's intellect
    {
        if(owner && (owner->getClass() == CLASS_WARLOCK || owner->getClass() == CLASS_MAGE) )
            value += float(owner->GetStat(stat)) * 0.3f;
    }  

    SetStat(stat, int32(value));

    switch(stat)
    {
    case STAT_STRENGTH:         UpdateAttackPowerAndDamage();        break;
    case STAT_AGILITY:          UpdateArmor();                       break;
    case STAT_STAMINA:          UpdateMaxHealth();                   break;
    case STAT_INTELLECT:        UpdateMaxPower(POWER_MANA);          break;
    case STAT_SPIRIT:
    default:
        break;
    }

     return true;
}

bool Pet::UpdateAllStats()
{
    for (int i = STAT_STRENGTH; i < MAX_STATS; i++)
        UpdateStats(Stats(i));

    for(int i = POWER_MANA; i < MAX_POWERS; i++)
        UpdateMaxPower(Powers(i));

    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
        UpdateResistances(i);

    return true;
}

void Pet::UpdateResistances(uint32 school)
{
    if(school > SPELL_SCHOOL_NORMAL)
    {
        float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));

        if(getPetType() == HUNTER_PET) //add resistance bonus only for hunter pets
        {
            Unit *owner = GetOwner();
            if(owner)
                value += float(owner->GetResistance(SpellSchools(school))) * 0.4f;
        }

        SetResistance(SpellSchools(school), int32(value));
    }
    else
        UpdateArmor();
}

void Pet::UpdateArmor()
{
    float value = 0.0f;
    float bonus_armor = 0.0f;
    UnitMods unitMod = UNIT_MOD_ARMOR;

    if(getPetType() == HUNTER_PET ) //hunter pets gain 35% of owner's armor value
    {
        Unit *owner = GetOwner();
        if(owner)
            bonus_armor = 0.35f * float(owner->GetArmor());
    }

    value  = GetModifierValue(unitMod, BASE_VALUE) + GetStat(STAT_AGILITY) * 2.0f;
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE) + bonus_armor;
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetArmor(int32(value));
}

void Pet::UpdateMaxHealth()
{
    UnitMods unitMod = UNIT_MOD_HEALTH;
    float stamina = GetStat(STAT_STAMINA) - GetCreateStat(STAT_STAMINA);

    float value   = GetModifierValue(unitMod, BASE_VALUE) + GetCreateHealth();
    value  *= GetModifierValue(unitMod, BASE_PCT);
    value  += GetModifierValue(unitMod, TOTAL_VALUE) + stamina * 10.0f;
    value  *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxHealth((uint32)value);
}

void Pet::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);
    float addValue = (power == POWER_MANA) ? GetStat(STAT_INTELLECT) - GetCreateStat(STAT_INTELLECT) : 0.0f;

    float value  = GetModifierValue(unitMod, BASE_VALUE) + GetCreatePowers(power);
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE) +  addValue * 15.0f;
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxPower(power, uint32(value));
}

void Pet::UpdateAttackPowerAndDamage(bool ranged)
{
    if(ranged)
        return;

    float val = 0.0f;
    float bonusAP = 0.0f;
    UnitMods unitMod = UNIT_MOD_ATTACK_POWER;
    
    if(GetEntry() == 412) // imp's attack power
        val = GetStat(STAT_STRENGTH) - 10.0;
    else
        val = 2 * GetStat(STAT_STRENGTH) - 20.0;

    Unit* owner = GetOwner();
    if( owner )
    {
        if(getPetType() == HUNTER_PET) //hunter pets benefit from owner's attack power
        {
            bonusAP = owner->GetTotalAttackPowerValue(RANGED_ATTACK) * 0.22f;
            SetBonusDamage( int32(owner->GetTotalAttackPowerValue(RANGED_ATTACK) * 0.125f));
        }
        else if(getPetType() == SUMMON_PET && owner->getClass() == CLASS_WARLOCK)   //demons benefit from warlocks shadow or fire damage
        {
            uint32 fire, shadow, maximum;
            fire  = owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE) - owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_FIRE);
            shadow = owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW) - owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_SHADOW);
            maximum  = (fire > shadow) ? fire : shadow;
            if(maximum < 0)
                maximum = 0;
            SetBonusDamage( int32(maximum * 0.15f));
            bonusAP = maximum * 0.57f;
        }
        else if(getPetType() == SUMMON_PET && owner->getClass() == CLASS_MAGE)      //water elementals benefit from mage's frost damage
        {
            uint32 frost = owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FROST) - owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_FROST);
            if(frost < 0)
                frost = 0;
            SetBonusDamage( int32(frost * 0.4f));
        }
    }
    
    SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, val + bonusAP);

    //in BASE_VALUE of UNIT_MOD_ATTACK_POWER for creatures we store data of meleeattackpower field in DB
    float base_attPower  = GetModifierValue(unitMod, BASE_VALUE) * GetModifierValue(unitMod, BASE_PCT);
    float attPowerMod = GetModifierValue(unitMod, TOTAL_VALUE);
    float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    SetUInt32Value(UNIT_FIELD_ATTACK_POWER, (uint32)base_attPower);         //UNIT_FIELD_(RANGED)_ATTACK_POWER field
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS, (uint32)attPowerMod);      //UNIT_FIELD_(RANGED)_ATTACK_POWER_MODS field
    SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, attPowerMultiplier);  //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field

    //automatically update weapon damage after attack power modification
    UpdateDamagePhysical(BASE_ATTACK); 
}

void Pet::UpdateDamagePhysical(WeaponAttackType attType)
{
    if(attType > BASE_ATTACK)
        return;

    UnitMods unitMod = UNIT_MOD_DAMAGE_MAINHAND;

    float att_speed = float(GetAttackTime(BASE_ATTACK))/1000.0f;

    float base_value  = GetModifierValue(unitMod, BASE_VALUE) + GetTotalAttackPowerValue(attType)/ 14.0f * att_speed;
    float base_pct    = GetModifierValue(unitMod, BASE_PCT);
    float total_value = GetModifierValue(unitMod, TOTAL_VALUE);
    float total_pct   = GetModifierValue(unitMod, TOTAL_PCT);
    
    float weapon_mindamage = GetWeaponDamageRange(BASE_ATTACK, MINDAMAGE);
    float weapon_maxdamage = GetWeaponDamageRange(BASE_ATTACK, MAXDAMAGE);

    float mindamage = ((base_value + weapon_mindamage) * base_pct + total_value) * total_pct ;
    float maxdamage = ((base_value + weapon_maxdamage) * base_pct + total_value) * total_pct ;

    SetStatFloatValue(UNIT_FIELD_MINDAMAGE, mindamage);
    SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, maxdamage);
}
