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
#include "Group.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "Formulas.h"
#include "Pet.h"
#include "Util.h"
#include "Totem.h"
#include "TemporarySummon.h"

#include <math.h>

float baseMoveSpeed[MAX_MOVE_TYPE] =
{
    2.5f,                                                   // MOVE_WALK
    7.0f,                                                   // MOVE_RUN
    1.25f,                                                  // MOVE_WALKBACK
    4.722222f,                                              // MOVE_SWIM
    4.5f,                                                   // MOVE_SWIMBACK
    3.141594f,                                              // MOVE_TURN
    7.0f,                                                   // MOVE_FLY
    4.5f                                                    // MOVE_FLYBACK
};

// auraTypes contains auras capable of proc'ing for attacker
static std::set<uint32> GenerateAttakerProcAuraTypes()
{
    static std::set<uint32> auraTypes;
    auraTypes.insert(SPELL_AURA_DUMMY);
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_SPELL);
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_DAMAGE);
    return auraTypes;
}

// auraTypes contains auras capable of proc'ing for attacker
static std::set<uint32> GenerateVictimProcAuraTypes()
{
    static std::set<uint32> auraTypes;
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_SPELL);
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_DAMAGE);
    auraTypes.insert(SPELL_AURA_DUMMY);
    auraTypes.insert(SPELL_AURA_MOD_PARRY_PERCENT);
    return auraTypes;
}

static std::set<uint32> attackerProcAuraTypes = GenerateAttakerProcAuraTypes();
static std::set<uint32> victimProcAuraTypes   = GenerateVictimProcAuraTypes();

// auraTypes contains auras capable of proc'ing for attacker and victim
static std::set<uint32> GenerateProcAuraTypes()
{
    static std::set<uint32> auraTypes = victimProcAuraTypes;
    auraTypes.insert(attackerProcAuraTypes.begin(),attackerProcAuraTypes.end());
    return auraTypes;
}

static std::set<uint32> procAuraTypes         = GenerateProcAuraTypes();

bool IsPassiveStackableSpell( uint32 spellId )
{
    if(!IsPassiveSpell(spellId))
        return false;

    SpellEntry const* spellProto = sSpellStore.LookupEntry(spellId);
    if(!spellProto)
        return false;

    for(int j = 0; j < 3; ++j)
    {
        if(std::find(procAuraTypes.begin(),procAuraTypes.end(),spellProto->EffectApplyAuraName[j])!=procAuraTypes.end())
            return false;
    }

    return true;
}

Unit::Unit( WorldObject *instantiator ) : WorldObject( instantiator )
{
    m_objectType |= TYPE_UNIT;
    m_objectTypeId = TYPEID_UNIT;
    m_updateFlag = (UPDATEFLAG_ALL | UPDATEFLAG_LIVING | UPDATEFLAG_HASPOSITION);   // 2.1.2 - 0x70

    m_attackTimer[BASE_ATTACK]   = 0;
    m_attackTimer[OFF_ATTACK]    = 0;
    m_attackTimer[RANGED_ATTACK] = 0;
    m_modAttackSpeedPct[BASE_ATTACK] = 1.0f;
    m_modAttackSpeedPct[OFF_ATTACK] = 1.0f;
    m_modAttackSpeedPct[RANGED_ATTACK] = 1.0f;

    m_state = 0;
    m_form = 0;
    m_deathState = ALIVE;
    m_currentSpell = NULL;
    m_oldSpell = NULL;
    m_currentMeleeSpell = NULL;
    m_addDmgOnce = 0;
    m_TotemSlot[0] = m_TotemSlot[1] = m_TotemSlot[2] = m_TotemSlot[3]  = 0;
    m_ObjectSlot[0] = m_ObjectSlot[1] = m_ObjectSlot[2] = m_ObjectSlot[3] = 0;
    //m_Aura = NULL;
    //m_AurasCheck = 2000;
    //m_removeAuraTimer = 4;
    //tmpAura = NULL;
    m_silenced = false;
    waterbreath = false;

    m_Visibility = VISIBILITY_ON;

    m_detectStealth = 0;
    m_detectInvisibility = 0;
    m_stealthvalue = 0;
    m_invisibilityvalue = 0;
    m_transform = 0;
    m_ShapeShiftForm = 0;
    m_createHealth =  0.0f;
    m_canModifyStats = false;

    for (int i = 0; i < TOTAL_AURAS; i++)
        m_AuraModifiers[i] = 0;
    for (int i = 0; i < IMMUNITY_MECHANIC; i++)
        m_spellImmune[i].clear();
    for (int i = 0; i < UNIT_MOD_END; i++)
    {
        m_auraModifiersGroup[i][BASE_VALUE] = 0.0f;
        m_auraModifiersGroup[i][BASE_PCT] = 1.0f;
        m_auraModifiersGroup[i][TOTAL_VALUE] = 0.0f;
        m_auraModifiersGroup[i][TOTAL_PCT] = 1.0f;
    }
    m_auraModifiersGroup[UNIT_MOD_DAMAGE_OFFHAND][TOTAL_PCT] = 0.5f; // implement 50% base damage from offhand

    for (int i = 0; i < 3; i++)
    {
        m_weaponDamage[i][MINDAMAGE] = BASE_MINDAMAGE;
        m_weaponDamage[i][MAXDAMAGE] = BASE_MAXDAMAGE;
    }
    for (int i = 0; i < MAX_STATS; i++)
        m_createStats[i] = 0.0f;
    for (int i = 0; i < MAX_POWERS; i++)
        m_createPowers[i] = 0.0f;
    
    m_attacking = NULL;
    m_modHitChance = 0;
    m_modSpellHitChance = 0;
    m_baseSpellCritChance = 5;
    m_modResilience = 0.0;
    m_modCastSpeedPct = 0;
    m_CombatTimer = 0;
    //m_victimThreat = 0.0f;
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        m_threatModifier[i] = 1.0f;
    m_isSorted = true;
    for (int i = 0; i < MAX_MOVE_TYPE; ++i)
        m_speed_rate[i] = 1.0f;

    m_removedAuras = 0;
    getThreatManager().setOwner(this);
}

Unit::~Unit()
{
    // destroy not finished spells
    delete m_oldSpell;
    delete m_currentSpell;
    delete m_currentMeleeSpell;

    // remove references to unit
    for(std::list<GameObject*>::iterator i = m_gameObj.begin(); i != m_gameObj.end();)
    {
        (*i)->SetOwnerGUID(0);
        (*i)->SetRespawnTime(0);
        (*i)->Delete();
        i = m_gameObj.erase(i);
    }

    RemoveAllDynObjects();
}

void Unit::RemoveAllDynObjects()
{
    while(!m_dynObjGUIDs.empty())
    {
        DynamicObject* dynObj = ObjectAccessor::Instance().GetDynamicObject(*this,*m_dynObjGUIDs.begin());
        if(dynObj)
            dynObj->Delete();
        m_dynObjGUIDs.erase(m_dynObjGUIDs.begin());
    }
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

    if (isInCombat() && GetTypeId() == TYPEID_PLAYER )      //update combat timer only for players
    {
        if(m_HostilRefManager.isEmpty())
        {
            // m_CombatTimer set at aura start and it will be freeze until aura removing
            if(!HasAuraType(SPELL_AURA_INTERRUPT_REGEN))
            {
                if ( m_CombatTimer <= p_time )
                    ClearInCombat();
                else
                    m_CombatTimer -= p_time;
            }
        }
    }

    if(uint32 base_att = getAttackTimer(BASE_ATTACK))
    {
        setAttackTimer(BASE_ATTACK, (p_time >= base_att ? 0 : base_att - p_time) );
    }
    if(GetHealth() < GetMaxHealth()*0.2)
        ModifyAuraState(AURA_STATE_HEALTHLESS, true);
    else ModifyAuraState(AURA_STATE_HEALTHLESS, false);
}

bool Unit::haveOffhandWeapon() const
{
    if(GetTypeId() == TYPEID_PLAYER)
    {
        Item *tmpitem = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

        return tmpitem && !tmpitem->IsBroken() && (tmpitem->GetProto()->InventoryType == INVTYPE_WEAPON || tmpitem->GetProto()->InventoryType == INVTYPE_WEAPONOFFHAND);
    }
    else
        return false;
}

void Unit::SendMoveToPacket(float x, float y, float z, bool run, uint32 transitTime)
{
    float dx = x - GetPositionX();
    float dy = y - GetPositionY();
    float dz = z - GetPositionZ();
    if (!transitTime)
    {
        float dist = ((dx*dx) + (dy*dy) + (dz*dz));
        if(dist<0)
            dist = 0;
        else
            dist = sqrt(dist);
        double speed = GetSpeed(run ? MOVE_RUN : MOVE_WALK);
        if(speed<=0)
            speed = 2.5f;
        speed *= 0.001f;
        transitTime = static_cast<uint32>(dist / speed + 0.5);
    }
    //float orientation = (float)atan2((double)dy, (double)dx);
    SendMonsterMove(x,y,z,0,run,transitTime);
}

void Unit::SendMonsterMove(float NewPosX, float NewPosY, float NewPosZ, uint8 type, bool Run, uint32 Time)
{
    WorldPacket data( SMSG_MONSTER_MOVE, (41 + GetPackGUID().size()) );
    data.append(GetPackGUID());

    // Point A, starting location
    data << GetPositionX() << GetPositionY() << GetPositionZ();
    // unknown field - unrelated to orientation
    // seems to increment about 1000 for every 1.7 seconds
    // for now, we'll just use mstime
    data << getMSTime();

    data << uint8(type);                                    // unknown
    switch(type)
    {
        case 0:                                             // normal packet
            break;
        case 1:                                             // stop packet
            SendMessageToSet( &data, true );
            return;
        case 3:                                             // not used currently
            data << uint64(0);                              // probably target guid
            break;
        case 4:                                             // not used currently
            data << float(0);                               // probably orientation
            break;
    }

    data << uint32(Run ? 0x00000100 : 0x00000000);          // flags (0x100 - running, 0x200 - taxi)
    /* Flags:
    512: Floating, moving without walking/running
    */
    data << Time;                                           // Time in between points
    data << uint32(1);                                      // 1 single waypoint
    data << NewPosX << NewPosY << NewPosZ;                  // the single waypoint Point B

    SendMessageToSet( &data, true );
}

void Unit::resetAttackTimer(WeaponAttackType type)
{
    if (GetTypeId() == TYPEID_PLAYER)
        m_attackTimer[type] = uint32(GetAttackTime(type) * m_modAttackSpeedPct[type]);
    else
        m_attackTimer[type] = uint32(BASE_ATTACK_TIME * m_modAttackSpeedPct[type]);
}

bool Unit::canReachWithAttack(Unit *pVictim) const
{
    assert(pVictim);
    float reach = GetFloatValue(UNIT_FIELD_COMBATREACH);
    if( reach <= 0.0f )
        reach = 1.0f;
    return IsWithinDistInMap(pVictim, reach);
}

void Unit::RemoveSpellsCausingAura(uint32 auraType)
{
    if (auraType >= TOTAL_AURAS) return;
    AuraList::iterator iter, next;
    for (iter = m_modAuras[auraType].begin(); iter != m_modAuras[auraType].end(); iter = next)
    {
        next = iter;
        ++next;

        if (*iter)
        {
            RemoveAurasDueToSpell((*iter)->GetId());
            if (!m_modAuras[auraType].empty())
                next = m_modAuras[auraType].begin();
            else
                return;
        }
    }
}

bool Unit::HasAuraType(uint32 auraType) const
{
    return (!m_modAuras[auraType].empty());
}

/* Called by DealDamage for auras that have a chance to be dispelled on damage taken. */
void Unit::RemoveSpellbyDamageTaken(uint32 auraType, uint32 damage)
{
    if(!HasAuraType(auraType))
        return;

    // The chance to dispell an aura depends on the damage taken with respect to the casters level.
    uint32 max_dmg = getLevel() > 8 ? 25 * getLevel() - 150 : 50;
    float chance = float(damage) / max_dmg * 100.0;
    if (roll_chance_f(chance))
        RemoveSpellsCausingAura(auraType);
}

void Unit::DealDamage(Unit *pVictim, uint32 damage, CleanDamage const* cleanDamage, DamageEffectType damagetype, uint8 damageSchool, SpellEntry const *spellProto, uint32 procFlag, bool durabilityLoss)
{
    if (!pVictim->isAlive() || pVictim->isInFlight()) return;

    // remove affects at any damage (including 0 damage)
    if(HasStealthAura())
        RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
    if(HasInvisibilityAura())
        RemoveSpellsCausingAura(SPELL_AURA_MOD_INVISIBILITY);
    // remove death simulation at damage
    if(hasUnitState(UNIT_STAT_DIED))
        RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    //Script Event damage Deal
    if( GetTypeId()== TYPEID_UNIT && ((Creature *)this)->AI())
        ((Creature *)this)->AI()->DamageDeal(pVictim, damage);
    //Script Event damage taken
    if( pVictim->GetTypeId()== TYPEID_UNIT && ((Creature *)pVictim)->AI() )
        ((Creature *)pVictim)->AI()->DamageTaken(this, damage);

    if(!damage) 
    {
        // Rage from damage received.
        if(cleanDamage && cleanDamage->damage && pVictim->GetTypeId() == TYPEID_PLAYER && (pVictim->getPowerType() == POWER_RAGE))
            ((Player*)pVictim)->RewardRage(cleanDamage->damage, 0, false);

        return;
    }

    pVictim->RemoveSpellbyDamageTaken(SPELL_AURA_MOD_FEAR, damage);
    // root type spells do not dispell the root effect
    if(!spellProto || spellProto->Mechanic != MECHANIC_ROOT)
        pVictim->RemoveSpellbyDamageTaken(SPELL_AURA_MOD_ROOT, damage);

    if(pVictim->GetTypeId() != TYPEID_PLAYER)
    {
        //pVictim->SetInFront(this);
        // no loot,xp,health if type 8 /critters/
        if ( ((Creature*)pVictim)->GetCreatureInfo()->type == CREATURE_TYPE_CRITTER)
        {
            pVictim->setDeathState(JUST_DIED);
            pVictim->SetHealth(0);
            pVictim->CombatStop(true);
            pVictim->DeleteThreatList();
            return;
        }
        if(!pVictim->isInCombat() && ((Creature*)pVictim)->AI())
            ((Creature*)pVictim)->AI()->AttackStart(this);
    }

    DEBUG_LOG("DealDamageStart");

    uint32 health = pVictim->GetHealth();
    sLog.outDetail("deal dmg:%d to health:%d ",damage,health);

    // duel ends when player has 1 or less hp
    bool duel_hasEnded = false;
    if(pVictim->GetTypeId() == TYPEID_PLAYER && ((Player*)pVictim)->duel && damage >= (health-1))
    {
        // prevent kill only if killed in duel and killed by opponent or opponent controlled creature
        if(((Player*)pVictim)->duel->opponent==this || ((Player*)pVictim)->duel->opponent->GetGUID() == GetOwnerGUID())
            damage = health-1;

        duel_hasEnded = true;
    }
    //Get in CombatState
    if(pVictim != this && damagetype != DOT)
    {
        SetInCombat();
        pVictim->SetInCombat();
    }

    // Rage from Damage made
    if( cleanDamage && this != pVictim && GetTypeId() == TYPEID_PLAYER && (getPowerType() == POWER_RAGE))
    {
        uint32 weaponSpeedHitFactor;

        switch(cleanDamage->attackType)
        {
            case BASE_ATTACK:
            {
                if(cleanDamage->hitOutCome == MELEE_HIT_CRIT)
                    weaponSpeedHitFactor = uint32(GetAttackTime(cleanDamage->attackType)/1000.0f * 7);
                else 
                    weaponSpeedHitFactor = uint32(GetAttackTime(cleanDamage->attackType)/1000.0f * 3.5f);

               ((Player*)this)->RewardRage(damage, weaponSpeedHitFactor, true);

               break;
            }
            case OFF_ATTACK:
            {
                if(cleanDamage->hitOutCome == MELEE_HIT_CRIT)
                    weaponSpeedHitFactor = uint32(GetAttackTime(cleanDamage->attackType)/1000.0f * 3.5f);
                else
                    weaponSpeedHitFactor = uint32(GetAttackTime(cleanDamage->attackType)/1000.0f * 1.75f);

                ((Player*)this)->RewardRage(damage, weaponSpeedHitFactor, true);

                break;
            }
            case RANGED_ATTACK:
                break;
        }
    }

    if (health <= damage)
    {

        DEBUG_LOG("DealDamage: victim just died");

        DEBUG_LOG("DealDamageAttackStop");
        AttackStop();
        pVictim->CombatStop(true);

        DEBUG_LOG("SET JUST_DIED");
        pVictim->setDeathState(JUST_DIED);

        DEBUG_LOG("DealDamageHealth1");
        pVictim->SetHealth(0);

        // Call KilledUnit for creatures
        if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->AI())
            ((Creature*)this)->AI()->KilledUnit(pVictim);

        // 10% durability loss on death
        // clean InHateListOf
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
        {
            if (GetTypeId() != TYPEID_PLAYER && durabilityLoss)
            {
                DEBUG_LOG("We are dead, loosing 10 percents durability");
                ((Player*)pVictim)->DurabilityLossAll(0.10);
                // durability lost message
                WorldPacket data(SMSG_DURABILITY_DAMAGE_DEATH, 0);
                ((Player*)pVictim)->GetSession()->SendPacket(&data);
            }
            pVictim->getHostilRefManager().deleteReferences();

            Pet *pet = pVictim->GetPet();
            if(pet)
            {
                pet->setDeathState(JUST_DIED);
                pet->CombatStop(true);
                pet->SetHealth(0);
                pet->addUnitState(UNIT_STAT_DIED);
                pet->getHostilRefManager().deleteReferences();
            }
        }
        else                                                // creature died
        {
            DEBUG_LOG("DealDamageNotPlayer");

            if(((Creature*)pVictim)->isPet())
                pVictim->getHostilRefManager().deleteReferences();
            else
            {
                pVictim->DeleteThreatList();
                pVictim->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            }
            // Call creature just died function
            if (((Creature*)pVictim)->AI())
                ((Creature*)pVictim)->AI()->JustDied(this);
        }

        //judge if GainXP, Pet kill like player kill,kill pet not like PvP
        bool PvP = false;
        Player *player = NULL;

        if(GetTypeId() == TYPEID_PLAYER)
        {
            player = (Player*)this;
            if(pVictim->GetTypeId() == TYPEID_PLAYER)
                PvP = true;
        }
        // FIXME: or charmed (can be player). Maybe must be check before GetTypeId() == TYPEID_PLAYER
        else if(GetCharmerOrOwnerGUID())                             // Pet or timed creature, etc
        {
            Unit* pet = this;
            Unit* owner = pet->GetCharmerOrOwner();

            if(owner && owner->GetTypeId() == TYPEID_PLAYER)
            {
                player = (Player*)owner;
                player->ClearInCombat();
                if(pVictim->GetTypeId() == TYPEID_PLAYER)
                    PvP = true;
            }

            if(pet->GetTypeId()==TYPEID_UNIT && ((Creature*)pet)->isPet())
            {
                uint32 petxp = MaNGOS::XP::BaseGain(getLevel(), pVictim->getLevel());
                ((Pet*)pet)->GivePetXP(petxp);
            }
        }

        // self or owner of pet
        if(player)
        {
            if(player!=pVictim)
            {
                // prepare data for near group iteration (PvP and !PvP cases
                uint32 xp = PvP ? 0 : MaNGOS::XP::Gain(player, pVictim);

                Group *pGroup = player->GetGroup();
                if(pGroup)
                {
                    uint32 count = pGroup->GetMemberCountForXPAtKill(pVictim);
                    if(count)
                    {
                        // skip in check PvP case (for speed, not used)
                        bool is_raid = PvP ? false : MapManager::Instance().GetBaseMap(player->GetMapId())->IsRaid() && pGroup->isRaidGroup();

                        for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                        {
                            Player *pGroupGuy = pGroup->GetMemberForXPAtKill(itr->getSource(),pVictim);
                            if(!pGroupGuy)
                                continue;

                            // honor can be in PvP and !PvP (racial leader) cases
                            pGroupGuy->RewardHonor(pVictim,count);

                            // xp and reputation only in !PvP case
                            if(!PvP)
                            {
                                // FIXME: xp/count for all in group at this moment, must be level dependent
                                float rate = 1.0f/count;

                                // if with raid in raid dungeon then all receive full reputation at kill
                                pGroupGuy->RewardReputation(pVictim,is_raid ? 1.0f : rate);

                                uint32 itr_xp = uint32(xp*rate);

                                pGroupGuy->GiveXP(itr_xp, pVictim);
                                if(Pet* pet = player->GetPet())
                                {
                                    pet->GivePetXP(itr_xp/2);
                                }

                                // normal creature (not pet/etc) can be only in !PvP case
                                if(pVictim->GetTypeId()==TYPEID_UNIT)
                                    pGroupGuy->KilledMonster(pVictim->GetEntry(), pVictim->GetGUID());
                            }
                        }
                    }
                }
                else // if (!pGroup)
                {
                    // honor can be in PvP and !PvP (racial leader) cases
                    player->RewardHonor(pVictim,1);

                    // xp and reputation only in !PvP case
                    if(!PvP)
                    {
                        player->RewardReputation(pVictim,1);
                        player->GiveXP(xp, pVictim);
                        if(Pet* pet = player->GetPet())
                        {
                            pet->GivePetXP(xp);
                        }

                        // normal creature (not pet/etc) can be only in !PvP case
                        if(pVictim->GetTypeId()==TYPEID_UNIT)
                            player->KilledMonster(pVictim->GetEntry(),pVictim->GetGUID());
                    }
                }

                if(xp)
                    ProcDamageAndSpell(pVictim,PROC_FLAG_KILL_XP_GIVER,PROC_FLAG_NONE);
            }
        }
        else                                                // if (player)
        {
            DEBUG_LOG("Monster kill Monster");
        }

        // last damage NOT from duel opponent or opponent controlled creature
        if(duel_hasEnded)
        {
            assert(pVictim->GetTypeId()==TYPEID_PLAYER);
            Player *he = (Player*)pVictim;

            assert(he->duel);

            CombatStop();                                   // for case killed by pet
            if(m_currentSpell)
                m_currentSpell->cancel();
            if(he->duel->opponent!=this)
            {
                he->duel->opponent->CombatStop();
                if(he->duel->opponent->m_currentSpell)
                    he->duel->opponent->m_currentSpell->cancel();
            }
            he->CombatStop();
            if(he->m_currentSpell)
                he->m_currentSpell->cancel();

            he->DuelComplete(0);
        }
    }
    else                                                    // if (health <= damage)
    {
        DEBUG_LOG("DealDamageAlive");

        pVictim->ModifyHealth(- (int32)damage);

        // Check if health is below 20% (apply damage before to prevent case when after ProcDamageAndSpell health < damage
        if(pVictim->GetHealth()*5 < pVictim->GetMaxHealth())
        {
            uint32 procVictim = PROC_FLAG_NONE;

            // if just dropped below 20% (for CheatDeath)
            if((pVictim->GetHealth()+damage)*5 > pVictim->GetMaxHealth())
                procVictim = PROC_FLAG_LOW_HEALTH;

            ProcDamageAndSpell(pVictim,PROC_FLAG_TARGET_LOW_HEALTH,procVictim);
        }

        if(damagetype != DOT)
        {
            //start melee attacks only after melee hit
            Attack(pVictim,(damagetype == DIRECT_DAMAGE));
        }

        if(pVictim->getTransForm() && pVictim->hasUnitState(UNIT_STAT_CONFUSED))
        {
            pVictim->RemoveAurasDueToSpell(pVictim->getTransForm());
            pVictim->setTransForm(0);
        }

        if (pVictim->GetTypeId() != TYPEID_PLAYER)
        {
            if(spellProto && IsDamageToThreatSpell(spellProto))
                damage *= 2;
            pVictim->AddThreat(this, damage, damageSchool, spellProto);
        }
        else                                                // victim is a player
        {
            // Rage from damage received
            if(this != pVictim && pVictim->GetTypeId() == TYPEID_PLAYER && pVictim->getPowerType() == POWER_RAGE)
            {
                uint32 rage_damage = damage + (cleanDamage ? cleanDamage->damage : 0);
                ((Player*)pVictim)->RewardRage(rage_damage, 0, false);
            }

            // random durability for items (HIT)
            if (urand(0,300) == 10)
            {
                DEBUG_LOG("HIT: We decrease durability with 5 percent");
                ((Player*)pVictim)->DurabilityLossAll(0.05);
            }
        }

        // TODO: Store auras by interrupt flag to speed this up.
        AuraMap& vAuras = pVictim->GetAuras();
        for (AuraMap::iterator i = vAuras.begin(), next; i != vAuras.end(); i = next)
        {
            const SpellEntry *se = i->second->GetSpellProto();
            next = i; next++;
            if( se->AuraInterruptFlags & AURA_INTERRUPT_FLAG_DAMAGE )
            {
                bool remove = true;
                if (se->procFlags & (1<<3))
                {
                    if (!roll_chance_i(se->procChance))
                        remove = false;
                }
                if (remove)
                {
                    pVictim->RemoveAurasDueToSpell(i->second->GetId());
                    // FIXME: this may cause the auras with proc chance to be rerolled several times
                    next = vAuras.begin();
                }
            }
        }

        if(pVictim->m_currentSpell && pVictim->GetTypeId() == TYPEID_PLAYER && damage
            && pVictim->m_currentSpell->getState() == SPELL_STATE_CASTING)
        {
            if (damagetype != NODAMAGE)
            {
                uint32 channelInterruptFlags = pVictim->m_currentSpell->m_spellInfo->ChannelInterruptFlags;
                if( channelInterruptFlags & CHANNEL_FLAG_DELAY )
                {
                    sLog.outDetail("Spell %u delayed (%d) at damage!",pVictim->m_currentSpell->m_spellInfo->Id,(int32)(0.25f * GetDuration(pVictim->m_currentSpell->m_spellInfo)));
                    pVictim->m_currentSpell->DelayedChannel((int32)(0.25f * GetDuration(pVictim->m_currentSpell->m_spellInfo)));
                }
                else if( (channelInterruptFlags & (CHANNEL_FLAG_DAMAGE | CHANNEL_FLAG_DAMAGE2)) )
                {
                    sLog.outDetail("Spell %u canceled at damage!",pVictim->m_currentSpell->m_spellInfo->Id);
                    pVictim->m_currentSpell->cancel();
                }
            }
        }

        // last damage from duel opponent
        if(duel_hasEnded)
        {
            assert(pVictim->GetTypeId()==TYPEID_PLAYER);
            Player *he = (Player*)pVictim;

            assert(he->duel);

            he->ModifyHealth(1);
            CombatStop();                                   // for case killed by pet
            if(he->duel->opponent!=this)
                he->duel->opponent->CombatStop();
            he->CombatStop();

            he->CastSpell(he, 7267, true);                  // beg
            he->DuelComplete(1);
        }
    }

    DEBUG_LOG("DealDamageEnd");
}

void Unit::CastSpell(Unit* Victim, uint32 spellId, bool triggered, Item *castItem, Aura* triggredByAura, uint64 originalCaster)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId );

    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    CastSpell(Victim,spellInfo,triggered,castItem,triggredByAura, originalCaster);
}

void Unit::CastSpell(Unit* Victim,SpellEntry const *spellInfo, bool triggered, Item *castItem, Aura* triggredByAura, uint64 originalCaster)
{
    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell ");
        return;
    }

    if (castItem)
        DEBUG_LOG("WORLD: cast Item spellId - %i", spellInfo->Id);

    Spell *spell = new Spell(this, spellInfo, triggered, triggredByAura,originalCaster);

    SpellCastTargets targets;
    targets.setUnitTarget( Victim );
    spell->m_CastItem = castItem;
    spell->prepare(&targets);
    m_canMove = false;
    if (triggered) delete spell;                            // triggered spell not self deleted
}

void Unit::CastCustomSpell(Unit* Victim,uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item *castItem, Aura* triggredByAura, uint64 originalCaster)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId );

    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    CastCustomSpell(Victim,spellInfo,bp0,bp1,bp2,triggered,castItem,triggredByAura, originalCaster);
}

void Unit::CastCustomSpell(Unit* Victim,SpellEntry const *spellInfo, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item *castItem, Aura* triggredByAura, uint64 originalCaster)
{
    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell ");
        return;
    }

    if (castItem)
        DEBUG_LOG("WORLD: cast Item spellId - %i", spellInfo->Id);

    Spell *spell = new Spell(this, spellInfo, triggered, triggredByAura,originalCaster);

    if(bp0)
        spell->m_currentBasePoints[0] = *bp0;

    if(bp1)
        spell->m_currentBasePoints[1] = *bp1;

    if(bp2)
        spell->m_currentBasePoints[2] = *bp2;

    SpellCastTargets targets;
    targets.setUnitTarget( Victim );
    spell->m_CastItem = castItem;
    spell->prepare(&targets);
    m_canMove = false;
    if (triggered) delete spell;                            // triggered spell not self deleted
}

void Unit::DealDamageBySchool(Unit *pVictim, SpellEntry const *spellInfo, uint32 *damage, CleanDamage *cleanDamage, bool *crit, bool isTriggeredSpell)
{

    // TODO this in only generic way, check for exceptions
    DEBUG_LOG("DealDamageBySchool (BEFORE) SCHOOL %u >> DMG:%u", spellInfo->School, *damage);

    // Per-school calc
    switch (spellInfo->School)
    {
        // Physical damage school
        case SPELL_SCHOOL_NORMAL:

            // Calculate physical outcome
            MeleeHitOutcome outcome;
            outcome = RollPhysicalOutcomeAgainst(pVictim, BASE_ATTACK, spellInfo);
            
            //Used to store the Hit Outcome
            cleanDamage->hitOutCome = outcome;

            // Return miss first (sends miss message)
            if(outcome == MELEE_HIT_MISS)
            {
                SendAttackStateUpdate(HITINFO_MISS, pVictim, 1, spellInfo->School, 0, 0,0,1,0);
                *damage = 0;
                if(GetTypeId()== TYPEID_PLAYER)
                    ((Player*)this)->UpdateWeaponSkill(BASE_ATTACK);
                return;
            }

            //  Hitinfo, Victimstate
            uint32 hitInfo, victimState;
            hitInfo = HITINFO_NORMALSWING;

            //Calculate armor mitigation
            uint32 damageAfterArmor;
            damageAfterArmor = CalcArmorReducedDamage(pVictim, *damage);
            cleanDamage->damage += *damage - damageAfterArmor;
            *damage = damageAfterArmor;

            // Classify outcome
            switch (outcome)
            {
                case MELEE_HIT_CRIT:
                {
                    *damage *= 2;
                    // Resilience - reduce crit damage by 2x%
                    uint32 resilienceReduction = uint32(pVictim->m_modResilience * 2/100 * (*damage));
                    cleanDamage->damage += resilienceReduction;
                    *damage -=  resilienceReduction;
                    *crit = true;
                    hitInfo |= HITINFO_CRITICALHIT;
                    break;
                }
                case MELEE_HIT_PARRY:
                {
                    cleanDamage->damage += *damage; // To Help Calculate Rage
                    *damage = 0;
                    victimState = VICTIMSTATE_PARRY;

                    // Counter-attack ( explained in Unit::DoAttackDamage() )
                    {
                        // Get attack timers
                        float offtime  = float(pVictim->getAttackTimer(OFF_ATTACK));
                        float basetime = float(pVictim->getAttackTimer(BASE_ATTACK));

                        // Reduce attack time
                        if (pVictim->haveOffhandWeapon() && offtime < basetime)
                        {
                            float percent20 = pVictim->GetAttackTime(OFF_ATTACK) * 0.20;
                            float percent60 = 3 * percent20;
                            if(offtime > percent20 && offtime <= percent60)
                            {
                                pVictim->setAttackTimer(OFF_ATTACK, uint32(percent20));
                            }
                            else if(offtime > percent60)
                            {
                                offtime -= 2 * percent20;
                                pVictim->setAttackTimer(OFF_ATTACK, uint32(offtime));
                            }
                        }
                        else
                        {
                            float percent20 = pVictim->GetAttackTime(BASE_ATTACK) * 0.20;
                            float percent60 = 3 * percent20;
                            if(basetime > percent20 && basetime <= percent60)
                            {
                                pVictim->setAttackTimer(BASE_ATTACK, uint32(percent20));
                            }
                            else if(basetime > percent60)
                            {
                                basetime -= 2 * percent20;
                                pVictim->setAttackTimer(BASE_ATTACK, uint32(basetime));
                            }
                        }
                    }

                    // Update victim defense ?
                    if(pVictim->GetTypeId() == TYPEID_PLAYER)
                        ((Player*)pVictim)->UpdateDefense();

                    // Set parry flags
                    pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);
                    pVictim->ModifyAuraState(AURA_STATE_PARRY, true);
                    break;
                }
                case MELEE_HIT_DODGE:
                {
                    if(pVictim->GetTypeId() == TYPEID_PLAYER)
                        ((Player*)pVictim)->UpdateDefense();

                    cleanDamage->damage += *damage; // To Help Calculate Rage
                    *damage = 0;
                    hitInfo |= HITINFO_SWINGNOHITSOUND;
                    victimState = VICTIMSTATE_DODGE;
                    break;
                }
                case MELEE_HIT_BLOCK:
                {
                    uint32 blocked_amount;
                    blocked_amount = uint32(pVictim->GetShieldBlockValue());
                    if (blocked_amount >= *damage)
                    {
                        hitInfo |= HITINFO_SWINGNOHITSOUND;
                        victimState = VICTIMSTATE_BLOCKS;
                        cleanDamage->damage += *damage; // To Help Calculate Rage
                        *damage = 0;
                    }
                    else
                    {
                        cleanDamage->damage += blocked_amount; // To Help Calculate Rage
                        *damage = *damage - blocked_amount;
                    }
                    break;

                }
                case MELEE_HIT_MISS:
                case MELEE_HIT_GLANCING:
                case MELEE_HIT_CRUSHING:
                case MELEE_HIT_NORMAL:
                    break;
            }

            // Update attack state
            SendAttackStateUpdate(victimState ? hitInfo|victimState : hitInfo, pVictim, 1, spellInfo->School, 0, 0,0,1,0);

            break;

        // Other schools
        case SPELL_SCHOOL_HOLY:
        case SPELL_SCHOOL_FIRE:
        case SPELL_SCHOOL_NATURE:
        case SPELL_SCHOOL_FROST:
        case SPELL_SCHOOL_SHADOW:
        case SPELL_SCHOOL_ARCANE:

            //Spell miss (sends resist message)
            if(SpellMissChanceCalc(pVictim) > urand(0,10000))
            {
                cleanDamage->damage = 0;
                *damage = 0;
                ProcDamageAndSpell(pVictim, PROC_FLAG_TARGET_RESISTS, PROC_FLAG_RESIST_SPELL, 0, spellInfo,isTriggeredSpell);
                SendAttackStateUpdate(HITINFO_RESIST|HITINFO_SWINGNOHITSOUND, pVictim, 1, spellInfo->School, 0, 0,0,1,0);
                return;
            }

            // Calculate damage bonus
            *damage = SpellDamageBonus(pVictim, spellInfo, *damage, SPELL_DIRECT_DAMAGE);

            // Calculate critical bonus
            *crit = SpellCriticalBonus(spellInfo, (int32*)damage, pVictim);
            cleanDamage->hitOutCome = MELEE_HIT_CRIT;
            break;
    }

    // TODO this in only generic way, check for exceptions
    DEBUG_LOG("DealDamageBySchool (AFTER) SCHOOL %u >> DMG:%u", spellInfo->School, *damage);

}

void Unit::SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage, bool isTriggeredSpell, bool useSpellDamage)
{
    if(!this || !pVictim)
        return;
    if(!this->isAlive() || !pVictim->isAlive())
        return;

    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellID);
    if(!spellInfo)
        return;

    CleanDamage cleanDamage = CleanDamage(0, BASE_ATTACK, MELEE_HIT_NORMAL);
    bool crit = false;

    if (useSpellDamage)
        DealDamageBySchool(pVictim, spellInfo, &damage, &cleanDamage, &crit, isTriggeredSpell);

    // If we actually dealt some damage
    if(damage > 0)
    {
        // Calculate absorb & resists
        uint32 absorb = 0;
        uint32 resist = 0;

        CalcAbsorbResist(pVictim,spellInfo->School, damage, &absorb, &resist);

        // Only send absorbed message if we actually absorbed some damage
        if(damage > 0)
        {
            // Handle absorb & resists
            if(damage <= absorb + resist && absorb)
            {
                SendAttackStateUpdate(HITINFO_ABSORB|HITINFO_SWINGNOHITSOUND, pVictim, 1, spellInfo->School,damage, absorb,resist,1,0);
                return;
            }
            else if(damage <= resist)   // If we didn't fully absorb check if we fully resisted
            {
                ProcDamageAndSpell(pVictim, PROC_FLAG_TARGET_RESISTS, PROC_FLAG_RESIST_SPELL, 0, spellInfo,isTriggeredSpell);
                SendAttackStateUpdate(HITINFO_RESIST|HITINFO_SWINGNOHITSOUND, pVictim, 1, spellInfo->School, damage, absorb,resist,1,0);
                return;
            }
        }

        // Send damage log
        sLog.outDetail("SpellNonMeleeDamageLog: %u (TypeId: %u) attacked %u (TypeId: %u) for %u dmg inflicted by %u,absorb is %u,resist is %u",
            GetGUIDLow(), GetTypeId(), pVictim->GetGUIDLow(), pVictim->GetTypeId(), damage, spellID, absorb,resist);
        SendSpellNonMeleeDamageLog(pVictim, spellID, damage, spellInfo->School, absorb, resist, false, 0, crit);

        // Deal damage done
        DealDamage(pVictim, (damage-absorb-resist), &cleanDamage, SPELL_DIRECT_DAMAGE, spellInfo->School, spellInfo, 0, true);

        // Procflags
        uint32 procAttacker = PROC_FLAG_HIT_SPELL;
        uint32 procVictim   = (PROC_FLAG_STRUCK_SPELL|PROC_FLAG_TAKE_DAMAGE);

        if (crit)
        {
            procAttacker |= PROC_FLAG_CRIT_SPELL;
            procVictim   |= PROC_FLAG_STRUCK_CRIT_SPELL;
        }

        ProcDamageAndSpell(pVictim, procAttacker, procVictim, (damage-absorb-resist), spellInfo, isTriggeredSpell);
    }
    //Check for rage
    else if(cleanDamage.damage) 
        // Rage from damage received.
        if(pVictim->GetTypeId() == TYPEID_PLAYER && (pVictim->getPowerType() == POWER_RAGE))
            ((Player*)pVictim)->RewardRage(cleanDamage.damage, 0, false);
}

void Unit::PeriodicAuraLog(Unit *pVictim, SpellEntry const *spellProto, Modifier *mod, uint8 effect_idx)
{
    uint32 procFlag = 0;
    if(!this || !pVictim || !isAlive() || !pVictim->isAlive())
    {
        return;
    }
    uint32 absorb=0;
    uint32 resist=0;
    CleanDamage cleanDamage =  CleanDamage(0, BASE_ATTACK, MELEE_HIT_NORMAL );
    uint32 pdamage = mod->m_amount;

    if(mod->m_auraname != SPELL_AURA_PERIODIC_HEAL && mod->m_auraname != SPELL_AURA_OBS_MOD_HEALTH)
    {
        //Calculate armor mitigation if it is a physical spell
        if (spellProto->School == 0)
        {
            uint32 pdamageReductedArmor = CalcArmorReducedDamage(pVictim, pdamage);
            cleanDamage.damage += pdamage - pdamageReductedArmor;
            pdamage = pdamageReductedArmor;
        }

        CalcAbsorbResist(pVictim, spellProto->School, pdamage, &absorb, &resist);
    }

    sLog.outDetail("PeriodicAuraLog: %u (TypeId: %u) attacked %u (TypeId: %u) for %u dmg inflicted by %u abs is %u",
        GetGUIDLow(), GetTypeId(), pVictim->GetGUIDLow(), pVictim->GetTypeId(), pdamage, spellProto->Id,absorb);

    switch(mod->m_auraname)
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        {
            pdamage = SpellDamageBonus(pVictim,spellProto,pdamage,DOT);

            if(mod->m_auraname == SPELL_AURA_PERIODIC_DAMAGE_PERCENT)
                pdamage = GetHealth()*(100+mod->m_amount)/100;

            WorldPacket data(SMSG_PERIODICAURALOG, (21+16));        // we guess size
            data.append(pVictim->GetPackGUID());
            data.append(GetPackGUID());
            data << uint32(spellProto->Id);
            data << uint32(1);
            data << uint32(mod->m_auraname);
            data << (uint32)pdamage;
            data << (uint32)spellProto->School;
            data << (uint32)absorb;
            data << (uint32)resist;
            SendMessageToSet(&data,true);

            DealDamage(pVictim, (pdamage <= absorb+resist) ? 0 : (pdamage-absorb-resist), &cleanDamage, DOT, spellProto->School, spellProto, procFlag, true);
            ProcDamageAndSpell(pVictim, 0, PROC_FLAG_TAKE_DAMAGE, (pdamage <= absorb+resist) ? 0 : (pdamage-absorb-resist), spellProto);
            break;
        }
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_OBS_MOD_HEALTH:
        {
            pdamage = SpellHealingBonus(spellProto, pdamage, DOT, pVictim);

            WorldPacket data(SMSG_PERIODICAURALOG, (21+16));        // we guess size
            data.append(pVictim->GetPackGUID());
            data.append(GetPackGUID());
            data << uint32(spellProto->Id);
            data << uint32(1);
            data << uint32(mod->m_auraname);
            data << (uint32)pdamage;
            SendMessageToSet(&data,true);

            int32 gain = pVictim->ModifyHealth(pdamage);
            pVictim->getHostilRefManager().threatAssist(this, float(gain) * 0.5f, spellProto->School, spellProto);

            // heal for caster damage
            if(pVictim!=this && spellProto->SpellVisual==163)
            {
                uint32 dmg = spellProto->manaPerSecond;
                if(GetHealth() <= dmg && GetTypeId()==TYPEID_PLAYER)
                {
                    RemoveAurasDueToSpell(spellProto->Id);
                    if(m_currentSpell)
                    {
                        if(m_currentSpell->IsChanneledSpell())
                            m_currentSpell->SendChannelUpdate(0);
                        m_currentSpell->finish();
                    }
                }
                else
                {
                    SendSpellNonMeleeDamageLog(this, spellProto->Id, gain, spellProto->School, 0, 0, false, 0, false);
                    DealDamage(this, gain, &cleanDamage, NODAMAGE, spellProto->School, spellProto, PROC_FLAG_HEAL, true);
                }
            }

            if(mod->m_auraname == SPELL_AURA_PERIODIC_HEAL && pVictim != this)
                ProcDamageAndSpell(pVictim, PROC_FLAG_HEAL, PROC_FLAG_HEALED, pdamage, spellProto);
            break;
        }
        case SPELL_AURA_PERIODIC_LEECH:
        {
            float multiplier = spellProto->EffectMultipleValue[effect_idx] > 0 ? spellProto->EffectMultipleValue[effect_idx] : 1;
            uint32 pdamage = mod->m_amount;

            pdamage = SpellDamageBonus(pVictim,spellProto,pdamage,DOT);

            if(pVictim->GetHealth() < pdamage)
                pdamage = uint32(pVictim->GetHealth());

            SendSpellNonMeleeDamageLog(pVictim, spellProto->Id, pdamage, spellProto->School, absorb, resist, false, 0);
            DealDamage(pVictim, (pdamage <= absorb+resist) ? 0 : (pdamage-absorb-resist), &cleanDamage, DOT, spellProto->School, spellProto, procFlag, false);
            ProcDamageAndSpell(pVictim, PROC_FLAG_HIT_SPELL, PROC_FLAG_TAKE_DAMAGE, (pdamage <= absorb+resist) ? 0 : (pdamage-absorb-resist), spellProto);
            if (!pVictim->isAlive() && m_currentSpell)
                if (m_currentSpell->m_spellInfo)
                    if (m_currentSpell->m_spellInfo->Id == spellProto->Id)
                        m_currentSpell->cancel();

            int32 gain = ModifyHealth(int32(pdamage * multiplier));
            getHostilRefManager().threatAssist(this, float(gain) * 0.5f, spellProto->School, spellProto);

            if(GetTypeId() == TYPEID_PLAYER)
                SendHealSpellOnPlayer(this, spellProto->Id, uint32(pdamage * multiplier));
            break;
        }
        case SPELL_AURA_PERIODIC_MANA_LEECH:
        {
            if(mod->m_miscvalue < 0 || mod->m_miscvalue > 4)
                break;

            Powers power = Powers(mod->m_miscvalue);

            int32 drain_amount = pVictim->GetPower(power) > pdamage ? pdamage : pVictim->GetPower(power);

            pVictim->ModifyPower(power, -drain_amount);

            float gain_multiplier = GetMaxPower(power) > 0 ? spellProto->EffectMultipleValue[effect_idx] : 0;

            WorldPacket data(SMSG_PERIODICAURALOG, (21+16));    // we guess size
            data.append(pVictim->GetPackGUID());
            data.append(GetPackGUID());
            data << uint32(spellProto->Id);
            data << uint32(1);
            data << uint32(mod->m_auraname);
            data << (uint32)power;                              // power type
            data << (uint32)drain_amount;
            data << (float)gain_multiplier;
            SendMessageToSet(&data,true);

            int32 gain_amount = int32(drain_amount*gain_multiplier);

            if(gain_amount)
            {
                int32 gain = ModifyPower(power,gain_amount);
                pVictim->AddThreat(this, float(gain) * 0.5f, spellProto->School, spellProto);
            }
            break;
        }
        case SPELL_AURA_PERIODIC_ENERGIZE:
        {
            if(mod->m_miscvalue < 0 || mod->m_miscvalue > 4)
                break;

            Powers power = Powers(mod->m_miscvalue);

            if(pVictim->GetMaxPower(power) == 0)
                break;

            WorldPacket data(SMSG_PERIODICAURALOG, (21+16));    // we guess size
            data.append(pVictim->GetPackGUID());
            data.append(GetPackGUID());
            data << uint32(spellProto->Id);
            data << uint32(1);
            data << uint32(mod->m_auraname);
            data << (uint32)power;                              // power type
            data << (uint32)mod->m_amount;
            SendMessageToSet(&data,true);

            int32 gain = pVictim->ModifyPower(power,mod->m_amount);
            pVictim->getHostilRefManager().threatAssist(this, float(gain) * 0.5f, spellProto->School, spellProto);
            break;
        }
        case SPELL_AURA_OBS_MOD_MANA:
        {
            if(GetMaxPower(POWER_MANA) == 0)
                break;

            WorldPacket data(SMSG_PERIODICAURALOG, (21+16));        // we guess size
            data.append(pVictim->GetPackGUID());
            data.append(GetPackGUID());
            data << uint32(spellProto->Id);
            data << uint32(1);
            data << uint32(mod->m_auraname);
            data << (uint32)mod->m_amount;
            data << (uint32)0;                  // ?
            SendMessageToSet(&data,true);

            int32 gain = ModifyPower(POWER_MANA, mod->m_amount);
            getHostilRefManager().threatAssist(this, float(gain) * 0.5f, spellProto->School, spellProto);
            break;
        }
    }
}

void Unit::HandleEmoteCommand(uint32 anim_id)
{
    WorldPacket data( SMSG_EMOTE, 12 );
    data << anim_id << GetGUID();
    WPAssert(data.size() == 12);

    SendMessageToSet(&data, true);
}

uint32 Unit::CalcArmorReducedDamage(Unit* pVictim, const uint32 damage)
{
    uint32 newdamage = 0;
    float armor = pVictim->GetArmor();

    float tmpvalue = 0.0;
    if(getLevel() <= 59)                                    //Level 1-59
        tmpvalue = armor / (armor + 400.0 + 85.0 * getLevel());
    else if(getLevel() < 70)                                //Level 60-69
        tmpvalue = armor / (armor - 22167.5 + 467.5 * getLevel());
    else                                                    //Level 70+
        tmpvalue = armor / (armor + 10557.5);

    if(tmpvalue < 0.0)
        tmpvalue = 0.0;
    if(tmpvalue > 0.75)
        tmpvalue = 0.75;
    newdamage = uint32(damage - (damage * tmpvalue));

    return (newdamage > 1) ? newdamage : 1;
}

void Unit::CalcAbsorbResist(Unit *pVictim,uint32 School, const uint32 damage, uint32 *absorb, uint32 *resist)
{
    if(!pVictim || !pVictim->isAlive() || !damage)
        return;

    // Magic damage, check for resists
    if (School != SPELL_SCHOOL_NORMAL)
    {
        int32 tmpvalue2 = pVictim->GetResistance(SpellSchools(School));
        AuraList const& mModTargetRes = GetAurasByType(SPELL_AURA_MOD_TARGET_RESISTANCE);
        for(AuraList::const_iterator i = mModTargetRes.begin(); i != mModTargetRes.end(); ++i)
            if ((*i)->GetModifier()->m_miscvalue & (1 << School))
                tmpvalue2 += (*i)->GetModifier()->m_amount;
        if (tmpvalue2 < 0) tmpvalue2 = 0;
        *resist += uint32(damage*tmpvalue2*0.0025*pVictim->getLevel()/getLevel());
        if(*resist > damage)
            *resist = damage;
    }
    else 
        *resist = 0;

    int32 RemainingDamage = damage - *resist;
    int32 currentAbsorb, manaReduction, maxAbsorb;
    float manaMultiplier;

    if (School == SPELL_SCHOOL_NORMAL)
    {
        AuraList const& vManaShield = pVictim->GetAurasByType(SPELL_AURA_MANA_SHIELD);
        for(AuraList::const_iterator i = vManaShield.begin(), next; i != vManaShield.end() && RemainingDamage >= 0; i = next)
        {
            next = i; next++;
            if (RemainingDamage - (*i)->m_absorbDmg >= 0)
                currentAbsorb = (*i)->m_absorbDmg;
            else
                currentAbsorb = RemainingDamage;

            manaMultiplier = (*i)->GetSpellProto()->EffectMultipleValue[(*i)->GetEffIndex()];
            maxAbsorb = int32(pVictim->GetPower(POWER_MANA) / manaMultiplier);
            if (currentAbsorb > maxAbsorb)
                currentAbsorb = maxAbsorb;

            (*i)->m_absorbDmg -= currentAbsorb;
            if((*i)->m_absorbDmg <= 0)
            {
                pVictim->RemoveAurasDueToSpell((*i)->GetId());
                next = vManaShield.begin();
            }

            manaReduction = int32(currentAbsorb * manaMultiplier);
            pVictim->ApplyPowerMod(POWER_MANA, manaReduction, false);

            RemainingDamage -= currentAbsorb;
        }
    }

    AuraList const& vSchoolAbsorb = pVictim->GetAurasByType(SPELL_AURA_SCHOOL_ABSORB);
    for(AuraList::const_iterator i = vSchoolAbsorb.begin(), next; i != vSchoolAbsorb.end() && RemainingDamage >= 0; i = next)
    {
        next = i; next++;
        if ((*i)->GetModifier()->m_miscvalue & int32(1<<School))
        {
            if (RemainingDamage - (*i)->m_absorbDmg >= 0)
            {
                currentAbsorb = (*i)->m_absorbDmg;
                pVictim->RemoveAurasDueToSpell((*i)->GetId());
                next = vSchoolAbsorb.begin();
            }
            else
            {
                currentAbsorb = RemainingDamage;
                (*i)->m_absorbDmg -= RemainingDamage;
            }

            RemainingDamage -= currentAbsorb;
        }
    }

    *absorb = damage - RemainingDamage - *resist;
}

void Unit::DoAttackDamage (Unit *pVictim, uint32 *damage, CleanDamage *cleanDamage, uint32 *blocked_amount, uint32 *damageType, uint32 *hitInfo, uint32 *victimState, uint32 *absorbDamage, uint32 *resistDamage, WeaponAttackType attType, SpellEntry const *spellCasted, bool isTriggeredSpell)
{
    pVictim->ModifyAuraState(AURA_STATE_PARRY, false);
    pVictim->ModifyAuraState(AURA_STATE_DEFENSE, false);
    ModifyAuraState(AURA_STATE_CRIT, false);

    MeleeHitOutcome outcome;

    // If is casted Melee spell, calculate like physical
    if(!spellCasted)
        outcome = RollMeleeOutcomeAgainst (pVictim, attType);
    else
        outcome = RollPhysicalOutcomeAgainst (pVictim, attType, spellCasted);

    if (outcome == MELEE_HIT_MISS)
    {
        *hitInfo |= HITINFO_MISS;
        *damage = 0;
        cleanDamage->damage = 0;
        if(GetTypeId()== TYPEID_PLAYER)
            ((Player*)this)->UpdateWeaponSkill(attType);
        return;
    }

    *damage += CalculateDamage (attType);

    //Calculate the damage after armor mitigation if SPELL_SCHOOL_NORMAL
    if (*damageType == SPELL_SCHOOL_NORMAL)
    {
        uint32 damageAfterArmor = CalcArmorReducedDamage(pVictim, *damage);
        cleanDamage->damage += *damage - damageAfterArmor;
        *damage = damageAfterArmor;
    }
    // Instant Attacks (Spellmods) 
    // TODO: AP bonus related to mainhand weapon 
    if(spellCasted) 
        if(GetTypeId()== TYPEID_PLAYER) 
            ((Player*)this)->ApplySpellMod(spellCasted->Id, SPELLMOD_DAMAGE, *damage); 

    if(GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() != TYPEID_PLAYER && ((Creature*)pVictim)->GetCreatureInfo()->type != CREATURE_TYPE_CRITTER )
        ((Player*)this)->UpdateCombatSkills(pVictim, attType, outcome, false);

    if(GetTypeId() != TYPEID_PLAYER && pVictim->GetTypeId() == TYPEID_PLAYER)
        ((Player*)pVictim)->UpdateCombatSkills(this, attType, outcome, true);

    switch (outcome)
    {
        case MELEE_HIT_CRIT:
            //*hitInfo = 0xEA;
            // 0xEA
            *hitInfo  = HITINFO_CRITICALHIT | HITINFO_NORMALSWING2 | 0x8;

            // Crit bonus calc
            uint32 crit_bonus;
            crit_bonus = *damage;

            // Apply crit_damage bonus for melee spells
            if (GetTypeId() == TYPEID_PLAYER && spellCasted)
            {
                ((Player*)this)->ApplySpellMod(spellCasted->Id, SPELLMOD_CRIT_DAMAGE_BONUS, crit_bonus);
            }

            *damage += crit_bonus;
            
            // Resilience - reduce crit damage by 2x%
            uint32 resilienceReduction; 
            resilienceReduction = uint32(pVictim->m_modResilience * 2/100 * (*damage));
            *damage -= resilienceReduction;
            cleanDamage->damage += resilienceReduction;

            if(GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() != TYPEID_PLAYER && ((Creature*)pVictim)->GetCreatureInfo()->type != CREATURE_TYPE_CRITTER )
                ((Player*)this)->UpdateWeaponSkill(attType);

            ModifyAuraState(AURA_STATE_CRIT, true);

            pVictim->HandleEmoteCommand(EMOTE_ONESHOT_WOUNDCRITICAL);
            break;

        case MELEE_HIT_PARRY:
            if(attType == RANGED_ATTACK)                    //range attack - no parry
                break;

            cleanDamage->damage += *damage;
            *damage = 0;
            *victimState = VICTIMSTATE_PARRY;

            // instant (maybe with small delay) counter attack
            {
                float offtime  = float(pVictim->getAttackTimer(OFF_ATTACK));
                float basetime = float(pVictim->getAttackTimer(BASE_ATTACK));

                // after parry nearest next attack time will reduced at %40 from full attack time.
                // The delay cannot be reduced to less than 20% of your weapon’s base swing delay.
                if (pVictim->haveOffhandWeapon() && offtime < basetime)
                {
                    float percent20 = pVictim->GetAttackTime(OFF_ATTACK)*0.20;
                    float percent60 = 3*percent20;
                    // set to 20% if in range 20%...20+40% of full time
                    if(offtime > percent20 && offtime <= percent60)
                    {
                        pVictim->setAttackTimer(OFF_ATTACK,uint32(percent20));
                    }
                    // decrease at %40 from full time
                    else if(offtime > percent60)
                    {
                        offtime -= 2*percent20;
                        pVictim->setAttackTimer(OFF_ATTACK,uint32(offtime));
                    }
                    // ELSE not changed
                }
                else
                {
                    float percent20 = pVictim->GetAttackTime(BASE_ATTACK)*0.20;
                    float percent60 = 3*percent20;
                    // set to 20% if in range 20%...20+40% of full time
                    if(basetime > percent20 && basetime <= percent60)
                    {
                        pVictim->setAttackTimer(BASE_ATTACK,uint32(percent20));
                    }
                    // decrease at %40 from full time
                    else if(basetime > percent60)
                    {
                        basetime -= 2*percent20;
                        pVictim->setAttackTimer(BASE_ATTACK,uint32(basetime));
                    }
                    // ELSE not changed
                }
            }

            if(pVictim->GetTypeId() == TYPEID_PLAYER)
                ((Player*)pVictim)->UpdateDefense();

            pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);
            pVictim->ModifyAuraState(AURA_STATE_PARRY,true);
            if (pVictim->getClass() != CLASS_HUNTER) // Mongoose Bite
                pVictim->ModifyAuraState(AURA_STATE_DEFENSE, true);

            CastMeleeProcDamageAndSpell(pVictim, 0, attType, outcome, spellCasted, isTriggeredSpell);
            return;

        case MELEE_HIT_DODGE:
            if(attType == RANGED_ATTACK)                    //range attack - no dodge
                break;
            cleanDamage->damage += *damage;
            *damage = 0;
            *victimState = VICTIMSTATE_DODGE;

            if(pVictim->GetTypeId() == TYPEID_PLAYER)
                ((Player*)pVictim)->UpdateDefense();

            pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);

            if (pVictim->getClass() != CLASS_ROGUE) // Riposte
                pVictim->ModifyAuraState(AURA_STATE_DEFENSE, true);

            CastMeleeProcDamageAndSpell(pVictim, 0, attType, outcome, spellCasted, isTriggeredSpell);
            return;

        case MELEE_HIT_BLOCK:
            *blocked_amount = uint32(pVictim->GetShieldBlockValue() + (pVictim->GetStat(STAT_STRENGTH) / 20.0f) -1);

            if (pVictim->GetUnitBlockChance())
                pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYSHIELD);
            else
                pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);

            //Only set VICTIMSTATE_BLOCK on a full block
            if (*blocked_amount >= *damage)
            {
                *victimState = VICTIMSTATE_BLOCKS;
                *blocked_amount = *damage;
            }

            if(pVictim->GetTypeId() == TYPEID_PLAYER)
                ((Player*)pVictim)->UpdateDefense();
            pVictim->ModifyAuraState(AURA_STATE_DEFENSE,true);
            break;

        case MELEE_HIT_GLANCING:
        {
            // 30% reduction at 15 skill diff, no reduction at 5 skill diff
            int32 reducePerc = 100 - (pVictim->GetDefenseSkillValue() - GetWeaponSkillValue(attType) - 5) * 3;
            if (reducePerc < 70)
                reducePerc = 70;
            else if(reducePerc > 100)
                reducePerc = 100;
                
            cleanDamage->damage += (100-reducePerc)/100 * *damage;
            *damage *= reducePerc / 100;
            *hitInfo |= HITINFO_GLANCING;
            break;
        }
        case MELEE_HIT_CRUSHING:
        {
            // 150% normal damage
            *damage += (*damage / 2);
            cleanDamage->damage = *damage;
            *hitInfo |= HITINFO_CRUSHING;
            // TODO: victimState, victim animation?
            break;
        }
        default:
            break;
    }

    // apply melee damage bonus and absorb only if base damage not fully blocked to prevent negative damage or damage with full block
    if(*victimState != VICTIMSTATE_BLOCKS)
    {
        MeleeDamageBonus(pVictim, damage,attType);
        CalcAbsorbResist(pVictim, *damageType, *damage-*blocked_amount, absorbDamage, resistDamage);
    }

    if (*absorbDamage) *hitInfo |= HITINFO_ABSORB;
    if (*resistDamage) *hitInfo |= HITINFO_RESIST;
    cleanDamage += *blocked_amount;

    if (*damage <= *absorbDamage + *resistDamage + *blocked_amount)
    {
        //*hitInfo = 0x00010020;
        //*hitInfo |= HITINFO_SWINGNOHITSOUND;
        //*damageType = 0;
        CastMeleeProcDamageAndSpell(pVictim, 0, attType, outcome, spellCasted, isTriggeredSpell);
        return;
    }

    CastMeleeProcDamageAndSpell(pVictim, (*damage - *absorbDamage - *resistDamage - *blocked_amount), attType, outcome, spellCasted, isTriggeredSpell);

    // victim's damage shield
    // yet another hack to fix crashes related to the aura getting removed during iteration
    std::set<Aura*> alreadyDone;
    uint32 removedAuras = pVictim->m_removedAuras;
    AuraList const& vDamageShields = pVictim->GetAurasByType(SPELL_AURA_DAMAGE_SHIELD);
    for(AuraList::const_iterator i = vDamageShields.begin(), next = vDamageShields.begin(); i != vDamageShields.end(); i = next)
    {
        next++;
        if (alreadyDone.find(*i) == alreadyDone.end())
        {
            alreadyDone.insert(*i);
            pVictim->SpellNonMeleeDamageLog(this, (*i)->GetId(), (*i)->GetModifier()->m_amount, false, false);
            if (pVictim->m_removedAuras > removedAuras)
            {
                removedAuras = pVictim->m_removedAuras;
                next = vDamageShields.begin();
            }
        }
    }

    if(pVictim->m_currentSpell && pVictim->GetTypeId() == TYPEID_PLAYER && *damage)
    {
        if (pVictim->m_currentSpell->getState() != SPELL_STATE_CASTING)
        {
            sLog.outDetail("Spell Delayed!%d",(int32)(0.25f * pVictim->m_currentSpell->casttime));
            pVictim->m_currentSpell->Delayed((int32)(0.25f * pVictim->m_currentSpell->casttime));
        }
        else
        {
            uint32 channelInterruptFlags = pVictim->m_currentSpell->m_spellInfo->ChannelInterruptFlags;
            if( channelInterruptFlags & CHANNEL_FLAG_DELAY )
            {
                sLog.outDetail("Spell Delayed!%d",(int32)(0.25f * GetDuration(pVictim->m_currentSpell->m_spellInfo)));
                pVictim->m_currentSpell->DelayedChannel((int32)(0.25f * GetDuration(pVictim->m_currentSpell->m_spellInfo)));
                return;
            }
            else if( !(channelInterruptFlags & (CHANNEL_FLAG_DAMAGE | CHANNEL_FLAG_DAMAGE2)) )
                return;

            sLog.outDetail("Spell Canceled!");
            pVictim->m_currentSpell->cancel();
        }
    }
}

void Unit::AttackerStateUpdate (Unit *pVictim, WeaponAttackType attType, bool isTriggered)
{
    if(hasUnitState(UNIT_STAT_CONFUSED | UNIT_STAT_STUNDED | UNIT_STAT_FLEEING) || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED) )
        return;

    if (!pVictim->isAlive())
        return;

    if(!isTriggered)
    {
        if(m_currentSpell)
            return;

        // melee attack spell casted at main hand attack only
        if (m_currentMeleeSpell && attType == BASE_ATTACK)
        {
            m_currentMeleeSpell->cast();
            return;
        }
    }

    uint32 hitInfo;
    if (attType == BASE_ATTACK)
        hitInfo = HITINFO_NORMALSWING2;
    else if (attType == OFF_ATTACK)
        hitInfo = HITINFO_LEFTSWING;
    else
        return;

    uint32   damageType = NORMAL_DAMAGE;
    uint32   victimState = VICTIMSTATE_NORMAL;

    uint32   damage = 0;
    CleanDamage cleanDamage = CleanDamage(0, BASE_ATTACK, MELEE_HIT_NORMAL );
    uint32   blocked_dmg = 0;
    uint32   absorbed_dmg = 0;
    uint32   resisted_dmg = 0;

    if( pVictim->IsImmunedToPhysicalDamage() )
    {
        SendAttackStateUpdate (HITINFO_MISS, pVictim, 1, NORMAL_DAMAGE, 0, 0, 0, VICTIMSTATE_IS_IMMUNE, 0);
        return;
    }

    DoAttackDamage (pVictim, &damage, &cleanDamage, &blocked_dmg, &damageType, &hitInfo, &victimState, &absorbed_dmg, &resisted_dmg, attType);

    cleanDamage.damage += blocked_dmg;

    if (hitInfo & HITINFO_MISS)
        //send miss
        SendAttackStateUpdate (hitInfo, pVictim, 1, damageType, damage, absorbed_dmg, resisted_dmg, victimState, blocked_dmg);
    else
    {
        //do animation
        SendAttackStateUpdate (hitInfo, pVictim, 1, damageType, damage, absorbed_dmg, resisted_dmg, victimState, blocked_dmg);

        if (damage > (absorbed_dmg + resisted_dmg + blocked_dmg))
            damage -= (absorbed_dmg + resisted_dmg + blocked_dmg);
        else
            damage = 0;

        DealDamage (pVictim, damage, &cleanDamage, DIRECT_DAMAGE, SPELL_SCHOOL_NORMAL, NULL, 0, true);

        if(GetTypeId() == TYPEID_PLAYER && pVictim->isAlive())
        {
            for(int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
                ((Player*)this)->CastItemCombatSpell(((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0,i),pVictim);
        }
    }

    if (GetTypeId() == TYPEID_PLAYER)
        DEBUG_LOG("AttackerStateUpdate: (Player) %u attacked %u (TypeId: %u) for %u dmg, absorbed %u, blocked %u, resisted %u.",
            GetGUIDLow(), pVictim->GetGUIDLow(), pVictim->GetTypeId(), damage, absorbed_dmg, blocked_dmg, resisted_dmg);
    else
        DEBUG_LOG("AttackerStateUpdate: (NPC)    %u attacked %u (TypeId: %u) for %u dmg, absorbed %u, blocked %u, resisted %u.",
            GetGUIDLow(), pVictim->GetGUIDLow(), pVictim->GetTypeId(), damage, absorbed_dmg, blocked_dmg, resisted_dmg);
}

MeleeHitOutcome Unit::RollPhysicalOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType, SpellEntry const *spellInfo)
{
    // Miss chance based on melee
    int32 miss_chance = (int32)(MeleeMissChanceCalc(pVictim));

    // Critical hit chance
    float crit_chance = GetUnitCriticalChance();

    // Only players can have Talent&Spell bonuses
    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Talents
        AuraList const& mSpellCritSchool = GetAurasByType(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL);
        for(AuraList::const_iterator i = mSpellCritSchool.begin(); i != mSpellCritSchool.end(); ++i)
            if((*i)->GetModifier()->m_miscvalue == -2 || ((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellInfo->School)) != 0)
                crit_chance += (*i)->GetModifier()->m_amount;

        // Spellmods
        ((Player*)this)->ApplySpellMod(spellInfo->Id, SPELLMOD_CRITICAL_CHANCE, crit_chance);
    }

    DEBUG_LOG("PHYSICAL OUTCOME: hit %u crit %f miss %u",m_modHitChance,crit_chance,miss_chance);

    return RollMeleeOutcomeAgainst(pVictim, attType, int32(crit_chance * 100 ), miss_chance, m_modHitChance);
}

MeleeHitOutcome Unit::RollMeleeOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType) const
{
    // This is only wrapper

    // Miss chance based on melee
    int32 miss_chance = (int32)(MeleeMissChanceCalc(pVictim));

    // Critical hit chance
    int32 crit_chance = (int32)(GetUnitCriticalChance()*100);

    // Useful if want to specify crit & miss chances for melee, else it could be removed
    DEBUG_LOG("MELEE OUTCOME: hit %u crit %u miss %u", m_modHitChance,crit_chance,miss_chance);
    return RollMeleeOutcomeAgainst(pVictim, attType, crit_chance, miss_chance, m_modHitChance);
}

MeleeHitOutcome Unit::RollMeleeOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType, int32 crit_chance, int32 miss_chance, int32 hit_chance) const
{
    int32 skillDiff =  GetWeaponSkillValue(attType) - pVictim->GetDefenseSkillValue();
    // bonus from skills is 0.04%
    int32    skillBonus = skillDiff * 4;
    int32    skillBonus2 = 4 * ( GetWeaponSkillValue(attType) - pVictim->GetPureDefenseSkillValue() );
    int32    sum = 0, tmp = 0;
    int32    roll = urand (0, 10000);

    DEBUG_LOG ("RollMeleeOutcomeAgainst: skill bonus of %d for attacker", skillBonus);
    DEBUG_LOG ("RollMeleeOutcomeAgainst: rolled %d, +hit %d, dodge %u, parry %u, block %u, crit %u",
        roll, hit_chance, (uint32)(pVictim->GetUnitDodgeChance()*100), (uint32)(pVictim->GetUnitParryChance()*100),
        (uint32)(pVictim->GetUnitBlockChance()*100), crit_chance);

    // dual wield has 24% base chance to miss instead of 5%, also
    // base miss rate is 5% and can't get higher than 60%

    // Inherit if passed
    tmp = miss_chance - skillBonus;

    if(tmp > 6000)
        tmp = 6000;

    if (tmp > 0 && roll < (sum += tmp ))
    {
        DEBUG_LOG ("RollMeleeOutcomeAgainst: MISS");
        return MELEE_HIT_MISS;
    }

    // always crit against a sitting target
    if (   (pVictim->GetTypeId() == TYPEID_PLAYER)
        && (((Player*)pVictim)->getStandState() & (PLAYER_STATE_SLEEP | PLAYER_STATE_SIT
        | PLAYER_STATE_SIT_CHAIR
        | PLAYER_STATE_SIT_LOW_CHAIR
        | PLAYER_STATE_SIT_MEDIUM_CHAIR
        | PLAYER_STATE_SIT_HIGH_CHAIR)))
    {
        DEBUG_LOG ("RollMeleeOutcomeAgainst: CRIT (sitting victim)");
        return MELEE_HIT_CRIT;
    }

    // stunned target cannot dodge and this is check in GetUnitDodgeChance()
    tmp = (int32)(pVictim->GetUnitDodgeChance()*100) - skillBonus2;
    if (tmp > 0 && roll < (sum += tmp))
    {
        DEBUG_LOG ("RollMeleeOutcomeAgainst: DODGE <%d, %d)", sum-tmp, sum);
        return MELEE_HIT_DODGE;
    }

    int32   modCrit = 0;

    // check if attack comes from behind
    if (!pVictim->HasInArc(M_PI,this))
    {
        // ASSUME +10% crit from behind
        DEBUG_LOG ("RollMeleeOutcomeAgainst: attack came from behind.");
        modCrit += 1000;
    }
    else
    {
        // cannot parry or block attacks from behind, but can from forward
        tmp = (int32)(pVictim->GetUnitParryChance()*100);
        if (   (tmp > 0)                                    // check if unit _can_ parry
            && ((tmp -= skillBonus2) > 0)
            && (roll < (sum += tmp)))
        {
            DEBUG_LOG ("RollMeleeOutcomeAgainst: PARRY <%d, %d)", sum-tmp, sum);
            return MELEE_HIT_PARRY;
        }

        tmp = (int32)(pVictim->GetUnitBlockChance()*100);
        if (   (tmp > 0)                                    // check if unit _can_ block
            && ((tmp -= skillBonus2) > 0)
            && (roll < (sum += tmp)))
        {
            DEBUG_LOG ("RollMeleeOutcomeAgainst: BLOCK <%d, %d)", sum-tmp, sum);
            return MELEE_HIT_BLOCK;
        }
    }

    // Resilience - reduce crit chance by x%
    modCrit -= int32(pVictim->m_modResilience*100);

    // Critical chance
    tmp = crit_chance + skillBonus + modCrit;

    if (tmp > 0 && roll < (sum += tmp))
    {
        DEBUG_LOG ("RollMeleeOutcomeAgainst: CRIT <%d, %d)", sum-tmp, sum);
        return MELEE_HIT_CRIT;
    }

    // Max 40% chance to score a glancing blow against mobs that are higher level
    if (   (GetTypeId() == TYPEID_PLAYER)
        && (pVictim->GetTypeId() != TYPEID_PLAYER)
        && ((getLevel() < pVictim->getLevel())))
    {
        tmp = GetWeaponSkillValue(attType);
        int32   maxskill = GetMaxSkillValueForLevel();
        tmp = (tmp > maxskill) ? maxskill : tmp;
        tmp = ((pVictim->GetMaxSkillValueForLevel() - tmp - 5) * 300 + 1000 );
        tmp = tmp > 4000 ? 4000 : tmp;
        if (roll < (sum += tmp))
        {
            DEBUG_LOG ("RollMeleeOutcomeAgainst: GLANCING <%d, %d)", sum-4000, sum);
            return MELEE_HIT_GLANCING;
        }
    }

    // mobs can score crushing blows if they're 3 or more levels above victim
    // or when their weapon skill is 15 or more above victim's defense skill
    tmp = pVictim->GetDefenseSkillValue();
    int32 tmpmax = pVictim->GetMaxSkillValueForLevel();
    // having defense above your maximum (from items, talents etc.) has no effect
    tmp = tmp > tmpmax ? tmpmax : tmp;
    // tmp = mob's level * 5 - player's current defense skill
    tmp = GetMaxSkillValueForLevel() - tmp;
    if (GetTypeId() != TYPEID_PLAYER && (tmp >= 15))
    {
        // add 2% chance per lacking skill point, min. is 15%
        tmp = tmp * 200 - 1500;
        if (roll < (sum += tmp))
        {
            DEBUG_LOG ("RollMeleeOutcomeAgainst: CRUSHING <%d, %d)", sum-tmp, sum);
            return MELEE_HIT_CRUSHING;
        }
    }

    DEBUG_LOG ("RollMeleeOutcomeAgainst: NORMAL");
    return MELEE_HIT_NORMAL;
}

uint32 Unit::CalculateDamage (WeaponAttackType attType)
{
    float min_damage, max_damage;

    switch (attType)
    {
        case RANGED_ATTACK:
            min_damage = GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE);
            max_damage = GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE);
            break;
        case BASE_ATTACK:
            min_damage = GetFloatValue(UNIT_FIELD_MINDAMAGE);
            max_damage = GetFloatValue(UNIT_FIELD_MAXDAMAGE);
            break;
        case OFF_ATTACK:
            min_damage = GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE);
            max_damage = GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE);
            break;
    }

    if (min_damage > max_damage)
    {
        std::swap(min_damage,max_damage);
    }

    if(max_damage == 0.0)
        max_damage = 5.0;

    return urand ((uint32)min_damage, (uint32)max_damage);
}

void Unit::SendAttackStart(Unit* pVictim)
{
    if(GetTypeId()!=TYPEID_PLAYER || !pVictim)
        return;

    WorldPacket data( SMSG_ATTACKSTART, 16 );
    data << GetGUID();
    data << pVictim->GetGUID();

    ((Player*)this)->SendMessageToSet(&data, true);
    DEBUG_LOG( "WORLD: Sent SMSG_ATTACKSTART" );
}

void Unit::SendAttackStop(Unit* victim)
{
    if(!victim)
        return;

    WorldPacket data( SMSG_ATTACKSTOP, (4+16) );            // we guess size
    data.append(GetPackGUID());
    data.append(victim->GetPackGUID()); // can be 0x00...
    data << uint32( 0 ); // can be 0x1
    //data << (uint32)0; // removed in 2.0.8

    SendMessageToSet(&data, true);
    sLog.outDetail("%s %u stopped attacking %s %u", (GetTypeId()==TYPEID_PLAYER ? "player" : "creature"), GetGUIDLow(), (victim->GetTypeId()==TYPEID_PLAYER ? "player" : "creature"),victim->GetGUIDLow());

    /*if(victim->GetTypeId() == TYPEID_UNIT)
    ((Creature*)victim)->AI().EnterEvadeMode(this);*/
}

uint32 Unit::SpellMissChanceCalc(Unit *pVictim) const
{
    if(!pVictim)
        return 0;

    // PvP : PvE spell misschances per leveldif > 2
    int32 chance = pVictim->GetTypeId() == TYPEID_PLAYER ? 700 : 1100;

    int32 leveldif = pVictim->getLevel() - getLevel();
    if(leveldif < 0)
        leveldif = 0;

    int32 misschance = 400 - m_modSpellHitChance*100;
    if(leveldif < 3)
        misschance += leveldif * 100;
    else
        misschance += (leveldif - 2) * chance;

    return misschance < 100 ? 100 : misschance;
}

int32 Unit::MeleeMissChanceCalc(const Unit *pVictim) const
{
    if(!pVictim)
        return 0;

    // Base misschance 5%
    int32 misschance = 500;

    // DualWield - Melee spells and physical dmg spells - 5% , white damage 24%
    if (haveOffhandWeapon())
    {
        if ((m_currentMeleeSpell) ||
           (m_currentSpell && m_currentSpell->m_spellInfo->School == SPELL_SCHOOL_NORMAL))
            misschance = 500;
        else
            misschance = 2400;
    }

    // PvP : PvE melee misschances per leveldif > 2
    int32 chance = pVictim->GetTypeId() == TYPEID_PLAYER ? 500 : 700;

    int32 leveldif = pVictim->getLevel() - getLevel();
    if(leveldif < 0)
        leveldif = 0;

    if(leveldif < 3)
        misschance += leveldif * 100 - m_modHitChance*100;
    else
        misschance += (leveldif - 2) * chance - m_modHitChance*100;

    return misschance > 6000 ? 6000 : misschance;
}

uint16 Unit::GetDefenseSkillValue() const
{
    if(GetTypeId() == TYPEID_PLAYER)
        return ((Player*)this)->GetSkillValue (SKILL_DEFENSE);
    else
        return GetUnitMeleeSkill();
}

uint16 Unit::GetPureDefenseSkillValue() const
{
    if(GetTypeId() == TYPEID_PLAYER)
        return ((Player*)this)->GetPureSkillValue(SKILL_DEFENSE);
    else
        return GetUnitMeleeSkill();
}

float Unit::GetUnitDodgeChance() const
{
    if(hasUnitState(UNIT_STAT_STUNDED))
        return 0;
    return GetTypeId() == TYPEID_PLAYER ? GetFloatValue(PLAYER_DODGE_PERCENTAGE) : 5;
}

float Unit::GetUnitParryChance() const
{
    float chance = 0;
    if(GetTypeId() == TYPEID_PLAYER)
    {
        Player const* player = (Player const*)this;
        if(player->CanParry() && player->IsUseEquipedWeapon() )
        {
            Item *tmpitem = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            if(!tmpitem || tmpitem->IsBroken())
                tmpitem = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

            if(tmpitem && !tmpitem->IsBroken() && (
                tmpitem->GetProto()->InventoryType == INVTYPE_WEAPON ||
                tmpitem->GetProto()->InventoryType == INVTYPE_WEAPONOFFHAND ||
                tmpitem->GetProto()->InventoryType == INVTYPE_WEAPONMAINHAND ||
                tmpitem->GetProto()->InventoryType == INVTYPE_2HWEAPON))
                chance = GetFloatValue(PLAYER_PARRY_PERCENTAGE);
        }
    }
    else if(GetTypeId() == TYPEID_UNIT)
    {
        if(((Creature const*)this)->GetCreatureInfo()->type == CREATURE_TYPE_HUMANOID)
            chance = 5;
    }

    return chance;
}

float Unit::GetUnitBlockChance() const
{
    float chance = 0;
    if(GetTypeId() == TYPEID_PLAYER)
    {
        Item *tmpitem = ((Player const*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        if(tmpitem && !tmpitem->IsBroken() && tmpitem->GetProto()->Block)
            chance = GetFloatValue(PLAYER_BLOCK_PERCENTAGE);
    }
    else
        chance = 5;

    return chance;
}

uint16 Unit::GetWeaponSkillValue (WeaponAttackType attType) const
{
    if(GetTypeId() == TYPEID_PLAYER)
    {
        uint16  slot;
        switch (attType)
        {
            case BASE_ATTACK:   slot = EQUIPMENT_SLOT_MAINHAND; break;
            case OFF_ATTACK:    slot = EQUIPMENT_SLOT_OFFHAND;  break;
            case RANGED_ATTACK: slot = EQUIPMENT_SLOT_RANGED;   break;
            default:
                return 0;
        }
        Item    *item = ((Player*)this)->GetItemByPos (INVENTORY_SLOT_BAG_0, slot);

        if(slot != EQUIPMENT_SLOT_MAINHAND && (!item || item->IsBroken() ||
            item->GetProto()->Class != ITEM_CLASS_WEAPON || !((Player*)this)->IsUseEquipedWeapon() ))
            return 0;

        // in range
        uint32  skill = item && !item->IsBroken() && ((Player*)this)->IsUseEquipedWeapon()
            ? item->GetSkill() : SKILL_UNARMED;
        return ((Player*)this)->GetSkillValue (skill);
    }
    else
        return GetUnitMeleeSkill();
}

uint16 Unit::GetPureWeaponSkillValue (WeaponAttackType attType) const
{
    if(GetTypeId() == TYPEID_PLAYER)
    {
        uint16  slot;
        switch (attType)
        {
            case BASE_ATTACK:   slot = EQUIPMENT_SLOT_MAINHAND; break;
            case OFF_ATTACK:    slot = EQUIPMENT_SLOT_OFFHAND;  break;
            case RANGED_ATTACK: slot = EQUIPMENT_SLOT_RANGED;   break;
            default:
                return 0;
        }
        Item    *item = ((Player*)this)->GetItemByPos (INVENTORY_SLOT_BAG_0, slot);

        if(slot != EQUIPMENT_SLOT_MAINHAND && (!item || item->IsBroken() ||
            item->GetProto()->Class != ITEM_CLASS_WEAPON || !((Player*)this)->IsUseEquipedWeapon() ))
            return 0;

        // in range
        uint32  skill = item && !item->IsBroken() && ((Player*)this)->IsUseEquipedWeapon()
            ? item->GetSkill() : SKILL_UNARMED;
        return ((Player*)this)->GetPureSkillValue (skill);
    }
    else
        return GetUnitMeleeSkill();
}

void Unit::_UpdateSpells( uint32 time )
{
    if(m_oldSpell != NULL)
    {
        delete m_oldSpell;
        m_oldSpell = NULL;
    }

    if(m_currentSpell != NULL)
    {
        m_currentSpell->update(time);
        if(m_currentSpell->IsAutoRepeat())
        {
            if(m_currentSpell->getState() == SPELL_STATE_FINISHED)
            {
                //Auto Shot & Shoot
                if( m_currentSpell->m_spellInfo->AttributesEx2 == 0x000020 && GetTypeId() == TYPEID_PLAYER )
                    resetAttackTimer( RANGED_ATTACK );
                else
                    setAttackTimer( RANGED_ATTACK, m_currentSpell->m_spellInfo->RecoveryTime);

                m_currentSpell->setState(SPELL_STATE_IDLE);
            }
            else if(m_currentSpell->getState() == SPELL_STATE_IDLE && isAttackReady(RANGED_ATTACK) )
            {
                // check movement in player case
                if(GetTypeId() == TYPEID_PLAYER && ((Player*)this)->GetMovementFlags())
                {
                    // cancel wand shooting
                    if(m_currentSpell->m_spellInfo->Category == 351)
                    {
                        WorldPacket data(SMSG_CANCEL_AUTO_REPEAT, 0);
                        ((Player*)this)->GetSession()->SendPacket(&data);
                        SetCurrentCastedSpell(NULL);
                    }
                    // ELSE delay auto-repeat ranged weapon until player movement stop
                }
                else
                    // recheck range and req. items (ammo and gun, etc)
                if(m_currentSpell->CheckRange() == 0 && m_currentSpell->CheckItems() == 0 )
                {
                    m_currentSpell->setState(SPELL_STATE_PREPARING);
                    m_currentSpell->ReSetTimer();
                }
                else
                {
                    if(GetTypeId()==TYPEID_PLAYER)
                    {
                        WorldPacket data(SMSG_CANCEL_AUTO_REPEAT, 0);
                        ((Player*)this)->GetSession()->SendPacket(&data);
                    }
                    SetCurrentCastedSpell(NULL);
                }
            }
        }
        else if(m_currentSpell->getState() == SPELL_STATE_FINISHED)
        {
            delete m_currentSpell;
            m_currentSpell = NULL;
        }
    }

    if(m_currentMeleeSpell != NULL)
    {
        m_currentMeleeSpell ->update(time);
        if(m_currentMeleeSpell ->getState() == SPELL_STATE_FINISHED)
        {
            delete m_currentMeleeSpell ;
            m_currentMeleeSpell  = NULL;
        }
    }

    // TODO: Find a better way to prevent crash when multiple auras are removed.
    m_removedAuras = 0;
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
        if ((*i).second)
            (*i).second->SetUpdated(false);

    for (AuraMap::iterator i = m_Auras.begin(), next; i != m_Auras.end(); i = next)
    {
        next = i;
        next++;
        if ((*i).second)
        {
            // prevent double update
            if ((*i).second->IsUpdated())
                continue;
            (*i).second->SetUpdated(true);
            (*i).second->Update( time );
            // several auras can be deleted due to update
            if (m_removedAuras)
            {
                if (m_Auras.empty()) break;
                next = m_Auras.begin();
                m_removedAuras = 0;
            }
        }
    }

    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end();)
    {
        if ((*i).second)
        {
            if ( !(*i).second->GetAuraDuration() && !((*i).second->IsPermanent() || ((*i).second->IsPassive())) )
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

    if(!m_gameObj.empty())
    {
        std::list<GameObject*>::iterator ite1, dnext1;
        for (ite1 = m_gameObj.begin(); ite1 != m_gameObj.end(); ite1 = dnext1)
        {
            dnext1 = ite1;
            //(*i)->Update( difftime );
            if( !(*ite1)->isSpawned() )
            {
                (*ite1)->SetOwnerGUID(0);
                (*ite1)->SetRespawnTime(0);
                (*ite1)->Delete();
                dnext1 = m_gameObj.erase(ite1);
            }
            else
                ++dnext1;
        }
    }
}

void Unit::SetCurrentCastedSpell( Spell * pSpell )
{

    if(pSpell && pSpell->IsMeleeSpell())
    {
        if(m_currentMeleeSpell)
        {
            m_currentMeleeSpell->cancel();
            delete m_currentMeleeSpell;
            m_currentMeleeSpell = NULL;
        }
        m_currentMeleeSpell = pSpell;
    }
    else
    {
        if(m_currentSpell)
        {
            m_currentSpell->cancel();
            // let call spell from spell (single level recursion) and not crash at returning to procesiing old spell
            if(m_oldSpell)
                delete m_oldSpell;
            m_oldSpell = m_currentSpell;
        }
        m_currentSpell = pSpell;
    }
}

void Unit::InterruptSpell()
{
    if(m_currentSpell)
    {
        //m_currentSpell->SendInterrupted(0x20);
        m_currentSpell->cancel();
    }
}

bool Unit::isInFront(Unit const* target, float radius) const
{
    return IsWithinDistInMap(target, radius) && HasInArc( M_PI, target );
}

void Unit::SetInFront(Unit const* target)
{
    SetOrientation(GetAngle(target));
}

bool Unit::isInAccessablePlaceFor(Creature* c) const
{
    if(IsInWater())
        return c->isCanSwimOrFly();
    else
        return c->isCanWalkOrFly();
}

bool Unit::IsInWater() const
{
    return MapManager::Instance().GetMap(GetMapId(), this)->IsInWater(GetPositionX(),GetPositionY(), GetPositionZ());
}

bool Unit::IsUnderWater() const
{
    return MapManager::Instance().GetMap(GetMapId(), this)->IsUnderWater(GetPositionX(),GetPositionY(),GetPositionZ());
}

void Unit::DeMorph()
{
    SetUInt32Value(UNIT_FIELD_DISPLAYID, GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
}

long Unit::GetTotalAuraModifier(uint32 ModifierID) const
{
    uint32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(ModifierID);
    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
        modifier += (*i)->GetModifier()->m_amount;

    return modifier;
}

bool Unit::AddAura(Aura *Aur, bool uniq)
{
    if (!isAlive() && !(Aur->GetSpellProto()->Id == 20584 || Aur->GetSpellProto()->Id == 8326)) // ghost spell check
    {
        delete Aur;
        return false;
    }

    if(Aur->GetTarget() != this)
    {
        sLog.outError("Aura (spell %u eff %u) add to aura list of %s (lowguid: %u) but Aura target is %s (lowguid: %u)",
            Aur->GetId(),Aur->GetEffIndex(),(GetTypeId()==TYPEID_PLAYER?"player":"creature"),GetGUIDLow(),
            (Aur->GetTarget()->GetTypeId()==TYPEID_PLAYER?"player":"creature"),Aur->GetTarget()->GetGUIDLow());
        delete Aur;
        return false;
    }

    AuraMap::iterator i = m_Auras.find( spellEffectPair(Aur->GetId(), Aur->GetEffIndex()) );

    // take out same spell
    if (i != m_Auras.end())
    {
        /*(*i).second->SetAuraDuration(Aur->GetAuraDuration());
        if ((*i).second->GetTarget())
        if ((*i).second->GetTarget()->GetTypeId() == TYPEID_PLAYER )
        (*i).second->UpdateAuraDuration();
        delete Aur;
        return false;*/
        // passive and persistent auras can stack with themselves any number of times
        if (!Aur->IsPassive() && !Aur->IsPersistent() && m_Auras.count(spellEffectPair(Aur->GetId(), Aur->GetEffIndex())) >= Aur->GetSpellProto()->StackAmount)
            RemoveAura(i);
    }

    // passive auras stack with all (except passive spell proc auras)
    if ((!Aur->IsPassive() || !IsPassiveStackableSpell(Aur->GetId())) &&          
        !(Aur->GetSpellProto()->Id == 20584 || Aur->GetSpellProto()->Id == 8326)) 
    {
        if (!RemoveNoStackAurasDueToAura(Aur))
        {
            delete Aur;
            return false;                                   // couldnt remove conflicting aura with higher rank
        }
    }

    // adding linked auras
    // add the shapeshift aura's boosts
    if(Aur->GetModifier()->m_auraname == SPELL_AURA_MOD_SHAPESHIFT)
        Aur->HandleShapeshiftBoosts(true);

    Aur->_AddAura();
    m_Auras.insert(AuraMap::value_type(spellEffectPair(Aur->GetId(), Aur->GetEffIndex()), Aur));
    if (Aur->GetModifier()->m_auraname < TOTAL_AURAS)
    {
        m_modAuras[Aur->GetModifier()->m_auraname].push_back(Aur);
        m_AuraModifiers[Aur->GetModifier()->m_auraname] += (Aur->GetModifier()->m_amount);
    }

    if (IsSingleTarget(Aur->GetId()) && Aur->GetTarget() && Aur->GetSpellProto())
    {
        if(Unit* caster = Aur->GetCaster())
        {
            AuraList& scAuras = caster->GetSingleCastAuras();
            AuraList::iterator itr, next;
            for (itr = scAuras.begin(); itr != scAuras.end(); itr = next)
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
                    if(scAuras.empty())
                        break;
                    else
                        next = scAuras.begin();
                }
            }
            scAuras.push_back(Aur);
        }
    }
    return true;
}

void Unit::RemoveRankAurasDueToSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
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
            if(objmgr.IsRankSpellDueToSpell(spellInfo,i_spellId))
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

    SpellEntry const* spellProto = Aur->GetSpellProto();
    if (!spellProto) 
        return false;

    uint32 spellId = Aur->GetId();
    uint32 effIndex = Aur->GetEffIndex();
    bool is_sec = IsSpellSingleEffectPerCaster(spellId);
    AuraMap::iterator i,next;
    for (i = m_Auras.begin(); i != m_Auras.end(); i = next)
    {
        next = i;
        next++;
        if (!(*i).second) continue;

        if (!(*i).second->GetSpellProto())
            continue;

        uint32 i_spellId = (*i).second->GetId();

        if(IsPassiveSpell(i_spellId))
        {
            if(IsPassiveStackableSpell(i_spellId))
                continue;

            // passive non-stackable spells not stackable only with another rank of same spell
            if (!objmgr.IsRankSpellDueToSpell(Aur->GetSpellProto(), i_spellId))
                continue;
        }

        uint32 i_effIndex = (*i).second->GetEffIndex();

        if(i_spellId == spellId) continue;

        bool is_triggered_by_spell = false;
        // prevent triggered aura of removing aura that triggered it
        for(int j = 0; j < 3; ++j)
            if ((*i).second->GetSpellProto()->EffectTriggerSpell[j] == spellProto->Id)
                is_triggered_by_spell = true;
        if (is_triggered_by_spell) continue;

        // prevent remove dummy triggered spells at next effect aura add
        for(int j = 0; j < 3; ++j)
        {
            switch(spellProto->Effect[j])
            {
                case SPELL_EFFECT_DUMMY:
                    switch(spellId)
                    {
                        case 5420: if(i_spellId==34123) is_triggered_by_spell = true; break;
                    }
                    break;
            }
            if(is_triggered_by_spell)
                break;

            switch(spellProto->EffectApplyAuraName[j])
            {
                case SPELL_AURA_MOD_SHAPESHIFT:
                    switch(spellId)
                    {
                        case 33891: if(i_spellId==5420 || i_spellId==34123) is_triggered_by_spell = true; break;
                    }
                    break;
            }
        }

        if(!is_triggered_by_spell)
        {
            bool sec_match = false;
            bool is_i_sec = IsSpellSingleEffectPerCaster(i_spellId);
            if( is_sec && is_i_sec )
                if (Aur->GetCasterGUID() == (*i).second->GetCasterGUID())
                    if (GetSpellSpecific(spellId) == GetSpellSpecific(i_spellId))
                        sec_match = true;
            if( sec_match || objmgr.IsNoStackSpellDueToSpell(spellId, i_spellId) && !is_sec && !is_i_sec )
            {
                // if sec_match this isn't always true, needs to be rechecked
                if (objmgr.IsRankSpellDueToSpell(Aur->GetSpellProto(), i_spellId))
                    if(CompareAuraRanks(spellId, effIndex, i_spellId, i_effIndex) < 0)
                        return false;                       // cannot remove higher rank

                RemoveAurasDueToSpell(i_spellId);

                if( m_Auras.empty() )
                    break;
                else
                    next =  m_Auras.begin();
            }
            else                                            // Potions stack aura by aura
            if (Aur->GetSpellProto()->SpellFamilyName == SPELLFAMILY_POTION &&
                (*i).second->GetSpellProto()->SpellFamilyName == SPELLFAMILY_POTION)
            {
                if (IsNoStackAuraDueToAura(spellId, effIndex, i_spellId, i_effIndex))
                {
                    if(CompareAuraRanks(spellId, effIndex, i_spellId, i_effIndex) < 0)
                        return false;                       // cannot remove higher rank

                    RemoveAura(i);
                    next = i;
                }
            }
        }
    }
    return true;
}

void Unit::RemoveFirstAuraByDispel(uint32 dispel_type, Unit *pCaster)
{
    AuraMap::iterator i;
    for (i = m_Auras.begin(); i != m_Auras.end();)
    {
        if ((*i).second && (*i).second->GetSpellProto()->Dispel == dispel_type)
        {
            if(dispel_type == 1)
            {
                bool positive = true;
                switch((*i).second->GetSpellProto()->EffectImplicitTargetA[(*i).second->GetEffIndex()])
                {
                    case TARGET_CHAIN_DAMAGE:
                    case TARGET_ALL_ENEMY_IN_AREA:
                    case TARGET_ALL_ENEMY_IN_AREA_INSTANT:
                    case TARGET_ALL_ENEMIES_AROUND_CASTER:
                    case TARGET_IN_FRONT_OF_CASTER:
                    case TARGET_DUELVSPLAYER:
                    case TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
                    case TARGET_CURRENT_SELECTED_ENEMY:
                        positive = false;
                        break;

                    default:
                        positive = ((*i).second->GetSpellProto()->AttributesEx & (1<<7)) ? false : true;
                }
                if(positive && IsFriendlyTo(pCaster)) // PBW
                {
                    ++i;
                    continue;
                }
            }
            RemoveAura(i);
            break;
        }
        else
            ++i;
    }
}

void Unit::RemoveAreaAurasByOthers(uint64 guid)
{
    int j = 0;
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end();)
    {
        if (i->second && i->second->IsAreaAura())
        {
            uint64 casterGuid = i->second->GetCasterGUID();
            uint64 targetGuid = i->second->GetTarget()->GetGUID();
            // if area aura cast by someone else or by the specified caster
            if (casterGuid == guid || (guid == 0 && casterGuid != targetGuid))
            {
                for (j = 0; j < 4; j++)
                    if (m_TotemSlot[j] == casterGuid)
                        break;
                // and not by one of my totems
                if (j == 4)
                    RemoveAura(i);
                else
                    ++i;
            }
            else
                ++i;
        }
        else
            ++i;
    }
}

void Unit::RemoveAura(uint32 spellId, uint32 effindex)
{
    AuraMap::iterator iter;
    while((iter = m_Auras.find(spellEffectPair(spellId, effindex))) != m_Auras.end())
        RemoveAura(iter);
}

void Unit::RemoveAurasDueToSpell(uint32 spellId)
{
    for (int i = 0; i < 3; ++i)
        RemoveAura(spellId,i);
}

void Unit::RemoveAurasDueToItem(Item* castItem)
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end(); )
    {
        if (iter->second->GetCastItemGUID() == castItem->GetGUID())
            RemoveAura(iter);
        else
            ++iter;
    }
}

void Unit::RemoveAura(AuraMap::iterator &i, bool onDeath)
{
    if (IsSingleTarget((*i).second->GetId()))
    {
        if(Unit* caster = (*i).second->GetCaster())
        {
            AuraList& scAuras = caster->GetSingleCastAuras();
            scAuras.remove((*i).second);
        }
    }
    // remove aura from party members when the caster turns off the aura
    if((*i).second->IsAreaAura())
    {
        Unit *i_target = (*i).second->GetTarget();
        if((*i).second->GetCasterGUID() == i_target->GetGUID())
        {
            Unit* i_caster = i_target;

            Unit* owner = NULL;
            Group *pGroup = NULL;
            Player *pGroupOf = NULL;
            if (i_caster->GetTypeId() == TYPEID_PLAYER)
            {
                pGroupOf = (Player*)i_caster;
                pGroup = pGroupOf->GetGroup();
            }
            else if(((Creature*)i_caster)->isTotem() || ((Creature*)i_caster)->isPet() || i_caster->isCharmed())
            {
                owner = i_caster->GetCharmerOrOwner();
                if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                {
                    pGroupOf = (Player*)owner;
                    pGroup = pGroupOf->GetGroup();
                }
            }

            //float radius =  GetRadius(sSpellRadiusStore.LookupEntry((*i).second->GetSpellProto()->EffectRadiusIndex[(*i).second->GetEffIndex()]));
            if(pGroup && pGroupOf)
            {
                for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    Player* Target = itr->getSource();
                    if(!Target || !pGroup->SameSubGroup(pGroupOf, Target))
                        continue;
                    
                    if(Target->GetGUID() == i_caster->GetGUID())
                        continue;
                    Aura *t_aura = Target->GetAura((*i).second->GetId(), (*i).second->GetEffIndex());
                    if (t_aura)
                        if (t_aura->GetCasterGUID() == i_caster->GetGUID())
                            Target->RemoveAura((*i).second->GetId(), (*i).second->GetEffIndex());
                }
            }
            else if(owner)
            {
                Aura *t_aura = owner->GetAura((*i).second->GetId(), (*i).second->GetEffIndex());
                if (t_aura)
                    if (t_aura->GetCasterGUID() == i_caster->GetGUID())
                        owner->RemoveAura((*i).second->GetId(), (*i).second->GetEffIndex());
            }
        }
    }
    if ((*i).second->GetModifier()->m_auraname < TOTAL_AURAS)
    {
        m_AuraModifiers[(*i).second->GetModifier()->m_auraname] -= ((*i).second->GetModifier()->m_amount);
        m_modAuras[(*i).second->GetModifier()->m_auraname].remove((*i).second);
    }
    (*i).second->SetRemoveOnDeath(onDeath);

    // remove from list before mods removing (prevent cyclic calls, mods added before including to aura list - use reverse order)
    Aura* Aur = i->second;

    DiminishingMechanics mech = DIMINISHING_NONE;
    if(Aur->GetSpellProto()->Mechanic)
    {
        mech = Unit::Mechanic2DiminishingMechanics(Aur->GetSpellProto()->Mechanic);
        if(mech == DIMINISHING_MECHANIC_STUN || GetTypeId() == TYPEID_PLAYER && mech != DIMINISHING_NONE)
            UpdateDiminishingTime(mech);
    }

    // must remove before removing from list (its remove dependent auras and _i_ is only safe iterator value
    // remove the shapeshift aura's boosts
    if(Aur->GetModifier()->m_auraname == SPELL_AURA_MOD_SHAPESHIFT)
        Aur->HandleShapeshiftBoosts(false);

    m_Auras.erase(i);
    m_removedAuras++;                                       // internal count used by unit update

    Aur->_RemoveAura();
    delete Aur;

    // only way correctly remove all auras from list
    if( m_Auras.empty() )
        i = m_Auras.end();
    else
        i = m_Auras.begin();
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

void Unit::RemoveAllAurasOnDeath()
{
    // used just after dieing to remove all visible auras
    // and disable the mods for the passive ones
    for(AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        if (!iter->second->IsPassive())
            RemoveAura(iter, true);
        else
            ++iter;
    }
}

void Unit::DelayAura(uint32 spellId, uint32 effindex, int32 delaytime)
{
    AuraMap::iterator iter = m_Auras.find(spellEffectPair(spellId, effindex));
    if (iter != m_Auras.end())
    {
        if (iter->second->GetAuraDuration() < delaytime)
            iter->second->SetAuraDuration(0);
        else
            iter->second->SetAuraDuration(iter->second->GetAuraDuration() - delaytime);
        iter->second->UpdateAuraDuration();
        sLog.outDebug("Aura %u partially interrupted on unit %u, new duration: %u ms",iter->second->GetModifier()->m_auraname, GetGUIDLow(), iter->second->GetAuraDuration());
    }
}

void Unit::_RemoveAllAuraMods()
{
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        (*i).second->ApplyModifier(false);
    }
}

void Unit::_ApplyAllAuraMods()
{
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
         (*i).second->ApplyModifier(true);
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

void Unit::AddDynObject(DynamicObject* dynObj)
{
    m_dynObjGUIDs.push_back(dynObj->GetGUID());
}

void Unit::RemoveDynObject(uint32 spellid)
{
    if(m_dynObjGUIDs.empty())
        return;
    for (DynObjectGUIDs::iterator i = m_dynObjGUIDs.begin(); i != m_dynObjGUIDs.end();)
    {
        DynamicObject* dynObj = ObjectAccessor::Instance().GetDynamicObject(*this,*m_dynObjGUIDs.begin());
        if(!dynObj)
        {
            i = m_dynObjGUIDs.erase(i);
        }
        else if(spellid == 0 || dynObj->GetSpellId() == spellid)
        {
            dynObj->Delete();
            i = m_dynObjGUIDs.erase(i);
        }
        else
            ++i;
    }
}

DynamicObject * Unit::GetDynObject(uint32 spellId, uint32 effIndex)
{
    for (DynObjectGUIDs::iterator i = m_dynObjGUIDs.begin(); i != m_dynObjGUIDs.end();)
    {
        DynamicObject* dynObj = ObjectAccessor::Instance().GetDynamicObject(*this,*m_dynObjGUIDs.begin());
        if(!dynObj)
        {
            i = m_dynObjGUIDs.erase(i);
            continue;
        }

        if (dynObj->GetSpellId() == spellId && dynObj->GetEffIndex() == effIndex)
            return dynObj;
        ++i;
    }
    return NULL;
}

void Unit::AddGameObject(GameObject* gameObj)
{
    assert(gameObj && gameObj->GetOwnerGUID()==0);
    m_gameObj.push_back(gameObj);
    gameObj->SetOwnerGUID(GetGUID());
}

void Unit::RemoveGameObject(GameObject* gameObj, bool del)
{
    assert(gameObj && gameObj->GetOwnerGUID()==GetGUID());
    gameObj->SetOwnerGUID(0);
    m_gameObj.remove(gameObj);
    if(del)
    {
        gameObj->SetRespawnTime(0);
        gameObj->Delete();
    }
}

void Unit::RemoveGameObject(uint32 spellid, bool del)
{
    if(m_gameObj.empty())
        return;
    std::list<GameObject*>::iterator i, next;
    for (i = m_gameObj.begin(); i != m_gameObj.end(); i = next)
    {
        next = i;
        if(spellid == 0 || (*i)->GetSpellId() == spellid)
        {
            (*i)->SetOwnerGUID(0);
            if(del)
            {
                (*i)->SetRespawnTime(0);
                (*i)->Delete();
            }

            next = m_gameObj.erase(i);
        }
        else
            ++next;
    }
}

void Unit::SendSpellNonMeleeDamageLog(Unit *target,uint32 SpellID,uint32 Damage, uint8 DamageType,uint32 AbsorbedDamage, uint32 Resist,bool PhysicalDamage, uint32 Blocked, bool CriticalHit)
{
    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, (16+31)); // we guess size
    data.append(target->GetPackGUID());
    data.append(GetPackGUID());
    data << uint32(SpellID);
    data << uint32(Damage-AbsorbedDamage-Resist-Blocked);
    data << uint8(DamageType);                              //damagetype
    data << uint32(AbsorbedDamage);                         //AbsorbedDamage
    data << uint32(Resist);                                 //resist
    data << (uint8)PhysicalDamage;
    data << uint8(0);
    data << uint32(Blocked);                                        //blocked
    data << uint8(CriticalHit ? 2 : 0);                     //seen 0x05 also...
    data << uint32(0);
    SendMessageToSet( &data, true );
}

void Unit::SendAttackStateUpdate(uint32 HitInfo, Unit *target, uint8 SwingType, uint32 DamageType, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, uint32 TargetState, uint32 BlockedAmount)
{
    sLog.outDebug("WORLD: Sending SMSG_ATTACKERSTATEUPDATE");

    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, (16+45));    // we guess size
    data << (uint32)HitInfo;
    data.append(GetPackGUID());
    data.append(target->GetPackGUID());
    data << (uint32)(Damage-AbsorbDamage-Resist-BlockedAmount);

    data << (uint8)SwingType;
    data << (uint32)DamageType;

    //
    data << (float)(Damage-AbsorbDamage-Resist-BlockedAmount);
    // still need to double check damaga
    data << (uint32)(Damage-AbsorbDamage-Resist-BlockedAmount);
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

struct ProcTriggeredData
{
    ProcTriggeredData(SpellEntry const * _spellInfo, uint32 _spellParam, Aura* _triggeredByAura)
        : spellInfo(_spellInfo), spellParam(_spellParam), triggeredByAura(_triggeredByAura) {}

    SpellEntry const * spellInfo;
    uint32 spellParam;
    Aura* triggeredByAura;
};

typedef std::list< ProcTriggeredData > ProcTriggeredList;

// used to prevent spam in log about same non-handled spells
static std::set<uint32> nonHandledSpellProcSet;

void Unit::ProcDamageAndSpell(Unit *pVictim, uint32 procAttacker, uint32 procVictim, uint32 damage, SpellEntry const *procSpell, bool isTriggeredSpell, WeaponAttackType attType)
{
    sLog.outDebug("ProcDamageAndSpell: attacker flags are 0x%x, victim flags 0x%x", procAttacker, procVictim);
    if(procSpell)
        sLog.outDebug("ProcDamageAndSpell: invoked due to spell id %u %s", procSpell->Id, (isTriggeredSpell?"(triggered)":""));

    // Assign melee/ranged proc flags for magic attacks, that are actually melee/ranged abilities
    // That is the question though if it's fully correct
    if(procSpell)
    {
        if(procSpell->DmgClass == SPELL_DAMAGE_CLASS_MELEE)
        {
            if(procAttacker &  PROC_FLAG_HIT_SPELL) procAttacker |= PROC_FLAG_HIT_MELEE;
            if(procAttacker & PROC_FLAG_CRIT_SPELL) procAttacker |= PROC_FLAG_CRIT_MELEE;
            if(procVictim & PROC_FLAG_STRUCK_SPELL) procVictim |= PROC_FLAG_STRUCK_MELEE;
            if(procVictim & PROC_FLAG_STRUCK_CRIT_SPELL) procVictim |= PROC_FLAG_STRUCK_CRIT_MELEE;
            attType = BASE_ATTACK;                          // Melee abilities are assumed to be dealt with mainhand weapon
        }
        else if (procSpell->DmgClass == SPELL_DAMAGE_CLASS_RANGED)
        {
            if(procAttacker &  PROC_FLAG_HIT_SPELL) procAttacker |= PROC_FLAG_HIT_RANGED;
            if(procAttacker & PROC_FLAG_CRIT_SPELL) procAttacker |= PROC_FLAG_CRIT_RANGED;
            if(procVictim & PROC_FLAG_STRUCK_SPELL) procVictim |= PROC_FLAG_STRUCK_RANGED;
            if(procVictim & PROC_FLAG_STRUCK_CRIT_SPELL) procVictim |= PROC_FLAG_STRUCK_CRIT_RANGED;
            attType = RANGED_ATTACK;
        }
    }
    if(damage && (procVictim & (PROC_FLAG_STRUCK_MELEE|PROC_FLAG_STRUCK_RANGED|PROC_FLAG_STRUCK_SPELL)))
        procVictim |= (PROC_FLAG_TAKE_DAMAGE|PROC_FLAG_TOUCH);

    // Not much to do if no flags are set. 
    if (procAttacker)
    {
        for(std::set<uint32>::iterator aur = attackerProcAuraTypes.begin(); aur != attackerProcAuraTypes.end(); ++aur)
        {
            // List of spells (effects) that proced. Spell prototype and aura-specific value (damage for TRIGGER_DAMAGE)
            ProcTriggeredList procTriggered;

            AuraList const& attackerAuras = GetAurasByType(*aur);
            for(AuraList::const_iterator i = attackerAuras.begin(), next; i != attackerAuras.end(); i = next)
            {
                next = i; next++;
                uint32 procFlag = procAttacker;

                SpellEntry const *spellProto = (*i)->GetSpellProto();
                if(!spellProto) continue;
                SpellProcEventEntry const *spellProcEvent = objmgr.GetSpellProcEvent(spellProto->Id);

                if(!spellProcEvent && spellProto->procFlags != 0 && nonHandledSpellProcSet.find(spellProto->Id)==nonHandledSpellProcSet.end())
                {
                    sLog.outError("ProcDamageAndSpell: spell %u (attacker's aura source) not have record in `spell_proc_event`)",spellProto->Id);
                    nonHandledSpellProcSet.insert(spellProto->Id);
                }

                uint32 procFlags = spellProcEvent ? spellProcEvent->procFlags : spellProto->procFlags;
                // Check if current equipment allows aura to proc
                if(GetTypeId() == TYPEID_PLAYER && ((Player*)this)->IsUseEquipedWeapon())
                {
                    if(spellProto->EquippedItemClass == ITEM_CLASS_WEAPON)
                    {
                        Item *item = NULL;
                        if(attType == BASE_ATTACK)
                            item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                        else if (attType == OFF_ATTACK)
                            item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                        else
                            item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);

                        if(!item || item->IsBroken() || item->GetProto()->Class != ITEM_CLASS_WEAPON || !((1<<item->GetProto()->SubClass) & spellProto->EquippedItemSubClassMask))
                            continue;
                    }
                    else if(spellProto->EquippedItemClass == ITEM_CLASS_ARMOR)
                    {
                        // Check if player is wearing shield
                        Item *item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                        if(!item || item->IsBroken() || item->GetProto()->Class != ITEM_CLASS_ARMOR || !((1<<item->GetProto()->SubClass) & spellProto->EquippedItemSubClassMask))
                            continue;
                    }
                }
                if((procFlag & procFlags) == 0)
                    continue;

                // Additional checks in case spell cast/hit/crit is the event
                // Check (if set) school, category, skill line, spell talent mask
                if(spellProcEvent)
                {
                    if(spellProcEvent->schoolMask && (!procSpell || !procSpell->School || ((1<<(procSpell->School-1)) & spellProcEvent->schoolMask) == 0))
                        continue;
                    if(spellProcEvent->category && (!procSpell || procSpell->Category != spellProcEvent->category))
                        continue;
                    if(spellProcEvent->skillId)
                    {
                        if (!procSpell) continue;
                        SkillLineAbilityEntry const *skillLineEntry = sSkillLineAbilityStore.LookupEntry(procSpell->Id);
                        if(!skillLineEntry || skillLineEntry->skillId != spellProcEvent->skillId)
                            continue;
                    }
                    if(spellProcEvent->spellFamilyName && (!procSpell || spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
                        continue;
                    if(spellProcEvent->spellFamilyMask && (!procSpell || (spellProcEvent->spellFamilyMask & procSpell->SpellFamilyFlags) == 0))
                        continue;
                }

                // Need to use floats here, cuz calculated PPM chance often is about 1-2%
                float chance = (float)spellProto->procChance;
                if(GetTypeId() == TYPEID_PLAYER)
                    ((Player*)this)->ApplySpellMod(spellProto->Id,SPELLMOD_CHANCE_OF_SUCCESS,chance);
                uint32 WeaponSpeed = GetAttackTime(attType);
                if(spellProcEvent && spellProcEvent->ppmRate != 0)
                    chance = GetPPMProcChance(WeaponSpeed, spellProcEvent->ppmRate);

                if(roll_chance_f(chance))
                {
                    if((*i)->m_procCharges > 0)
                        (*i)->m_procCharges -= 1;

                    uint32 i_spell_eff = (*i)->GetEffIndex();

                    int32 i_spell_param;
                    switch(*aur)
                    {
                        case SPELL_AURA_PROC_TRIGGER_SPELL: i_spell_param = procFlag;    break;
                        case SPELL_AURA_DUMMY:              i_spell_param = i_spell_eff; break;
                        default: i_spell_param = (*i)->GetModifier()->m_amount;          break;
                    }

                    procTriggered.push_back( ProcTriggeredData(spellProto,i_spell_param,*i) );
                }
            }

            // Handle effects proceed this time
            for(ProcTriggeredList::iterator i = procTriggered.begin(); i != procTriggered.end(); i++)
            {
                if(*aur == SPELL_AURA_PROC_TRIGGER_SPELL)
                {
                    sLog.outDebug("ProcDamageAndSpell: casting spell %u (triggered by an attacker's aura of spell %u)", i->spellInfo->Id,i->triggeredByAura->GetId());
                    HandleProcTriggerSpell(pVictim, damage, i->triggeredByAura, procSpell,i->spellParam);
                }
                else if(*aur == SPELL_AURA_PROC_TRIGGER_DAMAGE)
                {
                    sLog.outDebug("ProcDamageAndSpell: doing %u damage from spell id %u (triggered by an attacker's aura of spell %u)", i->spellParam, i->spellInfo->Id,i->triggeredByAura->GetId());
                    uint32 damage = i->spellParam;
                    // TODO: remove hack for Seal of Righteousness. That should not be there
                    if(i->spellInfo->SpellVisual == 7986)
                        damage = (damage * GetAttackTime(BASE_ATTACK))/60/1000;
                    if(pVictim && pVictim->isAlive())
                        SpellNonMeleeDamageLog(pVictim, i->spellInfo->Id, damage, true, true);
                }
                else if(*aur == SPELL_AURA_DUMMY)
                {
                    // TODO: write a DUMMY aura handle code
                    if (pVictim && pVictim->isAlive())
                    {
                        sLog.outDebug("ProcDamageAndSpell: casting spell id %u (triggered by an attacker dummy aura of spell %u)", i->spellInfo->Id,i->triggeredByAura->GetId());
                        HandleDummyAuraProc(pVictim, i->spellInfo, i->spellParam, damage, i->triggeredByAura, procAttacker);
                    }
                }
            }

            // Safely remove attacker auras with zero charges
            for(AuraList::const_iterator i = attackerAuras.begin(), next; i != attackerAuras.end(); i = next)
            {
                next = i; ++next;
                if((*i)->m_procCharges == 0)
                {
                    RemoveAurasDueToSpell((*i)->GetId());
                    next = attackerAuras.begin();
                }
            }
        }
    }

    // Now go on with a victim's events'n'auras
    // Not much to do if no flags are set or there is no victim
    if(pVictim && pVictim->isAlive() && procVictim)
    {
        for(std::set<uint32>::iterator aur = victimProcAuraTypes.begin(); aur != victimProcAuraTypes.end(); aur++)
        {
            // List of spells (effects) that proceed. Spell prototype and aura-specific value (damage for TRIGGER_DAMAGE)
            ProcTriggeredList procTriggered;

            AuraList const& victimAuras = pVictim->GetAurasByType(*aur);
            for(AuraList::const_iterator i = victimAuras.begin(), next; i != victimAuras.end(); i = next)
            {
                next = i; next++;
                uint32 procFlag = procVictim;

                SpellEntry const *spellProto = (*i)->GetSpellProto();
                if(!spellProto) continue;
                SpellProcEventEntry const *spellProcEvent = objmgr.GetSpellProcEvent(spellProto->Id);

                if(!spellProcEvent && spellProto->procFlags != 0 && nonHandledSpellProcSet.find(spellProto->Id)==nonHandledSpellProcSet.end())
                {
                    sLog.outError("ProcDamageAndSpell: spell %u (victim's aura source) not have record in `spell_proc_event`)",spellProto->Id);
                    nonHandledSpellProcSet.insert(spellProto->Id);
                }

                uint32 procFlags = spellProcEvent ? spellProcEvent->procFlags : spellProto->procFlags;
                if((procFlag & procFlags) == 0)
                    continue;

                // Additional checks in case spell cast/hit/crit is the event
                // Check (if set) school, category, skill line, spell talent mask
                if(spellProcEvent)
                {
                    if(spellProcEvent->schoolMask && (!procSpell || !procSpell->School || ((1<<(procSpell->School-1)) & spellProcEvent->schoolMask) == 0))
                        continue;
                    if(spellProcEvent->category && (!procSpell || procSpell->Category != spellProcEvent->category))
                        continue;
                    if(spellProcEvent->skillId)
                    {
                        if (!procSpell) continue;
                        SkillLineAbilityEntry const *skillLineEntry = sSkillLineAbilityStore.LookupEntry(procSpell->Id);
                        if(!skillLineEntry || skillLineEntry->skillId != spellProcEvent->skillId)
                            continue;
                    }
                    if(spellProcEvent->spellFamilyName && (!procSpell || spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
                        continue;
                    if(spellProcEvent->spellFamilyMask && (!procSpell || (spellProcEvent->spellFamilyMask & procSpell->SpellFamilyFlags) == 0))
                        continue;
                }

                // procChance is exact number in percents anyway
                uint32 chance = spellProto->procChance;
                if(pVictim->GetTypeId() == TYPEID_PLAYER)
                    ((Player*)pVictim)->ApplySpellMod(spellProto->Id,SPELLMOD_CHANCE_OF_SUCCESS,chance);
                if(roll_chance_i(chance))
                {
                    if((*i)->m_procCharges > 0)
                        (*i)->m_procCharges -= 1;

                    uint32 i_spell_eff = (*i)->GetEffIndex();
                    int32 i_spell_param;
                    switch(*aur)
                    {
                        case SPELL_AURA_PROC_TRIGGER_SPELL: i_spell_param = procFlag;    break;
                        case SPELL_AURA_DUMMY:              i_spell_param = i_spell_eff; break;
                        default: i_spell_param = (*i)->GetModifier()->m_amount;          break;
                    }

                    procTriggered.push_back( ProcTriggeredData(spellProto,i_spell_param,*i) );
                }
            }

            // Handle effects proced this time
            for(ProcTriggeredList::iterator i = procTriggered.begin(); i != procTriggered.end(); i++)
            {
                if(*aur == SPELL_AURA_PROC_TRIGGER_SPELL)
                {
                    sLog.outDebug("ProcDamageAndSpell: casting spell %u (triggered by a victim's aura of spell %u))",i->spellInfo->Id, i->triggeredByAura);
                    pVictim->HandleProcTriggerSpell(this, damage, i->triggeredByAura, procSpell,i->spellParam);
                }
                else if(*aur == SPELL_AURA_PROC_TRIGGER_DAMAGE)
                {
                    sLog.outDebug("ProcDamageAndSpell: doing %u damage from spell id %u (triggered by a victim's aura of spell %u))", i->spellParam, i->spellInfo->Id, i->triggeredByAura);
                    pVictim->SpellNonMeleeDamageLog(this, i->spellInfo->Id, i->spellParam, true, true);
                }
                else if(*aur == SPELL_AURA_DUMMY)
                {
                    // TODO: write a DUMMY aura handle code
                    sLog.outDebug("ProcDamageAndSpell: casting spell %u (triggered by a victim's dummy aura of spell %u))",i->spellInfo->Id, i->triggeredByAura);
                    pVictim->HandleDummyAuraProc(this, i->spellInfo, i->spellParam, damage, i->triggeredByAura, procVictim);
                }
            }

            // Safely remove auras with zero charges
            for(AuraList::const_iterator i = victimAuras.begin(), next; i != victimAuras.end(); i = next)
            {
                next = i; ++next;
                if((*i)->m_procCharges == 0)
                {
                    pVictim->RemoveAurasDueToSpell((*i)->GetId());
                    next = victimAuras.begin();
                }
            }
        }
    }
}

void Unit::CastMeleeProcDamageAndSpell(Unit* pVictim, uint32 damage, WeaponAttackType attType, MeleeHitOutcome outcome, SpellEntry const *spellCasted, bool isTriggeredSpell)
{
    if(!pVictim)
        return;

    uint32 procAttacker = PROC_FLAG_NONE;
    uint32 procVictim   = PROC_FLAG_NONE;

    switch(outcome)
    {
        case MELEE_HIT_MISS:
            if(attType == BASE_ATTACK || attType == OFF_ATTACK)
            {
                procAttacker = PROC_FLAG_MISS;
            }
            break;
        case MELEE_HIT_CRIT:
            if(spellCasted && attType == BASE_ATTACK)
            {
                procAttacker |= PROC_FLAG_CRIT_SPELL;
                procVictim   |= PROC_FLAG_STRUCK_CRIT_SPELL;
            }
            else if(attType == BASE_ATTACK || attType == OFF_ATTACK)
            {
                procAttacker = PROC_FLAG_HIT_MELEE | PROC_FLAG_CRIT_MELEE;
                procVictim = PROC_FLAG_STRUCK_MELEE | PROC_FLAG_STRUCK_CRIT_MELEE;
            }
            else
            {
                procAttacker = PROC_FLAG_HIT_RANGED | PROC_FLAG_CRIT_RANGED;
                procVictim = PROC_FLAG_STRUCK_RANGED | PROC_FLAG_STRUCK_CRIT_RANGED;
            }
            break;
        case MELEE_HIT_PARRY:
            procAttacker = PROC_FLAG_TARGET_DODGE_OR_PARRY;
            procVictim = PROC_FLAG_PARRY;
            break;
        case MELEE_HIT_BLOCK:
            procAttacker = PROC_FLAG_TARGET_BLOCK;
            procVictim = PROC_FLAG_BLOCK;
            break;
        case MELEE_HIT_DODGE:
            procAttacker = PROC_FLAG_TARGET_DODGE_OR_PARRY;
            procVictim = PROC_FLAG_DODGE;
            break;
        case MELEE_HIT_CRUSHING:
            if(attType == BASE_ATTACK || attType == OFF_ATTACK)
            {
                procAttacker = PROC_FLAG_HIT_MELEE | PROC_FLAG_CRIT_MELEE;
                procVictim = PROC_FLAG_STRUCK_MELEE | PROC_FLAG_STRUCK_CRIT_MELEE;
            }
            else
            {
                procAttacker = PROC_FLAG_HIT_RANGED | PROC_FLAG_CRIT_RANGED;
                procVictim = PROC_FLAG_STRUCK_RANGED | PROC_FLAG_STRUCK_CRIT_RANGED;
            }
            break;
        default:
            if(attType == BASE_ATTACK || attType == OFF_ATTACK)
            {
                procAttacker = PROC_FLAG_HIT_MELEE;
                procVictim = PROC_FLAG_STRUCK_MELEE;
            }
            else
            {
                procAttacker = PROC_FLAG_HIT_RANGED;
                procVictim = PROC_FLAG_STRUCK_RANGED;
            }
            break;
    }

    if(damage > 0)
        procVictim |= PROC_FLAG_TAKE_DAMAGE;

    if(procAttacker != PROC_FLAG_NONE || procVictim != PROC_FLAG_NONE)
        ProcDamageAndSpell(pVictim, procAttacker, procVictim, damage, spellCasted, isTriggeredSpell, attType);
}

void Unit::HandleDummyAuraProc(Unit *pVictim, SpellEntry const *dummySpell, uint32 effIndex, uint32 damage, Aura* triggredByAura, uint32 procFlag)
{
    switch(dummySpell->Id )
    {
        // Example. Ignite. Though it looks like hack, it isn't )
        case 11119:
        case 11120:
        case 12846:
        case 12847:
        case 12848:
        {
            if(!pVictim)
                return;

            int32 igniteDotBasePoints0;

            switch (dummySpell->Id)
            {
                case 11119: igniteDotBasePoints0=int32(0.04f*damage)-1; break;
                case 11120: igniteDotBasePoints0=int32(0.08f*damage)-1; break;
                case 12846: igniteDotBasePoints0=int32(0.12f*damage)-1; break;
                case 12847: igniteDotBasePoints0=int32(0.16f*damage)-1; break;
                case 12848: igniteDotBasePoints0=int32(0.20f*damage)-1; break;
                default:
                    sLog.outError("Unit::HandleDummyAuraProc: non handled spell id: %u (IG)",dummySpell->Id);
                    return;
            };
            CastCustomSpell(pVictim, 12654, &igniteDotBasePoints0, NULL, NULL, true, NULL, triggredByAura);
            return;
        }

        // Combustion
        case 11129:
            {
                CastSpell(this, 28682, true, NULL, triggredByAura);
                if (!(procFlag & PROC_FLAG_CRIT_SPELL))         //no crit
                    triggredByAura->m_procCharges += 1;         //-> reincrease procCharge count since it was decreased before
                else if (triggredByAura->m_procCharges == 0)    //no more charges left and crit
                    RemoveAurasDueToSpell(28682);               //-> remove Combustion auras
                return;
            }

        // VE
        case 15286:
            if(!pVictim)
                return;

            if(triggredByAura->GetCasterGUID() == pVictim->GetGUID())
            {
                int32 VEHealBasePoints0 = triggredByAura->GetModifier()->m_amount*damage/100; //VEHeal has a BaseDice of 0, so no decrement needed
                pVictim->CastCustomSpell(pVictim, 15290, &VEHealBasePoints0, NULL, NULL, true, NULL, triggredByAura);
            }
            return;

        // Eye of Eye
        case 9799:
        case 25988:
        {
            if(!pVictim)
                return;

            // return damage % to attacker but < 50% own total health
            uint32 backDamage = triggredByAura->GetModifier()->m_amount*damage/100;
            if(backDamage > GetMaxHealth()/2)
                backDamage = GetMaxHealth()/2;

            int32 YYDamageBasePoints0 = backDamage-1;
            CastCustomSpell(pVictim, 25997, &YYDamageBasePoints0, NULL, NULL, true, NULL, triggredByAura);

            return;
        }

        // Spiritual Att.
        case 33776:
        case 31785:
        {
            if(!pVictim)
                return;

            // if healed by another unit (pVictim)
            if(this != pVictim)
            {
                int32 SAHealBasePoints0 = triggredByAura->GetModifier()->m_amount*damage/100-1;
                CastCustomSpell(this, 31786, &SAHealBasePoints0, NULL, NULL, true, NULL, triggredByAura);
            }

            return;
        }

        default: break;
    }

    // Non SpellID checks

    // VT
    if (dummySpell->SpellIconID == 2213)
    {
        if(!pVictim)
            return;

        if(triggredByAura->GetCasterGUID() == pVictim->GetGUID())
        {
            int32 VTEnergizeBasePoints0 = triggredByAura->GetModifier()->m_amount*damage/100 - 1;
            pVictim->CastCustomSpell(pVictim,34919,&VTEnergizeBasePoints0,NULL,NULL,true,NULL, triggredByAura);
        }
        return;
    }
}

void Unit::HandleProcTriggerSpell(Unit *pVictim, uint32 damage, Aura* triggredByAura, SpellEntry const *procSpell, uint32 procFlags)
{
    SpellEntry const* auraSpellInfo = triggredByAura->GetSpellProto();

    switch(auraSpellInfo->SpellIconID)
    {
        case 19:
            //Lightning Shield (Shaman T2 bonus)
            //Effect: 23552
            /*if (pVictim && pVictim->isAlive())
                CastSpell(pVictim, 23552, true, NULL, triggredByAura);*/
            return;
        case 87:
        {
            //Mana Surge (Shaman T1 bonus)
            //Effect: 23571
            int32 manaSurgeSpellBasePoints0 = procSpell->manaCost * 35/100;
            CastCustomSpell(this, 23571, &manaSurgeSpellBasePoints0, NULL, NULL, true, NULL, triggredByAura);
            return;
        }    
        case 113:
        {
            //Improved Drain Soul
            //Effect: 18371
            Unit::AuraList const& mAddFlatModifier = GetAurasByType(SPELL_AURA_ADD_FLAT_MODIFIER);
            for(Unit::AuraList::const_iterator i = mAddFlatModifier.begin(); i != mAddFlatModifier.end(); ++i)
            {
                if ((*i)->GetModifier()->m_miscvalue == SPELLMOD_CHANCE_OF_SUCCESS && (*i)->GetSpellProto()->SpellIconID == 113)
                {
                    int32 impDrainSoulBasePoints0 = (*i)->GetSpellProto()->EffectBasePoints[2] * GetMaxPower(POWER_MANA) / 100;
                    CastCustomSpell(this, 18371, &impDrainSoulBasePoints0, NULL, NULL, true, NULL, triggredByAura);
                }
            }
            return;
        }
        case 241:
        {
            switch(auraSpellInfo->EffectTriggerSpell[0])
            {
                //Illumination
                case 18350:
                {
                    // must be HLight || HShock (triggered) || FlashOL
                    if( (procSpell->SpellVisual != 2936 || procSpell->SpellIconID != 70 ) && 
                        (procSpell->SpellVisual != 135  || procSpell->SpellIconID != 156) &&
                        procSpell->SpellVisual != 6623 )
                        return;

                    SpellEntry const *originalSpell = procSpell;

                    // in case HShock procspell is triggered spell but we need mana cost of original casted spell
                    if(procSpell->SpellVisual == 135 && procSpell->SpellIconID == 156)
                    {
                        uint32 originalSpellId = 0;
                        switch(procSpell->Id)
                        {
                            case 25914: originalSpellId = 20473; break;
                            case 25913: originalSpellId = 20929; break;
                            case 25903: originalSpellId = 20930; break;
                            case 27175: originalSpellId = 27174; break;
                            case 33074: originalSpellId = 33072; break;
                            default: 
                                sLog.outError("Unit::HandleProcTriggerSpell: Spell %u not handled in HShock",procSpell->Id);
                                return;
                        }
                        SpellEntry const *HSSpell= sSpellStore.LookupEntry(originalSpellId);
                        if(!HSSpell)
                        {
                            sLog.outError("Unit::HandleProcTriggerSpell: Spell %u unknown but used in HShock",originalSpellId);
                            return;
                        }
                        originalSpell = HSSpell;
                    }

                    // BasePoints = val -1 not required (EffectBaseDice==0)
                    int32 ILManaSpellBasePoints0 = originalSpell->manaCost;
                    CastCustomSpell(this, 20272, &ILManaSpellBasePoints0, NULL, NULL, true, NULL, triggredByAura);
                    return;
                }
            }
        }
        case 312:
        {
            //Improved Leader of the Pack
            //Effect 34299
            //Cooldown: 6 secs
            if (triggredByAura->GetModifier()->m_amount == 0)
                break;
            int32 improvedLotPBasePoints0 = triggredByAura->GetModifier()->m_amount * GetMaxHealth() / 100 - 1;
            CastCustomSpell(this, 34299, &improvedLotPBasePoints0, NULL, NULL, true, NULL, triggredByAura);
            if (GetTypeId() == TYPEID_PLAYER)
                ((Player*)this)->AddSpellCooldown(34299,0,time(NULL) + 6);
            return;
        }
        case 1137:
        {
            //Pyroclasm
            //Effect: 18093
            float chance = 0;
            switch (triggredByAura->GetSpellProto()->Id)
            {
                case 18096:
                    chance = 13.0;
                    break;
                case 18073:
                    chance = 26.0;
                    break;
            }
            if (pVictim && pVictim->isAlive() && roll_chance_f(chance))
                CastSpell(pVictim, 18093, true, NULL, triggredByAura);
            return;
        }
        case 1875:
        {
            //Blessed Recovery
            uint32 EffectId = 0;
            switch (triggredByAura->GetSpellProto()->Id)
            {
                case 27811: EffectId = 27813; break;
                case 27815: EffectId = 27817; break;
                case 27816: EffectId = 27818; break;
                default: 
                    sLog.outError("Unit::HandleProcTriggerSpell: Spell %u not handled in BR",procSpell->Id);
                    return;
            }

            int32 heal_amount = damage * triggredByAura->GetModifier()->m_amount / 100;
            int32 BRHealBasePoints0 = heal_amount/3-1;
            CastCustomSpell(this, EffectId, &BRHealBasePoints0, NULL, NULL, true, NULL, triggredByAura);
            return;
        }
        case 2006:
            //Rampage
            //Effects: 30029(Rank 1), 30031(Rank 2), 30032(Rank 3)
            //Check EffectTriggerSpell[1] to determine correct effect id
            //CastSpell(this, triggredByAura->GetSpellProto()->EffectTriggerSpell[1], true, NULL, triggredByAura);
            return;
        case 2013:
        {
            //Nature's Guardian
            //Effects: 31616, 39301
            //Cooldown: 5 secs
            /*float HealthRatio = GetHealth() / GetMaxHealth();
            float HealthRatioBefore = (GetHealth() + damage) / GetMaxHealth();
            if (HealthRatio < 0.3 && HealthRatioBefore >= 0.3)
            {
                SpellEntry const *NGHealTemplate = sSpellStore.LookupEntry(31616);
                SpellEntry NGHeal = *NGHealTemplate;
                NGHeal.EffectBasePoints[0] = triggredByAura->GetModifier()->m_amount * GetMaxHealth() / 100;
                CastSpell(this, &NGHeal, true, NULL, triggredByAura);
                if (pVictim && pVictim->isAlive())
                    CastSpell(pVictim, 39301, true, NULL, triggredByAura);
                if (GetTypeId() == TYPEID_PLAYER)
                {
                    ((Player*)this)->AddSpellCooldown(31616,0,time(NULL) + 5);
                    ((Player*)this)->AddSpellCooldown(39301,0,time(NULL) + 5);
                }
            }*/
            return;
        }
        case 2127:
            //Blazing Speed
            //Effect: 31643
            CastSpell(this, 31643, true, NULL, triggredByAura);
            return;
    }

    // custom check for proc spell
    switch(auraSpellInfo->Id)
    {
    // Impr. Countercasting
    case 11255:
    case 12598:
        // if Countercasting spell casted and target also cating in this moment.
        if(procSpell->SpellVisual==239 && pVictim && pVictim->m_currentSpell)
            break;                                          // fall through to std. handler
        return;
    }

    // standard non-dummy case
    uint32 trigger_spell_id = auraSpellInfo->EffectTriggerSpell[triggredByAura->GetEffIndex()];
    if(!trigger_spell_id)
    {
        sLog.outError("Unit::HandleProcTriggerSpell: Spell %u have 0 in EffectTriggered[%d], not handled custom case?",auraSpellInfo->Id,triggredByAura->GetEffIndex());
        return;
    }

    if(IsPositiveSpell(trigger_spell_id) && !(procFlags & PROC_FLAG_HEAL))
        CastSpell(this,trigger_spell_id,true,NULL,triggredByAura);
    else if(pVictim && pVictim->isAlive())
        CastSpell(pVictim,trigger_spell_id,true,NULL,triggredByAura);
}

void Unit::setPowerType(Powers new_powertype)
{
    uint32 tem_bytes_0 = GetUInt32Value(UNIT_FIELD_BYTES_0);
    SetUInt32Value(UNIT_FIELD_BYTES_0,((tem_bytes_0<<8)>>8) + (uint32(new_powertype)<<24));

    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POWER_TYPE);

    switch(new_powertype)
    {
        default:
        case POWER_MANA:
            break;
        case POWER_RAGE:
            SetMaxPower(POWER_RAGE,1000);
            SetPower(   POWER_RAGE,0);
            break;
        case POWER_FOCUS:
            SetMaxPower(POWER_FOCUS,100);
            SetPower(   POWER_FOCUS,100);
            break;
        case POWER_ENERGY:
            SetMaxPower(POWER_ENERGY,100);
            SetPower(   POWER_ENERGY,0);
            break;
        case POWER_HAPPINESS:
            SetMaxPower(POWER_HAPPINESS,1000000);
            SetPower(POWER_HAPPINESS,1000000);
            break;
    }
}

FactionTemplateEntry const* Unit::getFactionTemplateEntry() const
{
    FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(getFaction());
    if(!entry)
    {
        static uint64 guid = 0;                             // prevent repeating spam same faction problem

        if(GetGUID() != guid)
        {
            if(GetTypeId() == TYPEID_PLAYER)
                sLog.outError("Player %s have invalid faction (faction template id) #%u", ((Player*)this)->GetName(), getFaction());
            else
                sLog.outError("Creature (template id: %u) have invalid faction (faction template id) #%u", ((Creature*)this)->GetCreatureInfo()->Entry, getFaction());
            guid = GetGUID();
        }
    }
    return entry;
}

bool Unit::IsHostileTo(Unit const* unit) const
{
    // always non-hostile to self
    if(unit==this)
        return false;

    // always hostile to enemy
    if(getVictim()==unit || unit->getVictim()==this)
        return true;

    // test pet/charm masters instead pers/charmeds
    Unit const* testerOwner = GetCharmerOrOwner();
    Unit const* targetOwner = unit->GetCharmerOrOwner();

    // always hostile to owner's enemy
    if(testerOwner && (testerOwner->getVictim()==unit || unit->getVictim()==testerOwner))
        return true;

    // always hostile to enemy owner
    if(targetOwner && (getVictim()==targetOwner || targetOwner->getVictim()==this))
        return true;

    // always hostile to owner of owner's enemy
    if(testerOwner && targetOwner && (testerOwner->getVictim()==targetOwner || targetOwner->getVictim()==testerOwner))
        return true;

    Unit const* tester = testerOwner ? testerOwner : this;
    Unit const* target = targetOwner ? targetOwner : unit;

    // special cases (Duel, etc)
    if(tester->GetTypeId()==TYPEID_PLAYER && target->GetTypeId()==TYPEID_PLAYER)
    {
        // Duel
        if(((Player const*)tester)->duel && ((Player const*)tester)->duel->opponent == target && ((Player const*)tester)->duel->startTime != 0)
            return true;

        //= PvP states
        // Green/Blue (can't attack)
        if(((Player*)tester)->GetTeam()==((Player*)target)->GetTeam())
            return false;

        // Red (can attack) if true, Blue/Yellow (can't attack) in another case
        return ((Player*)tester)->IsPvP() && ((Player*)target)->IsPvP();
    }

    // faction base cases
    FactionTemplateEntry const*tester_faction = tester->getFactionTemplateEntry();
    FactionTemplateEntry const*target_faction = target->getFactionTemplateEntry();
    if(!tester_faction || !target_faction)
        return false;

    // PvC forced reaction and reputation case
    if(tester->GetTypeId()==TYPEID_PLAYER)
    {
        // apply forced faction only in target with identical faction in other case provided original faction
        if(tester->HasAuraType(SPELL_AURA_FORCE_REACTION) && tester->getFaction()!= target_faction->ID)
        {
            FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(Player::getFactionForRace(tester->getRace()));
            if(entry)
                tester_faction = entry;
        }

        // apply reputation state
        FactionEntry const* raw_target_faction = sFactionStore.LookupEntry(target_faction->faction);
        if(raw_target_faction && raw_target_faction->reputationListID >=0 )
        {
            if(((Player*)tester)->IsFactionAtWar(raw_target_faction))
                return true;
        }
    }
    // CvP forced reaction and reputation case
    else if(target->GetTypeId()==TYPEID_PLAYER)
    {
        // apply forced faction only in target with identical faction in other case provided original faction
        if(target->HasAuraType(SPELL_AURA_FORCE_REACTION) && target->getFaction()!= tester_faction->ID)
        {
            FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(Player::getFactionForRace(target->getRace()));
            if(entry)
                target_faction = entry;
        }

        // apply reputation state
        FactionEntry const* raw_tester_faction = sFactionStore.LookupEntry(tester_faction->faction);
        if(raw_tester_faction && raw_tester_faction->reputationListID >=0 )
        {
            return ((Player*)target)->GetReputationRank(raw_tester_faction) <= REP_HOSTILE;
        }
    }     

    // common faction based case (CvC,PvC,CvP)
    return tester_faction->IsHostileTo(*target_faction);
}

bool Unit::IsFriendlyTo(Unit const* unit) const
{
    // always friendly to self
    if(unit==this)
        return true;

    // always non-friendly to enemy
    if(getVictim()==unit || unit->getVictim()==this)
        return false;

    // test pet/charm masters instead pers/charmeds
    Unit const* testerOwner = GetCharmerOrOwner();
    Unit const* targetOwner = unit->GetCharmerOrOwner();

    // always non-friendly to owner's enemy
    if(testerOwner && (testerOwner->getVictim()==unit || unit->getVictim()==testerOwner))
        return false;

    // always non-friendly to enemy owner
    if(targetOwner && (getVictim()==targetOwner || targetOwner->getVictim()==this))
        return false;

    // always non-friendly to owner of owner's enemy
    if(testerOwner && targetOwner && (testerOwner->getVictim()==targetOwner || targetOwner->getVictim()==testerOwner))
        return false;

    Unit const* tester = testerOwner ? testerOwner : this;
    Unit const* target = targetOwner ? targetOwner : unit;

    // special cases (Duel)
    if(tester->GetTypeId()==TYPEID_PLAYER && target->GetTypeId()==TYPEID_PLAYER)
    {
        // Duel
        if(((Player const*)tester)->duel && ((Player const*)tester)->duel->opponent == target && ((Player const*)tester)->duel->startTime != 0)
            return false;

        //= PvP states
        // Green/Blue (non-attackable)
        if(((Player*)tester)->GetTeam()==((Player*)target)->GetTeam())
            return true;

        // Blue (friendly/non-attackable) if not PVP, or Yellow/Red in another case (attackable)
        return !((Player*)target)->IsPvP();
    }

    // faction base cases
    FactionTemplateEntry const*tester_faction = tester->getFactionTemplateEntry();
    FactionTemplateEntry const*target_faction = target->getFactionTemplateEntry();
    if(!tester_faction || !target_faction)
        return false;

    // PvC forced reaction and reputation case
    if(tester->GetTypeId()==TYPEID_PLAYER)
    {
        // apply forced faction only in target with identical faction in other case provided original faction
        if(tester->HasAuraType(SPELL_AURA_FORCE_REACTION) && tester->getFaction()!= target_faction->ID)
        {
            FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(Player::getFactionForRace(tester->getRace()));
            if(entry)
                tester_faction = entry;
        }

        // apply reputation state
        FactionEntry const* raw_target_faction = sFactionStore.LookupEntry(target_faction->faction);
        if(raw_target_faction && raw_target_faction->reputationListID >=0 )
        {
            if(((Player*)tester)->IsFactionAtWar(raw_target_faction))
                return false;
        }
    }
    // CvP forced reaction and reputation case
    else if(target->GetTypeId()==TYPEID_PLAYER)
    {
        // apply forced faction only in target with identical faction in other case provided original faction
        if(target->HasAuraType(SPELL_AURA_FORCE_REACTION) && target->getFaction()!= tester_faction->ID)
        {
            FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(Player::getFactionForRace(target->getRace()));
            if(entry)
                target_faction = entry;
        }

        // apply reputation state
        FactionEntry const* raw_tester_faction = sFactionStore.LookupEntry(tester_faction->faction);
        if(raw_tester_faction && raw_tester_faction->reputationListID >=0 )
        {
            return ((Player*)target)->GetReputationRank(raw_tester_faction) >= REP_FRIENDLY;
        }
    }     

    // common faction based case (CvC,PvC,CvP)
    return tester_faction->IsFriendlyTo(*target_faction);
}

bool Unit::IsHostileToPlayer() const
{
    FactionTemplateEntry const* my_faction = getFactionTemplateEntry();
    if(!my_faction)
        return false;

    return my_faction->IsHostileToPlayer();
}

bool Unit::IsNeutralToAll() const
{
    FactionTemplateEntry const* my_faction = getFactionTemplateEntry();
    if(!my_faction)
        return true;

    return my_faction->IsNeutralToAll();
}

bool Unit::Attack(Unit *victim, bool playerMeleeAttack)
{
    if(!victim || victim == this)
        return false;

    // player don't must attack in mount state
    if(GetTypeId()==TYPEID_PLAYER && IsMounted())
        return false;

    // anyone don't must attack GM in GM-mode
    if(victim->GetTypeId()==TYPEID_PLAYER && ((Player*)victim)->isGameMaster())
        return false;

    if (m_attacking)
    {
        if (m_attacking == victim)
            return false;
        AttackStop();
    }

    //Set our target
    SetUInt64Value(UNIT_FIELD_TARGET, victim->GetGUID());

    addUnitState(UNIT_STAT_ATTACKING);
    SetInCombat();
    m_attacking = victim;
    m_attacking->_addAttacker(this);

    if( GetTypeId()==TYPEID_UNIT && !GetOwnerGUID() )
    {
        ((Creature*)this)->CallAssistence();
    }
    //if(!isAttackReady(BASE_ATTACK))
    //resetAttackTimer(BASE_ATTACK);

    // delay offhand weapon attack to next attack time
    if(haveOffhandWeapon())
        resetAttackTimer(OFF_ATTACK);

    if(playerMeleeAttack)
        SendAttackStart(victim);

    return true;
}

bool Unit::AttackStop()
{
    if (!m_attacking)
        return false;

    Unit* victim = m_attacking;

    m_attacking->_removeAttacker(this);
    m_attacking = NULL;

    //Clear our target
    SetUInt64Value(UNIT_FIELD_TARGET, 0);

    clearUnitState(UNIT_STAT_ATTACKING);

    if(m_currentMeleeSpell)
        m_currentMeleeSpell->cancel();

    if( GetTypeId()==TYPEID_UNIT )
    {
        // reset call assistance
        ((Creature*)this)->SetNoCallAssistence(false);
    }

    SendAttackStop(victim);

    return true;
}

bool Unit::isAttackingPlayer() const
{
    if(getVictim())
    {
        if(getVictim()->GetTypeId() == TYPEID_PLAYER)
            return true;

        if(getVictim()->GetOwnerGUID() && GUID_HIPART(getVictim()->GetOwnerGUID())==HIGHGUID_PLAYER)
            return true;
    }

    Pet* pet = GetPet();
    if(pet && pet->isAttackingPlayer())
        return true;

    Unit* charmed = GetCharm();
    if(charmed && charmed->isAttackingPlayer())
        return true;

    for (int8 i = 0; i < 4; i++)
    {
        if(m_TotemSlot[i])
        {
            Creature *totem = ObjectAccessor::Instance().GetCreature(*this, m_TotemSlot[i]);
            if(totem && totem->isAttackingPlayer())
                return true;
        }
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

void Unit::ModifyAuraState(uint32 flag, bool apply)
{
    if (apply)
    {
        if (!HasFlag(UNIT_FIELD_AURASTATE, 1<<(flag-1)))
        {
            SetFlag(UNIT_FIELD_AURASTATE, 1<<(flag-1));
            if(GetTypeId() == TYPEID_PLAYER)
            {
                const PlayerSpellMap& sp_list = ((Player*)this)->GetSpellMap();
                for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
                {
                    if(itr->second->state == PLAYERSPELL_REMOVED) continue;
                    SpellEntry const *spellInfo = sSpellStore.LookupEntry(itr->first);
                    if (!spellInfo || !IsPassiveSpell(itr->first)) continue;
                    if (spellInfo->CasterAuraState == flag)
                        CastSpell(this, itr->first, true, NULL);
                }
            }
        }
    }
    else
    {
        if (HasFlag(UNIT_FIELD_AURASTATE,1<<(flag-1)))
        {
            RemoveFlag(UNIT_FIELD_AURASTATE, 1<<(flag-1));
            Unit::AuraMap& tAuras = GetAuras();
            for (Unit::AuraMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
            {
                if ((*itr).second->GetSpellProto()->CasterAuraState == flag)
                    RemoveAura(itr);
                else
                    ++itr;
            }
        }
    }
}

Unit *Unit::GetOwner() const
{
    uint64 ownerid = GetOwnerGUID();
    if(!ownerid)
        return NULL;
    return ObjectAccessor::Instance().GetUnit(*this, ownerid);
}

Unit *Unit::GetCharmer() const
{
    uint64 charmerid = GetCharmerGUID();
    if(!charmerid)
        return NULL;
    return ObjectAccessor::Instance().GetUnit(*this, charmerid);
}

Pet* Unit::GetPet() const
{
    uint64 pet_guid = GetPetGUID();
    if(pet_guid)
    {
        Pet* pet = ObjectAccessor::Instance().GetPet(pet_guid);
        if(!pet)
        {
            sLog.outError("Unit::GetPet: Pet %u not exist.",GUID_LOPART(pet_guid));
            const_cast<Unit*>(this)->SetPet(0);
            return NULL;
        }
        return pet;
    }

    return NULL;
}

Unit* Unit::GetCharm() const
{
    uint64 charm_guid = GetCharmGUID();
    if(charm_guid)
    {
        Unit* pet = ObjectAccessor::Instance().GetUnit(*this, charm_guid);
        if(!pet)
        {
            sLog.outError("Unit::GetCharm: Charmed creature %u not exist.",GUID_LOPART(charm_guid));
            const_cast<Unit*>(this)->SetCharm(0);
        }
        return pet;
    }
    else
        return NULL;
}

void Unit::SetPet(Pet* pet)
{
    SetUInt64Value(UNIT_FIELD_SUMMON,pet ? pet->GetGUID() : 0);

    if(pet)
    {
        for(int i = 0; i < MAX_MOVE_TYPE; ++i)
        {
            pet->SetSpeed(UnitMoveType(i),m_speed_rate[i],true);
        }
    }
}

void Unit::SetCharm(Unit* charmed)
{
    SetUInt64Value(UNIT_FIELD_CHARM,charmed ? charmed->GetGUID() : 0);
}

void Unit::UnsummonAllTotems()
{
    for (int8 i = 0; i < 4; ++i)
    {
        if(!m_TotemSlot[i])
            continue;

        Creature *OldTotem = ObjectAccessor::Instance().GetCreature(*this, m_TotemSlot[i]);
        if (OldTotem && OldTotem->isTotem()) 
            ((Totem*)OldTotem)->UnSummon();
    }
}

void Unit::SendHealSpellOnPlayer(Unit *pVictim, uint32 SpellID, uint32 Damage, bool critical)
{
    // we guess size
    WorldPacket data(SMSG_HEALSPELL_ON_PLAYER_OBSOLETE, (8+8+4+4+1));
    data.append(pVictim->GetPackGUID());
    data.append(GetPackGUID());
    data << SpellID;
    data << Damage;
    data << uint8(critical ? 1 : 0);
    SendMessageToSet(&data, true);
}

void Unit::SendHealSpellOnPlayerPet(Unit *pVictim, uint32 SpellID, uint32 Damage,Powers powertype, bool critical)
{
    WorldPacket data(SMSG_HEALSPELL_ON_PLAYERS_PET_OBSOLETE, (8+8+4+4+4+1));
    data.append(pVictim->GetPackGUID());
    data.append(GetPackGUID());
    data << SpellID;
    data << uint32(powertype);
    data << Damage;
    data << uint8(critical ? 1 : 0);
    SendMessageToSet(&data, true);
}

uint32 Unit::SpellDamageBonus(Unit *pVictim, SpellEntry const *spellProto, uint32 pdamage, DamageEffectType damagetype)
{
    if(!spellProto || !pVictim || damagetype==DIRECT_DAMAGE )
        return pdamage;

    if(pVictim->IsImmunedToSpellDamage(spellProto))
        return 0;

    CreatureInfo const *cinfo = NULL;
    if(pVictim->GetTypeId() != TYPEID_PLAYER)
        cinfo = ((Creature*)pVictim)->GetCreatureInfo();

    // Damage Done
    uint32 CastingTime = GetCastTime(sCastTimesStore.LookupEntry(spellProto->CastingTimeIndex));
    if (CastingTime > 7000) CastingTime = 7000;             // Plus Damage efficient maximum 200% ( 7.0 seconds )
    if (CastingTime < 1500) CastingTime = 1500;

    // Taken/Done fixed damage bonus auras
    int32 DoneAdvertisedBenefit = 0;
    int32 TakenAdvertisedBenefit = 0;

    // ..done (for creature type by mask) in taken
    AuraList const& mDamageDoneCreature = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE_CREATURE);
    for(AuraList::const_iterator i = mDamageDoneCreature.begin();i != mDamageDoneCreature.end(); ++i)
        if(cinfo && cinfo->type && ((1 << (cinfo->type-1)) & uint32((*i)->GetModifier()->m_miscvalue)))
            TakenAdvertisedBenefit += (*i)->GetModifier()->m_amount;

    // ..done
    AuraList const& mDamageDone = this->GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE);
    for(AuraList::const_iterator i = mDamageDone.begin();i != mDamageDone.end(); ++i)
        if(((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
            DoneAdvertisedBenefit += (*i)->GetModifier()->m_amount;

    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Damage bonus of spirit
        AuraList const& mDamageDonebySpi = GetAurasByType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_SPIRIT);
        for(AuraList::const_iterator i = mDamageDonebySpi.begin();i != mDamageDonebySpi.end(); ++i)
            if((*i)->GetModifier()->m_miscvalue & 1 << spellProto->School)
                DoneAdvertisedBenefit += int32(GetStat(STAT_SPIRIT) * (*i)->GetModifier()->m_amount / 100.0f);
    
        // ... and intellect
        AuraList const& mDamageDonebyInt = GetAurasByType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_INTELLECT);
        for(AuraList::const_iterator i = mDamageDonebyInt.begin();i != mDamageDonebyInt.end(); ++i)
            if ((*i)->GetModifier()->m_miscvalue & 1 << spellProto->School)
                DoneAdvertisedBenefit += int32(GetStat(STAT_INTELLECT) * (*i)->GetModifier()->m_amount / 100.0f);
    }

    // ..taken
    AuraList const& mDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_TAKEN);
    for(AuraList::const_iterator i = mDamageTaken.begin();i != mDamageTaken.end(); ++i)
        if(((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
            TakenAdvertisedBenefit += (*i)->GetModifier()->m_amount;

    // Damage over Time spells bonus calculation
    float DotFactor = 1.0f;
    if(damagetype == DOT)
    {
        CastingTime = 3500;
        uint32 DotDuration = GetDuration(spellProto);
        // 200% limit
        if(DotDuration > 0)
        {
            if(DotDuration > 30000) DotDuration = 30000;
            DotFactor = DotDuration / 15000.0f;
            int x = 0;
            for(int j = 0; j < 3; j++)
                if(spellProto->Effect[j] == 6) x = j;
            int DotTicks = 6;
            if(spellProto->EffectAmplitude[x] != 0)
                DotTicks = DotDuration / spellProto->EffectAmplitude[x];
            if(DotTicks)
            {
                DoneAdvertisedBenefit /= DotTicks;
                TakenAdvertisedBenefit /= DotTicks;
            }
        }
    }

    // Taken/Done total percent damage auras
    float DoneTotalMod = 1.0f;
    float TakenTotalMod = 1.0f;

    // ..done
    AuraList const& mModDamagePercentDone = this->GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for(AuraList::const_iterator i = mModDamagePercentDone.begin(); i != mModDamagePercentDone.end(); ++i)
        if( spellProto->School != 0 && ((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0 )
            DoneTotalMod *= ((*i)->GetModifier()->m_amount+100.0f)/100.0f;

    // ..taken
    AuraList const& mModDamagePercentTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    for(AuraList::const_iterator i = mModDamagePercentTaken.begin(); i != mModDamagePercentTaken.end(); ++i)
        if( spellProto->School != 0 && ((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0 )
            TakenTotalMod *= ((*i)->GetModifier()->m_amount+100.0f)/100.0f;

    // Exceptions
    // Lifetap
    if(spellProto->SpellVisual == 1225 && spellProto->SpellIconID == 208)
    {
        CastingTime = 2800; // 80% from +shadow damage
        DoneTotalMod = 1.0f;
        TakenTotalMod = 1.0f;
    }
    // Dark Pact
    if(spellProto->SpellVisual == 827 && spellProto->SpellIconID == 154 && GetPet())
    {
        CastingTime = 3360; // 96% from +shadow damage
        DoneTotalMod = 1.0f;
        TakenTotalMod = 1.0f;
    }

    // Level Factor
    float LvlPenalty = 0.0f;
    if(spellProto->spellLevel < 20)
        LvlPenalty = (20.0f - (float)(spellProto->spellLevel)) * 3.75f;
    float LvlFactor = ((float)(spellProto->spellLevel) + 6.0f) / (float)(getLevel());
    if(LvlFactor > 1.0f)
        LvlFactor = 1.0f;

    // Spellmod SpellDamage
    float SpellModSpellDamage = 100.0f;
    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->ApplySpellMod(spellProto->Id,SPELLMOD_SPELL_DAMAGE,SpellModSpellDamage);
    SpellModSpellDamage /= 100.0f;

    float DoneActualBenefit = DoneAdvertisedBenefit * (CastingTime / 3500.0f) * (100.0f - LvlPenalty) * LvlFactor * DotFactor * SpellModSpellDamage / 100.0f;
    float TakenActualBenefit = TakenAdvertisedBenefit * (CastingTime / 3500.0f) * (100.0f - LvlPenalty) * LvlFactor * DotFactor / 100.0f;

    float tmpDamage = (float(pdamage)+DoneActualBenefit)*DoneTotalMod;
    tmpDamage = (tmpDamage+TakenActualBenefit)*TakenTotalMod;

    return tmpDamage > 0 ? uint32(tmpDamage) : 0;
}

bool Unit::SpellCriticalBonus(SpellEntry const *spellProto, int32 *peffect, Unit *pVictim)
{
    // Chance to crit is computed from INT and LEVEL as follows:
    //   chance = base + INT / (rate0 + rate1 * LEVEL)
    // The formula keeps the crit chance at %5 on every level unless the player
    // increases his intelligence by other means (enchants, buffs, talents, ...)
    if(spellProto->Id == 15290 || spellProto->Id == 39373) return false;

    float crit_chance = 0.0f;

    // base value
    if (GetTypeId() != TYPEID_PLAYER)
    {
        // flat done
        // TODO: can creatures have critical chance auras?
        crit_chance = m_baseSpellCritChance;
        AuraList const& mSpellCritSchool = GetAurasByType(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL);
        for(AuraList::const_iterator i = mSpellCritSchool.begin(); i != mSpellCritSchool.end(); ++i)
            if((*i)->GetModifier()->m_miscvalue == -2 || ((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
                crit_chance += (*i)->GetModifier()->m_amount;
    }
    else
        crit_chance = GetFloatValue( PLAYER_SPELL_CRIT_PERCENTAGE1 + spellProto->School);

    // percent done
    // only players use intelligence for critical chance computations
    if (GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)this)->ApplySpellMod(spellProto->Id, SPELLMOD_CRITICAL_CHANCE, crit_chance);
    }


    // taken
    if (pVictim)
    {
        // flat
        AuraList const& mAttackerSpellCrit = pVictim->GetAurasByType(SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE);
        for(AuraList::const_iterator i = mAttackerSpellCrit.begin(); i != mAttackerSpellCrit.end(); ++i)
            if((*i)->GetModifier()->m_miscvalue == -2 || ((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
                crit_chance += (*i)->GetModifier()->m_amount;

        // flat: Resilience - reduce crit chance by x%
        crit_chance -= pVictim->m_modResilience;

        // percent
        AuraList const& mAttackerSpellCritPCT = pVictim->GetAurasByType(SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE_PCT);
        for(AuraList::const_iterator i = mAttackerSpellCritPCT.begin(); i != mAttackerSpellCritPCT.end(); ++i)
            if((*i)->GetModifier()->m_miscvalue == -2 || ((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
                crit_chance += crit_chance * (*i)->GetModifier()->m_amount/1000;

    }

    crit_chance = crit_chance > 0.0 ? crit_chance : 0.0;
    if (roll_chance_f(crit_chance))
    {
        int32 crit_bonus = *peffect / 2;
        if (GetTypeId() == TYPEID_PLAYER)                   // adds additional damage to crit_bonus (from talents)
            ((Player*)this)->ApplySpellMod(spellProto->Id, SPELLMOD_CRIT_DAMAGE_BONUS, crit_bonus);
        
        *peffect += crit_bonus;
        // Resilience - reduce crit damage by 2x%
        if (pVictim)
            *peffect -= int32(pVictim->m_modResilience * 2/100 * (*peffect));

        return true;
    }
    return false;
}

uint32 Unit::SpellHealingBonus(SpellEntry const *spellProto, uint32 healamount, DamageEffectType damagetype, Unit *pVictim)
{
    // Healing Done

    // Vampiric Embrace, Shadowmend - cannot critically heal
    if(spellProto->Id == 15290 || spellProto->Id == 39373) return healamount;

    int32 AdvertisedBenefit = 0;
    uint32 CastingTime = GetCastTime(sCastTimesStore.LookupEntry(spellProto->CastingTimeIndex));
    if (CastingTime > 7000) CastingTime = 7000;
    if (CastingTime < 1500) CastingTime = 1500;
    if (spellProto->Effect[0] == SPELL_EFFECT_APPLY_AURA) CastingTime = 3500;

    AuraList const& mHealingDone = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE);
    for(AuraList::const_iterator i = mHealingDone.begin();i != mHealingDone.end(); ++i)
        if(((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
            AdvertisedBenefit += (*i)->GetModifier()->m_amount;

    // Healing bonus of spirit, intellect and strength
    if (GetTypeId() == TYPEID_PLAYER)
    {
        AdvertisedBenefit += int32(GetStat(STAT_SPIRIT) * GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HEALING_OF_SPIRIT) / 100.0f);
        AdvertisedBenefit += int32(GetStat(STAT_INTELLECT) * GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HEALING_OF_INTELLECT) / 100.0f);
        AdvertisedBenefit += int32(GetStat(STAT_STRENGTH) * GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HEALING_OF_STRENGTH) / 100.0f);
    }

    // Healing Taken
    AdvertisedBenefit += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_HEALING);

    //BoL dummy effects
    if (spellProto->SpellFamilyName == SPELLFAMILY_PALADIN && (spellProto->SpellFamilyFlags & 0xC0000000))
    {
        AuraList const& mDummyAuras = pVictim->GetAurasByType(SPELL_AURA_DUMMY);
        for(AuraList::const_iterator i = mDummyAuras.begin();i != mDummyAuras.end(); ++i)
            if((*i)->GetSpellProto()->SpellVisual == 9180)
                if (spellProto->SpellFamilyFlags & 0x40000000 && (*i)->GetEffIndex() == 1) //FoL
                    AdvertisedBenefit += (*i)->GetModifier()->m_amount;
                else if (spellProto->SpellFamilyFlags & 0x80000000 && (*i)->GetEffIndex() == 0) //HL
                    AdvertisedBenefit += (*i)->GetModifier()->m_amount;
    }

    // Healing over Time spells
    float DotFactor = 1.0f;
    if(damagetype == DOT)
    {
        CastingTime = 3500;
        uint32 DotDuration = GetDuration(spellProto);
        // 200% limit
        if(DotDuration > 30000) DotDuration = 30000;
        DotFactor = DotDuration / 15000.0f;
        int x = 0;
        for(int j = 0; j < 3; j++)
            if(spellProto->Effect[j] == 6) x = j;
        int DotTicks = 6;
        if(spellProto->EffectAmplitude[x] != 0)
            DotTicks = DotDuration / spellProto->EffectAmplitude[x];
        if(DotTicks)
            AdvertisedBenefit /= DotTicks;
    }

    // Level Factor
    float LvlPenalty = 0.0f;
    if(spellProto->spellLevel < 20)
        LvlPenalty = (20.0f - (float)(spellProto->spellLevel)) * 3.75f;
    float LvlFactor = ((float)(spellProto->spellLevel) + 6.0f) / (float)(getLevel());
    if(LvlFactor > 1.0f)
        LvlFactor = 1.0f;

    float ActualBenefit = (float)AdvertisedBenefit * ((float)CastingTime / 3500.0f) * (100.0f - LvlPenalty) * LvlFactor * DotFactor / 100.0f;

    // use float as more appropriate for negative values and percent applying
    float heal = healamount + ActualBenefit;

    // TODO: check for ALL/SPELLS type
    // Healing done percent
    AuraList const& mHealingDonePct = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
    for(AuraList::const_iterator i = mHealingDonePct.begin();i != mHealingDonePct.end(); ++i)
        heal *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;

    // Healing taken percent
    AuraList const& mHealingPct = pVictim->GetAurasByType(SPELL_AURA_MOD_HEALING_PCT);
    for(AuraList::const_iterator i = mHealingPct.begin();i != mHealingPct.end(); ++i)
        heal *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;

    if (heal < 0) heal = 0;

    return uint32(heal);
}

bool Unit::IsImmunedToPhysicalDamage() const
{
    //If m_immuneToDamage type contain magic, IMMUNE damage.
    SpellImmuneList const& damageImmList = m_spellImmune[IMMUNITY_DAMAGE];
    for (SpellImmuneList::const_iterator itr = damageImmList.begin(); itr != damageImmList.end(); ++itr)
        if(itr->type & IMMUNE_DAMAGE_PHYSICAL)
            return true;

    //If m_immuneToSchool type contain this school type, IMMUNE damage.
    SpellImmuneList const& spellImmList = m_spellImmune[IMMUNITY_SCHOOL];
    for (SpellImmuneList::const_iterator itr = spellImmList.begin(); itr != spellImmList.end(); ++itr)
        if(itr->type & IMMUNE_SCHOOL_PHYSICAL)
            return true;

    return false;
}

bool Unit::IsImmunedToSpellDamage(SpellEntry const* spellInfo) const
{
    //If m_immuneToDamage type contain magic, IMMUNE damage.
    SpellImmuneList const& damageList = m_spellImmune[IMMUNITY_DAMAGE];
    for (SpellImmuneList::const_iterator itr = damageList.begin(); itr != damageList.end(); ++itr)
        if(itr->type & uint32(1<<spellInfo->School))
            return true;

    //If m_immuneToSchool type contain this school type, IMMUNE damage.
    SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
    for (SpellImmuneList::const_iterator itr = schoolList.begin(); itr != schoolList.end(); ++itr)
        if(itr->type & uint32(1<<spellInfo->School))
            return true;

    return false;
}

bool Unit::IsImmunedToSpell(SpellEntry const* spellInfo) const
{
    if (!spellInfo)
        return false;

    SpellImmuneList const& dispelList = m_spellImmune[IMMUNITY_DISPEL];
    for(SpellImmuneList::const_iterator itr = dispelList.begin(); itr != dispelList.end(); ++itr)
        if(itr->type == spellInfo->Dispel)
            return true;

    SpellImmuneList const& mechanicList = m_spellImmune[IMMUNITY_MECHANIC];
    for(SpellImmuneList::const_iterator itr = mechanicList.begin(); itr != mechanicList.end(); ++itr)
        if(itr->type == spellInfo->Mechanic)
            return true;

    int32 chance = 0;
    AuraList const& mModMechanicRes = GetAurasByType(SPELL_AURA_MOD_MECHANIC_RESISTANCE);
    for(AuraList::const_iterator i = mModMechanicRes.begin();i != mModMechanicRes.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue == int32(spellInfo->Mechanic))
            chance+= (*i)->GetModifier()->m_amount;
    if(roll_chance_i(chance))
        return true;

    return false;
}

bool Unit::IsImmunedToSpellEffect(uint32 effect) const
{
    //If m_immuneToEffect type contain this effect type, IMMUNE effect.
    SpellImmuneList const& effectList = m_spellImmune[IMMUNITY_EFFECT];
    for (SpellImmuneList::const_iterator itr = effectList.begin(); itr != effectList.end(); ++itr)
        if(itr->type == effect)
            return true;

    return false;
}

bool Unit::IsDamageToThreatSpell(SpellEntry const * spellInfo) const
{
    if(!spellInfo)
        return false;

    uint32 family = spellInfo->SpellFamilyName;
    uint32 flags = spellInfo->SpellFamilyFlags;

    if((family == 5 && flags == 256) ||                     //Searing Pain
        (family == 6 && flags == 8192) ||                   //Mind Blast
        (family == 11 && flags == 1048576))                 //Earth Shock
        return true;

    return false;
}

void Unit::MeleeDamageBonus(Unit *pVictim, uint32 *pdamage,WeaponAttackType attType)
{
    if(!pVictim) return;

    if(*pdamage == 0)
        return;

    CreatureInfo const *cinfo = NULL;
    if(pVictim->GetTypeId() != TYPEID_PLAYER)
        cinfo = ((Creature*)pVictim)->GetCreatureInfo();

    if(GetTypeId() != TYPEID_PLAYER && ((Creature*)this)->isPet())
    {
        if(getPowerType() == POWER_FOCUS)
        {
            uint32 happiness = GetPower(POWER_HAPPINESS);
            if(happiness>=666000)
                *pdamage = uint32(*pdamage * 1.25);
            else if(happiness<333000)
                *pdamage = uint32(*pdamage * 0.75);
            else *pdamage = uint32(*pdamage * 1.0);
        }
    }

    // Taken/Done fixed damage bonus auras
    int32 DoneFlatBenefit = 0;
    int32 TakenFlatBenefit = 0;

    // ..done (for creature type by mask) in taken
    AuraList const& mDamageDoneCreature = this->GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE_CREATURE);
    for(AuraList::const_iterator i = mDamageDoneCreature.begin();i != mDamageDoneCreature.end(); ++i)
        if(cinfo && cinfo->type && ((1 << (cinfo->type-1)) & uint32((*i)->GetModifier()->m_miscvalue)))
            DoneFlatBenefit += (*i)->GetModifier()->m_amount;

    // ..done
    // SPELL_AURA_MOD_DAMAGE_DONE included in weapon damage

    // ..done (base at attack power and creature type)
    AuraList const& mCreatureAttackPower = GetAurasByType(SPELL_AURA_MOD_CREATURE_ATTACK_POWER);
    for(AuraList::const_iterator i = mCreatureAttackPower.begin();i != mCreatureAttackPower.end(); ++i)
        if(cinfo && cinfo->type && ((1 << (cinfo->type-1)) & uint32((*i)->GetModifier()->m_miscvalue)))
            DoneFlatBenefit += int32((*i)->GetModifier()->m_amount/14.0f * GetAttackTime(attType)/1000);

    // ..done (base at attack power for marked target)
    if(attType == RANGED_ATTACK)
    {
        AuraList const& mRangedAttackPowerAttackerBonus = pVictim->GetAurasByType(SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS);
        for(AuraList::const_iterator i = mRangedAttackPowerAttackerBonus.begin();i != mRangedAttackPowerAttackerBonus.end(); ++i)
            DoneFlatBenefit += int32((*i)->GetModifier()->m_amount/14.0f * GetAttackTime(RANGED_ATTACK)/1000);
    }

    // ..taken
    AuraList const& mDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_TAKEN);
    for(AuraList::const_iterator i = mDamageTaken.begin();i != mDamageTaken.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue & IMMUNE_SCHOOL_PHYSICAL)
            TakenFlatBenefit += (*i)->GetModifier()->m_amount;

    if(attType!=RANGED_ATTACK)
    {
        AuraList const& mModMeleeDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN);
        for(AuraList::const_iterator i = mModMeleeDamageTaken.begin(); i != mModMeleeDamageTaken.end(); ++i)
            TakenFlatBenefit += (*i)->GetModifier()->m_amount;
    }
    else
    {
        AuraList const& mModRangedDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN);
        for(AuraList::const_iterator i = mModRangedDamageTaken.begin(); i != mModRangedDamageTaken.end(); ++i)
            TakenFlatBenefit += (*i)->GetModifier()->m_amount;
    }

    // Done/Taken total percent damage auras
    float TakenTotalMod = 1;

    // ..done
    // SPELL_AURA_MOD_DAMAGE_PERCENT_DONE included in weapon damage
    // SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT  included in weapon damage

    // ..taken
    AuraList const& mModDamagePercentTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    for(AuraList::const_iterator i = mModDamagePercentTaken.begin(); i != mModDamagePercentTaken.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue & IMMUNE_SCHOOL_PHYSICAL)
            TakenTotalMod *= ((*i)->GetModifier()->m_amount+100.0f)/100.0f;

    if(attType != RANGED_ATTACK)
    {
        AuraList const& mModMeleeDamageTakenPercent = pVictim->GetAurasByType(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT);
        for(AuraList::const_iterator i = mModMeleeDamageTakenPercent.begin(); i != mModMeleeDamageTakenPercent.end(); ++i)
            TakenTotalMod *= ((*i)->GetModifier()->m_amount+100.0f)/100.0f;
    }
    else
    {
        AuraList const& mModRangedDamageTakenPercent = pVictim->GetAurasByType(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT);
        for(AuraList::const_iterator i = mModRangedDamageTakenPercent.begin(); i != mModRangedDamageTakenPercent.end(); ++i)
            TakenTotalMod *= ((*i)->GetModifier()->m_amount+100.0f)/100.0f;
    }

    float tmpDamage = ((int32(*pdamage) + DoneFlatBenefit) + TakenFlatBenefit)*TakenTotalMod;

    // bonus result can be negative
    *pdamage =  tmpDamage > 0 ? uint32(tmpDamage) : 0;
}

void Unit::ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply)
{
    if (apply)
    {
        for (SpellImmuneList::iterator itr = m_spellImmune[op].begin(), next; itr != m_spellImmune[op].end(); itr = next)
        {
            next = itr; next++;
            if(itr->type == type)
            {
                m_spellImmune[op].erase(itr);
                next = m_spellImmune[op].begin();
            }
        }
        SpellImmune Immune;
        Immune.spellId = spellId;
        Immune.type = type;
        m_spellImmune[op].push_back(Immune);
    }
    else
    {
        for (SpellImmuneList::iterator itr = m_spellImmune[op].begin(); itr != m_spellImmune[op].end(); ++itr)
        {
            if(itr->spellId == spellId)
            {
                m_spellImmune[op].erase(itr);
                break;
            }
        }
    }

}

float Unit::GetWeaponProcChance() const
{
    // normalized proc chance for weapon attack speed
    // (odd formulae...)
    if(isAttackReady(BASE_ATTACK))
        return (GetAttackTime(BASE_ATTACK) * 1.8f / 1000.0f);
    else if (haveOffhandWeapon() && isAttackReady(OFF_ATTACK))
        return (GetAttackTime(OFF_ATTACK) * 1.6f / 1000.0f);
    return 0;
}

float Unit::GetPPMProcChance(uint32 WeaponSpeed, float PPM) const
{
    // proc per minute chance calculation
    if (PPM <= 0) return 0.0f;
    uint32 result = uint32((WeaponSpeed * PPM) / 600.0f);   // result is chance in percents (probability = Speed_in_sec * (PPM / 60))
    return result;
}

void Unit::Mount(uint32 mount, bool taxi)
{
    if(!mount)
        return;

    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, mount);

    uint32 flag = UNIT_FLAG_MOUNT;
    if(taxi)
        flag |= UNIT_FLAG_DISABLE_MOVE;

    SetFlag( UNIT_FIELD_FLAGS, flag );
}

void Unit::Unmount()
{
    if(!IsMounted())
        return;

    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
    RemoveFlag( UNIT_FIELD_FLAGS ,UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_MOUNT );
}

void Unit::SetInCombat()
{
    m_CombatTimer = 5000;
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
}

void Unit::ClearInCombat(bool force)
{
    // wait aura and combat timer expire
    if(!force && HasAuraType(SPELL_AURA_INTERRUPT_REGEN))
        return;

    m_CombatTimer = 0;
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    // remove combo points
    if(GetTypeId()==TYPEID_PLAYER)
        ((Player*)this)->ClearComboPoints();
}

bool Unit::isTargetableForAttack()
{
    if (GetTypeId()==TYPEID_PLAYER && ((Player *)this)->isGameMaster())
        return false;
    return isAlive() && !hasUnitState(UNIT_STAT_DIED)&& !isInFlight() /*&& !isStealth()*/;
}

int32 Unit::ModifyHealth(int32 dVal)
{
    int32 gain = 0;

    if(dVal==0)
        return 0;

    int32 curHealth = (int32)GetHealth();

    int32 val = dVal + curHealth;
    if(val <= 0)
    {
        SetHealth(0);
        return -curHealth;
    }

    int32 maxHealth = (int32)GetMaxHealth();

    if(val < maxHealth)
    {
        SetHealth(val);
        gain = val - curHealth;
    }
    else if(curHealth != maxHealth)
    {
        SetHealth(maxHealth);
        gain = maxHealth - curHealth;
    }

    return gain;
}

int32 Unit::ModifyPower(Powers power, int32 dVal)
{
    int32 gain = 0;

    if(dVal==0)
        return 0;

    int32 curPower = (int32)GetPower(power);

    int32 val = dVal + curPower;
    if(val <= 0)
    {
        SetPower(power,0);
        return -curPower;
    }

    int32 maxPower = (int32)GetMaxPower(power);

    if(val < maxPower)
    {
        SetPower(power,val);
        gain = val - curPower;
    }
    else if(curPower != maxPower)
    {
        SetPower(power,maxPower);
        gain = maxPower - curPower;
    }

    return gain;
}

bool Unit::isVisibleForOrDetect(Unit const* u, bool detect, bool inVisibleList) const
{
    if(!u)
        return false;

    // Always can see self
    if (u==this)
        return true;

    // not in world
    if(!IsInWorld() || !u->IsInWorld())
        return false;

    // always seen by owner
    if(GetCharmerOrOwnerGUID()==u->GetGUID())
        return true;

    // Grid dead/alive checks
    if( u->GetTypeId()==TYPEID_PLAYER)
    {
        // non visible at grid for any stealth state
        if(!IsVisibleInGridForPlayer((Player *)u))
            return false;

        // if player is dead then he can't detect anyone in anycases
        if(!u->isAlive())
            detect = false;
    }
    else
    {
        // all dead creatures/players not visible for any creatures
        if(!u->isAlive() || !isAlive())
            return false;
    }

    // different visible distance checks
    if(u->isInFlight())                                     // what see player in flight
    {
        // use object grey distance for all (only see objects any way)
        if (!IsWithinDistInMap(u,World::GetMaxVisibleDistanceInFlight()+(inVisibleList ? World::GetVisibleObjectGreyDistance() : 0.0f)))
            return false;
    }
    else if(!isAlive())                                     // distance for show body
    {
        if (!IsWithinDistInMap(u,World::GetMaxVisibleDistanceForObject()+(inVisibleList ? World::GetVisibleObjectGreyDistance() : 0.0f)))
            return false;
    }
    else if(GetTypeId()==TYPEID_PLAYER)                     // distance for show player
    {
        // in case two player at same transport ignore distance checks
        if(!((Player*)this)->GetTransport() || u->GetTypeId()!=TYPEID_PLAYER || ((Player*)this)->GetTransport() != ((Player*)u)->GetTransport())
        {
            // Players far than max visible distance for player or not in our map are not visible too
            if (!IsWithinDistInMap(u,World::GetMaxVisibleDistanceForPlayer()+(inVisibleList ? World::GetVisibleUnitGreyDistance() : 0.0f)))
                return false;
        }
    }
    else if(GetCharmerOrOwnerGUID())                        // distance for show pet/charmed
    {
        // Pet/charmed far than max visible distance for player or not in our map are not visible too
        if (!IsWithinDistInMap(u,World::GetMaxVisibleDistanceForPlayer()+(inVisibleList ? World::GetVisibleUnitGreyDistance() : 0.0f)))
            return false;
    }
    else                                                    // distance for show creature
    {
        // Units far than max visible distance for creature or not in our map are not visible too
        if (!IsWithinDistInMap(u,World::GetMaxVisibleDistanceForCreature()+(inVisibleList ? World::GetVisibleUnitGreyDistance() : 0.0f)))
            return false;
    }

    // Visible units, always are visible for all units, except for units under invisibility
    if (m_Visibility == VISIBILITY_ON && u->GetVisibility()!= VISIBILITY_GROUP_INVISIBILITY)
        return true;

    // GMs are visible for higher gms (or players are visible for gms)
    if (u->GetTypeId() == TYPEID_PLAYER && ((Player *)u)->isGameMaster())
        return (GetTypeId() == TYPEID_PLAYER && ((Player *)this)->GetSession()->GetSecurity() <= ((Player *)u)->GetSession()->GetSecurity());

    // non faction visibility non-breakable for non-GMs 
    if (m_Visibility == VISIBILITY_OFF)
        return false;

    // Invisible units, always are visible for units under invisibility or unit that can detect this invisibility
    if (m_Visibility == VISIBILITY_GROUP_INVISIBILITY)
    {
        if(u->GetVisibility()== VISIBILITY_GROUP_INVISIBILITY || m_invisibilityvalue <= u->m_detectInvisibility)
            return true;
    }

    // Units that can detect invisibility always are visible for units that can be detected
    if (u->GetVisibility()== VISIBILITY_GROUP_INVISIBILITY)
    {
        if(m_detectInvisibility >= u->m_invisibilityvalue)
            return true;
    }

    // Stealth/invisible not hostile units, not visible (except Player-with-Player case)
    if (!u->IsHostileTo(this))
    {
        // player see other player with stealth/invisibility only if he in same group or raid or same team (raid/team case dependent from conf setting)
        if(GetTypeId()==TYPEID_PLAYER && u->GetTypeId()==TYPEID_PLAYER)
        {
            if(((Player*)this)->IsGroupVisibleFor(((Player*)u)))
                return true;

            // else apply same rules as for hostile case (detecting check)
        }
    }
    else
    {
        // Hunter mark functionality
        AuraList const& auras = GetAurasByType(SPELL_AURA_MOD_STALKED);
        for(AuraList::const_iterator iter = auras.begin(); iter != auras.end(); ++iter)
            if((*iter)->GetCasterGUID()==u->GetGUID())
                return true;
    }

    // unit got in stealth in this moment and must ignore old detected state
    // invisibility not have chance for detection
    if (m_Visibility == VISIBILITY_ON || m_Visibility == VISIBILITY_GROUP_NO_DETECT || m_Visibility == VISIBILITY_GROUP_INVISIBILITY)
        return false;

    // NOW ONLY STEALTH CASE

    // stealth and detected
    if (u->GetTypeId() == TYPEID_PLAYER && ((Player*)u)->HaveAtClient(this) )
        return true;

    //if in non-detect mode then invisible for unit
     if ((!detect) && (!u->IsHostileTo(this)))
         return false;


    // Special cases
    bool IsVisible = true;
    bool isInFront = u->isInFront(this,World::GetMaxVisibleDistanceForObject());
    float distance = sqrt(GetDistanceSq(u));

    // If is attacked then stealth is lost, some creature can use stealth too
    if (this->isAttacked())
        return IsVisible;

    // If there is collision rogue is seen regardless of level difference
    // TODO: check sizes in DB
    if (distance < 0.24f)
        return IsVisible;

    //If a mob or player is stunned he will not be able to detect stealth
    if ((u->hasUnitState(UNIT_STAT_STUNDED)) && (u != this))
    {
        IsVisible=false;
        return IsVisible;
    }

    // Cases based on level difference and position
    int32 levelDiff = u->getLevel() - this->getLevel();

    //If mob is 5 levels more than player he gets detected automaticly
    if (u->GetTypeId()!=TYPEID_PLAYER && levelDiff > 5)
        return IsVisible;

    // If victim has more than 5 lvls above caster
    if ((this->GetTypeId() == TYPEID_UNIT)&& ( levelDiff > 5 ))
        return IsVisible;

    // If caster has more than 5 levels above victim
    if ((this->GetTypeId() == TYPEID_UNIT)&& ( levelDiff < -5 ))
    {
        IsVisible=false;
        return IsVisible;
    }

    float modifier = 1; // 100 pct
    float pvpMultiplier = 0;
    float distanceFormula = 0;
    int32 rank = 0;
    //This allows to check talent tree and will add addition stealth dependant on used points)
    uint32 stealthMod = GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH_LEVEL);
    //****************************************************************************************
    int32 x = (((u->m_detectStealth+1) / 5) - (((m_stealthvalue+1) / 5) + (stealthMod/5) + 59)); // Stealth detection calculation

    if (x<0) x = 1;
    // Check rank if mob is not a player otherwise rank = 1 and there is no modifier when in pvp
    if (u->GetTypeId() != TYPEID_PLAYER)
        rank = ((Creature const*)u)->GetCreatureInfo()->rank;

    // Probabilty of being seen
    if (levelDiff < 0)
        distanceFormula = ((sWorld.getRate(RATE_CREATURE_AGGRO) * 16) / (abs(levelDiff) + 1.5)) * 2;
    if (levelDiff >= 0)
        distanceFormula = (((sWorld.getRate(RATE_CREATURE_AGGRO) * 16)) / 2) * (levelDiff + 1);

    // Original probability values
    // removed level of mob in calculation as it should not affect the detection, it is mainly dependant on level difference
    float averageDist = 1 - 0.11016949 * x + 0.00301637 * x * x;  //at this distance, the detector has to be a 15% prob of detect
    if (averageDist < 1) averageDist = 1;

    float prob = 0;
    if (distance > averageDist)
        prob = (averageDist - 200 + 9 * distance) / (averageDist - 20);     //prob between 10% and 0%
    else
        prob = 75 - (60 / averageDist) * distance;                          //prob between 15% and 75% (75% max prob)

    // If is not in front, probability floor
    if (!isInFront)
        prob = prob / 100;
    if (prob < 0.1)
        prob = 0.1;

    // Mob rank affects modifier
    modifier = rank <= 4 ? 1 + rank * 0.2f : 1;

    if (distance < 0.24f)
    {
        return IsVisible;
    }

    // PVP distance assigned depending on level
    if (this->GetTypeId() == TYPEID_UNIT)
    {
        // Level diff floor/ceiling <-5,5>
        pvpMultiplier = levelDiff > 5 ? 12 : levelDiff < 5 ? 2 : 7 + levelDiff;
        pvpMultiplier = pvpMultiplier - (x / 100);
    }

    // PVP stealth handler
    if (this->GetTypeId() == TYPEID_PLAYER)
    {
        // Do not loose stealth when coming from back
        if (!isInFront)
        {
            IsVisible=false;
            return IsVisible;
        }

        // If comes in front
        if (isInFront && (distance >= pvpMultiplier))
        {
            IsVisible=false;
            return IsVisible;
        }

        if (isInFront && (distance < pvpMultiplier))
        {
            return IsVisible;
        }
    }
    else
    {
        // PVE stealth handler
        // Distance of approch player stays stealth 100% is dependant of level. No probabiliy or detection is rolled
        // This establishes a buffer zone in between mob start to see you and mob start to roll probabilities or detect you
        if ((distance < 100) && (distance > ((distanceFormula * 2) * modifier)) && (distance > 0.24f))
        {
            IsVisible=false;
            return IsVisible;

        }
        //If victim is level lower or more probability of detection drops
        if ((levelDiff < 0) && (distance > 0.24f))
        {
            if (abs(levelDiff ) > 4)
                IsVisible = false;
            else
            {
                if (rand_chance() > ( prob * modifier / (30 + levelDiff * 5)))
                    IsVisible = false;
            }
            return IsVisible;
        }
        // Level detection based on level, the higher the mob level the higher the chance of detection.
        if ((distance > 0.24f) && (levelDiff < 5) && (levelDiff >= 0))
        {
            if (rand_chance() > ((prob * modifier) / (30 - levelDiff * 5) ))
                IsVisible = false;
            else
                IsVisible = true;

            return IsVisible;
        }
    }

    // Didn't match any criteria ?
    DEBUG_LOG("Unit::isVisibleForFor unhandled result, dist %f levelDiff %i target_type %u prob %u modifier %u",distance,levelDiff,u->GetTypeId(),prob, modifier);

    // Safety return
    return IsVisible;
}

void Unit::SetVisibility(UnitVisibility x)
{
    m_Visibility = x;

    if(IsInWorld())
    {
        Map *m = MapManager::Instance().GetMap(GetMapId(), this);

        if(GetTypeId()==TYPEID_PLAYER)
            m->PlayerRelocation((Player*)this,GetPositionX(),GetPositionY(),GetPositionZ(),GetOrientation());
        else
            m->CreatureRelocation((Creature*)this,GetPositionX(),GetPositionY(),GetPositionZ(),GetOrientation());
    }
}

float Unit::GetSpeed( UnitMoveType mtype ) const
{
    return m_speed_rate[mtype]*baseMoveSpeed[mtype];
}

void Unit::SetSpeed(UnitMoveType mtype, float rate, bool forced)
{
    m_speed_rate[mtype] = 1.0f;
    ApplySpeedMod(mtype, rate, forced, true);
}

void Unit::ApplySpeedMod(UnitMoveType mtype, float rate, bool forced, bool apply)
{
    WorldPacket data;

    if(apply)
        m_speed_rate[mtype] *= rate;
    else
        m_speed_rate[mtype] /= rate;

    switch(mtype)
    {
        case MOVE_WALK:
            if(forced) { data.Initialize(SMSG_FORCE_WALK_SPEED_CHANGE, 16); }
            else { data.Initialize(MSG_MOVE_SET_WALK_SPEED, 16); }
            break;
        case MOVE_RUN:
            if(forced) { data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE, 16); }
            else { data.Initialize(MSG_MOVE_SET_RUN_SPEED, 16); }
            break;
        case MOVE_WALKBACK:
            if(forced) { data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE, 16); }
            else { data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED, 16); }
            break;
        case MOVE_SWIM:
            if(forced) { data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE, 16); }
            else { data.Initialize(MSG_MOVE_SET_SWIM_SPEED, 16); }
            break;
        case MOVE_SWIMBACK:
            if(forced) { data.Initialize(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE, 16); }
            else { data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED, 16); }
            break;
        case MOVE_TURN:
            if(forced) { data.Initialize(SMSG_FORCE_TURN_RATE_CHANGE, 16); }
            else { data.Initialize(MSG_MOVE_SET_TURN_RATE, 16); }
            break;
        case MOVE_FLY:
            if(forced) { data.Initialize(SMSG_FORCE_FLY_SPEED_CHANGE, 16); }
            else { data.Initialize(SMSG_MOVE_SET_FLY_SPEED, 16); }
            break;
        case MOVE_FLYBACK:
            break;
        default:
            sLog.outError("Unit::SetSpeed: Unsupported move type (%d), data not sent to client.",mtype);
            return;
    }

    data.append(GetPackGUID());
    data << (uint32)0;
    if (mtype == MOVE_RUN) data << uint8(0);// new 2.1.0
    data << float(GetSpeed(mtype));
    SendMessageToSet( &data, true );

    if(Pet* pet = GetPet())
        pet->SetSpeed(mtype,m_speed_rate[mtype],forced);
}

void Unit::SetHover(bool on)
{
    if(on)
    {
        SpellEntry const *sInfo = sSpellStore.LookupEntry(11010);
        if(!sInfo)
            return;

        Spell spell(this, sInfo, true,0);
        SpellCastTargets targets;
        targets.setUnitTarget(this);
        targets.m_targetMask = TARGET_FLAG_SELF;
        spell.prepare(&targets);
    }
    else
    {
        RemoveAurasDueToSpell(11010);
    }
}

void Unit::setDeathState(DeathState s)
{
    if (s != ALIVE)
    {
        CombatStop(true);

        if(m_currentSpell)
            m_currentSpell->cancel();
    }

    if (s == JUST_DIED)
    {
        RemoveAllAurasOnDeath();
        UnsummonAllTotems();
    }
    if (m_deathState != ALIVE && s == ALIVE)
    {
        //_ApplyAllAuraMods();
    }
    m_deathState = s;
}

/*########################################
########                          ########
########       AGGRO SYSTEM       ########
########                          ########
########################################*/
bool Unit::CanHaveThreatList() const
{
    if(GetTypeId() == TYPEID_UNIT && !((Creature*)this)->isPet() && !((Creature*)this)->isTotem() )
        return true;
    else
        return false;
}

//======================================================================

float Unit::ApplyTotalThreatModifier(float threat, uint8 school)
{
    if(!HasAuraType(SPELL_AURA_MOD_THREAT))
        return threat;

    if(school >= MAX_SPELL_SCHOOL)
        return threat;

    return threat * m_threatModifier[school];
}

//======================================================================

void Unit::AddThreat(Unit* pVictim, float threat, uint8 school, SpellEntry const *threatSpell)
{
    // Only mobs can manage threat lists
    if(CanHaveThreatList())
        m_ThreatManager.addThreat(pVictim, threat, school, threatSpell);
}

//======================================================================

void Unit::DeleteThreatList()
{
    m_ThreatManager.clearReferences();
}

//======================================================================

void Unit::TauntApply(Unit* taunter)
{
    assert(GetTypeId()== TYPEID_UNIT);

    if(!taunter || (taunter->GetTypeId() == TYPEID_PLAYER && ((Player*)taunter)->isGameMaster()))
        return;

    if(!CanHaveThreatList())
        return;

    Unit *target = getVictim();
    if(target && target == taunter)
        return;

    SetInFront(taunter);
    if (((Creature*)this)->AI())
        ((Creature*)this)->AI()->AttackStart(taunter);

    m_ThreatManager.tauntApply(taunter);
}

//======================================================================

void Unit::TauntFadeOut(Unit *taunter)
{
    assert(GetTypeId()== TYPEID_UNIT);

    if(!taunter || (taunter->GetTypeId() == TYPEID_PLAYER && ((Player*)taunter)->isGameMaster()))
        return;

    if(!CanHaveThreatList())
        return;

    Unit *target = getVictim();
    if(!target || target != taunter)
        return;

    if(m_ThreatManager.isThreatListEmpty())
    {
        if(((Creature*)this)->AI())
            ((Creature*)this)->AI()->EnterEvadeMode();
        return;
    }

    m_ThreatManager.tauntFadeOut(taunter);
    target = m_ThreatManager.getHostilTarget();

    if (target && target != taunter)
    {
        SetInFront(target);
        if (((Creature*)this)->AI())
            ((Creature*)this)->AI()->AttackStart(target);
    }
}

//======================================================================

bool Unit::SelectHostilTarget()
{
    //function provides main threat functionality
    //next-victim-selection algorithm and evade mode are called
    //threat list sorting etc.

    assert(GetTypeId()== TYPEID_UNIT);
    Unit* target = NULL;

    //This function only useful once AI has been initilazied
    if (!((Creature*)this)->AI())
        return false;

    if(!m_ThreatManager.isThreatListEmpty())
    {
        if(!HasAuraType(SPELL_AURA_MOD_TAUNT))
        {
            target = m_ThreatManager.getHostilTarget();
        }
    }

    if(target)
    {
        SetInFront(target);
        ((Creature*)this)->AI()->AttackStart(target);
        return true;
    }

    if(isInCombat() && !HasAuraType(SPELL_AURA_MOD_TAUNT) && CanFreeMove())
        ((Creature*)this)->AI()->EnterEvadeMode();

    return false;
}

//======================================================================
//======================================================================
//======================================================================

int32 Unit::CalculateSpellDamage(SpellEntry const* spellProto, uint8 effect_index, int32 effBasePoints)
{
    int32 value = 0;
    uint32 level = 0;

    Player* unitPlayer = (GetTypeId() == TYPEID_PLAYER) ? (Player*)this : NULL;

    level = getLevel() - spellProto->spellLevel;
    if (level > spellProto->maxLevel && spellProto->maxLevel > 0)
        level = spellProto->maxLevel;

    float basePointsPerLevel = spellProto->EffectRealPointsPerLevel[effect_index];
    float randomPointsPerLevel = spellProto->EffectDicePerLevel[effect_index];
    int32 basePoints = int32(effBasePoints + level * basePointsPerLevel);
    int32 randomPoints = int32(spellProto->EffectDieSides[effect_index] + level * randomPointsPerLevel);
    float comboDamage = spellProto->EffectPointsPerComboPoint[effect_index];
    uint8 comboPoints=0;
    if(unitPlayer)
    {
        if (m_attacking && (m_attacking->GetGUID() == unitPlayer->GetComboTarget()))
            comboPoints = unitPlayer->GetComboPoints();
    }

    value = basePoints + rand32(spellProto->EffectBaseDice[effect_index], randomPoints);
    //random damage
    if(int32(spellProto->EffectBaseDice[effect_index]) != randomPoints && GetTypeId() == TYPEID_UNIT && ((Creature*)this)->isPet())
        value += ((Pet*)this)->GetBonusDamage();                //bonus damage only on spells without fixed basePoints?)
    if(comboDamage > 0)
    {
        value += (int32)(comboDamage * comboPoints);
        // Eviscerate
        if( spellProto->SpellIconID == 514 && spellProto->SpellFamilyName == SPELLFAMILY_ROGUE)
            value += (int32)(GetTotalAttackPowerValue(BASE_ATTACK) * comboPoints * 0.03);
        if(unitPlayer)
            unitPlayer->SetComboPoints(unitPlayer->GetComboTarget(), 0);
    }

    if (GetTypeId() == TYPEID_PLAYER)
    {
        ((Player *)this)->ApplySpellMod(spellProto->Id,SPELLMOD_ALL_EFFECTS, value);
        switch(effect_index)
        {
        case 0:
            ((Player*)this)->ApplySpellMod(spellProto->Id,SPELLMOD_EFFECT1, value);
            break;
        case 1:
            ((Player*)this)->ApplySpellMod(spellProto->Id,SPELLMOD_EFFECT2, value);
            break;
        case 2:
            ((Player*)this)->ApplySpellMod(spellProto->Id,SPELLMOD_EFFECT3, value);
            break;
        }
    }

    return value;
}

void Unit::AddDiminishing(DiminishingMechanics mech, uint32 hitTime, uint32 hitCount)
{
    m_Diminishing.push_back(DiminishingReturn(mech,hitTime,hitCount));
}

DiminishingLevels Unit::GetDiminishing(DiminishingMechanics mech)
{
    for(Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if(i->Mechanic != mech) continue;
        if(!i->hitCount) return DIMINISHING_LEVEL_1;
        if(!i->hitTime)  return DIMINISHING_LEVEL_1;
        // If last spell was casted more than 15 seconds ago - reset the count.
        if((getMSTime() - i->hitTime) > 15000)
        {
            i->hitCount = DIMINISHING_LEVEL_1;
            return DIMINISHING_LEVEL_1;
        }
        // or else increase the count.
        else
        {
            if(i->hitCount > DIMINISHING_LEVEL_2)
            {
                i->hitCount = DIMINISHING_LEVEL_IMMUNE;
                return DIMINISHING_LEVEL_IMMUNE;
            }
            else return DiminishingLevels(i->hitCount);
        }
    }
    return DIMINISHING_LEVEL_1;
}

void Unit::IncrDiminishing(DiminishingMechanics mech, uint32 duration)
{
    // Checking for existing in the table
    bool IsExist = false;
    for(Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if(i->Mechanic != mech)
            continue;

        IsExist = true;
        if(i->hitCount < DIMINISHING_LEVEL_IMMUNE)
        {
            i->hitCount += 1;
            switch(i->hitCount)
            {
                case DIMINISHING_LEVEL_2:       i->hitTime = uint32(getMSTime() + duration); break;
                case DIMINISHING_LEVEL_3:       i->hitTime = uint32(getMSTime() + duration*0.5); break;
                case DIMINISHING_LEVEL_IMMUNE:  i->hitTime = uint32(getMSTime() + duration*0.25); break;
                default: break;
            }
        }
        break;
    }

    if(!IsExist)
        AddDiminishing(mech,uint32(getMSTime() + duration),DIMINISHING_LEVEL_2);
}

void Unit::UpdateDiminishingTime(DiminishingMechanics mech)
{
    // Checking for existing in the table
    bool IsExist = false;
    for(Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if(i->Mechanic != mech)
            continue;

        IsExist = true;
        i->hitTime = getMSTime();
        break;
    }

    if(!IsExist)
        AddDiminishing(mech,getMSTime(),DIMINISHING_LEVEL_1);
}

DiminishingMechanics Unit::Mechanic2DiminishingMechanics(uint32 mech)
{
    switch(mech)
    {
        case MECHANIC_CHARM: case MECHANIC_FEAR: case MECHANIC_SLEEP:
            return DIMINISHING_MECHANIC_CHARM;
        case MECHANIC_CONFUSED: case MECHANIC_KNOCKOUT: case MECHANIC_POLYMORPH:
            return DIMINISHING_MECHANIC_CONFUSE;
        case MECHANIC_ROOT: case MECHANIC_FREEZE:
            return DIMINISHING_MECHANIC_ROOT;
        case MECHANIC_STUNDED:
            return DIMINISHING_MECHANIC_STUN;
        case MECHANIC_CHASE:
            return DIMINISHING_MECHANIC_SPEED;
        default:
            break;
    }
    return DIMINISHING_NONE;
}

void Unit::ApplyDiminishingToDuration(DiminishingMechanics  mech, int32& duration)
{
    if(duration == -1 || mech == DIMINISHING_NONE)
        return;

    // Stun diminishing is applies to mobs too
    if(mech == DIMINISHING_MECHANIC_STUN || GetTypeId() == TYPEID_PLAYER)
    {
        DiminishingLevels diminish = GetDiminishing(mech);
        switch(diminish)
        {
            case DIMINISHING_LEVEL_1: IncrDiminishing(mech, duration); break;
            case DIMINISHING_LEVEL_2: IncrDiminishing(mech, duration); duration = int32(duration*0.5f); break;
            case DIMINISHING_LEVEL_3: IncrDiminishing(mech, duration); duration = int32(duration*0.25f); break;
            case DIMINISHING_LEVEL_IMMUNE: duration = 0; break;
            default: break;
        }
    }
}

Creature* Unit::SummonCreature(uint32 id, float x, float y, float z, float ang,TempSummonType spwtype,uint32 despwtime)
{
    TemporarySummon* pCreature = new TemporarySummon(this,this);

    pCreature->SetInstanceId(GetInstanceId());

    if (!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), GetMapId(), x, y, z, ang, id))
    {
        delete pCreature;
        return NULL;
    }

    pCreature->Summon(spwtype, despwtime);
    
    //return the creature therewith the summoner has access to it
    return pCreature;
}

Unit* Unit::GetUnit(WorldObject& object, uint64 guid)
{
    return ObjectAccessor::Instance().GetUnit(object,guid);
}

bool Unit::isVisibleForInState( Player const* u, bool inVisibleList ) const
{
    return isVisibleForOrDetect(u,false,inVisibleList);
}

uint32 Unit::GetCreatureTypeMask() const
{
    uint32 creatureType = 0;
    if(GetTypeId() == TYPEID_PLAYER)
        creatureType = 0x40;                  //1<<(7-1)
    else
    {
        uint32 CType = ((Creature*)this)->GetCreatureInfo()->type;
        if(CType>=1)
            creatureType = 1 << (CType - 1);
        else
            creatureType = 0;
    }
    return creatureType;
}


/*#######################################
########                         ########
########       STAT SYSTEM       ########
########                         ########
#######################################*/

bool Unit::HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply)
{
    if(GetTypeId() == TYPEID_UNIT) //forbid to modify stats for mobs until mob stat system creation
        return false;

    if(unitMod >= UNIT_MOD_END || modifierType >= MODIFIER_TYPE_END)
    {
        sLog.outError("ERROR in HandleStatModifier(): nonexisted UnitMods or wrong UnitModifierType!");
        return false;
    }

    float val = 1.0f;

    switch(modifierType)
    {
    case BASE_VALUE:
    case TOTAL_VALUE: 
        m_auraModifiersGroup[unitMod][modifierType] += apply ? amount : -amount;
        break;
    case BASE_PCT:
    case TOTAL_PCT:
        if(amount <= -100.0f)  //small hack-fix for -100% modifiers
            amount = -200.0f; 

        val = (100.0f + amount) / 100.0f;
        m_auraModifiersGroup[unitMod][modifierType] *= apply ? val : (1.0f/val);
        break;

    default:
        break;
    }

    if(!CanModifyStats())
        return false;

    switch(unitMod)
    {
    case UNIT_MOD_STAT_STRENGTH:
    case UNIT_MOD_STAT_AGILITY:
    case UNIT_MOD_STAT_STAMINA:
    case UNIT_MOD_STAT_INTELLECT:
    case UNIT_MOD_STAT_SPIRIT:         UpdateStats(GetStatByAuraGroup(unitMod));  break;

    case UNIT_MOD_ARMOR:               UpdateArmor();           break;
    case UNIT_MOD_HEALTH:              UpdateMaxHealth();       break;

    case UNIT_MOD_MANA:
    case UNIT_MOD_RAGE:       
    case UNIT_MOD_FOCUS:      
    case UNIT_MOD_ENERGY:     
    case UNIT_MOD_HAPPINESS:           UpdateMaxPower(GetPowerTypeByAuraGroup(unitMod));         break;

    case UNIT_MOD_RESISTANCE_HOLY:
    case UNIT_MOD_RESISTANCE_FIRE:
    case UNIT_MOD_RESISTANCE_NATURE:
    case UNIT_MOD_RESISTANCE_FROST:
    case UNIT_MOD_RESISTANCE_SHADOW:
    case UNIT_MOD_RESISTANCE_ARCANE:   UpdateResistances(GetSpellSchoolByAuraGroup(unitMod));      break;

    case UNIT_MOD_ATTACK_POWER:        UpdateAttackPowerAndDamage();         break;
    case UNIT_MOD_ATTACK_POWER_RANGED: UpdateAttackPowerAndDamage(true);     break;

    case UNIT_MOD_DAMAGE_MAINHAND:     UpdateDamagePhysical(BASE_ATTACK);    break;
    case UNIT_MOD_DAMAGE_OFFHAND:      UpdateDamagePhysical(OFF_ATTACK);     break;
    case UNIT_MOD_DAMAGE_RANGED:       UpdateDamagePhysical(RANGED_ATTACK);  break;

    default:
        break;
    }

    return true;
}

float Unit::GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const
{
    if( unitMod >= UNIT_MOD_END || modifierType >= MODIFIER_TYPE_END)
    {
        sLog.outError("ERROR: trial to access nonexisted modifier value from UnitMods!");
        return 0.0f;
    }

    if(modifierType == TOTAL_PCT && m_auraModifiersGroup[unitMod][modifierType] <= 0.0f)
        return 0.0f;

    return m_auraModifiersGroup[unitMod][modifierType];
}

float Unit::GetTotalStatValue(Stats stat) const
{
    UnitMods unitMod = UnitMods(UNIT_MOD_STAT_START + stat);

    if(m_auraModifiersGroup[unitMod][TOTAL_PCT] <= 0.0f)
        return 0.0f;

     // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = m_auraModifiersGroup[unitMod][BASE_VALUE] + GetCreateStat(stat);
    value *= m_auraModifiersGroup[unitMod][BASE_PCT];
    value += m_auraModifiersGroup[unitMod][TOTAL_VALUE];
    value *= m_auraModifiersGroup[unitMod][TOTAL_PCT];

    return value;
}

float Unit::GetTotalAuraModValue(UnitMods unitMod) const
{
    if(unitMod >= UNIT_MOD_END)
    {
        sLog.outError("ERROR: trial to access nonexisted UnitMods in GetTotalAuraModValue()!");
        return 0.0f;
    }

    if(m_auraModifiersGroup[unitMod][TOTAL_PCT] <= 0.0f)
        return 0.0f;

    float value  = m_auraModifiersGroup[unitMod][BASE_VALUE];
    value *= m_auraModifiersGroup[unitMod][BASE_PCT];
    value += m_auraModifiersGroup[unitMod][TOTAL_VALUE];
    value *= m_auraModifiersGroup[unitMod][TOTAL_PCT];

    return value;
}

SpellSchools Unit::GetSpellSchoolByAuraGroup(UnitMods unitMod) const
{
    SpellSchools school = SPELL_SCHOOL_NORMAL;

    switch(unitMod)
    {
     case UNIT_MOD_RESISTANCE_HOLY:     school = SPELL_SCHOOL_HOLY;          break;
     case UNIT_MOD_RESISTANCE_FIRE:     school = SPELL_SCHOOL_FIRE;          break;
     case UNIT_MOD_RESISTANCE_NATURE:   school = SPELL_SCHOOL_NATURE;        break;
     case UNIT_MOD_RESISTANCE_FROST:    school = SPELL_SCHOOL_FROST;         break;
     case UNIT_MOD_RESISTANCE_SHADOW:   school = SPELL_SCHOOL_SHADOW;        break;
     case UNIT_MOD_RESISTANCE_ARCANE:   school = SPELL_SCHOOL_ARCANE;        break;

     default:
         break;
    }

    return school;
}

Stats Unit::GetStatByAuraGroup(UnitMods unitMod) const
{
    Stats stat = STAT_STRENGTH;

    switch(unitMod)
    {
     case UNIT_MOD_STAT_STRENGTH:    stat = STAT_STRENGTH;      break;
     case UNIT_MOD_STAT_AGILITY:     stat = STAT_AGILITY;       break;
     case UNIT_MOD_STAT_STAMINA:     stat = STAT_STAMINA;       break;
     case UNIT_MOD_STAT_INTELLECT:   stat = STAT_INTELLECT;     break;
     case UNIT_MOD_STAT_SPIRIT:      stat = STAT_SPIRIT;        break;

     default:
         break;
    }

    return stat;
}

Powers Unit::GetPowerTypeByAuraGroup(UnitMods unitMod) const
{
    Powers power = POWER_MANA;

    switch(unitMod)
    {
    case UNIT_MOD_MANA:       power = POWER_MANA;       break;
    case UNIT_MOD_RAGE:       power = POWER_RAGE;       break;
    case UNIT_MOD_FOCUS:      power = POWER_FOCUS;      break;
    case UNIT_MOD_ENERGY:     power = POWER_ENERGY;     break;
    case UNIT_MOD_HAPPINESS:  power = POWER_HAPPINESS;  break;

    default:
        break;
    }

    return power;
}

bool Unit::UpdateStats(Stats stat)
{
    if(stat > STAT_SPIRIT)
        return false;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = GetTotalStatValue(stat);

    SetStat(stat, int32(value));

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
        if(GetTypeId() == TYPEID_PLAYER)
        {
            ((Player*)this)->UpdateAllCritPercentages();
            ((Player*)this)->UpdateDodgePercentage();
        }
        break;

    case STAT_STAMINA:   UpdateMaxHealth(); break;
    case STAT_INTELLECT: 
        UpdateMaxPower(POWER_MANA);
        if(GetTypeId() == TYPEID_PLAYER)
            ((Player*)this)->UpdateAllSpellCritChances();
        break;

    case STAT_SPIRIT:                       break;

    default:
        break;
    }

     return true;
}

bool Unit::UpdateAllStats()
{
    if(GetTypeId() != TYPEID_PLAYER)
        return false;

    for (int i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        float value = GetTotalStatValue(Stats(i));
        SetStat(Stats(i), int32(value));
    }

    UpdateAttackPowerAndDamage();
    UpdateAttackPowerAndDamage(true);
    UpdateArmor();
    UpdateMaxHealth();

    for(int i = POWER_MANA; i < MAX_POWERS; i++)
        UpdateMaxPower(Powers(i));

    if(GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)this)->UpdateAllCritPercentages();
        ((Player*)this)->UpdateAllSpellCritChances();
        ((Player*)this)->UpdateDefenseBonusesMod();
    }

    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
        UpdateResistances(i);

    return true;
}

void Unit::UpdateResistances(uint32 school)
{
    UnitMods unitMod = UNIT_MOD_RESISTANCE_HOLY;

    switch(school)
    {
    case SPELL_SCHOOL_NORMAL: UpdateArmor(); return;
    case SPELL_SCHOOL_HOLY:   unitMod = UNIT_MOD_RESISTANCE_HOLY;     break;
    case SPELL_SCHOOL_FIRE:   unitMod = UNIT_MOD_RESISTANCE_FIRE;     break;
    case SPELL_SCHOOL_NATURE: unitMod = UNIT_MOD_RESISTANCE_NATURE;    break;
    case SPELL_SCHOOL_FROST:  unitMod = UNIT_MOD_RESISTANCE_FROST;    break;
    case SPELL_SCHOOL_SHADOW: unitMod = UNIT_MOD_RESISTANCE_SHADOW;     break;
    case SPELL_SCHOOL_ARCANE: unitMod = UNIT_MOD_RESISTANCE_ARCANE;   break;

    default:
        break;
    }

    float value  = GetTotalAuraModValue(unitMod);

    SetResistance(SpellSchools(school), int32(value));
}

void Unit::UpdateMaxHealth()
{
    UnitMods unitMod = UNIT_MOD_HEALTH;
    float stamina = GetStat(STAT_STAMINA) - GetCreateStat(STAT_STAMINA);

    float value   = GetModifierValue(unitMod, BASE_VALUE) + GetCreateHealth();
    value  *= GetModifierValue(unitMod, BASE_PCT);
    value  += GetModifierValue(unitMod, TOTAL_VALUE) + stamina * 10.0f;
    value  *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxHealth(uint32(value));
}

void Unit::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);
    float addValue = (power == POWER_MANA) ? GetStat(STAT_INTELLECT) - GetCreateStat(STAT_INTELLECT) : 0.0f;

    float value  = GetModifierValue(unitMod, BASE_VALUE) + GetCreatePowers(power);
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE) +  addValue * 15.0f;
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxPower(power, uint32(value));
}

void Unit::UpdateArmor()
{
    float value = 0.0f;
    UnitMods unitMod = UNIT_MOD_ARMOR;

    value  = GetModifierValue(unitMod, BASE_VALUE) + GetStat(STAT_AGILITY) * 2.0f;
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE);
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetArmor(int32(value));
}

void Unit::UpdateAttackPowerAndDamage(bool ranged)
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
        default:           val2 = 0.0f;                                                   break;
        }
    }
    else
    {
        switch(getClass())
        {
        case CLASS_WARRIOR: val2 = level*3.0f + GetStat(STAT_STRENGTH)*2.0f - 20.0f; break;
        case CLASS_PALADIN: val2 = GetStat(STAT_STRENGTH)*2.0f - 20.0f; break;
        case CLASS_ROGUE:   val2 = GetStat(STAT_STRENGTH) + GetStat(STAT_AGILITY) - 20.0f; break;
        case CLASS_HUNTER:  val2 = GetStat(STAT_STRENGTH) + GetStat(STAT_AGILITY) - 20.0f; break;
        case CLASS_SHAMAN:  val2 = level*2.0f + GetStat(STAT_STRENGTH)*2.0f - 20.0f; break;
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

    SetUInt32Value(index, uint32(base_attPower));           //UNIT_FIELD_(RANGED)_ATTACK_POWER field
    SetUInt32Value(index_mod, uint32(attPowerMod));         //UNIT_FIELD_(RANGED)_ATTACK_POWER_MODS field
    SetFloatValue(index_mult, attPowerMultiplier);          //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field

    //automatically update weapon damage after attack power modification
    if(ranged)
    {
        UpdateDamagePhysical(RANGED_ATTACK);
    }
    else
    {
        UpdateDamagePhysical(BASE_ATTACK);
        UpdateDamagePhysical(OFF_ATTACK);
    }  
}

void Unit::UpdateDamagePhysical(WeaponAttackType attType)
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
    
    if(GetTypeId() == TYPEID_PLAYER)
    {
        if(!((Player*)this)->IsUseEquipedWeapon()) //check if player is druid and in cat or bear forms
        {
            weapon_mindamage = 0.9 * GetTotalAuraModValue(attPower)/ 14.0f * att_speed;
            weapon_maxdamage = 1.1 * GetTotalAuraModValue(attPower)/ 14.0f * att_speed;
        }
        else if(attType == RANGED_ATTACK) //add ammo DPS to ranged damage
        {
            weapon_mindamage += ((Player*)this)->GetAmmoDPS() * att_speed;
            weapon_maxdamage += ((Player*)this)->GetAmmoDPS() * att_speed;
        }
    }

    float mindamage = ((base_value + weapon_mindamage) * base_pct + total_value) * total_pct ;
    float maxdamage = ((base_value + weapon_maxdamage) * base_pct + total_value) * total_pct ;

    SetStatFloatValue(index1, mindamage);
    SetStatFloatValue(index2, maxdamage);
}

float Unit::GetTotalAttackPowerValue(WeaponAttackType attType) const
{
    UnitMods unitMod = (attType == RANGED_ATTACK) ? UNIT_MOD_ATTACK_POWER_RANGED : UNIT_MOD_ATTACK_POWER;

    float val = GetTotalAuraModValue(unitMod);
    if(val < 0.0f)
        val = 0.0f;

    return val;
}

float Unit::GetWeaponDamageRange(WeaponAttackType attType ,WeaponDamageRange type) const
{
    if (attType == OFF_ATTACK && !haveOffhandWeapon())
        return 0.0f;

    return m_weaponDamage[attType][type];
}

void Unit::SetLevel(uint32 lvl)
{
    SetUInt32Value(UNIT_FIELD_LEVEL,lvl);
    
    // group update
    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_LEVEL);
}

void Unit::SetHealth(uint32 val)
{
    uint32 maxHealth = GetMaxHealth();
    if(maxHealth < val)
        val = maxHealth;

    SetUInt32Value(UNIT_FIELD_HEALTH,val);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_HP);
}

void Unit::SetMaxHealth(uint32 val) 
{
    uint32 health = GetHealth();
    SetUInt32Value(UNIT_FIELD_MAXHEALTH,val); 

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_MAX_HP);

    if(val < health)
        SetHealth(val);
}

void Unit::SetPower(Powers power, uint32 val) 
{ 
    uint32 maxPower = GetMaxPower(power);
    if(maxPower < val)
        val = maxPower;

    SetStatInt32Value(UNIT_FIELD_POWER1 + power,val);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POWER);
}

void Unit::SetMaxPower(Powers power, uint32 val) 
{
    uint32 cur_power = GetPower(power);
    SetStatInt32Value(UNIT_FIELD_MAXPOWER1 + power,val);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_MAX_POWER);

    if(val < cur_power)
        SetPower(power, val);
}

void Unit::ApplyPowerMod(Powers power, uint32 val, bool apply)
{
    ApplyModUInt32Value(UNIT_FIELD_POWER1+power, val, apply);
    
    // group update
    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POWER);
}

void Unit::ApplyMaxPowerMod(Powers power, uint32 val, bool apply)
{
    ApplyModUInt32Value(UNIT_FIELD_MAXPOWER1+power, val, apply);
    
    // group update
    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_MAX_POWER);
}

void Unit::ApplyAuraProcTriggerDamage( Aura* aura, bool apply )
{
    AuraList& tAuraProcTriggerDamage = m_modAuras[SPELL_AURA_PROC_TRIGGER_DAMAGE];
    if(apply)
        tAuraProcTriggerDamage.push_back(aura);
    else
        tAuraProcTriggerDamage.remove(aura);
}
