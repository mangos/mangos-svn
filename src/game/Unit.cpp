/* Unit.cpp
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

#include "Common.h"
#include "Log.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Unit.h"
#include "Quest.h"
#include "Player.h"
#include "Creature.h"
#include "Spell.h"
#include "Stats.h"
#include "Group.h"
#include "Affect.h"
#include <math.h>

#ifdef ENABLE_GRID_SYSTEM
#include "MapManager.h"
#include "ObjectAccessor.h"
#endif

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
    Log::getSingleton( ).outError("DealDamageStart");
    uint32 health = pVictim->GetUInt32Value(UNIT_FIELD_HEALTH );
    if (health <= damage && pVictim->isAlive())
    {
        Log::getSingleton( ).outError("DealDamageDied");
        if(pVictim->GetTypeId() == TYPEID_UNIT)
            ((Creature*)pVictim)->generateLoot();

/*
    // FIXME: should we remove all equipment affects too
        if(pVictim->GetTypeId() == TYPEID_PLAYER)
            _RemoveAllItemMods();
*/
        Log::getSingleton( ).outError("DealDamageAffects");
        pVictim->RemoveAllAffects();

        /* victim died! */
        pVictim->setDeathState(JUST_DIED);

/*
    Send SMSG_PARTYKILLLOG 0x1e6
    To everyone in the party?
*/
        /* SMSG_ATTACKSTOP */
        uint64 attackerGuid, victimGuid;
        attackerGuid = GetGUID();
        victimGuid = pVictim->GetGUID();

        Log::getSingleton( ).outError("DealDamageAttackStop");
        pVictim->smsg_AttackStop(attackerGuid);

        /* Send MSG_MOVE_ROOT   0xe7 */

/*
    Set update values... try flags 917504
    health
*/
        Log::getSingleton( ).outError("DealDamageHealth1");
        pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);

        /* then another update message, sets health to 0, maxhealth to 100, and dynamic flags */
        Log::getSingleton( ).outError("DealDamageHealth2");
        pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
        pVictim->RemoveFlag(UNIT_FIELD_FLAGS, 0x00080000);

        if (pVictim->GetTypeId() != TYPEID_PLAYER)
        {
            Log::getSingleton( ).outError("DealDamageNotPlayer");
            pVictim->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 1);
        }

        if (GetTypeId() == TYPEID_PLAYER)
        {
            Log::getSingleton( ).outError("DealDamageIsPlayer");
            uint32 xp = CalculateXpToGive(pVictim, this);

            // check running quests in case this monster belongs to it
            uint32 entry = 0;
            if (pVictim->GetTypeId() != TYPEID_PLAYER)
                entry = pVictim->GetUInt32Value(OBJECT_FIELD_ENTRY );

            // Is this player part of a group?
            Group *pGroup = objmgr.GetGroupByLeader(((Player*)this)->GetGroupLeader());
            if (pGroup)
            {
                Log::getSingleton( ).outError("DealDamageInGroup");
                xp /= pGroup->GetMembersCount();
                for (uint32 i = 0; i < pGroup->GetMembersCount(); i++)
                {
#ifndef ENABLE_GRID_SYSTEM
                    Player *pGroupGuy = objmgr.GetObject<Player>(pGroup->GetMemberGUID(i));
#else
		    Player *pGroupGuy = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(i));
#endif
                    pGroupGuy->GiveXP(xp, victimGuid);

                    if (pVictim->GetTypeId() != TYPEID_PLAYER)
                        pGroupGuy->KilledMonster(entry, victimGuid);
                }
            }
            else
            {
                Log::getSingleton( ).outError("DealDamageNotInGroup");
                // update experience
                ((Player*)this)->GiveXP(xp, victimGuid);

                if (pVictim->GetTypeId() != TYPEID_PLAYER)
                    ((Player*)this)->KilledMonster(entry, victimGuid);
            }
        }
        else
        {
            Log::getSingleton( ).outError("DealDamageIsCreature");
            smsg_AttackStop(victimGuid);
            RemoveFlag(UNIT_FIELD_FLAGS, 0x00080000);
            addStateFlag(UF_TARGET_DIED);
        }
    }
    else
    {
        Log::getSingleton( ).outError("DealDamageAlive");
        pVictim->SetUInt32Value(UNIT_FIELD_HEALTH , health - damage);

        // this need alot of work.
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
        {

            Log::getSingleton( ).outDetail("Attacking back");
            // when attacked mobs stop moving around
            ((Creature*)pVictim)->AI_ChangeState(ATTACKING);
            ((Creature*)pVictim)->AI_AttackReaction(this, damage);
/*
            // uint32 max_health = GetUInt32Value(UNIT_FIELD_MAXHEALTH);
            // uint32 health_porcent = (max_health*10)/100; // this if for know 10% of total healt,need changes about mobs lvls
            pVictim->AI_ChangeState(3); //if mob are attack then they stop moving around
            pVictim->AI_AttackReaction(pAttacker, damage);

            //well mobs scape if have a movement assignet atm
            // if(health<=health_porcent)
            {
            }
*/
            if( getClass() == 1 )
            {
                CalcRage(damage);
            }

        }
        else
        {
            /* player is been attacked so they can't regen */
            ((Player*)pVictim)->addStateFlag(UF_ATTACKING);

            if( (((Player*)pVictim)->getClass()) == 1 )
            {
                ((Player*)pVictim)->CalcRage(damage);
            }
        }

        // Deal Damage to Attacker
        for(std::list<struct DamageShield>::iterator i = pVictim->m_damageShields.begin();i != pVictim->m_damageShields.end();i++)
        {
            pVictim->SpellNonMeleeDamageLog(this,i->m_spellId,i->m_damage);
        }
        /* Commented out as it is not doing any thing ? a few printf's maybe besides that nothing
        for(std::list<struct ProcTriggerSpell>::iterator itr = pVictim->m_procSpells.begin();itr != pVictim->m_procSpells.end();itr++)  // Proc Trigger Spells for Victim
        {
            printf("Proc Trigger spell: %u\n", itr->spellId);
            printf("proc: %u, %u, %u\n", itr->procChance,itr->procFlags,itr->procCharges);
            // if(rand()%100 < itr->procChance);
            //     pVictim->HandleProc(itr, procFlag);
        }

        for(std::list<struct ProcTriggerSpell>::iterator itr = m_procSpells.begin();itr != m_procSpells.end();itr++)  // Proc Trigger Spells for Attacker
        {
            printf("Proc Trigger spell: %u\n", itr->spellId);
            printf("proc: %u, %u, %u\n", itr->procChance,itr->procFlags,itr->procCharges);
            // if(rand()%100 < itr->procChance);
            //     HandleProc(itr, procFlag);
        }
*/

    }
    Log::getSingleton( ).outError("DealDamageEnd");
}


/*
void Unit::HandleProc(ProcTriggerSpell* pts, uint32 flag)
{
    cast = false;
    switch(procFlags)
    {
    case 1:{        // on hit melee
           }break;
    case 2:{        // on struck melee
           }break;
    case 4:{        // on kill xp giver
        }break;
    case 8:{        // unknown
        }break;
    case 16:{       // on dodge
        }break;
    case 32:{       // unknown
        }break;
    case 64:
    case 66:{       // on block
        }break;
    case 112:{      // unknown
        }break;
    case 128:
    case 129:{      // on next melee attack
        }break;
    case 256:{      // on cast spell
        }break;
    case 1026:{     // on struck
        }break;
    case 1138:
    case 1139:{     // unknown
        }break;
    case 2048:{     // on hit ranged
        }break;
    case 4096:{     // on hit critical
        }break;
    case 8192:{     // on struck critical melee
        }break;
    case 16384:{    // on cast spell
        }break;
    case 32768:{    // on take damage
        }break;
    case 65536:{    // on hit critical spell
        }break;
    case 69632:{    // on critical melee
        }break;
    case 131072:{   // on hit spell
        }break;
    case 270336:{   // on struck critical
        }break;
    case 1048578:{  // on struck in combat
        }break;
    case 1049602:{  // on struck melee/ranged
        }break;
    default:{
        }break;
    }
    if(cast)
        CastSpell(this, pVictim, itr->spellId, true);
}
*/

void Unit::CastSpell(Unit* caster,Unit* Victim, uint32 spellId, bool triggered)
{
    // check for spell id
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
    /* Calculate Rage */
    uint32 oldRage = GetUInt32Value(UNIT_FIELD_POWER2); uint32 maxRage = GetUInt32Value(UNIT_FIELD_MAXPOWER2);
    uint32 Rage = oldRage + damage*2;               /* 2x damage */
    if(Rage == oldRage) Rage = oldRage + 1;         /* min rate of 1 */
    if (Rage > maxRage) Rage = maxRage;             /* set the new rage */
    SetUInt32Value(UNIT_FIELD_POWER2, Rage);
}


void Unit::RegenerateAll()
{
    /* Added so that every thing regenerates at the same time
   instead of one regenerating till full then other starting	*/
    /* check if it's time to regen health */
    if (m_regenTimer != 0)
        return;
    uint32 regenDelay = 2000;
    /* Regenerate health, mana and energy if necessary. */
    if (!(m_state & UF_ATTACKING))                /* NOT in Combat */
    {
        Regenerate( UNIT_FIELD_HEALTH, UNIT_FIELD_MAXHEALTH, true );
        Regenerate( UNIT_FIELD_POWER2, UNIT_FIELD_MAXPOWER2, false );
    }
    /* Mana Regenerates while in combat but not for 5 seconds after each spell */
    Regenerate( UNIT_FIELD_POWER4, UNIT_FIELD_MAXPOWER4, true );
    Regenerate( UNIT_FIELD_POWER1, UNIT_FIELD_MAXPOWER1, true );

    m_regenTimer = regenDelay;

}


/// Regenerates the regenField's curValue to the maxValue/// Right now, everything regenerates at the same rate/// A possible mod is to add another parameter, the stat regeneration is based off of (Intelligence for mana, Strength for HP)/// And build a regen rate based on that
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

/*
    // Log::getSingleton( ).outError("Regen Health Rate: %f\n", HealthIncreaseRate);
    // Log::getSingleton( ).outError("Regen Mana Rate: %f\n", ManaIncreaseRate);
    // Log::getSingleton( ).outError("Regen Rage Rate: %f\n", RageIncreaseRate);
    // Log::getSingleton( ).outError("Regen Energy Rate: %f\n", EnergyIncreaseRate);
    // Log::getSingleton( ).outError("Spirit: %i\n", Spirit);
    // Log::getSingleton( ).outError("CurrentValue: %i\n", curValue);
    // Log::getSingleton( ).outError("Class: %i\n", Class);
*/

    uint32 oldCurValue = curValue;

    uint32 addvalue = 0;

    switch (field_cur)
    {
        /* mod by hann */
        case UNIT_FIELD_HEALTH:{
            switch (Class)
            {
                case 1: /* Warrior = 1 */{
                    addvalue = uint32(((Spirit*0.80) * HealthIncreaseRate));
                    break;              }
               case 2: /* Paladin = 2 */{
                    addvalue = uint32(((Spirit*0.25) * HealthIncreaseRate));
                    break;              }
                case 3: /* Hunter = 3 */{
                    addvalue = uint32(((Spirit*0.25) * HealthIncreaseRate));
                    break;              }
               case 4: /* Rogue = 4 */  {
                    addvalue = uint32(((Spirit*0.50) * HealthIncreaseRate));
                    break;              }
               case 5: /* Priest = 5 */ {
                    addvalue = uint32(((Spirit*0.10) * HealthIncreaseRate));
                    break;              }
               case 7: /* Shaman = 7 */ {
                    addvalue = uint32((((Spirit*0.11)+9) * HealthIncreaseRate));
                    break;              }
               case 8: /* Mage = 8 */   {
                   curValue+=uint32(((Spirit*0.10) * HealthIncreaseRate));
                    break;              }
               case 9: /* Warlock = 9 */{
                   addvalue = uint32(((Spirit*0.11) * HealthIncreaseRate));
                   break;               }
               case 11: /* Druid = 11 */{
                    /* TODO: change this one, cause hp regen
					   formula for druid was UNKNOWN */
                    addvalue = uint32(((Spirit+10) * HealthIncreaseRate));
                    break;              }
               default: /* Poor Creatures got left out */{
                   addvalue = uint32(((Spirit+10) * HealthIncreaseRate));
                      break;            }
            }
            break; }
        /* mod by hann */
        case UNIT_FIELD_POWER1: /* mana */{
            switch (Class)
            {
                case 2: /* Paladin = 2 */               {
                    addvalue = uint32((((Spirit/4)+8) * ManaIncreaseRate));
                    break;              }
                case 3: /* Hunter = 3 */                {
                    addvalue = uint32((((Spirit/4)+11) * ManaIncreaseRate));
                    break;              }
                case 5: /* Priest = 5 */                {
                    addvalue = uint32((((Spirit/4)+13) * ManaIncreaseRate));
                    break;              }
                case 7: /* Shaman = 7 */                {
                    addvalue = uint32((((Spirit/5)+17) * ManaIncreaseRate));
                    break;              }
                case 8: /* Mage = 8 */              {
                    addvalue = uint32((((Spirit/4)+11) * ManaIncreaseRate));
                    break;              }
                case 9: /* Warlock = 9 */               {
                    addvalue = uint32((((Spirit/4)+8) * ManaIncreaseRate));
                    break;              }
                case 11: /* Druid = 11 */               {
                    addvalue = uint32((((Spirit/5)+15) * ManaIncreaseRate));
                    break;              }
                default: /* Poor Creatures got left out */              {
                    addvalue = uint32((Spirit * ManaIncreaseRate));
                    break;                  }
           }
           break;}
        case UNIT_FIELD_POWER2: /* rage */{
            /* formula for rage required */
            addvalue = uint32((1 * RageIncreaseRate));
            break;}

        case UNIT_FIELD_POWER4: /* energy */{
            addvalue = uint32(20);
            break;}

        default:{
            break;}
    }
    if(addvalue == 0) addvalue = 1; /* min rate of 1 */
    if (switch_)
    {
        /* Log::getSingleton( ).outError("StandState: %i\n", getStandState()); */
        if((getStandState() == 1) || (getStandState() == 3))
        {
            /* we are sitting */
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

//================================================================================================
//  AttackerStateUpdate
//  This function determines whether there is a hit, and the resultant damage
//================================================================================================
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
    data << uint32(1);                            // Target Count ?

    data << uint32(3);                            // unknown if its somethine else then 3, then nothing shows up in CombatLog
    data << damage;
    data << damageType;
    data << uint32(0);
    SendMessageToSet(&data,true);

    DealDamage(pVictim, damage, procFlag);
}


void Unit::AttackerStateUpdate(Unit *pVictim,uint32 damage)
{
    uint32 procFlag = 0;
    if (pVictim->GetUInt32Value(UNIT_FIELD_HEALTH) == 0 ||
        GetUInt32Value(UNIT_FIELD_HEALTH) == 0 )
        return;

    WorldPacket data;
    uint32 hit_status = 0xe2;
    uint32 damageType = 0;

    if(damage == 0)
        damage = CalculateDamage(this);
    else
        damageType = 1;

    uint32 some_value = 0xffffffff;
    some_value = 0x0;

    data.Initialize(SMSG_ATTACKERSTATEUPDATE);
    data << (uint32)hit_status;                   // Attack flags
    data << GetGUID();
    data << pVictim->GetGUID();
    data << (uint32)damage;
    data << (uint8)1;                             // Damage type counter

    // for each...
    data << damageType;                           // Damage type, // 0 - white font, 1 - yellow
    data << (uint32)0;                            // damage float
    data << (uint32)damage;                       // Damage amount
    data << (uint32)0;                            // damage absorbed

    data << (uint32)1;                            // new victim state
    data << (uint32)0;                            // victim round duraction
    data << (uint32)0;                            // additional spell damage amount
    data << (uint32)0;                            // additional spell damage id
    data << (uint32)0;                            // Damage amount blocked

    WPAssert(data.size() == 61);
    SendMessageToSet(&data, true);

    Log::getSingleton( ).outDetail("AttackerStateUpdate: %u %X attacked %u %X for %u dmg.",
        GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage);

    DealDamage(pVictim, damage, procFlag);
}


void Unit::smsg_AttackStop(uint64 victimGuid)
{
    WorldPacket data;
    data.Initialize( SMSG_ATTACKSTOP );
    data << GetGUID();
    data << victimGuid;
    data << uint32( 0 );
    SendMessageToSet(&data, true);
    Log::getSingleton( ).outDetail("%u %X stopped attacking "I64FMT,
        GetGUIDLow(), GetGUIDHigh(), victimGuid);
}


void Unit::smsg_AttackStart(Unit* pVictim)
{
#ifndef ENABLE_GRID_SYSTEM
    Player* pThis = objmgr.GetObject<Player>(GetGUID());
#else
    Player *pThis = ObjectAccessor::Instance().FindPlayer(GetGUID());
#endif
    smsg_AttackStart(pVictim, pThis);
}

void Unit::smsg_AttackStart(Unit* pVictim, Player *pThis)
{
    WorldPacket data;

    // Prevent user from ignoring attack speed and stopping and start combat really really fast
    if(!isAttackReady())
        setAttackTimer(uint32(0));
    else if(!canReachWithAttack(pVictim))
    {
        setAttackTimer(uint32(500));
        data.Initialize(SMSG_CAST_RESULT);
        data << uint32(0);
        data << uint8(2);
        data << uint8(0x53);                      // Target out of Range
        if(pThis)
            pThis->GetSession()->SendPacket(&data);
    }
    else if(!isInFront(pVictim,10.00000f))
    {
        setAttackTimer(uint32(500));
        data.Initialize(SMSG_CAST_RESULT);
        data << uint32(0);
        data << uint8(2);
        data << uint8(0x76);                      // Target not in Front
        if(pThis)
            pThis->GetSession()->SendPacket(&data);
    }

    // Send out ATTACKSTART
    data.Initialize( SMSG_ATTACKSTART );
    data << GetGUID();
    data << pVictim->GetGUID();
    SendMessageToSet(&data, true);
    Log::getSingleton( ).outDebug( "WORLD: Sent SMSG_ATTACKSTART" );

    // FLAGS changed so other players see attack animation
    // addUnitFlag(0x00080000);
    // setUpdateMaskBit(UNIT_FIELD_FLAGS );
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
        if(i != 0)
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
    /* Changed Just testing: Please Feel Free to change back to the other way but comment it this time :D */
    Log::getSingleton( ).outError("RemoveAllAffects");
    AffectList::iterator i;
    Affect::ModList::const_iterator j;
    // Affect *aff;

    for (i = m_affects.begin(); i != m_affects.end(); i++)
    {
        Log::getSingleton( ).outError("First Loop");
        // Affect *aff = *i;
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
    // for (i = m_affects.begin(); i != m_affects.end(); i++)
    // {
    //     m_affects.erase(i);
    // }
    m_affects.clear();
    // delete aff;
    return;
}


/*
void Unit::RemoveAllAffects()
{
    // FIXME SOME THING WRONG HERE
    AffectList::iterator i, next;
    Affect *aff;
    bool result = false;
    Log::getSingleton( ).outError("RemoveAllAffects: Start");
    uint8 t = 0;
    uint8 s = 0;
    for (i = m_affects.begin(); i != m_affects.end(); i = next)
    {
        t+=1;
        Log::getSingleton( ).outError("RemoveAllAffects: MainLoop");
        next = i;
        next++;

        aff = *i;
        s = 0;
        for (Affect::ModList::const_iterator j = aff->GetModList().begin();
        j != aff->GetModList().end(); i++)
        {
            s +=1;
            Log::getSingleton( ).outError("RemoveAllAffects: Secondaryloop");
            ApplyModifier(&(*j), false, aff);
            if(s == 20)
            {
                Log::getSingleton( ).outError("RemoveAllAffects: Second Loop Reached Limit");
                break;
            }
        }
        Log::getSingleton( ).outError("RemoveAllAffects: RemoveAura");
        _RemoveAura(aff);
        Log::getSingleton( ).outError("RemoveAllAffects: Erase Affect");
        m_affects.erase(i);

        delete aff;
        if(t == 80)
        {
            Log::getSingleton( ).outError("RemoveAllAffects: First Loop Reached Limit");
            break;
        }
    }

    return;
}
*/

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
					// SHOULD BE ARCANE-- SHOULDN'T ?
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
					// -1 strenght ?
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
            if(powerType == 0)                    // Mana
                powerField = UNIT_FIELD_POWER1;
            else if(powerType == 1)               // Rage
                powerField = UNIT_FIELD_POWER2;
            else if(powerType == 3)               // Energy
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
            // check for spell id
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
                    // Periodic Trigger Damage
                    if(spellInfo->EffectApplyAuraName[i] == 3)
                    {
                        damage = spellInfo->EffectBasePoints[i]+rand()%spellInfo->EffectDieSides[i]+1;
                        tmpAff->SetDamagePerTick((uint16)damage, spellInfo->EffectAmplitude[i]);
                        tmpAff->SetNegative();
                    // Periodic Trigger Spell
                    }
                    else if(spellInfo->EffectApplyAuraName[i] == 23)
                        tmpAff->SetPeriodicTriggerSpell(spellInfo->EffectTriggerSpell[i],spellInfo->EffectAmplitude[i]);
                    // Periodic Heal
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
                parent->GetSpellProto()->procCharges == 0 ? pts->procCharges = -1
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
            // apply ? SetUInt32Value(UNIT_FIELD_ATTACKPOWER,GetUInt32Value(UNIT_FIELD_ATTACKPOWER) + mod->GetAmount()) : SetUInt32Value(UNIT_FIELD_ATTACKPOWER,GetUInt32Value(UNIT_FIELD_ATTACKPOWER) - mod->GetAmount());
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

#ifndef ENABLE_GRID_SYSTEM
    Player* pThis = objmgr.GetObject<Player>(GetGUID());
#else
    Player* pThis = ObjectAccessor::Instance().FindPlayer(GetGUID());
#endif

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
#ifndef ENABLE_GRID_SYSTEM
            pGroupGuy = objmgr.GetObject<Player>(pGroup->GetMemberGUID(i));
#else
	    pGroupGuy = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(i));
#endif
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
                    // pGroupGuy->SetAffDuration(m_aura->GetId(),this,0);
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
#ifndef ENABLE_GRID_SYSTEM
            Unit *attacker = (Unit*) objmgr.GetObject<Player>(aff->GetCasterGUID());
            if(!attacker)
                attacker = (Unit*) objmgr.GetObject<Creature>(aff->GetCasterGUID());
#else
	    Unit *attacker = ObjectAccessor::Instance().FindPlayer(aff->GetCasterGUID());
#endif

            // FIXME: we currently have a way to inflict damage w/o attacker, this should be changed
            if(attacker)
            {
                if(this->isAlive())
                    attacker->PeriodicAuraLog(this, aff->GetId(), aff->GetDamagePerTick(),aff->GetSpellProto()->School);
            }
        }
        if( AffResult == 4 || AffResult == 6 || AffResult == 12 || AffResult == 14)
        {
            // Trigger Spell
            // check for spell id
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

            spell->prepare(&targets);
        }
        if( AffResult == 8 || AffResult == 10 || AffResult == 12 || AffResult == 14)
        {
#ifndef ENABLE_GRID_SYSTEM
            Unit *attacker = (Unit*) objmgr.GetObject<Player>(aff->GetCasterGUID());
            if(!attacker)
                attacker = (Unit*) objmgr.GetObject<Creature>(aff->GetCasterGUID());
#else
	    Unit *attacker = ObjectAccessor::Instance().FindPlayer(aff->GetCasterGUID());
#endif

            // FIXME: we currently have a way to inflict damage w/o attacker, this should be changed
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
    // check if we have a spell already casting etc
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

    //UNIT_FIELD_AURAFLAGS 0-7;UNIT_FIELD_AURAFLAGS+1 8-15;UNIT_FIELD_AURAFLAGS+2 16-23 ... For each Aura 1 Byte

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

//  0000 0000 original
//  0000 1001 AFLAG_SET
//  1111 1111 0xFF

    uint8 appslot = slot >> 1;

/*
    value = GetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS + appslot);
    uint16 *dmg = ((uint16*)&value) + (slot & 1);
    *dmg = 5; // FIXME: use correct value
    SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS + appslot, value);
*/

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

    // UNIT_FIELD_AURAFLAGS 0-7;UNIT_FIELD_AURAFLAGS+1 8-15;UNIT_FIELD_AURAFLAGS+2 16-23 ... For each Aura 1 Byte

    uint8 slot = aff->GetAuraSlot();

    SetUInt32Value((uint16)(UNIT_FIELD_AURA + slot), 0);

    uint8 flagslot = slot >> 3;

    uint32 value = GetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot));
    value &= 0xFFFFFFFF ^ (0xF << ((slot & 7) << 2));
    SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + flagslot), value);

/*
    uint8 appslot = slot >> 1;
    value = GetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS + appslot);
    uint16 *dmg = ((uint16*)&value) + (slot & 1);
    *dmg = 0;
    SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS + appslot, value);
*/

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


/*
float Unit::calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y )
{
    float radians = atan2(Position2Y - Position1Y, Position2X - Position1X);
    Log::getSingleton().outError("Radians: %f",radians);
    float angle = (radians *(180 / M_PI));
    Log::getSingleton().outError("Angle: %f",angle);
    float angle1 = geteasyangle(angle);
    float angle2 = 360 - angle;
    float angle3 = 360 + angle;
    Log::getSingleton().outError("Angle1: %f",angle1);
    Log::getSingleton().outError("Angle2: %f",angle2);
    Log::getSingleton().outError("Angle3: %f",angle3);
    return angle1;
}
*/


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
    /* float FOV = 90; */
    /* float orientation = GetOrientation()/float(2*M_PI)*360; */
    /* float orientation = geteasyangle((GetOrientation() - (FOV))); */
    return inarc(distance,GetPositionX(),GetPositionY(),float(180),GetOrientation(),target->GetPositionX(),target->GetPositionY());
}


// not the best way to do it, though
bool Unit::setInFront(Unit* target, float distance)
{
    // very verry high cpu usage here actually infinite loop some times
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
#ifndef ENABLE_GRID_SYSTEM
            SetPosition(GetPositionX(), GetPositionY(), GetPositionZ(), orientation);
#else
	    m_orientation = orientation;
#endif
            break;
        }else
        orientation += 90;
    }
    Log::getSingleton().outError("Orentation after loop: %f",orientation);
    return inarc(distance,GetPositionX(),GetPositionY(),float(180),orientation,target->GetPositionX(),target->GetPositionY());
}


void Unit::DeMorph()
{
    // hope it solves it :)
    uint32 displayid = this->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    this->SetUInt32Value(UNIT_FIELD_DISPLAYID, displayid);
}


/*
float Unit::CalcDistance(Unit *Ua, Unit *Ub)
{
    return CalcDistance(Ua->getPositionX(), Ua->getPositionY(), Ua->getPositionZ(), Ub->getPositionX(), Ub->getPositionY(), Ub->getPositionZ());
}


float Unit::CalcDistance(Unit *Ua, float PaX, float PaY, float PaZ)
{
    return CalcDistance(Ua->getPositionX(), Ua->getPositionY(), Ua->getPositionZ(), PaX, PaY, PaZ);
}


float Unit::CalcDistance(float PaX, float PaY, float PaZ, float PbX, float PbY, float PbZ)
{
    float xdest = PaX - PbX;
    float ydest = PaY - PbY;
    float zdest = PaZ - PbZ;
    return sqrt(zdest*zdest + ydest*ydest + xdest*xdest);
}
*/

#ifdef ENABLE_GRID_SYSTEM
void
Unit::DealWithSpellDamage(DynamicObject &obj)
{
    obj.DealWithSpellDamage(*this);
}

#endif
