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
#include "BattleGround.h"

pEffect SpellEffects[TOTAL_SPELL_EFFECTS]=
{
    &Spell::EffectNULL,                                     //  0
    &Spell::EffectInstaKill,                                //  1 SPELL_EFFECT_INSTAKILL
    &Spell::EffectSchoolDMG,                                //  2 SPELL_EFFECT_SCHOOL_DAMAGE
    &Spell::EffectDummy,                                    //  3 SPELL_EFFECT_DUMMY
    &Spell::EffectNULL,                                     //  4 SPELL_EFFECT_PORTAL_TELEPORT          unused
    &Spell::EffectTeleportUnits,                            //  5 SPELL_EFFECT_TELEPORT_UNITS
    &Spell::EffectApplyAura,                                //  6 SPELL_EFFECT_APPLY_AURA
    &Spell::EffectSchoolDMG,                                //  7 SPELL_EFFECT_ENVIRONMENTAL_DAMAGE
    &Spell::EffectManaDrain,                                //  8 SPELL_EFFECT_MANA_DRAIN
    &Spell::EffectHealthLeach,                              //  9 SPELL_EFFECT_HEALTH_LEECH
    &Spell::EffectHeal,                                     // 10 SPELL_EFFECT_HEAL
    &Spell::EffectNULL,                                     // 11 SPELL_EFFECT_BIND
    &Spell::EffectNULL,                                     // 12 SPELL_EFFECT_PORTAL
    &Spell::EffectNULL,                                     // 13 SPELL_EFFECT_RITUAL_BASE              unused
    &Spell::EffectNULL,                                     // 14 SPELL_EFFECT_RITUAL_SPECIALIZE        unused
    &Spell::EffectNULL,                                     // 15 SPELL_EFFECT_RITUAL_ACTIVATE_PORTAL   unused
    &Spell::EffectQuestComplete,                            // 16 SPELL_EFFECT_QUEST_COMPLETE
    &Spell::EffectWeaponDmg,                                // 17 SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL
    &Spell::EffectResurrect,                                // 18 SPELL_EFFECT_RESURRECT
    &Spell::EffectNULL,                                     // 19 SPELL_EFFECT_ADD_EXTRA_ATTACKS
    &Spell::EffectNULL,                                     // 20 SPELL_EFFECT_DODGE                    one spell: Dodge
    &Spell::EffectNULL,                                     // 21 SPELL_EFFECT_EVADE                    one spell: Evade (DND)
    &Spell::EffectParry,                                    // 22 SPELL_EFFECT_PARRY
    &Spell::EffectNULL,                                     // 23 SPELL_EFFECT_BLOCK                    one spell: Block
    &Spell::EffectCreateItem,                               // 24 SPELL_EFFECT_CREATE_ITEM
    &Spell::EffectNULL,                                     // 25 SPELL_EFFECT_WEAPON
    &Spell::EffectNULL,                                     // 26 SPELL_EFFECT_DEFENSE                  one spell: Defense
    &Spell::EffectPersistentAA,                             // 27 SPELL_EFFECT_PERSISTENT_AREA_AURA
    &Spell::EffectSummon,                                   // 28 SPELL_EFFECT_SUMMON
    &Spell::EffectMomentMove,                               // 29 SPELL_EFFECT_LEAP
    &Spell::EffectEnergize,                                 // 30 SPELL_EFFECT_ENERGIZE
    &Spell::EffectWeaponDmg,                                // 31 SPELL_EFFECT_WEAPON_PERCENT_DAMAGE
    &Spell::EffectNULL,                                     // 32 SPELL_EFFECT_TRIGGER_MISSILE
    &Spell::EffectOpenLock,                                 // 33 SPELL_EFFECT_OPEN_LOCK
    &Spell::EffectSummonChangeItem,                         // 34 SPELL_EFFECT_SUMMON_CHANGE_ITEM
    &Spell::EffectApplyAA,                                  // 35 SPELL_EFFECT_APPLY_AREA_AURA
    &Spell::EffectLearnSpell,                               // 36 SPELL_EFFECT_LEARN_SPELL
    &Spell::EffectNULL,                                     // 37 SPELL_EFFECT_SPELL_DEFENSE            one spell: SPELLDEFENSE (DND)
    &Spell::EffectDispel,                                   // 38 SPELL_EFFECT_DISPEL
    &Spell::EffectNULL,                                     // 39 SPELL_EFFECT_LANGUAGE
    &Spell::EffectDualWield,                                // 40 SPELL_EFFECT_DUAL_WIELD
    &Spell::EffectSummonWild,                               // 41 SPELL_EFFECT_SUMMON_WILD
    &Spell::EffectSummonWild,                               // 42 SPELL_EFFECT_SUMMON_GUARDIAN
    &Spell::EffectTeleUnitsFaceCaster,                      // 43 SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER
    &Spell::EffectLearnSkill,                               // 44 SPELL_EFFECT_SKILL_STEP
    &Spell::EffectNULL,                                     // 45                                       honor/pvp related
    &Spell::EffectNULL,                                     // 46 SPELL_EFFECT_SPAWN                    we must spawn pet there
    &Spell::EffectTradeSkill,                               // 47 SPELL_EFFECT_TRADE_SKILL
    &Spell::EffectNULL,                                     // 48 SPELL_EFFECT_STEALTH                  one spell: Base Stealth
    &Spell::EffectNULL,                                     // 49 SPELL_EFFECT_DETECT                   one spell: Detect
    &Spell::EffectTransmitted,                              // 50 SPELL_EFFECT_TRANS_DOOR
    &Spell::EffectNULL,                                     // 51 SPELL_EFFECT_FORCE_CRITICAL_HIT       unused
    &Spell::EffectNULL,                                     // 52 SPELL_EFFECT_GUARANTEE_HIT            one spell: zzOLDCritical Shot
    &Spell::EffectEnchantItemPerm,                          // 53 SPELL_EFFECT_ENCHANT_ITEM
    &Spell::EffectEnchantItemTmp,                           // 54 SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
    &Spell::EffectTameCreature,                             // 55 SPELL_EFFECT_TAMECREATURE
    &Spell::EffectSummonPet,                                // 56 SPELL_EFFECT_SUMMON_PET
    &Spell::EffectLearnPetSpell,                            // 57 SPELL_EFFECT_LEARN_PET_SPELL
    &Spell::EffectWeaponDmg,                                // 58 SPELL_EFFECT_WEAPON_DAMAGE
    &Spell::EffectOpenSecretSafe,                           // 59 SPELL_EFFECT_OPEN_LOCK_ITEM
    &Spell::EffectProficiency,                              // 60 SPELL_EFFECT_PROFICIENCY
    &Spell::EffectSendEvent,                                // 61 SPELL_EFFECT_SEND_EVENT
    &Spell::EffectPowerDrain,                               // 62 SPELL_EFFECT_POWER_BURN
    &Spell::EffectThreat,                                   // 63 SPELL_EFFECT_THREAT
    &Spell::EffectTriggerSpell,                             // 64 SPELL_EFFECT_TRIGGER_SPELL
    &Spell::EffectNULL,                                     // 65 SPELL_EFFECT_HEALTH_FUNNEL            unused
    &Spell::EffectNULL,                                     // 66 SPELL_EFFECT_POWER_FUNNEL             unused
    &Spell::EffectHealMaxHealth,                            // 67 SPELL_EFFECT_HEAL_MAX_HEALTH
    &Spell::EffectInterruptCast,                            // 68 SPELL_EFFECT_INTERRUPT_CAST
    &Spell::EffectNULL,                                     // 69 SPELL_EFFECT_DISTRACT
    &Spell::EffectNULL,                                     // 70 SPELL_EFFECT_PULL                     one spell: Distract Move
    &Spell::EffectPickPocket,                               // 71 SPELL_EFFECT_PICKPOCKET
    &Spell::EffectAddFarsight,                              // 72 SPELL_EFFECT_ADD_FARSIGHT
    &Spell::EffectSummonWild,                               // 73 SPELL_EFFECT_SUMMON_POSSESSED
    &Spell::EffectNULL,                                     // 74 SPELL_EFFECT_SUMMON_TOTEM
    &Spell::EffectNULL,                                     // 75 SPELL_EFFECT_HEAL_MECHANICAL          one spell: Mechanical Patch Kit
    &Spell::EffectSummonObjectWild,                         // 76 SPELL_EFFECT_SUMMON_OBJECT_WILD
    &Spell::EffectScriptEffect,                             // 77 SPELL_EFFECT_SCRIPT_EFFECT
    &Spell::EffectNULL,                                     // 78 SPELL_EFFECT_ATTACK
    &Spell::EffectSanctuary,                                // 79 SPELL_EFFECT_SANCTUARY
    &Spell::EffectAddComboPoints,                           // 80 SPELL_EFFECT_ADD_COMBO_POINTS
    &Spell::EffectNULL,                                     // 81 SPELL_EFFECT_CREATE_HOUSE             one spell: Create House (TEST)
    &Spell::EffectNULL,                                     // 82 SPELL_EFFECT_BIND_SIGHT
    &Spell::EffectDuel,                                     // 83 SPELL_EFFECT_DUEL
    &Spell::EffectStuck,                                    // 84 SPELL_EFFECT_STUCK
    &Spell::EffectSummonPlayer,                             // 85 SPELL_EFFECT_SUMMON_PLAYER
    &Spell::EffectNULL,                                     // 86 SPELL_EFFECT_ACTIVATE_OBJECT
    &Spell::EffectSummonTotem,                              // 87 SPELL_EFFECT_SUMMON_TOTEM_SLOT1
    &Spell::EffectSummonTotem,                              // 88 SPELL_EFFECT_SUMMON_TOTEM_SLOT2
    &Spell::EffectSummonTotem,                              // 89 SPELL_EFFECT_SUMMON_TOTEM_SLOT3
    &Spell::EffectSummonTotem,                              // 90 SPELL_EFFECT_SUMMON_TOTEM_SLOT4
    &Spell::EffectNULL,                                     // 91 SPELL_EFFECT_THREAT_ALL               one spell: zzOLDBrainwash
    &Spell::EffectEnchantHeldItem,                          // 92 SPELL_EFFECT_ENCHANT_HELD_ITEM
    &Spell::EffectNULL,                                     // 93 SPELL_EFFECT_SUMMON_PHANTASM
    &Spell::EffectSelfResurrect,                            // 94 SPELL_EFFECT_SELF_RESURRECT
    &Spell::EffectSkinning,                                 // 95 SPELL_EFFECT_SKINNING
    &Spell::EffectCharge,                                   // 96 SPELL_EFFECT_CHARGE
    &Spell::EffectSummonCritter,                            // 97 SPELL_EFFECT_SUMMON_CRITTER
    &Spell::EffectKnockBack,                                // 98 SPELL_EFFECT_KNOCK_BACK
    &Spell::EffectDisEnchant,                               // 99 SPELL_EFFECT_DISENCHANT
    &Spell::EffectInebriate,                                //100 SPELL_EFFECT_INEBRIATE
    &Spell::EffectFeedPet,                                  //101 SPELL_EFFECT_FEED_PET
    &Spell::EffectDismissPet,                               //102 SPELL_EFFECT_DISMISS_PET
    &Spell::EffectReputation,                               //103 SPELL_EFFECT_REPUTATION
    &Spell::EffectSummonObject,                             //104 SPELL_EFFECT_SUMMON_OBJECT_SLOT1
    &Spell::EffectSummonObject,                             //105 SPELL_EFFECT_SUMMON_OBJECT_SLOT2
    &Spell::EffectSummonObject,                             //106 SPELL_EFFECT_SUMMON_OBJECT_SLOT3
    &Spell::EffectSummonObject,                             //107 SPELL_EFFECT_SUMMON_OBJECT_SLOT4
    &Spell::EffectNULL,                                     //108 SPELL_EFFECT_DISPEL_MECHANIC
    &Spell::EffectSummonDeadPet,                            //109 SPELL_EFFECT_SUMMON_DEAD_PET
    &Spell::EffectNULL,                                     //110 SPELL_EFFECT_DESTROY_ALL_TOTEMS
    &Spell::EffectNULL,                                     //111 SPELL_EFFECT_DURABILITY_DAMAGE
    &Spell::EffectNULL,                                     //112 SPELL_EFFECT_SUMMON_DEMON
    &Spell::EffectResurrectNew,                             //113 SPELL_EFFECT_RESURRECT_NEW
    &Spell::EffectAttackMe,                                 //114 SPELL_EFFECT_ATTACK_ME
    &Spell::EffectNULL,                                     //115 SPELL_EFFECT_DURABILITY_DAMAGE_PCT
    &Spell::EffectNULL,                                     //116 SPELL_EFFECT_SKIN_PLAYER_CORPSE       one spell: Remove Insignia, bg usage, required special corpse flags...
    &Spell::EffectNULL,                                     //117 SPELL_EFFECT_SPIRIT_HEAL              one spell: Spirit Heal
    &Spell::EffectSkill,                                    //118 SPELL_EFFECT_SKILL                    professions and more
    &Spell::EffectNULL,                                     //119 SPELL_EFFECT_APPLY_AURA_NEW
    &Spell::EffectNULL,                                     //120 SPELL_EFFECT_TELEPORT_GRAVEYARD       two spells: Graveyard Teleport and Graveyard Teleport Test
    &Spell::EffectWeaponDmg,                                //121 SPELL_EFFECT_NORMALIZED_WEAPON_DMG
    &Spell::EffectNULL,                                     //122 SPELL_EFFECT_122                      silithist cap reward spell
    &Spell::EffectNULL,                                     //123 SPELL_EFFECT_123                      taxi/flight related
    &Spell::EffectNULL,                                     //124 SPELL_EFFECT_124                      aggro redirect?
    &Spell::EffectNULL,                                     //125 SPELL_EFFECT_125                      invis?
    &Spell::EffectNULL,                                     //126 SPELL_EFFECT_126                      spell steal effect?
    &Spell::EffectNULL,                                     //127 SPELL_EFFECT_127                      Prospecting spell
    &Spell::EffectNULL,                                     //128 SPELL_EFFECT_128
    &Spell::EffectNULL,                                     //129 SPELL_EFFECT_129
    &Spell::EffectNULL,                                     //130 SPELL_EFFECT_130                      threat redirect
    &Spell::EffectNULL,                                     //131 SPELL_EFFECT_131                      unused
    &Spell::EffectNULL,                                     //132 SPELL_EFFECT_132                      taxi related, one spell: Brazen Taxi
    &Spell::EffectNULL,                                     //133 SPELL_EFFECT_133                      one spell: Forget
    &Spell::EffectNULL,                                     //134 SPELL_EFFECT_134
    &Spell::EffectNULL,                                     //135 SPELL_EFFECT_135                      pet related
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
        m_caster->DealDamage(unitTarget, health, DIRECT_DAMAGE, 0, NULL, 0, false);
    }
}

void Spell::EffectSchoolDMG(uint32 i)
{
    if( unitTarget && unitTarget->isAlive())
    {
        // Bloodthirst
        if(m_spellInfo->Category == 971 && m_spellInfo->SpellVisual == 372)
            return EffectWeaponDmg(i);

        if(damage >= 0)
            m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, damage, m_IsTriggeredSpell);
    }

    if (m_caster->GetTypeId()==TYPEID_PLAYER && m_spellInfo->Attributes == 0x150010)
        m_caster->AttackStop();
}

void Spell::EffectDummy(uint32 i)
{
    if(!unitTarget)
        return;

    // More spell specific code in beginning
    if(m_spellInfo->Id == 5420)                             // Tree of Life passive
    {

        // Tree of Life area effect
        SpellEntry const* spellInfo = sSpellStore.LookupEntry( 34123 );

        if(!spellInfo)
            return;

        int32 health_mod = int32(m_caster->GetStat(STAT_SPIRIT)/4);
        SpellEntry customSpellInfo = *spellInfo;
        customSpellInfo.EffectBasePoints[0] = health_mod-1;
        m_caster->CastSpell(m_caster,&customSpellInfo,true,NULL);
        return;
    }

    if(m_spellInfo->Id == 13535)
    {
        SpellEntry const* spellInfo = sSpellStore.LookupEntry( 13481 );

        if(m_caster->GetTypeId() != TYPEID_PLAYER || !spellInfo)
            return;

        Spell spell(m_caster, spellInfo, true, m_triggeredByAura);
        SpellCastTargets targets;

        Unit* target = ObjectAccessor::Instance().GetUnit(*m_caster, ((Player*)m_caster)->GetSelection());
        if(!target)
            return;

        targets.setUnitTarget(target);
        spell.prepare(&targets);
        return;
    }

    if( m_spellInfo->Id == SPELL_ID_AGGRO )
    {
        if( !m_caster || !m_caster->getVictim() )
            return;

        // only creature to creature
        if( unitTarget->GetTypeId() != TYPEID_UNIT || m_caster->GetTypeId() != TYPEID_UNIT )
            return;

        // if creature not fighting currently
        if( unitTarget->isInCombat() )
            return;

        // skip non hostile to caster enemy creatures
        if( !((Creature*)unitTarget)->IsHostileTo(m_caster->getVictim()) )
            return;

        // only from same creature faction
        if(unitTarget->getFaction() != m_caster->getFaction() )
            return;

        ((Creature*)m_caster)->SetNoCallAssistence(true);
        ((Creature*)unitTarget)->SetNoCallAssistence(true);
        if (((Creature*)unitTarget)->AI())
            ((Creature*)unitTarget)->AI()->AttackStart(m_caster->getVictim());
        return;
    }

    // Gift of Life (warrior bwl trinket)
    if(m_spellInfo->Id == 23725)
    {
        int32 health_mod = int32(m_caster->GetMaxHealth()*0.15);

        SpellEntry const *OriginalHealthModSpell = sSpellStore.LookupEntry(23782);
        SpellEntry CustomHealthModSpell = *OriginalHealthModSpell;
        SpellEntry const *OriginalHealingSpell = sSpellStore.LookupEntry(23783);
        SpellEntry CustomHealingSpell = *OriginalHealingSpell;
        CustomHealthModSpell.EffectBasePoints[0] = health_mod-1;
        CustomHealingSpell.EffectBasePoints[0] = health_mod-1;
        m_caster->CastSpell(m_caster,&CustomHealthModSpell,true,NULL);
        m_caster->CastSpell(m_caster,&CustomHealingSpell,true,NULL);
        return;
    }

    // Last Stand
    if(m_spellInfo->Id == 12975)
    {
        uint32 health_mod = uint32(m_caster->GetMaxHealth()*0.3);
        SpellEntry const* OriginalHealthModSpell = sSpellStore.LookupEntry(12976);
        SpellEntry CustomHealthModSpell = *OriginalHealthModSpell;
        CustomHealthModSpell.EffectBasePoints[0] = health_mod;
        m_caster->CastSpell(m_caster,&CustomHealthModSpell,true,NULL);
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

        CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
        Cell cell = RedZone::GetZone(p);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();

        Unit* result = NULL;

        MaNGOS::CannibalizeUnitCheck u_check(m_caster, max_range);
        MaNGOS::UnitSearcher<MaNGOS::CannibalizeUnitCheck> searcher(result, u_check);

        TypeContainerVisitor<MaNGOS::UnitSearcher<MaNGOS::CannibalizeUnitCheck>, GridTypeMapContainer > unit_searcher(searcher);
        CellLock<GridReadGuard> cell_lock(cell, p);
        cell_lock->Visit(cell_lock, unit_searcher, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));

        if (!result)
        {
            // clear cooldown at fail
            if(m_caster->GetTypeId()==TYPEID_PLAYER)
            {
                ((Player*)m_caster)->RemoveSpellCooldown(20577);

                WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8+4));
                data << uint32(20577);                      // spell id
                data << m_caster->GetGUID();
                ((Player*)m_caster)->GetSession()->SendPacket(&data);
            }

            SendCastResult(SPELL_FAILED_NO_EDIBLE_CORPSES);
            return;
        }

        // ok, main function spell can be casted

        finish();                                           // prepare to replacing this spell cast to main function spell

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

    // Mechanical Dragonling
    if (m_spellInfo->Id == 23076 )
    {
        // TODO: why dummy effect required? some animation or other...
        SpellEntry const *spell_proto = sSpellStore.LookupEntry(4073);
        if(!spell_proto)
            return;

        m_caster->CastSpell(m_caster,spell_proto,true,NULL);
    }

    if (m_spellInfo->Id == 16589)
    {
        if(m_caster->GetTypeId()!=TYPEID_PLAYER)
            return;

        uint32 spell_id = 0;

        switch(urand(1,3))
        {
            case 1: spell_id = 16595; break;
            case 2: spell_id = 16593; break;
            case 3: spell_id = 16591; break;
        }

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell_id);
        if(!spellInfo)
            return;

        m_caster->CastSpell(m_caster,spellInfo,true,NULL);
        return;
    }

    // net-o-matic
    if (m_spellInfo->Id == 13120)
    {
        uint32 spell_id = 0;

        uint32 roll = urand(0, 99);

        if(roll < 2)                                        // 2% for 30 sec self root (off-like chance unknown)
            spell_id = 16566;
        else if(roll < 4)                                   // 2% for 20 sec root, charge to target (off-like chance unknown)
            spell_id = 13119;
        else                                                // normal root
            spell_id = 13099;

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell_id);
        if(!spellInfo)
            return;

        m_caster->CastSpell(unitTarget,spellInfo,true,NULL);
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

    // Berserking (troll racial traits)
    if (m_spellInfo->SpellIconID == 1661)
    {
        uint32 healthPerc = uint32((float(m_caster->GetHealth())/m_caster->GetMaxHealth())*100);
        int32 melee_mod = 10;
        if (healthPerc <= 40)
            melee_mod = 30;
        if (healthPerc < 100 && healthPerc > 40)
            melee_mod = 10+(100-healthPerc)/3;
        SpellEntry const *OriginalHasteModSpell = sSpellStore.LookupEntry(26635);
        SpellEntry CustomHasteModSpell = *OriginalHasteModSpell;
        CustomHasteModSpell.EffectBasePoints[0] = melee_mod-1;
                                                            // (EffectBasePoints[0]+1)-1+(5-melee_mod) = (melee_mod-1+1)-1+5-melee_mod = 5-1
        CustomHasteModSpell.EffectBasePoints[1] = (5-melee_mod)-1;
        CustomHasteModSpell.EffectBasePoints[2] = 5-1;
        m_caster->CastSpell(m_caster,&CustomHasteModSpell,true,NULL);
        return;
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
            case 27174:
                hurt = 27176;
                heal = 27175;
                break;
            case 33072:
                hurt = 33073;
                heal = 33074;
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
        int32 dmg = damage;
        dmg += int32(m_caster->GetPower(POWER_RAGE) * m_spellInfo->DmgMultiplier[i]);
        SpellEntry const *tspellInfo = sSpellStore.LookupEntry(20647);
        SpellEntry sInfo = *tspellInfo;
        sInfo.EffectBasePoints[0] = dmg-1;
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

    // Charge
    if(m_spellInfo->SpellVisual == 867 && m_spellInfo->SpellIconID == 457)
    {
        uint32 rage = m_spellInfo->EffectBasePoints[i];
        SpellEntry const* OriginalCharge = sSpellStore.LookupEntry(34846);
        SpellEntry CustomCharge = *OriginalCharge;
        CustomCharge.EffectBasePoints[0] = rage;
        m_caster->CastSpell(m_caster,&CustomCharge,true);
        return;
    }

    // Judgement of command
    if (m_spellInfo->Attributes == 0x50800 && m_spellInfo->AttributesEx == 128)
    {
        uint32 spell_id = m_spellInfo->EffectBasePoints[i]+1;
        SpellEntry const* spell_proto = sSpellStore.LookupEntry(spell_id);
        if(!spell_proto)
            return;

        if( !unitTarget->hasUnitState(UNIT_STAT_STUNDED) )
        {
            // copy to decreased damage (/2) for non-stunned target.
            SpellEntry spell_proto_copy = *spell_proto;
            assert(spell_proto_copy.Effect[0]==SPELL_EFFECT_SCHOOL_DAMAGE);
            spell_proto_copy.EffectBasePoints[0] /= 2;
            spell_proto_copy.EffectBaseDice[0] /= 2;

            m_caster->CastSpell(unitTarget,&spell_proto_copy,true,NULL);
        }
        else
        {
            m_caster->CastSpell(unitTarget,spell_proto,true,NULL);
        }
    }

    //BattleGround spells
    if(m_caster->GetTypeId() == TYPEID_PLAYER && ((Player*)m_caster)->InBattleGround())
    {
        BattleGround* bg = sBattleGroundMgr.GetBattleGround(((Player*)m_caster)->GetBattleGroundId());
        if(bg && bg->GetStatus() == STATUS_INPROGRESS)
        {
            switch(m_spellInfo->Id)
            {
                case 23383:                             // Alliance Flag Click
                    sLog.outDebug("Alliance Flag Click");
                    if(((Player*)m_caster)->GetTeam() == ALLIANCE)
                                                            // Alliance Flag Returns (Event)
                        m_caster->CastSpell(m_caster, 23385, true, 0);
                    if(((Player*)m_caster)->GetTeam() == HORDE)
                                                            // Silverwing Flag
                        m_caster->CastSpell(m_caster, 23335, true, 0);
                    break;
                case 23384:                             // Horde Flag Click
                    sLog.outDebug("Horde Flag Click");
                    if(((Player*)m_caster)->GetTeam() == HORDE)
                                                            // Horde Flag Returns (Event)
                        m_caster->CastSpell(m_caster, 23386, true, 0);
                    if(((Player*)m_caster)->GetTeam() == ALLIANCE)
                                                            // Warsong Flag
                        m_caster->CastSpell(m_caster, 23333, true, 0);
                    break;
                case 23389:                             // Alliance Flag Capture
                    sLog.outDebug("Alliance Flag Capture");
                    bg->EventPlayerCapturedFlag((Player*)m_caster);
                    break;
                case 23390:                             // Horde Flag Capture
                    sLog.outDebug("Horde Flag Capture");
                    bg->EventPlayerCapturedFlag((Player*)m_caster);
                    break;
                default:
                    sLog.outDebug("Unknown spellid %u at BG dummy", m_spellInfo->Id);
                    break;
            }
        }
    }

    if(m_spellInfo->Id == 17251)                            // Spirit Healer Res
    {
                                                            // probably useless check
        if(m_targets.getUnitTarget()->GetTypeId() == TYPEID_PLAYER)
        {
            WorldPacket data(SMSG_SPIRIT_HEALER_CONFIRM, 8);
            data << m_caster->GetGUID();
                                                            // for this spell we have wrong unitTarget, but correct m_targets.getUnitTarget()...
            ((Player*)m_targets.getUnitTarget())->GetSession()->SendPacket( &data );
        }
    }

    // Shaman's "Lightning Shield"
    if(m_spellInfo->Id == 26545)
    {
        if(!m_caster) return;
        if(!unitTarget) return;
        if(!m_triggeredByAura) return;
        uint32 spell = 0;

        switch(m_triggeredByAura->GetId())
        {
            case   324: spell = 26364; break;               // Rank 1 
            case   325: spell = 26365; break;               // Rank 2
            case   905: spell = 26366; break;               // Rank 3
            case   945: spell = 26367; break;               // Rank 4
            case  8134: spell = 26369; break;               // Rank 5
            case 10431: spell = 26370; break;               // Rank 6
            case 10432: spell = 26363; break;               // Rank 7
            case 25469: spell = 26371; break;               // Rank 8
            case 25472: spell = 26372; break;               // Rank 9
            default:
                sLog.outError("Spell::EffectDummy: Spell 26545 triggered by unhandled spell %u",m_triggeredByAura->GetId());
                break;
        }
        m_caster->CastSpell(unitTarget, spell, true, NULL);
    }

    // Priest's "Shadowguard"
    if(m_spellInfo->Id == 28376)
    {
        if(!m_caster) return;
        if(!unitTarget) return;
        if(!m_triggeredByAura) return;

        uint32 spell = 0;
        switch(m_triggeredByAura->GetId())
        {
            case 18137: spell = 28377; break;               // Rank 1
            case 19308: spell = 28378; break;               // Rank 2
            case 19309: spell = 28379; break;               // Rank 3
            case 19310: spell = 28380; break;               // Rank 4
            case 19311: spell = 28381; break;               // Rank 5
            case 19312: spell = 28382; break;               // Rank 6
            case 25477: spell = 28385; break;               // Rank 7
            default:
                sLog.outError("Spell::EffectDummy: Spell 28376 triggered by unhandled spell %u",m_triggeredByAura->GetId());
                break;
        }

        m_caster->CastSpell(unitTarget, spell, true, NULL);
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

    m_TriggerSpell.push_back(spellInfo);
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
                                                            // ghost spell check
    if(!unitTarget->isAlive() && !(m_spellInfo->Id == 20584 || m_spellInfo->Id == 8326))
        return;

    //If m_immuneToState type contain this aura type, IMMUNE aura.
    if(unitTarget->IsImmunedToSpellEffect(m_spellInfo->EffectApplyAuraName[i]))
    {
        SendCastResult(SPELL_FAILED_IMMUNE);
        return;
    }

    sLog.outDebug("Spell: Aura is: %u", m_spellInfo->EffectApplyAuraName[i]);

    Aura* Aur = new Aura(m_spellInfo, i, unitTarget,m_caster, m_CastItem);

    if(!Aur)                                                // is it useless?
        return;

    if (!Aur->IsPositive() && Aur->GetCasterGUID() != Aur->GetTarget()->GetGUID())
    {
        switch (Aur->GetModifier()->m_auraname)
        {
            case SPELL_AURA_BIND_SIGHT:
            case SPELL_AURA_MOD_CHARM:
            case SPELL_AURA_FAR_SIGHT:
            case SPELL_AURA_MOD_DETECT_RANGE:
            case SPELL_AURA_AURAS_VISIBLE:
            case SPELL_AURA_MOD_STALKED:
            case SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS:
            case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
            case SPELL_AURA_EMPATHY:
            case SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS:
                break;

            default:
                //If Aura is applied to monster then attack caster
                if( Aur->GetTarget()->GetTypeId() == TYPEID_UNIT && !Aur->GetTarget()->isInCombat() && 
                    ((Creature*)Aur->GetTarget())->AI() )
                    ((Creature*)Aur->GetTarget())->AI()->AttackStart(m_caster);
        }
    }

    // Diminishing
    // Use Spell->Mechanic if possible
    DiminishingMechanics mech = Unit::Mechanic2DiminishingMechanics(Aur->GetSpellProto()->Mechanic);

    if(mech == DIMINISHING_NONE)
    {
        switch(m_spellInfo->EffectApplyAuraName[i])
        {
            case SPELL_AURA_MOD_CONFUSE:
                mech = DIMINISHING_MECHANIC_CONFUSE; break;
            case SPELL_AURA_MOD_CHARM: case SPELL_AURA_MOD_FEAR:
                mech = DIMINISHING_MECHANIC_CHARM; break;
            case SPELL_AURA_MOD_STUN:
                mech = DIMINISHING_MECHANIC_STUN; break;
            case SPELL_AURA_MOD_ROOT:
                mech = DIMINISHING_MECHANIC_ROOT; break;
            case SPELL_AURA_MOD_DECREASE_SPEED:
                mech = DIMINISHING_MECHANIC_SPEED; break;
            default: break;
        }
    }

    // Death Coil and Curse of Exhaustion are not diminished.
    if((m_spellInfo->SpellVisual == 64 && m_spellInfo->SpellIconID == 88) ||
        (m_spellInfo->SpellVisual == 185 && m_spellInfo->SpellIconID == 228))
        mech = DIMINISHING_NONE;

    int32 auraDuration = Aur->GetAuraDuration();

    unitTarget->ApplyDiminishingToDuration(mech,auraDuration);

    bool added = unitTarget->AddAura(Aur);

    // found crash at character loading, broken pointer to Aur...
    // Aur was deleted in AddAura()...
    if(!Aur)
        return;

    if (added)
    {
        if(auraDuration == 0)
            unitTarget->RemoveAura(m_spellInfo->Id,i);
        else
        if(auraDuration != Aur->GetAuraDuration())
            unitTarget->SetAurDuration(m_spellInfo->Id,i,auraDuration);

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
                sLog.outDebug("Spell: Additional Aura is: %u", WeakenedSoulSpellInfo->EffectApplyAuraName[i]);
            }
        }

        if(Aur->IsTrigger())
        {
            // arcane missiles
            SpellEntry const *spellInfo = sSpellStore.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
            if (!spellInfo) return;
            if (spellInfo->EffectImplicitTargetA[0] == TARGET_CHAIN_DAMAGE && m_caster->GetTypeId() == TYPEID_PLAYER)
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
    if(damage < 0)
        return;

    uint32 curPower = unitTarget->GetPower(drain_power);
    float tmpvalue = m_spellInfo->EffectMultipleValue[i];
    if(!tmpvalue)
        tmpvalue = 1;

    int32 new_damage;
    if(curPower < uint32(damage))
        new_damage = curPower;
    else
        new_damage = damage;

    unitTarget->ModifyPower(drain_power,-new_damage);

    if(drain_power == POWER_MANA)
        m_caster->ModifyPower(POWER_MANA,uint32(new_damage*tmpvalue));
}

void Spell::EffectSendEvent(uint32 i)
{
    if (m_caster->GetTypeId() == TYPEID_PLAYER && ((Player*)m_caster)->InBattleGround())
    {
        BattleGround* bg = sBattleGroundMgr.GetBattleGround(((Player *)m_caster)->GetBattleGroundId());
        if(bg && bg->GetStatus() == STATUS_INPROGRESS)
        {
            switch(m_spellInfo->Id)
            {
                case 23333:                             // Pickup Horde Flag
                    bg->EventPlayerPickedUpFlag(((Player*)m_caster));
                    sLog.outDebug("Send Event Horde Flag Picked Up");
                    break;
                case 23334:                             // Drop Horde Flag
                    bg->EventPlayerDroppedFlag(((Player*)m_caster));
                    sLog.outDebug("Drop Horde Flag");
                    break;
                case 23335:                             // Pickup Alliance Flag
                    bg->EventPlayerPickedUpFlag(((Player*)m_caster));
                    sLog.outDebug("Send Event Alliance Flag Picked Up");
                    break;
                case 23336:                             // Drop Alliance Flag
                    bg->EventPlayerDroppedFlag(((Player*)m_caster));
                    sLog.outDebug("Drop Alliance Flag");
                    break;
                case 23385:                             // Alliance Flag Returns
                    bg->EventPlayerReturnedFlag(((Player*)m_caster));
                    sLog.outDebug("Alliance Flag Returned");
                    break;
                case 23386:                             // Horde Flag Returns
                    bg->EventPlayerReturnedFlag(((Player*)m_caster));
                    sLog.outDebug("Horde Flag Returned");
                    break;
                default:
                    sLog.outDebug("Unknown spellid %u in BG event", m_spellInfo->Id);
                    break;
            }
        }
    }

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
    if(damage < 0)
        return;

    uint32 curPower = unitTarget->GetPower(POWER_MANA);

    int32 new_damage;
    if(curPower < uint32(damage))
        new_damage = curPower;
    else
        new_damage = damage;

    int32 tmpvalue = int32(new_damage*m_spellInfo->EffectMultipleValue[i]);

    unitTarget->ModifyPower(POWER_MANA,-new_damage);

    m_caster->ModifyPower(POWER_MANA,tmpvalue);
    m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, new_damage/2, m_IsTriggeredSpell);
}

void Spell::EffectHeal( uint32 i )
{
    // Frenzied Regeneration
    // Must be "hard coded" since 22845 is not set in spell
    // EffectTriggerSpell[m_effIndex] field for affected spells
    if (m_spellInfo->Id == 22845)
    {
        float LifePerRage = 0;
        Unit::AuraList& mPeriodicTriggerSpell = m_caster->GetAurasByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        for(Unit::AuraList::iterator i = mPeriodicTriggerSpell.begin(); i != mPeriodicTriggerSpell.end(); ++i)
            if ((*i)->GetSpellProto()->Category == 1011)
        {
            LifePerRage = (*i)->GetModifier()->m_amount / 10.0;
            break;
        }
        if (LifePerRage)
        {
            int32 lRage = 0;
            if (m_caster->GetPower(POWER_RAGE) > 100)
            {
                lRage = 100;
            }
            else
            {
                lRage = m_caster->GetPower(POWER_RAGE);
            }
            m_caster->SetPower(POWER_RAGE, m_caster->GetPower(POWER_RAGE) - lRage);
            damage = uint32(lRage*LifePerRage);
        }
    }

    if( unitTarget && unitTarget->isAlive() && damage >= 0)
    {
        int32 addhealth = m_caster->SpellHealingBonus(m_spellInfo, uint32(damage),HEAL);
        bool crit = m_caster->SpellCriticalBonus(m_spellInfo, &addhealth, NULL);
        if(unitTarget->GetTypeId() == TYPEID_PLAYER)
            SendHealSpellOnPlayer(((Player*)unitTarget), m_spellInfo->Id, addhealth, crit);

        unitTarget->ModifyHealth( addhealth );

        uint32 procHealer = PROC_FLAG_HEAL;
        if (crit)
            procHealer |= PROC_FLAG_CRIT_HEAL;
        if (m_caster != unitTarget)
            m_caster->ProcDamageAndSpell(unitTarget,procHealer,PROC_FLAG_NONE,addhealth,m_spellInfo,m_IsTriggeredSpell);
    }
}

void Spell::EffectHealthLeach(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    if(damage < 0)
        return;

    sLog.outDebug("HealthLeach :%i", damage);

    uint32 curHealth = unitTarget->GetHealth();

    int32 new_damage;
    if(curHealth < uint32(damage))
        new_damage = curHealth;
    else
        new_damage = damage;

    int32 tmpvalue = int32(new_damage*m_spellInfo->EffectMultipleValue[i]);

    m_caster->ModifyHealth(tmpvalue);

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        SendHealSpellOnPlayer(((Player*)unitTarget), m_spellInfo->Id, uint32(tmpvalue));

    m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, new_damage, m_IsTriggeredSpell);
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
    {
        int32 basePoints = m_spellInfo->EffectBasePoints[i];
        int32 randomPoints = m_spellInfo->EffectDieSides[i];
        if (randomPoints)
            num_to_add = basePoints + irand(1, randomPoints);
        else
            num_to_add = basePoints + 1;
    }
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

    // TODO: Maybe pProto->Extra store filter for possible random properties
    uint32 randomPropId = pProto->Extra ? Item::GenerateItemRandomPropertyId(newitemid) : 0;

    Item *pItem = player->StoreNewItem( dest, newitemid, num_to_add, true,randomPropId);

    if(!pItem)
    {
        player->SendEquipError( EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
        return;
    }

    if( pItem->GetProto()->Class != ITEM_CLASS_CONSUMABLE && pItem->GetProto()->Class != ITEM_CLASS_QUEST)
        pItem->SetUInt32Value(ITEM_FIELD_CREATOR,player->GetGUIDLow());

    player->SendNewItem(pItem, num_to_add, true, true);
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
    DynamicObject* dynObj = new DynamicObject(m_caster);
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
    MapManager::Instance().GetMap(dynObj->GetMapId(), dynObj)->Add(dynObj);
}

void Spell::EffectEnergize(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    if(m_spellInfo->EffectMiscValue[i] > 4)
        return;

    if(damage < 0)
        return;

    Powers power = Powers(m_spellInfo->EffectMiscValue[i]);

    if(unitTarget->getPowerType() != power)
        return;

    unitTarget->ModifyPower(power,damage);
    m_caster->SendHealSpellOnPlayerPet(unitTarget, m_spellInfo->Id, damage, power);
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

    if(!lockId)                                             // possible case for GO and maybe for items.
    {
        player->SendLoot(guid,loottype);
        return;
    }

    // Get LockInfo
    LockEntry const *lockInfo = sLockStore.LookupEntry(lockId);

    if (!lockInfo)
    {
        sLog.outError( "Spell::EffectOpenLock: %s [guid = %u] has an unknown lockId: %u!",
            (gameObjTarget ? "gameobject" : "item"), GUID_LOPART(guid), lockId);
        SendCastResult(SPELL_FAILED_BAD_TARGETS);
        return;
    }

    // check key
    if(lockInfo->key && m_CastItem && m_CastItem->GetEntry()==lockInfo->key)
    {
        player->SendLoot(guid,loottype);
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
            SendCastResult(SPELL_FAILED_FIZZLE);
            return;
        }

        reqSkillValue = lockInfo->requiredlockskill;
    }
    else if(SkillId == SKILL_LOCKPICKING)                   // apply picklock skill to wrong target
    {
        SendCastResult(SPELL_FAILED_BAD_TARGETS);
        return;
    }

    if ( SkillId )
    {
        loottype = LOOT_SKINNING;
        if ( player->GetSkillValue(SkillId) + spellSkillBonus < reqSkillValue )
        {
            SendCastResult(SPELL_FAILED_LOW_CASTLEVEL);
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
    Pet* spawnCreature = new Pet(m_caster, SUMMON_PET);

    // before caster
    float x,y,z;
    m_caster->GetClosePoint(NULL,x,y,z);

    if(!spawnCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),
        m_caster->GetMapId(),x,y,z,-m_caster->GetOrientation(),
        m_spellInfo->EffectMiscValue[i]))
    {
        sLog.outErrorDb("Spell::EffectSummon: no such creature entry %u",m_spellInfo->EffectMiscValue[i]);
        delete spawnCreature;
        return;
    }

    spawnCreature->SetUInt64Value(UNIT_FIELD_SUMMONEDBY,m_caster->GetGUID());
    spawnCreature->SetUInt32Value(UNIT_NPC_FLAGS , 0);
    spawnCreature->setPowerType(POWER_MANA);
    spawnCreature->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->getFaction());
    spawnCreature->SetUInt32Value(UNIT_FIELD_FLAGS,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
    spawnCreature->SetUInt64Value(UNIT_FIELD_CREATEDBY, m_caster->GetGUID());

    spawnCreature->InitStatsForLevel(level);

    spawnCreature->AIM_Initialize();

    std::string name;
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        name = ((Player*)m_caster)->GetName();
    else
        name = ((Creature*)m_caster)->GetCreatureInfo()->Name;
    name.append(petTypeSuffix[spawnCreature->getPetType()]);
    spawnCreature->SetName( name );

    ObjectAccessor::Instance().AddPet(spawnCreature);
    spawnCreature->AddToWorld();
    MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster)->Add((Creature*)spawnCreature);

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
            EffectLearnPetSpell(i);

        return;
    }

    Player *player = (Player*)unitTarget;

    uint32 spellToLearn = m_spellInfo->EffectTriggerSpell[i];
    //data.Initialize(SMSG_LEARNED_SPELL);
    //data << spellToLearn;
    //player->GetSession()->SendPacket(&data);
    player->learnSpell((uint16)spellToLearn);

    sLog.outDebug( "Spell: Player %u have learned spell %u from NpcGUID=%u", player->GetGUIDLow(), spellToLearn, m_caster->GetGUIDLow() );
}

void Spell::EffectDispel(uint32 i) // PBW
{
    for(int n = 0; n < (m_spellInfo->EffectBasePoints[i]+1); n++){
        if(m_spellInfo->rangeIndex == 1)
        { 
            //ToDo: Shaman Totems (Poison Cleansing Totem[8168] and Disease Cleansing Totem[8171]) are SelfOnly spells
			//      and will dispel one poison/disease from one party member every 5 second that activated ...
            m_caster->RemoveFirstAuraByDispel(m_spellInfo->EffectMiscValue[i], m_caster);
            sLog.outDebug("Spell: Removed aura type %u from caster", m_spellInfo->EffectMiscValue[i]);
        }
        else
        {
            for(std::list<uint64>::iterator iunit= m_targetUnitGUIDs[i].begin();iunit != m_targetUnitGUIDs[i].end();++iunit)
            {
                Unit* unit = m_caster->GetGUID()==*iunit ? m_caster : ObjectAccessor::Instance().GetUnit(*m_caster,*iunit);
                if(unit && unit->isAlive() && (unit->GetTypeId() == TYPEID_PLAYER || unit->GetTypeId() == TYPEID_UNIT))
                {
                    unit->RemoveFirstAuraByDispel(m_spellInfo->EffectMiscValue[i], m_caster);
                    sLog.outDebug("Spell: Removed aura type %u from %u", m_spellInfo->EffectMiscValue[i], unit->GetGUIDLow());
                }
            }
        }
    }
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
        int32 chance = 10 + m_caster->getLevel() - unitTarget->getLevel();

        if (chance > irand(0, 19))
        {
            //Stealing successful
            //sLog.outDebug("Sending loot from pickpocket");
            ((Player*)m_caster)->SendLoot(unitTarget->GetGUID(),LOOT_PICKPOCKETING);
        }
        else
        {
            //Reveal action + get attack
            m_caster->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
            if (((Creature*)unitTarget)->AI())
                ((Creature*)unitTarget)->AI()->AttackStart(m_caster);
        }
    }
}

void Spell::EffectAddFarsight(uint32 i)
{
    float radius = GetRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
    int32 duration = GetDuration(m_spellInfo);
    DynamicObject* dynObj = new DynamicObject(m_caster);
    if(!dynObj->Create(objmgr.GenerateLowGuid(HIGHGUID_DYNAMICOBJECT), m_caster, m_spellInfo->Id, i, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, duration, radius))
    {
        delete dynObj;
        return;
    }
    dynObj->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    dynObj->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x80000002);
    m_caster->AddDynObject(dynObj);
    dynObj->AddToWorld();
    MapManager::Instance().GetMap(dynObj->GetMapId(), dynObj)->Add(dynObj);
    m_caster->SetUInt64Value(PLAYER_FARSIGHT, dynObj->GetGUID());
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
        TypeContainerVisitor<MaNGOS::UnitSearcher<PetWithIdCheck>, WorldTypeMapContainer > object_checker(checker);
        CellLock<GridReadGuard> cell_lock(cell, p);
        cell_lock->Visit(cell_lock, object_checker, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
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

        Pet* spawnCreature = new Pet(m_caster, GUARDIAN_PET);

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

        // set timer for unsummon
        int32 duration = GetDuration(m_spellInfo);
        if(duration > 0)
            spawnCreature->SetDuration(duration);

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

        /* not set name for guardians/minpets
        std::string name;
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            name = ((Player*)m_caster)->GetName();
        else
            name = ((Creature*)m_caster)->GetCreatureInfo()->Name;
        name.append(petTypeSuffix[spawnCreature->getPetType()]);
        spawnCreature->SetName( name );
        */

        ObjectAccessor::Instance().AddPet(spawnCreature);
        spawnCreature->AddToWorld();
        MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster)->Add((Creature*)spawnCreature);
        /*
                guardians and wilds can't be controlled
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

    float fx,fy,fz;
    m_caster->GetClosePoint(NULL,fx,fy,fz,unitTarget->GetObjectSize() + dis);

    // teleport a bit above terrain level to avoid falling below it
    fz = MapManager::Instance ().GetMap(mapid, m_caster)->GetHeight(fx,fy) + 1.5;

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        ((Player*)unitTarget)->TeleportTo(mapid, fx, fy, fz, -m_caster->GetOrientation(), false);
    else
        MapManager::Instance().GetMap(mapid, m_caster)->CreatureRelocation((Creature*)m_caster, fx, fy, fz, -m_caster->GetOrientation());
}

void Spell::EffectLearnSkill(uint32 i)
{
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    if(damage < 0)
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
            sLog.outCommand("GM %s (Account: %u) enchanting(perm): %s (Entry: %d) for player: %s (Account: %u)",
                p_caster->GetName(),p_caster->GetSession()->GetAccountId(),
                itemTarget->GetProto()->Name1,itemTarget->GetEntry(),
                item_owner->GetName(),item_owner->GetSession()->GetAccountId());

        // remove old enchanting before applying new if equipped
        item_owner->ApplyEnchantment(itemTarget,PERM_ENCHANTMENT_SLOT,false);

        itemTarget->SetEchantment(PERM_ENCHANTMENT_SLOT, enchant_id, 0, 0);

        // add new enchanting if equipped
        item_owner->ApplyEnchantment(itemTarget,PERM_ENCHANTMENT_SLOT,true);
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
            sLog.outCommand("GM %s (Account: %u) enchanting(temp): %s (Entry: %d) for player: %s (Account: %u)",
                p_caster->GetName(),p_caster->GetSession()->GetAccountId(),
                itemTarget->GetProto()->Name1,itemTarget->GetEntry(),
                item_owner->GetName(),item_owner->GetSession()->GetAccountId());

        // remove old enchanting before applying new if equipped
        item_owner->ApplyEnchantment(itemTarget,TEMP_ENCHANTMENT_SLOT,false);

        uint32 charges = m_spellInfo->SpellFamilyName == 8 ? 45+objmgr.GetSpellRank(m_spellInfo->Id)*15 : 0;

        itemTarget->SetEchantment(TEMP_ENCHANTMENT_SLOT, enchant_id, duration*1000, charges);

        // add new enchanting if equipped
        item_owner->ApplyEnchantment(itemTarget,TEMP_ENCHANTMENT_SLOT,true);
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

        creatureTarget->CombatStop(true);
        creatureTarget->StopMoving();

        // cast finish successfully
        SendChannelUpdate(0);
        finish();

        Pet* pet = new Pet(m_caster, HUNTER_PET);

        if(!pet->CreateBaseAtCreature(creatureTarget))
        {
            delete pet;
            return;
        }

        pet->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, m_caster->GetGUID());
        pet->SetUInt64Value(UNIT_FIELD_CREATEDBY, m_caster->GetGUID());
        pet->SetMaxPower(POWER_HAPPINESS,1000000);
        pet->SetPower(   POWER_HAPPINESS,600000);
        pet->setPowerType(POWER_FOCUS);
        pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->getFaction());
        pet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,time(NULL));
        pet->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
        pet->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
                                                            // + UNIT_FLAG_RENAME + UNIT_FLAG_RESTING);
        pet->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN1);
                                                            // this enables popup window (pet details, abandon, rename)

        uint32 new_id = 1;
        QueryResult* result = sDatabase.Query("SELECT MAX(`id`) FROM `character_pet`");
        if(result)
        {
            Field *fields = result->Fetch();
            new_id = fields[0].GetUInt32()+1;
            delete result;
        }

        pet->SetUInt32Value(UNIT_FIELD_PETNUMBER,new_id);
                                                            // this enables pet details window (Shift+P)
        pet->AIM_Initialize();

        ObjectAccessor::Instance().AddPet(pet);
        pet->AddToWorld();
        MapManager::Instance().GetMap(pet->GetMapId(), pet)->Add((Creature*)pet);

        ObjectAccessor::Instance().RemoveCreatureCorpseFromPlayerView(creatureTarget);
        creatureTarget->setDeathState(JUST_DIED);

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

            MapManager::Instance().GetMap(OldSummon->GetMapId(), OldSummon)->Remove((Creature*)OldSummon,false);
            OldSummon->SetMapId(m_caster->GetMapId());
            OldSummon->Relocate(px, py, pz, OldSummon->GetOrientation());
            MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster)->Add((Creature*)OldSummon);

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

    Pet* NewSummon = new Pet(m_caster, m_caster->getClass() == CLASS_HUNTER ? HUNTER_PET : SUMMON_PET);

    // petentry==0 for hunter "call pet" (current pet summoned if any)
    if(NewSummon->LoadPetFromDB(m_caster,petentry))
        return;

    // not error in case fail hunter call pet
    if(!petentry)
    {
        delete NewSummon;
        return;
    }

    CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(petentry);

    if(!cInfo)
    {
        sLog.outError("EffectSummonPet: creature entry %u not found.",petentry);
        return;
    }

    if( NewSummon->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),  m_caster->GetMapId(), px, py, pz+1, m_caster->GetOrientation(), petentry))
    {
        uint32 petlevel = m_caster->getLevel();

        NewSummon->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, m_caster->GetGUID());
        NewSummon->SetUInt64Value(UNIT_FIELD_CREATEDBY, m_caster->GetGUID());
        NewSummon->SetUInt32Value(UNIT_NPC_FLAGS , 0);
        NewSummon->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->getFaction());
        NewSummon->SetUInt32Value(UNIT_FIELD_BYTES_0,2048);
        NewSummon->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
        NewSummon->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,time(NULL));
        NewSummon->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
        NewSummon->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
        NewSummon->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);

        uint32 new_id = 1;
        QueryResult* result = sDatabase.Query("SELECT MAX(`id`) FROM `character_pet`");
        if(result)
        {
            Field *fields = result->Fetch();
            new_id = fields[0].GetUInt32()+1;
            delete result;
        }

        NewSummon->SetUInt32Value(UNIT_FIELD_PETNUMBER,new_id);
                                                            // this enables pet details window (Shift+P)

        // this enables popup window (pet dismiss, cancel), hunter pet additional flags set later
        NewSummon->SetUInt32Value(UNIT_FIELD_FLAGS,UNIT_FLAG_UNKNOWN1);

        NewSummon->InitStatsForLevel( petlevel);

        for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
            NewSummon->m_spells[i] = 0;

        // starting spells
        switch(petentry)
        {
            case 416:
                NewSummon->m_spells[0] = 3110;
                NewSummon->AddActState(STATE_RA_SPELL1);
                break;
            case 417:
                NewSummon->m_spells[0] = 19505;
                NewSummon->AddActState(STATE_RA_SPELL1);
                break;
            case 1860:
                NewSummon->m_spells[0] = 3716;
                NewSummon->AddActState(STATE_RA_SPELL1);
                break;
            case 1863:
                NewSummon->m_spells[0] = 7814;
                NewSummon->AddActState(STATE_RA_SPELL1);
                break;
        }

        // generate new name for summon pet
        if(NewSummon->getPetType()==SUMMON_PET)
        {
            std::string new_name=objmgr.GeneratePetName(petentry);
            if(new_name!="")
                NewSummon->SetName(new_name);
        }
        else if(NewSummon->getPetType()==HUNTER_PET)
        {
            // this enables popup window (pet details, abandon, rename)
            //NewSummon->SetFlag(UNIT_FIELD_FLAGS,(UNIT_FLAG_RESTING | UNIT_FLAG_RENAME)); // changed in 2.0.8?
                                                            // check it!
            NewSummon->SetUInt32Value(UNIT_FIELD_BYTES_2, uint32(2 << 16));
        }

        NewSummon->AIM_Initialize();

        ObjectAccessor::Instance().AddPet(NewSummon);
        NewSummon->AddToWorld();
        MapManager::Instance().GetMap(NewSummon->GetMapId(), NewSummon)->Add((Creature*)NewSummon);

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
        if (((Creature*)unitTarget)->AI())
            ((Creature*)unitTarget)->AI()->AttackStart(m_caster);
    }
}

void Spell::EffectWeaponDmg(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    // Bloodthirst
    uint32 BTAura = 0;
    if(m_spellInfo->Category == 971 && m_spellInfo->SpellVisual == 372)
    {
        switch(m_spellInfo->Id)
        {
            case 23881: BTAura = 23885; break;
            case 23892: BTAura = 23886; break;
            case 23893: BTAura = 23887; break;
            case 23894: BTAura = 23888; break;
            case 25251: BTAura = 25252; break;
            case 30335: BTAura = 30339; break;
            default: break;
        }
        // FIX_ME: Where this value used???
        damage = uint32(0.45 * (m_caster->GetUInt32Value(UNIT_FIELD_ATTACK_POWER) + m_caster->GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS)));
    }

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
    uint32 blocked_dmg = 0;
    uint32 absorbed_dmg = 0;
    uint32 resisted_dmg = 0;
    bool criticalhit = false;

    if( unitTarget->IsImmunedToPhysicalDamage() )
    {
        m_caster->SendAttackStateUpdate (HITINFO_MISS, unitTarget, 1, NORMAL_DAMAGE, 0, 0, 0, VICTIMSTATE_IS_IMMUNE, 0);
        return;
    }

    //set base eff_damage, total normal hit damage after DoAttackDamage call will be bonus + weapon
    //if miss/parry, no eff=0 automatically by func DoAttackDamage
    //if crit eff = (bonus + weapon) * 2
    //In a word, bonus + weapon will be calculated together in cases of miss, armor reduce, crit, etc.
    uint32 eff_damage = bonus;
    m_caster->DoAttackDamage(unitTarget, &eff_damage, &blocked_dmg, &damageType, &hitInfo, &victimState, &absorbed_dmg, &resisted_dmg, attType, m_spellInfo, m_IsTriggeredSpell);

    for (j = 0; j < 3; j++)
        if (m_spellInfo->Effect[j] == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE)
            eff_damage = uint32(eff_damage * (m_spellInfo->EffectBasePoints[j]+1) / 100);

    if ((hitInfo & nohitMask) && attType != RANGED_ATTACK)  // not send ranged miss/etc
        m_caster->SendAttackStateUpdate(hitInfo & nohitMask, unitTarget, 1, m_spellInfo->School, eff_damage, absorbed_dmg, resisted_dmg, 1, blocked_dmg);

    if(hitInfo & HITINFO_CRITICALHIT)
        criticalhit = true;

    m_caster->SendSpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, eff_damage, m_spellInfo->School, absorbed_dmg, resisted_dmg, false, blocked_dmg, criticalhit);

    // Bloodthirst
    if (BTAura)
        m_caster->CastSpell(m_caster,BTAura,true);

    if (eff_damage > (absorbed_dmg + resisted_dmg + blocked_dmg))
        eff_damage -= (absorbed_dmg + resisted_dmg + blocked_dmg);
    else
        eff_damage = 0;

    m_caster->DealDamage(unitTarget, eff_damage, SPELL_DIRECT_DAMAGE, 0, NULL, 0, true);

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
                ((Player*)m_caster)->DestroyItemCount(ammo, 1, true);
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

    if(!unitTarget->CanHaveThreatList())
        return;

    unitTarget->AddThreat(m_caster, float(damage));
}

void Spell::EffectHealMaxHealth(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    uint32 heal = m_caster->GetMaxHealth();

    if(m_spellInfo->SpellVisual == 132)                     // drain all caster's mana
        m_caster->SetPower(POWER_MANA, 0);

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

    // TODO: not all spells that used this effect apply cooldown at school spells
    // also exist case: apply cooldown to interrupted cast only and to all spells
    if (unitTarget->m_currentSpell && unitTarget->m_currentSpell->m_spellInfo)
    {
        unitTarget->ProhibitSpellScholl(unitTarget->m_currentSpell->m_spellInfo->School, GetDuration(m_spellInfo));
        unitTarget->InterruptSpell();
    }
}

void Spell::EffectSummonObjectWild(uint32 i)
{
    GameObject* pGameObj = new GameObject(m_caster);

    uint32 gameobject_id = m_spellInfo->EffectMiscValue[i];

    WorldObject* target = focusObject;
    if( !target )
        target = m_caster;

    // before caster
    float x,y,z;
    m_caster->GetClosePoint(NULL,x,y,z);

    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), gameobject_id, target->GetMapId(),
        x, y, z, target->GetOrientation(), 0, 0, 0, 0, 100, 0))
    {
        delete pGameObj;
        return;
    }

    int32 duration = GetDuration(m_spellInfo);
    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);

    if(pGameObj->GetGoType() != GAMEOBJECT_TYPE_FLAGDROP)   // make dropped flag clickable for other players (not set owner guid (created by) for this)...
        m_caster->AddGameObject(pGameObj);
    pGameObj->AddToWorld();
    MapManager::Instance().GetMap(pGameObj->GetMapId(), pGameObj)->Add(pGameObj);
}

void Spell::EffectScriptEffect(uint32 i)
{
    // we must implement hunter pet summon at login there (spell 6962)

    if(!m_spellInfo->Reagent[0])
    {
        // paladin's holy light / flash of light
        if ((m_spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN) &&
            (m_spellInfo->SpellIconID == 70 || m_spellInfo->SpellIconID  == 242))
            EffectHeal( i );

        if(m_spellInfo->SpellVisual == 5651 && m_spellInfo->SpellIconID == 205)
        {
            // paladin's judgement
            if(!unitTarget || !unitTarget->isAlive())
                return;
            uint32 spellId2 = 0;

            // all seals have aura dummy
            Unit::AuraList& m_dummyAuras = m_caster->GetAurasByType(SPELL_AURA_DUMMY);

            for(Unit::AuraList::iterator itr = m_dummyAuras.begin(); itr != m_dummyAuras.end(); ++itr)
            {
                SpellEntry const *spellInfo = (*itr)->GetSpellProto();

                // search seal (all seals have judgement's aura dummy spell id in 2 effect
                if ( !spellInfo || !IsSealSpell((*itr)->GetId()) || (*itr)->GetEffIndex() != 2 )
                    continue;

                spellId2 = spellInfo->EffectBasePoints[(*itr)->GetEffIndex()]+1;

                if(spellId2 <= 1)
                    continue;

                // found, remove seal
                m_caster->RemoveAurasDueToSpell((*itr)->GetId());
                break;
            }

            SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId2);
            if(!spellInfo)
                return;
            Spell spell(m_caster,spellInfo,true,0);

            SpellCastTargets targets;
            targets.setUnitTarget(unitTarget);
            spell.prepare(&targets);
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

void Spell::EffectSanctuary(uint32 i)
{
    if(!unitTarget)
        return;
    unitTarget->CombatStop();
}

void Spell::EffectAddComboPoints(uint32 i)
{
    if(!unitTarget)
        return;

    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if(damage < 0)
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
    GameObject* pGameObj = new GameObject(m_caster);

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
    pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, GAMEOBJECT_TYPE_DUEL_ARBITER );
    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel()+1 );
    int32 duration = GetDuration(m_spellInfo);
    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);

    m_caster->AddGameObject(pGameObj);
    pGameObj->AddToWorld();
    MapManager::Instance().GetMap(pGameObj->GetMapId(), pGameObj)->Add(pGameObj);
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

void Spell::EffectStuck(uint32 i)
{
    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    sLog.outDebug("Spell Effect: Stuck");
    sLog.outCommand("Player %s (guid %u) used auto-unstuck future at map %u (%f, %f, %f)", m_caster->GetName(), m_caster->GetGUIDLow(), m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ());
    HandleTeleport(m_spellInfo->Id, m_caster);

    // from patch 2.0.12 Stuck spell trigger Hearthstone cooldown
    //((Player*)m_caster)->AddSpellCooldown(8690, time(NULL) + 3600); // add 1 hour cooldown to Hearthstone
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(8690);
    if(!spellInfo)
        return;
    Spell spell(m_caster,spellInfo,true,0);
    spell.SendSpellCooldown();
}

void Spell::EffectSummonPlayer(uint32 i)
{
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if(unitTarget->isInFlight())
        return;

    //FIXME: must send accepting request for summon to target instead explicit teleportation
    // SMSG_SUMMON_REQUEST (683)
    // uint64 summoner_guid
    // uint32 summon_zoneid
    // uint32 ms_time (auto decline?)
    // CMSG_SUMMON_RESPONSE - empty

    // before caster
    float x,y,z;
    m_caster->GetClosePoint(NULL,x,y,z);

    ((Player*)unitTarget)->TeleportTo(m_caster->GetMapId(), x, y, z,unitTarget->GetOrientation());
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

    Map* map = MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster);
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

    Totem* pTotem = new Totem(m_caster);

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
    pTotem->SetMaxHealth(m_spellInfo->EffectBasePoints[i]+1);
    pTotem->SetHealth(m_spellInfo->EffectBasePoints[i]+1);
    pTotem->Summon();
}

void Spell::EffectEnchantHeldItem(uint32 i)
{
    // this is only item spell effect applied to main-hand weapon of target player (players in area)
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* item_owner = (Player*)unitTarget;
    Item* item = item_owner->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

    if(!item )
        return;

    // must be equipped
    if(!item ->IsEquipped())
        return;

    if (m_spellInfo->EffectMiscValue[i])
    {
        uint32 enchant_id = m_spellInfo->EffectMiscValue[i];
        int32 duration = GetDuration(m_spellInfo);
        if(duration == 0)
            duration = m_spellInfo->EffectBasePoints[i]+1;
        if(duration == 1)
            duration = 300;

        SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if(!pEnchant)
            return;

        EnchantmentSlot slot = duration <= 0 ? HELD_PERM_ENCHANTMENT_SLOT : HELD_TEMP_ENCHANTMENT_SLOT;
        
        // remove old enchanting before applying new
        item_owner->ApplyEnchantment(item,slot,false);

        item->SetEchantment(slot, enchant_id, duration*1000, 0);

        // add new enchanting
        item_owner->ApplyEnchantment(item,slot,true);
    }
}

void Spell::EffectDisEnchant(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = (Player*)m_caster;
    if(!itemTarget || !itemTarget->GetProto()->DisenchantID)
        return;

    p_caster->UpdateCraftSkill(m_spellInfo->Id);

    ((Player*)m_caster)->SendLoot(itemTarget->GetGUID(),LOOT_DISENCHANTING);

    // item will be removed at disenchanting end
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

    if(!itemTarget)
        return;

    Creature *pet = _player->GetPet();
    if(!pet)
        return;

    if(!pet->isAlive())
        return;

    pet->ModifyPower(POWER_HAPPINESS,damage);

    uint32 count = 1;
    _player->DestroyItemCount(itemTarget,count,true);

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

    ((Player*)m_caster)->RemovePet(pet,PET_SAVE_NOT_IN_SLOT);
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

    GameObject* pGameObj = new GameObject(m_caster);
    uint32 display_id = m_spellInfo->EffectMiscValue[i];

    float rot2 = sin(m_caster->GetOrientation()/2);
    float rot3 = cos(m_caster->GetOrientation()/2);

    float x,y,z;
    m_caster->GetClosePoint(NULL,x,y,z);

    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), display_id,m_caster->GetMapId(), x, y, z, m_caster->GetOrientation(), 0, 0, rot2, rot3, 0, 0))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL,m_caster->getLevel());
    int32 duration = GetDuration(m_spellInfo);
    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);
    pGameObj->SetLootState(GO_CLOSED);
    m_caster->AddGameObject(pGameObj);

    pGameObj->AddToWorld();
    MapManager::Instance().GetMap(pGameObj->GetMapId(), pGameObj)->Add(pGameObj);
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

        // before caster
        float fx,fy,fz;
        m_caster->GetClosePoint(NULL,fx,fy,fz,dis);

        // teleport a bit above terrain level to avoid falling below it
        fz = MapManager::Instance ().GetMap(mapid, m_caster)->GetHeight(fx,fy) + 1.5;

        if(unitTarget->GetTypeId() == TYPEID_PLAYER)
            ((Player*)unitTarget)->TeleportTo(mapid, fx, fy, fz, m_caster->GetOrientation(), false);
        else
            MapManager::Instance().GetMap(mapid, m_caster)->CreatureRelocation((Creature*)m_caster, fx, fy, fz, m_caster->GetOrientation());
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
        if(damage < 0) return;
        health = uint32(damage/100*unitTarget->GetMaxHealth());
        if(unitTarget->getPowerType() == POWER_MANA)
            mana = uint32(damage/100*unitTarget->GetMaxPower(unitTarget->getPowerType()));
    }

    Player *plr = ((Player*)unitTarget);
    plr->ResurrectPlayer();

    if(plr->GetMaxHealth() > health)
        plr->SetHealth( health );
    else
        plr->SetHealth( plr->GetMaxHealth() );

    if(plr->GetMaxPower(POWER_MANA) > mana)
        plr->SetPower(POWER_MANA, mana );
    else
        plr->SetPower(POWER_MANA, plr->GetMaxPower(POWER_MANA) );

    plr->SetPower(POWER_RAGE, 0 );

    plr->SetPower(POWER_ENERGY, plr->GetMaxPower(POWER_ENERGY) );

    plr->SpawnCorpseBones();

    // we really need this teleport?
    WorldPacket data;
    plr->BuildTeleportAckMsg(&data, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), plr->GetOrientation());
    plr->GetSession()->SendPacket(&data);

    plr->SetPosition(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), plr->GetOrientation());

    plr->SaveToDB();
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
    unitTarget->GetContactPoint(m_caster, x, y, z);
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        ((Creature *)unitTarget)->StopMoving();

    m_caster->SendMonsterMove(x, y, z, 0, true,1);

    m_caster->Attack(unitTarget,true);
}

void Spell::EffectSummonCritter(uint32 i)
{
    uint32 pet_entry = m_spellInfo->EffectMiscValue[i];
    if(!pet_entry)
        return;

    Unit* old_critter = NULL;

    {
        CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
        Cell cell = RedZone::GetZone(p);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();

        PetWithIdCheck u_check(m_caster, pet_entry);
        MaNGOS::UnitSearcher<PetWithIdCheck> checker(old_critter, u_check);
        TypeContainerVisitor<MaNGOS::UnitSearcher<PetWithIdCheck>, WorldTypeMapContainer > object_checker(checker);
        CellLock<GridReadGuard> cell_lock(cell, p);
        cell_lock->Visit(cell_lock, object_checker, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
    }

    if (old_critter)                                        // find old critter, unsummon
    {
        // PetWithIdCheck return only Pets
        ((Pet*)old_critter)->Remove(PET_SAVE_AS_DELETED);
        return;
    }
    else                                                    // in another case summon new
    {
        Pet* critter = new Pet(m_caster, MINI_PET);

        // before caster
        float x,y,z;
        m_caster->GetClosePoint(NULL,x,y,z);

        if(!critter->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),
            m_caster->GetMapId(),x,y,z,m_caster->GetOrientation(),m_spellInfo->EffectMiscValue[i]))
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

        ObjectAccessor::Instance().AddPet(critter);
        critter->AddToWorld();
        MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster)->Add((Creature*)critter);
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
    if (randomPoints) value = basePoints + irand(1, randomPoints);
    else value = basePoints +1;

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
    if(damage < 0)
        return;
    pet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
    pet->RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
    pet->setDeathState( ALIVE );
    pet->clearUnitState(UNIT_STAT_ALL_STATE);
    pet->SetHealth( uint32(pet->GetMaxHealth()*(float(damage)/100)));

    pet->AIM_Initialize();

    _player->PetSpellInitialize();
    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
}

void Spell::EffectTransmitted(uint32 i)
{
    float min_dis = GetMinRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));
    float max_dis = GetMaxRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));
    float dis = rand_norm() * (max_dis - min_dis) + min_dis;

    float fx,fy,fz;
    m_caster->GetClosePoint(NULL,fx,fy,fz,dis);

    if(m_spellInfo->EffectMiscValue[i] == 35591)
    {
        Map* map = MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster);

        if ( !map->IsInWater(fx,fy) )
        {
            SendCastResult(SPELL_FAILED_NOT_HERE);
            SendChannelUpdate(0);
            return;
        }

        // replace by water level in this case
        fz = map->GetWaterLevel(fx,fy);
    }

    GameObject* pGameObj = new GameObject(m_caster);
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

        // end time of range when possible catch fish (FISHING_BOBBER_READY_TIME..GetDuration(m_spellInfo))
        // start time == fish-FISHING_BOBBER_READY_TIME (0..GetDuration(m_spellInfo)-FISHING_BOBBER_READY_TIME)
        uint32 fish = urand(FISHING_BOBBER_READY_TIME,GetDuration(m_spellInfo)/1000);
        pGameObj->SetRespawnTime(fish);
    }
    else
    {
        pGameObj->SetOwnerGUID(m_caster->GetGUID() );
        int32 duration = GetDuration(m_spellInfo);
        pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);
    }

    //pGameObj->SetUInt32Value(12, 0x3F63BB3C ); // useless rotation
    //pGameObj->SetUInt32Value(13, 0xBEE9E017 ); // useless rotation
    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel() );
    pGameObj->SetSpellId(m_spellInfo->Id);

    DEBUG_LOG("AddObject at SpellEfects.cpp EffectTransmitted\n");
    //m_caster->AddGameObject(pGameObj);
    //m_ObjToDel.push_back(pGameObj);

    pGameObj->AddToWorld();
    MapManager::Instance().GetMap(pGameObj->GetMapId(), pGameObj)->Add(pGameObj);

    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM, 8);
    data << uint64(pGameObj->GetGUID());
    m_caster->SendMessageToSet(&data,true);
}

void Spell::EffectSkill(uint32 i)
{
    sLog.outDebug("WORLD: SkillEFFECT");
}
