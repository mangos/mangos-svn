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

void WorldSession::HandleTabardVendorActivateOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    recv_data >> guid;
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

    sLog.outString( "WORLD: Received CMSG_BANKER_ACTIVATE" );

    recv_data >> guid;

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
        sLog.outDebug( "WORLD: SendTrainerList - (%u) NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)), guid );
        return;
    }

    CreatureInfo *ci = unit->GetCreatureInfo();

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
        else if(!((*itr)->reqspell) || _player->HasSpell((*itr)->reqspell))
            Tspells.push_back(*itr);
    }

    data.Initialize( SMSG_TRAINER_LIST );
    data << guid;
    data << uint32(0) << uint32(Tspells.size());

    SpellEntry *spellInfo;

    for (itr = Tspells.begin(); itr != Tspells.end();itr++)
    {
        spellInfo = sSpellStore.LookupEntry((*itr)->spell->EffectTriggerSpell[0]);
        if(!spellInfo) continue;

        data << uint32((*itr)->spell->Id);

        if(_player->getLevel() < spellInfo->spellLevel )
            data << uint8(1);
        else
            data << uint8(0);

        data << uint32((*itr)->spellcost);
        data << uint32(0) << uint32(0);

        data << uint8(spellInfo->spellLevel);
        data << uint32(0) << uint32(0);
        data << uint32(0) << uint32(0);
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
    uint32 spellId=0, playerGold=0;
    TrainerSpell *proto=NULL;

    recv_data >> guid >> spellId;
    sLog.outDebug( "WORLD: Received CMSG_TRAINER_BUY_SPELL NpcGUID=%u, learn spell id is: %u",uint32(GUID_LOPART(guid)), spellId );

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if(!unit) return;

    playerGold = _player->GetUInt32Value( PLAYER_FIELD_COINAGE );

    std::list<TrainerSpell*>::iterator titr;

    for (titr = unit->GetTspellsBegin(); titr != unit->GetTspellsEnd();titr++)
    {
        if((*titr)->spell->Id == spellId)
        {
            proto = *titr;
            break;
        }
    }

    if( playerGold >= proto->spellcost )
    {
        SpellEntry *spellInfo = sSpellStore.LookupEntry(proto->spell->EffectTriggerSpell[0]);
        if(!spellInfo) return;

        if(_player->GetUInt32Value( UNIT_FIELD_LEVEL ) < spellInfo->spellLevel)
            return;

        data.Initialize( SMSG_TRAINER_BUY_SUCCEEDED );
        data << guid << spellId;
        SendPacket( &data );

        _player->SetUInt32Value( PLAYER_FIELD_COINAGE, playerGold - proto->spellcost );

        Spell *spell = new Spell(unit, proto->spell, false, NULL);

        SpellCastTargets targets;
        targets.setUnitTarget( _player );
        if(proto->spell->SpellVisual == 222)
            spell = new Spell(_player, proto->spell, false, NULL);

        spell->prepare(&targets);

        SendTrainerList( guid );
    }
}

void WorldSession::HandlePetitionShowListOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;
    unsigned char tdata[21] =
    {
        0x01, 0x01, 0x00, 0x00, 0x00, 0xe7, 0x16, 0x00, 0x00, 0xef, 0x23, 0x00, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
    };
    recv_data >> guid;
    data.Initialize( SMSG_PETITION_SHOWLIST );
    data << guid;
    data.append( tdata, sizeof(tdata) );
    SendPacket( &data );
}

void WorldSession::HandleAuctionHelloOpcode( WorldPacket & recv_data )
{
    uint64 guid;

    recv_data >> guid;
    SendAuctionHello(guid);
}

void WorldSession::SendAuctionHello( uint64 guid )
{
    WorldPacket data;
    data.Initialize( MSG_AUCTION_HELLO );
    data << guid;
    data << uint32(0);

    SendPacket( &data );
}

void WorldSession::HandleGossipHelloOpcode( WorldPacket & recv_data )
{
    sLog.outString( "WORLD: Received CMSG_GOSSIP_HELLO" );

    WorldPacket data;
    uint64 guid;

    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (!unit)
    {
        sLog.outDebug( "WORLD: CMSG_GOSSIP_HELLO - (%u) NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)), guid );
        return;
    }
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
        sLog.outDebug( "WORLD: CMSG_GOSSIP_SELECT_OPTION - (%u) NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)), guid );
        return;
    }
    if(!Script->GossipSelect( _player, unit, _player->PlayerTalkClass->GossipOptionSender( option ), _player->PlayerTalkClass->GossipOptionAction( option )) )
        unit->OnGossipSelect( _player, option );
}

void WorldSession::HandleSpiritHealerActivateOpcode( WorldPacket & recv_data )
{
    SendSpiritRessurect();
}

void WorldSession::SendSpiritRessurect()
{

    SpellEntry *spellInfo = sSpellStore.LookupEntry( 15007 );
    if(spellInfo)
    {
        for(uint32 i = 0;i<3;i++)
        {
            uint8 eff = spellInfo->Effect[i];
            if(eff>=TOTAL_SPELL_EFFECTS)
                continue;
            Aura *Aur = new Aura(spellInfo, i, _player, _player);
            _player->AddAura(Aur);
        }
    }

    _player->DeathDurabilityLoss(0.25);
    _player->SetMovement(MOVE_LAND_WALK);
    _player->SetPlayerSpeed(MOVE_RUN, (float)7.5, true);
    _player->SetPlayerSpeed(MOVE_SWIM, (float)4.9, true);

    _player->SetUInt32Value(CONTAINER_FIELD_SLOT_1+29, 0);
    _player->SetUInt32Value(UNIT_FIELD_AURA+32, 0);
    _player->SetUInt32Value(UNIT_FIELD_AURALEVELS+8, 0xeeeeeeee);
    _player->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+8, 0xeeeeeeee);
    _player->SetUInt32Value(UNIT_FIELD_AURAFLAGS+4, 0);
    _player->SetUInt32Value(UNIT_FIELD_AURASTATE, 0);

    _player->ResurrectPlayer();
    //_player->SetUInt32Value(UNIT_FIELD_HEALTH, (uint32)(_player->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*0.50) );
    _player->SpawnCorpseBones();
}

void WorldSession::HandleBinderActivateOpcode( WorldPacket & recv_data )
{
    SendBindPoint();
}

void WorldSession::SendBindPoint()
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
    sDatabase.PExecute("UPDATE `character_homebind` SET `map` = '%u', `zone` = '%u', `position_x` = '%f', `position_y` = '%f', `position_z` = '%f' WHERE `guid` = '%u';", _player->GetMapId(), _player->GetZoneId(), _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetGUIDLow());

    // send spell for bind 3286 bind magic
    data.Initialize(SMSG_SPELL_START );
    data << uint8(0xFF) << _player->GetGUID() << uint8(0xFF) << _player->GetGUID() << uint16(3286);
    data << uint16(0x00) << uint16(0x0F) << uint32(0x00)<< uint16(0x00);
    SendPacket( &data );

    data.Initialize(SMSG_SPELL_GO);
    data << uint8(0xFF) << _player->GetGUID() << uint8(0xFF) << _player->GetGUID() << uint16(3286);
    data << uint16(0x00) << uint8(0x0D) <<  uint8(0x01)<< uint8(0x01) << _player->GetGUID();
    data << uint32(0x00) << uint16(0x0200) << uint16(0x00);
    SendPacket( &data );
}

//Need fix
void WorldSession::HandleListStabledPetsOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    sLog.outDetail("WORLD: Recv MSG_LIST_STABLED_PETS not dispose.");
}

void WorldSession::HandleRepairItemOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_REPAIR_ITEM");
    WorldPacket data;
    Item* pItem;
    uint64 npcGUID, itemGUID;

    recv_data >> npcGUID >> itemGUID;

    if (itemGUID)
    {
        sLog.outDetail("ITEM: Repair item, itemGUID = %u, npcGUID = %u", GUID_LOPART(itemGUID), GUID_LOPART(npcGUID));

        pItem = _player->GetItemByGUID(itemGUID);

        if (!pItem)
        {
            sLog.outDetail("PLAYER: Invalid item, GUID = %u", GUID_LOPART(itemGUID));
            return;
        }
        uint32 durability = pItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
        if (durability != 0)
        {
            // some simple repair formula depending on durability lost
            uint32 curdur = pItem->GetUInt32Value(ITEM_FIELD_DURABILITY);
            uint32 costs = durability - curdur;

            if (_player->GetUInt32Value(PLAYER_FIELD_COINAGE) >= costs)
            {
                uint32 newmoney = ((_player->GetUInt32Value(PLAYER_FIELD_COINAGE)) - costs);
                _player->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);
                // repair item
                pItem->SetUInt32Value(ITEM_FIELD_DURABILITY, durability);
            }
            else
            {
                DEBUG_LOG("You do not have enough money");
            }

        }

    }
    else
    {
        sLog.outDetail("ITEM: Repair all items, npcGUID = %u", GUID_LOPART(npcGUID));

        for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
        {
            pItem = _player->GetItemBySlot(i);
            if (pItem)
            {
                uint32 durability = pItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
                if (durability != 0)
                {

                    // some simple repair formula depending on durability lost
                    uint32 curdur = pItem->GetUInt32Value(ITEM_FIELD_DURABILITY);
                    uint32 costs = durability - curdur;

                    if (_player->GetUInt32Value(PLAYER_FIELD_COINAGE) >= costs)
                    {
                        uint32 newmoney = ((_player->GetUInt32Value(PLAYER_FIELD_COINAGE)) - costs);
                        _player->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);
                        // repair item
                        pItem->SetUInt32Value(ITEM_FIELD_DURABILITY, durability);
                        // DEBUG_LOG("Item is: %d, maxdurability is: %d", srcitem, durability);
                        // _player->_ApplyItemMods(srcitem,i, false);

                    }
                    else
                    {
                        DEBUG_LOG("You do not have enough money");
                    }

                }
            }
        }
    }
}

//Need fix
void WorldSession::HandleBattleMasterHelloOpcode( WorldPacket & recv_data )
{
    //WorldPacket data;
    sLog.outDetail("WORLD: Recv CMSG_BATTLEMASTER_HELLO not dispose.");
}
