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

#include "Totem.h"
#include "WorldPacket.h"
#include "MapManager.h"
#include "Database/DBCStores.h"

Totem::Totem()
{
    m_isTotem = true;
    m_spell = 0;
    m_duration = 0;
    m_type = TOTEM_PASSIVE;
}

void Totem::Update( uint32 time )
{
    Creature::Update( time );
    Unit *owner = GetOwner();
    if (!owner || !owner->isAlive() || !this->isAlive())
    {
        UnSummon();                                         // remove self
        return;
    }

    if (m_duration <= time)
    {
        UnSummon();                                         // remove self
        return;
    }
    else
        m_duration -= time;
}

void Totem::Summon()
{
    WorldPacket data;
    sLog.outDebug("AddObject at Totem.cpp line 49");
    MapManager::Instance().GetMap(GetMapId())->Add((Creature*)this);

    data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
    data << GetGUID();
    SendMessageToSet(&data,true);

    AIM_Initialize();

    if (m_type == TOTEM_PASSIVE)
        this->CastSpell(this, m_spell, true);
}

void Totem::UnSummon()
{
    WorldPacket data;
    data.Initialize(SMSG_GAMEOBJECT_DESPAWN_ANIM);
    data << GetGUID();
    SendMessageToSet(&data, true);
    data.Initialize(SMSG_DESTROY_OBJECT);
    data << GetGUID();
    SendMessageToSet(&data, true);

    CombatStop();
    RemoveAurasDueToSpell(m_spell);
    Unit *owner = this->GetOwner();
    if (owner)
        owner->RemoveAurasDueToSpell(m_spell);

    ObjectAccessor::Instance().AddObjectToRemoveList(this);
}

void Totem::SetOwner(uint64 guid)
{
    SetUInt64Value(UNIT_FIELD_SUMMONEDBY, guid);
    Unit *owner = this->GetOwner();
    if (owner)
    {
        this->setFaction(owner->getFaction());
        this->SetLevel(owner->getLevel());
    }
}

Unit *Totem::GetOwner()
{
    uint64 ownerid = GetUInt64Value(UNIT_FIELD_SUMMONEDBY);
    if(!ownerid)
        return NULL;
    return ObjectAccessor::Instance().GetUnit(*this, ownerid);
}

void Totem::SetSpell(uint32 spellId)
{
    m_spell = spellId;
    if (GetDuration(sSpellStore.LookupEntry(m_spell)) != -1)
        m_type = TOTEM_ACTIVE;
}
