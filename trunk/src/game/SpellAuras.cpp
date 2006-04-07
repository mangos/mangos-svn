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

pAuraHandler AuraHandler[TOTAL_AURAS]={
&Aura::HandleNULL,//SPELL_AURA_NONE
&Aura::HandleNULL,//SPELL_AURA_BIND_SIGHT
&Aura::HandleNULL,//SPELL_AURA_MOD_POSSESS = 2,                   
&Aura::HandlePeriodicDamage,//SPELL_AURA_PERIODIC_DAMAGE = 3,  
&Aura::HandleNULL,////missing 4
&Aura::HandleNULL,//SPELL_AURA_MOD_CONFUSE = 5,                   
&Aura::HandleNULL,//SPELL_AURA_MOD_CHARM = 6,                     
&Aura::HandleNULL,//SPELL_AURA_MOD_FEAR = 7,                      
&Aura::HandlePeriodicHeal,//SPELL_AURA_PERIODIC_HEAL = 8,                 
&Aura::HandleNULL,//SPELL_AURA_MOD_ATTACKSPEED = 9,      
&Aura::HandleNULL,//SPELL_AURA_MOD_THREAT = 10, 
&Aura::HandleNULL,//SPELL_AURA_MOD_TAUNT = 11,
&Aura::HandleAuraModStun,//SPELL_AURA_MOD_STUN = 12,   
&Aura::HandleNULL,//SPELL_AURA_MOD_DAMAGE_DONE = 13,     
&Aura::HandleNULL,//SPELL_AURA_MOD_DAMAGE_TAKEN = 14,             
&Aura::HandleAuraDamageShield,//SPELL_AURA_DAMAGE_SHIELD = 15,             
&Aura::HandleModStealth,//SPELL_AURA_MOD_STEALTH = 16,                  
&Aura::HandleNULL,//SPELL_AURA_MOD_DETECT = 17,                   
&Aura::HandleNULL,//SPELL_AURA_MOD_INVISIBILITY = 18,             
&Aura::HandleNULL,//SPELL_AURA_MOD_INVISIBILITY_DETECTION = 19, 
&Aura::HandleNULL,//missing 20,
&Aura::HandleNULL,//missing 21
&Aura::HandleAuraModResistance,//SPELL_AURA_MOD_RESISTANCE = 22,               
&Aura::HandlePeriodicTriggerSpell,//SPELL_AURA_PERIODIC_TRIGGER_SPELL = 23,       
&Aura::HandleNULL,//SPELL_AURA_PERIODIC_ENERGIZE = 24,            
&Aura::HandleNULL,//SPELL_AURA_MOD_PACIFY = 25,                   
&Aura::HandleAuraModRoot,//SPELL_AURA_MOD_ROOT = 26,                     
&Aura::HandleAuraModSilence,//SPELL_AURA_MOD_SILENCE = 27,                  
&Aura::HandleNULL,//SPELL_AURA_REFLECT_SPELLS = 28,               
&Aura::HandleAuraModStat,//SPELL_AURA_MOD_STAT = 29,      
&Aura::HandleAuraModSkill,//SPELL_AURA_MOD_SKILL = 30,                    
&Aura::HandleAuraModIncreaseSpeed,//SPELL_AURA_MOD_INCREASE_SPEED = 31,           
&Aura::HandleAuraModIncreaseMountedSpeed,//SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED = 32,   
&Aura::HandleAuraModDecreaseSpeed,//SPELL_AURA_MOD_DECREASE_SPEED = 33,           
&Aura::HandleAuraModIncreaseHealth,//SPELL_AURA_MOD_INCREASE_HEALTH = 34,          
&Aura::HandleAuraModIncreaseEnergy,//SPELL_AURA_MOD_INCREASE_ENERGY = 35,          
&Aura::HandleAuraModShapeshift,//SPELL_AURA_MOD_SHAPESHIFT = 36,           
&Aura::HandleAuraModEffectImmunity,//SPELL_AURA_EFFECT_IMMUNITY = 37,              
&Aura::HandleAuraModStateImmunity,//SPELL_AURA_STATE_IMMUNITY = 38,               
&Aura::HandleAuraModSchoolImmunity,//SPELL_AURA_SCHOOL_IMMUNITY = 39,              
&Aura::HandleAuraModDmgImmunity,//SPELL_AURA_DAMAGE_IMMUNITY = 40,              
&Aura::HandleAuraModDispelImmunity,//SPELL_AURA_DISPEL_IMMUNITY = 41,              
&Aura::HandleAuraProcTriggerSpell,//SPELL_AURA_PROC_TRIGGER_SPELL = 42,           
&Aura::HandleAuraProcTriggerDamage,//SPELL_AURA_PROC_TRIGGER_DAMAGE = 43,          
&Aura::HandleAuraTracCreatures,//SPELL_AURA_TRACK_CREATURES = 44,              
&Aura::HandleAuraTracResources,//SPELL_AURA_TRACK_RESOURCES = 45,              
&Aura::HandleNULL,//SPELL_AURA_MOD_PARRY_SKILL = 46, obsolete?              
&Aura::HandleAuraModParryPercent,//SPELL_AURA_MOD_PARRY_PERCENT = 47,            
&Aura::HandleNULL,//SPELL_AURA_MOD_DODGE_SKILL = 48, obsolete?             
&Aura::HandleAuraModDodgePercent,//SPELL_AURA_MOD_DODGE_PERCENT = 49,
&Aura::HandleNULL,//SPELL_AURA_MOD_BLOCK_SKILL = 50, obsolete?              
&Aura::HandleAuraModBlockPercent,//SPELL_AURA_MOD_BLOCK_PERCENT = 51,            
&Aura::HandleAuraModCritPercent,//SPELL_AURA_MOD_CRIT_PERCENT = 52,             
&Aura::HandleNULL,//SPELL_AURA_PERIODIC_LEECH = 53,               
&Aura::HandleNULL,//SPELL_AURA_MOD_HIT_CHANCE = 54,               
&Aura::HandleNULL,//SPELL_AURA_MOD_SPELL_HIT_CHANCE = 55,         
&Aura::HandleAuraTransform,//SPELL_AURA_TRANSFORM = 56,                    
&Aura::HandleNULL,//SPELL_AURA_MOD_SPELL_CRIT_CHANCE = 57,        
&Aura::HandleNULL,//SPELL_AURA_MOD_INCREASE_SWIM_SPEED = 58,      
&Aura::HandleNULL,//SPELL_AURA_MOD_DAMAGE_DONE_CREATURE = 59,  
&Aura::HandleNULL,//SPELL_AURA_MOD_PACIFY_SILENCE = 60,           
&Aura::HandleAuraModScale,//SPELL_AURA_MOD_SCALE = 61,                    
&Aura::HandleNULL,//SPELL_AURA_PERIODIC_HEALTH_FUNNEL = 62,       
&Aura::HandleNULL,//SPELL_AURA_PERIODIC_MANA_FUNNEL = 63,         
&Aura::HandleNULL,//SPELL_AURA_PERIODIC_MANA_LEECH = 64,          
&Aura::HandleNULL,//SPELL_AURA_MOD_CASTING_SPEED = 65,            
&Aura::HandleNULL,//SPELL_AURA_FEIGN_DEATH = 66,                  
&Aura::HandleNULL,//SPELL_AURA_MOD_DISARM = 67,                   
&Aura::HandleNULL,//SPELL_AURA_MOD_STALKED = 68,                     
&Aura::HandleAuraSchoolAbsorb,//SPELL_AURA_SCHOOL_ABSORB = 69, 
&Aura::HandleNULL,//SPELL_AURA_EXTRA_ATTACKS = 70,                
&Aura::HandleNULL,//SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL = 71, 
&Aura::HandleNULL,//SPELL_AURA_MOD_POWER_COST = 72,               
&Aura::HandleNULL,//SPELL_AURA_MOD_POWER_COST_SCHOOL = 73,	        
&Aura::HandleReflectSpellsSchool,//SPELL_AURA_REFLECT_SPELLS_SCHOOL = 74,        
&Aura::HandleNULL,//SPELL_AURA_MOD_LANGUAGE = 75,                 
&Aura::HandleNULL,//SPELL_AURA_FAR_SIGHT = 76,                    
&Aura::HandleNULL,//SPELL_AURA_MECHANIC_IMMUNITY = 77,            
&Aura::HandleAuraMounted,//SPELL_AURA_MOUNTED = 78,                      
&Aura::HandleNULL,//SPELL_AURA_MOD_DAMAGE_PERCENT_DONE = 79,  
&Aura::HandleNULL,//SPELL_AURA_MOD_PERCENT_STAT = 80,             
&Aura::HandleNULL,//SPELL_AURA_SPLIT_DAMAGE = 81,                 
&Aura::HandleWaterBreathing,//SPELL_AURA_WATER_BREATHING = 82,              
&Aura::HandleNULL,//SPELL_AURA_MOD_BASE_RESISTANCE = 83,          
&Aura::HandleModRegen,//SPELL_AURA_MOD_REGEN = 84,                    
&Aura::HandleModPowerRegen,//SPELL_AURA_MOD_POWER_REGEN = 85,              
&Aura::HandleNULL,//SPELL_AURA_CHANNEL_DEATH_ITEM = 86,           
&Aura::HandleNULL,//SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN = 87,     
&Aura::HandleNULL,//SPELL_AURA_MOD_PERCENT_REGEN = 88,            
&Aura::HandleNULL,//SPELL_AURA_PERIODIC_DAMAGE_PERCENT = 89,   
&Aura::HandleNULL,//SPELL_AURA_MOD_RESIST_CHANCE = 90,            
&Aura::HandleNULL,//SPELL_AURA_MOD_DETECT_RANGE = 91,             
&Aura::HandleNULL,//SPELL_AURA_PREVENTS_FLEEING = 92,             
&Aura::HandleNULL,//SPELL_AURA_MOD_UNATTACKABLE = 93,             
&Aura::HandleNULL,//SPELL_AURA_INTERRUPT_REGEN = 94,              
&Aura::HandleNULL,//SPELL_AURA_GHOST = 95,                        
&Aura::HandleNULL,//SPELL_AURA_SPELL_MAGNET = 96,                              
&Aura::HandleAuraManaShield,//SPELL_AURA_MANA_SHIELD = 97, 
&Aura::HandleNULL,//SPELL_AURA_MOD_SKILL_TALENT = 98,             
&Aura::HandleAuraModAttackPower,//SPELL_AURA_MOD_ATTACK_POWER = 99,       
&Aura::HandleNULL,//SPELL_AURA_AURAS_VISIBLE = 100,               
&Aura::HandleNULL,//SPELL_AURA_MOD_RESISTANCE_PCT = 101,          
&Aura::HandleNULL,//SPELL_AURA_MOD_CREATURE_ATTACK_POWER = 102,   
&Aura::HandleNULL,//SPELL_AURA_MOD_TOTAL_THREAT = 103,            
&Aura::HandleAuraWaterWalk,//SPELL_AURA_WATER_WALK = 104,                  
&Aura::HandleAuraFeatherFall,//SPELL_AURA_FEATHER_FALL = 105,                
&Aura::HandleNULL,//SPELL_AURA_HOVER = 106,                       
&Aura::HandleNULL,//SPELL_AURA_ADD_FLAT_MODIFIER = 107,           
&Aura::HandleNULL,//SPELL_AURA_ADD_PCT_MODIFIER = 108,            
&Aura::HandleNULL,//SPELL_AURA_ADD_TARGET_TRIGGER = 109,          
&Aura::HandleNULL,//SPELL_AURA_MOD_POWER_REGEN_PERCENT = 110,     
&Aura::HandleNULL,//SPELL_AURA_ADD_CASTER_HIT_TRIGGER = 111,      
&Aura::HandleNULL,//SPELL_AURA_OVERRIDE_CLASS_SCRIPTS = 112,      
&Aura::HandleNULL,//SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN = 113,     
&Aura::HandleNULL,//SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT = 114, 
&Aura::HandleNULL,//SPELL_AURA_MOD_HEALING = 115,                 
&Aura::HandleNULL,//SPELL_AURA_IGNORE_REGEN_INTERRUPT = 116,      
&Aura::HandleNULL,//SPELL_AURA_MOD_MECHANIC_RESISTANCE = 117,     
&Aura::HandleNULL,//SPELL_AURA_MOD_HEALING_PCT = 118,             
&Aura::HandleNULL,//SPELL_AURA_SHARE_PET_TRACKING = 119,          
&Aura::HandleNULL,//SPELL_AURA_UNTRACKABLE = 120,                 
&Aura::HandleNULL,//SPELL_AURA_EMPATHY = 121,                     
&Aura::HandleNULL,//SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT = 122,      
&Aura::HandleNULL,//SPELL_AURA_MOD_POWER_COST_PCT = 123,          
&Aura::HandleAuraModRangedAttackPower,//SPELL_AURA_MOD_RANGED_ATTACK_POWER = 124,     
&Aura::HandleNULL,//SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN = 125,      
&Aura::HandleNULL,//SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT = 126,  
&Aura::HandleNULL,//SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS = 127,
&Aura::HandleNULL,//SPELL_AURA_MOD_POSSESS_PET = 128,             
&Aura::HandleAuraModIncreaseSpeedAlways,//SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS = 129,   
&Aura::HandleNULL,//SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS = 130,    
&Aura::HandleNULL,//SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER = 131,
&Aura::HandleAuraModIncreaseEnergyPercent,//SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT = 132, 
&Aura::HandleAuraModIncreaseHealthPercent,//SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT = 133, 
&Aura::HandleNULL,//SPELL_AURA_MOD_MANA_REGEN_INTERRUPT = 134,    
&Aura::HandleNULL,//SPELL_AURA_MOD_HEALING_DONE = 135,            
&Aura::HandleNULL,//SPELL_AURA_MOD_HEALING_DONE_PERCENT = 136,    
&Aura::HandleNULL,//SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE = 137,   
&Aura::HandleNULL,//SPELL_AURA_MOD_HASTE = 138,                   
&Aura::HandleNULL,//SPELL_AURA_FORCE_REACTION = 139,              
&Aura::HandleNULL,//SPELL_AURA_MOD_RANGED_HASTE = 140,            
&Aura::HandleNULL,//SPELL_AURA_MOD_RANGED_AMMO_HASTE = 141,       
&Aura::HandleNULL,//SPELL_AURA_MOD_BASE_RESISTANCE_PCT = 142,     
&Aura::HandleAuraModResistanceExclusive,//SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE = 143,    
&Aura::HandleAuraSafeFall,//SPELL_AURA_SAFE_FALL = 144,                   
&Aura::HandleNULL,//SPELL_AURA_CHARISMA = 145,                    
&Aura::HandleNULL,//SPELL_AURA_PERSUADED = 146,                   
&Aura::HandleNULL,//SPELL_AURA_ADD_CREATURE_IMMUNITY = 147,       
&Aura::HandleNULL,//SPELL_AURA_RETAIN_COMBO_POINTS = 148,
&Aura::HandleNULL,//SPELL_AURA_RESIST_PUSHBACK	=	149	,//	Resist Pushback
&Aura::HandleNULL,//SPELL_AURA_MOD_SHIELD_BLOCK	=	150	,//	Mod Shield Block %
&Aura::HandleNULL,//SPELL_AURA_TRACK_STEALTHED	=	151	,//	Track Stealthed
&Aura::HandleNULL,//SPELL_AURA_MOD_DETECTED_RANGE	=	152	,//	Mod Detected Range
&Aura::HandleNULL,//SPELL_AURA_SPLIT_DAMAGE_FLAT	=	153	,//	Split Damage Flat
&Aura::HandleNULL,//SPELL_AURA_MOD_STEALTH_LEVEL	=	154	,//	Stealth Level Modifier
&Aura::HandleNULL,//SPELL_AURA_MOD_WATER_BREATHING	=	155	,//	Mod Water Breathing
&Aura::HandleNULL,//SPELL_AURA_MOD_REPUTATION_ADJUST	=	156	,//	Mod Reputation Gain
&Aura::HandleNULL//SPELL_AURA_PET_DAMAGE_MULTI	=	157	,//	Mod Pet Damage
};

void Aura::AddMod(uint8 t, int32 a, uint32 pt,uint32 miscValue, uint32 miscValue2)
{
	Modifier *newmod = new Modifier;
	newmod->m_auraname = t;
	newmod->m_amount   = a;
	newmod->m_miscvalue = miscValue;
	newmod->m_miscvalue2 = miscValue2;
	newmod->periodictime = pt;
	m_modList.push_back(newmod);
}

void Aura::Update(uint32 diff)
{
	if (m_duration > 0)
	{
		m_duration -= diff;
		if (m_duration < 0)
			m_duration = 0;
	}
}

void Aura::ApplyModifiers(bool apply)
{
	uint8 aura = 0;

	for (ModList::iterator j = m_modList.begin(); j != m_modList.end(); j++)
    {
		cmod = *j;
		aura = (*j)->m_auraname;
		if(aura<TOTAL_AURAS)
			(*this.*AuraHandler [aura])(apply);
	}
}

void Aura::_AddAura()
{
    if (!GetId())
		return;
 
    WorldPacket data;

    uint8 slot, i;

    slot = 0xFF;

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

    if (slot == 0xFF)
    {
        return;
    }

    m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURA + slot), GetId());

    uint8 flagslot = slot >> 3;
    uint32 value = m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot));
    value |= 0xFFFFFFFF & (AFLAG_SET << ((slot & 7) << 2));
    m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot), value);

    uint8 appslot = slot >> 1;

    if( m_target->GetTypeId() == TYPEID_PLAYER )
    {
        data.Initialize(SMSG_UPDATE_AURA_DURATION);
        data << (uint8)slot << (uint32)GetDuration();
        ((Player*)m_target)->GetSession()->SendPacket(&data);
    }

    SetAuraSlot( slot );
}


void Aura::_RemoveAura()
{   
    uint8 slot = GetAuraSlot();

    m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURA + slot), 0);

    uint8 flagslot = slot >> 3;

    uint32 value = m_target->GetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot));
    value &= 0xFFFFFFFF ^ (0xF << ((slot & 7) << 2));
    m_target->SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot), value);

    SetAuraSlot(0);
}


void Aura::HandleNULL(bool apply)
{
}

void HandleDOTEvent(void *obj)
{
	Aura *Aur = ((Aura*)obj);
	Aur->GetCaster()->PeriodicAuraLog(Aur->GetTarget(), Aur->GetSpellProto(), Aur->cmod);
}

void Aura::HandlePeriodicDamage(bool apply)
{
	if( apply )
		m_PeriodicEventId=AddEvent(&HandleDOTEvent,(void*)this,cmod->periodictime,true,true);
	else if(m_PeriodicEventId>0)
	{
		RemovePeriodicEvent(m_PeriodicEventId);
		m_PeriodicEventId=0;
	}
}

void HandleHealEvent(void *obj) 
{
	Aura *Aur = ((Aura*)obj);
	Aur->GetTarget()->PeriodicAuraLog(Aur->GetCaster(), Aur->GetSpellProto(), Aur->cmod);
}

void Aura::HandlePeriodicHeal(bool apply)
{
	AddEvent(&HandleHealEvent,(void*)this,cmod->periodictime,false,true);
}

void Aura::HandleAuraWaterWalk(bool apply)
{
	WorldPacket data;	
	apply ? data.Initialize(SMSG_MOVE_WATER_WALK) : data.Initialize(SMSG_MOVE_LAND_WALK);
    data << m_target->GetGUID();
    m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraFeatherFall(bool apply)
{
	WorldPacket data;
	apply ? data.Initialize(SMSG_MOVE_FEATHER_FALL)  : data.Initialize(SMSG_MOVE_NORMAL_FALL);
	data << m_target->GetGUID();
	m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraModStun(bool apply)
{
	if (apply) m_target->SetUInt64Value (UNIT_FIELD_TARGET, 0);
}


void Aura::HandleAuraModRangedAttackPower(bool apply)
{
	apply ? m_target->SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS,m_target->GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS) + cmod->m_amount) : 
			m_target->SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS,m_target->GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS) - cmod->m_amount);
}

void Aura::HandleAuraModIncreaseSpeedAlways(bool apply)
{
	WorldPacket data;	
	data.Initialize(MSG_MOVE_SET_RUN_SPEED);
	data << m_target->GetGUID();
	apply ? data << float(7.5+7.5/100*cmod->m_amount) : data << float(7.5);
	m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraModIncreaseEnergyPercent(bool apply)
{
	uint32 percent = cmod->m_amount;
	uint32 current = m_target->GetUInt32Value(UNIT_FIELD_POWER4);
	apply ? m_target->SetUInt32Value(UNIT_FIELD_POWER4,current+(current*percent)/100) : m_target->SetUInt32Value(UNIT_FIELD_POWER4,current-(current*100)/(100+percent));
}

void Aura::HandleAuraModIncreaseHealthPercent(bool apply)
{
	uint32 percent = cmod->m_amount;
	uint32 current = m_target->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
	apply ? m_target->SetUInt32Value(UNIT_FIELD_MAXHEALTH,current+(current*percent)/100) : m_target->SetUInt32Value(UNIT_FIELD_MAXHEALTH,current-(current*100)/(100+percent));
}
// FIX-ME!!
void HandleTriggerSpellEvent(void *obj) 
{
	Aura *Aur = ((Aura*)obj);
        /*                 
            SpellEntry *spellInfo = sSpellStore.LookupEntry( Aur->GetSpellPerTick() );

            if(!spellInfo)
            {
                sLog.outError("WORLD: unknown spell id %i\n", Aur->GetSpellPerTick());
                return;
            }
			
            Spell *spell = new Spell(Aur->GetTarget(), spellInfo, true, aff);
            SpellCastTargets targets;
            WorldPacket dump;
            dump.Initialize(0);
            dump << uint16(2) << GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT) << GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT+1);
            targets.read(&dump,this);

			spell->prepare(&targets);
        }*/

	/*else if(m_spellProto->EffectApplyAuraName[i] == 23)
	{
		unitTarget->tmpAura->SetPeriodicTriggerSpell(m_spellProto->EffectTriggerSpell[i],m_spellProto->EffectAmplitude[i]);
	}*/
}

void Aura::HandlePeriodicTriggerSpell(bool apply)
{
	AddEvent(&HandleTriggerSpellEvent,(void*)this,cmod->periodictime,false,true);
}

void Aura::HandleAuraModResistanceExclusive(bool apply)
{
	uint32 index = 0;
	uint32 index2 = 0;
	switch(cmod->m_miscvalue)
	{
                case 0:{
                    index = UNIT_FIELD_ARMOR;
                    cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE;
                }break;
                case 1:{
                    index = UNIT_FIELD_RESISTANCES_01;
                    cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01;
                }break;
                case 2:{
                    index = UNIT_FIELD_RESISTANCES_02;
                    cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_02 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_02;
                }break;
                case 3:{
                    index = UNIT_FIELD_RESISTANCES_03;
                    cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_03 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_03;
                }break;
                case 4:{
                    index = UNIT_FIELD_RESISTANCES_04;
                    cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_04 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_04;
                }break;
                case 5:{
                    index = UNIT_FIELD_RESISTANCES_05;
                    cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_05 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_05;
                }break;
                case 6:{
                    index = UNIT_FIELD_RESISTANCES_06;
                    cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06;
                }
                break;
                case -1:{
                    index = UNIT_FIELD_RESISTANCES_06;
                    cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06;
                    for(uint32 i=0;i<6;i++)
                        if(apply){
                            m_target->SetUInt32Value(index+i,m_target->GetUInt32Value(index+i)+cmod->m_amount);
                            if(m_target->GetTypeId() == TYPEID_PLAYER)
                                m_target->SetUInt32Value(index2+i,m_target->GetUInt32Value(index2+i)+cmod->m_amount);
                        }else{
                            m_target->SetUInt32Value(index+i,m_target->GetUInt32Value(index+i)-cmod->m_amount);
                            if(m_target->GetTypeId() == TYPEID_PLAYER)
                                m_target->SetUInt32Value(index2+i,m_target->GetUInt32Value(index2+i)-cmod->m_amount);
                        }
                    return;
                }break;
                default:{
                    sLog.outString("WARNING: Misc Value for SPELL_AURA_MOD_STAT not valid");
                    return;
                }break;
            }

            if(apply)
            {
                m_target->SetUInt32Value(index,m_target->GetUInt32Value(index)+cmod->m_amount);
                if(m_target->GetTypeId() == TYPEID_PLAYER)
                    m_target->SetUInt32Value(index2,m_target->GetUInt32Value(index2)+cmod->m_amount);
            }
            else
            {
                m_target->SetUInt32Value(index,m_target->GetUInt32Value(index)-cmod->m_amount);
                if(m_target->GetTypeId() == TYPEID_PLAYER)
                    m_target->SetUInt32Value(index2,m_target->GetUInt32Value(index2)-cmod->m_amount);
            }
}

void Aura::HandleAuraSafeFall(bool apply)
{	
	WorldPacket data;
	apply ? data.Initialize(SMSG_MOVE_FEATHER_FALL) : data.Initialize(SMSG_MOVE_NORMAL_FALL);
	data << m_target->GetGUID();
	m_target->SendMessageToSet(&data,true);
}
// FIX-ME!!!
void Aura::HandleAuraDamageShield(bool apply)
{
	/*if(apply)
	{
		DamageShield* ds = new DamageShield();
		ds->m_caster = GetCaster();
		ds->m_damage = cmod->m_amount;
		ds->m_spellId = GetId();
		m_damageShields.push_back((*ds));
	}
	else
	{
		for(std::list<struct DamageShield>::iterator i = m_damageShields.begin();i != m_damageShields.end();i++)
			if(i->m_spellId == GetId() && i->m_caster == GetCaster())
			{
				m_damageShields.erase(i);
				break;
			}
	}*/
}

void Aura::HandleModStealth(bool apply)
{
	apply ? m_target->SetFlag(UNIT_FIELD_BYTES_1, 0x21E0000 ) : m_target->RemoveFlag(UNIT_FIELD_BYTES_1, 0x21E0000 );// stealth state
	apply ? m_target->m_stealth = GetId() :  m_target->m_stealth = 0;  
}

void Aura::HandleAuraModResistance(bool apply)
{
	uint16 index = 0;
	uint16 index2 = 0;
	switch(cmod->m_miscvalue)
	{
		case 0:
			index = UNIT_FIELD_RESISTANCES_01;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01;
			break;
		case 1:
			index = UNIT_FIELD_ARMOR;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE;
			break;
		case 2:
			index = UNIT_FIELD_RESISTANCES_02;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_02 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_02;
			break;
		case 3:
			index = UNIT_FIELD_RESISTANCES_03;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_03 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_03;
			break;
		case 4:
			index = UNIT_FIELD_RESISTANCES_04;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_04 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_04;
			break;
		case 5:
			index = UNIT_FIELD_RESISTANCES_05;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_05 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_05;
			break;
		case 6:
			index = UNIT_FIELD_RESISTANCES_06;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06;
			break;
		case -1:
		{
			index = UNIT_FIELD_RESISTANCES_06;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06;
			for(uint32 i=0;i<5;i++)
				if(apply)
				{
					m_target->SetUInt32Value(index+i,m_target->GetUInt32Value(index+i)+cmod->m_amount);
					if(m_target->GetTypeId() == TYPEID_PLAYER)
						m_target->SetUInt32Value(index2+i,m_target->GetUInt32Value(index2+i)+cmod->m_amount);
				}
				else
				{
					m_target->SetUInt32Value(index+i,m_target->GetUInt32Value(index+i)-cmod->m_amount);
					if(m_target->GetTypeId() == TYPEID_PLAYER)
					m_target->SetUInt32Value(index2+i,m_target->GetUInt32Value(index2+i)-cmod->m_amount);
				}
			return;
		}break;
		default:
			sLog.outString("WARNING: Misc Value for SPELL_AURA_MOD_STAT not valid");
			return;
			break;
	}

	if(apply)
	{
		m_target->SetUInt32Value(index,m_target->GetUInt32Value(index)+cmod->m_amount);
		if(m_target->GetTypeId() == TYPEID_PLAYER)
			m_target->SetUInt32Value(index2,m_target->GetUInt32Value(index2)+cmod->m_amount);
	}
	else
	{
		m_target->SetUInt32Value(index,m_target->GetUInt32Value(index)-cmod->m_amount);
		if(m_target->GetTypeId() == TYPEID_PLAYER)
			m_target->SetUInt32Value(index2,m_target->GetUInt32Value(index2)-cmod->m_amount);
	}
}

void Aura::HandleAuraModRoot(bool apply)
{
	WorldPacket data;
	apply ? data.Initialize(MSG_MOVE_ROOT) : data.Initialize(MSG_MOVE_UNROOT);
    data << m_target->GetGUID();
    m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraModSilence(bool apply)
{
	apply ? m_target->m_silenced = true : m_target->m_silenced = false;
}

void Aura::HandleAuraModStat(bool apply)
{
	uint16 index = 0;
	uint16 index2 = 0;
	switch(cmod->m_miscvalue)
	{
		case 0:
			index = UNIT_FIELD_STR;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_POSSTAT0 : index2 = PLAYER_FIELD_NEGSTAT0;
			break;
		case 1:
			index = UNIT_FIELD_AGILITY;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_POSSTAT1 : index2 = PLAYER_FIELD_NEGSTAT1;
			break;
		case 2:
			index = UNIT_FIELD_STAMINA;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_POSSTAT2 : index2 = PLAYER_FIELD_NEGSTAT2;
			break;
		case 3:
			index = UNIT_FIELD_IQ;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_POSSTAT3 : index2 = PLAYER_FIELD_NEGSTAT3;
			break;
		case 4:
			index = UNIT_FIELD_SPIRIT;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_POSSTAT4 : index2 = PLAYER_FIELD_NEGSTAT4;
			break;
		case -1:
		{
			index = UNIT_FIELD_STR;
			cmod->m_miscvalue2 == 0 ? index2 = PLAYER_FIELD_POSSTAT0 : index2 = PLAYER_FIELD_NEGSTAT0;
			for(uint32 i=0;i<5;i++)
				if(apply)
				{
					m_target->SetUInt32Value(index+i,m_target->GetUInt32Value(index+i)+cmod->m_amount);
					if(m_target->GetTypeId() == TYPEID_PLAYER)
						m_target->SetUInt32Value(index2+i,m_target->GetUInt32Value(index2+i)+cmod->m_amount);
				}
				else
				{
					m_target->SetUInt32Value(index+i,m_target->GetUInt32Value(index+i)-cmod->m_amount);
					if(m_target->GetTypeId() == TYPEID_PLAYER)
					m_target->SetUInt32Value(index2+i,m_target->GetUInt32Value(index2+i)-cmod->m_amount);
				}
			return;
		}break;
		default:
			sLog.outString("WARNING: Misc Value for SPELL_AURA_MOD_STAT not valid");
			return;
			break;
	}
	
	if(apply)
	{
		m_target->SetUInt32Value(index,m_target->GetUInt32Value(index)+cmod->m_amount);
		if(m_target->GetTypeId() == TYPEID_PLAYER)
		m_target->SetUInt32Value(index2,m_target->GetUInt32Value(index2)+cmod->m_amount);
	}
	else
	{
		m_target->SetUInt32Value(index,m_target->GetUInt32Value(index)-cmod->m_amount);
		if(m_target->GetTypeId() == TYPEID_PLAYER)
			m_target->SetUInt32Value(index2,m_target->GetUInt32Value(index2)-cmod->m_amount);
	}
	
}

void Aura::HandleAuraModIncreaseSpeed(bool apply)
{
	WorldPacket data;
	m_target->m_speed += 7.0f/100*cmod->m_amount;
	data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
	data << uint8(0xFF);
	data << m_target->GetGUID();
	apply ? data << float(7.5+7.5/100*cmod->m_amount) : data << float(7.5);
	m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraModIncreaseMountedSpeed(bool apply)
{
	WorldPacket data;
	data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
	data << uint8(0xFF);
	data << m_target->GetGUID();
	apply ? data << float(7.5+7.5/100*cmod->m_amount) : data << float(7.5);
	m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraModDecreaseSpeed(bool apply)
{
	WorldPacket data;
	m_target->m_speed -= 7.0f/100*cmod->m_amount;
	data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
	data << uint8(0xFF);
	data << m_target->GetGUID();
	apply ? data << float(7.5-7.5/100*cmod->m_amount) : data << float(7.5);
	m_target->SendMessageToSet(&data,true);
}

void Aura::HandleAuraModIncreaseHealth(bool apply)
{
	uint32 newValue;
	newValue = m_target->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
	apply ? newValue += cmod->m_amount : newValue -= cmod->m_amount;
	m_target->SetUInt32Value(UNIT_FIELD_MAXHEALTH,newValue);
}

void Aura::HandleAuraModIncreaseEnergy(bool apply)
{
	uint32 powerField = 23;
	uint8 powerType = (uint8)(m_target->GetUInt32Value(UNIT_FIELD_BYTES_0) >> 24);
	if(powerType == 0)                    
		powerField = UNIT_FIELD_POWER1;
	else if(powerType == 1)               
		powerField = UNIT_FIELD_POWER2;
    else if(powerType == 3)               
		powerField = UNIT_FIELD_POWER4;

	uint32 newValue = m_target->GetUInt32Value(powerType);
	apply ? newValue += cmod->m_amount : newValue -= cmod->m_amount;
	m_target->SetUInt32Value(powerType,newValue);
}
// FIX-ME PWEEZEE!!
void Aura::HandleAuraModShapeshift(bool apply)
{
	//Aura* tmpAur;
	uint32 spellId;
	switch(cmod->m_miscvalue)
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
		case FORM_AMBIENT:
			spellId = 0;
			break;
		case FORM_GHOUL:
			spellId = 0;
			break;
		case FORM_DIREBEAR:
			spellId = 9635;
			break;
		case FORM_CREATUREBEAR:
			spellId = 2882;
			break;
		case FORM_GHOSTWOLF:
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
			spellId = 3025;
			break;
		default:
			sLog.outString("Unknown Shapeshift Type");
			break;
	}
            
	SpellEntry *spellInfo = sSpellStore.LookupEntry( spellId );

	if(!spellInfo)
	{
		sLog.outError("WORLD: unknown spell id %i\n", spellId);
		return;
	}

	/*tmpAff = new Affect(spellInfo,GetDuration(),GetCaster());

            if(tmpAff)
            {
                SetCoAffect(tmpAff);
                AddAffect(tmpAff);
            }*/
}

void Aura::HandleAuraModEffectImmunity(bool apply)
{
	apply ? m_target->m_immuneToEffect = cmod->m_miscvalue : m_target->m_immuneToEffect = 0;
}	
void Aura::HandleAuraModStateImmunity(bool apply)
{
	apply ? m_target->m_immuneToState = cmod->m_miscvalue : m_target->m_immuneToState = 0;
}	
void Aura::HandleAuraModSchoolImmunity(bool apply)
{
	apply ? m_target->m_immuneToSchool = cmod->m_miscvalue : m_target->m_immuneToSchool = 0;
}	
void Aura::HandleAuraModDmgImmunity(bool apply)
{
	apply ? m_target->m_immuneToDmg = cmod->m_miscvalue : m_target->m_immuneToDmg = 0;
}	
void Aura::HandleAuraModDispelImmunity(bool apply)
{
	apply ? m_target->m_immuneToDispel = cmod->m_miscvalue : m_target->m_immuneToDispel = 0;
}	
// FIX-ME PLS!!!
void Aura::HandleAuraProcTriggerSpell(bool apply)
{
/*	uint32 i=0;
            for(i=0;i<2;i++)
                if(GetSpellProto()->EffectApplyAuraName[i] == cmod->m_auraname)
                    break;
            if(apply)
            {
                ProcTriggerSpell* pts = new ProcTriggerSpell();
                pts->caster = GetCaster();
                pts->spellId = GetSpellProto()->EffectTriggerSpell[i];
                pts->trigger = GetSpellProto()->EffectBasePoints[i];
                pts->procChance = GetSpellProto()->procChance;
                pts->procFlags = GetSpellProto()->procFlags;
                GetSpellProto()->procCharges == 0 ? pts->procCharges = 0
                    : pts->procCharges = GetSpellProto()->procCharges;
                m_procSpells.push_back((*pts));
            }
            else
            {
                for(std::list<struct ProcTriggerSpell>::iterator itr = m_procSpells.begin();itr != m_procSpells.end();itr++)
                    if(itr->spellId == GetId() && itr->caster == GetCaster())
                {
                    m_procSpells.erase(itr);
                    break;
                }
            }*/
}
// FIX-ME PLS!!!
void Aura::HandleAuraProcTriggerDamage(bool apply)
{
	/*if(apply)
            {
                DamageShield* ds = new DamageShield();
                ds->m_caster = GetCaster();
                ds->m_damage = cmod->m_amount;
                ds->m_spellId = GetId();
                m_damageShields.push_back((*ds));
            }
            else
            {
                for(std::list<struct DamageShield>::iterator i = m_damageShields.begin();i != m_damageShields.end();i++)
                    if(i->m_spellId == GetId() && i->m_caster == GetCaster())
                {
                    m_damageShields.erase(i);
                    break;
                }
            }*/
}

void Aura::HandleAuraTracCreatures(bool apply)
{
	m_target->SetUInt32Value(PLAYER_TRACK_CREATURES, apply ? cmod->m_miscvalue : 0 );
}

void Aura::HandleAuraTracResources(bool apply)
{
	m_target->SetUInt32Value(PLAYER_TRACK_RESOURCES, apply ? ((uint32)1)<<(cmod->m_miscvalue-1): 0 );
}

void Aura::HandleAuraModParryPercent(bool apply)
{
	float current = m_target->GetFloatValue(PLAYER_PARRY_PERCENTAGE);
	apply ? m_target->SetFloatValue(PLAYER_PARRY_PERCENTAGE,current+cmod->m_amount) : m_target->SetFloatValue(PLAYER_PARRY_PERCENTAGE,current-cmod->m_amount);
}

void Aura::HandleAuraModDodgePercent(bool apply)
{
	float current = m_target->GetFloatValue(PLAYER_DODGE_PERCENTAGE);
	apply ? m_target->SetFloatValue(PLAYER_DODGE_PERCENTAGE,current+cmod->m_amount) : m_target->SetFloatValue(PLAYER_DODGE_PERCENTAGE,current-cmod->m_amount);
}
void Aura::HandleAuraModBlockPercent(bool apply)
{
	float current = m_target->GetFloatValue(PLAYER_BLOCK_PERCENTAGE);
	apply ? m_target->SetFloatValue(PLAYER_BLOCK_PERCENTAGE,current+cmod->m_amount) : m_target->SetFloatValue(PLAYER_BLOCK_PERCENTAGE,current-cmod->m_amount);
}

void Aura::HandleAuraModCritPercent(bool apply)
{
	float current = m_target->GetFloatValue(PLAYER_CRIT_PERCENTAGE);
	apply ? m_target->SetFloatValue(PLAYER_CRIT_PERCENTAGE,current+cmod->m_amount) : m_target->SetFloatValue(PLAYER_CRIT_PERCENTAGE,current-cmod->m_amount);
}

void Aura::HandleAuraModScale(bool apply)
{
	float current = m_target->GetFloatValue(OBJECT_FIELD_SCALE_X);
	apply ? m_target->SetFloatValue(OBJECT_FIELD_SCALE_X,current+current/100*10) : m_target->SetFloatValue(OBJECT_FIELD_SCALE_X,current-current/110*100);
}

void Aura::HandleAuraMounted(bool apply)
{
	if(apply)
	{
		CreatureInfo* ci = objmgr.GetCreatureTemplate(cmod->m_miscvalue);
		if(!ci)return;
		uint32 displayId = ci->DisplayID;
		if(displayId != 0)
		{
			m_target->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID , displayId);
			m_target->SetUInt32Value( UNIT_FIELD_FLAGS , 0x002000 );
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
	apply ? m_target->waterbreath = true : m_target->waterbreath = false;
}

void Aura::HandleModRegen(bool apply) // eating
{
	apply ? m_target->SetFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT) : m_target->RemoveFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT);
}

void Aura::HandleModPowerRegen(bool apply) // drinking  
{
	apply ? m_target->SetFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT) : m_target->RemoveFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT);
}

void Aura::HandleAuraModAttackPower(bool apply)
{
	m_target->SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS,m_target->GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS)+ apply?(cmod->m_amount):(-cmod->m_amount));
}

void Aura::HandleAuraTransform(bool apply)
{
	uint32 id=GetId();
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

	}
}
// FIX-ME PLS!!!
void Aura::HandleAuraManaShield(bool apply)
{
/*	if(apply)
    {
		if(m_damageManaShield){
			delete m_damageManaShield;
			m_damageManaShield = NULL;
		}
        m_damageManaShield = new DamageManaShield();
		m_damageManaShield->m_spellId = GetId();
		m_damageManaShield->m_modType = cmod->m_auraname;
		m_damageManaShield->m_totalAbsorb = cmod->m_amount;
		m_damageManaShield->m_currAbsorb = 0;
		m_damageManaShield->m_schoolAbsorb = NORMAL_DAMAGE;
    }else{
		if(m_damageManaShield){
			delete m_damageManaShield;
			m_damageManaShield = NULL;
		}
    }*/
}
// FIX-ME PLS!!!
void Aura::HandleAuraSchoolAbsorb(bool apply)
{
	/*if(apply)
    {
		if(m_damageManaShield){
			delete m_damageManaShield;
			m_damageManaShield = NULL;
		}
        m_damageManaShield = new DamageManaShield();
		m_damageManaShield->m_spellId = GetId();
		m_damageManaShield->m_modType = cmod->m_auraname;
		m_damageManaShield->m_totalAbsorb = cmod->m_amount;
		m_damageManaShield->m_currAbsorb = 0;
		m_damageManaShield->m_schoolAbsorb = GetSpellProto()->EffectBasePoints[0];
    }else{
		if(m_damageManaShield){
			delete m_damageManaShield;
			m_damageManaShield = NULL;
		}
    }*/
}

void Aura::HandleReflectSpellsSchool(bool apply)
{	
  apply ? m_target->m_ReflectSpellSchool = cmod->m_miscvalue : m_target->m_ReflectSpellSchool = 0;
  apply ? m_target->m_ReflectSpellPerc = cmod->m_amount : m_target->m_ReflectSpellPerc = 0;	
}

void Aura::HandleAuraModSkill(bool apply)
{
	 SpellEntry* prot=GetSpellProto();
	
	 ((Player*)m_target)->ModifySkillBonus(prot->EffectMiscValue[0],
		 (apply ? (prot->EffectBasePoints[0]+1): (-(prot->EffectBasePoints[0]+1))));
}
