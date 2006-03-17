/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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
#include "Spell.h"
#include "DynamicObject.h"
#include "Affect.h"
#include "Group.h"
#include "UpdateData.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "RedZoneDistrict.h"
#include "CellImpl.h"


void Spell::EffectSchoolDMG(uint32 i)
{
	
	if(!unitTarget) return;
	if(!unitTarget->isAlive()) return;

	uint32 baseDamage = m_spellInfo->EffectBasePoints[i];

	

	uint32 randomDamage = rand()%m_spellInfo->EffectDieSides[i];
	uint32 damage = baseDamage+randomDamage;

	m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, damage);
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
                                
	if(unitTarget->tmpAffect == 0)
	{
		Affect* aff = new Affect(m_spellInfo,GetDuration(sSpellDuration.LookupEntry(m_spellInfo->DurationIndex)),m_caster->GetGUID());
		unitTarget->tmpAffect = aff;
	}
                                
	if(m_spellInfo->EffectBasePoints[0] < 0)
		unitTarget->tmpAffect->SetNegative();

	uint32 type = 0;
                               												
	if(m_spellInfo->EffectBasePoints[i] < 0)
		type = 1;
                                    
	
	if(m_spellInfo->EffectApplyAuraName[i] == 3)
	{
		unitTarget->tmpAffect->SetDamagePerTick(damage,m_spellInfo->EffectAmplitude[i]);
		unitTarget->tmpAffect->SetNegative();
           
	}
	
	else if(m_spellInfo->EffectApplyAuraName[i] == 23)
		unitTarget->tmpAffect->SetPeriodicTriggerSpell(m_spellInfo->EffectTriggerSpell[i],m_spellInfo->EffectAmplitude[i]);
	
	else if(m_spellInfo->EffectApplyAuraName[i] == 8)
		unitTarget->tmpAffect->SetHealPerTick(damage,m_spellInfo->EffectAmplitude[i]);
	else
		unitTarget->tmpAffect->AddMod(m_spellInfo->EffectApplyAuraName[i],damage,m_spellInfo->EffectMiscValue[i],type);
}

void Spell::EffectPowerDrain(uint32 i)
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

void Spell::EffectWeaponDmgNS(uint32 i)
{
	if(!unitTarget)
		return;
	if(!unitTarget->isAlive())
		return;

	uint32 minDmg,maxDmg;
	minDmg = maxDmg = 0;
	if(m_spellInfo->rangeIndex == 1 || m_spellInfo->rangeIndex == 2 || m_spellInfo->rangeIndex == 7)
	{
		minDmg = m_caster->GetUInt32Value(UNIT_FIELD_MINDAMAGE);
		maxDmg = m_caster->GetUInt32Value(UNIT_FIELD_MAXDAMAGE);
	}
	else
	{
		minDmg = m_caster->GetUInt32Value(UNIT_FIELD_MINRANGEDDAMAGE);
		maxDmg = m_caster->GetUInt32Value(UNIT_FIELD_MAXRANGEDDAMAGE);
	}
	uint32 randDmg = maxDmg-minDmg;
	uint32 dmg = minDmg;
	if(randDmg > 1)
		dmg += rand()%randDmg;
	dmg += damage;
	m_caster->AttackerStateUpdate(unitTarget,dmg);
}

void Spell::EffectCreateItem(uint32 i) 
{
	Player* pUnit = (Player*)m_caster;
	uint8 slot = 0;
	for(uint8 i=INVENTORY_SLOT_ITEM_START;i<INVENTORY_SLOT_ITEM_END;i++)
	{
		if (pUnit->GetItemBySlot(i) != 0)
		{
			if (pUnit->GetItemBySlot(i)->GetItemProto()->Class == ITEM_CLASS_CONSUMABLE
				&& pUnit->GetItemBySlot(i)->GetItemProto()->ItemId == m_spellInfo->EffectItemType[i]
				&& pUnit->GetItemBySlot(i)->GetCount() < pUnit->GetItemBySlot(i)->GetItemProto()->MaxCount-1) 
			{
				slot = i;
				break;
			}  
		}
	}

	if (slot == 0)
	{
		for(uint8 i=INVENTORY_SLOT_ITEM_START;i<INVENTORY_SLOT_ITEM_END;i++)
		{
			if(pUnit->GetItemBySlot(i) == 0)
			{
				slot = i;
				break;
			}
		}
	}

	if(slot == 0)
	{
		SendCastResult(0x18);
		return;
	}

	Item* pItem;
	uint8 curSlot;
	for(uint32 i=0;i<8;i++)
	{
		for(uint32 j=0;j<m_spellInfo->ReagentCount[i];j++)
		{
			if(j>10)
				break;
			if(m_spellInfo->Reagent[i] == 0)
				continue;
			curSlot = (uint8)pUnit->GetSlotByItemID(m_spellInfo->Reagent[i]);
			if(curSlot == 0)
				continue;
			pItem = new Item;
			pItem = pUnit->GetItemBySlot(curSlot);
				
			
			if(pItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT) > 1) 
				pItem->SetUInt32Value(ITEM_FIELD_STACK_COUNT,pItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT)-1);
			else
			{
				pUnit->RemoveItemFromSlot(curSlot);
				pItem->DeleteFromDB();
			}
			pItem = NULL;
			curSlot = 0;
		}
	}

	pItem = NULL;
	Item* newItem;

	for(i=0;i<2;i++)
	{
		if(m_spellInfo->EffectItemType[i] == 0)
			continue;

		slot = 0;

		uint32 num_to_add = ((pUnit->GetLevel() - (m_spellInfo->spellLevel-1))*2);

		
		for (uint8 i = INVENTORY_SLOT_ITEM_START; i<INVENTORY_SLOT_ITEM_END;i++)
		{
			if (pUnit->GetItemBySlot(i) != 0)
			{
				if (pUnit->GetItemBySlot(i)->GetCount() <= 0)
					pUnit->GetItemBySlot(i)->SetCount(1); 

				if (pUnit->GetItemBySlot(i)->GetItemProto()->Class != ITEM_CLASS_CONSUMABLE
					&& pUnit->GetItemBySlot(i)->GetItemProto()->ItemId == m_spellInfo->EffectItemType[i])
				{
					num_to_add = 1;
				}
				if (pUnit->GetItemBySlot(i)->GetItemProto()->MaxCount > 1
					&& pUnit->GetItemBySlot(i)->GetItemProto()->ItemId == m_spellInfo->EffectItemType[i]
					&& pUnit->GetItemBySlot(i)->GetCount() < pUnit->GetItemBySlot(i)->GetItemProto()->MaxCount-num_to_add) 
				{
				
					pUnit->GetItemBySlot(i)->SetCount(pUnit->GetItemBySlot(i)->GetCount()+num_to_add);
					pUnit->UpdateSlot(i);
					pUnit->GetItemBySlot(i)->UpdateStats();
					slot = i; 
					continue;
				}
			}
		}

		if (slot != 0)
			return; 

		
		for (uint8 i = INVENTORY_SLOT_ITEM_START; i<INVENTORY_SLOT_ITEM_END;i++)
			if(pUnit->GetItemBySlot(i) == 0)
				slot = i;
		
		if(slot == 0)
		{
			SendCastResult(0x18);
			return;
		}
		newItem = new Item;
		newItem->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM),m_spellInfo->EffectItemType[i],pUnit);
		pUnit->AddItemToSlot(slot,newItem);

		num_to_add = 1;

		if (newItem->GetItemProto()->Class == ITEM_CLASS_CONSUMABLE && newItem->GetItemProto()->MaxCount > 1)
		{
			num_to_add = ((pUnit->GetLevel() - (m_spellInfo->spellLevel-1))*2);
		}

		if (num_to_add > 1)
		{
			pUnit->GetItemBySlot(slot)->SetCount((pUnit->GetItemBySlot(slot)->GetCount()+num_to_add)-1);
			pUnit->UpdateSlot(slot);
			pUnit->GetItemBySlot(slot)->UpdateStats();
		}
		newItem = NULL;
	}
}

void Spell::EffectPresistentAA(uint32 i)
{

	if(m_AreaAura == true)
		return;

	m_AreaAura = true;
	
	DynamicObject* dynObj = new DynamicObject();
	dynObj->Create(objmgr.GenerateLowGuid(HIGHGUID_DYNAMICOBJECT), m_caster, m_spellInfo, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, GetDuration(sSpellDuration.LookupEntry(m_spellInfo->DurationIndex)));
	dynObj->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
	dynObj->SetUInt32Value(GAMEOBJECT_DISPLAYID, 368003);
	dynObj->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x01eeeeee);
	dynObj->PeriodicTriggerDamage(damage, m_spellInfo->EffectAmplitude[i], GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i])));
	m_currdynObjID = dynObj->GetGUID();
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

void Spell::EffectWeaponDmgPerc(uint32 i)
{
	if(!unitTarget) return;
	if(!unitTarget->isAlive()) return;
		                        
	
	
}

void Spell::EffectOpenLock(uint32 i)
{
	WorldPacket data;
	playerTarget = ((Player*)m_caster);

	if(!gameObjTarget || !playerTarget)
	{
		if(!gameObjTarget)
			Log::getSingleton( ).outDebug( "WORLD: Open Lock - No GameObject Target!"); 
		if(!playerTarget)
			Log::getSingleton( ).outDebug( "WORLD: Open Lock - No Player Target!"); 
		return;
	}

	data.Initialize(SMSG_LOOT_RESPONSE);
	gameObjTarget->FillLoot(*playerTarget, &data);
	playerTarget->SetLootGUID(m_targets.m_unitTarget);
	playerTarget->GetSession()->SendPacket(&data);

}

void Spell::EffectOpenSecretSafe(uint32 i) 
{
	WorldPacket data;
	playerTarget = ((Player*)m_caster);

	if(!gameObjTarget || !playerTarget)
	{
		if(!gameObjTarget)
			Log::getSingleton( ).outDebug( "WORLD: Open Lock - No GameObject Target!"); 
		if(!playerTarget)
			Log::getSingleton( ).outDebug( "WORLD: Open Lock - No Player Target!"); 
		return;
	}


	data.Initialize(SMSG_LOOT_RESPONSE);
	gameObjTarget->FillLoot(*playerTarget, &data);
	playerTarget->SetLootGUID(m_targets.m_unitTarget);
	playerTarget->GetSession()->SendPacket(&data);

}

void Spell::EffectApplyAA(uint32 i)
{
	if(!unitTarget)
		return;
	if(!unitTarget->isAlive())
		return;

	Affect* aff = new Affect(m_spellInfo,6000,m_caster->GetGUID());
	aff->AddMod(m_spellInfo->EffectApplyAuraName[i],m_spellInfo->EffectBasePoints[i]+rand()%m_spellInfo->EffectDieSides[i]+1,m_spellInfo->EffectMiscValue[i],0);

	unitTarget->SetAura(aff);

}

void Spell::EffectLearnSpell(uint32 i)
{
	WorldPacket data;
	if(!playerTarget)
		return;
	uint32 spellToLearn = m_spellInfo->EffectTriggerSpell[i];
	playerTarget->addSpell((uint16)spellToLearn);
	data.Initialize(SMSG_LEARNED_SPELL);
	data << spellToLearn;
	playerTarget->GetSession()->SendPacket(&data);

}

void Spell::EffectSummonWild(uint32 i)
{
	if(!playerTarget)
		return;

	CreatureInfo *ci;
	ci = objmgr.GetCreatureName(m_spellInfo->EffectMiscValue[i]);
	if(!ci)
	{
		printf("unknown entry ID. return\n");
		return;
	}
 
	uint32 level = m_caster->getLevel();
	Creature* spawnCreature = new Creature();
	spawnCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT),ci->Name.c_str(),m_caster->GetMapId(),m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ(),m_caster->GetOrientation(), ci->DisplayID);
	spawnCreature->SetUInt32Value(UNIT_FIELD_DISPLAYID, ci->DisplayID);
	spawnCreature->SetUInt32Value(UNIT_NPC_FLAGS , 0);
	spawnCreature->SetUInt32Value(UNIT_FIELD_HEALTH, 28 + 30*level);
	spawnCreature->SetUInt32Value(UNIT_FIELD_MAXHEALTH, 28 + 30*level);
	spawnCreature->SetUInt32Value(UNIT_FIELD_LEVEL , level);
	spawnCreature->SetUInt32Value(OBJECT_FIELD_TYPE,ci->Type);
	spawnCreature->AIM_Initialize();
	Log::getSingleton( ).outError("AddObject at Spell.cpp");
	MapManager::Instance().GetMap(spawnCreature->GetMapId())->Add(spawnCreature);

}

void Spell::EffectEnchantItemPerm(uint32 i) 
{

	Player* p_caster = (Player*)m_caster;
	uint32 add_slot = 0;
	uint8 item_slot = 0;

	uint32 field = 99;
	if(m_CastItem)
		field = 1;
	else
		field = 3;
                                
	if(!m_CastItem)
	{
		for(uint8 i=0;i<INVENTORY_SLOT_ITEM_END;i++)
			if(p_caster->GetItemBySlot(i) != 0 && p_caster->GetItemBySlot(i)->GetProto()->ItemId == m_targets.m_itemTarget)
			{
				m_CastItem = p_caster->GetItemBySlot(i);
				item_slot = i;
			}
	}

	for(add_slot = 0; add_slot < 22; add_slot++)
		if (!m_CastItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot))
			break;

	if (add_slot < 32)
	{
		for(uint8 i=0;i<3;i++)
			if (m_spellInfo->EffectMiscValue[i])
				m_CastItem->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+(add_slot+i), m_spellInfo->EffectMiscValue[i]);

		
		UpdateData upd;
		WorldPacket packet;

		p_caster->ApplyItemMods( m_CastItem, item_slot, true );
		upd.Clear();
		m_CastItem->UpdateStats();
		m_CastItem->BuildCreateUpdateBlockForPlayer(&upd, (Player *)p_caster);
		upd.BuildPacket(&packet);
		p_caster->GetSession()->SendPacket(&packet);
	}


}

void Spell::EffectEnchantItemTmp(uint32 i)
{
	Player* p_caster = (Player*)m_caster;
	uint32 add_slot = 0;
	uint8 item_slot = 0;

	uint32 field = 99;
	if(m_CastItem)
		field = 1;
	else
		field = 3;
                                
	if(!m_CastItem)
	{
		for(uint8 i=0;i<INVENTORY_SLOT_ITEM_END;i++)
			if(p_caster->GetItemBySlot(i) != 0 && p_caster->GetItemBySlot(i)->GetProto()->ItemId == m_targets.m_itemTarget)
			{
				m_CastItem = p_caster->GetItemBySlot(i);
				item_slot = i;
			}
	}

	for(add_slot = 0; add_slot < 22; add_slot++)
		if (!m_CastItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot))
				break;

	if (add_slot < 32)
	{
		for(uint8 i=0;i<3;i++)
			if (m_spellInfo->EffectMiscValue[i])
				m_CastItem->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+(add_slot+i), m_spellInfo->EffectMiscValue[i]);

		
		UpdateData upd;
		WorldPacket packet;

		p_caster->ApplyItemMods( m_CastItem, item_slot, true );
		upd.Clear();
		m_CastItem->UpdateStats();
		m_CastItem->BuildCreateUpdateBlockForPlayer(&upd, (Player *)p_caster);
		upd.BuildPacket(&packet);
		p_caster->GetSession()->SendPacket(&packet);
	}




}

void Spell::EffectSummonPet(uint32 i)
{
	WorldPacket data;

	if(m_caster->GetUInt64Value(UNIT_FIELD_SUMMON) != 0)
	{
		Creature *OldSummon;
		OldSummon = ObjectAccessor::Instance().GetCreature(*m_caster, m_caster->GetUInt64Value(UNIT_FIELD_SUMMON));
		if(OldSummon)
		{
			m_caster->SetUInt64Value(UNIT_FIELD_SUMMON, 0);
			data.Initialize(SMSG_DESTROY_OBJECT);
			data << OldSummon->GetGUID();
			OldSummon->SendMessageToSet (&data, true);
			MapManager::Instance().GetMap(OldSummon->GetMapId())->Remove(OldSummon,true);
		}
	}
	Creature* NewSummon = new Creature();
	CreatureInfo *SummonInfo = objmgr.GetCreatureName(m_spellInfo->EffectMiscValue[i]);
	if( NewSummon && SummonInfo)
	{
		NewSummon->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), SummonInfo->Name.c_str(), m_caster->GetMapId(), 
		m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(),
		objmgr.AddCreatureName(SummonInfo->Name.c_str(), SummonInfo->DisplayID));

		NewSummon->SetUInt32Value(UNIT_FIELD_LEVEL,m_caster->GetUInt32Value(UNIT_FIELD_LEVEL));
		NewSummon->SetUInt32Value(UNIT_FIELD_DISPLAYID,  SummonInfo->DisplayID);
		NewSummon->SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, SummonInfo->DisplayID);
		NewSummon->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, m_caster->GetGUID());
		NewSummon->SetUInt32Value(UNIT_NPC_FLAGS , 0);
		NewSummon->SetUInt32Value(UNIT_FIELD_HEALTH , 28 + 30 * m_caster->GetUInt32Value(UNIT_FIELD_LEVEL));
		NewSummon->SetUInt32Value(UNIT_FIELD_MAXHEALTH , 28 + 30 * m_caster->GetUInt32Value(UNIT_FIELD_LEVEL));
		NewSummon->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
		NewSummon->SetFloatValue(OBJECT_FIELD_SCALE_X,m_caster->GetFloatValue(OBJECT_FIELD_SCALE_X));
		
		NewSummon->SetUInt32Value(UNIT_FIELD_BYTES_0,2048); 
		NewSummon->SetUInt32Value(UNIT_FIELD_FLAGS,0);
		NewSummon->SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, SummonInfo->unknown1); 
		NewSummon->SetUInt32Value(UNIT_FIELD_BASEATTACKTIME+1, SummonInfo->unknown2); 
		NewSummon->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, SummonInfo->bounding_radius);  
		NewSummon->SetFloatValue(UNIT_FIELD_COMBATREACH,m_caster->GetFloatValue(UNIT_FIELD_COMBATREACH));
		
		
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
		NewSummon->SetUInt32Value(UNIT_FIELD_RESISTANCES+0,0); 
		NewSummon->SetUInt32Value(UNIT_FIELD_RESISTANCES+1,0); 
		NewSummon->SetUInt32Value(UNIT_FIELD_RESISTANCES+2,0); 
		NewSummon->SetUInt32Value(UNIT_FIELD_RESISTANCES+3,0); 
		NewSummon->SetUInt32Value(UNIT_FIELD_RESISTANCES+4,0); 
		NewSummon->SetUInt32Value(UNIT_FIELD_RESISTANCES+5,0); 
		NewSummon->SetUInt32Value(UNIT_FIELD_RESISTANCES+6,0); 
		NewSummon->SetUInt32Value(UNIT_FIELD_ATTACKPOWER,24);
		NewSummon->SetUInt32Value(UNIT_FIELD_BASE_MANA, SummonInfo->maxmana); 
		NewSummon->SetUInt32Value(OBJECT_FIELD_ENTRY, m_spellInfo->EffectMiscValue[i]);
		NewSummon->SetZoneId(m_caster->GetZoneId());

		NewSummon->SaveToDB();
		NewSummon->AIM_Initialize(); 
		MapManager::Instance().GetMap(NewSummon->GetMapId())->Add(NewSummon);

		m_caster->SetUInt64Value(UNIT_FIELD_SUMMON, NewSummon->GetGUID());
		Log::getSingleton().outDebug("New Pet has guid %u", NewSummon->GetGUID());
                                
		if(m_caster->isPlayer())
		{
			data.clear();
			data.Initialize(SMSG_PET_SPELLS);
			data << (uint64)NewSummon->GetGUID() << uint32(0x00000101) << uint32(0x00000000) << uint32(0x07000001) << uint32(0x07000002);
			data << uint32(0x02000000) << uint32(0x07000000) << uint32(0x04000000) << uint32(0x03000000) << uint32(0x06000002) << uint32(0x05000000);
			data << uint32(0x06000000) << uint32(0x06000001) << uint8(0x02) << uint32(0x0c26) << uint32(0x18a3);
			((Player*)m_caster)->GetSession()->SendPacket(&data);
		}
	}

}

void Spell::EffectWeaponDmg(uint32 i)
{
	if(!unitTarget)
		return;
	if(!unitTarget->isAlive())
		return;

	uint32 minDmg,maxDmg;
	minDmg = maxDmg = 0;
	if(m_spellInfo->rangeIndex == 1 || m_spellInfo->rangeIndex == 2 || m_spellInfo->rangeIndex == 7)
	{
		minDmg = m_caster->GetUInt32Value(UNIT_FIELD_MINDAMAGE);
		maxDmg = m_caster->GetUInt32Value(UNIT_FIELD_MAXDAMAGE);
	}
	else
	{
		minDmg = m_caster->GetUInt32Value(UNIT_FIELD_MINRANGEDDAMAGE);
		maxDmg = m_caster->GetUInt32Value(UNIT_FIELD_MAXRANGEDDAMAGE);
	}
	uint32 randDmg = maxDmg-minDmg;
	uint32 dmg = minDmg;
	if(randDmg > 1)
		dmg += rand()%randDmg;
	dmg += damage;
	m_caster->AttackerStateUpdate(unitTarget,dmg);

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

void Spell::EffectAddComboPoints(uint32 i)
{
	if(!unitTarget)
		return;
	uint8 comboPoints = ((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & 0xFF00) >> 8);
	if(m_caster->GetUInt64Value(PLAYER__FIELD_COMBO_TARGET) != unitTarget->GetGUID())
	{
		m_caster->SetUInt64Value(PLAYER__FIELD_COMBO_TARGET,unitTarget->GetGUID());
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
	GameObject* pGameObj = new GameObject();

	uint32 gameobject_id = m_spellInfo->EffectMiscValue[i];

	
	pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), gameobject_id,playerCaster->GetMapId(), 
	playerCaster->GetPositionX()+(playerTarget->GetPositionX()-playerCaster->GetPositionX())/2 , 
	playerCaster->GetPositionY()+(playerTarget->GetPositionY()-playerCaster->GetPositionY())/2 , 
	playerCaster->GetPositionZ(), 
	playerCaster->GetOrientation(), 0, 0, 0, 0);
	pGameObj->SetUInt32Value(OBJECT_FIELD_ENTRY, m_spellInfo->EffectMiscValue[i] );
	pGameObj->SetUInt32Value(GAMEOBJECT_DISPLAYID, 787 );
	pGameObj->SetUInt32Value(GAMEOBJECT_FACTION, 4 );
	pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, 16 );
	pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, 57 );

	Log::getSingleton( ).outError("AddObject at Spell.cpp 1247");
	MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);

	SendDuelRequest(playerCaster, playerTarget , pGameObj->GetGUID());

}

void Spell::SendDuelRequest(Player* caster, Player* target,uint64 ArbiterID)
{
   WorldPacket data;
   static uint64 aGUID = 0x213632;
   aGUID++;
   data.Initialize(SMSG_DUEL_REQUESTED);
   data << ArbiterID << caster->GetGUID();

   target->GetSession()->SendPacket(&data);
   caster->GetSession()->SendPacket(&data);

   caster->SetDuelVsGUID(target->GetGUID());
   target->SetDuelVsGUID(caster->GetGUID());

   caster->SetInDuel(false);
   target->SetInDuel(false);

   caster->SetDuelSenderGUID(caster->GetGUID());
   target->SetDuelSenderGUID(caster->GetGUID());
                        
   caster->SetDuelFlagGUID(ArbiterID);
   target->SetDuelFlagGUID(ArbiterID);
}

void Spell::EffectSummonTotem(uint32 i)
{
	WorldPacket data;
	uint64 guid = 0;
	
	switch(m_spellInfo->Effect[i])
	{
		case EFF_SUMMON_TOTEM_SLOT1:
		{
			guid = m_caster->m_TotemSlot1;
			m_caster->m_TotemSlot1 = 0;
		}break;
		case EFF_SUMMON_TOTEM_SLOT2:
		{
			guid = m_caster->m_TotemSlot2;
			m_caster->m_TotemSlot2 = 0;
		}break;
        case EFF_SUMMON_TOTEM_SLOT3:
		{
			guid = m_caster->m_TotemSlot3;
			m_caster->m_TotemSlot3 = 0;
		}break;
		case EFF_SUMMON_TOTEM_SLOT4:
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
	CreatureInfo* ci = objmgr.GetCreatureName(m_spellInfo->EffectMiscValue[i]);
	if(!ci)
	{
		printf("break: unknown CreatureEntry\n");
		return;
	}
	char* name = (char*)ci->Name.c_str();

	
	pTotem->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), name, m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), ci->DisplayID );
	pTotem->SetUInt32Value(OBJECT_FIELD_TYPE,33);
	pTotem->SetUInt32Value(UNIT_FIELD_DISPLAYID,ci->DisplayID);
	pTotem->SetUInt32Value(UNIT_FIELD_LEVEL,m_caster->getLevel());
	Log::getSingleton( ).outError("AddObject at Spell.cppl line 1040");
	MapManager::Instance().GetMap(pTotem->GetMapId())->Add(pTotem);


	data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
	data << pTotem->GetGUID();
	m_caster->SendMessageToSet(&data,true);

	switch(m_spellInfo->Effect[i])
	{
		case EFF_SUMMON_TOTEM_SLOT1:
		{
			m_caster->m_TotemSlot1 = pTotem->GetGUID();
		}break;
		case EFF_SUMMON_TOTEM_SLOT2:
		{
			m_caster->m_TotemSlot2 = pTotem->GetGUID();
		}break;
        case EFF_SUMMON_TOTEM_SLOT3:
		{
			m_caster->m_TotemSlot3 = pTotem->GetGUID();
		}break;
		case EFF_SUMMON_TOTEM_SLOT4:
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
	if(m_CastItem)
		field = 1;
	else
		field = 3;
                                
	if(!m_CastItem)
	{
		for(uint8 i=0;i<INVENTORY_SLOT_ITEM_END;i++)
			if(p_caster->GetItemBySlot(i) != 0 && p_caster->GetItemBySlot(i)->GetProto()->ItemId == m_targets.m_itemTarget)
			{
				m_CastItem = p_caster->GetItemBySlot(i);
				item_slot = i;
			}
	}

	for(add_slot = 0; add_slot < 22; add_slot++)
	{
		if (!m_CastItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+add_slot))
			break;
	}

	if (add_slot < 32)
	{
		for(uint8 i=0;i<3;i++)
			if (m_spellInfo->EffectMiscValue[i])
				m_CastItem->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+(add_slot+i), m_spellInfo->EffectMiscValue[i]);

		
		UpdateData upd;
		WorldPacket packet;

		p_caster->ApplyItemMods( m_CastItem, item_slot, true );
		upd.Clear();
		m_CastItem->UpdateStats();
		m_CastItem->BuildCreateUpdateBlockForPlayer(&upd, (Player *)p_caster);
		upd.BuildPacket(&packet);
		p_caster->GetSession()->SendPacket(&packet);
	}

}

void Spell::EffectSummonObject(uint32 i)
{
	WorldPacket data;
	uint64 guid = 0;
	
	switch(m_spellInfo->Effect[i])
	{
		case EFF_SUMMON_OBJECT_SLOT1:
		{
			guid = m_caster->m_TotemSlot1;
			m_caster->m_TotemSlot1 = 0;
		}break;
		case EFF_SUMMON_OBJECT_SLOT2:
		{
			guid = m_caster->m_TotemSlot2;
            m_caster->m_TotemSlot2 = 0;
		}break;
		case EFF_SUMMON_OBJECT_SLOT3:
		{
			guid = m_caster->m_TotemSlot3;
            m_caster->m_TotemSlot3 = 0;
		}break;
		case EFF_SUMMON_OBJECT_SLOT4:
		{
			guid = m_caster->m_TotemSlot4;
            m_caster->m_TotemSlot4 = 0;
		}break;
	}
	if(guid != 0)
	{
		GameObject* obj = NULL;
		if( playerCaster )
			obj = ObjectAccessor::Instance().GetGameObject(*playerCaster, guid);

		if(obj)
		{
			
			MapManager::Instance().GetMap(obj->GetMapId())->Remove(obj, true);
			obj = NULL;
		}
	}

	
	GameObject* pGameObj = new GameObject();
	uint16 display_id = m_spellInfo->EffectMiscValue[i];

	
	pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), display_id,m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), 0, 0, 0, 0);
	pGameObj->SetUInt32Value(OBJECT_FIELD_ENTRY, m_spellInfo->EffectMiscValue[i]);
	pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, 6);
	pGameObj->SetUInt32Value(OBJECT_FIELD_TYPE,33);
	pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL,m_caster->getLevel());
	Log::getSingleton( ).outError("AddObject at Spell.cpp 1100");

	MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);
	data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
	data << pGameObj->GetGUID();
	m_caster->SendMessageToSet(&data,true);

	switch(m_spellInfo->Effect[i])
	{
		case EFF_SUMMON_OBJECT_SLOT1:
		{
			m_caster->m_TotemSlot1 = pGameObj->GetGUID();
		}break;
		case EFF_SUMMON_OBJECT_SLOT2:
		{
			m_caster->m_TotemSlot2 = pGameObj->GetGUID();
		}break;
		case EFF_SUMMON_OBJECT_SLOT3:
		{
			m_caster->m_TotemSlot3 = pGameObj->GetGUID();
		}break;
		case EFF_SUMMON_OBJECT_SLOT4:
		{
			m_caster->m_TotemSlot4 = pGameObj->GetGUID();
		}break;
	}

}

void Spell::EffectResurrect(uint32 i)
{
	if(!playerTarget)
		return;
	if(playerTarget->isAlive())
		return;
	if(!playerTarget->IsInWorld())
		return;

	uint32 health = m_spellInfo->EffectBasePoints[i];
	uint32 mana = m_spellInfo->EffectMiscValue[i];
	playerTarget->setResurrect(m_caster->GetGUID(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
	SendResurrectRequest(playerTarget);


}









