/* Spell.cpp
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

// written by nothin ask him if you dont understand parts of it
// also ask me if you have problems with functions i call within my Code or the Affect system

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

    m_spellInfo = info;
    m_caster = Caster;

    m_spellState = SPELL_STATE_NULL;

    m_castPositionX = m_castPositionY = m_castPositionZ;
    TriggerSpellId = 0;
    m_targetCount = 0;
    m_triggeredSpell = triggered;
    m_AreaAura = false;

    m_triggeredByAffect = aff;
}


void Spell::FillTargetMap()
{
    Player* p_caster = (Player*)m_caster;
    std::list<uint64> tmpMap;
    uint32 cur = 0;
    for(uint32 i=0;i<3;i++)
    {
        for(uint32 j=0;j<2;j++)
        {
            if(j==0)
                cur = m_spellInfo->EffectImplicitTargetA[i];
            if(j==1)
                cur = m_spellInfo->EffectImplicitTargetB[i];
            switch(cur)
            {
                case 1:                           // Self Target
                {
                    tmpMap.push_back(m_caster->GetGUID());
                }break;
                case 5:                           // Target: Pet
                {
                    tmpMap.push_back(m_caster->GetUInt32Value(UNIT_FIELD_PETNUMBER));
                }break;
                case 6:                           // Single Target Enemy
                {
                    tmpMap.push_back(m_targets.m_unitTarget);
                }break;
                case 15:                          // All Enemies in Area of Effect (TEST)
                case 16:                          // All Enemies in Area of Effect instant (e.g. Flamestrike)
                {
		    CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
		    Cell cell = RedZone::GetZone(p);
		    cell.data.Part.reserved = ALL_DISTRICT;
		    cell.SetNoCreate();
		    MaNGOS::SpellNotifierPlayer notifier(*this, tmpMap, i);
		    TypeContainerVisitor<MaNGOS::SpellNotifierPlayer, ContainerMapList<Player> > player_notifier(notifier);
		    CellLock<GridReadGuard> cell_lock(cell, p);
		    cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
		}break;
                    case 20:                      // All Party Members around the Caster
                    {
                        Group* pGroup = objmgr.GetGroupByLeader(p_caster->GetGroupLeader());
                        if(pGroup)
                            for(uint32 p=0;p<pGroup->GetMembersCount();p++)
                        {
                            Unit* Target = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(p));
                            if(!Target || Target->GetGUID() == m_caster->GetGUID())
                                continue;
                            if(_CalcDistance(m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ(),Target->GetPositionX(),Target->GetPositionY(),Target->GetPositionZ()) < GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i])))
                                tmpMap.push_back(Target->GetGUID());
                        }
                        else
                            tmpMap.push_back(m_caster->GetGUID());
                    }break;
                    case 21:                      // Single Target Friend
                    {
                        tmpMap.push_back(m_targets.m_unitTarget);
                    }break;
                    case 22:                      // Enemy Targets around the Caster
                    {
			CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
			Cell cell = RedZone::GetZone(p);
			cell.data.Part.reserved = ALL_DISTRICT;
			cell.SetNoCreate();
			MaNGOS::SpellNotifierPlayer notifier(*this, tmpMap, i);
			TypeContainerVisitor<MaNGOS::SpellNotifierPlayer, ContainerMapList<Player> > player_notifier(notifier);
			CellLock<GridReadGuard> cell_lock(cell, p);
			cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));

		    }break;
                        case 23:                  // Gameobject Target
                        {
                            tmpMap.push_back(m_targets.m_unitTarget);
                        }break;
                        case 24:                  // Targets in Front of the Caster
                        {
			    CellPair p(MaNGOS::ComputeCellPair(p_caster->GetPositionX(), p_caster->GetPositionY()));
			    Cell cell = RedZone::GetZone(p);
			    cell.data.Part.reserved = ALL_DISTRICT;
			    cell.SetNoCreate();
			    MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, tmpMap, i);
			    TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, ContainerMapList<Player> > player_notifier(notifier);
			    TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
			    CellLock<GridReadGuard> cell_lock(cell, p);
			    cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
			    cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
			}break;
                            case 25:              //Target is duel vs player add by vendy
                            {
                                tmpMap.push_back(m_targets.m_unitTarget);
                            }break;
                            case 26:              // Gameobject/Item Target
                            {
                                if(m_targets.m_unitTarget)
                                    tmpMap.push_back(m_targets.m_unitTarget);
                                if(m_targets.m_itemTarget)
                                    tmpMap.push_back(m_targets.m_itemTarget);
                            }break;
                            case 28:              // All Enemies in Area of Effect(Blizzard/Rain of Fire/volley) channeled
			    {
				CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
				Cell cell = RedZone::GetZone(p);
				cell.data.Part.reserved = ALL_DISTRICT;
				cell.SetNoCreate();
				MaNGOS::SpellNotifierPlayer notifier(*this, tmpMap, i);
				TypeContainerVisitor<MaNGOS::SpellNotifierPlayer, ContainerMapList<Player> > player_notifier(notifier);
				CellLock<GridReadGuard> cell_lock(cell, p);
				cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
                                }break;
                                case 32:          // Minion Target
                                {
                                }break;
                                case 35:          // Single Target Party Member
                                {
                                    tmpMap.push_back(m_targets.m_unitTarget);
                                }break;
                                case 45:          // Chain
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
                                            tmpMap.push_back(Target->GetGUID());
                                    }
                                }break;
                                case 53:          // Target Area by Players CurrentSelection()
                                {
				    CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
				    Cell cell = RedZone::GetZone(p);
				    cell.data.Part.reserved = ALL_DISTRICT;
				    cell.SetNoCreate();
				    MaNGOS::SpellNotifierPlayer notifier(*this, tmpMap, i);
				    TypeContainerVisitor<MaNGOS::SpellNotifierPlayer, ContainerMapList<Player> > player_notifier(notifier);
				    CellLock<GridReadGuard> cell_lock(cell, p);
				    cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
				}break;
                                    default:
                                    {
                                    }break;
                                }
// Add Chain Targets
                                if(m_spellInfo->EffectChainTarget[i] != 0)
                                {
                                    if(m_targets.m_unitTarget)
                                    {
                                        Unit* target = ObjectAccessor::Instance().GetCreature(*p_caster, m_targets.m_unitTarget);
                                    }
                                }

                            }
                            if(i == 0)
                                m_targetUnits1 = tmpMap;
                            else if(i == 1)
                                m_targetUnits2 = tmpMap;
                            else if(i == 2)
                                m_targetUnits3 = tmpMap;
                            tmpMap.clear();
                        }
                    }

                    void Spell::prepare(SpellCastTargets * targets)
                    {
                        uint8 result;

                        WorldPacket data;

                        m_targets = *targets;

                        SendSpellStart();

                        m_timer = GetCastTime(sCastTime.LookupEntry(m_spellInfo->CastingTimeIndex));
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
                            SendCastResult(0x20); // Interrupted
                        }
                        else if(m_spellState = SPELL_STATE_CASTING)
                        {
                            m_caster->RemoveAffectById(m_spellInfo->Id);
                            SendChannelUpdate(0);
                        }

                        m_spellState = SPELL_STATE_FINISHED;
                    }

                    void Spell::cast()
                    {
                        WorldPacket data;

                        bool Instant = true;
// for(uint32 i=0;i<=2;i++)
// {
//     if(m_spellInfo->Effect[i] == 17 || m_spellInfo->Effect[i] == 58)
//         Instant = false;
// }

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
                                                  // Area Aura
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
                        if(m_castPositionX != m_caster->GetPositionX() ||
                            m_castPositionY != m_caster->GetPositionY() ||
                            m_castPositionZ != m_caster->GetPositionZ() )
                        {
                            SendInterrupted(0);
                            SendCastResult(0x20); // Interrupted
                            if(m_spellState == SPELL_STATE_CASTING)
                            {
                                m_caster->RemoveAffectById(m_spellInfo->Id);
                                SendChannelUpdate(0);
                            }
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
                                { }
                                break;
                        }
                    }

                    void Spell::finish()
                    {
                        WorldPacket data;

                        m_spellState = SPELL_STATE_FINISHED;
                        m_caster->m_meleeSpell = false;
                        m_caster->m_canMove = true;

/* Mana Regenerates while in combat but not for 5 seconds after each spell */
                                                  /* 5 Seconds */
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
#endif                    //_VERSION_1_7_0_

                        ((Player*)m_caster)->GetSession()->SendPacket(&data);
                    }

                    void Spell::SendSpellStart()
                    {
                        // Send Spell Start
                        WorldPacket data;
                        uint16 cast_flags;

                        cast_flags = 2;

                        data.Initialize(SMSG_SPELL_START);
                        data << m_caster->GetGUID() << m_caster->GetGUID();
                        data << m_spellInfo->Id;
                        data << cast_flags;
                        data << GetCastTime(sCastTime.LookupEntry(m_spellInfo->CastingTimeIndex));

                        m_targets.write( &data );
                        ((Player*)m_caster)->SendMessageToSet(&data, true);

                    }

                    void Spell::SendSpellGo()
                    {
                        // Start Spell
                        WorldPacket data;
                        uint16 flags;

                        flags = m_targets.m_targetMask;
                        if(flags == 0)
                            flags = 2;

                        data.Initialize(SMSG_SPELL_GO);

                        data << m_caster->GetGUID() << m_caster->GetGUID();
                        data << m_spellInfo->Id;
                        // data << flags;
                        data << uint16(0x0500);
                        writeSpellGoTargets(&data);

                        data << (uint8)0;         // number of misses

#if defined ( _VERSION_1_7_0_ ) || defined ( _VERSION_1_7_0_ )
                        data << uint32(0) << uint32(0) << uint32(0);
#endif // defined ( _VERSION_1_7_0_ ) || defined ( _VERSION_1_7_0_ )

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
#endif // defined ( _VERSION_1_7_0_ ) || defined ( _VERSION_1_7_0_ )

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

                        if(!target)
                            return;
                        if (m_caster->GetTypeId() == TYPEID_PLAYER)
                        {
                            // Send Channel Start
                            WorldPacket data;
                            data.Initialize( MSG_CHANNEL_START );
                            data << m_spellInfo->Id;
                            data << duration;

#if defined ( _VERSION_1_7_0_ ) || defined ( _VERSION_1_7_0_ )
                        data << uint32(0) << uint32(0) << uint32(0);
#endif // defined ( _VERSION_1_7_0_ ) || defined ( _VERSION_1_7_0_ )

                            ((Player*)m_caster)->GetSession()->SendPacket( &data );
                        }

                        m_timer = duration;

                        m_caster->SetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT,target->GetGUIDLow());
                        m_caster->SetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT+1,target->GetGUIDHigh());
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
#endif // defined ( _VERSION_1_7_0_ ) || defined ( _VERSION_1_7_0_ )

                        target->GetSession()->SendPacket(&data);
                        return;
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
                    void Spell::TakePower()
                    {
                        uint16 powerField;
                        uint32 currentPower;

                        uint8 powerType = (uint8)(m_caster->GetUInt32Value(UNIT_FIELD_BYTES_0) >> 24);
                        if(powerType == 0)        // Mana
                            powerField = UNIT_FIELD_POWER1;
                        else if(powerType == 1)   // Rage
                            powerField = UNIT_FIELD_POWER2;
                        else if(powerType == 3)   // Energy
                            powerField = UNIT_FIELD_POWER4;

                        currentPower = m_caster->GetUInt32Value(powerField);

                        if(currentPower < m_spellInfo->manaCost)
                            m_caster->SetUInt32Value(powerField, 0);
                        else
                            m_caster->SetUInt32Value(powerField, currentPower - m_spellInfo->manaCost);
                    }

                    void Spell::HandleEffects(uint64 guid,uint32 i)
                    {
                        Unit* unitTarget;
                        // Item* itemTarget;
                        GameObject* gameObjTarget;
                        Player* playerTarget, *playerCaster;
                        WorldPacket data;
                        data.clear();
                        playerCaster = ObjectAccessor::Instance().FindPlayer(m_caster->GetGUID());
                        if( playerCaster != NULL )
                        {
                            unitTarget = ObjectAccessor::Instance().GetUnit(*playerCaster, guid);
                            gameObjTarget = ObjectAccessor::Instance().GetGameObject(*playerCaster, guid);
                            playerTarget = ObjectAccessor::Instance().GetPlayer(*playerCaster, guid);
                        }

                        uint32 damage = 0;
                        damage = CalculateDamage((uint8)i);

                        switch(m_spellInfo->Effect[i])
                        {
                            case 2:               // School Damage
                            {
                                if(!unitTarget)
                                    break;
                                if(!unitTarget->isAlive())
                                    break;

                                m_caster->SpellNonMeleeDamageLog(unitTarget,m_spellInfo->Id, damage);
                            }break;
                            case 3:               // Dummy
                            {
                            }break;
                            case 5:               // Teleport Units
                            {
                                if(!unitTarget)
                                    break;
                                HandleTeleport(m_spellInfo->Id,unitTarget);
                            }break;
                            case 6:               // Apply Aura
                            {
                                if(!unitTarget)
                                    break;
                                if(!unitTarget->isAlive())
                                    break;

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

                                // Periodic Trigger Damage
                                if(m_spellInfo->EffectApplyAuraName[i] == 3)
                                {
                                    unitTarget->tmpAffect->SetDamagePerTick(damage,m_spellInfo->EffectAmplitude[i]);
                                    unitTarget->tmpAffect->SetNegative();
                                    // Periodic Trigger Spell
                                }
                                else if(m_spellInfo->EffectApplyAuraName[i] == 23)
                                    unitTarget->tmpAffect->SetPeriodicTriggerSpell(m_spellInfo->EffectTriggerSpell[i],m_spellInfo->EffectAmplitude[i]);
                                    // Periodic Heal
                                else if(m_spellInfo->EffectApplyAuraName[i] == 8)
                                    unitTarget->tmpAffect->SetHealPerTick(damage,m_spellInfo->EffectAmplitude[i]);
                                else
                                {
                                    unitTarget->tmpAffect->AddMod(m_spellInfo->EffectApplyAuraName[i],damage,m_spellInfo->EffectMiscValue[i],type);
                                }
                            }break;
                            case 8:               // Power Drain
                            {
                                if(!unitTarget)
                                    break;
                                if(!unitTarget->isAlive())
                                    break;

                                uint32 curPower = unitTarget->GetUInt32Value(UNIT_FIELD_POWER1);
                                if(curPower < damage)
                                    unitTarget->SetUInt32Value(UNIT_FIELD_POWER1,0);
                                else
                                    unitTarget->SetUInt32Value(UNIT_FIELD_POWER1,curPower-damage);
                            }break;
                            case 10:              // Heal
                            {
                                if(!unitTarget)
                                    break;
                                if(!unitTarget->isAlive())
                                    break;

                                uint32 curHealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
                                uint32 maxHealth = unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
                                if(curHealth+damage > maxHealth)
                                    unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,maxHealth);
                                else
                                    unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,curHealth+damage);
                            }break;
                            case 17:              // Weapon Damage + (no School)
                            {
                                if(!unitTarget)
                                    break;
                                if(!unitTarget->isAlive())
                                    break;

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
                            }break;
                            case 24:              // Create item      // NEEDS TO BE REDONE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                            {
/*
                Player* pUnit = (Player*)m_caster;
                uint8 slot = 0;
                for(uint32 i=INVENTORY_SLOT_ITEM_START;i<INVENTORY_SLOT_ITEM_END;i++){// check if there is a free slot for the item to conjure
                    if(pUnit->GetItemBySlot(i) == 0)
                        slot = i;
                }
                if(slot == 0){
                    SendCastResult(0x18);
                    return;
                }

Item* newItem;
for(i=0;i<2;i++)
{
// now create the Items
if(m_spellInfo->EffectItemType[i] == 0)
continue;

slot = 0;
for(uint32 i=INVENTORY_SLOT_ITEM_START;i<INVENTORY_SLOT_ITEM_END;i++)
{
// check if there is a free slot for the item to conjure
if(pUnit->GetItemBySlot(i) == 0)
slot = i;
}
if(slot == 0)
{
SendCastResult(0x18);
return;
}
newItem = new Item;
newItem->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM),m_spellInfo->EffectItemType[i],pUnit);
pUnit->AddItemToSlot(slot,newItem);
newItem = NULL;
}
*/
                            }break;
                            case 27:              // Persistent Area Aura
                            {
                                if(m_AreaAura == true)
                                    break;

                                m_AreaAura = true;
                                // Spawn dyn GameObject
                                DynamicObject* dynObj = new DynamicObject();
                                dynObj->Create(objmgr.GenerateLowGuid(HIGHGUID_DYNAMICOBJECT), m_caster, m_spellInfo, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, GetDuration(sSpellDuration.LookupEntry(m_spellInfo->DurationIndex)));
                                dynObj->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
                                dynObj->SetUInt32Value(GAMEOBJECT_DISPLAYID, 368003);
                                dynObj->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x01eeeeee);
                                dynObj->PeriodicTriggerDamage(damage, m_spellInfo->EffectAmplitude[i], GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i])));

                                // objmgr.AddObject(dynObj);
                                // dynObj->AddToWorld();
                            }break;
                            case 30:              // Energize
                            {
                                if(!unitTarget)
                                    break;
                                if(!unitTarget->isAlive())
                                    break;
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
                            }break;
                            case 33:              // Open Lock
                            {
                                if(!gameObjTarget || !playerTarget)
                                    break;

                                data.clear();
                                data.Initialize(SMSG_LOOT_RESPONSE);
                                gameObjTarget->FillLoot(*playerTarget, &data);
                                playerTarget->SetLootGUID(m_targets.m_unitTarget);
                                playerTarget->GetSession()->SendPacket(&data);
                            }break;
                            case 35:              // Apply Area Aura
                            {
                                if(!unitTarget)
                                    break;
                                if(!unitTarget->isAlive())
                                    break;

                                Affect* aff = new Affect(m_spellInfo,6000,m_caster->GetGUID());
                                aff->AddMod(m_spellInfo->EffectApplyAuraName[i],m_spellInfo->EffectBasePoints[i]+rand()%m_spellInfo->EffectDieSides[i]+1,m_spellInfo->EffectMiscValue[i],0);

                                unitTarget->SetAura(aff);
                            }break;
                            case 36:              // Learn Spell
                            {
                                if(!playerTarget)
                                    return;
                                uint32 spellToLearn = m_spellInfo->EffectTriggerSpell[i];
                                playerTarget->addSpell((uint16)spellToLearn);
                                data.Initialize(SMSG_LEARNED_SPELL);
                                data << spellToLearn;
                                playerTarget->GetSession()->SendPacket(&data);
                            }break;
                            case 41:              // Summon Wild
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

                                Log::getSingleton( ).outError("AddObject at Spell.cpp");
                                MapManager::Instance().GetMap(spawnCreature->GetMapId())->Add(spawnCreature);

                            }break;
                            case 53:              // Enchant Item Permanent
                            {// UQ1: I'd like this working... So, ...
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
								    {
										if(p_caster->GetItemBySlot(i) != 0)
											if(p_caster->GetItemBySlot(i)->GetProto()->ItemId == m_targets.m_itemTarget)
											{
												m_CastItem = p_caster->GetItemBySlot(i);
												item_slot = i;
											}
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
									{
										if (m_spellInfo->EffectMiscValue[i])
											m_CastItem->SetUInt32Value(ITEM_FIELD_ENCHANTMENT+(add_slot+i), m_spellInfo->EffectMiscValue[i]);
									}

									// Now actually set it up, and transmit info...
									UpdateData upd;
									WorldPacket packet;

									p_caster->ApplyItemMods( m_CastItem, item_slot, true );
									upd.Clear();
									m_CastItem->UpdateStats();
									m_CastItem->BuildCreateUpdateBlockForPlayer(&upd, (Player *)p_caster);
									upd.BuildPacket(&packet);
									p_caster->GetSession()->SendPacket(&packet);
								}
/*
                Player* p_caster = (Player*)m_caster;
                uint32 field = 99;
                if(m_CastItem)
                    field = 1;
                else
                    field = 3;
                if(!m_CastItem)
                {
                    for(uint8 i=0;i<INVENTORY_SLOT_ITEM_END;i++)
                    {
if(p_caster->GetItemBySlot(i) != 0)
if(p_caster->GetItemBySlot(i)->GetProto()->ItemId == m_targets.m_itemTarget)
m_CastItem = p_caster->GetItemBySlot(i);
}
}
// m_CastItem->Enchant(m_spellInfo->EffectMiscValue[i],0);
*/
                            }break;
                            case 54:              // Enchant Item Temporary
                            {
/*
                Player* p_caster = (Player*)m_caster;
                uint32 duration = GetDuration(sSpellDuration.LookupEntry(m_spellInfo->DurationIndex));
                uint32 field = 99;
                if(m_CastItem)
                    field = 1;
                else
                    field = 3;
                if(!m_CastItem)
                {
                    for(uint8 i=0;i<INVENTORY_SLOT_ITEM_END;i++)
{
if(p_caster->GetItemBySlot(i) != 0)
if(p_caster->GetItemBySlot(i)->GetProto()->ItemId == m_targets.m_itemTarget)
m_CastItem = p_caster->GetItemBySlot(i);
}
}

// m_CastItem->Enchant(m_spellInfo->EffectMiscValue[i],duration,field);
*/
                            }break;
                            case 58:              // Weapon damage +
                            {
                                if(!unitTarget)
                                    break;
                                if(!unitTarget->isAlive())
                                    break;

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
                            }break;
                            case 63:              // Threat
                            {
                                                  // reduce Threat
                            }break;
                            case 64:              // Trigger Spell
                            {
                                TriggerSpellId = m_spellInfo->EffectTriggerSpell[i];
                            }break;
                            case 67:              // Heal Max Health
                            {
                                if(!unitTarget)
                                    break;
                                if(!unitTarget->isAlive())
                                    break;

                                uint32 heal;
                                heal = m_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH);

                                uint32 curHealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
                                uint32 maxHealth = unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
                                if(curHealth+heal > maxHealth)
                                    unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,maxHealth);
                                else
                                    unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH,curHealth+heal);
                            }break;
                            case 68:              // Interrupt Cast
                            {
                                if(!unitTarget)
                                    break;
                                if(!unitTarget->isAlive())
                                    break;

                                unitTarget->InterruptSpell();
                            }break;
                            case 80:              // Add Combo Points
                            {
                                if(!unitTarget)
                                    return;
                                uint8 comboPoints = ((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & 0xFF00) >> 8);
                                if(m_caster->GetUInt64Value(PLAYER__FIELD_COMBO_TARGET) != unitTarget->GetGUID())
                                {
                                    m_caster->SetUInt64Value(PLAYER__FIELD_COMBO_TARGET,unitTarget->GetGUID());
                                    m_caster->SetUInt32Value(PLAYER_FIELD_BYTES,((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (0x01 << 8)));
                                }
                                else
                                {
                                    if(comboPoints < 5)
                                    {
                                        comboPoints += 1;
                                        m_caster->SetUInt32Value(PLAYER_FIELD_BYTES,((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (comboPoints << 8)));
                                    }
                                }
                            }break;
                            case 83:              //Duel
                            {
                             GameObject* pGameObj = new GameObject();

                             uint32 gameobject_id = m_spellInfo->EffectMiscValue[i];;

                             // uint32 guidlow, uint16 display_id, uint8 state, uint32 obj_field_entry, uint8 scale, uint16 type, uint16 faction,  float x, float y, float z, float ang
                             pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), gameobject_id,playerCaster->GetMapId(), 
                                 playerCaster->GetPositionX()+(playerTarget->GetPositionX()-playerCaster->GetPositionX())/2 , 
                                 playerCaster->GetPositionY()+(playerTarget->GetPositionY()-playerCaster->GetPositionY())/2 , 
                                 playerCaster->GetPositionZ(), 
                                 playerCaster->GetOrientation());
                             pGameObj->SetUInt32Value(OBJECT_FIELD_ENTRY, m_spellInfo->EffectMiscValue[i] );
                             pGameObj->SetUInt32Value(GAMEOBJECT_DISPLAYID, 787 );
                             pGameObj->SetUInt32Value(GAMEOBJECT_FACTION, 4 );
                             pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, 16 );
                             pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, 57 );

                             Log::getSingleton( ).outError("AddObject at Spell.cpp 1247");
                             MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);

                             SendDuelRequest(playerCaster, playerTarget , pGameObj->GetGUID());

                            }break;
                            case 87:              // Summon Totem (slot 1)
                            case 88:              // Summon Totem (slot 2)
                            case 89:              // Summon Totem (slot 3)
                            case 90:              // Summon Totem (slot 4)
                            {
                                uint64 guid = 0;
// delete old summoned object
                                if(m_spellInfo->Effect[i] == 87)
                                {
                                    guid = m_caster->m_TotemSlot1;
                                    m_caster->m_TotemSlot1 = 0;
                                }
                                else if(m_spellInfo->Effect[i] == 88)
                                {
                                    guid = m_caster->m_TotemSlot2;
                                    m_caster->m_TotemSlot2 = 0;
                                }
                                else if(m_spellInfo->Effect[i] == 89)
                                {
                                    guid = m_caster->m_TotemSlot3;
                                    m_caster->m_TotemSlot3 = 0;
                                }
                                else if(m_spellInfo->Effect[i] == 90)
                                {
                                    guid = m_caster->m_TotemSlot4;
                                    m_caster->m_TotemSlot4 = 0;
                                }
                                if(guid != 0)
                                {
                                    Creature* Totem = NULL;
				    /* TODO siuolly: get the totem for the caster
				    ObjectAccessor::Instance().GetCreature(m_caster, guid);				    
				    */

                                    if(Totem)
                                    {
                                        MapManager::Instance().GetMap(Totem->GetMapId())->Remove(Totem, true);
                                        Totem = NULL; // protect it.
                                    }
                                }

				// spawn a new one
                                Creature* pTotem = new Creature();
                                CreatureInfo* ci = objmgr.GetCreatureName(m_spellInfo->EffectMiscValue[i]);
                                if(!ci)
                                {
                                    printf("break: unknown CreatureEntry\n");
                                    return;
                                }
                                char* name = (char*)ci->Name.c_str();

                                // uint32 guidlow, uint16 display_id, uint8 state, uint32 obj_field_entry, uint8 scale, uint16 type, uint16 faction,  float x, float y, float z, float ang
								pTotem->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), name, m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), ci->DisplayID );
                                pTotem->SetUInt32Value(OBJECT_FIELD_TYPE,33);
                                pTotem->SetUInt32Value(UNIT_FIELD_DISPLAYID,ci->DisplayID);
                                pTotem->SetUInt32Value(UNIT_FIELD_LEVEL,m_caster->getLevel());
                                Log::getSingleton( ).outError("AddObject at Spell.cppl line 1040");
                                MapManager::Instance().GetMap(pTotem->GetMapId())->Add(pTotem);


                                data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
                                data << pTotem->GetGUID();
                                m_caster->SendMessageToSet(&data,true);

                                if(m_spellInfo->Effect[i] == 87)
                                    m_caster->m_TotemSlot1 = pTotem->GetGUID();
                                else if(m_spellInfo->Effect[i] == 88)
                                    m_caster->m_TotemSlot2 = pTotem->GetGUID();
                                else if(m_spellInfo->Effect[i] == 89)
                                    m_caster->m_TotemSlot3 = pTotem->GetGUID();
                                else if(m_spellInfo->Effect[i] == 90)
                                    m_caster->m_TotemSlot4 = pTotem->GetGUID();
                            }break;
                            case 92:              // Enchant Held Item
                            {

                            }break;
                            case 101:             // Feed Pet
                            {
                                TriggerSpellId = m_spellInfo->EffectTriggerSpell[i];
                            }break;
                            case 104:             // Summon Object (slot 1)
                            case 105:             // Summon Object (slot 2)
                            case 106:             // Summon Object (slot 3)
                            case 107:             // Summon Object (slot 4)
                            {

                                uint64 guid = 0;
                                // delete old summoned object
                                if(m_spellInfo->Effect[i] == 104)
                                {
                                    guid = m_caster->m_TotemSlot1;
                                    m_caster->m_TotemSlot1 = 0;
                                }
                                else if(m_spellInfo->Effect[i] == 105)
                                {
                                    guid = m_caster->m_TotemSlot2;
                                    m_caster->m_TotemSlot2 = 0;
                                }
                                else if(m_spellInfo->Effect[i] == 106)
                                {
                                    guid = m_caster->m_TotemSlot3;
                                    m_caster->m_TotemSlot3 = 0;
                                }
                                else if(m_spellInfo->Effect[i] == 107)
                                {
                                    guid = m_caster->m_TotemSlot4;
                                    m_caster->m_TotemSlot4 = 0;
                                }
                                if(guid != 0)
                                {
                                    GameObject* obj = NULL;
                                    if( playerCaster )
                                        obj = ObjectAccessor::Instance().GetGameObject(*playerCaster, guid);

                                    if(obj)
                                    {
					// Remove and delete
                                        MapManager::Instance().GetMap(obj->GetMapId())->Remove(obj, true);
					obj = NULL;
                                    }
                                }

                                // spawn a new one
                                GameObject* pGameObj = new GameObject();
                                uint16 display_id = m_spellInfo->EffectMiscValue[i];

                                // uint32 guidlow, uint16 display_id, uint8 state, uint32 obj_field_entry, uint8 scale, uint16 type, uint16 faction,  float x, float y, float z, float ang
                                pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), display_id,m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation());
                                pGameObj->SetUInt32Value(OBJECT_FIELD_ENTRY, m_spellInfo->EffectMiscValue[i]);
                                pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, 6);
                                pGameObj->SetUInt32Value(OBJECT_FIELD_TYPE,33);
                                pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL,m_caster->getLevel());
                                Log::getSingleton( ).outError("AddObject at Spell.cpp 1100");

                                MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);
                                data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
                                data << pGameObj->GetGUID();
                                m_caster->SendMessageToSet(&data,true);

                                if(m_spellInfo->Effect[i] == 104)
                                    m_caster->m_TotemSlot1 = pGameObj->GetGUID();
                                else if(m_spellInfo->Effect[i] == 105)
                                    m_caster->m_TotemSlot2 = pGameObj->GetGUID();
                                else if(m_spellInfo->Effect[i] == 106)
                                    m_caster->m_TotemSlot3 = pGameObj->GetGUID();
                                else if(m_spellInfo->Effect[i] == 107)
                                    m_caster->m_TotemSlot4 = pGameObj->GetGUID();
                            }break;
                            case 113:             // Resurrect (Flat)
                            {
                                if(!playerTarget)
                                    break;
                                if(playerTarget->isAlive())
                                    break;
                                if(!playerTarget->IsInWorld())
                                    break;

                                uint32 health = m_spellInfo->EffectBasePoints[i];
                                uint32 mana = m_spellInfo->EffectMiscValue[i];
                                playerTarget->setResurrect(m_caster->GetGUID(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
                                SendResurrectRequest(playerTarget);
                            }break;
                            default:
                            {
                                // PLAYER_TRACK_CREATURES 2^X
                                // printf("unknown effect\n");
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
                            return;               // FIX ME

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
                            // check for spell id
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
						{// UQ1: Cast a spell on an item should always be OK! ???
							castResult = CheckItems();

							 if(castResult != 0)
								SendCastResult(castResult);

							 //uint8(0x54)

							return castResult;
						}

                        Unit *target = NULL;
                        Player *pl = dynamic_cast<Player *>(m_caster);
                        if( pl != NULL )
                            target = ObjectAccessor::Instance().GetUnit(*pl, m_targets.m_unitTarget);
                        else
			    Log::getSingleton( ).outError("SPELL: (grid system) player invalid!!!");   // FIX ME PLEASE..

                        if(target)
                        {
                            if(!m_caster->isInFront(target,GetMaxRange(sSpellRange.LookupEntry(m_spellInfo->rangeIndex))))
                                castResult = 0x76;// Target needs to be in Front of you
                            if(_CalcDistance(m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ(),target->GetPositionX(),target->GetPositionY(),target->GetPositionZ()) > GetMaxRange(sSpellRange.LookupEntry(m_spellInfo->rangeIndex)))
                                castResult = 0x56;// Target out of Range
                        }

                        if(m_targets.m_destX != 0 && m_targets.m_destY != 0  && m_targets.m_destZ != 0 )
                        {
                            if(_CalcDistance(m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ(),m_targets.m_destX,m_targets.m_destY,m_targets.m_destZ) > GetMaxRange(sSpellRange.LookupEntry(m_spellInfo->rangeIndex)))
                                castResult = 0x56;// Target out of Range
                        }

                        if(m_caster->m_silenced)
                            castResult = 0x5A;

                        castResult = CheckItems();
/*
    // Cheat Detection, crashes somehow need to fix it
    if(m_caster->GetTypeId() == TYPEID_PLAYER && m_triggeredByAffect == 0)
    {
        std::list<struct spells>::iterator i;
        for(i = ((Player*)m_caster)->getSpellList().begin();i != ((Player*)m_caster)->getSpellList().end();i++)
        {
            if(i->spellId == m_spellInfo->Id)
                break;
        }
        if(i != ((Player*)m_caster)->getSpellList().end())
{
castResult = 0x20;
FILE *pFile = fopen("spells.log", "a+");
fprintf(pFile,"Player: %u tried to use a not known spell: %u\n", m_caster->GetGUID(),m_spellInfo->Id);
fclose(pFile);
}
}
*/

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

                        // Check Reagents
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
                                if(itm->GetProto()->ItemId == m_spellInfo->Reagent[i])
                                    if(tmpReagentCount[i] > 0)
                                        if(itm->GetUInt32Value(ITEM_FIELD_STACK_COUNT) > tmpReagentCount[i])
                                            tmpReagentCount[i] = 0;
                                else
                                    tmpReagentCount[i] -= itm->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
                            }
                            if(tmpReagentCount[i] != 0)
                                return uint8(0x54);
                        }

                        // Check Totems
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

                        // increase/decrease Damage by Talents etc.

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
#endif                //_VERSION_1_7_0_
                            pTarget->GetSession()->SendPacket(&data);
                        }

                        MapManager::Instance().GetMap(Target->GetMapId())->Remove(pTarget, false);


                        // Build a NEW WORLD packet
                        data.Initialize(SMSG_NEW_WORLD);
                        data << TC->mapId << TC->x << TC->y << TC->z << (float)0.0f;
                        if(pTarget)
                        {
#ifdef _VERSION_1_7_0_
                            data << uint32(0) << uint32(0) << uint32(0);
#endif                //_VERSION_1_7_0_
                            pTarget->GetSession()->SendPacket(&data);
                        }

                        // TODO: clear attack list

                        pTarget->SetMapId(TC->mapId);
                        pTarget->Relocate(TC->x, TC->y, TC->z, 0);
			// enters the new world
                        MapManager::Instance().GetMap(Target->GetMapId())->Add(pTarget);
                    }
