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
#include "FactionTemplateResolver.h"

#include <math.h>

float baseMoveSpeed[MAX_MOVE_TYPE] =
{
    2.5f,                                                   // MOVE_WALK
    7.0f,                                                   // MOVE_RUN
    1.25f,                                                  // MOVE_WALKBACK
    4.722222f,                                              // MOVE_SWIM
    4.5f,                                                   // MOVE_SWIMBACK
    3.141594f                                               // MOVE_TURN
};

Unit::Unit() : WorldObject()
{
    m_objectType |= TYPE_UNIT;
    m_objectTypeId = TYPEID_UNIT;

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
    //m_Aura = NULL;
    //m_AurasCheck = 2000;
    //m_removeAuraTimer = 4;
    //tmpAura = NULL;
    m_silenced = false;
    waterbreath = false;

    m_Visibility = VISIBILITY_ON;
    m_UpdateVisibility = VISIBLE_NOCHANGES;

    m_detectStealth = 0;
    m_stealthvalue = 0;
    m_transform = 0;
    m_ShapeShiftForm = 0;

    for (int i = 0; i < TOTAL_AURAS; i++)
        m_AuraModifiers[i] = -1;
    for (int i = 0; i < IMMUNITY_MECHANIC; i++)
        m_spellImmune[i].clear();

    m_attacking = NULL;
    m_modDamagePCT = 0;
    m_modHitChance = 0;
    m_modSpellHitChance = 0;
    m_baseSpellCritChance = 5;
    m_modCastSpeedPct = 0;
    m_CombatTimer = 0;

    for (int i = 0; i < MAX_MOVE_TYPE; ++i)
        m_speed_rate[i] = 1.0f;
}

Unit::~Unit()
{
    // remove references to unit
    std::list<GameObject*>::iterator i;
    for (i = m_gameObj.begin(); i != m_gameObj.end();)
    {
        (*i)->SetOwnerGUID(0);
        (*i)->SetRespawnTime(0);
        (*i)->Delete();
        i = m_gameObj.erase(i);
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
    _UpdateHostil( p_time );

    if (isInCombat() && GetTypeId() == TYPEID_PLAYER )      //update combat timer only for players
    {
        if ( m_CombatTimer <= p_time )
        {
            ClearInCombat();
        }
        else
            m_CombatTimer -= p_time;
    }

    if(uint32 base_att = getAttackTimer(BASE_ATTACK))
    {
        setAttackTimer(BASE_ATTACK, (p_time >= base_att ? 0 : base_att - p_time) );
    }
    if(GetHealth() < GetMaxHealth()*0.2)
        SetFlag(UNIT_FIELD_AURASTATE, uint32(1<<1));
    else RemoveFlag(UNIT_FIELD_AURASTATE, uint32(1<<1));
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
            dist = ::sqrt(dist);
        double speed = GetSpeed(run ? MOVE_RUN : MOVE_WALK);
        if(speed<=0)
            speed = 2.5f;
        speed *= 0.001f;
        transitTime = static_cast<uint32>(dist / speed + 0.5);
    }
    //float orientation = (float)atan2((double)dy, (double)dx);
    SendMonsterMove(x,y,z,false,run,transitTime);
}

void Unit::SendMonsterMove(float NewPosX, float NewPosY, float NewPosZ, bool Walkback, bool Run, uint32 Time)
{
    WorldPacket data( SMSG_MONSTER_MOVE, (41+GetPackGUID().size()) );
    data.append(GetPackGUID());
    // Point A, starting location
    data << GetPositionX() << GetPositionY() << GetPositionZ();
    // unknown field - unrelated to orientation
    // seems to increment about 1000 for every 1.7 seconds
    // for now, we'll just use mstime
    data << getMSTime();

    data << uint8(Walkback);                                // walkback when walking from A to B
    data << uint32(Run ? 0x00000100 : 0x00000000);          // flags
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
        m_attackTimer[type] = uint32(2000 * m_modAttackSpeedPct[type]);
}

bool Unit::canReachWithAttack(Unit *pVictim) const
{
    assert(pVictim);
    float reach = GetFloatValue(UNIT_FIELD_COMBATREACH);
    if( reach <= 0.0f )
        reach = 1.0f;
    return IsWithinDist(pVictim, reach);
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

void Unit::RemoveSpellbyDamageTaken(uint32 auraType, DamageEffectType damagetype)
{
    if(!HasAuraType(auraType))
        return;

    AuraList& damageaura = GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);

    switch(damagetype)
    {
        case DIRECT_DAMAGE:
        case SPELL_DIRECT_DAMAGE:
            RemoveSpellsCausingAura(auraType);
            return;
        case DOT:
            if(damageaura.size()==0)
                return;
            //if(auraType == SPELL_AURA_MOD_ROOT && damageaura.size() < 2) //sometimes Roots can break themselves - need to fix
            //    return;
            if(30.0f > urand(0,100))                        //30% chance to break a spell
                RemoveSpellsCausingAura(auraType);
            return;

        default:
            return;
    }
}

void Unit::DealDamage(Unit *pVictim, uint32 damage, DamageEffectType damagetype, uint8 damageSchool, SpellEntry const *spellProto, uint32 procFlag, bool durabilityLoss)
{
    if (!pVictim->isAlive() || pVictim->isInFlight()) return;

    if(!damage) return;

    if(HasStealthAura())
        RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
    if(HasInvisibilityAura())
        RemoveSpellsCausingAura(SPELL_AURA_MOD_INVISIBILITY);

    pVictim->RemoveSpellbyDamageTaken(SPELL_AURA_MOD_FEAR, damagetype);
    pVictim->RemoveSpellbyDamageTaken(SPELL_AURA_MOD_ROOT, damagetype);

    if(pVictim->GetTypeId() != TYPEID_PLAYER)
    {
        //pVictim->SetInFront(this);
        // no loot,xp,health if type 8 /critters/
        if ( ((Creature*)pVictim)->GetCreatureInfo()->type == 8)
        {
            pVictim->setDeathState(JUST_DIED);
            pVictim->SetHealth(0);
            pVictim->CombatStop();
            return;
        }
        ((Creature*)pVictim)->AI().AttackStart(this);
    }

    DEBUG_LOG("DealDamageStart");

    uint32 health = pVictim->GetHealth();
    sLog.outDetail("deal dmg:%d to heals:%d ",damage,health);

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

    if (health <= damage)
    {
        
        DEBUG_LOG("DealDamage: victim just died");

        DEBUG_LOG("DealDamageAttackStop");
        AttackStop();
        pVictim->CombatStop();

        DEBUG_LOG("SET JUST_DIED");
        pVictim->setDeathState(JUST_DIED);

        DEBUG_LOG("DealDamageHealth1");
        pVictim->SetHealth(0);

        // 10% durability loss on death
        // clean hostilList
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
            HostilList::iterator i;
            for(i = m_hostilList.begin(); i != m_hostilList.end(); i++)
            {
                if(i->UnitGuid==pVictim->GetGUID())
                {
                    m_hostilList.erase(i);
                    break;
                }
            }

            Pet *pet = pVictim->GetPet();
            if(pet)
            {
                pet->setDeathState(JUST_DIED);
                pet->CombatStop();
                pet->SetHealth(0);
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
            if(!((Creature*)pVictim)->isPet())
                pVictim->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        }

        //judge if GainXP, Pet kill like player kill,kill pet not like PvP
        bool PvP = false;
        Player *player = 0;

        if(GetTypeId() == TYPEID_PLAYER)
        {
            player = (Player*)this;
            if(pVictim->GetTypeId() == TYPEID_PLAYER)
                PvP = true;
        }
        else if(GetOwnerGUID())                             // Pet or timed creature, etc
        {
            Creature* pet = (Creature*)this;
            Unit* owner = ((Creature*)this)->GetOwner();

            if(owner && owner->GetTypeId() == TYPEID_PLAYER)
            {
                player = (Player*)owner;
                player->ClearInCombat();
            }

            if(pet->isPet())
            {
                uint32 petxp = MaNGOS::XP::BaseGain(getLevel(), pVictim->getLevel());
                ((Pet*)pet)->GivePetXP(petxp);
            }
        }

        // self or owner of pet
        if(player)
        {
            float honordiff = player->GetTotalHonor();
            player->CalculateHonor(pVictim);
            honordiff -= player->GetTotalHonor();

            player->CalculateReputation(pVictim);

            if(!PvP)
            {
                DEBUG_LOG("DealDamageIsPvE");
                uint32 xp = MaNGOS::XP::Gain(player, pVictim);

                Group *pGroup = player->groupInfo.group;
                if(pGroup)
                {
                    DEBUG_LOG("Kill Enemy In Group");
                    xp /= pGroup->GetMembersCount();
                    for (uint32 i = 0; i < pGroup->GetMembersCount(); i++)
                    {
                        Player *pGroupGuy = objmgr.GetPlayer(pGroup->GetMemberGUID(i));
                        if(!pGroupGuy)
                            continue;
                        if(pVictim->GetDistanceSq(pGroupGuy) > sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE))
                            continue;
                        if(uint32(abs((int)pGroupGuy->getLevel() - (int)pVictim->getLevel())) > sWorld.getConfig(CONFIG_GROUP_XP_LEVELDIFF))
                            continue;
                        pGroupGuy->GiveXP(xp, pVictim);
                        if(Pet* pet = player->GetPet())
                        {
                            pet->GivePetXP(xp/2);
                        }
                        pGroupGuy->KilledMonster(pVictim->GetEntry(), pVictim->GetGUID());
                    }
                }
                else
                {
                    DEBUG_LOG("Player kill enemy alone");
                    player->GiveXP(xp, pVictim);
                    if(Pet* pet = player->GetPet())
                    {
                        pet->GivePetXP(xp);
                    }
                    player->KilledMonster(pVictim->GetEntry(),pVictim->GetGUID());
                }

                if(xp || honordiff < 0)
                    ProcDamageAndSpell(pVictim,PROC_FLAG_KILL_XP_GIVER,PROC_FLAG_NONE);
            }
        }
        else
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
    else
    {
        DEBUG_LOG("DealDamageAlive");

        // Check if health is below 20%
        if((health-damage)*5 < pVictim->GetMaxHealth())
        {
            uint32 procVictim = PROC_FLAG_NONE;
            if(health*5 > pVictim->GetMaxHealth())          // if just dropped below 20% (for CheatDeath)
                procVictim = PROC_FLAG_LOW_HEALTH;
            ProcDamageAndSpell(pVictim,PROC_FLAG_TARGET_LOW_HEALTH,procVictim);
        }

        pVictim->ModifyHealth(- (int32)damage);

        if(damagetype != DOT)
        {
            Attack(pVictim);
            if(damagetype == DIRECT_DAMAGE)                 //start melee attacks only after melee hit
                SendAttackStart(pVictim);
        }


        if(pVictim->getTransForm())
        {
            pVictim->RemoveAurasDueToSpell(pVictim->getTransForm());
            pVictim->setTransForm(0);
        }

        if (pVictim->GetTypeId() != TYPEID_PLAYER)
        {
            ((Creature *)pVictim)->AI().DamageInflict(this, damage);
            if(spellProto && IsDamageToThreatSpell(spellProto))
                damage *= 2;
            pVictim->AddHostil(this->GetGUID(), float (damage));
        }
        else
        {
            // rage from recieved damage (from creatures and players)
            if( pVictim != this                             // not generate rage for self damage (falls, ...)
                // warrior and some druid forms
                && (((Player*)pVictim)->getPowerType() == POWER_RAGE))
                ((Player*)pVictim)->CalcRage(damage,false);

            // random durability for items (HIT)
            int randdurability = urand(0, 300);
            if (randdurability == 10)
            {
                DEBUG_LOG("HIT: We decrease durability with 5 percent");
                ((Player*)pVictim)->DurabilityLossAll(0.05);
            }
        }

        // TODO: Store auras by interrupt flag to speed this up.
        // TODO: Fix roots that should not break from its own damage.
        AuraMap& vAuras = pVictim->GetAuras();
        for (AuraMap::iterator i = vAuras.begin(), next; i != vAuras.end(); i = next)
        {
            next = i; next++;
            if (i->second->GetSpellProto()->AuraInterruptFlags & (1<<1))
            {
                bool remove = true;
                if (i->second->GetSpellProto()->procFlags & (1<<3))
                    if (i->second->GetSpellProto()->procChance < rand_chance())
                        remove = false;
                if (remove)
                {
                    pVictim->RemoveAurasDueToSpell(i->second->GetId());
                    next = vAuras.begin();
                }
            }
        }

        if(pVictim->m_currentSpell && pVictim->GetTypeId() == TYPEID_PLAYER && damage
            && pVictim->m_currentSpell->getState() == SPELL_STATE_CASTING)
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

            he->CastSpell(he, 7267, false);                 // beg
            he->DuelComplete(1);
        }
    }

    DEBUG_LOG("DealDamageEnd");
}

void Unit::CastSpell(Unit* Victim, uint32 spellId, bool triggered, Item *castItem)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId );

    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    CastSpell(Victim,spellInfo,triggered,castItem);

    /*     if (castItem)
    DEBUG_LOG("WORLD: cast Item spellId - %i", spellId);

    Spell *spell = new Spell(this, spellInfo, triggered, 0);
    WPAssert(spell);

    SpellCastTargets targets;
    targets.setUnitTarget( Victim );
    spell->m_CastItem = castItem;
    spell->prepare(&targets);
    m_canMove = false;
    if (triggered) delete spell;
    */
}

void Unit::CastSpell(Unit* Victim,SpellEntry const *spellInfo, bool triggered, Item *castItem)
{
    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell ");
        return;
    }

    if (castItem)
        DEBUG_LOG("WORLD: cast Item spellId - %i", spellInfo->Id);

    Spell *spell = new Spell(this, spellInfo, triggered, 0);
    WPAssert(spell);

    SpellCastTargets targets;
    targets.setUnitTarget( Victim );
    spell->m_CastItem = castItem;
    spell->prepare(&targets);
    m_canMove = false;
    if (triggered) delete spell;                            // triggered spell not self deleted
}

void Unit::SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage, bool isTriggeredSpell)
{
    if(!this || !pVictim)
        return;
    if(!this->isAlive() || !pVictim->isAlive())
        return;
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellID);
    if(!spellInfo)
        return;

    uint32 absorb=0;
    uint32 resist=0;

    //Spell miss (sends resist message)
    if(SpellMissChanceCalc(pVictim) > urand(0,10000))
    {
        ProcDamageAndSpell(pVictim, PROC_FLAG_TARGET_RESISTS, PROC_FLAG_RESIST_SPELL, 0, spellInfo, isTriggeredSpell);
        SendAttackStateUpdate(HITINFO_RESIST|HITINFO_SWINGNOHITSOUND, pVictim, 1, spellInfo->School, 0, 0,0,1,0);
        return;
    }

    uint32 pdamage = SpellDamageBonus(pVictim,spellInfo,damage);
    bool crit = SpellCriticalBonus(spellInfo, (int32*)&pdamage);

    //Calculate armor mitigation if it is a physical spell
    if (spellInfo->School == 0)
        pdamage = CalcArmorReducedDamage(pVictim, damage);
    CalcAbsorbResist(pVictim,spellInfo->School,pdamage, &absorb, &resist);

    // Only send absorbed message if we actually absorbed some damage
    if( pdamage <= absorb+resist && absorb)
    {
        SendAttackStateUpdate(HITINFO_ABSORB|HITINFO_SWINGNOHITSOUND, pVictim, 1, spellInfo->School, pdamage, absorb,resist,1,0);
        return;
    }else                                                   // If we didn't fully absorb check if we fully resisted
    if( pdamage <= resist)
    {
        ProcDamageAndSpell(pVictim, PROC_FLAG_TARGET_RESISTS, PROC_FLAG_RESIST_SPELL, 0, spellInfo, isTriggeredSpell);
        SendAttackStateUpdate(HITINFO_RESIST|HITINFO_SWINGNOHITSOUND, pVictim, 1, spellInfo->School, pdamage, absorb,resist,1,0);
        return;
    }

    sLog.outDetail("SpellNonMeleeDamageLog: %u %X attacked %u %X for %u dmg inflicted by %u,abs is %u,resist is %u",
        GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), pdamage, spellID, absorb, resist);

    SendSpellNonMeleeDamageLog(pVictim, spellID, pdamage, spellInfo->School, absorb, resist, false, 0, crit);
    DealDamage(pVictim, (pdamage-absorb-resist), SPELL_DIRECT_DAMAGE, spellInfo->School, spellInfo, 0, true);


    uint32 procAttacker = PROC_FLAG_HIT_SPELL;
    uint32 procVictim   = (PROC_FLAG_STRUCK_SPELL|PROC_FLAG_TAKE_DAMAGE);

    if (crit)
    {
        procAttacker |= PROC_FLAG_CRIT_SPELL;
        procVictim   |= PROC_FLAG_STRUCK_CRIT_SPELL;
    }

    ProcDamageAndSpell(pVictim, procAttacker, procVictim, (pdamage-absorb-resist), spellInfo, isTriggeredSpell);
}

void Unit::PeriodicAuraLog(Unit *pVictim, SpellEntry const *spellProto, Modifier *mod)
{
    uint32 procFlag = 0;
    if(!this || !pVictim || !isAlive() || !pVictim->isAlive())
    {
        return;
    }
    uint32 absorb=0;
    uint32 resist=0;

    uint32 pdamage = mod->m_amount;

    if(mod->m_auraname != SPELL_AURA_PERIODIC_HEAL && mod->m_auraname != SPELL_AURA_OBS_MOD_HEALTH)
    {
        //Calculate armor mitigation if it is a physical spell
        if (spellProto->School == 0)
            pdamage = CalcArmorReducedDamage(pVictim, pdamage);

        CalcAbsorbResist(pVictim, spellProto->School, pdamage, &absorb, &resist);
    }

    sLog.outDetail("PeriodicAuraLog: %u %X attacked %u %X for %u dmg inflicted by %u abs is %u",
        GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), pdamage, spellProto->Id,absorb);

    WorldPacket data(SMSG_PERIODICAURALOG, (21+16));        // we guess size
    data.append(pVictim->GetPackGUID());
    data.append(this->GetPackGUID());
    data << spellProto->Id;
    data << uint32(1);

    data << mod->m_auraname;
    data << (uint32)(mod->m_amount);
    data << spellProto->School;
    data << uint32(0);
    SendMessageToSet(&data,true);

    if(mod->m_auraname == SPELL_AURA_PERIODIC_DAMAGE)
    {
        DamageEffectType damagetype = DOT;
        pdamage = SpellDamageBonus(pVictim, spellProto, pdamage);
        SendSpellNonMeleeDamageLog(pVictim, spellProto->Id, mod->m_amount, spellProto->School, absorb, resist, false, 0);
        SendMessageToSet(&data,true);
        for(uint8 i = 0; i < 3; i++)
        {
            //HasAuraType(SPELL_AURA_MOD_ROOT))
            if(spellProto->EffectApplyAuraName[i] == SPELL_AURA_MOD_ROOT)
                damagetype = SELF_DAMAGE;
        }
        DealDamage(pVictim, mod->m_amount <= int32(absorb+resist) ? 0 : (mod->m_amount-absorb-resist), damagetype, spellProto->School, spellProto, procFlag, true);
        ProcDamageAndSpell(pVictim, PROC_FLAG_HIT_SPELL, PROC_FLAG_TAKE_DAMAGE, mod->m_amount <= int32(absorb+resist) ? 0 : (mod->m_amount-absorb-resist), spellProto);
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_DAMAGE_PERCENT)
    {
        pdamage = SpellDamageBonus(pVictim, spellProto, pdamage);
        int32 pdamage = GetHealth()*(100+mod->m_amount)/100;
        SendSpellNonMeleeDamageLog(pVictim, spellProto->Id, pdamage, spellProto->School, absorb, resist, false, 0);
        SendMessageToSet(&data,true);
        DealDamage(pVictim, pdamage <= int32(absorb+resist) ? 0 : (pdamage-absorb-resist), DOT, spellProto->School, spellProto, procFlag, true);

        ProcDamageAndSpell(pVictim, PROC_FLAG_HIT_SPELL, PROC_FLAG_TAKE_DAMAGE, pdamage <= int32(absorb+resist) ? 0 : (pdamage-absorb-resist), spellProto);
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_HEAL || mod->m_auraname == SPELL_AURA_OBS_MOD_HEALTH)
    {
        pdamage = SpellHealingBonus(spellProto, pdamage);
        pVictim->ModifyHealth(pdamage);

        if(pVictim->GetTypeId() == TYPEID_PLAYER || GetTypeId() == TYPEID_PLAYER)
            SendHealSpellOnPlayer(pVictim, spellProto->Id, pdamage);
        if(mod->m_auraname == SPELL_AURA_PERIODIC_HEAL && pVictim != this)
            ProcDamageAndSpell(pVictim, PROC_FLAG_HEAL, PROC_FLAG_NONE, pdamage, spellProto);
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_LEECH)
    {
        uint32 tmpvalue = 0;
        float tmpvalue2 = 0;
        for(int x=0;x<3;x++)
        {
            if(mod->m_auraname != spellProto->EffectApplyAuraName[x])
                continue;
            tmpvalue2 = spellProto->EffectMultipleValue[x];
            tmpvalue2 = tmpvalue2 > 0 ? tmpvalue2 : 1;

            if(pVictim->GetHealth() - mod->m_amount > 0)
                tmpvalue = uint32(mod->m_amount*tmpvalue2);
            else
                tmpvalue = uint32(pVictim->GetHealth()*tmpvalue2);

            SendSpellNonMeleeDamageLog(pVictim, spellProto->Id, tmpvalue, spellProto->School, absorb, resist, false, 0);
            DealDamage(pVictim, mod->m_amount <= int32(absorb+resist) ? 0 : (mod->m_amount-absorb-resist), DOT, spellProto->School, spellProto, procFlag, false);
            ProcDamageAndSpell(pVictim, PROC_FLAG_HIT_SPELL, PROC_FLAG_TAKE_DAMAGE, mod->m_amount <= int32(absorb+resist) ? 0 : (mod->m_amount-absorb-resist), spellProto);
            if (!pVictim->isAlive() && m_currentSpell)
                if (m_currentSpell->m_spellInfo)
                    if (m_currentSpell->m_spellInfo->Id == spellProto->Id)
                        m_currentSpell->cancel();

            break;
        }

        ModifyHealth(tmpvalue);

        if(pVictim->GetTypeId() == TYPEID_PLAYER || GetTypeId() == TYPEID_PLAYER)
            pVictim->SendHealSpellOnPlayer(this, spellProto->Id, tmpvalue);
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_MANA_LEECH)
    {
        if(pVictim->getPowerType() != POWER_MANA)
            return;
        if(getPowerType() != POWER_MANA)
            return;

        uint32 tmpvalue = 0;
        for(int x=0;x<3;x++)
        {
            if(mod->m_auraname != spellProto->EffectApplyAuraName[x])
                continue;

            int32 amount;
            if(int32(pVictim->GetPower(POWER_MANA)) > mod->m_amount)
                amount = mod->m_amount;
            else
                amount = pVictim->GetPower(POWER_MANA);

            pVictim->ModifyPower(POWER_MANA, - amount);

            tmpvalue = uint32(amount*spellProto->EffectMultipleValue[x]);
            break;
        }

        ModifyPower(POWER_MANA,tmpvalue);

        if(pVictim->GetTypeId() == TYPEID_PLAYER || GetTypeId() == TYPEID_PLAYER)
            SendHealSpellOnPlayerPet(this, spellProto->Id, tmpvalue, POWER_MANA);
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_ENERGIZE)
    {
        if(mod->m_miscvalue < 0 || mod->m_miscvalue > 4)
            return;

        Powers power = Powers(mod->m_miscvalue);

        if(getPowerType() != power)
            return;

        ModifyPower(power,mod->m_amount);

        if(pVictim->GetTypeId() == TYPEID_PLAYER || GetTypeId() == TYPEID_PLAYER)
            SendHealSpellOnPlayerPet(pVictim, spellProto->Id, mod->m_amount, power);
    }
    else if(mod->m_auraname == SPELL_AURA_OBS_MOD_MANA)
    {
        if(getPowerType() != POWER_MANA)
            return;

        ModifyPower(POWER_MANA, mod->m_amount);
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
    float tmpvalue = armor / (getLevel() * 85.0 + 400.0 +armor);

    if(tmpvalue < 0)
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
        float tmpvalue2 = pVictim->GetResistance(SpellSchools(School));
        if (tmpvalue2 < 0) tmpvalue2 = 0;
        *resist += uint32(damage*tmpvalue2*0.0025*pVictim->getLevel()/getLevel());
        if(*resist > damage)
            *resist = damage;
    }else *resist = 0;

    int32 RemainingDamage = damage - *resist;
    int32 currentAbsorb, manaReduction, maxAbsorb;
    float manaMultiplier;

    if (School == SPELL_SCHOOL_NORMAL)
    {
        AuraList& vManaShield = pVictim->GetAurasByType(SPELL_AURA_MANA_SHIELD);
        for(AuraList::iterator i = vManaShield.begin(), next; i != vManaShield.end() && RemainingDamage >= 0; i = next)
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

    AuraList& vSchoolAbsorb = pVictim->GetAurasByType(SPELL_AURA_SCHOOL_ABSORB);
    for(AuraList::iterator i = vSchoolAbsorb.begin(), next; i != vSchoolAbsorb.end() && RemainingDamage >= 0; i = next)
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

void Unit::DoAttackDamage (Unit *pVictim, uint32 *damage, uint32 *blocked_amount, uint32 *damageType, uint32 *hitInfo, uint32 *victimState, uint32 *absorbDamage, uint32 *resistDamage, WeaponAttackType attType, SpellEntry const *spellCasted, bool isTriggeredSpell)
{
    pVictim->RemoveFlag(UNIT_FIELD_AURASTATE, uint32((1<<(AURA_STATE_PARRY-1)) | 1<<(AURA_STATE_DODGE-1)));
    MeleeHitOutcome outcome = RollMeleeOutcomeAgainst (pVictim, attType);
    if (outcome == MELEE_HIT_MISS)
    {
        *hitInfo |= HITINFO_MISS;
        if(GetTypeId()== TYPEID_PLAYER)
            ((Player*)this)->UpdateWeaponSkill(attType);
        return;
    }

    *damage += CalculateDamage (attType);

    //Calculate the damage after armor mitigation if SPELL_SCHOOL_NORMAL
    if (*damageType == SPELL_SCHOOL_NORMAL)
        *damage = CalcArmorReducedDamage(pVictim, *damage);

    if(GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() != TYPEID_PLAYER && ((Creature*)pVictim)->GetCreatureInfo()->type != 8 )
        ((Player*)this)->UpdateCombatSkills(pVictim, attType, outcome, false);
    if(GetTypeId() != TYPEID_PLAYER && pVictim->GetTypeId() == TYPEID_PLAYER)
        ((Player*)pVictim)->UpdateCombatSkills(this, attType, outcome, true);

    switch (outcome)
    {
        case MELEE_HIT_CRIT:
            //*hitInfo = 0xEA;
            // 0xEA
            *hitInfo  = HITINFO_CRITICALHIT | HITINFO_NORMALSWING2 | 0x8;
            *damage *= 2;

            if(GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() != TYPEID_PLAYER && ((Creature*)pVictim)->GetCreatureInfo()->type != 8 )
                ((Player*)this)->UpdateWeaponSkill(attType);

            pVictim->HandleEmoteCommand(EMOTE_ONESHOT_WOUNDCRITICAL);
            break;

        case MELEE_HIT_PARRY:
            *damage = 0;
            *victimState = VICTIMSTATE_PARRY;

            // instant (maybe with small delay) counter attack
            {
                uint32 offtime  = pVictim->getAttackTimer(OFF_ATTACK);
                uint32 basetime = pVictim->getAttackTimer(BASE_ATTACK);

                if (pVictim->haveOffhandWeapon() && offtime < basetime)
                {
                    if( offtime > ATTACK_DISPLAY_DELAY )
                        pVictim->setAttackTimer(OFF_ATTACK,ATTACK_DISPLAY_DELAY);
                }
                else
                {
                    if ( basetime > ATTACK_DISPLAY_DELAY )
                        pVictim->setAttackTimer(BASE_ATTACK,ATTACK_DISPLAY_DELAY);
                }
            }

            if(pVictim->GetTypeId() == TYPEID_PLAYER)
                ((Player*)pVictim)->UpdateDefense();

            pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);
            pVictim->SetFlag(UNIT_FIELD_AURASTATE, uint32(1<<(AURA_STATE_DODGE-1)));
            return;

        case MELEE_HIT_DODGE:
            *damage = 0;
            *victimState = VICTIMSTATE_DODGE;

            if(pVictim->GetTypeId() == TYPEID_PLAYER)
                ((Player*)pVictim)->UpdateDefense();

            pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);
            pVictim->SetFlag(UNIT_FIELD_AURASTATE, uint32(1<<(AURA_STATE_DODGE-1)));
            return;

        case MELEE_HIT_BLOCK:
            *blocked_amount = uint32(pVictim->GetBlockValue() + (pVictim->GetStat(STAT_STRENGTH) / 20) -1);

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
            pVictim->SetFlag(UNIT_FIELD_AURASTATE, uint32(1<<(AURA_STATE_DODGE-1)));
            break;

        case MELEE_HIT_GLANCING:
        {
            // 30% reduction at 15 skill diff, no reduction at 5 skill diff
            int32 reducePerc = 100 - (pVictim->GetDefenceSkillValue() - GetWeaponSkillValue(attType) - 5) * 3;
            if (reducePerc < 70)
                reducePerc = 70;
            *damage = *damage * reducePerc / 100;
            *hitInfo |= HITINFO_GLANCING;
            break;
        }
        case MELEE_HIT_CRUSHING:
        {
            // 150% normal damage
            *damage += (*damage / 2);
            *hitInfo |= HITINFO_CRUSHING;
            // TODO: victimState, victim animation?
            break;
        }
        default:
            break;
    }

    MeleeDamageBonus(pVictim, damage);
    CalcAbsorbResist(pVictim, *damageType, *damage-*blocked_amount, absorbDamage, resistDamage);

    if (*absorbDamage) *hitInfo |= HITINFO_ABSORB;
    if (*resistDamage) *hitInfo |= HITINFO_RESIST;

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
    AuraList& vDamageShields = pVictim->GetAurasByType(SPELL_AURA_DAMAGE_SHIELD);
    for(AuraList::iterator i = vDamageShields.begin(); i != vDamageShields.end(); ++i)
        pVictim->SpellNonMeleeDamageLog(this, (*i)->GetId(), (*i)->GetModifier()->m_amount);

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
    if(hasUnitState(UNIT_STAT_CONFUSED | UNIT_STAT_STUNDED | UNIT_STAT_FLEEING) )
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
    uint32   blocked_dmg = 0;
    uint32   absorbed_dmg = 0;
    uint32   resisted_dmg = 0;

    if( pVictim->IsImmunedToPhysicalDamage() )
    {
        SendAttackStateUpdate (HITINFO_MISS, pVictim, 1, NORMAL_DAMAGE, 0, 0, 0, VICTIMSTATE_IS_IMMUNE, 0);
        return;
    }

    DoAttackDamage (pVictim, &damage, &blocked_dmg, &damageType, &hitInfo, &victimState, &absorbed_dmg, &resisted_dmg, attType);

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

        DealDamage (pVictim, damage, DIRECT_DAMAGE, SPELL_SCHOOL_NORMAL, NULL, 0, true);

        // rage from maked damage TO creatures and players (target dead case)
        if(GetTypeId() == TYPEID_PLAYER && (getPowerType() == POWER_RAGE))
            ((Player*)this)->CalcRage(damage,true);

        if(GetTypeId() == TYPEID_PLAYER && pVictim->isAlive())
        {
            for(int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
                ((Player*)this)->CastItemCombatSpell(((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0,i),pVictim);
        }
    }

    if (GetTypeId() == TYPEID_PLAYER)
        DEBUG_LOG("AttackerStateUpdate: (Player) %u %X attacked %u %X for %u dmg, absorbed %u, blocked %u, resisted %u.",
            GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage, absorbed_dmg, blocked_dmg, resisted_dmg);
    else
        DEBUG_LOG("AttackerStateUpdate: (NPC)    %u %X attacked %u %X for %u dmg, absorbed %u, blocked %u, resisted %u.",
            GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage, absorbed_dmg, blocked_dmg, resisted_dmg);
}

MeleeHitOutcome Unit::RollMeleeOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType) const
{
    int32 skillDiff =  GetWeaponSkillValue(attType) - pVictim->GetDefenceSkillValue();
    // bonus from skills is 0.04%
    int32    skillBonus = skillDiff * 4;
    int32    skillBonus2 = 4 * ( GetWeaponSkillValue(attType) - pVictim->GetPureDefenceSkillValue() );
    int32    sum = 0, tmp = 0;
    int32    roll = urand (0, 10000);

    DEBUG_LOG ("RollMeleeOutcomeAgainst: skill bonus of %d for attacker", skillBonus);
    DEBUG_LOG ("RollMeleeOutcomeAgainst: rolled %d, +hit %d, dodge %u, parry %u, block %u, crit %u",
        roll, m_modHitChance, (uint32)(pVictim->GetUnitDodgeChance()*100), (uint32)(pVictim->GetUnitParryChance()*100),
        (uint32)(pVictim->GetUnitBlockChance()*100), (uint32)(GetUnitCriticalChance()*100));

    // dual wield has 24% base chance to miss instead of 5%, also
    // base miss rate is 5% and can't get higher than 60%
    tmp = MeleeMissChanceCalc(pVictim) - skillBonus;

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

    // FIXME: +skill and +defense has no effect on crit chance in PvP combat
    tmp = (int32)(GetUnitCriticalChance()*100) + skillBonus + modCrit;
    if (tmp > 0 && roll < (sum += tmp))
    {
        DEBUG_LOG ("RollMeleeOutcomeAgainst: CRIT <%d, %d)", sum-tmp, sum);
        return MELEE_HIT_CRIT;
    }

    // mobs can score crushing blows if they're 3 or more levels above victim
    // or when their weapon skill is 15 or more above victim's defense skill
    tmp = pVictim->GetDefenceSkillValue();
    uint32 tmpmax = pVictim->GetMaxSkillValueForLevel();
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
            // TODO: add offhand dmg from talents
            min_damage = GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE) * 0.5;
            max_damage = GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE) * 0.5;
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
    data.append(victim->GetPackGUID());
    data << uint32( 0 );
    data << (uint32)0;

    SendMessageToSet(&data, true);
    sLog.outDetail("%s %u stopped attacking %s %u", (GetTypeId()==TYPEID_PLAYER ? "player" : "creature"), GetGUIDLow(), (victim->GetTypeId()==TYPEID_PLAYER ? "player" : "creature"),victim->GetGUIDLow());

    if(victim->GetTypeId() == TYPEID_UNIT)
        ((Creature*)victim)->AI().AttackStop(this);
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

    int32 misschance = haveOffhandWeapon() ? 2400 : 500;    //base misschance for DW : melee attacks

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

uint16 Unit::GetDefenceSkillValue() const
{
    if(GetTypeId() == TYPEID_PLAYER)
        return ((Player*)this)->GetSkillValue (SKILL_DEFENSE);
    else
        return GetUnitMeleeSkill();
}

uint16 Unit::GetPureDefenceSkillValue() const
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

    return GetTypeId() == TYPEID_PLAYER ? m_floatValues[ PLAYER_DODGE_PERCENTAGE ] : 5;
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

            if(tmpitem && !tmpitem->IsBroken() &&
                (tmpitem->GetProto()->InventoryType == INVTYPE_WEAPON || tmpitem->GetProto()->InventoryType == INVTYPE_2HWEAPON))
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
            case BASE_ATTACK: slot = EQUIPMENT_SLOT_MAINHAND; break;
            case OFF_ATTACK: slot = EQUIPMENT_SLOT_OFFHAND; break;
            case RANGED_ATTACK: slot = EQUIPMENT_SLOT_RANGED; break;
        }
        Item    *item = ((Player*)this)->GetItemByPos (INVENTORY_SLOT_BAG_0, slot);

        if(attType != EQUIPMENT_SLOT_MAINHAND && (!item || item->IsBroken() ||  !((Player*)this)->IsUseEquipedWeapon() ))
            return 0;

        // in range
        uint32  skill = item && !item->IsBroken() ? item->GetSkill() : SKILL_UNARMED;
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
            case BASE_ATTACK: slot = EQUIPMENT_SLOT_MAINHAND; break;
            case OFF_ATTACK: slot = EQUIPMENT_SLOT_OFFHAND; break;
            case RANGED_ATTACK: slot = EQUIPMENT_SLOT_RANGED; break;
        }
        Item    *item = ((Player*)this)->GetItemByPos (INVENTORY_SLOT_BAG_0, slot);

        if(attType != EQUIPMENT_SLOT_MAINHAND && (!item || item->IsBroken() ||  !((Player*)this)->IsUseEquipedWeapon() ))
            return 0;

        // in range
        uint32  skill = item && !item->IsBroken() ? item->GetSkill() : SKILL_UNARMED;
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
                        castSpell(NULL);
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
                    castSpell(NULL);
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

    if(!m_dynObj.empty())
    {
        std::list<DynamicObject*>::iterator ite, dnext;
        for (ite = m_dynObj.begin(); ite != m_dynObj.end(); ite = dnext)
        {
            dnext = ite;
            //(*i)->Update( difftime );
            if( (*ite)->isFinished() )
            {
                (*ite)->Delete();
                dnext = m_dynObj.erase(ite);
            }
            else
                ++dnext;
        }
    }
    if(!m_gameObj.empty())
    {
        std::list<GameObject*>::iterator ite1, dnext1;
        for (ite1 = m_gameObj.begin(); ite1 != m_gameObj.end(); ite1 = dnext1)
        {
            dnext1 = ite1;
            //(*i)->Update( difftime );
            if( (*ite1)->isFinished() )
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

void Unit::_UpdateHostil( uint32 time )
{
    if(!isInCombat() && m_hostilList.size() )
    {
        HostilList::iterator iter, next;
        for(iter=m_hostilList.begin(); iter!=m_hostilList.end(); iter = next)
        {
            next = iter;
            iter->Hostility-=time/1000.0f;
            if(iter->Hostility<=0.0f)
            {
                next = m_hostilList.erase(iter);
            }
            else
                ++next;
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
    if(!getVictim() || guid != getVictim()->GetGUID())
        return ObjectAccessor::Instance().GetUnit(*this, guid);
    else
        return NULL;
}

void Unit::castSpell( Spell * pSpell )
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

bool Unit::isInFront(Unit const* target, float radius)
{
    return IsWithinDist(target, radius) && HasInArc( M_PI, target );
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
    return MapManager::Instance().GetMap(GetMapId())->IsInWater(GetPositionX(),GetPositionY());
}

bool Unit::IsUnderWater() const
{
    return MapManager::Instance().GetMap(GetMapId())->IsUnderWater(GetPositionX(),GetPositionY(),GetPositionZ());
}

void Unit::DeMorph()
{

    uint32 displayid = GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    SetUInt32Value(UNIT_FIELD_DISPLAYID, displayid);
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
    if (!isAlive())
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

    if (!Aur->IsPassive())                                  // passive auras stack with all
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
            std::list<Aura *> *scAuras = caster->GetSingleCastAuras();
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
        if (IsPassiveSpell((*i).second->GetId())) continue;

        uint32 i_spellId = (*i).second->GetId();
        uint32 i_effIndex = (*i).second->GetEffIndex();
        if(i_spellId != spellId)
        {
            bool sec_match = false;
            bool is_i_sec = IsSpellSingleEffectPerCaster(i_spellId);
            if( is_sec && is_i_sec )
                if (Aur->GetCasterGUID() == (*i).second->GetCasterGUID())
                    if (GetSpellSpecific(spellId) == GetSpellSpecific(i_spellId))
                        sec_match = true;
            if( sec_match || IsNoStackSpellDueToSpell(spellId, i_spellId) && !is_sec && !is_i_sec )
            {
                // if sec_match this isnt always true, needs to be rechecked
                if (IsRankSpellDueToSpell(Aur->GetSpellProto(), i_spellId))
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
    AuraMap::iterator i,next;
    for (i = m_Auras.begin(); i != m_Auras.end(); i = next)
    {
        next = i;
        next++;
        if ((*i).second && (*i).second->GetSpellProto()->Dispel == dispel_type)
        {
            if(dispel_type == 1)
            {
                bool positive = true;
                switch((*i).second->GetSpellProto()->EffectImplicitTargetA[(*i).second->GetEffIndex()])
                {
                    case TARGET_SINGLE_ENEMY:
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
                if(positive)
                    continue;
            }
            RemoveAura(i);
            if( m_Auras.empty() )
                break;
            else
                next =  m_Auras.begin();
        }
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

void Unit::RemoveAura(AuraMap::iterator &i, bool onDeath)
{
    if (IsSingleTarget((*i).second->GetId()))
    {
        if(Unit* caster = (*i).second->GetCaster())
        {
            std::list<Aura *> *scAuras = caster->GetSingleCastAuras();
            scAuras->remove((*i).second);
        }
    }
    // remove aura from party members when the caster turns off the aura
    if((*i).second->IsAreaAura())
    {
        Unit *i_target = (*i).second->GetTarget();
        if((*i).second->GetCasterGUID() == i_target->GetGUID())
        {
            Unit* i_caster = i_target;

            Group *pGroup = NULL;
            if (i_caster->GetTypeId() == TYPEID_PLAYER)
                pGroup = ((Player*)i_caster)->groupInfo.group;
            else if(((Creature*)i_caster)->isTotem())
            {
                Unit *owner = ((Totem*)i_caster)->GetOwner();
                if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                    pGroup = ((Player*)owner)->groupInfo.group;
            }

            //float radius =  GetRadius(sSpellRadiusStore.LookupEntry((*i).second->GetSpellProto()->EffectRadiusIndex[(*i).second->GetEffIndex()]));
            if(pGroup)
            {
                for(uint32 p=0;p<pGroup->GetMembersCount();p++)
                {
                    if(!pGroup->SameSubGroup(i_caster->GetGUID(), pGroup->GetMemberGUID(p)))
                        continue;

                    Unit* Target = objmgr.GetPlayer(pGroup->GetMemberGUID(p));
                    if(!Target || Target->GetGUID() == i_caster->GetGUID())
                        continue;
                    Aura *t_aura = Target->GetAura((*i).second->GetId(), (*i).second->GetEffIndex());
                    if (t_aura)
                        if (t_aura->GetCasterGUID() == i_caster->GetGUID())
                            Target->RemoveAura((*i).second->GetId(), (*i).second->GetEffIndex());
                }
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

    // must remove before removeing from list (its remove dependent auras and _i_ is only safe iterator value
    // remove the shapeshift aura's boosts
    if(Aur->GetModifier()->m_auraname == SPELL_AURA_MOD_SHAPESHIFT)
        Aur->HandleShapeshiftBoosts(false);

    m_Auras.erase(i++);
    m_removedAuras++;                                       // internal count used by unit update

    Aur->_RemoveAura();
    delete Aur;
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
        if (!iter->second->IsPassive())
            RemoveAura(iter, true);
    else
        ++iter;
    _RemoveAllAuraMods();
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

    float val;
    int32 val2,tem_att_power;
    float totalstatmods[5] = {1,1,1,1,1};
    float totalresmods[7] = {1,1,1,1,1,1,1};
    float totaldamgemods[3] = {1,1,1};

    AuraList& mModTotalStatPct = GetAurasByType(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
    for(AuraList::iterator i = mModTotalStatPct.begin(); i != mModTotalStatPct.end(); ++i)
    {
        if((*i)->GetModifier()->m_miscvalue != -1)
            totalstatmods[(*i)->GetModifier()->m_miscvalue] *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;
        else
            for (uint8 j = 0; j < MAX_STATS; j++)
                totalstatmods[j] *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;
    }
    AuraList& mModResistancePct = GetAurasByType(SPELL_AURA_MOD_RESISTANCE_PCT);
    for(AuraList::iterator i = mModResistancePct.begin(); i != mModResistancePct.end(); ++i)
        for(uint8 j = 0; j < MAX_SPELL_SCHOOOL; j++)
            if((*i)->GetModifier()->m_miscvalue & (1<<j))
                totalresmods[j] *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;

    AuraList& mModDamagePct = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for(AuraList::iterator i = mModDamagePct.begin(); i != mModDamagePct.end(); ++i)
    {
        if(((*i)->GetModifier()->m_miscvalue & 1) == 0)
            continue;

        if ((*i)->GetSpellProto()->EquippedItemClass == -1)
        {
            float mod = (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;
            for(uint8 j = 0; j < 3; ++j)
                totaldamgemods[j] *= mod;
        }
        else
        {
            Item* pItem = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            if (pItem && pItem->IsFitToSpellRequirements((*i)->GetSpellProto()))
                totaldamgemods[BASE_ATTACK] *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;

            pItem = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            if (pItem && pItem->IsFitToSpellRequirements((*i)->GetSpellProto()))
                totaldamgemods[OFF_ATTACK] *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;

            pItem = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
            if (pItem && pItem->IsFitToSpellRequirements((*i)->GetSpellProto()))
                totaldamgemods[RANGED_ATTACK] *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;
        }
    }

    for (uint8 i = 0; i < MAX_STATS; i++)
        totalstatmods[i] = totalstatmods[i] * 100.0f - 100.0f;
    for (uint8 i = 0; i < MAX_SPELL_SCHOOOL; i++)
        totalresmods[i] = totalresmods[i] * 100.0f - 100.0f;
    for (uint8 i = 0; i < 3; i++)
        totaldamgemods[i] = totaldamgemods[i] * 100.0f - 100.0f;

    // restore percent mods
    if (apply)
    {
        for (uint8 i = 0; i < MAX_STATS; i++)
        {
            if (totalstatmods[i] != 0)
            {
                ApplyStatPercentMod(Stats(i),totalstatmods[i], apply );
                ((Player*)this)->ApplyPosStatPercentMod(Stats(i),totalstatmods[i], apply );
                ((Player*)this)->ApplyNegStatPercentMod(Stats(i),totalstatmods[i], apply );
            }
        }
        for (uint8 i = 0; i < MAX_SPELL_SCHOOOL; i++)
        {
            if (totalresmods[i] != 0)
            {
                ApplyResistancePercentMod(SpellSchools(i), totalresmods[i], apply );
                ((Player*)this)->ApplyResistanceBuffModsPercentMod(SpellSchools(i),true, totalresmods[i], apply);
                ((Player*)this)->ApplyResistanceBuffModsPercentMod(SpellSchools(i),false, totalresmods[i], apply);
            }
        }

        ApplyPercentModFloatValue(UNIT_FIELD_MINDAMAGE, totaldamgemods[BASE_ATTACK], apply );
        ApplyPercentModFloatValue(UNIT_FIELD_MAXDAMAGE, totaldamgemods[BASE_ATTACK], apply );
        ApplyPercentModFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, totaldamgemods[OFF_ATTACK], apply );
        ApplyPercentModFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, totaldamgemods[OFF_ATTACK], apply );
        ApplyPercentModFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, totaldamgemods[RANGED_ATTACK], apply );
        ApplyPercentModFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, totaldamgemods[RANGED_ATTACK], apply );
    }

    // Armor
    val = 2*(GetStat(STAT_AGILITY) - ((Player*)this)->GetCreateStat(STAT_AGILITY));

    ApplyArmorMod( val, apply);

    // HP
    val2 = uint32((GetStat(STAT_STAMINA) - ((Player*)this)->GetCreateStat(STAT_STAMINA))*10);

    ApplyMaxHealthMod( val2, apply);

    // MP
    if(getClass() != CLASS_WARRIOR && getClass() != CLASS_ROGUE)
    {
        val2 = uint32((GetStat(STAT_INTELLECT) - ((Player*)this)->GetCreateStat(STAT_INTELLECT))*15);

        ApplyMaxPowerMod(POWER_MANA, val2, apply);

    }

    float classrate = 0;

    // Melee Attack Power
    // && Melee DPS - (Damage Per Second)

    //Ranged
    if(getClass() == CLASS_HUNTER)
        val2 = uint32(getLevel() * 2 + GetStat(STAT_AGILITY) * 2 - 20);
    else
        val2 = uint32(getLevel() + GetStat(STAT_AGILITY) * 2 - 20);

    if(!apply)
        tem_att_power = GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER) + GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS);

    ApplyModUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER, val2, apply);

    if(apply)
        tem_att_power = GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER) + GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS);

    val = GetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER);
    if(val>0)
        tem_att_power = uint32(val*tem_att_power);

    val = tem_att_power/14.0f * GetAttackTime(RANGED_ATTACK)/1000;
    ApplyModFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, val, apply);
    ApplyModFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, val, apply);

    //Not-ranged

    switch(getClass())
    {
        case CLASS_WARRIOR: val2 = uint32(getLevel()*3 + GetStat(STAT_STRENGTH)*2 - 20); break;
        case CLASS_PALADIN: val2 = uint32(getLevel()*3 + GetStat(STAT_STRENGTH)*2 - 20); break;
        case CLASS_ROGUE:   val2 = uint32(getLevel()*2 + GetStat(STAT_STRENGTH) + GetStat(STAT_AGILITY) - 20); break;
        case CLASS_HUNTER:  val2 = uint32(getLevel()*2 + GetStat(STAT_STRENGTH) + GetStat(STAT_AGILITY) - 20); break;
        case CLASS_SHAMAN:  val2 = uint32(getLevel()*2 + GetStat(STAT_STRENGTH)*2 - 20); break;
        case CLASS_DRUID:
            switch(m_form)
            {
                case FORM_CAT:
                    val2 = uint32(GetStat(STAT_STRENGTH)*2 - 20 + GetStat(STAT_AGILITY)); break;
                case FORM_BEAR:
                case FORM_DIREBEAR:
                    val2 = uint32(getLevel()*3 + GetStat(STAT_STRENGTH)*2 - 20); break;
                default:
                    val2 = uint32(GetStat(STAT_STRENGTH)*2 - 20); break;
            }
            break;
        case CLASS_MAGE:    val2 = uint32(GetStat(STAT_STRENGTH) - 10); break;
        case CLASS_PRIEST:  val2 = uint32(GetStat(STAT_STRENGTH) - 10); break;
        case CLASS_WARLOCK: val2 = uint32(GetStat(STAT_STRENGTH) - 10); break;
    }
    tem_att_power = GetUInt32Value(UNIT_FIELD_ATTACK_POWER) + GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS);

    ApplyModUInt32Value(UNIT_FIELD_ATTACK_POWER, val2, apply);

    if(apply)
        tem_att_power = GetUInt32Value(UNIT_FIELD_ATTACK_POWER) + GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS);

    val = GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER);
    if(val>0)
        tem_att_power = uint32(val*tem_att_power);

    val = tem_att_power/14.0f * GetAttackTime(BASE_ATTACK)/1000;

    ApplyModFloatValue(UNIT_FIELD_MINDAMAGE, val, apply);
    ApplyModFloatValue(UNIT_FIELD_MAXDAMAGE, val, apply);

    val = tem_att_power/14.0f * GetAttackTime(OFF_ATTACK)/1000;

    ApplyModFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, val, apply);
    ApplyModFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, val, apply);

    // critical
    if(getClass() == CLASS_HUNTER) classrate = 53;
    else if(getClass() == CLASS_ROGUE)  classrate = 29;
    else classrate = 20;

    val = GetStat(STAT_AGILITY)/classrate;

    ApplyModFloatValue(PLAYER_CRIT_PERCENTAGE, val, apply);

    //dodge
    if(getClass() == CLASS_HUNTER) classrate = 26.5;
    else if(getClass() == CLASS_ROGUE)  classrate = 14.5;
    else classrate = 20;
    ///*+(Defense*0,04);
    if (getRace() == RACE_NIGHTELF)
        val = GetStat(STAT_AGILITY)/classrate + 1;
    else
        val = GetStat(STAT_AGILITY)/classrate;

    ApplyModFloatValue(PLAYER_DODGE_PERCENTAGE, val, apply);

    // remove percent mods to see original stats when adding buffs/items
    if (!apply)
    {
        for (uint8 i = 0; i < MAX_STATS; i++)
        {
            if (totalstatmods[i])
            {
                ApplyStatPercentMod(Stats(i),totalstatmods[i], apply );
                ((Player*)this)->ApplyPosStatPercentMod(Stats(i),totalstatmods[i], apply );
                ((Player*)this)->ApplyNegStatPercentMod(Stats(i),totalstatmods[i], apply );
            }
        }
        for (uint8 i = 0; i < MAX_SPELL_SCHOOOL; i++)
        {
            if (totalresmods[i])
            {
                ApplyResistancePercentMod(SpellSchools(i), totalresmods[i], apply );
                ((Player*)this)->ApplyResistanceBuffModsPercentMod(SpellSchools(i),true, totalresmods[i], apply);
                ((Player*)this)->ApplyResistanceBuffModsPercentMod(SpellSchools(i),false, totalresmods[i], apply);
            }
        }

        ApplyPercentModFloatValue(UNIT_FIELD_MINDAMAGE, totaldamgemods[BASE_ATTACK], apply );
        ApplyPercentModFloatValue(UNIT_FIELD_MAXDAMAGE, totaldamgemods[BASE_ATTACK], apply );
        ApplyPercentModFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, totaldamgemods[OFF_ATTACK], apply );
        ApplyPercentModFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, totaldamgemods[OFF_ATTACK], apply );
        ApplyPercentModFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, totaldamgemods[RANGED_ATTACK], apply );
        ApplyPercentModFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, totaldamgemods[RANGED_ATTACK], apply );
    }
}

void Unit::_RemoveAllAuraMods()
{
    ApplyStats(false);
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        switch ((*i).second->GetModifier()->m_auraname)
        {
            case SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE:
            case SPELL_AURA_MOD_RESISTANCE_PCT:
            case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE:
                // these are already removed by applystats
                break;
            default:
                (*i).second->ApplyModifier(false);
        }
    }
    ApplyStats(true);

    // these must be removed after applystats
    AuraList& mModTotalStatPct = GetAurasByType(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
    for(AuraList::iterator i = mModTotalStatPct.begin(); i != mModTotalStatPct.end(); ++i)
        (*i)->ApplyModifier(false);
    AuraList& mModResistancePct = GetAurasByType(SPELL_AURA_MOD_RESISTANCE_PCT);
    for(AuraList::iterator i = mModResistancePct.begin(); i != mModResistancePct.end(); ++i)
        (*i)->ApplyModifier(false);
    AuraList& mModDamagePct = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for(AuraList::iterator i = mModDamagePct .begin(); i != mModDamagePct .end(); ++i)
        (*i)->ApplyModifier(false);
}

void Unit::_ApplyAllAuraMods()
{
    // these must be applied before applystats
    AuraList& mModDamagePct = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for(AuraList::iterator i = mModDamagePct .begin(); i != mModDamagePct .end(); ++i)
        (*i)->ApplyModifier(true);
    AuraList& mModResistancePct = GetAurasByType(SPELL_AURA_MOD_RESISTANCE_PCT);
    for(AuraList::iterator i = mModResistancePct.begin(); i != mModResistancePct.end(); ++i)
        (*i)->ApplyModifier(true);
    AuraList& mModTotalStatPct = GetAurasByType(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
    for(AuraList::iterator i = mModTotalStatPct.begin(); i != mModTotalStatPct.end(); ++i)
        (*i)->ApplyModifier(true);

    ApplyStats(false);

    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        switch ((*i).second->GetModifier()->m_auraname)
        {
            case SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE:
            case SPELL_AURA_MOD_RESISTANCE_PCT:
            case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE:
                // these are already applied by applystats
                break;
            default:
                (*i).second->ApplyModifier(true);
        }
    }
    ApplyStats(true);
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

void Unit::AddItemEnchant(Item *item,uint32 enchant_id,uint8 enchant_slot,bool apply)
{
    if (GetTypeId() != TYPEID_PLAYER)
        return;

    if(!item)
        return;

    if(!item->IsEquipped())
        return;

    if(enchant_slot > 7)
        return;

    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if(!pEnchant)
        return;
    uint32 enchant_display = pEnchant->display_type;
    uint32 enchant_value1 = pEnchant->value1;
    uint32 enchant_spell_id = pEnchant->spellid;

    SpellEntry const *enchantSpell_info = sSpellStore.LookupEntry(enchant_spell_id);

    if(enchant_display ==4)
    {
        ApplyArmorMod(enchant_value1,apply);
    }
    else if(enchant_display ==2)
    {
        if(getClass() == CLASS_HUNTER)
        {
            ApplyModFloatValue(UNIT_FIELD_MINRANGEDDAMAGE,enchant_value1,apply);
            ApplyModFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE,enchant_value1,apply);
        }
        else
        {
            ApplyModFloatValue(UNIT_FIELD_MINDAMAGE,enchant_value1,apply);
            ApplyModFloatValue(UNIT_FIELD_MAXDAMAGE,enchant_value1,apply);
        }
    }
    else
    {
        if(apply && enchant_display == 3)
        {
            Spell spell(this, enchantSpell_info, true, 0);
            SpellCastTargets targets;
            targets.setUnitTarget(this);
            spell.prepare(&targets);
        }
        else RemoveAurasDueToSpell(enchant_spell_id);
    }

    // visualize enchantment at player and equipped items
    int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (item->GetSlot() * 12);
    SetUInt32Value(VisibleBase+1 + enchant_slot, apply? item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+enchant_slot*3) : 0);
}

void Unit::AddDynObject(DynamicObject* dynObj)
{
    m_dynObj.push_back(dynObj);
}

void Unit::RemoveDynObject(uint32 spellid)
{
    if(m_dynObj.empty())
        return;
    for (std::list<DynamicObject*>::iterator i = m_dynObj.begin(); i != m_dynObj.end();)
    {
        if(spellid == 0 || (*i)->GetSpellId() == spellid)
        {
            (*i)->Delete();
            i = m_dynObj.erase(i);
        }
        else
            ++i;
    }
}

DynamicObject * Unit::GetDynObject(uint32 spellId, uint32 effIndex)
{
    std::list<DynamicObject*>::iterator i;
    for (i = m_dynObj.begin(); i != m_dynObj.end(); ++i)
        if ((*i)->GetSpellId() == spellId && (*i)->GetEffIndex() == effIndex)
            return *i;
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
    data << SpellID;
    data << (Damage-AbsorbedDamage-Resist-Blocked);
    data << DamageType;                                     //damagetype
    data << AbsorbedDamage;                                 //AbsorbedDamage
    data << Resist;                                         //resist
    data << (uint8)PhysicalDamage;
    data << uint8(0);
    data << Blocked;                                        //blocked
    data << uint8(CriticalHit ? 2 : 0);
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

typedef std::list< std::pair<SpellEntry const *,uint32> > ProcTriggeredList;

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

    // auraTypes contains auras capable of proc'ing
    std::list<uint32> auraTypes;
    auraTypes.push_back(SPELL_AURA_PROC_TRIGGER_SPELL);
    auraTypes.push_back(SPELL_AURA_PROC_TRIGGER_DAMAGE);
    auraTypes.push_back(SPELL_AURA_DUMMY);

    // Not much to do if no flags are set. Check also if we called by a effect that is self in turn triggered
    if (procAttacker &&  !(procSpell && isTriggeredSpell))
    {
        for(std::list<uint32>::iterator aur = auraTypes.begin(); aur != auraTypes.end(); ++aur)
        {
            // List of spells (effects) that proced. Spell prototype and aura-specific value (damage for TRIGGER_DAMAGE)
            ProcTriggeredList procTriggered;

            AuraList& attackerAuras = GetAurasByType(*aur);
            for(AuraList::iterator i = attackerAuras.begin(), next; i != attackerAuras.end(); i = next)
            {
                next = i; next++;
                uint32 procFlag = procAttacker;

                SpellEntry const *spellProto = (*i)->GetSpellProto();
                if(!spellProto) continue;
                SpellProcEventEntry const *spellProcEvent = sSpellProcEventStore.LookupEntry<SpellProcEventEntry>(spellProto->Id);

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
                    if(spellProcEvent->spellMask && (!procSpell || (spellProcEvent->spellMask & procSpell->SpellFamilyFlags) == 0))
                        continue;
                }

                // Need to use floats here, cuz calculated PPM chance often is about 1-2%
                float chance = (float)spellProto->procChance;
                uint32 WeaponSpeed = GetAttackTime(attType);
                if(spellProcEvent && spellProcEvent->ppmRate != 0)
                    chance = GetPPMProcChance(WeaponSpeed, spellProcEvent->ppmRate);

                if(chance > rand_chance())
                {
                    if((*i)->m_procCharges > 0)
                        (*i)->m_procCharges -= 1;

                    uint32 i_spell_eff = (*i)->GetEffIndex();
                    int32 i_spell_param = (*i)->GetModifier()->m_amount;
                    if(*aur == SPELL_AURA_PROC_TRIGGER_SPELL)
                    {
                        spellProto = sSpellStore.LookupEntry(spellProto->EffectTriggerSpell[i_spell_eff]);
                        i_spell_param = procFlag;
                    }
                    if(*aur == SPELL_AURA_DUMMY)
                        i_spell_param = i_spell_eff;

                    if(spellProto)
                        procTriggered.push_back( std::pair<SpellEntry const*,uint32>(spellProto,i_spell_param) );
                }
            }

            // Handle effects proced this time
            for(ProcTriggeredList::iterator i = procTriggered.begin(); i != procTriggered.end(); i++)
            {
                if(*aur == SPELL_AURA_PROC_TRIGGER_SPELL)
                {
                    sLog.outDebug("ProcDamageAndSpell: casting triggered by an attacker spell %u", i->first->Id);
                    if(IsPositiveSpell(i->first->Id) && !(i->second & PROC_FLAG_HEAL))
                        CastSpell(this,i->first->Id,true);
                    else if(pVictim)
                        CastSpell(pVictim,i->first->Id,true);
                }
                else if(*aur == SPELL_AURA_PROC_TRIGGER_DAMAGE)
                {
                    sLog.outDebug("ProcDamageAndSpell: doing %u damage from aura id %u (triggered by an attacker)", i->second, i->first->Id);
                    uint32 damage = i->second;
                    // TODO: remove hack for Seal of Righteousness. That should not be there
                    if(i->first->SpellIconID == 25 && i->first->SpellVisual == 5622)
                        damage = (damage * GetAttackTime(BASE_ATTACK))/60/1000;
                    if(pVictim)
                        SpellNonMeleeDamageLog(pVictim, i->first->Id, damage, true);
                }
                else if(*aur == SPELL_AURA_DUMMY)
                {
                    // TODO: write a DUMMY aura handle code
                    sLog.outDebug("ProcDamageAndSpell: aura id %u with dummy effect (triggered by an attacker)", (*i).first->Id);
                    HandleDummyAuraProc(pVictim, i->first, i->second, damage);
                }
            }

            // Safely remove attacker auras with zero charges
            for(AuraList::iterator i = attackerAuras.begin(), next; i != attackerAuras.end(); i = next)
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
    if(pVictim && procVictim)
    {
        for(std::list<uint32>::iterator aur = auraTypes.begin(); aur != auraTypes.end(); aur++)
        {
            // List of spells (effects) that proced. Spell prototype and aura-specific value (damage for TRIGGER_DAMAGE)
            ProcTriggeredList procTriggered;

            AuraList& victimAuras = pVictim->GetAurasByType(*aur);
            for(AuraList::iterator i = victimAuras.begin(), next; i != victimAuras.end(); i = next)
            {
                next = i; next++;
                uint32 procFlag = procVictim;

                SpellEntry const *spellProto = (*i)->GetSpellProto();
                if(!spellProto) continue;
                SpellProcEventEntry const *spellProcEvent = sSpellProcEventStore.LookupEntry<SpellProcEventEntry>(spellProto->Id);

                uint32 procFlags = spellProcEvent ? spellProcEvent->procFlags : spellProto->procFlags;
                if((procFlag & procFlags) == 0)
                    continue;

                // procChance is exact number in percents anyway
                uint32 chance = spellProto->procChance;
                if(chance > uint32(rand_chance()))
                {
                    if((*i)->m_procCharges > 0)
                        (*i)->m_procCharges -= 1;

                    uint32 i_spell_eff = (*i)->GetEffIndex();
                    int32 i_spell_param = (*i)->GetModifier()->m_amount;
                    if(*aur == SPELL_AURA_PROC_TRIGGER_SPELL)
                    {
                        spellProto = sSpellStore.LookupEntry(spellProto->EffectTriggerSpell[i_spell_eff]);
                        i_spell_param = procFlag;
                    }
                    if(*aur == SPELL_AURA_DUMMY)
                        i_spell_param = i_spell_eff;

                    if(spellProto)
                        procTriggered.push_back( std::pair<SpellEntry const*,uint32>(spellProto,i_spell_param) );
                }
            }

            // Handle effects proced this time
            for(ProcTriggeredList::iterator i = procTriggered.begin(); i != procTriggered.end(); i++)
            {
                if(*aur == SPELL_AURA_PROC_TRIGGER_SPELL)
                {
                    sLog.outDebug("ProcDamageAndSpell: casting triggered by a victim spell %u", i->first->Id);
                    if(IsPositiveSpell(i->first->Id) && !(i->second&PROC_FLAG_HEAL))
                        pVictim->CastSpell(pVictim,i->first->Id,true);
                    else
                        pVictim->CastSpell(this,i->first->Id,true);
                }
                else if(*aur == SPELL_AURA_PROC_TRIGGER_DAMAGE)
                {
                    sLog.outDebug("ProcDamageAndSpell: doing %u damage from aura id %u (triggered by a victim)", i->second, i->first->Id);
                    pVictim->SpellNonMeleeDamageLog(this, i->first->Id, i->second, true);
                }
                else if(*aur == SPELL_AURA_DUMMY)
                {
                    // TODO: write a DUMMY aura handle code
                    sLog.outDebug("ProcDamageAndSpell: aura id %u with dummy effect (triggered by a victim)", i->first->Id);
                    pVictim->HandleDummyAuraProc(this, i->first, i->second, damage);
                }
            }

            // Safely remove auras with zero charges
            for(AuraList::iterator i = victimAuras.begin(), next; i != victimAuras.end(); i = next)
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
            return;
        case MELEE_HIT_CRIT:
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
        case MELEE_HIT_PARRY:
            procAttacker = PROC_FLAG_TARGET_AVOID_ATTACK;
            procVictim = PROC_FLAG_PARRY;
            break;
        case MELEE_HIT_BLOCK:
            procAttacker = PROC_FLAG_TARGET_AVOID_ATTACK;
            procVictim = PROC_FLAG_BLOCK;
            break;
        case MELEE_HIT_DODGE:
            procAttacker = PROC_FLAG_TARGET_AVOID_ATTACK;
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

    ProcDamageAndSpell(pVictim, procAttacker, procVictim, damage, spellCasted, isTriggeredSpell, attType);
}

void Unit::HandleDummyAuraProc(Unit *pVictim, SpellEntry const *dummySpell, uint32 effIndex, uint32 damage)
{
    // Example. Ignite. Though it looks like hack, it isn't )
    if(dummySpell->Id == 11119 || dummySpell->Id == 11120 || dummySpell->Id == 12846 || dummySpell->Id == 12847 || dummySpell->Id == 12848)
    {
        SpellEntry const *igniteDotTemplate = sSpellStore.LookupEntry(12654);
        SpellEntry igniteDot = *igniteDotTemplate;

        switch (dummySpell->Id)
        {
            case 11119:
                igniteDot.EffectBasePoints[0]=uint32(0.04f*damage);break;
            case 11120:
                igniteDot.EffectBasePoints[0]=uint32(0.08f*damage);break;
            case 12846:
                igniteDot.EffectBasePoints[0]=uint32(0.12f*damage);break;
            case 12847:
                igniteDot.EffectBasePoints[0]=uint32(0.16f*damage);break;
            case 12848:
                igniteDot.EffectBasePoints[0]=uint32(0.20f*damage);break;
        };
        CastSpell(pVictim, &igniteDot, true, NULL);
    }
}

void Unit::setPowerType(Powers new_powertype)
{
    uint32 tem_bytes_0 = GetUInt32Value(UNIT_FIELD_BYTES_0);
    SetUInt32Value(UNIT_FIELD_BYTES_0,((tem_bytes_0<<8)>>8) + (uint32(new_powertype)<<24));
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
            SetPower(   POWER_ENERGY,100);
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
                sLog.outError("Player %s have invalide faction (faction template id) #%u", ((Player*)this)->GetName(), getFaction());
            else
                sLog.outError("Creature (template id: %u) have invalide faction (faction template id) #%u", ((Creature*)this)->GetCreatureInfo()->Entry, getFaction());
            guid = GetGUID();
        }
    }
    return entry;
}

bool Unit::IsHostileToAll() const
{
    FactionTemplateResolver my_faction = getFactionTemplateEntry();

    return my_faction.IsHostileToAll();
}

bool Unit::IsHostileTo(Unit const* unit) const
{
    if(unit==this)
        return false;

    // test pet/charm masters instead pers/charmeds
    Unit const* testerOwner = GetOwner();
    Unit const* targetOwner = unit->GetOwner();

    Unit const* tester = testerOwner ? testerOwner : this;
    Unit const* target = targetOwner ? targetOwner : unit;

    // special cases (Duel)
    if(tester->GetTypeId()==TYPEID_PLAYER && target->GetTypeId()==TYPEID_PLAYER)
    {
        // Duel
        if(((Player const*)tester)->duel && ((Player const*)tester)->duel->opponent == target)
            return true;

        //= PvP states
        // Green/Blue (can't attack)
        if(((Player*)tester)->GetTeam()==((Player*)target)->GetTeam())
            return false;

        // Red (can attack) if true, Blue/Yellow (can't attack) in another case
        return ((Player*)tester)->IsPvP() && ((Player*)target)->IsPvP();
    }

    // common case (CvC,PvC, CvP)
    FactionTemplateResolver tester_faction = tester->getFactionTemplateEntry();
    FactionTemplateResolver target_faction = target->getFactionTemplateEntry();

    return tester_faction.IsHostileTo(target_faction);
}

bool Unit::IsFriendlyTo(Unit const* unit) const
{
    if(unit==this)
        return true;

    // test pet/charm masters instead pers/charmeds
    Unit const* testerOwner = GetOwner();
    Unit const* targetOwner = unit->GetOwner();

    Unit const* tester = testerOwner ? testerOwner : this;
    Unit const* target = targetOwner ? targetOwner : unit;

    // special cases (Duel)
    if(tester->GetTypeId()==TYPEID_PLAYER && target->GetTypeId()==TYPEID_PLAYER)
    {
        // Duel
        if(((Player const*)tester)->duel && ((Player const*)tester)->duel->opponent == target)
            return false;

        //= PvP states
        // Green/Blue (non-attackable)
        if(((Player*)tester)->GetTeam()==((Player*)target)->GetTeam())
            return true;

        // Blue (friendly/non-attackable) if not PVP, or Yellow/Red in another case (attackable)
        return !((Player*)target)->IsPvP();
    }

    // common case (CvC, PvC, CvP)
    FactionTemplateResolver tester_faction = tester->getFactionTemplateEntry();
    FactionTemplateResolver target_faction = target->getFactionTemplateEntry();

    return tester_faction.IsFriendlyTo(target_faction);
}

bool Unit::IsNeutralToAll() const
{
    FactionTemplateResolver my_faction = getFactionTemplateEntry();

    return my_faction.IsNeutralToAll();
}

bool Unit::Attack(Unit *victim)
{
    if(!victim || victim == this)
        return false;

    // player don't must attack in mount state
    if(GetTypeId()==TYPEID_PLAYER && IsMounted())
        return false;

    if (m_attacking)
    {
        if (m_attacking == victim)
            return false;
        AttackStop();
    }

    //Set our target
    SetUInt32Value(UNIT_FIELD_TARGET, victim->GetGUIDLow());

    addUnitState(UNIT_STAT_ATTACKING);
    if(GetTypeId()==TYPEID_UNIT)
        SetInCombat();
    m_attacking = victim;
    m_attacking->_addAttacker(this);

    if( GetTypeId()==TYPEID_UNIT && !((Creature*)this)->isPet() )
    {
        ((Creature*)this)->CallAssistence();
    }
    //if(!isAttackReady(BASE_ATTACK))
    //resetAttackTimer(BASE_ATTACK);

    // delay offhand weapon attack to next attack time
    if(haveOffhandWeapon())
        resetAttackTimer(OFF_ATTACK);

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
    SetUInt32Value(UNIT_FIELD_TARGET, 0);

    clearUnitState(UNIT_STAT_ATTACKING);
    if(GetTypeId()!=TYPEID_PLAYER && m_attackers.empty())
        ClearInCombat();

    if(m_currentMeleeSpell)
        m_currentMeleeSpell->cancel();

    if( GetTypeId()==TYPEID_UNIT )
    {
        ((Creature*)this)->SetNoCallAssistence(false);
    }

    SendAttackStop(victim);

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

Unit *Unit::GetOwner() const
{
    uint64 ownerid = GetOwnerGUID();
    if(!ownerid)
        return NULL;
    return ObjectAccessor::Instance().GetUnit(*this, ownerid);
}

Pet* Unit::GetPet() const
{
    uint64 pet_guid = GetPetGUID();
    if(pet_guid)
    {
        Creature* pet = ObjectAccessor::Instance().GetCreature(*this, pet_guid);
        if(!pet||!pet->isPet())
        {
            sLog.outError("Unit::GetPet: Pet %u not exist.",GUID_LOPART(pet_guid));
            const_cast<Unit*>(this)->SetPet(0);
            return NULL;
        }
        return (Pet*)pet;
    }

    return NULL;
}

Creature* Unit::GetCharm() const
{
    uint64 charm_guid = GetCharmGUID();
    if(charm_guid)
    {
        Creature* pet = ObjectAccessor::Instance().GetCreature(*this, charm_guid);
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

void Unit::SetCharm(Creature* charmed)
{
    SetUInt64Value(UNIT_FIELD_CHARM,charmed ? charmed->GetGUID() : 0);
}

void Unit::UnsummonTotem(int8 slot)
{
    for (int8 i = 0; i < 4; i++)
    {
        if (i != slot && slot != -1) continue;
        Creature *OldTotem = ObjectAccessor::Instance().GetCreature(*this, m_TotemSlot[i]);
        if (!OldTotem || !OldTotem->isTotem()) continue;
        ((Totem*)OldTotem)->UnSummon();
    }
}

void Unit::SendHealSpellOnPlayer(Unit *pVictim, uint32 SpellID, uint32 Damage, bool critical)
{
    // we guess size
    WorldPacket data(SMSG_HEALSPELL_ON_PLAYER_OBSOLETE, (9+16));
    data.append(pVictim->GetPackGUID());
    data.append(GetPackGUID());
    data << SpellID;
    data << Damage;
    data << uint8(critical ? 1 : 0);
    SendMessageToSet(&data, true);
}

void Unit::SendHealSpellOnPlayerPet(Unit *pVictim, uint32 SpellID, uint32 Damage,Powers powertype, bool critical)
{
    WorldPacket data(SMSG_HEALSPELL_ON_PLAYERS_PET_OBSOLETE, (13+8));
    data.append(pVictim->GetPackGUID());
    data.append(GetPackGUID());
    data << SpellID;
    data << uint32(powertype);
    data << Damage;
    data << uint8(critical ? 1 : 0);
    SendMessageToSet(&data, true);
}

uint32 Unit::SpellDamageBonus(Unit *pVictim, SpellEntry const *spellProto, uint32 pdamage)
{
    if(!spellProto || !pVictim) return pdamage;

    if(pVictim->IsImmunedToSpellDamage(spellProto))
        return 0;

    CreatureInfo const *cinfo = NULL;
    if(pVictim->GetTypeId() != TYPEID_PLAYER)
        cinfo = ((Creature*)pVictim)->GetCreatureInfo();

    // Damage Done
    int32 AdvertisedBenefit = 0;
    uint32 PenaltyFactor = 0;
    uint32 CastingTime = GetCastTime(sCastTimesStore.LookupEntry(spellProto->CastingTimeIndex));
    if (CastingTime > 3500) CastingTime = 3500;
    if (CastingTime < 1500) CastingTime = 1500;

    AuraList& mDamageDoneCreature = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE_CREATURE);
    for(AuraList::iterator i = mDamageDoneCreature.begin();i != mDamageDoneCreature.end(); ++i)
        if(cinfo && (cinfo->type & (*i)->GetModifier()->m_miscvalue) != 0)
            AdvertisedBenefit += (*i)->GetModifier()->m_amount;

    AuraList& mDamageDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE);
    for(AuraList::iterator i = mDamageDone.begin();i != mDamageDone.end(); ++i)
        if(((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
            AdvertisedBenefit += (*i)->GetModifier()->m_amount;

    AuraList& mDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_TAKEN);
    for(AuraList::iterator i = mDamageTaken.begin();i != mDamageTaken.end(); ++i)
        if(((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
            AdvertisedBenefit += (*i)->GetModifier()->m_amount;

    float TotalMod = 1;
    AuraList& mModDamagePercentDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for(AuraList::iterator i = mModDamagePercentDone.begin(); i != mModDamagePercentDone.end(); ++i)
        if( spellProto->School != 0 && ((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0 )
            TotalMod *= ((*i)->GetModifier()->m_amount+100.0f)/100.0f;

    // TODO - fix PenaltyFactor and complete the formula from the wiki
    float ActualBenefit = AdvertisedBenefit * (CastingTime / 3500.0f) * (100.0f - PenaltyFactor) / 100.0f;

    int32 tmpDamage = int32(pdamage*TotalMod+ActualBenefit);

    return tmpDamage > 0 ? tmpDamage : 0;
}

bool Unit::SpellCriticalBonus(SpellEntry const *spellProto, int32 *peffect)
{
    int32 critchance = m_baseSpellCritChance + int32(GetStat(STAT_INTELLECT)/100-1);
    critchance = critchance > 0 ? critchance :0;

    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->ApplySpellMod(spellProto->Id, SPELLMOD_CRITICAL_CHANCE, critchance);

    AuraList& mSpellCritSchool = GetAurasByType(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL);
    for(AuraList::iterator i = mSpellCritSchool.begin(); i != mSpellCritSchool.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue == -2 || ((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
            critchance += (*i)->GetModifier()->m_amount;

    critchance = critchance > 0 ? critchance :0;
    if(uint32(critchance) >= urand(0,100))
    {
        int32 critbonus = *peffect / 2;
        if (GetTypeId() == TYPEID_PLAYER)
            ((Player*)this)->ApplySpellMod(spellProto->Id, SPELLMOD_CRIT_DAMAGE_BONUS, critbonus);
        *peffect += critbonus;
        return true;
    }
    return false;
}

uint32 Unit::SpellHealingBonus(SpellEntry const *spellProto, uint32 healamount)
{
    // Healing Done
    int32 AdvertisedBenefit = 0;
    uint32 PenaltyFactor = 0;
    uint32 CastingTime = GetCastTime(sCastTimesStore.LookupEntry(spellProto->CastingTimeIndex));
    if (CastingTime > 3500) CastingTime = 3500;
    if (CastingTime < 1500) CastingTime = 1500;

    AuraList& mHealingDone = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE);
    for(AuraList::iterator i = mHealingDone.begin();i != mHealingDone.end(); ++i)
        if(((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
            AdvertisedBenefit += (*i)->GetModifier()->m_amount;

    // TODO - fix PenaltyFactor and complete the formula from the wiki
    float ActualBenefit = (float)AdvertisedBenefit * ((float)CastingTime / 3500) * (float)(100 - PenaltyFactor) / 100;
    healamount += uint32(ActualBenefit);

    // TODO: check for ALL/SPELLS type
    AuraList& mHealingPct = GetAurasByType(SPELL_AURA_MOD_HEALING_PCT);
    for(AuraList::iterator i = mHealingPct.begin();i != mHealingPct.end(); ++i)
        healamount *= uint32((100.0f + (*i)->GetModifier()->m_amount) / 100.0f);
    AuraList& mHealingDonePct = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
    for(AuraList::iterator i = mHealingDonePct.begin();i != mHealingDonePct.end(); ++i)
        healamount *= uint32((100.0f + (*i)->GetModifier()->m_amount) / 100.0f);

    healamount += m_AuraModifiers[SPELL_AURA_MOD_HEALING];
    if (int32(healamount) < 0) healamount = 0;

    return healamount;
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
       (family == 6 && flags == 8192) ||                    //Mind Blast
       (family == 11 && flags == 1048576))                  //Earth Shock
       return true;

    return false;
}

void Unit::MeleeDamageBonus(Unit *pVictim, uint32 *pdamage)
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
            if(happiness>=750000)
                *pdamage = uint32(*pdamage * 1.25);
            else if(happiness>=500000)
                *pdamage = uint32(*pdamage * 1.0);
            else *pdamage = uint32(*pdamage * 0.75);
        }
    }

    // bonus result can be negative
    int32 damage = *pdamage;

    AuraList& mDamageDoneCreature = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE_CREATURE);
    for(AuraList::iterator i = mDamageDoneCreature.begin();i != mDamageDoneCreature.end(); ++i)
        if(cinfo && cinfo->type == uint32((*i)->GetModifier()->m_miscvalue))
            damage += (*i)->GetModifier()->m_amount;

    AuraList& mDamageDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE);
    for(AuraList::iterator i = mDamageDone.begin();i != mDamageDone.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue & IMMUNE_SCHOOL_PHYSICAL)
            damage += (*i)->GetModifier()->m_amount;

    AuraList& mDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_TAKEN);
    for(AuraList::iterator i = mDamageTaken.begin();i != mDamageTaken.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue & IMMUNE_SCHOOL_PHYSICAL)
            damage += (*i)->GetModifier()->m_amount;

    AuraList& mCreatureAttackPower = GetAurasByType(SPELL_AURA_MOD_CREATURE_ATTACK_POWER);
    for(AuraList::iterator i = mCreatureAttackPower.begin();i != mCreatureAttackPower.end(); ++i)
        if(cinfo && (cinfo->type & uint32((*i)->GetModifier()->m_miscvalue)) != 0)
            damage += int32((*i)->GetModifier()->m_amount/14.0f * GetAttackTime(BASE_ATTACK)/1000);

    // bonus result can be negative
    *pdamage = damage < 0 ? 0 : damage;

    *pdamage = uint32(*pdamage * (m_modDamagePCT+100)/100);
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

void Unit::ClearInCombat()
{
    m_CombatTimer = 0;
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
}

bool Unit::isTargetableForAttack()
{
    if (GetTypeId()==TYPEID_PLAYER && ((Player *)this)->isGameMaster())
        return false;
    return isAlive() && !isInFlight() /*&& !isStealth()*/;
}

void Unit::ModifyHealth(int32 dVal)
{
    if(dVal==0)
        return;

    uint32 curHealth = GetHealth();

    int32 val = dVal + curHealth;
    if(val <= 0)
    {
        SetHealth(0);
        return;
    }

    uint32 maxHealth = GetMaxHealth();

    if(uint32(val) < maxHealth)
        SetHealth(val);
    else
    if(curHealth!=maxHealth)
        SetHealth(maxHealth);
}

void Unit::ModifyPower(Powers power, int32 dVal)
{
    if(dVal==0)
        return;

    uint32 curPower = GetPower(power);

    int32 val = dVal + curPower;
    if(val <= 0)
    {
        SetPower(power,0);
        return;
    }

    uint32 maxPower = GetMaxPower(power);

    if(uint32(val) < maxPower)
        SetPower(power,val);
    else
    if(curPower != maxPower)
        SetPower(power,maxPower);
}

bool Unit::isVisibleFor(Unit* u, bool detect)
{
    // Visible units, always are visible for all pjs
    if (m_Visibility == VISIBILITY_ON)
        return true;

    // GMs are visible for higher gms (or players are visible for gms)
    if (u->GetTypeId() == TYPEID_PLAYER && ((Player *)u)->isGameMaster())
        return (GetTypeId() == TYPEID_PLAYER && ((Player *)this)->GetSession()->GetSecurity() <= ((Player *)u)->GetSession()->GetSecurity());

    // non faction visibility non-breakable for non-GMs
    if (m_Visibility == VISIBILITY_OFF)
        return false;

    // Units far than MAX_DIST_INVISIBLE, that are not gms and are stealth, are not visibles too
    if (!this->IsWithinDist(u,MAX_DIST_INVISIBLE_UNIT))
        return false;

    // Stealth not hostile units, not visibles (except Player-with-Player case)
    if (!u->IsHostileTo(this))
    {
        // player autodetect other player with stealth only if he in same group or raid or same team (raid/team case dependent from conf setting)
        if(GetTypeId()==TYPEID_PLAYER && u->GetTypeId()==TYPEID_PLAYER)
        {
            if(((Player*)this)->IsGroupVisibleFor(((Player*)u)))
                return true;

            // else apply same rules as for hostile case (detecting check)
        }
        else
            return true;
    }

    // if in non-detect mode then invisible for unit
    if(!detect)
        return false;

    bool IsVisible = true;
    bool notInFront = u->isInFront(this, MAX_DIST_INVISIBLE_UNIT * MAX_DIST_INVISIBLE_UNIT) ? 0 : 1;
    float Distance = sqrt(GetDistanceSq(u));
    float prob = 0;

    // Function for detection (can be improved)
    // Take into account that this function is executed every x secs, so prob must be low for right working

    int32 x = u->getLevel() + (u->m_detectStealth / 5) - (m_stealthvalue / 5) + 59;
    if (x<0) x = 0;
    float AverageDist = 1 - 0.11016949*x + 0.00301637*x*x;  //at this distance, the detector has to be a 15% prob of detect
    if (AverageDist < 1) AverageDist = 1;
    if (Distance > AverageDist)
        //prob between 10% and 0%
        prob = (AverageDist-200+9*Distance)/(AverageDist-20);
    else
        prob = 75 - (60/AverageDist)*Distance;              //prob between 15% and 75% (75% max prob)
    if (notInFront)
        prob = prob/100;
    if (prob < 0.1)
        prob = 0.1;                                         //min prob of detect is 0.1

    if (rand_chance() > prob)
        IsVisible = false;
    else
        IsVisible = true;

    return IsVisible && ( Distance <= MAX_DIST_INVISIBLE_UNIT * MAX_DIST_INVISIBLE_UNIT) ;
}

void Unit::SetVisibility(UnitVisibility x)
{
    m_Visibility = x;

    switch (x)
    {
        case VISIBILITY_ON:
            m_UpdateVisibility = VISIBLE_SET_VISIBLE;
            break;
        case VISIBILITY_OFF:
            m_UpdateVisibility = VISIBLE_SET_INVISIBLE;
            break;
        case VISIBILITY_GROUP:
            m_UpdateVisibility = VISIBLE_SET_INVISIBLE_FOR_GROUP;
            break;
    }
    if(GetTypeId() == TYPEID_PLAYER)
    {
        Map *m = MapManager::Instance().GetMap(GetMapId());
        m->PlayerRelocation((Player *)this,GetPositionX(),GetPositionY(),
            GetPositionZ(),GetOrientation(), true);
    }

    m_UpdateVisibility = VISIBLE_NOCHANGES;
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
            data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED, 16);
            break;
        case MOVE_TURN:
            return;                                         // ignore
        default:
            sLog.outError("Unit::SetSpeed: Unsupported move type (%d), data not sent to client.",mtype);
            return;
    }

    data.append(GetPackGUID());
    data << (uint32)0;
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
        CombatStop();

        if(m_currentSpell)
            m_currentSpell->cancel();
    }

    if (s == JUST_DIED)
    {
        RemoveAllAurasOnDeath();
        UnsummonTotem();
    }
    if (m_deathState != ALIVE && s == ALIVE)
    {
        _ApplyAllAuraMods();
    }
    m_deathState = s;
}
