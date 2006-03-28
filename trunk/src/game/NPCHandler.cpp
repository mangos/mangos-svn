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
    
    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);
    
    if(!unit) return;
    
    playerGold = GetPlayer( )->GetUInt32Value( PLAYER_FIELD_COINAGE );

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
			
    	if(GetPlayer()->GetUInt32Value( UNIT_FIELD_LEVEL ) < spellInfo->spellLevel)
    		return; 
      
      data.Initialize( SMSG_TRAINER_BUY_SUCCEEDED );
      data << guid << spellId;
      SendPacket( &data );
        
      GetPlayer( )->SetUInt32Value( PLAYER_FIELD_COINAGE, playerGold - proto->spellcost );      

      Spell *spell = new Spell(unit, proto->spell, false, NULL);

      SpellCastTargets targets;
      targets.m_unitTarget = GetPlayer();

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

	Script->GossipHello( GetPlayer(), unit );
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
    
    Script->GossipSelect( GetPlayer(), unit, GetPlayer()->PlayerTalkClass->GossipOptionSender( option ), GetPlayer()->PlayerTalkClass->GossipOptionAction( option ) );
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
        Aura *Aur = new Aura(spellInfo,600000,GetPlayer(),GetPlayer());
        GetPlayer( )->AddAura(Aur);
    }

    GetPlayer( )->DeathDurabilityLoss(0.25);
    GetPlayer( )->SetMovement(MOVE_LAND_WALK);
    GetPlayer( )->SetPlayerSpeed(RUN, (float)7.5, true);
    GetPlayer( )->SetPlayerSpeed(SWIM, (float)4.9, true);

    GetPlayer( )->SetUInt32Value(CONTAINER_FIELD_SLOT_1+29, 0);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURA+32, 0);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURALEVELS+8, 0xeeeeeeee);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+8, 0xeeeeeeee);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURAFLAGS+4, 0);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURASTATE, 0);

    GetPlayer( )->ResurrectPlayer();
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_HEALTH, (uint32)(GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*0.50) );
    GetPlayer( )->SpawnCorpseBones();
}

void WorldSession::HandleBinderActivateOpcode( WorldPacket & recv_data )
{
	WorldPacket data;
	
	// binding
	data.Initialize( SMSG_BINDPOINTUPDATE );
	data << float(GetPlayer( )->GetPositionX());
	data << float(GetPlayer( )->GetPositionY());
	data << float(GetPlayer( )->GetPositionZ());
	data << uint32(GetPlayer( )->GetMapId());
	data << uint32(GetPlayer( )->GetZoneId());
	SendPacket( &data );

	DEBUG_LOG("New Home Position X is %f",GetPlayer( )->GetPositionX());
	DEBUG_LOG("New Home Position Y is %f",GetPlayer( )->GetPositionY());
	DEBUG_LOG("New Home Position Z is %f",GetPlayer( )->GetPositionZ());
	DEBUG_LOG("New Home MapId is %d",GetPlayer( )->GetMapId());
	DEBUG_LOG("New Home ZoneId is %d",GetPlayer( )->GetZoneId());

	// zone update
	data.Initialize( SMSG_PLAYERBOUND );
	data << uint64(GetPlayer( )->GetGUID());
	data << uint32(GetPlayer( )->GetZoneId());
	SendPacket( &data );	

	// update sql homebind
	sDatabase.PExecute("UPDATE `homebind` SET mapID = '%d', zoneID = '%d', positionX = '%f', positionY = '%f', positionZ = '%f' WHERE guid = '%lu';", GetPlayer( )->GetMapId(), GetPlayer( )->GetZoneId(), GetPlayer( )->GetPositionX(), GetPlayer( )->GetPositionY(), GetPlayer( )->GetPositionZ(), (unsigned long)GetPlayer( )->GetGUID());

	// send spell for bind 3286 bind magic
	data.Initialize(SMSG_SPELL_START );
	data << uint8(0xFF) << GetPlayer()->GetGUID() << uint8(0xFF) << GetPlayer()->GetGUID() << uint16(3286);
	data << uint16(0x00) << uint16(0x0F) << uint32(0x00)<< uint16(0x00);
	SendPacket( &data );

	data.Initialize(SMSG_SPELL_GO);
	data << uint8(0xFF) << GetPlayer()->GetGUID() << uint8(0xFF) << GetPlayer()->GetGUID() << uint16(3286);
	data << uint16(0x00) << uint8(0x0D) <<  uint8(0x01)<< uint8(0x01) << GetPlayer()->GetGUID();
	data << uint32(0x00) << uint16(0x0200) << uint16(0x00);
	SendPacket( &data );
}

void WorldSession::HandleRepairItemOpcode( WorldPacket & recv_data ) {
	sLog.outDebug("WORLD: CMSG_REPAIR_ITEM");
	WorldPacket data;
	Item* pItem;
	uint64 npcGUID, itemGUID;

	recv_data >> npcGUID >> itemGUID;

	if (itemGUID) {
		sLog.outDetail("ITEM: Repair item, itemGUID = %d, npcGUID = %d", GUID_LOPART(itemGUID), GUID_LOPART(npcGUID));

		pItem = GetPlayer()->GetItemByGUID(itemGUID);

		if (!pItem) {
			sLog.outDetail("PLAYER: Invalid item, GUID = %d", GUID_LOPART(itemGUID));
			return;
		}
		uint32 durability = pItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
		if (durability != 0) {

		// some simple repair formula depending on durability lost
		uint32 curdur = pItem->GetUInt32Value(ITEM_FIELD_DURABILITY);
		uint32 costs = durability - curdur;

		if (GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE) >= costs)
		{
		    uint32 newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) - costs);
		    GetPlayer()->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);
		    // repair item
		    pItem->SetUInt32Value(ITEM_FIELD_DURABILITY, durability);
		}
                else {
                    DEBUG_LOG("You do not have enough money");
                }


		}

	} else {
		sLog.outDetail("ITEM: Repair all items, npcGUID = %d", GUID_LOPART(npcGUID));

		for (int i = 0; i < EQUIPMENT_SLOT_END; i++) {
			pItem = GetPlayer()->GetItemBySlot(i);
			if (pItem) {
				uint32 durability = pItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
				if (durability != 0) {

		// some simple repair formula depending on durability lost
		uint32 curdur = pItem->GetUInt32Value(ITEM_FIELD_DURABILITY);
		uint32 costs = durability - curdur;

		if (GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE) >= costs)
		{
		    uint32 newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) - costs);
		    GetPlayer()->SetUInt32Value( PLAYER_FIELD_COINAGE , newmoney);
		    // repair item
		    pItem->SetUInt32Value(ITEM_FIELD_DURABILITY, durability);
		    // DEBUG_LOG("Item is: %d, maxdurability is: %d", srcitem, durability);
		    // GetPlayer()->_ApplyItemMods(srcitem,i, false);

		}
		else {
		    DEBUG_LOG("You do not have enough money");
		}

				}
			}
		}
	}
}
