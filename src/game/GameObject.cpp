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
#include "QuestDef.h"
#include "GameObject.h"
#include "ObjectMgr.h"
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
#include "Transports.h"

GameObject::GameObject( WorldObject *instantiator ) : WorldObject( instantiator )
{
    m_objectType |= TYPE_GAMEOBJECT;
    m_objectTypeId = TYPEID_GAMEOBJECT;

    m_valuesCount = GAMEOBJECT_END;
    m_respawnTime = 0;
    m_respawnDelayTime = 25;
    m_lootState = GO_CLOSED;
    m_usetimes = 0;
    m_spellId = 0;
    lootid=0;
}

GameObject::~GameObject()
{
    if(m_uint32Values)                                      // field array can be not exist if GameOBject not loaded
    {
        // crash possable at access to deleted GO in Unit::m_gameobj
        uint64 owner_guid = GetOwnerGUID();
        if(owner_guid)
        {
            Unit* owner = ObjectAccessor::Instance().GetUnit(*this,owner_guid);
            if(owner)
                owner->RemoveGameObject(this,false);
            else if(GUID_HIPART(owner_guid)!=HIGHGUID_PLAYER)
                sLog.outError("Delete GameObject (GUID: %u Entry: %u ) that have references in not found creature %u GO list. Crash possable later.",GetGUIDLow(),GetGOInfo()->id,GUID_LOPART(owner_guid));
        }
    }
}

bool GameObject::Create(uint32 guidlow, uint32 name_id, uint32 mapid, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 animprogress, uint32 dynflags)
{
    Relocate(x,y,z,ang);
    SetMapId(mapid);

    if(!IsPositionValid())
    {
        sLog.outError("ERROR: Gameobject (GUID: %u Entry: %u ) not created. Suggested coordinates isn't valid (X: %d Y: ^%d)",guidlow,name_id,x,y);
        return false;
    }

    Object::_Create(guidlow, HIGHGUID_GAMEOBJECT);

    m_DBTableGuid = guidlow;

    GameObjectInfo const* goinfo = objmgr.GetGameObjectInfo(name_id);

    if (!goinfo)
    {
        sLog.outErrorDb("Gameobject not created: it have not exist entry in `gameobject_template`. guidlow: %u id: %u map: %u  (X: %f Y: %f Z: %f) ang: %f rotation0: %f rotation1: %f rotation2: %f rotation3: %f",guidlow, name_id, mapid, x, y, z, ang, rotation0, rotation1, rotation2, rotation3);
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

    SetUInt32Value (GAMEOBJECT_STATE, 1);
    SetUInt32Value (GAMEOBJECT_TYPE_ID, goinfo->type);

    SetUInt32Value (GAMEOBJECT_ANIMPROGRESS, animprogress);
    SetUInt32Value (GAMEOBJECT_DYN_FLAGS, dynflags);
    return true;
}

void GameObject::Update(uint32 p_time)
{
    if (GUID_HIPART(GetGUID()) == HIGHGUID_TRANSPORT)
    {
        //((Transport*)this)->Update(p_time);
        return;
    }

    switch (m_lootState)
    {
        case GO_NOT_READY:
            if (GetGoType()==17)
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

                        WorldPacket data;
                        data.Initialize(SMSG_GAMEOBJECT_CUSTOM_ANIM);
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

                    switch (GetGoType())
                    {
                        case GAMEOBJECT_TYPE_FISHINGNODE:   //  can't fish now
                        {
                            Unit* caster = GetOwner();
                            if(caster && caster->GetTypeId()==TYPEID_PLAYER)
                            {
                                if(caster->m_currentSpell)
                                {
                                    caster->m_currentSpell->SendChannelUpdate(0);
                                    caster->m_currentSpell->finish(false);
                                }

                                WorldPacket data;
                                data.Initialize(SMSG_FISH_NOT_HOOKED);
                                ((Player*)caster)->GetSession()->SendPacket(&data);
                            }
                            m_lootState = GO_LOOTED;        // can be delete
                            return;
                        }
                        case GAMEOBJECT_TYPE_TRAP:
                            break;
                        default:
                            if(GetOwnerGUID())              // despawn timer
                            {
                                m_respawnTime = 0;
                                Delete();
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
            if(GetOwnerGUID())
            {
                m_respawnTime = 0;
                Delete();
                return;
            }

            loot.clear();
            SetLootState(GO_CLOSED);

            SendDestroyObject(GetGUID());
            m_respawnTime = time(NULL) + m_respawnDelayTime;

            // if option not set then object will be ssaved at grif unload
            if(sWorld.getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATLY))
                SaveRespawnTime();

            break;
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
        Cell cell = RedZone::GetZone(p);
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
            owner->CastSpell(ok, GetGOInfo()->sound3, true);
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
    if(m_respawnTime > 0 && !GetOwnerGUID())
        return;

    SendDestroyObject(GetGUID());

    MapManager::Instance().GetMap(GetMapId(), this)->Add(this);
}

void GameObject::CountUseTimes()
{
    m_usetimes += 1;
}

void GameObject::Delete()
{

    SendObjectDeSpawnAnim(GetGUID());

    SetUInt32Value(GAMEOBJECT_STATE, 1);
    SetUInt32Value(GAMEOBJECT_FLAGS, m_flags);

    SendDestroyObject(GetGUID());
    //TODO: set timestamp
    RemoveFromWorld();
    ObjectAccessor::Instance().AddObjectToRemoveList(this);
}

void GameObject::getFishLoot(Loot *fishloot)
{
    uint32 zone = GetZoneId();
    FillLoot(fishloot, zone, LootTemplates_Fishing);
}

void GameObject::SaveToDB()
{
    std::ostringstream ss;

    sDatabase.BeginTransaction();
    sDatabase.PExecute("DELETE FROM `gameobject` WHERE `guid` = '%u'", GetGUIDLow());

    const GameObjectInfo *goI = GetGOInfo();

    if (goI)
    {
        ss << "INSERT INTO `gameobject` VALUES ( "
            << GetGUIDLow() << ", "
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
            << lootid <<", "
            << m_respawnDelayTime << ", "
            << GetUInt32Value (GAMEOBJECT_ANIMPROGRESS) << ", "
            << GetUInt32Value (GAMEOBJECT_DYN_FLAGS) << ")";;

        sDatabase.Execute( ss.str( ).c_str( ) );
    }
    sDatabase.CommitTransaction();
}

bool GameObject::LoadFromDB(uint32 guid, QueryResult *result, uint32 InstanceId)
{
    bool external = (result != NULL);
    if (!external)
        //                                0    1     2            3            4            5             6           7           8           9           10     11              12             13         14            15
        result = sDatabase.PQuery("SELECT `id`,`map`,`position_x`,`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`loot`,`spawntimesecs`,`animprogress`,`dynflags`,`respawntime`,`guid` "
            "FROM `gameobject` LEFT JOIN `gameobject_respawn` ON ((`gameobject`.`guid`=`gameobject_respawn`.`guid`) AND (`gameobject_respawn`.`instance` = '%u')) WHERE `gameobject`.`guid` = '%u'", InstanceId, guid);

    if( !result )
    {
        sLog.outErrorDb("ERROR: Gameobject (GUID: %u) not found in table `gameobject`, can't load. ",guid);
        return false;
    }

    Field *fields = result->Fetch();
    uint32 entry = fields[0].GetUInt32();
    uint32 map_id=fields[1].GetUInt32();
    float x = fields[2].GetFloat();
    float y = fields[3].GetFloat();
    float z = fields[4].GetFloat();
    float ang = fields[5].GetFloat();

    float rotation0 = fields[6].GetFloat();
    float rotation1 = fields[7].GetFloat();
    float rotation2 = fields[8].GetFloat();
    float rotation3 = fields[9].GetFloat();

    uint32 animprogress = fields[12].GetUInt32();
    uint32 dynflags = fields[13].GetUInt32();

    m_DBTableGuid = guid;
    if (InstanceId) guid = objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT);
    SetInstanceId(InstanceId);

    if (!Create(guid,entry, map_id, x, y, z, ang, rotation0, rotation1, rotation2, rotation3, animprogress, dynflags) )
    {
        if (!external) delete result;
        return false;
    }

    lootid=fields[10].GetUInt32();
    m_respawnDelayTime=fields[11].GetUInt32();
    m_respawnTime=fields[14].GetUInt64();
    if(m_respawnTime && m_respawnTime <= time(NULL))        // ready to respawn
    {
        m_respawnTime = 0;
        sDatabase.PExecute("DELETE FROM `gameobject_respawn` WHERE `guid` = '%u' AND `instance` = '%u'", m_DBTableGuid, GetInstanceId());
    }

    if (!external) delete result;

    _LoadQuests();
    return true;
}

void GameObject::DeleteFromDB()
{
    sDatabase.PExecute("DELETE FROM `gameobject` WHERE `guid` = '%u'", GetGUIDLow());
    sDatabase.PExecute("DELETE FROM `gameobject_respawn` WHERE `guid` = '%u' AND `instance` = '%u'", m_DBTableGuid, GetInstanceId());
}

GameObjectInfo const *GameObject::GetGOInfo() const
{
    return objmgr.GetGameObjectInfo(GetUInt32Value (OBJECT_FIELD_ENTRY));
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/

void GameObject::_LoadQuests()
{
    mQuests.clear();
    mInvolvedQuests.clear();

    Field *fields;
    Quest *pQuest;

    QueryResult *result = sDatabase.PQuery("SELECT `quest` FROM `gameobject_questrelation` WHERE `id` = '%u'", GetEntry ());

    if(result)
    {
        do
        {
            fields = result->Fetch();
            pQuest = objmgr.QuestTemplates[ fields[0].GetUInt32() ];
            if (!pQuest) continue;

            addQuest(pQuest->GetQuestId());
        }
        while( result->NextRow() );

        delete result;
    }

    QueryResult *result1 = sDatabase.PQuery("SELECT `quest` FROM `gameobject_involvedrelation` WHERE `id` = '%u'", GetEntry ());

    if(!result1) return;

    do
    {
        fields = result1->Fetch();
        pQuest = objmgr.QuestTemplates[ fields[0].GetUInt32() ];
        if (!pQuest) continue;

        addInvolvedQuest(pQuest->GetQuestId());
    }
    while( result1->NextRow() );

    delete result1;
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
    return ObjectAccessor::Instance().GetUnit(*this, GetOwnerGUID());
}

void GameObject::SaveRespawnTime()
{
    if(m_respawnTime > time(NULL) && !GetOwnerGUID())
    {
        sDatabase.PExecute("DELETE FROM `gameobject_respawn` WHERE `guid` = '%u' AND `instance` = '%u'", m_DBTableGuid, GetInstanceId());
        sDatabase.PExecute("INSERT INTO `gameobject_respawn` VALUES ( '%u', '" I64FMTD "', '%u' )", m_DBTableGuid, uint64(m_respawnTime), GetInstanceId());
    }
}
