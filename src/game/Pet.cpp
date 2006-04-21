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
#include "Log.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "MapManager.h"

Pet::Pet()
{
    m_name = "Pet";
    m_actState = STATE_RA_FOLLOW;
    m_fealty = 0;
    for(uint32 i=0;i<UNIT_MAX_SPELLS;i++)
        m_spells[i]=0;
}

Unit *Pet::GetOwner()
{
    uint64 ownerid = GetUInt64Value(UNIT_FIELD_SUMMONEDBY);
	if(!ownerid)
		return NULL;
	return ObjectAccessor::Instance().GetUnit(*this, ownerid);
}

void Pet::SavePetToDB()
{
    if(!isPet())
        return;

    uint32 owner = uint32(GUID_LOPART(GetUInt64Value(UNIT_FIELD_SUMMONEDBY)));
    sDatabase.PExecute("DELETE FROM pets WHERE owner = '%u' AND current = 1", owner );

    std::stringstream ss;
    ss.rdbuf()->str("");
    ss << "INSERT INTO pets (entry,owner,level,exp,nextlvlexp,spell1,spell2,spell3,spell4,action,fealty,name,current) VALUES (";
    ss << GetEntry() << ","
        << owner << ","
        << GetUInt32Value(UNIT_FIELD_LEVEL) << ","
        << GetUInt32Value(UNIT_FIELD_PETEXPERIENCE) << ","
        << GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP) << ","
        << m_spells[0] << ","
        << m_spells[1] << ","
        << m_spells[2] << ","
        << m_spells[3] << ","
        << m_actState << ","
        << m_fealty << ",'"
        << m_name <<"',";
    ss << "1 )";
    sDatabase.Execute( ss.str( ).c_str( ) );
}

bool Pet::LoadPetFromDB( Unit* owner )
{
    WorldPacket data;
    uint32 ownerid = owner->GetGUIDLow();
    QueryResult *result = sDatabase.PQuery("SELECT * FROM pets WHERE owner = '%u' AND current = 1;", ownerid );
    if(!result)
        return false;
    Field *fields = result->Fetch();

    float px, py, pz;
    owner->GetClosePoint(NULL, px, py, pz);
    uint32 guid=objmgr.GenerateLowGuid(HIGHGUID_UNIT);
    Create(guid, owner->GetMapId(), px, py, pz, owner->GetOrientation(), fields[1].GetUInt32());

    uint32 petlevel=owner->getLevel();
    SetUInt32Value(UNIT_FIELD_LEVEL, fields[3].GetUInt32());
    SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner->GetGUID());
    SetUInt32Value(UNIT_NPC_FLAGS , 0);
    SetUInt32Value(UNIT_FIELD_HEALTH , 28 + 10 * petlevel);
    SetUInt32Value(UNIT_FIELD_MAXHEALTH , 28 + 10 * petlevel);
    SetUInt32Value(UNIT_FIELD_POWER1 , 28 + 10 * petlevel);
    SetUInt32Value(UNIT_FIELD_MAXPOWER1 , 28 + 10 * petlevel);

    SetUInt32Value(UNIT_FIELD_BYTES_0,2048);

    SetUInt32Value(UNIT_FIELD_FLAGS,0);

    SetUInt32Value(UNIT_FIELD_BYTES_1,0);
    SetUInt32Value(UNIT_FIELD_PETNUMBER, guid );
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,5);
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, fields[4].GetUInt32());
    SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, fields[5].GetUInt32());
    //SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);
    SetUInt32Value(UNIT_FIELD_STAT0,22);
    SetUInt32Value(UNIT_FIELD_STAT1,22);
    //SetUInt32Value(UNIT_FIELD_STAT2,25);
    //SetUInt32Value(UNIT_FIELD_STAT3,28);
    SetUInt32Value(UNIT_FIELD_STAT4,27);

    m_fealty = fields[11].GetUInt32();
    m_name = fields[12].GetString();

    m_spells[0] = fields[6].GetUInt32();
    m_spells[1] = fields[7].GetUInt32();
    m_spells[2] = fields[8].GetUInt32();
    m_spells[3] = fields[9].GetUInt32();
    m_actState = fields[10].GetUInt32();
    SetisPet(true);
    AIM_Initialize();
    MapManager::Instance().GetMap(owner->GetMapId())->Add((Creature*)this);
    owner->SetUInt64Value(UNIT_FIELD_SUMMON, GetGUID());
    sLog.outDebug("New Pet has guid %u", GetGUID());

    //summon imp Template ID is 416
    if(owner->GetTypeId() == TYPEID_PLAYER)
    {
        uint16 Command = 7;
        uint16 State = 6;

        sLog.outDebug("Pet Spells Groups");
        data.clear();
        data.Initialize(SMSG_PET_SPELLS);

        data << (uint64)GetGUID() << uint32(0x00000000) << uint32(0x00001000);

        data << uint16 (2) << uint16(Command << 8) << uint16 (1) << uint16(Command << 8) << uint16 (0) << uint16(Command << 8);

        for(uint32 i=0;i<UNIT_MAX_SPELLS;i++)
                                                            //C100 = maybe group
            data << uint16 (m_spells[i]) << uint16 (0xC100);

        data << uint16 (2) << uint16(State << 8) << uint16 (1) << uint16(State << 8) << uint16 (0) << uint16(State << 8);

        ((Player*)owner)->GetSession()->SendPacket(&data);
    }
    return true;
}

void Pet::DeletePetFromDB()
{
    uint32 owner = uint32(GUID_LOPART(GetUInt64Value(UNIT_FIELD_SUMMONEDBY)));
    sDatabase.PExecute("DELETE FROM pets WHERE owner = '%u' AND current = 1", owner );
}
