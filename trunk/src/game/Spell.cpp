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

    //SpellEntry *spellInfo;
    Player* p_caster;

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

    if( m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo )
    {
        p_caster = (Player*)m_caster;
        /*
        PlayerSpellList const& player_spells = p_caster->getSpellList();
        for (PlayerSpellList::const_iterator itr = player_spells.begin(); itr != player_spells.end(); ++itr)
        {
            if ((*itr)->spellId != m_spellInfo->Id && (*itr)->active == 1)
            {
                spellInfo = sSpellStore.LookupEntry((*itr)->spellId);
                if(spellInfo && spellInfo->SpellIconID == m_spellInfo->SpellIconID && spellInfo->EffectMiscValue[0] ==10)
                {
                    casttime=casttime+(spellInfo->EffectBasePoints[0]+1);
                }
            }
        }
        */
        p_caster->ApplySpellMod(m_spellInfo->Id, SPELLMOD_CASTING_TIME, casttime);
        casttime = int32(casttime/(100+p_caster->m_modCastSpeedPct)*100);
    }

    m_timer = casttime<0?0:casttime;

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
                    tmpUnitMap.push_back(m_targets.getUnitTarget());
                    break;
                case SPELL_EFFECT_SKILL:
                case SPELL_EFFECT_FEED_PET:
                case SPELL_EFFECT_LEARN_PET_SPELL:
                    tmpUnitMap.push_back(m_caster);
                    break;
                case SPELL_EFFECT_DISENCHANT:
                case SPELL_EFFECT_ENCHANT_ITEM:
                case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
                case SPELL_EFFECT_ENCHANT_HELD_ITEM:
                    tmpItemMap.push_back(itemTarget);
                    break;
                case SPELL_EFFECT_APPLY_AREA_AURA:
                    if(m_spellInfo->Attributes == 0x9050000)// AreaAura
                        SetTargetMap(i,TARGET_AF_P,tmpUnitMap,tmpItemMap,tmpGOMap);
                    break;
                default:
                    break;
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
    float radius =  GetRadius(sSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
    switch(cur)
    {
        case TARGET_TOTEM_EARTH:
        case TARGET_TOTEM_WATER:
        case TARGET_TOTEM_AIR:
        case TARGET_TOTEM_FIRE:
        case TARGET_SELF:
        case TARGET_DY_OBJ:                                 //add by vendy
        {
            TagUnitMap.push_back(m_caster);
        }break;
        case TARGET_PET:
        {
            Unit* tmpUnit = m_caster->GetPet();
            if (!tmpUnit) break;
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
            // targets the ground, not the units in the area
            if (m_spellInfo->Effect[i]!=SPELL_EFFECT_PERSISTENT_AREA_AURA)
            {
                CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
                Cell cell = RedZone::GetZone(p);
                cell.data.Part.reserved = ALL_DISTRICT;
                cell.SetNoCreate();

                MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i,PUSH_DEST_CENTER);
                TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
                CellLock<GridReadGuard> cell_lock(cell, p);
                cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
            }
        }break;
        case TARGET_AC_P:
        {
            Group* pGroup = m_caster->GetTypeId() == TYPEID_PLAYER ? objmgr.GetGroupByLeader(((Player*)m_caster)->GetGroupLeader()) : NULL;
            if(pGroup)
            {
                for(uint32 p=0;p<pGroup->GetMembersCount();p++)
                {
                    Unit* Target = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(p));
                    if(!Target || Target->GetGUID() == m_caster->GetGUID())
                        continue;
                    if(m_caster->GetDistanceSq(Target) < radius * radius )
                        TagUnitMap.push_back(Target);
                }
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
            // targets the ground, not the units in the area
            if (m_spellInfo->Effect[i]!=SPELL_EFFECT_PERSISTENT_AREA_AURA)
            {
                CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
                Cell cell = RedZone::GetZone(p);
                cell.data.Part.reserved = ALL_DISTRICT;
                cell.SetNoCreate();

                MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i,PUSH_DEST_CENTER);
                TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
                CellLock<GridReadGuard> cell_lock(cell, p);
                cell_lock->Visit(cell_lock, object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId()));
            }
        }break;
        case TARGET_MINION:
        {
            if(m_spellInfo->Effect[i] != 83) TagUnitMap.push_back(m_caster);
        }break;
        case TARGET_S_P:
        {
            TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
        case TARGET_AF_P:
        {
            Player* targetPlayer = m_targets.getUnitTarget()->GetTypeId() == TYPEID_PLAYER ? (Player*)m_targets.getUnitTarget() : NULL;

            Group* pGroup = targetPlayer ? objmgr.GetGroupByLeader(targetPlayer->GetGroupLeader()) : NULL;
            if(pGroup)
            {
                for(uint32 p=0;p<pGroup->GetMembersCount();p++)
                {
                    Unit* Target = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(p));
                    if(m_targets.getUnitTarget()->GetDistanceSq(Target) < radius * radius )
                        TagUnitMap.push_back(Target);
                }
            }
            else
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

            Group* pGroup = m_caster->GetTypeId() == TYPEID_PLAYER ? objmgr.GetGroupByLeader(((Player*)m_caster)->GetGroupLeader()) : NULL;
            if(pGroup)
            {
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
                    if(Target->getFaction() == m_caster->getFaction() && m_caster->GetDistanceSq(Target) < radius * radius)
                        TagUnitMap.push_back(Target);
                }
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
        case TARGET_AF_PC:
        {
            Player* targetPlayer = m_targets.getUnitTarget()->GetTypeId() == TYPEID_PLAYER ? (Player*)m_targets.getUnitTarget() : NULL;

            Group* pGroup = targetPlayer ? objmgr.GetGroupByLeader(targetPlayer->GetGroupLeader()) : NULL;
            if(pGroup)
            {
                for(uint32 p=0;p<pGroup->GetMembersCount();p++)
                {
                    Unit* Target = ObjectAccessor::Instance().FindPlayer(pGroup->GetMemberGUID(p));
                    if(targetPlayer->GetDistanceSq(Target) < radius * radius &&
                        targetPlayer->getClass() == Target->getClass())
                        TagUnitMap.push_back(Target);
                }
            }
            else
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
        if (removed_utarget)
            TagUnitMap.push_back(m_targets.getUnitTarget());
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
        return;
    }

    // do first cast of autorepeat spell with recovery time delay (like after any authocast)
    if(IsAutoRepeat())
        m_spellState = SPELL_STATE_FINISHED;

    if(m_Istriggeredpell)
        cast();
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

    WorldPacket data;
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
                if (*iunit) (*iunit)->RemoveAurasDueToSpell(m_spellInfo->Id);
        m_caster->RemoveAurasDueToSpell(m_spellInfo->Id);
        SendChannelUpdate(0);
    }

    finish();
    m_caster->RemoveDynObject(m_spellInfo->Id);
    m_caster->RemoveGameObject(m_spellInfo->Id,true);
}

void Spell::cast()
{
    uint32 mana = 0;
    uint8 castResult = 0;
    if(m_caster->GetTypeId() != TYPEID_PLAYER && unitTarget)
        m_caster->SetInFront(unitTarget);
    castResult = CheckMana( &mana);
    if(castResult != 0)
    {
        SendCastResult(castResult);
        return;
    }
    castResult = CanCast();
    if(castResult == 0)
    {
        TakePower(mana);
        TakeCastItem();
        TakeReagents();
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

        bool needspelllog = true;
        for(uint32 j = 0;j<3;j++)
        {
                                                            // Dont do spell log, if is school damage spell
            if(m_spellInfo->Effect[j] == 2 || m_spellInfo->Effect[j] == 0)
                needspelllog = false;
            for(iunit= m_targetUnits[j].begin();iunit != m_targetUnits[j].end();iunit++)
            {
                if((*iunit)->GetTypeId() != TYPEID_PLAYER && m_spellInfo->TargetCreatureType)
                {
                    CreatureInfo const *cinfo = ((Creature*)(*iunit))->GetCreatureInfo();
                    if(m_spellInfo->TargetCreatureType != cinfo->type)
                        continue;
                }
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
                case TARGET_S_E:
                case TARGET_AE_E:
                case TARGET_AE_E_INSTANT:
                case TARGET_AC_E:
                case TARGET_INFRONT:
                case TARGET_DUELVSPLAYER:
                case TARGET_AE_E_CHANNEL:
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

        SendSpellCooldown();

        if( ( IsAutoRepeat() || m_rangedShoot ) && m_caster->GetTypeId() == TYPEID_PLAYER )
            ((Player*)m_caster)->UpdateRangedSkillWeapon();
    }

    if(m_spellState != SPELL_STATE_CASTING)
        finish();

    //if(castResult == 0)
    //{
    //    TriggerSpell();

    //}

}

void Spell::SendSpellCooldown()
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* _player = (Player*)m_caster;

    int32 rec = m_spellInfo->RecoveryTime;
    int32 catrec = m_spellInfo->CategoryRecoveryTime;

    // throw spell used equiped item cooldown values
    if (m_spellInfo->Id == 2764)
        rec = _player->GetAttackTime(RANGED_ATTACK);

    // shoot spell used equiped wand cooldown values
    if (m_spellInfo->Id == 5019)
    {
        if(Item* item = _player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
            if(ItemPrototype const* proto = item->GetProto())
                rec = proto->Delay;
    }

    // some special item spells without correct cooldown in SpellInfo
    // cooldown information stored in item prototype
    if( rec == 0 && catrec == 0 && m_CastItem)
    {
        ItemPrototype const* proto = m_CastItem->GetProto();
        if(proto)
        {
            for(int idx = 0; idx < 5; ++idx)
            {
                if(proto->Spells[idx].SpellId == m_spellInfo->Id)
                {
                    rec    = proto->Spells[idx].SpellCooldown;
                    catrec = proto->Spells[idx].SpellCategoryCooldown;
                    break;
                }
            }
        }
    }

    // no cooldown
    if( rec == 0 && catrec == 0)
        return;

    _player->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COOLDOWN, rec);
    _player->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COOLDOWN, catrec);
    if (rec < 0) rec = 0;
    if (catrec < 0) catrec = 0;

    WorldPacket data;

    data.clear();
    data.Initialize(SMSG_SPELL_COOLDOWN);
    data << m_caster->GetGUID();
    if (catrec > 0)
    {
        PlayerSpellList const& player_spells = _player->getSpellList();
        for (PlayerSpellList::const_iterator itr = player_spells.begin(); itr != player_spells.end(); ++itr)
        {
            if(!(*itr)->spellId || !(*itr)->active)
                continue;
            SpellEntry *spellInfo = sSpellStore.LookupEntry((*itr)->spellId);
            if( spellInfo->Category == m_spellInfo->Category)
            {
                data << uint32((*itr)->spellId);
                if ((*itr)->spellId != m_spellInfo->Id || rec == 0)
                    data << uint32(catrec);
                else
                    data << uint32(rec);
            }
        }
    }
    else if (rec > 0)
    {
        data << uint32(m_spellInfo->Id);
        data << uint32(rec);
    }
    _player->GetSession()->SendPacket(&data);
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

    if( ( m_timer != 0 && !m_meleeSpell) && (m_caster->GetTypeId() == TYPEID_PLAYER) &&
        ( m_castPositionX != m_caster->GetPositionX() || m_castPositionY != m_caster->GetPositionY() || m_castPositionZ != m_caster->GetPositionZ() ) )
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

            if(m_timer == 0 && !m_meleeSpell)
                cast();
        } break;
        case SPELL_STATE_CASTING:
        {
            if(m_timer > 0)
            {
                // TODO:Fix me
                // If m_spellInfo->ChannelInterruptFlags & m_caster->m_channelInterruptFlag,stop the channel;
                // else channel can't be stoped,and can't attack the target when being attacked.
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

    if(m_TriggerSpell)
        TriggerSpell();
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

    m_caster->SendMessageToSet(&data, true);
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
    Unit* target = 0;

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {

        target = ObjectAccessor::Instance().GetUnit(*m_caster, ((Player *)m_caster)->GetSelection());

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
    data << m_caster->GetGUID();
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

void Spell::SendHealSpellOnPlayerPet(Player* target, uint32 SpellID, uint32 Damage)
{
    WorldPacket data;
    data.Initialize(SMSG_HEALSPELL_ON_PLAYERS_PET_OBSOLETE);
    data << uint8(0xFF) << target->GetPetGUID();
    data << uint8(0xFF) << m_caster->GetGUID();
    data << SpellID;
    data << Damage;
    data << uint8(0);
    target->GetSession()->SendPacket(&data);
}

void Spell::SendPlaySpellVisual(uint32 SpellID)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data;
    data.Initialize(SMSG_PLAY_SPELL_VISUAL);
    data << m_caster->GetGUID();
    data << SpellID;
    ((Player*)m_caster)->GetSession()->SendPacket(&data);
}

void Spell::TakeCastItem()
{
    if(!m_CastItem || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    ItemPrototype const *proto = m_CastItem->GetProto();
    uint32 ItemCount = m_CastItem->GetCount();
    uint32 ItemClass = proto->Class;

    if (ItemClass == ITEM_CLASS_CONSUMABLE)
    {
        if(proto->DisplayInfoID == 6009 && m_spellInfo->School == 5)
            return;
        ((Player*)m_caster)->DestroyItemCount(proto->ItemId, 1, true);
        if(ItemCount<=1)
        {
            //delete m_CastItem;
            m_CastItem = NULL;
        }
    }
}

void Spell::TakePower(uint32 mana)
{
    if(m_CastItem)
        return;

    Powers powerType = Powers(m_spellInfo->powerType);

    uint32 currentPower = m_caster->GetPower(powerType);

    m_caster->SetPower(powerType, currentPower - mana);
    if (powerType == POWER_MANA)
    {
        // Set the five second timer
        if (m_caster->GetTypeId() == TYPEID_PLAYER)
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
            p_caster->DestroyItemCount(itemid, itemcount, true);
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
    SpellSchools school = SpellSchools(m_spellInfo->School);
    unitTarget = pUnitTarget;
    itemTarget = pItemTarget;
    gameObjTarget = pGOTarget;

    damage = CalculateDamage((uint8)i);
    uint8 eff = m_spellInfo->Effect[i];

    sLog.outDebug( "WORLD: Spell FX id is %u", eff);
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
        sLog.outDebug( "WORLD: Spell FX %d < TOTAL_SPELL_EFFECTS ", eff);
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
    uint8 castResult = 0;

    Unit *target = m_targets.getUnitTarget();
    if(target)
    {
        //If m_immuneToDispel type contain this spell type, IMMUNE spell.
        for (SpellImmuneList::iterator itr = target->m_spellImmune[IMMUNITY_DISPEL].begin(), next; itr != target->m_spellImmune[IMMUNITY_DISPEL].end(); itr = next)
        {
            next = itr;
            next++;
            if((*itr)->type == m_spellInfo->Dispel)
            {
                castResult = CAST_FAIL_IMMUNE;
                break;
            }
        }
        for (SpellImmuneList::iterator itr = unitTarget->m_spellImmune[IMMUNITY_MECHANIC].begin(), next; itr != unitTarget->m_spellImmune[IMMUNITY_MECHANIC].end(); itr = next)
        {
            next = itr;
            next++;
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
        if((m_spellInfo->AttributesExEx & 0x4000000) && !target->HasInArc(M_PI, m_caster) )
            castResult = CAST_FAIL_NOT_BEHIND_TARGET;

        if(m_caster->hasUnitState(UNIT_STAT_CONFUSED))
            castResult = CAST_FAIL_CANT_DO_WHILE_CONFUSED;
    }

    if(m_caster->hasUnitState(UNIT_STAT_STUNDED))
        castResult = CAST_FAIL_CANT_DO_WHILE_STUNNED;

    if(m_caster->m_silenced)
        castResult = CAST_FAIL_SILENCED;                    //0x5A;

    if(m_CastItem || itemTarget)
        castResult = CheckItems();

    if(castResult == 0)
        castResult = CheckRange();

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
                if (m_caster->GetTypeId() != TYPEID_PLAYER) return CAST_FAIL_FAILED;
                if(!unitTarget) return CAST_FAIL_FAILED;
                if(unitTarget->GetTypeId() != TYPEID_UNIT) return CAST_FAIL_FAILED;
                CreatureInfo const *cinfo = ((Creature*)unitTarget)->GetCreatureInfo();
                if(cinfo->type != CREATURE_TYPE_BEAST && cinfo->type != CREATURE_TYPE_DRAGON)
                {
                    castResult = CAST_FAIL_INVALID_TARGET;
                    break;
                }
                if(unitTarget->m_form == 99)
                {
                    castResult = CAST_FAIL_NOT_SKINNABLE;
                    break;
                }
                int32 fishvalue = ((Player*)m_caster)->GetSkillValue(SKILL_SKINNING);
                int32 targetlevel = unitTarget->getLevel();
                if(fishvalue < (targetlevel-5)*5)
                    castResult = CAST_FAIL_FAILED;
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
            case SPELL_EFFECT_SUMMON_WILD:
            case SPELL_EFFECT_SUMMON_GUARDIAN:
            case SPELL_EFFECT_SUMMON_PET:
            case SPELL_EFFECT_SUMMON_POSSESSED:
            case SPELL_EFFECT_SUMMON_PHANTASM:
            case SPELL_EFFECT_SUMMON_CRITTER:
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
            SendCastResult(CAST_FAIL_FAILED);               // TODO: find a correct error message
            return CAST_FAIL_FAILED;
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

                    FactionTemplateResolver my_faction = m_caster->getFactionTemplateEntry();
                    FactionTemplateResolver its_faction = (*itr)->getFactionTemplateEntry();
                    if( my_faction.IsFriendlyTo(its_faction) )
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

    SpellRange* srange = sSpellRange.LookupEntry(m_spellInfo->rangeIndex);
    float max_range = GetMaxRange(srange);
    float min_range = GetMinRange(srange);

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
        ((Player *)m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RANGE, max_range);

    Unit *target = m_targets.getUnitTarget();

    if(target && target->GetGUID() != m_caster->GetGUID())
    {
        float dist = m_caster->GetDistanceSq(target);
        if(dist > max_range * max_range)
            return CAST_FAIL_OUT_OF_RANGE;                  //0x56;
        if(dist < min_range * min_range)
            return CAST_FAIL_TOO_CLOSE;
        if( m_caster != target && !m_Istriggeredpell && !m_caster->isInFront( target, max_range) )
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
    Powers powerType = Powers(m_spellInfo->powerType);

    uint32 currentPower = m_caster->GetPower(powerType);
    uint32 manaCost = m_spellInfo->manaCost;
    if(m_spellInfo->manaCostPerlevel)
        manaCost += uint32(m_spellInfo->manaCostPerlevel*m_caster->getLevel());
    if(m_spellInfo->ManaCostPercentage)
        manaCost += uint32(m_spellInfo->ManaCostPercentage/100*m_caster->GetMaxPower(powerType));

    Unit::AuraList& mPowerCostSchool = m_caster->GetAurasByType(SPELL_AURA_MOD_POWER_COST_SCHOOL);
    for(Unit::AuraList::iterator i = mPowerCostSchool.begin(); i != mPowerCostSchool.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue == m_spellInfo->School)
            manaCost += (*i)->GetModifier()->m_amount;

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
        ((Player *)m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COST, manaCost);

    manaCost += m_caster->GetUInt32Value(UNIT_FIELD_POWER_COST_MODIFIER);
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

            uint32 ItemClass = proto->Class;
            if (ItemClass == ITEM_CLASS_CONSUMABLE && unitTarget)
            {
                for (int i = 0; i < 3; i++)
                {
                    if (m_spellInfo->Effect[i] == SPELL_EFFECT_HEAL)
                        if (unitTarget->GetHealth() == unitTarget->GetMaxHealth())
                            return (uint8)CAST_FAIL_ALREADY_FULL_HEALTH;
                    if (m_spellInfo->Effect[i] == SPELL_EFFECT_ENERGIZE)
                        if (unitTarget->GetPower(POWER_MANA) == unitTarget->GetMaxPower(POWER_MANA))
                            return (uint8)CAST_FAIL_ALREADY_FULL_MANA;
                }
            }
            return uint8(0);
        }
    }
    if(itemTarget)
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo->EquippedItemClass > 0)
        {
            if(!(itemTarget->GetProto()->Class & m_spellInfo->EquippedItemClass))
                return CAST_FAIL_ITEM_NOT_READY;
            else if(!(itemTarget->GetProto()->SubClass & m_spellInfo->EquippedItemSubClass))
                return CAST_FAIL_ITEM_NOT_READY;
        }
    }

    if(m_spellInfo->RequiresSpellFocus)
    {
        SpellFocusObject* focusobj = sSpellFocusObject.LookupEntry(m_spellInfo->RequiresSpellFocus);
        assert(focusobj);
        char const* focusname = focusobj->Name;

        // Find GO
        SpellRange* srange = sSpellRange.LookupEntry(m_spellInfo->rangeIndex);
        float range = GetMaxRange(srange);

        CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
        Cell cell = RedZone::GetZone(p);
        cell.data.Part.reserved = ALL_DISTRICT;

        GameObject* ok = NULL;
        MaNGOS::GameObjectWithNameIn2DRangeChecker checker(ok,m_caster,focusobj->Name, range);

        TypeContainerVisitor<MaNGOS::GameObjectWithNameIn2DRangeChecker, TypeMapContainer<AllObjectTypes> > object_checker(checker);
        CellLock<GridReadGuard> cell_lock(cell, p);
        cell_lock->Visit(cell_lock, object_checker, *MapManager::Instance().GetMap(m_caster->GetMapId()));

        if(!ok) return (uint8)CAST_FAIL_REQUIRES_SOMETHING;

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

    if( !p_caster->HasItemCount(itemTarget->GetEntry(), 1) )
        return (uint8)CAST_FAIL_ITEM_NOT_READY;

    for(int i = 0; i < 3; i++)
    {
        switch (m_spellInfo->Effect[i])
        {
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if (m_spellInfo->EffectItemType[i])
                {
                    uint16 dest;
                    uint8 msg = p_caster->CanStoreNewItem(0, NULL_SLOT, dest, m_spellInfo->EffectItemType[i], 1, false );
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
                if((itemTarget->GetProto()->Class & m_spellInfo->EquippedItemClass) == 0)
                    return CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM;
                if (m_spellInfo->Effect[i] == SPELL_EFFECT_ENCHANT_HELD_ITEM &&
                    itemTarget->GetSlot() < EQUIPMENT_SLOT_END)
                    return CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM;
                break;
            }
            case SPELL_EFFECT_DISENCHANT:
            {
                uint32 item_quality = itemTarget->GetProto()->Quality;
                if(item_quality > 4 || item_quality < 2)
                    return CAST_FAIL_CANT_BE_DISENCHANTED;
                if(itemTarget->GetProto()->Class != 2 && itemTarget->GetProto()->Class != 4)
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
                if(!pItem)
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
    uint32 level = m_caster->getLevel();
    if( level > m_spellInfo->maxLevel && m_spellInfo->maxLevel > 0)
        level = m_spellInfo->maxLevel;
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

    TeleportCoords* TC = new TeleportCoords();

    if(m_spellInfo->Id == 8690 )
    {
        Field *fields;
        QueryResult *result4 = sDatabase.PQuery("SELECT `map`,`zone`,`position_x`,`position_y`,`position_z` FROM `character_homebind` WHERE `guid` = '%u'", m_caster->GetGUIDLow());
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

    ((Player*)Target)->TeleportTo(TC->mapId,TC->x,TC->y,TC->z,0.0f);
}

void Spell::Delayed(int32 delaytime)
{
    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

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
        if(m_spellInfo->Effect[i] == 6 && (m_spellInfo->EffectApplyAuraName[i] == 74 || m_spellInfo->EffectApplyAuraName[i] == 28))
            return;
    for(std::list<struct ReflectSpellSchool*>::iterator i = refunit->m_reflectSpellSchool.begin();i != refunit->m_reflectSpellSchool.end();i++)
    {
        if((*i)->school == -1 || (*i)->school == m_spellInfo->School)
        {
            if((*i)->chance >= urand(0,100))
                refspell = m_spellInfo;
        }
    }

    if(!refspell || m_caster == refunit) return;

    Spell spell(refunit, refspell, true, 0);

    SpellCastTargets targets;
    targets.setUnitTarget( m_caster );
    spell.prepare(&targets);
}
