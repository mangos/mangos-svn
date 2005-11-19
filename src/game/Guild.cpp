/* Guild.cpp
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

#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "Guild.h"
#include "Chat.h"

Guild::Guild()
{
	Id = 0;
	name = "";
	leaderGuid = 0;
	nrranks = 0;
	for(int i=0;i<MAXRANKS;i++)
	{
		ranks[i].name = "";
		ranks[i].GuildchatListen = 1;
		ranks[i].GuildchatSpeak = 1;
		ranks[i].OfficerchatListen = 0;
		ranks[i].OfficerchatSpeak = 0;
		ranks[i].Promote = 0;
		ranks[i].Demote = 0;
		ranks[i].InviteMember = 0;
		ranks[i].RemovePlayer = 0;
		ranks[i].SetMotd = 0;
		ranks[i].EditPublicNote = 0;
		ranks[i].ViewOfficerNote = 0;
		ranks[i].EditOfficerNote = 0;
	}
	MOTD = "";	
	EmblemStyle = 0;
	EmblemColor = 0;
	BorderStyle = 0;
	BorderColor = 0;
	BackgroundColor = 0;

	CreatedYear = 0;
	CreatedMonth = 0;
	CreatedDay = 0;

}

Guild::~Guild()
{



}


void Guild::create(uint64 lGuid, std::string gname)
{
	Player *pl;
	MemberSlot *newmember;
	
	pl = ObjectAccessor::Instance().FindPlayer(lGuid);
	if(!pl) return;
	
	leaderGuid = lGuid;
	name = gname;
	MOTD = "No message set.";
	ranks[0].name = "Guild Master";
	ranks[1].name = "Officer";
	ranks[2].name = "Veteran";
	ranks[3].name = "Member";
	ranks[4].name = "Initiate";
	nrranks = 5;
	for(int i=5;i<MAXRANKS;i++)
		ranks[i].name = "";

	Log::getSingleton().outDebug("GUILD: creating guild %s to leader:%d", gname.c_str(), leaderGuid);

	
	QueryResult *result = sDatabase.Query( "SELECT MAX(guildId) FROM guilds" );
	if( result )
	{
  		Id = (*result)[0].GetUInt32()+1;
  		delete result;
	}
	else Id = 1;	
	
	SaveGuildToDB();

	if(pl) 
	{
		pl->SetInGuild(Id);
		pl->SetRank( GUILD_RANK_GUILDMASTER	);
	}
	
	newmember = new MemberSlot;
	newmember->guid = leaderGuid;
	newmember->RankId = GUILD_RANK_GUILDMASTER;
	addMember(newmember);

	SaveGuildMembers();

}

void Guild::LoadGuildFromDB(uint32 GuildId)
{

	std::stringstream query;

	query << "SELECT * FROM `guilds` where guildId= " << GuildId;
	QueryResult *result = sDatabase.Query( query.str().c_str() );

	if(!result)
		return;

	Field *fields = result->Fetch();

	Id = fields[0].GetUInt64();
	name = fields[1].GetString();
	leaderGuid = fields[2].GetUInt64();
	for(int i=3;i<13;i++)
	{
		ranks[i-3].name = fields[i].GetString();
		if(ranks[i-3].name != "") nrranks++;
	}
	EmblemStyle = fields[13].GetUInt32();
	EmblemColor = fields[14].GetUInt32();
	BorderStyle = fields[15].GetUInt32();
	BorderColor = fields[16].GetUInt32();
	BackgroundColor = fields[17].GetUInt32();
	MOTD = fields[18].GetString();

	LoadMembersFromDB(GuildId);
}

void Guild::LoadMembersFromDB(uint32 GuildId)
{
	Player *pl;
	std::stringstream query;
	Field *fields;
	MemberSlot *newmember;
	query << "SELECT * FROM `guilds_members` where guildId= " << GuildId;
	QueryResult *result = sDatabase.Query( query.str().c_str() );

	if(!result)
		return;
	do
	{
		fields = result->Fetch();
		newmember = new MemberSlot;
		newmember->guid = fields[1].GetUInt64();
		newmember->RankId = fields[2].GetUInt32();
		addMember(newmember);
		
	}while( result->NextRow() );
}

void Guild::SaveGuildToDB()
{
	std::stringstream ss;
	ss << "INSERT INTO guilds VALUES ("; 
	ss << Id << ", '";
	ss << name << "', "; 
	ss << leaderGuid << ", '"; 
	for(int i=0;i<MAXRANKS;i++)
	{
		if(i==MAXRANKS-1) ss << ranks[i].name << "', ";
		else ss << ranks[i].name << "', '";
	}
	ss << EmblemStyle << ", ";
	ss << EmblemColor << ", ";
	ss << BorderStyle << ", ";
	ss << BorderColor << ", ";
	ss << BackgroundColor << ", '";
	ss << MOTD << "')";

	sDatabase.Execute( ss.str( ).c_str( ) );
}

void Guild::SaveGuildMembers()
{
	std::stringstream ss;
	ss << "INSERT INTO guilds_members VALUES (";
	
	std::list<MemberSlot*>::iterator itr;
	int i=0;
	for (itr = members.begin(); itr != members.end(); itr++)
	{
		ss << Id << ", ";
		ss << (*itr)->guid << ", ";
		ss << (*itr)->RankId;
		if(i != (members.size()-1)) ss << ", ";
		i++;
	}
	ss << " )";

	sDatabase.Execute( ss.str( ).c_str( ) );
}

void Guild::BroadcastToGuild(WorldSession *session, std::string msg)
{	
    if (session && session->GetPlayer() && ranks[session->GetPlayer()->GetRank()].GuildchatSpeak)
    {
    	WorldPacket datal;
    	Player *pl;
		std::list<MemberSlot*>::iterator itr;
						
		for (itr = members.begin(); itr != members.end(); itr++)
		{
			WorldPacket data;
			sChatHandler.FillMessageData(&data, session, CHAT_MSG_GUILD, LANG_UNIVERSAL, NULL, msg.c_str());

        	pl = ObjectAccessor::Instance().FindPlayer((*itr)->guid);

			if (pl && pl->GetSession() && ranks[pl->GetRank()].GuildchatListen)
				pl->GetSession()->SendPacket(&data);
		}
    }
}

void Guild::BroadcastToOfficers(WorldSession *session, std::string msg)
{
    if (session && session->GetPlayer() && ranks[session->GetPlayer()->GetRank()].OfficerchatSpeak)
    {
    	WorldPacket datal;
    	Player *pl;
		std::list<MemberSlot*>::iterator itr;
						
		for (itr = members.begin(); itr != members.end(); itr++)
		{
			WorldPacket data;
			sChatHandler.FillMessageData(&data, session, CHAT_MSG_GUILD, LANG_UNIVERSAL, NULL, msg.c_str());

        	pl = ObjectAccessor::Instance().FindPlayer((*itr)->guid);

			if (pl && pl->GetSession() && ranks[pl->GetRank()].OfficerchatListen)
				pl->GetSession()->SendPacket(&data);
		}
    }
}

// return a uint32 with the ranks compiled FIX-ME
uint32 Guild::CompileRankRights(RankInfo rankinfo)
{
	
	
	
	
	
	
	
}	