/* Guild.h
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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
#ifndef MANGOSSERVER_GUILD_H
#define MANGOSSERVER_GUILD_H

#define MAXRANKS 10

enum GuildDefaultRanks
{	
	GUILD_RANK_GUILDMASTER  = 0,
	GUILD_RANK_OFFICER      = 1,
	GUILD_RANK_VETERAN      = 2,
	GUILD_RANK_MEMBER       = 3,
	GUILD_RANK_INITIATE     = 4,
};

enum typecommand
{
	GUILD_CREATE_S	= 0x00,
	GUILD_INVITE_S	= 0x01,
	GUILD_QUIT_S		= 0x02,
	GUILD_FOUNDER_S = 0x0C,
};

enum CommandErrors
{
	GUILD_INTERNAL              = 0x00,
	GUILD_ALREADY_IN_GUILD      = 0x01,
	ALREADY_IN_GUILD            = 0x02, // (Includes the string parameter)
	INVITED_TO_GUILD            = 0x03,
	ALREADY_INVITED_TO_GUILD    = 0x04, // (includes string)
	GUILD_NAME_INVALID          = 0x05,
	GUILD_NAME_EXISTS           = 0x06, // (includes string)
	GUILD_LEADER_LEAVE          = 0x07, // use this if typecommand == GUILD_QUIT_S
	GUILD_PERMISSIONS		    		= 0x07, // use this if typecommand != GUILD_QUIT_S
	GUILD_PLAYER_NOT_IN_GUILD   = 0x08, 
	GUILD_PLAYER_NOT_IN_GUILD_S = 0x09, // (includes string)
	GUILD_PLAYER_NOT_FOUND      = 0x0A, // (includes string)
	GUILD_NOT_ALLIED            = 0x0B,
};

enum GuildEvents
{
	GUILD_EVENT_PROMOTION       = 0,
	GUILD_EVENT_DEMOTION        = 1,
	GUILD_EVENT_MOTD            = 2,
	GUILD_EVENT_JOINED          = 3,
	GUILD_EVENT_LEFT            = 4,
	GUILD_EVENT_REMOVED         = 5,
	GUILD_EVENT_LEADER_IS       = 6,
	GUILD_EVENT_LEADER_CHANGED  = 7,
	GUILD_EVENT_DISBANDED       = 8,
	GUILD_EVENT_TABARDCHANGE    = 9,
};

struct MemberSlot
{
  uint64 guid;
  uint32 RankId;
};

struct RankInfo
{
	std::string name;
	uint8 GuildchatListen;
	uint8 GuildchatSpeak;
	uint8 OfficerchatListen;
	uint8 OfficerchatSpeak;
	uint8 Promote;
	uint8 Demote;
	uint8 InviteMember;
	uint8 RemovePlayer;
	uint8 SetMotd;
	uint8 EditPublicNote;
	uint8 ViewOfficerNote;
	uint8 EditOfficerNote;
};

class Guild
{
	public:
		Guild();
		~Guild();
    
		void create(uint64 lGuid, std::string gname);

		typedef std::list<MemberSlot*> MemberList;


		uint32 GetId(){ return Id; }
		const uint64& GetLeader(){ return leaderGuid; }
		std::string GetName(){ return name; }
		std::string GetMOTD(){ return MOTD; }
			
		uint32 GetCreatedYear(){ return CreatedYear; }
		uint32 GetCreatedMonth(){ return CreatedMonth; }
		uint32 GetCreatedDay(){ return CreatedDay; }
		
		uint32 GetEmblemStyle(){ return EmblemStyle; }
		uint32 GetEmblemColor(){ return EmblemColor; }
		uint32 GetBorderStyle(){ return BorderStyle; }
		uint32 GetBorderColor(){ return BorderColor; }
		uint32 GetBackgroundColor(){ return BackgroundColor; }
		
		uint32 GetMemberSize(){ return members.size(); }
		std::list<MemberSlot*>::iterator membersbegin(){ return members.begin(); }
		std::list<MemberSlot*>::iterator membersEnd(){ return members.end(); }

		void SetLeader(uint64 guid){ leaderGuid = guid; }
		void addMember(MemberSlot *memslot){ members.push_back(memslot); }
		void SetMOTD(std::string motd) { MOTD = motd; }

			
		
		// Serialization
		void LoadGuildFromDB(uint32 GuildId);
		void LoadMembersFromDB(uint32 GuildId);
		
		void SaveGuildToDB();
		void SaveGuildMembers();
		
		//broadcast menssages
		void BroadcastToGuild(WorldSession *session, std::string msg);
		void BroadcastToOfficers(WorldSession *session, std::string msg);
			
		//ranks
		RankInfo* GetRankInfo(uint32 rankId){ return &ranks[rankId]; }
		uint32 GetNrRanks(){ return nrranks; }
		uint32 Guild::CompileRankRights(RankInfo rankinfo);

	protected:
		
		// Guild Info
		uint32 Id;
		std::string name;
		uint64 leaderGuid;
		std::string MOTD;
		uint32 CreatedYear;
		uint32 CreatedMonth;
		uint32 CreatedDay;

		// Tabard Info
		uint32 EmblemStyle;
		uint32 EmblemColor;
		uint32 BorderStyle;
		uint32 BorderColor;
		uint32 BackgroundColor;
	
		// Ranks Info
		uint32 nrranks;
		RankInfo ranks[MAXRANKS];
		
		// Members Info
		MemberList members;
};



#endif
