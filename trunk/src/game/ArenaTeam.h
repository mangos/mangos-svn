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

#ifndef MANGOSSERVER_ARENATEAM_H
#define MANGOSSERVER_ARENATEAM_H

enum ArenaTeamCommandTypes
{
    ERR_ARENA_TEAM_CREATE_S                 = 0x00,
    ERR_ARENA_TEAM_INVITE_SS                = 0x01,
    //ERR_ARENA_TEAM_QUIT_S                   = 0x02,
    ERR_ARENA_TEAM_QUIT_S                   = 0x03,
    ERR_ARENA_TEAM_FOUNDER_S                = 0x0C  // need check, probably wrong...
};

enum ArenaTeamCommandErrors
{
    //ARENA_TEAM_PLAYER_NO_MORE_IN_ARENA_TEAM = 0x00,
    ERR_ARENA_TEAM_INTERNAL                 = 0x01,
    ERR_ALREADY_IN_ARENA_TEAM               = 0x02,
    ERR_ALREADY_IN_ARENA_TEAM_S             = 0x03,
    ERR_INVITED_TO_ARENA_TEAM               = 0x04,
    ERR_ALREADY_INVITED_TO_ARENA_TEAM_S     = 0x05,
    ERR_ARENA_TEAM_NAME_INVALID             = 0x06,
    ERR_ARENA_TEAM_NAME_EXISTS_S            = 0x07,
    ERR_ARENA_TEAM_LEADER_LEAVE_S           = 0x08,
    ERR_ARENA_TEAM_PERMISSIONS              = 0x08,
    ERR_ARENA_TEAM_PLAYER_NOT_IN_TEAM       = 0x09,
    ERR_ARENA_TEAM_PLAYER_NOT_IN_TEAM_SS    = 0x0A,
    ERR_ARENA_TEAM_PLAYER_NOT_FOUND_S       = 0x0B,
    ERR_ARENA_TEAM_NOT_ALLIED               = 0x0C
};

enum ArenaTeamEvents
{
    ERR_ARENA_TEAM_JOIN_SS                  = 3,    // player name + arena team name
    ERR_ARENA_TEAM_LEAVE_SS                 = 4,    // player name + arena team name
    ERR_ARENA_TEAM_REMOVE_SSS               = 5,    // player name + arena team name + captain name
    ERR_ARENA_TEAM_LEADER_IS_SS             = 6,    // player name + arena team name
    ERR_ARENA_TEAM_LEADER_CHANGED_SSS       = 7,    // old captain + new captain + arena team name
    ERR_ARENA_TEAM_DISBANDED_S              = 8     // captain name + arena team name
};

/*
need info how to send these ones:
ERR_ARENA_TEAM_YOU_JOIN_S - client show it automatically when accept invite
ERR_ARENA_TEAM_TARGET_TOO_LOW_S
ERR_ARENA_TEAM_TOO_MANY_MEMBERS_S
ERR_ARENA_TEAM_LEVEL_TOO_LOW_I
*/

enum ArenaTeamStatTypes
{
    STAT_TYPE_RATING    = 0,
    STAT_TYPE_GAMES     = 1,
    STAT_TYPE_WINS      = 2,
    STAT_TYPE_PLAYED    = 3,
    STAT_TYPE_WINS2     = 4,
    STAT_TYPE_RANK      = 5
};

enum ArenaTeamTypes
{
    ARENA_TEAM_2v2      = 2,
    ARENA_TEAM_3v3      = 3,
    ARENA_TEAM_5v5      = 5
};

struct ArenaTeamMember
{
    uint64 guid;
    std::string name;
    //uint32 unk2;
    //uint8 unk1;
    uint8 Class;
    uint32 played_week;
    uint32 wons_week;
    uint32 played_season;
    uint32 wons_season;
};

struct ArenaTeamStats
{
    uint32 rating;
    uint32 games;
    uint32 wins;
    uint32 played;
    uint32 wins2;
    uint32 rank;
};

class ArenaTeam
{
    public:
        ArenaTeam();
        ~ArenaTeam();

        bool create(uint64 CaptainGuid, uint32 type, std::string ArenaTeamName);
        void Disband(WorldSession *session);

        typedef std::list<ArenaTeamMember> MemberList;

        uint32 GetId() { return Id; }
        uint32 GetType() { return Type; }
        uint8 GetSlot();
        const uint64& GetCaptain() { return CaptainGuid; }
        std::string GetName() { return Name; }
        ArenaTeamStats GetStats() { return stats; }
        void SetStats(uint32 stat_type, uint32 value);

        uint32 GetEmblemStyle(){ return EmblemStyle; }
        uint32 GetEmblemColor(){ return EmblemColor; }
        uint32 GetBorderStyle(){ return BorderStyle; }
        uint32 GetBorderColor(){ return BorderColor; }
        uint32 GetBackgroundColor(){ return BackgroundColor; }

        void SetCaptain(uint64 guid);
        bool AddMember(uint64 PlayerGuid);
        void DelMember(uint64 guid);

        void SetEmblem(uint32 emblemStyle, uint32 emblemColor, uint32 borderStyle, uint32 borderColor, uint32 backgroundColor);

        uint32 GetMembersSize(){ return members.size(); }
        MemberList::iterator membersbegin(){ return members.begin(); }
        MemberList::iterator membersEnd(){ return members.end(); }

        bool LoadArenaTeamFromDB(uint32 ArenaTeamId);
        void LoadMembersFromDB(uint32 ArenaTeamId);
        void LoadStatsFromDB(uint32 ArenaTeamId);
        void LoadPlayerStats(ArenaTeamMember* member);

        void BroadcastPacket(WorldPacket *packet);

        void Roster(WorldSession *session);
        void Query(WorldSession *session);
        void Stats(WorldSession *session);
        void InspectStats(WorldSession *session);

    protected:

        uint32 Id;
        uint32 Type;
        std::string Name;
        uint64 CaptainGuid;

        uint32 EmblemStyle;
        uint32 EmblemColor;
        uint32 BorderStyle;
        uint32 BorderColor;
        uint32 BackgroundColor;

        MemberList members;
        ArenaTeamStats stats;
};
#endif
