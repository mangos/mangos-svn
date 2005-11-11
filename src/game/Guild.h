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
/*enum GuildRanks
{
	GUILD_RANK_INITIATE     = 0,
	GUILD_RANK_MEMBER       = 1,
	GUILD_RANK_VETERAN      = 2,
	GUILD_RANK_OFFICER      = 3,
	GUILD_RANK_GUILDMASTER  = 4,
}*/

struct MemberSlot
{
  uint64 guid;
  uint32 Rank;
};

class Guild
{
	public:

        Guild()
		{
			leaderGuid = 0;
		}
        ~Guild()
		{
		}

		typedef std::list<MemberSlot*> MemberList;


		uint32 GetId(){ return Id; }
		uint64 GetLeader(){ return leaderGuid; }
		std::string GetName(){ return name; }
		std::string GetRankName(uint32 rankId){ return ranknames[rankId]; }
		uint32 GetEmblemStyle(){ return EmblemStyle; }
		uint32 GetEmblemColor(){ return EmblemColor; }
		uint32 GetBorderStyle(){ return BorderStyle; }
		uint32 GetBorderColor(){ return BorderColor; }
		uint32 GetBackgroundColor(){ return BackgroundColor; }
		
		void addMember(MemberSlot *memslot){ members.push_back(memslot); }
		
		// Serialization
		void LoadGuildFromDB(uint32 GuildId);
		void LoadMembersFromDB(uint32 GuildId);
		
		void SaveGuild()
		
		//broadcast menssage
		void BroadcastToGuild(WorldSession *session, std::string msg);
		void BroadcastToOfficers(WorldSession *session, std::string msg);

	protected:
		
		uint32 Id;
		std::string name;
		uint64 leaderGuid;
		
		std::string ranknames[MAXRANKS];

		uint32 EmblemStyle;
		uint32 EmblemColor;
		uint32 BorderStyle;
		uint32 BorderColor;
		uint32 BackgroundColor;
		
		MemberList members;


};



#endif