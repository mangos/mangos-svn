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

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Guild.h"
#include "MapManager.h"
#include "GossipDef.h"

void WorldSession::HandleGuildQueryOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 4);

    uint32 guildId;
    Guild *guild;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_QUERY");

    recvPacket >> guildId;

    guild = objmgr.GetGuildById(guildId);
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    guild->Query(this);
}

void WorldSession::HandleGuildCreateOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string gname;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_CREATE");

    recvPacket >> gname;

    if(GetPlayer()->GetGuildId())
        return;

    Guild *guild = new Guild;
    if(!guild->create(GetPlayer()->GetGUID(),gname))
    {
        delete guild;
        return;
    }

    objmgr.AddGuild(guild);
}

void WorldSession::HandleGuildInviteOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string Invitedname, plname;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_INVITE");

    Player * player = NULL;

    recvPacket >> Invitedname;

    if(Invitedname.size() != 0)
    {
        normalizePlayerName(Invitedname);

        player = ObjectAccessor::Instance().FindPlayerByName(Invitedname.c_str());
    }

    if(!player)
    {
        SendGuildCommandResult(GUILD_INVITE_S, Invitedname, GUILD_PLAYER_NOT_FOUND);
        return;
    }

    Guild *guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    // OK result but not send invite
    if(player->HasInIgnoreList(GetPlayer()->GetGUID()))
        return;

    // not let enemies sign guild charter
    if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD) && player->GetTeam() != GetPlayer()->GetTeam())
    {
        SendGuildCommandResult(GUILD_INVITE_S, Invitedname, GUILD_NOT_ALLIED);
        return;
    }

    if(player->GetGuildId())
    {
        plname = player->GetName();
        SendGuildCommandResult(GUILD_INVITE_S, plname, ALREADY_IN_GUILD);
        return;
    }

    if(player->GetGuildIdInvited())
    {
        plname = player->GetName();
        SendGuildCommandResult(GUILD_INVITE_S, plname, ALREADY_INVITED_TO_GUILD);
        return;
    }

    if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_INVITE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }

    sLog.outDebug("Player %s Invited %s to Join his Guild", GetPlayer()->GetName(), Invitedname.c_str());

    player->SetGuildIdInvited(GetPlayer()->GetGuildId());

    WorldPacket data(SMSG_GUILD_INVITE, (8+10));            // guess size
    data << GetPlayer()->GetName();
    data << guild->GetName();
    player->GetSession()->SendPacket(&data);

    //sLog.outDebug("WORLD: Sent (SMSG_GUILD_INVITE)");
}

void WorldSession::HandleGuildRemoveOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string plName;
    uint64 plGuid;
    Guild *guild;
    Player *player;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_REMOVE");

    recvPacket >> plName;

    if(plName.size() == 0)
        return;

    normalizePlayerName(plName);
    //sDatabase.escape_string(plName);                        // prevent SQL injection - normal name don't must changed by this call

    player = ObjectAccessor::Instance().FindPlayerByName(plName.c_str());
    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(player)
        plGuid = player->GetGUID();
    else
        plGuid = objmgr.GetPlayerGUIDByName(plName);

    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    else if(!plGuid)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, GUILD_PLAYER_NOT_FOUND);
        return;
    }
    else if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_REMOVE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }
    if(plGuid == guild->GetLeader())
    {
        SendGuildCommandResult(GUILD_QUIT_S, "", GUILD_LEADER_LEAVE);
        return;
    }

    guild->DelMember(plGuid);

    WorldPacket data(SMSG_GUILD_EVENT, (2+20));             // guess size
    data << (uint8)GE_REMOVED;
    data << (uint8)2;
    data << plName;
    data << GetPlayer()->GetName();
    guild->BroadcastPacket(&data);
}

void WorldSession::HandleGuildAcceptOpcode(WorldPacket& /*recvPacket*/)
{
    Guild *guild;
    Player *player = GetPlayer();

    //sLog.outDebug("WORLD: Received CMSG_GUILD_ACCEPT");

    guild = objmgr.GetGuildById(player->GetGuildIdInvited());
    if(!guild || player->GetGuildId())
        return;

    // not let enemies sign guild charter
    if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD) && player->GetTeam() != objmgr.GetPlayerTeamByGUID(guild->GetLeader()))
        return;

    if(!guild->AddMember(GetPlayer()->GetGUID()))
        return;

    WorldPacket data(SMSG_GUILD_EVENT, (2+10));             // guess size
    data << (uint8)GE_JOINED;
    data << (uint8)1;
    data << player->GetName();
    guild->BroadcastPacket(&data);

    //sLog.outDebug("WORLD: Sent (SMSG_GUILD_EVENT)");
}

void WorldSession::HandleGuildDeclineOpcode(WorldPacket& /*recvPacket*/)
{
    //sLog.outDebug("WORLD: Received CMSG_GUILD_DECLINE");

    GetPlayer()->SetGuildIdInvited(0);
    GetPlayer()->SetInGuild(0);
}

void WorldSession::HandleGuildInfoOpcode(WorldPacket& /*recvPacket*/)
{
    Guild *guild;
    //sLog.outDebug("WORLD: Received CMSG_GUILD_INFO");

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    WorldPacket data(SMSG_GUILD_INFO, (5*4 + guild->GetName().size() + 1));
    data << guild->GetName();
    data << guild->GetCreatedDay();
    data << guild->GetCreatedMonth();
    data << guild->GetCreatedYear();
    data << guild->GetMemberSize();
    data << guild->GetMemberSize();

    SendPacket(&data);
}

void WorldSession::HandleGuildRosterOpcode(WorldPacket& /*recvPacket*/)
{
    //sLog.outDebug("WORLD: Received CMSG_GUILD_ROSTER");

    Guild* guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild) return;

    guild->Roster(this);
}

void WorldSession::HandleGuildPromoteOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string plName;
    uint64 plGuid;
    uint32 plGuildId;
    uint32 plRankId;
    Player *player;
    Guild *guild;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_PROMOTE");

    recvPacket >> plName;

    if(plName.size() == 0)
        return;

    normalizePlayerName(plName);
    //sDatabase.escape_string(plName);                        // prevent SQL injection - normal name don't must changed by this call

    player = ObjectAccessor::Instance().FindPlayerByName(plName.c_str());
    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(player)
    {
        plGuid = player->GetGUID();
        plGuildId = player->GetGuildId();
        plRankId = player->GetRank();
    }
    else
    {
        plGuid = objmgr.GetPlayerGUIDByName(plName);
        plGuildId = Player::GetGuildIdFromDB(plGuid);
        plRankId = Player::GetRankFromDB(plGuid);
    }

    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    else if(!plGuid)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, GUILD_PLAYER_NOT_FOUND);
        return;
    }
    else if(plGuid == GetPlayer()->GetGUID())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_NAME_INVALID);
        return;
    }
    else if(GetPlayer()->GetGuildId() != plGuildId)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }
    else if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_PROMOTE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }
    else if((plRankId-1) == 0 || (plRankId-1) < this->GetPlayer()->GetRank())
        return;

    if(plRankId < 1)
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_INTERNAL);
        return;
    }

    uint32 newRankId = plRankId < guild->GetNrRanks() ? plRankId-1 : guild->GetNrRanks()-1;

    guild->ChangeRank(plGuid, newRankId);

    WorldPacket data(SMSG_GUILD_EVENT, (2+30));             // guess size
    data << (uint8)GE_PROMOTION;
    data << (uint8)3;
    data << GetPlayer()->GetName();
    data << plName;
    data << guild->GetRankName(newRankId);
    guild->BroadcastPacket(&data);
}

void WorldSession::HandleGuildDemoteOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string plName;
    uint64 plGuid;
    uint32 plGuildId;
    uint32 plRankId;
    Player *player;
    Guild *guild;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_DEMOTE");

    recvPacket >> plName;

    if(plName.size() == 0)
        return;

    normalizePlayerName(plName);
    //sDatabase.escape_string(plName);                        // prevent SQL injection - normal name don't must changed by this call

    player = ObjectAccessor::Instance().FindPlayerByName(plName.c_str());
    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(player)
    {
        plGuid = player->GetGUID();
        plGuildId = player->GetGuildId();
        plRankId = player->GetRank();
    }
    else
    {
        plGuid = objmgr.GetPlayerGUIDByName(plName);
        plGuildId = Player::GetGuildIdFromDB(plGuid);
        plRankId = Player::GetRankFromDB(plGuid);
    }

    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if( !plGuid )
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, GUILD_PLAYER_NOT_FOUND);
        return;
    }

    if(plGuid == GetPlayer()->GetGUID())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_NAME_INVALID);
        return;
    }

    if(GetPlayer()->GetGuildId() != plGuildId)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_DEMOTE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }

    if((plRankId+1) >= guild->GetNrRanks() || plRankId <= this->GetPlayer()->GetRank())
        return;

    guild->ChangeRank(plGuid, (plRankId+1));

    WorldPacket data(SMSG_GUILD_EVENT, (2+30));             // guess size
    data << (uint8)GE_DEMOTION;
    data << (uint8)3;
    data << GetPlayer()->GetName();
    data << plName;
    data << guild->GetRankName(plRankId+1);
    guild->BroadcastPacket(&data);
}

void WorldSession::HandleGuildLeaveOpcode(WorldPacket& /*recvPacket*/)
{
    std::string plName;
    Guild *guild;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_LEAVE");

    guild = objmgr.GetGuildById(_player->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if(_player->GetGUID() == guild->GetLeader() && guild->GetMemberSize() > 1)
    {
        SendGuildCommandResult(GUILD_QUIT_S, "", GUILD_LEADER_LEAVE);
        return;
    }

    if(_player->GetGUID() == guild->GetLeader())
    {
        guild->Disband();
        return;
    }

    plName = _player->GetName();

    guild->DelMember(_player->GetGUID());

    WorldPacket data(SMSG_GUILD_EVENT, (2+10));             // guess size
    data << (uint8)GE_LEFT;
    data << (uint8)1;
    data << plName;
    guild->BroadcastPacket(&data);

    //sLog.outDebug("WORLD: Sent (SMSG_GUILD_EVENT)");

    SendGuildCommandResult(GUILD_QUIT_S, plName, GUILD_PLAYER_NO_MORE_IN_GUILD);
}

void WorldSession::HandleGuildDisbandOpcode(WorldPacket& /*recvPacket*/)
{
    std::string name;
    Guild *guild;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_DISBAND");

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if(GetPlayer()->GetGUID() != guild->GetLeader())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }

    guild->Disband();

    //sLog.outDebug("WORLD: Guild Sucefully Disbanded");
}

void WorldSession::HandleGuildLeaderOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string name;
    Player *newLeader;
    uint64 newLeaderGUID;
    uint32 newLeaderGuild;
    Player *oldLeader = GetPlayer();
    Guild *guild;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_LEADER");

    recvPacket >> name;

    if(name.size() == 0)
        return;

    normalizePlayerName(name);
    //sDatabase.escape_string(name);                          // prevent SQL injection - normal name don't must changed by this call

    newLeader = ObjectAccessor::Instance().FindPlayerByName(name.c_str());
    if(newLeader)
    {
        newLeaderGUID = newLeader->GetGUID();
        newLeaderGuild = newLeader->GetGuildId();
    }
    else
    {
        newLeaderGUID = objmgr.GetPlayerGUIDByName(name);
        newLeaderGuild = Player::GetGuildIdFromDB(newLeaderGUID);
    }
    guild = objmgr.GetGuildById(oldLeader->GetGuildId());

    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    else if(!newLeaderGUID)
    {
        SendGuildCommandResult(GUILD_INVITE_S, name, GUILD_PLAYER_NOT_FOUND);
        return;
    }
    if(oldLeader->GetGuildId() != newLeaderGuild)
    {
        SendGuildCommandResult(GUILD_INVITE_S, name, GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }
    if(oldLeader->GetGUID() != guild->GetLeader())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }

    guild->SetLeader(newLeaderGUID);
    guild->ChangeRank(oldLeader->GetGUID(), GR_OFFICER);

    WorldPacket data(SMSG_GUILD_EVENT, (2+20));             // guess size
    data << (uint8)GE_LEADER_CHANGED;
    data << (uint8)2;
    data << oldLeader->GetName();
    data << name.c_str();
    guild->BroadcastPacket(&data);

    //sLog.outDebug("WORLD: Sent (SMSG_GUILD_EVENT)");
}

void WorldSession::HandleGuildMOTDOpcode(WorldPacket& recvPacket)
{
    Guild *guild;
    std::string MOTD;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_MOTD");

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_SETMOTD))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }

    if(recvPacket.size() != 0)
        recvPacket >> MOTD;
    else
        MOTD = "";

    guild->SetMOTD(MOTD);

    WorldPacket data(SMSG_GUILD_EVENT, (2+MOTD.size()+1));
    data << (uint8)GE_MOTD;
    data << (uint8)1;
    data << MOTD;
    guild->BroadcastPacket(&data);

    //sLog.outDebug("WORLD: Sent (SMSG_GUILD_EVENT)");
}

void WorldSession::HandleGuildSetPublicNoteOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);

    Guild *guild;
    Player *player;
    uint64 plGuid;
    uint32 plGuildId;
    std::string name,PNOTE;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_SET_PUBLIC_NOTE");

    recvPacket >> name;

    if(name.size() == 0)
        return;

    normalizePlayerName(name);
    //sDatabase.escape_string(name);                          // prevent SQL injection - normal name don't must changed by this call

    player = ObjectAccessor::Instance().FindPlayerByName(name.c_str());
    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(player)
    {
        plGuid = player->GetGUID();
        plGuildId = player->GetGuildId();
    }
    else
    {
        plGuid = objmgr.GetPlayerGUIDByName(name);
        plGuildId = Player::GetGuildIdFromDB(plGuid);
    }

    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    else if(!plGuid)
    {
        SendGuildCommandResult(GUILD_INVITE_S, name, GUILD_PLAYER_NOT_FOUND);
        return;
    }
    else if(GetPlayer()->GetGuildId() != plGuildId)
    {
        SendGuildCommandResult(GUILD_INVITE_S, name, GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }
    if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_EPNOTE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }

    recvPacket >> PNOTE;
    guild->SetPNOTE(plGuid, PNOTE);

    guild->Roster(this);
}

void WorldSession::HandleGuildSetOfficerNoteOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);

    Guild *guild;
    Player *player;
    uint64 plGuid;
    uint32 plGuildId;
    std::string plName, OFFNOTE;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_SET_OFFICER_NOTE");

    recvPacket >> plName;

    if(plName.size() == 0)
        return;

    normalizePlayerName(plName);
    //sDatabase.escape_string(plName);                        // prevent SQL injection - normal name don't must changed by this call

    player = ObjectAccessor::Instance().FindPlayerByName(plName.c_str());
    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(player)
    {
        plGuid = player->GetGUID();
        plGuildId = player->GetGuildId();
    }
    else
    {
        plGuid = objmgr.GetPlayerGUIDByName(plName);
        plGuildId = Player::GetGuildIdFromDB(plGuid);
    }

    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    else if( !plGuid )
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, GUILD_PLAYER_NOT_FOUND);
        return;
    }
    else if(GetPlayer()->GetGuildId() != plGuildId)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }
    if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_EOFFNOTE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }

    recvPacket >> OFFNOTE;
    guild->SetOFFNOTE(plGuid, OFFNOTE);

    guild->Roster(this);
}

void WorldSession::HandleGuildRankOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 4+4+1);

    Guild *guild;
    std::string rankname;
    uint32 rankId;
    uint32 rights;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_RANK");

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    else if(GetPlayer()->GetGUID() != guild->GetLeader())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }

    recvPacket >> rankId;
    recvPacket >> rights;
    recvPacket >> rankname;

    sLog.outDebug("WORLD: Changed RankName to %s , Rights to 0x%.4X",rankname.c_str(), rights);

    guild->SetRankName(rankId, rankname);
    guild->SetRankRights(rankId, rights);

    guild->Query(this);
    guild->Roster(this);
}

void WorldSession::HandleGuildAddRankOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);

    Guild *guild;
    std::string rankname;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_ADD_RANK");

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if(GetPlayer()->GetGUID() != guild->GetLeader())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }

    if(guild->GetNrRanks() >= 10)                           // client not let create more 10 ranks
        return;

    recvPacket >> rankname;

    guild->CreateRank(rankname, GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);

    guild->Query(this);
    guild->Roster(this);
}

void WorldSession::HandleGuildDelRankOpcode(WorldPacket& /*recvPacket*/)
{
    Guild *guild;
    std::string rankname;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_DEL_RANK");

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    else if(GetPlayer()->GetGUID() != guild->GetLeader())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }

    guild->DelRank();

    guild->Query(this);
    guild->Roster(this);
}

void WorldSession::SendGuildCommandResult(uint32 typecmd,std::string str,uint32 cmdresult)
{
    WorldPacket data(SMSG_GUILD_COMMAND_RESULT, (8+str.size()+1));
    data << typecmd;
    data << str;
    data << cmdresult;
    SendPacket(&data);

    //sLog.outDebug("WORLD: Sent (SMSG_GUILD_COMMAND_RESULT)");
}

void WorldSession::HandleGuildChangeInfoOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);

    //sLog.outDebug("WORLD: Received CMSG_GUILD_CHANGEINFO");

    std::string GINFO;

    recvPacket >> GINFO;

    Guild *guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    guild->SetGINFO(GINFO);
}

void WorldSession::HandleGuildSaveEmblemOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 8+4+4+4+4+4);

    //sLog.outDebug("WORLD: Received MSG_SAVE_GUILD_EMBLEM");

    uint64 vendorGuid;

    uint32 EmblemStyle;
    uint32 EmblemColor;
    uint32 BorderStyle;
    uint32 BorderColor;
    uint32 BackgroundColor;

    recvPacket >> vendorGuid;

    Creature *pCreature = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, vendorGuid,UNIT_NPC_FLAG_TABARDDESIGNER);
    if (!pCreature)
    {
        sLog.outDebug("WORLD: HandleGuildSaveEmblemOpcode - Unit (GUID: %u) not found or you can't interact with him.", GUID_LOPART(vendorGuid));
        return;
    }

    recvPacket >> EmblemStyle;
    recvPacket >> EmblemColor;
    recvPacket >> BorderStyle;
    recvPacket >> BorderColor;
    recvPacket >> BackgroundColor;

    Guild *guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        WorldPacket data(MSG_SAVE_GUILD_EMBLEM, 4);
        data << (uint32)2;                                  // not part of guild
        SendPacket( &data );
        return;
    }

    if (guild->GetLeader() != GetPlayer()->GetGUID())
    {
        WorldPacket data(MSG_SAVE_GUILD_EMBLEM, 4);
        data << (uint32)3;                                  // only leader can
        SendPacket( &data );
        return;
    }

    if(GetPlayer()->GetMoney() < 10*GOLD)
    {
        WorldPacket data(MSG_SAVE_GUILD_EMBLEM, 4);
        data << (uint32)4;                                  //"You have not enough money"
        SendPacket(&data);
        return;
    }

    GetPlayer()->ModifyMoney(-10*GOLD);
    guild->SetEmblem(EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor);

    WorldPacket data(MSG_SAVE_GUILD_EMBLEM, 4);
    data << (uint32)0;
    SendPacket( &data );

    guild->Query(this);
}
