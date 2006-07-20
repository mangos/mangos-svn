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
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "Auth/BigNumber.h"
#include "Auth/Sha1.h"
#include "UpdateData.h"
#include "LootMgr.h"
#include "Chat.h"
#include "ScriptCalls.h"
#include <zlib/zlib.h>
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Object.h"
#include "BattleGround.h"

void my_esc( char * r, const char * s )
{
    int i, j;
    for ( i = 0, j = 0; s[i]; i++ )
    {
        if ( s[i] == '"' || s[i] == '\\' || s[i] == '\'' ) r[j++] = '\\';
        r[j++] = s[i];
    }
    r[j] = 0;
}

void WorldSession::HandleRepopRequestOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Recvd CMSG_REPOP_REQUEST Message" );

    GetPlayer()->BuildPlayerRepop();
    GetPlayer()->RepopAtGraveyard();
}

extern int num_item_prototypes;

void WorldSession::HandleWhoOpcode( WorldPacket & recv_data )
{
    uint32 clientcount = 0;
    size_t datalen = 8;
    uint32 countcheck = 0;
    WorldPacket data;

    sLog.outDebug( "WORLD: Recvd CMSG_WHO Message" );

    uint32 team = this->_player->GetTeam();
    uint32 security = this->GetSecurity();

    ObjectAccessor::PlayersMapType &m(ObjectAccessor::Instance().GetPlayers());
    for(ObjectAccessor::PlayersMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        // PLAYER see his team only and PLAYER can't see MODERATOR, GAME MASTER, ADMINISTRATOR characters
        // MODERATOR, GAME MASTER, ADMINISTRATOR can see all
        if ( itr->second->GetName() && ( security > 0 || itr->second->GetTeam() == team && itr->second->GetSession()->GetSecurity() == 0 ) )
        {
            clientcount++;

            datalen = datalen + strlen(itr->second->GetName()) + 1 + 17;
        }
    }

    data.Initialize( SMSG_WHO );
    data << uint32( clientcount );
    data << uint32( clientcount );

    for(ObjectAccessor::PlayersMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {

        if( countcheck >= clientcount) break;

        // PLAYER see his team only and PLAYER can't see MODERATOR, GAME MASTER, ADMINISTRATOR characters
        // MODERATOR, GAME MASTER, ADMINISTRATOR can see all
        if ( itr->second->GetName() && ( security > 0 || itr->second->GetTeam() == team && itr->second->GetSession()->GetSecurity() == 0 ) )
        {
            countcheck++;

            data.append(itr->second->GetName() , strlen(itr->second->GetName()) + 1);
            data << uint8( 0x00 );
            data << uint32( itr->second->getLevel() );
            data << uint32( itr->second->getClass() );
            data << uint32( itr->second->getRace() );
            data << uint32( itr->second->GetAreaId() );
        }
    }

    WPAssert(data.size() == datalen);
    SendPacket(&data);
}

void WorldSession::HandleLogoutRequestOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    sLog.outDebug( "WORLD: Recvd CMSG_LOGOUT_REQUEST Message" );

    Player* Target = GetPlayer();
    uint32 MapID = Target->GetMapId();
    Map* Map = MapManager::Instance().GetMap(MapID);
    float posz = Map->GetHeight(Target->GetPositionX(),Target->GetPositionY());
    sLog.outDebug("Current z:%f \tMap Z:%f", Target->GetPositionZ(),posz);

    //Can not logout if...
    if( (GetPlayer()->isInCombat()) ||                      //...is in combat
        (GetPlayer()->isInDuel())   ||                      //...is in Duel
        (Target->GetPositionZ() > posz + 1) )               //...need more judge and handle while jumping and falling
    {
        data.Initialize( SMSG_LOGOUT_RESPONSE );
        data << (uint8)0xC;
        data << uint32(0);
        data << uint8(0);
        SendPacket( &data );
        LogoutRequest(0);
        return;
    }

    Target->SetFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT);

    if(!GetPlayer()->HasFlag(PLAYER_FLAGS, 0x20))
    {                                                       //in city no root no lock rotate
        data.Initialize( SMSG_FORCE_MOVE_ROOT );
        data << (uint8)0xFF << Target->GetGUID() << (uint32)2;
        SendPacket( &data );
        Target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
    }

    data.Initialize( SMSG_LOGOUT_RESPONSE );
    data << uint32(0);
    data << uint8(0);
    SendPacket( &data );
    LogoutRequest(time(NULL));

}

void WorldSession::HandlePlayerLogoutOpcode( WorldPacket & recv_data )
{
    //WorldPacket data;

    sLog.outDebug( "WORLD: Recvd CMSG_PLAYER_LOGOUT Message" );
    if (GetPlayer()->GetSession()->GetSecurity() > 0)
    {
        LogoutRequest(0);
        LogoutPlayer(true);
    }
}

void WorldSession::HandleLogoutCancelOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    sLog.outDebug( "WORLD: Recvd CMSG_LOGOUT_CANCEL Message" );

    LogoutRequest(0);

    data.Initialize( SMSG_LOGOUT_CANCEL_ACK );
    SendPacket( &data );

    //!we can move again
    data.Initialize( SMSG_FORCE_MOVE_UNROOT );
    data << (uint8)0xFF << GetPlayer()->GetGUID();
    SendPacket( &data );

    //! Stand Up
    //! Removes the flag so player stands
    GetPlayer()->RemoveFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT);

    //! DISABLE_ROTATE
    GetPlayer()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);

    sLog.outDebug( "WORLD: sent SMSG_LOGOUT_CANCEL_ACK Message" );
}

void WorldSession::SendGMTicketGetTicket(uint32 status, char const* text)
{
    int len = text ? strlen(text) : 0;
    WorldPacket data;
    data.Initialize( SMSG_GMTICKET_GETTICKET );
    data << uint32(6);
    if(len > 0)
        data.append((uint8 *)text,len+1);
    else
        data << uint32(0);
    SendPacket( &data );
}

void WorldSession::HandleGMTicketGetTicketOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    data.Initialize( SMSG_QUERY_TIME_RESPONSE );
    //    data << (uint32)20;
    data << (uint32)getMSTime();
    SendPacket( &data );

    uint64 guid;
    Field *fields;
    guid = GetPlayer()->GetGUID();

    QueryResult *result = sDatabase.PQuery("SELECT COUNT(*) FROM `character_ticket` WHERE `guid` = '%u';", GUID_LOPART(guid));

    if (result)
    {
        int cnt;
        fields = result->Fetch();
        cnt = fields[0].GetUInt32();

        if ( cnt > 0 )
        {
            QueryResult *result = sDatabase.PQuery("SELECT * FROM `character_ticket` WHERE `guid` = '%u';", GUID_LOPART(guid));
            fields = result->Fetch();

            SendGMTicketGetTicket(6,fields[2].GetString());
            delete result;
        }
        else
            SendGMTicketGetTicket(1,0);
    }
    delete result;
}

void WorldSession::HandleGMTicketUpdateTextOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 guid = GetPlayer()->GetGUIDLow();
    std::string ticketText = "";
    char * p, p1[512];
    uint8 buf[516];
    memcpy( buf, recv_data.contents(), sizeof buf <recv_data.size() ? sizeof buf : recv_data.size() );
    p = (char *)buf + 1;
    my_esc( p1, (const char *)buf + 1 );
    ticketText = p1;
    sDatabase.PExecute("UPDATE `character_ticket` SET `ticket_text` = '%s' WHERE `guid` = '%u';", ticketText.c_str(), guid);

}

void WorldSession::HandleGMTicketDeleteOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 guid = GetPlayer()->GetGUIDLow();

    sDatabase.PExecute("DELETE FROM `character_ticket` WHERE `guid` = '%u' LIMIT 1",guid);

    data.Initialize( SMSG_GMTICKET_DELETETICKET );
    data << uint32(1);
    data << uint32(0);
    SendPacket( &data );

    SendGMTicketGetTicket(1,0);
}

void WorldSession::HandleGMTicketCreateOpcode( WorldPacket & recv_data )
{

    WorldPacket data;
    uint32 guid;
    guid = GetPlayer()->GetGUIDLow();
    std::string ticketText = "";
    Field *fields;
    char * p, p1[512];
    uint8 buf[516];
    uint32   cat[] = { 0,5,1,2,0,6,4,7,0,8,3 };
    memcpy( buf, recv_data.contents(), sizeof buf < recv_data.size() ? sizeof buf : recv_data.size() );
    buf[272] = 0;
    p = (char *)buf + 17;
    my_esc( p1, (const char *)buf + 17 );
    ticketText = p1;

    QueryResult *result = sDatabase.PQuery("SELECT COUNT(*) FROM `character_ticket` WHERE `guid` = '%u';",guid);

    if (result)
    {
        int cnt;
        fields = result->Fetch();
        cnt = fields[0].GetUInt32();

        if ( cnt > 0 )
        {
            data.Initialize( SMSG_GMTICKET_CREATE );
            data << uint32(1);
            SendPacket( &data );
        }
        else
        {

            sDatabase.PExecute("INSERT INTO `character_ticket` (`guid`,`ticket_text`,`ticket_category`) VALUES ('%u', '%s', '%u');", guid, ticketText.c_str(), cat[buf[0]]);

            data.Initialize( SMSG_QUERY_TIME_RESPONSE );
            //data << (uint32)20;
            data << (uint32)getMSTime();
            SendPacket( &data );

            data.Initialize( SMSG_GMTICKET_CREATE );
            data << uint32(2);
            SendPacket( &data );
            DEBUG_LOG("update the ticket\n");

            ObjectAccessor::PlayersMapType &m = ObjectAccessor::Instance().GetPlayers();
            for(ObjectAccessor::PlayersMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
            {
                if(itr->second->GetSession()->GetSecurity() >= 2 && itr->second->isAcceptTickets())
                    ChatHandler::PSendSysMessage(itr->second->GetSession(),"New ticket from %s",GetPlayer()->GetName());
            }
        }
    }

    delete result;
}

void WorldSession::HandleGMTicketSystemStatusOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    data.Initialize( SMSG_GMTICKET_SYSTEMSTATUS );
    data << uint32(1);

    SendPacket( &data );
}

void WorldSession::HandleEnablePvP(WorldPacket& recvPacket)
{

    if ( (!GetPlayer()->isAlive()) || GetPlayer()->isInCombat() || GetPlayer()->isInDuel() )
    {
        WorldPacket data;
        data.Initialize(SMSG_CAST_RESULT);
        data << uint32(0);
        data << uint8(2);
        data << uint8(97);
        SendPacket(&data);
        return;
    }

    if( GetPlayer()->GetPvP() )
    {
        sChatHandler.SendSysMessage(GetPlayer()->GetSession(), "You will be unflagged for PvP combat after five minutes of non-PvP action in friendly territory.");
    }
    else
    {
        sChatHandler.SendSysMessage(GetPlayer()->GetSession(), "You are now flagged PvP combat and will remain so until toggled off.");
    }
    GetPlayer()->SetPVPCount(time(NULL));

}

void WorldSession::HandleZoneUpdateOpcode( WorldPacket & recv_data )
{
    uint32 newZone;
    WPAssert(GetPlayer());

    // if player is resting stop resting
    GetPlayer()->RemoveFlag(PLAYER_FLAGS, 0x20);

    recv_data >> newZone;
    sLog.outDetail("WORLD: Recvd ZONE_UPDATE: %u", newZone);

}

void WorldSession::HandleSetTargetOpcode( WorldPacket & recv_data )
{
    uint64 guid ;
    recv_data >> guid;

    if( _player != 0 )
    {
        _player->SetTarget(guid);
    }
}

void WorldSession::HandleSetSelectionOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    recv_data >> guid;

    if( _player != 0 )
        _player->SetSelection(guid);

    if(_player->GetUInt64Value(PLAYER_FIELD_COMBO_TARGET) != guid)
    {
        _player->SetUInt64Value(PLAYER_FIELD_COMBO_TARGET,0);
        _player->SetUInt32Value(PLAYER_FIELD_BYTES,((_player->GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (0x00 << 8)));
    }
}

void WorldSession::HandleStandStateChangeOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_STAND_STATE_CHANGE"  );
    if( GetPlayer() != 0 )
    {
        uint8 animstate;
        recv_data >> animstate;

        uint32 bytes1 = _player->GetUInt32Value( UNIT_FIELD_BYTES_1 );
        bytes1 &=0xFFFFFF00;
        bytes1 |=animstate;
        _player->SetUInt32Value(UNIT_FIELD_BYTES_1 , bytes1);
    }
}

void WorldSession::HandleFriendListOpcode( WorldPacket & recv_data )
{
    WorldPacket data, dataI;

    sLog.outDebug( "WORLD: Received CMSG_FRIEND_LIST"  );

    unsigned char Counter=0, nrignore=0;
    int i=0;
    uint32 guid;
    Field *fields;
    Player* pObj;
    FriendStr friendstr[255];

    guid=GetPlayer()->GetGUIDLow();

    QueryResult *result = sDatabase.PQuery("SELECT COUNT(*) FROM `character_social` WHERE `flags` = 'FRIEND' AND `guid` = '%u';",guid);

    if(result)
    {
        fields = result->Fetch();
        Counter=fields[0].GetUInt32();
        delete result;

        result = sDatabase.PQuery("SELECT * FROM `character_social` WHERE `flags` = 'FRIEND' AND `guid` = '%u';",guid);
        if(result)
        {
            fields = result->Fetch();
            friendstr[i].PlayerGUID = fields[2].GetUInt64();
            pObj = ObjectAccessor::Instance().FindPlayer( friendstr[i].PlayerGUID );

            if(pObj && pObj->IsInWorld())
            {
                friendstr[i].Status = 1;
                friendstr[i].Area = pObj->GetZoneId();
                friendstr[i].Level = pObj->getLevel();
                friendstr[i].Class = pObj->getClass();
                i++;
            }
            else
            {
                friendstr[i].Status = 0;
                friendstr[i].Area = 0;
                friendstr[i].Level = 0;
                friendstr[i].Class = 0;
                i++;
            }

            while( result->NextRow() )
            {
                friendstr[i].PlayerGUID = fields[2].GetUInt64();
                pObj = ObjectAccessor::Instance().FindPlayer(friendstr[i].PlayerGUID);
                if(pObj)
                {
                    friendstr[i].Status = 1;
                    friendstr[i].Area = pObj->GetZoneId();
                    friendstr[Counter].Level = pObj->getLevel();
                    friendstr[Counter].Class = pObj->getClass();
                    i++;
                }
                else
                {
                    friendstr[i].Status = 0;
                    friendstr[i].Area = 0;
                    friendstr[Counter].Level = 0;
                    friendstr[Counter].Class = 0;
                    i++;
                }
            }
            delete result;
        }
    }

    data.Initialize( SMSG_FRIEND_LIST );
    data << Counter;

    for (int j=0; j<Counter; j++)
    {

        sLog.outDetail( "WORLD: Adding Friend - Guid:%lu, Status:%u, Area:%u, Level:%u Class:%u",friendstr[j].PlayerGUID, friendstr[j].Status, friendstr[j].Area,friendstr[j].Level,friendstr[j].Class  );

        data << friendstr[j].PlayerGUID << friendstr[j].Status ;
        if (friendstr[j].Status != 0)
            data << friendstr[j].Area << friendstr[j].Level << friendstr[j].Class;
    }

    SendPacket( &data );
    sLog.outDebug( "WORLD: Sent (SMSG_FRIEND_LIST)" );

    result = sDatabase.PQuery("SELECT COUNT(*) FROM `character_social` WHERE `flags` = 'IGNORE' AND `guid` = '%u';", guid);

    if(!result) return;

    fields = result->Fetch();
    nrignore=fields[0].GetUInt32();
    delete result;

    dataI.Initialize( SMSG_IGNORE_LIST );
    dataI << nrignore;

    result = sDatabase.PQuery("SELECT * FROM `character_social` WHERE `flags` = 'IGNORE' AND `guid` = '%u';", guid);

    if(!result) return;

    do
    {

        fields = result->Fetch();
        dataI << fields[2].GetUInt64();

    }while( result->NextRow() );
    delete result;

    SendPacket( &dataI );
    sLog.outDebug( "WORLD: Sent (SMSG_IGNORE_LIST)" );

}

void WorldSession::HandleAddFriendOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_ADD_FRIEND"  );

    std::string friendName = "UNKNOWN";
    unsigned char friendResult = FRIEND_NOT_FOUND;
    Player *pfriend=NULL;
    uint64 friendGuid = 0;
    uint32 friendArea = 0, friendLevel = 0, friendClass = 0;
    WorldPacket data;

    recv_data >> friendName;

    sLog.outDetail( "WORLD: %s asked to add friend : '%s'",
        GetPlayer()->GetName(), friendName.c_str() );

    friendGuid = objmgr.GetPlayerGUIDByName(friendName.c_str());
    pfriend = ObjectAccessor::Instance().FindPlayer(friendGuid);

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `character_social` WHERE `flags` = 'FRIEND' AND `friend` = '%u';", friendGuid);

    if( result )
        friendResult = FRIEND_ALREADY;

    if (!strcmp(GetPlayer()->GetName(),friendName.c_str())) friendResult = FRIEND_SELF;

    data.Initialize( SMSG_FRIEND_STATUS );

    if (friendGuid > 0 && friendResult!=FRIEND_ALREADY && friendResult!=FRIEND_SELF)
    {
        if( pfriend != NULL && pfriend->IsInWorld())
        {
            friendResult = FRIEND_ADDED_ONLINE;
            friendArea = pfriend->GetZoneId();
            friendLevel = pfriend->getLevel();
            friendClass = pfriend->getClass();

            data << (uint8)friendResult << (uint64)friendGuid << (uint8)0;
            data << (uint32)friendArea << (uint32)friendLevel << (uint32)friendClass;

            delete result;

            uint32 guid;
            guid=GetPlayer()->GetGUIDLow();

            result = sDatabase.PQuery("INSERT INTO `character_social` (`guid`,`name`,`friend`,`flags`) VALUES ('%u', '%s', '%u', 'FRIEND');", (uint32)guid, friendName.c_str(), (uint32)friendGuid);

            delete result;
        }
        else
            friendResult = FRIEND_ADDED_OFFLINE;

        sLog.outDetail( "WORLD: %s Guid found '%u' area:%u Level:%u Class:%u. ",
            friendName.c_str(), friendGuid, friendArea, friendLevel, friendClass);

    }
    else if(friendResult==FRIEND_ALREADY)
    {
        data << (uint8)friendResult << (uint64)friendGuid;
        sLog.outDetail( "WORLD: %s Guid Already a Friend. ", friendName.c_str() );
    }
    else if(friendResult==FRIEND_SELF)
    {
        data << (uint8)friendResult << (uint64)friendGuid;
        sLog.outDetail( "WORLD: %s Guid can't add himself. ", friendName.c_str() );
    }
    else
    {
        data << (uint8)friendResult << (uint64)friendGuid;
        sLog.outDetail( "WORLD: %s Guid not found. ", friendName.c_str() );
    }

    SendPacket( &data );

    sLog.outDebug( "WORLD: Sent (SMSG_FRIEND_STATUS)" );
}

void WorldSession::HandleDelFriendOpcode( WorldPacket & recv_data )
{
    uint64 FriendGUID;
    WorldPacket data;

    sLog.outDebug( "WORLD: Received CMSG_DEL_FRIEND"  );
    recv_data >> FriendGUID;

    uint8 FriendResult = FRIEND_REMOVED;

    data.Initialize( SMSG_FRIEND_STATUS );

    data << (uint8)FriendResult << (uint64)FriendGUID;

    uint32 guid = GetPlayer()->GetGUIDLow();

    sDatabase.PExecute("DELETE FROM `character_social` WHERE `flags` = 'FRIEND' AND `guid` = '%u' AND `friend` = '%u'",(uint32)guid, GUID_LOPART(FriendGUID));

    SendPacket( &data );

    sLog.outDebug( "WORLD: Sent motd (SMSG_FRIEND_STATUS)" );
}

void WorldSession::HandleAddIgnoreOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_ADD_IGNORE"  );

    std::string IgnoreName = "UNKNOWN";
    unsigned char ignoreResult = FRIEND_IGNORE_NOT_FOUND;
    Player *pIgnore=NULL;
    uint64 IgnoreGuid = 0;

    WorldPacket data;

    recv_data >> IgnoreName;

    sLog.outDetail( "WORLD: %s asked to Ignore: '%s'",
        GetPlayer()->GetName(), IgnoreName.c_str() );

    IgnoreGuid = objmgr.GetPlayerGUIDByName(IgnoreName.c_str());
    pIgnore = ObjectAccessor::Instance().FindPlayer((uint64)IgnoreGuid);

    QueryResult *result = sDatabase.PQuery("SELECT `guid`,`name`,`friend`,`flags` FROM `character_social` WHERE `flags` = 'IGNORE' AND `friend` = '%u';", (uint32)IgnoreGuid);

    if( result )
        ignoreResult = FRIEND_IGNORE_ALREADY;
    delete result;

    if (!strcmp(GetPlayer()->GetName(),IgnoreName.c_str())) ignoreResult = FRIEND_IGNORE_SELF;

    data.Initialize( SMSG_FRIEND_STATUS );

    if (pIgnore && ignoreResult!=FRIEND_IGNORE_ALREADY && ignoreResult!=FRIEND_IGNORE_SELF)
    {
        ignoreResult = FRIEND_IGNORE_ADDED;

        uint32 guid;
        guid=GetPlayer()->GetGUIDLow();

        data << (uint8)ignoreResult << (uint64)IgnoreGuid;

        sDatabase.PExecute("INSERT INTO `character_social` (`guid`,`name`,`friend`,`flags`) VALUES ('%u', '%s', '%u', 'IGNORE');", guid, IgnoreName.c_str(), IgnoreGuid);

    }
    else if(ignoreResult==FRIEND_IGNORE_ALREADY)
    {
        data << (uint8)ignoreResult << (uint64)IgnoreGuid;
        sLog.outDetail( "WORLD: %s Guid Already Ignored. ", IgnoreName.c_str() );
    }
    else if(ignoreResult==FRIEND_IGNORE_SELF)
    {
        data << (uint8)ignoreResult << (uint64)IgnoreGuid;
        sLog.outDetail( "WORLD: %s Guid can't add himself. ", IgnoreName.c_str() );
    }
    else
    {
        data << (uint8)ignoreResult << (uint64)IgnoreGuid;
        sLog.outDetail( "WORLD: %s Guid not found. ", IgnoreName.c_str() );
    }

    SendPacket( &data );
    delete result;
    sLog.outDebug( "WORLD: Sent (SMSG_FRIEND_STATUS)" );
}

void WorldSession::HandleDelIgnoreOpcode( WorldPacket & recv_data )
{
    uint64 IgnoreGUID;
    WorldPacket data;

    sLog.outDebug( "WORLD: Received CMSG_DEL_IGNORE"  );
    recv_data >> IgnoreGUID;

    unsigned char IgnoreResult = FRIEND_IGNORE_REMOVED;

    data.Initialize( SMSG_FRIEND_STATUS );

    data << (uint8)IgnoreResult << (uint64)IgnoreGUID;

    uint32 guid;
    guid=GetPlayer()->GetGUIDLow();

    sDatabase.PExecute("DELETE FROM `character_social` WHERE `flags` = 'IGNORE' AND `guid` = '%u' AND `friend` = '%u'",(uint32)guid, GUID_LOPART(IgnoreGUID));

    SendPacket( &data );

    sLog.outDebug( "WORLD: Sent motd (SMSG_FRIEND_STATUS)" );

}

void WorldSession::HandleBugOpcode( WorldPacket & recv_data )
{
    uint32 suggestion, contentlen;
    std::string content;
    uint32 typelen;
    std::string type;

    recv_data >> suggestion >> contentlen >> content >> typelen >> type;

    if( suggestion == 0 )
        sLog.outDebug( "WORLD: Received CMSG_BUG [Bug Report]" );
    else
        sLog.outDebug( "WORLD: Received CMSG_BUG [Suggestion]" );

    sLog.outDebug( type.c_str( ) );
    sLog.outDebug( content.c_str( ) );

    sDatabase.PExecute ("INSERT INTO `bugreport` (`type`,`content`) VALUES('%s', '%s');", type.c_str( ), content.c_str( ));

}

void WorldSession::HandleCorpseReclaimOpcode(WorldPacket &recv_data)
{
    sLog.outDetail("WORLD: Received CMSG_RECLAIM_CORPSE");
    if (GetPlayer()->isAlive())
        return;
    if (!GetPlayer()->m_pCorpse)
        return;
    sLog.outDebug("Corpse Dis: \t%f",GetPlayer()->m_pCorpse->GetDistanceSq(((Object *)GetPlayer())));
    if (GetPlayer()->m_pCorpse->GetDistanceSq(((Object *)GetPlayer()))>1470)
        return;

    //need a accurate value of Max distance
    //need to judge Resurrect Time

    uint64 guid;
    recv_data >> guid;

    // resurrect
    GetPlayer()->ResurrectPlayer();

    // spawnbones
    GetPlayer()->SpawnCorpseBones();

    // set health, mana
    GetPlayer()->ApplyStats(false);
    GetPlayer()->SetUInt32Value(UNIT_FIELD_HEALTH,(uint32)(GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*0.50) );
    GetPlayer()->SetUInt32Value(UNIT_FIELD_POWER1,(uint32)(GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXPOWER1)*0.50) );
    GetPlayer()->ApplyStats(true);

    // update world right away
    MapManager::Instance().GetMap(GetPlayer()->GetMapId())->Add(GetPlayer());
}

void WorldSession::HandleResurrectResponseOpcode(WorldPacket & recv_data)
{
    sLog.outDetail("WORLD: Received CMSG_RESURRECT_RESPONSE");

    if(GetPlayer()->isAlive())
        return;

    WorldPacket data;
    uint64 guid;
    uint8 status;
    recv_data >> guid;
    recv_data >> status;

    if(status == 0)
        return;

    if(GetPlayer()->m_resurrectGUID == 0)
        return;

    GetPlayer()->ResurrectPlayer();

    if(GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXHEALTH) > GetPlayer()->m_resurrectHealth)
        GetPlayer()->SetUInt32Value(UNIT_FIELD_HEALTH, GetPlayer()->m_resurrectHealth );
    else
        GetPlayer()->SetUInt32Value(UNIT_FIELD_HEALTH, GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXHEALTH) );

    if(GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXPOWER1) > GetPlayer()->m_resurrectMana)
        GetPlayer()->SetUInt32Value(UNIT_FIELD_POWER1, GetPlayer()->m_resurrectMana );
    else
        GetPlayer()->SetUInt32Value(UNIT_FIELD_POWER1, GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXPOWER1) );

    if(GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXPOWER2) > GetPlayer()->m_resurrectMana)
        GetPlayer()->SetUInt32Value(UNIT_FIELD_POWER2, GetPlayer()->m_resurrectMana );
    else
        GetPlayer()->SetUInt32Value(UNIT_FIELD_POWER2, GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXPOWER2) );

    GetPlayer()->SetUInt32Value(UNIT_FIELD_POWER3, 0 );

    if(GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXPOWER4) > GetPlayer()->m_resurrectMana)
        GetPlayer()->SetUInt32Value(UNIT_FIELD_POWER4, GetPlayer()->m_resurrectMana );
    else
        GetPlayer()->SetUInt32Value(UNIT_FIELD_POWER4, GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXPOWER4) );

    if(GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXPOWER5) > GetPlayer()->m_resurrectMana)
        GetPlayer()->SetUInt32Value(UNIT_FIELD_POWER5, GetPlayer()->m_resurrectMana );
    else
        GetPlayer()->SetUInt32Value(UNIT_FIELD_POWER5, GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXPOWER5) );

    GetPlayer()->SpawnCorpseBones();

    GetPlayer()->BuildTeleportAckMsg(&data, GetPlayer()->m_resurrectX, GetPlayer()->m_resurrectY, GetPlayer()->m_resurrectZ, GetPlayer()->GetOrientation());
    GetPlayer()->GetSession()->SendPacket(&data);

    GetPlayer()->SetPosition(GetPlayer()->m_resurrectX ,GetPlayer()->m_resurrectY ,GetPlayer()->m_resurrectZ,GetPlayer()->GetOrientation());

    GetPlayer()->m_resurrectGUID = 0;
    GetPlayer()->m_resurrectHealth = GetPlayer()->m_resurrectMana = 0;
    GetPlayer()->m_resurrectX = GetPlayer()->m_resurrectY = GetPlayer()->m_resurrectZ = 0;
}

void WorldSession::HandleAreaTriggerOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("WORLD: Received CMSG_AREATRIGGER");

    uint32 Trigger_ID;
    WorldPacket data;

    recv_data >> Trigger_ID;
    sLog.outDebug("Trigger ID:%u",Trigger_ID);

    if(GetPlayer()->isInFlight())
    {
        sLog.outDebug("Player '%s' in flight, ignore Area Trigger ID:%u",GetPlayer()->GetName(),Trigger_ID);
        return;
    }

    AreaTriggerPoint *pArea = objmgr.GetAreaTriggerQuestPoint( Trigger_ID );
    if( pArea )
    {
        Quest *pQuest = objmgr.GetQuest( pArea->Quest_ID );
        if( pQuest )
        {
            if( !Script->scriptAreaTrigger( GetPlayer(), pQuest, Trigger_ID ) )
            {
                if(GetPlayer()->GetQuestStatus(pQuest) == QUEST_STATUS_INCOMPLETE)
                    GetPlayer()->AreaExplored( pQuest );
            }
        }
    }

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `areatrigger_tavern` WHERE `id` = '%u';", Trigger_ID);
    if(!result)
        result = sDatabase.PQuery("SELECT * FROM `areatrigger_city` WHERE `id` = '%u';", Trigger_ID);
    if(result)
    {
        // player flag 0x20 - resting
        if(!GetPlayer()->HasFlag(PLAYER_FLAGS, 0x20))
            GetPlayer()->SetFlag(PLAYER_FLAGS, 0x20);
    }
    else if(AreaTrigger * at = objmgr.GetAreaTrigger(Trigger_ID))
    {
        if(at->mapId == GetPlayer()->GetMapId() && !GetPlayer()->m_bgInBattleGround )
        {
            WorldPacket movedata;
            _player->BuildTeleportAckMsg(&movedata, at->X, at->Y, at->Z, at->Orientation );
            _player->SendMessageToSet(&movedata,true);
            //_player->SetPosition( at->X, at->Y, at->Z, _player->GetOrientation());
            //_player->BuildHeartBeatMsg(&data);
            //_player->SendMessageToSet(&data, true);
        }
        else if (GetPlayer()->m_bgInBattleGround)           //if player is playing in a BattleGround
        {
            //! AreaTrigger BattleGround

            //Get current BattleGround the player is in
            BattleGround* TempBattlegrounds = sBattleGroundMgr.GetBattleGround(GetPlayer()->GetBattleGroundId());
            //Run the areatrigger code
            TempBattlegrounds->HandleAreaTrigger(GetPlayer(),Trigger_ID);
        }
        else
        {
            GetPlayer()->TeleportTo(at->mapId,at->X,at->Y,at->Z,GetPlayer()->GetOrientation());
        }
        delete at;
    }
    delete result;

    // set resting flag we are in the inn

}

void WorldSession::HandleUpdateAccountData(WorldPacket &recv_data)
{
}

void WorldSession::HandleRequestAccountData(WorldPacket& recv_data)
{
    sLog.outDetail("WORLD: Received CMSG_REQUEST_ACCOUNT_DATA");
}

void WorldSession::HandleSetActionButtonOpcode(WorldPacket& recv_data)
{
    sLog.outString( "WORLD: Received CMSG_SET_ACTION_BUTTON" );
    uint8 button, misc, type;
    uint16 action;
    recv_data >> button >> action >> type >> misc;
    sLog.outString( "BUTTON: %u ACTION: %u TYPE: %u MISC: %u", button, action, type, misc );
    if(action==0)
    {
        sLog.outString( "MISC: Remove action from button %u", button );

        GetPlayer()->removeAction(button);
    }
    else
    {
        if(type==64)
        {
            sLog.outString( "MISC: Added Macro %u into button %u", action, button );
            GetPlayer()->addAction(button,action,type,misc);
        }
        else if(type==0)
        {
            sLog.outString( "MISC: Added Action %u into button %u", action, button );
            GetPlayer()->addAction(button,action,type,misc);
        }
        else if(type==128)
        {
            sLog.outString( "MISC: Added Item %u into button %u", action, button );
            GetPlayer()->addAction(button,action,type,misc);
        }
    }
}

void WorldSession::HandleCompleteCinema( WorldPacket & recv_data )
{
    DEBUG_LOG( "WORLD: Player is watching cinema" );
}

void WorldSession::HandleNextCinematicCamera( WorldPacket & recv_data )
{
    DEBUG_LOG( "WORLD: Which movie to play" );
}

void WorldSession::HandleBattlefieldStatusOpcode( WorldPacket & recv_data )
{

    DEBUG_LOG( "WORLD: Battleground status - not yet" );
}

void WorldSession::HandleMoveTimeSkippedOpcode( WorldPacket & recv_data )
{

    DEBUG_LOG( "WORLD: Move time lag/synchronization fix - not yet" );
}

void WorldSession::HandleMooveUnRootAck(WorldPacket& recv_data)
{

    sLog.outDebug( "WORLD: CMSG_FORCE_MOVE_UNROOT_ACK" );
    WorldPacket data;
    uint64 guid;
    uint64 unknown1;
    uint32 unknown2;
    float PositionX;
    float PositionY;
    float PositionZ;
    float Orientation;

    recv_data >> guid;
    recv_data >> unknown1;
    recv_data >> unknown2;
    recv_data >> PositionX;
    recv_data >> PositionY;
    recv_data >> PositionZ;
    recv_data >> Orientation;

    DEBUG_LOG("Guid %lu",guid);
    DEBUG_LOG("unknown1 %lu",unknown1);
    DEBUG_LOG("unknown2 %lu",unknown2);
    DEBUG_LOG("X %f",PositionX);
    DEBUG_LOG("Y %f",PositionY);
    DEBUG_LOG("Z %f",PositionZ);
    DEBUG_LOG("O %f",Orientation);
}

void WorldSession::HandleLookingForGroup(WorldPacket& recv_data)
{
    // TODO send groups need data
    sLog.outDebug( "WORLD: MSG_LOOKING_FOR_GROUP" );
}

void WorldSession::HandleMooveRootAck(WorldPacket& recv_data)
{

    sLog.outDebug( "WORLD: CMSG_FORCE_MOVE_ROOT_ACK" );
    WorldPacket data;
    uint64 guid;
    uint64 unknown1;
    uint32 unknown2;
    float PositionX;
    float PositionY;
    float PositionZ;
    float Orientation;

    recv_data >> guid;
    recv_data >> unknown1;
    recv_data >> unknown2;
    recv_data >> PositionX;
    recv_data >> PositionY;
    recv_data >> PositionZ;
    recv_data >> Orientation;

    DEBUG_LOG("Guid %lu",guid);
    DEBUG_LOG("unknown1 %lu",unknown1);
    DEBUG_LOG("unknown1 %lu",unknown2);
    DEBUG_LOG("X %f",PositionX);
    DEBUG_LOG("Y %f",PositionY);
    DEBUG_LOG("Z %f",PositionZ);
    DEBUG_LOG("O %f",Orientation);
}

void WorldSession::HandleMoveTeleportAck(WorldPacket& recv_data)
{

    WorldPacket data;
    uint64 guid;
    uint32 value1;

    recv_data >> guid;
    recv_data >> value1;                                    // ms time ?
    DEBUG_LOG("Guid %lu",guid);
    DEBUG_LOG("Value 1 %u",value1);
}

void WorldSession::HandleForceRunSpeedChangeAck(WorldPacket& recv_data)
{
    //check for speed stuff
    uint64 GUID;
    uint32 Flags,unk1, unk0;
    uint32 d_time;
    float X,Y,Z,O;
    float NewSpeed;

    recv_data >> GUID;
    recv_data >> unk0 >> Flags;
    if (Flags & 0x2000)                                     //double check, old value and new calue
    {
        // Crash server. Wrong package structure used???
        // Crash at "recv_data >> unk1 >> OldSpeed >> NewSpeed" and have recv_data._wpos = 60
        return;

        uint32 unk2, unk3, unk4, unk5;
        float OldSpeed;

        recv_data >> d_time;
        recv_data >> X >> Y >> Z >> O;
        recv_data >> unk2 >> unk3;                          //no idea, maybe unk2 = flags2
        recv_data >> unk4 >> unk5;                          //no idea
        recv_data >> unk1 >> OldSpeed >> NewSpeed;
    }
    else                                                    //single check
    {
        recv_data >> d_time;
        recv_data >> X >> Y >> Z >> O;
        recv_data >> unk1 >> NewSpeed;
    }
    if (GetPlayer()->GetSpeed(MOVE_RUN) != NewSpeed)
                                                            //now kick player???
        sLog.outError("SpeedChange player is NOT correct, its set to: %f", NewSpeed);
}

void WorldSession::HandleForceSwimSpeedChangeAck(WorldPacket& recv_data)
{
    // set swim speed ? received data is more
    sLog.outDebug("CMSG_FORCE_SWIM_SPEED_CHANGE_ACK");

}

void WorldSession::HandleSetActionBar(WorldPacket& recv_data)
{
    uint8 ActionBar;
    uint32 temp;

    recv_data >> ActionBar;

    temp = ((GetPlayer()->GetUInt32Value( PLAYER_FIELD_BYTES )) & 0xFFF0FFFF) + (ActionBar << 16);
    GetPlayer()->SetUInt32Value( PLAYER_FIELD_BYTES, temp);
}

void WorldSession::HandleMoveWaterWalkAck(WorldPacket& recv_data)
{
    // TODO
    // we receive guid,x,y,z
}

void WorldSession::HandleChangePlayerNameOpcode(WorldPacket& recv_data)
{
    // TODO
    // need to be written
}

void WorldSession::HandleWardenDataOpcode(WorldPacket& recv_data)
{
    uint8 tmp;
    recv_data >> tmp;
    sLog.outDebug("Received opcode CMSG_WARDEN_DATA, not resolve.uint8 = %u",tmp);
}
