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
                        ((Pet*)pet)->SetCommandState( COMMAND_STAY );
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
                        DEBUG_LOG("Start shits 7");
                       ((Pet*)pet)->SetCommandState( COMMAND_FOLLOW );
                    }
                    break;
                case COMMAND_ATTACK:                        //spellid=1792  //ATTACK
                {
                    // only place where pet can be player
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

                        //10% chance to play special pet attack talk, else growl
                        if(((Creature*)pet)->isPet() && ((Pet*)pet)->getPetType() == SUMMON_PET && pet != TargetUnit && urand(0, 100) < 10)
                            ((Pet*)pet)->SendPetTalk(PET_TALK_ATTACK);
                        else
                        {
                            // 90% chance for pet and 100% chance for charmed creature
                            WorldPacket data(SMSG_AI_REACTION, 12);
                            data << guid1 << uint32(00000002);
                            SendPacket(&data);
                        }
                    }
                    else                                    // charmed player
                        pet->Attack(TargetUnit);

                    WorldPacket data(SMSG_AI_REACTION, 12);
                    data << guid1 << uint32(00000002);
                    SendPacket(&data);
                    break;
                }
                case COMMAND_ABANDON:                       // abandon (hunter pet) or dismiss (summoned pet)
                    if(((Creature*)pet)->isPet())
                    {
                        if(((Pet*)pet)->getPetType()==HUNTER_PET)
                            _player->RemovePet((Pet*)pet,PET_SAVE_AS_DELETED);
                        else
                            //dismissing a summoned pet is like killing them (this prevents returning a soulshard...)
                            ((Pet*)pet)->setDeathState(CORPSE);
                    }
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
                        ((Pet*)pet)->SetReactState( REACT_PASSIVE );
                    }
                    break;
                case REACT_DEFENSIVE:                       //recovery
                    if(((Creature*)pet)->isPet())
                    {
                        ((Pet*)pet)->SetReactState( REACT_DEFENSIVE );
                    }
                    break;
                case REACT_AGGRESSIVE:                      //activete
                    if(((Creature*)pet)->isPet())
                    {
                        ((Pet*)pet)->SetReactState( REACT_AGGRESSIVE );
                    }
                    break;
            }
            break;
        case ACT_DISABLED:                                  //0x8100    spell (disabled), ignore
        case ACT_CAST:                                      //0x0100
        case ACT_ENABLED:                                   //0xc100    spell
        {
            Unit* unit_target;
            if(guid2)
                unit_target = ObjectAccessor::Instance().GetUnit(*_player,guid2);
            else
                unit_target = NULL;

            // do not cast unknown spells
            SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellid );
            if(!spellInfo)
            {
                sLog.outError("WORLD: unknown PET spell id %i\n", spellid);
                return;
            }
            
            for(uint32 i = 0;i<3;i++)
            {
                if(spellInfo->EffectImplicitTargetA[i] == TARGET_ALL_ENEMY_IN_AREA || spellInfo->EffectImplicitTargetA[i] == TARGET_ALL_ENEMY_IN_AREA_INSTANT || spellInfo->EffectImplicitTargetA[i] == TARGET_ALL_ENEMY_IN_AREA_CHANNELED)
                    return;
            }

            // do not cast not learned spells
            if(!((Pet*)pet)->HasSpell(spellid) || IsPassiveSpell(spellid))
                return;

            pet->clearUnitState(UNIT_STAT_FOLLOW);

            Spell *spell = new Spell(pet, spellInfo, false, 0);

            int16 result = spell->PetCanCast(unit_target);

            if(result == SPELL_FAILED_UNIT_NOT_INFRONT && !pet->HasAuraType(SPELL_AURA_MOD_POSSESS)) //auto turn to target unless possessed
            {
                pet->SetInFront(unit_target);
                if( unit_target->GetTypeId() == TYPEID_PLAYER )
                    pet->SendUpdateToPlayer( (Player*)unit_target );
                if(pet->GetCharmerOrOwner() && pet->GetCharmerOrOwner()->GetTypeId() == TYPEID_PLAYER)
                    pet->SendUpdateToPlayer((Player*)pet->GetCharmerOrOwner());
                result = -1;
            }

            if(result == -1)
            {
                ((Creature*)pet)->AddCreatureSpellCooldown(spellid);
                ((Pet*)pet)->CheckLearning(spellid);

                unit_target = spell->m_targets.getUnitTarget();

                //10% chance to play special pet attack talk, else growl
                //actually this only seems to happen on special spells, fire shield for imp, torment for voidwalker, but it's stupid to check every spell
                if((((Pet*)pet)->getPetType() == SUMMON_PET) && (pet != unit_target) && (urand(0, 100) < 10))
                    ((Pet*)pet)->SendPetTalk(PET_TALK_SPECIAL_SPELL);
                else
                {
                    WorldPacket data(SMSG_AI_REACTION, 12);
                    data << guid1 << uint32(00000002);
                    SendPacket(&data);
                }

                if( unit_target && !GetPlayer()->IsFriendlyTo(unit_target) && !pet->HasAuraType(SPELL_AURA_MOD_POSSESS))
                {
                    pet->clearUnitState(UNIT_STAT_FOLLOW);
                    if(unit_target!=pet->getVictim())
                        pet->AttackStop();
                    (*(Creature*)pet)->Clear();
                    if (((Creature*)pet)->AI())
                        ((Creature*)pet)->AI()->AttackStart(unit_target);
                }

                spell->prepare(&(spell->m_targets));
            }
            else
            {
                if(pet->HasAuraType(SPELL_AURA_MOD_POSSESS))
                {
                    WorldPacket data(SMSG_CAST_RESULT, (4+2));
                    data << uint32(spellid) << uint8(2) << uint8(result);
                    switch (result)
                    {
                        case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
                            data << uint32(spellInfo->RequiresSpellFocus);
                            break;
                        case SPELL_FAILED_REQUIRES_AREA:
                            data << uint32(spellInfo->AreaId);
                            break;
                    }
                    GetPlayer()->GetSession()->SendPacket(&data);
                }
                else
                    ((Pet*)pet)->SendCastFail(spellid, result);
                    
                if(!((Creature*)pet)->HasSpellCooldown(spellid))
                {
                    WorldPacket data2(SMSG_CLEAR_COOLDOWN, (4+8+4));
                    data2 << uint32(spellid);
                    data2 << pet->GetGUID();
                    data2 << uint32(0);
                    GetPlayer()->GetSession()->SendPacket(&data2);
                }

                spell->finish(false);
                delete spell;
            }
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
    uint8  count;

    recv_data >> petguid;

    // FIXME: charmed case
    //Pet* pet = ObjectAccessor::Instance().GetPet(petguid);
    if(ObjectAccessor::Instance().FindPlayer(petguid))
        return;

    Creature* pet = ObjectAccessor::Instance().GetCreatureOrPet(*_player, petguid);

    if(!pet || (pet != _player->GetPet() && pet != _player->GetCharm()))
    {
        sLog.outError( "HandlePetSetAction: Unknown pet or pet owner.\n" );
        return;
    }

    count = (recv_data.size() == 24) ? 2 : 1;
    for(uint8 i = 0; i < count; i++)
    {
        recv_data >> position;
        recv_data >> spell_id;
        recv_data >> act_state;

        sLog.outDetail( "Player %s has changed pet spell action. Position: %u, Spell: %u, State: 0x%X\n", _player->GetName(), position, spell_id, act_state);

        if(!((act_state == ACT_ENABLED || act_state == ACT_DISABLED || act_state == ACT_CAST) && spell_id && !((Pet*)pet)->HasSpell(spell_id))) //if it's act for spell (en/disable/cast) and there is a spell given (0 = remove spell) which pet doesn't know, don't add
        {
            if(!pet->isCharmed())
            {
                //sign for autocast
                if(act_state == ACT_ENABLED && spell_id)
                    ((Pet*)pet)->ToggleAutocast(spell_id, true);
                //sign for no/turn off autocast
                else if(act_state == ACT_DISABLED && spell_id)
                    ((Pet*)pet)->ToggleAutocast(spell_id, false);
            }

            pet->PetActionBar[position].Type = act_state;
            pet->PetActionBar[position].SpellOrAction = spell_id;
        }
    }
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
    if(!pet || !pet->isPet() || ((Pet*)pet)->getPetType()!= HUNTER_PET || (pet->GetUInt32Value(UNIT_FIELD_BYTES_2) >> 16) != 3 || pet->GetOwnerGUID() != _player->GetGUID() ) // check it!
        return;

    pet->SetName(name);
    //pet->RemoveFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_RENAME);
    pet->SetUInt32Value(UNIT_FIELD_BYTES_2, uint32(2 << 16)); // check it!

    sDatabase.escape_string(name);
    sDatabase.PExecute("UPDATE `character_pet` SET `name` = '%s', `renamed` = '1' WHERE `owner` = '%u' AND `id` = '%u'", name.c_str(),_player->GetGUIDLow(),pet->GetPetNumber() );

    pet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, time(NULL));
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

void WorldSession::HandlePetUnlearnOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket,8);

    sLog.outString("CMSG_PET_UNLEARN");
    uint64 guid;
    recvPacket >> guid;
    
    if(!_player->GetPet() || _player->GetPet()->getPetType() != HUNTER_PET || _player->GetPet()->m_spells.size() <= 1)
        return;

    if(guid != _player->GetPet()->GetGUID())
    {
        sLog.outError( "HandlePetUnlearnOpcode.Pet %u isn't pet of player %s .\n", uint32(GUID_LOPART(guid)),GetPlayer()->GetName() );
        return;
    }

    Creature* pet = _player->GetPet();
    
    uint32 cost = ((Pet*)pet)->resetTalentsCost();

    if (GetPlayer()->GetMoney() < cost)
    {
        GetPlayer()->SendBuyError( BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
        return;
    }

    for(PetSpellMap::iterator itr = ((Pet*)pet)->m_spells.begin(); itr != ((Pet*)pet)->m_spells.end(); ++itr)
        ((Pet*)pet)->removeSpell(itr->first);
    ((Pet*)pet)->SetTP(pet->getLevel() * (((Pet*)pet)->GetLoyaltyLevel() - 1));
    for(uint8 i = 0; i < 10; i++)
    {
        if(((Pet*)pet)->PetActionBar[i].SpellOrAction && (((Pet*)pet)->PetActionBar[i].Type == ACT_ENABLED || ((Pet*)pet)->PetActionBar[i].Type == ACT_DISABLED))
            ((Pet*)pet)->PetActionBar[i].SpellOrAction = 0;
    }

    //still keep passive
    PetCreateSpellEntry const* CreateSpells = objmgr.GetPetCreateSpellEntry(pet->GetEntry());
    SpellEntry const *learn_spellproto = sSpellStore.LookupEntry(CreateSpells->familypassive);
    if(learn_spellproto)
        ((Pet*)pet)->addSpell(CreateSpells->familypassive);
    
    ((Pet*)pet)->m_resetTalentsTime = time(NULL);
    ((Pet*)pet)->m_resetTalentsCost = cost;
    GetPlayer()->ModifyMoney(-(int32)cost);

    GetPlayer()->PetSpellInitialize();
}

void WorldSession::HandlePetSpellAutocastOpcode( WorldPacket& recvPacket )
{
    CHECK_PACKET_SIZE(recvPacket,8+2+2+1);

    sLog.outString("CMSG_PET_SPELL_AUTOCAST");
    uint64 guid;
    uint16 spellid;
    uint16 spellid2;                                        //maybe second spell, automatically toggled off when first toggled on?
    uint8  state;                                           //1 for on, 0 for off
    recvPacket >> guid >> spellid >> spellid2 >> state;

    if(!_player->GetPet())
        return;

    if(guid != _player->GetPet()->GetGUID())
    {
        sLog.outError( "HandlePetSpellAutocastOpcode.Pet %u isn't pet of player %s .\n", uint32(GUID_LOPART(guid)),GetPlayer()->GetName() );
        return;
    }

    Creature* pet = _player->GetPet();

    // do not add not learned spells/ passive spells
    if(!((Pet*)pet)->HasSpell(spellid) || IsPassiveSpell(spellid))
        return;

    ((Pet*)pet)->ToggleAutocast(spellid, state); //state can be used as boolean
    for(uint8 i = 0; i < 10; ++i)
    {
        if((((Pet*)pet)->PetActionBar[i].Type == ACT_ENABLED || ((Pet*)pet)->PetActionBar[i].Type == ACT_DISABLED) && spellid == ((Pet*)pet)->PetActionBar[i].SpellOrAction)
            ((Pet*)pet)->PetActionBar[i].Type = state ? ACT_ENABLED : ACT_DISABLED;
    }
}

void WorldSession::HandleAddDynamicTargetObsoleteOpcode( WorldPacket& recvPacket )
{
    sLog.outString("WORLD: MSG_ADD_DYNAMIC_TARGET_OBSOLETE");

    //CHECK_PACKET_SIZE(recvPacket,8+4+14);
    CHECK_PACKET_SIZE(recvPacket,4);
    uint64 guid;
    uint32 spellid;

    recvPacket >> guid >> spellid;

    if(!_player->GetPet() && !_player->GetCharm())
        return;

    if(ObjectAccessor::Instance().FindPlayer(guid))
        return;

    Creature* pet=ObjectAccessor::Instance().GetCreatureOrPet(*_player,guid);

    if(!pet || (pet != _player->GetPet() && pet!= _player->GetCharm()))
    {
        sLog.outError( "HandleAddDynamicTargetObsoleteOpcode.Pet %u isn't pet of player %s .\n", uint32(GUID_LOPART(guid)),GetPlayer()->GetName() );
        return;
    }

    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellid);
    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown PET spell id %i\n", spellid);
        return;
    }

    // do not cast not learned spells
    if(!((Pet*)pet)->HasSpell(spellid) || IsPassiveSpell(spellid))
        return;

    pet->clearUnitState(UNIT_STAT_FOLLOW);

    Spell *spell = new Spell(pet, spellInfo, false, 0);

    SpellCastTargets targets;
    targets.read(&recvPacket,pet);
    spell->m_targets = targets;

    int16 result = spell->PetCanCast(NULL);
    if(result == -1)
    {
        pet->AddCreatureSpellCooldown(spellid);
        if(pet->isPet())
        {
            ((Pet*)pet)->CheckLearning(spellid);
            //10% chance to play special pet attack talk, else growl
            //actually this only seems to happen on special spells, fire shield for imp, torment for voidwalker, but it's stupid to check every spell
            if((((Pet*)pet)->getPetType() == SUMMON_PET) && (urand(0, 100) < 10))
                ((Pet*)pet)->SendPetTalk(PET_TALK_SPECIAL_SPELL);
            else
            {
                WorldPacket data(SMSG_AI_REACTION, 12);
                data << guid << uint32(00000002);
                SendPacket(&data);
            }
        }

        spell->prepare(&(spell->m_targets));
    }
    else
    {
        ((Pet*)pet)->SendCastFail(spellid, result);
        if(!pet->HasSpellCooldown(spellid))
        {
            WorldPacket data2(SMSG_CLEAR_COOLDOWN, (4+8+4));
            data2 << uint32(spellid);
            data2 << pet->GetGUID();
            data2 << uint32(0);
            GetPlayer()->GetSession()->SendPacket(&data2);
           }

        spell->finish(false);
        delete spell;
    }
}
