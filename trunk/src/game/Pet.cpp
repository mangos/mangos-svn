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

bool Pet::LoadPetFromDB( Unit* owner, uint32 petentry )
{
    WorldPacket data;
    uint32 ownerid = owner->GetGUIDLow();

    QueryResult *result;

    if(petentry)
        // known entry
        result = sDatabase.PQuery("SELECT `id`,`entry`,`owner`,`level`,`exp`,`nextlvlexp`,`spell1`,`spell2`,`spell3`,`spell4`,`action`,`fealty`,`loyalty`,`trainpoint`,`current`,`name` FROM `character_pet` WHERE `owner` = '%u' AND `entry` = '%u'",ownerid, petentry );
    else
        // current pet
        result = sDatabase.PQuery("SELECT `id`,`entry`,`owner`,`level`,`exp`,`nextlvlexp`,`spell1`,`spell2`,`spell3`,`spell4`,`action`,`fealty`,`loyalty`,`trainpoint`,`current`,`name` FROM `character_pet` WHERE `owner` = '%u' AND `current` = '1'",ownerid );

    if(!result)
        return false;

    Field *fields = result->Fetch();

    // update for case "current = 1"
    petentry = fields[1].GetUInt32();
    if(!petentry)
        return false;

    float px, py, pz;
    owner->GetClosePoint(NULL, px, py, pz);
    uint32 guid=objmgr.GenerateLowGuid(HIGHGUID_UNIT);
    if(!Create(guid, owner->GetMapId(), px, py, pz, owner->GetOrientation(), petentry))
    {
        delete result;
        return false;
    }

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
    SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner->GetGUID());
    uint32 petlevel=fields[3].GetUInt32();
    SetUInt32Value(UNIT_NPC_FLAGS , 0);
    SetName(fields[15].GetString());
    if(owner->getClass() == CLASS_WARLOCK)
    {
        petlevel=owner->getLevel();

        SetUInt32Value(UNIT_FIELD_BYTES_0,2048);
        SetUInt32Value(UNIT_FIELD_PETNUMBER, fields[0].GetUInt32() );
        SetStat(STAT_STRENGTH,22);
        SetStat(STAT_AGILITY,22);
        SetStat(STAT_STAMINA,25);
        SetStat(STAT_INTELLECT,28);
        SetStat(STAT_SPIRIT,27);
        SetArmor(petlevel*50);
    }
    else if(owner->getClass() == CLASS_HUNTER && cinfo->type == CREATURE_TYPE_BEAST)
    {
        SetUInt32Value(UNIT_FIELD_BYTES_1,(fields[12].GetUInt32()<<8));
        SetFloatValue(UNIT_FIELD_MINDAMAGE, cinfo->mindmg + float(petlevel-cinfo->minlevel)*1.5f);
        SetFloatValue(UNIT_FIELD_MAXDAMAGE, cinfo->maxdmg + float(petlevel-cinfo->minlevel)*1.5f);
        SetUInt32Value(UNIT_MOD_CAST_SPEED, fields[13].GetUInt32() );
        SetUInt32Value(UNIT_TRAINING_POINTS, (getLevel()<<16) + getUsedTrainPoint() );
        SetUInt32Value(UNIT_FIELD_PETNUMBER, fields[0].GetUInt32() );
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
    SetLevel( petlevel);
    SetHealth( 28 + 10 * petlevel);
    SetMaxHealth( 28 + 10 * petlevel);
    SetMaxPower(POWER_MANA, 28 + 10 * petlevel);
    SetPower(   POWER_MANA, 28 + 10 * petlevel);
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,0);
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, fields[4].GetUInt32());
    SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, fields[5].GetUInt32());
    SetUInt64Value(UNIT_FIELD_CREATEDBY, owner->GetGUID());

    m_fealty = fields[11].GetUInt32();

    m_spells[0] = fields[6].GetUInt32();
    m_spells[1] = fields[7].GetUInt32();
    m_spells[2] = fields[8].GetUInt32();
    m_spells[3] = fields[9].GetUInt32();
    m_actState = fields[10].GetUInt32();

    // set current pet as current
    if(fields[14].GetUInt32() != 1)
    {
        sDatabase.PExecute("UPDATE `character_pet` SET `current` = '0' WHERE `owner` = '%u' AND `current` <> '0' AND `entry` <> '%u'",ownerid, petentry);
        sDatabase.PExecute("UPDATE `character_pet` SET `current` = '1' WHERE `owner` = '%u' AND `entry` = '%u'",ownerid, petentry);
    }

    delete result;

    AIM_Initialize();
    MapManager::Instance().GetMap(owner->GetMapId())->Add((Creature*)this);
    owner->SetPet(this);
    sLog.outDebug("New Pet has guid %u", GetGUIDLow());

    //summon imp Template ID is 416
    if(owner->GetTypeId() == TYPEID_PLAYER)
    {
        /*        std::string name;
                name = ((Player*)owner)->GetName();
                name.append("'s Pet");
                SetName( name );*/
        ((Player*)owner)->PetSpellInitialize();
    }
    return true;
}

void Pet::SaveToDB()
{
    if(!isPet())
        return;
    if(GetEntry())
        return;

    uint32 owner = GUID_LOPART(GetOwnerGUID());
    sDatabase.PExecute("DELETE FROM `character_pet` WHERE `owner` = '%u' AND `entry` = '%u'", owner,GetEntry() );
    sDatabase.PExecute("UPDATE `character_pet` SET `current` = 0 WHERE `owner` = '%u' AND `current` = 1", owner );
    sDatabase.PExecute("INSERT INTO `character_pet` (`entry`,`owner`,`level`,`exp`,`nextlvlexp`,`spell1`,`spell2`,`spell3`,`spell4`,`action`,`fealty`,`loyalty`,`trainpoint`,`name`,`current`) VALUES (%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s',1)",
        GetEntry(), owner, getLevel(), GetUInt32Value(UNIT_FIELD_PETEXPERIENCE), GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP),
        m_spells[0], m_spells[1], m_spells[2], m_spells[3], m_actState, GetPower(POWER_HAPPINESS),getloyalty(),getUsedTrainPoint(), GetName());
}

void Pet::DeleteFromDB()
{
    uint32 owner = GUID_LOPART(GetOwnerGUID());
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

void Pet::setDeathState(DeathState s)                       // overwrite virtual Creature::setDeathState and Unit::setDeathState
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
