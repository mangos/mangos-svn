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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "SpellAuras.h"
#include "UpdateMask.h"
#include "ScriptCalls.h"
#include "ObjectAccessor.h"
#include "Creature.h"
#include "MapManager.h"
#include "Pet.h"

void WorldSession::HandleTabardVendorActivateOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleTabardVendorActivateOpcode - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    if( !unit->isTabardVendor())                            // it's not tabard vendor
        return;

    SendTabardVendorActivate(guid);
}

void WorldSession::SendTabardVendorActivate( uint64 guid )
{
    WorldPacket data;
    data.Initialize( MSG_TABARDVENDOR_ACTIVATE );
    data << guid;
    SendPacket( &data );
}

void WorldSession::HandleBankerActivateOpcode( WorldPacket & recv_data )
{
    uint64 guid;

    sLog.outDetail( "WORLD: Received CMSG_BANKER_ACTIVATE" );

    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleBankerActivateOpcode - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    if( !unit->isBanker())                                  // it's not banker
        return;

    SendShowBank(guid);
}

void WorldSession::SendShowBank( uint64 guid )
{
    WorldPacket data;
    data.Initialize( SMSG_SHOW_BANK );
    data << guid;
    SendPacket( &data );
}

void WorldSession::HandleTrainerListOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;

    recv_data >> guid;
    SendTrainerList( guid );
}

void WorldSession::SendTrainerList( uint64 guid )
{
    std::string str = "Hello! Ready for some training?";
    SendTrainerList( guid, str );
}

void WorldSession::SendTrainerList( uint64 guid,std::string strTitle )
{
    WorldPacket data;

    sLog.outDebug( "WORLD: SendTrainerList" );

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (!unit)
    {
        sLog.outDebug( "WORLD: SendTrainerList - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    if(!unit->isCanTrainingOf(_player,true))
        return;

    CreatureInfo const *ci = unit->GetCreatureInfo();

    if (!ci)
    {
        sLog.outDebug( "WORLD: SendTrainerList - (%u) NO CREATUREINFO! (GUID: %u)", uint32(GUID_LOPART(guid)), guid );
        return;
    }

    std::list<TrainerSpell*> Tspells;
    std::list<TrainerSpell*>::iterator itr;

    for (itr = unit->GetTspellsBegin(); itr != unit->GetTspellsEnd();itr++)
    {
        if(!(*itr)->spell  || _player->HasSpell((*itr)->spell->Id))
            continue;
        //if(!(*itr)->reqspell || _player->HasSpell((*itr)->reqspell))
        //    Tspells.push_back(*itr);
        if((*itr)->spell)
            Tspells.push_back(*itr);
    }

    data.Initialize( SMSG_TRAINER_LIST );
    data << guid;
    data << uint32(0) << uint32(Tspells.size());

    SpellEntry *spellInfo;

    for (itr = Tspells.begin(); itr != Tspells.end();itr++)
    {
        uint8 canlearnflag = 1;
        bool ReqskillValueFlag = false;
        bool LevelFlag = false;
        bool ReqspellFlag = false;
        spellInfo = sSpellStore.LookupEntry((*itr)->spell->EffectTriggerSpell[0]);
        if(!spellInfo)
            continue;
        if((*itr)->reqskill)
        {
            if(_player->GetPureSkillValue((*itr)->reqskill) >= (*itr)->reqskillvalue)
                ReqskillValueFlag = true;
        }
        else ReqskillValueFlag = true;

        uint32 spellLevel = ( (*itr)->reqlevel ? (*itr)->reqlevel : spellInfo->spellLevel);
        if(_player->getLevel() >= spellLevel)
            LevelFlag = true;
        if(!(*itr)->reqspell || _player->HasSpell((*itr)->reqspell))
            ReqspellFlag = true;

        if(ReqskillValueFlag && LevelFlag && ReqspellFlag)
            canlearnflag = 0;                               //green, can learn
        else canlearnflag = 1;                              //red, can't learn

        if(_player->HasSpell(spellInfo->Id))
            canlearnflag = 2;                               //gray, can't learn

        if((*itr)->spell->Effect[1] == 44)
            if(!_player->CanLearnProSpell((*itr)->spell->Id))
                canlearnflag = 1;

        data << uint32((*itr)->spell->Id);
        data << uint8(canlearnflag);
        data << uint32((*itr)->spellcost);
        data << uint32(0);
        data << uint32(0);
        data << uint8(spellLevel);
        data << uint32((*itr)->reqskill);
        data << uint32((*itr)->reqskillvalue);
        data << uint32((*itr)->reqspell);
        data << uint32(0);
        data << uint32(0);
    }

    data << strTitle;
    SendPacket( &data );

    Tspells.clear();

}

void WorldSession::HandleTrainerBuySpellOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;
    uint32 spellId = 0;
    TrainerSpell *proto=NULL;

    recv_data >> guid >> spellId;
    sLog.outDebug( "WORLD: Received CMSG_TRAINER_BUY_SPELL NpcGUID=%u, learn spell id is: %u",uint32(GUID_LOPART(guid)), spellId );

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if(!unit) return;

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    if(!unit->isCanTrainingOf(_player,true))
        return;

    std::list<TrainerSpell*>::iterator titr;

    for (titr = unit->GetTspellsBegin(); titr != unit->GetTspellsEnd();titr++)
    {
        if((*titr)->spell->Id == spellId)
        {
            proto = *titr;
            break;
        }
    }

    if (proto == NULL) return;

    SpellEntry *spellInfo = sSpellStore.LookupEntry(proto->spell->EffectTriggerSpell[0]);

    if(!spellInfo) return;
    if(_player->HasSpell(spellInfo->Id))
        return;
    if(_player->getLevel() < (proto->reqlevel ? proto->reqlevel : spellInfo->spellLevel))
        return;
    if(proto->reqskill && _player->GetSkillValue(proto->reqskill) < proto->reqskillvalue)
        return;
    if(proto->reqspell && !_player->HasSpell(proto->reqspell))
        return;
    if(proto->spell->Effect[1] == 44)
        if(!_player->CanLearnProSpell(spellId))
            return;

    if(!proto)
    {
        sLog.outError("TrainerBuySpell: Trainer(%u) has not the spell(%u).", uint32(GUID_LOPART(guid)), spellId);
        return;
    }
    if( _player->GetMoney() >= proto->spellcost )
    {
        data.Initialize( SMSG_TRAINER_BUY_SUCCEEDED );
        data << guid << spellId;
        SendPacket( &data );

        _player->ModifyMoney( -int32(proto->spellcost) );
        if(spellInfo->powerType == 2)
        {
            _player->addSpell(spellId,4);                   // ative = 4 for spell book of hunter's pet
            return;
        }

        Spell *spell;
        if(proto->spell->SpellVisual == 222)
            spell = new Spell(_player, proto->spell, false, NULL);
        else
            spell = new Spell(unit, proto->spell, false, NULL);

        SpellCastTargets targets;
        targets.setUnitTarget( _player );

        float u_oprientation = unit->GetOrientation();

        // trainer always see at customer in time of training (part of client functionality)
        unit->SetInFront(_player);

        spell->prepare(&targets);

        // trainer always return to original orientation
        unit->Relocate(unit->GetPositionX(),unit->GetPositionY(),unit->GetPositionZ(),u_oprientation);
    }
}

void WorldSession::HandleGossipHelloOpcode( WorldPacket & recv_data )
{
    sLog.outDetail( "WORLD: Received CMSG_GOSSIP_HELLO" );

    WorldPacket data;
    uint64 guid;

    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (!unit)
    {
        sLog.outDebug( "WORLD: CMSG_GOSSIP_HELLO - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    if(!Script->GossipHello( _player, unit ))
    {
        unit->prepareGossipMenu(_player,0);
        unit->sendPreparedGossip( _player );
    }
}

void WorldSession::HandleGossipSelectOptionOpcode( WorldPacket & recv_data )
{
    sLog.outDetail("WORLD: CMSG_GOSSIP_SELECT_OPTION");
    WorldPacket data;
    uint32 option;
    uint64 guid;

    recv_data >> guid >> option;
    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if (!unit)
    {
        sLog.outDebug( "WORLD: CMSG_GOSSIP_SELECT_OPTION - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    if(!Script->GossipSelect( _player, unit, _player->PlayerTalkClass->GossipOptionSender( option ), _player->PlayerTalkClass->GossipOptionAction( option )) )
        unit->OnGossipSelect( _player, option );
}

void WorldSession::HandleSpiritHealerActivateOpcode( WorldPacket & recv_data )
{
    sLog.outDetail("WORLD: CMSG_SPIRIT_HEALER_ACTIVATE");

    if( !GetPlayer()->isDead() )
        return;

    uint64 guid;

    recv_data >> guid;
    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if (!unit)
    {
        sLog.outDebug( "WORLD: CMSG_SPIRIT_HEALER_ACTIVATE - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)) );
        return;
    }

    // prevent cheating
    if(!unit->isSpiritHealer() || !unit->IsWithinDistInMap(_player,OBJECT_ITERACTION_DISTANCE))
        return;

    SendSpiritResurrect();
}

void WorldSession::SendSpiritResurrect()
{
    if (!_player)
        return;

    _player->ResurrectPlayer();
    uint32 level = _player->getLevel();

    //Characters from level 1-10 are not affected by resurrection sickness.
    //Characters from level 11-19 will suffer from one minute of sickness
    //for each level they are above 10.
    //Characters level 20 and up suffer from ten minutes of sickness.
    if (level > 10)
    {
        SpellEntry *spellInfo = sSpellStore.LookupEntry( SPELL_PASSIVE_RESURRECTION_SICKNESS );
        if(spellInfo)
        {
            for(uint32 i = 0;i<3;i++)
            {
                uint8 eff = spellInfo->Effect[i];
                if(eff>=TOTAL_SPELL_EFFECTS)
                    continue;
                if(eff==6)
                {
                    Aura *Aur = new Aura(spellInfo, i, _player);
                    bool added = _player->AddAura(Aur);
                    if (added && level < 20)
                    {
                        Aur->SetAuraDuration((level-10)*60000);
                        Aur->UpdateAuraDuration();
                    }
                }
            }
        }
    }

    _player->ApplyStats(false);
    _player->SetHealth( _player->GetMaxHealth()/2 );
    _player->SetPower(POWER_MANA, _player->GetMaxPower(POWER_MANA)/2 );
    _player->SetPower(POWER_RAGE, 0 );
    _player->SetPower(POWER_ENERGY, _player->GetMaxPower(POWER_ENERGY));
    _player->ApplyStats(true);

    _player->DurabilityLossAll(0.25);
    _player->SpawnCorpseBones();

    // update world right away
    MapManager::Instance().GetMap(_player->GetMapId())->Add(GetPlayer());
}

void WorldSession::HandleBinderActivateOpcode( WorldPacket & recv_data )
{
    uint64 npcGUID;
    recv_data >> npcGUID;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, npcGUID);
    if (!unit)
    {
        sLog.outDebug( "WORLD: CMSG_BINDER_ACTIVATE - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(npcGUID)));
        return;
    }

    // prevent cheating
    if(!unit->isInnkeeper() || !unit->IsWithinDistInMap(_player,OBJECT_ITERACTION_DISTANCE))
        return;

    SendBindPoint(unit);
}

void WorldSession::SendBindPoint(Creature *npc)
{
    WorldPacket data;

    // binding
    data.Initialize( SMSG_BINDPOINTUPDATE );
    data << float(_player->GetPositionX());
    data << float(_player->GetPositionY());
    data << float(_player->GetPositionZ());
    data << uint32(_player->GetMapId());
    data << uint32(_player->GetZoneId());
    SendPacket( &data );

    DEBUG_LOG("New Home Position X is %f",_player->GetPositionX());
    DEBUG_LOG("New Home Position Y is %f",_player->GetPositionY());
    DEBUG_LOG("New Home Position Z is %f",_player->GetPositionZ());
    DEBUG_LOG("New Home MapId is %u",_player->GetMapId());
    DEBUG_LOG("New Home ZoneId is %u",_player->GetZoneId());

    // zone update
    data.Initialize( SMSG_PLAYERBOUND );
    data << uint64(_player->GetGUID());
    data << uint32(_player->GetZoneId());
    SendPacket( &data );

    // update sql homebind
    sDatabase.PExecute("UPDATE `character_homebind` SET `map` = '%u', `zone` = '%u', `position_x` = '%f', `position_y` = '%f', `position_z` = '%f' WHERE `guid` = '%u'", _player->GetMapId(), _player->GetZoneId(), _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetGUIDLow());

    // send spell for bind 3286 bind magic
    data.Initialize(SMSG_SPELL_START );
    data.append(_player->GetPackGUID());
    data.append(npc->GetPackGUID());
    data << uint16(3286) << uint16(0x00) << uint16(0x0F) << uint32(0x00)<< uint16(0x00);
    SendPacket( &data );

    data.Initialize(SMSG_SPELL_GO);
    data.append(_player->GetPackGUID());
    data.append(npc->GetPackGUID());
    data << uint16(3286) << uint16(0x00) << uint8(0x0D) <<  uint8(0x01)<< uint8(0x01) << _player->GetGUID();
    data << uint32(0x00) << uint16(0x0200) << uint16(0x00);
    SendPacket( &data );
    _player->PlayerTalkClass->CloseGossip();
}

//Need fix
void WorldSession::HandleListStabledPetsOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    sLog.outDetail("WORLD: Recv MSG_LIST_STABLED_PETS not dispose.");
    uint64 npcGUID;

    recv_data >> npcGUID;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, npcGUID);
    if (!unit)
    {
        sLog.outDebug( "WORLD: MSG_LIST_STABLED_PETS - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    SendStablePet(npcGUID);
}

void WorldSession::SendStablePet(uint64 guid )
{
    sLog.outDetail("WORLD: Recv MSG_LIST_STABLED_PETS Send.");
    WorldPacket data;
    data.clear();
    data.Initialize(MSG_LIST_STABLED_PETS);
    data << uint64 ( guid );

    QueryResult *result,*result_1;
    uint8 slot = 0;
    uint8 num = 0;

    result_1 = sDatabase.PQuery("SELECT `slot`,`petnumber` FROM `character_stable` WHERE `owner` = '%u'",_player->GetGUIDLow());
    if(result_1)
    {
        do
        {
            Field *fields = result_1->Fetch();

            if(fields[0].GetUInt32())
                slot++;
            if(fields[1].GetUInt32())
                num++;
        }while( result_1->NextRow() );
    }
    delete result_1;

    if(_player->GetPet())
        num++;

    data << uint8(num) << uint8(slot);

    if(_player->GetPet())
    {
        Creature *unit = _player->GetPet();
        if(!unit->GetUInt32Value(UNIT_FIELD_PETNUMBER))
            return;
        Creature *pet = _player->GetPet();
                                                            // petnumber
        data << uint32(pet->GetUInt32Value(UNIT_FIELD_PETNUMBER));
        data << uint32(pet->GetEntry());
        data << uint32(pet->getLevel());
        //data << cinfo->Name;                                                    // petname
        data << uint8(0x00);
        data << uint32(pet->getloyalty());                  // loyalty
        data << uint8(0x01);                                // slot
    }

    result = sDatabase.PQuery("SELECT `owner`,`slot`,`petnumber`,`entry`,`level`,`loyalty`,`trainpoint` FROM `character_stable` WHERE `owner` = '%u'",_player->GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 petentry = fields[3].GetUInt32();
            if(petentry)
            {
                CreatureInfo const *cinfo = objmgr.GetCreatureTemplate(petentry);
                data << uint32(fields[2].GetUInt32());      // petnumber
                data << uint32(petentry);
                data << uint32(fields[4].GetUInt32());
                data << cinfo->Name;
                data << uint32(fields[5].GetUInt32());      // loyalty
                data << uint8(fields[1].GetUInt32()+2);     // slot
            }
        }while( result->NextRow() );
    }
    delete result;
    SendPacket(&data);
}

void WorldSession::HandleStablePet( WorldPacket & recv_data )
{
    WorldPacket data;
    sLog.outDetail("WORLD: Recv CMSG_STABLE_PET not dispose.");
    uint64 npcGUID;

    recv_data >> npcGUID;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, npcGUID);
    if (!unit)
    {
        sLog.outDebug( "WORLD: CMSG_STABLE_PET - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    data.clear();
    data.Initialize(SMSG_STABLE_RESULT);
    if(!_player->GetPet())
        return;
    if(_player->GetPet())
    {
        Pet *unit = _player->GetPet();
        if(!unit->GetUInt32Value(UNIT_FIELD_PETNUMBER))
            return;
    }

    QueryResult *result;
    bool flag = false;
    Pet *pet = _player->GetPet();

    result = sDatabase.PQuery("SELECT `owner`,`slot`,`petnumber` FROM `character_stable` WHERE `owner` = '%u' ORDER BY `slot` ",_player->GetGUIDLow());
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 slot = fields[1].GetUInt32();
            if(fields[2].GetUInt32())
                continue;
            else if(pet->GetUInt32Value(UNIT_FIELD_PETNUMBER) == fields[2].GetUInt32())
                break;
            else if( slot == 1 || slot == 2)
            {
                sDatabase.BeginTransaction();
                sDatabase.PExecute("DELETE FROM `character_stable` WHERE `owner` = '%u' AND `slot` = '%u'", _player->GetGUIDLow(),slot);
                sDatabase.PExecute("INSERT INTO `character_stable` (`owner`,`slot`,`petnumber`,`entry`,`level`,`loyalty`,`trainpoint`) VALUES (%u,%u,%u,%u,%u,%u,%u)",
                    _player->GetGUIDLow(),slot,pet->GetUInt32Value(UNIT_FIELD_PETNUMBER),pet->GetEntry(),pet->getLevel(),pet->getloyalty(),pet->getUsedTrainPoint());
                sDatabase.CommitTransaction();
                data << uint8(0x08);
                flag = true;
                _player->AbandonPet(pet);
                break;
            }
        }while( result->NextRow() );
    }
    delete result;

    if(!flag)
        data << uint8(0x06);
    SendPacket(&data);
    SendStablePet(npcGUID);
}

void WorldSession::HandleUnstablePet( WorldPacket & recv_data )
{
    WorldPacket data;
    sLog.outDetail("WORLD: Recv CMSG_UNSTABLE_PET.");
    uint64 npcGUID;
    uint32 petnumber;

    recv_data >> npcGUID >> petnumber;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, npcGUID);
    if (!unit)
    {
        sLog.outDebug( "WORLD: CMSG_UNSTABLE_PET - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    data.clear();
    data.Initialize(SMSG_STABLE_RESULT);
    if(_player->GetPet())
    {
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    QueryResult *result;

    result = sDatabase.PQuery("SELECT `owner`,`slot`,`petnumber`,`entry`,`level`,`loyalty`,`trainpoint` FROM `character_stable` WHERE `owner` = '%u'",_player->GetGUIDLow());
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 number = fields[2].GetUInt32();
            uint32 slot = fields[1].GetUInt32();

            if(petnumber != number)
                continue;
            else
            {
                Pet *newpet = new Pet(_player->getClass()==CLASS_HUNTER?HUNTER_PET:SUMMON_PET);
                newpet->LoadPetFromDB(_player,fields[3].GetUInt32());
                sDatabase.PExecute("UPDATE `character_stable` SET `petnumber` = '0',`entry` = '0',`level` = '0',`loyalty` = '0',`trainpoint` = '0' WHERE `owner` = '%u' AND `slot` = '%u'",_player->GetGUIDLow(), slot);
            }
        }while( result->NextRow() );
    }
    delete result;

    if(_player->GetPet())
        data << uint8(0x09);
    else data << uint8(0x06);
    SendPacket(&data);
    SendStablePet(npcGUID);
}

void WorldSession::HandleBuyStableSlot( WorldPacket & recv_data )
{
    WorldPacket data;
    sLog.outDetail("WORLD: Recv CMSG_BUY_STABLE_SLOT.");
    uint64 npcGUID;

    recv_data >> npcGUID;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, npcGUID);
    if (!unit)
    {
        sLog.outDebug( "WORLD: CMSG_BUY_STABLE_SLOT - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    data.clear();
    data.Initialize(SMSG_STABLE_RESULT);

    QueryResult *result;
    uint8 slot = 0;

    result = sDatabase.PQuery("SELECT `slot` FROM `character_stable` WHERE `owner` = '%u'",_player->GetGUIDLow());
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            if(fields[0].GetUInt32())
                slot++;
        }while( result->NextRow() );
    }
    delete result;

    switch(slot)
    {
        case 2:data << uint8(0x06);break;
        case 1:
            /*
            if(_player->GetMoney() < 50000)
            {
                data << uint8(0x06);break;
            }
            else
            {
                sDatabase.PExecute("INSERT INTO `character_stable` (`owner`,`slot`,`petnumber`,`entry`,`level`,`loyalty`,`trainpoint`) VALUES (%u,2,0,0,0,0,0)",_player->GetGUIDLow());
                _player->SetMoney(_player->GetMoney() - 50000);
                data << uint8(0x0A);                             // success buy
                break;
            }
            */
            break;                                          // temparay only one slot can be used.
        case 0:
            if(_player->GetMoney() < 500)
            {
                data << uint8(0x06);
                break;
            }
            else
            {
                sDatabase.PExecute("INSERT INTO `character_stable` (`owner`,`slot`,`petnumber`,`entry`,`level`,`loyalty`,`trainpoint`) VALUES (%u,1,0,0,0,0,0)",_player->GetGUIDLow());
                _player->SetMoney(_player->GetMoney() - 500);
                data << uint8(0x0A);                        // success buy
                break;
            }break;
        default :data << uint8(0x06);break;
    }
    SendPacket(&data);
    SendStablePet(npcGUID);
}

void WorldSession::HandleStableRevivePet( WorldPacket & recv_data )
{
}

void WorldSession::HandleStableSwapPet( WorldPacket & recv_data )
{
    WorldPacket data;
    sLog.outDetail("WORLD: Recv CMSG_STABLE_SWAP_PET.");
    uint64 npcGUID;
    uint32 pet_number;

    recv_data >> npcGUID >> pet_number;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, npcGUID);
    if (!unit)
    {
        sLog.outDebug( "WORLD: CMSG_STABLE_SWAP_PET - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    data.clear();
    data.Initialize(SMSG_STABLE_RESULT);

    if(_player->GetPet())
    {
        Creature *unit = _player->GetPet();
        if(!unit->GetUInt32Value(UNIT_FIELD_PETNUMBER))
            return;
    }
    else return;

    QueryResult *result;

    result = sDatabase.PQuery("SELECT `owner`,`slot`,`petnumber`,`entry`,`level`,`loyalty`,`trainpoint` FROM `character_stable` WHERE `owner` = '%u' AND `petnumber` = '%u'",_player->GetGUIDLow(),pet_number);
    if(!result)
    {
        delete result;
        return;
    }
    else
    {
        Pet *pet = _player->GetPet();

        Field *fields = result->Fetch();

        uint32 slot = fields[1].GetUInt32();
        uint32 petentry = fields[3].GetUInt32();

        sDatabase.BeginTransaction();
        sDatabase.PExecute("DELETE FROM `character_stable` WHERE `owner` = '%u' AND `slot` = '%u'", _player->GetGUIDLow(),slot);
        sDatabase.PExecute("INSERT INTO `character_stable` (`owner`,`slot`,`petnumber`,`entry`,`level`,`loyalty`,`trainpoint`) VALUES (%u,%u,%u,%u,%u,%u,%u)",
            _player->GetGUIDLow(),slot,pet->GetUInt32Value(UNIT_FIELD_PETNUMBER),pet->GetEntry(),pet->getLevel(),pet->getloyalty(),pet->getUsedTrainPoint());
        sDatabase.CommitTransaction();
        _player->AbandonPet(pet);
        Pet *newpet = new Pet(_player->getClass()==CLASS_HUNTER?HUNTER_PET:SUMMON_PET);
        newpet->LoadPetFromDB(_player,petentry);
    }
    delete result;

    if(_player->GetPet())
        data << uint8(0x09);
    else data << uint8(0x06);
    SendPacket(&data);
    SendStablePet(npcGUID);
}

void WorldSession::HandleRepairItemOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_REPAIR_ITEM");
    WorldPacket data;

    uint64 npcGUID, itemGUID;

    recv_data >> npcGUID >> itemGUID;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, npcGUID);
    if (!unit)
    {
        sLog.outDebug( "WORLD: CMSG_REPAIR_ITEM - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    if (itemGUID)
    {
        sLog.outDetail("ITEM: Repair item, itemGUID = %u, npcGUID = %u", GUID_LOPART(itemGUID), GUID_LOPART(npcGUID));

        uint16 pos = _player->GetPosByGuid(itemGUID);

        _player->DurabilityRepair(pos,true);

    }
    else
    {
        sLog.outDetail("ITEM: Repair all items, npcGUID = %u", GUID_LOPART(npcGUID));

        _player->DurabilityRepairAll(true);
    }
}
