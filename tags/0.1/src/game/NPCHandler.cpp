/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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
#include "Affect.h"
#include "UpdateMask.h"
#include "ScriptCalls.h"
#include "ObjectAccessor.h"





void WorldSession::HandleTabardVendorActivateOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;
    recv_data >> guid;
    data.Initialize( MSG_TABARDVENDOR_ACTIVATE );
    data << guid;
    SendPacket( &data );
}





void WorldSession::HandleBankerActivateOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;
    recv_data >> guid;

    data.Initialize( SMSG_SHOW_BANK );
    data << guid;
    SendPacket( &data );
}








extern uint32 default_trainer_guids[12];

void WorldSession::HandleTrainerListOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;
    uint32 cnt;

    recv_data >> guid;
    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);        
    Trainerspell *strainer = objmgr.GetTrainerspell(unit->GetNameID());

	CreatureInfo *ci = objmgr.GetCreatureName(unit->GetNameID());

	if ((ci->flags1 & UNIT_NPC_FLAG_TRAINER) && !strainer)
	{
		strainer = objmgr.GetTrainerspell(default_trainer_guids[ci->classNum]);
	}



    cnt = 0;
    if(strainer)
    {

        for (unsigned int t = 0;t < sSkillStore.GetNumRows();t++)
        {
            skilllinespell *skill = sSkillStore.LookupEntry(t);
            if ((skill->skilline == strainer->skilline1) || (skill->skilline == strainer->skilline2) || (skill->skilline == strainer->skilline3)
                || (skill->skilline == strainer->skilline4) || (skill->skilline == strainer->skilline5) || (skill->skilline == strainer->skilline6)
                || (skill->skilline == strainer->skilline7) || (skill->skilline == strainer->skilline8) || (skill->skilline == strainer->skilline9)
                || (skill->skilline == strainer->skilline10) || (skill->skilline == strainer->skilline11) || (skill->skilline == strainer->skilline12)
                || (skill->skilline == strainer->skilline13) || (skill->skilline == strainer->skilline14) || (skill->skilline == strainer->skilline15)
                || (skill->skilline == strainer->skilline16) || (skill->skilline == strainer->skilline17) || (skill->skilline == strainer->skilline18)
                || (skill->skilline == strainer->skilline19) || (skill->skilline == strainer->skilline20))
            {
                Log::getSingleton().outString("skill %u with skillline %u matches",skill->spell,skill->skilline);
                SpellEntry *proto = sSpellStore.LookupEntry(skill->spell);
                if ((proto) )
                {
                    
                    

                    cnt++;
                }
            }
        }

        data.Initialize( SMSG_TRAINER_LIST );     
        data << guid;
        data << uint32(0) << uint32(cnt);

        uint32 num_added = 0;

        
        for (unsigned int t = 0;t < sSkillStore.GetNumRows();t++)
        {
            skilllinespell *skill = sSkillStore.LookupEntry(t);
            if ((skill->skilline == strainer->skilline1) || (skill->skilline == strainer->skilline2) || (skill->skilline == strainer->skilline3)
                || (skill->skilline == strainer->skilline4) || (skill->skilline == strainer->skilline5) || (skill->skilline == strainer->skilline6)
                || (skill->skilline == strainer->skilline7) || (skill->skilline == strainer->skilline8) || (skill->skilline == strainer->skilline9)
                || (skill->skilline == strainer->skilline10) || (skill->skilline == strainer->skilline11) || (skill->skilline == strainer->skilline12)
                || (skill->skilline == strainer->skilline13) || (skill->skilline == strainer->skilline14) || (skill->skilline == strainer->skilline15)
                || (skill->skilline == strainer->skilline16) || (skill->skilline == strainer->skilline17) || (skill->skilline == strainer->skilline18)
                || (skill->skilline == strainer->skilline19) || (skill->skilline == strainer->skilline20))
            {
                SpellEntry *proto = sSpellStore.LookupEntry(skill->spell);
                if ((proto))
                {
                    
                    

                    
                    data << uint32(skill->spell);
                    

                    if (GetPlayer()->HasSpell(skill->spell))
                    {
                        data << uint8(2);
                    }
                    else
                    {
                        if (((GetPlayer()->GetUInt32Value( UNIT_FIELD_LEVEL )) < (proto->spellLevel)) 
                            || (GetPlayer()->GetUInt32Value( PLAYER_FIELD_COINAGE ) < sWorld.mPrices[proto->spellLevel]))
                        {
                            data << uint8(1);
                        }
                        else
                        {
                            data << uint8(0);
                        }
                    }

                    data << uint32(sWorld.mPrices[proto->spellLevel]) << uint32(0);
                    data << uint32(0) << uint8(proto->spellLevel);
                    data << uint32(0);            
                    data << uint32(0);            
                    data << uint32(0);
                    data << uint32(0) << uint32(0);
                    
                    num_added++;
                    
                }
            }
        }

        

        data << "Hello! Ready for some training?";
        SendPacket( &data );
    }
}

void WorldSession::SendTrainerList( uint64 guid )
{
    WorldPacket data;
    uint32 cnt;
	uint64 useGuid = guid;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);        
    

	if (unit == NULL)
	{
		Log::getSingleton( ).outDebug( "WORLD: SendTrainerList - (%u) NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)), guid );
		return;
	}

	CreatureInfo *ci = objmgr.GetCreatureName(unit->GetNameID());

	if (!ci)
	{
		Log::getSingleton( ).outDebug( "WORLD: SendTrainerList - (%u) NO CREATUREINFO! (GUID: %u)", uint32(GUID_LOPART(guid)), guid );
		return;
	}

	Trainerspell *strainer = objmgr.GetTrainerspell(unit->GetNameID());
	
	if ((ci->flags1 & UNIT_NPC_FLAG_TRAINER) && !strainer)
	{
		strainer = objmgr.GetTrainerspell(default_trainer_guids[ci->classNum]);
		useGuid = default_trainer_guids[ci->classNum];
	}

    cnt = 0;
    if(strainer)
    {

        for (unsigned int t = 0;t < sSkillStore.GetNumRows();t++)
        {
            skilllinespell *skill = sSkillStore.LookupEntry(t);
            if ((skill->skilline == strainer->skilline1) || (skill->skilline == strainer->skilline2) || (skill->skilline == strainer->skilline3)
                || (skill->skilline == strainer->skilline4) || (skill->skilline == strainer->skilline5) || (skill->skilline == strainer->skilline6)
                || (skill->skilline == strainer->skilline7) || (skill->skilline == strainer->skilline8) || (skill->skilline == strainer->skilline9)
                || (skill->skilline == strainer->skilline10) || (skill->skilline == strainer->skilline11) || (skill->skilline == strainer->skilline12)
                || (skill->skilline == strainer->skilline13) || (skill->skilline == strainer->skilline14) || (skill->skilline == strainer->skilline15)
                || (skill->skilline == strainer->skilline16) || (skill->skilline == strainer->skilline17) || (skill->skilline == strainer->skilline18)
                || (skill->skilline == strainer->skilline19) || (skill->skilline == strainer->skilline20))
            {
                Log::getSingleton().outString("skill %u with skillline %u matches",skill->spell,skill->skilline);
                SpellEntry *proto = sSpellStore.LookupEntry(skill->spell);
                if ((proto) )
                {
                    
                    

                    cnt++;
                }
            }
        }

        data.Initialize( SMSG_TRAINER_LIST );     
        data << guid;
        data << uint32(0) << uint32(cnt);

        uint32 num_added = 0;

        
        for (unsigned int t = 0;t < sSkillStore.GetNumRows();t++)
        {
            skilllinespell *skill = sSkillStore.LookupEntry(t);
            if ((skill->skilline == strainer->skilline1) || (skill->skilline == strainer->skilline2) || (skill->skilline == strainer->skilline3)
                || (skill->skilline == strainer->skilline4) || (skill->skilline == strainer->skilline5) || (skill->skilline == strainer->skilline6)
                || (skill->skilline == strainer->skilline7) || (skill->skilline == strainer->skilline8) || (skill->skilline == strainer->skilline9)
                || (skill->skilline == strainer->skilline10) || (skill->skilline == strainer->skilline11) || (skill->skilline == strainer->skilline12)
                || (skill->skilline == strainer->skilline13) || (skill->skilline == strainer->skilline14) || (skill->skilline == strainer->skilline15)
                || (skill->skilline == strainer->skilline16) || (skill->skilline == strainer->skilline17) || (skill->skilline == strainer->skilline18)
                || (skill->skilline == strainer->skilline19) || (skill->skilline == strainer->skilline20))
            {
                SpellEntry *proto = sSpellStore.LookupEntry(skill->spell);
                if ((proto))
                {
                    
                    

                    
                    data << uint32(skill->spell);
                    

                    if (GetPlayer()->HasSpell(skill->spell))
                    {
                        data << uint8(2);
                    }
                    else
                    {
                        if (((GetPlayer()->GetUInt32Value( UNIT_FIELD_LEVEL )) < (proto->spellLevel)) 
                            || (GetPlayer()->GetUInt32Value( PLAYER_FIELD_COINAGE ) < sWorld.mPrices[proto->spellLevel]))
                        {
                            data << uint8(1);
                        }
                        else
                        {
                            data << uint8(0);
                        }
                    }

                    data << uint32(sWorld.mPrices[proto->spellLevel]) << uint32(0);
                    data << uint32(0) << uint8(proto->spellLevel);
                    data << uint32(0);            
                    data << uint32(0);            
                    data << uint32(0);
                    data << uint32(0) << uint32(0);
                    
                    num_added++;
                    
                }
            }
        }

        

        data << "Hello! Ready for some training?";
        SendPacket( &data );
    }
}




void WorldSession::HandleTrainerBuySpellOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;
    uint32 spellId, playerGold, price;

    uint64 trainer = GetPlayer()->GetSelection();
    recv_data >> guid >> spellId;
    playerGold = GetPlayer( )->GetUInt32Value( PLAYER_FIELD_COINAGE );

    data.Initialize( SMSG_TRAINER_BUY_SUCCEEDED );
    data << guid << spellId;
    SendPacket( &data );
    SpellEntry *proto = sSpellStore.LookupEntry(spellId);
    price = sWorld.mPrices[proto->spellLevel];

    if( playerGold >= price 
        && ((GetPlayer()->GetUInt32Value( UNIT_FIELD_LEVEL )) >= (proto->spellLevel)))
    {
        GetPlayer( )->SetUInt32Value( PLAYER_FIELD_COINAGE, playerGold - price );

        

        data.Initialize( SMSG_SPELL_START );
        data << guid;
        data << guid;
        data << spellId;
        data << uint16(0);
        data << uint32(0);
        data << uint16(2);
        data << GetPlayer()->GetGUID();
        WPAssert(data.size() == 36);
        SendPacket( &data );

        data.Initialize( SMSG_LEARNED_SPELL );
        data << spellId;
        SendPacket( &data );
        GetPlayer()->addSpell((uint16)spellId);

        data.Initialize( SMSG_SPELL_GO );
        data << guid;
        data << guid;
        data << spellId;
        data << uint8(0) << uint8(1) << uint8(1);
        data << GetPlayer()->GetGUID();
        data << uint8(0);
        data << uint16(2);
        data << GetPlayer()->GetGUID();
        WPAssert(data.size() == 42);
        SendPacket( &data );

        data.Initialize( SMSG_SPELLLOGEXECUTE );
        data << guid;
        data << spellId;
        data << uint32(1);
        data << uint32(0x24);
        data << uint32(1);
        data << GetPlayer()->GetGUID();
        WPAssert(data.size() == 32);
        SendPacket( &data );

		
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
    WorldPacket data;
    uint64 guid;

    recv_data >> guid;

    data.Initialize( MSG_AUCTION_HELLO );
    data << guid;
    data << uint32(0);

    SendPacket( &data );
}





void WorldSession::HandleGossipHelloOpcode( WorldPacket & recv_data )
{
	Log::getSingleton( ).outString( "WORLD: Recieved CMSG_GOSSIP_HELLO" );

    WorldPacket data;
    uint64 guid;

    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

	if (unit == NULL)
	{
		Log::getSingleton( ).outDebug( "WORLD: CMSG_GOSSIP_HELLO - (%u) NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)), guid );
		return;
	}

	GetPlayer()->PlayerTalkClass->ClearMenus();

	scriptCallGossipHello( GetPlayer(), unit );
}





void WorldSession::HandleGossipSelectOptionOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDetail("WORLD: CMSG_GOSSIP_SELECT_OPTION");
    WorldPacket data;
    uint32 option;
    uint64 guid;
    
    recv_data >> guid >> option;
    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, GUID_LOPART(guid));
    if (unit == NULL)
    {
	Log::getSingleton( ).outDebug( "WORLD: CMSG_GOSSIP_SELECT_OPTION - (%u) NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)), guid );
	return;
    }
    
    scriptCallGossipSelect( GetPlayer(), unit, option, GetPlayer()->PlayerTalkClass->GossipOption( option ) );
}





void WorldSession::HandleSpiritHealerActivateOpcode( WorldPacket & recv_data )
{
    Affect *aff;

    SpellEntry *spellInfo = sSpellStore.LookupEntry( 2146 );
    if(spellInfo)
    {
        aff = new Affect(spellInfo,600000,GetPlayer()->GetGUID());
        GetPlayer( )->AddAffect(aff);
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
    GetPlayer( )->SetUInt32Value( UNIT_FIELD_FLAGS, (0xffffffff - 65536) & GetPlayer( )->GetUInt32Value( UNIT_FIELD_FLAGS ) );
    GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURA +32, 0 );
    GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURAFLAGS +4, 0 );
    GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURASTATE, 0 );
    GetPlayer( )->SetUInt32Value( PLAYER_BYTES_2, (0xffffffff - 0x10) & GetPlayer( )->GetUInt32Value( PLAYER_BYTES_2 ) );
    
    GetPlayer()->setDeathState(ALIVE);
}
