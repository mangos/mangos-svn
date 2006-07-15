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
#include "Pet.h"
#include "Util.h"

#include <math.h>

Unit::Unit() : Object()
{
    m_objectType |= TYPE_UNIT;
    m_objectTypeId = TYPEID_UNIT;

    m_attackTimer = 0;

    m_state = 0;
    m_form = 0;
    m_deathState = ALIVE;
    m_currentSpell = NULL;
    m_meleeSpell = false;
    m_addDmgOnce = 0;
    m_TotemSlot1 = m_TotemSlot2 = m_TotemSlot3 = m_TotemSlot4  = 0;
    //m_Aura = NULL;
    //m_AurasCheck = 2000;
    //m_removeAuraTimer = 4;
    //tmpAura = NULL;
    m_silenced = false;
    waterbreath = false;

    m_immuneToMechanic = 0;
    m_immuneToEffect = 0;
    m_immuneToState  = 0;
    m_immuneToSchool = 0;
    m_immuneToDmg    = 0;
    m_immuneToDispel = 0;
    m_detectStealth = 0;
    m_stealthvalue = 0;
    m_transform = 0;
    m_ShapeShiftForm = 0;

    for (int i = 0; i < TOTAL_AURAS; i++)
        m_AuraModifiers[i] = -1;

    m_attacking = NULL;
    m_modDamagePCT = 0;
    m_RegenPCT = 0;
    m_modHitChance = 0;
    m_modSpellHitChance = 0;
    m_baseSpellCritChance = 5;
    m_spellCritSchool.clear();
    m_reflectSpellSchool.clear();
    m_scAuras.clear();
    m_damageDoneCreature.clear();
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
    _UpdateHostil( p_time );

    if(m_attackTimer > 0)
    {
        if(p_time >= m_attackTimer)
            m_attackTimer = 0;
        else
            m_attackTimer -= p_time;
    }
}

void Unit::SendMoveToPacket(float x, float y, float z, bool run)
{
    float dx = x - GetPositionX();
    float dy = y - GetPositionY();
    float dz = z - GetPositionZ();
    float dist = ((dx*dx) + (dy*dy) + (dz*dz));
    if(dist<0)
        dist = 0;
    else
        dist = ::sqrt(dist);
    double speed = GetSpeed(run ? MOVE_RUN : MOVE_WALK);
    if(speed<=0)
        speed = 2.5f;
    speed *= 0.001f;
    uint32 time = static_cast<uint32>(dist / speed + 0.5);
    //float orientation = (float)atan2((double)dy, (double)dx);
    SendMonsterMove(x,y,z,false,run,time);

}

void Unit::SendMonsterMove(float NewPosX, float NewPosY, float NewPosZ, bool Walkback, bool Run, uint32 Time)
{
    WorldPacket data;
    data.Initialize( SMSG_MONSTER_MOVE );
    data << uint8(0xFF) << GetGUID();
                                                            // Point A, starting location
    data << GetPositionX() << GetPositionY() << GetPositionZ();
                                                            // little trick related to orientation
    data << (uint32)((*((uint32*)&GetOrientation())) & 0x30000000);
    data << uint8(Walkback);                                // walkback when walking from A to B
    data << uint32(Run ? 0x00000100 : 0x00000000);          // flags
    /* Flags:
    512: Floating, moving without walking/running
    */
    data << Time;                                           // Time in between points
    data << uint32(1);                                      // 1 single waypoint
    data << NewPosX << NewPosY << NewPosZ;                  // the single waypoint Point B
    WPAssert( data.size() == 50 );
    SendMessageToSet( &data, true );
}

void Unit::setAttackTimer(uint32 time, bool rangeattack)
{
    if(time)
        m_attackTimer = time;
    else
    {
        if(rangeattack)
        {
            if (GetTypeId() == TYPEID_PLAYER)
            {
                m_attackTimer = GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME);
            }
        }
        else
        {
            if (GetTypeId() == TYPEID_PLAYER)
            {
                m_attackTimer = GetUInt32Value(UNIT_FIELD_BASEATTACKTIME);
            }
        }
        m_attackTimer = (m_attackTimer >= 200) ? m_attackTimer : 2000;
    }
}

bool Unit::canReachWithAttack(Unit *pVictim) const
{
    assert(pVictim);
    float reach = GetFloatValue(UNIT_FIELD_COMBATREACH);
    if( reach <= 0.0f )
        reach = 1.0f;
    float distance = GetDistanceSq(pVictim);

    return ( distance <= reach * reach );
}

void Unit::RemoveSpellsCausingAura(uint32 auraType)
{
    AuraMap::iterator iter, next;
    for (iter = m_Auras.begin(); iter != m_Auras.end(); iter = next)
    {
        next = iter;
        ++next;

        if ((*iter).second)
        {
            if (((*iter).second)->GetModifier()->m_auraname == auraType)
            {
                uint32 spellId = ((*iter).second)->GetId();
                RemoveAurasDueToSpell(spellId);
                if (!m_Auras.empty())
                    next = m_Auras.begin();
                else
                    return;
            }
        }
    }
}

bool Unit::HasAuraType(uint32 auraType) const
{
    return (m_AuraModifiers[auraType] != -1);
}

void Unit::DealDamage(Unit *pVictim, uint32 damage, uint32 procFlag, bool durabilityLoss)
{
    if (!pVictim->isAlive()) return;

    if(isStealth())
        RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
   if(pVictim->hasUnitState(UNIT_STAT_ROOT))
      pVictim->clearUnitState(UNIT_STAT_ROOT);

    if(pVictim->GetTypeId() != TYPEID_PLAYER)
    {
        //pVictim->SetInFront(this);
        // no loot,xp,health if type 8 /critters/
        if ( ((Creature*)pVictim)->GetCreatureInfo()->type == 8)
        {
            pVictim->setDeathState(JUST_DIED);
            ((Creature*)pVictim)->SetUInt32Value( UNIT_FIELD_HEALTH, 0);
            pVictim->RemoveFlag(UNIT_FIELD_FLAGS, 0x00080000);
            return;
        }
        ((Creature*)pVictim)->AI().AttackStart(this);
    }

    DEBUG_LOG("DealDamageStart");

    uint32 health = pVictim->GetUInt32Value(UNIT_FIELD_HEALTH );
    sLog.outDetail("deal dmg:%d to heals:%d ",damage,health);
    if (health <= damage)
    {
        DEBUG_LOG("DealDamage: victim just died");

        DEBUG_LOG("SET JUST_DIED");
        pVictim->setDeathState(JUST_DIED);

        uint64 attackerGuid, victimGuid;
        attackerGuid = GetGUID();
        victimGuid = pVictim->GetGUID();

        DEBUG_LOG("DealDamageAttackStop");
        pVictim->SendAttackStop(attackerGuid);

        DEBUG_LOG("DealDamageHealth1");
        pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
        pVictim->RemoveFlag(UNIT_FIELD_FLAGS, 0x00080000);

        // 10% durability loss on death
        // clean hostilList
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
        {
            DEBUG_LOG("We are dead, loosing 10 percents durability");
            if (durabilityLoss)
            {
                ((Player*)pVictim)->DeathDurabilityLoss(0.10);
            }
            HostilList::iterator i;
            for(i = m_hostilList.begin(); i != m_hostilList.end(); i++)
            {
                if(i->UnitGuid==victimGuid)
                {
                    m_hostilList.erase(i);
                    break;
                }
            }

            Creature *pet = pVictim->GetPet();
            if(pet && pet->isPet())
            {
                pet->setDeathState(JUST_DIED);
                pet->SendAttackStop(attackerGuid);
                pet->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
                pet->RemoveFlag(UNIT_FIELD_FLAGS, 0x00080000);
                pet->addUnitState(UNIT_STAT_DIED);
                for(i = m_hostilList.begin(); i != m_hostilList.end(); ++i)
                {
                    if(i->UnitGuid==pet->GetGUID())
                    {
                        m_hostilList.erase(i);
                        break;
                    }
                }
            }
        }
        else
        {
            pVictim->m_hostilList.clear();
            DEBUG_LOG("DealDamageNotPlayer");
            pVictim->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 1);
        }

        //judge if GainXP, Pet kill like player kill,kill pet not like PvP
        bool playerkill = false;
        bool PvP = false;
        Player *player;

        if(GetTypeId() == TYPEID_PLAYER)
        {
            playerkill = true;
            player = (Player*)this;
            if(pVictim->GetTypeId() == TYPEID_PLAYER)
                PvP = true;
        }
        else if(((Creature*)this)->isPet())
        {
            Unit* owner = ((Pet*)this)->GetOwner();
            if(!owner)
                playerkill = false;
            else if(owner->GetTypeId() == TYPEID_PLAYER)
            {
                player = (Player*)owner;
                playerkill = true;
            }
        }

        if(playerkill)
        {
            player->CalculateHonor(pVictim);
            player->CalculateReputation(pVictim);

            if(!PvP)
            {
                DEBUG_LOG("DealDamageIsPvE");
                uint32 xp = MaNGOS::XP::Gain(player, pVictim);
                uint32 entry = 0;
                entry = pVictim->GetUInt32Value(OBJECT_FIELD_ENTRY );

                Group *pGroup = objmgr.GetGroupByLeader(player->GetGroupLeader());
                if(pGroup)
                {
                    DEBUG_LOG("Kill Enemy In Group");
                    xp /= pGroup->GetMembersCount();
                    for (uint32 i = 0; i < pGroup->GetMembersCount(); i++)
                    {
                        Player *pGroupGuy = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(i));
                        if(!pGroupGuy)
                            continue;
                        if(GetDistanceSq(pGroupGuy) > sWorld.getConfig(CONFIG_GETXP_DISTANCE))
                            continue;
                        if(uint32(abs((int)pGroupGuy->getLevel() - (int)pVictim->getLevel())) > sWorld.getConfig(CONFIG_GETXP_LEVELDIFF))
                            continue;
                        pGroupGuy->GiveXP(xp, pVictim);
                        pGroupGuy->KilledMonster(entry, victimGuid);
                    }
                }
                else
                {
                    DEBUG_LOG("Player kill enemy alone");
                    player->GiveXP(xp, pVictim);
                    player->KilledMonster(entry,victimGuid);
                }
            }
        }
        else
        {
            DEBUG_LOG("Monster kill Monster");
            SendAttackStop(victimGuid);
            addUnitState(UNIT_STAT_DIED);
        }
        AttackStop();
    }
    else
    {
        DEBUG_LOG("DealDamageAlive");
        pVictim->SetUInt32Value(UNIT_FIELD_HEALTH , health - damage);
        Attack(pVictim);

        if(pVictim->getTransForm())
        {
            pVictim->RemoveAurasDueToSpell(pVictim->getTransForm());
            pVictim->setTransForm(0);
        }

        if (pVictim->GetTypeId() != TYPEID_PLAYER)
        {
            ((Creature *)pVictim)->AI().DamageInflict(this, damage);
            pVictim->AddHostil(GetGUID(), damage);
            if( GetTypeId() == TYPEID_PLAYER
                && (getClass() == WARRIOR || m_form == 5 || m_form == 8) )
                ((Player*)this)->CalcRage(damage,true);
        }
        else
        {
            if( pVictim->getClass() == WARRIOR )
                ((Player*)pVictim)->CalcRage(damage,false);

            // random durability for items (HIT)
            int randdurability = urand(0, 300);
            if (randdurability == 10)
            {
                DEBUG_LOG("HIT: We decrease durability with 5 percent");
                ((Player*)pVictim)->DeathDurabilityLoss(0.05);
            }
        }
    }

    DEBUG_LOG("DealDamageEnd");
}

void Unit::CastSpell(Unit* Victim, uint32 spellId, bool triggered)
{

    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId );

    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    Spell *spell = new Spell(this, spellInfo, triggered, 0);
    WPAssert(spell);

    SpellCastTargets targets;
    targets.setUnitTarget( Victim );
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
    uint32 resist=0;
    int32 critchance = m_baseSpellCritChance;
    int crit = 0;
    CreatureInfo *cinfo = NULL;
    if(pVictim->GetTypeId() != TYPEID_PLAYER)
        cinfo = ((Creature*)pVictim)->GetCreatureInfo();

    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellID);
    if(spellInfo)
    {
        for(std::list<struct SpellCritSchool*>::iterator i = m_spellCritSchool.begin();i != m_spellCritSchool.end();i++)
        {
            if((*i)->school == -1 || (*i)->school == spellInfo->School)
            {
                critchance += (*i)->chance;
            }
        }
        if(critchance + (GetUInt32Value(UNIT_FIELD_IQ) - 100)/100 >= urand(0,100))
        {
            damage = uint32(damage*1.5);
            crit = 1;
        }
        for(std::list<struct DamageDoneCreature*>::iterator i = m_damageDoneCreature.begin();i != m_damageDoneCreature.end();i++)
        {
            if(cinfo && cinfo->type == (*i)->creaturetype)
            {
                damage += (*i)->damage;
                break;
            }
        }
        absorb = CalDamageAbsorb(pVictim,spellInfo->School,damage,&resist);
    }

    WorldPacket data;
    if(m_modSpellHitChance+100 < urand(0,100))
    {
        SendAttackStateUpdate(HITINFO_HITSTRANGESOUND1|HITINFO_MISS, pVictim->GetGUID(), 1, spellInfo->School, 0, 0,0,1,0);
        return;
    }

    if( (damage-absorb-resist)<= 0 )
    {
        SendAttackStateUpdate(HITINFO_HITSTRANGESOUND1|HITINFO_NOACTION, pVictim->GetGUID(), 1, spellInfo->School, damage, absorb,resist,1,0);
        return;
    }

    sLog.outDetail("SpellNonMeleeDamageLog: %u %X attacked %u %X for %u dmg inflicted by %u,abs is %u,resist is %u crit is %i.",
        GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage, spellID, absorb, resist,crit);

    SendSpellNonMeleeDamageLog(pVictim->GetGUID(), spellID, damage, spellInfo->School, absorb, resist, false, 0);
    DealDamage(pVictim, damage<(absorb+resist)?0:(damage-absorb-resist), 0, true);
}

void Unit::PeriodicAuraLog(Unit *pVictim, SpellEntry *spellProto, Modifier *mod)
{
    uint32 procFlag = 0;
    if(!this || !pVictim || !isAlive() || !pVictim->isAlive())
    {
        return;
    }
    uint32 absorb=0;
    uint32 resist=0;
    int32 critchance = m_baseSpellCritChance;
    int crit = 0;
    uint32 pdamage = mod->m_amount;
    CreatureInfo *cinfo = NULL;
    if(pVictim->GetTypeId() != TYPEID_PLAYER)
        cinfo = ((Creature*)pVictim)->GetCreatureInfo();

    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellProto->Id);
    if(spellInfo)
    {
        for(std::list<struct SpellCritSchool*>::iterator i = m_spellCritSchool.begin();i != m_spellCritSchool.end();i++)
        {
            if((*i)->school == -2 || (*i)->school == spellInfo->School)
            {
                critchance += (*i)->chance;
            }
        }
        if(critchance + (GetUInt32Value(UNIT_FIELD_IQ) - 100)/100 >= urand(0,100))
        {
            pdamage = uint32(pdamage*1.5);
            crit = 1;
        }
        for(std::list<struct DamageDoneCreature*>::iterator i = m_damageDoneCreature.begin();i != m_damageDoneCreature.end();i++)
        {
            if(cinfo && cinfo->type == (*i)->creaturetype)
            {
                pdamage += (*i)->damage;
                break;
            }
        }
        absorb = CalDamageAbsorb(pVictim,spellInfo->School,pdamage,&resist);
    }

    sLog.outDetail("PeriodicAuraLog: %u %X attacked %u %X for %u dmg inflicted by %u abs is %u crit is %u",
        GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), pdamage, spellProto->Id,absorb,crit);

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

    if(mod->m_auraname == SPELL_AURA_PERIODIC_DAMAGE)
    {
        SendSpellNonMeleeDamageLog(pVictim->GetGUID(), spellProto->Id, mod->m_amount, spellProto->School, absorb, resist, false, 0);
        SendMessageToSet(&data,true);

        DealDamage(pVictim, mod->m_amount <= int32(absorb+resist) ? 0 : (mod->m_amount-absorb-resist), procFlag, true);
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_DAMAGE_PERCENT)
    {
        int32 pdamage = GetUInt32Value(UNIT_FIELD_HEALTH)*(100+mod->m_amount)/100;
        SendSpellNonMeleeDamageLog(pVictim->GetGUID(), spellProto->Id, pdamage, spellProto->School, absorb, resist, false, 0);
        SendMessageToSet(&data,true);
        DealDamage(pVictim, pdamage <= int32(absorb+resist) ? 0 : (pdamage-absorb-resist), procFlag, true);
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_HEAL)
    {
        if(GetUInt32Value(UNIT_FIELD_HEALTH) + mod->m_amount*(100+m_RegenPCT)/100 < GetUInt32Value(UNIT_FIELD_MAXHEALTH) )
            SetUInt32Value(UNIT_FIELD_HEALTH,GetUInt32Value(UNIT_FIELD_HEALTH) + mod->m_amount*(100+m_RegenPCT)/100);
        else
            SetUInt32Value(UNIT_FIELD_HEALTH,GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_LEECH)
    {
        uint32 tmpvalue = 0;
        for(int x=0;x<3;x++)
        {
            if(mod->m_auraname != spellInfo->EffectApplyAuraName[x])
                continue;
            if(pVictim->GetUInt32Value(UNIT_FIELD_HEALTH) - mod->m_amount > 0)
                tmpvalue = uint32(mod->m_amount*spellInfo->EffectMultipleValue[x]);
            else
                tmpvalue = uint32(pVictim->GetUInt32Value(UNIT_FIELD_HEALTH)*spellInfo->EffectMultipleValue[x]);

            DealDamage(pVictim, mod->m_amount <= int32(absorb+resist) ? 0 : (mod->m_amount-absorb-resist), procFlag, false);
            if (!pVictim->isAlive() && m_currentSpell)
                if (m_currentSpell->m_spellInfo)
                    if (m_currentSpell->m_spellInfo->Id == spellProto->Id)
                        m_currentSpell->cancel();

            break;
        }
        if(GetUInt32Value(UNIT_FIELD_HEALTH) + tmpvalue*(100+m_RegenPCT)/100 < GetUInt32Value(UNIT_FIELD_MAXHEALTH) )
            SetUInt32Value(UNIT_FIELD_HEALTH,GetUInt32Value(UNIT_FIELD_HEALTH) + tmpvalue*(100+m_RegenPCT)/100);
        else SetUInt32Value(UNIT_FIELD_HEALTH,GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_MANA_LEECH)
    {
        uint32 tmpvalue = 0;
        for(int x=0;x<3;x++)
        {
            if(mod->m_auraname != spellInfo->EffectApplyAuraName[x])
                continue;
            if(pVictim->GetUInt32Value(UNIT_FIELD_POWER1) - mod->m_amount > 0)
            {
                pVictim->SetUInt32Value(UNIT_FIELD_POWER1,uint32(pVictim->GetUInt32Value(UNIT_FIELD_POWER1) - mod->m_amount));
                tmpvalue = uint32(mod->m_amount*spellInfo->EffectMultipleValue[x]);
            }
            else
            {
                tmpvalue = uint32(pVictim->GetUInt32Value(UNIT_FIELD_POWER1)*spellInfo->EffectMultipleValue[x]);
                pVictim->SetUInt32Value(UNIT_FIELD_POWER1,0);
            }
            break;
        }
        if(GetUInt32Value(UNIT_FIELD_POWER1) + tmpvalue < GetUInt32Value(UNIT_FIELD_MAXPOWER1) )
            SetUInt32Value(UNIT_FIELD_POWER1,GetUInt32Value(UNIT_FIELD_POWER1) + tmpvalue);
        else SetUInt32Value(UNIT_FIELD_POWER1,GetUInt32Value(UNIT_FIELD_MAXPOWER1));
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_ENERGIZE)
    {
        uint16 field = 0;
        switch(mod->m_miscvalue)
        {
            case 0:field = UNIT_FIELD_POWER1;break;
            case 1:field = UNIT_FIELD_POWER2;break;
            case 2:field = UNIT_FIELD_POWER3;break;
            case 3:field = UNIT_FIELD_POWER4;break;
            case 4:field = UNIT_FIELD_POWER5;break;
            default:break;
        }
        if(!field)
            return;
        SetUInt32Value(field,GetUInt32Value(field)+mod->m_amount);
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

uint32 Unit::CalDamageAbsorb(Unit *pVictim,uint32 School,const uint32 damage,uint32 *resist)
{
    uint32 AbsorbDamage=0;
    uint32 currAbsorbDamage=0;
    uint32 currentPower;
    bool  removeAura=false;

    if(!pVictim)
        return 0;
    if(!pVictim->isAlive())
        return 0;

    for(std::list<struct DamageManaShield*>::iterator i = pVictim->m_damageManaShield.begin();i != pVictim->m_damageManaShield.end();i++)
    {
        SpellEntry *spellInfo = sSpellStore.LookupEntry( (*i)->m_spellId);

        if(((*i)->m_schoolType & School) || (*i)->m_schoolType == School || (*i)->m_schoolType ==127)
        {
            currAbsorbDamage = damage+ (*i)->m_currAbsorb;
            if(currAbsorbDamage < (*i)->m_totalAbsorb)
            {
                AbsorbDamage = damage;
                (*i)->m_currAbsorb = currAbsorbDamage;
            }
            else
            {
                AbsorbDamage = (*i)->m_totalAbsorb - (*i)->m_currAbsorb;
                (*i)->m_currAbsorb = (*i)->m_totalAbsorb;
                removeAura = true;
            }

            if((*i)->m_modType == SPELL_AURA_MANA_SHIELD)
            {
                float multiple;
                for(int x=0;x<3;x++)
                    if(spellInfo->EffectApplyAuraName[x] == SPELL_AURA_MANA_SHIELD)
                {
                    multiple = spellInfo->EffectMultipleValue[x];
                    break;
                }
                currentPower = pVictim->GetUInt32Value(UNIT_FIELD_POWER1);
                if ( (float)(currentPower) > AbsorbDamage*multiple )
                {
                    pVictim->SetUInt32Value(UNIT_FIELD_POWER1, (uint32)(currentPower-AbsorbDamage*multiple) );
                }
                else
                {
                    pVictim->SetUInt32Value(UNIT_FIELD_POWER1, 0 );
                }
            }

            if(removeAura)
                pVictim->RemoveAurasDueToSpell((*i)->m_spellId);
        }
        break;
    }
    if(School == 0)
    {
        uint32 armor = pVictim->GetUInt32Value(UNIT_FIELD_ARMOR);
        float tmpvalue = armor/(pVictim->getLevel()*85.0 +400.0 +armor);
        if(tmpvalue < 0)
            tmpvalue = 0.0;
        if(tmpvalue > 1.0)
            tmpvalue = 1.0;
        AbsorbDamage += uint32(damage * tmpvalue);
        if(AbsorbDamage > damage)
            AbsorbDamage = damage;
    }
    if( School > 0)
    {
        uint32 tmpvalue2 = pVictim->GetUInt32Value(UNIT_FIELD_ARMOR + School);
        *resist += uint32(damage*tmpvalue2*0.0025*pVictim->getLevel()/getLevel());
        if(*resist > damage)
            *resist = damage;
    }

    // random durability loss for items on absorb (ABSORB)
    if (pVictim->GetTypeId() == TYPEID_PLAYER)
    {
        int randdurability = urand(0, 300);
        if (randdurability == 10)
        {
            DEBUG_LOG("BLOCK: We decrease durability with 5 percent");
            ((Player*)pVictim)->DeathDurabilityLoss(0.05);
        }
    }

    return AbsorbDamage;
}

void Unit::DoAttackDamage(Unit *pVictim, uint32 *damage, uint32 *blocked_amount, uint32 *damageType, uint32 *hitInfo, uint32 *victimState,uint32 *absorbDamage,uint32 *resist)
{
    uint16 pos;
    CreatureInfo *cinfo = NULL;
    if(pVictim->GetTypeId() != TYPEID_PLAYER)
        cinfo = ((Creature*)pVictim)->GetCreatureInfo();

    *damage = int32(*damage*(m_modDamagePCT+100)/100);
    for(std::list<struct DamageDoneCreature*>::iterator i = m_damageDoneCreature.begin();i != m_damageDoneCreature.end();i++)
    {
        if(cinfo && cinfo->type == (*i)->creaturetype)
        {
            *damage += (*i)->damage;
            break;
        }
    }

    if(GetTypeId() == TYPEID_PLAYER)
    {
        for(int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        {
            pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
            ((Player*)this)->CastItemSpell(((Player*)this)->GetItemByPos(pos),pVictim);
        }
    }

    if(GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() != TYPEID_PLAYER && ((Creature*)pVictim)->GetCreatureInfo()->type != 8 )
        ((Player*)this)->UpdateSkillWeapon();

    if (GetUnitCriticalChance()/100 * 512 >= urand(0, 512))
    {
        *hitInfo = 0xEA;
        *damage *= 2;

        pVictim->HandleEmoteCommand(EMOTE_ONESHOT_WOUNDCRITICAL);
    }
    else if ((pVictim->GetUnitParryChance()/100) * 512 >= urand(0, 512))
    {
        *damage = 0;
        *victimState = 2;

        if(pVictim->GetTypeId() == TYPEID_PLAYER)
            ((Player*)pVictim)->UpdateDefense();

        pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);
    }
    else if ((pVictim->GetUnitDodgeChance()/100) * 512 >= urand(0, 512))
    {
        *damage = 0;
        *victimState = 3;

        if(pVictim->GetTypeId() == TYPEID_PLAYER)
            ((Player*)pVictim)->UpdateDefense();

        pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);
    }
    else if ((pVictim->GetUnitBlockChance()/100) * 512 >= urand(0, 512))
    {
        *blocked_amount = (pVictim->GetUnitBlockValue() * (pVictim->GetUnitStrength() / 10));

        if (*blocked_amount < *damage) *damage = *damage - *blocked_amount;
        else *damage = 0;

        if (pVictim->GetUnitBlockValue())
            pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYSHIELD);
        else
            pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);

        *victimState = 4;

        if(pVictim->GetTypeId() == TYPEID_PLAYER)
            ((Player*)pVictim)->UpdateDefense();
    }

    for(std::list<struct DamageShield>::iterator i = pVictim->m_damageShields.begin();i != pVictim->m_damageShields.end();i++)
    {
        pVictim->SpellNonMeleeDamageLog(this,i->m_spellId,i->m_damage);
    }
    uint32 absorb= CalDamageAbsorb(pVictim,NORMAL_DAMAGE,*damage,resist);

    if( (*damage-absorb-*resist) <= 0 )
    {
        *hitInfo = 0x00010020;
        *absorbDamage = absorb;
        *damageType = 0;
        return;
    }
    else
    {
        *absorbDamage = absorb;
    }
    // proc trigger damage
    for (AuraMap::iterator i = pVictim->m_Auras.begin(); i != pVictim->m_Auras.end(); ++i)
    {
        ProcTriggerDamage *procdamage = (*i).second->GetProcDamage();
        if(procdamage)
        {
            bool nocharges = procdamage->procCharges == 0 ? true : false;
            if(procdamage->procFlags & 0x402 && procdamage->procChance > rand_chance()
                && (procdamage->procCharges > 0 || nocharges) && *victimState == 1)
            {
                pVictim->SpellNonMeleeDamageLog(this,(*i).second->GetSpellProto()->Id,procdamage->procDamage);
                if(!nocharges)
                    procdamage->procCharges -= 1;
            }
            else if(procdamage->procFlags == 64 && procdamage->procChance > rand_chance() 
                && (procdamage->procCharges > 0 || nocharges) && *victimState == 4)
            {
                pVictim->SpellNonMeleeDamageLog(this,(*i).second->GetSpellProto()->Id,procdamage->procDamage);
                if(!nocharges)
                    procdamage->procCharges -= 1;
            }
        }
    }
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        ProcTriggerDamage *procdamage = (*i).second->GetProcDamage();
        if(procdamage)
        {
            bool nocharges = procdamage->procCharges == 0 ? true : false;
            if(procdamage->procFlags == 1 && procdamage->procChance > rand_chance()
                && (procdamage->procCharges > 0 || nocharges)  && *victimState == 1)
            {
                SpellNonMeleeDamageLog(pVictim,(*i).second->GetSpellProto()->Id,procdamage->procDamage);
                if(!nocharges)
                    procdamage->procCharges -= 1;
            }
        }
    }
    // proc trigger aura
    for (AuraMap::iterator i = pVictim->m_Auras.begin(); i != pVictim->m_Auras.end(); ++i)
    {
        if(ProcTriggerSpell* procspell = (*i).second->GetProcSpell())
        {
            if(procspell->procFlags & 0x2 && procspell->procChance > rand_chance() )
            {
                SpellEntry *spellInfo = sSpellStore.LookupEntry((*i).second->GetProcSpell()->spellId );

                if(!spellInfo)
                {
                    sLog.outError("WORLD: unknown spell id %i\n", (*i).second->GetProcSpell()->spellId);
                    return;
                }

                Spell *spell = new Spell(pVictim, spellInfo, true, 0);
                WPAssert(spell);

                SpellCastTargets targets;
                targets.setUnitTarget( this );
                spell->prepare(&targets);
            }
        }
    }
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        if(ProcTriggerSpell* procspell = (*i).second->GetProcSpell())
        {
            if(procspell->procFlags & 0x1 && procspell->procChance > rand_chance() )
            {
                SpellEntry *spellInfo = sSpellStore.LookupEntry((*i).second->GetProcSpell()->spellId );

                if(!spellInfo)
                {
                    sLog.outError("WORLD: unknown spell id %i\n", (*i).second->GetProcSpell()->spellId);
                    return;
                }

                Spell *spell = new Spell(pVictim, spellInfo, true, 0);
                WPAssert(spell);

                SpellCastTargets targets;
                targets.setUnitTarget( this );
                spell->prepare(&targets);
            }
        }
    }

    if(pVictim->m_currentSpell && pVictim->GetTypeId() == TYPEID_PLAYER && *damage)
    {
        if (pVictim->m_currentSpell->getState() != SPELL_STATE_CASTING)
        {
            sLog.outString("Spell Delayed!%d",(int32)(0.25f * pVictim->m_currentSpell->casttime));
            pVictim->m_currentSpell->Delayed((int32)(0.25f * pVictim->m_currentSpell->casttime));
        }
        else
        {
            sLog.outString("Spell Canceled!");
            pVictim->m_currentSpell->cancel();
        }
    }
}

void Unit::AttackerStateUpdate (Unit *pVictim)
{
    if(hasUnitState(UNIT_STAT_CONFUSED) || hasUnitState(UNIT_STAT_STUNDED))
        return;
    WorldPacket data;
    uint32   hitInfo = HITINFO_NORMALSWING2|HITINFO_HITSTRANGESOUND1;
    uint32   damageType = NORMAL_DAMAGE;
    uint32   blocked_amount = 0;
    uint32   victimState = VICTIMSTATE_NORMAL;
    int32    attackerSkill = 0;//GetUnitMeleeSkill();
    int32    victimSkill = 0;//pVictim->GetUnitMeleeSkill();
    float    chanceToHit = 100.0f;
    uint32   AbsorbDamage = 0;
    uint32   resist=0;

    if(GetTypeId() == TYPEID_PLAYER)
    {
        uint32 weaponskill = 0;
        Item *tmpitem = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        if (!tmpitem)
            tmpitem = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
        if (tmpitem)
            weaponskill = tmpitem->GetSkill();
        else weaponskill = SKILL_UNARMED;
        attackerSkill = ((Player*)this)->GetSkillValue(weaponskill);
    }
    else attackerSkill = (getLevel()-1)*5;

    if(pVictim->GetTypeId() == TYPEID_PLAYER)
    {
        uint32 weaponskill = 0;
        Item *tmpitem = ((Player*)pVictim)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        if (!tmpitem)
            tmpitem = ((Player*)pVictim)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
        if (tmpitem)
            weaponskill = tmpitem->GetSkill();
        else weaponskill = SKILL_UNARMED;
        victimSkill = ((Player*)pVictim)->GetSkillValue(weaponskill);
    }
    else victimSkill = (pVictim->getLevel()-1)*5;

    //uint32    victimAgility = pVictim->GetUInt32Value(UNIT_FIELD_AGILITY);
    //uint32    attackerAgility = pVictim->GetUInt32Value(UNIT_FIELD_AGILITY);

    if (!pVictim->isAlive())
    {
        SendAttackStop(pVictim->GetGUID());
        return;
    }

    if (m_currentSpell)
    {
        if (m_currentSpell->IsMeleeSpell())
            m_currentSpell->cast();
        return;
    }

    //if(isStunned()) return;

    if (attackerSkill <= victimSkill - 24)
        chanceToHit = 0;
    else if (attackerSkill <= victimSkill)
        chanceToHit = 100.0f - (victimSkill - attackerSkill) * (100.0f / 30.0f);
    if(GetTypeId() != TYPEID_PLAYER && chanceToHit ==0)
        chanceToHit = 15;

    uint32 damage = CalculateDamage (false);

    if((chanceToHit+m_modHitChance)/100 * 512 >= urand(0, 512) )
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
    {
        DoAttackDamage(pVictim, &damage, &blocked_amount, &damageType, &hitInfo, &victimState,&AbsorbDamage,&resist);
        //do animation
        SendAttackStateUpdate(hitInfo, pVictim->GetGUID(), 1, damageType, damage, AbsorbDamage,resist,victimState,blocked_amount);
        DealDamage(pVictim, damage <= (AbsorbDamage+resist) ? 0 : (damage-AbsorbDamage-resist), 0, true);
    }
    else
        //send miss
        SendAttackStateUpdate(hitInfo|HITINFO_MISS, pVictim->GetGUID(), 1, damageType, damage, AbsorbDamage,resist,victimState,blocked_amount);

    if (GetTypeId() == TYPEID_PLAYER)
        DEBUG_LOG("AttackerStateUpdate: (Player) %u %X attacked %u %X for %u dmg,abs is %u,resist is %u .",
            GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage,AbsorbDamage,resist);
    else
        DEBUG_LOG("AttackerStateUpdate: (NPC) %u %X attacked %u %X for %u dmg,abs is %uresist is %u .",
            GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage,AbsorbDamage,resist);
}

uint32 Unit::CalculateDamage(bool ranged)
{
    float min_damage, max_damage, dmg;
    if(ranged)
    {
        min_damage = GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE);
        max_damage = GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE);
    }
    else
    {
        min_damage = GetFloatValue(UNIT_FIELD_MINDAMAGE)+GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE)/2;
        max_damage = GetFloatValue(UNIT_FIELD_MAXDAMAGE)+GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE)/2;
    }
    if (min_damage > max_damage)
    {
        std::swap(min_damage,max_damage);
    }

    if(max_damage == 0.0)
        max_damage = 5.0;

    float diff = max_damage - min_damage + 1;

    dmg = float (rand()%(uint32)diff + (uint32)min_damage);
    return (uint32)dmg;
}

void Unit::SendAttackStop(uint64 victimGuid)
{
    WorldPacket data;
    data.Initialize( SMSG_ATTACKSTOP );
    data << uint8(0xFF) << GetGUID();
    data << uint8(0xFF) << victimGuid;
    data << uint32( 0 );
    data << (uint32)0;

    SendMessageToSet(&data, true);
    sLog.outDetail("%u %X stopped attacking "I64FMT, GetGUIDLow(), GetGUIDHigh(), victimGuid);

    Creature *pVictim = ObjectAccessor::Instance().GetCreature(*this, victimGuid);
    if( pVictim != NULL )
        pVictim->AI().AttackStop(this);
}

void Unit::_UpdateSpells( uint32 time )
{
    if(m_currentSpell != NULL)
    {
        m_currentSpell->update(time);
        if(m_currentSpell->IsAutoRepeat())
        {
            if(m_currentSpell->getState() == SPELL_STATE_FINISHED)
            {
                                                            //Auto shot
                if( m_currentSpell->m_spellInfo->Id == 75 && GetTypeId() == TYPEID_PLAYER )
                    setAttackTimer( 0, true );
                else
                    setAttackTimer(m_currentSpell->m_spellInfo->RecoveryTime);

                m_currentSpell->setState(SPELL_STATE_IDLE);
            }
            else if(m_currentSpell->getState() == SPELL_STATE_IDLE && m_attackTimer == 0)
            {
                m_currentSpell->setState(SPELL_STATE_PREPARING);
                m_currentSpell->ReSetTimer();
            }
        }
        else if(m_currentSpell->getState() == SPELL_STATE_FINISHED)
        {
            delete m_currentSpell;
            m_currentSpell = NULL;
        }
    }

    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end();)
    {
        if ((*i).second)
        {
            (*(i++)).second->Update( time );
        }
    }

    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end();)
    {
        if ((*i).second)
        {
            if ( !(*i).second->GetAuraDuration() && !(*i).second->IsPermanent() )
            {
                RemoveAura(i);
            }
            else
            {
                ++i;
            }
        }
        else
        {
            ++i;
        }
    }

    if(m_dynObj.empty())
        return;
    std::list<DynamicObject*>::iterator ite, dnext;
    for (ite = m_dynObj.begin(); ite != m_dynObj.end(); ite = dnext)
    {
        dnext = ite;
        dnext++;
        //(*i)->Update( difftime );
        if( (*ite)->isFinished() )
        {
            (*ite)->Delete();
            m_dynObj.erase(ite);
            if(m_dynObj.empty())
                break;
            else
                dnext = m_dynObj.begin();
        }
    }
    if(m_gameObj.empty())
        return;
    std::list<GameObject*>::iterator ite1, dnext1;
    for (ite1 = m_gameObj.begin(); ite1 != m_gameObj.end(); ite1 = dnext1)
    {
        dnext1 = ite1;
        dnext1++;
        //(*i)->Update( difftime );
        if( (*ite1)->isFinished() )
        {
            (*ite1)->Delete();
            m_gameObj.erase(ite1);
            if(m_gameObj.empty())
                break;
            else
                dnext1 = m_gameObj.begin();
        }
    }
}

void Unit::_UpdateHostil( uint32 time )
{
    if(!isInCombat() && m_hostilList.size() )
    {
        HostilList::iterator iter;
        for(iter=m_hostilList.begin(); iter!=m_hostilList.end(); ++iter)
        {
            iter->Hostility-=time/1000.0f;
            if(iter->Hostility<=0.0f)
            {
                m_hostilList.erase(iter);
                if(!m_hostilList.size())
                    break;
                else
                    iter = m_hostilList.begin();
            }
        }
    }
}

Unit* Unit::SelectHostilTarget() 
{
    if(!m_hostilList.size())
        return NULL;

    m_hostilList.sort();
    m_hostilList.reverse();
    uint64 guid = m_hostilList.front().UnitGuid;
    if(guid != getVictim()->GetGUID())
        return ObjectAccessor::Instance().GetUnit(*this, guid);
    else
        return NULL;
}


void Unit::castSpell( Spell * pSpell )
{

    if(m_currentSpell)
    {
        m_currentSpell->cancel();
        delete m_currentSpell;
        m_currentSpell = NULL;
    }

    m_currentSpell = pSpell;
}

void Unit::InterruptSpell()
{
    if(m_currentSpell)
    {
        //m_currentSpell->SendInterrupted(0x20);
        m_currentSpell->cancel();
    }
}

bool Unit::isInFront(Unit const* target, float radius)
{
    return GetDistanceSq(target)<=radius * radius && HasInArc( M_PI, target );
}

void Unit::SetInFront(Unit const* target)
{
    m_orientation = GetAngle(target);
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

long Unit::GetTotalAuraModifier(uint32 ModifierID)
{
    uint32 modifier = 0;
    bool auraFound = false;

    AuraMap::const_iterator i;
    for (i = m_Auras.begin(); i != m_Auras.end(); i++)
    {
        if ((*i).second && (*i).second->GetModifier()->m_auraname == ModifierID)
        {
            auraFound = true;
            modifier += (*i).second->GetModifier()->m_amount;
        }
    }
    if (auraFound)
        modifier++;

    return modifier;
}

bool Unit::AddAura(Aura *Aur, bool uniq)
{
    AuraMap::iterator i = m_Auras.find( spellEffectPair(Aur->GetId(), Aur->GetEffIndex()) );
    // take out same spell
    if (i != m_Auras.end())
    {
        (*i).second->SetAuraDuration(Aur->GetAuraDuration());
        if ((*i).second->GetTarget())
            if ((*i).second->GetTarget()->GetTypeId() == TYPEID_PLAYER )
                (*i).second->UpdateAuraDuration();
        delete Aur;
    }
    else
    {
        if (!RemoveNoStackAurasDueToAura(Aur))
        {
            delete Aur;
            return false; // couldnt remove conflicting aura with higher rank
        }

        Aur->_AddAura();
        m_Auras[spellEffectPair(Aur->GetId(), Aur->GetEffIndex())] = Aur;
        m_AuraModifiers[Aur->GetModifier()->m_auraname] += (Aur->GetModifier()->m_amount + 1);

        if (Aur->IsSingleTarget() && Aur->GetTarget() && Aur->GetSpellProto())
        {
            std::list<Aura *> *scAuras = Aur->GetCaster()->GetSingleCastAuras();
            std::list<Aura *>::iterator itr, next;
            for (itr = scAuras->begin(); itr != scAuras->end(); itr = next)
            {
                next = itr;
                next++;
                if ((*itr)->GetTarget() != Aur->GetTarget() &&
                    (*itr)->GetSpellProto()->Category == Aur->GetSpellProto()->Category &&
                    (*itr)->GetSpellProto()->SpellIconID == Aur->GetSpellProto()->SpellIconID &&
                    (*itr)->GetSpellProto()->SpellVisual == Aur->GetSpellProto()->SpellVisual &&
                    (*itr)->GetSpellProto()->Attributes == Aur->GetSpellProto()->Attributes &&
                    (*itr)->GetSpellProto()->AttributesEx == Aur->GetSpellProto()->AttributesEx &&
                    (*itr)->GetSpellProto()->AttributesExEx == Aur->GetSpellProto()->AttributesExEx)
                {
                    (*itr)->GetTarget()->RemoveAura((*itr)->GetId(), (*itr)->GetEffIndex());
                    if(scAuras->empty())
                        break;
                    else
                        next = scAuras->begin();
                }
            }
            scAuras->push_back(Aur);
        }
    }
    return true;
}

void Unit::RemoveRankAurasDueToSpell(uint32 spellId)
{
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return;
    AuraMap::iterator i,next;
    for (i = m_Auras.begin(); i != m_Auras.end(); i = next)
    {
        next = i;
        next++;
        uint32 i_spellId = (*i).second->GetId();
        if((*i).second && i_spellId && i_spellId != spellId)
        {
            if(IsRankSpellDueToSpell(spellInfo,i_spellId))
            {
                RemoveAurasDueToSpell(i_spellId);

                if( m_Auras.empty() )
                    break;
                else
                    next =  m_Auras.begin();
            }
        }
    }
}

bool Unit::RemoveNoStackAurasDueToAura(Aura *Aur)
{
    if (!Aur) 
        return false;
    if (!Aur->GetSpellProto()) return false;
    uint32 spellId = Aur->GetId(); 
    uint32 effIndex = Aur->GetEffIndex();
    bool is_sec = IsSpellSingleEffectPerCaster(spellId);
    AuraMap::iterator i,next;
    for (i = m_Auras.begin(); i != m_Auras.end(); i = next)
    {
        next = i;
        next++;
        if (!(*i).second) continue;
        if (!(*i).second->GetSpellProto()) continue;

        uint32 i_spellId = (*i).second->GetId();
        uint32 i_effIndex = (*i).second->GetEffIndex();
        if(i_spellId != spellId)
        {
            bool sec_match = false;
            if (is_sec && IsSpellSingleEffectPerCaster(i_spellId))
                if (Aur->GetCaster() == (*i).second->GetCaster())
                    if (GetSpellSpecific(spellId) == GetSpellSpecific(i_spellId))
                        sec_match = true;
            
            if(IsNoStackSpellDueToSpell(spellId, i_spellId) || sec_match)
            {
                // if sec_match this isnt always true, needs to be rechecked
                if (IsRankSpellDueToSpell(Aur->GetSpellProto(), i_spellId))
                    if(CompareAuraRanks(spellId, effIndex, i_spellId, i_effIndex) < 0)
                        return false; // cannot remove higher rank
                
                RemoveAurasDueToSpell(i_spellId);

                if( m_Auras.empty() )
                    break;
                else
                    next =  m_Auras.begin();
            }
            else // Potions stack aura by aura
            if (Aur->GetSpellProto()->SpellFamilyName == SPELLFAMILY_POTION &&
                (*i).second->GetSpellProto()->SpellFamilyName == SPELLFAMILY_POTION)
            {
                if (IsNoStackAuraDueToAura(spellId, effIndex, i_spellId, i_effIndex))
                {
                    if(CompareAuraRanks(spellId, effIndex, i_spellId, i_effIndex) < 0)
                    return false; // cannot remove higher rank
                    
                    RemoveAura(i);

                    if( m_Auras.empty() )
                        break;
                    else
                        next =  m_Auras.begin();
                }
            }
        }
    }
    return true;
}

void Unit::RemoveFirstAuraByDispel(uint32 dispel_type)
{
    AuraMap::iterator i;
    for (i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        if ((*i).second && (*i).second->GetSpellProto()->Dispel == dispel_type)
      {
         if(dispel_type == 1)
         {
            bool positive = true;
            switch((*i).second->GetSpellProto()->EffectImplicitTargetA[(*i).second->GetEffIndex()])
            {
               case TARGET_S_E:
               case TARGET_AE_E:
               case TARGET_AE_E_INSTANT:
               case TARGET_AC_E:
               case TARGET_INFRONT:
               case TARGET_DUELVSPLAYER:
               case TARGET_AE_E_CHANNEL:
               case TARGET_AE_SELECTED:
               positive = false;
               break;

               default:
                  positive = ((*i).second->GetSpellProto()->AttributesEx & (1<<7)) ? false : true;
            }
            if(positive)
               continue;
         }
            break;
      }
    }

    if(i == m_Auras.end()) return;

    RemoveAura(i);
}

void Unit::RemoveAura(uint32 spellId, uint32 effindex)
{
    AuraMap::iterator i = m_Auras.find( spellEffectPair(spellId, effindex) );
    if(i != m_Auras.end())
        RemoveAura(i);
}

void Unit::RemoveAurasDueToSpell(uint32 spellId)
{
    for (int i = 0; i < 3; i++)
    {
        AuraMap::iterator iter = m_Auras.find(spellEffectPair(spellId, i));
        if (iter != m_Auras.end())
            RemoveAura(iter);
    }
}

void Unit::RemoveAura(AuraMap::iterator &i)
{
    if ((*i).second->IsSingleTarget())
    {
        std::list<Aura *> *scAuras = (*i).second->GetCaster()->GetSingleCastAuras();
        scAuras->remove((*i).second);
    }
    // remove aura from party members when the caster turns off the aura
    if((*i).second->IsAreaAura())
    {
        Unit *i_caster = (*i).second->GetCaster(), *i_target = (*i).second->GetTarget();
        if(i_caster->GetTypeId() == TYPEID_PLAYER && i_caster->GetGUID() == i_target->GetGUID())
        {
            Group* pGroup = objmgr.GetGroupByLeader(((Player*)i_caster)->GetGroupLeader());
            float radius =  GetRadius(sSpellRadius.LookupEntry((*i).second->GetSpellProto()->EffectRadiusIndex[(*i).second->GetEffIndex()]));
            if(pGroup)
            {
                for(uint32 p=0;p<pGroup->GetMembersCount();p++)
                {
                    Unit* Target = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(p));
                    if(!Target || Target->GetGUID() == i_caster->GetGUID())
                        continue;
                    Aura *t_aura = Target->GetAura((*i).second->GetId(), (*i).second->GetEffIndex());
                    if (t_aura)
                        if (t_aura->GetCaster()->GetGUID() == i_caster->GetGUID())
                            Target->RemoveAura((*i).second->GetId(), (*i).second->GetEffIndex());
                }
            }
        }
    }
    m_AuraModifiers[(*i).second->GetModifier()->m_auraname] -= ((*i).second->GetModifier()->m_amount + 1);
    (*i).second->_RemoveAura();
    delete (*i).second;
    m_Auras.erase(i++);
}

bool Unit::SetAurDuration(uint32 spellId, uint32 effindex,uint32 duration)
{
    AuraMap::iterator iter = m_Auras.find(spellEffectPair(spellId, effindex));
    if (iter != m_Auras.end())
    {
        (*iter).second->SetAuraDuration(duration);
        return true;
    }
    return false;
}

uint32 Unit::GetAurDuration(uint32 spellId, uint32 effindex)
{
    AuraMap::iterator iter = m_Auras.find(spellEffectPair(spellId, effindex));
    if (iter != m_Auras.end())
    {
        return (*iter).second->GetAuraDuration();
    }
    return 0;
}

void Unit::RemoveAllAuras()
{
    while (!m_Auras.empty())
    {
        AuraMap::iterator iter = m_Auras.begin();
        RemoveAura(iter);
    }
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
    uint32 val2,tem_att_power;

    // Armor
    val2 = (uint32)(2*GetUInt32Value(UNIT_FIELD_AGILITY));

    ApplyModUInt32Value(UNIT_FIELD_ARMOR, val2, apply);

    // HP
    val2 = (uint32)((GetUInt32Value(UNIT_FIELD_STAMINA) - pinfo->stamina)*10);

    ApplyModUInt32Value(UNIT_FIELD_MAXHEALTH, val2, apply);

    // MP
    if(getClass() != WARRIOR && getClass() != ROGUE)
    {
        val2 = (uint32)((GetUInt32Value(UNIT_FIELD_IQ) - pinfo->intellect)*15);

        ApplyModUInt32Value(UNIT_FIELD_MAXPOWER1, val2, apply);

    }

    float classrate = 0;

    // Melee Attack Power
    // && Melee DPS - (Damage Per Second)

    //Ranged
    if(getClass() == HUNTER)
        val2 = (getLevel() * 2) + (GetUInt32Value(UNIT_FIELD_AGILITY) * 2) - 20;
    else
        val2 = getLevel() + (GetUInt32Value(UNIT_FIELD_AGILITY) * 2) - 20;


    if(!apply)
        tem_att_power = GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER);

    ApplyModUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER, val2, apply);

    if(apply)
        tem_att_power = GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER);

    val = tem_att_power/14.0f * GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME)/1000;
    ApplyModFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, val, apply);
    ApplyModFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, val, apply);

    //Not-ranged

    switch(getClass())
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
    tem_att_power = GetUInt32Value(UNIT_FIELD_ATTACK_POWER);

    ApplyModUInt32Value(UNIT_FIELD_ATTACK_POWER, val2, apply);

    if(apply)
        tem_att_power = GetUInt32Value(UNIT_FIELD_ATTACK_POWER);

    val = tem_att_power/14.0f * GetUInt32Value(UNIT_FIELD_BASEATTACKTIME)/1000;

    ApplyModFloatValue(UNIT_FIELD_MINDAMAGE, val, apply);
    ApplyModFloatValue(UNIT_FIELD_MAXDAMAGE, val, apply);

    // critical
    if(getClass() == HUNTER) classrate = 53;
    else if(getClass() == ROGUE)  classrate = 29;
    else classrate = 20;

    val = (float)(5 + GetUInt32Value(UNIT_FIELD_AGILITY)/classrate);

    ApplyModFloatValue(PLAYER_CRIT_PERCENTAGE, val, apply);

    //dodge
    if(getClass() == HUNTER) classrate = 26.5;
    else if(getClass() == ROGUE)  classrate = 14.5;
    else classrate = 20;
                                                                  ///*+(Defense*0,04);
    if (getRace() == NIGHTELF)
   val = (float)(GetUInt32Value(UNIT_FIELD_AGILITY)/classrate + 1);
   else
    val = (float)(GetUInt32Value(UNIT_FIELD_AGILITY)/classrate);

    ApplyModFloatValue(PLAYER_DODGE_PERCENTAGE, val, apply);

    //parry
    val = (float)(5);

    ApplyModFloatValue(PLAYER_PARRY_PERCENTAGE, val, apply);

    //block
    val = (float)(GetUInt32Value(UNIT_FIELD_STR)/22);

    ApplyModFloatValue(PLAYER_BLOCK_PERCENTAGE, val, apply);

}

void Unit::_RemoveAllAuraMods()
{
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        ApplyStats(false);
        (*i).second->ApplyModifier(false);
        ApplyStats(true);
    }
}

void Unit::_ApplyAllAuraMods()
{
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        ApplyStats(false);
        (*i).second->ApplyModifier(true);
        ApplyStats(true);
    }
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

Aura* Unit::GetAura(uint32 spellId, uint32 effindex)
{
    AuraMap::iterator iter = m_Auras.find(spellEffectPair(spellId, effindex));
    if (iter != m_Auras.end())
        return iter->second;
    return NULL;
}

float Unit::GetHostility(uint64 guid) const
{
    HostilList::const_iterator i;
    for ( i = m_hostilList.begin(); i!= m_hostilList.end(); i++)
    {
        if(i->UnitGuid==guid)
            return i->Hostility;
    }
    return 0.0f;
}

void Unit::AddHostil(uint64 guid, float hostility)
{
    HostilList::iterator i;
    for(i = m_hostilList.begin(); i != m_hostilList.end(); i++)
    {
        if(i->UnitGuid==guid)
        {
            i->Hostility+=hostility;
            return;
        }
    }
    m_hostilList.push_back(Hostil(guid,hostility));
}

void Unit::AddItemEnchant(uint32 enchant_id,bool apply)
{
    SpellItemEnchantment *pEnchant;
    pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if(!pEnchant)
        return;
    uint32 enchant_display = pEnchant->display_type;
    uint32 enchant_value1 = pEnchant->value1;
    //uint32 enchant_value2 = pEnchant->value2;
    uint32 enchant_spell_id = pEnchant->spellid;
    //uint32 enchant_aura_id = pEnchant->aura_id;
    //uint32 enchant_description = pEnchant->description;
    SpellEntry *enchantSpell_info = sSpellStore.LookupEntry(enchant_spell_id);

    if(enchant_display ==4)
    {
        ApplyModUInt32Value(UNIT_FIELD_ARMOR,enchant_value1,apply);
    }
    else if(enchant_display ==2)
    {
        if(getClass() == CLASS_HUNTER)
        {
            ApplyModUInt32Value(UNIT_FIELD_MINRANGEDDAMAGE,enchant_value1,apply);
            ApplyModUInt32Value(UNIT_FIELD_MAXRANGEDDAMAGE,enchant_value1,apply);
        }
        else
        {
            ApplyModUInt32Value(UNIT_FIELD_MINDAMAGE,enchant_value1,apply);
            ApplyModUInt32Value(UNIT_FIELD_MAXDAMAGE,enchant_value1,apply);
        }
    }
    else
    {
        if(apply)
        {
            Spell *spell = new Spell(this, enchantSpell_info, true, 0);
            SpellCastTargets targets;
            targets.setUnitTarget(this);
            spell->prepare(&targets);
        }
        else RemoveAurasDueToSpell(enchant_spell_id);
    }
}

void Unit::AddDynObject(DynamicObject* dynObj)
{
    m_dynObj.push_back(dynObj);
}

void Unit::RemoveDynObject(uint32 spellid)
{
    if(m_dynObj.empty())
        return;
    std::list<DynamicObject*>::iterator i, next;
    for (i = m_dynObj.begin(); i != m_dynObj.end(); i = next)
    {
        next = i;
        next++;
        if(spellid == 0 || (*i)->GetSpellId() == spellid)
        {
            (*i)->Delete();
            m_dynObj.erase(i);
            if(m_dynObj.empty())
                break;
            else
                next = m_dynObj.begin();
        }
    }
}

void Unit::AddGameObject(GameObject* gameObj)
{
    m_gameObj.push_back(gameObj);
}

void Unit::RemoveGameObject(uint32 spellid, bool del)
{
    if(m_gameObj.empty())
        return;
    std::list<GameObject*>::iterator i, next;
    for (i = m_gameObj.begin(); i != m_gameObj.end(); i = next)
    {
        next = i;
        next++;
        if(spellid == 0 || (*i)->GetSpellId() == spellid)
        {
            if(del) 
                (*i)->Delete();

            m_gameObj.erase(i);
            if(m_gameObj.empty())
                break;
            else
                next = m_gameObj.begin();
        }
    }
}

void Unit::SendSpellNonMeleeDamageLog(uint64 targetGUID,uint32 SpellID,uint32 Damage, uint8 DamageType,uint32 AbsorbedDamage, uint32 Resist,bool PhysicalDamage, uint32 Blocked)
{
    WorldPacket data;
    data.Initialize(SMSG_SPELLNONMELEEDAMAGELOG);
    data << uint8(0xFF) << targetGUID;
    data << uint8(0xFF) << GetGUID();
    data << SpellID;
    data << Damage;
    data << DamageType;                                     //damagetype
    data << AbsorbedDamage;                                 //AbsorbedDamage
    data << Resist;                                         //resist
    data << (uint8)PhysicalDamage;
    data << uint8(0);
    data << Blocked;                                        //blocked
    data << uint8(0);
    SendMessageToSet( &data, true );
}

void Unit::SendAttackStateUpdate(uint32 HitInfo, uint64 targetGUID, uint8 SwingType, uint32 DamageType, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, uint32 TargetState, uint32 BlockedAmount)
{
    sLog.outDebug("WORLD: Sending SMSG_ATTACKERSTATEUPDATE");

    WorldPacket data;
    data.Initialize(SMSG_ATTACKERSTATEUPDATE);
    data << (uint32)HitInfo;
    data << uint8(0xFF) << GetGUID();                       //source GUID
    data << uint8(0xFF) << targetGUID;                      //Target GUID
    data << (uint32)(Damage-AbsorbDamage);

    data << (uint8)SwingType;
    data << (uint32)DamageType;

    data << (float)Damage;                                  //
    data << (uint32)Damage;                                 // still need to double check damaga
    data << (uint32)AbsorbDamage;
    data << (uint32)Resist;
    data << (uint32)TargetState;

    if( AbsorbDamage == 0 )                                 //also 0x3E8 = 0x3E8, check when that happens
        data << (uint32)0;
    else
        data << (uint32)-1;

    data << (uint32)0;
    data << (uint32)BlockedAmount;

    SendMessageToSet( &data, true );
}

void Unit::setPowerType(uint8 PowerType)
{
    uint32 tem_bytes_0 = GetUInt32Value(UNIT_FIELD_BYTES_0);
    SetUInt32Value(UNIT_FIELD_BYTES_0,((tem_bytes_0<<8)>>8) + (uint32(PowerType)<<24));
    uint8 new_powertype = getPowerType();
    if(new_powertype == 3)
    {
        SetUInt32Value(UNIT_FIELD_MAXPOWER4,100);
        SetUInt32Value(UNIT_FIELD_POWER4,100);
    }
    if(new_powertype == 2)
    {
        SetUInt32Value(UNIT_FIELD_MAXPOWER3,100);
        SetUInt32Value(UNIT_FIELD_POWER3,100);
    }
    if(new_powertype == 1)
    {
        SetUInt32Value(UNIT_FIELD_MAXPOWER2,1000);
        SetUInt32Value(UNIT_FIELD_POWER2,0);
    }
    if(new_powertype == 4)
    {
        SetUInt32Value(UNIT_FIELD_MAXPOWER5,1000000);
        SetUInt32Value(UNIT_FIELD_POWER5,1000000);
    }
}

FactionTemplateEntry* Unit::getFactionTemplateEntry() const
{
    FactionTemplateEntry* entry = sFactionTemplateStore.LookupEntry(getFaction());
    if(!entry) 
    {
        if(GetTypeId() == TYPEID_PLAYER)
            sLog.outError("Player %s have invalide faction (fuction template id) #%u", ((Player*)this)->GetName(), getFaction());
        else
            sLog.outError("Creature (template id: %u) have invalide faction (fuction template id) #%u", ((Creature*)this)->GetCreatureInfo()->Entry, getFaction());
    }
    return entry;
}


bool Unit::Attack(Unit *victim)
{
    if(victim == this)
        return false;

    if (m_attacking)
    {
        if (m_attacking == victim)
            return false;
        AttackStop();
    }
    addUnitState(UNIT_STAT_ATTACKING);
    SetFlag(UNIT_FIELD_FLAGS, 0x80000);
    m_attacking = victim;
    m_attacking->_addAttacker(this);
    return true;
}

bool Unit::AttackStop()
{
    if (!m_attacking)
        return false;

    m_attacking->_removeAttacker(this);
    m_attacking = NULL;
    clearUnitState(UNIT_STAT_ATTACKING);
    RemoveFlag(UNIT_FIELD_FLAGS, 0x80000);
    return true;
}

bool Unit::isInCombatWithPlayer() const
{
    if(getVictim() && getVictim()->GetTypeId() == TYPEID_PLAYER) 
        return true;

    for(AttackerSet::const_iterator i = m_attackers.begin(); i != m_attackers.end(); ++i)
    {
        if((*i)->GetTypeId() == TYPEID_PLAYER) return true;
    }
    return false;
}

void Unit::RemoveAllAttackers()
{
    while (m_attackers.size() != 0)
    {
        AttackerSet::iterator iter = m_attackers.begin();
        if(!(*iter)->AttackStop())
        {
            sLog.outError("WORLD: Unit has an attacker that isnt attacking it!");
            m_attackers.erase(iter);
        }
    }
}

void Unit::SetStateFlag(uint32 index, uint32 newFlag )
{
    index |= newFlag;
}

void Unit::RemoveStateFlag(uint32 index, uint32 oldFlag )
{
    index &= ~ oldFlag;
}

Creature* Unit::GetPet() const
{
    uint64 pet_guid = GetPetGUID();
    if(pet_guid)
    {
        Creature* pet = ObjectAccessor::Instance().GetCreature(*this, pet_guid);
        if(!pet)
        {
            sLog.outError("Unit::GetPet: Pet %u not exist.",pet->GetGUIDLow());
            const_cast<Unit*>(this)->SetPet(0);
        }
        return pet;
    }
    else
        return NULL;
}

void Unit::SetPet(Creature* pet) 
{ 
    SetUInt64Value(UNIT_FIELD_SUMMON,pet ? pet->GetGUID() : 0); 
}
