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

#define SPELL_CHANNEL_UPDATE_INTERVAL 1000

void SpellCastTargets::read ( WorldPacket * data,uint64 caster )
{
    m_unitTarget = m_itemTarget = 0;m_srcX = m_srcY = m_srcZ = m_destX = m_destY = m_destZ = 0;
    m_strTarget = "";

    *data >> m_targetMask;

    if(m_targetMask & TARGET_FLAG_SELF)
        m_unitTarget = caster;

    if(m_targetMask & TARGET_FLAG_UNIT)
        *data >> m_unitTarget;

    if(m_targetMask & TARGET_FLAG_OBJECT)
        *data >> m_unitTarget;

    if(m_targetMask & TARGET_FLAG_ITEM)
        *data >> m_itemTarget;

    if(m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
        *data >> m_srcX >> m_srcY >> m_srcZ;

    if(m_targetMask & TARGET_FLAG_DEST_LOCATION)
        *data >> m_destX >> m_destY >> m_destZ;

    if(m_targetMask & TARGET_FLAG_STRING)
        *data >> m_strTarget;
}


void SpellCastTargets::write ( WorldPacket * data)
{
    *data << m_targetMask;

    if(m_targetMask & TARGET_FLAG_SELF)
        *data << m_unitTarget;

    if(m_targetMask & TARGET_FLAG_UNIT)
        *data << m_unitTarget;

    if(m_targetMask & TARGET_FLAG_OBJECT)
        *data << m_unitTarget;

    if(m_targetMask & TARGET_FLAG_ITEM)
        *data << m_itemTarget;

    if(m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
        *data << m_srcX << m_srcY << m_srcZ;

    if(m_targetMask & TARGET_FLAG_DEST_LOCATION)
        *data << m_destX << m_destY << m_destZ;

    if(m_targetMask & TARGET_FLAG_STRING)
        *data << m_strTarget;
}


Spell::Spell( Unit* Caster, SpellEntry *info, bool triggered, Affect* aff )
{
    ASSERT( Caster != NULL && info != NULL );

    int32 temptime;
    SpellEntry *spellInfo;
    Player* p_caster;
    std::list<struct spells>::iterator itr;

    std::list<struct spells> player_spells;

    m_spellInfo = info;
    m_caster = Caster;

    m_spellState = SPELL_STATE_NULL;

    m_castPositionX = m_castPositionY = m_castPositionZ;
    TriggerSpellId = 0;
    m_targetCount = 0;
    m_triggeredSpell = triggered;
    m_AreaAura = false;
	m_currdynObjID = 0;

    m_triggeredByAffect = aff;

    temptime = GetCastTime(sCastTime.LookupEntry(m_spellInfo->CastingTimeIndex)); 
    
    if( Caster->isPlayer() && m_spellInfo )
    {
        p_caster = (Player*)m_caster;
        player_spells = p_caster->getSpellList();
        for (itr = player_spells.begin(); itr != player_spells.end(); ++itr)
        {
            if (itr->spellId != m_spellInfo->Id)
            {    
                 spellInfo = sSpellStore.LookupEntry(itr->spellId);
                 if(spellInfo && spellInfo->SpellIconID == m_spellInfo->SpellIconID && spellInfo->EffectMiscValue[0] ==10)
                 {
                     temptime=temptime+(spellInfo->EffectBasePoints[0]+1);
                 }
            }
        }    
    }
    
    m_timer = temptime<0?0:temptime;

}


void Spell::FillTargetMap()
{
    Player* p_caster = (Player*)m_caster;
    std::list<uint64> tmpMap;
    uint32 cur = 0;
    for(uint32 i=0;i<3;i++)
    { 
       
       
       SetTargetMap(i,m_spellInfo->EffectImplicitTargetA[i],p_caster,tmpMap);
       SetTargetMap(i,m_spellInfo->EffectImplicitTargetB[i],p_caster,tmpMap);
       
	   if(i == 0)
         m_targetUnits1 = tmpMap;
       else if(i == 1)
         m_targetUnits2 = tmpMap;
       else if(i == 2)
         m_targetUnits3 = tmpMap;
       tmpMap.clear();
    }
}



void Spell::SetTargetMap(uint32 i,uint32 cur,Player* p_caster,std::list<uint64> &TagMap)
{
  
	
  switch(cur)
  {
	 case TARGET_SELF:  
     {
     	TagMap.push_back(m_caster->GetGUID());
     }break;
     case TARGET_PET:   
     {
        TagMap.push_back(m_caster->GetUInt32Value(UNIT_FIELD_PETNUMBER));
     }break;
     case TARGET_S_E:   
     {
       TagMap.push_back(m_targets.m_unitTarget);
     }break;
     case TARGET_AE_E:  
     {
  	 }break;
     case TARGET_AE_E_INSTANT:           
     {
	     	CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
	      Cell cell = RedZone::GetZone(p);
	      cell.data.Part.reserved = ALL_DISTRICT;
	      cell.SetNoCreate();
	      
	      
	      
	      
	      MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagMap, i,PUSH_DEST_CENTER);
	      TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
	      CellLock<GridReadGuard> cell_lock(cell, p);
	      cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
     }break;
     case TARGET_AC_P:  
     {
	      Group* pGroup = objmgr.GetGroupByLeader(p_caster->GetGroupLeader());
	      if(pGroup)
	        for(uint32 p=0;p<pGroup->GetMembersCount();p++)
	        {
	           Unit* Target = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(p));
	           if(!Target || Target->GetGUID() == m_caster->GetGUID())
	               continue;
	           if(_CalcDistance(m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ(),Target->GetPositionX(),Target->GetPositionY(),Target->GetPositionZ()) < GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i])))
	               TagMap.push_back(Target->GetGUID());
	        }
	      else
	        TagMap.push_back(m_caster->GetGUID());
     }break;
     case TARGET_S_F:  
     {
        TagMap.push_back(m_targets.m_unitTarget);
     }break;
     case TARGET_AC_E: 
     {
        CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
        Cell cell = RedZone::GetZone(p);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();
        
        
        
        
        MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagMap, i,PUSH_SELF_CENTER);
        TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
        CellLock<GridReadGuard> cell_lock(cell, p);
        cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));

     }break;
     case TARGET_S_GO: 
     {
        TagMap.push_back(m_targets.m_unitTarget);
     }break;
     case TARGET_INFRONT: 
     {
        CellPair p(MaNGOS::ComputeCellPair(p_caster->GetPositionX(), p_caster->GetPositionY()));
        Cell cell = RedZone::GetZone(p);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();
        MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagMap, i,PUSH_IN_FRONT);
        
        TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
        CellLock<GridReadGuard> cell_lock(cell, p);
        
        cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
     }break;
     case TARGET_DUELVSPLAYER: 
     {
         TagMap.push_back(m_targets.m_unitTarget);
     }break;
     case TARGET_GOITEM: 
     {
         if(m_targets.m_unitTarget)
            TagMap.push_back(m_targets.m_unitTarget);
         if(m_targets.m_itemTarget)
            TagMap.push_back(m_targets.m_itemTarget);
     }break;
     case TARGET_AE_E_CHANNEL: 
     {
         CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
         Cell cell = RedZone::GetZone(p);
         cell.data.Part.reserved = ALL_DISTRICT;
         cell.SetNoCreate();                
		 
         
         
         
         MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagMap, i,PUSH_DEST_CENTER);
         TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
         CellLock<GridReadGuard> cell_lock(cell, p);
         cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
     }break;
     case TARGET_MINION: 
     {
			if(m_caster->GetUInt64Value(UNIT_FIELD_SUMMON) == 0)
				TagMap.push_back(m_caster->GetGUID());
			else
				TagMap.push_back(m_caster->GetUInt64Value(UNIT_FIELD_SUMMON));
     }break;
     case TARGET_S_P: 
     {
         TagMap.push_back(m_targets.m_unitTarget);
     }break;
     case TARGET_CHAIN: 
     {
         bool onlyParty = false;
         Unit* firstTarget;
         firstTarget = ObjectAccessor::Instance().FindPlayer(m_targets.m_unitTarget);

         if(!firstTarget)
            firstTarget = ObjectAccessor::Instance().GetCreature(*p_caster, m_targets.m_unitTarget);

         if(!firstTarget)
            break;
         Group* pGroup = objmgr.GetGroupByLeader(p_caster->GetGroupLeader());
         for(uint32 p=0;p<pGroup->GetMembersCount();p++)
         {
            if(firstTarget->GetGUID() == pGroup->GetMemberGUID(p))
               onlyParty = true;
         }
         for(uint32 p=0;p<pGroup->GetMembersCount();p++)
         {
            Unit* Target = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(p));
    
            if(!Target || Target->GetGUID() == m_caster->GetGUID())
                continue;
            if(_CalcDistance(Target->GetPositionX(),Target->GetPositionY(),Target->GetPositionZ(),m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ()) < GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i])) && Target->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) == m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE))
                TagMap.push_back(Target->GetGUID());
         }
     }break;
     case TARGET_AE_SELECTED:          
     {
         CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
         Cell cell = RedZone::GetZone(p);
         cell.data.Part.reserved = ALL_DISTRICT;
         cell.SetNoCreate();
         MaNGOS::SpellNotifierPlayer notifier(*this, TagMap, i);
         TypeContainerVisitor<MaNGOS::SpellNotifierPlayer, ContainerMapList<Player> > player_notifier(notifier);
         CellLock<GridReadGuard> cell_lock(cell, p);
         cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
     }break;
     default:
     {
     }break;
  }             
  
  

  
}

void Spell::prepare(SpellCastTargets * targets)
{
     uint8 result;

     WorldPacket data;

     m_targets = *targets;

     SendSpellStart();

     
     m_spellState = SPELL_STATE_PREPARING;

     m_castPositionX = m_caster->GetPositionX();
     m_castPositionY = m_caster->GetPositionY();
     m_castPositionZ = m_caster->GetPositionZ();

     result = CanCast();
     if(result != 0)
     {
          if(m_triggeredByAffect)
          {
           SendChannelUpdate(0);
           m_triggeredByAffect->SetDuration(0);
          }
          finish();
     }

     if(m_triggeredSpell)
        cast();
     else
         m_caster->castSpell( this );
}

void Spell::cancel()
{
   WorldPacket data;

   if(m_spellState == SPELL_STATE_PREPARING)
   {
        SendInterrupted(0);
        SendCastResult(0x20); 
   }
   else if(m_spellState = SPELL_STATE_CASTING)
   {
        m_caster->RemoveAffectById(m_spellInfo->Id);
        SendChannelUpdate(0);
   }
    
   finish();
   m_spellState = SPELL_STATE_FINISHED;
}

void Spell::cast()
{
	WorldPacket data;

    bool Instant = true;
	
	
	
	
	

    if(Instant)
    {
      uint8 castResult = 0;
      castResult = CanCast();
      if(castResult == 0)
      {
        TakePower();
        RemoveItems();
        FillTargetMap();
        SendCastResult(castResult);
        SendSpellGo();
        SendLogExecute();

        if(m_spellInfo->ChannelInterruptFlags != 0)
        {
           m_spellState = SPELL_STATE_CASTING;
           SendChannelStart(GetDuration(sSpellDuration.LookupEntry(m_spellInfo->DurationIndex)));
        }

        for(uint32 j = 0;j<3;j++)
        {
           
           if(m_spellInfo->Effect[j] == 27)
              HandleEffects(m_caster->GetGUID(),j);
        }

        std::list<uint64>::iterator i;
        for(i= m_targetUnits1.begin();i != m_targetUnits1.end();i++)
          HandleEffects((*i),0);
        for(i= m_targetUnits2.begin();i != m_targetUnits2.end();i++)
          HandleEffects((*i),1);
        for(i= m_targetUnits3.begin();i != m_targetUnits3.end();i++)
          HandleEffects((*i),2);
        for(i= UniqueTargets.begin();i != UniqueTargets.end();i++)
          HandleAddAffect((*i));
      }

      if(m_spellState != SPELL_STATE_CASTING)
        finish();

      if(castResult == 0)
        TriggerSpell();
    }
    else
    {
      m_caster->m_meleeSpell = true;
                            m_spellState = SPELL_STATE_IDLE;
    }
}

void Spell::update(uint32 difftime)
{
                        
  if( (m_castPositionX != m_caster->GetPositionX()  ||
       m_castPositionY != m_caster->GetPositionY()  ||
       m_castPositionZ != m_caster->GetPositionZ() ) && 
       ( m_timer != 0 )) 
  {
    SendInterrupted(0);
    SendCastResult(0x20); 
    if(m_spellState == SPELL_STATE_CASTING)
    {
       m_caster->RemoveAffectById(m_spellInfo->Id);
       SendChannelUpdate(0);
    }
	finish();
    m_spellState = SPELL_STATE_FINISHED;
  }
  switch(m_spellState)
  {
     case SPELL_STATE_PREPARING:
     {
        if(m_timer)
        {
          if(difftime >= m_timer)
             m_timer = 0;
          else
             m_timer -= difftime;
        }

        if(m_timer == 0)
          cast();
     } break;
     case SPELL_STATE_CASTING:
     {
        if(m_timer > 0)
        {
           if(difftime >= m_timer)
              m_timer = 0;
           else
              m_timer -= difftime;
           m_intervalTimer += difftime;
										
        }

        if(m_timer == 0)
        {
           SendChannelUpdate(0);
           finish();
        }
     } break;
     default:
     { 
	 }break;
  }

}

void Spell::finish()
{
  WorldPacket data;

  m_spellState = SPELL_STATE_FINISHED;
  m_caster->m_meleeSpell = false;
  m_caster->m_canMove = true;
  if(m_currdynObjID!=0)
  {
	 Player *pl=(Player*)m_caster;
     DynamicObject* obj = NULL;
	 WorldPacket data;

     if( m_caster )
       obj = ObjectAccessor::Instance().GetDynamicObject(*pl, m_currdynObjID);
     if(obj)
	 {
        data.Initialize(SMSG_GAMEOBJECT_DESPAWN_ANIM);
        data << obj->GetGUID();
        pl->SendMessageToSet(&data, true);	

        data.Initialize(SMSG_DESTROY_OBJECT);
        data << obj->GetGUID();
		pl->SendMessageToSet(&data, true);	
		MapManager::Instance().GetMap(obj->GetMapId())->Remove(obj, true);
	 }
	 m_currdynObjID = 0;
  }
  
                                                  
  m_caster->setRegenTimer(5000);
}

void Spell::SendCastResult(uint8 result)
{
   if (m_caster->GetTypeId() != TYPEID_PLAYER)
       return;

   WorldPacket data;

   data.Initialize(SMSG_CAST_RESULT);
   data << m_spellInfo->Id;
   if(result != 0)
      data << uint8(2);
   data << result;

#ifdef _VERSION_1_7_0_
   data << uint32(0) << uint32(0) << uint32(0);
#endif                    

   ((Player*)m_caster)->GetSession()->SendPacket(&data);
}

void Spell::SendSpellStart()
{
   
   WorldPacket data;
   uint16 cast_flags;

   cast_flags = 2;

   data.Initialize(SMSG_SPELL_START);
   data << m_caster->GetGUID() << m_caster->GetGUID();
   data << m_spellInfo->Id;
   data << cast_flags;
   data << uint32(m_timer);
   
                        
   m_targets.write( &data );
   ((Player*)m_caster)->SendMessageToSet(&data, true);

}

void Spell::SendSpellGo()
{
   
   WorldPacket data;
   uint16 flags;

   flags = m_targets.m_targetMask;
   if(flags == 0)
     flags = 2;

   data.Initialize(SMSG_SPELL_GO);

   data << m_caster->GetGUID() << m_caster->GetGUID();
   data << m_spellInfo->Id;
   
   data << uint16(0x0500);
   writeSpellGoTargets(&data);

   data << (uint8)0;         

#if defined ( _VERSION_1_7_0_ ) || defined ( _VERSION_1_7_0_ )
   data << uint32(0) << uint32(0) << uint32(0);
#endif 

   m_targets.write( &data );
   m_caster->SendMessageToSet(&data, true);

}

void Spell::writeSpellGoTargets( WorldPacket * data )
{
   bool add = true;
   for ( std::list<uint64>::iterator i = m_targetUnits1.begin(); i != m_targetUnits1.end(); i++ )
   {
      for(std::list<uint64>::iterator j = UniqueTargets.begin(); j != UniqueTargets.end(); j++ )
      {
         if((*j) == (*i))
            add = false;
      }
      if(add)
        UniqueTargets.push_back((*i));
      add = true;
   }
   for ( std::list<uint64>::iterator i = m_targetUnits2.begin(); i != m_targetUnits2.end(); i++ )
   {
      for(std::list<uint64>::iterator j = UniqueTargets.begin(); j != UniqueTargets.end(); j++ )
      {
        if((*j) == (*i))
          add = false;
      }
      if(add)
        UniqueTargets.push_back((*i));
      add = true;
   }
   for ( std::list<uint64>::iterator i = m_targetUnits3.begin(); i != m_targetUnits3.end(); i++ )
   {
      for(std::list<uint64>::iterator j = UniqueTargets.begin(); j != UniqueTargets.end(); j++ )
      {
        if((*j) == (*i))
          add = false;
      }
      if(add)
        UniqueTargets.push_back((*i));
      add = true;
   }
   m_targetCount = UniqueTargets.size();

   *data << m_targetCount;
   for ( std::list<uint64>::iterator i = UniqueTargets.begin(); i != UniqueTargets.end(); i++ )
     *data << (*i);
}

void Spell::SendLogExecute()
{
  WorldPacket data;
  data.Initialize(SMSG_SPELLLOGEXECUTE);
  data << m_caster->GetGUID();
  data << m_spellInfo->Id;
  data<< uint32(0x00000001);
  data << uint32(0x0000071);
  data << uint32(000000001);
  data << m_targets.m_unitTarget;
  m_caster->SendMessageToSet(&data,true);
};

void Spell::SendInterrupted(uint8 result)
{
  WorldPacket data;

  data.Initialize(SMSG_SPELL_FAILURE);

  data << m_caster->GetGUID();
  data << m_spellInfo->Id;
  data << result;

  m_caster->SendMessageToSet(&data, true);
}

void Spell::SendChannelUpdate(uint32 time)
{
   if (m_caster->GetTypeId() != TYPEID_PLAYER)
      return;

   WorldPacket data;

   data.Initialize( MSG_CHANNEL_UPDATE );
   data << time;

#if defined ( _VERSION_1_7_0_ ) || defined ( _VERSION_1_7_0_ )
   data << uint32(0) << uint32(0) << uint32(0);
#endif 

   ((Player*)m_caster)->GetSession()->SendPacket( &data );

   if(time == 0)
   {
      m_caster->SetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT,0);
      m_caster->SetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT+1,0);
      m_caster->SetUInt32Value(UNIT_CHANNEL_SPELL,0);
   }
}

void Spell::SendChannelStart(uint32 duration)
{
   Unit* target = ObjectAccessor::Instance().GetCreature(*((Player *)m_caster), ((Player *)m_caster)->GetSelection());
   if( !target )
     target = ObjectAccessor::Instance().GetCreature(*((Player *)m_caster), ((Player *)m_caster)->GetSelection());
   if (m_caster->GetTypeId() == TYPEID_PLAYER)
   {
      
      WorldPacket data;
      data.Initialize( MSG_CHANNEL_START );
      data << m_spellInfo->Id;
      data << duration;

#if defined ( _VERSION_1_7_0_ ) || defined ( _VERSION_1_7_0_ )
      data << uint32(0) << uint32(0) << uint32(0);
#endif 

      ((Player*)m_caster)->GetSession()->SendPacket( &data );
   }

   m_timer = duration;
   if(target)
   {    
      m_caster->SetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT,target->GetGUIDLow());
      m_caster->SetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT+1,target->GetGUIDHigh());
   }
   m_caster->SetUInt32Value(UNIT_CHANNEL_SPELL,m_spellInfo->Id);
}

void Spell::SendResurrectRequest(Player* target)
{
   WorldPacket data;
   data.Initialize(SMSG_RESURRECT_REQUEST);
   data << m_caster->GetGUID();
   data << uint32(0) << uint8(0);

#if defined ( _VERSION_1_7_0_ ) || defined ( _VERSION_1_7_0_ )
   data << uint32(0) << uint32(0) << uint32(0);
#endif 

   target->GetSession()->SendPacket(&data);
   return;
}

void Spell::TakePower()
{
   uint16 powerField;
   uint32 currentPower;

   uint8 powerType = (uint8)(m_caster->GetUInt32Value(UNIT_FIELD_BYTES_0) >> 24);
   if(powerType == 0)        
      powerField = UNIT_FIELD_POWER1;
   else if(powerType == 1)   
      powerField = UNIT_FIELD_POWER2;
   else if(powerType == 3)   
      powerField = UNIT_FIELD_POWER4;

   currentPower = m_caster->GetUInt32Value(powerField);

   if(currentPower < m_spellInfo->manaCost)
      m_caster->SetUInt32Value(powerField, 0);
   else
      m_caster->SetUInt32Value(powerField, currentPower - m_spellInfo->manaCost);
}

void Spell::HandleEffects(uint64 guid,uint32 i)
{

	playerCaster = ObjectAccessor::Instance().FindPlayer(m_caster->GetGUID());
	if( playerCaster != NULL )
	{
		unitTarget = ObjectAccessor::Instance().GetUnit(*playerCaster, guid);
		gameObjTarget = ObjectAccessor::Instance().GetGameObject(*playerCaster, guid);
		playerTarget = ObjectAccessor::Instance().GetPlayer(*playerCaster, guid);
	}

	damage = CalculateDamage((uint8)i);

	Log::getSingleton( ).outDebug( "WORLD: Spell FX id is %u", m_spellInfo->Effect[i]); 

	switch(m_spellInfo->Effect[i])
	{
		case EFF_SCHOOL_DMG: 
		{
			EffectWeaponDmgNS(i);
		}break;
		case EFF_DUMMY: 
		{
		}break;
		case EFF_TELEPORT_UNITS: 
		{
			EffectTepeportUnits(i);
		}break;
		case EFF_APPLY_AURA: 
		{
			EffectApplyAura(i);
		}break;
		case EFF_POWER_DRAIN: 
		{
			EffectPowerDrain(i);
		}break;
		case EFF_HEAL: 
		{
			EffectHeal(i);
		}break;
		case EFF_WEAPON_DMG_NS: 
		{
			EffectWeaponDmg(i);
		}break;
		case EFF_CREATE_ITEM: 
		{
			EffectCreateItem(i);
		}break;
		case EFF_PRESISTENT_AA: 
		{
			EffectPresistentAA(i);
		}break;
		case EFF_ENERGIZE: 
		{
			EffectEnergize(i);                         
		}break;
		case EFF_WEAPON_DMG_PERC: 
		{
			EffectWeaponDmgPerc(i);
		}break;
		case EFF_OPEN_LOCK: 
		{
			EffectOpenLock(i);
		}break;
		case EFF_OPEN_SECRECTSAFE: 
		{
			EffectOpenSecretSafe(i);
		}break;
		case EFF_APPLY_AA: 
		{
			EffectApplyAA(i);
		}break;
		case EFF_LEARN_SPELL: 
		{
			EffectLearnSpell(i);
		}break;
		case EFF_SUMMON_WILD: 
		{
			EffectSummonWild(i);
		}break;
		case EFF_ENCHANT_ITEM_PERM: 
		{
			EffectEnchantItemPerm(i);
		}break;
		case EFF_ENCHANT_ITEM_TMP: 
		{
			EffectEnchantItemTmp(i);
		}break;
		case EFF_SUMMON_PET: 
		{
			EffectSummonPet(i);
		}break;
		case EFF_WEAPON_DMG: 
		{
			EffectWeaponDmg(i);
		}break;
		case EFF_THREAT: 
		{
			
		}break;
		case EFF_TRIGGER_SPELL: 
		{
			TriggerSpellId = m_spellInfo->EffectTriggerSpell[i];
		}break;
		case EFF_HEAL_MAX_HEALTH: 
		{
			EffectHealMaxHealth(i);             
		}break;
		case EFF_INTERRUPT_CAST: 
		{
			EffectInterruptCast(i);                   
		}break;
		case EFF_ADD_COMBO_POINTS: 
		{
			EffectAddComboPoints(i);
		}break;
		case EFF_DUEL:              
		{
			EffectDuel(i);          
		}break;
		case EFF_SUMMON_TOTEM_SLOT1: 
		case EFF_SUMMON_TOTEM_SLOT2: 
		case EFF_SUMMON_TOTEM_SLOT3: 
		case EFF_SUMMON_TOTEM_SLOT4: 
		{
            EffectSummonTotem(i);                    
		}break;
		case EFF_ENCHANT_HELD_ITEM: 
		{
			EffectEnchantHeldItem(i);
        }break;
		case EFF_FEED_PET: 
		{
			TriggerSpellId = m_spellInfo->EffectTriggerSpell[i];
		}break;
		case EFF_SUMMON_OBJECT_SLOT1: 
		case EFF_SUMMON_OBJECT_SLOT2: 
		case EFF_SUMMON_OBJECT_SLOT3: 
		case EFF_SUMMON_OBJECT_SLOT4: 
		{
			EffectSummonObject(i);
		}break;
		case EFF_RESURRECT: 
		{
			EffectResurrect(i);
		}break;
		default:
		{
			
			
			Log::getSingleton( ).outError("SPELL: unknown effect %d spell id %i\n",
			m_spellInfo->Effect[i], m_spellInfo->Id);
		}break;
	}
}

void Spell::HandleAddAffect(uint64 guid)
{
   Player *player = dynamic_cast<Player *>(m_caster);
   Unit *Target = NULL;
   if( player != NULL )
   {
      Target = ObjectAccessor::Instance().GetUnit(*player, guid);
      if( Target == NULL )
        return;
   }
   else
     return;               

   if(Target->tmpAffect != 0)
   {
     Target->AddAffect(Target->tmpAffect);
     Target->tmpAffect = 0;
   }
}

void Spell::TriggerSpell()
{
  if(TriggerSpellId != 0)
  {
     
     SpellEntry *spellInfo = sSpellStore.LookupEntry(TriggerSpellId );

     if(!spellInfo)
     {
       Log::getSingleton( ).outError("WORLD: unknown spell id %i\n", TriggerSpellId);
       return;
     }

     Spell spell(m_caster, spellInfo,false, 0);
     SpellCastTargets targets;
     targets.m_unitTarget = m_targets.m_unitTarget;
     spell.prepare(&targets);
  }
}

uint8 Spell::CanCast()
{
  uint8 castResult = 0;

  if (m_CastItem)
  {
     castResult = CheckItems();

     if(castResult != 0)
        SendCastResult(castResult);

     

     return castResult;
  }

  Unit *target = NULL;
  Player *pl = dynamic_cast<Player *>(m_caster);
  if( pl != NULL )
    target = ObjectAccessor::Instance().GetUnit(*pl, m_targets.m_unitTarget);
  else
    Log::getSingleton( ).outError("SPELL: (grid system) player invalid!!!");   

  if(target)
  {
    if(!m_caster->isInFront(target,GetMaxRange(sSpellRange.LookupEntry(m_spellInfo->rangeIndex))))
      castResult = 0x76;
    if(_CalcDistance(m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ(),target->GetPositionX(),target->GetPositionY(),target->GetPositionZ()) > GetMaxRange(sSpellRange.LookupEntry(m_spellInfo->rangeIndex)))
      castResult = 0x56;
  }

  if(m_targets.m_destX != 0 && m_targets.m_destY != 0  && m_targets.m_destZ != 0 )
  {
    if(_CalcDistance(m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ(),m_targets.m_destX,m_targets.m_destY,m_targets.m_destZ) > GetMaxRange(sSpellRange.LookupEntry(m_spellInfo->rangeIndex)))
      castResult = 0x56;
  }

  if(m_caster->m_silenced)
      castResult = 0x5A;

  castResult = CheckItems();

  if(castResult != 0)
    SendCastResult(castResult);

  return castResult;
}

uint8 Spell::CheckItems()
{
   if (m_caster->GetTypeId() != TYPEID_PLAYER)
       return uint8(0);

   Player* p_caster = (Player*)m_caster;
   Item* itm;
   uint32 tmpReagentCount[8];

   
   for(uint32 i=0;i<8;i++)
      tmpReagentCount[i] = m_spellInfo->ReagentCount[i];

   for(uint32 i=0;i<8;i++)
   {
      if(m_spellInfo->Reagent[i] == 0)
         continue;
      for(uint32 j=0;j<INVENTORY_SLOT_ITEM_END;j++)
      {
         itm = p_caster->GetItemBySlot(j);
         if(!itm)
           continue;
         if(itm->GetProto()->ItemId == m_spellInfo->Reagent[i] && tmpReagentCount[i] > 0)
           if(itm->GetUInt32Value(ITEM_FIELD_STACK_COUNT) > tmpReagentCount[i])
               tmpReagentCount[i] = 0;
           else
               tmpReagentCount[i] -= itm->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
      }
      if(tmpReagentCount[i] != 0)
         return uint8(0x54);
    }

    
    uint32 totems = 2;
    for(uint32 i=0;i<2;i++)
    {
       if(m_spellInfo->Totem[i] != 0)
       {
          for(uint32 j=0;j<INVENTORY_SLOT_ITEM_END;j++)
          {
             itm = p_caster->GetItemBySlot(j);
             if(itm->GetProto()->ItemId == m_spellInfo->Totem[i])
             {
               totems -= 1;
               continue;
             }
          }
       }else
         totems -= 1;
     }
     if(totems != 0)
        return uint8(0x70);

   return uint8(0);
}

void Spell::RemoveItems()
{
   if (m_caster->GetTypeId() != TYPEID_PLAYER)
      return;

   Player* p_caster = (Player*)m_caster;
   Item* itm;

   for(uint32 i=0;i<8;i++)
   {
      if(m_spellInfo->Reagent[i] == 0)
         continue;
      for(uint8 j=0;j<INVENTORY_SLOT_ITEM_END;j++)
      {
         itm = p_caster->GetItemBySlot(j);
         if(!itm)
           continue;
         if(itm->GetProto()->ItemId == m_spellInfo->Reagent[i])
           p_caster->RemoveItemFromSlot(j);
         itm = NULL;
      }
   }
}

uint32 Spell::CalculateDamage(uint8 i)
{
   uint32 value = 0;
   float basePointsPerLevel = sqrt(m_spellInfo->EffectRealPointsPerLevel[i]*m_spellInfo->EffectRealPointsPerLevel[i]);
   float randomPointsPerLevel = sqrt(m_spellInfo->EffectDicePerLevel[i]*m_spellInfo->EffectDicePerLevel[i]);
   uint32 basePoints = uint32(sqrt((float)(m_spellInfo->EffectBasePoints[i]*(float)m_spellInfo->EffectBasePoints[i]))+(m_caster->getLevel()*basePointsPerLevel));
   uint32 randomPoints = uint32(sqrt((float)(m_spellInfo->EffectDieSides[i]*(float)m_spellInfo->EffectDieSides[i]))+(m_caster->getLevel()*randomPointsPerLevel));
   uint32 comboDamage = uint32(sqrt((float)m_spellInfo->EffectPointsPerComboPoint[i]*(float)m_spellInfo->EffectPointsPerComboPoint[i]));
   uint8 comboPoints = ((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & 0xFF00) >> 8);

   if(randomPoints <= 1)
      value = basePoints+1;
   else
      value = basePoints+rand()%randomPoints;

   if(comboDamage > 0)
   {
      for(uint32 j=0;j<comboPoints;j++)
         value += comboDamage;
      m_caster->SetUInt32Value(PLAYER_FIELD_BYTES,((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (0x00 << 8)));
   }

   

   return value;
}

void Spell::HandleTeleport(uint32 id, Unit* Target)
{
   Player* pTarget=NULL;
   pTarget = ObjectAccessor::Instance().FindPlayer(Target->GetGUID());

   TeleportCoords* TC = new TeleportCoords();
   WorldPacket data;

   TC = objmgr.GetTeleportCoords(id);
   if(!TC)
     return;

   data.Initialize(SMSG_TRANSFER_PENDING);
   data << uint32(0);

   if(pTarget)
   {
#ifdef _VERSION_1_7_0_
     data << uint32(0) << uint32(0) << uint32(0);
#endif                
     pTarget->GetSession()->SendPacket(&data);
   }

   MapManager::Instance().GetMap(Target->GetMapId())->Remove(pTarget, false);


   
   data.Initialize(SMSG_NEW_WORLD);
   data << TC->mapId << TC->x << TC->y << TC->z << (float)0.0f;
   if(pTarget)
   {
#ifdef _VERSION_1_7_0_
     data << uint32(0) << uint32(0) << uint32(0);
#endif                
     pTarget->GetSession()->SendPacket(&data);
  }

  

  pTarget->SetMapId(TC->mapId);
  pTarget->Relocate(TC->x, TC->y, TC->z, 0);
  
  MapManager::Instance().GetMap(Target->GetMapId())->Add(pTarget);
}
