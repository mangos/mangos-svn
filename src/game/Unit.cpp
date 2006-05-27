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

    m_immuneToEffect = 0;
    m_immuneToState  = 0;
    m_immuneToSchool = 0;
    m_immuneToDmg    = 0;
    m_immuneToDispel = 0;
    m_immuneToStealth = 0;
    m_stealthvalue = 0;

    m_ReflectSpellSchool = 0;
    m_ReflectSpellPerc   = 0;

    m_damageManaShield = NULL;
    m_spells[0] = 0;
    m_spells[1] = 0;
    m_spells[2] = 0;
    m_spells[3] = 0;
    m_transform = 0;
    m_ShapeShiftForm = 0;

    for (int i = 0; i < TOTAL_AURAS; i++)
        m_AuraModifiers[i] = -1;

    m_attacking = NULL;
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
    float orientation = (float)atan2((double)dy, (double)dx);

    WorldPacket data;
    data.Initialize( SMSG_MONSTER_MOVE );
    data << uint8(0xFF);
    data << GetGUID();
    data << GetPositionX() << GetPositionY() << GetPositionZ();
    data << (uint32)getMSTime();
    data << uint8(0);
    data << uint32(run ? 0x00000100 : 0x00000000);
    data << time;
    data << uint32(1);
    data << x << y << z;
    //WPAssert( data.size() == 49 );
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
                m_attackTimer = GetUInt32Value(UNIT_FIELD_BASEATTACKTIME+1);
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

SpellEntry *Unit::reachWithSpellAttack(Unit *pVictim)
{
    if(!pVictim)
        return NULL;
    SpellEntry *spellInfo;
    for(uint32 i=0;i<UNIT_MAX_SPELLS;i++)
    {
        if(!m_spells[i])
            continue;
        spellInfo = sSpellStore.LookupEntry(m_spells[i] );
        if(!spellInfo)
        {
            sLog.outError("WORLD: unknown spell id %i\n", m_spells[i]);
            continue;
        }

        /*spell = new Spell(this, spellInfo, false, 0);
        spell->m_targets = targets;
        if(!spell)
        {
            sLog.outError("WORLD: can't get spell. spell id %i\n", m_spells[i]);
            continue;
        }*/
        if(spellInfo->manaCost > GetUInt32Value(UNIT_FIELD_POWER1))
            continue;
        SpellRange* srange = sSpellRange.LookupEntry(spellInfo->rangeIndex);
        float range = GetMaxRange(srange);
        float minrange = GetMinRange(srange);
        float dist = GetDistanceSq(pVictim);
        //if(!isInFront( pVictim, range ) && spellInfo->AttributesEx )
        //    continue;
        if( dist > range * range || dist < minrange * minrange )
            continue;
        if(m_silenced)
            continue;
        return spellInfo;
    }
    return NULL;
}

bool Unit::canReachWithAttack(Unit *pVictim) const
{
    float reach = GetFloatValue(UNIT_FIELD_COMBATREACH);
    if( reach <= 0.0f )
        reach = 1.0f;
    float distance = GetDistanceSq(pVictim);

    return ( distance <= reach * reach );
}

void Unit::RemoveSpellsCausingAura(uint32 auraType) {
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end(); iter++) {
        if ((*iter).second) {
            if (((*iter).second)->GetModifier()->m_auraname == auraType)
            {
                uint32 spellId = ((*iter).second)->GetId();
                RemoveAurasDueToSpell(spellId);
            }
        }
    }
}

bool Unit::HasAuraType(uint32 auraType) const {
    return (m_AuraModifiers[auraType] != -1);
}


void Unit::DealDamage(Unit *pVictim, uint32 damage, uint32 procFlag, bool durabilityLoss)
{

    uint32 crtype = 0;

    if(isStealth())
        RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
    
    if(pVictim->GetTypeId() != TYPEID_PLAYER)
    {
        crtype = ((Creature*)pVictim)->GetCreatureInfo()->type;
        pVictim->SetInFront(this);
        ((Creature*)pVictim)->AI().AttackStart(this);
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
        {
            ((Creature*)pVictim)->generateLoot();
        }
        
        // If a player kill some one call honor calcules
        // TODO: We need to count dishonorable kills for civilian creatures.

        DEBUG_LOG("DealDamageAura");
        pVictim->setDeathState(JUST_DIED);
        //if(m_currentSpell && !m_currentSpell->IsAreaAura() && m_currentSpell->m_targets.getUnitTarget()->GetGUID() == pVictim->GetGUID())
        //    m_currentSpell->cancel();

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

        // 10% durability loss on death
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
        {
            DEBUG_LOG("We are dead, loosing 10 percents durability");
            if (durabilityLoss)
            {
                ((Player*)pVictim)->DeathDurabilityLoss(0.10);
            }
            std::list<Hostil*>::iterator i;
            for(i = m_hostilList.begin(); i != m_hostilList.end(); i++)
            {
                if((*i)->UnitGuid==victimGuid)
                {
                    m_hostilList.erase(i);
                    break;
                }
            }
            uint64 petguid;
            if((petguid=pVictim->GetUInt64Value(UNIT_FIELD_SUMMON)) != 0)
            {
                Creature *pet;
                pet = ObjectAccessor::Instance().GetCreature(*pVictim, petguid);
                if(pet && pet->isPet())
                {
                    pet->setDeathState(JUST_DIED);
                    pet->smsg_AttackStop(attackerGuid);
                    pet->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
                    pet->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
                    pet->RemoveFlag(UNIT_FIELD_FLAGS, 0x00080000);
                    pet->addUnitState(UNIT_STAT_DIED);
                    for(i = m_hostilList.begin(); i != m_hostilList.end(); i++)
                    {
                        if((*i)->UnitGuid==pet->GetGUID())
                        {
                            m_hostilList.erase(i);
                            break;
                        }
                    }
                }
                //pVictim->SetUInt64Value( UNIT_FIELD_SUMMON, 0 );
            }
        }
        else
        {
            pVictim->m_hostilList.clear();
            DEBUG_LOG("DealDamageNotPlayer");
            if (crtype == 8)
                pVictim->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
            else
                pVictim->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 1);
        }

        bool playerkill = false;
        Player *player;
        if(GetTypeId() != TYPEID_PLAYER && ((Creature*)this)->isPet() && (crtype != 8))
        {
            Unit* owner = ((Pet*)this)->GetOwner();
            if(!owner)
                playerkill = false;
            if(owner->GetTypeId() == TYPEID_PLAYER)
            {
                player = (Player*)owner;
                playerkill = true;
            }
        }
        if ((GetTypeId() == TYPEID_PLAYER) && (crtype != 8))
        {
            playerkill = true;
            player = (Player*)this;
        }
        if (playerkill)
        {
            if(pVictim->GetTypeId() != TYPEID_PLAYER)
                player->AddQuestsLoot((Creature*)pVictim);
            player->CalculateHonor(pVictim);
            DEBUG_LOG("DealDamageIsPlayer");
            uint32 xp = MaNGOS::XP::Gain(static_cast<Player *>(player), pVictim);
            uint32 entry = 0;
            if (pVictim->GetTypeId() != TYPEID_PLAYER)
                entry = pVictim->GetUInt32Value(OBJECT_FIELD_ENTRY );

            Group *pGroup = objmgr.GetGroupByLeader(player->GetGroupLeader());
            if (pGroup)
            {
                DEBUG_LOG("DealDamageInGroup");
                xp /= pGroup->GetMembersCount();
                for (uint32 i = 0; i < pGroup->GetMembersCount(); i++)
                {
                    Player *pGroupGuy = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(i));
                    if(!pGroupGuy)
                        continue;
                    if(GetDistanceSq(pGroupGuy) > sWorld.getConfig(CONFIG_GETXP_DISTANCE))
                        continue;
                    if(abs((int)pGroupGuy->getLevel() - (int)pVictim->getLevel()) > sWorld.getConfig(CONFIG_GETXP_LEVELDIFF))
                        continue;
                    pGroupGuy->GiveXP(xp, victimGuid);

                    if (pVictim->GetTypeId() != TYPEID_PLAYER)
                        pGroupGuy->KilledMonster(entry, victimGuid);
                }
            }
            else
            {
                DEBUG_LOG("DealDamageNotInGroup");

                player->GiveXP(xp, victimGuid);

                if (pVictim->GetTypeId() != TYPEID_PLAYER)
                    player->KilledMonster(entry,victimGuid);
            }
        }
        else
        {
            DEBUG_LOG("DealDamageIsCreature");
            smsg_AttackStop(victimGuid);
            addUnitState(UNIT_STAT_DIED);
        }
        AttackStop();
    }
    else
    {
        DEBUG_LOG("DealDamageAlive");
        pVictim->SetUInt32Value(UNIT_FIELD_HEALTH , health - damage);
        pVictim->addAttacker(this);

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
                && getClass() == WARRIOR || m_form == 5 || m_form == 8)
                ((Player*)this)->CalcRage(damage,true);
        }
        else
        {
            ((Player*)pVictim)->addUnitState(UNIT_STAT_ATTACKING);

            if( (((Player*)pVictim)->getClass()) == WARRIOR )
                ((Player*)pVictim)->CalcRage(damage,false);

            // random durability for items (HIT)
            int randdurability = urand(0, 300);
            if (randdurability == 10)
            {
                DEBUG_LOG("HIT: We decrease durability with 5 percent");
                ((Player*)pVictim)->DeathDurabilityLoss(0.05);
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

    //if(pVictim->m_damageManaShield)
    //    if(pVictim->m_damageManaShield->m_modType == SPELL_AURA_SCHOOL_ABSORB)
    // FIX-ME
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellID);
    if(spellInfo)
        absorb=CalDamageAbsorb(pVictim,spellInfo->School,damage);

    WorldPacket data;

    if( (damage-absorb)==0 )
    {
            SendAttackStateUpdate(HITINFO_HITSTRANGESOUND1|HITINFO_NOACTION, pVictim->GetGUID(), 1, 0, damage, absorb,0,1,0);
        return;
    }
    else damage=damage-absorb;

    sLog.outDetail("SpellNonMeleeDamageLog: %u %X attacked %u %X for %u dmg inflicted by %u",
        GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage, spellID);

    SendSpellNonMeleeDamageLog(pVictim->GetGUID(), spellID, damage, NORMAL_DAMAGE, 0,0,false,0);
    DealDamage(pVictim, damage, 0, true);
}

void Unit::PeriodicAuraLog(Unit *pVictim, SpellEntry *spellProto, Modifier *mod)
{
    uint32 procFlag = 0;
    if(!this || !pVictim || !isAlive() || !pVictim->isAlive())
    {
        return;
    }
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

    if(mod->m_auraname == SPELL_AURA_PERIODIC_DAMAGE)
    {
        SendSpellNonMeleeDamageLog(pVictim->GetGUID(), spellProto->Id, mod->m_amount, NORMAL_DAMAGE, 0,0,false,0);
        SendMessageToSet(&data,true);

        DealDamage(pVictim, mod->m_amount, procFlag, true);
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_HEAL)
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
        }
        else
        {
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
            }
            else
            {
                pVictim->SetUInt32Value(UNIT_FIELD_POWER1, 0 );
                AbsorbDamage = AbsorbDamage - currentPower/2;
                removeAura = true;
            }
        }

        if(removeAura)
            pVictim->RemoveAurasDueToSpell(pVictim->m_damageManaShield->m_spellId);
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

    // proc trigger aura
    for (AuraMap::iterator i = pVictim->m_Auras.begin(); i != pVictim->m_Auras.end(); ++i)
    {
        if(ProcTriggerSpell* procspell = (*i).second->GetProcSpell())
        {
            if(procspell->procFlags == 40 && procspell->procChance * 1000 > rand() % 100000)
            {
                SpellEntry *spellInfo = sSpellStore.LookupEntry((*i).second->GetProcSpell()->spellId );

                if(!spellInfo)
                {
                    sLog.outError("WORLD: unknown spell id %i\n", (*i).second->GetProcSpell()->spellId);
                    return;
                }

                Spell *spell = new Spell(pVictim, spellInfo, false, 0);
                WPAssert(spell);

                SpellCastTargets targets;
                targets.setUnitTarget( this );
                spell->prepare(&targets);
            }
        }
    }

    if(pVictim->m_currentSpell && pVictim->GetTypeId() == TYPEID_PLAYER && *damage)
    {
        sLog.outString("Spell Delayed!");
        pVictim->m_currentSpell->Delayed((int32)(0.25f * pVictim->m_currentSpell->casttime));
    }
}

void Unit::AttackerStateUpdate (Unit *pVictim, uint32 damage)
{
    if(hasUnitState(UNIT_STAT_CONFUSED))
        return;
    WorldPacket data;
    uint32   hitInfo = HITINFO_NORMALSWING2|HITINFO_HITSTRANGESOUND1;
    uint32   damageType = NORMAL_DAMAGE;
    uint32   blocked_amount = 0;
    uint32   victimState = VICTIMSTATE_NORMAL;
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

    if(!damage) damage = CalculateDamage (false);

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
    {
        DoAttackDamage(pVictim, &damage, &blocked_amount, &damageType, &hitInfo, &victimState,&AbsorbDamage,&Turn);
        //do animation
        SendAttackStateUpdate(hitInfo, pVictim->GetGUID(), 1, damageType, damage, AbsorbDamage,Turn,victimState,blocked_amount);
        DealDamage(pVictim, damage-AbsorbDamage, 0, true);
    }
    else
        //send miss
        SendAttackStateUpdate(hitInfo|HITINFO_MISS, pVictim->GetGUID(), 1, damageType, damage, AbsorbDamage,Turn,victimState,blocked_amount);
    

    if (GetTypeId() == TYPEID_PLAYER)
        DEBUG_LOG("AttackerStateUpdate: (Player) %u %X attacked %u %X for %u dmg.",
            GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage);
    else
        DEBUG_LOG("AttackerStateUpdate: (NPC) %u %X attacked %u %X for %u dmg.",
            GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage);
}

uint32 Unit::CalculateDamage(bool ranged)
{
    uint32 attack_power;

    float min_damage, max_damage, dmg;
    if(ranged)
    {
        if(getClass() == HUNTER)
            attack_power = (getLevel() * 2) + (GetUInt32Value(UNIT_FIELD_AGILITY) * 2) - 20;
        else
            attack_power = getLevel() + (GetUInt32Value(UNIT_FIELD_AGILITY) * 2) - 20;

        dmg = attack_power / 14.0f * GetFloatValue(UNIT_FIELD_RANGEDATTACKTIME);
        min_damage = GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE);
        max_damage = GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE);
    }
    else
    {
        attack_power = GetUInt32Value(UNIT_FIELD_ATTACK_POWER);
        dmg = attack_power / 14.0f * GetFloatValue(UNIT_FIELD_BASEATTACKTIME);
        min_damage = GetFloatValue(UNIT_FIELD_MINDAMAGE)+GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE);
        max_damage =GetFloatValue(UNIT_FIELD_MAXDAMAGE)+GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE);
    }
    if (min_damage > max_damage)
    {
        float temp = max_damage;
        max_damage = min_damage;
        min_damage = temp;
    }

    if(max_damage==0)
        max_damage=5;

    float diff = max_damage - min_damage + 1;

    dmg += float (rand()%(uint32)diff + (uint32)min_damage);
    return (uint32)dmg;
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

    // update aura (may set some pointer in iterators to NULL
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end();)
    {
        (*i).second->Update( time );
        if ( !(*i).second->GetAuraDuration() && !(*i).second->IsPermanent() ) {
            RemoveAura(i);
        } else {
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
}

void Unit::_UpdateHostil( uint32 time )
{
    if(!isInCombat() && m_hostilList.size() )
    {
        std::list<Hostil*>::iterator iter;
        for(iter=m_hostilList.begin(); iter!=m_hostilList.end(); iter++)
        {
            (*iter)->Hostility-=time/1000.0f;
            if((*iter)->Hostility<=0.0f)
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
        //m_currentSpell->SendInterrupted(0x20);
        m_currentSpell->cancel();
        m_currentSpell = NULL;
    }
}

bool Unit::isInFront(Unit const* target, float radius)
{
    return GetDistanceSq(target)<=radius * radius && IsInArc( M_PI, target );
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

long Unit::GetTotalAuraModifier(uint32 ModifierID) {
    uint32 modifier = 0;
    bool auraFound = false;
    
    AuraMap::const_iterator i;
    for (i = m_Auras.begin(); i != m_Auras.end(); i++) {
        if ((*i).second && (*i).second->GetModifier()->m_auraname == ModifierID) {
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
    //_RemoveStatsMods();

    // take out same spell
    if (i != m_Auras.end()) {
        (*i).second->SetAuraDuration(Aur->GetAuraDuration());
        delete Aur;
    }
    else
    {
        m_Auras[spellEffectPair(Aur->GetId(), Aur->GetEffIndex())] = Aur;
        m_AuraModifiers[Aur->GetModifier()->m_auraname] += ((*i).second->GetModifier()->m_amount + 1);
        Aur->_AddAura();
    }
    //_ApplyStatsMods();

    return true;
}

void Unit::RemoveFirstAuraByCategory(uint32 category)
{
    AuraMap::iterator i;
    for (i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        if ((*i).second && (*i).second->GetSpellProto()->Category == category)
            break;
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
    for (int i = 0; i < 3; i++) {
        AuraMap::iterator iter = m_Auras.find(spellEffectPair(spellId, i));
        if (iter != m_Auras.end())
            RemoveAura(iter);
    }
}

void Unit::RemoveAura(AuraMap::iterator &i)
{
    //_RemoveStatsMods();
    m_AuraModifiers[(*i).second->GetModifier()->m_auraname] -= ((*i).second->GetModifier()->m_amount + 1);
    (*i).second->_RemoveAura();
    delete (*i).second;
    m_Auras.erase(i++);
    //_ApplyStatsMods();
}

void Unit::RemoveAuraRank(uint32 spellId)
{
    SpellEntry *i_spellInfo;
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId);
    for(AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end();  ++i)
    {
        i_spellInfo =(*i).second->GetSpellProto();
        uint8 j=(*i).second->GetEffIndex();
        for(uint8 k=0;k<3;k++)
        if (i_spellInfo->EffectApplyAuraName[j] == spellInfo->EffectApplyAuraName[k] )
            RemoveAura(i);
    }
}

bool Unit::SetAurDuration(uint32 spellId, uint32 effindex,uint32 duration)
{
    AuraMap::iterator iter = m_Auras.find(spellEffectPair(spellId, effindex));
    if (iter != m_Auras.end()) {
        (*iter).second->SetAuraDuration(duration);
        return true;
    }
    return false;
}

uint32 Unit::GetAurDuration(uint32 spellId, uint32 effindex)
{
    AuraMap::iterator iter = m_Auras.find(spellEffectPair(spellId, effindex));
    if (iter != m_Auras.end()) {
        return (*iter).second->GetAuraDuration();
    }
    return 0;
}

void Unit::RemoveAllAuras()
{

    //_RemoveStatsMods();

    while (!m_Auras.empty()) {
        RemoveAura(m_Auras.begin()); 
    }

    //_ApplyStatsMods();
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

                                                            ///*+(Defense*0,04);
    val = (float)(GetUInt32Value(UNIT_FIELD_AGILITY)/classrate);
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
    AuraMap::iterator i;

    //_RemoveStatsMods();

    for (i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        //(*i)->ApplyModifier(false);
        (*i).second->_RemoveAura();
        //RemoveAura(i);
        //if(m_Auras.empty())
        //    break;
        //else
        //    i = m_Auras.begin();
    }

    //_ApplyStatsMods();
}

void Unit::_ApplyAllAuraMods()
{
    //_RemoveStatsMods();

    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        //(*i)->ApplyModifier(true);
        //(*i)->_RemoveAura();
        (*i).second->_AddAura();
    }

    //_ApplyStatsMods();
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

Hostil* Unit::GetHostil(uint64 guid)
{
    std::list<Hostil*>::iterator i;
    for(i = m_hostilList.begin(); i != m_hostilList.end(); i++)
        if((*i)->UnitGuid==guid)
            return (*i);
    return NULL;
}

float Unit::GetHostility(uint64 guid)
{
    std::list<Hostil*>::iterator i;
    for ( i = m_hostilList.begin(); i!= m_hostilList.end(); i++)
    {
        if((*i)->UnitGuid==guid)
            return (*i)->Hostility;
    }
    return 0.0f;
}

void Unit::AddHostil(uint64 guid, float hostility)
{
    std::list<Hostil*>::iterator i;
    for(i = m_hostilList.begin(); i != m_hostilList.end(); i++)
    {
        if((*i)->UnitGuid==guid)
        {
            (*i)->Hostility+=hostility;
            return;
        }
    }
    Hostil *uh=new Hostil;
    uh->UnitGuid=guid;
    uh->Hostility=hostility;
    m_hostilList.push_back(uh);
}

void Unit::AddItemEnchant(uint32 enchant_id)
{
    SpellItemEnchantment *pEnchant;
    Aura *pAura;
    pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if(!pEnchant)
        return;
    uint32 enchant_display = pEnchant->display_type;
    uint32 enchant_value1 = pEnchant->value1;
    uint32 enchant_value2 = pEnchant->value2;
    uint32 enchant_spell_id = pEnchant->spellid;
    uint32 enchant_aura_id = pEnchant->aura_id;
    uint32 enchant_description = pEnchant->description;
    SpellEntry *enchantSpell_info = sSpellStore.LookupEntry(enchant_spell_id);

    if(enchant_display ==4)
        SetUInt32Value(UNIT_FIELD_ARMOR,GetUInt32Value(UNIT_FIELD_ARMOR)+enchant_value1);
    else if(enchant_display ==2)
    {
        SetUInt32Value(UNIT_FIELD_MINDAMAGE,GetUInt32Value(UNIT_FIELD_MINDAMAGE)+enchant_value1);
        SetUInt32Value(UNIT_FIELD_MAXDAMAGE,GetUInt32Value(UNIT_FIELD_MAXDAMAGE)+enchant_value1);
    }
    else
    {
        for(int x = 0;x < 3;x++)
            if(enchantSpell_info->Effect[x])
        {
            //uint32 pAura_id = enchantSpell_info->Effect[x];
            pAura = new Aura(enchantSpell_info,x,this,this);
            AddAura(pAura);
        }
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

void Unit::SendSpellNonMeleeDamageLog(uint64 targetGUID,uint32 SpellID,uint32 Damage, uint8 DamageType,uint32 AbsorbedDamage, uint32 Resist,bool PhysicalDamage, uint32 Blocked)
{
    WorldPacket data;
    data.Initialize(SMSG_SPELLNONMELEEDAMAGELOG);
    data << uint8(0xFF) << targetGUID;
    data << uint8(0xFF) << GetGUID();
    data << SpellID;
    data << Damage;
    data << DamageType;//damagetype
    data << AbsorbedDamage; //AbsorbedDamage
    data << Resist; //resist
    data << (uint8)PhysicalDamage;
    data << uint8(0);
    data << Blocked; //blocked
    data << uint8(0);
    SendMessageToSet( &data, true );
}


void Unit::SendAttackStateUpdate(uint32 HitInfo, uint64 targetGUID, uint8 SwingType, uint32 DamageType, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, uint32 TargetState, uint32 BlockedAmount)
{
    sLog.outDebug("WORLD: Sending SMSG_ATTACKERSTATEUPDATE");

    WorldPacket data;
    data.Initialize(SMSG_ATTACKERSTATEUPDATE);      
    data << (uint32)HitInfo;                        
    data << uint8(0xFF) << GetGUID();               //source GUID
    data << uint8(0xFF) << targetGUID;               //Target GUID
    data << (uint32)(Damage-AbsorbDamage);        

    data << (uint8)SwingType;                     
    data << (uint32)DamageType;                   

    data << (float)Damage;                        //
    data << (uint32)Damage;                       // still need to double check damaga
    data << (uint32)AbsorbDamage;                 
    data << (uint32)Resist;                       
    data << (uint32)TargetState;                  

    if( AbsorbDamage == 0 )                       //also 0x3E8 = 0x3E8, check when that happens
        data << (uint32)0;                          
    else
        data << (uint32)-1;                       
    
    data << (uint32)0;                            
    data << (uint32)BlockedAmount;

    SendMessageToSet( &data, true );
}


void Unit::setShapeShiftForm(uint32 modelid)
{
    SetUInt32Value(GetUInt32Value(UNIT_FIELD_DISPLAYID),modelid);
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
    if(new_powertype == 1)
    {
        SetUInt32Value(UNIT_FIELD_MAXPOWER2,1000);
        SetUInt32Value(UNIT_FIELD_POWER2,1);
    }
}

void Unit::Attack(Unit *victim) {
    if (m_attacking)
        AttackStop();
    addUnitState(UNIT_STAT_ATTACKING);
    SetFlag(UNIT_FIELD_FLAGS, 0x80000);
    m_attacking = victim;
}

void Unit::AttackStop() {
    if (!m_attacking)
        return;

    m_attacking->removeAttacker(this);
    m_attacking = NULL;
    clearUnitState(UNIT_STAT_ATTACKING);
    RemoveFlag(UNIT_FIELD_FLAGS, 0x80000);
}

/*********************************************************/
/***                    SPELL SYSTEM                   ***/
/*********************************************************/
void Unit::SendDamageToLog( Unit *pUnit, Spell *pSpell, uint32 damage )
{
    if( pUnit && pSpell )
    {
        SendSpellNonMeleeDamageLog(pUnit->GetGUID(), pSpell->m_spellInfo->Id, damage, NORMAL_DAMAGE, 0,0,false,0);
    }
}

void Unit::SendHealToLog( Unit *pUnit, Spell *pSpell, uint32 heal )
{
    if( pUnit && pSpell )
    {
        SendSpellNonMeleeDamageLog(pUnit->GetGUID(), pSpell->m_spellInfo->Id, heal, NORMAL_DAMAGE, 0,0,false,0);
    }
}

