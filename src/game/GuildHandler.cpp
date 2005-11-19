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
	for(int i=0;i<guild->GetNrRanks();i++)
		data << guild->GetRankInfo(i)->name;
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
	Guild *guild;

	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_CREATE"  );

	recvPacket >> gname;
	if(!GetPlayer()->GetGuildId())
	{
		guild = new Guild;
		guild->create(GetPlayer()->GetGUID(),gname);
		objmgr.AddGuild(guild);
	}
}

void WorldSession::HandleGuildInviteOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	std::string Invitedname;
	Player * player;
	Guild *guild;

	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_INVITE"  );

	recvPacket >> Invitedname;

	player = ObjectAccessor::Instance().FindPlayerByName(Invitedname.c_str());
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());	
	if( !guild ) return;

	if( !player )
	{
		data.Initialize(SMSG_GUILD_COMMAND_RESULT);	
		data << (uint32)GUILD_INVITE_S;
		data << Invitedname.c_str();
		data << (uint32)GUILD_PLAYER_NOT_FOUND;
		SendPacket(&data);
		return;
	}
	else if( player->GetGuildId() )
	{
		data.Initialize(SMSG_GUILD_COMMAND_RESULT);	
		data << (uint32)GUILD_INVITE_S;
		data << player->GetName();
		data << (uint32)ALREADY_IN_GUILD;
		SendPacket(&data);
		return;
	}
	else if( player->GetGuildIdInvited() )
	{
		data.Initialize(SMSG_GUILD_COMMAND_RESULT);	
		data << (uint32)GUILD_INVITE_S;
		data << player->GetName();
		data << (uint32)ALREADY_INVITED_TO_GUILD;
		SendPacket(&data);
		return;
	}
	//If player isnt a leader or havent rights to invite, give error.
	/*else if(GetPlayer()->GetGUID() != guild->GetLeader()) || !guild->GetRankInfo(GetPlayer()->GetRank())->InviteMember)
	{
		data.Initialize(SMSG_GUILD_COMMAND_RESULT);	
		data << (uint32)GUILD_INVITE_S;
		data << (uint8)0;
		data << (uint32)GUILD_PERMISSIONS;
		SendPacket(&data);
		return;
	}*/

	Log::getSingleton( ).outDebug( "Player %s Invited %s to Join his Guild",GetPlayer()->GetName(),Invitedname.c_str());
	
	player->SetGuildIdInvited(GetPlayer()->GetGuildId());

	data.Initialize(SMSG_GUILD_INVITE);
	data << GetPlayer()->GetName();
	data << guild->GetName();
	player->GetSession()->SendPacket(&data);
	
	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_INVITE)" );
}

void WorldSession::HandleGuildAcceptOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	Guild *guild;
	MemberSlot *memslot;
	
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_ACCEPT"  );
		
	
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildIdInvited());	
	if(!guild || GetPlayer()->GetGuildId()) return;		
	
	memslot = new MemberSlot;
	memslot->guid = GetPlayer()->GetGUID();
	memslot->RankId = GUILD_RANK_INITIATE;
	guild->addMember(memslot);
	GetPlayer()->SetInGuild(GetPlayer()->GetGuildIdInvited());
	GetPlayer()->SetRank( GUILD_RANK_INITIATE );

	data.Initialize(SMSG_GUILD_EVENT);
	data << (uint8)GUILD_EVENT_JOINED;
	data << (uint8)1;
	data << GetPlayer()->GetName();
	SendPacket(&data);

	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
}

void WorldSession::HandleGuildDeclineOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_DECLINE"  );

	GetPlayer()->SetGuildIdInvited(0);
	GetPlayer()->SetInGuild(0);

	//(FIX-ME) send to the inviter
	/*data.Initialize(SMSG_GUILD_DECLINE);
	data << GetPlayer()->GetName();
	SendPacket(&data);*/
	
}

void WorldSession::HandleGuildInfoOpcode(WorldPacket& recvPacket)
{
	Guild *guild;
	WorldPacket data;
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_INFO"  );

	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
	if(!guild) return;

	data.Initialize( SMSG_GUILD_INFO );
	data << guild->GetCreatedYear();
	data << guild->GetCreatedMonth();
	data << guild->GetCreatedDay();
	data << guild->GetMemberSize();

	SendPacket(&data);
}

void WorldSession::HandleGuildRosterOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	Guild *guild;
	Player *pl;
	
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_ROSTER"  );
		
	
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());		
	if(!guild) return;

	data.Initialize(SMSG_GUILD_ROSTER);	
	
	data << guild->GetMemberSize();
	data << guild->GetMOTD();
	data << guild->GetNrRanks();

	//--ranks rights-- FIX-ME 
	data << (uint32)0xFFFF;
	for(int i=0;i<guild->GetNrRanks()-1;i++)
		data << (uint32)0;
	
	std::list<MemberSlot*>::iterator itr;
						
	for (itr = guild->membersbegin(); itr != guild->membersEnd(); itr++)
	{
		pl = ObjectAccessor::Instance().FindPlayer((*itr)->guid);

		if (pl)
		{
			data << pl->GetGUID();
			pl->IsInWorld() ? data << (uint8)1 : data << (uint8)0;
			data << pl->GetName();
			data << pl->GetRank();
			data << pl->getLevel();
			data << pl->getClass();
			data << pl->GetZoneId();
			data << (uint8)0;// days offline?
			data << (uint8)0;// hours offline?
			data << (uint8)0;// minutes offline?
		}
	}
	
	SendPacket(&data);
	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_ROSTER)" );

}

void WorldSession::HandleGuildPromoteOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	std::string name;
	Player * player;
	Guild *guild;

	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_PROMOTE"  );
	
	recvPacket >> name;

	player = ObjectAccessor::Instance().FindPlayerByName(name.c_str());
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());	
	if(!player || !guild || player == GetPlayer()) return;

	//If player isnt a leader or havent right to promote, sends error.
	/*if(GetPlayer()->GetGUID() != guild->GetLeader()) || !guild->GetRankInfo(GetPlayer()->GetRank())->Promote)
	{
		data.Initialize(SMSG_GUILD_COMMAND_RESULT);	
		data << (uint32)GUILD_INVITE_S;
		data << (uint8)0;
		data << (uint32)GUILD_PERMISSIONS;
		SendPacket(&data);
		return;
	}*/
	//If target player isnt in the same guild, sends a error.(FIX-ME)
	if(GetPlayer()->GetGuildId() != player->GetGuildId()) return;

	if(player->GetRank()-1 >= 0) player->SetRank(player->GetRank()-1);
}

void WorldSession::HandleGuildDemoteOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	std::string name;
	Player * player;
	Guild *guild;

	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_DEMOTE"  );
	
	recvPacket >> name;

	player = ObjectAccessor::Instance().FindPlayerByName(name.c_str());
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());	
	if(!player || !guild || player == GetPlayer()) return;

	//If player isnt a leader or havent right to demote, sends error.
	/*if(GetPlayer()->GetGUID() != guild->GetLeader()) || !guild->GetRankInfo(GetPlayer()->GetRank())->Demote)
	{
		data.Initialize(SMSG_GUILD_COMMAND_RESULT);	
		data << (uint32)GUILD_INVITE_S;
		data << (uint8)0;
		data << (uint32)GUILD_PERMISSIONS;
		SendPacket(&data);
		return;
	}*/
	//If target player isnt in the same guild, sends a error.(FIX-ME)
	if(GetPlayer()->GetGuildId() != player->GetGuildId()) return;

	if(player->GetRank()+1 <= 9) player->SetRank(player->GetRank()+1);
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
	WorldPacket data;
	std::string name;
	Player * player;
	Guild *guild;

	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_LEADER"  );
	
	recvPacket >> name;

	player = ObjectAccessor::Instance().FindPlayerByName(name.c_str());
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());	
	if(!player || !guild) return;

	//If player isnt a leader, sends error.
	/*if(GetPlayer()->GetGUID() != guild->GetLeader())
	{
		data.Initialize(SMSG_GUILD_COMMAND_RESULT);	
		data << (uint32)GUILD_INVITE_S;
		data << (uint8)0;
		data << (uint32)GUILD_PERMISSIONS;
		SendPacket(&data);
		return;
	}*/
	//If target player isnt in the same guild, sends a error.(FIX-ME)
	if(GetPlayer()->GetGuildId() != player->GetGuildId()) return;

	guild->SetLeader(player->GetGUID());
	

}

void WorldSession::HandleGuildMOTDOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	Guild *guild;
	std::string MOTD;
	
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_MOTD"  );
		
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
	if(!guild) return;

	//If player isnt a leader or havent rights to SetMOTD, give error.
	/*if(GetPlayer()->GetGUID() != guild->GetLeader() || !guild->GetRankInfo(GetPlayer()->GetRank())->SetMotd)
	{
		data.Initialize(SMSG_GUILD_COMMAND_RESULT);	
		data << (uint32)GUILD_INVITE_S;
		data << (uint8)0;
		data << (uint32)GUILD_PERMISSIONS;
		SendPacket(&data);
		return;
	}*/

	recvPacket >> MOTD;
	guild->SetMOTD(MOTD);
	
	data.Initialize(SMSG_GUILD_EVENT);
	data << (uint8)GUILD_EVENT_MOTD;
	data << (uint8)1;
	data << MOTD;
	
	SendPacket(&data);
	
	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
	
}








