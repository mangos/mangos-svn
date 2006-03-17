/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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
	uint32 guildId;
	Guild *guild;

	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_QUERY"  );
	
	recvPacket >> guildId;

	guild = objmgr.GetGuildById(guildId);
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	
	guild->Query(this);
	
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
	std::string Invitedname,plname;
	Player * player;
	Guild *guild;

	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_INVITE"  );

	recvPacket >> Invitedname;

	player = ObjectAccessor::Instance().FindPlayerByName(Invitedname.c_str());
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());	
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	else if( !player )
	{
		SendCommandResult(GUILD_INVITE_S,Invitedname,GUILD_PLAYER_NOT_FOUND);
		return;
	}
	else if( player->GetGuildId() )
	{
		plname = player->GetName();
		SendCommandResult(GUILD_INVITE_S,plname,ALREADY_IN_GUILD);
		return;
	}
	else if( player->GetGuildIdInvited() )
	{
		plname = player->GetName();
		SendCommandResult(GUILD_INVITE_S,plname,ALREADY_INVITED_TO_GUILD);
		return;
	}
	else if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_INVITE))
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
		return;
	}

	Log::getSingleton( ).outDebug( "Player %s Invited %s to Join his Guild",GetPlayer()->GetName(),Invitedname.c_str());
	
	player->SetGuildIdInvited(GetPlayer()->GetGuildId());

	data.Initialize(SMSG_GUILD_INVITE);
	data << GetPlayer()->GetName();
	data << guild->GetName();
	player->GetSession()->SendPacket(&data);
	
	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_INVITE)" );
}

void WorldSession::HandleGuildRemoveOpcode(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_REMOVE"  );


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
	memslot->name = GetPlayer()->GetName();
	memslot->RankId = GR_INITIATE;
	memslot->Pnote = "";
	memslot->OFFnote = "";
	guild->AddMember(memslot);
	guild->SaveMemberToDB(memslot);
	GetPlayer()->SetInGuild(GetPlayer()->GetGuildIdInvited());
	GetPlayer()->SetRank( GR_INITIATE );
	GetPlayer()->SetGuildIdInvited(0);

	data.Initialize(SMSG_GUILD_EVENT);
	data << (uint8)GE_JOINED;
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

	
	
	
}

void WorldSession::HandleGuildInfoOpcode(WorldPacket& recvPacket)
{
	Guild *guild;
	WorldPacket data;
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_INFO"  );

	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}

	data.Initialize( SMSG_GUILD_INFO );
	data << guild->GetCreatedYear();
	data << guild->GetCreatedMonth();
	data << guild->GetCreatedDay();
	data << guild->GetMemberSize();
	data << guild->GetMemberSize();

	SendPacket(&data);
}

void WorldSession::HandleGuildRosterOpcode(WorldPacket& recvPacket)
{
	Guild *guild;
	
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_ROSTER"  );
		
	
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());		
	if(!guild) return;

	guild->Roster(this);

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
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	else if( !player )
	{
		SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_FOUND);
		return;
	}
	else if(player == GetPlayer())
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_NAME_INVALID);
		return;
	}
	else if(GetPlayer()->GetGuildId() != player->GetGuildId())
	{
		SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_IN_GUILD_S);
		return;
	}
	else if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_PROMOTE))
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
		return;
	}
	
	if(player->GetRank() > 0) player->SetRank(player->GetRank()-1);

	
	data.Initialize(SMSG_GUILD_EVENT);
	data << (uint8)GE_PROMOTION;
	data << (uint8)2;
	data << player->GetName();
	data << guild->GetRankName(player->GetRank());
	
	SendPacket(&data);
	
	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
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
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	else if( !player )
	{
		SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_FOUND);
		return;
	}
	else if(player == GetPlayer())
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_NAME_INVALID);
		return;
	}
	else if(GetPlayer()->GetGuildId() != player->GetGuildId())
	{
		SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_IN_GUILD_S);
		return;
	}
	else if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_DEMOTE))
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
		return;
	}

	if(player->GetRank() < 9) player->SetRank(player->GetRank()+1);

	data.Initialize(SMSG_GUILD_EVENT);
	data << (uint8)GE_DEMOTION;
	data << (uint8)2;
	data << player->GetName();
	data << guild->GetRankName(player->GetRank());

	SendPacket(&data);
	
	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
}

void WorldSession::HandleGuildLeaveOpcode(WorldPacket& recvPacket)
{
	WorldPacket data,data2;
	std::string plname;
	Player * player;
	Guild *guild;
	
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_LEAVE"  );

	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());	
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	if(GetPlayer()->GetGUID() == guild->GetLeader())
	{
		SendCommandResult(GUILD_QUIT_S,"",GUILD_LEADER_LEAVE);
		return;
	}
	plname = GetPlayer()->GetName();
	
	guild->DelMember(GetPlayer()->GetGUID());
	guild->DelMemberFromDB(GetPlayer()->GetGUID());
	
	GetPlayer()->SetInGuild(0);
	GetPlayer()->SetRank(0);
	
	data.Initialize(SMSG_GUILD_EVENT);
	data << (uint8)GE_LEFT;
	data << (uint8)1;
	data << plname;

	SendPacket(&data);
	
	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
		
	SendCommandResult(GUILD_QUIT_S,plname,GUILD_PLAYER_NO_MORE_IN_GUILD);	

}

void WorldSession::HandleGuildDisbandOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	std::string name;
	Guild *guild;

	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_DISBAND"  );

	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());	
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	if(GetPlayer()->GetGUID() != guild->GetLeader())
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
		return;
	}
	guild->Disband();
	
	Log::getSingleton( ).outDebug( "WORLD: Guild Sucefully Disbanded" );
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
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	else if( !player )
	{
		SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_FOUND);
		return;
	}
	if(GetPlayer()->GetGuildId() != player->GetGuildId())
	{
		SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_IN_GUILD_S);
		return;
	}
	if(GetPlayer()->GetGUID() != guild->GetLeader())
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
		return;
	}


	guild->SetLeader(player->GetGUID());
	player->SetRank(GR_GUILDMASTER);


	data.Initialize(SMSG_GUILD_EVENT);
	data << (uint8)GE_LEADER_CHANGED;
	data << (uint8)2;
	data << GetPlayer()->GetName();
	data << player->GetName();
	
	SendPacket(&data);
	
	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
}

void WorldSession::HandleGuildMOTDOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	Guild *guild;
	Player * player;
	std::string MOTD;
	
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_MOTD"  );
		
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_SETMOTD))
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
		return;
	}

	recvPacket >> MOTD;
	guild->SetMOTD(MOTD);
	
	data.Initialize(SMSG_GUILD_EVENT);
	data << (uint8)GE_MOTD;
	data << (uint8)1;
	data << MOTD;
	
	SendPacket(&data);
	
	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
	
}

void WorldSession::HandleGuildSetPublicNoteOpcode(WorldPacket& recvPacket)
{
	
	Guild *guild;
	Player * player;
	std::string name,PNOTE;
	
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_SET_PUBLIC_NOTE"  );
	
	recvPacket >> name;

	player = ObjectAccessor::Instance().FindPlayerByName(name.c_str());
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());	
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	else if( !player )
	{
		SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_FOUND);
		return;
	}
	else if(GetPlayer()->GetGuildId() != player->GetGuildId())
	{
		SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_IN_GUILD_S);
		return;
	}
	if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_EPNOTE))
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
		return;
	}

	recvPacket >> PNOTE;
	guild->SetPNOTE(player->GetGUID(),PNOTE);

	guild->Roster(this);
	
	

}

void WorldSession::HandleGuildSetOfficerNoteOpcode(WorldPacket& recvPacket)
{
	
	
	Guild *guild;
	Player * player;
	std::string OFFNOTE,name;
	
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_SET_OFFICER_NOTE"  );
		
	recvPacket >> name;

	player = ObjectAccessor::Instance().FindPlayerByName(name.c_str());
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());	
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	else if( !player )
	{
		SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_FOUND);
		return;
	}
	else if(GetPlayer()->GetGuildId() != player->GetGuildId())
	{
		SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_IN_GUILD_S);
		return;
	}
	if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_EOFFNOTE))
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
		return;
	}

	recvPacket >> OFFNOTE;
	guild->SetOFFNOTE(player->GetGUID(),OFFNOTE);
	
	guild->Roster(this);
	
	
}

void WorldSession::HandleGuildRankOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	Guild *guild;
	std::string rankname;
	uint32 rankId;
	uint32 rights;
	
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_RANK"  );
		
	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	
	else if(GetPlayer()->GetGUID() != guild->GetLeader())
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
		return;
	}

	recvPacket >> rankId;
	recvPacket >> rights;
	recvPacket >> rankname;
	
	Log::getSingleton( ).outDebug( "WORLD: Changed RankName to %s , Rights to 0x%.4X",rankname.c_str() ,rights );

	guild->SetRankName(rankId,rankname);
	guild->SetRankRights(rankId,rights);

	guild->Query(this);
	guild->Roster(this);

}

void WorldSession::HandleGuildAddRankOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	Guild *guild;
	std::string rankname;
	
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_ADD_RANK"  );

	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	
	else if(GetPlayer()->GetGUID() != guild->GetLeader())
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
		return;
	}

	recvPacket >> rankname;

	guild->CreateRank(rankname,GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);

	guild->Query(this);
	guild->Roster(this);

}

void WorldSession::HandleGuildDelRankOpcode(WorldPacket& recvPacket)
{
	WorldPacket data;
	Guild *guild;
	std::string rankname;
	
	Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_GUILD_DEL_RANK"  );

	guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
	if(!guild)
	{
		SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
		return;
	}
	
	else if(GetPlayer()->GetGUID() != guild->GetLeader())
	{
		SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
		return;
	}

	guild->DelRank();

	guild->Query(this);
	guild->Roster(this);

}

void WorldSession::SendCommandResult(uint32 typecmd,std::string str,uint32 cmdresult)
{
	WorldPacket data;
	data.Initialize(SMSG_GUILD_COMMAND_RESULT);	
	data << typecmd;
	data << str;
	data << cmdresult;
	SendPacket(&data);
	
	Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_GUILD_COMMAND_RESULT)" );
}


