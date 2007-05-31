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
#include "RedZoneDistrict.h"

Corpse::Corpse( WorldObject *instantiator, CorpseType type ) : WorldObject( instantiator )
{
    m_objectType |= TYPE_CORPSE;
    m_objectTypeId = TYPEID_CORPSE;
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_ALL | UPDATEFLAG_HASPOSITION); // 2.0.10 - 0x58

    m_valuesCount = CORPSE_END;

    m_POI = false;
    m_type = type;
    m_time = time(NULL) - CORPSE_RECLAIM_DELAY;             // to prevent resurrecting delay at load
}

Corpse::~Corpse()
{
}

bool Corpse::Create( uint32 guidlow )
{
    Object::_Create(guidlow, HIGHGUID_CORPSE);
    return true;
}

bool Corpse::Create( uint32 guidlow, Player *owner, uint32 mapid, float x, float y, float z, float ang )
{
    SetInstanceId(owner->GetInstanceId());

    WorldObject::_Create(guidlow, HIGHGUID_CORPSE, mapid, x, y, z, ang, (uint8)-1);

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
    sDatabase.BeginTransaction();
    DeleteFromDB(false);

    std::ostringstream ss;
    ss  << "INSERT INTO `corpse` (`guid`,`player`,`position_x`,`position_y`,`position_z`,`orientation`,`zone`,`map`,`data`,`time`,`bones_flag`,`instance`) VALUES ("
        << GetGUIDLow() << ", " << GUID_LOPART(GetOwnerGUID()) << ", " << GetPositionX() << ", " << GetPositionY() << ", " << GetPositionZ() << ", "
        << GetOrientation() << ", "  << GetZoneId() << ", "  << GetMapId() << ", '";
    for(uint16 i = 0; i < m_valuesCount; i++ )
        ss << GetUInt32Value(i) << " ";
    ss << "', NOW(), " << int(GetType()) << ", " << int(GetInstanceId()) << ")";
    sDatabase.Execute( ss.str().c_str() );
    sDatabase.CommitTransaction();
}

void Corpse::DeleteBonesFromWorld()
{
    assert(GetType()==CORPSE_BONES);
    CorpsePtr corpse = MapManager::Instance().GetMap(GetMapId(), this)->GetObjectNear<Corpse>(*this, GetGUID());

    if (!corpse)
    {
        sLog.outError("Bones %u not found in world.", GetGUIDLow());
    }
    else
    {
        ObjectAccessor::Instance().RemoveBonesFromPlayerView(corpse);
        ObjectAccessor::Instance().AddObjectToRemoveList(this);
    }

    RemoveFromWorld();
}

void Corpse::DeleteFromDB(bool inner_transaction)
{
    std::ostringstream ss;

    if(inner_transaction)
        sDatabase.BeginTransaction();

    if(GetType() == CORPSE_BONES)
        // only specific bones
        ss  << "DELETE FROM `corpse` WHERE `guid` = '" << GetGUIDLow() << "'";
    else
        // all corpses (not bones)
        ss  << "DELETE FROM `corpse` WHERE `player` = '" << GUID_LOPART(GetOwnerGUID()) << "' AND `bones_flag` = '0'";
    sDatabase.Execute( ss.str().c_str() );

    if(inner_transaction)
        sDatabase.CommitTransaction();
}

bool Corpse::LoadFromDB(uint32 guid, QueryResult *result, uint32 InstanceId)
{
    bool external = (result != NULL);
    if (!external)
        result = sDatabase.PQuery("SELECT `position_x`,`position_y`,`position_z`,`orientation`,`map`,`data`,`bones_flag`,`instance` FROM `corpse` WHERE `guid` = '%u'",guid);

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

void Corpse::UpdateForPlayer(Player* player, bool first)
{
    /*if(player && player->GetGUID() == GetOwnerGUID() && IsInMap(player))
    {
        bool POI_range = (GetDistance2dSq(player) > CORPSE_RECLAIM_RADIUS*CORPSE_RECLAIM_RADIUS);

        if(first || POI_range && !m_POI)
        {
            std::string corpsename = player->GetName();
            corpsename.append(" corpse.");
            player->PlayerTalkClass->SendPointOfInterest( GetPositionX(), GetPositionY(), ICON_POI_TOMB, 6, 30, corpsename.c_str());
        }

        m_POI = POI_range;
    }*/
}

void Corpse::_ConvertCorpseToBones()
{
    // corpse can be converted in another thread already
    if(GetType()!=CORPSE_RESURRECTABLE)
        return;

    Player* player = ObjectAccessor::Instance().FindPlayer(GetOwnerGUID());
    CorpsePtr corpse = ObjectAccessor::Instance().GetCorpseForPlayerGUID(GetOwnerGUID());
    if(!corpse)
    {
        sLog.outError("ERROR: Try remove corpse that not in map for GUID %ul", GetOwnerGUID());
        return;
    }

    if ((&*corpse) != this)
    {
        sLog.outError("ERROR: Found another corpse while deleting corpse for GUID %ul", GetOwnerGUID());
        return;
    }

    /*// Removing outdated POI if at same map
    if(player && IsInMap(player))
        player->PlayerTalkClass->SendPointOfInterest( GetPositionX(), GetPositionY(), ICON_POI_TOMB, 0, 30, "" );*/

    DEBUG_LOG("Deleting Corpse and spawning bones.\n");

    // remove corpse from player_guid -> corpse map
    ObjectAccessor::Instance().RemoveCorpse(this);

    // remove resurrectble corpse from grid object registry (loaded state checked into call)
    MapManager::Instance().GetMap(GetMapId(), this)->Remove(corpse,false);

    // remove corpse from DB
    DeleteFromDB();

    // Create bones, don't change Corpse
    CorpsePtr bones = CorpsePtr(new Corpse(this));
    bones->Create(GetGUIDLow());

    for (int i = 0; i < CORPSE_END; i++)
    {
        bones->SetUInt32Value(i, GetUInt32Value(i));
    }
    bones->m_grid = m_grid;
    bones->m_time = m_time;
    bones->m_inWorld = m_inWorld;
    bones->Relocate(GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
    bones->SetMapId(GetMapId());

    // update data to bone state
    bones->m_type = CORPSE_BONES;

    uint32 flags = 0x05;
    if(player->InBattleGround())
        flags |= 0x20;                                  // make it lootable for money, TODO: implement effect
    bones->SetUInt32Value(CORPSE_FIELD_FLAGS, flags);

    bones->SetUInt64Value(CORPSE_FIELD_OWNER, 0);

    for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if(corpse->GetUInt32Value(CORPSE_FIELD_ITEM + i))
            bones->SetUInt32Value(CORPSE_FIELD_ITEM + i, 0);
    }

    // add bones to DB
    bones->SaveToDB();

    // add bones in grid store if grid loaded where corpse placed
    if(!MapManager::Instance().GetMap(bones->GetMapId(), &*bones)->IsRemovalGrid(bones->GetPositionX(),bones->GetPositionY()))
    {
        MapManager::Instance().GetMap(GetMapId(), &*bones)->Add(bones);
    }
    // or prepare to delete at next tick if grid not loaded
    else
        bones->DeleteBonesFromWorld();
}
