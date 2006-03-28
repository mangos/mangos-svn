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
#include "GameObject.h"
#include "UpdateMask.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Database/DatabaseEnv.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "RedZoneDistrict.h"

DynamicObject::DynamicObject() : Object()
{
    m_objectType |= TYPE_DYNAMICOBJECT;
    m_objectTypeId = TYPEID_DYNAMICOBJECT;

    m_valuesCount = DYNAMICOBJECT_END;
}


void DynamicObject::Create( uint32 guidlow, Unit *caster, SpellEntry * spell, float x, float y, float z, uint32 duration )
{
    Object::_Create(guidlow, 0xF0007000, caster->GetMapId(), x, y, z, 0, (uint8)-1);
    m_spell = spell;
    m_caster = caster;

    SetUInt32Value( OBJECT_FIELD_ENTRY, spell->Id );
    SetFloatValue( OBJECT_FIELD_SCALE_X, 1 );
    SetUInt64Value( DYNAMICOBJECT_CASTER, caster->GetGUID() );
    SetUInt32Value( DYNAMICOBJECT_BYTES, 0x00000001 );
    SetUInt32Value( DYNAMICOBJECT_SPELLID, spell->Id );
    SetFloatValue( DYNAMICOBJECT_RADIUS, (float)GetRadius(sSpellRadius.LookupEntry(spell->EffectRadiusIndex[0] )));
    SetFloatValue( DYNAMICOBJECT_POS_X, x );
    SetFloatValue( DYNAMICOBJECT_POS_Y, y );
    SetFloatValue( DYNAMICOBJECT_POS_Z, z );

    m_aliveDuration = duration;
	deleteThis = false;
}


void DynamicObject::Update(uint32 p_time)
{


    WorldPacket data;

    
    if(m_aliveDuration > 0)
    {
        if(m_aliveDuration > p_time)
            m_aliveDuration -= p_time;
        else
        {
            if(this->IsInWorld())
            {
                deleteThis = true;
            }
        }
    }

    if(GetUInt32Value(OBJECT_FIELD_TYPE ) == 65)  
    {
        if(m_PeriodicDamageCurrentTick > p_time)
            m_PeriodicDamageCurrentTick -= p_time;
        else
        {
            m_PeriodicDamageCurrentTick = m_PeriodicDamageTick;
            m_caster->DealWithSpellDamage(*this);
        }
    }

    if(deleteThis)
    {
        Delete();
    }
}

void DynamicObject::DealWithSpellDamage(Player &caster)
{
    uint32 runtimes=1;
    if(deleteThis)
        runtimes=m_DamageMaxTimes-m_DamageCurTimes;

	Modifier mod;
	mod.m_auraname = 3;                             
    mod.m_amount = m_PeriodicDamage;
       
    for(int i=0;i<runtimes;i++)
    {
      UnitList.clear();
      MapManager::Instance().GetMap(m_mapId)->GetUnitList(GetPositionX(), GetPositionY(),UnitList);
      for(std::list<Unit*>::iterator iter=UnitList.begin();iter!=UnitList.end();iter++)
      {
        if((*iter))
        {
            if( (*iter)->isAlive() )
           {
                if(_CalcDistance(GetPositionX(),GetPositionY(),GetPositionZ(),(*iter)->GetPositionX(),(*iter)->GetPositionY(),(*iter)->GetPositionZ()) < m_PeriodicDamageRadius && (*iter)->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) != caster.GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE))
                {
                 caster.PeriodicAuraLog((*iter),m_spell,&mod);
                }        
           }
        }
      }
      m_DamageCurTimes++;
    }
}

void DynamicObject::DealWithSpellDamage(Unit &caster)
{
}

void DynamicObject::Delete()
{  
	
    m_PeriodicDamage = 0;
    m_PeriodicDamageTick = 0;

    WorldPacket data;

    data.Initialize(SMSG_GAMEOBJECT_DESPAWN_ANIM);
    data << GetGUID();
    SendMessageToSet(&data,true);

    data.Initialize(SMSG_DESTROY_OBJECT);
    data << GetGUID();
    SendMessageToSet(&data,true);

	//FIX ME ,NEED TO DELETE
	//MapManager::Instance().GetMap(GetMapId())->Remove(this, true);   
	//Log::getSingleton( ).outError("Don't Forget FIX ME at DynamicObject.cpp \n");
	RemoveFromWorld();
	 
}
