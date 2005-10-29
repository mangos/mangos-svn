/* DynamicObject.cpp
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

#ifdef ENABLE_GRID_SYSTEM
#include "MapManager.h"
#endif

DynamicObject::DynamicObject() : Object()
{
    m_objectType |= TYPE_DYNAMICOBJECT;
    m_objectTypeId = TYPEID_DYNAMICOBJECT;

    m_valuesCount = DYNAMICOBJECT_END;
}


void DynamicObject::Create( uint32 guidlow, Unit *caster, SpellEntry * spell, float x, float y, float z, uint32 duration )
{
    Object::_Create(guidlow, 0xF0007000, caster->GetMapId(), x, y, z, 0);
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
}


void DynamicObject::Update(uint32 p_time)
{
/*
    if (m_nextThinkTime > time(NULL))
        return; // Think once every 5 secs only for GameObject updates...

    m_nextThinkTime = time(NULL) + 5;
*/

    bool deleteThis = false;
    WorldPacket data;
    // Delete Timer
    if(m_aliveDuration > 0)
    {
        if(m_aliveDuration > p_time)
            m_aliveDuration -= p_time;
        else
        {
            if(this->IsInWorld())
            {
                deleteThis = true;
                WorldPacket data;

                data.Initialize(SMSG_GAMEOBJECT_DESPAWN_ANIM);
                data << GetGUID();
                SendMessageToSet(&data,true);
            }
        }
    }

    if(GetUInt32Value(OBJECT_FIELD_TYPE ) == 65)  // Persistent Area Aura
    {
        if(m_PeriodicDamageCurrentTick > p_time)
            m_PeriodicDamageCurrentTick -= p_time;
        else
        {
            m_PeriodicDamageCurrentTick = m_PeriodicDamageTick;
#ifndef ENABLE_GRID_SYSTEM        
        DealDamage();
#else
        m_caster->DealWithSpellDamage(*this);
#endif
        }
    }

    if(deleteThis)
    {
        m_PeriodicDamage = 0;
        m_PeriodicDamageTick = 0;
        // RemoveFromMap();
        RemoveFromWorld();
#ifndef ENABLE_GRID_SYSTEM
        objmgr.RemoveObject(this);
        delete this;
#else
    MapManager::Instance().GetMap(m_mapId)->Remove(this, true);
#endif
    }
}

#ifndef ENABLE_GRID_SYSTEM
void DynamicObject::DealDamage()
{
    std::set<Object*>::iterator itr;
    for( itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); itr++ )
    {
        if( (*itr)->GetTypeId() != TYPEID_UNIT && (*itr)->GetTypeId() != TYPEID_PLAYER )
            continue;
        if(!((Unit*)(*itr))->isAlive())
            continue;

        if(_CalcDistance(GetPositionX(),GetPositionY(),GetPositionZ(),(*itr)->GetPositionX(),(*itr)->GetPositionY(),(*itr)->GetPositionZ()) < m_PeriodicDamageRadius && (*itr)->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) != m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE))
        {
            m_caster->PeriodicAuraLog((Unit*)(*itr),m_spell->Id,m_PeriodicDamage,m_spell->School);
        }
    }
}
#else
void DynamicObject::DealWithSpellDamage(Player &caster)
{
    for(Player::InRangeUnitsMapType::iterator iter = caster.InRangeUnitsBegin(); iter != caster.InRangeUnitsEnd(); ++iter)
    {
    if( iter->second->isAlive() )
    {
        if(_CalcDistance(GetPositionX(),GetPositionY(),GetPositionZ(),iter->second->GetPositionX(),iter->second->GetPositionY(),iter->second->GetPositionZ()) < m_PeriodicDamageRadius && iter->second->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) != caster.GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE))
        {
        caster.PeriodicAuraLog(iter->second,m_spell->Id,m_PeriodicDamage,m_spell->School);
        }        
    }
    }
}

void DynamicObject::DealWithSpellDamage(Unit &caster)
{
}
#endif
