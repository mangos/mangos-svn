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

void WorldSession::HandleCharEnumOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    /*// uknown opcode ?
    WorldPacket packet;
    packet.Initialize(SMSG_AUTH_RESPONSE2_UNKNOWN180);
    packet << uint8(10);
    for (int i=0; i<10; i++)
    packet << uint8(0x02) << uint8(0x01) << uint16(0) << uint32(0);
    SendPacket(&packet);*/

    data.Initialize(SMSG_CHAR_ENUM);

    QueryResult *result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `account` = '%lu';", (unsigned long)GetAccountId());

    uint8 num = 0;

    data << num;

    if( result )
    {
        Player *plr;
        do
        {
            plr = new Player(this);
            ASSERT(plr);

            sLog.outError("Loading char guid %d from account %d.",(*result)[0].GetUInt32(),GetAccountId());

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

    QueryResult *result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `name` = '%s';", name.c_str());

    if ( result )
    {
        delete result;

        data.Initialize(SMSG_CHAR_CREATE);
        data << (uint8)0x31;
        SendPacket( &data );

        return;
    }

    result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `account` = '%lu';", (unsigned long)GetAccountId());

    if ( result )
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

    if(pNewChar->Create( objmgr.GenerateLowGuid(HIGHGUID_PLAYER), recv_data ))
    {
        // Player create
        pNewChar->SaveToDB();

        delete pNewChar;
    }
    else
    {
        // Player not create (race/class problem?)
        delete pNewChar;

        data.Initialize(SMSG_CHAR_CREATE);
        data << (uint8)0x2F;
        SendPacket( &data );

        return;
    }

    // we have successfull creation
    // note all error codes moved + 1
    // 0x2E - Character created
    // 0x30 - Char create failed
    // 0x31 - Char name is in use
    // 0x35 - Char delete Okay
    // 0x36 - Char delete failed

    data.Initialize( SMSG_CHAR_CREATE );
    data << (uint8)0x2E;
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

    sChatHandler.FillSystemMessageData(&data, this, sWorld.GetMotd());
    SendPacket( &data );

    DEBUG_LOG( "WORLD: Sent motd (SMSG_MESSAGECHAT)" );

    // home bind stuff
    Field *fields;
    QueryResult *result7 = sDatabase.PQuery("SELECT COUNT(*) FROM `character_homebind` WHERE `guid` = '%d';", playerGuid);
    if (result7)
    {
        int cnt;
        fields = result7->Fetch();
        cnt = fields[0].GetUInt32();

        if ( cnt > 0 )
        {
            QueryResult *result4 = sDatabase.PQuery("SELECT `map`,`zone`,`position_x`,`position_y`,`position_z` FROM `character_homebind` WHERE `guid` = '%d';", playerGuid);
            fields = result4->Fetch();
            data.Initialize (SMSG_BINDPOINTUPDATE);
            data << fields[2].GetFloat() << fields[3].GetFloat() << fields[4].GetFloat();
            data << fields[0].GetUInt32();
            data << fields[1].GetUInt32();
            SendPacket (&data);
            DEBUG_LOG("Setting player home position: mapid is: %d, zoneid is %d, X is %f, Y is %f, Z is %f\n",fields[0].GetUInt32(),fields[1].GetUInt32(),fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat());
            delete result4;
        }
        else
        {
            int plrace = GetPlayer()->getRace();
            int plclass = GetPlayer()->getClass();
            QueryResult *result5 = sDatabase.PQuery("SELECT `map`,`zone`,`position_x`,`position_y`,`position_z` FROM `playercreateinfo` WHERE `race` = '%u' AND `class` = '%u';", plrace, plclass);
            fields = result5->Fetch();
            // store and send homebind for player
            sDatabase.PExecute("INSERT INTO `character_homebind` (`guid`,`map`,`zone`,`position_x`,`position_y`,`position_z`) VALUES ('%lu', '%d', '%d', '%f', '%f', '%f');", (unsigned long)playerGuid, fields[0].GetUInt32(), fields[1].GetUInt32(), fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat());
            data.Initialize (SMSG_BINDPOINTUPDATE);
            data << fields[2].GetFloat() << fields[3].GetFloat() << fields[4].GetFloat();
            data << fields[0].GetUInt32();
            data << fields[1].GetUInt32();
            SendPacket (&data);
            DEBUG_LOG("Setting player home position: mapid is: %d, zoneid is %d, X is %f, Y is %f, Z is %f\n",fields[0].GetUInt32(),fields[1].GetUInt32(),fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat());
            delete result5;
        }
        delete result7;
    }

    // set proficiency
    switch (GetPlayer()->getClass())
    {
        case CLASS_MAGE:
            SendProficiency (0x04, 0x02);
            SendProficiency (0x02, 0x00, 0x04);
            SendProficiency (0x02, 0x00, 0x44);
            SendProficiency (0x04, 0x03);
            SendProficiency (0x02, 0x00, 0x44, 0x08);
            break;
        case CLASS_ROGUE:
            SendProficiency (0x04, 0x02);
            SendProficiency (0x02, 0x00, 0x00, 0x01);
            SendProficiency (0x04, 0x06);
            SendProficiency (0x02, 0x00, 0x80, 0x01);
            SendProficiency (0x02, 0x00, 0xC0, 0x01);
            SendProficiency (0x04, 0x07);
            break;
        case CLASS_WARRIOR:
            SendProficiency (0x04, 0x02);
            SendProficiency (0x02, 0x01);
            SendProficiency (0x02, 0x11);
            SendProficiency (0x04, 0x42);
            SendProficiency (0x04, 0x4A);
            SendProficiency (0x04, 0x4E);
            SendProficiency (0x02, 0x11, 0x40);
            SendProficiency (0x04, 0x4F);
            SendProficiency (0x02, 0x91, 0x40);
            break;
        case CLASS_PALADIN:
            SendProficiency (0x04, 0x02);
            SendProficiency (0x02, 0x10);
            SendProficiency (0x04, 0x42);
            SendProficiency (0x02, 0x30);
            SendProficiency (0x04, 0x4A);
            SendProficiency (0x04, 0x4E);
            SendProficiency (0x02, 0x30, 0x40);
            SendProficiency (0x04, 0x4F);
            break;
        case CLASS_WARLOCK:
            SendProficiency (0x04, 0x02);
            SendProficiency (0x02, 0x00, 0x80);
            SendProficiency (0x02, 0x00, 0xC0);
            SendProficiency (0x04, 0x03);
            SendProficiency (0x02, 0x00, 0xC0, 0x08);
            break;
        case CLASS_PRIEST:
            SendProficiency (0x04, 0x02);
            SendProficiency (0x02, 0x10);
            SendProficiency (0x02, 0x10, 0x40);
            SendProficiency (0x04, 0x03);
            SendProficiency (0x02, 0x10, 0x40, 0x08);
            break;
        case CLASS_DRUID:
            SendProficiency (0x04, 0x02);
            SendProficiency (0x02, 0x00, 0x04);
            SendProficiency (0x04, 0x06);
            SendProficiency (0x02, 0x00, 0x84);
            SendProficiency (0x02, 0x00, 0xC4);
            SendProficiency (0x04, 0x07);
            break;
        case CLASS_HUNTER:
            SendProficiency (0x04, 0x02);
            SendProficiency (0x02, 0x01);
            SendProficiency (0x04, 0x06);
            SendProficiency (0x02, 0x05);
            SendProficiency (0x02, 0x05, 0x40);
            SendProficiency (0x04, 0x07);
            break;
        case CLASS_SHAMAN:
            SendProficiency (0x04, 0x02);
            SendProficiency (0x02, 0x00, 0x04);
            SendProficiency (0x02, 0x10, 0x04);
            SendProficiency (0x04, 0x42);
            SendProficiency (0x04, 0x46);
            SendProficiency (0x02, 0x10, 0x44);
            SendProficiency (0x04, 0x47);
            break;
    }

    data.Initialize( SMSG_TUTORIAL_FLAGS );

    for (int i = 0; i < 8; i++)
        data << uint32( GetPlayer()->GetTutorialInt(i) );

    SendPacket(&data);
    sLog.outDebug( "WORLD: Sent tutorial flags." );

    // Proficiency more to come
    switch (GetPlayer()->getClass())
    {
        case CLASS_MAGE:
            SendProficiency (0x02, 0x00, 0x44, 0x08);
            SendProficiency (0x04, 0x03);
            break;
        case CLASS_ROGUE:
            SendProficiency (0x02, 0x00, 0xC0, 0x01);
            SendProficiency (0x04, 0x07);
            break;
        case CLASS_WARRIOR:
            SendProficiency (0x02, 0x91, 0x40);
            SendProficiency (0x04, 0x4F);
            break;
        case CLASS_PALADIN:
            SendProficiency (0x02, 0x30, 0x40);
            SendProficiency (0x04, 0x4F);
            break;
        case CLASS_WARLOCK:
            SendProficiency (0x02, 0x00, 0xC0, 0x08);
            SendProficiency (0x04, 0x03);
            break;
        case CLASS_PRIEST:
            SendProficiency (0x02, 0x10, 0x40, 0x08);
            SendProficiency (0x04, 0x03);
            break;
        case CLASS_DRUID:
            SendProficiency (0x02, 0x00, 0xC4);
            SendProficiency (0x04, 0x07);
            break;
        case CLASS_HUNTER:
            SendProficiency (0x02, 0x05, 0x40);
            SendProficiency (0x04, 0x07);
            break;
        case CLASS_SHAMAN:
            SendProficiency (0x02, 0x10, 0x44);
            SendProficiency (0x04, 0x47);
            break;
    }

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

    GetPlayer()->UpdateHonor();

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

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `guild_member` WHERE `guid` = '%d';",pCurrChar->GetGUID());

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

void WorldSession::HandleSetFactionCheat( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD SESSION: HandleSetFactionCheat");

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