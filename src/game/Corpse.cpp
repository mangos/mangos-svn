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
#include "Corpse.h"
#include "Player.h"
#include "UpdateMask.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Database/DatabaseEnv.h"
#include "Opcodes.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "GossipDef.h"
#include "World.h"

Corpse::Corpse( WorldObject *instantiator, CorpseType type ) : WorldObject( instantiator )
{
    m_objectType |= TYPE_CORPSE;
    m_objectTypeId = TYPEID_CORPSE;
                                                            // 2.3.2 - 0x58
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID | UPDATEFLAG_HASPOSITION);

    m_valuesCount = CORPSE_END;

    m_type = type;
    m_time = time(NULL) - CORPSE_RECLAIM_DELAY;             // to prevent resurrecting delay at load

    lootForBody = false;
}

Corpse::~Corpse()
{
}

void Corpse::AddToWorld()
{
    ///- Register the corpse for guid lookup
    if(!IsInWorld()) ObjectAccessor::Instance().AddObject(this);
    Object::AddToWorld();
}

void Corpse::RemoveFromWorld()
{
    ///- Remove the corpse from the accessor
    if(IsInWorld()) ObjectAccessor::Instance().RemoveObject(this);
    Object::RemoveFromWorld();
}

bool Corpse::Create( uint32 guidlow )
{
    Object::_Create(guidlow, HIGHGUID_CORPSE);
    return true;
}

bool Corpse::Create( uint32 guidlow, Player *owner, uint32 mapid, float x, float y, float z, float ang )
{
    SetInstanceId(owner->GetInstanceId());

    WorldObject::_Create(guidlow, HIGHGUID_CORPSE, mapid, x, y, z, ang);

    if(!IsPositionValid())
    {
        sLog.outError("ERROR: Corpse (guidlow %d, owner %s) not created. Suggested coordinates isn't valid (X: %d Y: ^%d)",guidlow,owner->GetName(),x,y);
        return false;
    }

    SetFloatValue( OBJECT_FIELD_SCALE_X, 1 );
    SetFloatValue( CORPSE_FIELD_POS_X, x );
    SetFloatValue( CORPSE_FIELD_POS_Y, y );
    SetFloatValue( CORPSE_FIELD_POS_Z, z );
    SetFloatValue( CORPSE_FIELD_FACING, ang );
    SetUInt64Value( CORPSE_FIELD_OWNER, owner->GetGUID() );

    m_grid = MaNGOS::ComputeGridPair(GetPositionX(), GetPositionY());

    return true;
}

void Corpse::SaveToDB()
{
    // prevent DB data inconsistance problems and duplicates
    CharacterDatabase.BeginTransaction();
    DeleteFromDB();

    std::ostringstream ss;
    ss  << "INSERT INTO `corpse` (`guid`,`player`,`position_x`,`position_y`,`position_z`,`orientation`,`zone`,`map`,`data`,`time`,`bones_flag`,`instance`) VALUES ("
        << GetGUIDLow() << ", " << GUID_LOPART(GetOwnerGUID()) << ", " << GetPositionX() << ", " << GetPositionY() << ", " << GetPositionZ() << ", "
        << GetOrientation() << ", "  << GetZoneId() << ", "  << GetMapId() << ", '";
    for(uint16 i = 0; i < m_valuesCount; i++ )
        ss << GetUInt32Value(i) << " ";
    ss << "', NOW(), " << int(GetType()) << ", " << int(GetInstanceId()) << ")";
    CharacterDatabase.Execute( ss.str().c_str() );
    CharacterDatabase.CommitTransaction();
}

void Corpse::DeleteBonesFromWorld()
{
    assert(GetType()==CORPSE_BONES);
    Corpse* corpse = ObjectAccessor::GetCorpse(*this, GetGUID());

    if (!corpse)
    {
        sLog.outError("Bones %u not found in world.", GetGUIDLow());
        return;
    }

    ObjectAccessor::Instance().AddObjectToRemoveList(this);
}

void Corpse::DeleteFromDB()
{
    if(GetType() == CORPSE_BONES)
        // only specific bones
        CharacterDatabase.PExecute("DELETE FROM `corpse` WHERE `guid` = '%d'", GetGUIDLow());
    else
        // all corpses (not bones)
        CharacterDatabase.PExecute("DELETE FROM `corpse` WHERE `player` = '%d' AND `bones_flag` = '0'",  GUID_LOPART(GetOwnerGUID()));
}

bool Corpse::LoadFromDB(uint32 guid, QueryResult *result, uint32 InstanceId)
{
    bool external = (result != NULL);
    if (!external)
        result = CharacterDatabase.PQuery("SELECT `position_x`,`position_y`,`position_z`,`orientation`,`map`,`data`,`bones_flag`,`instance` FROM `corpse` WHERE `guid` = '%u'",guid);

    if( ! result )
    {
        sLog.outError("ERROR: Corpse (GUID: %u) not found in table `corpse`, can't load. ",guid);
        return false;
    }

    Field *fields = result->Fetch();

    if(!LoadFromDB(guid,fields))
    {
        if (!external) delete result;
        return false;
    }

    if (!external) delete result;
    return true;
}

bool Corpse::LoadFromDB(uint32 guid, Field *fields)
{
    // SELECT `position_x`,`position_y`,`position_z`,`orientation`,`map`,`data`,`bones_flag`,`instance` FROM `corpse`
    float positionX = fields[0].GetFloat();
    float positionY = fields[1].GetFloat();
    float positionZ = fields[2].GetFloat();
    float ort       = fields[3].GetFloat();
    //uint32 zoneid   = fields[6].GetUInt32();
    uint32 mapid    = fields[4].GetUInt32();
    uint32 bones   = fields[6].GetUInt32();
    uint32 instanceid   = fields[7].GetUInt32();

    if(!LoadValues( fields[5].GetString() ))
    {
        sLog.outError("ERROR: Corpse #%d have broken data in `data` field. Can't be loaded.",guid);
        return false;
    }

    // overwrite possible wrong/corrupted guid
    SetUInt64Value(OBJECT_FIELD_GUID,MAKE_GUID(guid,HIGHGUID_CORPSE));

    // place
    SetInstanceId(instanceid);
    SetMapId(mapid);
    Relocate(positionX,positionY,positionZ,ort);

    if(!IsPositionValid())
    {
        sLog.outError("ERROR: Corpse (guidlow %d, owner %d) not created. Suggested coordinates isn't valid (X: %d Y: ^%d)",GetGUIDLow(),GUID_LOPART(GetOwnerGUID()),GetPositionX(),GetPositionY());
        return false;
    }

    // set before return to prevent attempting remove Corpse (CORPSE_RESURRECTABLE) from World at Load fail
    m_type = (bones == 0) ? CORPSE_RESURRECTABLE : CORPSE_BONES;
    m_grid = MaNGOS::ComputeGridPair(GetPositionX(), GetPositionY());

    return true;
}

bool Corpse::isVisibleForInState(Player const* u, bool inVisibleList) const
{
    return IsInWorld() && u->IsInWorld() && IsWithinDistInMap(u,World::GetMaxVisibleDistanceForObject()+(inVisibleList ? World::GetVisibleObjectGreyDistance() : 0.0f));
}
