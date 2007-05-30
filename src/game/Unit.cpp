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

Unit::Unit( WorldObject *instantiator ) : WorldObject( instantiator )
{
    m_objectType |= TYPE_UNIT;
    m_objectTypeId = TYPEID_UNIT;
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_ALL | UPDATEFLAG_LIVING | UPDATEFLAG_HASPOSITION);

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
        m_AuraModifiers[i] = 0;
    for (int i = 0; i < IMMUNITY_MECHANIC; i++)
        m_spellImmune[i].clear();

    m_attacking = NULL;
    m_modHitChance = 0;
    m_modSpellHitChance = 0;
    m_baseSpellCritChance = 5;
    m_modResilience = 0.0;
    m_modCastSpeedPct = 0;
    m_CombatTimer = 0;
    m_victimThreat = 0.0f;
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        m_threatModifier[i] = 1.0f;
    m_isSorted = true;
    for (int i = 0; i < MAX_MOVE_TYPE; ++i)
        m_speed_rate[i] = 1.0f;
}

Unit::~Unit()
{
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
    for(std::list<DynamicObject*>::iterator i = m_dynObj.begin(); i != m_dynObj.end();)
    {
        (*i)->Delete();
        i = m_dynObj.erase(i);
    }
}

void Unit::Update( uint32 p_time )
{
    if( !m_movementData.Empty() )
    {
        //  If !consecutive packet or passed 9 packet SendMonsterMove(). 1 packetCount = 50-100ms
        if( m_movementData.holdTime/100 > m_movementData.packetCount || m_movementData.packetCount > sWorld.getConfig(CONFIG_MOVE_FILTER_COUNT) )
            SendMonsterMove(m_movementData.x,m_movementData.y, m_movementData.z, m_movementData.type, m_movementData.run, m_movementData.time);
        else
            m_movementData.holdTime += p_time;              //Update the holdTime to help with consecutive packet check
    }
    /*if(p_time > m_AurasCheck)
    {
    m_AurasCheck = 2000;
    _UpdateAura();
    }else
    m_AurasCheck -= p_time;*/

    _UpdateSpells( p_time );

    if (isInCombat() && GetTypeId() == TYPEID_PLAYER )      //update combat timer only for players
    {
        if(IsInHateListEmpty())
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
    //Will be: Checked, maybe Buffered, will be send on Unit::Update()
    m_movementData.Update(x,y,z,transitTime,run,0);  
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
    data << m_movementData.type;
    switch(m_movementData.type)
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
    m_movementData.Clear();                                 // reset move cache at explicit move
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

void Unit::DealDamage(Unit *pVictim, uint32 damage, DamageEffectType damagetype, uint8 damageSchool,
SpellEntry const *spellProto, uint32 procFlag, bool durabilityLoss)
{
    if (!pVictim->isAlive() || pVictim->isInFlight()) return;

    if(!damage) return;

    if(HasStealthAura())
        RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
    if(HasInvisibilityAura())
        RemoveSpellsCausingAura(SPELL_AURA_MOD_INVISIBILITY);
    // remove death simulation at damage
    if(hasUnitState(UNIT_STAT_DIED))
        RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

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
            pVictim->DeleteInHateListOf();

            Pet *pet = pVictim->GetPet();
            if(pet)
            {
                pet->setDeathState(JUST_DIED);
                pet->CombatStop(true);
                pet->SetHealth(0);
                pet->addUnitState(UNIT_STAT_DIED);
                pet->DeleteInHateListOf();
            }
        }
        else                                                // creature died
        {
            DEBUG_LOG("DealDamageNotPlayer");

            if(((Creature*)pVictim)->isPet())
                pVictim->DeleteInHateListOf();
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
        else if(GetOwnerGUID())                             // Pet or timed creature, etc
        {
            Unit* pet = this;
            Unit* owner = pet->GetOwner();

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
            player->CalculateHonor(pVictim);

            player->CalculateReputation(pVictim);

            if(!PvP)
            {
                DEBUG_LOG("DealDamageIsPvE");
                uint32 xp = MaNGOS::XP::Gain(player, pVictim);

                Group *pGroup = player->groupInfo.group;
                if(pGroup)
                {
                    DEBUG_LOG("Kill Enemy In Group");
                    uint32 count = pGroup->GetMemberCountForXPAtKill(pVictim);
                    if(count)
                    {
                        xp /= count;
                        Group::MemberList const& members = pGroup->GetMembers();
                        for(Group::member_citerator itr = members.begin(); itr != members.end(); ++itr)
                        {
                            Player *pGroupGuy = pGroup->GetMemberForXPAtKill(itr->guid,pVictim);
                            if(!pGroupGuy)
                                continue;

                            pGroupGuy->GiveXP(xp, pVictim);
                            if(Pet* pet = player->GetPet())
                            {
                                pet->GivePetXP(xp/2);
                            }
                            pGroupGuy->KilledMonster(pVictim->GetEntry(), pVictim->GetGUID());
                        }
                    }
                }
                else                                        // if (pGroup)
                {
                    DEBUG_LOG("Player kill enemy alone");
                    player->GiveXP(xp, pVictim);
                    if(Pet* pet = player->GetPet())
                    {
                        pet->GivePetXP(xp);
                    }
                    player->KilledMonster(pVictim->GetEntry(),pVictim->GetGUID());
                }                                           // if-else (pGroup)

                if(xp) //  || honordiff < 0)
                    ProcDamageAndSpell(pVictim,PROC_FLAG_KILL_XP_GIVER,PROC_FLAG_NONE);
            }                                               // if (!PvP)
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
            if (((Creature *)pVictim)->AI())
                ((Creature *)pVictim)->AI()->DamageInflict(this, damage);

            if(spellProto && IsDamageToThreatSpell(spellProto))
                damage *= 2;
            pVictim->AddThreat(this, damage, damageSchool, spellProto);
        }
        else                                                // victim is a player
        {
            // rage from received damage (from creatures and players)
            if( pVictim != this                             // not generate rage for self damage (falls, ...)
                // warrior and some druid forms
                && (((Player*)pVictim)->getPowerType() == POWER_RAGE))
                ((Player*)pVictim)->CalcRage(damage,false);

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
            if (se->AuraInterruptFlags & (1<<1))
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

void Unit::DealDamageBySchool(Unit *pVictim, SpellEntry const *spellInfo, uint32 *damage, bool *crit, bool isTriggeredSpell)
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
            *damage = CalcArmorReducedDamage(pVictim, *damage);

            // Classify outcome
            switch (outcome)
            {
                case MELEE_HIT_CRIT:
                {
                    *damage *= 2;
                    // Resilience - reduce crit damage by 2x%
                    *damage -= uint32(pVictim->m_modResilience * 2/100 * (*damage));
                    *crit = true;
                    hitInfo |= HITINFO_CRITICALHIT;
                    break;
                }
                case MELEE_HIT_PARRY:
                {
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
                    pVictim->SetFlag(UNIT_FIELD_AURASTATE, uint32(1<<(AURA_STATE_PARRY-1)));
                    break;
                }
                case MELEE_HIT_DODGE:
                {
                    if(pVictim->GetTypeId() == TYPEID_PLAYER)
                        ((Player*)pVictim)->UpdateDefense();

                    *damage = 0;
                    hitInfo |= HITINFO_SWINGNOHITSOUND;
                    victimState = VICTIMSTATE_DODGE;
                    break;
                }
                case MELEE_HIT_BLOCK:
                {
                    uint32 blocked_amount;
                    blocked_amount = uint32(pVictim->GetBlockValue() + (pVictim->GetStat(STAT_STRENGTH) / 20) -1);
                    if (blocked_amount >= *damage)
                    {
                        hitInfo |= HITINFO_SWINGNOHITSOUND;
                        victimState = VICTIMSTATE_BLOCKS;
                        *damage = 0;
                    }
                    else
                        *damage = *damage - blocked_amount;
                    break;

                }
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
                *damage = 0;
                ProcDamageAndSpell(pVictim, PROC_FLAG_TARGET_RESISTS, PROC_FLAG_RESIST_SPELL, 0, spellInfo,isTriggeredSpell);
                SendAttackStateUpdate(HITINFO_RESIST|HITINFO_SWINGNOHITSOUND, pVictim, 1, spellInfo->School, 0, 0,0,1,0);
                return;
            }

            // Calculate damage bonus
            *damage = SpellDamageBonus(pVictim, spellInfo, *damage, SPELL_DIRECT_DAMAGE);

            // Calculate critical bonus
            *crit = SpellCriticalBonus(spellInfo, (int32*)damage, pVictim);
            break;
    }

    // TODO this in only generic way, check for exceptions
    DEBUG_LOG("DealDamageBySchool (AFTER) SCHOOL %u >> DMG:%u", spellInfo->School, *damage);

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

    bool crit = false;

    DealDamageBySchool(pVictim, spellInfo, &damage, &crit, isTriggeredSpell);

    // If we actually dealt some damage
    if(damage > 0)
    {
        // Calculate absorb & resists
        uint32 absorb = 0;
        uint32 resist = 0;

        CalcAbsorbResist(pVictim,spellInfo->School,damage, &absorb, &resist);

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
        sLog.outDetail("SpellNonMeleeDamageLog: %u %X attacked %u %X for %u dmg inflicted by %u,absorb is %u,resist is %u",
            GetGUIDLow(), GetGUIDHigh(), pVictim->GetGUIDLow(), pVictim->GetGUIDHigh(), damage, spellID, absorb,resist);
        SendSpellNonMeleeDamageLog(pVictim, spellID, damage, spellInfo->School, absorb, resist, false, 0, crit);

        // Deal damage done
        DealDamage(pVictim, (damage-absorb-resist), SPELL_DIRECT_DAMAGE, spellInfo->School, spellInfo, 0, true);

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

    // this packet has random structure, probably based on auraid...
    WorldPacket data(SMSG_PERIODICAURALOG, (21+16));        // we guess size
    data.append(pVictim->GetPackGUID());
    data.append(GetPackGUID());
    data << spellProto->Id;
    data << uint32(1);

    data << mod->m_auraname;
    switch(mod->m_auraname)
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
            data << (uint32)mod->m_amount;  // ?
            data << spellProto->School;     // ?
            data << (uint32)0;              // ?
            data << (uint32)0;              // ?
            break;
        case SPELL_AURA_PERIODIC_HEAL:
            data << (uint32)mod->m_amount;  // ?
            break;
        case SPELL_AURA_OBS_MOD_HEALTH:
            data << (uint32)mod->m_amount;  // ?
            break;
        //case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        //    break;
        case SPELL_AURA_PERIODIC_ENERGIZE:
            data << (uint32)mod->m_amount;  // ?
            data << (uint32)0;              // ?
            break;
        case SPELL_AURA_PERIODIC_LEECH:
            break;
        //case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
        //    break;
        //case SPELL_AURA_PERIODIC_MANA_FUNNEL:
        //    break;
        //case SPELL_AURA_PERIODIC_MANA_LEECH:
        //    break;
        //case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        default:
            sLog.outError("PeriodicAuraLog: unhandled aura %u", mod->m_auraname);
            break;
    }
    //data << (uint32)mod->m_amount;
    //data << spellProto->School;
    SendMessageToSet(&data,true);

    if(mod->m_auraname == SPELL_AURA_PERIODIC_DAMAGE)
    {
        pdamage = SpellDamageBonus(pVictim,spellProto,pdamage,DOT);

        //pdamage = SpellDamageBonus(pVictim, spellProto, pdamage);
        SendSpellNonMeleeDamageLog(pVictim, spellProto->Id, pdamage, spellProto->School, absorb, resist, false, 0);

        DealDamage(pVictim, pdamage <= int32(absorb+resist) ? 0 : (pdamage-absorb-resist), DOT, spellProto->School, spellProto, procFlag, true);
        ProcDamageAndSpell(pVictim, PROC_FLAG_HIT_SPELL, PROC_FLAG_TAKE_DAMAGE, pdamage <= int32(absorb+resist) ? 0 : (pdamage-absorb-resist), spellProto);
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_DAMAGE_PERCENT)
    {
        pdamage = SpellDamageBonus(pVictim, spellProto, pdamage, DOT);
        int32 pdamage = GetHealth()*(100+mod->m_amount)/100;
        SendSpellNonMeleeDamageLog(pVictim, spellProto->Id, pdamage, spellProto->School, absorb, resist, false, 0);

        DealDamage(pVictim, pdamage <= int32(absorb+resist) ? 0 : (pdamage-absorb-resist), DOT, spellProto->School, spellProto, procFlag, true);
        ProcDamageAndSpell(pVictim, PROC_FLAG_HIT_SPELL, PROC_FLAG_TAKE_DAMAGE, pdamage <= int32(absorb+resist) ? 0 : (pdamage-absorb-resist), spellProto);
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_HEAL || mod->m_auraname == SPELL_AURA_OBS_MOD_HEALTH)
    {
        pdamage = SpellHealingBonus(spellProto, pdamage, DOT);

        int32 gain = pVictim->ModifyHealth(pdamage);
        ThreatAssist(pVictim, float(gain) * 0.5f, spellProto->School, spellProto);

        if(pVictim->GetTypeId() == TYPEID_PLAYER || GetTypeId() == TYPEID_PLAYER)
            SendHealSpellOnPlayer(pVictim, spellProto->Id, gain);

        // heal for caster damage
        if(pVictim!=this && spellProto->SpellVisual==163)
        {
            // FIXME: must be calculated base at spell data
            int32 dmg = gain;
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
                DealDamage(this, gain, NODAMAGE, spellProto->School, spellProto, PROC_FLAG_HEAL, true);
            }
        }

        if(mod->m_auraname == SPELL_AURA_PERIODIC_HEAL && pVictim != this)
            ProcDamageAndSpell(pVictim, PROC_FLAG_HEAL, PROC_FLAG_NONE, pdamage, spellProto);
    }
    else if(mod->m_auraname == SPELL_AURA_PERIODIC_LEECH)
    {
        uint32 tmpvalue = 0;
        float tmpvalue2 = 0;
        uint32 pdamage = mod->m_amount;
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
            pdamage = SpellDamageBonus(pVictim,spellProto,pdamage,DOT);
            DealDamage(pVictim, pdamage <= int32(absorb+resist) ? 0 : (pdamage-absorb-resist), DOT, spellProto->School, spellProto, procFlag, false);
            ProcDamageAndSpell(pVictim, PROC_FLAG_HIT_SPELL, PROC_FLAG_TAKE_DAMAGE, pdamage <= int32(absorb+resist) ? 0 : (pdamage-absorb-resist), spellProto);
            if (!pVictim->isAlive() && m_currentSpell)
                if (m_currentSpell->m_spellInfo)
                    if (m_currentSpell->m_spellInfo->Id == spellProto->Id)
                        m_currentSpell->cancel();

            break;
        }

        int32 gain = ModifyHealth(tmpvalue);
        ThreatAssist(this, float(gain) * 0.5f, spellProto->School, spellProto);

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

        int32 gain = ModifyPower(POWER_MANA,tmpvalue);
        pVictim->AddThreat(this, float(gain) * 0.5f, spellProto->School, spellProto);

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

        int32 gain = ModifyPower(power,mod->m_amount);
        ThreatAssist(this, float(gain) * 0.5f, spellProto->School, spellProto);

        if(pVictim->GetTypeId() == TYPEID_PLAYER || GetTypeId() == TYPEID_PLAYER)
            SendHealSpellOnPlayerPet(pVictim, spellProto->Id, mod->m_amount, power);
    }
    else if(mod->m_auraname == SPELL_AURA_OBS_MOD_MANA)
    {
        if(getPowerType() != POWER_MANA)
            return;

        int32 gain = ModifyPower(POWER_MANA, mod->m_amount);
        ThreatAssist(this, float(gain) * 0.5f, spellProto->School, spellProto);
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
        AuraList& mModTargetRes = GetAurasByType(SPELL_AURA_MOD_TARGET_RESISTANCE);
        for(AuraList::iterator i = mModTargetRes.begin(); i != mModTargetRes.end(); ++i)
            if ((*i)->GetModifier()->m_miscvalue & (1 << School))
                tmpvalue2 += (*i)->GetModifier()->m_amount;
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
        if(GetTypeId()== TYPEID_PLAYER)
            ((Player*)this)->UpdateWeaponSkill(attType);
        return;
    }

    *damage += CalculateDamage (attType);

    //Calculate the damage after armor mitigation if SPELL_SCHOOL_NORMAL
    if (*damageType == SPELL_SCHOOL_NORMAL)
        *damage = CalcArmorReducedDamage(pVictim, *damage);

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
            *damage -= uint32(pVictim->m_modResilience * 2/100 * (*damage));

            if(GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() != TYPEID_PLAYER && ((Creature*)pVictim)->GetCreatureInfo()->type != CREATURE_TYPE_CRITTER )
                ((Player*)this)->UpdateWeaponSkill(attType);

            pVictim->HandleEmoteCommand(EMOTE_ONESHOT_WOUNDCRITICAL);
            break;

        case MELEE_HIT_PARRY:
            if(attType == RANGED_ATTACK)                    //range attack - no parry
                break;
            // at parry warrior also receive rage like from hit to enemy
            if(GetTypeId() == TYPEID_PLAYER && getPowerType() == POWER_RAGE)
                ((Player*)this)->CalcRage(*damage,true);

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
            pVictim->SetFlag(UNIT_FIELD_AURASTATE, uint32(1<<(AURA_STATE_PARRY-1)));

            CastMeleeProcDamageAndSpell(pVictim, 0, attType, outcome, spellCasted, isTriggeredSpell);
            return;

        case MELEE_HIT_DODGE:
            if(attType == RANGED_ATTACK)                    //range attack - no dodge
                break;
            *damage = 0;
            *victimState = VICTIMSTATE_DODGE;

            if(pVictim->GetTypeId() == TYPEID_PLAYER)
                ((Player*)pVictim)->UpdateDefense();

            pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYUNARMED);
            pVictim->SetFlag(UNIT_FIELD_AURASTATE, uint32(1<<(AURA_STATE_DODGE-1)));

            CastMeleeProcDamageAndSpell(pVictim, 0, attType, outcome, spellCasted, isTriggeredSpell);
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

    // apply melee damage bonus and absorb only if base damage not fully blocked to prevent negative damage or damage with full block
    if(*victimState != VICTIMSTATE_BLOCKS)
    {
        MeleeDamageBonus(pVictim, damage,attType);
        CalcAbsorbResist(pVictim, *damageType, *damage-*blocked_amount, absorbDamage, resistDamage);
    }

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
        AuraList& mSpellCritSchool = GetAurasByType(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL);
        for(AuraList::iterator i = mSpellCritSchool.begin(); i != mSpellCritSchool.end(); ++i)
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
    int32 skillDiff =  GetWeaponSkillValue(attType) - pVictim->GetDefenceSkillValue();
    // bonus from skills is 0.04%
    int32    skillBonus = skillDiff * 4;
    int32    skillBonus2 = 4 * ( GetWeaponSkillValue(attType) - pVictim->GetPureDefenceSkillValue() );
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
        std::list<DynamicObject*>::iterator ite;
        for (ite = m_dynObj.begin(); ite != m_dynObj.end();)
        {
            //(*i)->Update( difftime );
            if( (*ite)->isFinished() )
            {
                (*ite)->Delete();
                ite = m_dynObj.erase(ite);
            }
            else
                ++ite;
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
    return MapManager::Instance().GetMap(GetMapId(), this)->IsInWater(GetPositionX(),GetPositionY());
}

bool Unit::IsUnderWater() const
{
    return MapManager::Instance().GetMap(GetMapId(), this)->IsUnderWater(GetPositionX(),GetPositionY(),GetPositionZ());
}

void Unit::DeMorph()
{
    SetUInt32Value(UNIT_FIELD_DISPLAYID, GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
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

    // passive auras stack with all
    if (!Aur->IsPassive() && !(Aur->GetSpellProto()->Id == 20584 || Aur->GetSpellProto()->Id == 8326)) // ghost spell check
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
        if (!(*i).second->GetSpellProto()) continue;
        if (IsPassiveSpell((*i).second->GetId())) continue;

        uint32 i_spellId = (*i).second->GetId();
        uint32 i_effIndex = (*i).second->GetEffIndex();

        // prevent remove dummy triggered spells at next effect aura add
        bool is_triggered_by_spell = false;
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

            if(is_triggered_by_spell)
                break;
        }

        if(i_spellId != spellId && !is_triggered_by_spell)
        {
            bool sec_match = false;
            bool is_i_sec = IsSpellSingleEffectPerCaster(i_spellId);
            if( is_sec && is_i_sec )
                if (Aur->GetCasterGUID() == (*i).second->GetCasterGUID())
                    if (GetSpellSpecific(spellId) == GetSpellSpecific(i_spellId))
                        sec_match = true;
            if( sec_match || objmgr.IsNoStackSpellDueToSpell(spellId, i_spellId) && !is_sec && !is_i_sec )
            {
                // if sec_match this isnt always true, needs to be rechecked
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
                Group::MemberList const& members = pGroup->GetMembers();
                for(Group::member_citerator itr = members.begin(); itr != members.end(); ++itr)
                {
                    if(!pGroup->SameSubGroup(i_caster->GetGUID(), &*itr))
                        continue;

                    Unit* Target = objmgr.GetPlayer(itr->guid);
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
        for(uint8 j = 0; j < MAX_SPELL_SCHOOL; j++)
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

    AuraList& mModOffhandDamagePct = GetAurasByType(SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT);
    for(AuraList::iterator i = mModOffhandDamagePct.begin(); i != mModOffhandDamagePct.end(); ++i)
    {
        if(((*i)->GetModifier()->m_miscvalue & 1) == 0)
            continue;

        Item* pItem = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        if (pItem && pItem->IsFitToSpellRequirements((*i)->GetSpellProto()))
            totaldamgemods[OFF_ATTACK] *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;
    }

    for (uint8 i = 0; i < MAX_STATS; i++)
        totalstatmods[i] = totalstatmods[i] * 100.0f - 100.0f;
    for (uint8 i = 0; i < MAX_SPELL_SCHOOL; i++)
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
        for (uint8 i = 0; i < MAX_SPELL_SCHOOL; i++)
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
    switch(getClass())
    {
        case CLASS_HUNTER: val2 = uint32(getLevel() * 2 + GetStat(STAT_AGILITY) * 2 - 20); break;
        case CLASS_ROGUE:  val2 = uint32(getLevel()     + GetStat(STAT_AGILITY) * 2 - 20); break;
        case CLASS_WARRIOR:val2 = uint32(getLevel()     + GetStat(STAT_AGILITY) * 2 - 20); break;
        default:           val2 = 0;                                                      break;
    }

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
                    val2 = uint32(getLevel()*2 + GetStat(STAT_STRENGTH)*2 - 20 + GetStat(STAT_AGILITY)); break;
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
    switch(getClass())
    {
        case CLASS_PALADIN: classrate = 19.77; break;
        case CLASS_SHAMAN:  classrate = 19.7;  break;
        case CLASS_MAGE:    classrate = 19.44; break;
        case CLASS_ROGUE:   classrate = 29.0;  break;
        case CLASS_HUNTER:  classrate = 33.0;  break;
        case CLASS_PRIEST:
        case CLASS_WARLOCK:
        case CLASS_DRUID:
        case CLASS_WARRIOR:
        default:            classrate = 20.0; break;
    }

    val = GetStat(STAT_AGILITY)/classrate;

    ApplyModFloatValue(PLAYER_CRIT_PERCENTAGE, val, apply);

    //Spell crit
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
    float crit_ratio = crit_data[getClass()].rate0 + crit_data[getClass()].rate1*getLevel();
    val = GetStat(STAT_INTELLECT) / crit_ratio;
    for (uint8 i = 0; i < 7; ++i)
        ApplyModFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1+i, val, apply);

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
        for (uint8 i = 0; i < MAX_SPELL_SCHOOL; i++)
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
            case SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT:
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
    for(AuraList::iterator i = mModDamagePct.begin(); i != mModDamagePct.end(); ++i)
        (*i)->ApplyModifier(false);
    AuraList& mModOffhandDamagePct = GetAurasByType(SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT);
    for(AuraList::iterator i = mModOffhandDamagePct.begin(); i != mModOffhandDamagePct.end(); ++i)
        (*i)->ApplyModifier(false);
}

void Unit::_ApplyAllAuraMods()
{
    // these must be applied before applystats
    AuraList& mModOffhandDamagePct = GetAurasByType(SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT);
    for(AuraList::iterator i = mModOffhandDamagePct.begin(); i != mModOffhandDamagePct.end(); ++i)
        (*i)->ApplyModifier(true);
    AuraList& mModDamagePct = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for(AuraList::iterator i = mModDamagePct.begin(); i != mModDamagePct.end(); ++i)
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
            case SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT:
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
                    int32 i_spell_param = (*i)->GetModifier()->m_amount;
                    if(*aur == SPELL_AURA_PROC_TRIGGER_SPELL)
                    {
                        // special case with random selection for druid melee attack event effect
                        if((*i)->GetId()==16864)
                        {
                            static uint32 spells[3] = { 12536, 16246, 16870 };
                            spellProto = sSpellStore.LookupEntry(spells[urand(0,2)]);
                        }
                        // normal case
                        else
                            spellProto = sSpellStore.LookupEntry(spellProto->EffectTriggerSpell[i_spell_eff]);

                        i_spell_param = procFlag;
                    }
                    if(*aur == SPELL_AURA_DUMMY)
                        i_spell_param = i_spell_eff;

                    if(spellProto)
                        procTriggered.push_back( ProcTriggeredData(spellProto,i_spell_param,*i) );
                }
            }

            // Handle effects proceed this time
            for(ProcTriggeredList::iterator i = procTriggered.begin(); i != procTriggered.end(); i++)
            {
                if(*aur == SPELL_AURA_PROC_TRIGGER_SPELL)
                {
                    sLog.outDebug("ProcDamageAndSpell: casting spell %u (triggered by an attacker's aura of spell %u)", i->spellInfo->Id,i->triggeredByAura->GetId());
                    if(IsPositiveSpell(i->spellInfo->Id) && !(i->spellParam & PROC_FLAG_HEAL))
                        CastSpell(this,i->spellInfo->Id,true,NULL,i->triggeredByAura);
                    else if(pVictim && pVictim->isAlive())
                        CastSpell(pVictim,i->spellInfo->Id,true,NULL,i->triggeredByAura);
                }
                else if(*aur == SPELL_AURA_PROC_TRIGGER_DAMAGE)
                {
                    sLog.outDebug("ProcDamageAndSpell: doing %u damage from spell id %u (triggered by an attacker's aura of spell %u)", i->spellParam, i->spellInfo->Id,i->triggeredByAura->GetId());
                    uint32 damage = i->spellParam;
                    // TODO: remove hack for Seal of Righteousness. That should not be there
                    if(i->spellInfo->SpellVisual == 7986)
                        damage = (damage * GetAttackTime(BASE_ATTACK))/60/1000;
                    if(pVictim && pVictim->isAlive())
                        SpellNonMeleeDamageLog(pVictim, i->spellInfo->Id, damage, true);
                }
                else if(*aur == SPELL_AURA_DUMMY)
                {
                    // TODO: write a DUMMY aura handle code
                    if (pVictim && pVictim->isAlive())
                    {
                        sLog.outDebug("ProcDamageAndSpell: casting spell id %u (triggered by an attacker dummy aura of spell %u)", i->spellInfo->Id,i->triggeredByAura->GetId());
                        HandleDummyAuraProc(pVictim, i->spellInfo, i->spellParam, damage, i->triggeredByAura);
                    }
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
    if(pVictim && pVictim->isAlive() && procVictim)
    {
        // additional auraTypes contains auras capable of proc'ing for victim
        auraTypes.push_back(SPELL_AURA_MOD_PARRY_PERCENT);

        for(std::list<uint32>::iterator aur = auraTypes.begin(); aur != auraTypes.end(); aur++)
        {
            // List of spells (effects) that proceed. Spell prototype and aura-specific value (damage for TRIGGER_DAMAGE)
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
                if(pVictim->GetTypeId() == TYPEID_PLAYER)
                    ((Player*)pVictim)->ApplySpellMod(spellProto->Id,SPELLMOD_CHANCE_OF_SUCCESS,chance);
                if(roll_chance_i(chance))
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
                        procTriggered.push_back( ProcTriggeredData(spellProto,i_spell_param,*i) );
                }
            }

            // Handle effects proced this time
            for(ProcTriggeredList::iterator i = procTriggered.begin(); i != procTriggered.end(); i++)
            {
                if(*aur == SPELL_AURA_PROC_TRIGGER_SPELL)
                {
                    sLog.outDebug("ProcDamageAndSpell: casting spell %u (triggered by a victim's aura of spell %u))",i->spellInfo->Id, i->triggeredByAura);
                    if(IsPositiveSpell(i->spellInfo->Id) && !(i->spellParam&PROC_FLAG_HEAL))
                        pVictim->CastSpell(pVictim,i->spellInfo->Id,true,NULL,i->triggeredByAura);
                    else
                        pVictim->CastSpell(this,i->spellInfo->Id,true,NULL,i->triggeredByAura);
                }
                else if(*aur == SPELL_AURA_PROC_TRIGGER_DAMAGE)
                {
                    sLog.outDebug("ProcDamageAndSpell: doing %u damage from spell id %u (triggered by a victim's aura of spell %u))", i->spellParam, i->spellInfo->Id, i->triggeredByAura);
                    pVictim->SpellNonMeleeDamageLog(this, i->spellInfo->Id, i->spellParam, true);
                }
                else if(*aur == SPELL_AURA_DUMMY)
                {
                    // TODO: write a DUMMY aura handle code
                    sLog.outDebug("ProcDamageAndSpell: casting spell %u (triggered by a victim's dummy aura of spell %u))",i->spellInfo->Id, i->triggeredByAura);
                    pVictim->HandleDummyAuraProc(this, i->spellInfo, i->spellParam, damage, i->triggeredByAura);
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

void Unit::HandleDummyAuraProc(Unit *pVictim, SpellEntry const *dummySpell, uint32 effIndex, uint32 damage, Aura* triggredByAura)
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
        CastSpell(pVictim, &igniteDot, true, NULL, triggredByAura);
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
    Unit const* testerOwner = GetOwner();
    Unit const* targetOwner = unit->GetOwner();

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

    // PvC forced reaction case
    if(tester->GetTypeId()==TYPEID_PLAYER)
    {
        // apply forced faction only in target with identical faction in other case provided original faction
        if(tester->HasAuraType(SPELL_AURA_FORCE_REACTION) && tester->getFaction()!= target_faction->ID)
        {
            FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(Player::getFactionForRace(tester->getRace()));
            if(entry)
                tester_faction = entry;
        }
    }
    // CvP forced reaction case
    else if(target->GetTypeId()==TYPEID_PLAYER)
    {
        // apply forced faction only in target with identical faction in other case provided original faction
        if(target->HasAuraType(SPELL_AURA_FORCE_REACTION) && target->getFaction()!= tester_faction->ID)
        {
            FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(Player::getFactionForRace(target->getRace()));
            if(entry)
                target_faction = entry;
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
    Unit const* testerOwner = GetOwner();
    Unit const* targetOwner = unit->GetOwner();

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

    // PvC forced reaction case
    if(tester->GetTypeId()==TYPEID_PLAYER)
    {
        // apply forced faction only in target with identical faction in other case provided original faction
        if(tester->HasAuraType(SPELL_AURA_FORCE_REACTION) && tester->getFaction()!= target_faction->ID)
        {
            FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(Player::getFactionForRace(tester->getRace()));
            if(entry)
                tester_faction = entry;
        }
    }
    // CvP forced reaction case
    else if(target->GetTypeId()==TYPEID_PLAYER)
    {
        // apply forced faction only in target with identical faction in other case provided original faction
        if(target->HasAuraType(SPELL_AURA_FORCE_REACTION) && target->getFaction()!= tester_faction->ID)
        {
            FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(Player::getFactionForRace(target->getRace()));
            if(entry)
                target_faction = entry;
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

    Creature* charmed = GetCharm();
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
    uint32 PenaltyFactor = 0;
    uint32 CastingTime = GetCastTime(sCastTimesStore.LookupEntry(spellProto->CastingTimeIndex));
    if (CastingTime > 7000) CastingTime = 7000;             // Plus Damage efficient maximum 200% ( 7.0 seconds )
    if (CastingTime < 1500) CastingTime = 1500;

    // Taken/Done fixed damage bonus auras
    int32 DoneAdvertisedBenefit = 0;
    int32 TakenAdvertisedBenefit = 0;

    // ..done (for creature type by mask) in taken
    AuraList& mDamageDoneCreature = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE_CREATURE);
    for(AuraList::iterator i = mDamageDoneCreature.begin();i != mDamageDoneCreature.end(); ++i)
        if(cinfo && cinfo->type && ((1 << (cinfo->type-1)) & uint32((*i)->GetModifier()->m_miscvalue)))
            TakenAdvertisedBenefit += (*i)->GetModifier()->m_amount;

    // ..done
    AuraList& mDamageDone = this->GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE);
    for(AuraList::iterator i = mDamageDone.begin();i != mDamageDone.end(); ++i)
        if(((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
            DoneAdvertisedBenefit += (*i)->GetModifier()->m_amount;

    // ..taken
    AuraList& mDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_TAKEN);
    for(AuraList::iterator i = mDamageTaken.begin();i != mDamageTaken.end(); ++i)
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
                DoneAdvertisedBenefit /= DotTicks;
        }
    }

    // Taken/Done total percent damage auras
    float DoneTotalMod = 1.0f;
    float TakenTotalMod = 1.0f;

    // ..done
    AuraList& mModDamagePercentDone = this->GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for(AuraList::iterator i = mModDamagePercentDone.begin(); i != mModDamagePercentDone.end(); ++i)
        if( spellProto->School != 0 && ((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0 )
            DoneTotalMod *= ((*i)->GetModifier()->m_amount+100.0f)/100.0f;

    // ..taken
    AuraList& mModDamagePercentTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    for(AuraList::iterator i = mModDamagePercentTaken.begin(); i != mModDamagePercentTaken.end(); ++i)
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
    float TakenActualBenefit = TakenAdvertisedBenefit * (CastingTime / 3500.0f) * (100.0f - LvlPenalty) * LvlFactor / 100.0f;

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
    static const struct
    {
        float base;
        float rate0, rate1;
    }
    crit_data[MAX_CLASSES] =
    {
        {                                                   //  0: unused
            0,0,10
        },
        {                                                   //  1: warrior
            0,0,10
        },
        {                                                   //  2: paladin
            3.70, 14.77, 0.65
        },
        {                                                   //  3: hunter
            0,0,10
        },
        {                                                   //  4: rogue
            0,0,10
        },
        {                                                   //  5: priest
            2.97, 10.03, 0.82
        },
        {                                                   //  6: unused
            0,0,10
        },
        {                                                   //  7: shaman
            3.54, 11.51, 0.80
        },
        {                                                   //  8: mage
            3.70, 14.77, 0.65
        },
        {                                                   //  9: warlock
            3.18, 11.30, 0.82
        },
        {                                                   // 10: unused
            0,0,10
        },
        {                                                   // 11: druid
            3.33, 12.41, 0.79
        }
    };
    float crit_chance;

    // only players use intelligence for critical chance computations
    if (GetTypeId() == TYPEID_PLAYER)
    {
        crit_chance = GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + spellProto->School);
        ((Player*)this)->ApplySpellMod(spellProto->Id, SPELLMOD_CRITICAL_CHANCE, crit_chance);
    }
    else
        crit_chance = m_baseSpellCritChance;

    // TODO: can creatures have critical chance auras?
    AuraList& mSpellCritSchool = GetAurasByType(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL);
    for(AuraList::iterator i = mSpellCritSchool.begin(); i != mSpellCritSchool.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue == -2 || ((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
            crit_chance += (*i)->GetModifier()->m_amount;

    // Resilience - reduce crit chance by x%
    if (pVictim)
        crit_chance -= pVictim->m_modResilience;

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

uint32 Unit::SpellHealingBonus(SpellEntry const *spellProto, uint32 healamount, DamageEffectType damagetype)
{
    // Healing Done

    // Vampiric Embrace, Shadowmend - cannot critically heal
    if(spellProto->Id == 15290 || spellProto->Id == 39373) return healamount;

    int32 AdvertisedBenefit = 0;
    uint32 PenaltyFactor = 0;
    uint32 CastingTime = GetCastTime(sCastTimesStore.LookupEntry(spellProto->CastingTimeIndex));
    if (CastingTime > 7000) CastingTime = 7000;
    if (CastingTime < 1500) CastingTime = 1500;
    if (spellProto->Effect[0] == SPELL_EFFECT_APPLY_AURA) CastingTime = 3500;

    AuraList& mHealingDone = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE);
    for(AuraList::iterator i = mHealingDone.begin();i != mHealingDone.end(); ++i)
        if(((*i)->GetModifier()->m_miscvalue & (int32)(1<<spellProto->School)) != 0)
            AdvertisedBenefit += (*i)->GetModifier()->m_amount;

    //put m_AuraModifiers here

    AdvertisedBenefit += m_AuraModifiers[SPELL_AURA_MOD_HEALING];

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
    AuraList& mHealingPct = GetAurasByType(SPELL_AURA_MOD_HEALING_PCT);
    for(AuraList::iterator i = mHealingPct.begin();i != mHealingPct.end(); ++i)
        heal *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;
    AuraList& mHealingDonePct = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
    for(AuraList::iterator i = mHealingDonePct.begin();i != mHealingDonePct.end(); ++i)
        heal *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;

    //heal += float(m_AuraModifiers[SPELL_AURA_MOD_HEALING]);

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
            if(happiness>=750000)
                *pdamage = uint32(*pdamage * 1.25);
            else if(happiness>=500000)
                *pdamage = uint32(*pdamage * 1.0);
            else *pdamage = uint32(*pdamage * 0.75);
        }
    }

    // Taken/Done fixed damage bonus auras
    int32 DoneFlatBenefit = 0;
    int32 TakenFlatBenefit = 0;

    // ..done (for creature type by mask) in taken
    AuraList& mDamageDoneCreature = this->GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE_CREATURE);
    for(AuraList::iterator i = mDamageDoneCreature.begin();i != mDamageDoneCreature.end(); ++i)
        if(cinfo && cinfo->type && ((1 << (cinfo->type-1)) & uint32((*i)->GetModifier()->m_miscvalue)))
            DoneFlatBenefit += (*i)->GetModifier()->m_amount;

    // ..done
    // SPELL_AURA_MOD_DAMAGE_DONE included in weapon damage

    // ..done (base at attack power and creature type)
    AuraList& mCreatureAttackPower = GetAurasByType(SPELL_AURA_MOD_CREATURE_ATTACK_POWER);
    for(AuraList::iterator i = mCreatureAttackPower.begin();i != mCreatureAttackPower.end(); ++i)
        if(cinfo && cinfo->type && ((1 << (cinfo->type-1)) & uint32((*i)->GetModifier()->m_miscvalue)))
            DoneFlatBenefit += int32((*i)->GetModifier()->m_amount/14.0f * GetAttackTime(attType)/1000);

    // ..done (base at attack power for marked target)
    if(attType == RANGED_ATTACK)
    {
        AuraList& mRangedAttackPowerAttackerBonus = pVictim->GetAurasByType(SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS);
        for(AuraList::iterator i = mRangedAttackPowerAttackerBonus.begin();i != mRangedAttackPowerAttackerBonus.end(); ++i)
            DoneFlatBenefit += int32((*i)->GetModifier()->m_amount/14.0f * GetAttackTime(RANGED_ATTACK)/1000);
    }

    // ..taken
    AuraList& mDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_TAKEN);
    for(AuraList::iterator i = mDamageTaken.begin();i != mDamageTaken.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue & IMMUNE_SCHOOL_PHYSICAL)
            TakenFlatBenefit += (*i)->GetModifier()->m_amount;

    if(attType!=RANGED_ATTACK)
    {
        AuraList& mModMeleeDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN);
        for(AuraList::iterator i = mModMeleeDamageTaken.begin(); i != mModMeleeDamageTaken.end(); ++i)
            TakenFlatBenefit += (*i)->GetModifier()->m_amount;
    }
    else
    {
        AuraList& mModRangedDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN);
        for(AuraList::iterator i = mModRangedDamageTaken.begin(); i != mModRangedDamageTaken.end(); ++i)
            TakenFlatBenefit += (*i)->GetModifier()->m_amount;
    }

    // Done/Taken total percent damage auras
    float TakenTotalMod = 1;

    // ..done
    // SPELL_AURA_MOD_DAMAGE_PERCENT_DONE included in weapon damage
    // SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT  included in weapon damage

    // ..taken
    AuraList& mModDamagePercentTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    for(AuraList::iterator i = mModDamagePercentTaken.begin(); i != mModDamagePercentTaken.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue & IMMUNE_SCHOOL_PHYSICAL)
            TakenTotalMod *= ((*i)->GetModifier()->m_amount+100.0f)/100.0f;

    if(attType != RANGED_ATTACK)
    {
        AuraList& mModMeleeDamageTakenPercent = pVictim->GetAurasByType(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT);
        for(AuraList::iterator i = mModMeleeDamageTakenPercent.begin(); i != mModMeleeDamageTakenPercent.end(); ++i)
            TakenTotalMod *= ((*i)->GetModifier()->m_amount+100.0f)/100.0f;
    }
    else
    {
        AuraList& mModRangedDamageTakenPercent = pVictim->GetAurasByType(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT);
        for(AuraList::iterator i = mModRangedDamageTakenPercent.begin(); i != mModRangedDamageTakenPercent.end(); ++i)
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

    if(uint32(val) < maxHealth)
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

bool Unit::isVisibleFor(Unit* u, bool detect)
{
    if(!u)
        return false;

    // Visible units, always are visible for all pjs
    if (m_Visibility == VISIBILITY_ON)
        return true;

    // Always can see self
    if (u==this)
        return true;

    // GMs are visible for higher gms (or players are visible for gms)
    if (u->GetTypeId() == TYPEID_PLAYER && ((Player *)u)->isGameMaster())
        return (GetTypeId() == TYPEID_PLAYER && ((Player *)this)->GetSession()->GetSecurity() <= ((Player *)u)->GetSession()->GetSecurity());

    // non faction visibility non-breakable for non-GMs
    if (m_Visibility == VISIBILITY_OFF)
        return false;

    // Units far than MAX_DIST_INVISIBLE or not in our map, that are not gms and are stealth, are not visibles too
    if (!this->IsWithinDistInMap(u,MAX_DIST_INVISIBLE_UNIT))
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

    IsVisible = roll_chance_f(prob);

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
    if(GetTypeId() == TYPEID_PLAYER && IsInWorld())
    {
        Map *m = MapManager::Instance().GetMap(GetMapId(), this);
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
    WorldPacket data(0,0);

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
        UnsummonTotem();
    }
    if (m_deathState != ALIVE && s == ALIVE)
    {
        _ApplyAllAuraMods();
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

float Unit::GetThreat(uint64 guid) const
{
    //use to get total threat of certain target in ThreatList
    ThreatList::const_iterator i;
    float threat = 0.0f;

    for ( i = m_threatList.begin(); i!= m_threatList.end(); i++)
    {
        if(i->UnitGuid==guid)
        {
            threat=i->Threat;
            break;
        }
    }

    return threat;
}

void Unit::AddThreat(Unit* pVictim, float threat, uint8 school, SpellEntry const *threatSpell)
{
    //function deals with adding threat and adding players and pets into ThreatList
    //mobs, NPCs, guards have ThreatList and HateOfflineList
    //players and pets have only InHateListOf
    //HateOfflineList is used co contain unattackable victims (in-flight, in-water, GM etc.)
                                                            //pVictim = player or pet
    if(!pVictim || (pVictim->GetTypeId() == TYPEID_PLAYER && ((Player*)pVictim)->isGameMaster()) )
        return;

    if(!CanHaveThreatList())
        return;

    assert(GetTypeId()== TYPEID_UNIT);
    bool isinlist = false;

    pVictim->AddToInHateList((Creature*)this);
    uint64 guid = pVictim->GetGUID();

    if (pVictim->GetTypeId() == TYPEID_PLAYER && threatSpell)
        ((Player*)pVictim)->ApplySpellMod(threatSpell->Id, SPELLMOD_THREAT, threat);

    threat = pVictim->ApplyTotalThreatModifier(threat, school);

    //add attacker into ThreatList of a creature
    for(ThreatList::iterator i = m_threatList.begin(); i != m_threatList.end(); i++)
    {
        if(i->UnitGuid==guid)
        {
            i->Threat += threat;
            isinlist = true;
            break;
        }
    }

    if(!isinlist)
        m_threatList.push_back(Hostil(guid,threat));

    if(getVictim() && (guid == getVictim()->GetGUID()))
    {
        UpdateCurrentVictimThreat(threat);
        if(threat<0.0f)
            SortList(true);
    }
    else
        SortList(true);

    //threat link to owner
    Unit* victim_owner = pVictim->GetOwner();
    if(victim_owner && victim_owner->isAlive())
        AddThreat(victim_owner, 0.0f);
}

void Unit::AddToInHateList(Creature* attacker)
{
    //adds attacker into InHateListOf list of players and pets
    //only players and pets can have InHateListOf list!
    if(!attacker)
        return;

    if(!attacker->CanHaveThreatList())
        return;

                                                            //add creature into InHateListOf
    InHateListOf::iterator iter = m_inhateList.find(attacker);
    if(iter == m_inhateList.end())
    {
        m_inhateList.insert(attacker);
        SetInCombat();                                      //when player is in threatlist he is in combat
    }
}

void Unit::ThreatAssist(Unit* target, float threat, uint8 school, SpellEntry const *threatSpell, bool singletarget)
{
    if(!target)
        return;

    //used to add player/pet into several ThreatLists
    //of those mobs which have target in it
    //use for buffs and healing threat functionality
    InHateListOf& InHateList = target->GetInHateListOf();
    uint32 count_enemies = singletarget ? 1 : InHateList.size();

    for(InHateListOf::iterator iter = InHateList.begin(); iter != InHateList.end(); ++iter)
    {
        (*iter)->AddThreat(this, float (threat) / count_enemies, school, threatSpell);
    }
}

void Unit::RemoveFromInHateListOf(Creature* attacker)
{
    //used to delete mobs from InHateListOf lists
    //of players and pets and leaving combat when list becomes empty
    if(!attacker)
        return;

    if(IsInHateListEmpty())
        return;

    //used to delete mobs from InHateList of players
    InHateListOf::iterator iter = m_inhateList.find(attacker);
    if(iter != m_inhateList.end())
        m_inhateList.erase(iter);

    if(IsInHateListEmpty())
        ClearInCombat();
}

void Unit::RemoveFromThreatList(uint64 guid)
{
    //function is used to delete guids from threat lists of mobs
    // and call for EnterEvadeMode() if it became empty
    if(!guid)
        return;

    if(IsThreatListEmpty())
        return;

    for(ThreatList::iterator i = m_threatList.begin(); i != m_threatList.end(); i++)
    {
        if(i->UnitGuid==guid)
        {
            if(getVictim() && getVictim()->GetGUID() == guid)
                SetCurrentVictimThreat(0.0f);
            m_threatList.erase(i);
            break;
        }
    }

    if(IsThreatListEmpty() && GetTypeId() == TYPEID_UNIT && ((Creature*)this)->AI())
        ((Creature*)this)->AI()->EnterEvadeMode();
}

void Unit::DeleteThreatList()
{
    //function is used for erasing ThreatList and HateOffline lists
    //on mob deathe with removing it from players and pets
    //who have them in their InHateListOf lists
    if(!CanHaveThreatList())
        return;

    if(GetTypeId()==TYPEID_UNIT)                            // only creatures can be in inHateListOf
    {
        //used to clear mob's threat list
        while(!m_threatList.empty())
        {
            ThreatList::iterator i = m_threatList.begin();
            uint64 guid = i->UnitGuid;
            m_threatList.erase(i);

            Unit* unit = ObjectAccessor::Instance().GetUnit(*this, guid);
            if(unit)
                unit->RemoveFromInHateListOf((Creature*)this);
        }

        while(!m_offlineList.empty())
        {
            HateOfflineList::iterator iter = m_offlineList.begin();
            uint64 guid = iter->UnitGuid;
            m_offlineList.erase(iter);

            Unit* unit = ObjectAccessor::Instance().GetUnit(*this, guid);
            if(unit)
                unit->RemoveFromInHateListOf((Creature*)this);
        }
    }

    SetCurrentVictimThreat(0.0f);
}

void Unit::DeleteInHateListOf()
{
    //function is used for erasing InHateListOf list
    //on player's or pet's deathes with removing them from mobs
    //who have them in their threat lists
    if(CanHaveThreatList())
        return;

    if(IsInHateListEmpty())
        return;

    uint64 guid = GetGUID();
    DEBUG_LOG("InHateList list deletion started");
    //use for players to delete InHateListOf
    while(!m_inhateList.empty())
    {
        InHateListOf::iterator iter = m_inhateList.begin();
        Creature* unit = *iter;
        m_inhateList.erase(iter);

        unit->RemoveFromThreatList(guid);
    }

    DEBUG_LOG("InHateList list deleted");
}

bool Unit::SelectHostilTarget()
{
    //function provides main threat functionality
    //next-victim-selection algorithm and evade mode are called
    //threat list sorting etc.
    assert(GetTypeId()== TYPEID_UNIT);

    if(IsThreatListEmpty())
    {
        if(!IsHateOfflineListEmpty() && ((Creature*)this)->AI())
            ((Creature*)this)->AI()->EnterEvadeMode();
        return false;
    }

    if(HasAuraType(SPELL_AURA_MOD_TAUNT))
        return true;

    if(IsThreatListNeedsSorting())                          //sort ThreatList if it is not sorted after AddThreat()
    {
        m_threatList.sort();
        m_threatList.reverse();
        SortList(false);                                    //set not to sort ThreatList until next AddThreat() call
    }

    Unit* target = NULL;
    float threat = m_threatList.front().Threat;
    if(!getVictim() || threat > 1.1f * GetCurrentVictimThreat())
        target = SelectNextVictim();
    else
        return true;

    if(target)
    {
        SetInFront(target);
        if (((Creature*)this)->AI())
            ((Creature*)this)->AI()->AttackStart(target);
        return true;
    }
    else if (((Creature*)this)->AI())
        ((Creature*)this)->AI()->EnterEvadeMode();

    return false;
}

Unit* Unit::SelectNextVictim()
{
    //function used to find first nearest attackable target
    //with max threat in threat list
    //ALL mobs threat behavior bugs are connected to this function!!!
    bool hasVictim = false;
    uint64 guid = 0;
    float threat = 0;
    Unit* target = NULL;

    if(getVictim())
    {
        hasVictim = true;
        threat = GetCurrentVictimThreat();
        guid = getVictim()->GetGUID();
    }

    for(ThreatList::iterator iter = m_threatList.begin(); iter != m_threatList.end(); ++iter)
    {
        target = ObjectAccessor::Instance().GetUnit(*this, iter->UnitGuid);
        if(!target)
            continue;

        if(hasVictim)
        {
            if(iter->UnitGuid == guid)
                return target;

            Map* map = MapManager::Instance().GetMap(GetMapId(), this);
            if((map->Instanceable() || (!((Creature*)this)->IsOutOfThreatArea(target))) && target->isInAccessablePlaceFor( ((Creature*)this) ))
            {
                if((IsWithinDistInMap(target, ATTACK_DIST) && (iter->Threat > 1.1f * threat)) || (iter->Threat > 1.3f * threat))
                {                                           //implement 110% threat rule for targets in melee range
                    SetCurrentVictimThreat(iter->Threat);   //and 130% rule for targets in ranged distances
                    return target;                          //for selecting alive targets
                }
            }
        }
        else if(!((Creature*)this)->IsOutOfThreatArea(target) && target->isInAccessablePlaceFor( ((Creature*)this) ) )
        {
            SetCurrentVictimThreat(iter->Threat);
            return target;
        }

    }

    return NULL;
}

void Unit::MoveToHateOfflineList()
{
    //use to move players and pets guids into HateOfflineLists of those mobs
    //which have them in threat lists. Cause: unattackable, in-flight, in-water etc.
    //InHateListOf list is not gets deleted!
    if(IsInHateListEmpty())
        return;

    uint64 guid = this->GetGUID();

    for(InHateListOf::iterator iter = m_inhateList.begin(); iter != m_inhateList.end(); iter++)
    {
        (*iter)->MoveGuidToOfflineList(guid);
    }
}

void Unit::MoveToThreatList()
{
    //move guid from HateOfflineList into ThreatList when player/pet became attackable, got on surface etc.
    if(IsInHateListEmpty())
        return;

    uint64 guid = this->GetGUID();

    for(InHateListOf::iterator iter = m_inhateList.begin(); iter != m_inhateList.end(); iter++)
    {
        (*iter)->MoveGuidToThreatList(guid);
    }
}

bool Unit::MoveGuidToThreatList(uint64 guid)
{
    //auxiliary function, part of MoveToThreatList() func, but could be used as standalone func
    //example in Player::SetInWater()
    if(!guid)
        return false;

    for(HateOfflineList::iterator itr = m_offlineList.begin(); itr != m_offlineList.end(); itr++)
    {
        if(itr->UnitGuid == guid)
        {
            m_threatList.push_back(Hostil(itr->UnitGuid, itr->Threat));
            m_offlineList.erase(itr);
            if(getVictim() && getVictim()->GetGUID() == guid)
                SetCurrentVictimThreat(0.0f);
            return true;
        }
    }

    return false;
}

bool Unit::MoveGuidToOfflineList(uint64 guid)
{
    //auxiliary func, part of MoveToHateOfflineList(), but could be used as standalone func
    //example in Player::SetInWater()
    if(!guid)
        return false;

    for(ThreatList::iterator itr = m_threatList.begin(); itr != m_threatList.end(); itr++)
    {
        if(itr->UnitGuid == guid)
        {
            m_offlineList.push_back(Hostil(itr->UnitGuid, itr->Threat));
            m_threatList.erase(itr);
            return true;
        }
    }

    return false;
}

float Unit::ApplyTotalThreatModifier(float threat, uint8 school)
{
    if(!HasAuraType(SPELL_AURA_MOD_THREAT))
        return threat;

    if(school >= MAX_SPELL_SCHOOL)
        return threat;

    return threat * m_threatModifier[school];
}

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

    uint64 guid = taunter->GetGUID();
    float threat = GetCurrentVictimThreat();

    bool isinlist = false;
    for(ThreatList::iterator i = m_threatList.begin(); i != m_threatList.end(); i++)
    {
        if(i->UnitGuid==guid)
        {
            i->Threat = threat;
            isinlist = true;
            break;
        }
    }

    SortList(true);
}

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

    if(IsThreatListEmpty())
    {
        if(!IsHateOfflineListEmpty() && ((Creature*)this)->AI())
            ((Creature*)this)->AI()->EnterEvadeMode();
        return;
    }

    if(IsThreatListNeedsSorting())                          //sort ThreatList if it is not sorted after AddThreat()
    {
        m_threatList.sort();
        m_threatList.reverse();
        SortList(false);                                    //set not to sort ThreatList until next AddThreat() call
    }

    float taunterThreat = GetCurrentVictimThreat();
    Hostil topgun  = m_threatList.front();
    if(topgun.UnitGuid != taunter->GetGUID())
    {
        target = ObjectAccessor::Instance().GetUnit(*this, topgun.UnitGuid);
    }
    else
    {
        m_threatList.pop_front();
        if(!IsThreatListEmpty())
        {
            Hostil secondgun = m_threatList.front();
            if(taunterThreat < 1.1f * m_threatList.front().Threat)
                target = ObjectAccessor::Instance().GetUnit(*this, secondgun.UnitGuid);
        }
        m_threatList.push_front(topgun);
    }

    if (target && target != taunter)
    {
        SetInFront(target);
        if (((Creature*)this)->AI())
            ((Creature*)this)->AI()->AttackStart(target);
    }
}

int32 Unit::CalculateSpellDamage(SpellEntry const* spellProto, uint8 effect_index)
{
    int32 value = 0;
    uint32 level = 0;

    // currently the damage should not be increased by level
    /*level= caster->getLevel();
    if( level > spellproto->maxLevel && spellproto->maxLevel > 0)
        level = spellproto->maxLevel;*/

    float basePointsPerLevel = spellProto->EffectRealPointsPerLevel[effect_index];
    float randomPointsPerLevel = spellProto->EffectDicePerLevel[effect_index];
    int32 basePoints = int32(spellProto->EffectBasePoints[effect_index] +1 + level * basePointsPerLevel);
    int32 randomPoints = int32(spellProto->EffectDieSides[effect_index] + level * randomPointsPerLevel);
    float comboDamage = spellProto->EffectPointsPerComboPoint[effect_index];
    uint8 comboPoints=0;
    if(GetTypeId() == TYPEID_PLAYER)
        comboPoints = (uint8)((GetUInt32Value(PLAYER_FIELD_BYTES) & 0xFF00) >> 8);

    value += spellProto->EffectBaseDice[effect_index];
    if(randomPoints <= 1)
        value = basePoints;
    else
        value = basePoints+urand(0, randomPoints-1);

    if(comboDamage > 0)
    {
        value += (int32)(comboDamage * comboPoints);
        // Eviscerate
        if( spellProto->SpellIconID == 514 && spellProto->SpellFamilyName == SPELLFAMILY_ROGUE)
            value += (int32)(GetUInt32Value(UNIT_FIELD_ATTACK_POWER) * comboPoints * 0.03);
        if(GetTypeId() == TYPEID_PLAYER)
            SetUInt32Value(PLAYER_FIELD_BYTES,((GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (0x00 << 8)));
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
    if(duration == -1)
        return;

    if(mech != DIMINISHING_NONE)
    {
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
}

Creature* Unit::SummonCreature(uint32 id, uint32 mapid, float x, float y, float z, float ang,TempSummonType spwtype,uint32 despwtime)
{
    TemporarySummon* pCreature = new TemporarySummon(this,this);

    pCreature->SetInstanceId(GetInstanceId());

    if (!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), mapid, x, y, z, ang, id))
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
