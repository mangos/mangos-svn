/* Corpse.cpp
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

#include "Common.h"
#include "Corpse.h"
#include "Player.h"
#include "UpdateMask.h"
#include "ObjectMgr.h"
#include "Database/DatabaseEnv.h"
#include "Opcodes.h"
#include "WorldSession.h"
#include "WorldPacket.h"

Corpse::Corpse() : Object()
{
    m_objectType |= TYPE_CORPSE;
    m_objectTypeId = TYPEID_CORPSE;

    m_valuesCount = CORPSE_END;
}


void Corpse::Create( uint32 guidlow )
{
    Object::_Create(guidlow, HIGHGUID_CORPSE);
}


void Corpse::Create( uint32 guidlow, Player *owner, uint32 mapid, float x, float y, float z, float ang )
{
    Object::_Create(guidlow, HIGHGUID_CORPSE, mapid, x, y, z, ang, -1);

    SetFloatValue( OBJECT_FIELD_SCALE_X, 1 );
    SetFloatValue( CORPSE_FIELD_POS_X, x );
    SetFloatValue( CORPSE_FIELD_POS_Y, y );
    SetFloatValue( CORPSE_FIELD_POS_Z, z );
    SetFloatValue( CORPSE_FIELD_FACING, ang );
    SetUInt64Value( CORPSE_FIELD_OWNER, owner->GetGUID() );
}


void Corpse::SaveToDB()
{
    //save corpse to DB
    std::stringstream ss;
    ss << "DELETE FROM corpses WHERE guid = " << GetGUIDLow();
    sDatabase.Execute( ss.str( ).c_str( ) );

    ss.rdbuf()->str("");
    ss << "INSERT INTO corpses (guid, positionX, positionY, positionZ, orientation, zoneId, mapId,  data) VALUES ("
        << GetGUIDLow() << ", '" << GetPositionX() << "', '" << GetPositionY() << "', '" << GetPositionZ() << "', '" << GetOrientation() << "', '" << GetZoneId() << "', '" << GetMapId() << "', '";

    for(uint16 i = 0; i < m_valuesCount; i++ )
        ss << GetUInt32Value(i) << " ";

    ss << "' )";

    sDatabase.Execute( ss.str().c_str() );
}


void Corpse::DeleteFromDB()
{
    //delete corpse from db when its not needed anymore
    char sql[256];

    sprintf(sql, "DELETE FROM corpses WHERE guid=%u", GetGUIDLow());
    sDatabase.Execute(sql);
}
