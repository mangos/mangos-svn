/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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
#include "Log.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Unit.h"
#include "QuestDef.h"
#include "Player.h"
#include "Creature.h"
#include "Spell.h"
#include "Stats.h"
#include "Group.h"
#include "Affect.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "Formulas.h"

#include <math.h>
#define DEG2RAD (M_PI/180.0)
#define M_PI       3.14159265358979323846

Unit::Unit() : Object()
{
    m_objectType |= TYPE_UNIT;
    m_objectTypeId = TYPEID_UNIT;

    m_attackTimer = 0;

    m_state = 0;
    m_deathState = ALIVE;
    m_currentSpell = NULL;
    m_meleeSpell = false;
    m_addDmgOnce = 0;
    m_TotemSlot1 = m_TotemSlot2 = m_TotemSlot3 = m_TotemSlot4  = 0;
    m_aura = NULL;
    m_auraCheck = 2000;
    m_removeAuraTimer = 4;
    tmpAffect = NULL;
    m_silenced = false;
}


Unit::~Unit()
{
}


void Unit::Update( uint32 p_time )
{
    if(p_time > m_auraCheck)
    {
        m_auraCheck = 2000;
        _UpdateAura();
    }else
    m_auraCheck -= p_time;

    _UpdateSpells( p_time );

    if(m_regenTimer > 0)
    {
        if(p_time >= m_regenTimer)
            m_regenTimer = 0;
        else
            m_regenTimer -= p_time;
    }

    if(m_attackTimer > 0)
    {
        if(p_time >= m_attackTimer)
            m_attackTimer = 0;
        else
            m_attackTimer -= p_time;
    }
}


void Unit::setAttackTimer(uint32 time)
{
    time ? m_attackTimer = time : m_attackTimer = GetUInt32Value(UNIT_FIELD_BASEATTACKTIME);
}


bool Unit::canReachWithAttack(Unit *pVictim) const
{
    float reach = GetFloatValue(UNIT_FIELD_COMBATREACH);
    float radius = GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS);
    float distance = GetDistanceSq(pVictim);

    if (distance > ((reach + radius)*(reach + radius)))
        return false;

    return true;
}


void Unit::DealDamage(Unit *pVictim, uint32 damage, uint32 procFlag)
{
    DEBUG_LOG("DealDamageStart");
    uint32 health = pVictim->GetUInt32Value(UNIT_FIELD_HEALTH );
    if (health <= damage && pVictim->isAlive())
    {
        DEBUG_LOG("DealDamage: victim just died");
        if(pVictim->GetTypeId() == TYPEID_UNIT)
	{
	    
            ((Creature*)pVictim)->generateLoot();
	}
	
	
	if ((this->GetTypeId() == TYPEID_PLAYER) && (pVictim->GetTypeId() == TYPEID_PLAYER))
	{
	    ((Player*)this)->CalculateHonor((Player*)pVictim);
	}
		
	

        DEBUG_LOG("DealDamageAffects");
        pVictim->RemoveAllAffects();

        
        pVictim->setDeathState(JUST_DIED);

	

        
        uint64 attackerGuid, victimGuid;
        attackerGuid = GetGUID();
        victimGuid = pVictim->GetGUID();

        DEBUG_LOG("DealDamageAttackStop");
        pVictim->smsg_AttackStop(attackerGuid);

        
	
	
        DEBUG_LOG("DealDamageHealth1");
        pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);

        
        DEBUG_LOG("DealDamageHealth2");
        pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
        pVictim->RemoveFlag(UNIT_FIELD_FLAGS, 0x00080000);

        if (pVictim->GetTypeId() != TYPEID_PLAYER)
        {
            DEBUG_LOG("DealDamageNotPlayer");
            pVictim->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 1);
        }

	
        if (GetTypeId() == TYPEID_PLAYER)
        {
            DEBUG_LOG("DealDamageIsPlayer");
            uint32 xp = MaNGOS::XP::Gain(static_cast<Player *>(this), pVictim); 

            
            uint32 entry = 0;
            if (pVictim->GetTypeId() != TYPEID_PLAYER)
                entry = pVictim->GetUInt32Value(OBJECT_FIELD_ENTRY );

            
            Group *pGroup = objmgr.GetGroupByLeader(((Player*)this)->GetGroupLeader());
            if (pGroup)
            {
                DEBUG_LOG("DealDamageInGroup");
                xp /= pGroup->GetMembersCount();
                for (uint32 i = 0; i < pGroup->GetMembersCount(); i++)
                {
		    Player *pGroupGuy = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(i));
                    pGroupGuy->GiveXP(xp, victimGuid);

                    if (pVictim->GetTypeId() != TYPEID_PLAYER)
                        pGroupGuy->KilledMonster(entry, victimGuid);
                }
            }
            else
            {
                DEBUG_LOG("DealDamageNotInGroup");
                
                ((Player*)this)->GiveXP(xp, victimGuid);

                if (pVictim->GetTypeId() != TYPEID_PLAYER)
                    ((Player*)this)->KilledMonster(entry, victimGuid);
            }
        }
        else
        {
            DEBUG_LOG("DealDamageIsCreature");
            smsg_AttackStop(victimGuid);
            RemoveFlag(UNIT_FIELD_FLAGS, 0x00080000);
            addStateFlag(UF_TARGET_DIED);
        }
    }
    else
    {
        DEBUG_LOG("DealDamageAlive");
        pVictim->SetUInt32Value(UNIT_FIELD_HEALTH , health - damage);

        
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
        {
	    ((Creature *)pVictim)->AI().DamageInflict(this, damage);
            if( getClass() == 1 )
            {
                CalcRage(damage);
            }
        }
        else
        {
            
            ((Player*)pVictim)->addStateFlag(UF_ATTACKING);

            if( (((Player*)pVictim)->getClass()) == 1 )
            {
                ((Player*)pVictim)->CalcRage(damage);
            }
        }

        
        for(std::list<struct DamageShield>::iterator i = pVictim->m_damageShields.begin();i != pVictim->m_damageShields.end();i++)
        {
            pVictim->SpellNonMeleeDamageLog(this,i->m_spellId,i->m_damage);
        }
        

    }
    DEBUG_LOG("DealDamageEnd");
}




void Unit::CastSpell(Unit* caster,Unit* Victim, uint32 spellId, bool triggered)
{
    
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId );

    if(!spellInfo)
    {
        Log::getSingleton( ).outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    Spell *spell = new Spell(caster, spellInfo, triggered, 0);
    WPAssert(spell);

    SpellCastTargets targets;
    targets.m_unitTarget = Victim->GetGUID();
    spell->prepare(&targets);
    m_canMove = false;
}


void Unit::CalcRage( uint32 damage )
{
    
    uint32 oldRage = GetUInt32Value(UNIT_FIELD_POWER2); uint32 maxRage = GetUInt32Value(UNIT_FIELD_MAXPOWER2);
    uint32 Rage = oldRage + damage*2;               
    if(Rage == oldRage) Rage = oldRage + 1;         
    if (Rage > maxRage) Rage = maxRage;             
    SetUInt32Value(UNIT_FIELD_POWER2, Rage);
}


void Unit::RegenerateAll()
{
    
    
    if (m_regenTimer != 0)
        return;
    uint32 regenDelay = 2000;
    
    if (!(m_state & UF_ATTACKING))                
    {
        Regenerate( UNIT_FIELD_HEALTH, UNIT_FIELD_MAXHEALTH, true );
        Regenerate( UNIT_FIELD_POWER2, UNIT_FIELD_MAXPOWER2, false );
    }
    
    Regenerate( UNIT_FIELD_POWER4, UNIT_FIELD_MAXPOWER4, true );
    Regenerate( UNIT_FIELD_POWER1, UNIT_FIELD_MAXPOWER1, true );

    m_regenTimer = regenDelay;

}



void Unit::Regenerate(uint16 field_cur, uint16 field_max, bool switch_)
{
    uint32 curValue = GetUInt32Value(field_cur);    uint32 maxValue = GetUInt32Value(field_max);
    if (switch_)
    {
        if (curValue >= maxValue)           return;
    }
    else
    {
        if (curValue == 0)          return;
    }   float HealthIncreaseRate = sWorld.getRate(RATE_HEALTH); float ManaIncreaseRate = sWorld.getRate(RATE_POWER1);   float RageIncreaseRate = sWorld.getRate(RATE_POWER2);   float EnergyIncreaseRate = sWorld.getRate(RATE_POWER3);
    uint16 Spirit = GetUInt32Value(UNIT_FIELD_SPIRIT);
    uint16 Class = getClass();

    if( HealthIncreaseRate <= 0 ) HealthIncreaseRate = 1;
    if( ManaIncreaseRate <= 0 ) ManaIncreaseRate = 1;
    if( RageIncreaseRate <= 0 ) RageIncreaseRate = 1;
    if( EnergyIncreaseRate <= 0 ) EnergyIncreaseRate = 1;



    uint32 oldCurValue = curValue;

    uint32 addvalue = 0;

    switch (field_cur)
    {
        
        case UNIT_FIELD_HEALTH:{
            switch (Class)
            {
                case 1: {
                    addvalue = uint32(((Spirit*0.80) * HealthIncreaseRate));
                    break;              }
               case 2: {
                    addvalue = uint32(((Spirit*0.25) * HealthIncreaseRate));
                    break;              }
                case 3: {
                    addvalue = uint32(((Spirit*0.25) * HealthIncreaseRate));
                    break;              }
               case 4:   {
                    addvalue = uint32(((Spirit*0.50) * HealthIncreaseRate));
                    break;              }
               case 5:  {
                    addvalue = uint32(((Spirit*0.10) * HealthIncreaseRate));
                    break;              }
               case 7:  {
                    addvalue = uint32((((Spirit*0.11)+9) * HealthIncreaseRate));
                    break;              }
               case 8:    {
                   curValue+=uint32(((Spirit*0.10) * HealthIncreaseRate));
                    break;              }
               case 9: {
                   addvalue = uint32(((Spirit*0.11) * HealthIncreaseRate));
                   break;               }
               case 11: {
                    
                    addvalue = uint32(((Spirit+10) * HealthIncreaseRate));
                    break;              }
               default: {
                   addvalue = uint32(((Spirit+10) * HealthIncreaseRate));
                      break;            }
            }
            break; }
        
        case UNIT_FIELD_POWER1: {
            switch (Class)
            {
                case 2:                {
                    addvalue = uint32((((Spirit/4)+8) * ManaIncreaseRate));
                    break;              }
                case 3:                 {
                    addvalue = uint32((((Spirit/4)+11) * ManaIncreaseRate));
                    break;              }
                case 5:                 {
                    addvalue = uint32((((Spirit/4)+13) * ManaIncreaseRate));
                    break;              }
                case 7:                 {
                    addvalue = uint32((((Spirit/5)+17) * ManaIncreaseRate));
                    break;              }
                case 8:               {
                    addvalue = uint32((((Spirit/4)+11) * ManaIncreaseRate));
                    break;              }
                case 9:                {
                    addvalue = uint32((((Spirit/4)+8) * ManaIncreaseRate));
                    break;              }
                case 11:                {
                    addvalue = uint32((((Spirit/5)+15) * ManaIncreaseRate));
                    break;              }
                default:               {
                    addvalue = uint32((Spirit * ManaIncreaseRate));
                    break;                  }
           }
           break;}
        case UNIT_FIELD_POWER2: {
            
            addvalue = uint32((1 * RageIncreaseRate));
            break;}

        case UNIT_FIELD_POWER4: {
            addvalue = uint32(20);
            break;}

        default:{
            break;}
    }
    if(addvalue == 0) addvalue = 1; 
    if (switch_)
    {
        
        if((getStandState() == 1) || (getStandState() == 3))
        {
            
            addvalue *= 2;
        }
        curValue += addvalue;
        if (curValue > maxValue) curValue = maxValue;
        SetUInt32Value(field_cur, curValue);
    }
    else
    {
        curValue -= addvalue;
        if (curValue > maxValue) curValue = 0;
        SetUInt32Value(field_cur, curValue);
    }
}





void Unit::SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage)
{
    uint32 procFlag = 0;

    if(!this || !pVictim)
        return;
    if(!this->isAlive() || !pVictim->isAlive())
        return;
    Log::getSingleton( ).outDetail("SpellNonMeleeDamageLog: %u %X attacked %u %X for %u dmg inflicted by %u",
        GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage, spellID);
    WorldPacket data;
    data.Initialize(SMSG_SPELLNONMELEEDAMAGELOG);
    data << pVictim->GetGUID();
    data << this->GetGUID();
    data << spellID;
    data << damage;
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    SendMessageToSet(&data,true);
    DealDamage(pVictim, damage, procFlag);
}


void Unit::PeriodicAuraLog(Unit *pVictim, uint32 spellID, uint32 damage, uint32 damageType)
{
    uint32 procFlag = 0;
    if(!this || !pVictim)
        return;
    if(!this->isAlive() || !pVictim->isAlive())
        return;
    Log::getSingleton( ).outDetail("PeriodicAuraLog: %u %X attacked %u %X for %u dmg inflicted by %u",
        GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage, spellID);

    WorldPacket data;
    data.Initialize(SMSG_PERIODICAURALOG);
    data << pVictim->GetGUID();
    data << this->GetGUID();
    data << spellID;
    data << uint32(1);                            

    data << uint32(3);                            
    data << damage;
    data << damageType;
    data << uint32(0);
    SendMessageToSet(&data,true);

    DealDamage(pVictim, damage, procFlag);
}



#define EMOTE_ONESHOT_NONE 0 
#define EMOTE_ONESHOT_TALK 1 
#define EMOTE_ONESHOT_BOW  2 
#define EMOTE_ONESHOT_WAVE 3 
#define EMOTE_ONESHOT_CHEER  4 
#define EMOTE_ONESHOT_EXCLAMATION 5 
#define EMOTE_ONESHOT_QUESTION 6 
#define EMOTE_ONESHOT_EAT  7 
#define EMOTE_STATE_DANCE  10 
#define EMOTE_ONESHOT_LAUGH  11 
#define EMOTE_STATE_SLEEP  12 
#define EMOTE_STATE_SIT  13 
#define EMOTE_ONESHOT_RUDE 14 
#define EMOTE_ONESHOT_ROAR 15 
#define EMOTE_ONESHOT_KNEEL  16 
#define EMOTE_ONESHOT_KISS 17 
#define EMOTE_ONESHOT_CRY  18 
#define EMOTE_ONESHOT_CHICKEN  19 
#define EMOTE_ONESHOT_BEG  20 
#define EMOTE_ONESHOT_APPLAUD  21 
#define EMOTE_ONESHOT_SHOUT  22 
#define EMOTE_ONESHOT_FLEX 23 
#define EMOTE_ONESHOT_SHY  24 
#define EMOTE_ONESHOT_POINT  25 
#define EMOTE_STATE_STAND  26 
#define EMOTE_STATE_READYUNARMED  27 
#define EMOTE_STATE_WORK 28 
#define EMOTE_STATE_POINT  29 
#define EMOTE_STATE_NONE 30 
#define EMOTE_ONESHOT_WOUND  33 
#define EMOTE_ONESHOT_WOUNDCRITICAL 34 
#define EMOTE_ONESHOT_ATTACKUNARMED 35 
#define EMOTE_ONESHOT_ATTACK1H 36 
#define EMOTE_ONESHOT_ATTACK2HTIGHT 37 
#define EMOTE_ONESHOT_ATTACK2HLOOSE 38 
#define EMOTE_ONESHOT_PARRYUNARMED  39 
#define EMOTE_ONESHOT_PARRYSHIELD 43 
#define EMOTE_ONESHOT_READYUNARMED  44 
#define EMOTE_ONESHOT_READY1H  45 
#define EMOTE_ONESHOT_READYBOW 48 
#define EMOTE_ONESHOT_SPELLPRECAST  50 
#define EMOTE_ONESHOT_SPELLCAST 51 
#define EMOTE_ONESHOT_BATTLEROAR  53 
#define EMOTE_ONESHOT_SPECIALATTACK1H 54 
#define EMOTE_ONESHOT_KICK 60 
#define EMOTE_ONESHOT_ATTACKTHROWN  61 
#define EMOTE_STATE_STUN 64 
#define EMOTE_STATE_DEAD 65 
#define EMOTE_ONESHOT_SALUTE 66 
#define EMOTE_STATE_KNEEL  68 
#define EMOTE_STATE_USESTANDING 69 
#define EMOTE_ONESHOT_WAVE_NOSHEATHE 70 
#define EMOTE_ONESHOT_CHEER_NOSHEATHE  71 
#define EMOTE_ONESHOT_EAT_NOSHEATHE 92 
#define EMOTE_STATE_STUN_NOSHEATHE  93 
#define EMOTE_ONESHOT_DANCE  94 
#define EMOTE_ONESHOT_SALUTE_NOSHEATH  113 
#define EMOTE_STATE_USESTANDING_NOSHEATHE  133 
#define EMOTE_ONESHOT_LAUGH_NOSHEATHE  153 
#define EMOTE_STATE_WORK_NOSHEATHE  173 
#define EMOTE_STATE_SPELLPRECAST  193 
#define EMOTE_ONESHOT_READYRIFLE  213 
#define EMOTE_STATE_READYRIFLE 214 
#define EMOTE_STATE_WORK_NOSHEATHE_MINING  233 
#define EMOTE_STATE_WORK_NOSHEATHE_CHOPWOOD 234 
#define EMOTE_zzOLDONESHOT_LIFTOFF  253 
#define EMOTE_ONESHOT_LIFTOFF  254 
#define EMOTE_ONESHOT_YES  273 
#define EMOTE_ONESHOT_NO 274 
#define EMOTE_ONESHOT_TRAIN  275 
#define EMOTE_ONESHOT_LAND 293 
#define EMOTE_STATE_READY1H  333 
#define EMOTE_STATE_AT_EASE  313 
#define EMOTE_STATE_SPELLKNEELSTART 353 
#define EMOTE_STATE_SUBMERGED  373 
#define EMOTE_ONESHOT_SUBMERGE 374

void Unit::HandleEmoteCommand(uint32 anim_id)
{
    WorldPacket data;

    data.Initialize( SMSG_EMOTE );
    data << anim_id << GetGUID();
    WPAssert(data.size() == 12);
    
    SendMessageToSet(&data, true);
}

void Unit::DoAttackDamage(Unit *pVictim, uint32 damage, uint32 blocked_amount, uint32 damageType, uint32 hitInfo, uint32 victimState)
{
    if (GetUnitCriticalChance() * 655.36f >= (uint16)irand(0, 512))
    {
        hitInfo = 0xEA;
        damage *= 2;
        damageType = 1;

        pVictim->HandleEmoteCommand(EMOTE_ONESHOT_WOUNDCRITICAL);

    }
    if (pVictim->GetUnitParryChance() * 655.36f >= (uint16)irand(0, 512))
    {
        
        damage = 0;
        victimState = 2;

        HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);
    }
    if (pVictim->GetUnitDodgeChance() * 655.36f >= (uint16)irand(0, 512))
    {
        
        damage = 0;
        victimState = 3;

        HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);
    }
    if (pVictim->GetUnitBlockChance() * 655.36f >= (uint16)irand(0, 512))
    {
        
        blocked_amount = (pVictim->GetUnitBlockValue() * (pVictim->GetUnitStrength() / 10));
            
        if (blocked_amount < damage) 
        {
            damage = damage - blocked_amount;
        }
        else 
        {
            damage = 0;
        }

        if (pVictim->isPlayer() && pVictim->GetUnitBlockValue())
        {
            HandleEmoteCommand(EMOTE_ONESHOT_PARRYSHIELD);
        }
        if (pVictim->isPlayer() && pVictim->GetUnitBlockValue() == 0)
        {
            HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);
        }

        victimState = 4;
    }
}

void Unit::AttackerStateUpdate (Unit *pVictim, uint32 damage)
{
    WorldPacket data;
    uint32    spell = 0;
    uint32    hitInfo = 0x22;
    uint32    damageType = 0;
    uint32    extraSpellID = 0;
    uint32    extraSpellDamge = 0;
    uint32    blocked_amount = 0;
    uint32    victimState = 1;
    int32    attackerSkill = GetUnitMeleeSkill();
    int32    victimSkill = pVictim->GetUnitMeleeSkill();
    float    chanceToHit = 100.0f;
    uint32    unknownValue = 0x0;
    bool    hit;
    uint32    victimAgility = pVictim->GetUInt32Value(UNIT_FIELD_AGILITY);
    uint32    attackerAgility = pVictim->GetUInt32Value(UNIT_FIELD_AGILITY);

    
    if (victimAgility < attackerAgility)
    {
        if (irand(0,(int)attackerAgility-victimAgility) >= 5)
            hit = true;
        else
            hit = false;
    }
    else if (victimAgility > attackerAgility)
    {
        if (irand(0,(int)victimAgility-attackerAgility) <= 5)
            hit = true;
        else
            hit = false;
    }
    else
    {
        if (irand(0,5) >= 2)
            hit = true;
        else
            hit = false;
    }

    if (pVictim->isDead())
    {
        smsg_AttackStop(pVictim->GetGUID());
        return;
    }

    if ((m_currentSpell != NULL) && !m_meleeSpell) 
        return;

    

    if(damage == 0) 
        damage = CalculateDamage (this);
    else 
        damageType = 1;    

    if(hit) 
        hitInfo = 0;

    if (m_meleeSpell == true)
    {
        if(m_currentSpell != NULL && m_currentSpell->getState() == SPELL_STATE_IDLE)
        {
            spell = m_currentSpell->m_spellInfo->Id;
            m_currentSpell->SendCastResult(0);
            m_currentSpell->SendSpellGo();
            
            for(uint32 i=0;i<2;i++)
            {
                m_currentSpell->HandleEffects(m_currentSpell->m_targets.m_unitTarget,i);
            }

            m_currentSpell->finish();
        }
    }
    
    if (isPlayer() && pVictim->isUnit())
    {
        if (attackerSkill <= victimSkill - 24) 
        {
            chanceToHit = 0;
        } 
        else 
        {
            if (attackerSkill <= victimSkill)
            {
                chanceToHit = 100.0f - (victimSkill - attackerSkill) * (100.0f / 30.0f);
            }
        }
        
        if (chanceToHit < 15.0f) 
            chanceToHit = 15.0f;
    }


    if (chanceToHit * 655.36f >= (uint16)irand(0, 32767))
    {
        if (isPlayer() && pVictim->isUnit())
        {
            if (attackerSkill < victimSkill - 20) 
            {
                damage = (damage * 30) / 100;
            }
            else
            {
                if (attackerSkill < victimSkill - 10) 
                {
                    damage = (damage * 60) / 100;
                }
            }
        }
    } 
    else 
    {
        damage = 0;
    }


    if (damage)
    {
        DoAttackDamage(pVictim, damage, blocked_amount, damageType, hitInfo, victimState);
    }
    
    if (extraSpellID && extraSpellDamge)
    {
        SpellNonMeleeDamageLog (pVictim, extraSpellID, extraSpellDamge);
        return;
    }

    data.Initialize(SMSG_ATTACKERSTATEUPDATE);
    data << (uint32)hitInfo;            
    data << GetGUID();                    
    data << pVictim->GetGUID();            
    data << (uint32)damage;                

    data << (uint8)1;                    
    data << damageType;                    
    data << (float)damage;                
    data << (uint32)damage;                
    data << (uint32)0;                    

    data << (uint32)0;                    

    data << (uint32)victimState;        

    
    
    data << (uint32)extraSpellDamge;    
    data << (uint32)extraSpellID;        
    data << (uint32)blocked_amount;        

    
    SendMessageToSet(&data, true);

    if (isPlayer())
        DEBUG_LOG("AttackerStateUpdate: (Player) %u %X attacked %u %X for %u dmg.",
            GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage);
    else
        DEBUG_LOG("AttackerStateUpdate: (NPC) %u %X attacked %u %X for %u dmg.",
            GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage);

    DealDamage(pVictim, damage, 0);
}

void Unit::smsg_AttackStop(uint64 victimGuid)
{
    WorldPacket data;
    data.Initialize( SMSG_ATTACKSTOP );
    data << GetGUID();
    data << victimGuid;
    data << uint32( 0 );
    data << (uint32)0;                    

    SendMessageToSet(&data, true);
    Log::getSingleton( ).outDetail("%u %X stopped attacking "I64FMT,
				   GetGUIDLow(), GetGUIDHigh(), victimGuid);

    
    if( IS_CREATURE_GUID(victimGuid) )
    {
	Creature *pVictim = ObjectAccessor::Instance().GetCreature(*this, victimGuid);
	if( pVictim != NULL )
	    pVictim->AI().AttackStop(this);
    }
}



bool Unit::AddAffect(Affect *aff, bool uniq)
{
    AffectList::const_iterator i;

    for (i = m_affects.begin(); i != m_affects.end(); i++)
    {
        if ((*i)->GetSpellId() == aff->GetSpellId() &&
            (uniq || (*i)->GetCasterGUID() == aff->GetCasterGUID()))
        {
            break;
        }
    }

    if (i != m_affects.end())
    {
#ifndef _VC80_UPGRADE 
        if(i != 0)
#endif 
            (*i)->SetDuration(0);
    }

    m_affects.push_back(aff);

    for (Affect::ModList::const_iterator j = aff->GetModList().begin();
        j != aff->GetModList().end(); j++)
    {
        ApplyModifier(&(*j), true, aff);
    }

    _AddAura(aff);

    return true;
}


void Unit::RemoveAffect(Affect *aff)
{
    AffectList::iterator i;

    for (i = m_affects.begin(); i != m_affects.end(); i++)
    {
        if (*i == aff)
            break;
    }

    ASSERT(i != m_affects.end());

    if(aff->GetCoAffect() != 0)
        RemoveAffect(aff->GetCoAffect());
    for (Affect::ModList::const_iterator j = aff->GetModList().begin();
        j != aff->GetModList().end(); j++)
    {
        ApplyModifier(&(*j), false, aff);
    }

    _RemoveAura(aff);

    m_affects.erase(i);
}


void Unit::RemoveAffectById(uint32 spellId)
{
    if(m_aura != NULL)
        if(m_aura->GetId() == spellId)
    {
        m_aura = NULL;
        return;
    }

    Affect* aff = FindAff(spellId);
    if(aff)
        aff->SetDuration(0);
}


bool Unit::SetAffDuration(uint32 spellId,Unit* caster,uint32 duration)
{
    AffectList::iterator i;

    for (i = m_affects.begin(); i != m_affects.end(); i++)
    {
        if ((*i)->GetId() == spellId && (*i)->GetCasterGUID() == caster->GetGUID())
        {
            (*i)->SetDuration(duration);
            return true;
        }
    }
    return false;
}


uint32 Unit::GetAffDuration(uint32 spellId,Unit* caster)
{
    AffectList::iterator i;

    for (i = m_affects.begin(); i != m_affects.end(); i++)
    {
        if ((*i)->GetId() == spellId && (*i)->GetCasterGUID() == caster->GetGUID())
        {
            return (*i)->GetDuration();
        }
    }
    return 0;
}


bool Unit::RemoveAffect(uint32 spellId)
{
    AffectList::iterator i, next;
    Affect *aff;
    bool result = false;

    for (i = m_affects.begin(); i != m_affects.end(); i = next)
    {
        next = i;
        next++;

        aff = *i;

        if (aff->GetSpellId() == spellId)
        {
            for (Affect::ModList::const_iterator j = aff->GetModList().begin();
                j != aff->GetModList().end(); i++)
            {
                ApplyModifier(&(*j), false, aff);
            }

            _RemoveAura(aff);

            m_affects.erase(i);

            delete aff;

            result = true;
        }
    }

    return result;
}


void Unit::RemoveAllAffects()
{
    
    Log::getSingleton( ).outError("RemoveAllAffects");
    AffectList::iterator i;
    Affect::ModList::const_iterator j;
    

    for (i = m_affects.begin(); i != m_affects.end(); i++)
    {
        Log::getSingleton( ).outError("First Loop");
        
        Log::getSingleton( ).outError("Affect set");
        for (j = (*i)->GetModList().begin(); j != (*i)->GetModList().end(); j++)
        {
            Log::getSingleton( ).outError("Second Loop");
            ApplyModifier(&(*j), false, (*i));
        }
        Log::getSingleton( ).outError("Remove");
        _RemoveAura((*i));
    }
    Log::getSingleton( ).outError("Erase");
    
    
    
    
    m_affects.clear();
    
    return;
}




void Unit::_RemoveAllAffectMods()
{
    AffectList::iterator i;
    Affect::ModList::const_iterator j;

    Affect *aff;

    for (i = m_affects.begin(); i != m_affects.end(); i++)
    {
        aff = *i;

        for (j = aff->GetModList().begin();
            j != aff->GetModList().end(); j++)
        {
            ApplyModifier(&(*j), false, aff);
        }

        _RemoveAura(aff);
    }
}


void Unit::_ApplyAllAffectMods()
{
    AffectList::iterator i;
    Affect::ModList::const_iterator j;

    Affect *aff;

    for (i = m_affects.begin(); i != m_affects.end(); i++)
    {
        aff = *i;

        for (j = aff->GetModList().begin();
            j != aff->GetModList().end(); j++)
        {
            ApplyModifier(&(*j), true, aff);
        }

        _AddAura(aff);
    }
}


void Unit::ApplyModifier(const Modifier *mod, bool apply, Affect* parent)
{
    WorldPacket data;
    switch(mod->GetType())
    {
        case SPELL_AURA_NONE:
        {
        }break;
        case SPELL_AURA_BIND_SIGHT:
        {
        }break;
        case SPELL_AURA_MOD_THREAT:
        {
        }break;
        case SPELL_AURA_AURAS_VISIBLE:
        {
        }break;
        case SPELL_AURA_MOD_RESISTANCE_PCT:
        {
        }break;
        case SPELL_AURA_MOD_CREATURE_ATTACK_POWER:
        {
        }break;
        case SPELL_AURA_MOD_TOTAL_THREAT:
        {
        }break;
        case SPELL_AURA_WATER_WALK:
        {
            apply ?
                data.Initialize(SMSG_MOVE_WATER_WALK)
                : data.Initialize(SMSG_MOVE_LAND_WALK);
            data << GetGUID();
            SendMessageToSet(&data,true);
        }break;
        case SPELL_AURA_FEATHER_FALL:
        {
            apply ?
                data.Initialize(SMSG_MOVE_FEATHER_FALL)
                : data.Initialize(SMSG_MOVE_NORMAL_FALL);
            data << GetGUID();
            SendMessageToSet(&data,true);
        }break;
        case SPELL_AURA_HOVER:
        {
        }break;
        case SPELL_AURA_ADD_FLAT_MODIFIER:
        {
        }break;
        case SPELL_AURA_ADD_PCT_MODIFIER:
        {
        }break;
        case SPELL_AURA_ADD_TARGET_TRIGGER:
        {
        }break;
        case SPELL_AURA_MOD_TAUNT:
        {
        }break;
        case SPELL_AURA_MOD_POWER_REGEN_PERCENT:
        {
        }break;
        case SPELL_AURA_ADD_CASTER_HIT_TRIGGER:
        {
        }break;
        case SPELL_AURA_OVERRIDE_CLASS_SCRIPTS:
        {
        }break;
        case SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN:
        {
        }break;
        case SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT:
        {
        }break;
        case SPELL_AURA_MOD_HEALING:
        {
        }break;
        case SPELL_AURA_IGNORE_REGEN_INTERRUPT:
        {
        }break;
        case SPELL_AURA_MOD_MECHANIC_RESISTANCE:
        {
        }break;
        case SPELL_AURA_MOD_HEALING_PCT:
        {
        }break;
        case SPELL_AURA_SHARE_PET_TRACKING:
        {
        }break;
        case SPELL_AURA_MOD_STUN:
        {
        }break;
        case SPELL_AURA_UNTRACKABLE:
        {
        }break;
        case SPELL_AURA_EMPATHY:
        {
        }break;
        case SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT:
        {
        }break;
        case SPELL_AURA_MOD_POWER_COST_PCT:
        {
        }break;
        case SPELL_AURA_MOD_RANGED_ATTACK_POWER:
        {
            apply ? SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS,GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS) + mod->GetAmount()) : SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS,GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS) - mod->GetAmount());
        }break;
        case SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN:
        {
        }break;
        case SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT:
        {
        }break;
        case SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS:
        {
        }break;
        case SPELL_AURA_MOD_POSSESS_PET:
        {
        }break;
        case SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS:
        {
            data.Initialize(MSG_MOVE_SET_RUN_SPEED);
            data << GetGUID();
            apply ? data << float(7.5+7.5/100*mod->GetAmount()) : data << float(7.5);
            SendMessageToSet(&data,true);
        }break;
        case SPELL_AURA_MOD_DAMAGE_DONE:
        {
        }break;
        case SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS:
        {
        }break;
        case SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER:
        {
        }break;
        case SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT:
        {
            uint32 percent = mod->GetAmount();
            uint32 current = GetUInt32Value(UNIT_FIELD_POWER4);
            apply ? SetUInt32Value(UNIT_FIELD_POWER4,current+current/100*percent) : SetUInt32Value(UNIT_FIELD_POWER4,current-current/(100+percent)*100);
        }break;
        case SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT:
        {
            uint32 percent = mod->GetAmount();
            uint32 current = GetUInt32Value(UNIT_FIELD_MAXHEALTH);
            apply ? SetUInt32Value(UNIT_FIELD_MAXHEALTH,current+current/100*percent) : SetUInt32Value(UNIT_FIELD_MAXHEALTH,current-current/(100+percent)*100);
        }break;
        case SPELL_AURA_MOD_MANA_REGEN_INTERRUPT:
        {
        }break;
        case SPELL_AURA_MOD_HEALING_DONE:
        {
        }break;
        case SPELL_AURA_MOD_HEALING_DONE_PERCENT:
        {
        }break;
        case SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE:
        {
        }break;
        case SPELL_AURA_MOD_HASTE:
        {
        }break;
        case SPELL_AURA_FORCE_REACTION:
        {
        }break;
        case SPELL_AURA_MOD_DAMAGE_TAKEN:
        {
        }break;
        case SPELL_AURA_MOD_RANGED_HASTE:
        {
        }break;
        case SPELL_AURA_MOD_RANGED_AMMO_HASTE:
        {
        }break;
        case SPELL_AURA_MOD_BASE_RESISTANCE_PCT:
        {
        }break;
        case SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE:
        {
            uint32 index = 0;
            uint32 index2 = 0;
            switch(mod->GetMiscValue())
            {
                case 0:{
                    index = UNIT_FIELD_ARMOR;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE;
                }break;
                case 1:{
                    index = UNIT_FIELD_RESISTANCES_01;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01;
                }break;
                case 2:{
                    index = UNIT_FIELD_RESISTANCES_02;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_02 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_02;
                }break;
                case 3:{
                    index = UNIT_FIELD_RESISTANCES_03;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_03 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_03;
                }break;
                case 4:{
                    index = UNIT_FIELD_RESISTANCES_04;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_04 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_04;
                }break;
                case 5:{
                    index = UNIT_FIELD_RESISTANCES_05;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_05 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_05;
                }break;
                case 6:{
                    index = UNIT_FIELD_RESISTANCES_06;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06;
                }
                break;
                case -1:{
                    index = UNIT_FIELD_RESISTANCES_06;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06;
                    for(uint32 i=0;i<6;i++)
                        if(apply){
                            SetUInt32Value(index+i,GetUInt32Value(index+i)+mod->GetAmount());
                            if(GetTypeId() == TYPEID_PLAYER)
                                SetUInt32Value(index2+i,GetUInt32Value(index2+i)+mod->GetAmount());
                        }else{
                            SetUInt32Value(index+i,GetUInt32Value(index+i)-mod->GetAmount());
                            if(GetTypeId() == TYPEID_PLAYER)
                                SetUInt32Value(index2+i,GetUInt32Value(index2+i)-mod->GetAmount());
                        }
                    return;
                }break;
                default:{
                    printf("WARNING: Misc Value for SPELL_AURA_MOD_STAT not valid\n");
                    return;
                }break;
            }

            if(apply)
            {
                SetUInt32Value(index,GetUInt32Value(index)+mod->GetAmount());
                if(GetTypeId() == TYPEID_PLAYER)
                    SetUInt32Value(index2,GetUInt32Value(index2)+mod->GetAmount());
            }
            else
            {
                SetUInt32Value(index,GetUInt32Value(index)-mod->GetAmount());
                if(GetTypeId() == TYPEID_PLAYER)
                    SetUInt32Value(index2,GetUInt32Value(index2)-mod->GetAmount());
            }
        }break;
        case SPELL_AURA_SAFE_FALL:
        {
            apply ? data.Initialize(SMSG_MOVE_FEATHER_FALL) : data.Initialize(SMSG_MOVE_NORMAL_FALL);
            data << GetGUID();
            SendMessageToSet(&data,true);
        }break;
        case SPELL_AURA_CHARISMA:
        {
        }break;
        case SPELL_AURA_PERSUADED:
        {
        }break;
        case SPELL_AURA_ADD_CREATURE_IMMUNITY:
        {
        }break;
        case SPELL_AURA_RETAIN_COMBO_POINTS:
        {
        }break;
        case SPELL_AURA_DAMAGE_SHIELD:
        {
            if(apply)
            {
                DamageShield* ds = new DamageShield();
                ds->m_caster = parent->GetCasterGUID();
                ds->m_damage = mod->GetAmount();
                ds->m_spellId = parent->GetId();
                m_damageShields.push_back((*ds));
            }
            else
            {
                for(std::list<struct DamageShield>::iterator i = m_damageShields.begin();i != m_damageShields.end();i++)
                    if(i->m_spellId == parent->GetId() && i->m_caster == parent->GetCasterGUID())
                {
                    m_damageShields.erase(i);
                    break;
                }
            }
        }break;
        case SPELL_AURA_MOD_STEALTH:
        {
        }break;
        case SPELL_AURA_MOD_DETECT:
        {
        }break;
        case SPELL_AURA_MOD_INVISIBILITY:
        {
        }break;
        case SPELL_AURA_MOD_INVISIBILITY_DETECTION:
        {
        }break;
        case SPELL_AURA_MOD_POSSESS:
        {
        }break;
        case SPELL_AURA_MOD_RESISTANCE:
        {
            uint16 index = 0;
            uint16 index2 = 0;
             switch(mod->GetMiscValue())
            {
                case 0:{
                    index = UNIT_FIELD_ARMOR;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE;
                }break;
                case 1:{
                    index = UNIT_FIELD_RESISTANCES_01;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_01 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_01;
                }break;
                case 2:{
                    index = UNIT_FIELD_RESISTANCES_02;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_02 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_02;
                }break;
                case 3:{
                    index = UNIT_FIELD_RESISTANCES_03;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_03 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_03;
                }break;
                case 4:{
                    index = UNIT_FIELD_RESISTANCES_04;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_04 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_04;
                }break;
                case 5:{
                    index = UNIT_FIELD_RESISTANCES_05;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_05 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_05;
                }
                case 6:{
                    index = UNIT_FIELD_RESISTANCES_06;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06;
                }
                break;
                case -1:{
                    
                    index = UNIT_FIELD_RESISTANCES_06;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE_06 : index2 = PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE_06;
                    for(uint32 i=0;i<5;i++)
                        if(apply){
                            SetUInt32Value(index+i,GetUInt32Value(index+i)+mod->GetAmount());
                            if(GetTypeId() == TYPEID_PLAYER)
                                SetUInt32Value(index2+i,GetUInt32Value(index2+i)+mod->GetAmount());
                        }else{
                            SetUInt32Value(index+i,GetUInt32Value(index+i)-mod->GetAmount());
                            if(GetTypeId() == TYPEID_PLAYER)
                                SetUInt32Value(index2+i,GetUInt32Value(index2+i)-mod->GetAmount());
                        }
                    return;
                }break;
                default:{
                    printf("WARNING: Misc Value for SPELL_AURA_MOD_STAT not valid\n");
                    return;
                }break;
            }

            if(apply)
            {
                SetUInt32Value(index,GetUInt32Value(index)+mod->GetAmount());
                if(GetTypeId() == TYPEID_PLAYER)
                    SetUInt32Value(index2,GetUInt32Value(index2)+mod->GetAmount());
            }
            else
            {
                SetUInt32Value(index,GetUInt32Value(index)-mod->GetAmount());
                if(GetTypeId() == TYPEID_PLAYER)
                    SetUInt32Value(index2,GetUInt32Value(index2)-mod->GetAmount());
            }
        }break;
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        {
        }break;
        case SPELL_AURA_PERIODIC_ENERGIZE:
        {
        }break;
        case SPELL_AURA_MOD_PACIFY:
        {
        }break;
        case SPELL_AURA_MOD_ROOT:
        {
            apply ?
                data.Initialize(MSG_MOVE_ROOT)
                : data.Initialize(MSG_MOVE_UNROOT);
            data << GetGUID();
            SendMessageToSet(&data,true);
        }break;
        case SPELL_AURA_MOD_SILENCE:
        {
            apply ? m_silenced = true : m_silenced = false;
        }break;
        case SPELL_AURA_REFLECT_SPELLS:
        {
        }break;
        case SPELL_AURA_MOD_STAT:
        {
            uint16 index = 0;
            uint16 index2 = 0;
             switch(mod->GetMiscValue())
            {
                case 0:{
                    index = UNIT_FIELD_STR;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_POSSTAT0 : index2 = PLAYER_FIELD_NEGSTAT0;
                }break;
                case 1:{
                    index = UNIT_FIELD_AGILITY;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_POSSTAT1 : index2 = PLAYER_FIELD_NEGSTAT1;
                }break;
                case 2:{
                    index = UNIT_FIELD_STAMINA;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_POSSTAT2 : index2 = PLAYER_FIELD_NEGSTAT2;
                }break;
                case 3:{
                    index = UNIT_FIELD_IQ;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_POSSTAT3 : index2 = PLAYER_FIELD_NEGSTAT3;
                }break;
                case 4:{
                    index = UNIT_FIELD_SPIRIT;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_POSSTAT4 : index2 = PLAYER_FIELD_NEGSTAT4;
                }break;
                case -1:{
                    
                    index = UNIT_FIELD_STR;
                    mod->GetMiscValue2() == 0 ? index2 = PLAYER_FIELD_POSSTAT0 : index2 = PLAYER_FIELD_NEGSTAT0;
                    for(uint32 i=0;i<5;i++)
                        if(apply){
                            SetUInt32Value(index+i,GetUInt32Value(index+i)+mod->GetAmount());
                            if(GetTypeId() == TYPEID_PLAYER)
                                SetUInt32Value(index2+i,GetUInt32Value(index2+i)+mod->GetAmount());
                        }else{
                            SetUInt32Value(index+i,GetUInt32Value(index+i)-mod->GetAmount());
                            if(GetTypeId() == TYPEID_PLAYER)
                                SetUInt32Value(index2+i,GetUInt32Value(index2+i)-mod->GetAmount());
                        }
                    return;
                }break;
                default:{
                    printf("WARNING: Misc Value for SPELL_AURA_MOD_STAT not valid\n");
                    return;
                }break;
            }
            if(apply)
            {
                SetUInt32Value(index,GetUInt32Value(index)+mod->GetAmount());
                if(GetTypeId() == TYPEID_PLAYER)
                    SetUInt32Value(index2,GetUInt32Value(index2)+mod->GetAmount());
            }
            else
            {
                SetUInt32Value(index,GetUInt32Value(index)-mod->GetAmount());
                if(GetTypeId() == TYPEID_PLAYER)
                    SetUInt32Value(index2,GetUInt32Value(index2)-mod->GetAmount());
            }
        }break;
        case SPELL_AURA_PERIODIC_DAMAGE:
        {
        }break;
        case SPELL_AURA_MOD_SKILL:
        {
        }break;
        case SPELL_AURA_MOD_INCREASE_SPEED:
        {
            m_speed = m_speed + 7.0f/100*mod->GetAmount();
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            data << GetGUID();
            apply ? data << float(7.5+7.5/100*mod->GetAmount()) : data << float(7.5);
            SendMessageToSet(&data,true);
        }break;
        case SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED:
        {
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            data << GetGUID();
            apply ? data << float(7.5+7.5/100*mod->GetAmount()) : data << float(7.5);
            SendMessageToSet(&data,true);
        }break;
        case SPELL_AURA_MOD_DECREASE_SPEED:
        {
            m_speed = m_speed - 7.0f/100*mod->GetAmount();
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            data << GetGUID();
            apply ? data << float(7.5-7.5/100*mod->GetAmount()) : data << float(7.5);
            SendMessageToSet(&data,true);
        }break;
        case SPELL_AURA_MOD_INCREASE_HEALTH:
        {
            uint32 newValue;
            newValue = GetUInt32Value(UNIT_FIELD_MAXHEALTH);
            apply ? newValue += mod->GetAmount() : newValue -= mod->GetAmount();
            SetUInt32Value(UNIT_FIELD_MAXHEALTH,newValue);
        }break;
        case SPELL_AURA_MOD_INCREASE_ENERGY:
        {
            uint32 powerField = 23;
            uint8 powerType = (uint8)(GetUInt32Value(UNIT_FIELD_BYTES_0) >> 24);
            if(powerType == 0)                    
                powerField = UNIT_FIELD_POWER1;
            else if(powerType == 1)               
                powerField = UNIT_FIELD_POWER2;
            else if(powerType == 3)               
                powerField = UNIT_FIELD_POWER4;

            uint32 newValue = GetUInt32Value(powerType);
            apply ? newValue += mod->GetAmount() : newValue -= mod->GetAmount();
            SetUInt32Value(powerType,newValue);
        }break;
        case SPELL_AURA_MOD_SHAPESHIFT:
        {
            Affect* tmpAff;
            uint32 spellId;
            switch(mod->GetMiscValue())
            {
                case FORM_CAT:
                {
                    spellId = 3025;
                } break;
                case FORM_TREE:
                {
                    spellId = 3122;
                } break;
                case FORM_TRAVEL:
                {
                    spellId = 5419;
                } break;
                case FORM_AQUA:
                {
                    spellId = 5421;
                } break;
                case FORM_BEAR:
                {
                    spellId = 1178;
                } break;
                case FORM_AMBIENT:
                {
                    spellId = 0;
                } break;
                case FORM_GHOUL:
                {
                    spellId = 0;
                } break;
                case FORM_DIREBEAR:
                {
                    spellId = 9635;
                } break;
                case FORM_CREATUREBEAR:
                {
                    spellId = 2882;
                } break;
                case FORM_GHOSTWOLF:
                {
                    spellId = 0;
                } break;
                case FORM_BATTLESTANCE:
                {
                    spellId = 0;
                } break;
                case FORM_DEFENSIVESTANCE:
                {
                    spellId = 7376;
                } break;
                case FORM_BERSERKERSTANCE:
                {
                    spellId = 7381;
                } break;
                case FORM_SHADOW:
                {
                    spellId = 0;
                } break;
                case FORM_STEALTH:
                {
                    spellId = 3025;
                } break;
                default:
                {
                    printf("Unknown Shapeshift Type\n");
                } break;
            }
            
            SpellEntry *spellInfo = sSpellStore.LookupEntry( spellId );

            if(!spellInfo)
            {
                Log::getSingleton( ).outError("WORLD: unknown spell id %i\n", spellId);
                break;
            }
            tmpAff = new Affect(spellInfo,parent->GetDuration(),parent->GetCasterGUID());
            for(uint8 i=0;i<3;i++)
            {
                if(spellInfo->Effect[i] == 6)
                {
                    uint32 value = 0;
                    uint32 type = 0;
                    uint32 damage = 0;
                    if(spellInfo->EffectBasePoints[i] < 0)
                    {
                        tmpAff->SetNegative();
                        type = 1;
                    }
                    uint32 sBasePoints = (uint32)sqrt((float)(spellInfo->EffectBasePoints[i]*spellInfo->EffectBasePoints[i]));
                    
                    if(spellInfo->EffectApplyAuraName[i] == 3)
                    {
                        damage = spellInfo->EffectBasePoints[i]+rand()%spellInfo->EffectDieSides[i]+1;
                        tmpAff->SetDamagePerTick((uint16)damage, spellInfo->EffectAmplitude[i]);
                        tmpAff->SetNegative();
                    
                    }
                    else if(spellInfo->EffectApplyAuraName[i] == 23)
                        tmpAff->SetPeriodicTriggerSpell(spellInfo->EffectTriggerSpell[i],spellInfo->EffectAmplitude[i]);
                    
                    else if(spellInfo->EffectApplyAuraName[i] == 8)
                        tmpAff->SetHealPerTick(damage,spellInfo->EffectAmplitude[i]);
                    else
                    {
                        if(spellInfo->EffectDieSides[i] != 0)
                            value = sBasePoints+rand()%spellInfo->EffectDieSides[i];
                        else
                            value = sBasePoints;
                        if(spellInfo->EffectDieSides[i] <= 1)
                            value += 1;
                        tmpAff->AddMod((uint8)spellInfo->EffectApplyAuraName[i],value,spellInfo->EffectMiscValue[i],type);
                    }
                }
            }
            if(tmpAff)
            {
                parent->SetCoAffect(tmpAff);
                AddAffect(tmpAff);
            }
        }break;
        case SPELL_AURA_EFFECT_IMMUNITY:
        {
        }break;
        case SPELL_AURA_STATE_IMMUNITY:
        {
        }break;
        case SPELL_AURA_SCHOOL_IMMUNITY:
        {
        }break;
        case SPELL_AURA_DAMAGE_IMMUNITY:
        {
        }break;
        case SPELL_AURA_DISPEL_IMMUNITY:
        {
        }break;
        case SPELL_AURA_PROC_TRIGGER_SPELL:
        {
            uint32 i=0;
            for(i=0;i<2;i++)
                if(parent->GetSpellProto()->EffectApplyAuraName[i] == mod->GetType())
                    break;
            if(apply)
            {
                ProcTriggerSpell* pts = new ProcTriggerSpell();
                pts->caster = parent->GetCasterGUID();
                pts->spellId = parent->GetSpellProto()->EffectTriggerSpell[i];
                pts->trigger = parent->GetSpellProto()->EffectBasePoints[i];
                pts->procChance = parent->GetSpellProto()->procChance;
                pts->procFlags = parent->GetSpellProto()->procFlags;
                parent->GetSpellProto()->procCharges == 0 ? pts->procCharges = 0
                    : pts->procCharges = parent->GetSpellProto()->procCharges;
                m_procSpells.push_back((*pts));
            }
            else
            {
                for(std::list<struct ProcTriggerSpell>::iterator itr = m_procSpells.begin();itr != m_procSpells.end();itr++)
                    if(itr->spellId == parent->GetId() && itr->caster == parent->GetCasterGUID())
                {
                    m_procSpells.erase(itr);
                    break;
                }
            }
        }break;
        case SPELL_AURA_PROC_TRIGGER_DAMAGE:
        {
            if(apply)
            {
                DamageShield* ds = new DamageShield();
                ds->m_caster = parent->GetCasterGUID();
                ds->m_damage = mod->GetAmount();
                ds->m_spellId = parent->GetId();
                m_damageShields.push_back((*ds));
            }
            else
            {
                for(std::list<struct DamageShield>::iterator i = m_damageShields.begin();i != m_damageShields.end();i++)
                    if(i->m_spellId == parent->GetId() && i->m_caster == parent->GetCasterGUID())
                {
                    m_damageShields.erase(i);
                    break;
                }
            }
        }break;
        case SPELL_AURA_TRACK_CREATURES:
        {
            apply ? SetUInt32Value(PLAYER_TRACK_CREATURES,mod->GetMiscValue()) : SetUInt32Value(PLAYER_TRACK_CREATURES,0);
        }break;
        case SPELL_AURA_TRACK_RESOURCES:
        {
            apply ? SetUInt32Value(PLAYER_TRACK_RESOURCES,mod->GetMiscValue()) : SetUInt32Value(PLAYER_TRACK_RESOURCES,0);
        }break;
        case SPELL_AURA_MOD_PARRY_SKILL:
        {
        }break;
        case SPELL_AURA_MOD_PARRY_PERCENT:
        {
            uint32 current = GetUInt32Value(PLAYER_PARRY_PERCENTAGE);
            apply ? SetUInt32Value(PLAYER_PARRY_PERCENTAGE,current+mod->GetAmount()) : SetUInt32Value(PLAYER_PARRY_PERCENTAGE,current-mod->GetAmount());
        }break;
        case SPELL_AURA_MOD_DODGE_SKILL:
        {
        }break;
        case SPELL_AURA_MOD_DODGE_PERCENT:
        {
            uint32 current = GetUInt32Value(PLAYER_DODGE_PERCENTAGE);
            apply ? SetUInt32Value(PLAYER_DODGE_PERCENTAGE,current+mod->GetAmount()) : SetUInt32Value(PLAYER_DODGE_PERCENTAGE,current-mod->GetAmount());
        }break;
        case SPELL_AURA_MOD_CONFUSE:
        {
        }break;
        case SPELL_AURA_MOD_BLOCK_SKILL:
        {
        }break;
        case SPELL_AURA_MOD_BLOCK_PERCENT:
        {
            uint32 current = GetUInt32Value(PLAYER_BLOCK_PERCENTAGE);
            apply ? SetUInt32Value(PLAYER_BLOCK_PERCENTAGE,current+mod->GetAmount()) : SetUInt32Value(PLAYER_BLOCK_PERCENTAGE,current-mod->GetAmount());
        }break;
        case SPELL_AURA_MOD_CRIT_PERCENT:
        {
            uint32 current = GetUInt32Value(PLAYER_CRIT_PERCENTAGE);
            apply ? SetUInt32Value(PLAYER_CRIT_PERCENTAGE,current+mod->GetAmount()) : SetUInt32Value(PLAYER_CRIT_PERCENTAGE,current-mod->GetAmount());
        }break;
        case SPELL_AURA_PERIODIC_LEECH:
        {
        }break;
        case SPELL_AURA_MOD_HIT_CHANCE:
        {
        }break;
        case SPELL_AURA_MOD_SPELL_HIT_CHANCE:
        {
        }break;
        case SPELL_AURA_TRANSFORM:
        {
        }break;
        case SPELL_AURA_MOD_SPELL_CRIT_CHANCE:
        {
        }break;
        case SPELL_AURA_MOD_INCREASE_SWIM_SPEED:
        {
        }break;
        case SPELL_AURA_MOD_DAMAGE_DONE_CREATURE:
        {
        }break;
        case SPELL_AURA_MOD_CHARM:
        {
        }break;
        case SPELL_AURA_MOD_PACIFY_SILENCE:
        {
        }break;
        case SPELL_AURA_MOD_SCALE:
        {
            float current = GetFloatValue(OBJECT_FIELD_SCALE_X);
            apply ? SetFloatValue(OBJECT_FIELD_SCALE_X,current+current/100*10) : SetFloatValue(OBJECT_FIELD_SCALE_X,current-current/110*100);
        }break;
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
        {
        }break;
        case SPELL_AURA_PERIODIC_MANA_FUNNEL:
        {
        }break;
        case SPELL_AURA_PERIODIC_MANA_LEECH:
        {
        }break;
        case SPELL_AURA_MOD_CASTING_SPEED:
        {
        }break;
        case SPELL_AURA_FEIGN_DEATH:
        {
        }break;
        case SPELL_AURA_MOD_DISARM:
        {
        }break;
        case SPELL_AURA_MOD_STALKED:
        {
        }break;
        case SPELL_AURA_SCHOOL_ABSORB:
        {
        }break;
        case SPELL_AURA_MOD_FEAR:
        {
        }break;
        case SPELL_AURA_EXTRA_ATTACKS:
        {
        }break;
        case SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL:
        {
        }break;
        case SPELL_AURA_MOD_POWER_COST:
        {
        }break;
        case SPELL_AURA_MOD_POWER_COST_SCHOOL:
        {
        }break;
        case SPELL_AURA_REFLECT_SPELLS_SCHOOL:
        {
        }break;
        case SPELL_AURA_MOD_LANGUAGE:
        {
        }break;
        case SPELL_AURA_FAR_SIGHT:
        {
        }break;
        case SPELL_AURA_MECHANIC_IMMUNITY:
        {
        }break;
        case SPELL_AURA_MOUNTED:
        {
            if(apply)
            {
                CreatureInfo* ci = objmgr.GetCreatureName(mod->GetMiscValue());
                if(!ci)
                    break;
                uint32 displayId = ci->DisplayID;
                if(displayId != 0)
                {
                    SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID , displayId);
                    SetUInt32Value( UNIT_FIELD_FLAGS , 0x002000 );
                }
            }else
            {
                SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
                RemoveFlag( UNIT_FIELD_FLAGS, 0x002000 );

                if (GetUInt32Value(UNIT_FIELD_FLAGS) & 0x000004 )
                    RemoveFlag( UNIT_FIELD_FLAGS, 0x000004 );
            }
        }break;
        case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE:
        {
        }break;
        case SPELL_AURA_PERIODIC_HEAL:
        {
        }break;
        case SPELL_AURA_MOD_PERCENT_STAT:
        {
        }break;
        case SPELL_AURA_SPLIT_DAMAGE:
        {
        }break;
        case SPELL_AURA_WATER_BREATHING:
        {
        }break;
        case SPELL_AURA_MOD_BASE_RESISTANCE:
        {
        }break;
        case SPELL_AURA_MOD_REGEN:
        {
        }break;
        case SPELL_AURA_MOD_POWER_REGEN:
        {
        }break;
        case SPELL_AURA_CHANNEL_DEATH_ITEM:
        {
        }break;
        case SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN:
        {
        }break;
        case SPELL_AURA_MOD_PERCENT_REGEN:
        {
        }break;
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        {
        }break;
        case SPELL_AURA_MOD_ATTACKSPEED:
        {
        }break;
        case SPELL_AURA_MOD_RESIST_CHANCE:
        {
        }break;
        case SPELL_AURA_MOD_DETECT_RANGE:
        {
        }break;
        case SPELL_AURA_PREVENTS_FLEEING:
        {
        }break;
        case SPELL_AURA_MOD_UNATTACKABLE:
        {
        }break;
        case SPELL_AURA_INTERRUPT_REGEN:
        {
        }break;
        case SPELL_AURA_GHOST:
        {
        }break;
        case SPELL_AURA_SPELL_MAGNET:
        {
        }break;
        case SPELL_AURA_MANA_SHIELD:
        {
        }break;
        case SPELL_AURA_MOD_SKILL_TALENT:
        {
        }break;
        case SPELL_AURA_MOD_ATTACK_POWER:
        {
            
            apply ? SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS,GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS) + mod->GetAmount()) : SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS,GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS) - mod->GetAmount());
        }break;
        default:
        {
            Log::getSingleton().outError("Unknown affect id %u", (uint32)mod->GetType());
        }
    }
}


void Unit::_UpdateAura()
{
    if(GetTypeId() != TYPEID_PLAYER || !m_aura)
        return;

    Player* pThis = ObjectAccessor::Instance().FindPlayer(GetGUID());

    Player* pGroupGuy;
    Group* pGroup;

    if(pThis)
        pGroup = objmgr.GetGroupByLeader(pThis->GetGroupLeader());

    if(!SetAffDuration(m_aura->GetId(),this,6000))
        AddAffect(m_aura);

    if(!pGroup)
        return;
    else
    {
        for(uint32 i=0;i<pGroup->GetMembersCount();i++)
        {
	    pGroupGuy = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(i));
	
            if(!pGroupGuy)
                continue;
            if(pGroupGuy->GetGUID() == GetGUID())
                continue;
            if(sqrt(
                (GetPositionX()-pGroupGuy->GetPositionX())*(GetPositionX()-pGroupGuy->GetPositionX())
                +(GetPositionY()-pGroupGuy->GetPositionY())*(GetPositionY()-pGroupGuy->GetPositionY())
                +(GetPositionZ()-pGroupGuy->GetPositionZ())*(GetPositionZ()-pGroupGuy->GetPositionZ())
                ) <=30)
            {
                if(!pGroupGuy->SetAffDuration(m_aura->GetId(),this,6000))
                    pGroupGuy->AddAffect(m_aura);
            }
            else
            {
                if(m_removeAuraTimer == 0)
                {
                    printf("remove aura from %u\n", pGroupGuy->GetGUID());
                    pGroupGuy->RemoveAffectById(m_aura->GetId());
                    
                }
            }
        }
    }
    if(m_removeAuraTimer > 0)
        m_removeAuraTimer -= 1;
    else
        m_removeAuraTimer = 4;
}


void Unit::_UpdateSpells( uint32 time )
{
    if(m_currentSpell != NULL)
    {
        m_currentSpell->update(time);
        if(m_currentSpell->getState() == SPELL_STATE_FINISHED)
        {
            delete m_currentSpell;
            m_currentSpell = NULL;
        }
    }

    Affect *aff;
    AffectList::iterator i, next;
    for (i = m_affects.begin(); i != m_affects.end(); i = next)
    {
        next = i;
        next++;

        aff = *i;

        uint8 AffResult = aff->Update( time );
        if( AffResult == 2 || AffResult == 6 || AffResult == 10 || AffResult == 14)
        {
	    Unit *attacker = ObjectAccessor::Instance().FindPlayer(aff->GetCasterGUID());

            
            if(attacker)
            {
                if(this->isAlive())
                    attacker->PeriodicAuraLog(this, aff->GetId(), aff->GetDamagePerTick(),aff->GetSpellProto()->School);
            }
        }
        if( AffResult == 4 || AffResult == 6 || AffResult == 12 || AffResult == 14)
        {
            
            
            SpellEntry *spellInfo = sSpellStore.LookupEntry( aff->GetSpellPerTick() );

            if(!spellInfo)
            {
                Log::getSingleton( ).outError("WORLD: unknown spell id %i\n", aff->GetSpellPerTick());
                return;
            }
			
            Spell *spell = new Spell(this, spellInfo, true, aff);
            SpellCastTargets targets;
            WorldPacket dump;
            dump.Initialize(0);
            dump << uint16(2) << GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT) << GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT+1);
            targets.read(&dump,GetGUID());
			if( aff->GetDuration() == 0 ){
                for(uint32 tm=aff->GetSpellPerCurrenttimes();tm<aff->GetSpellPerMaxtimes();tm++ )
                   spell->prepare(&targets);
			}else{
			       spell->prepare(&targets);
				   aff->SetSpellPerCurrenttimes(aff->GetSpellPerCurrenttimes()+1);
			}
			
        }
        if( AffResult == 8 || AffResult == 10 || AffResult == 12 || AffResult == 14)
        {
	    Unit *attacker = ObjectAccessor::Instance().FindPlayer(aff->GetCasterGUID());

            
            if(attacker)
            {
                if(this->isAlive())
                {
                    if(GetUInt32Value(UNIT_FIELD_HEALTH) < GetUInt32Value(UNIT_FIELD_MAXHEALTH) + aff->GetHealPerTick())
                        SetUInt32Value(UNIT_FIELD_HEALTH,GetUInt32Value(UNIT_FIELD_HEALTH) + aff->GetHealPerTick());
                    else
                        SetUInt32Value(UNIT_FIELD_HEALTH,GetUInt32Value(UNIT_FIELD_MAXHEALTH));
                }
            }
        }

        if ( !aff->GetDuration() )
        {
            if(aff)
                RemoveAffect(aff);
        }
    }
}


void Unit::castSpell( Spell * pSpell )
{
    
    if(m_currentSpell)
    {
        m_currentSpell->cancel();
        delete m_currentSpell;
    }

    m_currentSpell = pSpell;
}


void Unit::InterruptSpell()
{
    if(m_currentSpell)
    {
        m_currentSpell->SendInterrupted(0x20);
        m_currentSpell->cancel();
        m_currentSpell = NULL;
    }
}


void Unit::_AddAura(Affect *aff)
{
    ASSERT(aff);

    if (!aff->GetId())
    {
        return;
    }

    

    WorldPacket data;

    uint8 slot, i;

    slot = 0xFF;

    if (aff->IsPositive())
    {
        for (i = 0; i < MAX_POSITIVE_AURAS; i++)
        {
            if (GetUInt32Value((uint16)(UNIT_FIELD_AURA + i)) == 0)
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
            if (GetUInt32Value((uint16)(UNIT_FIELD_AURA + i)) == 0)
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

    SetUInt32Value((uint16)(UNIT_FIELD_AURA + slot), aff->GetId());

    uint8 flagslot = slot >> 3;
    uint32 value = GetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot));
    value |= 0xFFFFFFFF & (AFLAG_SET << ((slot & 7) << 2));
    SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot), value);





    uint8 appslot = slot >> 1;



    if( GetTypeId() == TYPEID_PLAYER )
    {
        data.Initialize(SMSG_UPDATE_AURA_DURATION);
        data << (uint8)slot << (uint32)aff->GetDuration();
        ((Player*)this)->GetSession()->SendPacket(&data);
    }

    aff->SetAuraSlot( slot );

    return;
}


void Unit::_RemoveAura(Affect *aff)
{
    ASSERT(aff);

    

    uint8 slot = aff->GetAuraSlot();

    SetUInt32Value((uint16)(UNIT_FIELD_AURA + slot), 0);

    uint8 flagslot = slot >> 3;

    uint32 value = GetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot));
    value &= 0xFFFFFFFF ^ (0xF << ((slot & 7) << 2));
    SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot), value);



    aff->SetAuraSlot(0);

    return;
}


Affect* Unit::FindAff(uint32 spellId)
{
    AffectList::iterator i;
    for (i = m_affects.begin(); i != m_affects.end(); i++)
        if((*i)->GetId() == spellId)
            return (*i);
    return NULL;
}


float Unit::getdistance( float xe, float ye, float xz, float yz )
{
    return sqrt( ( xe - xz ) * ( xe - xz ) + ( ye - yz ) * ( ye - yz ) );
}





float Unit::getangle( float xe, float ye, float xz, float yz )
{
    float w;
    w = atan( ( yz - ye ) / ( xz - xe ) );
    w = ( w / (float)DEG2RAD );
    if (xz>=xe)
    {
        w = 90+w;
    }
    else
    {
        w = 270+w;
    }
    return w;
}


float Unit::geteasyangle( float angle )
{
    while ( angle < 0 )
    {
        angle = angle + 360;
    }
    while ( angle >= 360 )
    {
        angle = angle - 360;
    }
    return angle;
}


bool Unit::inarc( float radius, float xM, float yM, float fov, float orientation, float xP, float yP )
{
    float distance = getdistance( xM, yM, xP, yP );
    float angle = getangle( xM, yM, xP, yP );
    float lborder = geteasyangle( ( orientation - (fov/2) ) );
    float rborder = geteasyangle( ( orientation + (fov/2) ) );
    if(radius>=distance &&( ( angle >= lborder ) &&
        ( angle <= rborder ) ||
        ( lborder > rborder && ( angle < rborder || angle > lborder ) ) ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}


bool Unit::isInFront(Unit* target,float distance)
{
    float orientation = GetOrientation()/float(2*M_PI)*360;
    orientation += 90.0f;
    return inarc(distance,GetPositionX(),GetPositionY(),float(180),GetOrientation(),target->GetPositionX(),target->GetPositionY());
}



bool Unit::setInFront(Unit* target, float distance)
{
    
    Log::getSingleton().outError("Orentation Start: %f",GetOrientation());
    float orientation = GetOrientation()/float(2*M_PI)*360;
    orientation += 45.0f;
    Log::getSingleton().outError("Orentation b4 loop: %f",orientation);
    for(int i=0;i<8;i++)
    {
        if(inarc(distance,GetPositionX(),GetPositionY(),float(180),orientation,target->GetPositionX(),target->GetPositionY()) == true)
        {
            orientation = float(2*M_PI)/360*(orientation-90.0f);
            Log::getSingleton().outError("Orentation: %f",orientation);
	    m_orientation = orientation;
            break;
        }else
        orientation += 90;
    }
    Log::getSingleton().outError("Orentation after loop: %f",orientation);
    return inarc(distance,GetPositionX(),GetPositionY(),float(180),orientation,target->GetPositionX(),target->GetPositionY());
}


void Unit::DeMorph()
{
    
    uint32 displayid = this->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    this->SetUInt32Value(UNIT_FIELD_DISPLAYID, displayid);
}

float 
Unit::GetUnitDodgeChance()
{
    Player *pThis = ObjectAccessor::Instance().FindPlayer(GetGUID());

    if (pThis) 
    return (float)m_uint32Values[ PLAYER_DODGE_PERCENTAGE ]; 
    else
    return (float)0;
}

float 
Unit::GetUnitParryChance()
{ 
    Player *pThis = ObjectAccessor::Instance().FindPlayer(GetGUID());

    if (pThis) 
    return (float)m_uint32Values[ PLAYER_PARRY_PERCENTAGE ]; 
    else
    return (float)0;
}

float
Unit::GetUnitCriticalChance()
{ 
    Player *pThis = ObjectAccessor::Instance().FindPlayer(GetGUID());

    if (pThis) 
    return (float)m_uint32Values[ PLAYER_CRIT_PERCENTAGE ]; 
    else
    return (float)0;
}


float
Unit::GetUnitBlockChance()
{ 
    Player *pThis = ObjectAccessor::Instance().FindPlayer(GetGUID());

    if (pThis) 
    return (float)m_uint32Values[ PLAYER_BLOCK_PERCENTAGE ]; 
    else
    return (float)0;
}

bool
Unit::isUnit()
{
    Player *pThis = ObjectAccessor::Instance().FindPlayer(GetGUID());
    
    if (pThis)
    return false;
    else
    return true;
}


bool
Unit::isPlayer()
{
    Player *pThis = ObjectAccessor::Instance().FindPlayer(GetGUID());
    
    if (pThis)
    return true;
    else
    return false;
}






void
Unit::DealWithSpellDamage(DynamicObject &obj)
{
    obj.DealWithSpellDamage(*this);
}

