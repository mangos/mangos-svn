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
#include "Spell.h"
#include "DynamicObject.h"
#include "SpellAuras.h"
#include "Group.h"
#include "UpdateData.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "RedZoneDistrict.h"
#include "CellImpl.h"
#include "Policies/SingletonImp.h"
#include "SharedDefines.h"
#include "Tools.h"

#define SPELL_CHANNEL_UPDATE_INTERVAL 1000

extern pEffect SpellEffects[TOTAL_SPELL_EFFECTS];

SpellCastTargets::SpellCastTargets()
{
    m_unitTarget = NULL;
    m_itemTarget = NULL;
    m_GOTarget   = NULL;
    m_srcX = m_srcY = m_srcZ = m_destX = m_destY = m_destZ = 0;
    m_strTarget = "";
}

SpellCastTargets::~SpellCastTargets()
{}

void SpellCastTargets::read ( WorldPacket * data,Unit *caster )
{

    *data >> m_targetMask;

    if(m_targetMask == TARGET_FLAG_SELF)
    {
        m_destX = caster->GetPositionX();
        m_destY = caster->GetPositionY();
        m_destZ = caster->GetPositionZ();
        m_unitTarget = caster;
        return;
    }

    if(m_targetMask & TARGET_FLAG_UNIT)
        m_unitTarget = ObjectAccessor::Instance().GetUnit(*caster, readGUID(data));

    if(m_targetMask & TARGET_FLAG_OBJECT)
        m_GOTarget = ObjectAccessor::Instance().GetGameObject(*caster, readGUID(data));

    if(m_targetMask & TARGET_FLAG_ITEM)
        m_itemTarget = ((Player*)caster)->GetItemByPos( ((Player*)caster)->GetPosByGuid(readGUID(data)));

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

    if(m_targetMask == TARGET_FLAG_SELF)
        *data << (m_unitTarget ? m_unitTarget->GetGUID(): (uint64)0);

    if(m_targetMask & TARGET_FLAG_UNIT)
        *data << (m_unitTarget ? m_unitTarget->GetGUID(): (uint64)0);

    if(m_targetMask & TARGET_FLAG_OBJECT)
        *data << (m_GOTarget ? m_GOTarget->GetGUID(): (uint64)0);

    if(m_targetMask & TARGET_FLAG_ITEM)
        *data << (m_itemTarget ? m_itemTarget->GetGUID(): (uint64)0);

    if(m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
        *data << m_srcX << m_srcY << m_srcZ;

    if(m_targetMask & TARGET_FLAG_DEST_LOCATION)
        *data << m_destX << m_destY << m_destZ;

    if(m_targetMask & TARGET_FLAG_STRING)
        *data << m_strTarget;

}

Spell::Spell( Unit* Caster, SpellEntry *info, bool triggered, Aura* Aur )
{
    ASSERT( Caster != NULL && info != NULL );

    SpellEntry *spellInfo;
    Player* p_caster;
    std::list<Playerspell*>::iterator itr;

    std::list<Playerspell*> player_spells;

    m_spellInfo = info;
    m_caster = Caster;

    m_spellState = SPELL_STATE_NULL;

    m_castPositionX = m_castPositionY = m_castPositionZ = 0;
    m_TriggerSpell = NULL;
    m_targetCount = 0;
    m_Istriggeredpell = triggered;
    //m_AreaAura = false;
    m_CastItem = NULL;

    unitTarget = NULL;
    itemTarget = NULL;
    gameObjTarget = NULL;

    m_triggeredByAura = Aur;
    m_autoRepeat = false;
    if(info->Id == 75)                                      //auto shot
        m_autoRepeat = true;

    casttime = GetCastTime(sCastTime.LookupEntry(m_spellInfo->CastingTimeIndex));

    if( Caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo )
    {
        p_caster = (Player*)m_caster;
        player_spells = p_caster->getSpellList();
        for (itr = player_spells.begin(); itr != player_spells.end(); ++itr)
        {
            if ((*itr)->spellId != m_spellInfo->Id)
            {
                spellInfo = sSpellStore.LookupEntry((*itr)->spellId);
                if(spellInfo && spellInfo->SpellIconID == m_spellInfo->SpellIconID && spellInfo->EffectMiscValue[0] ==10)
                {
                    casttime=casttime+(spellInfo->EffectBasePoints[0]+1);
                }
            }
        }
    }

    m_timer = casttime<0?0:casttime;

}

void Spell::FillTargetMap()
{

    std::list<Unit*> tmpUnitMap;
    std::list<Item*> tmpItemMap;
    std::list<GameObject*> tmpGOMap;

    for(uint32 i=0;i<3;i++)
    {

        SetTargetMap(i,m_spellInfo->EffectImplicitTargetA[i],tmpUnitMap,tmpItemMap,tmpGOMap);
        SetTargetMap(i,m_spellInfo->EffectImplicitTargetB[i],tmpUnitMap,tmpItemMap,tmpGOMap);

        if(!m_spellInfo->EffectImplicitTargetA[i] || m_spellInfo->EffectImplicitTargetB[i] )
        {
            // add where custom effects that need default target.
                                                            // Apply Aura
            if(m_spellInfo->Effect[i] == 27) tmpUnitMap.push_back(m_caster);
                                                            // Learn Spell
            else if(m_spellInfo->Effect[i] == 36) tmpUnitMap.push_back(m_targets.getUnitTarget());
                                                            // Learn Skill
            else if(m_spellInfo->Effect[i] == 44) tmpUnitMap.push_back(m_targets.getUnitTarget());
                                                            // Execute Skill
            else if(m_spellInfo->Effect[i] == 118) tmpUnitMap.push_back(m_caster);
                                                            // DisEnchant
            else if(m_spellInfo->Effect[i] == 99) tmpItemMap.push_back(itemTarget);
                                                            // EnchantPem
            else if(m_spellInfo->Effect[i] == 53) tmpItemMap.push_back(itemTarget);
                                                            // EnchantTmp
            else if(m_spellInfo->Effect[i] == 54) tmpItemMap.push_back(itemTarget);
                                                            // EnchantHeldItem
            else if(m_spellInfo->Effect[i] == 92) tmpItemMap.push_back(itemTarget);
        }

        m_targetUnits[i] = tmpUnitMap;
        m_targetItems[i] = tmpItemMap;
        m_targetGOs[i]   = tmpGOMap;

        tmpUnitMap.clear();
        tmpItemMap.clear();
        tmpGOMap.clear();
    }
}

void Spell::SetTargetMap(uint32 i,uint32 cur,std::list<Unit*> &TagUnitMap,std::list<Item*> &TagItemMap,std::list<GameObject*> &TagGOMap)
{
    float radius =  GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
    switch(cur)
    {
        case TARGET_SELF:
        case TARGET_DY_OBJ:                                 //add by vendy
        {
            TagUnitMap.push_back(m_caster);
        }break;
        case TARGET_PET:
        {
            Unit* tmpUnit = ObjectAccessor::Instance().GetUnit(*m_caster,m_caster->GetUInt32Value(UNIT_FIELD_PETNUMBER));
            TagUnitMap.push_back(tmpUnit);
        }break;
        case TARGET_S_E:
        {
            TagUnitMap.push_back(m_targets.getUnitTarget());
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

            MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i,PUSH_DEST_CENTER);
            TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
            CellLock<GridReadGuard> cell_lock(cell, p);
            cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
        }break;
        case TARGET_AC_P:
        {
            Group* pGroup = objmgr.GetGroupByLeader(((Player*)m_caster)->GetGroupLeader());
            if(pGroup)
                for(uint32 p=0;p<pGroup->GetMembersCount();p++)
            {
                Unit* Target = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(p));
                if(!Target || Target->GetGUID() == m_caster->GetGUID())
                    continue;
                if(m_caster->GetDistanceSq(Target) < radius * radius )
                    TagUnitMap.push_back(Target);
            }
            else
                TagUnitMap.push_back(m_caster);
        }break;
        case TARGET_S_F:
        case TARGET_S_F_2:
        {
            TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
        case TARGET_AC_E:
        {
            CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
            Cell cell = RedZone::GetZone(p);
            cell.data.Part.reserved = ALL_DISTRICT;
            cell.SetNoCreate();

            MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i,PUSH_SELF_CENTER);
            TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
            CellLock<GridReadGuard> cell_lock(cell, p);
            cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));

        }break;
        case TARGET_S_GO:
        {
            TagGOMap.push_back(m_targets.m_GOTarget);
        }break;
        case TARGET_INFRONT:
        {
            CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
            Cell cell = RedZone::GetZone(p);
            cell.data.Part.reserved = ALL_DISTRICT;
            cell.SetNoCreate();
            MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i,PUSH_IN_FRONT);

            TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
            CellLock<GridReadGuard> cell_lock(cell, p);

            cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
        }break;
        case TARGET_DUELVSPLAYER:
        {
            TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
        case TARGET_GOITEM:
        {
            if(m_targets.getUnitTarget())
                TagUnitMap.push_back(m_targets.getUnitTarget());
            if(m_targets.m_itemTarget)
                TagItemMap.push_back(m_targets.m_itemTarget);
        }break;
        case TARGET_AE_E_CHANNEL:
        {
            CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
            Cell cell = RedZone::GetZone(p);
            cell.data.Part.reserved = ALL_DISTRICT;
            cell.SetNoCreate();

            MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i,PUSH_DEST_CENTER);
            TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
            CellLock<GridReadGuard> cell_lock(cell, p);
            cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
        }break;
        case TARGET_MINION:
        {
            if(m_spellInfo->Effect[i] != 83) TagUnitMap.push_back(m_caster);
        }break;
        case TARGET_S_P:
        {
            TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
        case TARGET_SELF_FISHING:
        {
            TagUnitMap.push_back(m_caster);
        }break;
        case TARGET_CHAIN:
        {
            bool onlyParty = false;

            if(!m_targets.getUnitTarget())
                break;

            Group* pGroup = objmgr.GetGroupByLeader(((Player*)m_caster)->GetGroupLeader());
            for(uint32 p=0;p<pGroup->GetMembersCount();p++)
            {
                if(m_targets.getUnitTarget()->GetGUID() == pGroup->GetMemberGUID(p))
                {
                    onlyParty = true;
                    break;
                }
            }
            for(uint32 p=0;p<pGroup->GetMembersCount();p++)
            {
                Unit* Target = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(p));

                if(!Target || Target->GetGUID() == m_caster->GetGUID())
                    continue;
                if(m_caster->GetDistanceSq(Target) < radius * radius && Target->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) == m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE))
                    TagUnitMap.push_back(Target);
            }
        }break;
        case TARGET_AE_SELECTED:
        {
            CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
            Cell cell = RedZone::GetZone(p);
            cell.data.Part.reserved = ALL_DISTRICT;
            cell.SetNoCreate();
            MaNGOS::SpellNotifierPlayer notifier(*this, TagUnitMap, i);
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
    if(!unitTarget)
        unitTarget = m_targets.getUnitTarget();
    if(!itemTarget)
        itemTarget = m_targets.m_itemTarget;
    if(!gameObjTarget)
        gameObjTarget = m_targets.m_GOTarget;

    SendSpellStart();

    m_spellState = SPELL_STATE_PREPARING;

    m_castPositionX = m_caster->GetPositionX();
    m_castPositionY = m_caster->GetPositionY();
    m_castPositionZ = m_caster->GetPositionZ();

    result = CanCast();
    if(result != 0)
    {
        if(m_triggeredByAura)
        {
            SendChannelUpdate(0);
            m_triggeredByAura->SetAuraDuration(0);
        }
        finish();
    }

    if(m_Istriggeredpell)
        cast();
    else
        m_caster->castSpell( this );
}

void Spell::cancel()
{
    WorldPacket data;
    m_autoRepeat = false;
    if(m_spellState == SPELL_STATE_PREPARING)
    {
        SendInterrupted(0);
        SendCastResult(CAST_FAIL_INTERRUPTED);
    }
    else if(m_spellState == SPELL_STATE_CASTING)
    {
        m_caster->RemoveAurasDueToSpell(m_spellInfo->Id);
        SendChannelUpdate(0);
    }

    finish();
    m_caster->RemoveDynObject(m_spellInfo->Id);
    m_caster->RemoveGameObject(m_spellInfo->Id);
}

void Spell::cast()
{
    WorldPacket data;

    uint8 castResult = 0;
    if(m_caster->GetTypeId() != TYPEID_PLAYER && unitTarget)
        m_caster->SetInFront(unitTarget);
    castResult = CanCast();
    if(castResult == 0)
    {
        TakePower();
        FillTargetMap();
        SendCastResult(castResult);
        SendSpellGo();

        if(m_spellInfo->ChannelInterruptFlags != 0)
        {
            m_spellState = SPELL_STATE_CASTING;
            SendChannelStart(GetDuration(m_spellInfo));
        }

        std::list<Unit*>::iterator iunit;
        std::list<Item*>::iterator iitem;
        std::list<GameObject*>::iterator igo;

        bool needspelllog = false;

        for(uint32 j = 0;j<3;j++)
        {
            if(m_spellInfo->Effect[j] != 2)                 // Dont do spell log, if is school damage spell
                needspelllog = true;
            else
                needspelllog = false;

            for(iunit= m_targetUnits[j].begin();iunit != m_targetUnits[j].end();iunit++)
                HandleEffects((*iunit),NULL,NULL,j);
            for(iitem= m_targetItems[j].begin();iitem != m_targetItems[j].end();iitem++)
                HandleEffects(NULL,(*iitem),NULL,j);
            for(igo= m_targetGOs[j].begin();igo != m_targetGOs[j].end();igo++)
                HandleEffects(NULL,NULL,(*igo),j);
        }

        if(needspelllog) SendLogExecute();

        for(iunit= UniqueTargets.begin();iunit != UniqueTargets.end();iunit++)
        {
            if((*iunit)->m_ReflectSpellSchool) reflect(*iunit);
            //HandleAddAura((*iunit));
        }
    }

    if(m_spellState != SPELL_STATE_CASTING)
        finish();

    //if(castResult == 0)
    //{
    //    TriggerSpell();

    //}

}

void Spell::update(uint32 difftime)
{
    if(unitTarget && !unitTarget->isAlive())
    {
        if(m_autoRepeat)
        {
            m_autoRepeat = false;
            m_spellState = SPELL_STATE_FINISHED;
            return;
        }
    }

    if( ( m_timer != 0 ) && (m_caster->GetTypeId() == TYPEID_PLAYER) &&
        (m_castPositionX != m_caster->GetPositionX() ||
        m_castPositionY != m_caster->GetPositionY() ||
        m_castPositionZ != m_caster->GetPositionZ() ) )
    {
        cancel();
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

    if(!m_caster) return;

    m_spellState = SPELL_STATE_FINISHED;
    m_caster->m_meleeSpell = false;
    m_caster->m_canMove = true;

    WorldPacket data;

    /*std::list<DynamicObject*>::iterator i;
    for(i = m_dynObjToDel.begin() ; i != m_dynObjToDel.end() ; i++)
    {
        data.Initialize(SMSG_GAMEOBJECT_DESPAWN_ANIM);
        data << (*i)->GetGUID();
        m_caster->SendMessageToSet(&data, true);

        data.Initialize(SMSG_DESTROY_OBJECT);
        data << (*i)->GetGUID();
        m_caster->SendMessageToSet(&data, true);
        MapManager::Instance().GetMap((*i)->GetMapId())->Remove((*i), true);
        m_AreaAura = false;
    }
    m_dynObjToDel.clear();

    std::list<GameObject*>::iterator k;
    for(k = m_ObjToDel.begin() ; k != m_ObjToDel.end() ; k++)
    {
        data.Initialize(SMSG_GAMEOBJECT_DESPAWN_ANIM);
        data << (*k)->GetGUID();
        m_caster->SendMessageToSet(&data, true);

        data.Initialize(SMSG_DESTROY_OBJECT);
        data << (*k)->GetGUID();
        m_caster->SendMessageToSet(&data, true);
        MapManager::Instance().GetMap((*k)->GetMapId())->Remove((*k), true);
    }

    m_ObjToDel.clear();*/

    uint8 powerType = m_caster->getPowerType();
    if(m_TriggerSpell)
        TriggerSpell();

    if(!m_CastItem || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    ItemPrototype *proto = m_CastItem->GetProto();
    uint32 ItemCount = m_CastItem->GetCount();
    uint32 ItemClass = proto->Class;
    uint32 ItemId = proto->ItemId;

    if (ItemClass == ITEM_CLASS_CONSUMABLE)
    {
        ((Player*)m_caster)->DestroyItemCount(proto->ItemId, 1);
        if(ItemCount<=1)
        {
            //pItem->DeleteFromDB();
            //delete m_CastItem;
            m_CastItem = NULL;
        }
    }
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

    ((Player*)m_caster)->GetSession()->SendPacket(&data);
}

void Spell::SendSpellStart()
{

    WorldPacket data;
    uint16 cast_flags;

    cast_flags = 2;
    Unit * target;
    if(!unitTarget)
        target = m_caster;
    else
        target = unitTarget;

    data.Initialize(SMSG_SPELL_START);
    data << uint8(0xFF) << target->GetGUID() << uint8(0xFF) << m_caster->GetGUID();
    data << m_spellInfo->Id;
    data << cast_flags;
    data << uint32(m_timer);

    m_targets.write( &data );
    ((Player*)m_caster)->SendMessageToSet(&data, true);

}

void Spell::SendSpellGo()
{

    WorldPacket data;

    Unit * target;
    if(!unitTarget)
        target = m_caster;
    else
        target = unitTarget;

    data.Initialize(SMSG_SPELL_GO);
    data << uint8(0xFF)<< target->GetGUID() << uint8(0xFF) << m_caster->GetGUID();
    data << m_spellInfo->Id;

    data << uint16(0x0500);
    writeSpellGoTargets(&data);

    data << (uint8)0;

    m_targets.write( &data );
    m_caster->SendMessageToSet(&data, true);

}

void Spell::writeSpellGoTargets( WorldPacket * data )
{
    bool add = true;

    std::list<Unit*>::iterator i,j;
    std::list<GameObject*>::iterator m,n;

    for(int k=0;k<3;k++)
    {
        for ( i = m_targetUnits[k].begin(); i != m_targetUnits[k].end(); i++ )
        {
            for(j = UniqueTargets.begin(); j != UniqueTargets.end(); j++ )
            {
                if((*j) == (*i))
                {
                    add = false;
                    break;
                }
            }
            if(*i && add)
                UniqueTargets.push_back(*i);
            add = true;
        }
        for ( m = m_targetGOs[k].begin(); m != m_targetGOs[k].end(); m++ )
        {
            for(n = UniqueGOsTargets.begin(); n != UniqueGOsTargets.end(); n++ )
            {
                if((*n) == (*m))
                {
                    add = false;
                    break;
                }
            }
            if(*m && add)
                UniqueGOsTargets.push_back(*m);
            add = true;
        }
    }

    m_targetCount = UniqueTargets.size() + UniqueGOsTargets.size();
    *data << m_targetCount;

    for ( std::list<Unit*>::iterator ui = UniqueTargets.begin(); ui != UniqueTargets.end(); ui++ )
        *data << (*ui)->GetGUID();

    for ( std::list<GameObject*>::iterator uj = UniqueGOsTargets.begin(); uj != UniqueGOsTargets.end(); uj++ )
        *data << (*uj)->GetGUID();

}

void Spell::SendLogExecute()
{
    Unit * target;
    if(!unitTarget)
        target = m_caster;
    else
        target = unitTarget;
    WorldPacket data;
    data.Initialize(SMSG_SPELLLOGEXECUTE);

    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        data << uint8(0xFF) << m_caster->GetGUID();
    else
        data << target->GetGUID();

    data << m_spellInfo->Id;
    data << uint32(1);
    data << m_spellInfo->SpellVisual;
    data << uint32(1);

    if(m_targets.getUnitTarget())
        data << m_targets.getUnitTarget()->GetGUID();
    else if(m_targets.m_itemTarget)
        data << m_targets.m_itemTarget->GetGUID();
    else if(m_targets.m_GOTarget)
        data << m_targets.m_GOTarget->GetGUID();

    m_caster->SendMessageToSet(&data,true);
};

void Spell::SendInterrupted(uint8 result)
{
    WorldPacket data;

    data.Initialize(SMSG_SPELL_FAILURE);
    data << uint8(0xFF) << m_caster->GetGUID();
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
    Unit* target = ObjectAccessor::Instance().GetCreature(*m_caster, ((Player *)m_caster)->GetSelection());

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {

        WorldPacket data;
        data.Initialize( MSG_CHANNEL_START );
        data << m_spellInfo->Id;
        data << duration;

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
    data << uint8(0xFF) << m_caster->GetGUID();
    data << uint32(0) << uint8(0);

    target->GetSession()->SendPacket(&data);
}

void Spell::SendHealSpellOnPlayer(Player* target, uint32 SpellID, uint32 Damage)
{
    WorldPacket data;
    data.Initialize(SMSG_HEALSPELL_ON_PLAYER_OBSOLETE);
    data << uint8(0xFF) << target->GetGUID();
    data << uint8(0xFF) << m_caster->GetGUID();
    data << SpellID;
    data << Damage;
    data << uint8(0);
    target->GetSession()->SendPacket(&data);
}

void Spell::SendPlaySpellVisual(uint32 SpellID)
{
    WorldPacket data;
    data.Initialize(SMSG_PLAY_SPELL_VISUAL);
    data << m_caster->GetGUID();
    data << SpellID;
    ((Player*)m_caster)->GetSession()->SendPacket(&data);
}

void Spell::TakePower()
{
    uint16 powerField;

    uint8 powerType = m_caster->getPowerType();
    if(powerType == 0)
        powerField = UNIT_FIELD_POWER1;
    else if(powerType == 1)
        powerField = UNIT_FIELD_POWER2;
    else if(powerType == 3)
        powerField = UNIT_FIELD_POWER4;
    else
    {
        powerField = UNIT_FIELD_POWER1;
        sLog.outError("SPELL: unknown power type %i spell id %u\n",(int)powerType, m_spellInfo->Id);
    }

    uint32 currentPower = m_caster->GetUInt32Value(powerField);

    if(currentPower < m_spellInfo->manaCost)
        m_caster->SetUInt32Value(powerField, 0);
    else
    {
        m_caster->SetUInt32Value(powerField, currentPower - m_spellInfo->manaCost);
        if (powerField == UNIT_FIELD_POWER1)
        {
            // Set the five second timer
            if (m_caster->GetTypeId() == TYPEID_PLAYER)
            {
                ((Player *)m_caster)->SetLastManaUse((uint32)getMSTime());
            }
        }
    }
}

void Spell::HandleEffects(Unit *pUnitTarget,Item *pItemTarget,GameObject *pGOTarget,uint32 i)
{
    uint8 castResult = 0;
    unitTarget = pUnitTarget;
    itemTarget = pItemTarget;
    gameObjTarget = pGOTarget;

    damage = CalculateDamage((uint8)i);
    uint8 eff = m_spellInfo->Effect[i];

    sLog.outDebug( "WORLD: Spell FX id is %u", eff);
    if(unitTarget && (unitTarget->m_immuneToEffect & eff))
    {
        castResult = CAST_FAIL_IMMUNE;
        SendCastResult(castResult);
        return;
    }

    if(eff<TOTAL_SPELL_EFFECTS)
        (*this.*SpellEffects[eff])(i);
    /*
    else
    {
        if (m_CastItem)
            EffectEnchantItemTmp(i);
        else
        {
            sLog.outError("SPELL: unknown effect %u spell id %u\n",
                eff, m_spellInfo->Id);
        }
    }
    */
}

/*void Spell::HandleAddAura(Unit* Target)
{
    if(!Target) return;

    if(Target->tmpAura != 0)
    {
        Target->AddAura(Target->tmpAura);
        Target->tmpAura = 0;
    }
}
*/

void Spell::TriggerSpell()
{
    if(!m_TriggerSpell) return;

    Spell spell(m_caster, m_TriggerSpell, true, 0);
    SpellCastTargets targets;
    targets.setUnitTarget( m_targets.getUnitTarget());
    spell.prepare(&targets);

}

uint8 Spell::CanCast()
{
    uint8 castResult = 0;
    uint32 m_school = 0;

    if (m_CastItem || itemTarget)
    {
        castResult = CheckItems();

        if(castResult != 0)
            SendCastResult(castResult);

        return castResult;
    }
    Unit *target = NULL;
    target = m_targets.getUnitTarget();
    SpellRange* srange = sSpellRange.LookupEntry(m_spellInfo->rangeIndex);
    float range = GetMaxRange(srange);
    if(target)
    {
        //If m_immuneToDispel type contain this spell type, IMMUNE spell.
        if(target->m_immuneToDispel & m_spellInfo->Dispel)
            castResult = CAST_FAIL_IMMUNE;
        /*
        //If m_immuneToMechanic type contain this Mechanic type IMMUNE spell, or it should fit to aura?
        if(target->m_immuneToMechanic & m_spellInfo->Mechanic)
            castResult = CAST_FAIL_IMMUNE;
        //If m_immuneToDamage type contain magic, IMMUNE spell.
        if(unitTarget->m_immuneToDmg & IMMUNE_DAMAGE_MAGIC)
            castResult = CAST_FAIL_IMMUNE;
        //If m_immuneToSchool type contain this school type, IMMUNE spell.
        switch(m_spellInfo->School)
        {
        case 1:
            m_school = IMMUNE_SCHOOL_HOLY;
            break;
        case 2:
            m_school = IMMUNE_SCHOOL_FIRE;
            break;
        case 3:
            m_school = IMMUNE_SCHOOL_NATURE;
            break;
        case 4:
            m_school = IMMUNE_SCHOOL_FROST;
            break;
        case 5:
            m_school = IMMUNE_SCHOOL_SHADOW;
            break;
        case 6:
            m_school = IMMUNE_SCHOOL_ARCANE;
            break;
        default:break;
        }
        if(unitTarget->m_immuneToSchool & m_school)
            castResult = CAST_FAIL_IMMUNE;
        */

        if(m_caster->hasUnitState(UNIT_STAT_CONFUSED))
            castResult = CAST_FAIL_CANT_DO_WHILE_CONFUSED;

        if(!m_caster->isInFront( target, range ) && m_caster->GetGUID() != target->GetGUID())
            castResult = CAST_FAIL_TARGET_NEED_TO_BE_INFRONT;
        if(m_caster->GetDistanceSq(target) > range * range && m_caster->GetTypeId() != TYPEID_PLAYER)
            castResult = CAST_FAIL_OUT_OF_RANGE;            //0x56;
    }

    if(m_caster->hasUnitState(UNIT_STAT_STUNDED))
        castResult = CAST_FAIL_CANT_DO_WHILE_STUNNED;
    if(m_caster->m_silenced)
        castResult = CAST_FAIL_SILENCED;                    //0x5A;
    if( castResult != 0 )
    {
        SendCastResult(castResult);
        return castResult;
    }

    castResult = CheckItems();

    if(castResult != 0)
        SendCastResult(castResult);

    return castResult;
}

uint8 Spell::CheckItems()
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return uint8(0);

    uint32 itemid, itemcount;
    Player* p_caster = (Player*)m_caster;
    if (itemTarget)
    {
        if(p_caster->GetItemCount(itemTarget->GetEntry()) < 1)
            return (uint8)CAST_FAIL_ITEM_NOT_READY;
        else return uint8(0);
    }
    if(m_CastItem)
    {
        itemid = m_CastItem->GetEntry();
        if(p_caster->GetItemCount(itemid) < 1)
            return (uint8)CAST_FAIL_ITEM_NOT_READY;
        else return uint8(0);
    }

    for(uint32 i=0;i<8;i++)
    {
        if((itemid = m_spellInfo->Reagent[i]) == 0)
            continue;
        itemcount = m_spellInfo->ReagentCount[i];
        if(p_caster->GetItemCount(itemid) < itemcount)
            return (uint8)CAST_FAIL_ITEM_NOT_READY;         //0x54
    }

    uint32 totems = 2;
    for(int i=0;i<2;i++)
    {
        if(m_spellInfo->Totem[i] != 0)
        {
            if(p_caster->GetItemCount(m_spellInfo->Totem[i]) >= 1)
            {
                totems -= 1;
                continue;
            }
        }else
        totems -= 1;
    }
    if(totems != 0)
        return uint8(0x70);
    return uint8(0);
}

uint32 Spell::CalculateDamage(uint8 i)
{
    uint32 value = 0;
    float basePointsPerLevel = m_spellInfo->EffectRealPointsPerLevel[i];
    float randomPointsPerLevel = m_spellInfo->EffectDicePerLevel[i];
    uint32 basePoints = uint32(m_spellInfo->EffectBasePoints[i]+m_caster->getLevel()*basePointsPerLevel);
    uint32 randomPoints = uint32(m_spellInfo->EffectDieSides[i]+m_caster->getLevel()*randomPointsPerLevel);
    float comboDamage = m_spellInfo->EffectPointsPerComboPoint[i];
    uint8 comboPoints=0;
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        comboPoints = (uint8)((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & 0xFF00) >> 8);

    value += m_spellInfo->EffectBaseDice[i];
    if(randomPoints <= 1)
        value = basePoints+1;
    else
        value = basePoints+rand()%randomPoints;

    if(comboDamage > 0)
    {
        value += (uint32)(comboDamage * comboPoints);
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            m_caster->SetUInt32Value(PLAYER_FIELD_BYTES,((m_caster->GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (0x00 << 8)));
    }

    return value;
}

void Spell::HandleTeleport(uint32 id, Unit* Target)
{

    if(!Target) return;

    TeleportCoords* TC = new TeleportCoords();

    if(m_spellInfo->Id == 8690 )
    {
        Field *fields;
        QueryResult *result4 = sDatabase.PQuery("SELECT `map`,`zone`,`position_x`,`position_y`,`position_z` FROM `character_homebind` WHERE `guid` = '%u';", m_caster->GetGUIDLow());
        fields = result4->Fetch();
        TC->mapId = fields[0].GetUInt32();
        TC->x = fields[2].GetFloat();
        TC->y = fields[3].GetFloat();
        TC->z = fields[4].GetFloat();
        delete result4;
    }
    else
    {
        TC = objmgr.GetTeleportCoords(id);
        if(!TC)
        {
            sLog.outString( "SPELL: unknown Teleport Coords ID %i\n", id );
            return;
        }
    }

    ((Player*)Target)->SendNewWorld(TC->mapId,TC->x,TC->y,TC->z,0.0f);

}

void Spell::Delayed(int32 delaytime)
{
    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER) return;

    m_timer += delaytime;

    if(m_timer > casttime)
        m_timer = casttime;

    WorldPacket data;

    data.Initialize(SMSG_SPELL_DELAYED);
    data << m_caster->GetGUID();
    data << uint32(delaytime);

    ((Player*)m_caster)->GetSession()->SendPacket(&data);
}

void Spell::reflect(Unit *refunit)
{
    SpellEntry *refspell = NULL;

    // if the spell to reflect is a reflect spell, do nothing.
    for(int i=0; i<3; i++)
        if(m_spellInfo->Effect[i] == 6 && m_spellInfo->EffectApplyAuraName[i] == 74)
            return;

    switch(refunit->m_ReflectSpellSchool)
    {
        case 126:                                           // All
            refspell = m_spellInfo;
            break;
        case 4:                                             // Fire
            refspell = (m_spellInfo->School == 2 ? m_spellInfo : NULL);
            break;
        case 16:                                            // Frost
            refspell = (m_spellInfo->School == 4 ? m_spellInfo : NULL);
            break;
        case 32:                                            // Shadow
            refspell = (m_spellInfo->School == 5 ? m_spellInfo : NULL);
            break;
        case 64:                                            // Arcane
            refspell = (m_spellInfo->School == 6 ? m_spellInfo : NULL);
            break;
    }

    if(!refspell || m_caster == refunit) return;

    Spell *spell = new Spell(refunit, refspell, true, 0);

    SpellCastTargets targets;
    targets.setUnitTarget( m_caster );
    spell->prepare(&targets);

}
