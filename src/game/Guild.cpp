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


void Guild::LoadGuildFromDB(uint32 GuildId)
{

	std::stringstream query;

	query << "SELECT * FROM `guilds` where guildId= " << GuildId;
	QueryResult *result = sDatabase.Query( query.str().c_str() );

	if(!result)
	{
		
		return;
	}
	Field *fields = result->Fetch();

	Id = fields[0].GetUInt64();
	name = fields[1].GetString();
	leaderGuid = fields[2].GetUInt64();
	for(int i=3;i<13;i++)
	{
		ranknames[i-3] = fields[i].GetString();
	}
	EmblemStyle = fields[13].GetUInt32();
	EmblemColor = fields[14].GetUInt32();
	BorderStyle = fields[15].GetUInt32();
	BorderColor = fields[16].GetUInt32();
	BackgroundColor = fields[17].GetUInt32();

	LoadMembersFromDB(GuildId);
}

void Guild::LoadMembersFromDB(uint32 GuildId)
{
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
		newmember->Rank = fields[2].GetUInt32();
		addMember(newmember);

	}while( result->NextRow() );
}

void Guild::BroadcastToGuild(WorldSession *session, std::string msg)
{	
    if (session && session->GetPlayer())
    {
    	WorldPacket datal;
    	Player *pl, *pleader;
			std::list<MemberSlot*>::iterator itr;
			
			sChatHandler.FillMessageData(&datal, session, CHAT_MSG_GUILD, LANG_UNIVERSAL, NULL, msg.c_str());
			pleader = ObjectAccessor::Instance().FindPlayer(GetLeader());
			
			if (pleader && pleader->GetSession())
				pleader->GetSession()->SendPacket(&datal);
				
      for (itr = members.begin(); itr != members.end(); itr++)
      {
          WorldPacket data;
          sChatHandler.FillMessageData(&data, session, CHAT_MSG_GUILD, LANG_UNIVERSAL, NULL, msg.c_str());

        	pl = ObjectAccessor::Instance().FindPlayer((*itr)->guid);

          if (pl && pl->GetSession())
              pl->GetSession()->SendPacket(&data);
      }
    }
}

// not yet implemented
void Guild::BroadcastToOfficers(WorldSession *session, std::string msg)
{
	
	
	
	
}
