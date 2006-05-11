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

pEffect SpellEffects[TOTAL_SPELL_EFFECTS]=
{
    &Spell::EffectNULL,                                     //nothing
    &Spell::EffectInstaKill,                                //SPELL_EFFECT_INSTAKILL
    &Spell::EffectSchoolDMG,                                //SPELL_EFFECT_SCHOOL_DAMAGE
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DUMMY
    &Spell::EffectNULL,                                     //SPELL_EFFECT_PORTAL_TELEPORT
    &Spell::EffectTepeportUnits,                            //SPELL_EFFECT_TELEPORT_UNITS
    &Spell::EffectApplyAura,                                //SPELL_EFFECT_APPLY_AURA
    &Spell::EffectNULL,                                     //SPELL_EFFECT_ENVIRONMENTAL_DAMAGE
    &Spell::EffectManaDrain,                                //SPELL_EFFECT_MANA_DRAIN
    &Spell::EffectHealthLeach,                              //SPELL_EFFECT_HEALTH_LEECH
    &Spell::EffectHeal,                                     //SPELL_EFFECT_HEAL
    &Spell::EffectNULL,                                     //SPELL_EFFECT_BIND
    &Spell::EffectNULL,                                     //SPELL_EFFECT_PORTAL
    &Spell::EffectNULL,                                     //SPELL_EFFECT_RITUAL_BASE
    &Spell::EffectNULL,                                     //SPELL_EFFECT_RITUAL_SPECIALIZE
    &Spell::EffectNULL,                                     //SPELL_EFFECT_RITUAL_ACTIVATE_PORTAL
    &Spell::EffectNULL,                                     //SPELL_EFFECT_QUEST_COMPLETE
    &Spell::EffectWeaponDmg,                                //SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL
    &Spell::EffectResurrect,                                //SPELL_EFFECT_RESURRECT
    &Spell::EffectNULL,                                     //SPELL_EFFECT_ADD_EXTRA_ATTACKS
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DODGE
    &Spell::EffectNULL,                                     //SPELL_EFFECT_EVADE
    &Spell::EffectNULL,                                     //SPELL_EFFECT_PARRY
    &Spell::EffectNULL,                                     //SPELL_EFFECT_BLOCK
    &Spell::EffectCreateItem,                               //SPELL_EFFECT_CREATE_ITEM
    &Spell::EffectNULL,                                     //SPELL_EFFECT_WEAPON
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DEFENSE
    &Spell::EffectPresistentAA,                             //SPELL_EFFECT_PERSISTENT_AREA_AURA
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON
    &Spell::EffectMomentMove,                               //SPELL_EFFECT_LEAP
    &Spell::EffectEnergize,                                 //SPELL_EFFECT_ENERGIZE
    &Spell::EffectWeaponDmgPerc,                            //SPELL_EFFECT_WEAPON_PERCENT_DAMAGE
    &Spell::EffectNULL,                                     //SPELL_EFFECT_TRIGGER_MISSILE
    &Spell::EffectOpenLock,                                 //SPELL_EFFECT_OPEN_LOCK
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_MOUNT_OBSOLETE
    &Spell::EffectApplyAA,                                  //SPELL_EFFECT_APPLY_AREA_AURA
    &Spell::EffectLearnSpell,                               //SPELL_EFFECT_LEARN_SPELL
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SPELL_DEFENSE
    &Spell::EffectDispel,                                   //SPELL_EFFECT_DISPEL
    &Spell::EffectNULL,                                     //SPELL_EFFECT_LANGUAGE
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DUAL_WIELD
    &Spell::EffectSummonWild,                               //SPELL_EFFECT_SUMMON_WILD
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_GUARDIAN
    &Spell::EffectNULL,                                     //SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER
    &Spell::EffectLearnSkill,                               //SPELL_EFFECT_SKILL_STEP
    &Spell::EffectNULL,                                     //unknown45
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SPAWN
    &Spell::EffectNULL,                                     //SPELL_EFFECT_TRADE_SKILL
    &Spell::EffectNULL,                                     //SPELL_EFFECT_STEALTH
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DETECT
    &Spell::EffectTransmitted,                              //SPELL_EFFECT_TRANS_DOOR
    &Spell::EffectNULL,                                     //SPELL_EFFECT_FORCE_CRITICAL_HIT
    &Spell::EffectNULL,                                     //SPELL_EFFECT_GUARANTEE_HIT
    &Spell::EffectEnchantItemPerm,                          //SPELL_EFFECT_ENCHANT_ITEM
    &Spell::EffectEnchantItemTmp,                           //SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
    &Spell::EffectNULL,                                     //SPELL_EFFECT_TAMECREATURE
    &Spell::EffectSummonPet,                                //SPELL_EFFECT_SUMMON_PET
    &Spell::EffectNULL,                                     //SPELL_EFFECT_LEARN_PET_SPELL
    &Spell::EffectWeaponDmg,                                //SPELL_EFFECT_WEAPON_DAMAGE
    &Spell::EffectOpenSecretSafe,                           //SPELL_EFFECT_OPEN_LOCK_ITEM
    &Spell::EffectNULL,                                     //SPELL_EFFECT_PROFICIENCY
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SEND_EVENT
    &Spell::EffectPowerDrain,                               //SPELL_EFFECT_POWER_BURN
    &Spell::EffectNULL,                                     //SPELL_EFFECT_THREAT
    &Spell::EffectTriggerSpell,                             //SPELL_EFFECT_TRIGGER_SPELL
    &Spell::EffectNULL,                                     //SPELL_EFFECT_HEALTH_FUNNEL
    &Spell::EffectNULL,                                     //SPELL_EFFECT_POWER_FUNNEL
    &Spell::EffectHealMaxHealth,                            //SPELL_EFFECT_HEAL_MAX_HEALTH
    &Spell::EffectInterruptCast,                            //SPELL_EFFECT_INTERRUPT_CAST
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DISTRACT
    &Spell::EffectNULL,                                     //SPELL_EFFECT_PULL
    &Spell::EffectNULL,                                     //SPELL_EFFECT_PICKPOCKET
    &Spell::EffectNULL,                                     //SPELL_EFFECT_ADD_FARSIGHT
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_POSSESSED
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_TOTEM
    &Spell::EffectNULL,                                     //SPELL_EFFECT_HEAL_MECHANICAL
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_OBJECT_WILD
    &Spell::EffectScriptEffect,                             //SPELL_EFFECT_SCRIPT_EFFECT
    &Spell::EffectNULL,                                     //SPELL_EFFECT_ATTACK
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SANCTUARY
    &Spell::EffectAddComboPoints,                           //SPELL_EFFECT_ADD_COMBO_POINTS
    &Spell::EffectNULL,                                     //SPELL_EFFECT_CREATE_HOUSE
    &Spell::EffectNULL,                                     //SPELL_EFFECT_BIND_SIGHT
    &Spell::EffectDuel,                                     //SPELL_EFFECT_DUEL
    &Spell::EffectNULL,                                     //SPELL_EFFECT_STUCK
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_PLAYER
    &Spell::EffectNULL,                                     //SPELL_EFFECT_ACTIVATE_OBJECT
    &Spell::EffectSummonTotem,                              //SPELL_EFFECT_SUMMON_TOTEM_SLOT1
    &Spell::EffectSummonTotem,                              //SPELL_EFFECT_SUMMON_TOTEM_SLOT2
    &Spell::EffectSummonTotem,                              //SPELL_EFFECT_SUMMON_TOTEM_SLOT3
    &Spell::EffectSummonTotem,                              //SPELL_EFFECT_SUMMON_TOTEM_SLOT4
    &Spell::EffectNULL,                                     //SPELL_EFFECT_THREAT_ALL
    &Spell::EffectEnchantHeldItem,                          //SPELL_EFFECT_ENCHANT_HELD_ITEM
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_PHANTASM
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SELF_RESURRECT
    &Spell::EffectSkinning,                                 //SPELL_EFFECT_SKINNING
    &Spell::EffectCharge,                                   //SPELL_EFFECT_CHARGE
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_CRITTER
    &Spell::EffectNULL,                                     //SPELL_EFFECT_KNOCK_BACK
    &Spell::EffectDisEnchant,                               //SPELL_EFFECT_DISENCHANT
    &Spell::EffectNULL,                                     //SPELL_EFFECT_INEBRIATE
    &Spell::EffectTriggerSpell,                             //SPELL_EFFECT_FEED_PET
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DISMISS_PET
    &Spell::EffectNULL,                                     //SPELL_EFFECT_REPUTATION
    &Spell::EffectSummonObject,                             //SPELL_EFFECT_SUMMON_OBJECT_SLOT1
    &Spell::EffectSummonObject,                             //SPELL_EFFECT_SUMMON_OBJECT_SLOT2
    &Spell::EffectSummonObject,                             //SPELL_EFFECT_SUMMON_OBJECT_SLOT3
    &Spell::EffectSummonObject,                             //SPELL_EFFECT_SUMMON_OBJECT_SLOT4
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DISPEL_MECHANIC
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_DEAD_PET
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DESTROY_ALL_TOTEMS
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DURABILITY_DAMAGE
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SUMMON_DEMON
    &Spell::EffectNULL,                                     //SPELL_EFFECT_RESURRECT_NEW
    &Spell::EffectAttackMe,                                 //SPELL_EFFECT_ATTACK_ME
    &Spell::EffectNULL,                                     //SPELL_EFFECT_DURABILITY_DAMAGE_PCT
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SKIN_PLAYER_CORPSE
    &Spell::EffectNULL,                                     //SPELL_EFFECT_SPIRIT_HEAL
    &Spell::EffectSkill,                                    //SPELL_EFFECT_SKILL -- professions and more
    &Spell::EffectNULL,                                     //SPELL_EFFECT_APPLY_AURA_NEW
    &Spell::EffectNULL,                                     //SPELL_EFFECT_TELEPORT_GRAVEYARD
    &Spell::EffectWeaponDmg                                 //SPELL_EFFECT_ADICIONAL_DMG
};

void Spell::EffectNULL(uint32 i)
{
}

void Spell::EffectInstaKill(uint32 i)
{
    if(!unitTarget) return;
    if(!unitTarget->isAlive()) return;
    uint32 health = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
    m_caster->DealDamage(unitTarget, health, 0);
}

void Spell::EffectSchoolDMG(uint32 i)
{

    if(!unitTarget) return;
    if(!unitTarget->isAlive()) return;

    uint32 baseDamage = m_spellInfo->EffectBasePoints[i];

    uint32 randomDamage = rand()%m_spellInfo->EffectDieSides[i];
    uint32 damage = baseDamage+randomDamage;

    m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, damage);
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

void Spell::EffectTepeportUnits(uint32 i)
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

    sLog.outDebug("Apply Auraname is: %u", m_spellInfo->EffectApplyAuraName[i]);

    if(m_spellInfo->Id == 2457)
    {
        unitTarget->SetUInt32Value(UNIT_FIELD_BYTES_1, 0x0011EE00 );
        return;
    }

    else if(m_spellInfo->Id == 71)
    {
        unitTarget->SetUInt32Value(UNIT_FIELD_BYTES_1, 0x0012EE00 );
        return;
    }

    else if(m_spellInfo->Id == 2458)
    {
        unitTarget->SetUInt32Value(UNIT_FIELD_BYTES_1, 0x0013EE00 );
        return;
    }

    //int32 duration = GetDuration(m_spellInfo, i);
    Aura* Aur = new Aura(m_spellInfo, i, m_caster, unitTarget);
    unitTarget->AddAura(Aur);
}

void Spell::EffectManaDrain(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    uint32 curPower = unitTarget->GetUInt32Value(UNIT_FIELD_POWER1);
    if(curPower < damage)
        unitTarget->SetUInt32Value(UNIT_FIELD_POWER1,0);
    else
        unitTarget->SetUInt32Value(UNIT_FIELD_POWER1,curPower-damage);
}

void Spell::EffectPowerDrain(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    uint32 curPower = unitTarget->GetUInt32Value(UNIT_FIELD_POWER1);
    uint32 curHealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
    if(curPower < damage)
        unitTarget->SetUInt32Value(UNIT_FIELD_POWER1,0);
    else
        unitTarget->SetUInt32Value(UNIT_FIELD_POWER1,curPower-damage);
    unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,curHealth-damage/2);

}

void Spell::EffectHeal(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    uint32 curHealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 maxHealth = unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(curHealth+damage > maxHealth)
        unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,maxHealth);
    else
        unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,curHealth+damage);

}

void Spell::EffectHealthLeach(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    //okey Touch of Death works

    sLog.outDebug("HealthLeach :%u", damage);
    uint32 dHealth = damage;                                //something like this //maybe some other things are needed

    //Please let me know if this is correct
    m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, dHealth);

    uint32 curHealth = m_caster->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 maxHealth = m_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH);

    if ((curHealth + (dHealth/2)) < maxHealth)
        m_caster->SetUInt32Value(UNIT_FIELD_HEALTH,unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH) + (dHealth/2));
    else
        m_caster->SetUInt32Value(UNIT_FIELD_HEALTH,maxHealth);
}

void Spell::EffectCreateItem(uint32 i)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    Player* player = (Player*)m_caster;
    uint32 newitemid, itemid, itemcount;
    if((newitemid = m_spellInfo->EffectItemType[i]) == 0)
        return;

    for(uint32 x=0;x<8;x++)
    {
        if(m_spellInfo->Reagent[x] == 0)
            continue;
        itemid = m_spellInfo->Reagent[x];
        itemcount = m_spellInfo->ReagentCount[x];
        if(player->GetItemCount(itemid, false) >= itemcount && player->CanAddItemCount(newitemid) >= 1 )
            player->RemoveItemFromInventory(itemid, itemcount);
        else
        {
            SendCastResult(CAST_FAIL_ITEM_NOT_READY);
            return;
        }
    }

    //uint32 num_to_add = ((player->getLevel() - (m_spellInfo->spellLevel-1))*2);
    //if (m_itemProto->Class != ITEM_CLASS_CONSUMABLE)
    //   num_to_add = 1;
    //   if(num_to_add > m_itemProto->MaxCount)
    //       num_to_add = m_itemProto->MaxCount;
    if(! player->AddNewItem(newitemid, 1, false))
    {
        SendCastResult(CAST_FAIL_TOO_MANY_OF_THAT_ITEM_ALREADY);
        return;
    }
    //should send message "create item" to client.-FIX ME
    SkillLineAbility *pSkill;
    pSkill = sSkillLineAbilityStore.LookupEntry(m_spellInfo->Id);
    uint32 minValue = pSkill->min_value;
    uint32 maxValue = pSkill->max_value;
    uint32 skill_id = pSkill->miscid;
    player->UpdateSkillPro(skill_id,minValue,maxValue);
}

void Spell::EffectPresistentAA(uint32 i)
{

    if(m_AreaAura == true)
        return;

    m_AreaAura = true;

    DynamicObject* dynObj = new DynamicObject();
    if(dynObj->Create(objmgr.GenerateLowGuid(HIGHGUID_DYNAMICOBJECT), m_caster, m_spellInfo, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, GetDuration(m_spellInfo, i)))
        return;
    dynObj->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    dynObj->SetUInt32Value(GAMEOBJECT_DISPLAYID, 368003);
    dynObj->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x01eeeeee);
    dynObj->PeriodicTriggerDamage(damage, m_spellInfo->EffectAmplitude[i], GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i])));
    m_dynObjToDel.push_back(dynObj);
    dynObj->AddToWorld();
    MapManager::Instance().GetMap(dynObj->GetMapId())->Add(dynObj);

}

void Spell::EffectEnergize(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;
    uint32 POWER_TYPE;

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
    if(POWER_TYPE == UNIT_FIELD_POWER2)
        damage = damage*10;

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
        SkillLineAbility *pSkill;
        pSkill = sSkillLineAbilityStore.LookupEntry(m_spellInfo->Id);
        uint32 minValue = pSkill->min_value;
        uint32 maxValue = pSkill->max_value;
        uint32 skill_id = pSkill->miscid;
        ((Player*)m_caster)->UpdateSkillPro(skill_id,minValue,maxValue);
    }
    ((Player*)m_caster)->SendLoot(gameObjTarget->GetGUID(),loottype);

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

void Spell::EffectLearnSpell(uint32 i)
{
    WorldPacket data;

    if(!unitTarget)
        return;
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        unitTarget = m_targets.getUnitTarget();

    uint32 spellToLearn = m_spellInfo->EffectTriggerSpell[i];
    //data.Initialize(SMSG_LEARNED_SPELL);
    //data << spellToLearn;
    //((Player*)unitTarget)->GetSession()->SendPacket(&data);
    ((Player*)unitTarget)->learnSpell((uint16)spellToLearn);
    //some addspell isn't needed if you have a good DB,FISHING && MINING && HERBALISM have to be needed.
    switch(spellToLearn)
    {
        case 4036:                                          //SKILL_ENGINERING
        {
            ((Player*)unitTarget)->learnSpell(3918);
            ((Player*)unitTarget)->learnSpell(3919);
            ((Player*)unitTarget)->learnSpell(3920);
            break;
        }
        case 3908:                                          //SKILL_TAILORING
        {
            ((Player*)unitTarget)->learnSpell(2387);
            ((Player*)unitTarget)->learnSpell(2963);
            break;
        }
        case 7411:                                          //SKILL_ENCHANTING
        {
            ((Player*)unitTarget)->learnSpell(7418);
            ((Player*)unitTarget)->learnSpell(7421);
            ((Player*)unitTarget)->learnSpell(13262);
            break;
        }
        case 2259:                                          //SKILL_ALCHEMY
        {
            ((Player*)unitTarget)->learnSpell(2329);
            ((Player*)unitTarget)->learnSpell(7183);
            ((Player*)unitTarget)->learnSpell(2330);
            break;
        }
        case 2018:                                          //SKILL_BLACKSMITHING
        {
            ((Player*)unitTarget)->learnSpell(2663);
            ((Player*)unitTarget)->learnSpell(12260);
            ((Player*)unitTarget)->learnSpell(2660);
            ((Player*)unitTarget)->learnSpell(3115);
            break;
        }
        case 2108:                                          //SKILL_LEATHERWORKING
        {
            ((Player*)unitTarget)->learnSpell(2152);
            ((Player*)unitTarget)->learnSpell(9058);
            ((Player*)unitTarget)->learnSpell(9059);
            ((Player*)unitTarget)->learnSpell(2149);
            ((Player*)unitTarget)->learnSpell(7126);
            ((Player*)unitTarget)->learnSpell(2881);
            break;
        }
        case 2550:                                          //SKILL_COOKING
        {
            ((Player*)unitTarget)->learnSpell(2540);
            ((Player*)unitTarget)->learnSpell(2538);
            break;
        }
        case 3273:                                          //SKILL_FIRST_AID
        {
            ((Player*)unitTarget)->learnSpell(3275);
            break;
        }
        case 7620:                                          //SKILL_FISHING
        {
            ((Player*)unitTarget)->learnSpell(7738);
            break;
        }
        case 2575:                                          //SKILL_MINING
        {
            ((Player*)unitTarget)->learnSpell(2580);
            ((Player*)unitTarget)->learnSpell(2656);
            ((Player*)unitTarget)->learnSpell(2657);
            break;
        }
        case 2366:                                          //SKILL_HERBALISM
        {
            ((Player*)unitTarget)->learnSpell(2383);
            break;
        }
        default:break;
    }
    sLog.outDebug( "Spell: Player %u have learned spell %u from NpcGUID=%u", ((Player*)unitTarget)->GetGUIDLow(), spellToLearn, m_caster->GetGUIDLow() );
}

void Spell::EffectDispel(uint32 i)
{
    m_caster->RemoveFirstAuraByCategory(m_spellInfo->EffectMiscValue[i]);
}

void Spell::EffectSummonWild(uint32 i)
{
    if(!unitTarget)
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

    spawnCreature->SetUInt32Value(UNIT_NPC_FLAGS , 0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_HEALTH, 28 + 30*level);
    spawnCreature->SetUInt32Value(UNIT_FIELD_MAXHEALTH, 28 + 30*level);
    spawnCreature->SetUInt32Value(UNIT_FIELD_LEVEL , level);
    spawnCreature->AIM_Initialize();
    sLog.outError("AddObject at Spell.cpp");
    MapManager::Instance().GetMap(spawnCreature->GetMapId())->Add(spawnCreature);

}

void Spell::EffectLearnSkill(uint32 i)
{
    uint32 skillid =  m_spellInfo->EffectMiscValue[i];
    uint16 skillval = ((Player*)unitTarget)->GetSkillValue(skillid);
    ((Player*)unitTarget)->SetSkill(skillid,skillval?skillval:1,damage*75);
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
            if(p_caster->GetItemBySlot(j) != 0 && p_caster->GetItemBySlot(j)->GetProto()->ItemId == itemTarget->GetEntry())
        {
            itemTarget = p_caster->GetItemBySlot(j);
            item_slot = j;
        }
    }
    if(itemTarget->GetProto()->Class != m_spellInfo->EquippedItemClass || itemTarget->GetProto()->SubClass != m_spellInfo->EquippedItemSubClass)
    {
        SendCastResult(CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM);
        return;
    }

    for(add_slot = 0; add_slot < 22; /*add_slot++*/add_slot+=3)
        if (!itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot))
            break;

    if (add_slot < 32)
    {
        for(uint8 j=0;j<3;j++)
            if (m_spellInfo->EffectMiscValue[j])
                itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+(add_slot+j), m_spellInfo->EffectMiscValue[j]);

        p_caster->ApplyItemMods( itemTarget, item_slot, true );
        itemTarget->SendUpdateToPlayer((Player *)p_caster);

        Player *player = (Player*)m_caster;
        SkillLineAbility *pSkill;
        pSkill = sSkillLineAbilityStore.LookupEntry(m_spellInfo->Id);
        uint32 minValue = pSkill->min_value;
        uint32 maxValue = pSkill->max_value;
        uint32 skill_id = pSkill->miscid;
        player->UpdateSkillPro(skill_id,minValue,maxValue);
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
            if(p_caster->GetItemBySlot(j) != 0 && p_caster->GetItemBySlot(j)->GetProto()->ItemId == itemTarget->GetEntry())
        {
            itemTarget = p_caster->GetItemBySlot(j);
            item_slot = j;
        }
    }
    if(itemTarget->GetProto()->Class != m_spellInfo->EquippedItemClass || itemTarget->GetProto()->SubClass != m_spellInfo->EquippedItemSubClass)
    {
        SendCastResult(CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM);
        return;
    }

    for(add_slot = 0; add_slot < 22; /*add_slot++*/add_slot+=3)
        if (!m_CastItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot))
            break;

    if (add_slot < 32)
    {
        for(uint8 j=0;j<3;j++)
            if (m_spellInfo->EffectMiscValue[j])
                itemTarget->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+(add_slot+j), m_spellInfo->EffectMiscValue[j]);

        p_caster->ApplyItemMods( itemTarget, item_slot, true );
        itemTarget->SendUpdateToPlayer((Player *)p_caster);

        Player *player = (Player*)m_caster;
        SkillLineAbility *pSkill;
        pSkill = sSkillLineAbilityStore.LookupEntry(m_spellInfo->Id);
        uint32 minValue = pSkill->min_value;
        uint32 maxValue = pSkill->max_value;
        uint32 skill_id = pSkill->miscid;
        player->UpdateSkillPro(skill_id,minValue,maxValue);
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

                for(uint32 i=0;i<UNIT_MAX_SPELLS;i++)
                                                            //C100 = maybe group
                    data << uint16 (OldSummon->m_spells[i]) << uint16 (0xC100);

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
        //NewSummon->SetUInt32Value(UNIT_FIELD_STAT2,25);
        //NewSummon->SetUInt32Value(UNIT_FIELD_STAT3,28);
        NewSummon->SetUInt32Value(UNIT_FIELD_STAT4,27);
        std::string name;
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            name = ((Player*)m_caster)->GetName();
        else
            name = ((Creature*)m_caster)->GetCreatureInfo()->Name;
        name.append("\\\'s Pet");
        NewSummon->SetName( name );
        NewSummon->SetFealty( 10 );
        for(uint32 i=0;i<UNIT_MAX_SPELLS;i++)
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

            for(uint32 i=0;i<UNIT_MAX_SPELLS;i++)
                                                            //C100 = maybe group
                data << uint16 (NewSummon->m_spells[i]) << uint16 (0xC100);

            data << uint16 (2) << uint16(State << 8) << uint16 (1) << uint16(State << 8) << uint16 (0) << uint16(State << 8);

            ((Player*)m_caster)->GetSession()->SendPacket(&data);
        }
    }
}

void Spell::EffectAttackMe(uint32 i)
{
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
    {
        unitTarget->Relocate(unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), unitTarget->GetAngle(m_caster));
        ((Creature*)unitTarget)->AI().AttackStart(m_caster);
    }
}

void Spell::EffectWeaponDmg(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    float minDmg,maxDmg;
    minDmg = maxDmg = 0;

    if(m_spellInfo->rangeIndex == 1 || m_spellInfo->rangeIndex == 2 || m_spellInfo->rangeIndex == 7)
    {
        minDmg = m_caster->GetFloatValue(UNIT_FIELD_MINDAMAGE);
        maxDmg = m_caster->GetFloatValue(UNIT_FIELD_MAXDAMAGE);
    }
    else
    {
        if(m_caster->GetTypeId() != TYPEID_PLAYER)
        {
            minDmg = m_caster->GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE);
            maxDmg = m_caster->GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE);
        }
        else
        {
            uint8 slot=EQUIPMENT_SLOT_RANGED;
            Item *equipitem = ((Player*)m_caster)->GetItemBySlot(slot);
            Item *ammoitem = ((Player*)m_caster)->GetItemByItemType(INVTYPE_AMMO);
            Item *stackitem = NULL;

            if(!equipitem) return;

            uint32 equipInvType = equipitem->GetProto()->InventoryType;

            minDmg = equipitem->GetProto()->Damage[0].DamageMin;
            maxDmg = equipitem->GetProto()->Damage[0].DamageMax;

            if(equipInvType == INVTYPE_THROWN)
                stackitem = equipitem;
            else
                stackitem = ammoitem;

            slot = ((Player*)m_caster)->GetSlotByItemGUID(stackitem->GetGUID());
            if(stackitem)
            {
                uint32 ItemCount = stackitem->GetCount();
                uint32 ItemId = stackitem->GetProto()->ItemId;

                if (ItemCount > 1)
                {
                    stackitem->SetCount(ItemCount-1);
                    //m_TriggerSpell = m_spellInfo;
                }
                else
                {
                    ((Player*)m_caster)->RemoveItemFromSlot(0,slot);
                    //stackitem->DeleteFromDB();
                    //if(equipInvType == INVTYPE_THROWN)
                    //	stackitem = ((Player*)m_caster)->GetItemByItemType(INVTYPE_THROWN)
                    //else
                    //	stackitem = ((Player*)m_caster)->GetItemByItemType(INVTYPE_AMMO)
                    //if(!stackitem)
                    //{
                    delete stackitem;
                    //}
                }
            }
        }
    }

    uint32 randDmg = (uint32)(maxDmg-minDmg);
    uint32 dmg = (uint32)minDmg;
    if(randDmg > 1)
        dmg += rand()%randDmg;
    dmg += damage;

    m_caster->SpellNonMeleeDamageLog(unitTarget,m_spellInfo->Id,dmg);

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

void Spell::EffectHealMaxHealth(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    uint32 heal;
    heal = m_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH);

    uint32 curHealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 maxHealth = unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(curHealth+heal > maxHealth)
        unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,maxHealth);
    else
        unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,curHealth+heal);

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
                m_spellInfo->EffectItemType[0] = 5512;      //spell 6261;	//primary healstone
                break;
            case 6202:
                m_spellInfo->EffectItemType[0] = 5511;      //spell 6262;	//inferior healstone
                break;
            case 5699:
                m_spellInfo->EffectItemType[0] = 5509;      //spell 5720;	//healstone
                break;
            case 11729:
                m_spellInfo->EffectItemType[0] = 5510;      //spell 5723;	//strong healstone
                break;
            case 11730:
                m_spellInfo->EffectItemType[0] = 9421;      //spell 11732;	//super healstone
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

    if(!m_caster)
        return;

    WorldPacket data;

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

    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);

    Player *caster = (Player*)m_caster;
    Player *target = (Player*)unitTarget;

    data.Initialize(SMSG_DUEL_REQUESTED);
    data << pGameObj->GetGUID() << caster->GetGUID();
    target->GetSession()->SendPacket(&data);

    caster->SetDuelVs(target);
    target->SetDuelVs(caster);

    caster->SetInDuel(false);
    target->SetInDuel(false);

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
            if(p_caster->GetItemBySlot(j) != 0 && p_caster->GetItemBySlot(j)->GetProto()->ItemId == itemTarget->GetEntry())
        {
            itemTarget = p_caster->GetItemBySlot(j);
            item_slot = j;
        }
    }
    if(itemTarget->GetProto()->Class != m_spellInfo->EquippedItemClass || itemTarget->GetProto()->SubClass != m_spellInfo->EquippedItemSubClass)
    {
        SendCastResult(CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM);
        return;
    }

    for(add_slot = 0; add_slot < 22; /*add_slot++*/add_slot+=3)
    {
        if (!itemTarget->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot))
            break;
    }

    if (add_slot < 32)
    {
        for(uint8 j=0;j<3;j++)
            if (m_spellInfo->EffectMiscValue[j])
                m_CastItem->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+(add_slot+j), m_spellInfo->EffectMiscValue[j]);

        p_caster->ApplyItemMods( itemTarget, item_slot, true );
        itemTarget->SendUpdateToPlayer((Player *)p_caster);
    }

}

void Spell::EffectDisEnchant(uint32 i)
{
    Player* p_caster = (Player*)m_caster;
    if(!itemTarget)
        return;
    p_caster->RemoveItemFromInventory(itemTarget->GetEntry(),1);

    Player *player = (Player*)m_caster;
    SkillLineAbility *pSkill;
    pSkill = sSkillLineAbilityStore.LookupEntry(m_spellInfo->Id);
    uint32 minValue = pSkill->min_value;
    uint32 maxValue = pSkill->max_value;
    uint32 skill_id = pSkill->miscid;
    player->UpdateSkillPro(skill_id,minValue,maxValue);

    uint32 item_level = itemTarget->GetProto()->ItemLevel;
    uint32 item_quality = itemTarget->GetProto()->Quality;
    if(item_level >= 51 && item_level <= 60)
    {
        if(item_quality == 4)
        {
            p_caster->AddNewItem(14344,urand(3,5),false);
            return;
        }
        else if(item_quality == 3)
        {
            p_caster->AddNewItem(14344,1,false);
            return;
        }
        else if(item_quality == 2)
        {
            if(urand(1,100)< 85)
            {
                p_caster->AddNewItem(16204,urand(1,(item_level/10)),false);
                return;
            }
            else
            {
                p_caster->AddNewItem(16203,urand(1,(item_level/10)),false);
                return;
            }
        }

    }
    else if(item_level >= 46 && item_level <= 50)
    {
        if(item_quality == 4)
        {
            p_caster->AddNewItem(14343,urand(3,5),false);
            return;
        }
        else if(item_quality == 3)
        {
            p_caster->AddNewItem(14343,1,false);
            return;
        }
        else if(item_quality == 2)
        {
            if(urand(1,100)< 85)
            {
                p_caster->AddNewItem(11176,urand(1,(item_level/10)),false);
                return;
            }
            else
            {
                p_caster->AddNewItem(16202,urand(1,(item_level/10)),false);
                return;
            }
        }

    }
    else if(item_level >= 41 && item_level <= 45)
    {
        if(item_quality == 4)
        {
            p_caster->AddNewItem(11178,urand(3,5),false);
            return;
        }
        else if(item_quality == 3)
        {
            p_caster->AddNewItem(11178,1,false);
            return;
        }
        else if(item_quality == 2)
        {
            if(urand(1,100)< 85)
            {
                p_caster->AddNewItem(11176,urand(1,(item_level/10)),false);
                return;
            }
            else
            {
                p_caster->AddNewItem(11175,urand(1,(item_level/10)),false);
                return;
            }
        }
    }
    else if(item_level >= 36 && item_level <= 40)
    {
        if(item_quality == 4)
        {
            p_caster->AddNewItem(11177,urand(3,5),false);
            return;
        }
        else if(item_quality == 3)
        {
            p_caster->AddNewItem(11177,1,false);
            return;
        }
        else if(item_quality == 2)
        {
            if(urand(1,100)< 85)
            {
                p_caster->AddNewItem(11137,urand(1,(item_level/10)),false);
                return;
            }
            else
            {
                p_caster->AddNewItem(11174,urand(1,(item_level/10)),false);
                return;
            }
        }
    }
    else if(item_level >= 31 && item_level <= 35)
    {
        if(item_quality == 4)
        {
            p_caster->AddNewItem(11139,urand(3,5),false);
            return;
        }
        else if(item_quality == 3)
        {
            p_caster->AddNewItem(11139,1,false);
            return;
        }
        else if(item_quality == 2)
        {
            if(urand(1,100)< 85)
            {
                p_caster->AddNewItem(11137,urand(1,(item_level/10)),false);
                return;
            }
            else
            {
                p_caster->AddNewItem(11135,urand(1,(item_level/10)),false);
                return;
            }
        }
    }
    else if(item_level >= 25 && item_level <= 30)
    {
        if(item_quality == 4)
        {
            p_caster->AddNewItem(11138,urand(3,5),false);
            return;
        }
        else if(item_quality == 3)
        {
            p_caster->AddNewItem(11138,1,false);
            return;
        }
        else if(item_quality == 2)
        {
            if(urand(1,100)< 85)
            {
                p_caster->AddNewItem(11083,urand(1,(item_level/10)),false);
                return;
            }
            else
            {
                p_caster->AddNewItem(11134,urand(1,(item_level/10)),false);
                return;
            }
        }
    }
    else if(item_level >= 21 && item_level <= 25)
    {
        if(item_quality == 4)
        {
            p_caster->AddNewItem(11084,urand(3,5),false);
            return;
        }
        else if(item_quality == 3)
        {
            p_caster->AddNewItem(11084,1,false);
            return;
        }
        else if(item_quality == 2)
        {
            if(urand(1,100)< 85)
            {
                p_caster->AddNewItem(11083,urand(1,(item_level/10)),false);
                return;
            }
            else
            {
                p_caster->AddNewItem(11082,urand(1,(item_level/10)),false);
                return;
            }
        }
    }
    else if(item_level >= 1 && item_level <= 20)
    {
        if(item_quality == 4)
        {
            p_caster->AddNewItem(10978,urand(3,5),false);
            return;
        }
        else if(item_quality == 3 && item_level >=16)
        {
            p_caster->AddNewItem(10978,1,false);
            return;
        }
        else if(item_quality == 2)
        {
            if(urand(1,100)< 70)
            {
                p_caster->AddNewItem(10940,urand(1,3),false);
                return;
            }
            else if(item_level <=15 && urand(1,100)< 70 )
            {
                p_caster->AddNewItem(10938,urand(1,3),false);
                return;
            }
            else if(urand(1,100)< 50)
            {
                p_caster->AddNewItem(10939,urand(1,3),false);
                return;
            }
            else
            {
                p_caster->AddNewItem(10998,urand(1,3),false);
                return;
            }
        }
    }
    return ;
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
    ((Player*)unitTarget)->setResurrect(m_caster->GetGUID(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
    SendResurrectRequest((Player*)unitTarget);
}

void Spell::EffectMomentMove(uint32 i)
{
    if( m_spellInfo->rangeIndex== 1)                        //self range
    {
        float fx,fy;
        WorldPacket data;

        float dis = GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
        fx = m_caster->GetPositionX() + dis * cos(m_caster->GetOrientation());
        fy = m_caster->GetPositionY() + dis * sin(m_caster->GetOrientation());

        //TODO:Use client Height map fix Z position
        m_caster->BuildTeleportAckMsg(&data,fx,fy, m_caster->GetPositionZ()+10,m_caster->GetOrientation());
        m_caster->SendMessageToSet( &data, true );
    }
}

void Spell::EffectSkinning(uint32 i)
{
    assert(unitTarget);
    assert(m_caster);

    if(((Player*)m_caster)->GetSkillValue(SKILL_SKINNING) >= (unitTarget->getLevel()-1)*5)
    {
        ((Player*)m_caster)->SendLoot(unitTarget->GetGUID(),2);
    }else
    {
        SendCastResult(CAST_FAIL_FAILED);

    }

}

void Spell::EffectCharge(uint32 i)
{
    assert(unitTarget);
    assert(m_caster);

    float x, y, z;
    unitTarget->GetClosePoint(m_caster, x, y, z);
    float oldspeed = m_caster->GetSpeed();
    m_caster->SetSpeed(oldspeed * 3.5);
    m_caster->SendMoveToPacket(x, y, z, true);
    m_caster->addUnitState(UNIT_STAT_ATTACKING);
    m_caster->addAttacker(unitTarget);
    m_caster->SetSpeed(oldspeed);
    //m_caster->smsg_AttackStart(pEnemy);
}

void Spell::EffectTransmitted(uint32 i)
{
    float fx,fy;
    WorldPacket data;

    float dis = GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
    fx = m_caster->GetPositionX() + dis * cos(m_caster->GetOrientation());
    fy = m_caster->GetPositionY() + dis * sin(m_caster->GetOrientation());

    GameObject* pGameObj = new GameObject();
    uint32 name_id = m_spellInfo->EffectMiscValue[i];

    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), name_id,m_caster->GetMapId(),
        fx, fy, m_caster->GetPositionZ(),
        m_caster->GetOrientation(), 0, 0, 0, 0))
        return;

    pGameObj->SetUInt32Value(OBJECT_FIELD_ENTRY, m_spellInfo->EffectMiscValue[i] );
    pGameObj->SetUInt32Value(OBJECT_FIELD_TYPE, 33 );
    pGameObj->SetUInt32Value(OBJECT_FIELD_CREATED_BY, m_caster->GetGUIDLow() );
    pGameObj->SetUInt32Value(12, 0x3F63BB3C );
    pGameObj->SetUInt32Value(13, 0xBEE9E017 );
    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel() );

    DEBUG_LOG("AddObject at SpellEfects.cpp EffectTransmitted\n");
    m_ObjToDel.push_back(pGameObj);

    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);
    pGameObj->AddToWorld();

    data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
    data << (uint64)pGameObj->GetGUID();
    m_caster->SendMessageToSet(&data,true);
}

void Spell::EffectSkill(uint32)
{
    Player *player = (Player*)m_caster;
    SkillLineAbility *pSkill;
    pSkill = sSkillLineAbilityStore.LookupEntry(m_spellInfo->Id);
    uint32 minValue = pSkill->min_value;
    uint32 maxValue = pSkill->max_value;
    uint32 skill_id = pSkill->miscid;
    player->UpdateSkillPro(skill_id,minValue,maxValue);

    //((Player*)m_caster)->UpdateSkill(m_spellInfo->EffectMiscValue[1]);

}
