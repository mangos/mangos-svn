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
#include "UpdateMask.h"
#include "Chat.h"
#include "Auth/md5.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Opcodes.h"
#include "NameTables.h"

#include "AuthCodes.h"

int
WorldSession::HandleCharEnumOpcode( WorldPacket & recv_data )
{
	ACE_TRACE("WorldSession::HandleCharEnumOpcode( WorldPacket & recv_data )");
	
	mysqlpp::Result *res = 0;
	ACE_NEW_RETURN(res, mysqlpp::Result, -1);

	std::stringstream strbuf;
	strbuf << "SELECT `guid` FROM `character` WHERE `account` = '" << (unsigned long)GetAccountId() << "' ORDER BY `guid`";
	if ( sDatabaseMysql->PQuery(DATABASE_WORLD, strbuf.str() , *res) == 1 )
	{
		mysqlpp::Row::size_type size = res->size();

		ACE_DEBUG((LM_DEBUG, "%I-- %D - %M - Loaded %d characters from DataBase -- %N:%l --\n", size));

		WorldPacket data;
		data.Initialize(SMSG_CHAR_ENUM);
	
		uint8 num = 0;
		data << num;
		
		if ( size > 0 )
		{
			Player *plr = 0;
			ACE_NEW_RETURN(plr, Player(this), -1);

			mysqlpp::Row::size_type i;
			for (i = 0; i < size; i++)
			{
				mysqlpp::Row row = res->at(i);
				int ld = plr->LoadFromDB( uint32(row.at(0)) );
				if( ld == 1 )
				{
					ACE_DEBUG((LM_DEBUG, "%I-- %D - %M - Loaded Player from DataBase -- %N:%l --\n" ));
					plr->BuildEnumData( &data );
					num++;
				}
				else if ( ld == -1 )
				{
					delete plr;
					delete res;
					ACE_ERROR_RETURN((LM_ALERT,  "%I-- %D - %M - Unable to loaded Player from DataBase -- %N:%l --\n" ), -1);
				}
			}
			delete plr;
		}
		delete res;

		data.put<uint8>(0, num);
		return SendPacket( &data );
	}
	else
	{
		delete res;
		ACE_ERROR_RETURN((LM_ALERT, "%I-- %D - %M - Unable to load characters from DataBase -- %N:%l --\n"), -1);
	}
}

int
WorldSession::HandleCharCreateOpcode( WorldPacket & recv_data )
{
	ACE_TRACE("WorldSession::HandleCharCreateOpcode( WorldPacket & recv_data )\n");

	mysqlpp::Result *res = 0;
	ACE_NEW_RETURN(res, mysqlpp::Result, -1);
	
	std::string opcodehandler = LookupName(recv_data.GetOpcode(), g_worldOpcodeNames);
	std::string name;
    uint8 race_;

    recv_data >> name;
    recv_data >> race_;
    recv_data.rpos(0);
	
	WorldPacket data;
	data.Initialize(SMSG_CHAR_CREATE);

	std::stringstream strbuf;
	strbuf << "SELECT `guid` FROM `character` WHERE `name` = '" <<  name.c_str() << "'";

	if ( sDatabaseMysql->PQuery(DATABASE_WORLD, strbuf.str() , *res) == 1 )
	{

		if( res->size() > 0 )
		{
			ACE_DEBUG((LM_INFO, "%I-- %D - %M - [%d:%s] Character already in use --\n" , _accountId, opcodehandler.c_str() ));

			delete res;
			data << (uint8)CHAR_CREATE_IN_USE;
			return SendPacket( &data );
		}
		else
		{
			strbuf.str("");
			strbuf << "SELECT `guid` FROM `character` WHERE `account` = '" << (unsigned long)_accountId << "'";
			if ( sDatabaseMysql->PQuery(DATABASE_WORLD, strbuf.str() , *res) == 1 )
			{
				if (res->size() >= 10)
				{
					ACE_DEBUG((LM_INFO, "%I-- %D - %M - [%d:%s]: Character error - Reach Maximum Limit --\n" , _accountId, opcodehandler.c_str() ));
				}
				else
				{
					/* TODO: check account character creation limit */

					/* TODO: check mangosd character creation limit */

					/*uint32 GameType = sWorld.getConfig(CONFIG_GAME_TYPE);
					if(GameType == 1 || GameType == 8)
					{
						strbuf.flush();
						strbuf << "SELECT `race` FROM `character` WHERE `account` = '" << (unsigned long)GetAccountId() << "' LIMIT 1";
						if ( sDatabaseMysql->PQuery(DATABASE_WORLD, strbuf.str() , *res) == 1 )
						{
							if (res->size() > 0)
							{
								Field * field = result2->Fetch();
								uint8 race = field[0].GetUInt32();
								delete result2;
								uint32 team=0;
								if(race > 0)
								{
									switch(race)
									{
										case HUMAN:
											team = (uint32)ALLIANCE;
											break;
										case DWARF:
											team = (uint32)ALLIANCE;
											break;
										case NIGHTELF:
											team = (uint32)ALLIANCE;
											break;
										case GNOME:
											team = (uint32)ALLIANCE;
											break;
										case ORC:
											team = (uint32)HORDE;
											break;
										case UNDEAD_PLAYER:
											team = (uint32)HORDE;
											break;
										case TAUREN:
											team = (uint32)HORDE;
											break;
										case TROLL:
											team = (uint32)HORDE;
											break;
									}

								}
								uint32 team_=0;
								if(race_ > 0)
								{
									switch(race_)
									{
										case HUMAN:
											team_ = (uint32)ALLIANCE;
											break;
										case DWARF:
											team_ = (uint32)ALLIANCE;
											break;
										case NIGHTELF:
											team_ = (uint32)ALLIANCE;
											break;
										case GNOME:
											team_ = (uint32)ALLIANCE;
											break;
										case ORC:
											team_ = (uint32)HORDE;
											break;
										case UNDEAD_PLAYER:
											team_ = (uint32)HORDE;
											break;
										case TAUREN:
											team_ = (uint32)HORDE;
											break;
										case TROLL:
											team_ = (uint32)HORDE;
											break;
									}
								}
								if(team != team_)
								{
									data << (uint8)CHAR_CREATE_PVP_TEAMS_VIOLATION;
									return SendPacket( &data );
								}
							}
						}
						else
						{
						}
					}*/

					Player* pNewChar = 0;
					ACE_NEW_RETURN(pNewChar, Player(this), -1);

					if(pNewChar->Create( objmgr.GenerateLowGuid(HIGHGUID_PLAYER), recv_data )) /* TODO: check any error on load */
					{
						// Player create
						pNewChar->SaveToDB(); /* TODO: check for any error on save */
						
						strbuf.str("");
						strbuf << "SELECT COUNT(guid) FROM `character` WHERE `account` = '" << GetAccountId() << "'";
						if ( sDatabaseMysql->PQuery(DATABASE_WORLD, strbuf.str() , *res) == 1 )
						{
							if (res->size() > 0)
							{
								mysqlpp::Row row = res->at(0);
								uint32 charCount = uint32(row.at(0));
								strbuf.str("");
								strbuf << "INSERT INTO `realmcharacters` (`numchars`, `acctid`, `realmid`) VALUES (" << charCount;
								strbuf << "," << _accountId << ","<< realmID <<") ON DUPLICATE KEY UPDATE `numchars` = " << charCount;
								if ( sDatabaseMysql->PExecute(DATABASE_LOGIN, strbuf.str()) == 1 )
								{
									delete res;
									delete pNewChar;
									ACE_DEBUG((LM_INFO, "%I-- %D - %M - [%d:%s]: Character sucessufully created --\n" , _accountId, opcodehandler.c_str() ));
									data << (uint8)CHAR_CREATE_SUCCESS;
									return SendPacket( &data );
									
								}
								else
								{
									ACE_DEBUG((LM_ALERT, "%I-- %D - %M - [%d:%s]: Character error - Unable to insert characters -- %N:%l --\n" , _accountId, opcodehandler.c_str() ));
									delete pNewChar;
								}	
							}
							else
							{
								ACE_DEBUG((LM_ERROR, "%I-- %D - %M - [%d:%s]: Character error - Unable to count characters --\n" , _accountId, opcodehandler.c_str() ));
								delete pNewChar;
							}
						}
						else
						{
							ACE_DEBUG((LM_ALERT, "%I-- %D - %M - [%d:%s]: Unable to Count guid of Characters from DataBase -- %N:%l --\n", _accountId, opcodehandler.c_str() ));
							delete pNewChar;
						}
					}
					else
					{
						ACE_DEBUG((LM_ERROR, "%I-- %D - %M - [%d:%s]: Character error - Unable Create Player New Guid --\n" , _accountId, opcodehandler.c_str() ));
						delete pNewChar;
					}
				} 
			}
			else
			{
				ACE_DEBUG((LM_ALERT, "%I-- %D - %M - [%d:%s]: Unable to Count Characters from DataBase -- %N:%l --\n", _accountId, opcodehandler.c_str() ));
			}
		} 
	}
	else
	{
		ACE_DEBUG((LM_ALERT, "%I-- %D - %M - [%d:%s]: Unable to check if Character is in use from DataBase -- %N:%l --\n", _accountId, opcodehandler.c_str() ));
	}
	delete res;
	data << (uint8)CHAR_CREATE_ERROR;
	return SendPacket( &data );
}

int
WorldSession::HandleCharDeleteOpcode( WorldPacket & recv_data )
{
	ACE_TRACE("WorldSession::HandleCharDeleteOpcode( WorldPacket & recv_data )\n");

	std::string opcodehandler = LookupName(recv_data.GetOpcode(), g_worldOpcodeNames);

    WorldPacket data;
	data.Initialize(SMSG_CHAR_CREATE);

    uint64 guid;
    recv_data >> guid;
	
	Player* plr = 0;
	ACE_NEW_RETURN(plr, Player(this), -1);

	int ld = plr->LoadFromDB( GUID_LOPART(guid) );

    if( ld  == 1)
	{
		plr->DeleteFromDB(); /* todo: check for errors */
		data << (uint8)CHAR_DELETE_SUCCESS;
	}
	else if ( ld == 0)
	{
		ACE_DEBUG((LM_ERROR, "%I-- %D - %M - [%d:%s]: Unable to find Character on database to delete -- %N:%l --\n", _accountId, opcodehandler.c_str() ));
		data << (uint8)CHAR_DELETE_FAILED;
	}
	else
	{
		ACE_DEBUG((LM_ALERT, "%I-- %D - %M - [%d:%s]: Error loading Character from database -- %N:%l --\n", _accountId, opcodehandler.c_str() ));
		data << (uint8)CHAR_DELETE_FAILED;
	}
	delete plr;
	return SendPacket( &data );
}

int
WorldSession::HandleChangePlayerNameOpcode(WorldPacket& recv_data)
{
    // TODO
    // need to be written
	ACE_DEBUG( (LM_NOTICE, "TODO: WorldSession::HandleChangePlayerNameOpcode(WorldPacket& recv_data) needs to be written.\n"));
	return 0;
}

int
WorldSession::HandlePlayerLoginOpcode( WorldPacket & recv_data )
{
	ACE_TRACE("WorldSession::HandlePlayerLoginOpcode( WorldPacket & recv_data )\n");

    WorldPacket data;
    uint64 playerGuid = 0;

    DEBUG_LOG( "WORLD: Recvd Player Logon Message" );

    recv_data >> playerGuid;

    Player* plr = 0;
	ACE_NEW_RETURN(plr, Player(this), -1);

    if( plr->LoadFromDB(GUID_LOPART(playerGuid)) == -1)
        return -1;

    //plr->_RemoveAllItemMods();

    SetPlayer(plr);

    data.Initialize( SMSG_ACCOUNT_DATA_MD5 );

    for(int i = 0; i < 80; i++)
        data << uint8(0);

    SendPacket(&data);

    sChatHandler.FillSystemMessageData(&data, this, sWorld.GetMotd());
    SendPacket( &data );

    DEBUG_LOG( "WORLD: Sent motd (SMSG_MESSAGECHAT)" );

    // home bind stuff
    Field *fields;
    QueryResult *result7 = sDatabase.PQuery("SELECT COUNT(*) FROM `character_homebind` WHERE `guid` = '%u';", GUID_LOPART(playerGuid));
    if (result7)
    {
        int cnt;
        fields = result7->Fetch();
        cnt = fields[0].GetUInt32();

        if ( cnt > 0 )
        {
            QueryResult *result4 = sDatabase.PQuery("SELECT `map`,`zone`,`position_x`,`position_y`,`position_z` FROM `character_homebind` WHERE `guid` = '%u';", GUID_LOPART(playerGuid));
            fields = result4->Fetch();
            data.Initialize (SMSG_BINDPOINTUPDATE);
            data << fields[2].GetFloat() << fields[3].GetFloat() << fields[4].GetFloat();
            data << fields[0].GetUInt32();
            data << fields[1].GetUInt32();
            SendPacket (&data);
            DEBUG_LOG("Setting player home position: mapid is: %u, zoneid is %u, X is %f, Y is %f, Z is %f\n",fields[0].GetUInt32(),fields[1].GetUInt32(),fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat());
            delete result4;
        }
        else
        {
            int plrace = GetPlayer()->getRace();
            int plclass = GetPlayer()->getClass();
            QueryResult *result5 = sDatabase.PQuery("SELECT `map`,`zone`,`position_x`,`position_y`,`position_z` FROM `playercreateinfo` WHERE `race` = '%u' AND `class` = '%u';", plrace, plclass);
            fields = result5->Fetch();
            // store and send homebind for player
            sDatabase.PExecute("INSERT INTO `character_homebind` (`guid`,`map`,`zone`,`position_x`,`position_y`,`position_z`) VALUES ('%u', '%u', '%u', '%f', '%f', '%f');", GUID_LOPART(playerGuid), fields[0].GetUInt32(), fields[1].GetUInt32(), fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat());
            data.Initialize (SMSG_BINDPOINTUPDATE);
            data << fields[2].GetFloat() << fields[3].GetFloat() << fields[4].GetFloat();
            data << fields[0].GetUInt32();
            data << fields[1].GetUInt32();
            SendPacket (&data);
            DEBUG_LOG("Setting player home position: mapid is: %u, zoneid is %u, X is %f, Y is %f, Z is %f\n",fields[0].GetUInt32(),fields[1].GetUInt32(),fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat());
            delete result5;
        }
        delete result7;
    }

    data.Initialize( SMSG_TUTORIAL_FLAGS );

    for (int i = 0; i < 8; i++)
        data << uint32( GetPlayer()->GetTutorialInt(i) );

    SendPacket(&data);
    sLog.outDebug( "WORLD: Sent tutorial flags." );

    GetPlayer()->SendInitialSpells();

    GetPlayer()->SendInitialActions();

    /*if(GetPlayer()->getClass() == CLASS_HUNTER || GetPlayer()->getClass() == CLASS_ROGUE)
    {
        uint32 shiftdata=0x01;
        for(uint8 i=0;i<32;i++)
        {
            if ( 522753 & shiftdata )
            {
                data.Initialize(SMSG_SET_FLAT_SPELL_MODIFIER);
                data << uint8(i);
                data << uint8(5);
                data << uint16(1);
                data << uint16(0);
                SendPacket(&data);
            }
            shiftdata=shiftdata<<1;
        }
    }*/

    data.Initialize(SMSG_INITIALIZE_FACTIONS);
    data << uint32 (0x00000040);
    for(uint32 a=0; a<64; a++)
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

    GetPlayer()->UpdateHonor();

    GetPlayer()->SetPvP( !GetPlayer()->HasFlag(UNIT_FIELD_FLAGS , UNIT_FLAG_NOT_IN_PVP) );

    data.Initialize(SMSG_LOGIN_SETTIMESPEED);
    time_t minutes = sWorld.GetGameTime( ) / 60;
    time_t hours = minutes / 60; minutes %= 60;
    time_t gameTime = minutes + ( hours << 6 );
    data << (uint32)gameTime;
    data << (float)0.017f;
    SendPacket( &data );

    //Show cinematic at the first time that player login
    if( !GetPlayer()->getCinematic() )
    {
        GetPlayer()->setCinematic(1);

        data.Initialize( SMSG_TRIGGER_CINEMATIC );

        uint8 race = GetPlayer()->getRace();
        switch (race)
        {
            case HUMAN:         data << uint32(81);  break;
            case ORC:           data << uint32(21);  break;
            case DWARF:         data << uint32(41);  break;
            case NIGHTELF:      data << uint32(61);  break;
            case UNDEAD_PLAYER: data << uint32(2);   break;
            case TAUREN:        data << uint32(141); break;
            case GNOME:         data << uint32(101); break;
            case TROLL:         data << uint32(121); break;
            default:            data << uint32(0);
        }
        SendPacket( &data );
    }

	Player *pCurrChar = GetPlayer();

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `guild_member` WHERE `guid` = '%u';",pCurrChar->GetGUIDLow());

    if(result)
    {
        Field *fields = result->Fetch();
        pCurrChar->SetInGuild(fields[0].GetUInt32());
        pCurrChar->SetRank(fields[2].GetUInt32());
    }

    sLog.outError("AddObject at CharacterHandler.cpp");
    MapManager::Instance().GetMap(pCurrChar->GetMapId())->Add(pCurrChar);
    ObjectAccessor::Instance().InsertPlayer(pCurrChar);

    sDatabase.PExecute("UPDATE `character` SET `online` = 1 WHERE `guid` = '%u';", pCurrChar->GetGUID());
    loginDatabase.PExecute("UPDATE `account` SET `online` = 1 WHERE `id` = '%u';", GetAccountId());

    plr->SetInGameTime( getMSTime() );

    std::string outstring = pCurrChar->GetName();
    outstring.append( " has come online." );
    pCurrChar->BroadcastToFriends(outstring);

    // setting new speed if dead
    if ( pCurrChar->m_deathState == DEAD )
    {
        GetPlayer()->SetMovement(MOVE_WATER_WALK);

        if (GetPlayer()->getRace() == RACE_NIGHT_ELF)
        {
            pCurrChar->SetPlayerSpeed(MOVE_RUN, (float)12.75, true);
            pCurrChar->SetPlayerSpeed(MOVE_SWIM, (float)8.85, true);
        }
        else
        {
            pCurrChar->SetPlayerSpeed(MOVE_RUN, (float)10.625, true);
            pCurrChar->SetPlayerSpeed(MOVE_SWIM, (float)7.375, true);
        }
    }

    delete result;
	
	return 0;
}

void WorldSession::HandleSetFactionAtWar( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD SESSION: HandleSetFactionAtWar");

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

//I think this function is never used :/ I dunno, but i guess this opcode not exists
void WorldSession::HandleSetFactionCheat( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD SESSION: HandleSetFactionCheat");
    /*
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
    */
    GetPlayer()->UpdateReputation();
}

void WorldSession::HandleMeetingStoneInfo( WorldPacket & recv_data )
{
    DEBUG_LOG( "WORLD: Received CMSG_MEETING_STONE_INFO" );
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

    sLog.outDebug("Received Tutorial Flag Set {%u}.", iFlag);
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

void WorldSession::HandleSetWatchedFactionIndexOpcode(WorldPacket & recv_data)
{
    DEBUG_LOG("WORLD: Received CMSG_SET_WATCHED_FACTION_INDEX");
    uint32 fact;
    recv_data >> fact;
    GetPlayer()->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, fact);
}

int
WorldSession::HandleLogoutRequestOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    Player* Target = GetPlayer();

    ACE_DEBUG((LM_DEBUG, "WORLDSESSION: Received CMSG_LOGOUT_REQUEST Message.\n"));

    //Can not logout if...
    if( Target->isInCombat() ||                             //...is in combat
        Target->isInDuel()   ||                             //...is in Duel
                                                            //...is jumping ...is falling
        Target->HasMovementFlags( MOVEMENT_JUMPING | MOVEMENT_FALLING ))
    {
        data.Initialize( SMSG_LOGOUT_RESPONSE );
        data << (uint8)0xC;
        data << uint32(0);
        data << uint8(0);
        return SendPacket( &data );
    }

    Target->SetFlag(UNIT_FIELD_BYTES_1, PLAYER_STATE_SIT);

    if(!Target->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING))
    {                                                       //in city no root no lock rotate
        data.Initialize( SMSG_FORCE_MOVE_ROOT );
        data << (uint8)0xFF;
		data << Target->GetGUID();
		data << (uint32)2;
        SendPacket( &data );

        Target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
    }
	
	
	this->m_timer_id[STIMER_LOGOUT] = this->reactor()->schedule_timer(this,(const void *)STIMER_LOGOUT, ACE_Time_Value(20, 0));

	if( m_timer_id[STIMER_LOGOUT] == -1 )
		return -1;

    data.Initialize( SMSG_LOGOUT_RESPONSE );
    data << uint32(0);
    data << uint8(0);
    return SendPacket( &data );
}

int
WorldSession::HandlePlayerLogoutOpcode( WorldPacket & recv_data )
{
	ACE_DEBUG((LM_DEBUG, "WORLDSESSION:  Recvd CMSG_PLAYER_LOGOUT Message.\n"));

    if (_security > 0)
    {
        LogoutPlayer(1,0);
    }
	return 0;
}

int
WorldSession::HandleLogoutCancelOpcode( WorldPacket & recv_data )
{
	ACE_DEBUG((LM_DEBUG, "WORLDSESSION:  Recvd CMSG_LOGOUT_CANCEL Message.\n"));

	WorldPacket data;
    data.Initialize( SMSG_LOGOUT_CANCEL_ACK );
    if (SendPacket( &data ) == -1)
    {
		return -1;
    }

	//cancel timer
	if(this->m_timer_id[STIMER_LOGOUT] != STIMER_NOTSET)
	{
		this->reactor()->cancel_timer(this->m_timer_id[STIMER_LOGOUT]);
	}
    //!we can move again
    data.Initialize( SMSG_FORCE_MOVE_UNROOT );
    data << (uint8)0xFF;
	data << _player->GetGUID();
    if(SendPacket( &data ) == -1)
	{
		return -1;
	}
	
    //! Stand Up
    //! Removes the flag so player stands
    GetPlayer()->RemoveFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT);

    //! DISABLE_ROTATE
    GetPlayer()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
}
