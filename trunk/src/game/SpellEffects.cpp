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
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_GUARDIAN = 42
    &Spell::EffectTeleUnitsFaceCaster,                      //SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER = 43
    &Spell::EffectLearnSkill,                               //SPELL_EFFECT_SKILL_STEP = 44
    &Spell::EffectNULL,                                     //unknown45 = 45
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SPAWN = 46
    &Spell::EffectTradeSkill,                               //SPELL_EFFECT_TRADE_SKILL = 47
    &Spell::EffectNULL,                                     //SPELL_EFFECT_STEALTH = 48 //Useless
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DETECT = 49
    &Spell::EffectTransmitted,                              //SPELL_EFFECT_TRANS_DOOR = 50
    &Spell::EffectNULL,                                     //SPELL_EFFECT_FORCE_CRITICAL_HIT = 51
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
    &Spell::EffectNULL,                                     //SPELL_EFFECT_PICKPOCKET = 71
    &Spell::EffectNULL,                                     //SPELL_EFFECT_ADD_FARSIGHT = 72
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_POSSESSED = 73
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_TOTEM = 74
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
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SELF_RESURRECT = 94
    &Spell::EffectSkinning,                                 //SPELL_EFFECT_SKINNING = 95
    &Spell::EffectCharge,                                   //SPELL_EFFECT_CHARGE = 96
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_CRITTER = 97
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
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_DEAD_PET = 109
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
        uint32 health = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
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
    //If m_immuneToState type contain this aura type, IMMUNE aura.
    if(unitTarget->m_immuneToState & m_spellInfo->EffectApplyAuraName[i])
        return;

    sLog.outDebug("Apply Auraname is: %u", m_spellInfo->EffectApplyAuraName[i]);

    Aura* Aur = new Aura(m_spellInfo, i, m_caster, unitTarget);
    unitTarget->AddAura(Aur);
}

void Spell::EffectManaDrain(uint32 i)
{
    uint32 DrainType = m_spellInfo->EffectMiscValue[i];
    uint16 PowerField = 0;
    switch(DrainType)
    {
        case 0:PowerField = UNIT_FIELD_POWER1;break;
        case 1:PowerField = UNIT_FIELD_POWER2;break;
        case 2:PowerField = UNIT_FIELD_POWER3;break;
        case 3:PowerField = UNIT_FIELD_POWER4;break;
        case 4:PowerField = UNIT_FIELD_POWER5;break;
        default:break;
    }
    if(!PowerField)
        return;
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    uint32 curPower = unitTarget->GetUInt32Value(PowerField);
    float tmpvalue = m_spellInfo->EffectMultipleValue[i];
    if(!tmpvalue)
        tmpvalue = 1;
    if(curPower < damage)
    {
        unitTarget->SetUInt32Value(PowerField,0);
        if(DrainType == 0)
            m_caster->SetUInt32Value(PowerField,uint32(m_caster->GetUInt32Value(PowerField)+curPower*tmpvalue));
    }
    else
    {
        unitTarget->SetUInt32Value(PowerField,curPower-damage);
        if(DrainType == 0)
            m_caster->SetUInt32Value(PowerField,uint32(m_caster->GetUInt32Value(PowerField)+damage*tmpvalue));
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

    uint32 curPower = unitTarget->GetUInt32Value(UNIT_FIELD_POWER1);
    uint32 curHealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 caster_curPower = m_caster->GetUInt32Value(UNIT_FIELD_POWER1);
    uint32 tmpvalue = 0;
    if(curPower < damage)
    {
        unitTarget->SetUInt32Value(UNIT_FIELD_POWER1,0);
        tmpvalue = uint32(curPower*m_spellInfo->EffectMultipleValue[i]);
    }
    else
    {
        tmpvalue = uint32(damage*m_spellInfo->EffectMultipleValue[i]);
        unitTarget->SetUInt32Value(UNIT_FIELD_POWER1,curPower-damage);
    }
    if(caster_curPower + tmpvalue < m_caster->GetUInt32Value(UNIT_FIELD_MAXPOWER1))
        m_caster->SetUInt32Value(UNIT_FIELD_POWER1,caster_curPower + tmpvalue);
    else m_caster->SetUInt32Value(UNIT_FIELD_POWER1,m_caster->GetUInt32Value(UNIT_FIELD_MAXPOWER1));
    unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,curHealth-tmpvalue);

}

void Spell::EffectHeal( uint32 i )
{
    if( unitTarget && unitTarget->isAlive() )
    {
        float pct = (100+unitTarget->m_RegenPCT)/100;
        uint32 curhealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
        uint32 maxhealth = unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
        uint32 addhealth = ( curhealth + damage*pct < maxhealth ? damage*pct : maxhealth - curhealth );
        unitTarget->SetUInt32Value( UNIT_FIELD_HEALTH, curhealth + addhealth );
        //unitTarget->SendHealToLog( m_caster, m_spell, addhealth );
        SendHealSpellOnPlayer(((Player*)m_caster), m_spellInfo->Id, addhealth);

        //If the target is in combat, then player is in combat too
        if( m_caster != unitTarget &&
            m_caster->GetTypeId() == TYPEID_PLAYER &&
            unitTarget->isInCombat() &&
            unitTarget->GetTypeId() == TYPEID_PLAYER &&
            unitTarget->getVictim()->GetTypeId() == TYPEID_PLAYER )
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

    //SendHealSpellOnPlayer(((Player*)unitTarget), m_spellInfo->Id, damage);
    uint32 tmpvalue = 0;
    float pct = (100+unitTarget->m_RegenPCT)/100;

    if(unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH) - damage > 0)
    {
        unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,uint32(unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH) - damage));
        tmpvalue = uint32(damage*m_spellInfo->EffectMultipleValue[i]);
    }
    else
    {
        tmpvalue = uint32(unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH)*m_spellInfo->EffectMultipleValue[i]);
        unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,0);
    }
    if(m_caster->GetUInt32Value(UNIT_FIELD_HEALTH) + tmpvalue*pct < m_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH) )
        m_caster->SetUInt32Value(UNIT_FIELD_HEALTH,m_caster->GetUInt32Value(UNIT_FIELD_HEALTH) + tmpvalue*pct);
    else m_caster->SetUInt32Value(UNIT_FIELD_HEALTH,m_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH));
}

void Spell::EffectCreateItem(uint32 i)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    Player* player = (Player*)m_caster;
    uint32 newitemid, itemid, itemcount;
    if((newitemid = m_spellInfo->EffectItemType[i]) == 0)
        return;

    uint16 dest;
    for(uint32 x=0;x<8;x++)
    {
        if(m_spellInfo->Reagent[x] == 0)
            continue;
        itemid = m_spellInfo->Reagent[x];
        itemcount = m_spellInfo->ReagentCount[x];
        if( player->HasItemCount(itemid,itemcount) && player->CanStoreNewItem(0, NULL_SLOT, dest, newitemid, 1, false ) == EQUIP_ERR_OK )
            player->DestroyItemCount(itemid, itemcount, true);
        else
        {
            SendCastResult(CAST_FAIL_ITEM_NOT_READY);
            return;
        }
    }

    uint32 num_to_add = ((player->getLevel() - (m_spellInfo->spellLevel-1))*2);

    Item *pItem = player->CreateItem(newitemid,1);

    if(pItem->GetProto()->Class != ITEM_CLASS_CONSUMABLE)
        num_to_add = 1;
    if(num_to_add > pItem->GetProto()->Stackable)
        num_to_add = pItem->GetProto()->Stackable;

    uint8 msg = player->CanStoreItem( 0, NULL_SLOT, dest, pItem, false);
    if( msg == EQUIP_ERR_OK )
    {
        player->StoreItem( dest, pItem, true);
        if( pItem->GetProto()->Class != ITEM_CLASS_CONSUMABLE )
            pItem->SetUInt32Value(ITEM_FIELD_CREATOR,player->GetGUIDLow());
        //should send message "create item" to client.-FIX ME
        player->UpdateSkillPro(m_spellInfo->Id);
    }
    else
        player->SendEquipError( msg, NULL, NULL );
}

void Spell::EffectPresistentAA(uint32 i)
{

    //if(m_AreaAura == true)
    //    return;

    //m_AreaAura = true;

    DynamicObject* dynObj = new DynamicObject();
    if(!dynObj->Create(objmgr.GenerateLowGuid(HIGHGUID_DYNAMICOBJECT), m_caster, m_spellInfo, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, GetDuration(m_spellInfo)))
        return;
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
    uint16 POWER_TYPE;

    switch(m_spellInfo->EffectMiscValue[i])
    {
        case 0:
        {
            POWER_TYPE = UNIT_FIELD_POWER1;
        }break;
        case 1:
        {
            POWER_TYPE = UNIT_FIELD_POWER2;
        }break;
        case 2:
        {
            POWER_TYPE = UNIT_FIELD_POWER3;
        }break;
        case 3:
        {
            POWER_TYPE = UNIT_FIELD_POWER4;
        }break;
        case 4:
        {
            POWER_TYPE = UNIT_FIELD_POWER5;
        }break;
    }
    uint32 curEnergy = unitTarget->GetUInt32Value(POWER_TYPE);
    uint32 maxEnergy = unitTarget->GetUInt32Value(POWER_TYPE+6);
    if(curEnergy+damage > maxEnergy)
        unitTarget->SetUInt32Value(POWER_TYPE,maxEnergy);
    else
        unitTarget->SetUInt32Value(POWER_TYPE,curEnergy+damage);

}

void Spell::EffectOpenLock(uint32 i)
{

    if(!gameObjTarget)
    {
        sLog.outDebug( "WORLD: Open Lock - No GameObject Target!");
        return;
    }

    if(!m_caster)
    {
        sLog.outDebug( "WORLD: Open Lock - No Player Caster!");
        return;
    }
    uint8 loottype;
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
        loottype=2;

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

        loottype=2;
    }else loottype=1;
    if(loottype == 1)
    {
        ((Player*)m_caster)->UpdateSkillPro(m_spellInfo->Id);
    }
    ((Player*)m_caster)->SendLoot(gameObjTarget->GetGUID(),loottype);

}

void Spell::EffectSummonChangeItem(uint32 i)
{
    if(!itemTarget)
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
        player->SendEquipError( msg, NULL, NULL );
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
    Creature* spawnCreature = new Creature();

    if(!spawnCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),
        m_caster->GetMapId(),
        m_caster->GetPositionX(),m_caster->GetPositionY(),
        m_caster->GetPositionZ(),m_caster->GetOrientation(),
        m_spellInfo->EffectMiscValue[i]))
    {
        sLog.outString("no such creature entry %u",m_spellInfo->EffectMiscValue[i]);
        return;
    }

    spawnCreature->SetUInt64Value(UNIT_FIELD_SUMMONEDBY,m_caster->GetGUID());
    spawnCreature->SetUInt32Value(UNIT_FIELD_POWER5,1000000);
    spawnCreature->SetUInt32Value(UNIT_FIELD_MAXPOWER5,1000000);
    spawnCreature->SetUInt32Value(UNIT_FIELD_POWER1,28 + 10 * level);
    spawnCreature->SetUInt32Value(UNIT_FIELD_MAXPOWER1,28 + 10 * level);
    spawnCreature->SetUInt32Value(UNIT_NPC_FLAGS , 0);
    spawnCreature->setPowerType(0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_HEALTH, 28 + 30*level);
    spawnCreature->SetUInt32Value(UNIT_FIELD_MAXHEALTH, 28 + 30*level);
    spawnCreature->SetUInt32Value(UNIT_FIELD_LEVEL , level);
    spawnCreature->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    spawnCreature->SetUInt32Value(UNIT_FIELD_FLAGS,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETNUMBER, unitTarget->GetGUIDLow());
    //spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,5);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
    /*
    spawnCreature->SetUInt32Value(UNIT_FIELD_STAT0,int(20+level*1.55));
    spawnCreature->SetUInt32Value(UNIT_FIELD_STAT1,int(20+level*0.64));
    spawnCreature->SetUInt32Value(UNIT_FIELD_STAT2,int(20+level*1.27));
    spawnCreature->SetUInt32Value(UNIT_FIELD_STAT3,int(20+level*0.18));
    spawnCreature->SetUInt32Value(UNIT_FIELD_STAT4,int(20+level*0.36));
    */
    spawnCreature->SetUInt32Value(UNIT_FIELD_ARMOR,level*50);
    ((Pet*)spawnCreature)->SetisPet(true);
    ((Pet*)spawnCreature)->AIM_Initialize();

    std::string name;
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        name = ((Player*)m_caster)->GetName();
    else
        name = ((Creature*)m_caster)->GetCreatureInfo()->Name;
    name.append("\\\'s Pet");

    if(m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data;
        uint16 Command = 7;
        uint16 State = 6;

        sLog.outDebug("Pet Spells Groups");

        data.clear();
        data.Initialize(SMSG_PET_SPELLS);

        data << (uint64)spawnCreature->GetGUID() << uint32(0x00000000) << uint32(0x00001000);

        data << uint16 (2) << uint16(Command << 8) << uint16 (1) << uint16(Command << 8) << uint16 (0) << uint16(Command << 8);

        for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
            data << uint16 (spawnCreature->m_spells[i]) << uint16 (0xC100);    //C100 = maybe group

        data << uint16 (2) << uint16(State << 8) << uint16 (1) << uint16(State << 8) << uint16 (0) << uint16(State << 8);

        ((Player*)m_caster)->GetSession()->SendPacket(&data);
        ((Player*)m_caster)->SavePet();
        m_caster->SetUInt64Value(UNIT_FIELD_SUMMON,spawnCreature->GetGUID());
    }
}

void Spell::EffectLearnSpell(uint32 i)
{
    WorldPacket data;

    if(!unitTarget)
        return;
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        unitTarget = m_targets.getUnitTarget();
    Player *player = (Player*)unitTarget;

    uint32 spellToLearn = m_spellInfo->EffectTriggerSpell[i];
    //data.Initialize(SMSG_LEARNED_SPELL);
    //data << spellToLearn;
    //((Player*)unitTarget)->GetSession()->SendPacket(&data);
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
    switch(spellToLearn)
    {
        //Armor
        case 9078:                                          //Cloth
            player->SetSkill(415,1,player->getLevel()*5);
            break;
        case 9077:                                          //Leather
            player->SetSkill(414,1,player->getLevel()*5);
            break;
        case 8737:                                          //Mail
            player->SetSkill(413,1,player->getLevel()*5);
            break;
        case 750:                                           //Plate Mail
            player->SetSkill(293,1,player->getLevel()*5);
            break;
        case 9116:                                          //Shield
            player->SetSkill(433,1,player->getLevel()*5);
            break;
            //Melee Weapons
        case 196:                                           //Axes
            player->SetSkill(44,1,player->getLevel()*5);
            break;
        case 197:                                           //Two-Handed Axes
            player->SetSkill(172,1,player->getLevel()*5);
            break;
        case 227:                                           //Staves
            player->SetSkill(136,1,player->getLevel()*5);
            break;
        case 198:                                           //Maces
            player->SetSkill(54,1,player->getLevel()*5);
            break;
        case 199:                                           //Two-Handed Maces
            player->SetSkill(160,1,player->getLevel()*5);
            break;
        case 201:                                           //Swords
            player->SetSkill(43,1,player->getLevel()*5);
            break;
        case 202:                                           //Two-Handed Swords
            player->SetSkill(55,1,player->getLevel()*5);
            break;
        case 1180:                                          //Daggers
            player->SetSkill(173,1,player->getLevel()*5);
            break;
        case 15590:                                         //Fist Weapons
            player->SetSkill(473,1,player->getLevel()*5);
            break;
        case 200:                                           //Polearms
            player->SetSkill(229,1,player->getLevel()*5);
            break;
        case 3386:                                          //Polearms
            player->SetSkill(227,1,player->getLevel()*5);
            break;
            //Range Weapons
        case 264:                                           //Bows
            player->SetSkill(45,1,player->getLevel()*5);
            break;
        case 5011:                                          //Crossbows
            player->SetSkill(226,1,player->getLevel()*5);
            break;
        case 266:                                           //Guns
            player->SetSkill(46,1,player->getLevel()*5);
            break;
        case 2567:                                          //Thrown
            player->SetSkill(176,1,player->getLevel()*5);
            break;
        case 5009:                                          //Wands
            player->SetSkill(228,1,player->getLevel()*5);
            break;
        default:break;
    }
    sLog.outDebug( "Spell: Player %u have learned spell %u from NpcGUID=%u", player->GetGUIDLow(), spellToLearn, m_caster->GetGUIDLow() );
}

void Spell::EffectDispel(uint32 i)
{
    m_caster->RemoveFirstAuraByDispel(m_spellInfo->EffectMiscValue[i]);
}

void Spell::EffectSummonWild(uint32 i)
{
    if(!unitTarget)
        return;
    uint32 pet_entry = m_spellInfo->EffectMiscValue[i];
    if(!pet_entry)
        return;
    uint32 level = m_caster->getLevel();
    Creature* spawnCreature = new Creature();

    if(!spawnCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),
        m_caster->GetMapId(),
        m_caster->GetPositionX(),m_caster->GetPositionY(),
        m_caster->GetPositionZ(),m_caster->GetOrientation(),
        m_spellInfo->EffectMiscValue[i]))
    {
        sLog.outString("no such creature entry %u",m_spellInfo->EffectMiscValue[i]);
        return;
    }

    spawnCreature->SetUInt64Value(UNIT_FIELD_SUMMONEDBY,m_caster->GetGUID());
    spawnCreature->SetUInt32Value(UNIT_FIELD_POWER5,1000000);
    spawnCreature->SetUInt32Value(UNIT_FIELD_MAXPOWER5,1000000);
    spawnCreature->SetUInt32Value(UNIT_FIELD_POWER1,28 + 10 * level);
    spawnCreature->SetUInt32Value(UNIT_FIELD_MAXPOWER1,28 + 10 * level);
    spawnCreature->SetUInt32Value(UNIT_NPC_FLAGS , 0);
    spawnCreature->setPowerType(0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_HEALTH, 28 + 30*level);
    spawnCreature->SetUInt32Value(UNIT_FIELD_MAXHEALTH, 28 + 30*level);
    spawnCreature->SetUInt32Value(UNIT_FIELD_LEVEL , level);
    spawnCreature->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    spawnCreature->SetUInt32Value(UNIT_FIELD_FLAGS,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETNUMBER, unitTarget->GetGUIDLow());
    //spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,5);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
    /*
    spawnCreature->SetUInt32Value(UNIT_FIELD_STAT0,int(20+level*1.55));
    spawnCreature->SetUInt32Value(UNIT_FIELD_STAT1,int(20+level*0.64));
    spawnCreature->SetUInt32Value(UNIT_FIELD_STAT2,int(20+level*1.27));
    spawnCreature->SetUInt32Value(UNIT_FIELD_STAT3,int(20+level*0.18));
    spawnCreature->SetUInt32Value(UNIT_FIELD_STAT4,int(20+level*0.36));
    */
    spawnCreature->SetUInt32Value(UNIT_FIELD_ARMOR,level*50);
    ((Pet*)spawnCreature)->SetisPet(true);
    ((Pet*)spawnCreature)->AIM_Initialize();

    std::string name;
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        name = ((Player*)m_caster)->GetName();
    else
        name = ((Creature*)m_caster)->GetCreatureInfo()->Name;
    name.append("\\\'s Pet");

    if(m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data;
        uint16 Command = 7;
        uint16 State = 6;

        sLog.outDebug("Pet Spells Groups");

        data.clear();
        data.Initialize(SMSG_PET_SPELLS);

        data << (uint64)spawnCreature->GetGUID() << uint32(0x00000000) << uint32(0x00001000);

        data << uint16 (2) << uint16(Command << 8) << uint16 (1) << uint16(Command << 8) << uint16 (0) << uint16(Command << 8);

        for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
            data << uint16 (spawnCreature->m_spells[i]) << uint16 (0xC100);    //C100 = maybe group

        data << uint16 (2) << uint16(State << 8) << uint16 (1) << uint16(State << 8) << uint16 (0) << uint16(State << 8);

        ((Player*)m_caster)->GetSession()->SendPacket(&data);
        ((Player*)m_caster)->SavePet();
        m_caster->SetUInt64Value(UNIT_FIELD_SUMMON,spawnCreature->GetGUID());
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
    if(itemTarget->GetProto()->Class != m_spellInfo->EquippedItemClass
        /*|| !(itemTarget->GetProto()->SubClass & m_spellInfo->EquippedItemSubClass)*/)
    {
        SendCastResult(CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM);
        return;
    }

    for(uint32 x=0;x<8;x++)
    {
        if(m_spellInfo->Reagent[x] == 0)
            continue;
        uint32 itemid = m_spellInfo->Reagent[x];
        uint32 itemcount = m_spellInfo->ReagentCount[x];
        if( p_caster->HasItemCount(itemid,itemcount) )
            p_caster->DestroyItemCount(itemid, itemcount, true);
        else
        {
            SendCastResult(CAST_FAIL_ITEM_NOT_READY);
            return;
        }
    }

    for(add_slot = 0; add_slot < 21; add_slot ++)
        if (itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot))
    {
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
            if(itemTarget->GetSlot() < EQUIPMENT_SLOT_END)
                p_caster->AddItemEnchant(enchant_id,true);
        }
    }
}

void Spell::EffectEnchantItemTmp(uint32 i)
{
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
    if(itemTarget->GetProto()->Class != m_spellInfo->EquippedItemClass
        /*|| !(itemTarget->GetProto()->SubClass & m_spellInfo->EquippedItemSubClass)*/)
    {
        SendCastResult(CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM);
        return;
    }

    for(uint32 x=0;x<8;x++)
    {
        if(m_spellInfo->Reagent[x] == 0)
            continue;
        uint32 itemid = m_spellInfo->Reagent[x];
        uint32 itemcount = m_spellInfo->ReagentCount[x];
        if( p_caster->HasItemCount(itemid,itemcount) )
            p_caster->DestroyItemCount(itemid, itemcount, true);
        else
        {
            SendCastResult(CAST_FAIL_ITEM_NOT_READY);
            return;
        }
    }

    for(add_slot = 0; add_slot < 21; add_slot ++)
        if (itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot))
    {
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
    if(!unitTarget || unitTarget->getLevel() > m_caster->getLevel())
    {
        SendCastResult(CAST_FAIL_TARGET_IS_TOO_HIGH);
        return;
    }

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        return;

    Creature* creatureTarget = (Creature*)unitTarget;

    CreatureInfo *cinfo = creatureTarget->GetCreatureInfo();

    if(cinfo->type != CREATURE_TYPE_BEAST)
        return;

    if(m_caster->GetUInt64Value(UNIT_FIELD_SUMMON))
    {
        SendCastResult(CAST_FAIL_ALREADY_HAVE_SUMMON);
        return;
    }

    WorldPacket data;

    if(m_caster->getClass() == CLASS_HUNTER)
    {
        uint32 petlevel = creatureTarget->GetUInt32Value(UNIT_FIELD_LEVEL);
        creatureTarget->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, m_caster->GetGUID());
        creatureTarget->SetUInt64Value(UNIT_FIELD_CREATEDBY, m_caster->GetGUID());
        creatureTarget->SetUInt32Value(UNIT_NPC_FLAGS , 0);
        creatureTarget->SetUInt32Value(UNIT_FIELD_BYTES_0,0x2020100);
        creatureTarget->SetUInt32Value(UNIT_FIELD_HEALTH , 28 + 10 * petlevel);
        creatureTarget->SetUInt32Value(UNIT_FIELD_MAXHEALTH , 28 + 10 * petlevel);
        creatureTarget->SetUInt32Value(UNIT_FIELD_MAXPOWER1,28 + 10 * petlevel);
        creatureTarget->SetUInt32Value(UNIT_FIELD_POWER1,28 + 10 * petlevel);
        creatureTarget->SetUInt32Value(UNIT_FIELD_MAXPOWER5,1000000);
        creatureTarget->SetUInt32Value(UNIT_FIELD_POWER5,1000000);
        creatureTarget->setPowerType(2);
        creatureTarget->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
        creatureTarget->SetUInt32Value(UNIT_FIELD_FLAGS,0);
        creatureTarget->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
        creatureTarget->SetUInt32Value(UNIT_FIELD_PETNUMBER, creatureTarget->GetGUIDLow());
        creatureTarget->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,5);
        creatureTarget->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
        creatureTarget->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
        creatureTarget->SetUInt32Value(UNIT_FIELD_STAT0,int(20+petlevel*1.55));
        creatureTarget->SetUInt32Value(UNIT_FIELD_STAT1,int(20+petlevel*0.64));
        creatureTarget->SetUInt32Value(UNIT_FIELD_STAT2,int(20+petlevel*1.27));
        creatureTarget->SetUInt32Value(UNIT_FIELD_STAT3,int(20+petlevel*0.18));
        creatureTarget->SetUInt32Value(UNIT_FIELD_STAT4,int(20+petlevel*0.36));
        creatureTarget->SetUInt32Value(UNIT_FIELD_ARMOR,petlevel*50);
        creatureTarget->SetUInt32Value(UNIT_FIELD_BYTES_2,1);
        creatureTarget->SetisPet(true);
        creatureTarget->AIM_Initialize();

        std::string name;
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            name = ((Player*)m_caster)->GetName();
        else
            name = ((Creature*)m_caster)->GetCreatureInfo()->Name;
        name.append("\\\'s Pet");

        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            uint16 Command = 7;
            uint16 State = 6;

            sLog.outDebug("Pet Spells Groups");

            data.clear();
            data.Initialize(SMSG_PET_SPELLS);

            data << (uint64)creatureTarget->GetGUID() << uint32(0x00000000) << uint32(0x00001000);

            data << uint16 (2) << uint16(Command << 8) << uint16 (1) << uint16(Command << 8) << uint16 (0) << uint16(Command << 8);

            for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
                data << uint16(creatureTarget->m_spells[i]) << uint16(0xC100); //C100 = maybe group

            data << uint16 (2) << uint16(State << 8) << uint16 (1) << uint16(State << 8) << uint16 (0) << uint16(State << 8);

            ((Player*)m_caster)->GetSession()->SendPacket(&data);
            ((Player*)m_caster)->SavePet();
            m_caster->SetUInt64Value(UNIT_FIELD_SUMMON,creatureTarget->GetGUID());
        }
    }
}

void Spell::EffectSummonPet(uint32 i)
{
    WorldPacket data;
    uint64 petguid;
    float px, py, pz;
    m_caster->GetClosePoint(NULL, px, py, pz);
    if((petguid=m_caster->GetUInt64Value(UNIT_FIELD_SUMMON)) != 0)
    {
        Creature *OldSummon;
        OldSummon = ObjectAccessor::Instance().GetCreature(*m_caster, petguid);
        if(OldSummon && OldSummon->isPet())
        {
            if(OldSummon->isDead())
            {
                uint32 petlvl = OldSummon->GetUInt32Value(UNIT_FIELD_LEVEL);
                OldSummon->RemoveFlag (UNIT_FIELD_FLAGS, 0x4000000);
                OldSummon->SetUInt32Value(UNIT_FIELD_HEALTH, 28 + 10 * petlvl );
                OldSummon->SetUInt32Value(UNIT_FIELD_MAXHEALTH , 28 + 10 * petlvl );
                OldSummon->SetUInt32Value(UNIT_FIELD_POWER1 , 28 + 10 * petlvl);
                OldSummon->SetUInt32Value(UNIT_FIELD_MAXPOWER1 , 28 + 10 * petlvl);
                OldSummon->setDeathState(ALIVE);
                OldSummon->clearUnitState(UNIT_STAT_ALL_STATE);
                ((Creature&)*OldSummon)->Clear();
                MapManager::Instance().GetMap(m_caster->GetMapId())->Add(OldSummon);
            }
            OldSummon->Relocate(px, py, pz, OldSummon->GetOrientation());
            if(m_caster->GetTypeId() == TYPEID_PLAYER)
            {
                uint16 Command = 7;
                uint16 State = 6;

                sLog.outDebug("Pet Spells Groups");

                data.clear();
                data.Initialize(SMSG_PET_SPELLS);

                data << (uint64)OldSummon->GetGUID() << uint32(0x00000000) << uint32(0x00001000);

                data << uint16 (2) << uint16(Command << 8) << uint16 (1) << uint16(Command << 8) << uint16 (0) << uint16(Command << 8);

                for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
                    data << uint16(OldSummon->m_spells[i]) << uint16(0xC100);  //C100 = maybe group 

                data << uint16 (2) << uint16(State << 8) << uint16 (1) << uint16(State << 8) << uint16 (0) << uint16(State << 8);

                ((Player*)m_caster)->GetSession()->SendPacket(&data);
            }
            return;
        }
    }
    uint32 petentry = m_spellInfo->EffectMiscValue[i];
    Pet* NewSummon = new Pet();
    if(NewSummon->LoadPetFromDB( m_caster ))
        return;

    if( NewSummon->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),  m_caster->GetMapId(), px, py, pz+1, m_caster->GetOrientation(), petentry))
    {
        uint32 petlevel=m_caster->getLevel();
        NewSummon->SetUInt32Value(UNIT_FIELD_LEVEL,petlevel);
        NewSummon->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, m_caster->GetGUID());
        NewSummon->SetUInt32Value(UNIT_NPC_FLAGS , 0);
        NewSummon->SetUInt32Value(UNIT_FIELD_HEALTH , 28 + 10 * petlevel);
        NewSummon->SetUInt32Value(UNIT_FIELD_MAXHEALTH , 28 + 10 * petlevel);
        NewSummon->SetUInt32Value(UNIT_FIELD_POWER1 , 28 + 10 * petlevel);
        NewSummon->SetUInt32Value(UNIT_FIELD_MAXPOWER1 , 28 + 10 * petlevel);
        NewSummon->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));

        NewSummon->SetUInt32Value(UNIT_FIELD_BYTES_0,2048);

        NewSummon->SetUInt32Value(UNIT_FIELD_FLAGS,0);

        NewSummon->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
        NewSummon->SetUInt32Value(UNIT_FIELD_PETNUMBER, NewSummon->GetGUIDLow());
        NewSummon->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,5);
        NewSummon->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
        NewSummon->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP,1000);
        NewSummon->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);
        NewSummon->SetUInt32Value(UNIT_FIELD_STAT0,22);
        NewSummon->SetUInt32Value(UNIT_FIELD_STAT1,22);
        NewSummon->SetUInt32Value(UNIT_FIELD_STAT2,25);
        NewSummon->SetUInt32Value(UNIT_FIELD_STAT3,28);
        NewSummon->SetUInt32Value(UNIT_FIELD_STAT4,27);
        std::string name;
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            name = ((Player*)m_caster)->GetName();
        else
            name = ((Creature*)m_caster)->GetCreatureInfo()->Name;
        name.append("\\\'s Pet");
        NewSummon->SetName( name );
        NewSummon->SetFealty( 10 );

        for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
            NewSummon->m_spells[i] = 0;

        if(petentry == 416)                                 //imp
        {
            NewSummon->m_spells[0] = 133;                   //133---fire bolt 1
            NewSummon->AddActState(STATE_RA_SPELL1);
        }

        NewSummon->SetisPet(true);
        NewSummon->SavePetToDB();
        NewSummon->AIM_Initialize();
        MapManager::Instance().GetMap(NewSummon->GetMapId())->Add((Creature*)NewSummon);

        m_caster->SetUInt64Value(UNIT_FIELD_SUMMON, NewSummon->GetGUID());
        sLog.outDebug("New Pet has guid %u", NewSummon->GetGUID());

        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            uint16 Command = 7;
            uint16 State = 6;

            sLog.outDebug("Pet Spells Groups");

            data.clear();
            data.Initialize(SMSG_PET_SPELLS);

            data << (uint64)NewSummon->GetGUID() << uint32(0x00000000) << uint32(0x00001000);

            data << uint16 (2) << uint16(Command << 8) << uint16 (1) << uint16(Command << 8) << uint16 (0) << uint16(Command << 8);

            for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
                data << uint16(NewSummon->m_spells[i]) << uint16(0xC100);      //C100 = maybe group

            data << uint16 (2) << uint16(State << 8) << uint16 (1) << uint16(State << 8) << uint16 (0) << uint16(State << 8);

            ((Player*)m_caster)->GetSession()->SendPacket(&data);
        }
    }
}

void Spell::EffectLearnPetSpell(uint32 i)
{
    if(!unitTarget)
        return;

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        return;

    if(!unitTarget->isAlive())
        return;

    Creature* creatureTarget = (Creature*)unitTarget;

    SpellEntry *learn_spellproto = sSpellStore.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
    if(!learn_spellproto)
        return;
    SpellEntry *has_spellproto;
    uint8 learn_msg = 1;
    for(int8 x=0;x<4;x++)
    {
        has_spellproto = sSpellStore.LookupEntry(creatureTarget ->m_spells[x]);
        if(creatureTarget ->m_spells[x] == learn_spellproto->Id)
            return;
    }
    for(int8 x=0;x<4;x++)
    {
        has_spellproto = sSpellStore.LookupEntry(creatureTarget ->m_spells[x]);
        if(!has_spellproto)
        {
            creatureTarget ->m_spells[x] = learn_spellproto->Id;
            learn_msg = 0;
            break;
        }
        else if(has_spellproto->SpellIconID == learn_spellproto->SpellIconID)
        {
            creatureTarget ->m_spells[x] = learn_spellproto->Id;
            learn_msg = 0;
            break;
        }
    }
    if(learn_msg)
    {
        SendCastResult(CAST_FAIL_SPELL_NOT_LEARNED);
        return;
    }
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data;
        uint16 Command = 7;
        uint16 State = 6;

        sLog.outDebug("Pet Spells Groups");

        data.clear();
        data.Initialize(SMSG_PET_SPELLS);

        data << (uint64)creatureTarget ->GetGUID() << uint32(0x00000000) << uint32(0x00001000);

        data << uint16 (2) << uint16(Command << 8) << uint16 (1) << uint16(Command << 8) << uint16 (0) << uint16(Command << 8);

        for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
            data << uint16 (creatureTarget ->m_spells[i]) << uint16 (0xC100);  //C100 = maybe group

        data << uint16 (2) << uint16(State << 8) << uint16 (1) << uint16(State << 8) << uint16 (0) << uint16(State << 8);

        ((Player*)m_caster)->GetSession()->SendPacket(&data);
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
    int32    attackerSkill = m_caster->GetUnitMeleeSkill();
    int32    victimSkill = unitTarget->GetUnitMeleeSkill();
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
            uint32 type = pItem->GetProto()->InventoryType;
            uint32 ammo;
            if( type == INVTYPE_THROWN )
                ammo = pItem->GetEntry();
            else
                ammo = ((Player*)m_caster)->GetUInt32Value(PLAYER_AMMO_ID);

            if( ((Player*)m_caster)->HasItemCount( ammo, 1 ) )
                ((Player*)m_caster)->DestroyItemCount( ammo, 1, true);
            else
            {
                SendCastResult(CAST_FAIL_NO_AMMO);
                return;
            }
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
    dmg += (uint32)(dmg*(damage/100));

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
    if(unitTarget->m_immuneToMechanic == 16)
        return;

    uint32 heal;
    heal = m_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH);

    uint32 curHealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 maxHealth = unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(curHealth+heal > maxHealth)
        unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,maxHealth);
    else
        unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,curHealth+heal);

    SendHealSpellOnPlayer((Player*)m_caster, m_spellInfo->Id, maxHealth - curHealth);
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
    //temply use, need fix to right way.
    if(!m_spellInfo->Reagent[0])
        EffectHeal( i );
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
    uint8 comboPoints = ((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & 0xFF00) >> 8);
    if(m_caster->GetUInt64Value(PLAYER_FIELD_COMBO_TARGET) != unitTarget->GetGUID())
    {
        m_caster->SetUInt64Value(PLAYER_FIELD_COMBO_TARGET,unitTarget->GetGUID());
        m_caster->SetUInt32Value(PLAYER_FIELD_BYTES,((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (0x01 << 8)));
    }
    else if(comboPoints < 5)
    {
        comboPoints += 1;
        m_caster->SetUInt32Value(PLAYER_FIELD_BYTES,((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (comboPoints << 8)));
    }

}

void Spell::EffectDuel(uint32 i)
{
    WorldPacket data;

    if(!m_caster) return;

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
        return;
    pGameObj->SetUInt32Value(OBJECT_FIELD_ENTRY, m_spellInfo->EffectMiscValue[i] );
    pGameObj->SetUInt32Value(OBJECT_FIELD_TYPE, 33 );
    pGameObj->SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);

    pGameObj->SetUInt32Value(GAMEOBJECT_DISPLAYID, 787 );
    pGameObj->SetUInt32Value(GAMEOBJECT_FACTION, ((Player*)m_caster)->getFaction() );
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
        Creature* Totem = NULL;

        if(Totem)
        {
            MapManager::Instance().GetMap(Totem->GetMapId())->Remove(Totem, true);
            Totem = NULL;
        }
    }

    Creature* pTotem = new Creature();

    if(pTotem->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT),
        m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(),
        m_caster->GetPositionZ(),m_caster->GetOrientation(),
        m_spellInfo->EffectMiscValue[i] ))
    {

        return;
    }

    pTotem->SetUInt32Value(UNIT_FIELD_LEVEL,m_caster->getLevel());
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

}

void Spell::EffectEnchantHeldItem(uint32 i)
{
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
    if(itemTarget->GetSlot() < EQUIPMENT_SLOT_END)
    {
        SendCastResult(CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM);
        return;
    }
    if(itemTarget->GetProto()->Class != m_spellInfo->EquippedItemClass)
    {
        SendCastResult(CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM);
        return;
    }

    for(uint32 x=0;x<8;x++)
    {
        if(m_spellInfo->Reagent[x] == 0)
            continue;
        uint32 itemid = m_spellInfo->Reagent[x];
        uint32 itemcount = m_spellInfo->ReagentCount[x];
        if( p_caster->HasItemCount(itemid,itemcount) )
            p_caster->DestroyItemCount(itemid, itemcount, true);
        else
        {
            SendCastResult(CAST_FAIL_ITEM_NOT_READY);
            return;
        }
    }

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

void Spell::EffectDisEnchant(uint32 i)
{
    Player* p_caster = (Player*)m_caster;
    if(!itemTarget)
        return;
    uint32 item_level = itemTarget->GetProto()->ItemLevel;
    uint32 item_quality = itemTarget->GetProto()->Quality;
    if(item_quality > 4 || item_quality < 2)
    {
        SendCastResult(CAST_FAIL_CANT_BE_DISENCHANTED);
        return;
    }
    if(itemTarget->GetProto()->Class != 2 || itemTarget->GetProto()->Class != 4)
    {
        SendCastResult(CAST_FAIL_CANT_BE_DISENCHANTED);
        return;
    }
    p_caster->DestroyItemCount(itemTarget->GetEntry(),1, true);

    Player *player = (Player*)m_caster;
    p_caster->UpdateSkillPro(m_spellInfo->Id);

    uint32 item;
    uint32 count;
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
    uint8 msg = player->CanStoreNewItem( 0, NULL_SLOT, dest, item, count, false );
    if( msg == EQUIP_ERR_OK )
        player->StoreNewItem( dest, item, count, true );
    else
        player->SendEquipError( msg, NULL, NULL );
    return ;
}

void Spell::EffectInebriate(uint32 i)
{
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
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;
    uint32 feelty = unitTarget->GetUInt32Value(UNIT_FIELD_POWER5);
    if(damage + feelty <  unitTarget->GetUInt32Value(UNIT_FIELD_MAXPOWER5))
        unitTarget->SetUInt32Value(UNIT_FIELD_POWER5,damage + feelty);
    else unitTarget->SetUInt32Value(UNIT_FIELD_POWER5,unitTarget->GetUInt32Value(UNIT_FIELD_MAXPOWER5));
    //trigger spell crash server.it must be added after fixed.
    //m_TriggerSpell = sSpellStore.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
}

void Spell::EffectDismissPet(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)m_caster;

    uint64 guid = _player->GetUInt64Value(UNIT_FIELD_SUMMON);
    Creature* pet = ObjectAccessor::Instance().GetCreature(*_player, guid);
    unitTarget = (Unit*)pet;
    if(pet)
    {
        _player->SavePet();
        _player->SetUInt64Value(UNIT_FIELD_SUMMON, 0);
        WorldPacket data;
        data.Initialize(SMSG_DESTROY_OBJECT);
        data << pet->GetGUID();
        _player->GetSession()->SendPacket(&data);
        MapManager::Instance().GetMap(pet->GetMapId())->Remove(pet,false);
        data.clear();
        data.Initialize(SMSG_PET_SPELLS);
        data << uint64(0);
        _player->GetSession()->SendPacket(&data);
        //pet->LoadFromDB(pet->GetGUIDLow());
        pet = NULL;
    }
    else _player->SetUInt64Value(UNIT_FIELD_SUMMON, 0);
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
        return;
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
    if(unitTarget->isAlive())
        return;
    if(!unitTarget->IsInWorld())
        return;

    uint32 health = m_spellInfo->EffectBasePoints[i];
    uint32 mana = m_spellInfo->EffectMiscValue[i];
    ((Player*)unitTarget)->setResurrect(m_caster->GetGUID(), unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), health, mana);
    SendResurrectRequest((Player*)unitTarget);
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

void Spell::EffectSkinning(uint32 i)
{
    if(unitTarget->GetTypeId() != TYPEID_UNIT )
        return;
    if(!m_caster)
        return;
    CreatureInfo *cinfo = ((Creature*)unitTarget)->GetCreatureInfo();

    if(cinfo->type != CREATURE_TYPE_BEAST && cinfo->type != CREATURE_TYPE_DRAGON)
    {
        SendCastResult(CAST_FAIL_INVALID_TARGET);
        return;
    }
    if(unitTarget->m_form == 99)
    {
        SendCastResult(CAST_FAIL_NOT_SKINNABLE);
        return;
    }
    int32 fishvalue = ((Player*)m_caster)->GetSkillValue(SKILL_SKINNING);
    int32 targetlevel = unitTarget->getLevel();
    if(fishvalue >= (targetlevel-5)*5)
    {
        ((Player*)m_caster)->SendLoot(unitTarget->GetGUID(),2);
    }else
    {
        SendCastResult(CAST_FAIL_FAILED);
        return;
    }
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
    assert(unitTarget);
    assert(m_caster);

    float x, y, z;
    unitTarget->GetClosePoint(m_caster, x, y, z);

    m_caster->SendMonsterMove(x, y, z, false,true,0.5);
    m_caster->Attack(unitTarget);
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
        return;

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
        if(((Player*)m_caster)->CheckFishingAble() > 0)
        {
            //pGameObj->SetUInt32Value(GAMEOBJECT_STATE, 0);
            pGameObj->SetUInt32Value(12, 0x3F6262A6 );
            pGameObj->SetUInt32Value(13, 0xBEEF0B9F );
            data.Initialize(SMSG_GAMEOBJECT_CUSTOM_ANIM);
            data<<uint64(pGameObj->GetGUID());
            data<<uint8(0);
            ((Player*)m_caster)->GetSession()->SendPacket(&data);
            //need fixed produre of fishing.
            ((Player*)m_caster)->SendLoot(pGameObj->GetGUID(),3);
            SendChannelUpdate(0);
        }
    }
}

void Spell::EffectSkill(uint32 i)
{
    Player *player = (Player*)m_caster;

    uint32 skill_id = m_spellInfo->EffectMiscValue[i];
    if(skill_id == SKILL_FISHING && up_skillvalue != 4)
        up_skillvalue = player->CheckFishingAble();
    if(skill_id == SKILL_SKINNING || skill_id == SKILL_FISHING || SKILL_HERBALISM)
    {
        switch(up_skillvalue)
        {
            case 0:
                return;
            case 1:
                break;
            case 2:
            {
                if(urand(1,10) >= 3)
                    return;
                else break;
            }
            case 3:
            {
                if(urand(1,10) >= 7)
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
