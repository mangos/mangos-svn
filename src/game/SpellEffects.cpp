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
    &Spell::EffectWeaponDmgNOSchool,                        //SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL = 17
    &Spell::EffectResurrect,                                //SPELL_EFFECT_RESURRECT = 18
    &Spell::EffectNULL,                                     //SPELL_EFFECT_ADD_EXTRA_ATTACKS = 19
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DODGE = 20
    &Spell::EffectNULL,                                     //SPELL_EFFECT_EVADE = 21
    &Spell::EffectNULL,                                     //SPELL_EFFECT_PARRY = 22
    &Spell::EffectNULL,                                     //SPELL_EFFECT_BLOCK = 23
    &Spell::EffectCreateItem,                               //SPELL_EFFECT_CREATE_ITEM = 24
    &Spell::EffectNULL,                                     //SPELL_EFFECT_WEAPON = 25
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DEFENSE = 26
    &Spell::EffectPresistentAA,                             //SPELL_EFFECT_PERSISTENT_AREA_AURA = 27
    &Spell::EffectSummon,                                   //SPELL_EFFECT_SUMMON = 28
    &Spell::EffectMomentMove,                               //SPELL_EFFECT_LEAP = 29
    &Spell::EffectEnergize,                                 //SPELL_EFFECT_ENERGIZE = 30
    &Spell::EffectWeaponDmgPerc,                            //SPELL_EFFECT_WEAPON_PERCENT_DAMAGE = 31
    &Spell::EffectNULL,                                     //SPELL_EFFECT_TRIGGER_MISSILE = 32 //Useless
    &Spell::EffectOpenLock,                                 //SPELL_EFFECT_OPEN_LOCK = 33
    &Spell::EffectSummonChangeItem,                         //SPELL_EFFECT_SUMMON_MOUNT_OBSOLETE = 34
    &Spell::EffectApplyAA,                                  //SPELL_EFFECT_APPLY_AREA_AURA = 35
    &Spell::EffectLearnSpell,                               //SPELL_EFFECT_LEARN_SPELL = 36
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SPELL_DEFENSE = 37 //Useless
    &Spell::EffectDispel,                                   //SPELL_EFFECT_DISPEL = 38
    &Spell::EffectNULL,                                     //SPELL_EFFECT_LANGUAGE = 39
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DUAL_WIELD = 40
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
    &Spell::EffectNULL,                                     //SPELL_EFFECT_PROFICIENCY = 60
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
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_OBJECT_WILD = 76
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
    &Spell::EffectNULL,                                     //SPELL_EFFECT_KNOCK_BACK = 98
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
    &Spell::EffectWeaponDmg                                 //SPELL_EFFECT_ADICIONAL_DMG = 121
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

    uint32 health = m_spellInfo->EffectBasePoints[i];
    uint32 mana = m_spellInfo->EffectMiscValue[i];
    ((Player*)unitTarget)->setResurrect(m_caster->GetGUID(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
    SendResurrectRequest((Player*)unitTarget);
}

void Spell::EffectInstaKill(uint32 i)
{
    if( unitTarget && unitTarget->isAlive() )
    {
        uint32 health = unitTarget->GetHealth();
        m_caster->DealDamage(unitTarget, health, 0, false);
    }
}

void Spell::EffectSchoolDMG(uint32 i)
{
    if( unitTarget && unitTarget->isAlive() )
        m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, damage);
}

void Spell::EffectDummy(uint32 i)
{
    if(!unitTarget)
        return;
    // starshards/curse of agony hack .. this applies to 1.10 only
    if (m_triggeredByAura)
    {
        SpellEntry *trig_info = m_triggeredByAura->GetSpellProto();
        if ((trig_info->SpellIconID == 1485 && trig_info->SpellFamilyName == SPELLFAMILY_PRIEST) ||
            (trig_info->SpellIconID == 544 && trig_info->SpellFamilyName == SPELLFAMILY_WARLOCK))
        {
            Unit *tmpTarget = unitTarget;
            unitTarget = m_triggeredByAura->GetTarget();
            damage = trig_info->EffectBasePoints[i];
            EffectSchoolDMG(i);
            unitTarget = tmpTarget;
        }
    }
    if(m_spellInfo->SpellIconID == 1648)
    {
        uint32 dmg = damage;
        dmg += uint32(m_caster->GetPower(POWER_RAGE)/10 * FindSpellRank(m_spellInfo->Id)*3);
        m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, dmg);
        m_caster->SetPower(POWER_RAGE,0);
    }
}

void Spell::EffectTriggerSpell(uint32 i)
{
    SpellEntry *spellInfo = sSpellStore.LookupEntry( m_spellInfo->EffectTriggerSpell[i] );

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
    uint8 castResult = 0;
    //If m_immuneToState type contain this aura type, IMMUNE aura.
    for (SpellImmuneList::iterator itr = unitTarget->m_spellImmune[IMMUNITY_EFFECT].begin(), next; itr != unitTarget->m_spellImmune[IMMUNITY_EFFECT].end(); itr = next)
    {
        next = itr;
        next++;
        if((*itr)->type == m_spellInfo->EffectApplyAuraName[i])
        {
            castResult = CAST_FAIL_IMMUNE;
            break;
        }
    }
    if(castResult)
    {
        SendCastResult(castResult);
        return;
    }
    sLog.outDebug("Apply Auraname is: %u", m_spellInfo->EffectApplyAuraName[i]);

    Aura* Aur = new Aura(m_spellInfo, i, m_caster, unitTarget);
    if(m_CastItem)
        Aur->SetCastItem(m_CastItem);
    unitTarget->AddAura(Aur);
    if (Aur && Aur->IsTrigger())
    {
        // arcane missiles
        SpellEntry *spellInfo = sSpellStore.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
        if (!spellInfo) return;
        if (spellInfo->EffectImplicitTargetA[0] == TARGET_S_E && m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            FactionTemplateResolver my_faction = m_caster->getFactionTemplateEntry();
            Unit *target = ObjectAccessor::Instance().GetUnit(*m_caster, ((Player*)m_caster)->GetSelection());
            if (target)
            {
                FactionTemplateResolver sel_faction = target->getFactionTemplateEntry();
                if (!my_faction.IsFriendlyTo(sel_faction))
                    Aur->SetTarget(target);
                else
                    cancel();
            }
            else
                cancel();
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

    uint32 curPower = unitTarget->GetPower(drain_power);
    float tmpvalue = m_spellInfo->EffectMultipleValue[i];
    if(!tmpvalue)
        tmpvalue = 1;
    if(curPower < damage)
    {
        unitTarget->SetPower(drain_power,0);
        if(drain_power == POWER_MANA)
            m_caster->SetPower(POWER_MANA,uint32(m_caster->GetPower(POWER_MANA)+curPower*tmpvalue));
    }
    else
    {
        unitTarget->SetPower(drain_power,curPower-damage);
        if(drain_power == POWER_MANA)
            m_caster->SetPower(POWER_MANA,uint32(m_caster->GetPower(POWER_MANA)+damage*tmpvalue));
    }
}

void Spell::EffectSendEvent(uint32 i)
{
}

void Spell::EffectPowerDrain(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    uint32 curPower = unitTarget->GetPower(POWER_MANA);
    uint32 curHealth = unitTarget->GetHealth();
    uint32 caster_curPower = m_caster->GetPower(POWER_MANA);
    uint32 tmpvalue = 0;
    if(curPower < damage)
    {
        unitTarget->SetPower(POWER_MANA,0);
        tmpvalue = uint32(curPower*m_spellInfo->EffectMultipleValue[i]);
    }
    else
    {
        tmpvalue = uint32(damage*m_spellInfo->EffectMultipleValue[i]);
        unitTarget->SetPower(POWER_MANA,curPower-damage);
    }
    if(caster_curPower + tmpvalue < m_caster->GetMaxPower(POWER_MANA))
        m_caster->SetPower(POWER_MANA,caster_curPower + tmpvalue);
    else m_caster->SetPower(POWER_MANA,m_caster->GetMaxPower(POWER_MANA));
    m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, damage);

}

void Spell::EffectHeal( uint32 i )
{
    if( unitTarget && unitTarget->isAlive() )
    {
        float pct = (100+unitTarget->m_RegenPCT)/100;
        uint32 curhealth = unitTarget->GetHealth();
        uint32 maxhealth = unitTarget->GetMaxHealth();
        uint32 addhealth = ( curhealth + damage*pct < maxhealth ? uint32(damage*pct) : maxhealth - curhealth );
        unitTarget->SetHealth( curhealth + addhealth );
        if(unitTarget->GetTypeId() == TYPEID_PLAYER)
            SendHealSpellOnPlayer(((Player*)unitTarget), m_spellInfo->Id, addhealth);

        //If the target is in combat, then player is in combat too
        if( m_caster != unitTarget &&
            m_caster->GetTypeId() == TYPEID_PLAYER &&
            unitTarget->GetTypeId() == TYPEID_PLAYER &&
            unitTarget->isInCombatWithPlayer() )
        {
            ((Player*)m_caster)->SetPvP(true);
        }

    }
}

void Spell::EffectHealthLeach(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    sLog.outDebug("HealthLeach :%u", damage);

    uint32 tmpvalue = 0;
    float pct = (100+unitTarget->m_RegenPCT)/100;

    if(unitTarget->GetHealth() - damage > 0)
    {
        tmpvalue = uint32(damage*m_spellInfo->EffectMultipleValue[i]);
    }
    else
    {
        tmpvalue = uint32(unitTarget->GetHealth()*m_spellInfo->EffectMultipleValue[i]);
    }
    if(m_caster->GetHealth() + tmpvalue*pct < m_caster->GetMaxHealth() )
        m_caster->SetHealth(uint32(m_caster->GetHealth() + tmpvalue*pct));
    else m_caster->SetHealth(m_caster->GetMaxHealth());
    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        SendHealSpellOnPlayer(((Player*)unitTarget), m_spellInfo->Id, uint32(tmpvalue*pct));
    m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, damage);
}

void Spell::EffectWeaponDmgNOSchool(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    float minDmg,maxDmg;
    minDmg = maxDmg = 0;

    minDmg = m_caster->GetFloatValue(UNIT_FIELD_MINDAMAGE);
    maxDmg = m_caster->GetFloatValue(UNIT_FIELD_MAXDAMAGE);

    uint32 randDmg = (uint32)(maxDmg-minDmg);
    uint32 dmg = (uint32)minDmg;
    if(randDmg > 1)
        dmg += (uint32)(rand()%randDmg);
    dmg += damage;

    m_caster->SpellNonMeleeDamageLog(unitTarget,m_spellInfo->Id,dmg);
}

void Spell::EffectCreateItem(uint32 i)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = (Player*)m_caster;

    uint32 newitemid = m_spellInfo->EffectItemType[i];
    ItemPrototype *pProto = objmgr.GetItemPrototype( newitemid );
    if(!pProto)
    {
        player->SendEquipError( EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
        return;
    }

    uint32 num_to_add = ((player->getLevel() - (m_spellInfo->spellLevel-1))*2);
    if(pProto->Class != ITEM_CLASS_CONSUMABLE || m_spellInfo->SpellFamilyName != SPELLFAMILY_MAGE)
        num_to_add = 1;
    if(num_to_add > pProto->Stackable)
        num_to_add = pProto->Stackable;

    Item *pItem = player->CreateItem(newitemid, num_to_add);

    if(!pItem)
    {
        player->SendEquipError( EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
        return;
    }

    uint16 dest;
    uint8 msg = player->CanStoreItem( 0, NULL_SLOT, dest, pItem, false);
    if( msg != EQUIP_ERR_OK )
    {
        player->SendEquipError( msg, NULL, NULL );
        delete pItem;
        return;
    }

    player->StoreItem( dest, pItem, true);
    if( pItem->GetProto()->Class != ITEM_CLASS_CONSUMABLE )
        pItem->SetUInt32Value(ITEM_FIELD_CREATOR,player->GetGUIDLow());
    //should send message "create item" to client.-FIX ME
    player->UpdateSkillPro(m_spellInfo->Id);
}

void Spell::EffectPresistentAA(uint32 i)
{

    //if(m_AreaAura == true)
    //    return;

    //m_AreaAura = true;

    DynamicObject* dynObj = new DynamicObject();
    if(!dynObj->Create(objmgr.GenerateLowGuid(HIGHGUID_DYNAMICOBJECT), m_caster, m_spellInfo, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, GetDuration(m_spellInfo)))
    {
        delete dynObj;
        return;
    }
    dynObj->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    dynObj->SetUInt32Value(GAMEOBJECT_DISPLAYID, 368003);
    dynObj->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x01eeeeee);
    dynObj->PeriodicTriggerDamage(damage, m_spellInfo->EffectAmplitude[i], GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i])));
    //m_dynObjToDel.push_back(dynObj);
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
    uint32 curEnergy = unitTarget->GetPower(power);
    uint32 maxEnergy = unitTarget->GetMaxPower(power);
    if(curEnergy+damage > maxEnergy)
        unitTarget->SetPower(power,maxEnergy);
    else
        unitTarget->SetPower(power,curEnergy+damage);
}

void Spell::EffectOpenLock(uint32 i)
{

    if(!gameObjTarget)
    {
        sLog.outDebug( "WORLD: Open Lock - No GameObject Target!");
        return;
    }

    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
    {
        sLog.outDebug( "WORLD: Open Lock - No Player Caster!");
        return;
    }
    LootType loottype;
    if(m_spellInfo->EffectMiscValue[0]==LOCKTYPE_HERBALISM)
    {
        uint32 displayid= gameObjTarget->GetUInt32Value (GAMEOBJECT_DISPLAYID);
        uint32 requiredskill;
        switch(displayid)
        {
            case 269:
            case 270:
                requiredskill=1;
                break;

            case 414:
                requiredskill=15;
                break;
            case 268:
                requiredskill=50;
                break;
            case 271:
                requiredskill=70;
                break;
            case 700:
                requiredskill=85;
                break;
            case 358:
                requiredskill=100;
                break;
            case 371:
                requiredskill=115;
                break;
            case 357:
                requiredskill=120;
                break;
            case 320:
                requiredskill=125;
                break;
            case 677:
                requiredskill=150;
                break;
            case 697:
                requiredskill=160;
                break;
            case 701:
                requiredskill=185;
                break;
            case 699:
                requiredskill=195;
                break;
            case 2312:
                requiredskill=205;
                break;
            case 698:
                requiredskill=215;
                break;
            default:
                requiredskill=1;
                sLog.outString("Unknown herb %u",displayid);
                break;
        }
        if(((Player*)m_caster)->GetSkillValue(SKILL_HERBALISM)<requiredskill)
        {
            SendCastResult(CAST_FAIL_FAILED);
            return;
        }
        if(((Player*)m_caster)->GetSkillValue(SKILL_HERBALISM) >= requiredskill +75 )
            up_skillvalue = 4;
        else if(((Player*)m_caster)->GetSkillValue(SKILL_HERBALISM) >= requiredskill +50 )
            up_skillvalue = 3;
        else if(((Player*)m_caster)->GetSkillValue(SKILL_HERBALISM) >= requiredskill +25 )
            up_skillvalue = 2;
        else if(((Player*)m_caster)->GetSkillValue(SKILL_HERBALISM) >= requiredskill)
            up_skillvalue = 1;
        else up_skillvalue = 0;
        loottype = LOOT_SKINNING;

    }else if(m_spellInfo->EffectMiscValue[0]==LOCKTYPE_MINING)
    {
        uint32 id= gameObjTarget->GetGOInfo()->sound0;
        uint32 requiredskill=1;
        switch(id)
        {
            case 939:
                requiredskill=275;
                break;

            case 38:
                requiredskill=1;
                break;
            case 39:
                requiredskill=60;
                break;
            case 25:                                        //Indurium Mineral Vein

                break;
            case 40:
                requiredskill=70;
                break;

            case 42:
                requiredskill=120;
                break;

            case 400:
                requiredskill=260;
                break;

            default:
                requiredskill=1;
                sLog.outString("Unknown vein %u",id);
                break;
        }
        if(((Player*)m_caster)->GetSkillValue(SKILL_MINING) < requiredskill)
        {
            SendCastResult(CAST_FAIL_FAILED);
            return;
        }
        if(((Player*)m_caster)->GetSkillValue(SKILL_MINING) >= requiredskill +75 )
            up_skillvalue = 4;
        else if(((Player*)m_caster)->GetSkillValue(SKILL_MINING) >= requiredskill +50 )
            up_skillvalue = 3;
        else if(((Player*)m_caster)->GetSkillValue(SKILL_MINING) >= requiredskill +25 )
            up_skillvalue = 2;
        else if(((Player*)m_caster)->GetSkillValue(SKILL_MINING) >= requiredskill)
            up_skillvalue = 1;
        else up_skillvalue = 0;

        loottype = LOOT_SKINNING;
    }
    else
        loottype = LOOT_CORPSE;

    if(loottype == LOOT_CORPSE)
    {
        ((Player*)m_caster)->UpdateSkillPro(m_spellInfo->Id);
    }

    ((Player*)m_caster)->SendLoot(gameObjTarget->GetGUID(),loottype);

}

void Spell::EffectSummonChangeItem(uint32 i)
{
    if(!itemTarget)
        return;

    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 newitemid = m_spellInfo->EffectItemType[i];
    if(!newitemid)
        return;
    Player *player = (Player*)m_caster;
    Item *pItem = player->CreateItem(newitemid,1);
    uint16 dest;
    uint8 msg = player->CanStoreItem( 0, NULL_SLOT, dest, pItem, false);
    if( msg == EQUIP_ERR_OK )
    {
        player->StoreItem( dest, pItem, true);
        player->DestroyItemCount(pItem->GetEntry(),1,true);
    }
    else
    {
        delete pItem;
        player->SendEquipError( msg, NULL, NULL );
    }
}

void Spell::EffectOpenSecretSafe(uint32 i)
{
    EffectOpenLock(i);                                      //no difference for now

}

void Spell::EffectApplyAA(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    Aura* Aur = new Aura(m_spellInfo, i, m_caster, unitTarget);
    //Aur->SetModifier(m_spellInfo->EffectApplyAuraName[i],m_spellInfo->EffectBasePoints[i]+rand()%m_spellInfo->EffectDieSides[i]+1,0,m_spellInfo->EffectMiscValue[i],0);
    unitTarget->AddAura(Aur);
    //unitTarget->SetAura(aff); FIX-ME!

}

void Spell::EffectSummon(uint32 i)
{
    if(!unitTarget)
        return;
    uint32 pet_entry = m_spellInfo->EffectMiscValue[i];
    if(!pet_entry)
        return;
    uint32 level = m_caster->getLevel();
    Pet* spawnCreature = new Pet();

    if(!spawnCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),
        m_caster->GetMapId(),
        m_caster->GetPositionX(),m_caster->GetPositionY(),
        m_caster->GetPositionZ(),m_caster->GetOrientation(),
        m_spellInfo->EffectMiscValue[i]))
    {
        sLog.outString("no such creature entry %u",m_spellInfo->EffectMiscValue[i]);
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
    spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,5);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
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
    name.append("\\\'s Pet");
    spawnCreature->SetName( name );
    MapManager::Instance().GetMap(m_caster->GetMapId())->Add((Creature*)spawnCreature);

    if(m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)m_caster)->SavePet();
        m_caster->SetPet(spawnCreature);
        ((Player*)m_caster)->PetSpellInitialize();
    }
}

void Spell::EffectLearnSpell(uint32 i)
{
    WorldPacket data;

    if(!unitTarget)
        return;
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        unitTarget = m_targets.getUnitTarget();

    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

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
            player->learnSpell(2540);
            player->learnSpell(2538);
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
        default:break;
    }
    uint16 maxskill = player->getLevel()*5 > 300 ? 300 :player->getLevel()*5;
    switch(spellToLearn)
    {
        //Armor
        case 9078:                                          //Cloth
            player->SetSkill(415,1,1);
            break;
        case 9077:                                          //Leather
            player->SetSkill(414,1,1);
            break;
        case 8737:                                          //Mail
            player->SetSkill(413,1,1);
            break;
        case 750:                                           //Plate Mail
            player->SetSkill(293,1,1);
            break;
        case 9116:                                          //Shield
            player->SetSkill(433,1,1);
            break;
            //Melee Weapons
        case 196:                                           //Axes
            player->SetSkill(44,1,maxskill);
            break;
        case 197:                                           //Two-Handed Axes
            player->SetSkill(172,1,maxskill);
            break;
        case 227:                                           //Staves
            player->SetSkill(136,1,maxskill);
            break;
        case 198:                                           //Maces
            player->SetSkill(54,1,maxskill);
            break;
        case 199:                                           //Two-Handed Maces
            player->SetSkill(160,1,maxskill);
            break;
        case 201:                                           //Swords
            player->SetSkill(43,1,maxskill);
            break;
        case 202:                                           //Two-Handed Swords
            player->SetSkill(55,1,maxskill);
            break;
        case 1180:                                          //Daggers
            player->SetSkill(173,1,maxskill);
            break;
        case 15590:                                         //Fist Weapons
            player->SetSkill(473,1,maxskill);
            break;
        case 200:                                           //Polearms
            player->SetSkill(229,1,maxskill);
            break;
        case 3386:                                          //Polearms
            player->SetSkill(227,1,maxskill);
            break;
            //Range Weapons
        case 264:                                           //Bows
            player->SetSkill(45,1,maxskill);
            break;
        case 5011:                                          //Crossbows
            player->SetSkill(226,1,maxskill);
            break;
        case 266:                                           //Guns
            player->SetSkill(46,1,maxskill);
            break;
        case 2567:                                          //Thrown
            player->SetSkill(176,1,maxskill);
            break;
        case 5009:                                          //Wands
            player->SetSkill(228,1,maxskill);
            break;
        default:break;
    }
    sLog.outDebug( "Spell: Player %u have learned spell %u from NpcGUID=%u", player->GetGUIDLow(), spellToLearn, m_caster->GetGUIDLow() );
}

void Spell::EffectDispel(uint32 i)
{
    m_caster->RemoveFirstAuraByDispel(m_spellInfo->EffectMiscValue[i]);
}

void Spell::EffectPickPocket(uint32 i)
{
    if( m_caster->GetTypeId() != TYPEID_PLAYER )
        return;

    if( !unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT)
        return;

    if( unitTarget->getDeathState() == ALIVE &&             //victim is alive
                                                            //victim have to be humanoid
        ((Creature*)unitTarget)->GetCreatureInfo()->type == CREATURE_TYPE_HUMANOID )
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
            m_caster->RemoveAurasDueToSpell(1784);
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
    uint32 level = m_caster->getLevel();
    Pet* spawnCreature = new Pet();

    if(!spawnCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),
        m_caster->GetMapId(),
        m_caster->GetPositionX(),m_caster->GetPositionY(),
        m_caster->GetPositionZ(),m_caster->GetOrientation(),
        m_spellInfo->EffectMiscValue[i]))
    {
        sLog.outString("no such creature entry %u",m_spellInfo->EffectMiscValue[i]);
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
    spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,5);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);

    spawnCreature->SetArmor(level*50);
    spawnCreature->AIM_Initialize();

    std::string name;
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        name = ((Player*)m_caster)->GetName();
    else
        name = ((Creature*)m_caster)->GetCreatureInfo()->Name;
    name.append("\\\'s Pet");
    spawnCreature->SetName( name );
    MapManager::Instance().GetMap(m_caster->GetMapId())->Add((Creature*)spawnCreature);

    if(m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)m_caster)->SavePet();
        m_caster->SetPet(spawnCreature);
        ((Player*)m_caster)->PetSpellInitialize();
    }

}

void Spell::EffectTeleUnitsFaceCaster(uint32 i)
{
    if(!unitTarget)
        return;
    WorldPacket data;

    float dis = GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
    float fx = m_caster->GetPositionX() + dis * cos(m_caster->GetOrientation());
    float fy = m_caster->GetPositionY() + dis * sin(m_caster->GetOrientation());
    float fz = MapManager::Instance ().GetMap(m_caster->GetMapId())->GetHeight(fx,fy);

    unitTarget->BuildTeleportAckMsg(&data,fx,fy,fz+2,-m_caster->GetOrientation());
    m_caster->SendMessageToSet( &data, true );
}

void Spell::EffectLearnSkill(uint32 i)
{
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 skillid =  m_spellInfo->EffectMiscValue[i];
    uint16 skillval = ((Player*)unitTarget)->GetSkillValue(skillid);
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
    uint32 add_slot = 0;

    for(add_slot = 0; add_slot < 21; add_slot ++)
        if (itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot))
    {
        p_caster->AddItemEnchant(itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot),false);
        itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot,0);
    }
    add_slot = 0;

    p_caster->UpdateSkillPro(m_spellInfo->Id);

    if (add_slot < 21)
    {
        for(int j = 0;j < 3; j++)
            if (m_spellInfo->EffectMiscValue[j])
        {
            uint32 enchant_id = m_spellInfo->EffectMiscValue[j];
            itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+(add_slot+3*j), enchant_id);
            p_caster->AddItemEnchant(enchant_id,true);
        }
    }
}

void Spell::EffectEnchantItemTmp(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = (Player*)m_caster;
    uint32 add_slot = 0;
    uint8 item_slot = 0;

    uint32 field = 99;
    if(itemTarget)
        field = 1;
    else
        field = 3;

    if(!itemTarget)
    {
        for(uint8 j=0;j<INVENTORY_SLOT_ITEM_END;j++)
        {
            if(p_caster->GetItemByPos( INVENTORY_SLOT_BAG_0, j ) != 0 && p_caster->GetItemByPos( INVENTORY_SLOT_BAG_0, j )->GetProto()->ItemId == itemTarget->GetEntry())
            {
                itemTarget = p_caster->GetItemByPos( INVENTORY_SLOT_BAG_0, j );
                item_slot = j;
            }
        }
    }

    for(add_slot = 0; add_slot < 21; add_slot ++)
        if (itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot))
    {
        p_caster->AddItemEnchant(itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot),false);
        itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot,0);
    }
    add_slot = 0;

    if (add_slot < 21)
    {
        for(int j = 0;j < 3; j++)
            if (m_spellInfo->EffectMiscValue[j])
        {
            uint32 enchant_id = m_spellInfo->EffectMiscValue[j];
            itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+(add_slot+3*j), enchant_id);
            if(itemTarget->GetSlot() < EQUIPMENT_SLOT_END)
                p_caster->AddItemEnchant(enchant_id,true);
        }
    }
}

void Spell::EffectTameCreature(uint32 i)
{
    if(!unitTarget)
        return;

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        return;

    Creature* creatureTarget = (Creature*)unitTarget;

    WorldPacket data;

    if(m_caster->getClass() == CLASS_HUNTER)
    {
        uint32 petlevel = creatureTarget->getLevel();
        creatureTarget->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, m_caster->GetGUID());
        creatureTarget->SetUInt64Value(UNIT_FIELD_CREATEDBY, m_caster->GetGUID());
        creatureTarget->SetMaxPower(POWER_HAPPINESS,1000000);
        creatureTarget->SetPower(   POWER_HAPPINESS,600000);
        creatureTarget->setPowerType(POWER_FOCUS);
        creatureTarget->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->getFaction());
        creatureTarget->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,5);
        creatureTarget->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
        creatureTarget->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
        ((Pet*)creatureTarget)->AIM_Initialize();

        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            ((Player*)m_caster)->SavePet();
            m_caster->SetPet(creatureTarget);
            ((Player*)m_caster)->PetSpellInitialize();
        }
    }
}

void Spell::EffectSummonPet(uint32 i)
{
    WorldPacket data;
    float px, py, pz;
    m_caster->GetClosePoint(NULL, px, py, pz);

    uint32 petentry = m_spellInfo->EffectMiscValue[i];

    Creature *OldSummon = m_caster->GetPet();

    // if pet requested type already exist
    if(OldSummon && OldSummon->isPet() && OldSummon->GetCreatureInfo()->Entry == petentry)
    {
        if(OldSummon->isDead() )
        {
            uint32 petlvl = OldSummon->getLevel();
            OldSummon->RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
            OldSummon->SetHealth( 28 + 10 * petlvl );
            OldSummon->SetMaxHealth( 28 + 10 * petlvl );
            OldSummon->SetPower(   POWER_MANA, 28 + 10 * petlvl);
            OldSummon->SetMaxPower(POWER_MANA, 28 + 10 * petlvl);
            OldSummon->setDeathState(ALIVE);
            OldSummon->clearUnitState(UNIT_STAT_ALL_STATE);
            (*OldSummon)->Clear();
            MapManager::Instance().GetMap(m_caster->GetMapId())->Add(OldSummon);
        }
        OldSummon->Relocate(px, py, pz, OldSummon->GetOrientation());
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            ((Player*)m_caster)->PetSpellInitialize();
        }
        return;
    }

    if(OldSummon)
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            ((Player*)m_caster)->UnsummonPet(true);
        else
            return;
    }

    Pet* NewSummon = new Pet();
    if(NewSummon->LoadPetFromDB( m_caster ))
        return;

    if( NewSummon->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),  m_caster->GetMapId(), px, py, pz+1, m_caster->GetOrientation(), petentry))
    {
        uint32 petlevel=m_caster->getLevel();
        NewSummon->SetLevel(petlevel);
        NewSummon->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, m_caster->GetGUID());
        NewSummon->SetUInt32Value(UNIT_NPC_FLAGS , 0);
        NewSummon->SetHealth(    28 + 10 * petlevel);
        NewSummon->SetMaxHealth( 28 + 10 * petlevel);
        NewSummon->SetPower(   POWER_MANA , 28 + 10 * petlevel);
        NewSummon->SetMaxPower(POWER_MANA, 28 + 10 * petlevel);
        NewSummon->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->getFaction());
        NewSummon->SetUInt32Value(UNIT_FIELD_BYTES_0,2048);
        NewSummon->SetUInt32Value(UNIT_FIELD_FLAGS,0);
        NewSummon->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
        NewSummon->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,5);
        NewSummon->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
        NewSummon->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
        NewSummon->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);
        NewSummon->SetStat(STAT_STRENGTH,22);
        NewSummon->SetStat(STAT_AGILITY,22);
        NewSummon->SetStat(STAT_STAMINA,25);
        NewSummon->SetStat(STAT_INTELLECT,28);
        NewSummon->SetStat(STAT_SPIRIT,27);
        std::string name;
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            name = ((Player*)m_caster)->GetName();
        else
            name = ((Creature*)m_caster)->GetCreatureInfo()->Name;
        name.append("\\\'s Pet");
        NewSummon->SetName( name );

        for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
            NewSummon->m_spells[i] = 0;

        if(petentry == 416)                                 //imp
        {
            NewSummon->m_spells[0] = 133;                   //133---fire bolt 1
            NewSummon->AddActState(STATE_RA_SPELL1);
        }

        NewSummon->SavePetToDB();
        NewSummon->AIM_Initialize();
        MapManager::Instance().GetMap(NewSummon->GetMapId())->Add((Creature*)NewSummon);

        m_caster->SetPet(NewSummon);
        sLog.outDebug("New Pet has guid %u", NewSummon->GetGUID());

        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
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

    Creature *pet = _player->GetPet();
    if(!pet)
        return;
    if(!pet->isAlive())
        return;

    SpellEntry *learn_spellproto = sSpellStore.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
    if(!learn_spellproto)
        return;
    SpellEntry *has_spellproto;
    for(int8 x=0;x<4;x++)
    {
        has_spellproto = sSpellStore.LookupEntry(pet->m_spells[x]);
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
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)m_caster)->PetSpellInitialize();
    }
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

    float    chanceToHit = 100.0f;
    int32    attackerSkill = m_caster->GetWeaponSkillValue();
    int32    victimSkill = unitTarget->GetDefenceSkillValue();
    if (m_caster->GetTypeId() == TYPEID_PLAYER && unitTarget->GetTypeId() == TYPEID_PLAYER)
    {
        if (attackerSkill <= victimSkill - 24)
            chanceToHit = 0;
        else if (attackerSkill <= victimSkill)
            chanceToHit = 100.0f - (victimSkill - attackerSkill) * (100.0f / 30.0f);

        if (chanceToHit < 15.0f)
            chanceToHit = 15.0f;
    }

    float fdamage;
    if(m_spellInfo->rangeIndex == 1 || m_spellInfo->rangeIndex == 2 || m_spellInfo->rangeIndex == 7)
        fdamage = m_caster->CalculateDamage(false);
    else
    {
        fdamage = m_caster->CalculateDamage(true);
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            Item *pItem = ((Player*)m_caster)->GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED );
            if(!pItem)
                return;

            uint32 type = pItem->GetProto()->InventoryType;
            uint32 ammo;
            if( type == INVTYPE_THROWN )
                ammo = pItem->GetEntry();
            else
                ammo = ((Player*)m_caster)->GetUInt32Value(PLAYER_AMMO_ID);

            ((Player*)m_caster)->DestroyItemCount( ammo, 1, true);
        }
    }

    if((chanceToHit/100) * 512 >= urand(0, 512) )
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER && unitTarget->GetTypeId() == TYPEID_PLAYER)
        {
            if (attackerSkill < victimSkill - 20)
                fdamage = (fdamage * 30) / 100;
            else if (attackerSkill < victimSkill - 10)
                fdamage = (fdamage * 60) / 100;
        }
    }
    else fdamage = 0;
    fdamage += damage;

    // generate lot additional rage to worrior.
    //if (fdamage)
    //    fdamage += damage;

    m_caster->SpellNonMeleeDamageLog(unitTarget,m_spellInfo->Id,(uint32)fdamage);
}

void Spell::EffectWeaponDmgPerc(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    float minDmg,maxDmg;
    minDmg = maxDmg = 0;

    minDmg = m_caster->GetFloatValue(UNIT_FIELD_MINDAMAGE);
    maxDmg = m_caster->GetFloatValue(UNIT_FIELD_MAXDAMAGE);

    uint32 randDmg = (uint32)(maxDmg-minDmg);
    uint32 dmg = (uint32)minDmg;
    if(randDmg > 1)
        dmg += (uint32)(rand()%randDmg);
    dmg = (uint32)(dmg*(damage/100));

    m_caster->SpellNonMeleeDamageLog(unitTarget,m_spellInfo->Id,dmg);

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

    uint32 curHealth = unitTarget->GetHealth();
    uint32 maxHealth = unitTarget->GetMaxHealth();
    if(curHealth+heal > maxHealth)
        unitTarget->SetHealth(maxHealth);
    else
        unitTarget->SetHealth(curHealth+heal);

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        SendHealSpellOnPlayer((Player*)unitTarget, m_spellInfo->Id, maxHealth - curHealth);
}

void Spell::EffectInterruptCast(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    unitTarget->InterruptSpell();
}

void Spell::EffectScriptEffect(uint32 i)
{
    if(!m_spellInfo->Reagent[0])
    {
        // paladin's holy light / flash of light
        if ((m_spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN) &&
            (m_spellInfo->SpellIconID == 70 || m_spellInfo->SpellIconID  == 242))
            EffectHeal( i );
    }
    else
    {
        switch(m_spellInfo->Id)
        {
            case 6201:
                m_spellInfo->EffectItemType[0] = 5512;      //spell 6261;    //primary healstone
                break;
            case 6202:
                m_spellInfo->EffectItemType[0] = 5511;      //spell 6262;    //inferior healstone
                break;
            case 5699:
                m_spellInfo->EffectItemType[0] = 5509;      //spell 5720;    //healstone
                break;
            case 11729:
                m_spellInfo->EffectItemType[0] = 5510;      //spell 5723;    //strong healstone
                break;
            case 11730:
                m_spellInfo->EffectItemType[0] = 9421;      //spell 11732;    //super healstone
                break;
            default:
                return;
        }
        EffectCreateItem( i );
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
    WorldPacket data;

    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *caster = (Player*)m_caster;
    Player *target = (Player*)unitTarget;

    if( caster->isInDuel() || target->isInDuel() ) return;

    //CREATE DUEL FLAG OBJECT
    GameObject* pGameObj = new GameObject();

    uint32 gameobject_id = m_spellInfo->EffectMiscValue[i];

    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), gameobject_id,m_caster->GetMapId(),
        m_caster->GetPositionX()+(unitTarget->GetPositionX()-m_caster->GetPositionX())/2 ,
        m_caster->GetPositionY()+(unitTarget->GetPositionY()-m_caster->GetPositionY())/2 ,
        m_caster->GetPositionZ(),
        m_caster->GetOrientation(), 0, 0, 0, 0))
    {
        delete pGameObj;
        return;
    }
    pGameObj->SetUInt32Value(OBJECT_FIELD_ENTRY, m_spellInfo->EffectMiscValue[i] );
    pGameObj->SetUInt32Value(OBJECT_FIELD_TYPE, 33 );
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

    //Send request to opositor
    data.Initialize(SMSG_DUEL_REQUESTED);
    data << pGameObj->GetGUID();
    data << caster->GetGUID();
    target->GetSession()->SendPacket(&data);
    //Set who are the oponents
    caster->SetDuelVs(target);
    target->SetDuelVs(caster);
    //Players are not in duel yet...
    caster->SetInDuel(false);
    target->SetInDuel(false);
    //Set who is the duel caster
    caster->SetDuelSender(caster);
    target->SetDuelSender(caster);

    caster->SetUInt64Value(PLAYER_DUEL_ARBITER,pGameObj->GetGUID());
    target->SetUInt64Value(PLAYER_DUEL_ARBITER,pGameObj->GetGUID());

}

void Spell::EffectSummonTotem(uint32 i)
{
    WorldPacket data;
    uint64 guid = 0;

    switch(m_spellInfo->Effect[i])
    {
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT1:
        {
            guid = m_caster->m_TotemSlot1;
            m_caster->m_TotemSlot1 = 0;
        }break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT2:
        {
            guid = m_caster->m_TotemSlot2;
            m_caster->m_TotemSlot2 = 0;
        }break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT3:
        {
            guid = m_caster->m_TotemSlot3;
            m_caster->m_TotemSlot3 = 0;
        }break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT4:
        {
            guid = m_caster->m_TotemSlot4;
            m_caster->m_TotemSlot4 = 0;
        }break;
    }
    if(guid != 0)
    {
        Creature *OldTotem = ObjectAccessor::Instance().GetCreature(*m_caster, guid);

        if(OldTotem)
        {
            data.Initialize(SMSG_GAMEOBJECT_DESPAWN_ANIM);
            data << guid;
            m_caster->SendMessageToSet(&data, true);

            data.Initialize(SMSG_DESTROY_OBJECT);
            data << guid;
            m_caster->SendMessageToSet(&data, true);
            MapManager::Instance().GetMap(OldTotem->GetMapId())->Remove(OldTotem, true);
            OldTotem = NULL;
        }
    }

    Creature* pTotem = new Creature();

    if(!pTotem->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT),
        m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(),
        m_caster->GetPositionZ(),m_caster->GetOrientation(),
        m_spellInfo->EffectMiscValue[i] ))
    {
        delete pTotem;
        return;
    }
    pTotem->AIM_Initialize();

    pTotem->SetLevel(m_caster->getLevel());
    sLog.outError("AddObject at Spell.cppl line 1040");
    MapManager::Instance().GetMap(pTotem->GetMapId())->Add(pTotem);

    data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
    data << pTotem->GetGUID();
    m_caster->SendMessageToSet(&data,true);

    switch(m_spellInfo->Effect[i])
    {
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT1:
        {
            m_caster->m_TotemSlot1 = pTotem->GetGUID();
        }break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT2:
        {
            m_caster->m_TotemSlot2 = pTotem->GetGUID();
        }break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT3:
        {
            m_caster->m_TotemSlot3 = pTotem->GetGUID();
        }break;
        case SPELL_EFFECT_SUMMON_TOTEM_SLOT4:
        {
            m_caster->m_TotemSlot4 = pTotem->GetGUID();
        }break;
    }
    m_caster->CastSpell(m_caster, pTotem->GetCreatureInfo()->spell1, true);
}

void Spell::EffectEnchantHeldItem(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = (Player*)m_caster;
    uint32 add_slot = 0;
    uint8 item_slot = 0;

    uint32 field = 99;
    if(itemTarget)
        field = 1;
    else
        field = 3;

    if(!itemTarget)
    {
        for(uint8 j=0;j<INVENTORY_SLOT_ITEM_END;j++)
        {
            if(p_caster->GetItemByPos( INVENTORY_SLOT_BAG_0, j ) != 0 && p_caster->GetItemByPos( INVENTORY_SLOT_BAG_0, j )->GetProto()->ItemId == itemTarget->GetEntry())
            {
                itemTarget = p_caster->GetItemByPos( INVENTORY_SLOT_BAG_0, j );
                item_slot = j;
            }
        }
    }

    if (add_slot < 21)
    {
        for(int j = 0;j < 3; j++)
            if (m_spellInfo->EffectMiscValue[j])
        {
            uint32 enchant_id = m_spellInfo->EffectMiscValue[j];
            if (itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+(add_slot+3*j)))
                p_caster->AddItemEnchant(itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+(add_slot+3*j)),false);
            itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+(add_slot+3*j), enchant_id);
            if(itemTarget->GetSlot() < EQUIPMENT_SLOT_END)
                p_caster->AddItemEnchant(enchant_id,true);
        }
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
    p_caster->DestroyItemCount(itemTarget->GetEntry(),1, true);

    p_caster->UpdateSkillPro(m_spellInfo->Id);

    uint32 item;
    uint32 count = 0;
    if(item_level >= 51)
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
                count = 11176;
            else
                count = 16202;
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
    uint16 dest;
    uint8 msg = p_caster->CanStoreNewItem( 0, NULL_SLOT, dest, item, count, false );
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
    uint16 drunkMod = m_spellInfo->EffectBasePoints[i] * 0xFFFF / 100;
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
    uint32 feelty = pet->GetPower(POWER_HAPPINESS);
    if(damage + feelty <  pet->GetMaxPower(POWER_HAPPINESS))
        pet->SetPower(POWER_HAPPINESS,damage + feelty);
    else pet->SetPower(POWER_HAPPINESS,pet->GetMaxPower(POWER_HAPPINESS));

    SpellEntry *spellinfo = sSpellStore.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
    Spell _spell(m_caster, spellinfo, true, 0);
    SpellCastTargets targets;
    targets.setUnitTarget(pet);
    _spell.prepare(&targets);
}

void Spell::EffectDismissPet(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)m_caster;

    Creature* pet = _player->GetPet();
    unitTarget = (Unit*)pet;

    if(!pet)
        return;

    _player->UnsummonPet(false);
}

void Spell::EffectSummonObject(uint32 i)
{
    WorldPacket data;
    uint64 guid = 0;

    switch(m_spellInfo->Effect[i])
    {
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT1:
        {
            guid = m_caster->m_TotemSlot1;
            m_caster->m_TotemSlot1 = 0;
        }break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT2:
        {
            guid = m_caster->m_TotemSlot2;
            m_caster->m_TotemSlot2 = 0;
        }break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT3:
        {
            guid = m_caster->m_TotemSlot3;
            m_caster->m_TotemSlot3 = 0;
        }break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT4:
        {
            guid = m_caster->m_TotemSlot4;
            m_caster->m_TotemSlot4 = 0;
        }break;
    }
    if(guid != 0)
    {
        GameObject* obj = NULL;
        if( m_caster )
            obj = ObjectAccessor::Instance().GetGameObject(*m_caster, guid);

        if(obj)
        {

            MapManager::Instance().GetMap(obj->GetMapId())->Remove(obj, true);
            obj = NULL;
        }
    }

    GameObject* pGameObj = new GameObject();
    uint32 display_id = m_spellInfo->EffectMiscValue[i];

    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), display_id,m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), 0, 0, 0, 0))
    {
        delete pGameObj;
        return;
    }
    pGameObj->SetUInt32Value(OBJECT_FIELD_ENTRY, m_spellInfo->EffectMiscValue[i]);
    pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, 6);
    pGameObj->SetUInt32Value(OBJECT_FIELD_TYPE,33);
    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL,m_caster->getLevel());
    pGameObj->SetRespawnTimer(GetDuration(m_spellInfo));
    pGameObj->SetSpellId(m_spellInfo->Id);
    m_caster->AddGameObject(pGameObj);

    sLog.outError("AddObject at Spell.cpp 1100");

    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);
    data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
    data << pGameObj->GetGUID();
    m_caster->SendMessageToSet(&data,true);

    switch(m_spellInfo->Effect[i])
    {
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT1:
        {
            m_caster->m_TotemSlot1 = pGameObj->GetGUID();
        }break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT2:
        {
            m_caster->m_TotemSlot2 = pGameObj->GetGUID();
        }break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT3:
        {
            m_caster->m_TotemSlot3 = pGameObj->GetGUID();
        }break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT4:
        {
            m_caster->m_TotemSlot4 = pGameObj->GetGUID();
        }break;
    }

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

    uint32 health = m_spellInfo->EffectBasePoints[i];
    uint32 mana = m_spellInfo->EffectMiscValue[i];

    ((Player*)unitTarget)->setResurrect(m_caster->GetGUID(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
    SendResurrectRequest(pTarget);
}

void Spell::EffectMomentMove(uint32 i)
{
    if( m_spellInfo->rangeIndex== 1)                        //self range
    {
        WorldPacket data;

        float dis = GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
        float fx = m_caster->GetPositionX() + dis * cos(m_caster->GetOrientation());
        float fy = m_caster->GetPositionY() + dis * sin(m_caster->GetOrientation());
        float fz = MapManager::Instance ().GetMap(m_caster->GetMapId())->GetHeight(fx,fy);

        m_caster->BuildTeleportAckMsg(&data,fx,fy,fz,m_caster->GetOrientation());
        m_caster->SendMessageToSet( &data, true );
    }
}

void Spell::EffectReputation(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)m_caster;

    int32  rep_change = m_spellInfo->EffectBasePoints[i]+1; // field store reputation change -1

    uint32 faction_id = m_spellInfo->EffectMiscValue[i];

    FactionEntry* factionEntry = sFactionStore.LookupEntry(faction_id);

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

    Quest *pQuest = _player->GetActiveQuest(quest_id);

    if(!pQuest)
        return;

    if(_player->CanCompleteQuest( pQuest ) )
        _player->CompleteQuest( pQuest );
    else
        return;

    if(_player->GetQuestRewardStatus( pQuest ))
        return

            _player->PlayerTalkClass->SendQuestReward( pQuest, _player->GetGUID(), true, NULL, 0 );
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
        health = m_spellInfo->EffectBasePoints[i] > 0 ? m_spellInfo->EffectBasePoints[i] :(-m_spellInfo->EffectBasePoints[i]);
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

    int32 fishvalue = ((Player*)m_caster)->GetSkillValue(SKILL_SKINNING);
    int32 targetlevel = unitTarget->getLevel();

    ((Player*)m_caster)->SendLoot(unitTarget->GetGUID(),LOOT_SKINNING);

    if(fishvalue> (targetlevel +15)*5 )
        up_skillvalue = 4;
    else if(fishvalue>= (targetlevel +10)*5 )
        up_skillvalue = 3;
    else if(fishvalue >= (targetlevel +5)*5 )
        up_skillvalue = 2;
    else if(fishvalue >= (targetlevel <= 5?(targetlevel-5)*5:targetlevel*5))
        up_skillvalue = 1;
    else up_skillvalue = 0;
    unitTarget->m_form = 99;
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
    Pet* critter = new Pet();

    if(!critter->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),
        m_caster->GetMapId(),
        m_caster->GetPositionX(),m_caster->GetPositionY(),
        m_caster->GetPositionZ(),m_caster->GetOrientation(),
        m_spellInfo->EffectMiscValue[i]))
    {
        sLog.outString("no such creature entry %u",m_spellInfo->EffectMiscValue[i]);
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
    name.append("\\\'s Pet");
    critter->SetName( name );
    m_caster->SetPet(critter);
    MapManager::Instance().GetMap(m_caster->GetMapId())->Add((Creature*)critter);
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)m_caster)->PetSpellInitialize();
    }
}

void Spell::EffectSummonDeadPet(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_UNIT)
        return;
    Player *_player = (Player*)m_caster;
    Creature *_pet = m_caster->GetPet();
    if(!_pet)
        return;
    if(_pet->isAlive())
        return;
    Pet *pet = (Pet*)_pet;
    pet->setDeathState( ALIVE );
    pet->clearUnitState(UNIT_STAT_ALL_STATE);
    pet->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, m_caster->GetGUID());
    pet->SetHealth( uint32(pet->GetMaxHealth()*damage/100));

    pet->AIM_Initialize();

    std::string name;
    name = _player->GetName();
    name.append("\\\'s Pet");
    pet->SetName( name );
    _player->PetSpellInitialize();

}

void Spell::EffectTransmitted(uint32 i)
{
    float fx,fy;
    WorldPacket data;

    float min_dis = GetMinRange(sSpellRange.LookupEntry(m_spellInfo->rangeIndex));
    float max_dis = GetMaxRange(sSpellRange.LookupEntry(m_spellInfo->rangeIndex));
    float diff = max_dis - min_dis + 1;
    float dis = (float)(rand()%(uint32)diff + (uint32)min_dis);

    fx = m_caster->GetPositionX() + dis * cos(m_caster->GetOrientation());
    fy = m_caster->GetPositionY() + dis * sin(m_caster->GetOrientation());

    float fz = MapManager::Instance ().GetMap(m_caster->GetMapId())->GetHeight(fx,fy);

    if(m_spellInfo->EffectMiscValue[i] == 35591)
    {
        Map* Map = MapManager::Instance().GetMap(m_caster->GetMapId());
        //uint8 flag1 = Map->GetTerrainType(fx,fy);
        float posz = Map->GetWaterLevel(fx,fy);
        //!Underwater check
        if (fz > (posz - (float)2))
        {
            SendCastResult(CAST_FAIL_CANT_BE_CAST_HERE);
            up_skillvalue = 4;
            SendChannelUpdate(0);
            return;
        }
    }

    GameObject* pGameObj = new GameObject();
    uint32 name_id = m_spellInfo->EffectMiscValue[i];

    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), name_id,m_caster->GetMapId(),
        fx, fy, fz, m_caster->GetOrientation(), 0, 0, 0, 0))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetUInt32Value(OBJECT_FIELD_ENTRY, m_spellInfo->EffectMiscValue[i] );
    pGameObj->SetUInt32Value(OBJECT_FIELD_TYPE, 33 );
    pGameObj->SetUInt32Value(OBJECT_FIELD_CREATED_BY, m_caster->GetGUIDLow() );
    pGameObj->SetUInt32Value(12, 0x3F63BB3C );
    pGameObj->SetUInt32Value(13, 0xBEE9E017 );
    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel() );
    pGameObj->SetRespawnTimer(GetDuration(m_spellInfo));
    pGameObj->SetSpellId(m_spellInfo->Id);

    DEBUG_LOG("AddObject at SpellEfects.cpp EffectTransmitted\n");
    //m_caster->AddGameObject(pGameObj);
    //m_ObjToDel.push_back(pGameObj);

    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);
    pGameObj->AddToWorld();

    data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
    data << uint64(pGameObj->GetGUID());
    m_caster->SendMessageToSet(&data,true);

    if(m_spellInfo->EffectMiscValue[i] == 35591)
    {
        if( m_caster->GetTypeId() == TYPEID_PLAYER && ((Player*)m_caster)->CheckFishingAble() > 0)
        {
            //pGameObj->SetUInt32Value(GAMEOBJECT_STATE, 0);
                                                            //Orientation3
            pGameObj->SetFloatValue(GAMEOBJECT_ROTATION + 2, 0.88431775569915771 );
                                                            //Orientation4
            pGameObj->SetFloatValue(GAMEOBJECT_ROTATION + 3, -0.4668855369091033 );
            data.Initialize(SMSG_GAMEOBJECT_CUSTOM_ANIM);
            data<<uint64(pGameObj->GetGUID());
            data<<uint8(0);
            ((Player*)m_caster)->GetSession()->SendPacket(&data);
            //need fixed produre of fishing.
            ((Player*)m_caster)->SendLoot(pGameObj->GetGUID(),LOOT_FISHING);
            SendChannelUpdate(0);
        }
    }
}

void Spell::EffectSkill(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *player = (Player*)m_caster;

    uint32 skill_id = m_spellInfo->EffectMiscValue[i];
    if(skill_id == SKILL_FISHING && up_skillvalue != 4)
        up_skillvalue = player->CheckFishingAble();
    if(skill_id == SKILL_SKINNING || skill_id == SKILL_FISHING
        || skill_id == SKILL_HERBALISM || skill_id == SKILL_MINING)
    {
        switch(up_skillvalue)
        {
            case 0:
                return;
            case 1:
            {
                if(urand(1,100) <= 10)
                    return;
                else break;
            }
            case 2:
            {
                if(urand(1,100) <= 40)
                    return;
                else break;
            }
            case 3:
            {
                if(urand(1,100) <= 70)
                    return;
                else break;
            }
            case 4:
            default:return;
        }

        player->UpdateSkill(skill_id);
    }else
    player->UpdateSkillPro(m_spellInfo->Id);
}
