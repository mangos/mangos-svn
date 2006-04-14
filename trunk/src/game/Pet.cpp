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
    for(uint32 i=0;i<PETMAXSPELLS;i++)
        m_spells[i]=0;
}

void Pet::SavePetToDB()
{
    if(!isPet())
        return;

    sDatabase.PExecute("DELETE FROM pets WHERE guid = '%u'",GetGUIDLow());

    std::stringstream ss;
    ss.rdbuf()->str("");
    ss << "INSERT INTO pets (entry,owner,level,exp,nextlvlexp,spell1,spell2,spell3,spell4,action,fealty,name,current) VALUES (";
    ss << GetEntry() << ","
        << GetUInt64Value(UNIT_FIELD_SUMMONEDBY) << ","
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

void Pet::LoadPetFromDB(Unit* owner, uint32 id)
{
    WorldPacket data;
    QueryResult *result = sDatabase.PQuery("SELECT * FROM pets WHERE id = '%u';", id);
    ASSERT(result);
    Field *fields = result->Fetch();

    uint32 guid=objmgr.GenerateLowGuid(HIGHGUID_UNIT);
    Create(guid, owner->GetMapId(), owner->GetPositionX(), owner->GetPositionY(),
        owner->GetPositionZ(), owner->GetOrientation(), fields[1].GetUInt32());

    uint32 petlevel=owner->getLevel();
    SetUInt32Value(UNIT_FIELD_LEVEL, fields[3].GetUInt32());
    SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner->GetGUID());
    SetUInt32Value(UNIT_NPC_FLAGS , 0);
    SetUInt32Value(UNIT_FIELD_HEALTH , 28 + 10 * petlevel);
    SetUInt32Value(UNIT_FIELD_MAXHEALTH , 28 + 10 * petlevel);

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

    m_name = fields[11].GetString();
    m_fealty = fields[12].GetUInt32();

    SetisPet(true);
    AIM_Initialize();
    m_spells[0] = fields[6].GetUInt32();
    m_spells[1] = fields[7].GetUInt32();
    m_spells[2] = fields[8].GetUInt32();
    m_spells[3] = fields[9].GetUInt32();
    m_actState = fields[10].GetUInt32();

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

        //Data is send 2x

        data << (uint64)GetGUID() << uint32(0x00000000) << uint32(0x00001000);

        data << uint16 (2) << uint16(Command << 8) << uint16 (1) << uint16(Command << 8) << uint16 (0) << uint16(Command << 8);

        uint16 SpellID;
        SpellID = fields[6].GetUInt16();                    //0x2DF3;	//Firebolt = correct
        data << uint16 (SpellID) << uint16 (0xC100);        //C100 = maybe group
        SpellID = fields[7].GetUInt16();                    //0x2DF7;	//Blood Pact = correct
        data << uint16 (SpellID) << uint16 (0xC100);
        SpellID = fields[8].GetUInt16();                    //0x2DFB;	//Fire Shield = correct
        data << uint16 (SpellID) << uint16 (0xC100);
        SpellID = fields[9].GetUInt16();                    //0x119F;	//Phase Shift = correct
        data << uint16 (SpellID) << uint16 (0xC100);

        data << uint16 (2) << uint16(State << 8) << uint16 (1) << uint16(State << 8) << uint16 (0) << uint16(State << 8);

        ((Player*)owner)->GetSession()->SendPacket(&data);
    }
}

void Pet::DeletePetFromDB()
{
    sDatabase.PExecute("DELETE FROM pets WHERE guid = '%u'", GetGUIDLow());
}
