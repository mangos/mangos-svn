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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "TargetedMovementGenerator.h"
#include "CreatureAI.h"
#include "Util.h"
#include "Pet.h"

void WorldSession::HandlePetAction( WorldPacket & recv_data )
{

    uint64 guid1;
    uint16 spellid;
    uint16 flag;
    uint64 guid2;
    WorldPacket data;
    recv_data >> guid1;                                     //pet guid
    recv_data >> spellid;
    recv_data >> flag;                                      //delete = 0x0700 CastSpell = C100
    recv_data >> guid2;                                     //tag guid

    Creature* pet=ObjectAccessor::Instance().GetCreature(*_player,guid1);
    sLog.outDetail( "HandlePetAction.Pet %u flag is %u, spellid is %u, target %u.\n", uint32(GUID_LOPART(guid1)), flag, spellid, uint32(GUID_LOPART(guid2)) );
    if(!pet)
    {
        sLog.outError( "Pet %u not exist.\n", uint32(GUID_LOPART(guid1)) );
        return;
    }

    if(pet != GetPlayer()->GetPet() && pet != GetPlayer()->GetCharm())
    {
        sLog.outError( "HandlePetAction.Pet %u isn't pet of player %s .\n", uint32(GUID_LOPART(guid1)),GetPlayer()->GetName() );
        return;
    }

    switch(flag)
    {
        case 1792:                                          //0x0700
            switch(spellid)
            {
                case 0x0000:                                //flat=1792  //STAY
                    pet->StopMoving();
                    (*pet)->Clear();
                    (*pet)->Idle();
                    if(pet->isPet())
                    {
                        ((Pet*)pet)->AddActState( STATE_RA_STAY );
                        ((Pet*)pet)->ClearActState( STATE_RA_FOLLOW );
                    }
                    break;
                case 0x0001:                                //spellid=1792  //FOLLOW
                    DEBUG_LOG("Start shits 1");
                    pet->AttackStop();
                    DEBUG_LOG("Start shits 2");
                    pet->addUnitState(UNIT_STAT_FOLLOW);
                    DEBUG_LOG("Start shits 3");
                    (*pet)->Clear();
                    DEBUG_LOG("Start shits 4");
                    (*pet)->Mutate(new TargetedMovementGenerator(*_player,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE));
                    DEBUG_LOG("Start shits 5");

                    if(pet->isPet())
                    {
                        DEBUG_LOG("Start shits 6");
                        ((Pet*)pet)->AddActState( STATE_RA_FOLLOW );
                        DEBUG_LOG("Start shits 7");
                        ((Pet*)pet)->ClearActState( STATE_RA_STAY );
                    }
                    break;
                case 0x0002:                                //spellid=1792  //ATTACK
                {
                    pet->clearUnitState(UNIT_STAT_FOLLOW);
                    uint64 selguid = _player->GetSelection();
                    Unit *TargetUnit = ObjectAccessor::Instance().GetUnit(*_player, selguid);
                    if(TargetUnit == NULL) return;

                    // not let attack friendly units.
                    if( GetPlayer()->IsFriendlyTo(TargetUnit))
                        return;

                    if(TargetUnit!=pet->getVictim())
                        pet->AttackStop();
                    (*pet)->Clear();
                    pet->AI().AttackStart(TargetUnit);
                    data.Initialize(SMSG_AI_REACTION);
                    data << guid1 << uint32(00000002);
                    SendPacket(&data);
                    break;
                }
                case 3:
                    if(pet->isPet())
                        _player->AbandonPet((Pet*)pet);
                    break;
                default:
                    sLog.outError("WORLD: unknown PET flag Action %i and spellid %i.\n", flag, spellid);
            }
            break;
        case 1536:
            switch(spellid)
            {
                case 0:                                     //passive
                    if(pet->isPet())
                    {
                        ((Pet*)pet)->AddActState( STATE_RA_PASSIVE );
                        ((Pet*)pet)->ClearActState( STATE_RA_PROACTIVE | STATE_RA_PROACTIVE );
                    }
                    break;
                case 1:                                     //recovery
                    if(pet->isPet())
                    {
                        ((Pet*)pet)->AddActState( STATE_RA_REACTIVE );
                        ((Pet*)pet)->ClearActState( STATE_RA_PASSIVE | STATE_RA_PROACTIVE );
                    }
                    break;
                case 2:                                     //activete
                    if(pet->isPet())
                    {
                        ((Pet*)pet)->AddActState( STATE_RA_PROACTIVE );
                        ((Pet*)pet)->ClearActState( STATE_RA_PASSIVE | STATE_RA_REACTIVE );
                    }
                    break;
            }
            break;
        case 49408:                                         //0xc100    spell
        {
            uint64 selectguid = _player->GetSelection();
            Unit* unit_target=ObjectAccessor::Instance().GetUnit(*_player,selectguid);
            if(!unit_target)
                return;

            // do not spell attack of friends
            if(_player->IsFriendlyTo(unit_target))
                return;

            pet->clearUnitState(UNIT_STAT_FOLLOW);
            SpellEntry *spellInfo = sSpellStore.LookupEntry(spellid );
            if(!spellInfo)
            {
                sLog.outError("WORLD: unknown PET spell id %i\n", spellid);
                return;
            }

            Spell *spell = new Spell(pet, spellInfo, false, 0);
            WPAssert(spell);

            SpellCastTargets targets;
            targets.setUnitTarget( unit_target );
            spell->prepare(&targets);
            break;
        }
        default:
            sLog.outError("WORLD: unknown PET flag Action %i and spellid %i.\n", flag, spellid);
    }
}

void WorldSession::HandlePetNameQuery( WorldPacket & recv_data )
{
    sLog.outDetail( "HandlePetNameQuery. CMSG_PET_NAME_QUERY\n" );

    uint32 petnumber;
    uint64 petguid;

    recv_data >> petnumber;
    recv_data >> petguid;

    SendPetNameQuery(petguid,petnumber);
}

void WorldSession::SendPetNameQuery( uint64 petguid, uint32 petnumber)
{

    Creature* pet=ObjectAccessor::Instance().GetCreature(*_player,petguid);
    if(!pet || !pet->GetEntry())
        return;

    std::string name = pet->GetName();

    WorldPacket data;
    data.Initialize(SMSG_PET_NAME_QUERY_RESPONSE);
    data << uint32(petnumber);
    data << name.c_str();
    data << uint32(pet->GetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP));
    _player->GetSession()->SendPacket(&data);
}

void WorldSession::HandlePetSetAction( WorldPacket & recv_data )
{
    sLog.outDetail( "HandlePetSetAction. CMSG_PET_SET_ACTION\n" );
}

void WorldSession::HandlePetRename( WorldPacket & recv_data )
{
    sLog.outDetail( "HandlePetRename. CMSG_PET_RENAME\n" );

    uint64 petguid;

    std::string name;

    recv_data >> petguid;
    recv_data >> name;

    Creature* pet = ObjectAccessor::Instance().GetCreature(*_player,petguid);
    if(!pet || !pet->isPet() || ((Pet*)pet)->getPetType()!= HUNTER_PET || !pet->HasFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_RENAME) || pet->GetOwnerGUID() != _player->GetGUID() )
        return;

    pet->SetName(name);
    pet->RemoveFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_RENAME);
        
    sDatabase.escape_string(name);
    sDatabase.PExecute("UPDATE `character_pet` SET `name` = '%s', `renamed` = '1' WHERE `owner` = '%u' AND `entry` = '%u'", name.c_str(),_player->GetGUIDLow(),pet->GetEntry() );
    
    SendPetNameQuery(petguid,pet->GetUInt32Value(UNIT_FIELD_PETNUMBER));
}

void WorldSession::HandlePetAbandon( WorldPacket & recv_data )
{
    uint64 guid;
    recv_data >> guid;                                      //pet guid
    sLog.outDetail( "HandlePetAbandon. CMSG_PET_ABANDON pet guid is %u", GUID_LOPART(guid) );
    Creature* pet=ObjectAccessor::Instance().GetCreature(*_player, guid);
    if(pet)
    {
        if(pet->isPet())
        {
            if(pet->GetGUID() == _player->GetPetGUID())
            {
                uint32 feelty = pet->GetPower(POWER_HAPPINESS);
                pet->SetPower(POWER_HAPPINESS ,(feelty-50000) > 0 ?(feelty-50000) : 0);
            }

            _player->AbandonPet();
        }
        else if(pet->GetGUID() == _player->GetCharmGUID())
        {
            _player->Uncharm();
        }
    }
}
