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

#include "Totem.h"
#include "WorldPacket.h"
#include "MapManager.h"
#include "Database/DBCStores.h"
#include "Log.h"
#include "Group.h"
#include "Player.h"
#include "ObjectMgr.h"


Totem::Totem()
{
    m_isTotem = true;
    m_spell = 0;
    m_duration = 0;
    m_type = TOTEM_PASSIVE;
}

void Totem::Update( uint32 time )
{
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

    Creature::Update( time );
}

void Totem::Summon()
{
    sLog.outDebug("AddObject at Totem.cpp line 49");
    MapManager::Instance().GetMap(GetMapId())->Add((Creature*)this);

    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM, 8);
    data << GetGUID();
    SendMessageToSet(&data,true);

    AIM_Initialize();

    if (m_type == TOTEM_PASSIVE)
        this->CastSpell(this, m_spell, true);
}

void Totem::UnSummon()
{
    if (m_type == TOTEM_LAST_BURST)
        this->CastSpell(this, m_spell, true);

    SendObjectDeSpawnAnim(GetGUID());
    SendDestroyObject(GetGUID());

    CombatStop();
    RemoveAurasDueToSpell(m_spell);
    Unit *owner = this->GetOwner();
    if (owner)
    {
        owner->RemoveAurasDueToSpell(m_spell);
	//remove aura all party members too
        Group *pGroup = NULL;
        pGroup = ((Player*)owner)->groupInfo.group;
        if (pGroup)
        {
            for(uint32 p=0;p<pGroup->GetMembersCount();p++)
            {
                if(!pGroup->SameSubGroup(owner->GetGUID(), pGroup->GetMemberGUID(p)))
                    continue;

                Unit* Target = objmgr.GetPlayer(pGroup->GetMemberGUID(p));
                if (Target) Target->RemoveAurasDueToSpell(m_spell);
            }
        }
    }

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
    uint64 ownerid = GetOwnerGUID();
    if(!ownerid)
        return NULL;
    return ObjectAccessor::Instance().GetUnit(*this, ownerid);
}

void Totem::SetSpell(uint32 spellId)
{
    //now, spellId is the spell of EffectSummonTotem , not the spell1 of totem!
    m_spell = this->GetCreatureInfo()->spell1;
    if (GetDuration(sSpellStore.LookupEntry(m_spell)) != -1)
        m_type = TOTEM_ACTIVE;

    if(spellId)
    {
        SpellEntry const *spellinfo = sSpellStore.LookupEntry(spellId);
        if ( spellinfo && spellinfo->SpellFamilyFlags == 0x28000000 )
            m_type = TOTEM_LAST_BURST;                      //For Fire Nova Totem and Corrupted Fire Nova Totem
    }
}
