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
#include "Language.h"
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
#include "WaypointMovementGenerator.h"

void WorldSession::HandleTabardVendorActivateOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_TABARDDESIGNER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleTabardVendorActivateOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    SendTabardVendorActivate(guid);
}

void WorldSession::SendTabardVendorActivate( uint64 guid )
{
    WorldPacket data( MSG_TABARDVENDOR_ACTIVATE, 8 );
    data << guid;
    SendPacket( &data );
}

void WorldSession::HandleBankerActivateOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;

    sLog.outDetail( "WORLD: Received CMSG_BANKER_ACTIVATE" );

    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_BANKER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleBankerActivateOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    SendShowBank(guid);
}

void WorldSession::SendShowBank( uint64 guid )
{
    WorldPacket data( SMSG_SHOW_BANK, 8 );
    data << guid;
    SendPacket( &data );
}

void WorldSession::HandleTrainerListOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;

    recv_data >> guid;
    SendTrainerList( guid );
}

void WorldSession::SendTrainerList( uint64 guid )
{
    std::string str = LANG_NPC_TAINER_HELO;
    SendTrainerList( guid, str );
}

void WorldSession::SendTrainerList( uint64 guid,std::string strTitle )
{
    sLog.outDebug( "WORLD: SendTrainerList" );

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: SendTrainerList - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    // Lazy loading at first access
    unit->LoadTrainerSpells();

    // trainer list loaded at check;
    if(!unit->isCanTrainingOf(_player,true))
        return;

    CreatureInfo const *ci = unit->GetCreatureInfo();

    if (!ci)
    {
        sLog.outDebug( "WORLD: SendTrainerList - (%u) NO CREATUREINFO! (GUID: %u)", uint32(GUID_LOPART(guid)), guid );
        return;
    }

    Creature::SpellsList const& trainer_spells = unit->GetTrainerSpells();

    WorldPacket data( SMSG_TRAINER_LIST, 8+4+4+trainer_spells.size()*38 + strTitle.size()+1);
    data << guid;
    data << uint32(unit->GetTrainerType()) << uint32(trainer_spells.size());

    for(Creature::SpellsList::const_iterator itr = trainer_spells.begin(); itr != trainer_spells.end(); ++itr)
    {
        data << uint32(itr->spell->Id);
        data << uint8(_player->GetTrainerSpellState(&*itr));
        data << uint32(itr->spellcost);
        data << uint32(0);
        data << uint32(0);
        data << uint8(itr->reqlevel ? itr->reqlevel : itr->spell->spellLevel);
        data << uint32(itr->reqskill);
        data << uint32(itr->reqskillvalue);
        data << uint32(objmgr.GetPrevSpellInChain(itr->spell->EffectTriggerSpell[0]));
        data << uint32(0);
        data << uint32(0);
    }

    data << strTitle;
    SendPacket( &data );
}

void WorldSession::HandleTrainerBuySpellOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 guid;
    uint32 spellId = 0;

    recv_data >> guid >> spellId;
    sLog.outDebug( "WORLD: Received CMSG_TRAINER_BUY_SPELL NpcGUID=%u, learn spell id is: %u",uint32(GUID_LOPART(guid)), spellId );

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleTrainerBuySpellOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    // Lazy loading at first access
    unit->LoadTrainerSpells();

    if(!unit->isCanTrainingOf(_player,true))
        return;

    TrainerSpell const* trainer_spell = NULL;

    // check present spell in trainer spell list
    Creature::SpellsList const& trainer_spells = unit->GetTrainerSpells();
    for(Creature::SpellsList::const_iterator itr = trainer_spells.begin(); itr != trainer_spells.end(); ++itr)
    {
        if(itr->spell->Id == spellId)
        {
            trainer_spell = &*itr;
            break;
        }
    }

    // not found, cheat?
    if(!trainer_spell)
        return;

    // can't be learn, cheat?
    if(_player->GetTrainerSpellState(trainer_spell) != TRAINER_SPELL_GREEN)
        return;

    // check money requirement
    if(_player->GetMoney() < trainer_spell->spellcost )
        return;

    SpellEntry const *spellInfo = sSpellStore.LookupEntry(trainer_spell->spell->EffectTriggerSpell[0]);

    WorldPacket data( SMSG_TRAINER_BUY_SUCCEEDED, 12 );
    data << guid << spellId;
    SendPacket( &data );

    _player->ModifyMoney( -int32(trainer_spell->spellcost) );
    if(spellInfo->powerType == 2)
    {
        _player->addSpell(spellId,4);                   // active = 4 for spell book of hunter's pet
        return;
    }

    Spell *spell;
    if(trainer_spell->spell->SpellVisual == 222)
        spell = new Spell(_player, trainer_spell->spell, false, NULL);
    else
        spell = new Spell(unit, trainer_spell->spell, false, NULL);

    SpellCastTargets targets;
    targets.setUnitTarget( _player );

    float u_oprientation = unit->GetOrientation();

    // trainer always see at customer in time of training (part of client functionality)
    unit->SetInFront(_player);

    spell->prepare(&targets);

    // trainer always return to original orientation
    unit->Relocate(unit->GetPositionX(),unit->GetPositionY(),unit->GetPositionZ(),u_oprientation);
}

void WorldSession::HandleGossipHelloOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDetail( "WORLD: Received CMSG_GOSSIP_HELLO" );

    uint64 guid;
    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid, UNIT_NPC_FLAG_NONE);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleGossipHelloOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    if( unit->isArmorer() || unit->isCivilian() || unit->isQuestGiver() || unit->isServiceProvider())
    {
        unit->StopMoving();
        //npcIsStopped[unit->GetGUID()] = true;
    }

    if(!Script->GossipHello( _player, unit ))
    {
        unit->prepareGossipMenu(_player,0);
        unit->sendPreparedGossip( _player );
    }
}

void WorldSession::HandleGossipSelectOptionOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    sLog.outDetail("WORLD: CMSG_GOSSIP_SELECT_OPTION");

    uint32 option;
    uint64 guid;

    recv_data >> guid >> option;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid, UNIT_NPC_FLAG_NONE);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleGossipSelectOptionOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    if(!Script->GossipSelect( _player, unit, _player->PlayerTalkClass->GossipOptionSender( option ), _player->PlayerTalkClass->GossipOptionAction( option )) )
        unit->OnGossipSelect( _player, option );
}

void WorldSession::HandleSpiritHealerActivateOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDetail("WORLD: CMSG_SPIRIT_HEALER_ACTIVATE");

    uint64 guid;

    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid, UNIT_NPC_FLAG_SPIRITHEALER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleSpiritHealerActivateOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    SendSpiritResurrect();
}

void WorldSession::SendSpiritResurrect()
{
    if (!_player)
        return;

    uint32 level = _player->getLevel();

    //Characters from level 1-10 are not affected by resurrection sickness.
    //Characters from level 11-19 will suffer from one minute of sickness
    //for each level they are above 10.
    //Characters level 20 and up suffer from ten minutes of sickness.
    if (level > 10)
    {
        // prepere resurrection sickness setup (will be set in ResurrectPlayer())
        uint32 spellLvl = level < 20 ? level : 20;
        _player->m_resurrectingSicknessExpire = time(NULL) + (spellLvl-10)*MINUTE;
    }

    _player->ResurrectPlayer(0.5f,false);

    _player->DurabilityLossAll(0.25);

    // get corpse nearest graveyard
    WorldSafeLocsEntry const *corpseGrave = NULL;
    CorpsePtr corpse = _player->GetCorpse();
    if((bool)corpse)
        corpseGrave = objmgr.GetClosestGraveYard(
            corpse->GetPositionX(), corpse->GetPositionY(), corpse->GetPositionZ(), corpse->GetMapId(), _player->GetTeam() );

    // now can spawn bones
    _player->SpawnCorpseBones();

    // teleport to nearest from corpse graveyard, if different from nearest to player ghost
    if(corpseGrave)
    {
        WorldSafeLocsEntry const *ghostGrave = objmgr.GetClosestGraveYard(
            _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId(), _player->GetTeam() );

        if(corpseGrave != ghostGrave)
            _player->TeleportTo(corpseGrave->map_id, corpseGrave->x, corpseGrave->y, corpseGrave->z, _player->GetOrientation());
        // or update at original position
        else
            MapManager::Instance().GetMap(_player->GetMapId(), _player)->Add(_player);
    }
    // or update at original position
    else
        MapManager::Instance().GetMap(_player->GetMapId(), _player)->Add(_player);

    _player->SaveToDB();
}

void WorldSession::HandleBinderActivateOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 npcGUID;
    recv_data >> npcGUID;

    if(!GetPlayer()->isAlive())
        return;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_INNKEEPER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleBinderActivateOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    SendBindPoint(unit);
}

void WorldSession::SendBindPoint(Creature *npc)
{
    uint32 bindspell = 3286, hearthstone_itemid = 6948;

    // update sql homebind
    sDatabase.PExecute("UPDATE `character_homebind` SET `map` = '%u', `zone` = '%u', `position_x` = '%f', `position_y` = '%f', `position_z` = '%f' WHERE `guid` = '%u'", _player->GetMapId(), _player->GetZoneId(), _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetGUIDLow());
    _player->m_homebindMapId = _player->GetMapId();
    _player->m_homebindZoneId = _player->GetZoneId();
    _player->m_homebindX = _player->GetPositionX();
    _player->m_homebindY = _player->GetPositionY();
    _player->m_homebindZ = _player->GetPositionZ();

    // if a player lost/dropped hist hearthstone, he will get a new one
    if ( !_player->HasItemCount(hearthstone_itemid, 1) && _player->GetBankItemCount(hearthstone_itemid) <1)
    {
        uint16 dest;
        uint8 msg = _player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, hearthstone_itemid, 1, false );
        if( msg == EQUIP_ERR_OK )
        {
            Item* newitem = _player->StoreNewItem( dest, hearthstone_itemid, 1, true);
            _player->SendNewItem(newitem, 1, true, false);
        }
        else
        {
            _player->SendEquipError( msg, NULL, NULL );
        }
    }

    // send spell for bind 3286 bind magic
    WorldPacket data(SMSG_SPELL_START, (8+8+4+2+4+2+8) );
    data.append(npc->GetPackGUID());
    data.append(npc->GetPackGUID());
    data << bindspell;                                      // spell id
    data << uint16(0);                                      // cast flags
    data << uint32(0);                                      // time
    data << uint16(0x0002);                                 // target mask
    data.append(_player->GetPackGUID());                    // target's packed guid
    SendPacket( &data );

    data.Initialize(SMSG_SPELL_GO, (8+8+4+2+1+8+1+2+8));
    data.append(npc->GetPackGUID());
    data.append(npc->GetPackGUID());
    data << bindspell;                                      // spell id
    data << uint16(0x0100);                                 // cast flags
    data << uint8(0x01);                                    // targets count
    data << _player->GetGUID();                             // target's full guid
    data << uint8(0x00);                                    // ?
    data << uint16(0x0002);                                 // target mask
    data.append(_player->GetPackGUID());                    // target's packed guid
    SendPacket( &data );

    data.Initialize( SMSG_TRAINER_BUY_SUCCEEDED, (8+4));
    data << npc->GetGUID();
    data << bindspell;
    SendPacket( &data );

    // binding
    data.Initialize( SMSG_BINDPOINTUPDATE, (4+4+4+4+4) );
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
    data.Initialize( SMSG_PLAYERBOUND, 8+4 );
    data << uint64(_player->GetGUID());
    data << uint32(_player->GetZoneId());
    SendPacket( &data );

    _player->PlayerTalkClass->CloseGossip();
}

//Need fix
void WorldSession::HandleListStabledPetsOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDetail("WORLD: Recv MSG_LIST_STABLED_PETS not dispose.");
    uint64 npcGUID;

    recv_data >> npcGUID;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_STABLE);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleListStabledPetsOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    SendStablePet(npcGUID);
}

void WorldSession::SendStablePet(uint64 guid )
{
    sLog.outDetail("WORLD: Recv MSG_LIST_STABLED_PETS Send.");

    WorldPacket data(MSG_LIST_STABLED_PETS, 200);           // guess size
    data << uint64 ( guid );

    Pet *pet = _player->GetPet();

    data << uint8(0);                                       // place holder for slot show number
    data << uint8(GetPlayer()->m_stableSlots);

    uint8 num = 0;                                          // counter for place holder

    // not let move dead pet in slot
    if(pet && pet->isAlive() && pet->getPetType()==HUNTER_PET)
    {
        data << uint32(pet->GetPetNumber());
        data << uint32(pet->GetEntry());
        data << uint32(pet->getLevel());
        data << pet->GetName();                             // petname
        data << uint32(pet->getloyalty());                  // loyalty
        data << uint8(0x01);                                // client slot 1 == current pet (0)
        ++num;
    }

    //                                             0       1      2    3       4       5         6
    QueryResult* result = sDatabase.PQuery("SELECT `owner`,`slot`,`id`,`entry`,`level`,`loyalty`,`name` FROM `character_pet` WHERE `owner` = '%u' AND `slot` > 0 AND `slot` < 3",_player->GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            data << uint32(fields[2].GetUInt32());          // petnumber
            data << uint32(fields[3].GetUInt32());          // creature entry
            data << uint32(fields[4].GetUInt32());          // level
            data << fields[6].GetString();                  // name
            data << uint32(fields[5].GetUInt32());          // loyalty
            data << uint8(fields[1].GetUInt32()+1);         // slot

            ++num;
        }while( result->NextRow() );

        delete result;
    }

    data.put<uint8>(8, num);                                // set real data to placeholder
    SendPacket(&data);
}

void WorldSession::HandleStablePet( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDetail("WORLD: Recv CMSG_STABLE_PET not dispose.");
    uint64 npcGUID;

    recv_data >> npcGUID;

    if(!GetPlayer()->isAlive())
        return;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_STABLE);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleStablePet - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    Pet *pet = _player->GetPet();

    WorldPacket data(SMSG_STABLE_RESULT, 200);              // guess size

    // can't place in stable dead pet
    if(!pet||!pet->isAlive()||pet->getPetType()!=HUNTER_PET)
    {
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    uint32 free_slot = 1;

    QueryResult *result = sDatabase.PQuery("SELECT `owner`,`slot`,`id` FROM `character_pet` WHERE `owner` = '%u'  AND `slot` > 0 AND `slot` < 3 ORDER BY `slot` ",_player->GetGUIDLow());
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 slot = fields[1].GetUInt32();

            if(slot==free_slot)                             // this slot not free
                ++free_slot;
        }while( result->NextRow() );
    }
    delete result;

    if( free_slot > 0 && free_slot <= GetPlayer()->m_stableSlots)
    {
        _player->RemovePet(pet,PetSaveMode(free_slot));
        data << uint8(0x08);
    }
    else
        data << uint8(0x06);

    SendPacket(&data);
}

void WorldSession::HandleUnstablePet( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    sLog.outDetail("WORLD: Recv CMSG_UNSTABLE_PET.");
    uint64 npcGUID;
    uint32 petnumber;

    recv_data >> npcGUID >> petnumber;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_STABLE);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleUnstablePet - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    WorldPacket data(SMSG_STABLE_RESULT, 200);              // guess size

    Pet* pet = _player->GetPet();
    if(pet && pet->isAlive())
    {
        uint8 i = 0x06;
        data << uint8(i);
        SendPacket(&data);
        return;
    }

    // delete dead pet
    if(pet)
        _player->RemovePet(pet,PET_SAVE_AS_DELETED);

    Pet *newpet = NULL;

    QueryResult *result = sDatabase.PQuery("SELECT `entry` FROM `character_pet` WHERE `owner` = '%u' AND `id` = '%u' AND `slot` > 0 AND `slot` < 3",_player->GetGUIDLow(),petnumber);
    if(result)
    {
        Field *fields = result->Fetch();
        uint32 petentry = fields[0].GetUInt32();

        newpet = new Pet(_player, HUNTER_PET);
        if(!newpet->LoadPetFromDB(_player,petentry,petnumber))
        {
            delete newpet;
            newpet = NULL;
        }
        delete result;
    }

    if(newpet)
        data << uint8(0x09);
    else
        data << uint8(0x06);
    SendPacket(&data);
}

void WorldSession::HandleBuyStableSlot( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDetail("WORLD: Recv CMSG_BUY_STABLE_SLOT.");
    uint64 npcGUID;

    recv_data >> npcGUID;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_STABLE);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleBuyStableSlot - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    WorldPacket data(SMSG_STABLE_RESULT, 200);

    if(GetPlayer()->m_stableSlots < 2)                      // max slots amount = 2
    {
        StableSlotPricesEntry const *SlotPrice = sStableSlotPricesStore.LookupEntry(GetPlayer()->m_stableSlots+1);
        if(_player->GetMoney() >= SlotPrice->Price)
        {
            ++GetPlayer()->m_stableSlots;
            _player->SetMoney(_player->GetMoney() - SlotPrice->Price);
            data << uint8(0x0A);                            // success buy
        }
        else
            data << uint8(0x06);
    }
    else
        data << uint8(0x06);

    SendPacket(&data);
}

void WorldSession::HandleStableRevivePet( WorldPacket & recv_data )
{
}

void WorldSession::HandleStableSwapPet( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    sLog.outDetail("WORLD: Recv CMSG_STABLE_SWAP_PET.");
    uint64 npcGUID;
    uint32 pet_number;

    recv_data >> npcGUID >> pet_number;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_STABLE);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleStableSwapPet - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    WorldPacket data(SMSG_STABLE_RESULT, 200);              // guess size

    Pet* pet = _player->GetPet();

    if(!pet || pet->getPetType()!=HUNTER_PET)
        return;

    // find swapped pet slot in stable
    QueryResult *result = sDatabase.PQuery("SELECT `slot`,`entry` FROM `character_pet` WHERE `owner` = '%u' AND `id` = '%u'",_player->GetGUIDLow(),pet_number);
    if(!result)
        return;

    Field *fields = result->Fetch();

    uint32 slot     = fields[0].GetUInt32();
    uint32 petentry = fields[1].GetUInt32();
    delete result;

    // move alive pet to slot or delele dead pet
    _player->RemovePet(pet,pet->isAlive() ? PetSaveMode(slot) : PET_SAVE_AS_DELETED);

    // summon unstabled pet
    Pet *newpet = new Pet(_player, _player->getClass()==CLASS_HUNTER?HUNTER_PET:SUMMON_PET);
    if(!newpet->LoadPetFromDB(_player,petentry,pet_number))
    {
        delete newpet;
        data << uint8(0x06);
    }
    else
        data << uint8(0x09);

    SendPacket(&data);
}

void WorldSession::HandleRepairItemOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+8);

    sLog.outDebug("WORLD: CMSG_REPAIR_ITEM");

    uint64 npcGUID, itemGUID;

    recv_data >> npcGUID >> itemGUID;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_ARMORER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleStableSwapPet - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    // 10% reputation discount
    FactionTemplateEntry const* vendor_faction = unit->getFactionTemplateEntry();
    bool discount = vendor_faction && _player->GetReputationRank(vendor_faction->faction) >= REP_HONORED;

    if (itemGUID)
    {
        sLog.outDetail("ITEM: Repair item, itemGUID = %u, npcGUID = %u", GUID_LOPART(itemGUID), GUID_LOPART(npcGUID));

        uint16 pos = _player->GetPosByGuid(itemGUID);

        _player->DurabilityRepair(pos,true,discount);

    }
    else
    {
        sLog.outDetail("ITEM: Repair all items, npcGUID = %u", GUID_LOPART(npcGUID));

        _player->DurabilityRepairAll(true,discount);
    }
}
