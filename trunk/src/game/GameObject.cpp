/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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
#include "QuestDef.h"
#include "GameObject.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Spell.h"
#include "UpdateMask.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Database/DatabaseEnv.h"
#include "MapManager.h"
#include "LootMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "InstanceData.h"

GameObject::GameObject( WorldObject *instantiator ) : WorldObject( instantiator )
{
    m_objectType |= TYPE_GAMEOBJECT;
    m_objectTypeId = TYPEID_GAMEOBJECT;
                                                            // 2.3.2 - 0x58
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID | UPDATEFLAG_HASPOSITION);

    m_valuesCount = GAMEOBJECT_END;
    m_respawnTime = 0;
    m_respawnDelayTime = 25;
    m_lootState = GO_CLOSED;
    m_spawnedByDefault = true;
    m_usetimes = 0;
    m_spellId = 0;
}

GameObject::~GameObject()
{
    if(m_uint32Values)                                      // field array can be not exist if GameOBject not loaded
    {
        // crash possable at access to deleted GO in Unit::m_gameobj
        uint64 owner_guid = GetOwnerGUID();
        if(owner_guid)
        {
            Unit* owner = ObjectAccessor::GetUnit(*this,owner_guid);
            if(owner)
                owner->RemoveGameObject(this,false);
            else if(GUID_HIPART(owner_guid)!=HIGHGUID_PLAYER)
                sLog.outError("Delete GameObject (GUID: %u Entry: %u ) that have references in not found creature %u GO list. Crash possable later.",GetGUIDLow(),GetGOInfo()->id,GUID_LOPART(owner_guid));
        }
    }
}

void GameObject::AddToWorld()
{
    ///- Register the gameobject for guid lookup
    if(!IsInWorld()) ObjectAccessor::Instance().AddObject(this);
    Object::AddToWorld();
}

void GameObject::RemoveFromWorld()
{
    ///- Remove the gameobject from the accessor
    if(IsInWorld()) ObjectAccessor::Instance().RemoveObject(this);
    Object::RemoveFromWorld();
}

bool GameObject::Create(uint32 guidlow, uint32 name_id, uint32 mapid, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 animprogress, uint32 go_state)
{
    Relocate(x,y,z,ang);
    SetMapId(mapid);

    if(!IsPositionValid())
    {
        sLog.outError("ERROR: Gameobject (GUID: %u Entry: %u ) not created. Suggested coordinates isn't valid (X: %f Y: %f)",guidlow,name_id,x,y);
        return false;
    }

    Object::_Create(guidlow, HIGHGUID_GAMEOBJECT);

    m_DBTableGuid = guidlow;

    GameObjectInfo const* goinfo = objmgr.GetGameObjectInfo(name_id);

    if (!goinfo)
    {
        sLog.outErrorDb("Gameobject (GUID: %u Entry: %u) not created: it have not exist entry in `gameobject_template`. Map: %u  (X: %f Y: %f Z: %f) ang: %f rotation0: %f rotation1: %f rotation2: %f rotation3: %f",guidlow, name_id, mapid, x, y, z, ang, rotation0, rotation1, rotation2, rotation3);
        return false;
    }

    if (goinfo->type >= MAX_GAMEOBJECT_TYPE)
    {
        sLog.outErrorDb("Gameobject (GUID: %u Entry: %u) not created: it have not exist GO type '%u' in `gameobject_template`. It's will crash client if created.",guidlow,name_id,goinfo->type);
        return false;
    }

    //    SetUInt32Value(GAMEOBJECT_TIMESTAMP, (uint32)time(NULL));
    SetFloatValue(GAMEOBJECT_POS_X, x);
    SetFloatValue(GAMEOBJECT_POS_Y, y);
    SetFloatValue(GAMEOBJECT_POS_Z, z);
    SetFloatValue(GAMEOBJECT_FACING, ang);                  //this is not facing angle

    SetFloatValue (GAMEOBJECT_ROTATION, rotation0);
    SetFloatValue (GAMEOBJECT_ROTATION+1, rotation1);
    SetFloatValue (GAMEOBJECT_ROTATION+2, rotation2);
    SetFloatValue (GAMEOBJECT_ROTATION+3, rotation3);

    SetFloatValue(OBJECT_FIELD_SCALE_X, goinfo->size);

    SetUInt32Value(GAMEOBJECT_FACTION, goinfo->faction);
    SetUInt32Value(GAMEOBJECT_FLAGS, goinfo->flags);
    m_flags = goinfo->flags;

    SetUInt32Value (OBJECT_FIELD_ENTRY, goinfo->id);

    SetUInt32Value (GAMEOBJECT_DISPLAYID, goinfo->displayId);

    SetUInt32Value (GAMEOBJECT_STATE, go_state);
    SetUInt32Value (GAMEOBJECT_TYPE_ID, goinfo->type);

    SetUInt32Value (GAMEOBJECT_ANIMPROGRESS, animprogress);

    //Notify the map's instance data.
    //Only works if you create the object in it, not if it is moves to that map.
    //Normally non-players do not teleport to other maps.
    Map *map = MapManager::Instance().GetMap(GetMapId(), this);
    if(map && map->GetInstanceData())
    {
        map->GetInstanceData()->OnObjectCreate(this);
    }

    return true;
}

void GameObject::Update(uint32 /*p_time*/)
{
    if (GUID_HIPART(GetGUID()) == HIGHGUID_MO_TRANSPORT)
    {
        //((Transport*)this)->Update(p_time);
        return;
    }

    switch (m_lootState)
    {
        case GO_NOT_READY:
            if (GetGoType()==GAMEOBJECT_TYPE_FISHINGNODE)
            {
                // fishing code (bobber ready)
                if( time(NULL) > m_respawnTime - FISHING_BOBBER_READY_TIME )
                {
                    // splash bobber (bobber ready now)
                    Unit* caster = GetOwner();
                    if(caster && caster->GetTypeId()==TYPEID_PLAYER)
                    {
                        SetUInt32Value(GAMEOBJECT_STATE, 0);
                        SetUInt32Value(GAMEOBJECT_FLAGS, 32);

                        UpdateData udata;
                        WorldPacket packet;
                        BuildValuesUpdateBlockForPlayer(&udata,((Player*)caster));
                        udata.BuildPacket(&packet);
                        ((Player*)caster)->GetSession()->SendPacket(&packet);

                        WorldPacket data(SMSG_GAMEOBJECT_CUSTOM_ANIM,8+4);
                        data << GetGUID();
                        data << (uint32)(0);
                        ((Player*)caster)->SendMessageToSet(&data,true);
                    }

                    m_lootState = GO_CLOSED;                // can be succesfully open with some chance
                }
                return;
            }

            m_lootState = GO_CLOSED;                        // for not bobber is same as GO_CLOSED
            // NO BREAK
        case GO_CLOSED:
            if (m_respawnTime > 0)
                                                            // timer on
            {
                if (m_respawnTime <= time(NULL))            // timer expired
                {
                    m_respawnTime = 0;
                    m_SkillupList.clear();
                    m_usetimes = 0;

                    switch (GetGoType())
                    {
                        case GAMEOBJECT_TYPE_FISHINGNODE:   //  can't fish now
                        {
                            Unit* caster = GetOwner();
                            if(caster && caster->GetTypeId()==TYPEID_PLAYER)
                            {
                                if(caster->m_currentSpells[CURRENT_CHANNELED_SPELL])
                                {
                                    caster->m_currentSpells[CURRENT_CHANNELED_SPELL]->SendChannelUpdate(0);
                                    caster->m_currentSpells[CURRENT_CHANNELED_SPELL]->finish(false);
                                }

                                WorldPacket data(SMSG_FISH_NOT_HOOKED,0);
                                ((Player*)caster)->GetSession()->SendPacket(&data);
                            }
                            m_lootState = GO_LOOTED;        // can be delete
                            return;
                        }
                        case GAMEOBJECT_TYPE_DOOR:
                        case GAMEOBJECT_TYPE_BUTTON:
                            SetUInt32Value (GAMEOBJECT_FLAGS, m_flags);
                            if(GetUInt32Value(GAMEOBJECT_STATE))
                                SetUInt32Value (GAMEOBJECT_STATE, 0);
                            else
                                SetUInt32Value (GAMEOBJECT_STATE, 1);
                            break;
                        case GAMEOBJECT_TYPE_TRAP:
                            break;
                        default:
                            if(!m_spawnedByDefault)         // despawn timer
                            {
                                if(GetOwnerGUID())
                                {
                                    Delete();
                                    return;
                                }

                                SetLootState(GO_LOOTED);    // can be despawned
                                return;
                            }
                                                            // respawn timer
                            MapManager::Instance().GetMap(GetMapId(), this)->Add(this);
                            break;
                    }
                }
            }
            break;

        case GO_OPEN:
            break;

        case GO_LOOTED:
        {
            //if Gamebject should cast spell, then this, but some GOs (type = 10) should be destroyed
            if (GetGoType() == GAMEOBJECT_TYPE_GOOBER)
            {
                uint32 spellId = GetGOInfo()->data10;

                if(spellId)
                {
                    std::set<uint32>::iterator it = m_unique_users.begin();
                    std::set<uint32>::iterator end = m_unique_users.end();
                    for (; it != end; it++)
                    {
                        Unit* owner = Unit::GetUnit(*this, uint64(*it));
                        if (owner) owner->CastSpell(owner, spellId, false);
                    }

                    m_unique_users.clear();
                    m_usetimes = 0;
                    SetLootState(GO_CLOSED);
                    break;
                }
            }

            if(GetOwnerGUID())
            {
                m_respawnTime = 0;
                Delete();
                return;
            }

            loot.clear();
            SetLootState(GO_CLOSED);

            if(!m_spawnedByDefault)
            {
                m_respawnTime = 0;
                return;
            }

            m_respawnTime = time(NULL) + m_respawnDelayTime;

            // if option not set then object will be saved at grid unload
            if(sWorld.getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATLY))
                SaveRespawnTime();

            ObjectAccessor::UpdateObjectVisibility(this);

            break;
        }
    }

    SpellEntry const *createSpell = m_spellId ? sSpellStore.LookupEntry(m_spellId) : NULL;
    if (!createSpell)
        return;
    int i;
    for (i = 0; i < 3; i++)
        if (createSpell->Effect[i] == SPELL_EFFECT_SUMMON_OBJECT_SLOT1)
            break;
    if (i<3)
    {
        // traps
        CellPair p(MaNGOS::ComputeCellPair(GetPositionX(),GetPositionY()));
        Cell cell(p);
        cell.data.Part.reserved = ALL_DISTRICT;

        Unit* owner = GetOwner();
        if (!owner)
        {
            m_respawnTime = 0;                              // to prevent save respawn timer
            Delete();
            return;
        }

        Unit* ok = NULL;                                    // pointer to appropriate target if found any

        float radius = GetRadius(sSpellRadiusStore.LookupEntry(createSpell->EffectRadiusIndex[i]));
        MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck u_check(this, owner, radius);
        MaNGOS::UnitSearcher<MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck> checker(ok, u_check);

        CellLock<GridReadGuard> cell_lock(cell, p);

        // search unfriedly creature
        {
            TypeContainerVisitor<MaNGOS::UnitSearcher<MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck>, GridTypeMapContainer > grid_object_checker(checker);
            cell_lock->Visit(cell_lock, grid_object_checker, *MapManager::Instance().GetMap(GetMapId(), this));
        }

        // or unfriendly player/pet
        if(!ok)
        {
            TypeContainerVisitor<MaNGOS::UnitSearcher<MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck>, WorldTypeMapContainer > world_object_checker(checker);
            cell_lock->Visit(cell_lock, world_object_checker, *MapManager::Instance().GetMap(GetMapId(), this));
        }

        if (ok)
        {
            owner->CastSpell(ok, GetGOInfo()->data3, true);
            m_respawnTime = 0;                              // to prevent save respawn timer
            Delete();
        }
    }

    if (m_usetimes >= 5)
    {
        m_respawnTime = 0;                                  // to prevent save respawn timer
        Delete();
    }

}

void GameObject::Refresh()
{
    // not refresh despawned not casted GO (despawned casted GO destroyed in all cases anyway)
    if(m_respawnTime > 0 && m_spawnedByDefault)
        return;

    SendDestroyObject(GetGUID());

    if(isSpawned())
        MapManager::Instance().GetMap(GetMapId(), this)->Add(this);
}

void GameObject::AddUse(Player* player)
{
    ++m_usetimes;
    m_unique_users.insert(player->GetGUIDLow());
}

void GameObject::Delete()
{
    SendObjectDeSpawnAnim(GetGUID());

    SetUInt32Value(GAMEOBJECT_STATE, 1);
    SetUInt32Value(GAMEOBJECT_FLAGS, m_flags);

    SendDestroyObject(GetGUID());
    //TODO: set timestamp
    ObjectAccessor::Instance().AddObjectToRemoveList(this);
}

void GameObject::getFishLoot(Loot *fishloot)
{
    fishloot->clear();

    uint32 subzone = GetAreaId();

    // if subzone loot exist use it
    if(LootTemplates_Fishing.find(subzone) != LootTemplates_Fishing.end())
        FillLoot(fishloot, subzone, LootTemplates_Fishing, NULL);
    // else use zone loot
    else
        FillLoot(fishloot, GetZoneId(), LootTemplates_Fishing, NULL);
}

void GameObject::SaveToDB()
{
    const GameObjectInfo *goI = GetGOInfo();

    if (!goI)
        return;

    // update in loaded data (changing data only in this place)
    GameObjectData& data = objmgr.NewGOData(m_DBTableGuid);

    // data->guid = guid don't must be update at save
    data.id = GetEntry();
    data.mapid = GetMapId();
    data.posX = GetFloatValue(GAMEOBJECT_POS_X);
    data.posY = GetFloatValue(GAMEOBJECT_POS_Y);
    data.posZ = GetFloatValue(GAMEOBJECT_POS_Z);
    data.orientation = GetFloatValue(GAMEOBJECT_FACING);
    data.rotation0 = GetFloatValue(GAMEOBJECT_ROTATION+0);
    data.rotation1 = GetFloatValue(GAMEOBJECT_ROTATION+1);
    data.rotation2 = GetFloatValue(GAMEOBJECT_ROTATION+2);
    data.rotation3 = GetFloatValue(GAMEOBJECT_ROTATION+3);
    data.spawntimesecs = m_spawnedByDefault ? m_respawnDelayTime : -(int32)m_respawnDelayTime;
    data.animprogress = GetUInt32Value (GAMEOBJECT_ANIMPROGRESS);
    data.go_state = GetUInt32Value (GAMEOBJECT_STATE);

    // updated in DB
    std::ostringstream ss;
    ss << "INSERT INTO `gameobject` VALUES ( "
        << m_DBTableGuid << ", "
        << GetUInt32Value (OBJECT_FIELD_ENTRY) << ", "
        << GetMapId() << ", "
        << GetFloatValue(GAMEOBJECT_POS_X) << ", "
        << GetFloatValue(GAMEOBJECT_POS_Y) << ", "
        << GetFloatValue(GAMEOBJECT_POS_Z) << ", "
        << GetFloatValue(GAMEOBJECT_FACING) << ", "
        << GetFloatValue(GAMEOBJECT_ROTATION) << ", "
        << GetFloatValue(GAMEOBJECT_ROTATION+1) << ", "
        << GetFloatValue(GAMEOBJECT_ROTATION+2) << ", "
        << GetFloatValue(GAMEOBJECT_ROTATION+3) << ", "
        << m_respawnDelayTime << ", "
        << GetUInt32Value (GAMEOBJECT_ANIMPROGRESS) << ", "
        << GetUInt32Value (GAMEOBJECT_STATE) << ")";

    WorldDatabase.BeginTransaction();
    WorldDatabase.PExecuteLog("DELETE FROM `gameobject` WHERE `guid` = '%u'", m_DBTableGuid);
    WorldDatabase.PExecuteLog( ss.str( ).c_str( ) );
    WorldDatabase.CommitTransaction();
}

bool GameObject::LoadFromDB(uint32 guid, uint32 InstanceId)
{
    GameObjectData const* data = objmgr.GetGOData(guid);

    if( !data )
    {
        sLog.outErrorDb("ERROR: Gameobject (GUID: %u) not found in table `gameobject`, can't load. ",guid);
        return false;
    }

    uint32 entry = data->id;
    uint32 map_id = data->mapid;
    float x = data->posX;
    float y = data->posY;
    float z = data->posZ;
    float ang = data->orientation;

    float rotation0 = data->rotation0;
    float rotation1 = data->rotation1;
    float rotation2 = data->rotation2;
    float rotation3 = data->rotation3;

    uint32 animprogress = data->animprogress;
    uint32 go_state = data->go_state;

    uint32 stored_guid = guid;
    if (InstanceId != 0) guid = objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT);
    SetInstanceId(InstanceId);

    if (!Create(guid,entry, map_id, x, y, z, ang, rotation0, rotation1, rotation2, rotation3, animprogress, go_state) )
        return false;

    m_DBTableGuid = stored_guid;

    if(data->spawntimesecs >= 0)
    {
        m_spawnedByDefault = true;
        m_respawnDelayTime=data->spawntimesecs;
        m_respawnTime=objmgr.GetGORespawnTime(stored_guid,InstanceId);

        if(m_respawnTime && m_respawnTime <= time(NULL))    // ready to respawn
        {
            m_respawnTime = 0;
            objmgr.SaveGORespawnTime(m_DBTableGuid,GetInstanceId(),0);
        }
    }
    else
    {
        m_spawnedByDefault = false;
        m_respawnDelayTime=-data->spawntimesecs;
        m_respawnTime = 0;
    }

    return true;
}

void GameObject::DeleteFromDB()
{
    objmgr.SaveGORespawnTime(m_DBTableGuid,GetInstanceId(),0);
    objmgr.DeleteGOData(m_DBTableGuid);
    WorldDatabase.PExecute("DELETE FROM `gameobject` WHERE `guid` = '%u'", m_DBTableGuid);
}

GameObjectInfo const *GameObject::GetGOInfo() const
{
    return objmgr.GetGameObjectInfo(GetUInt32Value (OBJECT_FIELD_ENTRY));
}

uint32 GameObject::GetLootId(GameObjectInfo const* ginfo)
{
    if (!ginfo)
        return 0;
        
    switch(ginfo->type)
    {
        case GAMEOBJECT_TYPE_CHEST:
        case GAMEOBJECT_TYPE_FISHINGHOLE:
            return ginfo->data1;
        default:
            return 0;
    }
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/
bool GameObject::hasQuest(uint32 quest_id) const
{
    QuestRelations const& qr = objmgr.mGOQuestRelations;
    for(QuestRelations::const_iterator itr = qr.lower_bound(GetEntry()); itr != qr.upper_bound(GetEntry()); ++itr)
    {
        if(itr->second==quest_id)
            return true;
    }
    return false;
}

bool GameObject::hasInvolvedQuest(uint32 quest_id) const
{
    QuestRelations const& qr = objmgr.mGOQuestInvolvedRelations;
    for(QuestRelations::const_iterator itr = qr.lower_bound(GetEntry()); itr != qr.upper_bound(GetEntry()); ++itr)
    {
        if(itr->second==quest_id)
            return true;
    }
    return false;
}

bool GameObject::IsTransport() const
{
    // If something is marked as a transport, don't transmit an out of range packet for it.
    GameObjectInfo const * gInfo = GetGOInfo();
    if(!gInfo) return false;
    return gInfo->type == GAMEOBJECT_TYPE_TRANSPORT || gInfo->type == GAMEOBJECT_TYPE_MO_TRANSPORT;
}

Unit* GameObject::GetOwner() const
{
    return ObjectAccessor::GetUnit(*this, GetOwnerGUID());
}

void GameObject::SaveRespawnTime()
{
    if(m_respawnTime > time(NULL) && m_spawnedByDefault)
        objmgr.SaveGORespawnTime(m_DBTableGuid,GetInstanceId(),m_respawnTime);
}

bool GameObject::isVisibleForInState(Player const* u, bool inVisibleList) const
{
    return IsInWorld() && u->IsInWorld() && ( IsTransport() && IsInMap(u) ||
        isSpawned() && IsWithinDistInMap(u,World::GetMaxVisibleDistanceForObject()+(inVisibleList ? World::GetVisibleObjectGreyDistance() : 0.0f)) );
}

void GameObject::Respawn()
{
    if(m_spawnedByDefault && m_respawnTime > 0)
    {
        m_respawnTime = time(NULL);
        objmgr.SaveGORespawnTime(m_DBTableGuid,GetInstanceId(),0);
    }
}

bool GameObject::ActivateToQuest( Player *pTarget)const
{
    if(!objmgr.IsGameObjectForQuests(GetEntry()))
        return false;

    switch(GetGoType())
    {
        // scan GO chest with loot including quest items
        case GAMEOBJECT_TYPE_CHEST:
        {
            LootStore::iterator tab = LootTemplates_Gameobject.find(GetLootId());
            if (tab != LootTemplates_Gameobject.end())
            {
                for(LootStoreItemList::iterator item_iter = tab->second.begin(); item_iter != tab->second.end(); ++item_iter)
                {
                    if(pTarget->HasQuestForItem(item_iter->itemid))
                    return true;
                }
            }
            break;
        }
        default:
            break;
    }

    return false;
}
