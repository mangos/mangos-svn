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
    CHECK_PACKET_SIZE(recv_data,8+2+2+8);

    uint64 guid1;
    uint16 spellid;
    uint16 flag;
    uint64 guid2;
    recv_data >> guid1;                                     //pet guid
    recv_data >> spellid;
    recv_data >> flag;                                      //delete = 0x0700 CastSpell = C100
    recv_data >> guid2;                                     //tag guid

    // used also for charmed creature
    Unit* pet=ObjectAccessor::Instance().GetUnit(*_player,guid1);
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

    if(!pet->isAlive())
        return;

    if(pet->GetTypeId() == TYPEID_PLAYER && !(flag == ACT_COMMAND && spellid == COMMAND_ATTACK))
        return;

    switch(flag)
    {
        case ACT_COMMAND:                                   //0x0700
            switch(spellid)
            {
                case COMMAND_STAY:                          //flat=1792  //STAY
                    ((Creature*)pet)->StopMoving();
                    (*((Creature*)pet))->Clear();
                    (*((Creature*)pet))->Idle();
                    if(((Creature*)pet)->isPet())
                    {
                        ((Pet*)pet)->AddActState( STATE_RA_STAY );
                        ((Pet*)pet)->ClearActState( STATE_RA_FOLLOW );
                    }
                    break;
                case COMMAND_FOLLOW:                        //spellid=1792  //FOLLOW
                    DEBUG_LOG("Start shits 1");
                    pet->AttackStop();
                    DEBUG_LOG("Start shits 2");
                    pet->addUnitState(UNIT_STAT_FOLLOW);
                    DEBUG_LOG("Start shits 3");
                    (*((Creature*)pet))->Clear();
                    DEBUG_LOG("Start shits 4");
                    (*((Creature*)pet))->Mutate(new TargetedMovementGenerator(*_player,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE));
                    DEBUG_LOG("Start shits 5");

                    if(((Creature*)pet)->isPet())
                    {
                        DEBUG_LOG("Start shits 6");
                        ((Pet*)pet)->AddActState( STATE_RA_FOLLOW );
                        DEBUG_LOG("Start shits 7");
                        ((Pet*)pet)->ClearActState( STATE_RA_STAY );
                    }
                    break;
                case COMMAND_ATTACK:                        //spellid=1792  //ATTACK
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

                    if(pet->GetTypeId()!=TYPEID_PLAYER)
                    {
                        (*((Creature*)pet))->Clear();
                        if (((Creature*)pet)->AI())
                            ((Creature*)pet)->AI()->AttackStart(TargetUnit);
                    }
                    else
                        pet->Attack(TargetUnit);

                    WorldPacket data(SMSG_AI_REACTION, 12);
                    data << guid1 << uint32(00000002);
                    SendPacket(&data);
                    break;
                }
                case COMMAND_ABANDON:                       // abandon (hunter pet) or dismiss (summoned pet)
                    if(((Creature*)pet)->isPet())
                        _player->RemovePet((Pet*)pet,((Pet*)pet)->getPetType()==HUNTER_PET ? PET_SAVE_AS_DELETED : PET_SAVE_NOT_IN_SLOT);
                    else                                    // charmed
                        _player->Uncharm();
                    break;
                default:
                    sLog.outError("WORLD: unknown PET flag Action %i and spellid %i.\n", flag, spellid);
            }
            break;
        case ACT_REACTION:                                  // 0x600
            switch(spellid)
            {
                case REACT_PASSIVE:                         //passive
                    if(((Creature*)pet)->isPet())
                    {
                        ((Pet*)pet)->AddActState( STATE_RA_PASSIVE );
                        ((Pet*)pet)->ClearActState( STATE_RA_PROACTIVE | STATE_RA_REACTIVE );
                    }
                    break;
                case REACT_DEFENSIVE:                       //recovery
                    if(((Creature*)pet)->isPet())
                    {
                        ((Pet*)pet)->AddActState( STATE_RA_REACTIVE );
                        ((Pet*)pet)->ClearActState( STATE_RA_PASSIVE | STATE_RA_PROACTIVE );
                    }
                    break;
                case REACT_AGGRESSIVE:                      //activete
                    if(((Creature*)pet)->isPet())
                    {
                        ((Pet*)pet)->AddActState( STATE_RA_PROACTIVE );
                        ((Pet*)pet)->ClearActState( STATE_RA_PASSIVE | STATE_RA_REACTIVE );
                    }
                    break;
            }
            break;
        case ACT_DISABLED:                                  //0x8100    spell (disabled), ignore
            break;
        case ACT_CAST:                                      //0x0100
        case ACT_ENABLED:                                   //0xc100    spell
        {
            uint64 selectguid = _player->GetSelection();
            Unit* unit_target=ObjectAccessor::Instance().GetUnit(*_player,selectguid);
            if(!unit_target)
                return;

            // do not cast unknown spells
            SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellid );
            if(!spellInfo)
            {
                sLog.outError("WORLD: unknown PET spell id %i\n", spellid);
                return;
            }

            // do not cast not learned spells
            bool spell_found = false;
            for(int i = 0; i < 4; ++i)
            {
                if(((Creature*)pet)->m_spells[i] == spellid)
                {
                    spell_found = true;
                    break;
                }
            }

            if(!spell_found)
                return;

            // do cast now
            pet->clearUnitState(UNIT_STAT_FOLLOW);
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
    CHECK_PACKET_SIZE(recv_data,4+8);

    sLog.outDetail( "HandlePetNameQuery. CMSG_PET_NAME_QUERY\n" );

    uint32 petnumber;
    uint64 petguid;

    recv_data >> petnumber;
    recv_data >> petguid;

    SendPetNameQuery(petguid,petnumber);
}

void WorldSession::SendPetNameQuery( uint64 petguid, uint32 petnumber)
{
    Pet* pet=ObjectAccessor::Instance().GetPet(petguid);
    if(!pet || pet->GetPetNumber() != petnumber)
        return;

    std::string name = pet->GetName();

    WorldPacket data(SMSG_PET_NAME_QUERY_RESPONSE, (4+4+name.size()+1));
    data << uint32(petnumber);
    data << name.c_str();
    data << uint32(pet->GetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP));
    _player->GetSession()->SendPacket(&data);
}

void WorldSession::HandlePetSetAction( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4+2+2);

    sLog.outDetail( "HandlePetSetAction. CMSG_PET_SET_ACTION\n" );

    uint64 petguid;
    uint32 position;
    uint16 spell_id;
    uint16 act_state;

    recv_data >> petguid;
    recv_data >> position;
    recv_data >> spell_id;
    recv_data >> act_state;

    // FIXME: charmed case
    Pet* pet = ObjectAccessor::Instance().GetPet(petguid);

    if(!pet || pet->GetOwnerGUID() != _player->GetGUID() )
    {
        sLog.outError( "HandlePetSetAction: Unknown pet or pet owner.\n" );
        return;
    }

    sLog.outDetail( "Player %s has changed pet spell action. Position: %u, Spell: %u, State: %u\n", _player->GetName(), position, spell_id, act_state);

    if (act_state==0xC100)                                  // enable
    {
        if (position==3)
            pet->AddActState(STATE_RA_SPELL1);
        if (position==4)
            pet->AddActState(STATE_RA_SPELL2);
        if (position==5)
            pet->AddActState(STATE_RA_SPELL3);
        if (position==6)
            pet->AddActState(STATE_RA_SPELL4);
    } else
    if (act_state==0x8100)                                  // disable
    {
        if (position==3)
            pet->ClearActState(STATE_RA_SPELL1);
        if (position==4)
            pet->ClearActState(STATE_RA_SPELL2);
        if (position==5)
            pet->ClearActState(STATE_RA_SPELL3);
        if (position==6)
            pet->ClearActState(STATE_RA_SPELL4);
    } else
    sLog.outError( "Spell state %u is unknown.\n", act_state);
}

void WorldSession::HandlePetRename( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+1);

    sLog.outDetail( "HandlePetRename. CMSG_PET_RENAME\n" );

    uint64 petguid;

    std::string name;

    recv_data >> petguid;
    recv_data >> name;

    Pet* pet = ObjectAccessor::Instance().GetPet(petguid);
                                                            // check it!
    if(!pet || !pet->isPet() || ((Pet*)pet)->getPetType()!= HUNTER_PET || (pet->GetUInt32Value(UNIT_FIELD_BYTES_2) >> 16) != 3 || pet->GetOwnerGUID() != _player->GetGUID() )
        return;

    pet->SetName(name);
    //pet->RemoveFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_RENAME);
                                                            // check it!
    pet->SetUInt32Value(UNIT_FIELD_BYTES_2, uint32(2 << 16));

    sDatabase.escape_string(name);
    sDatabase.PExecute("UPDATE `character_pet` SET `name` = '%s', `renamed` = '1' WHERE `owner` = '%u' AND `id` = '%u'", name.c_str(),_player->GetGUIDLow(),pet->GetPetNumber() );

    SendPetNameQuery(petguid,pet->GetPetNumber());
}

void WorldSession::HandlePetAbandon( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;                                      //pet guid
    sLog.outDetail( "HandlePetAbandon. CMSG_PET_ABANDON pet guid is %u", GUID_LOPART(guid) );

    // pet/charmed
    Creature* pet=ObjectAccessor::Instance().GetCreatureOrPet(*_player, guid);
    if(pet)
    {
        if(pet->isPet())
        {
            if(pet->GetGUID() == _player->GetPetGUID())
            {
                uint32 feelty = pet->GetPower(POWER_HAPPINESS);
                pet->SetPower(POWER_HAPPINESS ,(feelty-50000) > 0 ?(feelty-50000) : 0);
            }

            _player->RemovePet((Pet*)pet,PET_SAVE_AS_DELETED);
        }
        else if(pet->GetGUID() == _player->GetCharmGUID())
        {
            _player->Uncharm();
        }
    }
}
