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

#include "WorldSession.h"
#include "WorldPacket.h"
#include "Log.h"
#include "Database/DatabaseEnv.h"
#include "Player.h"
#include "ObjectMgr.h"

void WorldSession::HandleInspectArenaStatsOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDebug("MSG_INSPECT_ARENA_STATS");
    recv_data.hexlike();
    uint64 guid;
    recv_data >> guid;
    sLog.outDebug("Inspect Arena stats " I64FMTD, guid);

    QueryResult *result = sDatabase.PQuery("SELECT `teamslot`, `teamguid`, `rating`, `games`, `wins`, `played` FROM `arena_team_member` WHERE `guid`='%u'", _player->GetGUIDLow());
    if(result)
    {
        Field *fields = result->Fetch();
        do
        {
            WorldPacket data(MSG_INSPECT_ARENA_STATS,8+1+4+4+4+4+4);
            data << guid;
            data << fields[0].GetUInt8();   // slot (0...2)
            data << fields[1].GetUInt32();  // arena team id
            data << fields[2].GetUInt32();  // rating
            data << fields[3].GetUInt32();  // games
            data << fields[4].GetUInt32();  // wins
            data << fields[5].GetUInt32();  // played (count of all games, that played...)
            SendPacket(&data);
        }
        while( result->NextRow() );
        delete result;
    }
    else
    {
        sLog.outDebug("Error...");
    }
}

void WorldSession::HandleArenaTeamQueryOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4);

    sLog.outDebug("CMSG_ARENA_TEAM_QUERY");
    recv_data.hexlike();

    char *teamname = "test_arena_team_name";

    uint32 teamid;
    recv_data >> teamid;
    sLog.outDebug("Arena Team Info %u", teamid);

    // check arena team existence

    WorldPacket data(SMSG_ARENA_TEAM_QUERY_RESPONSE, 4*7+strlen(teamname)+1);
    data << teamid;
    data << teamname;
    data << uint32(0x00000002);     // arena team type (2=2x2, 3=3x3 or 5=5x5) probably
    // emblem/color(RGB?) related things below?
    data << (float)urand(1, 1000);  // emblem style?
    data << urand(1, 100);          // emblem color?
    data << (float)urand(1, 1000);  // border style?
    data << urand(1, 100);          // border color?
    data << (float)urand(1, 1000);  // background color?
    SendPacket(&data);

    uint32 rating = 1, games = 2, wins = 3, played = 4, wins2 = 5, rank = 6; // for testing...
    WorldPacket data2(SMSG_ARENA_TEAM_STATS, 4*7);
    data2 << teamid;
    data2 << rating;
    data2 << games;
    data2 << wins;
    data2 << played;
    data2 << wins2;
    data2 << rank;
    SendPacket(&data2);
}

void WorldSession::HandleArenaTeamRosterOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,1);

    sLog.outDebug("CMSG_ARENA_TEAM_ROSTER");
    recv_data.hexlike();

    uint8 unk; // probably team_size or slot
    recv_data >> unk;

    // opcode structure only guessed
    WorldPacket data(SMSG_ARENA_TEAM_ROSTER, 4*7);
    data << uint8(0);                   // slot
    data << uint32(2);                  // members count
    data << uint32(0);                  // unknown (may be arena team id?)

    for(uint8 i = 0; i < 2; i++)
    {
        data << _player->GetGUID();     // guid
        data << uint8(1);               // online flag
        data << _player->GetName();     // member name
        data << uint8(1);               // unknown
        data << uint32(1);              // unknown
        data << _player->getClass();    // class there...
        data << uint32(20);             // played this week
        data << uint32(11);             // wins this week
        data << uint32(40);             // played this season
        data << uint32(12);             // wins this season
    }
    data.hexlike();
    SendPacket(&data);
}

void WorldSession::HandleArenaTeamAddMemberOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,1+1);

    sLog.outDebug("CMSG_ARENA_TEAM_ADD_MEMBER");
    recv_data.hexlike();

    uint8 team_slot; // slot?
    char *teamname = "test_arena_team_name";
    std::string name;

    recv_data >> team_slot >> name;

    Player *plr = objmgr.GetPlayer(name.c_str());

    if(plr)
    {
        // packet structure only guessed...
        WorldPacket data(SMSG_ARENA_TEAM_INVITE);           // FIX_ME: correct packet size set
        data << _player->GetName();                         // player name (maybe plr instead _player ?)
        data << teamname;                                   // arena team name
        plr->GetSession()->SendPacket(&data);
    }
}

void WorldSession::HandleArenaTeamInviteAcceptOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_ARENA_TEAM_INVITE_ACCEPT");         // empty opcode
}

void WorldSession::HandleArenaTeamInviteDeclineOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_ARENA_TEAM_INVITE_DECLINE");        // empty opcode
}

void WorldSession::HandleArenaTeamLeaveOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_ARENA_TEAM_LEAVE");
    uint8 team_slot; // slot?
    recv_data >> team_slot;
}

void WorldSession::HandleArenaTeamDisbandOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_ARENA_TEAM_DISBAND");
    uint8 team_slot; // slot?
    recv_data >> team_slot;
}
