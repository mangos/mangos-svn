/* GuildHandler.cpp
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

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"
#include "UpdateData.h"
#include "Chat.h"
#include "Guild.h"
#include "MapManager.h"



void WorldSession::HandleGuildQueryOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	uint32 guildId;
	Guild *guild;

	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_QUERY"  );
	
	recvPacket >> guildId;

	if(!(guild = objmgr.GetGuildById(guildId))) return;

	data.Initialize( SMSG_GUILD_QUERY_RESPONSE );

	data << guild->GetId();
	data << guild->GetName();
	for(int i=0;i<10;i++)
	{
		if(guild->GetRankName(i) == "Unused") break;
		data << guild->GetRankName(i);
	}
	data << guild->GetEmblemStyle();
	data << guild->GetEmblemColor();
	data << guild->GetBorderStyle();
	data << guild->GetBorderColor();
	data << guild->GetBackgroundColor();

	SendPacket( &data );
	
	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_QUERY_RESPONSE)" );
}

void WorldSession::HandleGuildCreateOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	std::string gname;

	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_CREATE"  );

	recvPacket >> gname;

	std::stringstream ss;
	ss << "INSERT INTO `guild` (name) VALUES ( " << gname <<  " )";
    sDatabase.Execute( ss.str( ).c_str( ) );	
}

void WorldSession::HandleGuildInviteOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	std::string Invitedname;
	Player * player;
	Guild *guild;

	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_INVITE"  );

	recvPacket >> Invitedname;

	player = objmgr.GetPlayer(Invitedname.c_str());
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());	
	
	
	Log::getSingleton( ).outDebug( "Player %s Invited %s to Join his Guild",GetPlayer()->GetName(),Invitedname.c_str());
	
	if(!player || !guild ) return;
	
	player->SetGuildIdInvited(GetPlayer()->GetGuildId());

	data.Initialize(SMSG_GUILD_INVITE);
	data << GetPlayer()->GetName();
	data << guild->GetName();
		
	player->GetSession()->SendPacket(&data);
	
	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_INVITE)" );
}

void WorldSession::HandleGuildAcceptOpcode(WorldPacket& recvPacket)
{
	Guild *guild;
	MemberSlot *memslot;
	
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_ACCEPT"  );
		
	if(GetPlayer()->GetGuildId()) return;
	
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildIdInvited());	
			
	GetPlayer()->SetInGuild(GetPlayer()->GetGuildIdInvited());
	memslot = new MemberSlot;
	memslot->guid = GetPlayer()->GetGUID();
	memslot->Rank = 0;
	guild->addMember(memslot);
		

}

void WorldSession::HandleGuildDeclineOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_DECLINE"  );

	GetPlayer()->UnSetGuildIdInvited();


}

void WorldSession::HandleGuildInfoOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_INFO"  );

}

void WorldSession::HandleGuildRosterOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_ROSTER"  );

}

void WorldSession::HandleGuildPromoteOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_PROMOTE"  );

}

void WorldSession::HandleGuildDemoteOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_DEMOTE"  );

}

void WorldSession::HandleGuildLeaveOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_LEAVE"  );


}

void WorldSession::HandleGuildRemoveOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_REMOVE"  );


}

void WorldSession::HandleGuildDisbandOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_DISBAND"  );


}

void WorldSession::HandleGuildLeaderOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_LEADER"  );


}

void WorldSession::HandleGuildMOTDOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_MOTD"  );


}








