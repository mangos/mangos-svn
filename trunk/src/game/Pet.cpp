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
    m_isPet = true;
    m_name = "Pet";
    m_actState = STATE_RA_FOLLOW;
    m_fealty = 0;
    for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
        m_spells[i]=0;
}

Unit *Pet::GetOwner()
{
    uint64 ownerid = GetUInt64Value(UNIT_FIELD_SUMMONEDBY);
    if(!ownerid)
        return NULL;
    return ObjectAccessor::Instance().GetUnit(*this, ownerid);
}

bool Pet::LoadPetFromDB( Unit* owner )
{
    WorldPacket data;
    uint32 ownerid = owner->GetGUIDLow();

    QueryResult *result = sDatabase.PQuery("SELECT `id`,`entry`,`owner`,`level`,`exp`,`nextlvlexp`,`spell1`,`spell2`,`spell3`,`spell4`,`action`,`fealty`,`name`,`current` FROM `character_pet` WHERE `owner` = '%u' AND `current` = 1", ownerid );

    if(!result)
        return false;

    Field *fields = result->Fetch();

    float px, py, pz;
    owner->GetClosePoint(NULL, px, py, pz);
    uint32 guid=objmgr.GenerateLowGuid(HIGHGUID_UNIT);
    if(!Create(guid, owner->GetMapId(), px, py, pz, owner->GetOrientation(), fields[1].GetUInt32()))
    {
        delete result;
        return false;
    }

    std::string name;
    if(owner->GetTypeId() == TYPEID_PLAYER)
        name = ((Player*)owner)->GetName();
    else
        name = ((Creature*)owner)->GetCreatureInfo()->Name;
    name.append("'s Pet");
    SetName( name );

    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,owner->getFaction());

    CreatureInfo const *cinfo = GetCreatureInfo();
    if(cinfo->type == CREATURE_TYPE_CRITTER)
    {
        AIM_Initialize();
        MapManager::Instance().GetMap(owner->GetMapId())->Add((Creature*)this);
        owner->SetPet(this);
        if(owner->GetTypeId() == TYPEID_PLAYER)
        {
            ((Player*)owner)->PetSpellInitialize();
        }
        return true;
    }
    uint32 petlevel=fields[3].GetUInt32();
    SetLevel( petlevel);
    SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner->GetGUID());
    SetUInt32Value(UNIT_NPC_FLAGS , 0);
    SetHealth( 28 + 10 * petlevel);
    SetMaxHealth( 28 + 10 * petlevel);
    if(owner->getClass() == CLASS_WARLOCK)
    {
        SetUInt32Value(UNIT_FIELD_BYTES_0,2048);
        SetStat(STAT_STRENGTH,22);
        SetStat(STAT_AGILITY,22);
        SetStat(STAT_STAMINA,25);
        SetStat(STAT_INTELLECT,28);
        SetStat(STAT_SPIRIT,27);
        SetArmor(petlevel*50);
    }
    else if(owner->getClass() == CLASS_HUNTER)
    {
        SetUInt32Value(UNIT_FIELD_PETNUMBER, guid );
        SetMaxPower(POWER_HAPPINESS,1000000);
        SetPower(   POWER_HAPPINESS,fields[11].GetUInt32());
        setPowerType(POWER_FOCUS);
        SetStat(STAT_STRENGTH,uint32(20+petlevel*1.55));
        SetStat(STAT_AGILITY,uint32(20+petlevel*0.64));
        SetStat(STAT_STAMINA,uint32(20+petlevel*1.27));
        SetStat(STAT_INTELLECT,uint32(20+petlevel*0.18));
        SetStat(STAT_SPIRIT,uint32(20+petlevel*0.36));
        SetArmor(petlevel*50);
    }

    SetMaxPower(POWER_MANA, 28 + 10 * petlevel);
    SetPower(   POWER_MANA, 28 + 10 * petlevel);
    SetUInt32Value(UNIT_FIELD_FLAGS,0);
    SetUInt32Value(UNIT_FIELD_BYTES_1,0);
    SetUInt32Value(UNIT_FIELD_BYTES_2,1);
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,5);
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, fields[4].GetUInt32());
    SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, fields[5].GetUInt32());
    //SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);

    m_fealty = fields[11].GetUInt32();
    m_name = fields[12].GetCppString();

    m_spells[0] = fields[6].GetUInt32();
    m_spells[1] = fields[7].GetUInt32();
    m_spells[2] = fields[8].GetUInt32();
    m_spells[3] = fields[9].GetUInt32();
    m_actState = fields[10].GetUInt32();

    delete result;

    AIM_Initialize();
    MapManager::Instance().GetMap(owner->GetMapId())->Add((Creature*)this);
    owner->SetPet(this);
    sLog.outDebug("New Pet has guid %u", GetGUID());

    //summon imp Template ID is 416
    if(owner->GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)owner)->PetSpellInitialize();
    }
    return true;
}

void Pet::SaveToDB()
{
    if(!isPet())
        return;

    uint32 owner = uint32(GUID_LOPART(GetUInt64Value(UNIT_FIELD_SUMMONEDBY)));
    sDatabase.PExecute("DELETE FROM `character_pet` WHERE `owner` = '%u' AND `current` = 1", owner );

    sDatabase.PExecute("INSERT INTO `character_pet` (`entry`,`owner`,`level`,`exp`,`nextlvlexp`,`spell1`,`spell2`,`spell3`,`spell4`,`action`,`fealty`,`name`,`current`) VALUES (%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,\"%s\",1)",
        GetEntry(), owner, getLevel(), GetUInt32Value(UNIT_FIELD_PETEXPERIENCE), GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP),
        m_spells[0], m_spells[1], m_spells[2], m_spells[3], m_actState, GetPower(POWER_HAPPINESS), m_name.c_str());
}

void Pet::DeleteFromDB()
{
    uint32 owner = uint32(GUID_LOPART(GetUInt64Value(UNIT_FIELD_SUMMONEDBY)));
    sDatabase.PExecute("DELETE FROM `character_pet` WHERE `owner` = '%u' AND `current` = 1", owner );
}

/*void Pet::SendPetQuery()
{
    Unit *player = GetOwner();
    if(player->GetTypeId() != TYPEID_PLAYER)
        return;
    char *subname = "Pet";
    CreatureInfo *ci = objmgr.GetCreatureTemplate(GetEntry());

    WorldPacket data;
    data.Initialize( SMSG_CREATURE_QUERY_RESPONSE );
    data << (uint32)GetEntry();
    data << m_name.c_str();
    data << uint8(0) << uint8(0) << uint8(0);
    data << subname;

    uint32 wdbFeild11=0,wdbFeild12=0;

    data << ci->flag1;                                      //flags          wdbFeild7=wad flags1
    data << uint32(ci->type);                               //creatureType   wdbFeild8
    data << (uint32)ci->family;                             //family         wdbFeild9
    data << (uint32)ci->rank;                               //rank           wdbFeild10
    data << (uint32)wdbFeild11;                             //unknow         wdbFeild11
    data << (uint32)wdbFeild12;                             //unknow         wdbFeild12
    data << ci->DisplayID;                                  //DisplayID      wdbFeild13

    data << (uint16)ci->civilian;                           //wdbFeild14

    player->GetSession()->SendPacket( &data );
}
*/

void Pet::setDeathState(DeathState s)                    // overwrite virtual Creature::setDeathState and Unit::setDeathState
{
    Creature::setDeathState(s);
    if(s == JUST_DIED)
    {
        Unit* owner = GetOwner();
        if(owner && owner->GetTypeId() == TYPEID_PLAYER)
            ((Player*)owner)->UnsummonPet();
        else
        {
            if(owner)
                owner->SetPet(0);

            ObjectAccessor::Instance().AddObjectToRemoveList(this);
        }
    }
}

void Pet::Unsummon()
{
    Unit* owner = GetOwner();

    if(owner)
    {
        if(owner->GetTypeId()==TYPEID_PLAYER)
            ((Player*)owner)->UnsummonPet(this);
        else
        {
            owner->SetPet(0);
            ObjectAccessor::Instance().AddObjectToRemoveList(this);
        }
    }
    else
        ObjectAccessor::Instance().AddObjectToRemoveList(this);
}
