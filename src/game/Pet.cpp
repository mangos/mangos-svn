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
#include "Log.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "MapManager.h"
#include "Formulas.h"
#include "SpellAuras.h"
#include "CreatureAI.h"
#include "Unit.h"

char const* petTypeSuffix[MAX_PET_TYPE] =
{
    "'s Minion",                                            // SUMMON_PET
    "'s Pet",                                               // HUNTER_PET
    "'s Guardian",                                          // GUARDIAN_PET
    "'s Pet"                                                // MINI_PET
};

//numbers represent minutes * 100 while happy (you get 100 loyalty points per min while happy)
uint32 const LevelUpLoyalty[6] =
{
    5500,
    11500,
    17000,
    23500,
    31000,
    39500,
};

uint32 const LevelStartLoyalty[6] =
{
    2000,
    4500,
    7000,
    10000,
    13500,
    17500,
};

Pet::Pet(WorldObject *instantiator, PetType type) : Creature( instantiator )
{
    m_isPet = true;
    m_name = "Pet";
    m_petType = type;

    m_removed = false;
    m_regenTimer = 4000;
    m_happinessTimer = 7500;
    m_loyaltyTimer = 12000;
    m_GlobalCooldown = 0;
    m_duration = 0;
    m_bonusdamage = 0;

    m_loyaltyPoints = 0;
    m_TrainingPoints = 0;
    m_resetTalentsCost = 0;
    m_resetTalentsTime = 0;

    // pets always have a charminfo, even if they are not actually charmed
    CharmInfo* charmInfo = InitCharmInfo(this);

    if(type == MINI_PET)                                    // always passive and follow
    {
        charmInfo->SetReactState(REACT_PASSIVE);
        charmInfo->SetCommandState(COMMAND_FOLLOW);
    }
    else if(type == GUARDIAN_PET)                           // always aggressive and follow
    {
        charmInfo->SetReactState(REACT_AGGRESSIVE);
        charmInfo->SetCommandState(COMMAND_FOLLOW);
    }

    m_spells.clear();
    m_Auras.clear();
    m_CreatureSpellCooldowns.clear();
    m_CreatureCategoryCooldowns.clear();
    m_autospells.clear();
}

Pet::~Pet()
{
    if(m_uint32Values)                                      // only for fully created Object
    {
        for (PetSpellMap::iterator i = m_spells.begin(); i != m_spells.end(); ++i)
            delete i->second;
        ObjectAccessor::Instance().RemoveObject(this);
    }
}

void Pet::AddToWorld()
{
    ///- Register the pet for guid lookup
    if(!IsInWorld()) ObjectAccessor::Instance().AddObject(this);
    Object::AddToWorld();
}

void Pet::RemoveFromWorld()
{
    ///- Remove the pet from the accessor
    if(IsInWorld()) ObjectAccessor::Instance().RemoveObject(this);
    ///- Don't call the function for Creature, normal mobs + totems go in a different storage
    Object::RemoveFromWorld();
}

bool Pet::LoadPetFromDB( Unit* owner, uint32 petentry, uint32 petnumber, bool current )
{
    uint32 ownerid = owner->GetGUIDLow();

    QueryResult *result;

    if(petnumber)
        // known petnumber entry           0    1       2       3         4       5     6            7            8              9               10        11           12     13     14        15          16        17             18        19              20         21                  22                  23               24
        result = sDatabase.PQuery("SELECT `id`,`entry`,`owner`,`modelid`,`level`,`exp`,`nextlvlexp`,`Reactstate`,`Commandstate`,`loyaltypoints`,`loyalty`,`trainpoint`,`slot`,`name`,`renamed`,`curhealth`,`curmana`,`curhappiness`,`ABData`,`TeachSpelldata`,`savetime`,`resettalents_cost`,`resettalents_time`,`CreatedBySpell`,`PetType` FROM `character_pet` WHERE `owner` = '%u' AND `id` = '%u'",ownerid, petnumber);
    else if(current)
        // current pet (slot 0)            0    1       2       3         4       5     6            7            8              9               10        11           12     13     14        15          16        17             18        19              20         21                  22                  23               24
        result = sDatabase.PQuery("SELECT `id`,`entry`,`owner`,`modelid`,`level`,`exp`,`nextlvlexp`,`Reactstate`,`Commandstate`,`loyaltypoints`,`loyalty`,`trainpoint`,`slot`,`name`,`renamed`,`curhealth`,`curmana`,`curhappiness`,`ABData`,`TeachSpelldata`,`savetime`,`resettalents_cost`,`resettalents_time`,`CreatedBySpell`,`PetType` FROM `character_pet` WHERE `owner` = '%u' AND `slot` = '0'",ownerid );
    else if(petentry)
        // known petentry entry (unique for summoned pet, but non unique for hunter pet (only from current or not stabled pets)
        //                                 0    1       2       3         4       5     6            7            8              9               10        11           12     13     14        15          16        17             18        19              20         21                  22                  23               24
        result = sDatabase.PQuery("SELECT `id`,`entry`,`owner`,`modelid`,`level`,`exp`,`nextlvlexp`,`Reactstate`,`Commandstate`,`loyaltypoints`,`loyalty`,`trainpoint`,`slot`,`name`,`renamed`,`curhealth`,`curmana`,`curhappiness`,`ABData`,`TeachSpelldata`,`savetime`,`resettalents_cost`,`resettalents_time`,`CreatedBySpell`,`PetType` FROM `character_pet` WHERE `owner` = '%u' AND `entry` = '%u' AND (`slot` = '0' OR `slot` = '3') ",ownerid, petentry );
    else
        // any current or other non-stabled pet (for hunter "call pet")
        //                                 0    1       2       3         4       5     6            7            8              9               10        11           12     13     14        15          16        17             18        19              20         21                  22                  23               24
        result = sDatabase.PQuery("SELECT `id`,`entry`,`owner`,`modelid`,`level`,`exp`,`nextlvlexp`,`Reactstate`,`Commandstate`,`loyaltypoints`,`loyalty`,`trainpoint`,`slot`,`name`,`renamed`,`curhealth`,`curmana`,`curhappiness`,`ABData`,`TeachSpelldata`,`savetime`,`resettalents_cost`,`resettalents_time`,`CreatedBySpell`,`PetType` FROM `character_pet` WHERE `owner` = '%u' AND (`slot` = '0' OR `slot` = '3') ",ownerid);

    if(!result)
        return false;

    Field *fields = result->Fetch();

    // update for case of current pet "slot = 0"
    petentry = fields[1].GetUInt32();
    if(!petentry)
    {
        delete result;
        return false;
    }

    float px, py, pz;
    owner->GetClosePoint(px, py, pz,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE);
    uint32 guid=objmgr.GenerateLowGuid(HIGHGUID_UNIT);
    if(!Create(guid, owner->GetMapId(), px, py, pz, owner->GetOrientation(), petentry))
    {
        delete result;
        return false;
    }

    setPetType(PetType(fields[24].GetUInt8()));
    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,owner->getFaction());
    SetUInt32Value(UNIT_CREATED_BY_SPELL, fields[23].GetUInt32());

    CreatureInfo const *cinfo = GetCreatureInfo();
    if(cinfo->type == CREATURE_TYPE_CRITTER)
    {
        AIM_Initialize();
        MapManager::Instance().GetMap(owner->GetMapId(), owner)->Add((Creature*)this);
        delete result;
        return true;
    }
    if(getPetType()==HUNTER_PET || getPetType()==SUMMON_PET && cinfo->type == CREATURE_TYPE_DEMON && owner->getClass() == CLASS_WARLOCK)
        m_charmInfo->SetPetNumber(fields[0].GetUInt32(), true);
    else
        m_charmInfo->SetPetNumber(fields[0].GetUInt32(), false);
    SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner->GetGUID());
    SetUInt64Value(UNIT_FIELD_DISPLAYID,       fields[3].GetUInt32());
    SetUInt64Value(UNIT_FIELD_NATIVEDISPLAYID, fields[3].GetUInt32());
    uint32 petlevel=fields[4].GetUInt32();
    SetUInt32Value(UNIT_NPC_FLAGS , 0);
    SetName(fields[13].GetString());

    switch(getPetType())
    {

        case SUMMON_PET:
            petlevel=owner->getLevel();

            SetUInt32Value(UNIT_FIELD_BYTES_0,2048);
            SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN1);
                                                            // this enables popup window (pet dismiss, cancel)
            break;
        case HUNTER_PET:
            SetUInt32Value(UNIT_FIELD_BYTES_0, 0x2020100);  //??
            SetUInt32Value(UNIT_FIELD_BYTES_1,(fields[10].GetUInt32()<<8));
            SetUInt32Value(UNIT_FIELD_BYTES_2, 0x00022801); // can't be renamed (byte (0x02))
                                                            // + UNIT_FLAG_RESTING);
            SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN1);
                                                            // this enables popup window (pet abandon, cancel)

            // pet not renamed yet, let rename if wont
            if(!fields[14].GetBool())
            {
                //SetUInt32Value(UNIT_FIELD_BYTES_2, uint32(0x03 << 16)); // check it...
                                                            // 0x03
                SetUInt32Value(UNIT_FIELD_BYTES_2, 0x00032801);
                //SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_RENAME); // old, not working...
            }

            SetTP(fields[11].GetInt32());
            SetMaxPower(POWER_HAPPINESS,GetCreatePowers(POWER_HAPPINESS));
            SetPower(   POWER_HAPPINESS,fields[17].GetUInt32());
            setPowerType(POWER_FOCUS);
            break;
        default:
            sLog.outError("Pet have incorrect type (%u) for pet loading.",getPetType());
    }
    InitStatsForLevel( petlevel);
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, time(NULL));
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, fields[5].GetUInt32());
    SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, fields[6].GetUInt32());
    SetUInt64Value(UNIT_FIELD_CREATEDBY, owner->GetGUID());

    m_charmInfo->SetReactState( ReactStates( fields[7].GetUInt8() ));
    m_charmInfo->SetCommandState( CommandStates( fields[8].GetUInt8() ));
    m_loyaltyPoints = fields[9].GetInt32();

    uint32 savedhealth = fields[15].GetUInt32();
    uint32 savedmana = fields[16].GetUInt32();

    // set current pet as current
    if(fields[12].GetUInt32() != 0)
    {
        sDatabase.BeginTransaction();
        sDatabase.PExecute("UPDATE `character_pet` SET `slot` = '3' WHERE `owner` = '%u' AND `slot` = '0' AND `id` <> '%u'",ownerid, m_charmInfo->GetPetNumber());
        sDatabase.PExecute("UPDATE `character_pet` SET `slot` = '0' WHERE `owner` = '%u' AND `id` = '%u'",ownerid, m_charmInfo->GetPetNumber());
        sDatabase.CommitTransaction();
    }

    //init AB
    Tokens tokens = StrSplit(fields[18].GetString(), " ");

    if(tokens.size() != 20)
    {
        delete result;
        return false;
    }

    int index;
    Tokens::iterator iter;
    for(iter = tokens.begin(), index = 0; index < 10; ++iter, ++index )
    {
        m_charmInfo->GetActionBarEntry(index)->Type = atol((*iter).c_str());
        ++iter;
        m_charmInfo->GetActionBarEntry(index)->SpellOrAction = atol((*iter).c_str());
    }

    //init teach spells
    tokens = StrSplit(fields[19].GetString(), " ");
    for (iter = tokens.begin(), index = 0; index < 4; ++iter, ++index)
    {
        uint32 tmp = atol((*iter).c_str());

        ++iter;

        if(tmp)
            AddTeachSpell(tmp, atol((*iter).c_str()));
        else
            break;
    }

    // since last save (in seconds)
    uint32 timediff = (time(NULL) - fields[20].GetUInt32());

    delete result;

    //load spells/cooldowns/auras
    SetCanModifyStats(true);
    _LoadAuras(timediff);

    if(getPetType() == SUMMON_PET && !current)              //all (?) summon pets come with full health when called, but not when they are current
    {
        SetHealth(GetMaxHealth());
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    }
    else
    {
        SetHealth(savedhealth > GetMaxHealth() ? GetMaxHealth() : savedhealth);
        SetPower(POWER_MANA, savedmana > GetMaxPower(POWER_MANA) ? GetMaxPower(POWER_MANA) : savedmana);
    }

    AIM_Initialize();
    MapManager::Instance().GetMap(owner->GetMapId(), owner)->Add((Creature*)this);

    // Spells should be loaded after pet is added to map, because in CanCast is check on it
    _LoadSpells(timediff);
    _LoadSpellCooldowns();

    owner->SetPet(this);                                    // in DB stored only full controlled creature
    sLog.outDebug("New Pet has guid %u", GetGUIDLow());

    if(owner->GetTypeId() == TYPEID_PLAYER)
        ((Player*)owner)->PetSpellInitialize();

    return true;
}

void Pet::SavePetToDB(PetSaveMode mode)
{
    if(!GetEntry())
        return;

    // save only fully controlled creature
    if(!isControlled())
        return;

    uint32 curhealth = GetHealth();
    uint32 curmana = GetPower(POWER_MANA);

    switch(mode)
    {
        case PET_SAVE_IN_STABLE_SLOT_1:
        case PET_SAVE_IN_STABLE_SLOT_2:
        case PET_SAVE_NOT_IN_SLOT:
        {
            RemoveAllAuras();

            //only alive hunter pets get auras saved, the others don't
            if(!(getPetType() == HUNTER_PET && isAlive()))
                m_Auras.clear();
        }
        default:
            break;
    }

    _SaveSpells();
    _SaveSpellCooldowns();
    _SaveAuras();

    switch(mode)
    {
        case PET_SAVE_AS_CURRENT:
        case PET_SAVE_IN_STABLE_SLOT_1:
        case PET_SAVE_IN_STABLE_SLOT_2:
        case PET_SAVE_NOT_IN_SLOT:
        {
            uint32 loyalty =1;
            if(getPetType()!=HUNTER_PET)
                loyalty = GetLoyaltyLevel();

            uint32 owner = GUID_LOPART(GetOwnerGUID());
            std::string name = m_name;
            sDatabase.escape_string(name);
            sDatabase.BeginTransaction();
            // remove current data
            sDatabase.PExecute("DELETE FROM `character_pet` WHERE `owner` = '%u' AND `id` = '%u'", owner,m_charmInfo->GetPetNumber() );

            // prevent duplicate using slot (except PET_SAVE_NOT_IN_SLOT)
            if(mode!=PET_SAVE_NOT_IN_SLOT)
                sDatabase.PExecute("UPDATE `character_pet` SET `slot` = 3 WHERE `owner` = '%u' AND `slot` = '%u'", owner, uint32(mode) );

            // prevent existence another hunter pet in PET_SAVE_AS_CURRENT and PET_SAVE_NOT_IN_SLOT
            if(getPetType()==HUNTER_PET && (mode==PET_SAVE_AS_CURRENT||mode==PET_SAVE_NOT_IN_SLOT))
                sDatabase.PExecute("DELETE FROM `character_pet` WHERE `owner` = '%u' AND (`slot` = '0' OR `slot` = '3')", owner );
            // save pet
            std::ostringstream ss;
            ss  << "INSERT INTO `character_pet` (`id`,`entry`,`owner`,`modelid`,`level`,`exp`,`nextlvlexp`,`Reactstate`,`Commandstate`,`loyaltypoints`,`loyalty`,`trainpoint`,`slot`,`name`,`renamed`,`curhealth`,`curmana`,`curhappiness`,`ABData`,`TeachSpelldata`,`savetime`,`resettalents_cost`,`resettalents_time`,`CreatedBySpell`,`PetType`) "
                << "VALUES ("
                << m_charmInfo->GetPetNumber() << ", "
                << GetEntry() << ", "
                << owner << ", "
                << GetUInt32Value(UNIT_FIELD_DISPLAYID) << ", "
                << getLevel() << ", "
                << GetUInt32Value(UNIT_FIELD_PETEXPERIENCE) << ", "
                << GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP) << ", "
                << uint32(m_charmInfo->GetReactState()) << ", "
                << uint32(m_charmInfo->GetCommandState()) << ", "
                << m_loyaltyPoints << ", "
                << GetLoyaltyLevel() << ", "
                << m_TrainingPoints << ", "
                << uint32(mode) << ", '"
                << name.c_str() << "', "
                << uint32(((GetUInt32Value(UNIT_FIELD_BYTES_2) >> 16) == 3)?0:1) << ", "
                << (curhealth<1?1:curhealth) << ", "
                << curmana << ", "
                << GetPower(POWER_HAPPINESS) << ", '";

            for(uint32 i = 0; i < 10; i++)
                ss << uint32(m_charmInfo->GetActionBarEntry(i)->Type) << " " << uint32(m_charmInfo->GetActionBarEntry(i)->SpellOrAction) << " ";
            ss << "', '";

            //save spells the pet can teach to it's Master
            uint8 i;
            TeachSpellMap::iterator itr;
            for(itr = m_teachspells.begin(), i = 0; i < 4 && itr != m_teachspells.end(); ++i, ++itr)
                ss << itr->first << " " << itr->second << " ";
            for(; i < 4; ++i)
                ss << uint32(0) << " " << uint32(0) << " ";

            ss  << "', "
                << time(NULL) << ", "
                << uint32(m_resetTalentsCost) << ", "
                << uint64(m_resetTalentsTime) << ", "
                << GetUInt32Value(UNIT_CREATED_BY_SPELL) << ", "
                << uint32(getPetType()) << ")";

            sDatabase.Execute( ss.str().c_str() );

            sDatabase.CommitTransaction();
            break;
        }
        case PET_SAVE_AS_DELETED:
        {
            RemoveAllAuras();
            uint32 owner = GUID_LOPART(GetOwnerGUID());
            sDatabase.PExecute("DELETE FROM `character_pet` WHERE `owner` = '%u' AND `id` = '%u'", owner,m_charmInfo->GetPetNumber());
            sDatabase.PExecute("DELETE FROM `pet_aura` WHERE `guid` = '%u'", m_charmInfo->GetPetNumber());
            sDatabase.PExecute("DELETE FROM `pet_spell` WHERE `guid` = '%u'", m_charmInfo->GetPetNumber());
            sDatabase.PExecute("DELETE FROM `pet_spell_cooldown` WHERE `guid` = '%u'", m_charmInfo->GetPetNumber());
            break;
        }
        default:
            sLog.outError("Unknown pet save/remove mode: %d",mode);
    }
}

/*void Pet::SendPetQuery()
{
    Unit *player = GetOwner();
    if(player->GetTypeId() != TYPEID_PLAYER)
        return;
    char *subname = "Pet";
    CreatureInfo *ci = objmgr.GetCreatureTemplate(GetEntry());

    WorldPacket data( SMSG_CREATURE_QUERY_RESPONSE, 4+m_name.size()+1+3+strlen(subname)+1+4+4+4+4+4+4+4+2);
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
    if(getDeathState()==CORPSE)
    {
        //remove summoned pet (no corpse)
        if(getPetType()==SUMMON_PET)
            Remove(PET_SAVE_NOT_IN_SLOT);
        // other will despawn at corpse desppawning (Pet::Update code)
        else
        {
            // pet corpse non lootable and non skinnable
            SetUInt32Value( UNIT_DYNAMIC_FLAGS, 0x00 );
            RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
                                                            //lose happiness when died
            ModifyPower(POWER_HAPPINESS, -HAPPINESS_LEVEL_SIZE);
            SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
        }
    }
    else if(getDeathState()==ALIVE)
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
}

void Pet::Update(uint32 diff)
{
    if(m_removed)                                           // pet already removed, just wait in remove queue, no updates
        return;

    switch( m_deathState )
    {
        case CORPSE:
        {
            if( m_deathTimer <= diff )
            {
                assert(getPetType()!=SUMMON_PET && "Must be already removed.");
                Remove(PET_SAVE_NOT_IN_SLOT);               //hunters' pets never get removed because of death, NEVER!
                return;
            }
            break;
        }
        case ALIVE:
        {
            // unsummon pet that lost owner
            Unit* owner = GetOwner();
            if(!owner || !IsWithinDistInMap(owner, OWNER_MAX_DISTANCE) || isControlled() && !owner->GetPetGUID())
            {
                Remove(PET_SAVE_NOT_IN_SLOT, true);
                return;
            }

            if(isControlled())
            {
                if( owner->GetPetGUID() != GetGUID() )
                {
                    Remove(getPetType()==HUNTER_PET?PET_SAVE_AS_DELETED:PET_SAVE_NOT_IN_SLOT);
                    return;
                }
            }
            else
            {
                if(m_duration > 0)
                {
                    if(m_duration > diff)
                        m_duration -= diff;
                    else
                    {
                        Remove(PET_SAVE_AS_DELETED);
                        return;
                    }
                }
            }

            if(getPetType() != HUNTER_PET)
                break;

            //regenerate Focus
            if(m_regenTimer <= diff)
            {
                RegenerateFocus();
                m_regenTimer = 4000;
            }
            else
                m_regenTimer -= diff;

            if(m_happinessTimer <= diff)
            {
                LooseHappiness();
                m_happinessTimer = 7500;
            }
            else
                m_happinessTimer -= diff;

            if(m_loyaltyTimer <= diff)
            {
                TickLoyaltyChange();
                m_loyaltyTimer = 12000;
            }
            else
                m_loyaltyTimer -= diff;

            break;
        }
        default:
            break;
    }
    Creature::Update(diff);
}

void Pet::RegenerateFocus()
{
    uint32 curValue = GetPower(POWER_FOCUS);
    uint32 maxValue = GetMaxPower(POWER_FOCUS);
    if (curValue >= maxValue) return;
    uint32 addvalue = 25;
    ModifyPower(POWER_FOCUS, addvalue);
}

void Pet::LooseHappiness()
{
    uint32 curValue = GetPower(POWER_HAPPINESS);
    if (curValue <= 0) return;
    int32 addvalue = (140 >> GetLoyaltyLevel()) * 125;      //value is 70/35/17/8/4 (per min) * 1000 / 8 (timer 7.5 secs)
    if(isInCombat())                                        //we know in combat happiness fades faster, multiplier guess
        addvalue = int32(addvalue * 1.5);
    ModifyPower(POWER_HAPPINESS, -addvalue);
}

void Pet::ModifyLoyalty(int32 addvalue)
{
    uint32 loyaltylevel = GetLoyaltyLevel();

    if(addvalue > 0)                                        //only gain influenced, not loss
        addvalue = int32((float)addvalue * sWorld.getRate(RATE_LOYALTY));

    if(loyaltylevel >= BEST_FRIEND && (addvalue + m_loyaltyPoints) > int32(GetMaxLoyaltyPoints(loyaltylevel)))
        return;

    m_loyaltyPoints += addvalue;

    if(m_loyaltyPoints < 0)
    {
        if(loyaltylevel > REBELLIOUS)
        {
            //level down
            --loyaltylevel;
            SetLoyaltyLevel(LoyaltyLevel(loyaltylevel));
            m_loyaltyPoints = GetStartLoyaltyPoints(loyaltylevel);
            SetTP(m_TrainingPoints - getLevel());
        }
        else
        {
            m_loyaltyPoints = 0;
            Unit* owner = GetOwner();
            if(owner && owner->GetTypeId() == TYPEID_PLAYER)
            {
                WorldPacket data(SMSG_PET_BROKEN, 8);
                data << GetGUID();
                ((Player*)owner)->GetSession()->SendPacket(&data);

                //run away
                ((Player*)owner)->RemovePet(this,PET_SAVE_AS_DELETED);
            }
        }
    }
    //level up
    else if(m_loyaltyPoints > int32(GetMaxLoyaltyPoints(loyaltylevel)))
    {
        ++loyaltylevel;
        SetLoyaltyLevel(LoyaltyLevel(loyaltylevel));
        m_loyaltyPoints = GetStartLoyaltyPoints(loyaltylevel);
        SetTP(m_TrainingPoints + getLevel());
    }
}

void Pet::TickLoyaltyChange()
{
    int32 addvalue;

    switch(GetHappinessState())
    {
        case HAPPY:   addvalue =  20; break;
        case CONTENT: addvalue =  10; break;
        case UNHAPPY: addvalue = -20; break;
        default:
            return;
    }
    ModifyLoyalty(addvalue);
}

void Pet::KillLoyaltyBonus(uint32 level)
{
    if(level > 100)
        return;

                                                            //at lower levels gain is faster | the lower loyalty the more loyalty is gained
    uint32 bonus = uint32(((100 - level) / 10) + (6 - GetLoyaltyLevel()));
    ModifyLoyalty(bonus);
}

HappinessState Pet::GetHappinessState()
{
    if(GetPower(POWER_HAPPINESS) < HAPPINESS_LEVEL_SIZE)
        return UNHAPPY;
    else if(GetPower(POWER_HAPPINESS) >= HAPPINESS_LEVEL_SIZE * 2)
        return HAPPY;
    else
        return CONTENT;
}

void Pet::SetLoyaltyLevel(LoyaltyLevel level)
{
    uint32 curvalue = GetUInt32Value(UNIT_FIELD_BYTES_1);
    curvalue &= 0xFFFF00FF;
    curvalue |= (level << 8);
    SetUInt32Value(UNIT_FIELD_BYTES_1, curvalue);
}

bool Pet::CanTakeMoreActiveSpells(uint32 spellid)
{
    uint8  activecount = 1;
    uint32 chainstartstore[ACTIVE_SPELLS_MAX];

    if(IsPassiveSpell(spellid))
        return true;

    chainstartstore[0] = objmgr.GetFirstSpellInChain(spellid);

    for (PetSpellMap::iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if(IsPassiveSpell(itr->first))
            continue;

        uint32 chainstart = objmgr.GetFirstSpellInChain(itr->first);

        uint8 x;

        for(x = 0; x < activecount; x++)
        {
            if(chainstart == chainstartstore[x])
                break;
        }

        if(x == activecount)                                //spellchain not yet saved -> add active count
        {
            activecount++;
            if(activecount > ACTIVE_SPELLS_MAX)
                return false;
            chainstartstore[x] = chainstart;
        }
    }
    return true;
}

bool Pet::HasTPForSpell(uint32 spellid)
{
    int32 neededtrainp = GetTPForSpell(spellid);
    if((m_TrainingPoints - neededtrainp < 0 || neededtrainp < 0) && neededtrainp != 0)
        return false;
    return true;
}

int32 Pet::GetTPForSpell(uint32 spellid)
{
    SkillLineAbilityEntry const *newAbility = sSkillLineAbilityStore.LookupEntry(spellid);
    if(!newAbility || !newAbility->reqtrainpoints)
        return 0;

    uint32 basetrainp = newAbility->reqtrainpoints;
    uint32 spenttrainp = 0;
    uint32 chainstart = objmgr.GetFirstSpellInChain(spellid);

    for (PetSpellMap::iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if(itr->second->state == PETSPELL_REMOVED) continue;

        if(objmgr.GetFirstSpellInChain(itr->first) == chainstart)
        {
            SkillLineAbilityEntry const *oldAbility = sSkillLineAbilityStore.LookupEntry(itr->first);
            if(oldAbility && oldAbility->reqtrainpoints > spenttrainp)
                spenttrainp = oldAbility->reqtrainpoints;
        }
    }

    return int32(basetrainp) - int32(spenttrainp);
}

uint32 Pet::GetMaxLoyaltyPoints(uint32 level)
{
    return LevelUpLoyalty[level - 1];
}

uint32 Pet::GetStartLoyaltyPoints(uint32 level)
{
    return LevelStartLoyalty[level - 1];
}

void Pet::SetTP(int32 TP)
{
    m_TrainingPoints = TP;
    SetUInt32Value(UNIT_TRAINING_POINTS, (uint32)GetDispTP());
}

int32 Pet::GetDispTP()
{
    if(getPetType()!= HUNTER_PET)
        return(0);
    if(m_TrainingPoints < 0)
        return -m_TrainingPoints;
    else
        return -(m_TrainingPoints + 1);
}

void Pet::Remove(PetSaveMode mode, bool returnreagent)
{
    Unit* owner = GetOwner();

    if(owner)
    {
        if(owner->GetTypeId()==TYPEID_PLAYER)
        {
            ((Player*)owner)->RemovePet(this,mode,returnreagent);
            return;
        }

        // only if current pet in slot
        if(owner->GetPetGUID()==GetGUID())
            owner->SetPet(0);
    }

    CleanupsBeforeDelete();
    ObjectAccessor::Instance().AddObjectToRemoveList(this);
    m_removed = true;
}

void Pet::GivePetXP(uint32 xp)
{
    if(getPetType() != HUNTER_PET)
        return;

    if ( xp < 1 )
        return;

    if(!isAlive())
        return;

    uint32 level = getLevel();

    // XP to money conversion processed in Player::RewardQuest
    if(level >= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        return;

    uint32 curXP = GetUInt32Value(UNIT_FIELD_PETEXPERIENCE);
    uint32 nextLvlXP = GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP);
    uint32 newXP = curXP + xp;

    if(newXP >= nextLvlXP && level+1 > GetOwner()->getLevel())
    {
        SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, nextLvlXP-1);
        return;
    }

    while( newXP >= nextLvlXP && level < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL) )
    {
        newXP -= nextLvlXP;

        SetLevel( level + 1 );
        SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, uint32((MaNGOS::XP::xp_to_level(level+1))/4));

        level = getLevel();
        nextLvlXP = GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP);
        GivePetLevel(level);
    }

    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, newXP);

    if(getPetType() == HUNTER_PET)
        KillLoyaltyBonus(level);
}

void Pet::GivePetLevel(uint32 level)
{
    if(!level)
        return;

    InitStatsForLevel( level);

    SetTP(m_TrainingPoints + (GetLoyaltyLevel() - 1));
}

bool Pet::CreateBaseAtCreature(Creature* creature)
{
    if(!creature)
    {
        sLog.outError("CRITICAL ERROR: NULL pointer parsed into CreateBaseAtCreature()");
        return false;
    }
    uint32 guid=objmgr.GenerateLowGuid(HIGHGUID_UNIT);

    sLog.outBasic("SetInstanceID()");
    SetInstanceId(creature->GetInstanceId());

    sLog.outBasic("Create pet");
    Create(guid, creature->GetMapId(), creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation(), creature->GetEntry());

    CreatureInfo const *cinfo = GetCreatureInfo();
    if(!cinfo)
    {
        sLog.outError("ERROR: CreateBaseAtCreature() failed, creatureInfo is missing!");
        return false;
    }

    if(cinfo->type == CREATURE_TYPE_CRITTER)
    {
        m_petType = MINI_PET;
        return true;
    }
    SetUInt64Value(UNIT_FIELD_DISPLAYID,       creature->GetUInt64Value(UNIT_FIELD_DISPLAYID));
    SetUInt64Value(UNIT_FIELD_NATIVEDISPLAYID, creature->GetUInt64Value(UNIT_FIELD_NATIVEDISPLAYID));
    SetMaxPower(POWER_HAPPINESS,GetCreatePowers(POWER_HAPPINESS));
    SetPower(   POWER_HAPPINESS,166500);
    setPowerType(POWER_FOCUS);
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,0);
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE,0);
    SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, uint32((MaNGOS::XP::xp_to_level(creature->getLevel()+1))/4));
    SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN1);
    SetUInt32Value(UNIT_NPC_FLAGS , 0);
    SetName(creature->GetName());
    m_loyaltyPoints = 1000;
    if(cinfo->type == CREATURE_TYPE_BEAST)
    {
        SetUInt32Value(UNIT_FIELD_BYTES_0, 0x2020100);
        SetUInt32Value(UNIT_FIELD_BYTES_2, 0x00032801);     // can be renamed (byte 0x03)...

        //SetUInt32Value(UNIT_FIELD_BYTES_1,creature->GetUInt32Value(UNIT_FIELD_BYTES_1));

        SetUInt32Value(UNIT_MOD_CAST_SPEED, creature->GetUInt32Value(UNIT_MOD_CAST_SPEED) );
        SetLoyaltyLevel(REBELLIOUS);
    }
    return true;
}

bool Pet::InitStatsForLevel(uint32 petlevel)
{
    CreatureInfo const *cinfo = GetCreatureInfo();
    assert(cinfo);

    Unit* owner = GetOwner();
    if(!owner)
    {
        sLog.outError("ERROR: attempt to summon pet (Entry %u) without owner! Attempt terminated.", cinfo->Entry);
        return false;
    }

    uint32 creature_ID = (getPetType() == HUNTER_PET) ? 1 : cinfo->Entry;

    SetLevel( petlevel);

    SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(petlevel*50));

    SetAttackTime(BASE_ATTACK, BASE_ATTACK_TIME);

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0);

    // Hunter pets' size does depend on level
    if(getPetType() == HUNTER_PET)
        SetFloatValue(OBJECT_FIELD_SCALE_X, 0.4 + float(petlevel) / 100);
    m_bonusdamage = 0;

    uint32 createResistance[MAX_SPELL_SCHOOL] = {0,0,0,0,0,0,0};

    if(cinfo && !(getPetType() == HUNTER_PET))
    {
        createResistance[SPELL_SCHOOL_HOLY]   = cinfo->resistance1;
        createResistance[SPELL_SCHOOL_FIRE]   = cinfo->resistance2;
        createResistance[SPELL_SCHOOL_NATURE] = cinfo->resistance3;
        createResistance[SPELL_SCHOOL_FROST]  = cinfo->resistance4;
        createResistance[SPELL_SCHOOL_SHADOW] = cinfo->resistance5;
        createResistance[SPELL_SCHOOL_ARCANE] = cinfo->resistance6;
    }

    if(owner->GetTypeId() == TYPEID_PLAYER)
    {
        float val = 0;
        float val2 = 0;

        switch(getPetType())
        {
            case SUMMON_PET:
                switch(owner->getClass())
                {
                    case CLASS_WARLOCK:

                        //the damage bonus used for pets is either fire or shadow damage, whatever is higher
                        val  = owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE);
                        val2 = owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
                        val  = (val > val2) ? val : val2;   //now val is warlock's damage bonus

                        SetBonusDamage(int32 (val * 0.15f));
                        //bonusAP += val * 0.57;
                        break;

                    case CLASS_MAGE:
                                                            //40% damage bonus of mage's frost damage
                        val = owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FROST) * 0.4;
                        if(val < 0)
                            val = 0;
                        SetBonusDamage( int32(val));
                        break;

                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }

    switch(getPetType())
    {
        case HUNTER_PET:
        {
            //these formula may not be correct; however, it is designed to be close to what it should be
                                                            //this makes dps 0.5 of pets level
            SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - (petlevel / 4)) );
                                                            //damage range is then petlevel / 2
            SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel + (petlevel / 4)) );
            //damage is increased afterwards as strength and pet scaling modify attack power

                                                            //stored standart pet stats are entry 1 in pet_levelinfo
            PetLevelInfo const* pInfo = objmgr.GetPetLevelInfo(creature_ID, petlevel);
            if(pInfo)                                       // exist in DB
            {
                SetCreateHealth(pInfo->health);
                SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(pInfo->armor));
                //SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, float(cinfo->attackpower));

                for( int i = STAT_STRENGTH; i < MAX_STATS; i++)
                {
                    SetCreateStat(Stats(i),  float(pInfo->stats[i]));
                }
            }
            else                                            // not exist in DB, use some default fake data
            {
                sLog.outErrorDb("Hunter pet levelstats missing in DB");

                // remove elite bonuses included in DB values
                SetCreateHealth( uint32(((float(cinfo->maxhealth) / cinfo->maxlevel) / (1 + 2 * cinfo->rank)) * petlevel) );

                SetCreateStat(STAT_STRENGTH,22);
                SetCreateStat(STAT_AGILITY,22);
                SetCreateStat(STAT_STAMINA,25);
                SetCreateStat(STAT_INTELLECT,28);
                SetCreateStat(STAT_SPIRIT,27);
            }
        };
        break;

        case SUMMON_PET:
        {
            SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - (petlevel / 4)) );
            SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel + (petlevel / 4)) );

            //SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, float(cinfo->attackpower));

            PetLevelInfo const* pInfo = objmgr.GetPetLevelInfo(creature_ID, petlevel);
            if(pInfo)                                       // exist in DB
            {
                SetCreateHealth(pInfo->health);
                SetCreateMana(pInfo->mana);

                for(int stat = 0; stat < MAX_STATS; ++stat)
                {
                    SetCreateStat(Stats(stat),float(pInfo->stats[stat]));
                }
            }
            else                                            // not exist in DB, use some default fake data
            {
                sLog.outErrorDb("Summoned pet (Entry: %u) not have pet stats data in DB",cinfo->Entry);

                // remove elite bonuses included in DB values
                SetCreateHealth(uint32(((float(cinfo->maxhealth) / cinfo->maxlevel) / (1 + 2 * cinfo->rank)) * petlevel) );
                SetCreateMana(  uint32(((float(cinfo->maxmana)   / cinfo->maxlevel) / (1 + 2 * cinfo->rank)) * petlevel) );

                SetCreateStat(STAT_STRENGTH,22);
                SetCreateStat(STAT_AGILITY,22);
                SetCreateStat(STAT_STAMINA,25);
                SetCreateStat(STAT_INTELLECT,28);
                SetCreateStat(STAT_SPIRIT,27);
            }
        };
        break;

        default:            sLog.outError("Pet have incorrect type (%u) for levelup.",getPetType());            break;
    }

    for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
        SetModifierValue(UnitMods(UNIT_MOD_RESISTANCE_START + i), BASE_VALUE, float(createResistance[i]) );

    UpdateAllStats();

    SetHealth(GetMaxHealth());
    SetPower(POWER_MANA, GetMaxPower(POWER_MANA));

    return true;
}

bool Pet::HaveInDiet(ItemPrototype const* item) const
{
    CreatureInfo const* cInfo = GetCreatureInfo();
    if(!cInfo)
        return false;

    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cInfo->family);
    if(!cFamily)
        return false;

    // all (?) pet food is in trade goods or consumable class
    if( item->Class != ITEM_CLASS_TRADE_GOODS && item->Class != ITEM_CLASS_CONSUMABLE ||
        item->Class == ITEM_CLASS_TRADE_GOODS && item->SubClass != ITEM_SUBCLASS_TRADE_GOODS ||
        item->Class == ITEM_CLASS_CONSUMABLE  && item->SubClass != 0 && item->SubClass != ITEM_SUBCLASS_FOOD )
        return false;

    // FIXME: food type check not implemented
    return true;

    /*
    uint32 diet = cFamily->petFoodMask;

    if(diet & PET_DIET_MEAT)
    {
    }

    if(diet & PET_DIET_FISH)
    {
    }

    if(diet & PET_DIET_CHEESE)
    {
    }

    if(diet & PET_DIET_BREAD)
    {
    }

    if(diet & PET_DIET_FUNGAS)
    {
    }

    if(diet & PET_DIET_FRUIT)
    {
    }

    if(diet & PET_DIET_RAW_MEAT)
    {
    }

    if(diet & PET_DIET_RAW_FISH)
    {
    }

    return false;
    */
}

uint32 Pet::GetCurrentFoodBenefitLevel(uint32 itemlevel)
{
    // -5 or greater food level
    if(getLevel() <= itemlevel +5)                          //possible to feed level 60 pet with level 55 level food for full effect
        return 35000;
    // -10..-6
    else if(getLevel() <= itemlevel + 10)                   //pure guess, but sounds good
        return 17000;
    // -14..-11
    else if(getLevel() <= itemlevel + 14)                   //level 55 food gets green on 70, makes sense to me
        return 8000;
    // -15 or less
    else
        return 0;                                           //food too low level
}

void Pet::_LoadSpellCooldowns()
{
    m_CreatureSpellCooldowns.clear();
    m_CreatureCategoryCooldowns.clear();

    QueryResult *result = sDatabase.PQuery("SELECT `spell`,`time` FROM `pet_spell_cooldown` WHERE `guid` = '%u'",m_charmInfo->GetPetNumber());

    if(result)
    {
        time_t curTime = time(NULL);

        WorldPacket data(SMSG_SPELL_COOLDOWN, (8+1+result->GetRowCount()*8));
        data << GetGUID();
        data << uint8(0x0);

        do
        {
            Field *fields = result->Fetch();

            uint32 spell_id = fields[0].GetUInt32();
            time_t db_time  = (time_t)fields[1].GetUInt64();

            if(!sSpellStore.LookupEntry(spell_id))
            {
                sLog.outError("Pet %u have unknown spell %u in `pet_spell_cooldown`, skipping.",m_charmInfo->GetPetNumber(),spell_id);
                continue;
            }

            // skip outdated cooldown
            if(db_time <= curTime)
                continue;

            data << uint32(spell_id);
            data << uint32(uint32(db_time-curTime)*1000);   // in m.secs

            _AddCreatureSpellCooldown(spell_id,db_time);

            sLog.outDebug("Pet (Number: %u) spell %u cooldown loaded (%u secs).",m_charmInfo->GetPetNumber(),spell_id,uint32(db_time-curTime));
        }
        while( result->NextRow() );

        delete result;

        if(!m_CreatureSpellCooldowns.empty() && GetOwner())
        {
            ((Player*)GetOwner())->GetSession()->SendPacket(&data);
        }
    }
}

void Pet::_SaveSpellCooldowns()
{
    sDatabase.PExecute("DELETE FROM `pet_spell_cooldown` WHERE `guid` = '%u'", m_charmInfo->GetPetNumber());

    time_t curTime = time(NULL);

    // remove oudated and save active
    for(CreatureSpellCooldowns::iterator itr = m_CreatureSpellCooldowns.begin();itr != m_CreatureSpellCooldowns.end();)
    {
        if(itr->second <= curTime)
            m_CreatureSpellCooldowns.erase(itr++);
        else
        {
            sDatabase.PExecute("INSERT INTO `pet_spell_cooldown` (`guid`,`spell`,`time`) VALUES ('%u', '%u', '" I64FMTD "')", m_charmInfo->GetPetNumber(), itr->first, uint64(itr->second));
            ++itr;
        }
    }
}

void Pet::_LoadSpells(uint32 timediff)
{
    for (PetSpellMap::iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
        delete itr->second;
    m_spells.clear();

    QueryResult *result = sDatabase.PQuery("SELECT `spell`,`slot`,`active` FROM `pet_spell` WHERE `guid` = '%u'",m_charmInfo->GetPetNumber());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            addSpell(fields[0].GetUInt16(), fields[2].GetUInt16(), PETSPELL_UNCHANGED, fields[1].GetUInt16());
        }
        while( result->NextRow() );

        delete result;
    }
}

void Pet::_SaveSpells()
{
    for (PetSpellMap::const_iterator itr = m_spells.begin(), next = m_spells.begin(); itr != m_spells.end(); itr = next)
    {
        next++;
        if (itr->second->state == PETSPELL_REMOVED || itr->second->state == PETSPELL_CHANGED)
            sDatabase.PExecute("DELETE FROM `pet_spell` WHERE `guid` = '%u' and `spell` = '%u'", m_charmInfo->GetPetNumber(), itr->first);
        if (itr->second->state == PETSPELL_NEW || itr->second->state == PETSPELL_CHANGED)
            sDatabase.PExecute("INSERT INTO `pet_spell` (`guid`,`spell`,`slot`,`active`) VALUES ('%u', '%u', '%u','%u')", m_charmInfo->GetPetNumber(), itr->first, itr->second->slotId,itr->second->active);

        if (itr->second->state == PETSPELL_REMOVED)
            _removeSpell(itr->first);
        else
            itr->second->state = PETSPELL_UNCHANGED;
    }
}

void Pet::_LoadAuras(uint32 timediff)
{
    m_Auras.clear();
    for (int i = 0; i < TOTAL_AURAS; i++)
        m_modAuras[i].clear();

    for(uint8 i = 0; i < 48; i++)
        SetUInt32Value((uint16)(UNIT_FIELD_AURA + i), 0);
    for(uint8 j = 0; j < 6; j++)
        SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + j), 0);

    QueryResult *result = sDatabase.PQuery("SELECT `caster_guid`,`spell`,`effect_index`,`amount`,`maxduration`,`remaintime`,`remaincharges` FROM `pet_aura` WHERE `guid` = '%u'",m_charmInfo->GetPetNumber());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 caster_guid = fields[0].GetUInt64();
            uint32 spellid = fields[1].GetUInt32();
            uint32 effindex = fields[2].GetUInt32();
            int32 damage     = (int32)fields[3].GetUInt32();
            int32 maxduration = (int32)fields[4].GetUInt32();
            int32 remaintime = (int32)fields[5].GetUInt32();
            int32 remaincharges = (int32)fields[6].GetUInt32();

            SpellEntry const* spellproto = sSpellStore.LookupEntry(spellid);
            if(!spellproto)
            {
                sLog.outError("Unknown aura (spellid %u, effindex %u), ignore.",spellid,effindex);
                continue;
            }

            if(effindex >= 3)
            {
                sLog.outError("Invalid effect index (spellid %u, effindex %u), ignore.",spellid,effindex);
                continue;
            }

            // negative effects should continue counting down after logout
            if (remaintime != -1 && !IsPositiveEffect(spellid, effindex))
            {
                if(remaintime  <= int32(timediff))
                    continue;

                remaintime -= timediff;
            }

            // prevent wrong values of remaincharges
            if(spellproto->procCharges)
            {
                if(remaincharges <= 0 || remaincharges > spellproto->procCharges)
                    remaincharges = spellproto->procCharges;
            }
            else
                remaincharges = -1;

            Aura* aura;
            if(spellproto->Effect[effindex] == SPELL_EFFECT_APPLY_AREA_AURA)
                aura = new AreaAura(spellproto, effindex, NULL, this, NULL);
            else
                aura = new Aura(spellproto, effindex, NULL, this, NULL);
            if(!damage)
                damage = aura->GetModifier()->m_amount;
            aura->SetLoadedState(caster_guid,damage,maxduration,remaintime,remaincharges);
            AddAura(aura);
        }
        while( result->NextRow() );

        delete result;
    }
}

void Pet::_SaveAuras()
{
    sDatabase.PExecute("DELETE FROM `pet_aura` WHERE `guid` = '%u'",m_charmInfo->GetPetNumber());

    AuraMap const& auras = GetAuras();
    for(AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        // skip all auras from spell that apply at cast SPELL_AURA_MOD_SHAPESHIFT or SPELL_AURA_MOD_STEALTH auras.
        SpellEntry const *spellInfo = itr->second->GetSpellProto();
        uint8 i;
        for (i = 0; i < 3; i++)
            if (spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_STEALTH)
                break;

        if (i == 3 && !itr->second->IsPassive())
            sDatabase.PExecute("INSERT INTO `pet_aura` (`guid`,`caster_guid`,`spell`,`effect_index`,`amount`,`maxduration`,`remaintime`,`remaincharges`) "
                "VALUES ('%u', '" I64FMTD "', '%u', '%u', '%d', '%d', '%d', '%d')",
                m_charmInfo->GetPetNumber(), itr->second->GetCasterGUID(),(uint32)(*itr).second->GetId(), (uint32)(*itr).second->GetEffIndex(),(*itr).second->GetModifier()->m_amount,int((*itr).second->GetAuraMaxDuration()),int((*itr).second->GetAuraDuration()),int((*itr).second->m_procCharges));
    }
}

bool Pet::addSpell(uint16 spell_id, uint16 active, PetSpellState state, uint16 slot_id)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spell_id);
    if (!spellInfo)
    {
        // do pet spell book cleanup
        if(state == PETSPELL_UNCHANGED)                     // spell load case
        {
            sLog.outError("Pet::addSpell: Non-existed in SpellStore spell #%u request, deleting for all pets in `pet_spell`.",spell_id);
            sDatabase.PExecute("DELETE FROM `pet_spell` WHERE `spell` = '%u'",spell_id);
        }
        else
            sLog.outError("Pet::addSpell: Non-existed in SpellStore spell #%u request.",spell_id);

        return false;
    }

    PetSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr != m_spells.end())
    {
        if (itr->second->state == PETSPELL_REMOVED)
        {
            delete itr->second;
            m_spells.erase(itr);
            state = PETSPELL_CHANGED;
        }
        else if (state == PETSPELL_UNCHANGED && itr->second->state != PETSPELL_UNCHANGED)
        {
            // can be in case spell loading but learned at some previous spell loading
            itr->second->state = PETSPELL_UNCHANGED;
            return false;
        }
        else
            return false;
    }

    uint32 oldspell_id = 0;

    PetSpell *newspell = new PetSpell;
    newspell->state = state;

    if(active == ACT_DECIDE)                                //active was not used before, so we save it's autocast/passive state here
    {
        if(IsPassiveSpell(spell_id))
            newspell->active = ACT_PASSIVE;
        else
            newspell->active = ACT_DISABLED;
    }
    else
        newspell->active = active;

    uint32 chainstart = objmgr.GetFirstSpellInChain(spell_id);

    for (PetSpellMap::iterator itr = m_spells.begin(); itr != m_spells.end(); itr++)
    {
        if(itr->second->state == PETSPELL_REMOVED) continue;

        if(objmgr.GetFirstSpellInChain(itr->first) == chainstart)
        {
            slot_id = itr->second->slotId;
            newspell->active = itr->second->active;

            if(newspell->active == ACT_ENABLED)
                ToggleAutocast(itr->first, false);

            oldspell_id = itr->first;
            removeSpell(itr->first);
        }
    }

    uint16 tmpslot=slot_id;

    if (tmpslot == 0xffff)
    {
        uint16 maxid = 0;
        PetSpellMap::iterator itr;
        for (itr = m_spells.begin(); itr != m_spells.end(); ++itr)
        {
            if(itr->second->state == PETSPELL_REMOVED) continue;
            if (itr->second->slotId > maxid) maxid = itr->second->slotId;
        }
        tmpslot = maxid + 1;
    }

    newspell->slotId = tmpslot;
    m_spells[spell_id] = newspell;

    if (IsPassiveSpell(spell_id))
        CastSpell(this, spell_id, true);
    else if(state == PETSPELL_NEW)
        m_charmInfo->AddSpellToAB(oldspell_id, spell_id);

    if(newspell->active == ACT_ENABLED)
        ToggleAutocast(spell_id, true);

    return true;
}

bool Pet::learnSpell(uint16 spell_id)
{
    // prevent duplicated entires in spell book
    if (!addSpell(spell_id))
        return false;

    Unit* owner = GetOwner();
    if(owner->GetTypeId()==TYPEID_PLAYER)
        ((Player*)owner)->PetSpellInitialize();
    return true;
}

void Pet::removeSpell(uint16 spell_id)
{
    PetSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr == m_spells.end())
        return;

    if(itr->second->state == PETSPELL_REMOVED)
        return;

    if(itr->second->state == PETSPELL_NEW)
    {
        delete itr->second;
        m_spells.erase(itr);
    }
    else
        itr->second->state = PETSPELL_REMOVED;

    RemoveAurasDueToSpell(spell_id);
}

bool Pet::_removeSpell(uint16 spell_id)
{
    PetSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr != m_spells.end())
    {
        delete itr->second;
        m_spells.erase(itr);
        return true;
    }
    return false;
}

void Pet::InitPetCreateSpells()
{
    m_charmInfo->InitPetActionBar();

    m_spells.clear();
    int32 usedtrainpoints = 0, petspellid;
    PetCreateSpellEntry const* CreateSpells = objmgr.GetPetCreateSpellEntry(GetEntry());
    if(CreateSpells)
    {
        for(uint8 i = 0; i < 4; i++)
        {
            if(!CreateSpells->spellid[i])
                break;

            SpellEntry const *learn_spellproto = sSpellStore.LookupEntry(CreateSpells->spellid[i]);
            if(!learn_spellproto)
                continue;

            if(learn_spellproto->Effect[0] == SPELL_EFFECT_LEARN_SPELL || learn_spellproto->Effect[0] == SPELL_EFFECT_LEARN_PET_SPELL)
            {
                petspellid = learn_spellproto->EffectTriggerSpell[0];
                Unit* owner = GetOwner();
                if(owner->GetTypeId() == TYPEID_PLAYER && !((Player*)owner)->HasSpell(learn_spellproto->Id))
                {
                    if(IsPassiveSpell(petspellid))          //learn passive skills when tamed, not sure if thats right
                        ((Player*)owner)->learnSpell(learn_spellproto->Id);
                    else
                        AddTeachSpell(learn_spellproto->EffectTriggerSpell[0], learn_spellproto->Id);
                }
            }
            else
                petspellid = learn_spellproto->Id;

            addSpell(petspellid);

            SkillLineAbilityEntry const *Ability = sSkillLineAbilityStore.LookupEntry(learn_spellproto->EffectTriggerSpell[0]);
            if(Ability)
                usedtrainpoints += Ability->reqtrainpoints;
        }

        //family passive
        SpellEntry const *learn_spellproto = sSpellStore.LookupEntry(CreateSpells->familypassive);
        if(learn_spellproto)
            addSpell(CreateSpells->familypassive);
    }
    SetTP(-usedtrainpoints);
}

void Pet::CheckLearning(uint32 spellid)
{
                                                            //charmed case -> prevent crash
    if(GetTypeId() == TYPEID_PLAYER || getPetType() != HUNTER_PET)
        return;

    Unit* owner = GetOwner();

    if(m_teachspells.empty() || !owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    TeachSpellMap::iterator itr = m_teachspells.find(spellid);
    if(itr == m_teachspells.end())
        return;

    if(urand(0, 100) < 10)
    {
        ((Player*)owner)->learnSpell(itr->second);
        m_teachspells.erase(itr);
    }
}

uint32 Pet::resetTalentsCost() const
{
    uint32 days = (sWorld.GetGameTime() - m_resetTalentsTime)/DAY;

    // The first time reset costs 10 silver; after 1 day cost is reset to 10 silver
    if(m_resetTalentsCost < 10*SILVER || days > 0)
        return 10*SILVER;
    // then 50 silver
    else if(m_resetTalentsCost < 50*SILVER)
        return 50*SILVER;
    // then 1 gold
    else if(m_resetTalentsCost < 1*GOLD)
        return 1*GOLD;
    // then increasing at a rate of 1 gold; cap 10 gold
    else
        return (m_resetTalentsCost + 1*GOLD > 10*GOLD ? 10*GOLD : m_resetTalentsCost + 1*GOLD);
}

void Pet::ToggleAutocast(uint32 spellid, bool apply)
{
    if(IsPassiveSpell(spellid))
        return;

    PetSpellMap::const_iterator itr = m_spells.find((uint16)spellid);

    int i;

    if(apply)
    {
        for (i = 0; i < m_autospells.size() && m_autospells[i] != spellid; i++);
        if (i == m_autospells.size())
        {
            m_autospells.push_back(spellid);
            itr->second->active = ACT_ENABLED;
            itr->second->state = PETSPELL_CHANGED;
        }
    }
    else
    {
        AutoSpellList::iterator itr2 = m_autospells.begin();
        for (i = 0; i < m_autospells.size() && m_autospells[i] != spellid; i++, itr2++);
        if (i < m_autospells.size())
        {
            m_autospells.erase(itr2);
            itr->second->active = ACT_DISABLED;
            itr->second->state = PETSPELL_CHANGED;
        }
    }
}

bool Pet::Create(uint32 guidlow, uint32 mapid, float x, float y, float z, float ang, uint32 Entry)
{
    respawn_cord[0] = x;
    respawn_cord[1] = y;
    respawn_cord[2] = z;
    SetMapId(mapid);
    Relocate(x,y,z);

    if(!IsPositionValid())
    {
        sLog.outError("ERROR: Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %d Y: ^%d)",guidlow,Entry,x,y);
        return false;
    }

    SetOrientation(ang);
    //oX = x;     oY = y;    dX = x;    dY = y;    m_moveTime = 0;    m_startMove = 0;

    Object::_Create(guidlow, HIGHGUID_UNIT);

    m_DBTableGuid = guidlow;

    SetUInt32Value(OBJECT_FIELD_ENTRY,Entry);
    CreatureInfo const *cinfo = objmgr.GetCreatureTemplate(Entry);
    if(!cinfo)
    {
        sLog.outError("Error: creature entry %u does not exist.",Entry);
        return false;
    }

    uint32 display_id = cinfo->randomDisplayID();

    SetUInt32Value(UNIT_FIELD_DISPLAYID,display_id );
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID,display_id );
    SetUInt32Value(UNIT_FIELD_BYTES_2,1);                   // let creature used equiped weapon in fight

    SetName(GetCreatureInfo()->Name);

    //this is probably wrong
    SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, cinfo->equipmodel[0]);
    SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO , cinfo->equipinfo[0]);
    SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO  + 1, cinfo->equipslot[0]);

    SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, cinfo->equipmodel[1]);
    SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 2, cinfo->equipinfo[1]);
    SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 2 + 1, cinfo->equipslot[1]);

    SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+2, cinfo->equipmodel[2]);
    SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 4, cinfo->equipinfo[2]);
    SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 4 + 1, cinfo->equipslot[2]);
    
    CreatureDisplayInfoEntry const* ScaleEntry = sCreatureDisplayInfoStore.LookupEntry(display_id);
    SetFloatValue(OBJECT_FIELD_SCALE_X, ScaleEntry ? ScaleEntry->scale : 1 );

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS,cinfo->bounding_radius);
    SetFloatValue(UNIT_FIELD_COMBATREACH,cinfo->combat_reach );

    SetSpeed(MOVE_WALK,     cinfo->speed );
    SetSpeed(MOVE_RUN,      cinfo->speed );
    SetSpeed(MOVE_WALKBACK, cinfo->speed );
    SetSpeed(MOVE_SWIM,     cinfo->speed);
    SetSpeed(MOVE_SWIMBACK, cinfo->speed);

    if(cinfo->MovementType < MAX_DB_MOTION_TYPE)
        m_defaultMovementType = MovementGeneratorType(cinfo->MovementType);
    else
    {
        m_defaultMovementType = IDLE_MOTION_TYPE;
        sLog.outErrorDb("Creature template %u have wrong movement generator type value %u, ignore and set to IDLE.",Entry,cinfo->MovementType);
    }

    return true;
}

bool Pet::HasSpell(uint32 spell) const
{
    return (m_spells.find(spell) != m_spells.end());
}
