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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
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
#include "LootMgr.h"
#include "VMapFactory.h"

#define SPELL_CHANNEL_UPDATE_INTERVAL 1000

extern pEffect SpellEffects[TOTAL_SPELL_EFFECTS];

bool IsQuestTameSpell(uint32 spellId)
{
    SpellEntry const *spellproto = sSpellStore.LookupEntry(spellId);
    if (!spellproto) return false;

    return spellproto->Effect[0] == SPELL_EFFECT_THREAT
        && spellproto->Effect[1] == SPELL_EFFECT_APPLY_AURA && spellproto->EffectApplyAuraName[1] == SPELL_AURA_DUMMY;
}

SpellCastTargets::SpellCastTargets()
{
    m_unitTarget = NULL;
    m_itemTarget = NULL;
    m_GOTarget   = NULL;

    m_unitTargetGUID   = 0;
    m_GOTargetGUID     = 0;
    m_CorpseTargetGUID = 0;
    m_itemTargetGUID   = 0;

    m_srcX = m_srcY = m_srcZ = m_destX = m_destY = m_destZ = 0;
    m_strTarget = "";
    m_targetMask = 0;
}

SpellCastTargets::~SpellCastTargets()
{
}

void SpellCastTargets::setUnitTarget(Unit *target)
{
    if (!target)
        return;

    m_destX = target->GetPositionX();
    m_destY = target->GetPositionY();
    m_destZ = target->GetPositionZ();
    m_unitTarget = target;
    m_unitTargetGUID = target->GetGUID();
    m_targetMask |= TARGET_FLAG_UNIT | TARGET_FLAG_DEST_LOCATION;
}

void SpellCastTargets::setGOTarget(GameObject *target)
{
    m_GOTarget = target;
    m_GOTargetGUID = target->GetGUID();
//    m_targetMask |= TARGET_FLAG_OBJECT;
}

void SpellCastTargets::Update(Unit* caster)
{
    m_GOTarget   = m_GOTargetGUID ? ObjectAccessor::Instance().GetGameObject(*caster,m_GOTargetGUID) : NULL;
    m_unitTarget = m_unitTargetGUID ? 
        ( m_unitTargetGUID==caster->GetGUID() ? caster : ObjectAccessor::Instance().GetUnit(*caster, m_unitTargetGUID) ) : 
        NULL;

    m_itemTarget = NULL;
    if(caster->GetTypeId()==TYPEID_PLAYER)
    {
        if(m_targetMask & TARGET_FLAG_ITEM)
            m_itemTarget = ((Player*)caster)->GetItemByGuid(m_itemTargetGUID);
        else
        {
            Player* pTrader = ((Player*)caster)->GetTrader();
            if(pTrader && m_itemTargetGUID < TRADE_SLOT_COUNT)
                m_itemTarget = pTrader->GetItemByPos(pTrader->GetItemPosByTradeSlot(m_itemTargetGUID));
        }
    }
}

void SpellCastTargets::read ( WorldPacket * data,Unit *caster )
{
    *data >> m_targetMask;

    if(m_targetMask == TARGET_FLAG_SELF)
    {
        m_destX = caster->GetPositionX();
        m_destY = caster->GetPositionY();
        m_destZ = caster->GetPositionZ();
        m_unitTarget = caster;
        m_unitTargetGUID = caster->GetGUID();
        return;
    }

    if(m_targetMask & TARGET_FLAG_UNIT)
        m_unitTargetGUID = readGUID(*data);

    if(m_targetMask & TARGET_FLAG_OBJECT)
        m_GOTargetGUID = readGUID(*data);

    if((m_targetMask & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM)) && caster->GetTypeId() == TYPEID_PLAYER)
        m_itemTargetGUID = readGUID(*data);

    if(m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
        *data >> m_srcX >> m_srcY >> m_srcZ;

    if(m_targetMask & TARGET_FLAG_DEST_LOCATION)
        *data >> m_destX >> m_destY >> m_destZ;

    if(m_targetMask & TARGET_FLAG_STRING)
        *data >> m_strTarget;

    if(m_targetMask & TARGET_FLAG_CORPSE)
        m_CorpseTargetGUID = readGUID(*data);

    // find real units/GOs
    Update(caster);
}

void SpellCastTargets::write ( WorldPacket * data, bool forceAppend)
{
    uint32 len = data->size();

    // don't append targets when spell's for your own..
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

    if(m_targetMask & TARGET_FLAG_CORPSE)
        data->appendPackGUID(m_CorpseTargetGUID);

    if(forceAppend && data->size() == len)
        *data << (uint8)0;
}

Spell::Spell( Unit* Caster, SpellEntry const *info, bool triggered, Aura* Aur, uint64 originalCasterGUID )
{
    ASSERT( Caster != NULL && info != NULL );

    Player* p_caster;

    // make own copy of custom `info` (`info` can be created at stack) for non-triggered spell
    // copy custom SpellEntry in m_spellInfo will be delete at Spell delete
    if(info != sSpellStore.LookupEntry( info->Id ) && !triggered)
    {
        SpellEntry* sInfo = new SpellEntry;
        *sInfo = *info;
        m_spellInfo = sInfo;
    }
    else
        m_spellInfo = info;

    m_caster = Caster;

    if(originalCasterGUID)
        m_originalCasterGUID = originalCasterGUID;
    else if(Aur)
        m_originalCasterGUID = Aur->GetCasterGUID();
    else
        m_originalCasterGUID = m_caster->GetGUID();

    if(m_originalCasterGUID==m_caster->GetGUID())
        m_originalCaster = m_caster;
    else
        m_originalCaster = ObjectAccessor::Instance().GetUnit(*m_caster,m_originalCasterGUID);

    m_spellState = SPELL_STATE_NULL;

    m_castPositionX = m_castPositionY = m_castPositionZ = 0;
    m_TriggerSpells.clear();
    m_IsTriggeredSpell = triggered;
    //m_AreaAura = false;
    m_CastItem = NULL;

    unitTarget = NULL;
    itemTarget = NULL;
    gameObjTarget = NULL;
    focusObject = NULL;

    m_triggeredByAura = Aur;
    m_autoRepeat = false;
    if( m_spellInfo->AttributesEx2 == 0x000020 )            //Auto Shot & Shoot
        m_autoRepeat = true;

    casttime = GetCastTime(sCastTimesStore.LookupEntry(m_spellInfo->CastingTimeIndex));

    if( m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo )
    {
        p_caster = ((Player*)m_caster);
        ((Player*)m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_CASTING_TIME, casttime);
        casttime = int32(float(casttime)/(100+p_caster->m_modCastSpeedPct)*100);
    }

    m_timer = casttime<0?0:casttime;

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

Spell::~Spell()
{
    // free custom m_spellInfo for non-triggered spell
    if(m_spellInfo != sSpellStore.LookupEntry(m_spellInfo->Id) && !m_IsTriggeredSpell)
        delete m_spellInfo;
}

void Spell::FillTargetMap()
{
    // TODO: ADD the correct target FILLS!!!!!!

    for(uint32 i=0;i<3;i++)
    {
        // not call for empty effect.
        // Also some spells use not used effect targets for store targets for dummy effect in triggred spells
        if(m_spellInfo->Effect[i]==0)
            continue;

        m_targetGameobjectGUIDs[i].clear();
        m_targetUnitGUIDs[i].clear();
        m_targetItems[i].clear();

        std::list<Unit*> tmpUnitMap;

        SetTargetMap(i,m_spellInfo->EffectImplicitTargetA[i],tmpUnitMap);
        SetTargetMap(i,m_spellInfo->EffectImplicitTargetB[i],tmpUnitMap);

        if( (m_spellInfo->EffectImplicitTargetA[i]==0 || m_spellInfo->EffectImplicitTargetA[i]==18) && 
            (m_spellInfo->EffectImplicitTargetB[i]==0 || m_spellInfo->EffectImplicitTargetB[i]==18) )
        {
            // add here custom effects that need default target.
            // FOR EVERY TARGET TYPE THERE IS A DIFFERENT FILL!!
            switch(m_spellInfo->Effect[i])
            {
                //case SPELL_EFFECT_PERSISTENT_AREA_AURA:
                case SPELL_EFFECT_RESURRECT:
                case SPELL_EFFECT_LEARN_SPELL:
                case SPELL_EFFECT_SKILL_STEP:
                case SPELL_EFFECT_SELF_RESURRECT:
                case SPELL_EFFECT_PROFICIENCY:
                case SPELL_EFFECT_PARRY:
                case SPELL_EFFECT_DUMMY:
                case SPELL_EFFECT_CREATE_ITEM:
                case SPELL_EFFECT_SUMMON_PLAYER:
                    if(m_targets.getUnitTarget())
                        tmpUnitMap.push_back(m_targets.getUnitTarget());
                    break;
                case SPELL_EFFECT_RESURRECT_NEW:
                    if(m_targets.getUnitTarget())
                        tmpUnitMap.push_back(m_targets.getUnitTarget());
                    if(m_targets.getCorpseTargetGUID())
                    {
                        CorpsePtr& corpse = ObjectAccessor::Instance().GetCorpse(*m_caster,m_targets.getCorpseTargetGUID());
                        if(corpse)
                        {
                            Player* owner = ObjectAccessor::Instance().FindPlayer(corpse->GetOwnerGUID());
                            if(owner)
                                tmpUnitMap.push_back(owner);
                        }
                    }
                    break;
                case SPELL_EFFECT_SKILL:
                case SPELL_EFFECT_SUMMON_CHANGE_ITEM:
                case SPELL_EFFECT_SUMMON_GUARDIAN:
                case SPELL_EFFECT_STUCK:
                case SPELL_EFFECT_ADD_FARSIGHT:
                case SPELL_EFFECT_DESTROY_ALL_TOTEMS:
                case SPELL_EFFECT_SUMMON_DEMON:
                    tmpUnitMap.push_back(m_caster);
                    break;
                case SPELL_EFFECT_LEARN_PET_SPELL:
                    if(Pet* pet = m_caster->GetPet())
                        tmpUnitMap.push_back(pet);
                    break;
                case SPELL_EFFECT_FEED_PET:
                case SPELL_EFFECT_PROSPECTING:
                case SPELL_EFFECT_DISENCHANT:
                case SPELL_EFFECT_ENCHANT_ITEM:
                case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
                case SPELL_EFFECT_ENCHANT_HELD_ITEM:
                    if(m_targets.m_itemTarget)
                        m_targetItems[i].push_back(m_targets.m_itemTarget);
                    break;
                case SPELL_EFFECT_APPLY_AREA_AURA:
                    if(m_spellInfo->Attributes == 0x9050000 || m_spellInfo->Attributes == 0x10000)// AreaAura
                        SetTargetMap(i,TARGET_AREAEFFECT_PARTY,tmpUnitMap);
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

        // filter targets by immunity and creature type 
        uint32 SpellCreatureType = GetTargetCreatureTypeMask();

        for (std::list<Unit*>::iterator itr = tmpUnitMap.begin() ; itr != tmpUnitMap.end();)
        {
            // Check targets for creature type mask and remove not appropriate (skip explicit self target case, maybe need other explicit targets)
            if(m_spellInfo->EffectImplicitTargetA[i]!=TARGET_SELF )
            {
                if (SpellCreatureType)
                {
                    uint32 TargetCreatureType = (*itr)->GetCreatureTypeMask();
                    if(TargetCreatureType && !(SpellCreatureType & TargetCreatureType))
                    {
                        itr = tmpUnitMap.erase(itr);
                        continue;
                    }
                }
            }

            //Check targets for not_selectable unit flag and remove
            if ((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
            {
                itr = tmpUnitMap.erase(itr);
                continue;
            }

            //Check targets for immune and remove immunes targets
            if ((*itr)->IsImmunedToSpell(m_spellInfo))
            {
                // FIXME: this must be spell immune message instead melee attack message
                m_caster->SendAttackStateUpdate(HITINFO_NOACTION, *itr, 1, NORMAL_DAMAGE, 0, 0, 0, VICTIMSTATE_IS_IMMUNE, 0);
                itr = tmpUnitMap.erase(itr);
                continue;
            }

            // ok
            ++itr;
        }

        for(std::list<Unit*>::iterator iunit= tmpUnitMap.begin();iunit != tmpUnitMap.end();++iunit)
            m_targetUnitGUIDs[i].push_back((*iunit)->GetGUID());
    }
}

// Helper for Chain Healing
// Spell target first
// Raidmates then descending by injury suffered (MaxHealth - Health)
// Other players/mobs then descending by injury suffered (MaxHealth - Health)
struct ChainHealingOrder : public binary_function<const Unit*, const Unit*, bool>
{    
    const Unit* MainTarget;
    ChainHealingOrder(const Unit* Target) : MainTarget(Target) {};
    // functor for operator ">"
    bool operator()(const Unit* _Left, const Unit* _Right) const
    {    
        return (ChainHealingHash(_Left) < ChainHealingHash(_Right));
    }
    int32 ChainHealingHash(const Unit* Target) const
    {
        if (Target == MainTarget)
            return 0;
        else if (Target->GetTypeId() == TYPEID_PLAYER && MainTarget->GetTypeId() == TYPEID_PLAYER &&             
            ((Player const*)Target)->IsInSameRaidWith((Player const*)MainTarget))
        {
            if (Target->GetHealth() == Target->GetMaxHealth())
                return 40000;
            else
                return 20000 - Target->GetMaxHealth() + Target->GetHealth();
        }
        else
            return 40000 - Target->GetMaxHealth() + Target->GetHealth();
    }
};

class ChainHealingFullHealth: unary_function<const Unit*, bool>
{
public:
    const Unit* MainTarget;
    ChainHealingFullHealth(const Unit* Target) : MainTarget(Target) {};

    bool operator()(const Unit* Target)
    {
        return (Target != MainTarget && Target->GetHealth() == Target->GetMaxHealth());
    }
};

// Helper for targets nearest to the spell target
// The spell target is always first unless there is a target at _completely_ the same position (unbelievable case)
struct TargetDistanceOrder : public binary_function<const Unit, const Unit, bool>
{    
    const Unit* MainTarget;
    TargetDistanceOrder(const Unit* Target) : MainTarget(Target) {};
    // functor for operator ">"
    bool operator()(const Unit* _Left, const Unit* _Right) const
    {    
        return (MainTarget->GetDistanceSq(_Left) < MainTarget->GetDistanceSq(_Right));
    }
};

void Spell::SetTargetMap(uint32 i,uint32 cur,std::list<Unit*> &TagUnitMap)
{
    float radius;
    if (m_spellInfo->EffectRadiusIndex[i])
        radius = GetRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
    else
        radius = GetMaxRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));

    uint32 unMaxTargets = m_spellInfo->MaxAffectedTargets;
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
        case TARGET_CHAIN_DAMAGE:
        {
            Unit* pUnitTarget = m_targets.getUnitTarget();
            if(!pUnitTarget)
                break;
            if (m_spellInfo->EffectChainTarget[i] <= 1)
                TagUnitMap.push_back(pUnitTarget);
            else
            {
                unMaxTargets = m_spellInfo->EffectChainTarget[i];
                CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
                Cell cell = RedZone::GetZone(p);
                cell.data.Part.reserved = ALL_DISTRICT;
                cell.SetNoCreate();

                Unit* originalCaster = GetOriginalCaster();
                if(originalCaster)
                {
                    MaNGOS::AnyAoETargetUnitInObjectRangeCheck u_check(pUnitTarget, originalCaster, radius ? radius : 5);
                    MaNGOS::UnitListSearcher<MaNGOS::AnyAoETargetUnitInObjectRangeCheck> searcher(TagUnitMap, u_check);

                    TypeContainerVisitor<MaNGOS::UnitListSearcher<MaNGOS::AnyAoETargetUnitInObjectRangeCheck>, WorldTypeMapContainer > world_unit_searcher(searcher);
                    TypeContainerVisitor<MaNGOS::UnitListSearcher<MaNGOS::AnyAoETargetUnitInObjectRangeCheck>, GridTypeMapContainer >  grid_unit_searcher(searcher);

                    CellLock<GridReadGuard> cell_lock(cell, p);
                    cell_lock->Visit(cell_lock, world_unit_searcher, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
                    cell_lock->Visit(cell_lock, grid_unit_searcher, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));

                    // sort TagUnitMap  and then cut down to size
                    TagUnitMap.sort(TargetDistanceOrder(pUnitTarget));
                    if (TagUnitMap.size() > unMaxTargets)
                        TagUnitMap.resize(unMaxTargets);
                }
            }
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

                MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i, PUSH_DEST_CENTER,SPELL_TARGETS_AOE_DAMAGE);

                TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, WorldTypeMapContainer > world_object_notifier(notifier);
                TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, GridTypeMapContainer >  grid_object_notifier(notifier);

                CellLock<GridReadGuard> cell_lock(cell, p);
                cell_lock->Visit(cell_lock, world_object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
                cell_lock->Visit(cell_lock, grid_object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
            }
        }break;
        case TARGET_ALL_PARTY_AROUND_CASTER:
        {
            Unit* owner = m_caster->GetCharmerOrOwner();
            Group  *pGroup = NULL;
            Player *groupMember = NULL;

            if(owner)
            {
                if(owner->GetTypeId() == TYPEID_PLAYER)
                {
                    groupMember = (Player*)owner;
                    pGroup = ((Player*)owner)->GetGroup();
                }
            }
            else if (m_caster->GetTypeId() == TYPEID_PLAYER)
            {
                groupMember = (Player*)m_caster;
                pGroup = groupMember->GetGroup();
            }

            if(pGroup)
            {
                for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    Player* Target = itr->getSource();
                    if(Target && m_caster->IsWithinDistInMap(Target, radius))
                        TagUnitMap.push_back(Target);
                }
            }
            else if (owner)
            {
                if(m_caster->IsWithinDistInMap(owner, radius))
                    TagUnitMap.push_back(owner);
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

            MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i, PUSH_SELF_CENTER,SPELL_TARGETS_AOE_DAMAGE);

            TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, WorldTypeMapContainer > world_object_notifier(notifier);
            TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, GridTypeMapContainer >  grid_object_notifier(notifier);

            CellLock<GridReadGuard> cell_lock(cell, p);
            cell_lock->Visit(cell_lock, world_object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
            cell_lock->Visit(cell_lock, grid_object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
        }break;
        case TARGET_GAMEOBJECT:
        {
            if(m_targets.getGOTarget())
                m_targetGameobjectGUIDs[i].push_back(m_targets.getGOTargetGUID());
        }break;
        case TARGET_IN_FRONT_OF_CASTER:
        {
            CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
            Cell cell = RedZone::GetZone(p);
            cell.data.Part.reserved = ALL_DISTRICT;
            cell.SetNoCreate();

            MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i, PUSH_IN_FRONT,SPELL_TARGETS_AOE_DAMAGE);

            TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, WorldTypeMapContainer > world_object_notifier(notifier);
            TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, GridTypeMapContainer >  grid_object_notifier(notifier);

            CellLock<GridReadGuard> cell_lock(cell, p);
            cell_lock->Visit(cell_lock, world_object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
            cell_lock->Visit(cell_lock, grid_object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
        }break;
        case TARGET_DUELVSPLAYER:
        {
            if(m_targets.getUnitTarget())
                TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
        case TARGET_GAMEOBJECT_ITEM:
        {
            if(m_targets.getGOTargetGUID())
                m_targetGameobjectGUIDs[i].push_back(m_targets.getGOTargetGUID());
            else if(m_targets.m_itemTarget)
                m_targetItems[i].push_back(m_targets.m_itemTarget);
            break;
        }
        case TARGET_MASTER:
        {
            Unit* owner = m_caster->GetCharmerOrOwner();
            if(owner)
                TagUnitMap.push_back(owner);
            break;
        }
        case TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
        {
            // targets the ground, not the units in the area
            if (m_spellInfo->Effect[i]!=SPELL_EFFECT_PERSISTENT_AREA_AURA)
            {
                CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
                Cell cell = RedZone::GetZone(p);
                cell.data.Part.reserved = ALL_DISTRICT;
                cell.SetNoCreate();

                MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i, PUSH_DEST_CENTER,SPELL_TARGETS_AOE_DAMAGE);

                TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, WorldTypeMapContainer > world_object_notifier(notifier);
                TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, GridTypeMapContainer >  grid_object_notifier(notifier);

                CellLock<GridReadGuard> cell_lock(cell, p);
                cell_lock->Visit(cell_lock, world_object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
                cell_lock->Visit(cell_lock, grid_object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
            }
        }break;
        case TARGET_MINION:
        {
            if(m_spellInfo->Effect[i] != SPELL_EFFECT_DUEL) TagUnitMap.push_back(m_caster);
        }break;
        case TARGET_SINGLE_ENEMY:
        {
            if(m_targets.getUnitTarget())
                TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
        case TARGET_AREAEFFECT_PARTY:
        {
            Unit* owner = m_caster->GetCharmerOrOwner();
            Player *groupMember = NULL;
            Group  *pGroup = NULL;

            if(owner)
            {
                TagUnitMap.push_back(m_caster);
                if(owner->GetTypeId() == TYPEID_PLAYER)
                    groupMember = (Player*)owner;
            }
            else if (m_caster->GetTypeId() == TYPEID_PLAYER)
                groupMember = (Player*)m_caster;

            if (groupMember) pGroup = groupMember->GetGroup();
            
            if(pGroup)
            {
                for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    Player* Target = itr->getSource();
                    if(Target && m_caster->IsWithinDistInMap(Target, radius))
                        TagUnitMap.push_back(Target);
                }
            }
            else if (owner)
            {
                if(m_caster->IsWithinDistInMap(owner, radius))
                    TagUnitMap.push_back(owner);
            }
            else
                TagUnitMap.push_back(m_caster);
        }break;
        case TARGET_SELF_FISHING:
        {
            TagUnitMap.push_back(m_caster);
        }break;
        case TARGET_CHAIN_HEAL:
        {
            Unit* pUnitTarget = m_targets.getUnitTarget();
            if(!pUnitTarget)
                break;

            if (m_spellInfo->EffectChainTarget[i] <= 1)
                TagUnitMap.push_back(pUnitTarget);
            else
            {
                unMaxTargets = m_spellInfo->EffectChainTarget[i];
                CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
                Cell cell = RedZone::GetZone(p);
                cell.data.Part.reserved = ALL_DISTRICT;
                cell.SetNoCreate();

                MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, i, PUSH_SELF_CENTER, SPELL_TARGETS_FRIENDLY);

                TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, WorldTypeMapContainer > world_object_notifier(notifier);
                TypeContainerVisitor<MaNGOS::SpellNotifierCreatureAndPlayer, GridTypeMapContainer >  grid_object_notifier(notifier);

                CellLock<GridReadGuard> cell_lock(cell, p);
                cell_lock->Visit(cell_lock, world_object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
                cell_lock->Visit(cell_lock, grid_object_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));

                // sort TagUnitMap  and then cut down to size
                TagUnitMap.sort(ChainHealingOrder(pUnitTarget));
                if (TagUnitMap.size() > unMaxTargets)
                    TagUnitMap.resize(unMaxTargets);
                TagUnitMap.remove_if(ChainHealingFullHealth(pUnitTarget));
            }
        }break;
        case TARGET_CURRENT_SELECTED_ENEMY:
        {
            CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
            Cell cell = RedZone::GetZone(p);
            cell.data.Part.reserved = ALL_DISTRICT;
            cell.SetNoCreate();
            MaNGOS::SpellNotifierPlayer notifier(*this, TagUnitMap, i);
            TypeContainerVisitor<MaNGOS::SpellNotifierPlayer, WorldTypeMapContainer > player_notifier(notifier);
            CellLock<GridReadGuard> cell_lock(cell, p);
            cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));
        }break;
        default:
        {
        }break;
        case TARGET_AREAEFFECT_PARTY_AND_CLASS:
        {
            Player* targetPlayer = m_targets.getUnitTarget() && m_targets.getUnitTarget()->GetTypeId() == TYPEID_PLAYER
                ? (Player*)m_targets.getUnitTarget() : NULL;

            Group* pGroup = targetPlayer ? targetPlayer->GetGroup() : NULL;
            if(pGroup)
            {
                for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    Player* Target = itr->getSource();
                    if(Target && targetPlayer->IsWithinDistInMap(Target, radius) && targetPlayer->getClass() == Target->getClass())
                        TagUnitMap.push_back(Target);
                }
            }
            else if(m_targets.getUnitTarget())
                TagUnitMap.push_back(m_targets.getUnitTarget());
        }break;
    }

    if (unMaxTargets && TagUnitMap.size() > unMaxTargets)
    {
        // make sure one unit is always removed per iteration
        uint32 removed_utarget = 0;
        for (std::list<Unit*>::iterator itr = TagUnitMap.begin(), next; itr != TagUnitMap.end(); itr = next)
        {
            next = itr;
            next++;
            if (!*itr) continue;
            if ((*itr) == m_targets.getUnitTarget())
            {
                TagUnitMap.erase(itr);
                removed_utarget = 1;
                //        break;
            }
        }
        // remove random units from the map
        while (TagUnitMap.size() > unMaxTargets - removed_utarget)
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
        SendCastResult(result);
        finish(false);
        return;
    }

    // stealth must be removed at cast starting (at show channel bar)
    // skip trigered spell (item equip spell casting and other not explicit character casts/item uses)
    if ( !m_IsTriggeredSpell && !CanUsedWhileStealthed(m_spellInfo->Id) )
        m_caster->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);

    // do first cast of autorepeat spell with recovery time delay (like after any autocast)
    if(IsAutoRepeat())
        m_spellState = SPELL_STATE_FINISHED;

    if(m_IsTriggeredSpell)
        cast(true);
    else
    {
        m_caster->SetCurrentCastedSpell( this );
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
        SendCastResult(SPELL_FAILED_INTERRUPTED);
    }
    else if(m_spellState == SPELL_STATE_CASTING)
    {
        for (int j = 0; j < 3; j++)
        {
            for(std::list<uint64>::iterator iunit= m_targetUnitGUIDs[j].begin();iunit != m_targetUnitGUIDs[j].end();++iunit)
            {
                // check m_caster->GetGUID() let load auras at login and speedup most often case
                Unit* unit = m_caster->GetGUID()==*iunit ? m_caster : ObjectAccessor::Instance().GetUnit(*m_caster,*iunit);
                if (unit && unit->isAlive())
                    unit->RemoveAurasDueToSpell(m_spellInfo->Id);
            }
        }

        m_caster->RemoveAurasDueToSpell(m_spellInfo->Id);
        SendChannelUpdate(0);
        SendInterrupted(0);
        SendCastResult(SPELL_FAILED_INTERRUPTED);
    }

    finish(false);
    m_caster->RemoveDynObject(m_spellInfo->Id);
    m_caster->RemoveGameObject(m_spellInfo->Id,true);
}

void Spell::cast(bool skipCheck)
{
    uint32 mana = 0;
    uint8 castResult = 0;

    // update pointers base at GUIDs to prevent access to non-existed already object
    UpdatePointers();

    // cancel at lost main target unit
    if(!m_targets.getUnitTarget() && m_targets.getUnitTargetGUID() && m_targets.getUnitTargetGUID() != m_caster->GetGUID())
    {
        cancel();
        return;
    }

    if(m_caster->GetTypeId() != TYPEID_PLAYER && m_targets.getUnitTarget())
        m_caster->SetInFront(m_targets.getUnitTarget());

    castResult = CheckMana( &mana);
    if(castResult != 0)
    {
        SendCastResult(castResult);
        finish(false);
        return;
    }

    // triggered cast called from Spell::prepare where it was already checked
    if(!skipCheck)
    {
        castResult = CanCast();
        if(castResult != 0)
        {
            SendCastResult(castResult);
            finish(false);
            return;
        }
    }

    // remove spell from another target if need
    // hunter's mark
    if(m_spellInfo->SpellVisual == 3239)
    {
        Unit::AuraList& scAuras = m_caster->GetSingleCastAuras();
        for(Unit::AuraList::iterator itr = scAuras.begin(); itr != scAuras.end(); ++itr)
        {
            if((*itr)->GetSpellProto()->SpellVisual == 3239)
            {
                (*itr)->GetTarget()->RemoveAura((*itr)->GetId(), (*itr)->GetEffIndex());
                break;
            }
        }
    }

    // Conflagrate - consumes immolate
    if ((m_spellInfo->TargetAuraState == AURA_STATE_IMMOLATE) && m_targets.getUnitTarget())
    {
        Unit::AuraList const &mPeriodic = m_targets.getUnitTarget()->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
        for(Unit::AuraList::const_iterator i = mPeriodic.begin(); i != mPeriodic.end(); ++i)
            if ((*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_WARLOCK && ((*i)->GetSpellProto()->SpellFamilyFlags & 4))
            {
                m_targets.getUnitTarget()->RemoveAura((*i)->GetId(), (*i)->GetEffIndex());
                break;
            }
    }

    // traded items have trade slot instead of guid in m_itemTargetGUID
    // set to real guid to be sent later to the client
    if(m_targets.m_itemTarget && (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM))
        m_targets.setItemTargetGUID(m_targets.m_itemTarget->GetGUID());

    // CAST SPELL
    SendSpellCooldown();

    TakePower(mana);
    TakeReagents();                                         // we must remove reagents before HandleEffects to allow place crafted item in same slot
    FillTargetMap();
    SendCastResult(castResult);
    SendSpellGo();                                          // we must send smsg_spell_go packet before m_castItem delete in TakeCastItem()...
    TakeCastItem();                                         // we must remove consumed cast item before HandleEffects to allow place crafted item in same slot

    if(IsChanneledSpell())
    {
        m_spellState = SPELL_STATE_CASTING;
        SendChannelStart(GetDuration(m_spellInfo));
    }

    // Pass cast spell event to handler (not send triggered by aura spells)
    if (m_spellInfo->DmgClass != SPELL_DAMAGE_CLASS_MELEE && m_spellInfo->DmgClass != SPELL_DAMAGE_CLASS_RANGED && !m_triggeredByAura)
        m_caster->ProcDamageAndSpell(m_caster->getVictim(), PROC_FLAG_CAST_SPELL, PROC_FLAG_NONE, 0, m_spellInfo, m_IsTriggeredSpell);

    HandleThreatSpells(m_spellInfo->Id);

    bool needspelllog = true;
    for(uint32 j = 0;j<3;j++)
    {
        if(m_spellInfo->Effect[j]==0)
            continue;

        if(m_spellInfo->Effect[j] == SPELL_EFFECT_SEND_EVENT)
        {
            HandleEffects(NULL,NULL,NULL, j);
            continue;
        }
                                                            // Dont do spell log, if is school damage spell
        if(m_spellInfo->Effect[j] == SPELL_EFFECT_SCHOOL_DAMAGE || m_spellInfo->Effect[j] == 0)
            needspelllog = false;
        float DamageMultiplier = 1.0;
        bool ApplyDamageMultiplier = 
            (m_spellInfo->EffectImplicitTargetA[j] == TARGET_CHAIN_DAMAGE || m_spellInfo->EffectImplicitTargetA[j] == TARGET_CHAIN_HEAL)
            && (m_spellInfo->EffectChainTarget[j] > 1);
        for(std::list<uint64>::iterator iunit= m_targetUnitGUIDs[j].begin();iunit != m_targetUnitGUIDs[j].end();++iunit)
        {
            // let the client worry about this
            /*if((*iunit)->GetTypeId() != TYPEID_PLAYER && m_spellInfo->TargetCreatureType)
            {
                CreatureInfo const *cinfo = ((Creature*)(*iunit))->GetCreatureInfo();
                if((m_spellInfo->TargetCreatureType & cinfo->type) == 0)
                    continue;
            }*/
            // check m_caster->GetGUID() let load auras at login and speedup most often case
            Unit* unit = m_caster->GetGUID()==*iunit ? m_caster : ObjectAccessor::Instance().GetUnit(*m_caster,*iunit);
            if(unit)
            {
                HandleEffects(unit,NULL,NULL,j, DamageMultiplier);
                
                if ( ApplyDamageMultiplier )
                    DamageMultiplier *= m_spellInfo->DmgMultiplier[j];

                //Call scripted function for AI if this spell is casted upon a creature
                // Some Script Spell Destroy the target or something and the target is always the player
                // then re-find it in grids if this creature
                if(GUID_HIPART(*iunit)==HIGHGUID_UNIT)
                {
                    Creature* creature = m_caster->GetGUID()==*iunit ? (Creature*)m_caster : ObjectAccessor::Instance().GetCreature(*m_caster,*iunit);
                    if( creature && creature->AI() )
                        creature->AI()->SpellHit(m_caster,m_spellInfo);
                }
            }
        }

        for(std::list<Item*>::iterator iitem = m_targetItems[j].begin();iitem != m_targetItems[j].end();iitem++)
            HandleEffects(NULL,(*iitem),NULL,j);

        for(std::list<uint64>::iterator igo= m_targetGameobjectGUIDs[j].begin();igo != m_targetGameobjectGUIDs[j].end();++igo)
        {
            GameObject* go = ObjectAccessor::Instance().GetGameObject(*m_caster,*igo);
            if(go)
                HandleEffects(NULL,NULL,go,j);
        }

        // persistent area auras target only the ground
        if(m_spellInfo->Effect[j] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
            HandleEffects(NULL,NULL,NULL, j);
    }

    if(needspelllog)
        SendLogExecute();

    //remove spell mods
    if (m_caster->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_caster)->RemoveSpellMods(m_spellInfo->Id);

    bool canreflect = false;
    for(int j=0;j<3;j++)
    {
        if(m_spellInfo->Effect[j]==0)
            continue;

        switch(m_spellInfo->EffectImplicitTargetA[j])
        {
            case TARGET_CHAIN_DAMAGE:
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
        else
            break;
    }

    if(canreflect)
    {
        // store already processed targets to skip repeated targets
        std::set<uint64> SkipTargets;

        for(int k=0;k<3;k++)
        {
            if(m_spellInfo->Effect[k]==0)
                continue;

            for(std::list<uint64>::iterator iunit= m_targetUnitGUIDs[k].begin();iunit != m_targetUnitGUIDs[k].end();++iunit)
            {
                if(SkipTargets.count(*iunit) > 0)
                    continue;

                // check m_caster->GetGUID() let load auras at login and speedup most often case
                Unit* unit = m_caster->GetGUID()==*iunit ? m_caster : ObjectAccessor::Instance().GetUnit(*m_caster,*iunit);
                if(unit)
                    reflect(unit);

                SkipTargets.insert(*iunit);
            }
        }
    }

    if(m_spellState != SPELL_STATE_CASTING)
        finish(true);                                       // successfully finish spell cast (not last in case autorepeat or channel spell)
}

void Spell::SendSpellCooldown()
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* _player = (Player*)m_caster;

    // init cooldown values
    uint32 cat   = 0;
    int32 rec    = 0;
    int32 catrec = 0;

    // some special item spells without correct cooldown in SpellInfo
    // cooldown information stored in item prototype
    // This used in same way in WorldSession::HandleItemQuerySingleOpcode data sending to client.

    if(m_CastItem)
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

    // if no cooldown found above then base at DBC data
    if(!rec && !catrec)
    {
        cat = m_spellInfo->Category;
        rec = m_spellInfo->RecoveryTime;
        catrec = m_spellInfo->CategoryRecoveryTime;
    }

    // shoot spells used equiped item cooldown values already assigned in GetAttackTime(RANGED_ATTACK)
    if (!rec && !catrec && (cat == 76 || cat == 351))
        rec = _player->GetAttackTime(RANGED_ATTACK);

    // Now we have cooldown data (if found any), time to apply mods
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

    // self spell cooldown
    if (rec > 0)
    {
        // only send if different from client known cooldown
        if(m_spellInfo->RecoveryTime != rec)
        {
            /*WorldPacket data(SMSG_SPELL_COOLDOWN, (8+1+4+4));
            data << m_caster->GetGUID();
            data << uint8(0x0);
            data << uint32(m_spellInfo->Id);
            data << uint32(rec);
            _player->GetSession()->SendPacket(&data);*/
        }
        if(m_CastItem)
            _player->AddSpellCooldown(m_spellInfo->Id, m_CastItem->GetEntry(), recTime);
        else
            _player->AddSpellCooldown(m_spellInfo->Id, 0, recTime);
    }
    else
    {
        // only send if different from client known cooldown
        if(m_spellInfo->RecoveryTime && m_spellInfo->RecoveryTime != catrec || m_spellInfo->CategoryRecoveryTime != catrec)
        {
            /*WorldPacket data(SMSG_SPELL_COOLDOWN, (8+1+4+4));
            data << m_caster->GetGUID();
            data << uint8(0x0);
            data << uint32(m_spellInfo->Id);
            data << uint32(catrec);
            _player->GetSession()->SendPacket(&data);*/
        }
        if(m_CastItem)
            _player->AddSpellCooldown(m_spellInfo->Id, m_CastItem->GetEntry(), catrecTime);
        else
            _player->AddSpellCooldown(m_spellInfo->Id, 0, catrecTime);
    }

    if (catrec)
    {
        SpellCategoryStore::const_iterator i_scstore = sSpellCategoryStore.find(cat);
        if(i_scstore != sSpellCategoryStore.end())
        {
            for(SpellCategorySet::const_iterator i_scset = i_scstore->second.begin(); i_scset != i_scstore->second.end(); ++i_scset)
            {
                if(*i_scset == m_spellInfo->Id)             // skip casted spell
                    continue;

                // only send if different from client known cooldown
                if(cat != m_spellInfo->Category || m_spellInfo->CategoryRecoveryTime != catrec)
                {
                    /*WorldPacket data(SMSG_SPELL_COOLDOWN, (8+1+4+4));
                    data << m_caster->GetGUID();
                    data << uint8(0x0);
                    data << uint32(*i_scset);
                    data << uint32(catrec);
                    _player->GetSession()->SendPacket(&data);*/
                }
                if(m_CastItem)
                    _player->AddSpellCooldown(*i_scset, m_CastItem->GetEntry(), catrecTime);
                else
                    _player->AddSpellCooldown(*i_scset, 0, catrecTime);
            }
        }
    }
}

void Spell::update(uint32 difftime)
{
    // update pointers based at it's GUIDs
    UpdatePointers();

    if(m_targets.getUnitTargetGUID())
    {
        if(!m_targets.getUnitTarget() || !m_targets.getUnitTarget()->isAlive())
        {
            if(m_autoRepeat)
            {
                m_autoRepeat = false;
                m_spellState = SPELL_STATE_FINISHED;
                return;
            }

            if(!m_targets.getUnitTarget())
            {
                cancel();
                return;
            }
        }
    }

    // check if the player caster has moved before the spell finished
    if ((m_caster->GetTypeId() == TYPEID_PLAYER && m_timer != 0) &&
        (m_castPositionX != m_caster->GetPositionX() || m_castPositionY != m_caster->GetPositionY() || m_castPositionZ != m_caster->GetPositionZ()) &&
        (m_spellInfo->Effect[0] != SPELL_EFFECT_STUCK || !((Player*)m_caster)->HasMovementFlags(MOVEMENTFLAG_FALLING)) )
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
                    if( m_caster->hasUnitState(UNIT_STAT_STUNDED | UNIT_STAT_ROOT | UNIT_STAT_CONFUSED) )
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
                        for(std::list<uint64>::iterator iunit= m_targetUnitGUIDs[i].begin();iunit != m_targetUnitGUIDs[i].end();++iunit)
                        {
                            // check m_caster->GetGUID() let load auras at login and speedup most often case
                            Unit* unit = m_caster->GetGUID()==*iunit ? m_caster : ObjectAccessor::Instance().GetUnit(*m_caster,*iunit);
                            if(unit && unit->isAlive())
                            {
                                targetLeft = true;
                                break;
                            }
                            if(!targetLeft)
                                cancel();
                        }
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

void Spell::finish(bool ok)
{

    if(!m_caster) return;

    if(m_spellState == SPELL_STATE_FINISHED)
        return;

    m_spellState = SPELL_STATE_FINISHED;
    m_caster->m_canMove = true;

    /*std::vector<DynamicObject*>::iterator i;
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

    // other code realated only to successfully finished spells
    if(!ok)
        return;

    // cast at creature (or GO) quest objectives update at succesful cast finished (+channel finished)
    // ignore autorepeat/melee casts for speed (not exist quest for spells (hm... )
    if( m_caster->GetTypeId() == TYPEID_PLAYER && !IsAutoRepeat() && !IsMeleeSpell() && !IsChannelActive() )
    {
        if( m_targets.getUnitTargetGUID() && GUID_HIPART(m_targets.getUnitTargetGUID())==HIGHGUID_UNIT )
        {
            // Some Script Spell Destroy the target or something and the target is always the player
            // then re-find it in grids if this creature
            Creature* creature = m_caster->GetGUID()==m_targets.getUnitTargetGUID() ? (Creature*)m_caster : ObjectAccessor::Instance().GetCreature(*m_caster,m_targets.getUnitTargetGUID());
            if( creature )
                ((Player*)m_caster)->CastedCreatureOrGO(creature->GetEntry(),creature->GetGUID(),m_spellInfo->Id);
        }

        if( m_targets.getGOTarget() )
        {
            ((Player*)m_caster)->CastedCreatureOrGO(m_targets.getGOTarget()->GetEntry(),m_targets.getGOTarget()->GetGUID(),m_spellInfo->Id);
        }
    }

    // call triggered spell only at successful cast
    if(m_TriggerSpells.size() > 0)
        TriggerSpell();

    //handle SPELL_AURA_ADD_TARGET_TRIGGER auras
    Unit::AuraList const& targetTriggers = m_caster->GetAurasByType(SPELL_AURA_ADD_TARGET_TRIGGER);
    for(Unit::AuraList::const_iterator i = targetTriggers.begin(); i != targetTriggers.end(); ++i)
        if (IsAffectedBy((*i)->GetSpellProto(),(*i)->GetEffIndex()))
            for(std::list<uint64>::iterator iunit= m_targetUnitGUIDs[(*i)->GetEffIndex()].begin();iunit != m_targetUnitGUIDs[(*i)->GetEffIndex()].end();++iunit)
            {
                // check m_caster->GetGUID() let load auras at login and speedup most often case
                Unit* unit = m_caster->GetGUID()==*iunit ? m_caster : ObjectAccessor::Instance().GetUnit(*m_caster,*iunit);
                if (unit && unit->isAlive() && roll_chance_f((*i)->GetModifier()->m_amount))
                    m_caster->CastSpell(unit,(*i)->GetSpellProto()->EffectTriggerSpell[(*i)->GetEffIndex()],true,NULL,(*i));
            }

    if (IsMeleeAttackResetSpell())
    {
        m_caster->resetAttackTimer(BASE_ATTACK);
        if(m_caster->haveOffhandWeapon())
            m_caster->resetAttackTimer(OFF_ATTACK);
    }
}

void Spell::SendCastResult(uint8 result)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if(((Player*)m_caster)->GetSession()->PlayerLoading()) // don't send cast results at loading time
        return;

    if(result != 0)
    {
        WorldPacket data(SMSG_CAST_RESULT, (4+2));
        data << m_spellInfo->Id;
        data << uint8(result);                              // problem
        switch (result)
        {
            case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
                data << uint32(m_spellInfo->RequiresSpellFocus);
                break;
            case SPELL_FAILED_REQUIRES_AREA:
                data << uint32(m_spellInfo->AreaId);
                break;
        }
        ((Player*)m_caster)->GetSession()->SendPacket(&data);
    }
    else
    {
        WorldPacket data(SMSG_CAST_SUCCESS, (8+4));
        data.append(m_caster->GetPackGUID());
        data << m_spellInfo->Id;
        ((Player*)m_caster)->GetSession()->SendPacket(&data);
    }
}

void Spell::SendSpellStart()
{
    sLog.outDebug("Sending SMSG_SPELL_START id=%u",m_spellInfo->Id);

    m_castFlags = CAST_FLAG_UNKNOWN1;
    if(m_rangedShoot)
        m_castFlags = m_castFlags | CAST_FLAG_AMMO;

    Unit * target;
    if(!m_targets.getUnitTarget())
        target = m_caster;
    else
        target = m_targets.getUnitTarget();

    WorldPacket data(SMSG_SPELL_START, (8+8+4+4+2));
    if(m_CastItem)
        data.append(m_CastItem->GetPackGUID());
    else
        data.append(m_caster->GetPackGUID());

    data.append(m_caster->GetPackGUID());
    data << uint32(m_spellInfo->Id);
    data << uint16(m_castFlags);
    data << uint32(m_timer);

    data << uint16(m_targets.m_targetMask);
    m_targets.write( &data );
    if( m_castFlags & CAST_FLAG_AMMO )
    {
        writeAmmoToPacket(&data);
    }
    m_caster->SendMessageToSet(&data, true);
}

void Spell::SendSpellGo()
{
    sLog.outDebug("Sending SMSG_SPELL_GO id=%u",m_spellInfo->Id);

    Unit * target;
    if(!m_targets.getUnitTarget())
        target = m_caster;
    else
        target = m_targets.getUnitTarget();

    m_castFlags = CAST_FLAG_UNKNOWN3;
    if(m_rangedShoot)
        m_castFlags = m_castFlags | CAST_FLAG_AMMO;

    WorldPacket data(SMSG_SPELL_GO, (50));                  // guess size
    if(m_CastItem)
        data.append(m_CastItem->GetPackGUID());
    else
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

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {
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
    }
    // TODO: implement selection ammo data based at ranged weapon stored in equipmodel/equipinfo/equipslot fields

    *data << ammoDisplayID;
    *data << ammoInventoryType;
}

void Spell::writeSpellGoTargets( WorldPacket * data )
{
    // List of all targets that aren't repeated. (Unique)
    std::list<Unit*> UniqueTargets;
    std::list<GameObject*> UniqueGOsTargets;

    for(int k=0;k<3;k++)
    {
        for(std::list<uint64>::iterator iunit= m_targetUnitGUIDs[k].begin();iunit != m_targetUnitGUIDs[k].end();++iunit)
        {
            bool add = true;

            for(std::list<Unit*>::iterator junit = UniqueTargets.begin(); junit != UniqueTargets.end(); junit++ )
            {
                if((*junit)->GetGUID() == (*iunit))
                {
                    add = false;
                    break;
                }
            }

            if(add)
            {
                // check m_caster->GetGUID() let load auras at login and speedup most often case
                Unit* unit = m_caster->GetGUID()==*iunit ? m_caster : ObjectAccessor::Instance().GetUnit(*m_caster,*iunit);
                if(unit)
                    UniqueTargets.push_back(unit);
            }
        }

        for(std::list<uint64>::iterator igo= m_targetGameobjectGUIDs[k].begin();igo != m_targetGameobjectGUIDs[k].end();++igo)
        {
            bool add = true;

            for(std::list<GameObject*>::iterator n = UniqueGOsTargets.begin(); n != UniqueGOsTargets.end(); n++ )
            {
                if((*n)->GetGUID() == (*igo))
                {
                    add = false;
                    break;
                }
            }
            if(add)
            {
                GameObject* go = ObjectAccessor::Instance().GetGameObject(*m_caster,*igo);
                if(go)
                    UniqueGOsTargets.push_back(go);
            }
        }
    }

    uint8 targetCount = UniqueTargets.size() + UniqueGOsTargets.size();
    *data << (uint8)targetCount;

    for ( std::list<Unit*>::iterator ui = UniqueTargets.begin(); ui != UniqueTargets.end(); ui++ )
        *data << (*ui)->GetGUID();

    for ( std::list<GameObject*>::iterator uj = UniqueGOsTargets.begin(); uj != UniqueGOsTargets.end(); uj++ )
        *data << (*uj)->GetGUID();

}

void Spell::SendLogExecute()
{
    Unit * target;
    if(!m_targets.getUnitTarget())
        target = m_caster;
    else
        target = m_targets.getUnitTarget();

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
        data << m_targets.getUnitTargetGUID();
    else if(m_targets.m_itemTarget)
        data << m_targets.getItemTargetGUID();
    else if(m_targets.getGOTarget())
        data << m_targets.getGOTargetGUID();

    m_caster->SendMessageToSet(&data,true);
}

void Spell::SendInterrupted(uint8 result)
{
    WorldPacket data(SMSG_SPELL_FAILURE, (8+4+1));
    data.append(m_caster->GetPackGUID());
    data << m_spellInfo->Id;
    data << result;
    m_caster->SendMessageToSet(&data, true);

    data.Initialize(SMSG_SPELL_FAILED_OTHER, (8+4));
    data.append(m_caster->GetPackGUID());
    data << m_spellInfo->Id;
    m_caster->SendMessageToSet(&data, true);
}

void Spell::SendChannelUpdate(uint32 time)
{
    if(time == 0)
    {
        m_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT,0);
        m_caster->SetUInt32Value(UNIT_CHANNEL_SPELL,0);
    }

    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data( MSG_CHANNEL_UPDATE, 8+4 );
    data.append(m_caster->GetPackGUID());
    data << time;

    ((Player*)m_caster)->GetSession()->SendPacket( &data );
}

void Spell::SendChannelStart(uint32 duration)
{
    Unit* target = 0;

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {

        target = ObjectAccessor::Instance().GetUnit(*m_caster, ((Player *)m_caster)->GetSelection());

        WorldPacket data( MSG_CHANNEL_START, (8+4+4) );
        data.append(m_caster->GetPackGUID());
        data << m_spellInfo->Id;
        data << duration;

        ((Player*)m_caster)->GetSession()->SendPacket( &data );
    }

    m_timer = duration;
    if(target)
        m_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, target->GetGUID());
    m_caster->SetUInt32Value(UNIT_CHANNEL_SPELL, m_spellInfo->Id);
}

void Spell::SendResurrectRequest(Player* target)
{
    WorldPacket data(SMSG_RESURRECT_REQUEST, (8+4+2+4));
    data << m_caster->GetGUID();
    data << uint32(1) << uint16(0) << uint32(1);

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

    for (int i = 0; i<5; i++)
    {
        if (proto->Spells[i].SpellId)
        {
            // item has limited charges
            if (proto->Spells[i].SpellCharges)
            {
                if (proto->Spells[i].SpellCharges < 0)
                    expendable = true;

                int32 charges = m_CastItem->GetSpellCharges(i);

                // item has charges left
                if (charges)
                {
                    (charges > 0) ? charges-- : charges++;  // abs(charges) less at 1 after use
                    if (proto->Stackable < 2)
                        m_CastItem->SetSpellCharges(i, charges);
                    m_CastItem->SetState(ITEM_CHANGED, (Player*)m_caster);
                }

                // all charges used
                withoutCharges = (charges == 0);
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

        if(m_CastItem && m_CastItem->GetProto()->ItemId == itemid)
        {
            itemcount += 1;
            m_CastItem = NULL;
        }

        if( p_caster->HasItemCount(itemid,itemcount) )
        {
            p_caster->DestroyItemCount(itemid, itemcount, true);
        }
        else
        {
            SendCastResult(SPELL_FAILED_ITEM_NOT_READY);
            return;
        }
    }
}

void Spell::HandleThreatSpells(uint32 spellId)
{
    if(!m_targets.getUnitTarget() || !spellId)
        return;

    if(!m_targets.getUnitTarget()->CanHaveThreatList())
        return;

    SpellThreatEntry const *threatSpell = sSpellThreatStore.LookupEntry<SpellThreatEntry>(spellId);
    if(!threatSpell)
        return;

    m_targets.getUnitTarget()->AddThreat(m_caster, float(threatSpell->threat));

    DEBUG_LOG("Spell %u, rank %u, added an additional %i threat", spellId, objmgr.GetSpellRank(spellId), threatSpell->threat);
}

void Spell::HandleEffects(Unit *pUnitTarget,Item *pItemTarget,GameObject *pGOTarget,uint32 i, float DamageMultiplier)
{
    unitTarget = pUnitTarget;
    itemTarget = pItemTarget;
    gameObjTarget = pGOTarget;

    damage = uint32(CalculateDamage((uint8)i)*DamageMultiplier);
    uint8 eff = m_spellInfo->Effect[i];

    sLog.outDebug( "Spell: Effect : %u", eff);
    if(unitTarget && unitTarget->IsImmunedToSpellEffect(eff))
    {
        SendCastResult(SPELL_FAILED_IMMUNE);
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

void Spell::TriggerSpell()
{
    if(m_TriggerSpells.size() < 1) return;

    for(std::list<SpellEntry const*>::iterator si=m_TriggerSpells.begin(); si!=m_TriggerSpells.end(); ++si)
    {
        Spell spell(m_caster, (*si), true, 0);
        spell.prepare(&m_targets);                          // use original spell original targets
    }

}

uint8 Spell::CanCast()
{
    // check cooldowns to prevent cheating
    if(m_caster->GetTypeId()==TYPEID_PLAYER && ((Player*)m_caster)->HasSpellCooldown(m_spellInfo->Id))
        return SPELL_FAILED_NOT_READY;

    // cancel autorepeat spells if cast start when moving
    // (not wand currently autorepeat cast delayed to moving stop anyway in spell update code)
    if( m_caster->GetTypeId()==TYPEID_PLAYER && ((Player*)m_caster)->GetMovementFlags() )
    {
        // skip stuck spell to allow use it in falling case and apply spell limitations at movement
        if( (!((Player*)m_caster)->HasMovementFlags(MOVEMENTFLAG_FALLING) || m_spellInfo->Effect[0] != SPELL_EFFECT_STUCK) &&
            (IsAutoRepeat() || m_rangedShoot || (m_spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED) != 0) )
            return SPELL_FAILED_MOVING;
    }

    Unit *target = m_targets.getUnitTarget();

    if(target)
    {
        if(VMAP::VMapFactory::checkSpellForLoS(m_spellInfo->Id) && !m_caster->IsWithinLOSInMap(target)) return SPELL_FAILED_LINE_OF_SIGHT;
        //TODO: after switch in Cast::preper (?) need implement auto-selecting appropriate cast level.
        if(m_caster->GetTypeId() == TYPEID_PLAYER && !IsPassiveSpell(m_spellInfo->Id) && !m_CastItem)
        {
            for(int i=0;i<3;i++)
            {
                if(IsPositiveEffect(m_spellInfo->Id, i) && m_spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA)
                    if(target->getLevel() + 10 < m_spellInfo->spellLevel)
                        return SPELL_FAILED_LOWLEVEL;
            }
        }

        // check pet presents
        for(int j=0;j<3;j++)
        {
            if(m_spellInfo->EffectImplicitTargetA[j] == TARGET_PET)
            {
                target = m_caster->GetPet();
                if(!target)
                    return SPELL_FAILED_NO_PET;

                break;
            }
        }

        //check creature type
        //ignore self casts (including area casts when caster selected as target)
        if(target != m_caster)
        {
            uint32 SpellCreatureType = GetTargetCreatureTypeMask();

            if(SpellCreatureType)
            {
                uint32 TargetCreatureType = target->GetCreatureTypeMask();

                if(TargetCreatureType && !(SpellCreatureType & TargetCreatureType))
                {
                    if(TargetCreatureType == 0x40)
                        return SPELL_FAILED_TARGET_IS_PLAYER;
                    else
                        return SPELL_FAILED_BAD_TARGETS;
                }
            }
        }

        // TODO: this check can be applied and for player to prevent cheating when IsPositiveSpell will return always correct result.
        // check target for pet/charmed casts (not self targeted), self targeted cast used for area effects and etc
        if(m_caster != target && m_caster->GetTypeId()==TYPEID_UNIT && m_caster->GetCharmerOrOwnerGUID())
        {
            // check correctness positive/negative cast target (pet cast real check and cheating check)
            if(IsPositiveSpell(m_spellInfo->Id))
            {
                if(m_caster->IsHostileTo(target))
                    return SPELL_FAILED_BAD_TARGETS;
            }
            else
            {
                if(m_caster->IsFriendlyTo(target))
                    return SPELL_FAILED_BAD_TARGETS;
            }
        }

        if(IsPositiveSpell(m_spellInfo->Id))
        {
            if(target->IsImmunedToSpell(m_spellInfo))
                return SPELL_FAILED_TARGET_AURASTATE;
        }

        /* Causes problems with berserking
        if (m_spellInfo->CasterAuraState && !(m_caster->HasFlag(UNIT_FIELD_AURASTATE, (1<<(m_spellInfo->CasterAuraState-1)))))
            return SPELL_FAILED_CASTER_AURASTATE;

        if (m_spellInfo->TargetAuraState && !(target->HasFlag(UNIT_FIELD_AURASTATE, (1<<(m_spellInfo->TargetAuraState-1)))))
            return SPELL_FAILED_TARGET_AURASTATE;*/

        //Must be behind the target.
        if( m_spellInfo->AttributesEx2 == 0x100000 && (m_spellInfo->AttributesEx & 0x200) == 0x200 && target->HasInArc(M_PI, m_caster) )
        {
            SendInterrupted(2);
            return SPELL_FAILED_NOT_BEHIND;
        }

        //Target must be facing you.
        if((m_spellInfo->Attributes == 0x150010) && !target->HasInArc(M_PI, m_caster) )
        {
            SendInterrupted(2);
            return SPELL_FAILED_NOT_INFRONT;
        }
    }

    if(m_caster->hasUnitState(UNIT_STAT_CONFUSED))
        return SPELL_FAILED_CONFUSED;

    if((m_spellInfo->AttributesEx3 & 0x800) != 0) // need check...
        return SPELL_FAILED_ONLY_BATTLEGROUNDS;

    if(m_spellInfo->AreaId && m_spellInfo->AreaId != m_caster->GetZoneId())
        return SPELL_FAILED_REQUIRES_AREA;

    if(m_caster->hasUnitState(UNIT_STAT_STUNDED))
        return SPELL_FAILED_STUNNED;

    // not let players cast non-triggered spells at mount (and let do it to creatures)
    if(m_caster->IsMounted() && !m_IsTriggeredSpell && m_caster->GetTypeId()==TYPEID_PLAYER)
        return SPELL_FAILED_NOT_MOUNTED;

    if(m_caster->m_silenced)
        return SPELL_FAILED_SILENCED;

    // always (except passive spells) check items (focus object can be required for any type casts)
    if(!IsPassiveSpell(m_spellInfo->Id))
        if(uint8 castResult = CheckItems())
            return castResult;

    if(uint8 castResult = CheckRange())
        return castResult;

    {
        uint32 mana = 0;
        if(uint8 castResult = CheckMana(&mana))
            return castResult;
    }

    for (int i = 0; i < 3; i++)
    {
        // for effects of spells that have only one target
        switch(m_spellInfo->Effect[i])
        {
            case SPELL_EFFECT_DUMMY:
            {
                if (!m_targets.getUnitTarget()&&!m_targets.getGOTarget())
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                // Execute
                if(m_spellInfo->SpellIconID == 1648)
                {
                    if(!m_targets.getUnitTarget() || m_targets.getUnitTarget()->GetHealth() > m_targets.getUnitTarget()->GetMaxHealth()*0.2)
                        return SPELL_FAILED_BAD_TARGETS;
                }
                break;
            }
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            {
                // Hammer of Wrath
                if(m_spellInfo->SpellVisual == 7250)
                {
                    if (!m_targets.getUnitTarget())
                        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                    if(m_targets.getUnitTarget()->GetHealth() > m_targets.getUnitTarget()->GetMaxHealth()*0.2)
                        return SPELL_FAILED_BAD_TARGETS;
                }
                break;
            }
            case SPELL_EFFECT_TAMECREATURE:
            {
                if (!m_targets.getUnitTarget() || m_targets.getUnitTarget()->GetTypeId() == TYPEID_PLAYER)
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if (m_targets.getUnitTarget()->getLevel() > m_caster->getLevel())
                    return SPELL_FAILED_HIGHLEVEL;

                CreatureInfo const *cinfo = ((Creature*)m_targets.getUnitTarget())->GetCreatureInfo();
                CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family);
                if( cinfo->type != CREATURE_TYPE_BEAST || !cFamily || !cFamily->tamable )
                    return SPELL_FAILED_BAD_TARGETS;

                if(m_caster->GetPetGUID())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                if(m_caster->GetCharmGUID())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                if(m_spellInfo->EffectImplicitTargetA[i] != TARGET_PET)
                    break;

                Pet* pet = m_caster->GetPet();
                
                if(!pet)
                    return SPELL_FAILED_NO_PET;

                SpellEntry const *learn_spellproto = sSpellStore.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);

                if(!learn_spellproto)
                    return SPELL_FAILED_NOT_KNOWN;

                if(!pet->CanTakeMoreActiveSpells(learn_spellproto->Id))
                    return SPELL_FAILED_TOO_MANY_SKILLS;

                if(m_spellInfo->spellLevel > pet->getLevel())
                    return SPELL_FAILED_LOWLEVEL;

                if(!pet->HasTPForSpell(learn_spellproto->Id))
                    return SPELL_FAILED_TRAINING_POINTS;
                    
                break;
            }
            case SPELL_EFFECT_LEARN_PET_SPELL:
            {
                Pet* pet = m_caster->GetPet();

                if(!pet)
                    return SPELL_FAILED_NO_PET;

                SpellEntry const *learn_spellproto = sSpellStore.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);

                if(!learn_spellproto)
                    return SPELL_FAILED_NOT_KNOWN;

                if(!pet->CanTakeMoreActiveSpells(learn_spellproto->Id))
                    return SPELL_FAILED_TOO_MANY_SKILLS;

                if(m_spellInfo->spellLevel > pet->getLevel())
                    return SPELL_FAILED_LOWLEVEL;

                if(!pet->HasTPForSpell(learn_spellproto->Id))
                    return SPELL_FAILED_TRAINING_POINTS;

                break;
            }
            case SPELL_EFFECT_FEED_PET:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER || !m_targets.m_itemTarget )
                    return SPELL_FAILED_BAD_TARGETS;

                Pet* pet = m_caster->GetPet();

                if(!pet)
                    return SPELL_FAILED_NO_PET;

                if(!pet->HaveInDiet(m_targets.m_itemTarget->GetProto()))
                    return SPELL_FAILED_WRONG_PET_FOOD;

                if(!pet->SetCurrentFoodBenefitLevel(m_targets.m_itemTarget->GetProto()->ItemLevel))
                    return SPELL_FAILED_FOOD_LOWLEVEL;

                if(m_caster->isInCombat() || pet->isInCombat())
                    return SPELL_FAILED_AFFECTING_COMBAT;

                break;
            }
            case SPELL_EFFECT_SKINNING:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER || !m_targets.getUnitTarget() || m_targets.getUnitTarget()->GetTypeId() != TYPEID_UNIT)
                    return SPELL_FAILED_BAD_TARGETS;

                if( !(m_targets.getUnitTarget()->GetUInt32Value(UNIT_FIELD_FLAGS) & UNIT_FLAG_SKINNABLE) )
                    return SPELL_FAILED_TARGET_UNSKINNABLE;

                if ( ( ((Creature*)m_targets.getUnitTarget())->GetCreatureInfo()->type != CREATURE_TYPE_CRITTER )
                    && ( !((Creature*)m_targets.getUnitTarget())->lootForBody || !((Creature*)m_targets.getUnitTarget())->loot.empty() ) )
                {
                    return SPELL_FAILED_TARGET_NOT_LOOTED;
                }

                int32 SkinningValue = ((Player*)m_caster)->GetSkillValue(SKILL_SKINNING);
                int32 TargetLevel = m_targets.getUnitTarget()->getLevel();
                int32 ReqValue = (SkinningValue < 100 ? (TargetLevel-10)*10 : TargetLevel*5);
                if (ReqValue > SkinningValue)
                    return SPELL_FAILED_LOW_CASTLEVEL;

                // chance for fail at orange skinning attempt
                if (m_caster->m_currentSpell == this && (ReqValue < 0 ? 0 : ReqValue) > irand(SkinningValue-25, SkinningValue+37) )
                    return SPELL_FAILED_TRY_AGAIN;

                break;
            }
            case SPELL_EFFECT_OPEN_LOCK:
            {
                if (m_spellInfo->EffectImplicitTargetA[i] != TARGET_GAMEOBJECT)
                    break;
                if (m_caster->GetTypeId() != TYPEID_PLAYER || !m_targets.getGOTarget())
                    return SPELL_FAILED_BAD_TARGETS;

                // chance for fail at orange mining/herb/LockPicking gathering attempt
                if (m_targets.getGOTarget()->GetGoType() == GAMEOBJECT_TYPE_CHEST && m_caster->m_currentSpell == this)
                {
                    int32 SkillValue;
                    if (m_spellInfo->EffectMiscValue[i] == LOCKTYPE_HERBALISM)
                        SkillValue = ((Player*)m_caster)->GetSkillValue(SKILL_HERBALISM);
                    else if (m_spellInfo->EffectMiscValue[i] == LOCKTYPE_MINING)
                        SkillValue = ((Player*)m_caster)->GetSkillValue(SKILL_MINING);
                    else if (m_spellInfo->EffectMiscValue[i] == LOCKTYPE_PICKLOCK)
                        SkillValue = ((Player*)m_caster)->GetSkillValue(SKILL_LOCKPICKING);
                    else
                        break;

                    int32 ReqValue;
                    LockEntry const *lockInfo = sLockStore.LookupEntry(m_targets.getGOTarget()->GetGOInfo()->sound0);
                    if (lockInfo)
                    {
                        if (m_spellInfo->EffectMiscValue[i] == LOCKTYPE_PICKLOCK)
                            ReqValue = lockInfo->requiredlockskill;
                        else
                            ReqValue = lockInfo->requiredminingskill;
                    }
                    else
                        break;

                    if (ReqValue > SkillValue)
                        return SPELL_FAILED_LOW_CASTLEVEL;

                    if (ReqValue > irand(SkillValue-25, SkillValue+37))
                        return SPELL_FAILED_TRY_AGAIN;
                }
                break;
            }
            case SPELL_EFFECT_SUMMON_DEAD_PET:
            {
                Creature *pet = m_caster->GetPet();
                if(!pet)
                    return SPELL_FAILED_NO_PET;

                if(pet->isAlive())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                break;
            }
            case SPELL_EFFECT_SUMMON:
                //case SPELL_EFFECT_SUMMON_WILD:                //not store in pet field
                //case SPELL_EFFECT_SUMMON_GUARDIAN:            //not store in pet field
            case SPELL_EFFECT_SUMMON_PET:
            case SPELL_EFFECT_SUMMON_POSSESSED:
            case SPELL_EFFECT_SUMMON_PHANTASM:
            case SPELL_EFFECT_SUMMON_CRITTER:               //not store in pet field
            case SPELL_EFFECT_SUMMON_DEMON:
            {
                if(m_caster->GetPetGUID())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                if(m_caster->GetCharmGUID())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                break;
            }
           case SPELL_EFFECT_SUMMON_PLAYER:
           {
               if(!m_targets.getUnitTarget())
                   return SPELL_FAILED_BAD_TARGETS;

               // check if our map is instanceable
               if( MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster)->Instanceable() &&
                   !m_caster->IsInMap(m_targets.getUnitTarget()) )
               {
                   return SPELL_FAILED_TARGET_NOT_IN_INSTANCE;
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
                float fz = MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster)->GetHeight(fx,fy,m_caster->GetPositionZ()) + 1.5;
                float caster_pos_z = m_caster->GetPositionZ();
                // Control the caster to not climb or drop when +-fz > 8
                if(!(fz<=caster_pos_z+8 && fz>=caster_pos_z-8))
                    return SPELL_FAILED_TRY_AGAIN;
                break;
            }
            default:break;
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
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                if(m_caster->GetCharmGUID())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                if(!m_targets.getUnitTarget())
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if(int32(m_targets.getUnitTarget()->getLevel()) > CalculateDamage(i))
                    return SPELL_FAILED_HIGHLEVEL;
            };break;
            case SPELL_AURA_MOD_STEALTH:
            case SPELL_AURA_MOD_INVISIBILITY:
            {

                //detect if any mod is in x range.if true,can't steath.FIX ME!
                if(m_spellInfo->Attributes == 169148432 || m_caster->GetTypeId() != TYPEID_PLAYER)
                    break;

                CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
                Cell cell = RedZone::GetZone(p);
                cell.data.Part.reserved = ALL_DISTRICT;

                Creature* found_creature = NULL;

                MaNGOS::InAttackDistanceFromAnyHostileCreatureCheck u_check(m_caster);
                MaNGOS::CreatureSearcher<MaNGOS::InAttackDistanceFromAnyHostileCreatureCheck> checker(found_creature, u_check);
                TypeContainerVisitor<MaNGOS::CreatureSearcher<MaNGOS::InAttackDistanceFromAnyHostileCreatureCheck>, GridTypeMapContainer > object_checker(checker);
                CellLock<GridReadGuard> cell_lock(cell, p);
                cell_lock->Visit(cell_lock, object_checker, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));

                if(found_creature)
                    return SPELL_FAILED_CANT_STEALTH;

            };break;
            case SPELL_AURA_MOUNTED:
            {
                if (m_caster->IsInWater())
                    return SPELL_FAILED_ONLY_ABOVEWATER;

                uint32 form = m_caster->m_form;
                if( form == FORM_CAT || form == FORM_TREE || form == FORM_TRAVEL || form == FORM_AQUA || form == FORM_BEAR || 
                    form == FORM_DIREBEAR || form == FORM_CREATUREBEAR || form == FORM_GHOSTWOLF || form == FORM_FLIGHT || form == FORM_SWIFT_FLIGHT)
                    return SPELL_FAILED_NOT_SHAPESHIFT;

                break;
            }
            case SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS:
            {
                if(!m_targets.getUnitTarget())
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                // can be casted at non-friendly unit or own pet/charm
                if(m_caster->IsFriendlyTo(m_targets.getUnitTarget()))
                    return SPELL_FAILED_TARGET_FRIENDLY;
            };break;
            case SPELL_AURA_MOD_SPEED_MOUNTED_FLIGHT:
            case SPELL_AURA_FLY:
            {
                // not allow cast fly spells at old maps
                MapEntry const* mEntry = sMapStore.LookupEntry(m_caster->GetMapId());
                if(!mEntry || (mEntry->map_flag & 0x10))      // non TBC map
                    return SPELL_FAILED_NOT_HERE;
            };break;
            default:break;
        }
    }

    // all ok
    return 0;
}

int16 Spell::PetCanCast(Unit* target)
{
    if(!m_caster->isAlive())
        return SPELL_FAILED_CASTER_DEAD;

    if(m_caster->m_currentSpell) //prevent spellcast interuption by another spellcast
        return SPELL_FAILED_SPELL_IN_PROGRESS;
    if(m_caster->isInCombat() && IsNonCombatSpell(m_spellInfo->Id))
        return SPELL_FAILED_AFFECTING_COMBAT;

    if(m_caster->GetTypeId()==TYPEID_UNIT && (((Creature*)m_caster)->isPet() || m_caster->isCharmed()))
    {
        if(m_caster->GetCharmerOrOwner() && !m_caster->GetCharmerOrOwner()->isAlive()) //dead owner (pets still alive when owners ressed?)
            return SPELL_FAILED_CASTER_DEAD;

        if(!target && m_targets.getUnitTarget())
            target = m_targets.getUnitTarget();

        bool need = false;
        for(uint32 i = 0;i<3;i++)
        {
            if(m_spellInfo->EffectImplicitTargetA[i] == TARGET_CHAIN_DAMAGE || m_spellInfo->EffectImplicitTargetA[i] == TARGET_SINGLE_FRIEND || m_spellInfo->EffectImplicitTargetA[i] == TARGET_DUELVSPLAYER || m_spellInfo->EffectImplicitTargetA[i] == TARGET_SINGLE_PARTY || m_spellInfo->EffectImplicitTargetA[i] == TARGET_CURRENT_SELECTED_ENEMY)
            {
                need = true;
                if(!target)
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
                break;
            }
        }
        if(need)
            m_targets.setUnitTarget(target);

        Unit* _target = m_targets.getUnitTarget();

        if(_target) //for target dead/target not valid
        {
            if(!_target->isAlive())
                return SPELL_FAILED_BAD_TARGETS;

            if(IsPositiveSpell(m_spellInfo->Id))
            {
                if(m_caster->IsHostileTo(_target))
                    return SPELL_FAILED_BAD_TARGETS;
            }
            else
            {
                bool duelvsplayertar = false;
                for(int j=0;j<3;j++)
                {
                    duelvsplayertar |= (m_spellInfo->EffectImplicitTargetA[j] == TARGET_DUELVSPLAYER); //TARGET_DUELVSPLAYER is positive AND negative
                }
                if(m_caster->IsFriendlyTo(target) && !duelvsplayertar)
                {
                    return SPELL_FAILED_BAD_TARGETS;
                }
            }
        }
        if(((Pet*)m_caster)->HasSpellCooldown(m_spellInfo->Id)) //cooldown
            return SPELL_FAILED_NOT_READY;
    }

    uint16 result = CanCast();
    if(result != 0)
        return result;
    else
        return -1;                                          //this allows to check spell fail 0, in combat
}

bool Spell::CanAutoCast(Unit* target)
{
    uint64 targetguid = target->GetGUID();

    for(uint32 j = 0;j<3;j++)
    {
        if((m_spellInfo->Effect[j] == SPELL_EFFECT_APPLY_AURA || m_spellInfo->Effect[j] == SPELL_EFFECT_APPLY_AREA_AURA) && target->HasAura(m_spellInfo->Id, j))
            return false;                                   //don't buff an already buffed unit
    }

    int16 result = PetCanCast(target);

    if(result == -1 || result == SPELL_FAILED_UNIT_NOT_INFRONT)
    {
        FillTargetMap();
        for(uint32 j = 0;j<3;j++)                           //check if among target units, our WANTED target is as well (->only self cast spells return false)
        {
            for(std::list<uint64>::iterator iunit= m_targetUnitGUIDs[j].begin();iunit != m_targetUnitGUIDs[j].end();++iunit)
            {
                if(*iunit == targetguid)
                    return true;
            }
         }
    }
    return false;    //target invalid
}

uint8 Spell::CheckRange()
{
    // self cast doesnt need range checking -- also for Starshards fix
    if (m_spellInfo->rangeIndex == 1) return 0;

    SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex);
    float max_range = GetMaxRange(srange);
    float min_range = GetMinRange(srange);

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
        ((Player *)m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RANGE, max_range);

    Unit *target = m_targets.getUnitTarget();

    if(target && target != m_caster)
    {
        float dist = m_caster->GetDistanceSq(target);
        if(dist > max_range * max_range)
            return SPELL_FAILED_OUT_OF_RANGE;                  //0x5A;
        if(dist < min_range * min_range)
            return SPELL_FAILED_TOO_CLOSE;
        if( !m_IsTriggeredSpell && !m_caster->isInFront( target, max_range) )
            if (m_rangedShoot || !IsPositiveSpell(m_spellInfo->Id) && casttime != 0 && !IsSingleTarget(m_spellInfo->Id))
                return SPELL_FAILED_UNIT_NOT_INFRONT;
    }

    if(m_targets.m_targetMask == TARGET_FLAG_DEST_LOCATION && m_targets.m_destX != 0 && m_targets.m_destY != 0 && m_targets.m_destY != 0)
    {
        float dist = m_caster->GetDistanceSq(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ);
        if(dist > max_range * max_range)
            return SPELL_FAILED_OUT_OF_RANGE;
        if(dist < min_range * min_range)
            return SPELL_FAILED_TOO_CLOSE;
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
            return SPELL_FAILED_CASTER_AURASTATE;

        return 0;
    }

    if(m_spellInfo->powerType <0 || m_spellInfo->powerType > POWER_HAPPINESS)
    {
        sLog.outError("Spell::CheckMana: Unknown power type '%d'", m_spellInfo->powerType);
        return SPELL_FAILED_UNKNOWN;
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

    Unit::AuraList const& mPowerCostSchool = m_caster->GetAurasByType(SPELL_AURA_MOD_POWER_COST_SCHOOL);
    for(Unit::AuraList::const_iterator i = mPowerCostSchool.begin(); i != mPowerCostSchool.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue & int32(1 << m_spellInfo->School))
            manaCost += (*i)->GetModifier()->m_amount;

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
        ((Player *)m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COST, manaCost);

    manaCost *= 1.0f + m_caster->GetFloatValue(UNIT_FIELD_POWER_COST_MODIFIER);

    if (manaCost < 0)
        manaCost = 0;

    *mana = manaCost;

    if(currentPower < manaCost)
        return SPELL_FAILED_NO_POWER;
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
            return SPELL_FAILED_ITEM_NOT_READY;
        else
        {
            ItemPrototype const *proto = m_CastItem->GetProto();
            if(!proto)
                return SPELL_FAILED_ITEM_NOT_READY;

            for (int i = 0; i<5; i++)
            {
                if (proto->Spells[i].SpellCharges)
                {
                    if(m_CastItem->GetSpellCharges(i)==0)
                        return SPELL_FAILED_NO_CHARGES_REMAIN;
                }
            }

            uint32 ItemClass = proto->Class;
            if (ItemClass == ITEM_CLASS_CONSUMABLE && m_targets.getUnitTarget())
            {
                for (int i = 0; i < 3; i++)
                {
                    if (m_spellInfo->Effect[i] == SPELL_EFFECT_HEAL)
                        if (m_targets.getUnitTarget()->GetHealth() == m_targets.getUnitTarget()->GetMaxHealth())
                            return (uint8)SPELL_FAILED_ALREADY_AT_FULL_HEALTH;

                    // Mana Potion, Rage Potion, Thistle Tea(Rogue), ...
                    if (m_spellInfo->Effect[i] == SPELL_EFFECT_ENERGIZE)
                    {
                        //Check if the Caster Has Rage For Power
                        if (m_caster->GetMaxPower(POWER_RAGE))
                        {
                            if (m_targets.getUnitTarget()->GetPower(POWER_RAGE) == m_targets.getUnitTarget()->GetMaxPower(POWER_RAGE))
                                return (uint8)SPELL_FAILED_ALREADY_AT_FULL_POWER;
                        }
                        //Check if the Caster Has Energy For Power
                        else if (m_caster->GetMaxPower(POWER_ENERGY))
                        {
                            if (m_targets.getUnitTarget()->GetPower(POWER_ENERGY) == m_targets.getUnitTarget()->GetMaxPower(POWER_ENERGY))
                                return (uint8)SPELL_FAILED_ALREADY_AT_FULL_POWER;
                        }
                        //So The Player Has Mana
                        else if (m_targets.getUnitTarget()->GetPower(POWER_MANA) == m_targets.getUnitTarget()->GetMaxPower(POWER_MANA))
                        {
                            return (uint8)SPELL_FAILED_ALREADY_AT_FULL_POWER;
                        }
                    }

                }
            }
        }
    }
    if(m_targets.m_itemTarget)
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER && !m_targets.m_itemTarget->IsFitToSpellRequirements(m_spellInfo))
            return SPELL_FAILED_BAD_TARGETS;
    }

    if(m_spellInfo->RequiresSpellFocus)
    {
        CellPair p(MaNGOS::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
        Cell cell = RedZone::GetZone(p);
        cell.data.Part.reserved = ALL_DISTRICT;

        GameObject* ok = NULL;
        MaNGOS::GameObjectFocusCheck go_check(m_caster,m_spellInfo->RequiresSpellFocus);
        MaNGOS::GameObjectSearcher<MaNGOS::GameObjectFocusCheck> checker(ok,go_check);

        TypeContainerVisitor<MaNGOS::GameObjectSearcher<MaNGOS::GameObjectFocusCheck>, GridTypeMapContainer > object_checker(checker);
        CellLock<GridReadGuard> cell_lock(cell, p);
        cell_lock->Visit(cell_lock, object_checker, *MapManager::Instance().GetMap(m_caster->GetMapId(), m_caster));

        if(!ok) return (uint8)SPELL_FAILED_REQUIRES_SPELL_FOCUS;

        focusObject = ok;

        if(m_targets.IsEmpty())
            m_targets.setGOTarget(ok);

        // game object found in range
    }

    for(uint32 i=0;i<8;i++)
    {
        if((itemid = m_spellInfo->Reagent[i]) == 0)
            continue;
        itemcount = m_spellInfo->ReagentCount[i];
        // CastItem is also spell reagent
        if( m_CastItem && m_CastItem->GetEntry() == itemid )
        {
            ItemPrototype const *proto = m_CastItem->GetProto();
            if(!proto)
                return SPELL_FAILED_ITEM_NOT_READY;
            for(int s=0;s<5;s++)
            {
                // CastItem will be used up and does not count as reagent
                int32 charges = m_CastItem->GetSpellCharges(s);
                if (proto->Spells[s].SpellCharges < 0 && abs(charges) < 2)
                {
                    itemcount++;
                    break;
                }
            }
        }
        if( !p_caster->HasItemCount(itemid,itemcount) )
            return (uint8)SPELL_FAILED_ITEM_NOT_READY;         //0x54
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
        return uint8(0x70); // TODO: replace this const

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
                        return uint8(SPELL_FAILED_DONT_REPORT);     // TODO: don't show two errors
                    }
                }
                break;
            }
            case SPELL_EFFECT_ENCHANT_ITEM:
            case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
                if(!m_targets.m_itemTarget)
                    return SPELL_FAILED_ITEM_NOT_FOUND;
                break;
            case SPELL_EFFECT_ENCHANT_HELD_ITEM:
                // check item existence in effect code (not output errors at offhand hold item effect to main hand for example
                break;
            case SPELL_EFFECT_DISENCHANT:
            {
                if(!m_targets.m_itemTarget)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;

                // prevent disenchanting in trade slot
                if( m_targets.m_itemTarget->GetOwnerGUID() != m_caster->GetGUID() )
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                uint32 item_quality = m_targets.m_itemTarget->GetProto()->Quality;
                // 2.0.x addon: Check player enchanting level agains the item desenchanting requirements
                uint32 item_disenchantskilllevel = m_targets.m_itemTarget->GetProto()->RequiredDisenchantSkill;
                if (item_disenchantskilllevel == uint32(-1))
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                if (item_disenchantskilllevel > p_caster->GetSkillValue(SKILL_ENCHANTING))
                    return SPELL_FAILED_LOW_CASTLEVEL;
                if(item_quality > 4 || item_quality < 2)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                if(m_targets.m_itemTarget->GetProto()->Class != ITEM_CLASS_WEAPON && m_targets.m_itemTarget->GetProto()->Class != ITEM_CLASS_ARMOR)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                if (!m_targets.m_itemTarget->GetProto()->DisenchantID)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                break;
            }
            case SPELL_EFFECT_PROSPECTING:
            {
                if(!m_targets.m_itemTarget)
                    return SPELL_FAILED_CANT_BE_PROSPECTED;
                //ensure item is a prospectable ore
                if(m_targets.m_itemTarget->GetProto()->BagFamily != BAG_FAMILY_MINING_SUPP || m_targets.m_itemTarget->GetProto()->Class != ITEM_CLASS_TRADE_GOODS)
                    return SPELL_FAILED_CANT_BE_PROSPECTED;
                //prevent prospecting in trade slot
                if( m_targets.m_itemTarget->GetOwnerGUID() != m_caster->GetGUID() )
                    return SPELL_FAILED_CANT_BE_PROSPECTED;
                //Check for enough skill in jewelcrafting
                uint32 item_prospectingskilllevel = m_targets.m_itemTarget->GetProto()->RequiredSkillRank;
                if(item_prospectingskilllevel >p_caster->GetSkillValue(SKILL_JEWELCRAFTING))
                    return SPELL_FAILED_LOW_CASTLEVEL;
                //make sure the player has the required ores in inventory
                if(m_targets.m_itemTarget->GetCount() < 5)
                    return SPELL_FAILED_PROSPECT_NEED_MORE;

                if(LootTemplates_Prospecting.find(m_targets.m_itemTarget->GetEntry())==LootTemplates_Prospecting.end())
                    return SPELL_FAILED_CANT_BE_PROSPECTED;

                break;
            }
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            {
                if(m_caster->GetTypeId() != TYPEID_PLAYER) return SPELL_FAILED_TARGET_NOT_PLAYER;
                if(m_spellInfo->rangeIndex == 1 || m_spellInfo->rangeIndex == 2 || m_spellInfo->rangeIndex == 7)
                    break;
                Item *pItem = ((Player*)m_caster)->GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED );
                if(!pItem || pItem->IsBroken() || pItem->GetProto()->Class != ITEM_CLASS_WEAPON )
                    return SPELL_FAILED_EQUIPPED_ITEM;

                switch(pItem->GetProto()->SubClass)
                {
                    case ITEM_SUBCLASS_WEAPON_THROWN:
                    {
                        uint32 ammo = pItem->GetEntry();
                        if( !((Player*)m_caster)->HasItemCount( ammo, 1 ) )
                            return SPELL_FAILED_NO_AMMO;
                    };  break;
                    case ITEM_SUBCLASS_WEAPON_GUN:
                    case ITEM_SUBCLASS_WEAPON_BOW:
                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    {
                        uint32 ammo = ((Player*)m_caster)->GetUInt32Value(PLAYER_AMMO_ID);
                        if(!ammo)
                            return SPELL_FAILED_NO_AMMO;

                        ItemPrototype const *ammoProto = objmgr.GetItemPrototype( ammo );
                        if(!ammoProto)
                            return SPELL_FAILED_NO_AMMO;

                        if(ammoProto->Class != ITEM_CLASS_PROJECTILE)
                            return SPELL_FAILED_NO_AMMO;

                        // check ammo ws. weapon compatibility
                        switch(pItem->GetProto()->SubClass)
                        {
                            case ITEM_SUBCLASS_WEAPON_BOW:
                            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                                if(ammoProto->SubClass!=ITEM_SUBCLASS_ARROW)
                                    return SPELL_FAILED_NO_AMMO;
                                break;
                            case ITEM_SUBCLASS_WEAPON_GUN:
                                if(ammoProto->SubClass!=ITEM_SUBCLASS_BULLET)
                                    return SPELL_FAILED_NO_AMMO;
                                break;
                            default:
                                return SPELL_FAILED_NO_AMMO;
                        }

                        if( !((Player*)m_caster)->HasItemCount( ammo, 1 ) )
                            return SPELL_FAILED_NO_AMMO;
                    };  break;
                    case ITEM_SUBCLASS_WEAPON_WAND:
                    default:
                        break;
                }
                break;
            }
            default:break;
        }
    }

    return uint8(0);
}

int32 Spell::CalculateDamage(uint8 i)
{
    int32 value = m_caster->CalculateSpellDamage(m_spellInfo,i);

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

    if(m_spellInfo->Id == 8690 || m_spellInfo->Id == 556 || m_spellInfo->Id == 7355)
    {
        //old code, slow:
        /*Field *fields;
        QueryResult *result = sDatabase.PQuery("SELECT `map`,`zone`,`position_x`,`position_y`,`position_z` FROM `character_homebind` WHERE `guid` = '%u'", m_caster->GetGUIDLow());
        if(!result)
        {
            sLog.outError( "SPELL: No homebind location set for %i\n", m_caster->GetGUIDLow());
            return;
        }
        fields = result->Fetch();

        uint32 TC_mapId = fields[0].GetUInt32();
        float  TC_x = fields[2].GetFloat();
        float  TC_y = fields[3].GetFloat();
        float  TC_z = fields[4].GetFloat();

        delete result;*/

        //homebind location is loaded always
        ((Player*)Target)->TeleportTo(((Player*)m_caster)->m_homebindMapId,((Player*)m_caster)->m_homebindX,((Player*)m_caster)->m_homebindY,((Player*)m_caster)->m_homebindZ,Target->GetOrientation());
    }
    else
    {
        AreaTrigger const* at = objmgr.GetAreaTrigger(id);
        if(!at || !at->IsTeleport())
        {
            sLog.outError( "SPELL: unknown Teleport Coords ID %i\n", id );
            return;
        }
        ((Player*)Target)->TeleportTo(at->target_mapId,at->target_X,at->target_Y,at->target_Z,at->target_Orientation);
    }
}

void Spell::Delayed(int32 delaytime)
{
    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    //check resist chance
    int32 resistChance = 100; //must be initialized to 100 for percent modifiers
    ((Player*)m_caster)->ApplySpellMod(m_spellInfo->Id,SPELLMOD_NOT_LOSE_CASTING_TIME,resistChance);
    resistChance += m_caster->GetTotalAuraModifier(SPELL_AURA_RESIST_PUSHBACK) - 100;
    if (roll_chance_f(resistChance))
        return;    
    
    m_timer += delaytime;

    if(m_timer > casttime)
        m_timer = (casttime > 0 ? casttime : 0);

    WorldPacket data(SMSG_SPELL_DELAYED, 8+4);
    data.append(m_caster->GetPackGUID());
    data << uint32(delaytime);

    ((Player*)m_caster)->GetSession()->SendPacket(&data);
}

void Spell::DelayedChannel(int32 delaytime)
{
    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER || getState() != SPELL_STATE_CASTING)
        return;

    //check resist chance
    int32 resistChance = 100; //must be initialized to 100 for percent modifiers
    ((Player*)m_caster)->ApplySpellMod(m_spellInfo->Id,SPELLMOD_NOT_LOSE_CASTING_TIME,resistChance);
    resistChance += m_caster->GetTotalAuraModifier(SPELL_AURA_RESIST_PUSHBACK) - 100;
    if (roll_chance_f(resistChance))
        return;
    
    int32 appliedDelayTime = delaytime;

    if(int32(m_timer) < delaytime)
    {
        appliedDelayTime = m_timer;
        m_timer = 0;
    } else
    m_timer -= delaytime;

    sLog.outDebug("Spell %u partially interrupted for %i ms, new duration: %u ms", m_spellInfo->Id, appliedDelayTime, m_timer);

    for(int j = 0; j < 3; j++)
    {
        // partially interrupt auras with fixed targets
        for(std::list<uint64>::iterator iunit= m_targetUnitGUIDs[j].begin();iunit != m_targetUnitGUIDs[j].end();++iunit)
        {
            // check m_caster->GetGUID() let load auras at login and speedup most often case
            Unit* unit = m_caster->GetGUID()==*iunit ? m_caster : ObjectAccessor::Instance().GetUnit(*m_caster,*iunit);
            if (unit)
                unit->DelayAura(m_spellInfo->Id, j, appliedDelayTime);
        }

        // partially interrupt persistent area auras
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

    // if the spell to reflect is a reflect spell, do nothing.
    for(int i=0; i<3; i++)
        if(m_spellInfo->Effect[i] == 6 && (m_spellInfo->EffectApplyAuraName[i] == 74 || m_spellInfo->EffectApplyAuraName[i] == 28))
            return;

    int32 reflectchance = 0;                                // proper base reflect chance is ?

    Unit::AuraList const& mReflectSpells = refunit->GetAurasByType(SPELL_AURA_REFLECT_SPELLS);
    for(Unit::AuraList::const_iterator i = mReflectSpells.begin(); i != mReflectSpells.end(); ++i)
        reflectchance += (*i)->GetModifier()->m_amount;

    Unit::AuraList const& mReflectSpellsSchool = refunit->GetAurasByType(SPELL_AURA_REFLECT_SPELLS_SCHOOL);
    for(Unit::AuraList::const_iterator i = mReflectSpellsSchool.begin(); i != mReflectSpellsSchool.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue & int32(1 << m_spellInfo->School))
            reflectchance += (*i)->GetModifier()->m_amount;

    if (reflectchance > 0 && roll_chance_i(reflectchance))
    {
        Spell spell(refunit, m_spellInfo, true, 0);

        SpellCastTargets targets;
        targets.setUnitTarget( m_caster );
        spell.prepare(&targets);
    }
}

void Spell::UpdatePointers()
{
    if(m_originalCasterGUID==m_caster->GetGUID())
        m_originalCaster = m_caster;
    else
        m_originalCaster = ObjectAccessor::Instance().GetUnit(*m_caster,m_originalCasterGUID);

    m_targets.Update(m_caster);
}

bool Spell::IsAffectedBy(SpellEntry const *spellInfo, uint32 effectId)
{
    if (!spellInfo) 
        return false;

    SpellAffection const *spellAffect = objmgr.GetSpellAffection(spellInfo->Id, effectId);

    if (spellAffect)
    {
        if (spellAffect->SpellId && (spellAffect->SpellId == m_spellInfo->Id))
            return true;
        if (spellAffect->SchoolMask && (spellAffect->SchoolMask & m_spellInfo->School))
            return true;
        if (spellAffect->Category && (spellAffect->Category == m_spellInfo->Category))
            return true;
        if (spellAffect->SkillId)
        {
            SkillLineAbilityEntry const *skillLineEntry = sSkillLineAbilityStore.LookupEntry(m_spellInfo->Id);
            if(skillLineEntry && skillLineEntry->skillId == spellAffect->SkillId)
                return true;
        }
        if (spellAffect->SpellFamily && spellAffect->SpellFamily == m_spellInfo->SpellFamilyName)
            return true;

        if (spellAffect->SpellFamilyMask && (spellAffect->SpellFamilyMask & m_spellInfo->SpellFamilyFlags))
            return true;
    }
    else
        if (spellInfo->EffectItemType[effectId] & m_spellInfo->SpellFamilyFlags)
            return true;
    
    return false;
}

uint32 Spell::GetTargetCreatureTypeMask() const
{
    uint32 SpellCreatureType = m_spellInfo->TargetCreatureType;

    // not find another way to fix spell target check :/
    if(m_spellInfo->Id == 603)
        SpellCreatureType = 0x7FF - 0x40;                   //Curse of Doom
    else
        if(m_spellInfo->Id == 2641)                         // Dismiss Pet
            SpellCreatureType = 0;
    return SpellCreatureType;
}
