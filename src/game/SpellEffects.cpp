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
#include "CreatureAI.h"
#include "Spell.h"
#include "DynamicObject.h"
#include "SpellAuras.h"
#include "Group.h"
#include "UpdateData.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "RedZoneDistrict.h"
#include "CellImpl.h"
#include "SharedDefines.h"
#include "Pet.h"
#include "GameObject.h"
#include "GossipDef.h"
#include "Creature.h"
#include "Totem.h"
#include "CreatureAI.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

pEffect SpellEffects[TOTAL_SPELL_EFFECTS]=
{
    &Spell::EffectNULL,                                     //nothing
    &Spell::EffectInstaKill,                                //SPELL_EFFECT_INSTAKILL = 1
    &Spell::EffectSchoolDMG,                                //SPELL_EFFECT_SCHOOL_DAMAGE = 2
    &Spell::EffectDummy,                                    //SPELL_EFFECT_DUMMY = 3
    &Spell::EffectNULL,                                     //SPELL_EFFECT_PORTAL_TELEPORT = 4
    &Spell::EffectTeleportUnits,                            //SPELL_EFFECT_TELEPORT_UNITS = 5
    &Spell::EffectApplyAura,                                //SPELL_EFFECT_APPLY_AURA = 6
    &Spell::EffectSchoolDMG,                                //SPELL_EFFECT_ENVIRONMENTAL_DAMAGE =7
    &Spell::EffectManaDrain,                                //SPELL_EFFECT_MANA_DRAIN = 8
    &Spell::EffectHealthLeach,                              //SPELL_EFFECT_HEALTH_LEECH = 9
    &Spell::EffectHeal,                                     //SPELL_EFFECT_HEAL = 10
    &Spell::EffectNULL,                                     //SPELL_EFFECT_BIND = 11
    &Spell::EffectNULL,                                     //SPELL_EFFECT_PORTAL = 12
    &Spell::EffectNULL,                                     //SPELL_EFFECT_RITUAL_BASE = 13
    &Spell::EffectNULL,                                     //SPELL_EFFECT_RITUAL_SPECIALIZE = 14
    &Spell::EffectNULL,                                     //SPELL_EFFECT_RITUAL_ACTIVATE_PORTAL = 15
    &Spell::EffectQuestComplete,                            //SPELL_EFFECT_QUEST_COMPLETE = 16
    &Spell::EffectWeaponDmg,                                //SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL = 17
    &Spell::EffectResurrect,                                //SPELL_EFFECT_RESURRECT = 18
    &Spell::EffectNULL,                                     //SPELL_EFFECT_ADD_EXTRA_ATTACKS = 19
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DODGE = 20
    &Spell::EffectNULL,                                     //SPELL_EFFECT_EVADE = 21
    &Spell::EffectParry,                                    //SPELL_EFFECT_PARRY = 22
    &Spell::EffectNULL,                                     //SPELL_EFFECT_BLOCK = 23
    &Spell::EffectCreateItem,                               //SPELL_EFFECT_CREATE_ITEM = 24
    &Spell::EffectNULL,                                     //SPELL_EFFECT_WEAPON = 25
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DEFENSE = 26
    &Spell::EffectPersistentAA,                             //SPELL_EFFECT_PERSISTENT_AREA_AURA = 27
    &Spell::EffectSummon,                                   //SPELL_EFFECT_SUMMON = 28
    &Spell::EffectMomentMove,                               //SPELL_EFFECT_LEAP = 29
    &Spell::EffectEnergize,                                 //SPELL_EFFECT_ENERGIZE = 30
    &Spell::EffectWeaponDmg,                                //SPELL_EFFECT_WEAPON_PERCENT_DAMAGE = 31
    &Spell::EffectNULL,                                     //SPELL_EFFECT_TRIGGER_MISSILE = 32 //Useless
    &Spell::EffectOpenLock,                                 //SPELL_EFFECT_OPEN_LOCK = 33
    &Spell::EffectSummonChangeItem,                         //SPELL_EFFECT_SUMMON_CHANGE_ITEM = 34
    &Spell::EffectApplyAA,                                  //SPELL_EFFECT_APPLY_AREA_AURA = 35
    &Spell::EffectLearnSpell,                               //SPELL_EFFECT_LEARN_SPELL = 36
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SPELL_DEFENSE = 37 //Useless
    &Spell::EffectDispel,                                   //SPELL_EFFECT_DISPEL = 38
    &Spell::EffectNULL,                                     //SPELL_EFFECT_LANGUAGE = 39
    &Spell::EffectDualWield,                                //SPELL_EFFECT_DUAL_WIELD = 40
    &Spell::EffectSummonWild,                               //SPELL_EFFECT_SUMMON_WILD = 41
    &Spell::EffectSummonWild,                               //SPELL_EFFECT_SUMMON_GUARDIAN = 42
    &Spell::EffectTeleUnitsFaceCaster,                      //SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER = 43
    &Spell::EffectLearnSkill,                               //SPELL_EFFECT_SKILL_STEP = 44
    &Spell::EffectNULL,                                     //unknown45 = 45
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SPAWN = 46
    &Spell::EffectTradeSkill,                               //SPELL_EFFECT_TRADE_SKILL = 47
    &Spell::EffectNULL,                                     //SPELL_EFFECT_STEALTH = 48 //Useless
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DETECT = 49
    &Spell::EffectTransmitted,                              //SPELL_EFFECT_TRANS_DOOR = 50
    &Spell::EffectNULL,                                     //SPELL_EFFECT_FORCE_CRITICAL_HIT = 51 //Useless
    &Spell::EffectNULL,                                     //SPELL_EFFECT_GUARANTEE_HIT = 52
    &Spell::EffectEnchantItemPerm,                          //SPELL_EFFECT_ENCHANT_ITEM = 53
    &Spell::EffectEnchantItemTmp,                           //SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY = 54
    &Spell::EffectTameCreature,                             //SPELL_EFFECT_TAMECREATURE = 55
    &Spell::EffectSummonPet,                                //SPELL_EFFECT_SUMMON_PET = 56
    &Spell::EffectLearnPetSpell,                            //SPELL_EFFECT_LEARN_PET_SPELL = 57
    &Spell::EffectWeaponDmg,                                //SPELL_EFFECT_WEAPON_DAMAGE = 58
    &Spell::EffectOpenSecretSafe,                           //SPELL_EFFECT_OPEN_LOCK_ITEM = 59
    &Spell::EffectProficiency,                              //SPELL_EFFECT_PROFICIENCY = 60
    &Spell::EffectSendEvent,                                //SPELL_EFFECT_SEND_EVENT = 61
    &Spell::EffectPowerDrain,                               //SPELL_EFFECT_POWER_BURN = 62
    &Spell::EffectThreat,                                   //SPELL_EFFECT_THREAT = 63
    &Spell::EffectTriggerSpell,                             //SPELL_EFFECT_TRIGGER_SPELL = 64
    &Spell::EffectNULL,                                     //SPELL_EFFECT_HEALTH_FUNNEL = 65 //Useless
    &Spell::EffectNULL,                                     //SPELL_EFFECT_POWER_FUNNEL = 66 //Useless
    &Spell::EffectHealMaxHealth,                            //SPELL_EFFECT_HEAL_MAX_HEALTH = 67
    &Spell::EffectInterruptCast,                            //SPELL_EFFECT_INTERRUPT_CAST = 68
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DISTRACT = 69
    &Spell::EffectNULL,                                     //SPELL_EFFECT_PULL = 70
    &Spell::EffectPickPocket,                               //SPELL_EFFECT_PICKPOCKET = 71
    &Spell::EffectNULL,                                     //SPELL_EFFECT_ADD_FARSIGHT = 72
    &Spell::EffectSummonWild,                               //SPELL_EFFECT_SUMMON_POSSESSED = 73
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_TOTEM = 74 //Useless
    &Spell::EffectNULL,                                     //SPELL_EFFECT_HEAL_MECHANICAL = 75
    &Spell::EffectSummonObjectWild,                         //SPELL_EFFECT_SUMMON_OBJECT_WILD = 76
    &Spell::EffectScriptEffect,                             //SPELL_EFFECT_SCRIPT_EFFECT = 77
    &Spell::EffectNULL,                                     //SPELL_EFFECT_ATTACK = 78 //Useless
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SANCTUARY = 79
    &Spell::EffectAddComboPoints,                           //SPELL_EFFECT_ADD_COMBO_POINTS = 80
    &Spell::EffectNULL,                                     //SPELL_EFFECT_CREATE_HOUSE = 81
    &Spell::EffectNULL,                                     //SPELL_EFFECT_BIND_SIGHT = 82
    &Spell::EffectDuel,                                     //SPELL_EFFECT_DUEL = 83
    &Spell::EffectNULL,                                     //SPELL_EFFECT_STUCK = 84
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_PLAYER = 85
    &Spell::EffectNULL,                                     //SPELL_EFFECT_ACTIVATE_OBJECT = 86
    &Spell::EffectSummonTotem,                              //SPELL_EFFECT_SUMMON_TOTEM_SLOT1 = 87
    &Spell::EffectSummonTotem,                              //SPELL_EFFECT_SUMMON_TOTEM_SLOT2 = 88
    &Spell::EffectSummonTotem,                              //SPELL_EFFECT_SUMMON_TOTEM_SLOT3 = 89
    &Spell::EffectSummonTotem,                              //SPELL_EFFECT_SUMMON_TOTEM_SLOT4 = 90
    &Spell::EffectNULL,                                     //SPELL_EFFECT_THREAT_ALL = 91
    &Spell::EffectEnchantHeldItem,                          //SPELL_EFFECT_ENCHANT_HELD_ITEM = 92
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_PHANTASM = 93
    &Spell::EffectSelfResurrect,                            //SPELL_EFFECT_SELF_RESURRECT = 94
    &Spell::EffectSkinning,                                 //SPELL_EFFECT_SKINNING = 95
    &Spell::EffectCharge,                                   //SPELL_EFFECT_CHARGE = 96
    &Spell::EffectSummonCritter,                            //SPELL_EFFECT_SUMMON_CRITTER = 97
    &Spell::EffectKnockBack,                                //SPELL_EFFECT_KNOCK_BACK = 98
    &Spell::EffectDisEnchant,                               //SPELL_EFFECT_DISENCHANT = 99
    &Spell::EffectInebriate,                                //SPELL_EFFECT_INEBRIATE = 100
    &Spell::EffectFeedPet,                                  //SPELL_EFFECT_FEED_PET = 101
    &Spell::EffectDismissPet,                               //SPELL_EFFECT_DISMISS_PET = 102
    &Spell::EffectReputation,                               //SPELL_EFFECT_REPUTATION = 103
    &Spell::EffectSummonObject,                             //SPELL_EFFECT_SUMMON_OBJECT_SLOT1 = 104
    &Spell::EffectSummonObject,                             //SPELL_EFFECT_SUMMON_OBJECT_SLOT2 = 105
    &Spell::EffectSummonObject,                             //SPELL_EFFECT_SUMMON_OBJECT_SLOT3 = 106
    &Spell::EffectSummonObject,                             //SPELL_EFFECT_SUMMON_OBJECT_SLOT4 = 107
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DISPEL_MECHANIC = 108
    &Spell::EffectSummonDeadPet,                            //SPELL_EFFECT_SUMMON_DEAD_PET = 109
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DESTROY_ALL_TOTEMS = 110
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DURABILITY_DAMAGE = 111
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_DEMON = 112
    &Spell::EffectResurrectNew,                             //SPELL_EFFECT_RESURRECT_NEW = 113
    &Spell::EffectAttackMe,                                 //SPELL_EFFECT_ATTACK_ME = 114
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DURABILITY_DAMAGE_PCT = 115
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SKIN_PLAYER_CORPSE = 116
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SPIRIT_HEAL = 117
    &Spell::EffectSkill,                                    //SPELL_EFFECT_SKILL = 118 -- professions and more
    &Spell::EffectNULL,                                     //SPELL_EFFECT_APPLY_AURA_NEW = 119
    &Spell::EffectNULL,                                     //SPELL_EFFECT_TELEPORT_GRAVEYARD = 120
    &Spell::EffectWeaponDmg                                 //SPELL_EFFECT_NORMALIZED_WEAPON_DMG = 121
};

void Spell::EffectNULL(uint32 i)
{
    sLog.outDebug("WORLD: Spell Effect DUMMY");
}

void Spell::EffectResurrectNew(uint32 i)
{
    if(!unitTarget) return;
    if(unitTarget->GetTypeId() != TYPEID_PLAYER) return;
    if(unitTarget->isAlive()) return;
    if(!unitTarget->IsInWorld()) return;

    uint32 health = m_spellInfo->EffectBasePoints[i]+1;
    uint32 mana = m_spellInfo->EffectMiscValue[i];
    ((Player*)unitTarget)->setResurrect(m_caster->GetGUID(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
    SendResurrectRequest((Player*)unitTarget);
}

void Spell::EffectInstaKill(uint32 i)
{
    if( unitTarget && unitTarget->isAlive() )
    {
        uint32 health = unitTarget->GetHealth();
        m_caster->DealDamage(unitTarget, health, DIRECT_DAMAGE, 0, false);
    }
}

void Spell::EffectSchoolDMG(uint32 i)
{
    if( unitTarget && unitTarget->isAlive() )
        m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, damage);

    if (m_caster->GetTypeId()==TYPEID_PLAYER && m_spellInfo->Attributes == 0x150010)
        m_caster->AttackStop();
}

void Spell::EffectDummy(uint32 i)
{
    if(!unitTarget)
        return;

    // More spell specific code in begining
    if( m_spellInfo->Id == SPELL_ID_AGGRO )
    {
        if( !unitTarget || !m_caster || !m_caster->getVictim() )
            return;

        // only creature to creature
        if( unitTarget->GetTypeId() != TYPEID_UNIT || m_caster->GetTypeId() != TYPEID_UNIT )
            return;

        // skip non hostile to caster enemy creatures
        if( !((Creature*)unitTarget)->IsHostileTo(m_caster->getVictim()) )
            return;

        // only from same creature faction
        if(unitTarget->getFaction() != m_caster->getFaction() )
            return;

        // only if enimy is player or pet.
        if( m_caster->getVictim()->GetTypeId() == TYPEID_PLAYER || ((Creature*)m_caster->getVictim())->isPet() )
        {
            // and finally if creature not figthing currently
            if( !unitTarget->isInCombat() )
            {
                ((Creature*)m_caster)->SetNoCallAssistence(true);
                ((Creature*)unitTarget)->SetNoCallAssistence(true);
                ((Creature*)unitTarget)->AI().AttackStart(m_caster->getVictim());
            }
        }
        return;
    }

    // Preparation Rogue - immediately finishes the cooldown on other Rogue abilities
    if(m_spellInfo->Id == 14185)
    {
        if(m_caster->GetTypeId()!=TYPEID_PLAYER)
            return;

        const PlayerSpellMap& sp_list = ((Player *)m_caster)->GetSpellMap();
        for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
        {
            uint32 classspell = itr->first;
            SpellEntry const *spellInfo = sSpellStore.LookupEntry(classspell);

            if (spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE && spellInfo->Id != 14185 &&
                (spellInfo->RecoveryTime > 0 || spellInfo->CategoryRecoveryTime > 0))
            {
                ((Player*)m_caster)->RemoveSpellCooldown(classspell);

                WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8+4));
                data << classspell;
                data << m_caster->GetGUID();
                data << uint32(0);
                ((Player*)m_caster)->GetSession()->SendPacket(&data);
            }
        }
        return;
    }

    // Cold Snap - immediately finishes the cooldown on Frost spells
    if(m_spellInfo->Id == 12472)
    {
        if(m_caster->GetTypeId()!=TYPEID_PLAYER)
            return;

        const PlayerSpellMap& sp_list = ((Player *)m_caster)->GetSpellMap();
        for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
        {
            if (itr->second->state == PLAYERSPELL_REMOVED) continue;
            uint32 classspell = itr->first;
            SpellEntry const *spellInfo = sSpellStore.LookupEntry(classspell);

            if (spellInfo->SpellFamilyName == SPELLFAMILY_MAGE && spellInfo->School == SPELL_SCHOOL_FROST && spellInfo->Id != 12472 &&
                (spellInfo->RecoveryTime > 0 || spellInfo->CategoryRecoveryTime > 0))
            {
                ((Player*)m_caster)->RemoveSpellCooldown(classspell);

                WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8+4));
                data << classspell;
                data << m_caster->GetGUID();
                data << uint32(0);
                ((Player*)m_caster)->GetSession()->SendPacket(&data);
            }
        }
        return;
    }

    // If spell cannibalize and his casted, check special requirements and cast aura Cannibalize is all ok
    if(m_spellInfo->Id == 20577)
    {
        // non-standard cast requirement check
        SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex);
        float max_range = GetMaxRange(srange);
        bool found=false;
        std::list<Unit*> UnitList;
        MapManager::Instance().GetMap(m_caster->GetMapId())->GetUnitList(m_caster->GetPositionX(), m_caster->GetPositionY(),UnitList);
        for(std::list<Unit*>::iterator iter=UnitList.begin();iter!=UnitList.end();iter++)
        {
            if( m_caster->IsFriendlyTo(*iter) || (*iter)->isAlive() || (*iter)->isInFlight() ||
                (((Creature*)(*iter))->GetCreatureInfo()->type != CREATURE_TYPE_HUMANOID && ((Creature*)(*iter))->GetCreatureInfo()->type != CREATURE_TYPE_UNDEAD))
                continue;

            if(m_caster->IsWithinDist(*iter, max_range) )
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            // clear cooldown at fail
            if(m_caster->GetTypeId()==TYPEID_PLAYER)
            {
                ((Player*)m_caster)->RemoveSpellCooldown(20577);

                WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8+4));
                data << uint32(20577);                      // spell id
                data << m_caster->GetGUID();
                data << uint32(0);
                ((Player*)m_caster)->GetSession()->SendPacket(&data);
            }

            SendCastResult(CAST_FAIL_NO_NEARBY_CORPSES_TO_EAT);
            return;
        }

        // ok, main function spell can be casted

        finish();                                           // prepere to replacing this cpell cast to main function spell

        // casting
        SpellEntry const *spellInfo = sSpellStore.LookupEntry( 20578 );
        Spell *spell = new Spell(m_caster, spellInfo, false, 0);
        if(!spellInfo)
        {
            sLog.outError("WORLD: unknown spell id %i\n", 20578);
            return;
        }

        SpellCastTargets targets;
        targets.setUnitTarget(m_caster);
        spell->prepare(&targets);
        return;
    }

    // More generic code later

    // starshards/curse of agony hack .. this applies to 1.10 only
    if (m_triggeredByAura)
    {
        SpellEntry const *trig_info = m_triggeredByAura->GetSpellProto();
        if ((trig_info->SpellIconID == 1485 && trig_info->SpellFamilyName == SPELLFAMILY_PRIEST) ||
            (trig_info->SpellIconID == 544 && trig_info->SpellFamilyName == SPELLFAMILY_WARLOCK))
        {
            Unit *tmpTarget = unitTarget;
            unitTarget = m_triggeredByAura->GetTarget();
            damage = trig_info->EffectBasePoints[i]+1;
            EffectSchoolDMG(i);
            unitTarget = tmpTarget;
        }
    }

    //Holy Shock For Paladins
    if (m_spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && m_spellInfo->SpellIconID == 156)
    {
        int hurt = 0;
        int heal = 0;

        switch(m_spellInfo->Id)
        {
            case 20473:
                hurt = 25912;
                heal = 25914;
                break;
            case 20929:
                hurt = 25911;
                heal = 25913;
                break;
            case 20930:
                hurt = 25902;
                heal = 25903;
                break;
            default:
                break;
        }

        if(m_caster->IsFriendlyTo(unitTarget))
            m_caster->CastSpell(unitTarget, heal, true, 0);
        else
            m_caster->CastSpell(unitTarget, hurt, true, 0);
    }

    if(m_spellInfo->SpellIconID == 1648)
    {
        uint32 dmg = damage;
        dmg += uint32(m_caster->GetPower(POWER_RAGE)/10 * objmgr.GetSpellRank(m_spellInfo->Id)*3);
        SpellEntry const *tspellInfo = sSpellStore.LookupEntry(20647);
        SpellEntry sInfo = *tspellInfo;
        sInfo.EffectBasePoints[0] = dmg;
        m_caster->CastSpell(unitTarget, &sInfo, true, 0);
        m_caster->SetPower(POWER_RAGE,0);
    }

    //Life Tap
    if(m_spellInfo->SpellVisual == 1225 && m_spellInfo->SpellIconID == 208)
    {
        int32 mod = m_spellInfo->EffectBasePoints[0]+1;
        if(m_caster)
        {
            if(m_caster->GetHealth()>mod)
            {
                m_caster->ModifyHealth(-mod);
                m_caster->ModifyPower(POWER_MANA,mod);
            }
        }
    }
}

void Spell::EffectTriggerSpell(uint32 i)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry( m_spellInfo->EffectTriggerSpell[i] );

    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i\n", m_spellInfo->EffectTriggerSpell[i]);
        return;
    }

    m_TriggerSpell = spellInfo;
}

void Spell::EffectTeleportUnits(uint32 i)
{
    if(!unitTarget)
        return;
    HandleTeleport(m_spellInfo->Id,unitTarget);
}

void Spell::EffectApplyAura(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    //If m_immuneToState type contain this aura type, IMMUNE aura.
    if(unitTarget->IsImmunedToSpellEffect(m_spellInfo->EffectApplyAuraName[i]))
    {
        SendCastResult(CAST_FAIL_IMMUNE);
        return;
    }

    sLog.outDebug("Spell: Aura is: %u", m_spellInfo->EffectApplyAuraName[i]);

    Aura* Aur = new Aura(m_spellInfo, i, unitTarget,m_caster, m_CastItem);

    if (!Aur->IsPositive() && Aur->GetCasterGUID() != Aur->GetTarget()->GetGUID())
    {
        switch (Aur->GetModifier()->m_auraname)
        {
            case SPELL_AURA_MOD_DETECT_RANGE:
            case SPELL_AURA_AURAS_VISIBLE:
            case SPELL_AURA_MOD_CHARM:
                break;
            default:
                if(Aur->GetTarget()->GetTypeId() == TYPEID_UNIT)
                    ((Creature*)Aur->GetTarget())->AI().AttackStart(m_caster);
                else
                {
                    m_caster->Attack(Aur->GetTarget());
                    m_caster->SetInCombat();
                    Aur->GetTarget()->SetInCombat();
                }
        }
    }

    bool added = unitTarget->AddAura(Aur);

    if (added)
    {
        // Check for Power Word: Shield
        // TODO Make a way so it works for every related spell!
        if(unitTarget->GetTypeId()==TYPEID_PLAYER)          // Negative buff should only be applied on players
        {
            // This should cover all Power Word: Shield spells
            if ((m_spellInfo->SpellVisual == 784) && (m_spellInfo->SpellIconID == 566))
            {
                                                            // Weakened Soul
                SpellEntry const *WeakenedSoulSpellInfo = sSpellStore.LookupEntry( 6788 );
                Aura* WeakenedSoulAura = new Aura(WeakenedSoulSpellInfo, 0, unitTarget,m_caster, 0);
                unitTarget->AddAura(WeakenedSoulAura, 0);
                sLog.outDebug("Spell: Additinal Aura is: %u", WeakenedSoulSpellInfo->EffectApplyAuraName[i]);
            }
        }

        if(Aur->IsTrigger())
        {
            // arcane missiles
            SpellEntry const *spellInfo = sSpellStore.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
            if (!spellInfo) return;
            if (spellInfo->EffectImplicitTargetA[0] == TARGET_SINGLE_ENEMY && m_caster->GetTypeId() == TYPEID_PLAYER)
            {
                Unit *target = ObjectAccessor::Instance().GetUnit(*m_caster, ((Player*)m_caster)->GetSelection());
                if (target)
                {
                    if (!m_caster->IsFriendlyTo(target))
                        Aur->SetTarget(target);
                    else
                        cancel();
                }
                else
                    cancel();
            }
        }
    }
}

void Spell::EffectManaDrain(uint32 i)
{
    if(m_spellInfo->EffectMiscValue[i] > 4)
        return;

    Powers drain_power = Powers(m_spellInfo->EffectMiscValue[i]);

    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;
    if(unitTarget->getPowerType() != drain_power)
        return;

    uint32 curPower = unitTarget->GetPower(drain_power);
    float tmpvalue = m_spellInfo->EffectMultipleValue[i];
    if(!tmpvalue)
        tmpvalue = 1;

    int32 new_damage;
    if(curPower < damage)
        new_damage = curPower;
    else
        new_damage = damage;

    unitTarget->ModifyPower(drain_power,-new_damage);

    if(drain_power == POWER_MANA)
        m_caster->ModifyPower(POWER_MANA,uint32(new_damage*tmpvalue));
}

void Spell::EffectSendEvent(uint32 i)
{
    sWorld.ScriptsStart(sSpellScripts, m_spellInfo->Id, m_caster, focusObject);
}

void Spell::EffectPowerDrain(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;
    if(unitTarget->getPowerType()!=POWER_MANA)
        return;

    uint32 curPower = unitTarget->GetPower(POWER_MANA);

    int32 new_damage;
    if(curPower < damage)
        new_damage = curPower;
    else
        new_damage = damage;

    int32 tmpvalue = int32(new_damage*m_spellInfo->EffectMultipleValue[i]);

    unitTarget->ModifyPower(POWER_MANA,-new_damage);

    m_caster->ModifyPower(POWER_MANA,tmpvalue);
    m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, new_damage/2);

}

void Spell::EffectHeal( uint32 i )
{
    if( unitTarget && unitTarget->isAlive() )
    {
        int32 addhealth = m_caster->SpellHealingBonus(m_spellInfo, damage);
        bool crit = m_caster->SpellCriticalBonus(m_spellInfo, &addhealth);
        if(unitTarget->GetTypeId() == TYPEID_PLAYER)
            SendHealSpellOnPlayer(((Player*)unitTarget), m_spellInfo->Id, addhealth, crit);

        unitTarget->ModifyHealth( addhealth );
    }
}

void Spell::EffectHealthLeach(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    sLog.outDebug("HealthLeach :%u", damage);

    uint32 curHealth = unitTarget->GetHealth();

    int32 new_damage;
    if(curHealth < damage)
        new_damage = curHealth;
    else
        new_damage = damage;

    int32 tmpvalue = int32(new_damage*m_spellInfo->EffectMultipleValue[i]);

    m_caster->ModifyHealth(tmpvalue);

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        SendHealSpellOnPlayer(((Player*)unitTarget), m_spellInfo->Id, uint32(tmpvalue));

    m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, new_damage);
}

void Spell::DoCreateItem(uint32 i, uint32 itemtype)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = (Player*)m_caster;

    uint32 newitemid = itemtype;
    ItemPrototype const *pProto = objmgr.GetItemPrototype( newitemid );
    if(!pProto)
    {
        player->SendEquipError( EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
        return;
    }

    uint32 num_to_add;

    if(pProto->Class != ITEM_CLASS_CONSUMABLE || m_spellInfo->SpellFamilyName != SPELLFAMILY_MAGE)
        num_to_add = max(m_spellInfo->EffectBasePoints[i]+1, 1);
    else if(player->getLevel() >= m_spellInfo->spellLevel)
        num_to_add = ((player->getLevel() - (m_spellInfo->spellLevel-1))*2);
    else
        num_to_add = 2;

    uint16 dest;
    uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, newitemid, num_to_add, false);
    if( msg != EQUIP_ERR_OK )
    {
        player->SendEquipError( msg, NULL, NULL );
        return;
    }

    Item *pItem = player->StoreNewItem( dest, newitemid, num_to_add, true);

    if(!pItem)
    {
        player->SendEquipError( EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
        return;
    }

    if( pItem->GetProto()->Class != ITEM_CLASS_CONSUMABLE )
        pItem->SetUInt32Value(ITEM_FIELD_CREATOR,player->GetGUIDLow());

    //should send message "create item" to client.-FIX ME
    player->UpdateCraftSkill(m_spellInfo->Id);
}

void Spell::EffectCreateItem(uint32 i)
{
    DoCreateItem(i,m_spellInfo->EffectItemType[i]);
}

void Spell::EffectPersistentAA(uint32 i)
{
    float radius = GetRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
    int32 duration = GetDuration(m_spellInfo);
    DynamicObject* dynObj = new DynamicObject();
    if(!dynObj->Create(objmgr.GenerateLowGuid(HIGHGUID_DYNAMICOBJECT), m_caster, m_spellInfo->Id, i, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, duration, radius))
    {
        delete dynObj;
        return;
    }
    dynObj->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    dynObj->SetUInt32Value(GAMEOBJECT_DISPLAYID, 368003);
    dynObj->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x01eeeeee);
    m_caster->AddDynObject(dynObj);
    dynObj->AddToWorld();
    MapManager::Instance().GetMap(dynObj->GetMapId())->Add(dynObj);

}

void Spell::EffectEnergize(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    if(m_spellInfo->EffectMiscValue[i] > 4)
        return;

    Powers power = Powers(m_spellInfo->EffectMiscValue[i]);

    if(unitTarget->getPowerType() != power)
        return;

    unitTarget->ModifyPower(power,damage);
}

void Spell::EffectOpenLock(uint32 i)
{

    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
    {
        sLog.outDebug( "WORLD: Open Lock - No Player Caster!");
        return;
    }

    Player* player = (Player*)m_caster;

    LootType loottype = LOOT_CORPSE;
    uint32 lockId = 0;
    uint64 guid = 0;

    // Get lockId
    if(gameObjTarget)
    {
        lockId = gameObjTarget->GetGOInfo()->sound0;
        guid = gameObjTarget->GetGUID();
    }
    else if(itemTarget)
    {
        lockId = itemTarget->GetProto()->LockID;
        guid = itemTarget->GetGUID();
    }
    else
    {
        sLog.outDebug( "WORLD: Open Lock - No GameObject/Item Target!");
        return;
    }

    // Get LockInfo
    LockEntry const *lockInfo = sLockStore.LookupEntry(lockId);

    if (!lockInfo)
    {
        sLog.outError( "Spell::EffectOpenLock: %s [guid = %u] has an unknown lockId: %u!",
            (gameObjTarget ? "gameobject" : "item"), GUID_LOPART(guid), lockId);
        SendCastResult(CAST_FAIL_INVALID_TARGET);
        return;
    }

    uint32 SkillId = 0;
    // Check and skill-up skill
    if(m_spellInfo->Effect[1]==SPELL_EFFECT_SKILL)
        SkillId = m_spellInfo->EffectMiscValue[1];
    else
    if(m_spellInfo->EffectMiscValue[0]==1)              // picklocking spells
        SkillId = SKILL_LOCKPICKING;

    // skill bonus provided by casting spell (mostly item spells)
    uint32 spellSkillBonus = uint32(m_spellInfo->EffectBasePoints[0]+1);

    uint32 reqSkillValue = lockInfo->requiredskill;

    if(lockInfo->requiredlockskill)                         // required pick lock skill applying
    {
        if(SkillId != SKILL_LOCKPICKING)                    // wrong skill (cheating?)
        {
            SendCastResult(CAST_FAIL_FIZZLED);
            return;
        }

        reqSkillValue = lockInfo->requiredlockskill;
    }
    else
    if(SkillId == SKILL_LOCKPICKING)                    // apply picklock skill to wrong target
    {
        SendCastResult(CAST_FAIL_INVALID_TARGET);
        return;
    }

    if ( SkillId )
    {
        loottype = LOOT_SKINNING;
        if ( player->GetSkillValue(SkillId) + spellSkillBonus < reqSkillValue )
        {
            SendCastResult(CAST_FAIL_SKILL_NOT_HIGH_ENOUGH);
            return;
        }

        // update skill if really known
        uint32 SkillValue = player->GetPureSkillValue(SkillId);
        if(SkillValue)                                      // non only item base skill
        {
            if(gameObjTarget)
            {
                // Allow one skill-up until respawned
                if ( !gameObjTarget->IsInSkillupList( player->GetGUIDLow() ) &&
                    player->UpdateGatherSkill(SkillId, SkillValue, reqSkillValue) )
                    gameObjTarget->AddToSkillupList( player->GetGUIDLow() );
            }
            else if(itemTarget)
            {
                // Do one skill-up
                uint32 SkillValue = player->GetPureSkillValue(SkillId);
                player->UpdateGatherSkill(SkillId, SkillValue, reqSkillValue);
            }
        }
    }

    // Send loot
    player->SendLoot(guid,loottype);
}

void Spell::EffectSummonChangeItem(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *player = (Player*)m_caster;

    // applied only to using item
    if(!m_CastItem)
        return;

    // ... only to item in own inventory/bank/equip_slot
    if(m_CastItem->GetOwnerGUID()!=player->GetGUID())
        return;

    uint32 newitemid = m_spellInfo->EffectItemType[i];
    if(!newitemid)
        return;

    uint16 pos = (m_CastItem->GetBagSlot() << 8) | m_CastItem->GetSlot();

    Item *pNewItem = player->CreateItem( newitemid, 1 );
    if( !pNewItem )
        return;

    uint16 dest;
    uint8 msg;

    if( player->IsInventoryPos( pos ) )
    {
        msg = player->CanStoreItem( m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), dest, pNewItem, true );
        if( msg == EQUIP_ERR_OK )
        {
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(),true);
            player->StoreItem( dest, pNewItem, true);
            return;
        }
    }
    else if( player->IsBankPos ( pos ) )
    {
        msg = player->CanBankItem( m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), dest, pNewItem, true );
        if( msg == EQUIP_ERR_OK )
        {
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(),true);
            player->BankItem( dest, pNewItem, true);
            return;
        }
    }
    else if( player->IsEquipmentPos ( pos ) )
    {
        msg = player->CanEquipItem( m_CastItem->GetSlot(), dest, pNewItem, true );
        if( msg == EQUIP_ERR_OK )
        {
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(),true);
            player->EquipItem( dest, pNewItem, true);
            return;
        }
    }

    // fail
    delete pNewItem;
}

void Spell::EffectOpenSecretSafe(uint32 i)
{
    EffectOpenLock(i);                                      //no difference for now
}

void Spell::EffectProficiency(uint32 i)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    Player *p_target = (Player*)unitTarget;

    uint32 subClassMask = m_spellInfo->EquippedItemSubClassMask;
    if(m_spellInfo->EquippedItemClass == 2 && !(p_target->GetWeaponProficiency() & subClassMask))
    {
        p_target->AddWeaponProficiency(subClassMask);
        p_target->SendProficiency(uint8(0x02),p_target->GetWeaponProficiency());
    }
    if(m_spellInfo->EquippedItemClass == 4 && !(p_target->GetArmorProficiency() & subClassMask))
    {
        p_target->AddArmorProficiency(subClassMask);
        p_target->SendProficiency(uint8(0x04),p_target->GetArmorProficiency());
    }
}

void Spell::EffectApplyAA(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    AreaAura* Aur = new AreaAura(m_spellInfo, i, unitTarget, m_caster);
    unitTarget->AddAura(Aur);
}

void Spell::EffectSummon(uint32 i)
{
    if(m_caster->GetPetGUID())
        return;

    if(!unitTarget)
        return;
    uint32 pet_entry = m_spellInfo->EffectMiscValue[i];
    if(!pet_entry)
        return;
    uint32 level = m_caster->getLevel();
    Pet* spawnCreature = new Pet(SUMMON_PET);

    if(!spawnCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),
        m_caster->GetMapId(),
        m_caster->GetPositionX(),m_caster->GetPositionY(),
        m_caster->GetPositionZ(),m_caster->GetOrientation(),
        m_spellInfo->EffectMiscValue[i]))
    {
        sLog.outError("no such creature entry %u",m_spellInfo->EffectMiscValue[i]);
        delete spawnCreature;
        return;
    }

    spawnCreature->SetUInt64Value(UNIT_FIELD_SUMMONEDBY,m_caster->GetGUID());
    spawnCreature->SetPower(   POWER_MANA,28 + 10 * level);
    spawnCreature->SetMaxPower(POWER_MANA,28 + 10 * level);
    spawnCreature->SetUInt32Value(UNIT_NPC_FLAGS , 0);
    spawnCreature->setPowerType(POWER_MANA);
    spawnCreature->SetHealth( 28 + 30*level);
    spawnCreature->SetMaxHealth( 28 + 30*level);
    spawnCreature->SetLevel( level);
    spawnCreature->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->getFaction());
    spawnCreature->SetUInt32Value(UNIT_FIELD_FLAGS,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
    spawnCreature->SetUInt64Value(UNIT_FIELD_CREATEDBY, m_caster->GetGUID());
    /*
    spawnCreature->SetStat(STAT_STRENGTH,int(20+level*1.55));
    spawnCreature->SetStat(STAT_AGILITY,int(20+level*0.64));
    spawnCreature->SetStat(STAT_STAMINA,int(20+level*1.27));
    spawnCreature->SetStat(STAT_INTELLECT,int(20+level*0.18));
    spawnCreature->SetStat(STAT_SPIRIT,int(20+level*0.36));
    */
    spawnCreature->SetArmor(level*50);
    spawnCreature->AIM_Initialize();

    std::string name;
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        name = ((Player*)m_caster)->GetName();
    else
        name = ((Creature*)m_caster)->GetCreatureInfo()->Name;
    name.append(petTypeSuffix[spawnCreature->getPetType()]);
    spawnCreature->SetName( name );

    spawnCreature->AddToWorld();
    MapManager::Instance().GetMap(m_caster->GetMapId())->Add((Creature*)spawnCreature);

    if(m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        m_caster->SetPet(spawnCreature);
        spawnCreature->SavePetToDB(PET_SAVE_AS_CURRENT);
        ((Player*)m_caster)->PetSpellInitialize();
    }
}

void Spell::EffectLearnSpell(uint32 i)
{
    if(!unitTarget)
        return;

    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            Creature *pet = m_caster->GetPet();

            if( !pet || !pet->isPet() || pet!=unitTarget || !pet->isAlive() )
                return;

            EffectLearnPetSpell(i);
        }
        return;
    }

    Player *player = (Player*)unitTarget;

    uint32 spellToLearn = m_spellInfo->EffectTriggerSpell[i];
    //data.Initialize(SMSG_LEARNED_SPELL);
    //data << spellToLearn;
    //player->GetSession()->SendPacket(&data);
    player->learnSpell((uint16)spellToLearn);
    //some addspell isn't needed if you have a good DB,FISHING && MINING && HERBALISM have to be needed.
    switch(spellToLearn)
    {
        case 4036:                                          //SKILL_ENGINERING
        {
            player->learnSpell(3918);
            player->learnSpell(3919);
            player->learnSpell(3920);
            break;
        }
        case 3908:                                          //SKILL_TAILORING
        {
            player->learnSpell(2387);
            player->learnSpell(2963);
            break;
        }
        case 7411:                                          //SKILL_ENCHANTING
        {
            player->learnSpell(7418);
            player->learnSpell(7421);
            player->learnSpell(13262);
            break;
        }
        case 2259:                                          //SKILL_ALCHEMY
        {
            player->learnSpell(2329);
            player->learnSpell(7183);
            player->learnSpell(2330);
            break;
        }
        case 2018:                                          //SKILL_BLACKSMITHING
        {
            player->learnSpell(2663);
            player->learnSpell(12260);
            player->learnSpell(2660);
            player->learnSpell(3115);
            break;
        }
        case 2108:                                          //SKILL_LEATHERWORKING
        {
            player->learnSpell(2152);
            player->learnSpell(9058);
            player->learnSpell(9059);
            player->learnSpell(2149);
            player->learnSpell(7126);
            player->learnSpell(2881);
            break;
        }
        case 2550:                                          //SKILL_COOKING
        {
            player->learnSpell(818);
            player->learnSpell(2540);
            player->learnSpell(2538);
            player->learnSpell(8604);
            break;
        }
        case 3273:                                          //SKILL_FIRST_AID
        {
            player->learnSpell(3275);
            break;
        }
        case 7620:                                          //SKILL_FISHING
        {
            player->learnSpell(7738);
            break;
        }
        case 2575:                                          //SKILL_MINING
        {
            player->learnSpell(2580);
            player->learnSpell(2656);
            player->learnSpell(2657);
            break;
        }
        case 2366:                                          //SKILL_HERBALISM
        {
            player->learnSpell(2383);
            break;
        }
        case 264:                                           //SKILL_BOWS
        {
            if(!player->HasSpell(75))
                player->learnSpell(2480);
            break;
        }
        case 266:                                           //SKILL_GUNS
        {
            if(!player->HasSpell(75))
                player->learnSpell(7918);
            break;
        }
        case 5011:                                          //SKILL_CROSSBOWS
        {
            if(!player->HasSpell(75))
                player->learnSpell(7919);
            break;
        }
        case 2567:                                          //SKILL_THROWN
        {
            player->learnSpell(2764);
            break;
        }
        case 2842:                                          //SKILL_POISONS
        {
            player->learnSpell(8681);
            break;
        }
        default:break;
    }
    sLog.outDebug( "Spell: Player %u have learned spell %u from NpcGUID=%u", player->GetGUIDLow(), spellToLearn, m_caster->GetGUIDLow() );
}

void Spell::EffectDispel(uint32 i)
{
    m_caster->RemoveFirstAuraByDispel(m_spellInfo->EffectMiscValue[i]);
}

void Spell::EffectDualWield(uint32 i)
{
    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
        ((Player*)unitTarget)->SetCanDualWield(true);
}

void Spell::EffectPickPocket(uint32 i)
{
    if( m_caster->GetTypeId() != TYPEID_PLAYER )
        return;

    //victim must be creature and attackable
    if( !unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT || m_caster->IsFriendlyTo(unitTarget) )
        return;

    //victim have to be alive and humanoid or undead
    if( unitTarget->isAlive() &&
        (((Creature*)unitTarget)->GetCreatureInfo()->type == CREATURE_TYPE_HUMANOID ||
        ((Creature*)unitTarget)->GetCreatureInfo()->type == CREATURE_TYPE_UNDEAD))
    {
        int chance = 10 + m_caster->getLevel() - unitTarget->getLevel();

        if ( ( rand() % 20 ) <= chance )
        {
            //Stealing successfull
            //sLog.outDebug("Sending loot from pickpocket");
            ((Player*)m_caster)->SendLoot(unitTarget->GetGUID(),LOOT_PICKPOKETING);
        }
        else
        {
            //Reveal action + get attack
            m_caster->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
            ((Creature*)unitTarget)->AI().AttackStart(m_caster);
        }
    }
}

void Spell::EffectSummonWild(uint32 i)
{
    if(!unitTarget)
        return;
    uint32 pet_entry = m_spellInfo->EffectMiscValue[i];
    if(!pet_entry)
        return;

    Pet* old_wild = NULL;

    {
        CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
        Cell cell = RedZone::GetZone(p);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();

        PetWithIdCheck u_check(m_caster, pet_entry);
        MaNGOS::UnitSearcher<PetWithIdCheck> checker((Unit*&)old_wild, u_check);
        TypeContainerVisitor<MaNGOS::UnitSearcher<PetWithIdCheck>, TypeMapContainer<AllObjectTypes> > object_checker(checker);
        CellLock<GridReadGuard> cell_lock(cell, p);
        cell_lock->Visit(cell_lock, object_checker, *MapManager::Instance().GetMap(m_caster->GetMapId()));
    }

    if (old_wild)                                           // find old critter, unsummon
    {
        old_wild->Remove(PET_SAVE_AS_DELETED);
        return;
    }
    else                                                    // in another case summon new
    {

        uint32 level = m_caster->getLevel();

        // level of pet summoned using engineering item based at engineering skill level
        if(m_caster->GetTypeId()==TYPEID_PLAYER && m_CastItem)
        {
            ItemPrototype const *proto = m_CastItem->GetProto();
            if(proto && proto->RequiredSkill == SKILL_ENGINERING)
            {
                uint16 skill202 = ((Player*)m_caster)->GetSkillValue(SKILL_ENGINERING);
                if(skill202)
                {
                    level = skill202/5;
                }
            }
        }

        Pet* spawnCreature = new Pet(GUARDIAN_PET);

        if(!spawnCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),
            m_caster->GetMapId(),
            m_caster->GetPositionX(),m_caster->GetPositionY(),
            m_caster->GetPositionZ(),m_caster->GetOrientation(),
            m_spellInfo->EffectMiscValue[i]))
        {
            sLog.outError("no such creature entry %u",m_spellInfo->EffectMiscValue[i]);
            delete spawnCreature;
            return;
        }

        spawnCreature->SetUInt64Value(UNIT_FIELD_SUMMONEDBY,m_caster->GetGUID());
        spawnCreature->setPowerType(POWER_MANA);
        spawnCreature->SetPower(   POWER_MANA,28 + 10 * level);
        spawnCreature->SetMaxPower(POWER_MANA,28 + 10 * level);
        spawnCreature->SetUInt32Value(UNIT_NPC_FLAGS , 0);
        spawnCreature->SetHealth(    28 + 30*level);
        spawnCreature->SetMaxHealth( 28 + 30*level);
        spawnCreature->SetLevel(level);
        spawnCreature->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->getFaction());
        spawnCreature->SetUInt32Value(UNIT_FIELD_FLAGS,0);
        spawnCreature->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
        spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,0);
        spawnCreature->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
        spawnCreature->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
        spawnCreature->SetUInt64Value(UNIT_FIELD_CREATEDBY, m_caster->GetGUID());

        spawnCreature->SetArmor(level*50);
        spawnCreature->AIM_Initialize();

        std::string name;
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            name = ((Player*)m_caster)->GetName();
        else
            name = ((Creature*)m_caster)->GetCreatureInfo()->Name;
        name.append(petTypeSuffix[spawnCreature->getPetType()]);
        spawnCreature->SetName( name );

        spawnCreature->AddToWorld();
        MapManager::Instance().GetMap(m_caster->GetMapId())->Add((Creature*)spawnCreature);
        /*
                guardians and wilds can't be controled
                if(m_caster->GetTypeId() == TYPEID_PLAYER)
                {
                    m_caster->SetPet(spawnCreature);
                    ((Player*)m_caster)->PetSpellInitialize();
                    ((Player*)m_caster)->SavePet();
                }
        */
    }
}

void Spell::EffectTeleUnitsFaceCaster(uint32 i)
{
    if(!unitTarget)
        return;

    if(unitTarget->isInFlight())
        return;

    uint32 mapid = m_caster->GetMapId();
    float dis = GetRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
    float fx = m_caster->GetPositionX() + dis * cos(m_caster->GetOrientation());
    float fy = m_caster->GetPositionY() + dis * sin(m_caster->GetOrientation());
    // teleport a bit above terrainlevel to avoid falling below it
    float fz = MapManager::Instance ().GetMap(mapid)->GetHeight(fx,fy) + 1.5;

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        ((Player*)unitTarget)->TeleportTo(mapid, fx, fy, fz, -m_caster->GetOrientation(), false);
    else
        MapManager::Instance().GetMap(mapid)->CreatureRelocation((Creature*)m_caster, fx, fy, fz, -m_caster->GetOrientation());
}

void Spell::EffectLearnSkill(uint32 i)
{
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 skillid =  m_spellInfo->EffectMiscValue[i];
    uint16 skillval = ((Player*)unitTarget)->GetPureSkillValue(skillid);
    ((Player*)unitTarget)->SetSkill(skillid,skillval?skillval:1,damage*75);
}

void Spell::EffectTradeSkill(uint32 i)
{
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    // uint32 skillid =  m_spellInfo->EffectMiscValue[i];
    // uint16 skillmax = ((Player*)unitTarget)->(skillid);
    // ((Player*)unitTarget)->SetSkill(skillid,skillval?skillval:1,skillmax+75);
}

void Spell::EffectEnchantItemPerm(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    if (!itemTarget)
        return;

    Player* p_caster = (Player*)m_caster;

    p_caster->UpdateCraftSkill(m_spellInfo->Id);

    if (m_spellInfo->EffectMiscValue[i])
    {
        uint32 enchant_id = m_spellInfo->EffectMiscValue[i];

        SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if(!pEnchant)
            return;

        // item can be in trade slot and have owner diff. from caster
        Player* item_owner = itemTarget->GetOwner();
        if(!item_owner)
            return;

        if(item_owner!=p_caster && p_caster->GetSession()->GetSecurity() > 0 && sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
            sLog.outCommand("GM Enchanting: %s (Entry: %d) GM: %s (Account: %u) Player: %s (Account: %u)",
                itemTarget->GetProto()->Name1,itemTarget->GetEntry(),
                p_caster->GetName(),p_caster->GetSession()->GetAccountId(),
                item_owner->GetName(),item_owner->GetSession()->GetAccountId());

        // remove old enchanting before appling new if equiped
        if(itemTarget->IsEquipped())
            if(uint32 old_enchant_id = itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT))
                item_owner->AddItemEnchant(itemTarget,old_enchant_id,false);

        for(int x=0;x<3;x++)
            itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+x,0);

        itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT, enchant_id);

        // add new enchanting if equiped
        if(itemTarget->IsEquipped())
            item_owner->AddItemEnchant(itemTarget,enchant_id,true);

        itemTarget->SetState(ITEM_CHANGED);
    }
}

void Spell::EffectEnchantItemTmp(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = (Player*)m_caster;

    if(!itemTarget)
        return;

    if (m_spellInfo->EffectMiscValue[i])
    {
        uint32 enchant_id = m_spellInfo->EffectMiscValue[i];
        int32 duration = GetDuration(m_spellInfo);
        if(duration == 0)
            duration = m_spellInfo->EffectBasePoints[i]+1;
        if(duration <= 1)
            duration = 300;
        SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if(!pEnchant)
            return;

        // item can be in trade slot and have owner diff. from caster
        Player* item_owner = itemTarget->GetOwner();
        if(!item_owner)
            return;

        if(item_owner!=p_caster && p_caster->GetSession()->GetSecurity() > 0 && sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
            sLog.outCommand("GM Enchanting: %s (Entry: %d) GM: %s (Account: %u) Player: %s (Account: %u)",
                itemTarget->GetProto()->Name1,itemTarget->GetEntry(),
                p_caster->GetName(),p_caster->GetSession()->GetAccountId(),
                item_owner->GetName(),item_owner->GetSession()->GetAccountId());

        // remove old enchanting before applying new if equipped
        if(uint32 old_enchant_id = itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+1*3))
        {
            if(itemTarget->IsEquipped())
                item_owner->AddItemEnchant(itemTarget,old_enchant_id,false);

            // duration == 0 will remove EnchantDuration
            item_owner->AddEnchantDuration(itemTarget,1,0);
        }

        for(int x=0;x<3;x++)
            itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+3+x,0);

        itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+3, enchant_id);
        itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+3+1, duration*1000);
        if(m_spellInfo->SpellFamilyName == 8)
            itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+3+2, 45+objmgr.GetSpellRank(m_spellInfo->Id)*15);

        // add new enchanting if equipped
        if(itemTarget->IsEquipped())
            item_owner->AddItemEnchant(itemTarget,enchant_id,true);

        itemTarget->SetState(ITEM_CHANGED);

        // set duration
        item_owner->AddEnchantDuration(itemTarget,1,duration*1000);
    }
}

void Spell::EffectTameCreature(uint32 i)
{
    if(m_caster->GetPetGUID())
        return;

    if(!unitTarget)
        return;

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        return;

    Creature* creatureTarget = (Creature*)unitTarget;

    if(creatureTarget->isPet())
        return;

    if(m_caster->getClass() == CLASS_HUNTER)
    {
        creatureTarget->AttackStop();
        if(m_caster->getVictim()==creatureTarget)
            m_caster->AttackStop();

        creatureTarget->CombatStop();
        creatureTarget->StopMoving();

        // cast finish succesfully
        SendChannelUpdate(0);
        finish();

        Pet* pet = new Pet(HUNTER_PET);

        pet->CreateBaseAtCreature(creatureTarget);

        ObjectAccessor::Instance().RemoveCreatureCorpseFromPlayerView(creatureTarget);
        creatureTarget->setDeathState(JUST_DIED);

        pet->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, m_caster->GetGUID());
        pet->SetUInt64Value(UNIT_FIELD_CREATEDBY, m_caster->GetGUID());
        pet->SetMaxPower(POWER_HAPPINESS,1000000);
        pet->SetPower(   POWER_HAPPINESS,600000);
        pet->setPowerType(POWER_FOCUS);
        pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->getFaction());
        pet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,0);
        pet->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
        pet->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
        pet->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN1 + UNIT_FLAG_RESTING + UNIT_FLAG_RENAME);
                                                            // this enables popup window (pet detals, abandon, rename)
        pet->SetUInt32Value(UNIT_FIELD_PETNUMBER,1);
                                                            // this enables pet detals window (Shift+P)
        pet->AIM_Initialize();

        pet->AddToWorld();
        MapManager::Instance().GetMap(pet->GetMapId())->Add((Creature*)pet);

        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            m_caster->SetPet(pet);
            pet->SavePetToDB(PET_SAVE_AS_CURRENT);
            ((Player*)m_caster)->PetSpellInitialize();
        }
    }
}

void Spell::EffectSummonPet(uint32 i)
{
    float px, py, pz;
    m_caster->GetClosePoint(NULL, px, py, pz);

    uint32 petentry = m_spellInfo->EffectMiscValue[i];

    Pet *OldSummon = m_caster->GetPet();

    // if pet requested type already exist
    if( OldSummon )
    {

        if(petentry == 0 || OldSummon->GetCreatureInfo()->Entry == petentry)
        {
            // pet in corpse state can't be summoned
            if( OldSummon->isDead() )
                return;

            MapManager::Instance().GetMap(OldSummon->GetMapId())->Remove((Creature*)OldSummon,false);
            OldSummon->SetMapId(m_caster->GetMapId());
            OldSummon->Relocate(px, py, pz, OldSummon->GetOrientation());
            MapManager::Instance().GetMap(m_caster->GetMapId())->Add((Creature*)OldSummon);

            if(m_caster->GetTypeId() == TYPEID_PLAYER && OldSummon->isControlled() )
            {
                ((Player*)m_caster)->PetSpellInitialize();
            }
            return;
        }

        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            ((Player*)m_caster)->RemovePet(OldSummon,PET_SAVE_AS_DELETED);
        else
            return;
    }

    Pet* NewSummon = new Pet(m_caster->getClass() == CLASS_HUNTER ? HUNTER_PET : SUMMON_PET);

    // petentry==0 for hunter "call pet" (current pet summoned if any)
    if(NewSummon->LoadPetFromDB(m_caster,petentry))
    {
        NewSummon->SavePetToDB(PET_SAVE_AS_CURRENT);
        return;
    }

    // not error in case fail hunter call pet
    if(!petentry)
        return;

    CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(petentry);

    if(!cInfo)
    {
        sLog.outError("EffectSummonPet: creature entry %u not found.",petentry);
        return;
    }

    if( NewSummon->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),  m_caster->GetMapId(), px, py, pz+1, m_caster->GetOrientation(), petentry))
    {
        uint32 petlevel=m_caster->getLevel();
        NewSummon->SetLevel(petlevel);
        NewSummon->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, m_caster->GetGUID());
        NewSummon->SetUInt64Value(UNIT_FIELD_CREATEDBY, m_caster->GetGUID());
        NewSummon->SetUInt32Value(UNIT_NPC_FLAGS , 0);
        NewSummon->SetHealth(    28 + 10 * petlevel);
        NewSummon->SetMaxHealth( 28 + 10 * petlevel);
        NewSummon->SetPower(   POWER_MANA , 28 + 10 * petlevel);
        NewSummon->SetMaxPower(POWER_MANA, 28 + 10 * petlevel);
        NewSummon->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->getFaction());
        NewSummon->SetUInt32Value(UNIT_FIELD_BYTES_0,2048);
        NewSummon->SetUInt32Value(UNIT_FIELD_FLAGS,UNIT_FLAG_UNKNOWN1 + UNIT_FLAG_RESTING);
                                                            // this enables popup window (pet detals, abandon)
        NewSummon->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
        NewSummon->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,5);
        NewSummon->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
        NewSummon->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
        NewSummon->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);
        NewSummon->SetUInt32Value(UNIT_FIELD_PETNUMBER,1);  // this enables pet detals window (Shift+P)
        NewSummon->SetStat(STAT_STRENGTH,22);
        NewSummon->SetStat(STAT_AGILITY,22);
        NewSummon->SetStat(STAT_STAMINA,25);
        NewSummon->SetStat(STAT_INTELLECT,28);
        NewSummon->SetStat(STAT_SPIRIT,27);

        for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
            NewSummon->m_spells[i] = 0;

        if(petentry == 416)                                 //imp
        {
            NewSummon->m_spells[0] = 3110;                  //3110---fire bolt 1
            NewSummon->AddActState(STATE_RA_SPELL1);
        }

        // generate new name for warlock pet
        if(m_caster->getClass() == CLASS_WARLOCK)
        {
            std::string new_name=objmgr.GeneratePetName(petentry);
            if(new_name!="")
                NewSummon->SetName(new_name);
        }
        else if(m_caster->getClass() == CLASS_HUNTER && cInfo->type == CREATURE_TYPE_BEAST)
        {
            // this enables popup window (pet detals, abandon, rename)
            NewSummon->SetUInt32Value(UNIT_FIELD_FLAGS,UNIT_FLAG_UNKNOWN1 + UNIT_FLAG_RESTING + UNIT_FLAG_RENAME);
        }

        NewSummon->AIM_Initialize();

        NewSummon->AddToWorld();
        MapManager::Instance().GetMap(NewSummon->GetMapId())->Add((Creature*)NewSummon);

        m_caster->SetPet(NewSummon);
        sLog.outDebug("New Pet has guid %u", NewSummon->GetGUIDLow());

        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            NewSummon->SavePetToDB(PET_SAVE_AS_CURRENT);
            ((Player*)m_caster)->PetSpellInitialize();
        }
    }
    else
        delete NewSummon;
}

void Spell::EffectLearnPetSpell(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)m_caster;

    Pet *pet = _player->GetPet();
    if(!pet)
        return;
    if(!pet->isAlive())
        return;

    SpellEntry const *learn_spellproto = sSpellStore.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
    if(!learn_spellproto)
        return;

    for(int8 x=0;x<4;x++)
    {
        SpellEntry const *has_spellproto = sSpellStore.LookupEntry(pet->m_spells[x]);
        if(!has_spellproto)
        {
            pet->m_spells[x] = learn_spellproto->Id;
            break;
        }
        else if(has_spellproto->SpellIconID == learn_spellproto->SpellIconID)
        {
            pet->m_spells[x] = learn_spellproto->Id;
            break;
        }
    }
    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    _player->PetSpellInitialize();
}

void Spell::EffectAttackMe(uint32 i)
{
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
    {
        unitTarget->SetInFront(m_caster);
        ((Creature*)unitTarget)->AI().AttackStart(m_caster);
    }
}

void Spell::EffectWeaponDmg(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    uint32 wp[4] = { SPELL_EFFECT_WEAPON_DAMAGE, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE, SPELL_EFFECT_NORMALIZED_WEAPON_DMG, SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL };

    // multiple weap dmg effect workaround
    // execute only the first weapon damage
    // and handle all effects at once
    uint8 j,k;
    int32 bonus = 0;

    for (j = 0; j < 3; j++)
    {
        for (k = 0; k < 4; k++)
            if (m_spellInfo->Effect[j] == wp[k])
                break;
        if (k != 4)
        {
            if (j < i)
                return;
            if (m_spellInfo->Effect[j] != SPELL_EFFECT_WEAPON_PERCENT_DAMAGE)
                bonus += m_spellInfo->EffectBasePoints[j]+1;
        }
    }

    WeaponAttackType attType = BASE_ATTACK;
    if(m_spellInfo->rangeIndex != 1 && m_spellInfo->rangeIndex != 2 && m_spellInfo->rangeIndex != 7)
        attType = RANGED_ATTACK;

    uint32 hitInfo = 0;
    uint32 nohitMask = HITINFO_ABSORB | HITINFO_RESIST | HITINFO_MISS;
    uint32 damageType = NORMAL_DAMAGE;
    uint32 victimState = VICTIMSTATE_NORMAL;
    uint32 damage = 0;
    uint32 blocked_dmg = 0;
    uint32 absorbed_dmg = 0;
    uint32 resisted_dmg = 0;
    bool criticalhit = false;

    if( unitTarget->IsImmunedToPhysicalDamage() )
    {
        m_caster->SendAttackStateUpdate (HITINFO_MISS, unitTarget, 1, NORMAL_DAMAGE, 0, 0, 0, VICTIMSTATE_IS_IMMUNE, 0);
        return;
    }

    m_caster->DoAttackDamage(unitTarget, &damage, &blocked_dmg, &damageType, &hitInfo, &victimState, &absorbed_dmg, &resisted_dmg, attType);

    // not add bonus to 0 damage
    if( damage > 0 && damage + bonus > 0 )
        damage += bonus;
    else
        damage = 0;

    for (j = 0; j < 3; j++)
        if (m_spellInfo->Effect[j] == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE)
            damage = uint32(damage * (m_spellInfo->EffectBasePoints[j]+1) / 100);

    if (hitInfo & nohitMask)
        m_caster->SendAttackStateUpdate(hitInfo & nohitMask, unitTarget, 1, m_spellInfo->School, damage, absorbed_dmg, resisted_dmg, 1, blocked_dmg);

    if(hitInfo & HITINFO_CRITICALHIT)
        criticalhit = true;

    m_caster->SendSpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, damage + absorbed_dmg + resisted_dmg + blocked_dmg, m_spellInfo->School, absorbed_dmg, resisted_dmg, false, blocked_dmg, criticalhit);
    m_caster->DealDamage(unitTarget, damage, SPELL_DIRECT_DAMAGE, 0, true);

    // take ammo
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        if(m_spellInfo->rangeIndex != 1 && m_spellInfo->rangeIndex != 2 && m_spellInfo->rangeIndex != 7)
        {
            Item *pItem = ((Player*)m_caster)->GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED );
            if(!pItem  || pItem->IsBroken())
                return;

            if( pItem->GetProto()->InventoryType == INVTYPE_THROWN )
            {
                uint32 count = 1;
                ((Player*)m_caster)->DestroyItemCount( pItem, count, true);
            }
            else
            if(uint32 ammo = ((Player*)m_caster)->GetUInt32Value(PLAYER_AMMO_ID))
                ((Player*)m_caster)->DestroyItemCount( ammo , 1, true);
            // wand not have ammo
        }
    }

    /*if(m_spellInfo->Effect[i] == 121)
    {
        m_caster->resetAttackTimer(BASE_ATTACK);
        m_caster->resetAttackTimer(OFF_ATTACK);
        m_caster->resetAttackTimer(RANGED_ATTACK);
    }*/
}

void Spell::EffectThreat(uint32 i)
{
    if(!unitTarget || !unitTarget->isAlive() || !m_caster->isAlive())
        return;
    unitTarget->AddHostil(m_caster->GetGUID(),float(damage));
}

void Spell::EffectHealMaxHealth(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    uint32 heal = m_caster->GetMaxHealth();

    unitTarget->ModifyHealth(heal);

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        SendHealSpellOnPlayer((Player*)unitTarget, m_spellInfo->Id, heal);
}

void Spell::EffectInterruptCast(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    unitTarget->InterruptSpell();
}

void Spell::EffectSummonObjectWild(uint32 i)
{
    GameObject* pGameObj = new GameObject();

    uint32 gameobject_id = m_spellInfo->EffectMiscValue[i];

    Object* target = focusObject;
    if( !target )
        target = m_caster;

    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), gameobject_id, target->GetMapId(),
        target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
        target->GetOrientation(), 0, 0, 0, 0, 0, 0))
    {
        delete pGameObj;
        return;
    }
    pGameObj->SetRespawnTimer(GetDuration(m_spellInfo));

    m_caster->AddGameObject(pGameObj);
    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);
}

void Spell::EffectScriptEffect(uint32 i)
{
    if(!m_spellInfo->Reagent[0])
    {
        // paladin's holy light / flash of light
        if ((m_spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN) &&
            (m_spellInfo->SpellIconID == 70 || m_spellInfo->SpellIconID  == 242))
            EffectHeal( i );

        if(m_spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && (m_spellInfo->SpellFamilyFlags & (1<<23)))
        {
            // paladin's judgement
            if(!unitTarget || !unitTarget->isAlive())
                return;
            uint32 spellId2 = 0;
            Unit::AuraMap& m_auras = m_caster->GetAuras();
            Unit::AuraMap::iterator itr,next;

            for(itr = m_auras.begin(); itr != m_auras.end(); itr = next)
            {
                next = itr;
                next++;
                if (itr->second)
                {
                    SpellEntry const *spellInfo = itr->second->GetSpellProto();
                    if (!spellInfo) continue;
                    if (spellInfo->SpellVisual != 5622 || spellInfo->SpellFamilyName != SPELLFAMILY_PALADIN) continue;
                    spellId2 = spellInfo->EffectBasePoints[2]+1;
                    if(spellId2 <= 1)
                        continue;
                    m_caster->RemoveAurasDueToSpell(spellInfo->Id);
                    next = m_auras.begin();
                    break;
                }
            }

            SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId2);
            if(!spellInfo)
                return;
            Spell *p_spell = new Spell(m_caster,spellInfo,true,0);

            SpellCastTargets targets;
            Unit *ptarget = unitTarget;
            targets.setUnitTarget(ptarget);
            p_spell->prepare(&targets);

            delete p_spell;                                 // triggered spell not self deleted
        }
    }
    else
    {
        uint32 itemtype;
        switch(m_spellInfo->Id)
        {
            case 6201:
                itemtype = 5512;                            //spell 6261;    //primary healstone
                break;
            case 6202:
                itemtype = 5511;                            //spell 6262;    //inferior healstone
                break;
            case 5699:
                itemtype = 5509;                            //spell 5720;    //healstone
                break;
            case 11729:
                itemtype = 5510;                            //spell 5723;    //strong healstone
                break;
            case 11730:
                itemtype = 9421;                            //spell 11732;    //super healstone
                break;
            default:
                return;
        }
        DoCreateItem( i, itemtype );
    }
}

void Spell::EffectAddComboPoints(uint32 i)
{
    if(!unitTarget)
        return;

    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    uint8 comboPoints = ((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & 0xFF00) >> 8);
    if(m_caster->GetUInt64Value(PLAYER_FIELD_COMBO_TARGET) != unitTarget->GetGUID())
    {
        comboPoints = damage;
        m_caster->SetUInt64Value(PLAYER_FIELD_COMBO_TARGET,unitTarget->GetGUID());
        m_caster->SetUInt32Value(PLAYER_FIELD_BYTES,((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (comboPoints << 8)));
    }
    else if(comboPoints < 5)
    {
        comboPoints += damage;
        if(comboPoints > 5)
            comboPoints = 5;
        m_caster->SetUInt32Value(PLAYER_FIELD_BYTES,((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (comboPoints << 8)));
    }

}

void Spell::EffectDuel(uint32 i)
{
    if(!m_caster || !unitTarget || m_caster->GetTypeId() != TYPEID_PLAYER || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *caster = (Player*)m_caster;
    Player *target = (Player*)unitTarget;

    // caster or target already have requested duel
    if( caster->duel || target->duel || target->HasInIgnoreList(caster->GetGUID()) )
        return;

    //CREATE DUEL FLAG OBJECT
    GameObject* pGameObj = new GameObject();

    uint32 gameobject_id = m_spellInfo->EffectMiscValue[i];

    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), gameobject_id,m_caster->GetMapId(),
        m_caster->GetPositionX()+(unitTarget->GetPositionX()-m_caster->GetPositionX())/2 ,
        m_caster->GetPositionY()+(unitTarget->GetPositionY()-m_caster->GetPositionY())/2 ,
        m_caster->GetPositionZ(),
        m_caster->GetOrientation(), 0, 0, 0, 0, 0, 0))
    {
        delete pGameObj;
        return;
    }
    pGameObj->SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);

    pGameObj->SetUInt32Value(GAMEOBJECT_DISPLAYID, 787 );
    pGameObj->SetUInt32Value(GAMEOBJECT_FACTION, m_caster->getFaction() );
    pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, 16 );
    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel()+1 );
    pGameObj->SetRespawnTimer(GetDuration(m_spellInfo));
    pGameObj->SetSpellId(m_spellInfo->Id);

    m_caster->AddGameObject(pGameObj);
    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);
    //END

    //Send request
    WorldPacket data(SMSG_DUEL_REQUESTED, 16);
    data << pGameObj->GetGUID();
    data << caster->GetGUID();
    caster->GetSession()->SendPacket(&data);
    target->GetSession()->SendPacket(&data);

    // create duel-info
    DuelInfo *duel   = new DuelInfo;
    duel->initiator  = caster;
    duel->opponent   = target;
    duel->startTime  = 0;
    duel->startTimer = 0;
    caster->duel     = duel;

    DuelInfo *duel2   = new DuelInfo;
    duel2->initiator  = caster;
    duel2->opponent   = caster;
    duel2->startTime  = 0;
    duel2->startTimer = 0;
    target->duel      = duel2;

    caster->SetUInt64Value(PLAYER_DUEL_ARBITER,pGameObj->GetGUID());
    target->SetUInt64Value(PLAYER_DUEL_ARBITER,pGameObj->GetGUID());

}

void Spell::EffectSummonTotem(uint32 i)
{
    uint8 slot = 0;
    switch(m_spellInfo->Effect[i])
    {
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT1: slot = 0; break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT2: slot = 1; break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT3: slot = 2; break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT4: slot = 3; break;
        default: return;
    }

    float angle = m_caster->GetOrientation() + M_PI/4 - (slot*M_PI/2);
    float x = m_caster->GetPositionX() + 2 * cos(angle);
    float y = m_caster->GetPositionY() + 2 * sin(angle);
    float z = m_caster->GetPositionZ();

    Map* map = MapManager::Instance().GetMap(m_caster->GetMapId());
    float z2 = map->GetHeight(x,y);
    if( abs( z2 - z ) < 5 )
        z = z2;

    uint64 guid = m_caster->m_TotemSlot[slot];
    if(guid != 0)
    {
        Creature *OldTotem = ObjectAccessor::Instance().GetCreature(*m_caster, guid);
        if(OldTotem && OldTotem->isTotem()) ((Totem*)OldTotem)->UnSummon();
        m_caster->m_TotemSlot[slot] = 0;
    }

    Totem* pTotem = new Totem();

    if(!pTotem->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT),
        m_caster->GetMapId(), x, y, z, m_caster->GetOrientation(),
        m_spellInfo->EffectMiscValue[i] ))
    {
        delete pTotem;
        return;
    }

    m_caster->m_TotemSlot[slot] = pTotem->GetGUID();
    pTotem->SetOwner(m_caster->GetGUID());
    //pTotem->SetSpell(pTotem->GetCreatureInfo()->spell1);
    pTotem->SetSpell(m_spellInfo->Id);                      //use SummonTotem spellid
    pTotem->SetDuration(GetDuration(m_spellInfo));
    pTotem->SetHealth(5);
    pTotem->SetMaxHealth(5);
    pTotem->Summon();
}

void Spell::EffectEnchantHeldItem(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = (Player*)m_caster;

    if(!itemTarget)
        return;

    // must be equipped
    if(!itemTarget->IsEquipped())
        return;

    if (m_spellInfo->EffectMiscValue[i])
    {
        uint32 enchant_id = m_spellInfo->EffectMiscValue[i];
        int32 duration = GetDuration(m_spellInfo);
        if(duration == 0)
            duration = m_spellInfo->EffectBasePoints[i]+1;
        if(duration <= 1)
            duration = 300;
        SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if(!pEnchant)
            return;

        // can be held by another player and accessable to caster in trade slot
        Player* item_owner = itemTarget->GetOwner();
        if(!item_owner)
            return;

        if(item_owner!=p_caster && p_caster->GetSession()->GetSecurity() > 0 && sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
            sLog.outCommand("GM Enchanting: %s (Entry: %d) GM: %s (Account: %u) Player: %s (Account: %u)",
                itemTarget->GetProto()->Name1,itemTarget->GetEntry(),
                p_caster->GetName(),p_caster->GetSession()->GetAccountId(),
                item_owner->GetName(),item_owner->GetSession()->GetAccountId());

        // remove old enchanting before appling new
        if(uint32 old_enchant_id = itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+pEnchant->display_type*3))
            item_owner->AddItemEnchant(itemTarget,old_enchant_id,false);

        for(int x=0;x<3;x++)
            itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+pEnchant->display_type*3+x,0);

        itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+pEnchant->display_type*3, enchant_id);
        itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+pEnchant->display_type*3+1, duration*1000);
        itemTarget->SetState(ITEM_CHANGED);

        // add new enchanting
        item_owner->AddItemEnchant(itemTarget,enchant_id,true);

        // set duration
        item_owner->AddEnchantDuration(itemTarget,1,duration*1000);
    }
}

void Spell::EffectDisEnchant(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = (Player*)m_caster;
    if(!itemTarget)
        return;
    uint32 item_level = itemTarget->GetProto()->ItemLevel;
    uint32 item_quality = itemTarget->GetProto()->Quality;

    uint32 item = 0;
    uint32 count = 0;
    if(item_level >= 66)
    {
        {
            count = 1;
            item = 20725;
        }
    }
    else if(item_level >= 51 && item_level <= 65)
    {
        if(item_quality == 4)
        {
            count = urand(3,5);
            item = 14344;
        }
        else if(item_quality == 3)
        {
            count = 1;
            item = 14344;
        }
        else if(item_quality == 2)
        {
            count = urand(1,(item_level/10));
            if(urand(1,100)< 85)
                item = 16204;
            else
                item = 16203;
        }
    }
    else if(item_level >= 46 && item_level <= 50)
    {
        if(item_quality == 4)
        {
            count = urand(3,5);
            item = 14343;
        }
        else if(item_quality == 3)
        {
            count = 1;
            item = 14343;
        }
        else if(item_quality == 2)
        {
            count = urand(1,(item_level/10));
            if(urand(1,100)< 85)
                item = 11176;
            else
                item = 16202;
        }
    }
    else if(item_level >= 41 && item_level <= 45)
    {
        if(item_quality == 4)
        {
            count = urand(3,5);
            item = 11178;
        }
        else if(item_quality == 3)
        {
            count = 1;
            item = 11178;
        }
        else if(item_quality == 2)
        {
            count = urand(1,(item_level/10));
            if(urand(1,100)< 85)
                item = 11176;
            else
                item = 11175;
        }
    }
    else if(item_level >= 36 && item_level <= 40)
    {
        if(item_quality == 4)
        {
            count = urand(3,5);
            item = 11177;
        }
        else if(item_quality == 3)
        {
            count = 1;
            item = 11177;
        }
        else if(item_quality == 2)
        {
            count = urand(1,(item_level/10));
            if(urand(1,100)< 85)
                item = 11137;
            else
                item = 11174;
        }
    }
    else if(item_level >= 31 && item_level <= 35)
    {
        if(item_quality == 4)
        {
            count = urand(3,5);
            item = 11139;
        }
        else if(item_quality == 3)
        {
            count = 1;
            item = 11139;
        }
        else if(item_quality == 2)
        {
            count = (item_level/10);
            if(urand(1,100)< 85)
                item = 11137;
            else
                item = 11135;
        }
    }
    else if(item_level >= 25 && item_level <= 30)
    {
        if(item_quality == 4)
        {
            count = urand(3,5);
            item = 11138;
        }
        else if(item_quality == 3)
        {
            count = 1;
            item = 11138;
        }
        else if(item_quality == 2)
        {
            count = (item_level/10);
            if(urand(1,100)< 85)
                item = 11083;
            else
                item = 11134;
        }
    }
    else if(item_level >= 21 && item_level <= 25)
    {
        if(item_quality == 4)
        {
            count = urand(3,5);
            item = 11084;
        }
        else if(item_quality == 3)
        {
            count = 1;
            item = 11084;
        }
        else if(item_quality == 2)
        {
            count = (item_level/10);
            if(urand(1,100)< 85)
                item = 11083;
            else
                item = 11082;
        }
    }
    else if(item_level >= 1 && item_level <= 20)
    {
        if(item_quality == 4)
        {
            count = urand(3,5);
            item = 10978;
        }
        else if(item_quality == 3 && item_level >=16)
        {
            count = 1;
            item = 10978;
        }
        else if(item_quality == 2)
        {
            if(urand(1,100)< 70)
            {
                count = urand(1,3);
                item = 10940;
            }
            else if(item_level <=15 && urand(1,100)< 70 )
            {
                count = urand(1,3);
                item = 10938;
            }
            else if(urand(1,100)< 50)
            {
                count = urand(1,3);
                item = 10939;
            }
            else
            {
                count = urand(1,3);
                item = 10998;
            }
        }
    }

    if ( item == 0 || count == 0 )
    {                                                       //Fix crash
        SendCastResult(CAST_FAIL_CANT_BE_DISENCHANTED);
        //SendChannelUpdate(0);
        return;
    }

    uint32 item_count = 1;
    p_caster->DestroyItemCount(itemTarget,item_count, true);
    p_caster->UpdateCraftSkill(m_spellInfo->Id);

    uint16 dest;
    uint8 msg = p_caster->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, item, count, false );
    if( msg == EQUIP_ERR_OK )
        p_caster->StoreNewItem( dest, item, count, true );
    else
        p_caster->SendEquipError( msg, NULL, NULL );
    return ;
}

void Spell::EffectInebriate(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *player = (Player*)m_caster;
    uint16 currentDrunk = player->GetDrunkValue();
    uint16 drunkMod = (m_spellInfo->EffectBasePoints[i]+1) * 0xFFFF / 100;
    if (currentDrunk + drunkMod > 0xFFFF)
        currentDrunk = 0xFFFF;
    else
        currentDrunk += drunkMod;
    player->SetDrunkValue(currentDrunk);
}

void Spell::EffectFeedPet(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)m_caster;

    Creature *pet = _player->GetPet();
    if(!pet)
        return;
    if(!pet->isAlive())
        return;

    pet->ModifyPower(POWER_HAPPINESS,damage);

    SpellEntry const *spellinfo = sSpellStore.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
    Spell _spell(m_caster, spellinfo, true, 0);
    SpellCastTargets targets;
    targets.setUnitTarget(m_caster);
    _spell.prepare(&targets);
}

void Spell::EffectDismissPet(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Pet* pet = m_caster->GetPet();

    // not let dismiss dead pet
    if(!pet||!pet->isAlive())
        return;

    ((Player*)m_caster)->RemovePet(pet,PET_SAVE_AS_CURRENT);
}

void Spell::EffectSummonObject(uint32 i)
{
    uint8 slot = 0;
    switch(m_spellInfo->Effect[i])
    {
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT1: slot = 0; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT2: slot = 1; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT3: slot = 2; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT4: slot = 3; break;
        default: return;
    }

    uint64 guid = m_caster->m_ObjectSlot[slot];
    if(guid != 0)
    {
        GameObject* obj = NULL;
        if( m_caster )
            obj = ObjectAccessor::Instance().GetGameObject(*m_caster, guid);

        if(obj) obj->Delete();
        m_caster->m_ObjectSlot[slot] = 0;
    }

    GameObject* pGameObj = new GameObject();
    uint32 display_id = m_spellInfo->EffectMiscValue[i];

    float rot2 = sin(m_caster->GetOrientation()/2);
    float rot3 = cos(m_caster->GetOrientation()/2);

    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), display_id,m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), 0, 0, rot2, rot3, 0, 0))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL,m_caster->getLevel());
    pGameObj->SetRespawnTimer(GetDuration(m_spellInfo));
    pGameObj->SetSpellId(m_spellInfo->Id);
    pGameObj->SetLootState(GO_CLOSED);
    m_caster->AddGameObject(pGameObj);

    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);
    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM, 8);
    data << pGameObj->GetGUID();
    m_caster->SendMessageToSet(&data,true);

    m_caster->m_ObjectSlot[slot] = pGameObj->GetGUID();
}

void Spell::EffectResurrect(uint32 i)
{

    if(!unitTarget)
        return;
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if(unitTarget->isAlive())
        return;
    if(!unitTarget->IsInWorld())
        return;

    Player* pTarget = ((Player*)unitTarget);

    uint32 health = m_spellInfo->EffectBasePoints[i]+1;
    uint32 mana = m_spellInfo->EffectMiscValue[i];

    ((Player*)unitTarget)->setResurrect(m_caster->GetGUID(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
    SendResurrectRequest(pTarget);
}

void Spell::EffectParry(uint32 i)
{
    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)unitTarget)->SetCanParry(true);
    }
}

void Spell::EffectMomentMove(uint32 i)
{
    if(unitTarget->isInFlight())
        return;

    if( m_spellInfo->rangeIndex== 1)                        //self range
    {
        uint32 mapid = m_caster->GetMapId();
        float dis = GetRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
        float fx = m_caster->GetPositionX() + dis * cos(m_caster->GetOrientation());
        float fy = m_caster->GetPositionY() + dis * sin(m_caster->GetOrientation());
        // teleport a bit above terrainlevel to avoid falling below it
        float fz = MapManager::Instance ().GetMap(mapid)->GetHeight(fx,fy) + 1.5;

        if(unitTarget->GetTypeId() == TYPEID_PLAYER)
            ((Player*)unitTarget)->TeleportTo(mapid, fx, fy, fz, m_caster->GetOrientation(), false);
        else
            MapManager::Instance().GetMap(mapid)->CreatureRelocation((Creature*)m_caster, fx, fy, fz, m_caster->GetOrientation());
    }
}

void Spell::EffectReputation(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)m_caster;

    int32  rep_change = m_spellInfo->EffectBasePoints[i]+1; // field store reputation change -1

    uint32 faction_id = m_spellInfo->EffectMiscValue[i];

    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);

    if(!factionEntry)
        return;

    _player->ModifyFactionReputation(factionEntry,rep_change);
}

void Spell::EffectQuestComplete(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)m_caster;

    uint32 quest_id = m_spellInfo->EffectMiscValue[i];

    if(_player->CanCompleteQuest( quest_id ) )
        _player->CompleteQuest( quest_id );
    else
        return;

    if(_player->GetQuestRewardStatus( quest_id ))
        return;

    _player->PlayerTalkClass->SendQuestGiverOfferReward( quest_id, _player->GetGUID(), true, NULL, 0 );
}

void Spell::EffectSelfResurrect(uint32 i)
{
    if(!unitTarget) return;
    if(unitTarget->GetTypeId() != TYPEID_PLAYER) return;
    if(unitTarget->isAlive()) return;
    if(!unitTarget->IsInWorld()) return;

    uint32 health = 0;
    uint32 mana = 0;

    if(m_spellInfo->SpellVisual == 99 && m_spellInfo->SpellIconID ==1)
    {
        health = m_spellInfo->EffectBasePoints[i]+1 > 0 ? m_spellInfo->EffectBasePoints[i]+1 :(-(m_spellInfo->EffectBasePoints[i]+1));
        if(unitTarget->getPowerType() == POWER_MANA)
            mana = m_spellInfo->EffectMiscValue[i];
    }
    else
    {
        health = uint32(damage/100*unitTarget->GetMaxHealth());
        if(unitTarget->getPowerType() == POWER_MANA)
            mana = uint32(damage/100*unitTarget->GetMaxPower(unitTarget->getPowerType()));
    }
    ((Player*)unitTarget)->setResurrect(m_caster->GetGUID(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
    SendResurrectRequest((Player*)unitTarget);
}

void Spell::EffectSkinning(uint32 i)
{
    if(unitTarget->GetTypeId() != TYPEID_UNIT )
        return;
    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    int32 targetLevel = unitTarget->getLevel();

    ((Player*)m_caster)->SendLoot(unitTarget->GetGUID(),LOOT_SKINNING);
    unitTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

    int32 reqValue = targetLevel < 10 ? 0 : targetLevel < 20 ? (targetLevel-10)*10 : targetLevel*5;

    int32 skinningValue = ((Player*)m_caster)->GetPureSkillValue(SKILL_SKINNING);

    // Double chances for elites
    ((Player*)m_caster)->UpdateGatherSkill(SKILL_SKINNING, skinningValue, reqValue, ((Creature*)unitTarget)->isElite() ? 2 : 1 );
}

void Spell::EffectCharge(uint32 i)
{
    if(!unitTarget || !m_caster)
        return;

    float x, y, z;
    unitTarget->GetClosePoint(m_caster, x, y, z);
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        ((Creature *)unitTarget)->StopMoving();

    m_caster->SendMonsterMove(x, y, z, false,true,1);
    m_caster->Attack(unitTarget);
}

void Spell::EffectSummonCritter(uint32 i)
{
    uint32 pet_entry = m_spellInfo->EffectMiscValue[i];
    if(!pet_entry)
        return;

    Pet* old_critter = NULL;

    {
        CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
        Cell cell = RedZone::GetZone(p);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();

        PetWithIdCheck u_check(m_caster, pet_entry);
        MaNGOS::UnitSearcher<PetWithIdCheck> checker((Unit*&)old_critter, u_check);
        TypeContainerVisitor<MaNGOS::UnitSearcher<PetWithIdCheck>, TypeMapContainer<AllObjectTypes> > object_checker(checker);
        CellLock<GridReadGuard> cell_lock(cell, p);
        cell_lock->Visit(cell_lock, object_checker, *MapManager::Instance().GetMap(m_caster->GetMapId()));
    }

    if (old_critter)                                        // find old critter, unsummon
    {
        old_critter->Remove(PET_SAVE_AS_DELETED);
        return;
    }
    else                                                    // in another case summon new
    {
        Pet* critter = new Pet(MINI_PET);

        if(!critter->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),
            m_caster->GetMapId(),
            m_caster->GetPositionX(),m_caster->GetPositionY(),
            m_caster->GetPositionZ(),m_caster->GetOrientation(),
            m_spellInfo->EffectMiscValue[i]))
        {
            sLog.outError("no such creature entry %u",m_spellInfo->EffectMiscValue[i]);
            delete critter;
            return;
        }

        critter->SetUInt64Value(UNIT_FIELD_SUMMONEDBY,m_caster->GetGUID());
        critter->SetUInt64Value(UNIT_FIELD_CREATEDBY,m_caster->GetGUID());
        critter->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->getFaction());

        critter->AIM_Initialize();

        std::string name;
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            name = ((Player*)m_caster)->GetName();
        else
            name = ((Creature*)m_caster)->GetCreatureInfo()->Name;
        name.append(petTypeSuffix[critter->getPetType()]);
        critter->SetName( name );
        //m_caster->SetPet(critter);

        critter->AddToWorld();
        MapManager::Instance().GetMap(m_caster->GetMapId())->Add((Creature*)critter);
    }
}

void Spell::EffectKnockBack(uint32 i)
{
    if(!unitTarget || !m_caster)
        return;

    //Effect only works on players
    if(unitTarget->GetTypeId()!=TYPEID_PLAYER)
        return;

    float value = 0;
    int32 basePoints = m_spellInfo->EffectBasePoints[i];
    int32 randomPoints = m_spellInfo->EffectDieSides[i];
    if (randomPoints) value = basePoints + rand()%randomPoints;
    else value = basePoints;

    //Only allowed to knock ourselves straight up to prevent exploiting
    if (unitTarget == m_caster)value = 0;

    WorldPacket data(SMSG_MOVE_KNOCK_BACK, (8+4+4+4+4+4));
    data.append(unitTarget->GetPackGUID());
    data << uint32(0);                                      //Sequence
    data << cos(m_caster->GetAngle(unitTarget));            //xdirection
    data << sin(m_caster->GetAngle(unitTarget));            //ydirection
    data << value/10;                                       //Horizontal speed
    data << float(m_spellInfo->EffectMiscValue[i])/-10;     //Z Movement speed

    ((Player*)unitTarget)->SendMessageToSet(&data,true);
}

void Spell::EffectSummonDeadPet(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    Player *_player = (Player*)m_caster;
    Pet *pet = _player->GetPet();
    if(!pet)
        return;
    if(pet->isAlive())
        return;
    pet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
    pet->RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
    pet->setDeathState( ALIVE );
    pet->clearUnitState(UNIT_STAT_ALL_STATE);
    pet->SetHealth( uint32(pet->GetMaxHealth()*damage/100));

    pet->AIM_Initialize();

    _player->PetSpellInitialize();
    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
}

void Spell::EffectTransmitted(uint32 i)
{
    float fx,fy;

    float min_dis = GetMinRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));
    float max_dis = GetMaxRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));
    float diff = max_dis - min_dis + 1;
    float dis = (float)(rand()%(uint32)diff + (uint32)min_dis);

    fx = m_caster->GetPositionX() + dis * cos(m_caster->GetOrientation());
    fy = m_caster->GetPositionY() + dis * sin(m_caster->GetOrientation());

    float fz = MapManager::Instance ().GetMap(m_caster->GetMapId())->GetHeight(fx,fy);

    if(m_spellInfo->EffectMiscValue[i] == 35591)
    {
        Map* map = MapManager::Instance().GetMap(m_caster->GetMapId());
        if ( !map->IsUnderWater(fx,fy,fz) )
        {
            SendCastResult(CAST_FAIL_CANT_BE_CAST_HERE);
            up_skillvalue = 4;
            SendChannelUpdate(0);
            return;
        }

        // replace by water level in this case
        fz = MapManager::Instance ().GetMap(m_caster->GetMapId())->GetWaterLevel(fx,fy);

    }

    GameObject* pGameObj = new GameObject();
    uint32 name_id = m_spellInfo->EffectMiscValue[i];

    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), name_id,m_caster->GetMapId(),
        fx, fy, fz, m_caster->GetOrientation(), 0, 0, 0, 0, 100, 0))
    {
        delete pGameObj;
        return;
    }

    if(m_spellInfo->EffectMiscValue[i] == 35591)
    {
        m_caster->SetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT,pGameObj->GetGUIDLow());
        m_caster->SetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT+1,pGameObj->GetGUIDHigh());
                                                            //Orientation3
        pGameObj->SetFloatValue(GAMEOBJECT_ROTATION + 2, 0.88431775569915771 );
                                                            //Orientation4
        pGameObj->SetFloatValue(GAMEOBJECT_ROTATION + 3, -0.4668855369091033 );
        pGameObj->SetLootState(GO_NOT_READY);               // bobber not move
        m_caster->AddGameObject(pGameObj);                  // will removed at spell cancel

        // end time of range when posable catch fish (FISHING_BOBBER_READY_TIME..GetDuration(m_spellInfo))
        // start time == fish-FISHING_BOBBER_READY_TIME (0..GetDuration(m_spellInfo)-FISHING_BOBBER_READY_TIME)
        uint32 fish = urand(FISHING_BOBBER_READY_TIME,GetDuration(m_spellInfo));
        pGameObj->SetRespawnTimer(fish);
    }
    else
    {
        pGameObj->SetOwnerGUID(m_caster->GetGUID() );
        pGameObj->SetRespawnTimer(GetDuration(m_spellInfo));
    }

    pGameObj->SetUInt32Value(12, 0x3F63BB3C );
    pGameObj->SetUInt32Value(13, 0xBEE9E017 );
    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel() );
    pGameObj->SetSpellId(m_spellInfo->Id);

    DEBUG_LOG("AddObject at SpellEfects.cpp EffectTransmitted\n");
    //m_caster->AddGameObject(pGameObj);
    //m_ObjToDel.push_back(pGameObj);

    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);
    pGameObj->AddToWorld();

    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM, 8);
    data << uint64(pGameObj->GetGUID());
    m_caster->SendMessageToSet(&data,true);
}

void Spell::EffectSkill(uint32 i)
{
    sLog.outDebug("WORLD: SkillEFFECT");
}
