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
#include "SpellAuras.h"
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
    //m_Aura = NULL;
    //m_AurasCheck = 2000;
    //m_removeAuraTimer = 4;
    tmpAura = NULL;
    m_silenced = false;
	waterbreath = false;
    
    m_immuneToEffect = 0;
    m_immuneToState  = 0;
    m_immuneToSchool = 0;
    m_immuneToDmg    = 0; 
    m_immuneToDispel = 0;
    m_stealth        = 0;
    
    m_ReflectSpellSchool = 0;
    m_ReflectSpellPerc   = 0;

	m_damageManaShield = NULL;
}


Unit::~Unit()
{
}


void Unit::Update( uint32 p_time )
{
    /*if(p_time > m_AurasCheck)
    {
        m_AurasCheck = 2000;
        _UpdateAura();
    }else
    m_AurasCheck -= p_time;*/

    _UpdateSpells( p_time );

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

	uint32 crtype = 0;
	
	if(m_stealth)
  {
   	RemoveAura(m_stealth);
   	m_stealth = 0;
  }
	
	if(pVictim->GetTypeId() != TYPEID_PLAYER)
	{
		crtype = ((Creature*)pVictim)->GetCreatureInfo()->type;
	 	((Creature*)pVictim)->AI().AttackStart((Player*)this);
	}
	
  // no loot,xp,health if type 8 /critters/
  if ( crtype == 8)
	 ((Creature*)pVictim)->SetUInt32Value( UNIT_FIELD_HEALTH, 0);

  DEBUG_LOG("DealDamageStart");
  
  uint32 health = pVictim->GetUInt32Value(UNIT_FIELD_HEALTH );
  if (health <= damage && pVictim->isAlive())
  {
		DEBUG_LOG("DealDamage: victim just died");
		if(pVictim->GetTypeId() == TYPEID_UNIT && crtype != 8)
			((Creature*)pVictim)->generateLoot();
		
		//If a player kill some one call honor calcules
		((Player*)this)->CalculateHonor(pVictim);

		DEBUG_LOG("DealDamageAura");
		pVictim->RemoveAllAuras();

		pVictim->setDeathState(JUST_DIED);
		uint64 attackerGuid, victimGuid;
		attackerGuid = GetGUID();
		victimGuid = pVictim->GetGUID();

		// 10% durability loss on death
		if (pVictim->GetTypeId() == TYPEID_PLAYER)
		{
		DEBUG_LOG("We are dead, loosing 10 percents durability");
		((Player*)pVictim)->DeathDurabilityLoss(0.10);
		}

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
			if (crtype == 8)
				pVictim->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
			else 
				pVictim->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 1);
		}

		if ((GetTypeId() == TYPEID_PLAYER) && (crtype != 8))
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
			if( getClass() == WARRIOR )
				((Player*)this)->CalcRage(damage,true);
    }
		else
		{
			((Player*)pVictim)->addStateFlag(UF_ATTACKING);

			if( (((Player*)pVictim)->getClass()) == WARRIOR )
				((Player*)pVictim)->CalcRage(damage,false);

		// random durability for items (HIT)
                if (pVictim->GetTypeId() == TYPEID_PLAYER)
                {
			int randdurability = urand(0, 30);
			if (randdurability == 10)
			{
			DEBUG_LOG("HIT: We decrease durability with 5 percent");
			((Player*)pVictim)->DeathDurabilityLoss(0.05);
			}
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
        sLog.outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    Spell *spell = new Spell(caster, spellInfo, triggered, 0);
    WPAssert(spell);

    SpellCastTargets targets;
    targets.m_unitTarget = Victim;
    spell->prepare(&targets);
    m_canMove = false;
}

void Unit::SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage)
{
      
    if(!this || !pVictim)
        return;
    if(!this->isAlive() || !pVictim->isAlive())
        return;
    uint32 absorb=0;

	//if(pVictim->m_damageManaShield)
	//    if(pVictim->m_damageManaShield->m_modType == SPELL_AURA_SCHOOL_ABSORB)
 // FIX-ME
	SpellEntry *spellInfo = sSpellStore.LookupEntry(spellID);
	if(spellInfo)
		absorb=CalDamageAbsorb(pVictim,spellInfo->School,damage);

	WorldPacket data;

	if( (damage-absorb)==0 )
	{
        data.Initialize(SMSG_ATTACKERSTATEUPDATE);
        data << (uint32)0x00010020;       
        data << uint8(0xFF) << GetGUID();                    
        data << uint8(0xFF) << pVictim->GetGUID();      
        data << (uint32)0;                

        data << (uint8)1; // SubBlocks                   
        data << (uint32)0;                    
        data << (float)damage;                
        data << (uint32)damage;                
	    data <<  absorb;
	    data << (uint32)0;
        data << (uint32)1;
        data << (uint32)0xFFFFFFFF;
        data << (uint32)0;        
        data << (uint32)0;
        SendMessageToSet(&data, true);
	    return;
	}
	else damage=damage-absorb;

    sLog.outDetail("SpellNonMeleeDamageLog: %u %X attacked %u %X for %u dmg inflicted by %u",
        GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage, spellID);
    
    data.Initialize(SMSG_SPELLNONMELEEDAMAGELOG);
    data << uint8(0xFF) << pVictim->GetGUID();
    data << uint8(0xFF) << this->GetGUID();
    data << spellID;
    data << damage;
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    SendMessageToSet(&data,true);
    DealDamage(pVictim, damage,0);
}

void Unit::PeriodicAuraLog(Unit *pVictim, SpellEntry *spellProto, Modifier *mod)
{
    uint32 procFlag = 0;
    if(!this || !pVictim)
        return;
    if(!this->isAlive() || !pVictim->isAlive())
        return;
    sLog.outDetail("PeriodicAuraLog: %u %X attacked %u %X for %u dmg inflicted by %u",
        GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), mod->m_amount, spellProto->Id);

    WorldPacket data;
    data.Initialize(SMSG_PERIODICAURALOG);
    data << uint8(0xFF) << pVictim->GetGUID();
    data << uint8(0xFF) << this->GetGUID();
    data << spellProto->Id;
    data << uint32(1);                            

    data << mod->m_auraname;
    data << (uint32)(mod->m_amount);
    data << spellProto->School;
    data << uint32(0);
    SendMessageToSet(&data,true);

    if(mod->m_auraname == 3) DealDamage(pVictim, mod->m_amount, procFlag);
	else if(mod->m_auraname == 8)
	{
		if(GetUInt32Value(UNIT_FIELD_HEALTH) < GetUInt32Value(UNIT_FIELD_MAXHEALTH) + mod->m_amount)
			SetUInt32Value(UNIT_FIELD_HEALTH,GetUInt32Value(UNIT_FIELD_HEALTH) + mod->m_amount);
		else
			SetUInt32Value(UNIT_FIELD_HEALTH,GetUInt32Value(UNIT_FIELD_MAXHEALTH));
	}
}

void Unit::HandleEmoteCommand(uint32 anim_id)
{
    WorldPacket data;

    data.Initialize( SMSG_EMOTE );
    data << anim_id << GetGUID();
    WPAssert(data.size() == 12);
    
    SendMessageToSet(&data, true);
}

uint32 Unit::CalDamageAbsorb(Unit *pVictim,uint32 School,const uint32 damage)
{
	int32 AbsorbDamage=0;
	int32 currAbsorbDamage=0;
    uint32 currentPower;
	bool  removeAura=false;

	if(!pVictim)
		return 0;

	if( pVictim->m_damageManaShield && pVictim->isAlive() ) 
	{
		//check if absorb
		if( pVictim->m_damageManaShield->m_schoolAbsorb != ALL_DAMAGE_ABSORB )
		{
			if( ( pVictim->m_damageManaShield->m_schoolAbsorb == ONLY_MAGIC_ABSORB) &&
                ( School == NORMAL_DAMAGE ) )
				return 0;
			else
				if( School != pVictim->m_damageManaShield->m_schoolAbsorb )
					return 0;
		}

		//calculate absorb damage
		currAbsorbDamage= damage+ pVictim->m_damageManaShield->m_currAbsorb;
  	    if( currAbsorbDamage < pVictim->m_damageManaShield->m_totalAbsorb )
		{
			AbsorbDamage = damage; 
            pVictim->m_damageManaShield->m_currAbsorb = currAbsorbDamage; 
		}else{
            AbsorbDamage = pVictim->m_damageManaShield->m_totalAbsorb 
				           - pVictim->m_damageManaShield->m_currAbsorb;
            pVictim->m_damageManaShield->m_currAbsorb =
				           pVictim->m_damageManaShield->m_totalAbsorb;
			removeAura = true;
		}

		if(pVictim->m_damageManaShield->m_modType == SPELL_AURA_MANA_SHIELD)
		{   
			currentPower = pVictim->GetUInt32Value(UNIT_FIELD_POWER1);
			if ( currentPower > AbsorbDamage*2 )
			{
			    pVictim->SetUInt32Value(UNIT_FIELD_POWER1, currentPower-AbsorbDamage*2 );
			}else{
                pVictim->SetUInt32Value(UNIT_FIELD_POWER1, 0 );
				AbsorbDamage = AbsorbDamage - currentPower/2;
				removeAura = true;
			}
		}

		if(removeAura)
			pVictim->RemoveAura(pVictim->m_damageManaShield->m_spellId);
	}


	// random durability loss for items on absorb (ABSORB)
	if (pVictim->GetTypeId() == TYPEID_PLAYER)
	{
		int randdurability = urand(0, 30);
		if (randdurability == 10)
		{
			DEBUG_LOG("BLOCK: We decrease durability with 5 percent");
			((Player*)pVictim)->DeathDurabilityLoss(0.05);
		}
	}

	return AbsorbDamage;
}

void Unit::DoAttackDamage(Unit *pVictim, uint32 *damage, uint32 *blocked_amount, uint32 *damageType, uint32 *hitInfo, uint32 *victimState,uint32 *absorbDamage,uint32 *turn)
{
    uint32 absorb=CalDamageAbsorb(pVictim,NORMAL_DAMAGE,*damage);

	if( (*damage-absorb) == 0 )
	{
		*hitInfo = 0x00010020;
		*turn=0;
		*victimState=1;
		*blocked_amount=0;
		*absorbDamage=*damage;
		*damageType = 0;
		return;
	}
	else
		*damage -= absorb;

	if(GetTypeId() == TYPEID_PLAYER)
	{
		for(int i = 0; i < EQUIPMENT_SLOT_END; i++)
			((Player*)this)->CastItemSpell(((Player*)this)->GetItemBySlot(i),pVictim);
	}

	if(GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() != TYPEID_PLAYER && ((Creature*)pVictim)->GetCreatureInfo()->type != 8 )
		((Player*)this)->UpdateSkillWeapon();

	if (GetTypeId() == TYPEID_PLAYER && (GetUnitCriticalChance()/100) * 512 >= urand(0, 512))
	{
		*hitInfo = 0xEA;
		*damage *= 2;
              		       
		pVictim->HandleEmoteCommand(EMOTE_ONESHOT_WOUNDCRITICAL);
	}
	else if (pVictim->GetTypeId() == TYPEID_PLAYER && (pVictim->GetUnitParryChance()/100) * 512 >= urand(0, 512))
	{    
		*damage = 0;
		*victimState = 2;
    
    ((Player*)pVictim)->UpdateDefense();
		HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);
	}
	else if (pVictim->GetTypeId() == TYPEID_PLAYER && (pVictim->GetUnitDodgeChance()/100) * 512 >= urand(0, 512))
	{
		*damage = 0;
		*victimState = 3;

		((Player*)pVictim)->UpdateDefense(); 
		HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);
	}
	else if (pVictim->GetTypeId() == TYPEID_PLAYER && (pVictim->GetUnitBlockChance()/100) * 512 >= urand(0, 512))
	{
		*blocked_amount = (pVictim->GetUnitBlockValue() * (pVictim->GetUnitStrength() / 10));
            
		if (*blocked_amount < *damage) *damage = *damage - *blocked_amount;
		else *damage = 0;

		if (pVictim->GetUnitBlockValue())
			HandleEmoteCommand(EMOTE_ONESHOT_PARRYSHIELD);
		else
			HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);

		*victimState = 4;
		((Player*)pVictim)->UpdateDefense(); 
	}

	if(pVictim->m_currentSpell && pVictim->GetTypeId() == TYPEID_PLAYER && *damage)
	{
		sLog.outString("Spell Delayed!");
		pVictim->m_currentSpell->Delayed((int32)(0.25f * pVictim->m_currentSpell->casttime));
	}
}

void Unit::AttackerStateUpdate (Unit *pVictim, uint32 damage)
{
    WorldPacket data;
    uint32    hitInfo = 0x22;
    uint32    damageType = 0;
    uint32    blocked_amount = 0;
    uint32    victimState = 1;
    int32    attackerSkill = GetUnitMeleeSkill();
    int32    victimSkill = pVictim->GetUnitMeleeSkill();
    float    chanceToHit = 100.0f;
		uint32   AbsorbDamage = 0;
		uint32   Turn=0;

    uint32    victimAgility = pVictim->GetUInt32Value(UNIT_FIELD_AGILITY);
    uint32    attackerAgility = pVictim->GetUInt32Value(UNIT_FIELD_AGILITY);
 
 		if (pVictim->isDead())
    {
        smsg_AttackStop(pVictim->GetGUID());
        return;
    }

    if (m_currentSpell) 
        return;
       
    //if(isStunned()) return;
      
    if (GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() == TYPEID_PLAYER)
    {
        if (attackerSkill <= victimSkill - 24) 
        	chanceToHit = 0;
        else if (attackerSkill <= victimSkill)
          chanceToHit = 100.0f - (victimSkill - attackerSkill) * (100.0f / 30.0f);
        
        if (chanceToHit < 15.0f) 
            chanceToHit = 15.0f;
    }  
      
		if(!damage) damage = CalculateDamage (this);

    if((chanceToHit/100) * 512 >= urand(0, 512) )
    {
			if(GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() == TYPEID_PLAYER)
			{
	      	    if (attackerSkill < victimSkill - 20)
	      		    damage = (damage * 30) / 100;
	       	    else if (attackerSkill < victimSkill - 10) 
	      		        damage = (damage * 60) / 100;
			}
		}
		else damage = 0;
		
    if (damage)
    	DoAttackDamage(pVictim, &damage, &blocked_amount, &damageType, &hitInfo, &victimState,&AbsorbDamage,&Turn);

       
    data.Initialize(SMSG_ATTACKERSTATEUPDATE);
    data << (uint32)hitInfo;            
    data << uint8(0xFF)<< GetGUID();                    
    data << uint8(0xFF)<< pVictim->GetGUID();            
    data << (uint32)(damage-AbsorbDamage);                

    data << (uint8)1; // SubBlocks                   
    data << damageType;                    
    data << (float)damage;                
    data << (uint32)damage;                
		data <<  AbsorbDamage;
		data <<  Turn;
    data << (uint32)victimState;
    if(AbsorbDamage==0)
			data << (uint32)0;   
		else
			data << (uint32)0xFFFFFFFF;

    data << (uint32)0;        
    data << (uint32)blocked_amount;

    SendMessageToSet(&data, true);

    if (GetTypeId() == TYPEID_PLAYER)
        DEBUG_LOG("AttackerStateUpdate: (Player) %u %X attacked %u %X for %u dmg.",
            GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage);
    else
        DEBUG_LOG("AttackerStateUpdate: (NPC) %u %X attacked %u %X for %u dmg.",
            GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage);

    DealDamage(pVictim, damage-AbsorbDamage, 0);
}

void Unit::smsg_AttackStop(uint64 victimGuid)
{
    WorldPacket data;
    data.Initialize( SMSG_ATTACKSTOP );
    data << uint8(0xFF) << GetGUID();
    data << uint8(0xFF) << victimGuid;
    data << uint32( 0 );
    data << (uint32)0;                    

    SendMessageToSet(&data, true);
    sLog.outDetail("%u %X stopped attacking "I64FMT,
				   GetGUIDLow(), GetGUIDHigh(), victimGuid);

    
	Creature *pVictim = ObjectAccessor::Instance().GetCreature(*this, victimGuid);
	if( pVictim != NULL )
	    pVictim->AI().AttackStop(this);

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

    AuraList::iterator i;
    for (i = m_Auras.begin(); i != m_Auras.end(); i++)
    {
      (*i)->Update( time );
	  if ( !(*i)->GetDuration() && !(*i)->IsPermanent() )
          RemoveAura(i);
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
    
    sLog.outError("Orentation Start: %f",GetOrientation());
    float orientation = GetOrientation()/float(2*M_PI)*360;
    orientation += 45.0f;
    sLog.outError("Orentation b4 loop: %f",orientation);
    for(int i=0;i<8;i++)
    {
        if(inarc(distance,GetPositionX(),GetPositionY(),float(180),orientation,target->GetPositionX(),target->GetPositionY()) == true)
        {
            orientation = float(2*M_PI)/360*(orientation-90.0f);
            sLog.outError("Orentation: %f",orientation);
	    m_orientation = orientation;
            break;
        }else
        orientation += 90;
    }
    sLog.outError("Orentation after loop: %f",orientation);
    return inarc(distance,GetPositionX(),GetPositionY(),float(180),orientation,target->GetPositionX(),target->GetPositionY());
}


void Unit::DeMorph()
{
    
    uint32 displayid = GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    SetUInt32Value(UNIT_FIELD_DISPLAYID, displayid);
}

void Unit::DealWithSpellDamage(DynamicObject &obj)
{
    obj.DealWithSpellDamage(*this);
}

bool Unit::AddAura(Aura *Aur, bool uniq)
{
    AuraList::const_iterator i;

	_RemoveStatsMods();

    for (i = m_Auras.begin(); i != m_Auras.end(); i++)
    {
        if ((*i)->GetId() == Aur->GetId() &&
            (uniq || (*i)->GetCaster() == Aur->GetCaster()))
        {
            break;
        }
    }
	// take out same spell
    if (i != m_Auras.end())
		(*i)->SetDuration(0);

    m_Auras.push_back(Aur);

    Aur->ApplyModifiers(true);
    Aur->_AddAura();

	_ApplyStatsMods();

    return true;
}

void Unit::RemoveFirstAuraByCategory(uint32 category)
{
	AuraList::iterator i;
    for (i = m_Auras.begin(); i != m_Auras.end(); i++)
    {
        if ((*i)->GetSpellProto()->Category == category)
            break;
    }

    if(i == m_Auras.end()) return;

	RemoveAura(i);	
}

void Unit::RemoveAura(uint32 spellId)
{
	AuraList::iterator i;
    for (i = m_Auras.begin(); i != m_Auras.end(); i++)
    {
        if ((*i)->GetId() == spellId)
            break;
    }

    if(i == m_Auras.end()) return;

	RemoveAura(i);
}

void Unit::RemoveAura(AuraList::iterator i)
{
	_RemoveStatsMods();

    if((*i)->GetCoAura() != 0)
        RemoveAura((*i)->GetCoAura()->GetId());

    (*i)->ApplyModifiers(false);

    (*i)->_RemoveAura();
    
    delete *i;
    m_Auras.erase(i);
    
    _ApplyStatsMods();
}

bool Unit::SetAurDuration(uint32 spellId,Unit* caster,uint32 duration)
{
    AuraList::iterator i;

    for (i = m_Auras.begin(); i != m_Auras.end(); i++)
		if ((*i)->GetId() == spellId && (*i)->GetCaster() == caster)
        {
            (*i)->SetDuration(duration);
            return true;
        }

    return false;
}


uint32 Unit::GetAurDuration(uint32 spellId,Unit* caster)
{
    AuraList::iterator i;

    for (i = m_Auras.begin(); i != m_Auras.end(); i++)
		if ((*i)->GetId() == spellId && (*i)->GetCaster() == caster)
			return (*i)->GetDuration();

    return 0;
}

void Unit::RemoveAllAuras()
{
    
    AuraList::iterator i;
    
	_RemoveStatsMods();

	for (i = m_Auras.begin(); i != m_Auras.end(); i++)
    {
       (*i)->ApplyModifiers(false);
       (*i)->_RemoveAura();
    }

    m_Auras.clear();
    
    _ApplyStatsMods();
}

void Unit::_RemoveStatsMods()
{
	ApplyStats(false);
}

void Unit::_ApplyStatsMods()
{
	ApplyStats(true);
}

void Unit::ApplyStats(bool apply)
{
	// TODO:
	// -- add --
	// spell crit formula: 5 + INT/100
	// skill formula:  skill*0,04 for all, use defense skill for parry/dodge
	// froze spells gives + 50% change to crit
	
	if(GetTypeId() != TYPEID_PLAYER) return;
	
	PlayerCreateInfo* pinfo = ((Player*)this)->GetPlayerInfo();
	if(!pinfo) return;
	
	float val;
	uint32 val2;
	
	// Armor
	val2 = (uint32)(2*GetUInt32Value(UNIT_FIELD_AGILITY));
	if(val2 > GetUInt32Value(UNIT_FIELD_ARMOR) && !apply) val2 = 0;
	apply ? 
	SetUInt32Value(UNIT_FIELD_ARMOR,GetUInt32Value(UNIT_FIELD_ARMOR)+val2):
	SetUInt32Value(UNIT_FIELD_ARMOR,GetUInt32Value(UNIT_FIELD_ARMOR)-val2);
	
	// HP
	val2 = (uint32)((GetUInt32Value(UNIT_FIELD_STAMINA) - pinfo->stamina)*10);
	if(val2 > GetUInt32Value(UNIT_FIELD_MAXHEALTH) && !apply) val2 = 0;
	apply ? 
	SetUInt32Value(UNIT_FIELD_MAXHEALTH,GetUInt32Value(UNIT_FIELD_MAXHEALTH)+val2):
	SetUInt32Value(UNIT_FIELD_MAXHEALTH,GetUInt32Value(UNIT_FIELD_MAXHEALTH)-val2);
	
	// MP
	if(getClass() != WARRIOR && getClass() != ROGUE)
	{
		val2 = (uint32)((GetUInt32Value(UNIT_FIELD_IQ) - pinfo->intellect)*15);
		if(val2 > GetUInt32Value(UNIT_FIELD_MAXPOWER1) && !apply) val2 = 0;
		apply ? 
		SetUInt32Value(UNIT_FIELD_MAXPOWER1,GetUInt32Value(UNIT_FIELD_MAXPOWER1)+val2):
		SetUInt32Value(UNIT_FIELD_MAXPOWER1,GetUInt32Value(UNIT_FIELD_MAXPOWER1)-val2);
	}
	
	
	float classrate = 0;

	// Melee Attack Power
	// //needs testing, prob wrong value type, or field position

	/*switch(getClass())
	{
		case WARRIOR: val2 = (uint32)(getLevel()*3 + GetUInt32Value(UNIT_FIELD_STR)*2 - 20); break;
		case PALADIN: val2 = (uint32)(getLevel()*3 + GetUInt32Value(UNIT_FIELD_STR)*2 - 20); break;
		case ROGUE:   val2 = (uint32)(getLevel()*2 + GetUInt32Value(UNIT_FIELD_STR) + GetUInt32Value(UNIT_FIELD_AGILITY) - 20); break;
		case HUNTER:  val2 = (uint32)(getLevel()*2 + GetUInt32Value(UNIT_FIELD_STR) + GetUInt32Value(UNIT_FIELD_AGILITY) - 20); break;
		case SHAMAN:  val2 = (uint32)(getLevel()*2 + GetUInt32Value(UNIT_FIELD_STR)*2 - 20); break;
		case DRUID:   val2 = (uint32)(GetUInt32Value(UNIT_FIELD_STR)*2 - 20); break;
		case MAGE:    val2 = (uint32)(GetUInt32Value(UNIT_FIELD_STR) - 10); break;
		case PRIEST:  val2 = (uint32)(GetUInt32Value(UNIT_FIELD_STR) - 10); break;
		case WARLOCK: val2 = (uint32)(GetUInt32Value(UNIT_FIELD_STR) - 10); break;
	}

	if(val2 > GetUInt32Value(UNIT_FIELD_ATTACKPOWER) && !apply) val2 = 0;
	apply ? 
	SetUInt32Value(UNIT_FIELD_ATTACKPOWER,GetUInt32Value(UNIT_FIELD_ATTACKPOWER)+val2):
	SetUInt32Value(UNIT_FIELD_ATTACKPOWER,GetUInt32Value(UNIT_FIELD_ATTACKPOWER)-val2); */

	// Melee DPS - (Damage Per Second)
	//needs testing, prob wrong value type, or field position
	
	/*val = (float)((GetUInt32Value(UNIT_FIELD_ATTACKPOWER)/14)*GetUInt32Value(UNIT_FIELD_BASEATTACKTIME) );
	if(val > GetFloatValue(UNIT_FIELD_MINDAMAGE) && val > GetFloatValue(UNIT_FIELD_MAXDAMAGE) && !apply) val = 0;

	apply ? 
	SetFloatValue(UNIT_FIELD_MINDAMAGE, GetFloatValue(UNIT_FIELD_MINDAMAGE)+val):
	SetFloatValue(UNIT_FIELD_MINDAMAGE, GetFloatValue(UNIT_FIELD_MINDAMAGE)-val);

	apply ? 
	SetFloatValue(UNIT_FIELD_MAXDAMAGE, GetFloatValue(UNIT_FIELD_MAXDAMAGE)+val):
	SetFloatValue(UNIT_FIELD_MAXDAMAGE, GetFloatValue(UNIT_FIELD_MAXDAMAGE)-val);*/

	// critical
	if(getClass() == HUNTER) classrate = 53;
	else if(getClass() == ROGUE)  classrate = 29;
	else classrate = 20;
	

	val = (float)(5 + GetUInt32Value(UNIT_FIELD_AGILITY)/classrate);
	if(val > GetFloatValue(PLAYER_CRIT_PERCENTAGE)  && !apply) val = 0;
		
	apply ? 
	SetFloatValue(PLAYER_CRIT_PERCENTAGE, GetFloatValue(PLAYER_CRIT_PERCENTAGE)+val):
	SetFloatValue(PLAYER_CRIT_PERCENTAGE, GetFloatValue(PLAYER_CRIT_PERCENTAGE)-val);
	
	//dodge
	if(getClass() == HUNTER) classrate = 26.5;
	else if(getClass() == ROGUE)  classrate = 14.5;
	else classrate = 20;


	val = (float)(GetUInt32Value(UNIT_FIELD_AGILITY)/classrate); ///*+(Defense*0,04);
	if(val > GetFloatValue(PLAYER_DODGE_PERCENTAGE)  && !apply) val = 0;
		
	apply ? 
	SetFloatValue(PLAYER_DODGE_PERCENTAGE, GetFloatValue(PLAYER_DODGE_PERCENTAGE)+val):
	SetFloatValue(PLAYER_DODGE_PERCENTAGE, GetFloatValue(PLAYER_DODGE_PERCENTAGE)-val);

	//parry
	val = (float)(5);
	if(val > GetFloatValue(PLAYER_PARRY_PERCENTAGE)  && !apply) val = 0;
	
	apply ? 
	SetFloatValue(PLAYER_PARRY_PERCENTAGE, GetFloatValue(PLAYER_PARRY_PERCENTAGE)+val):
	SetFloatValue(PLAYER_PARRY_PERCENTAGE, GetFloatValue(PLAYER_PARRY_PERCENTAGE)-val);

	//block	
	val = (float)(GetUInt32Value(UNIT_FIELD_STR)/22);
	if(val > GetFloatValue(PLAYER_BLOCK_PERCENTAGE)  && !apply) val = 0;
	
	apply ? 
	SetFloatValue(PLAYER_BLOCK_PERCENTAGE, GetFloatValue(PLAYER_BLOCK_PERCENTAGE)+val):
	SetFloatValue(PLAYER_BLOCK_PERCENTAGE, GetFloatValue(PLAYER_BLOCK_PERCENTAGE)-val);
	
}	

void Unit::_RemoveAllAuraMods()
{
    AuraList::iterator i;
    
    _RemoveStatsMods();

    for (i = m_Auras.begin(); i != m_Auras.end(); i++)
    {
       (*i)->ApplyModifiers(false);
       (*i)->_RemoveAura();
    }
    
    _ApplyStatsMods();
}


void Unit::_ApplyAllAuraMods()
{
    AuraList::iterator i;
    
    _RemoveStatsMods();

    for (i = m_Auras.begin(); i != m_Auras.end(); i++)
    {
       (*i)->ApplyModifiers(true);
       (*i)->_RemoveAura();
    }
    
    _ApplyStatsMods();
}




// TODO: FIX-ME!!!
/*void Unit::_UpdateAura()
{
	if(GetTypeId() != TYPEID_PLAYER || !m_Auras)
		return;

	Player* pThis = (Player*)this;

	Player* pGroupGuy;
	Group* pGroup;

	pGroup = objmgr.GetGroupByLeader(pThis->GetGroupLeader());

	if(!SetAffDuration(m_Auras->GetId(),this,6000))
		AddAura(m_Auras);

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
				if(!pGroupGuy->SetAffDuration(m_Auras->GetId(),this,6000))
				pGroupGuy->AddAura(m_Auras);
			}
			else
			{
				if(m_removeAuraTimer == 0)
				{
					printf("remove aura from %u\n", pGroupGuy->GetGUID());
					pGroupGuy->RemoveAura(m_Auras->GetId());     
				}
			}
		}
	}
	if(m_removeAuraTimer > 0)
		m_removeAuraTimer -= 1;
	else
		m_removeAuraTimer = 4;
}*/

Aura* Unit::FindAur(uint32 spellId)
{
    AuraList::iterator i;
    for (i = m_Auras.begin(); i != m_Auras.end(); i++)
        if((*i)->GetId() == spellId)
            return (*i);
    return NULL;
}

