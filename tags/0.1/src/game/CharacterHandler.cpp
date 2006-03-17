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
#include "UpdateMask.h"
#include "Chat.h"
#include "Auth/md5.h"
#include "MapManager.h"
#include "ObjectAccessor.h"


void WorldSession::HandleCharEnumOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    
    
    data.Initialize(SMSG_CHAR_ENUM);

    
    std::stringstream ss;
    ss << "SELECT guid FROM characters WHERE acct=" << GetAccountId();

    QueryResult* result = sDatabase.Query( ss.str().c_str() );
    uint8 num = 0;

    data << num;

    if( result )
    {
        Player *plr;
        do
        {
            plr = new Player(this);
            ASSERT(plr);
            
            Log::getSingleton().outError("Loading char guid %d from account %d.",(*result)[0].GetUInt32(),GetAccountId());

            plr->LoadFromDB( (*result)[0].GetUInt32() );

            plr->BuildEnumData( &data );

            delete plr;

            num++;
        }
        while( result->NextRow() );

        delete result;
    }

    data.put<uint8>(0, num);

    SendPacket( &data );
}


void WorldSession::HandleCharCreateOpcode( WorldPacket & recv_data )
{
    std::string name;
    WorldPacket data;

    recv_data >> name;
    recv_data.rpos(0);

    std::stringstream ss;
    ss << "SELECT guid FROM characters WHERE name = '" << name << "'";

    QueryResult *result = sDatabase.Query( ss.str( ).c_str( ) );
    if (result)
    {
        delete result;

        data.Initialize(SMSG_CHAR_CREATE);
        data << (uint8)0x30;                      
        SendPacket( &data );

        return;
    }

    
    ss.rdbuf()->str("");
    ss << "SELECT guid FROM characters WHERE acct=" << GetAccountId();
    result = sDatabase.Query( ss.str( ).c_str( ) );
    if (result)
    {
        if (result->GetRowCount() >= 10)
        {
            data.Initialize(SMSG_CHAR_CREATE);
            data << (uint8)0x2F;                  
            SendPacket( &data );
            delete result;
            return;
        }
        delete result;
    }

    Player * pNewChar = new Player(this);
    pNewChar->Create( objmgr.GenerateLowGuid(HIGHGUID_PLAYER), recv_data );
    pNewChar->SaveToDB();

    delete pNewChar;

    data.Initialize( SMSG_CHAR_CREATE );
    data << (uint8)0x2C;                          
    data << uint8(0x2D);
    SendPacket( &data );


    data.Initialize(SMSG_CHAR_ENUM);
    ss << "SELECT guid FROM characters WHERE acct=" << GetAccountId();
    QueryResult* result1 = sDatabase.Query( ss.str().c_str() );
    uint8 num = 0;
    data << num;
    if( result1 )
    {
        Player *plr;
        do
        {
            plr = new Player(this);
            ASSERT(plr);
            Log::getSingleton().outError("Loading char guid %d from account %d.",(*result1)[0].GetUInt32(),GetAccountId());
            plr->LoadFromDB( (*result1)[0].GetUInt32() );
            plr->BuildEnumData( &data );

            delete plr;
            num++;
        }
        while( result1->NextRow() );
        delete result1;
    }
    data.put<uint8>(0, num);
    SendPacket( &data );
}




void WorldSession::HandleCharDeleteOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    uint64 guid;
    recv_data >> guid;

    Player* plr = new Player(this);
    ASSERT(plr);

    plr->LoadFromDB( GUID_LOPART(guid) );
    plr->DeleteFromDB();

    delete plr;

    data.Initialize(SMSG_CHAR_CREATE);
    data << (uint8)0x34;
    SendPacket( &data );
}

void WorldSession::HandlePlayerLoginOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 playerGuid = 0;

    DEBUG_LOG( "WORLD: Recvd Player Logon Message" );

    recv_data >> playerGuid;                        

    Player* plr = new Player(this);
    ASSERT(plr);

	plr->SetSession(this);  
    plr->LoadFromDB(GUID_LOPART(playerGuid));
    plr->_RemoveAllItemMods();

    SetPlayer(plr);

    
    data.Initialize( SMSG_ACCOUNT_DATA_MD5 );
    
    for(int i = 0; i < 80; i++)
        data << uint8(0);

    SendPacket(&data);

    
    extern char last_ip[32];
    printf("Character have IP: %s\n",last_ip);
    std::stringstream ips;
    ips << "UPDATE `characters` SET last_ip = '" << last_ip << "' WHERE guid = '" << playerGuid << "'";
    sDatabase.Execute(ips.str().c_str());

    
    sChatHandler.FillSystemMessageData(&data, this, sWorld.GetMotd());
    SendPacket( &data );

    DEBUG_LOG( "WORLD: Sent motd (SMSG_MESSAGECHAT)" );

    
    
    

    
    std::stringstream homeloc;
    Field *fields;
    int plrace = GetPlayer()->getRace();
    int plclass = GetPlayer()->getClass();

    homeloc << "SELECT mapID,zoneID,positionX,positionY,positionZ from playercreateinfo where race='" << plrace << "' AND class='" << plclass << "'";
    QueryResult *homeresult = sDatabase.Query( homeloc.str().c_str() );
    fields = homeresult->Fetch();
    DEBUG_LOG("Setting player home position: mapID is: %d, zoneID is %d, X is %f, Y is %f, Z is %f\n",fields[0].GetUInt32(),fields[1].GetUInt32(),fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat());

    data.Initialize (SMSG_BINDPOINTUPDATE);
    data << fields[2].GetFloat() << fields[3].GetFloat() << fields[4].GetFloat(); 
    data << fields[0].GetUInt32(); 
    data << fields[1].GetUInt32(); 
    SendPacket (&data);
    delete homeresult;

    

	
	
	
    data.Initialize( SMSG_TUTORIAL_FLAGS );
    
	for (int i = 0; i < 8; i++)
		data << uint32( GetPlayer()->GetTutorialInt(i) );
    
	SendPacket(&data);
    Log::getSingleton( ).outDebug( "WORLD: Sent tutorial flags." );

    
    GetPlayer()->smsg_InitialSpells();

    
    GetPlayer()->smsg_InitialActions();

    
    


	
	data.Initialize(SMSG_INITIALIZE_FACTIONS);
	data << uint32 (0x00000040); 
	for(int a=0; a<64; a++)
	{
		if(GetPlayer()->FactionIsInTheList(a))
		{
			std::list<struct Factions>::iterator itr;
			for(itr = GetPlayer()->factions.begin(); itr != GetPlayer()->factions.end(); ++itr)
			{
				if(itr->ReputationListID == a)
				{
					data << uint8  (itr->Flags);	
					data << uint32 (itr->Standing); 
					break;
				}
			}
		}
		else
		{
			data << uint8  (0x00);		 
			data << uint32 (0x00000000); 
		}
	}
	SendPacket(&data);

	
	GetPlayer()->InitializeHonor();
	GetPlayer()->SetPatent();


    

    

    data.Initialize(SMSG_LOGIN_SETTIMESPEED);
    time_t minutes = sWorld.GetGameTime( ) / 60;
    time_t hours = minutes / 60; minutes %= 60;
    time_t gameTime = minutes + ( hours << 6 );
    data << (uint32)gameTime;
    data << (float)0.017f;                          
    SendPacket( &data );

    

    
    
    
    

    data.Initialize( SMSG_TRIGGER_CINEMATIC );
    uint8 theRace = GetPlayer()->getRace();         

    PlayerCreateInfo *info = objmgr.GetPlayerCreateInfo(theRace, 1);
    ASSERT(info);

    if (theRace == HUMAN)                           
    {
        if (GetPlayer()->m_positionX == info->positionX)
            if (GetPlayer()->m_positionY == info->positionY)
                if (GetPlayer()->m_positionZ == info->positionZ)
                {
                    data << uint32(81);
                    SendPacket( &data );
                }
    }

    if (theRace == ORC)                             
    {
        if (GetPlayer()->m_positionX == info->positionX)
            if (GetPlayer()->m_positionY == info->positionY)
                if (GetPlayer()->m_positionZ == info->positionZ)
                {
                    data << uint32(21);
                    SendPacket( &data );
                }
    }

    if (theRace == DWARF)                           
    {
        if (GetPlayer()->m_positionX == info->positionX)
            if (GetPlayer()->m_positionY == info->positionY)
                if (GetPlayer()->m_positionZ == info->positionZ)
                {
                    data << uint32(41);
                    SendPacket( &data );
                }
    }
    if (theRace == NIGHTELF)                        
    {
        if (GetPlayer()->m_positionX == info->positionX)
            if (GetPlayer()->m_positionY == info->positionY)
                if (GetPlayer()->m_positionZ == info->positionZ)
                {
                    data << uint32(61);
                    SendPacket( &data );
                }
    }
    if (theRace == UNDEAD_PLAYER)                   
    {
        if (GetPlayer()->m_positionX == info->positionX)
            if (GetPlayer()->m_positionY == info->positionY)
                if (GetPlayer()->m_positionZ == info->positionZ)
                {
                    data << uint32(2);
                    SendPacket( &data );
                }
    }
    if (theRace == TAUREN)                          
    {
        if (GetPlayer()->m_positionX == info->positionX)
            if (GetPlayer()->m_positionY == info->positionY)
                if (GetPlayer()->m_positionZ == info->positionZ)
                {
                    data << uint32(141);
                    SendPacket( &data );
                }
    }
    if (theRace == GNOME)                           
    {
        if (GetPlayer()->m_positionX == info->positionX)
            if (GetPlayer()->m_positionY == info->positionY)
                if (GetPlayer()->m_positionZ == info->positionZ)
                {
                    data << uint32(101);
                    SendPacket( &data );
                }
    }
    if (theRace == TROLL)                           
    {
        if (GetPlayer()->m_positionX == info->positionX)
            if (GetPlayer()->m_positionY == info->positionY)
                if (GetPlayer()->m_positionZ == info->positionZ)
                {
                    data << uint32(121);
                    SendPacket( &data );
                }
    }

    Player *pCurrChar = GetPlayer();

    pCurrChar->InitExploreSystem();

	
	std::stringstream query;
	query << "SELECT * FROM `guilds_members` where memguid= " << pCurrChar->GetGUID();
	QueryResult *result = sDatabase.Query( query.str().c_str() );

	if(result)
	{
		Field *fields = result->Fetch();
		pCurrChar->SetInGuild(fields[0].GetUInt32());
		pCurrChar->SetRank(fields[2].GetUInt32());
	}

    
    
    
    
    
    Log::getSingleton( ).outError("AddObject at CharacterHandler.cpp");
    MapManager::Instance().GetMap(pCurrChar->GetMapId())->Add(pCurrChar);
    ObjectAccessor::Instance().InsertPlayer(pCurrChar);

    std::stringstream ss;
    ss << "UPDATE characters SET online = 1 WHERE guid = " << pCurrChar->GetGUID();
    sDatabase.Execute(ss.str().c_str());

    
    for (uint16 sl = PLAYER_SKILL_INFO_1_1; sl < PLAYER_SKILL_INFO_1_1_381; sl += 3)
    {
        uint16 curr = 0, max = 0;
        uint32 id = pCurrChar->GetUInt32Value(sl);
        if (id == 0) continue;
        curr = (uint16)pCurrChar->GetUInt32Value(sl + 1);
        max = (uint16)(pCurrChar->GetUInt32Value(sl + 1) >> 16);
        pCurrChar->AddSkillLine(id, curr, max, false);
    }
    

    

    std::string outstring = pCurrChar->GetName();
    outstring.append( " has come online." );
    pCurrChar->BroadcastToFriends(outstring);

}

void WorldSession::HandleSetFactionAtWar( WorldPacket & recv_data )
{
	Log::getSingleton().outDebug("WORLD SESSION: HandleSetFactionAtWar");

	uint32 FactionID;
	uint8  Flag;

	recv_data >> FactionID;
	recv_data >> Flag;

	std::list<struct Factions>::iterator itr;

	for(itr = GetPlayer()->factions.begin(); itr != GetPlayer()->factions.end(); ++itr)
	{
		if(itr->ReputationListID == FactionID) 
		{
			if( Flag )
				itr->Flags = (itr->Flags | 2);
			else
				if( itr->Flags >= 2) itr->Flags -= 2;
			break;
		}
	}
}

void WorldSession::HandleSetFactionCheat( WorldPacket & recv_data )
{
	Log::getSingleton().outDebug("WORLD SESSION: HandleSetFactionCheat");

	uint32 FactionID;
	uint32 Standing;

	recv_data >> FactionID;
	recv_data >> Standing;

	std::list<struct Factions>::iterator itr;

	for(itr = GetPlayer()->factions.begin(); itr != GetPlayer()->factions.end(); ++itr)
	{
		if(itr->ReputationListID == FactionID) 
		{
			itr->Standing += Standing;
			itr->Flags = (itr->Flags | 1); 
			break;
		}
	}

	GetPlayer()->UpdateReputation();
}

void WorldSession::HandleMeetingStoneInfo( WorldPacket & recv_data )
{
    DEBUG_LOG( "WORLD: Recieved CMSG_MEETING_STONE_INFO" );
#ifndef _VERSION_1_7_0_                             
    WorldPacket data;
    data.Initialize( SMSG_MEETING_STONE_INFO );
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    SendPacket(&data);
#endif 
}




void WorldSession::HandleTutorialFlag( WorldPacket & recv_data )
{
	uint32 iFlag;
	recv_data >> iFlag;

	uint32 wInt = (iFlag / 32);
	uint32 rInt = (iFlag % 32);

	uint32 tutflag = GetPlayer()->GetTutorialInt( wInt );
	tutflag |= (1 << rInt);
	GetPlayer()->SetTutorialInt( wInt, tutflag );

	Log::getSingleton().outDebug("Received Tutorial Flag Set {%u}.", iFlag);
}


void WorldSession::HandleTutorialClear( WorldPacket & recv_data )
{
	for ( uint32 iI = 0; iI < 8; iI++)
		GetPlayer()->SetTutorialInt( iI, 0xFFFFFFFF );
}


void WorldSession::HandleTutorialReset( WorldPacket & recv_data )
{
	for ( uint32 iI = 0; iI < 8; iI++)
		GetPlayer()->SetTutorialInt( iI, 0x00000000 );
}
