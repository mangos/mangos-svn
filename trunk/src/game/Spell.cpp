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
#include "GridNotifiers.h"
#include "Opcodes.h"
#include "Log.h"
#include "UpdateMask.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Pet.h"
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
    m_targetMask = 0;
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

    if((m_targetMask & TARGET_FLAG_ITEM) && caster->GetTypeId() == TYPEID_PLAYER)
    {
        uint64 _guid = readGUID(data);
        m_itemTarget = ((Player*)caster)->GetItemByPos( ((Player*)caster)->GetPosByGuid(_guid));
        if (!m_itemTarget)
        {
            Player* pTrader = ((Player*)caster)->GetTrader();
            if(pTrader)
                m_itemTarget = pTrader->GetItemByPos(pTrader->GetPosByGuid(_guid));
        }
    }

    if(m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
        *data >> m_srcX >> m_srcY >> m_srcZ;

    if(m_targetMask & TARGET_FLAG_DEST_LOCATION)
        *data >> m_destX >> m_destY >> m_destZ;

    if(m_targetMask & TARGET_FLAG_STRING)
        *data >> m_strTarget;
    if(m_targetMask & 0x8000)
    {
        //0x8000 TARGET_CORPSE NEED TO ADD
    }
}

void SpellCastTargets::write ( WorldPacket * data, bool forceAppend)
{
    uint32 len = data->size();

    // dont append targets when spell's for your own..
    /*if(m_targetMask == TARGET_FLAG_SELF)
     *data << (m_unitTarget ? m_unitTarget->GetGUID(): (uint64)0);*/

    if(m_targetMask & TARGET_FLAG_UNIT)
        if(m_unitTarget)
            data->append(m_unitTarget->GetPackGUID());
    else
        *data << (uint8)0;

    if(m_targetMask & TARGET_FLAG_OBJECT)
        if(m_GOTarget)
            data->append(m_GOTarget->GetPackGUID());
    else
        *data << (uint8)0;

    if(m_targetMask & TARGET_FLAG_ITEM)
        if(m_itemTarget)
            data->append(m_itemTarget->GetPackGUID());
    else
        *data << (uint8)0;

    if(m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
        *data << m_srcX << m_srcY << m_srcZ;

    if(m_targetMask & TARGET_FLAG_DEST_LOCATION)
        *data << m_destX << m_destY << m_destZ;

    if(m_targetMask & TARGET_FLAG_STRING)
        *data << m_strTarget;
    if(m_targetMask & 0x8000)
    {
        //0x8000 TARGET_CORPSE NEED TO ADD
    }

    if(forceAppend && data->size() == len)
        *data << (uint8)0;
}

Spell::Spell( Unit* Caster, SpellEntry *info, bool triggered, Aura* Aur )
{
    ASSERT( Caster != NULL && info != NULL );

    //SpellEntry *spellInfo;
    Player* p_caster;

    m_spellInfo = info;
    m_caster = Caster;

    m_spellState = SPELL_STATE_NULL;

    m_castPositionX = m_castPositionY = m_castPositionZ = 0;
    m_TriggerSpell = NULL;
    m_targetCount = 0;
    m_IsTriggeredSpell = triggered;
    //m_AreaAura = false;
    m_CastItem = NULL;

    unitTarget = NULL;
    itemTarget = NULL;
    gameObjTarget = NULL;

    m_triggeredByAura = Aur;
    m_autoRepeat = false;
    if( m_spellInfo->AttributesEx2 == 0x000020 )            //Auto Shot & Shoot
        m_autoRepeat = true;

    casttime = GetCastTime(sCastTimesStore.LookupEntry(m_spellInfo->CastingTimeIndex));

    if( m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo )
    {
        p_caster = ((Player*)m_caster);
        ((Player*)m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_CASTING_TIME, casttime);
        casttime = int32(casttime/(100+p_caster->m_modCastSpeedPct)*100);
    }

    m_timer = casttime<0?0:casttime;
    m_delayedTime = 0;

    for(int i=0;i<3;i++)
        m_needAliveTarget[i] = false;

    m_meleeSpell = false;

    m_rangedShoot = ((m_spellInfo->Attributes & 18) == 18);
    if( m_spellInfo->StartRecoveryTime == 0 && !m_autoRepeat && !m_rangedShoot )
    {
        for (int i = 0; i < 3; i++)
        {
            if (m_spellInfo->Effect[i]==SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL ||
                m_spellInfo->Effect[i]==SPELL_EFFECT_WEAPON_DAMAGE)
            {
                m_meleeSpell = true;
                break;
            }
        }
    }
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

        if(!m_spellInfo->EffectImplicitTargetA[i] && !m_spellInfo->EffectImplicitTargetB[i] )
        {
            // add here custom effects that need default target.
            switch(m_spellInfo->Effect[i])
            {
                //case SPELL_EFFECT_PERSISTENT_AREA_AURA:
                case SPELL_EFFECT_RESURRECT:
                case SPELL_EFFECT_LEARN_SPELL:
                case SPELL_EFFECT_SKILL_STEP:
                case SPELL_EFFECT_SELF_RESURRECT:
                case SPELL_EFFECT_RESURRECT_NEW:
                case SPELL_EFFECT_PROFICIENCY:
                case SPELL_EFFECT_PARRY:
                case SPELL_EFFECT_DUMMY:
                    if(m_targets.getUnitTarget())
                        tmpUnitMap.push_back(m_targets.getUnitTarget());
                    break;
                case SPELL_EFFECT_SKILL:
                case SPELL_EFFECT_FEED_PET:
                case SPELL_EFFECT_SUMMON_CHANGE_ITEM:
                    tmpUnitMap.push_back(m_caster);
                    break;
                case SPELL_EFFECT_LEARN_PET_SPELL:
                    if(Pet* pet = m_caster->GetPet())
                        tmpUnitMap.push_back(pet);
                    break;
                case SPELL_EFFECT_DISENCHANT:
                case SPELL_EFFECT_ENCHANT_ITEM:
                case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
                case SPELL_EFFECT_ENCHANT_HELD_ITEM:
                    tmpItemMap.push_back(itemTarget);
                    break;
                case SPELL_EFFECT_APPLY_AREA_AURA:
                    if(m_spellInfo->Attributes == 0x9050000)// AreaAura
                        SetTargetMap(i,TARGET_AREAEFFECT_PARTY,tmpUnitMap,tmpItemMap,tmpGOMap);
                    break;
                default:
                    break;
            }
        }
        if(IsChanneledSpell() && !tmpUnitMap.empty())
            m_needAliveTarget[i] = true;

        if(m_caster->GetTypeId() == TYPEID_PLAYER && (!m_caster->IsPvP() || ((Player*)m_caster)->pvpInfo.endTimer != 0))
        {
            Player *me = (Player*)m_caster;
            for (std::list<Unit*>::const_iterator itr = tmpUnitMap.begin(); itr != tmpUnitMap.end(); itr++)
            {
                Unit *owner = (*itr)->GetOwner();
                Unit *u = owner ? owner : (*itr);
                if(u->IsPvP() && (!me->duel || me->duel->opponent != u))
                    me->UpdatePvP(true);
            }
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
    float radius =  GetRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
    switch(cur)
    {
        case TARGET_TOTEM_EARTH:
        case TARGET_TOTEM_WATER:
        case TARGET_TOTEM_AIR:
        case TARGET_TOTEM_FIRE:
        case TARGET_SELF:
        case TARGET_DYNAMIC_OBJECT:
        {
            TagUnitMap.push_back(m_caster);
        }break;
        case TARGET_PET:
        {
            Pet* tmpUnit = m_caster->GetPet();
            if (!tmpUnit) break;
            TagUnitMap.push_back(tmpUnit);
        }break;
        case TARGET_SINGLE_ENEMY:
        {
            if(m_targets.getUnitTarget())
                TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
        case TARGET_ALL_ENEMY_IN_AREA:
        {
        }break;
        case TARGET_ALL_ENEMY_IN_AREA_INSTANT:
        {
            // targets the ground, not the units in the area
            if (m_spellInfo->Effect[i]!=SPELL_EFFECT_PERSISTENT_AREA_AURA)
            {
                CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
                Cell cell = RedZone::GetZone(p);
                cell.data.Part.reserved = ALL_DISTRICT;
                cell.SetNoCreate();

                MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i, PUSH_DEST_CENTER);

                TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, ContainerMapList<Player> > player_notifier(notifier);
                TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);

                CellLock<GridReadGuard> cell_lock(cell, p);
                cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
                cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
            }
        }break;
        case TARGET_ALL_PARTY_AROUND_CASTER:
        {
            Group* pGroup = m_caster->GetTypeId() == TYPEID_PLAYER ? ((Player*)m_caster)->groupInfo.group : NULL;
            if(pGroup)
            {
                for(uint32 p=0;p<pGroup->GetMembersCount();p++)
                {
                    if(!pGroup->SameSubGroup(m_caster->GetGUID(), pGroup->GetMemberGUID(p)))
                        continue;

                    Unit* Target = objmgr.GetPlayer(pGroup->GetMemberGUID(p));
                    if(!Target)
                        continue;
                    if(m_caster->IsWithinDist(Target, radius))
                        TagUnitMap.push_back(Target);
                }
            }
            else
                TagUnitMap.push_back(m_caster);
        }break;
        case TARGET_SINGLE_FRIEND:
        case TARGET_SINGLE_FRIEND_2:
        {
            if(m_targets.getUnitTarget())
                TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
        case TARGET_ALL_ENEMIES_AROUND_CASTER:
        {
            CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
            Cell cell = RedZone::GetZone(p);
            cell.data.Part.reserved = ALL_DISTRICT;
            cell.SetNoCreate();

            MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i, PUSH_SELF_CENTER);

            TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, ContainerMapList<Player> > player_notifier(notifier);
            TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);

            CellLock<GridReadGuard> cell_lock(cell, p);
            cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
            cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
        }break;
        case TARGET_GAMEOBJECT:
        {
            if(m_targets.m_GOTarget)
                TagGOMap.push_back(m_targets.m_GOTarget);
        }break;
        case TARGET_IN_FRONT_OF_CASTER:
        {
            CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
            Cell cell = RedZone::GetZone(p);
            cell.data.Part.reserved = ALL_DISTRICT;
            cell.SetNoCreate();

            MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i, PUSH_IN_FRONT);

            TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, ContainerMapList<Player> > player_notifier(notifier);
            TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);

            CellLock<GridReadGuard> cell_lock(cell, p);
            cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
            cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
        }break;
        case TARGET_DUELVSPLAYER:
        {
            if(m_targets.getUnitTarget())
                TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
        case TARGET_GAMEOBJECT_ITEM:
        {
            if(m_targets.getUnitTarget())
                TagUnitMap.push_back(m_targets.getUnitTarget());
            if(m_targets.m_itemTarget)
                TagItemMap.push_back(m_targets.m_itemTarget);
        }break;
        case TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
        {
            // targets the ground, not the units in the area
            if (m_spellInfo->Effect[i]!=SPELL_EFFECT_PERSISTENT_AREA_AURA)
            {
                CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
                Cell cell = RedZone::GetZone(p);
                cell.data.Part.reserved = ALL_DISTRICT;
                cell.SetNoCreate();

                MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i, PUSH_DEST_CENTER);

                TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, ContainerMapList<Player> > player_notifier(notifier);
                TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);

                CellLock<GridReadGuard> cell_lock(cell, p);
                cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
                cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
            }
        }break;
        case TARGET_MINION:
        {
            if(m_spellInfo->Effect[i] != 83) TagUnitMap.push_back(m_caster);
        }break;
        case TARGET_SINGLE_PARTY:
        {
            if(m_targets.getUnitTarget())
                TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
        case TARGET_AREAEFFECT_PARTY:
        {
            Player* targetPlayer = m_targets.getUnitTarget() && m_targets.getUnitTarget()->GetTypeId() == TYPEID_PLAYER
                ? (Player*)m_targets.getUnitTarget() : NULL;

            Group* pGroup = targetPlayer ? targetPlayer->groupInfo.group : NULL;
            if(pGroup)
            {
                for(uint32 p=0;p<pGroup->GetMembersCount();p++)
                {
                    if(!pGroup->SameSubGroup(m_caster->GetGUID(), pGroup->GetMemberGUID(p)))
                        continue;

                    Unit* Target = objmgr.GetPlayer(pGroup->GetMemberGUID(p));
                    if(m_targets.getUnitTarget() && Target && m_targets.getUnitTarget()->IsWithinDist(Target, radius) )
                        TagUnitMap.push_back(Target);
                }
            }
            else if(m_targets.getUnitTarget())
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

            Group* pGroup = m_caster->GetTypeId() == TYPEID_PLAYER ? ((Player*)m_caster)->groupInfo.group : NULL;
            if(pGroup)
            {
                for(uint32 p=0;p<pGroup->GetMembersCount();p++)
                {
                    if(!pGroup->SameSubGroup(m_caster->GetGUID(), pGroup->GetMemberGUID(p)))
                        continue;

                    if(m_targets.getUnitTarget() && m_targets.getUnitTarget()->GetGUID() == pGroup->GetMemberGUID(p))
                    {
                        onlyParty = true;
                        break;
                    }
                }
                for(uint32 p=0;p<pGroup->GetMembersCount();p++)
                {
                    if(!pGroup->SameSubGroup(m_caster->GetGUID(), pGroup->GetMemberGUID(p)))
                        continue;

                    Unit* Target = objmgr.GetPlayer(pGroup->GetMemberGUID(p));

                    if(!Target || Target->GetGUID() == m_caster->GetGUID())
                        continue;
                    if(Target->getFaction() == m_caster->getFaction() && m_caster->IsWithinDist(Target, radius))
                        TagUnitMap.push_back(Target);
                }
            }
            else if(m_targets.getUnitTarget())
                TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
        case TARGET_CURRENT_SELECTED_ENEMY:
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
        case TARGET_AREAEFFECT_PARTY_AND_CLASS:
        {
            Player* targetPlayer = m_targets.getUnitTarget() && m_targets.getUnitTarget()->GetTypeId() == TYPEID_PLAYER
                ? (Player*)m_targets.getUnitTarget() : NULL;

            Group* pGroup = targetPlayer ? targetPlayer->groupInfo.group : NULL;
            if(pGroup)
            {
                for(uint32 p=0;p<pGroup->GetMembersCount();p++)
                {
                    if(!pGroup->SameSubGroup(m_caster->GetGUID(), pGroup->GetMemberGUID(p)))
                        continue;

                    Unit* Target = objmgr.GetPlayer(pGroup->GetMemberGUID(p));
                    if(Target && targetPlayer->IsWithinDist(Target, radius) &&
                        targetPlayer->getClass() == Target->getClass())
                        TagUnitMap.push_back(Target);
                }
            }
            else if(m_targets.getUnitTarget())
                TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
    }

    if (m_spellInfo->MaxAffectedTargets != 0 && TagUnitMap.size() > m_spellInfo->MaxAffectedTargets)
    {
        // make sure one unit is always removed per iteration
        uint32 removed_utarget = 0;
        for (std::list<Unit*>::iterator itr = TagUnitMap.begin(); itr != TagUnitMap.end(); ++itr)
        {
            if (!*itr) continue;
            if ((*itr) == m_targets.getUnitTarget())
            {
                TagUnitMap.erase(itr);
                removed_utarget = 1;
                break;
            }
        }
        // remove random units from the map
        while (TagUnitMap.size() > m_spellInfo->MaxAffectedTargets - removed_utarget)
        {
            uint32 poz = urand(0, TagUnitMap.size()-1);
            for (std::list<Unit*>::iterator itr = TagUnitMap.begin(); itr != TagUnitMap.end(); ++itr, --poz)
            {
                if (!*itr) continue;
                if (!poz)
                {
                    TagUnitMap.erase(itr);
                    break;
                }
            }
        }
        // the player's target will always be added to the map
        if (removed_utarget && m_targets.getUnitTarget())
            TagUnitMap.push_back(m_targets.getUnitTarget());
    }
}

void Spell::prepare(SpellCastTargets * targets)
{
    m_targets = *targets;
    if(!unitTarget)
        unitTarget = m_targets.getUnitTarget();
    if(!itemTarget)
        itemTarget = m_targets.m_itemTarget;
    if(!gameObjTarget)
        gameObjTarget = m_targets.m_GOTarget;

    m_spellState = SPELL_STATE_PREPARING;

    m_castPositionX = m_caster->GetPositionX();
    m_castPositionY = m_caster->GetPositionY();
    m_castPositionZ = m_caster->GetPositionZ();
    m_castOrientation = m_caster->GetOrientation();

    uint8 result = CanCast();
    if(result != 0)
    {
        if(m_triggeredByAura)
        {
            SendChannelUpdate(0);
            m_triggeredByAura->SetAuraDuration(0);
        }
        finish();
        return;
    }

    // do first cast of autorepeat spell with recovery time delay (like after any authocast)
    if(IsAutoRepeat())
        m_spellState = SPELL_STATE_FINISHED;

    if(m_IsTriggeredSpell)
        cast(true);
    else
    {
        m_caster->castSpell( this );
        SendSpellStart();
    }
}

void Spell::cancel()
{
    if(m_spellState == SPELL_STATE_FINISHED)
        return;

    m_autoRepeat = false;
    if(m_spellState == SPELL_STATE_PREPARING)
    {
        SendInterrupted(0);
        SendCastResult(CAST_FAIL_INTERRUPTED);
    }
    else if(m_spellState == SPELL_STATE_CASTING)
    {
        for (int j = 0; j < 3; j++)
            for(std::list<Unit*>::iterator iunit= m_targetUnits[j].begin();iunit != m_targetUnits[j].end();++iunit)
                if (*iunit && (*iunit)->isAlive())
                    (*iunit)->RemoveAurasDueToSpell(m_spellInfo->Id);

        m_caster->RemoveAurasDueToSpell(m_spellInfo->Id);
        SendChannelUpdate(0);
        SendInterrupted(0);
        SendCastResult(CAST_FAIL_INTERRUPTED);
    }

    finish();
    m_caster->RemoveDynObject(m_spellInfo->Id);
    m_caster->RemoveGameObject(m_spellInfo->Id,true);
}

void Spell::cast(bool skipCheck)
{
    uint32 mana = 0;
    uint8 castResult = 0;
    if(m_caster->GetTypeId() != TYPEID_PLAYER && unitTarget)
        m_caster->SetInFront(unitTarget);

    castResult = CheckMana( &mana);
    if(castResult != 0)
    {
        SendCastResult(castResult);
        finish();
        return;
    }

    // triggered cast called from Spell::preper where it already checked
    if(!skipCheck)
        castResult = CanCast();

    if(castResult == 0)
    {
        SendSpellCooldown();

        TakePower(mana);
        TakeCastItem();
        TakeReagents();
        FillTargetMap();
        SendCastResult(castResult);
        SendSpellGo();

        if(IsChanneledSpell())
        {
            m_spellState = SPELL_STATE_CASTING;
            SendChannelStart(GetDuration(m_spellInfo));
        }

        std::list<Unit*>::iterator iunit;
        std::list<Item*>::iterator iitem;
        std::list<GameObject*>::iterator igo;

        bool needspelllog = true;
        for(uint32 j = 0;j<3;j++)
        {
                                                            // Dont do spell log, if is school damage spell
            if(m_spellInfo->Effect[j] == 2 || m_spellInfo->Effect[j] == 0)
                needspelllog = false;
            for(iunit= m_targetUnits[j].begin();iunit != m_targetUnits[j].end();iunit++)
            {
                // let the client worry about this
                /*if((*iunit)->GetTypeId() != TYPEID_PLAYER && m_spellInfo->TargetCreatureType)
                {
                    CreatureInfo const *cinfo = ((Creature*)(*iunit))->GetCreatureInfo();
                    if((m_spellInfo->TargetCreatureType & cinfo->type) == 0)
                        continue;
                }*/
                HandleEffects((*iunit),NULL,NULL,j);
            }
            for(iitem= m_targetItems[j].begin();iitem != m_targetItems[j].end();iitem++)
                HandleEffects(NULL,(*iitem),NULL,j);
            for(igo= m_targetGOs[j].begin();igo != m_targetGOs[j].end();igo++)
                HandleEffects(NULL,NULL,(*igo),j);

            // persistent area auras target only the ground
            if(m_spellInfo->Effect[j] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                HandleEffects(NULL,NULL,NULL, j);
        }

        if(needspelllog) SendLogExecute();

        bool canreflect = false;
        for(int j=0;j<3;j++)
        {
            switch(m_spellInfo->EffectImplicitTargetA[j])
            {
                case TARGET_SINGLE_ENEMY:
                case TARGET_ALL_ENEMY_IN_AREA:
                case TARGET_ALL_ENEMY_IN_AREA_INSTANT:
                case TARGET_ALL_ENEMIES_AROUND_CASTER:
                case TARGET_IN_FRONT_OF_CASTER:
                case TARGET_DUELVSPLAYER:
                case TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
                    //case TARGET_AE_SELECTED:
                    canreflect = true;
                    break;

                default:
                    canreflect = (m_spellInfo->AttributesEx & (1<<7)) ? true : false;
            }
            if(canreflect)
                continue;
            else break;
        }
        if(canreflect)
        {
            for(iunit= UniqueTargets.begin();iunit != UniqueTargets.end();iunit++)
            {
                reflect(*iunit);
            }
        }

        //if( ( IsAutoRepeat() || m_rangedShoot ) && m_caster->GetTypeId() == TYPEID_PLAYER )
        //((Player*)m_caster)->UpdateWeaponSkill(RANGED_ATTACK);
    }

    if(m_spellState != SPELL_STATE_CASTING)
        finish();

    //if(castResult == 0)
    //{
    //    TriggerSpell();

    //}

    // cast at creature (or GO) quest objectives update
    if( m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        if( unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT )
        {
            ((Player*)m_caster)->CastedCreatureOrGO(unitTarget->GetEntry(),unitTarget->GetGUID(),m_spellInfo->Id);
        }

        if( gameObjTarget )
        {
            ((Player*)m_caster)->CastedCreatureOrGO(gameObjTarget->GetEntry(),gameObjTarget->GetGUID(),m_spellInfo->Id);
        }    
    }
}

void Spell::SendSpellCooldown()
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* _player = (Player*)m_caster;

    uint32 cat = m_spellInfo->Category;
    int32 rec = m_spellInfo->RecoveryTime;
    int32 catrec = m_spellInfo->CategoryRecoveryTime;

    // shoot spells used equiped item cooldown values already assigned in GetAttackTime(RANGED_ATTACK)
    if (!rec && !catrec && (cat == 76 || cat == 351))
        rec = _player->GetAttackTime(RANGED_ATTACK);

    // some special item spells without correct cooldown in SpellInfo
    // cooldown information stored in item prototype
    // This used in same way in WorldSession::HandleItemQuerySingleOpcode data sending to client.
    if( rec == 0 && catrec == 0 && m_CastItem)
    {
        ItemPrototype const* proto = m_CastItem->GetProto();
        if(proto)
        {
            for(int idx = 0; idx < 5; ++idx)
            {
                if(proto->Spells[idx].SpellId == m_spellInfo->Id)
                {
                    cat    = proto->Spells[idx].SpellCategory;
                    rec    = proto->Spells[idx].SpellCooldown;
                    catrec = proto->Spells[idx].SpellCategoryCooldown;
                    break;
                }
            }
        }
    }

    if(rec > 0)
        _player->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COOLDOWN, rec);

    if(catrec > 0)
        _player->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COOLDOWN, catrec);

    if (rec < 0) rec = 0;
    if (catrec < 0) catrec = 0;

    // no cooldown after applying spell mods
    if( rec == 0 && catrec == 0)
        return;

    time_t curTime = time(NULL);

    time_t recTime    = curTime+rec/1000;                   // in secs
    time_t catrecTime = curTime+catrec/1000;                // in secs

    WorldPacket data(SMSG_SPELL_COOLDOWN, (8+4+4+4+4));
    data << m_caster->GetGUID();

    // self spell cooldown
    if (rec > 0)
    {
        data << uint32(m_spellInfo->Id);
        data << uint32(rec);
        _player->AddSpellCooldown(m_spellInfo->Id,recTime);
    }
    else
    {
        data << uint32(m_spellInfo->Id);
        data << uint32(catrec);
        _player->AddSpellCooldown(m_spellInfo->Id,catrecTime);
    }

    if (catrec > 0)
    {
        PlayerSpellMap const& player_spells = _player->GetSpellMap();
        for (PlayerSpellMap::const_iterator itr = player_spells.begin(); itr != player_spells.end(); ++itr)
        {
            if(m_spellInfo->Id==itr->first)
                continue;

            if(itr->second->state == PLAYERSPELL_REMOVED || !itr->second->active)
                continue;

            SpellEntry *spellInfo = sSpellStore.LookupEntry(itr->first);
            if( spellInfo->Category == cat)
            {
                data << uint32(itr->first);
                data << uint32(catrec);
                _player->AddSpellCooldown(itr->first,catrecTime);
            }
        }
    }

    _player->GetSession()->SendPacket(&data);

    // show cooldown for item
    if(m_CastItem)
    {
        data.Initialize(SMSG_ITEM_COOLDOWN, (8+4));
        data << m_CastItem->GetGUID();
        data << uint32(m_spellInfo->Id);
        _player->GetSession()->SendPacket(&data);
    }
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

    // check if the player caster has moved before the spell finished
    if ((m_caster->GetTypeId() == TYPEID_PLAYER && m_timer != 0) &&
        (m_castPositionX != m_caster->GetPositionX() || m_castPositionY != m_caster->GetPositionY() || m_castPositionZ != m_caster->GetPositionZ()))
    {
        // always cancel for channeled spells
        if( m_spellState == SPELL_STATE_CASTING )
            cancel();
        // don't cancel for instant and melee spells
        else if(!m_meleeSpell && casttime != 0)
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

            if(m_timer == 0 && !m_meleeSpell)
                cast();
        } break;
        case SPELL_STATE_CASTING:
        {
            if(m_timer > 0)
            {
                if( m_caster->GetTypeId() == TYPEID_PLAYER )
                {
                    // check if player has jumped before the channeling finished
                    if( ((Player*)m_caster)->HasMovementFlags(MOVEMENTFLAG_JUMPING) )
                        cancel();

                    // check for incapacitating player states
                    if( m_caster->hasUnitState(UNIT_STAT_STUNDED) ||
                        m_caster->hasUnitState(UNIT_STAT_ROOT) ||
                        m_caster->hasUnitState(UNIT_STAT_CONFUSED) )
                        cancel();

                    // check if player has turned if flag is set
                    if( m_spellInfo->ChannelInterruptFlags & CHANNEL_FLAG_TURNING && m_castOrientation != m_caster->GetOrientation() )
                        cancel();
                }

                // check if there are alive targets left
                for(int i=0;i<3;i++)
                {
                    if(m_needAliveTarget[i])
                    {
                        bool targetLeft = false;
                        for(std::list<Unit*>::iterator iunit= m_targetUnits[i].begin();iunit != m_targetUnits[i].end();++iunit)
                            if(*iunit && (*iunit)->isAlive())
                        {
                            targetLeft = true;
                            break;
                        }
                        if(!targetLeft)
                            cancel();
                    }
                }

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
    m_caster->m_canMove = true;

    /*std::list<DynamicObject*>::iterator i;
    for(i = m_dynObjToDel.begin() ; i != m_dynObjToDel.end() ; i++)
    {
        data.Initialize(SMSG_GAMEOBJECT_DESPAWN_ANIM);
        data << (*i)->GetGUID();
        m_caster->SendMessageToSet(&data, true);

        data.Initialize(SMSG_DESTROY_OBJECT);
        data << (*i)->GetGUID();
        m_caster->SendMessageToSet(&data, true);
        ObjectAccessor::Instance().AddObjectToRemoveList(*i);
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
        ObjectAccessor::Instance().AddObjectToRemoveList(*k);
    }

    m_ObjToDel.clear();*/

    if(m_TriggerSpell)
        TriggerSpell();
}

void Spell::SendCastResult(uint8 result)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_CAST_RESULT, (4+2));
    data << m_spellInfo->Id;
    if(result != 0)
    {
        data << uint8(2);                                   // status = fail
        data << uint8(result);                              // problem
        switch (result)
        {
            case CAST_FAIL_REQUIRES_XXX:
                data << uint32(m_spellInfo->RequiresSpellFocus);
                break;
        }
    }
    else
        data << uint8(0);                                   // status = ok

    ((Player*)m_caster)->GetSession()->SendPacket(&data);
}

void Spell::SendSpellStart()
{
    sLog.outDebug("Sending SMSG_SPELL_START");

    m_castFlags = CAST_FLAG_UNKNOWN1;
    if(m_rangedShoot)
        m_castFlags = m_castFlags | CAST_FLAG_AMMO;

    Unit * target;
    if(!unitTarget)
        target = m_caster;
    else
        target = unitTarget;

    WorldPacket data(SMSG_SPELL_START, (8+8+4+4+2));
    //data.append(target->GetPackGUID());
    /* in fact this should be the causer's guid. so if you clicked on a item and
       this caused spell it has to be the item's guid
    */
    data.append(m_caster->GetPackGUID());
    data.append(m_caster->GetPackGUID());
    data << m_spellInfo->Id;
    data << m_castFlags;
    data << uint32(m_timer);

    data << m_targets.m_targetMask;
    m_targets.write( &data );
    if( m_castFlags & CAST_FLAG_AMMO )
    {
        writeAmmoToPacket(&data);
    }
    m_caster->SendMessageToSet(&data, true);
}

void Spell::SendSpellGo()
{
    sLog.outDebug("Sending SMSG_SPELL_GO");

    Unit * target;
    if(!unitTarget)
        target = m_caster;
    else
        target = unitTarget;

    m_castFlags = CAST_FLAG_UNKNOWN3;
    if(m_rangedShoot)
        m_castFlags = m_castFlags | CAST_FLAG_AMMO;

    WorldPacket data(SMSG_SPELL_GO, (50)); // guess size
    //data.append(target->GetPackGUID());
    /* in fact this should be the causer's guid. so if you clicked on a item and
       this caused spell it has to be the item's guid
    */
    data.append(m_caster->GetPackGUID());
    data.append(m_caster->GetPackGUID());
    data << m_spellInfo->Id;

    data << m_castFlags;
    writeSpellGoTargets(&data);

    data << (uint8)0;                                       // miss count

    data << m_targets.m_targetMask;
    m_targets.write( &data, true );
    if( m_castFlags & CAST_FLAG_AMMO )
    {
        writeAmmoToPacket(&data);
    }

    m_caster->SendMessageToSet(&data, true);
}

void Spell::writeAmmoToPacket( WorldPacket * data )
{
    uint32 ammoInventoryType = 0;
    uint32 ammoDisplayID = 0;
    Item *pItem = ((Player*)m_caster)->GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED );
    if(pItem)
    {
        ammoInventoryType = pItem->GetProto()->InventoryType;
        if( ammoInventoryType == INVTYPE_THROWN )
            ammoDisplayID = pItem->GetProto()->DisplayInfoID;
        else
        {
            uint32 ammoID = ((Player*)m_caster)->GetUInt32Value(PLAYER_AMMO_ID);
            if(ammoID)
            {
                ItemPrototype const *pProto = objmgr.GetItemPrototype( ammoID );
                if(pProto)
                {
                    ammoDisplayID = pProto->DisplayInfoID;
                    ammoInventoryType = pProto->InventoryType;
                }
            }
        }
    }
    *data << ammoDisplayID;
    *data << ammoInventoryType;
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
    WorldPacket data(SMSG_SPELLLOGEXECUTE, (8+4+4+4+4+8));

    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        data.append(m_caster->GetPackGUID());
    else
        data.append(target->GetPackGUID());

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
}

void Spell::SendInterrupted(uint8 result)
{
    WorldPacket data;

    data.Initialize(SMSG_SPELL_FAILURE, (8+4+1));
    data.append(m_caster->GetGUID());
    data << m_spellInfo->Id;
    data << result;
    m_caster->SendMessageToSet(&data, true);

    data.Initialize(SMSG_SPELL_FAILED_OTHER, (8+4));
    data.append(m_caster->GetGUID());
    data << m_spellInfo->Id;
    m_caster->SendMessageToSet(&data, true);
}

void Spell::SendChannelUpdate(uint32 time)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data( MSG_CHANNEL_UPDATE, 4 );
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
    Unit* target = 0;

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {

        target = ObjectAccessor::Instance().GetUnit(*m_caster, ((Player *)m_caster)->GetSelection());

        WorldPacket data( MSG_CHANNEL_START, (4+4) );
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
    WorldPacket data(SMSG_RESURRECT_REQUEST, (8+4+1));
    data << m_caster->GetGUID();
    data << uint32(0) << uint8(0);

    target->GetSession()->SendPacket(&data);
}

void Spell::SendHealSpellOnPlayer(Player* target, uint32 SpellID, uint32 Damage, bool CriticalHeal)
{
    WorldPacket data(SMSG_HEALSPELL_ON_PLAYER_OBSOLETE, (8+8+4+4+1));
    data.append(target->GetPackGUID());
    data.append(m_caster->GetPackGUID());
    data << SpellID;
    data << Damage;
    data << uint8(CriticalHeal);
    target->GetSession()->SendPacket(&data);
}

void Spell::SendHealSpellOnPlayerPet(Player* target, uint32 SpellID, uint32 Damage, bool CriticalHeal)
{
    Pet* pet = target->GetPet();
    if(!pet)
        return;

    WorldPacket data(SMSG_HEALSPELL_ON_PLAYERS_PET_OBSOLETE, (8+8+4+4+1));
    data.append(pet->GetPackGUID());
    data.append(m_caster->GetPackGUID());
    data << SpellID;
    data << Damage;
    data << uint8(CriticalHeal);
    target->GetSession()->SendPacket(&data);
}

void Spell::SendPlaySpellVisual(uint32 SpellID)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 12);
    data << m_caster->GetGUID();
    data << SpellID;
    ((Player*)m_caster)->GetSession()->SendPacket(&data);
}

void Spell::TakeCastItem()
{
    if(!m_CastItem || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    // not remove cast item at triggered spell (equipping, weapon damage, etc)
    if(m_IsTriggeredSpell)
        return;

    ItemPrototype const *proto = m_CastItem->GetProto();
    uint32 ItemClass = proto->Class;
    uint32 ItemSubClass = proto->SubClass;

    bool expendable = false;
    bool withoutCharges = false;
    int32 charges;

    for (int i = 0; i<5; i++)
    {
        if (proto->Spells[i].SpellId)
        {
            // item has limited charges
            if (proto->Spells[i].SpellCharges)
            {
                if (proto->Spells[i].SpellCharges < 0)
                    expendable = true;
                charges = int32(m_CastItem->GetUInt32Value(ITEM_FIELD_SPELL_CHARGES+i));

                // item has charges left
                if (charges > 0)
                {
                    --charges;
                    if (proto->Stackable < 2)
                        m_CastItem->SetUInt32Value(ITEM_FIELD_SPELL_CHARGES+i, charges);
                    m_CastItem->SetState(ITEM_CHANGED, (Player*)m_caster);
                }

                // all charges used
                if (charges == 0)
                    withoutCharges = true;
            }
        }
    }

    if (expendable && withoutCharges)
    {
        uint32 count = 1;
        ((Player*)m_caster)->DestroyItemCount(m_CastItem, count, true);

        m_CastItem = NULL;
    }
}

void Spell::TakePower(uint32 mana)
{
    if(m_CastItem)
        return;

    // health as power used
    if(m_spellInfo->powerType == -2)
    {
        m_caster->ModifyHealth( -(int32)mana );
        return;
    }

    if(m_spellInfo->powerType <0 || m_spellInfo->powerType > POWER_HAPPINESS)
    {
        sLog.outError("Spell::TakePower: Unknown power type '%d'", m_spellInfo->powerType);
        return;
    }

    Powers powerType = Powers(m_spellInfo->powerType);

    m_caster->ModifyPower(powerType, -(int32)mana);
    if (powerType == POWER_MANA)
    {
        // Set the five second timer
        if (m_caster->GetTypeId() == TYPEID_PLAYER && mana > 0)
        {
            ((Player *)m_caster)->SetLastManaUse((uint32)getMSTime());
        }
    }
}

void Spell::TakeReagents()
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = (Player*)m_caster;
    for(uint32 x=0;x<8;x++)
    {
        if(m_spellInfo->Reagent[x] == 0)
            continue;
        uint32 itemid = m_spellInfo->Reagent[x];
        uint32 itemcount = m_spellInfo->ReagentCount[x];
        if( p_caster->HasItemCount(itemid,itemcount) )
        {
            if(m_CastItem && m_CastItem->GetProto()->ItemId == itemid)
                m_CastItem = NULL;

            p_caster->DestroyItemCount(itemid, itemcount, true);

        }
        else
        {
            SendCastResult(CAST_FAIL_ITEM_NOT_READY);
            return;
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

    // DBC already have eff 33 in SpellEffect1 (this lines totaly redundant)
    //if(m_spellInfo->Id==1804)
    //    eff = 33;

    sLog.outDebug( "Spell: Effect : %u", eff);
    if(unitTarget)
    {
        //If m_immuneToEffect type contain this effect type, IMMUNE effect.
        for (SpellImmuneList::iterator itr = unitTarget->m_spellImmune[IMMUNITY_EFFECT].begin(), next; itr != unitTarget->m_spellImmune[IMMUNITY_EFFECT].end(); itr = next)
        {
            next = itr;
            next++;
            if((*itr)->type == eff)
            {
                castResult = CAST_FAIL_IMMUNE;
                break;
            }
        }
    }
    if(castResult)
    {
        SendCastResult(castResult);
        return;
    }

    if(eff<TOTAL_SPELL_EFFECTS)
    {
        //sLog.outDebug( "WORLD: Spell FX %d < TOTAL_SPELL_EFFECTS ", eff);
        (*this.*SpellEffects[eff])(i);
    }
    /*
    else
    {
        sLog.outDebug( "WORLD: Spell FX %d > TOTAL_SPELL_EFFECTS ", eff);
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
    // check cooldowns to prevent cheating
    if(m_caster->GetTypeId()==TYPEID_PLAYER && ((Player*)m_caster)->HaveSpellCooldown(m_spellInfo->Id))
    {
        SendCastResult(CAST_FAIL_SPELL_NOT_READY_YET);
        return CAST_FAIL_SPELL_NOT_READY_YET;
    }

    // cancel autorepeat spells if cast start when moving 
    // (not wand currently autorepeat cast delayed to moving stop anyway in spell update code)
    if( ((Player*)m_caster)->GetMovementFlags() &&  (IsAutoRepeat() || m_rangedShoot) )
	{
		SendCastResult(CAST_FAIL_CANT_DO_WHILE_MOVING);
		return CAST_FAIL_CANT_DO_WHILE_MOVING;
	}

    uint8 castResult = 0;

    Unit *target = m_targets.getUnitTarget();
    if(target)
    {
        //check creaturetype
        uint32 SpellCreatureType = m_spellInfo->TargetCreatureType;

        // not find another way to fix spell target check :/
        if(m_spellInfo->Id == 603)
            SpellCreatureType = 0x7FF - 0x40;               //Curse of Doom
        else
        if(m_spellInfo->Id == 2641)                         // Dismiss Pet
            SpellCreatureType = 0;

        if(SpellCreatureType)
        {
            uint32 TargetCreatureType = 0;
            if(target->GetTypeId() == TYPEID_PLAYER)
                TargetCreatureType = 0x40;                  //1<<(7-1)
            else if ( target->GetTypeId() == TYPEID_UNIT )
            {
                uint32 CType = ((Creature*)target)->GetCreatureInfo()->type;
                if(CType>=1)
                    TargetCreatureType = 1 << ( ((Creature*)target)->GetCreatureInfo()->type - 1);
                else
                    TargetCreatureType = 0;
            }

            if(TargetCreatureType && !(SpellCreatureType & TargetCreatureType))
            {
                if(TargetCreatureType == 0x40)
                    castResult = CAST_FAIL_CANT_TARGET_PLAYERS;
                else
                    castResult = CAST_FAIL_INVALID_TARGET;
            }
        }

        //If m_immuneToDispel type contain this spell type, IMMUNE spell.
        for (SpellImmuneList::iterator itr = target->m_spellImmune[IMMUNITY_DISPEL].begin(); itr != target->m_spellImmune[IMMUNITY_DISPEL].end(); ++itr)
        {
            if((*itr)->type == m_spellInfo->Dispel)
            {
                castResult = CAST_FAIL_IMMUNE;
                break;
            }
        }
        for (SpellImmuneList::iterator itr = target->m_spellImmune[IMMUNITY_MECHANIC].begin(); itr != unitTarget->m_spellImmune[IMMUNITY_MECHANIC].end(); ++itr)
        {
            if((*itr)->type == m_spellInfo->Mechanic)
            {
                castResult = CAST_FAIL_IMMUNE;
                break;
            }
        }
        /*
        if(m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo->EquippedItemClass > 0)
        {
            Item *pitem = ((Player*)m_caster)->GetItemByPos(INVENTORY_SLOT_BAG_0,INVTYPE_WEAPON);
            if(!pitem)
                castResult = CAST_FAIL_MUST_HAVE_XXXX_IN_MAINHAND;
            else if(pitem->GetProto()->Class != m_spellInfo->EquippedItemClass)
                castResult = CAST_FAIL_MUST_HAVE_XXXX_IN_MAINHAND;
            else if(!(pitem->GetProto()->SubClass & m_spellInfo->EquippedItemSubClass))
                castResult = CAST_FAIL_MUST_HAVE_XXXX_IN_MAINHAND;
        }*/
        //Old not working code
        //if((m_spellInfo->AttributesExEx & 0x4000000) && !target->HasInArc(M_PI, m_caster) )
        //    castResult = CAST_FAIL_NOT_BEHIND_TARGET;

        //Must be behind the target.
        if( m_spellInfo->AttributesEx2 == 0x100000 && (m_spellInfo->AttributesEx & 0x200) == 0x200 && target->HasInArc(M_PI, m_caster) )
        {
            SendInterrupted(2);
            castResult = CAST_FAIL_NOT_BEHIND_TARGET;
        }
        else
            //Target must be facing you.
        if((m_spellInfo->Attributes == 0x150010) && !target->HasInArc(M_PI, m_caster) )
        {
            SendInterrupted(2);
            castResult = CAST_FAIL_NOT_IN_FRONT_OF_TARGET;
        }
        else
        if(m_caster->hasUnitState(UNIT_STAT_CONFUSED))
            castResult = CAST_FAIL_CANT_DO_WHILE_CONFUSED;
    }

    if(m_caster->hasUnitState(UNIT_STAT_STUNDED))
        castResult = CAST_FAIL_CANT_DO_WHILE_STUNNED;

    if(m_caster->m_silenced)
        castResult = CAST_FAIL_SILENCED;                    //0x5A;

    if(castResult == 0)
        castResult = CheckItems();                          // always check items (focus object can be required for any type casts)

    if(castResult == 0)
        castResult = CheckRange();

    if(castResult == 0)
    {
        uint32 mana = 0;
        castResult = CheckMana(&mana);
    }

    if( castResult != 0 )
    {
        SendCastResult(castResult);
        return castResult;
    }

    for (int i = 0; i < 3; i++)
    {
        // for effects of spells that have only one target
        switch(m_spellInfo->Effect[i])
        {
            case SPELL_EFFECT_DUMMY:
            {
                if (!unitTarget) return CAST_FAIL_FAILED;
                if(m_spellInfo->SpellIconID == 1648)
                {
                    if(unitTarget->GetHealth() > unitTarget->GetMaxHealth()*0.2)
                    {
                        castResult = CAST_FAIL_INVALID_TARGET;
                        break;
                    }
                }
                break;
            }
            case SPELL_EFFECT_TAMECREATURE:
            {
                if (!unitTarget) return CAST_FAIL_FAILED;
                if (unitTarget->GetTypeId() == TYPEID_PLAYER) return CAST_FAIL_FAILED;
                if (unitTarget->getLevel() > m_caster->getLevel())
                {
                    castResult = CAST_FAIL_TARGET_IS_TOO_HIGH;
                    break;
                }
                CreatureInfo const *cinfo = ((Creature*)unitTarget)->GetCreatureInfo();
                if(cinfo->type != CREATURE_TYPE_BEAST)
                {
                    castResult = CAST_FAIL_INVALID_TARGET;
                    break;
                }
                if(m_caster->GetPetGUID())
                {
                    castResult = CAST_FAIL_ALREADY_HAVE_SUMMON;
                    break;
                }
                if(m_caster->GetCharmGUID())
                {
                    castResult = CAST_FAIL_ALREADY_HAVE_CHARMED;
                    break;
                }
                break;
            }
            case SPELL_EFFECT_LEARN_PET_SPELL:
            {
                if(!unitTarget) return CAST_FAIL_FAILED;
                if(unitTarget->GetTypeId() == TYPEID_PLAYER) return CAST_FAIL_FAILED;
                SpellEntry *learn_spellproto = sSpellStore.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
                if(!learn_spellproto) return CAST_FAIL_FAILED;
                Creature* creatureTarget = (Creature*)unitTarget;
                uint8 learn_msg = 1;
                for(int8 x=0;x<4;x++)
                {
                    if((creatureTarget)->m_spells[x] == learn_spellproto->Id)
                    {
                        castResult = CAST_FAIL_ALREADY_LEARNED_THAT_SPELL;
                        break;
                    }
                    SpellEntry *has_spellproto = sSpellStore.LookupEntry(creatureTarget ->m_spells[x]);
                    if (!has_spellproto) learn_msg = 0;
                    else if (has_spellproto->SpellIconID == learn_spellproto->SpellIconID)
                        learn_msg = 0;
                }
                if(learn_msg)
                    castResult = CAST_FAIL_SPELL_NOT_LEARNED;
                break;
            }
            case SPELL_EFFECT_SKINNING:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER || !unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT)
                    return CAST_FAIL_FAILED;

                if( !(unitTarget->GetUInt32Value(UNIT_FIELD_FLAGS) & UNIT_FLAG_SKINNABLE) )
                {
                    castResult = CAST_FAIL_NOT_SKINNABLE;
                    break;
                }

                if ( ( ((Creature*)unitTarget)->GetCreatureInfo()->type != CREATURE_TYPE_CRITTER )
                    && ( !((Creature*)unitTarget)->lootForBody || !((Creature*)unitTarget)->loot.empty() ) )
                {
                    castResult = CAST_FAIL_CREATURE_MUST_BE_LOOTED_FIRST;
                    break;
                }

                int32 SkinningValue = ((Player*)m_caster)->GetSkillValue(SKILL_SKINNING);
                int32 TargetLevel = unitTarget->getLevel();
                int32 ReqValue = (SkinningValue < 100 ? (TargetLevel-10)*10 : TargetLevel*5);
                if (ReqValue > SkinningValue)
                {
                    castResult = CAST_FAIL_SKILL_NOT_HIGH_ENOUGH;
                    break;
                }

                // Fizzle at the skinning attempt finish
                if (m_caster->m_currentSpell == this && ReqValue > irand(SkinningValue-25, SkinningValue+5) )
                {
                    castResult = CAST_FAIL_FIZZLED;
                    // 10% chance to damage the skin when fizzled
                    if ( urand(1, 100) <= 10 )
                        unitTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
                }
                break;
            }
            case SPELL_EFFECT_SUMMON_DEAD_PET:
            {
                Creature *pet = m_caster->GetPet();
                if(!pet)
                    return CAST_FAIL_FAILED;
                if(pet->isAlive())
                    return CAST_FAIL_FAILED;
                break;
            }
            case SPELL_EFFECT_SUMMON:
            //case SPELL_EFFECT_SUMMON_WILD:                //not store in pet field
            //case SPELL_EFFECT_SUMMON_GUARDIAN:            //not store in pet field
            case SPELL_EFFECT_SUMMON_PET:
            case SPELL_EFFECT_SUMMON_POSSESSED:
            case SPELL_EFFECT_SUMMON_PHANTASM:
            case SPELL_EFFECT_SUMMON_CRITTER:             //not store in pet field
            case SPELL_EFFECT_SUMMON_DEMON:
            {
                if(m_caster->GetPetGUID())
                {
                    castResult = CAST_FAIL_ALREADY_HAVE_SUMMON;
                    break;
                }
                if(m_caster->GetCharmGUID())
                {
                    castResult = CAST_FAIL_ALREADY_HAVE_CHARMED;
                    break;
                }
                break;
            }
            case SPELL_EFFECT_LEAP:
            case SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER:
            {
                float dis = GetRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
                float fx = m_caster->GetPositionX() + dis * cos(m_caster->GetOrientation());
                float fy = m_caster->GetPositionY() + dis * sin(m_caster->GetOrientation());
                // teleport a bit above terrainlevel to avoid falling below it
                float fz = MapManager::Instance().GetMap(m_caster->GetMapId())->GetHeight(fx,fy) + 1.5;

                float caster_pos_z = m_caster->GetPositionZ();
                // Control the caster to not climb or drop when +-fz > 8
                if(!(fz<=caster_pos_z+8 && fz>=caster_pos_z-8))
                    castResult = CAST_FAIL_FAILED_ATTEMPT;
                break;
            }
            default:break;
        }

        if(castResult != 0)
        {
            SendCastResult(castResult);
            return castResult;
        }
    }

    // Conflagrate - do only when preparing
    if (m_caster->m_currentSpell != this && m_spellInfo->SpellIconID == 12 &&
        m_spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK && m_targets.getUnitTarget())
    {
        Unit::AuraMap& t_auras = m_targets.getUnitTarget()->GetAuras();
        bool hasImmolate = false;
        for(Unit::AuraMap::iterator itr = t_auras.begin(); itr != t_auras.end(); ++itr)
        {
            if (itr->second && !IsPassiveSpell(itr->second->GetId()))
            {
                SpellEntry *spellInfo = itr->second->GetSpellProto();
                if (!spellInfo) continue;
                if (spellInfo->SpellIconID != 31 || spellInfo->SpellFamilyName != SPELLFAMILY_WARLOCK) continue;
                hasImmolate = true;
                m_targets.getUnitTarget()->RemoveAurasDueToSpell(spellInfo->Id);
                break;
            }
        }
        if(!hasImmolate)
        {
            SendCastResult(CAST_FAIL_CANT_DO_THAT_YET);
            return CAST_FAIL_CANT_DO_THAT_YET;
        }
    }

    for (int i = 0; i < 3; i++)
    {
        switch(m_spellInfo->EffectApplyAuraName[i])
        {
            case SPELL_AURA_MOD_POSSESS:
            case SPELL_AURA_MOD_CHARM:
            {
                if(m_caster->GetPetGUID())
                {
                    castResult = CAST_FAIL_ALREADY_HAVE_SUMMON;
                    break;
                }
                if(m_caster->GetCharmGUID())
                {
                    castResult = CAST_FAIL_ALREADY_HAVE_CHARMED;
                    break;
                }
                if(unitTarget->getLevel() > CalculateDamage(i))
                {
                    castResult = CAST_FAIL_TARGET_IS_TOO_HIGH;
                    break;
                }
            }
            case SPELL_AURA_MOD_STEALTH:
            case SPELL_AURA_MOD_INVISIBILITY:
            {

                //detect if any mod is in x range.if true,can't steath.FIX ME!
                if(m_spellInfo->Attributes == 169148432 || m_caster->GetTypeId() != TYPEID_PLAYER)
                    break;

                CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
                Cell cell = RedZone::GetZone(p);
                cell.data.Part.reserved = ALL_DISTRICT;

                std::list<Unit*> i_data;
                std::list<Unit*>::iterator itr;
                MaNGOS::GridUnitListNotifier checker(i_data);

                TypeContainerVisitor<MaNGOS::GridUnitListNotifier, TypeMapContainer<AllObjectTypes> > object_checker(checker);
                CellLock<GridReadGuard> cell_lock(cell, p);
                cell_lock->Visit(cell_lock, object_checker, *MapManager::Instance().GetMap(m_caster->GetMapId()));
                for(itr = i_data.begin();itr != i_data.end();++itr)
                {
                    if( !(*itr)->isAlive() )
                        continue;

                    if( !(*itr)->IsHostileTo(m_caster) )
                        continue;

                    if((*itr)->GetTypeId() != TYPEID_PLAYER)
                    {
                        float attackdis = ((Creature*)(*itr))->GetAttackDistance(m_caster);
                        if((*itr)->GetDistanceSq(m_caster) < attackdis*attackdis )
                        {
                            castResult = CAST_FAIL_TOO_CLOSE_TO_ENEMY;
                            break;
                        }
                    }
                }

            };break;
            default:break;
        }
        if(castResult != 0)
        {
            SendCastResult(castResult);
            return castResult;
        }
    }
    return castResult;
}

uint8 Spell::CheckRange()
{
    // self cast doesnt need range checking -- also for Starshards fix
    if (m_spellInfo->rangeIndex == 1) return 0;

    SpellRangeEntry* srange = sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex);
    float max_range = GetMaxRange(srange);
    float min_range = GetMinRange(srange);

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
        ((Player *)m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RANGE, max_range);

    Unit *target = m_targets.getUnitTarget();

    if(target && target != m_caster)
    {
        float dist = m_caster->GetDistanceSq(target);
        if(dist > max_range * max_range)
            return CAST_FAIL_OUT_OF_RANGE;                  //0x56;
        if(dist < min_range * min_range)
            return CAST_FAIL_TOO_CLOSE;
        if( !m_IsTriggeredSpell && !m_caster->isInFront( target, max_range) )
            if (m_rangedShoot || !IsPositiveSpell(m_spellInfo->Id) && casttime != 0 && !IsSingleTarget(m_spellInfo->Id))
                return CAST_FAIL_TARGET_NEED_TO_BE_INFRONT;
    }

    if(m_targets.m_targetMask == TARGET_FLAG_DEST_LOCATION && m_targets.m_destX != 0 && m_targets.m_destY != 0 && m_targets.m_destY != 0)
    {
        float dist = m_caster->GetDistanceSq(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ);
        if(dist > max_range * max_range)
            return CAST_FAIL_OUT_OF_RANGE;
        if(dist < min_range * min_range)
            return CAST_FAIL_TOO_CLOSE;
    }

    return 0;                                               // ok
}

uint8 Spell::CheckMana(uint32 *mana)
{
    // item cast not used power
    if(m_CastItem)
        return 0;

    // health as power used
    if(m_spellInfo->powerType == -2)
    {
        uint32 currentHealth = m_caster->GetHealth();

        uint32 healthCost;

        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            PlayerLevelInfo info;
            objmgr.GetPlayerLevelInfo(m_caster->getRace(),m_caster->getClass(),m_caster->getLevel(),&info);
            healthCost = m_spellInfo->manaCost + int32(float(m_spellInfo->ManaCostPercentage)/100.0 * info.health);
        }
        else
            healthCost = m_spellInfo->manaCost + int32(float(m_spellInfo->ManaCostPercentage)/100.0 * m_caster->GetMaxHealth());

        *mana = healthCost;
        if(currentHealth <= healthCost)
            return CAST_FAIL_CANT_DO_THAT_YET;

        return 0;
    }

    if(m_spellInfo->powerType <0 || m_spellInfo->powerType > POWER_HAPPINESS)
    {
        sLog.outError("Spell::CheckMana: Unknown power type '%d'", m_spellInfo->powerType);
        return CAST_FAIL_UNKNOWN_REASON;
    }

    Powers powerType = Powers(m_spellInfo->powerType);

    int32 currentPower = m_caster->GetPower(powerType);
    int32 manaCost = m_spellInfo->manaCost;
    if(m_spellInfo->manaCostPerlevel)
        manaCost += int32(m_spellInfo->manaCostPerlevel*m_caster->getLevel());
    if(m_spellInfo->ManaCostPercentage)
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER && powerType==POWER_MANA)
        {
            PlayerLevelInfo info;
            objmgr.GetPlayerLevelInfo(m_caster->getRace(),m_caster->getClass(),m_caster->getLevel(),&info);
            manaCost += int32(float(m_spellInfo->ManaCostPercentage)/100.0 * info.mana);
        }
        else
            manaCost += int32(float(m_spellInfo->ManaCostPercentage)/100.0*m_caster->GetMaxPower(powerType));
    }

    Unit::AuraList& mPowerCostSchool = m_caster->GetAurasByType(SPELL_AURA_MOD_POWER_COST_SCHOOL);
    for(Unit::AuraList::iterator i = mPowerCostSchool.begin(); i != mPowerCostSchool.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue & int32(1 << m_spellInfo->School))
            manaCost += (*i)->GetModifier()->m_amount;

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
        ((Player *)m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COST, manaCost);

    manaCost += m_caster->GetUInt32Value(UNIT_FIELD_POWER_COST_MODIFIER);

    if (manaCost < 0)
        manaCost = 0;

    *mana = manaCost;

    if(currentPower < manaCost)
        return CAST_FAIL_NOT_ENOUGH_MANA;
    else return 0;
}

uint8 Spell::CheckItems()
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return 0;

    uint32 itemid, itemcount;
    Player* p_caster = (Player*)m_caster;

    if(m_CastItem)
    {
        itemid = m_CastItem->GetEntry();
        if( !p_caster->HasItemCount(itemid,1) )
            return CAST_FAIL_ITEM_NOT_READY;
        else
        {
            ItemPrototype const *proto = m_CastItem->GetProto();
            if(!proto)
                return CAST_FAIL_ITEM_NOT_READY;

            uint32 charges;
            for (int i = 0; i<5; i++)
                if (proto->Spells[i].SpellCharges)
                {
                    charges = m_CastItem->GetUInt32Value(ITEM_FIELD_SPELL_CHARGES+i);
                    if (charges == 0)
                        return CAST_FAIL_NO_CHARGES_REMAIN;
                }

            uint32 ItemClass = proto->Class;
            if (ItemClass == ITEM_CLASS_CONSUMABLE && unitTarget)
            {
                for (int i = 0; i < 3; i++)
                {
                    if (m_spellInfo->Effect[i] == SPELL_EFFECT_HEAL)
                        if (unitTarget->GetHealth() == unitTarget->GetMaxHealth())
                            return (uint8)CAST_FAIL_ALREADY_FULL_HEALTH;

            // Mana Potion, Rage Potion, Thistle Tea(Rogue), ... 
            if (m_spellInfo->Effect[i] == SPELL_EFFECT_ENERGIZE) 
            { 
            //Check if the Caster Has Rage For Power 
            if (m_caster->GetMaxPower(POWER_RAGE)) 
                        { 
                            if (unitTarget->GetPower(POWER_RAGE) == unitTarget->GetMaxPower(POWER_RAGE)) 
                                    return (uint8)CAST_FAIL_ALREADY_FULL_MANA; 
                        } 
                        //Check if the Caster Has Energy For Power 
                        else if (m_caster->GetMaxPower(POWER_ENERGY)) 
                        { 
                            if (unitTarget->GetPower(POWER_ENERGY) == unitTarget->GetMaxPower(POWER_ENERGY)) 
                                return (uint8)CAST_FAIL_ALREADY_FULL_MANA; 
                        } 
                        //So The Player Has Mana 
                        else if (unitTarget->GetPower(POWER_MANA) == unitTarget->GetMaxPower(POWER_MANA)) 
                        { 
                            return (uint8)CAST_FAIL_ALREADY_FULL_MANA; 
                        } 
                    } 

                }
            }
        }
    }
    if(itemTarget)
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo->EquippedItemClass >= 0)
        {
            // filter by equiped class (weapon or armor) - raw class value
            if(int32(itemTarget->GetProto()->Class) != m_spellInfo->EquippedItemClass)
                return CAST_FAIL_ITEM_NOT_READY;
            // filter by equiped subclass - bitmask of subclasses
            if((( 1 << itemTarget->GetProto()->SubClass ) & m_spellInfo->EquippedItemSubClass) == 0 )
                return CAST_FAIL_ITEM_NOT_READY;
        }
    }

    if(m_spellInfo->RequiresSpellFocus)
    {
        CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
        Cell cell = RedZone::GetZone(p);
        cell.data.Part.reserved = ALL_DISTRICT;

        GameObject* ok = NULL;
        MaNGOS::GameObjectFocusCheck go_check(m_caster,m_spellInfo->RequiresSpellFocus);
        MaNGOS::GameObjectSearcher<MaNGOS::GameObjectFocusCheck> checker(ok,go_check);

        TypeContainerVisitor<MaNGOS::GameObjectSearcher<MaNGOS::GameObjectFocusCheck>, TypeMapContainer<AllObjectTypes> > object_checker(checker);
        CellLock<GridReadGuard> cell_lock(cell, p);
        cell_lock->Visit(cell_lock, object_checker, *MapManager::Instance().GetMap(m_caster->GetMapId()));

        if(!ok) return (uint8)CAST_FAIL_REQUIRES_XXX;

        // game object found in range
    }

    for(uint32 i=0;i<8;i++)
    {
        if((itemid = m_spellInfo->Reagent[i]) == 0)
            continue;
        itemcount = m_spellInfo->ReagentCount[i];
        if( !p_caster->HasItemCount(itemid,itemcount) )
            return (uint8)CAST_FAIL_ITEM_NOT_READY;         //0x54
    }

    uint32 totems = 2;
    for(int i=0;i<2;i++)
    {
        if(m_spellInfo->Totem[i] != 0)
        {
            if( p_caster->HasItemCount(m_spellInfo->Totem[i],1) )
            {
                totems -= 1;
                continue;
            }
        }else
        totems -= 1;
    }
    if(totems != 0)
        return uint8(0x70);

    if (!itemTarget)
        return uint8(0);

    for(int i = 0; i < 3; i++)
    {
        switch (m_spellInfo->Effect[i])
        {
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if (m_spellInfo->EffectItemType[i])
                {
                    uint16 dest;
                    uint8 msg = p_caster->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, m_spellInfo->EffectItemType[i], 1, false );
                    if (msg != EQUIP_ERR_OK )
                    {
                        p_caster->SendEquipError( msg, NULL, NULL );
                        return uint8(CAST_FAIL_FAILED);     // TODO: don't show two errors
                    }
                }
                break;
            }
            case SPELL_EFFECT_ENCHANT_ITEM:
            case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
            case SPELL_EFFECT_ENCHANT_HELD_ITEM:
            {
                if(int32(m_spellInfo->EquippedItemClass) >= 0 && itemTarget->GetProto()->Class != m_spellInfo->EquippedItemClass)
                    return CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM;
                if (m_spellInfo->Effect[i] == SPELL_EFFECT_ENCHANT_HELD_ITEM && !itemTarget->IsEquipped())
                    return CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM;
                break;
            }
            case SPELL_EFFECT_DISENCHANT:
            {
                uint32 item_quality = itemTarget->GetProto()->Quality;
                if(item_quality > 4 || item_quality < 2)
                    return CAST_FAIL_CANT_BE_DISENCHANTED;
                if(itemTarget->GetProto()->Class != ITEM_CLASS_WEAPON && itemTarget->GetProto()->Class != ITEM_CLASS_ARMOR)
                    return CAST_FAIL_CANT_BE_DISENCHANTED;
                break;
            }
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            {
                if(m_caster->GetTypeId() != TYPEID_PLAYER) return CAST_FAIL_FAILED;
                if(m_spellInfo->rangeIndex == 1 || m_spellInfo->rangeIndex == 2 || m_spellInfo->rangeIndex == 7)
                    break;
                Item *pItem = ((Player*)m_caster)->GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED );
                if(!pItem || pItem->IsBroken())
                    return CAST_FAIL_MUST_HAVE_ITEM_EQUIPPED;

                uint32 type = pItem->GetProto()->InventoryType;
                uint32 ammo;
                if( type == INVTYPE_THROWN )
                    ammo = pItem->GetEntry();
                else
                    ammo = ((Player*)m_caster)->GetUInt32Value(PLAYER_AMMO_ID);

                if( !((Player*)m_caster)->HasItemCount( ammo, 1 ) )
                    return CAST_FAIL_NO_AMMO;
                break;
            }
            default:break;
        }
    }

    return uint8(0);
}

uint32 Spell::CalculateDamage(uint8 i)
{
    uint32 value = 0;
    uint32 level = 0;
    // currently the damage should not be increased by level
    /*uint32 level = m_caster->getLevel();
    if( level > m_spellInfo->maxLevel && m_spellInfo->maxLevel > 0)
        level = m_spellInfo->maxLevel;*/
    float basePointsPerLevel = m_spellInfo->EffectRealPointsPerLevel[i];
    float randomPointsPerLevel = m_spellInfo->EffectDicePerLevel[i];
    uint32 basePoints = uint32(m_spellInfo->EffectBasePoints[i]+level*basePointsPerLevel);
    uint32 randomPoints = uint32(m_spellInfo->EffectDieSides[i]+level*randomPointsPerLevel);
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

    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_DAMAGE, value);

    return value;
}

void Spell::HandleTeleport(uint32 id, Unit* Target)
{

    if(!Target || Target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(Target->isInFlight())
        return;

    if(m_spellInfo->Id == 8690 || m_spellInfo->Id == 556 )
    {
        Field *fields;
        QueryResult *result = sDatabase.PQuery("SELECT `map`,`zone`,`position_x`,`position_y`,`position_z` FROM `character_homebind` WHERE `guid` = '%u'", m_caster->GetGUIDLow());
        if(!result)
        {
            sLog.outError( "SPELL: No homebind location set for %i\n", m_caster->GetGUIDLow());
            return;
        }
        fields = result->Fetch();

        TeleportCoords* TC = new TeleportCoords();
        TC->mapId = fields[0].GetUInt32();
        TC->x = fields[2].GetFloat();
        TC->y = fields[3].GetFloat();
        TC->z = fields[4].GetFloat();

        delete result;

        ((Player*)Target)->TeleportTo(TC->mapId,TC->x,TC->y,TC->z,Target->GetOrientation());
        delete TC;
    }
    else
    {
        TeleportCoords const* TC = objmgr.GetTeleportCoords(id);
        if(!TC)
        {
            sLog.outError( "SPELL: unknown Teleport Coords ID %i\n", id );
            return;
        }
        ((Player*)Target)->TeleportTo(TC->mapId,TC->x,TC->y,TC->z,Target->GetOrientation());
    }
}

void Spell::Delayed(int32 delaytime)
{
    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    m_timer += delaytime;

    if(m_timer > casttime)
        m_timer = (casttime > 0 ? casttime : 0);

    WorldPacket data(SMSG_SPELL_DELAYED, 12);
    data << m_caster->GetGUID();
    data << uint32(delaytime);

    ((Player*)m_caster)->GetSession()->SendPacket(&data);
}

void Spell::DelayedChannel(int32 delaytime)
{
    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER || getState() != SPELL_STATE_CASTING)
        return;

    int32 appliedDelayTime = delaytime;
    uint32 duration = GetDuration(m_spellInfo);

    if(m_timer + delaytime < 0)
    {
        appliedDelayTime = m_timer;
        m_timer = 0;
    }
    else
        m_timer += delaytime;

    if(m_timer > duration)
    {
        appliedDelayTime -= (m_timer - duration);
        m_timer = duration;
    }

    m_delayedTime += appliedDelayTime;

    // Cancel spell if aggregate channeling delay is greater than base channeling duration
    if(m_delayedTime >= int32(duration))
    {
        sLog.outDebug("Spell %u canceled because of accumulated delay: %i ms", m_spellInfo->Id, m_delayedTime);
        cancel();
        return;
    }

    sLog.outDebug("Spell %u delayed for %i ms, new duration: %u ms", m_spellInfo->Id, appliedDelayTime, m_timer);

    for(int j = 0; j < 3; j++)
    {
        // Delay auras with fixed targets
        for(std::list<Unit*>::iterator iunit= m_targetUnits[j].begin();iunit != m_targetUnits[j].end();++iunit)
            if (*iunit)
                (*iunit)->DelayAura(m_spellInfo->Id, j, appliedDelayTime);

        // Delay persistent area auras
        DynamicObject* dynObj = m_caster->GetDynObject(m_spellInfo->Id, j);
        if(dynObj)
            dynObj->Delay(appliedDelayTime);
    }

    SendChannelUpdate(m_timer);
}

void Spell::reflect(Unit *refunit)
{
    if (m_caster == refunit)
        return;

    SpellEntry *refspell = NULL;

    // if the spell to reflect is a reflect spell, do nothing.
    for(int i=0; i<3; i++)
        if(m_spellInfo->Effect[i] == 6 && (m_spellInfo->EffectApplyAuraName[i] == 74 || m_spellInfo->EffectApplyAuraName[i] == 28))
            return;

    int32 reflectchance = 0;                                // proper base reflect chance is ?

    Unit::AuraList& mReflectSpells = refunit->GetAurasByType(SPELL_AURA_REFLECT_SPELLS);
    for(Unit::AuraList::iterator i = mReflectSpells.begin(); i != mReflectSpells.end(); ++i)
        reflectchance += (*i)->GetModifier()->m_amount;

    Unit::AuraList& mReflectSpellsSchool = refunit->GetAurasByType(SPELL_AURA_REFLECT_SPELLS_SCHOOL);
    for(Unit::AuraList::iterator i = mReflectSpellsSchool.begin(); i != mReflectSpellsSchool.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue & int32(1 << m_spellInfo->School))
            reflectchance += (*i)->GetModifier()->m_amount;

    if (reflectchance > 0 && uint32(reflectchance) >= urand(0,100))
    {
        Spell spell(refunit, refspell, true, 0);

        SpellCastTargets targets;
        targets.setUnitTarget( m_caster );
        spell.prepare(&targets);
    }
}
