/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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

    if(!Invitedname.empty())
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
    uint32 plGuildId;
    Guild *guild;
    Player *player;

    //sLog.outDebug("WORLD: Received CMSG_GUILD_REMOVE");

    recvPacket >> plName;

    if(plName.empty())
        return;

    normalizePlayerName(plName);
    //CharacterDatabase.escape_string(plName);              // prevent SQL injection - normal name don't must changed by this call

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

    if(!plGuid)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, GUILD_PLAYER_NOT_FOUND);
        return;
    }

    if(!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_REMOVE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }

    if(plGuid == guild->GetLeader())
    {
        SendGuildCommandResult(GUILD_QUIT_S, "", GUILD_LEADER_LEAVE);
        return;
    }

    if(GetPlayer()->GetGuildId() != plGuildId)
    {
        SendGuildCommandResult(GUILD_INVITE_S, plName, GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    guild->DelMember(plGuid);

    WorldPacket data(SMSG_GUILD_EVENT, (2+20));             // guess size
    data << (uint8)GE_REMOVED;
    data << (uint8)2;                                       // strings count
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

    if(!guild->AddMember(GetPlayer()->GetGUID(),guild->GetLowestRank()))
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

    if(plName.empty())
        return;

    normalizePlayerName(plName);
    //CharacterDatabase.escape_string(plName);              // prevent SQL injection - normal name don't must changed by this call

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

    if(plName.empty())
        return;

    normalizePlayerName(plName);
    //CharacterDatabase.escape_string(plName);              // prevent SQL injection - normal name don't must changed by this call

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

    if(name.empty())
        return;

    normalizePlayerName(name);
    //CharacterDatabase.escape_string(name);                // prevent SQL injection - normal name don't must changed by this call

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

    if(!recvPacket.empty())
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

    if(name.empty())
        return;

    normalizePlayerName(name);
    //CharacterDatabase.escape_string(name);                // prevent SQL injection - normal name don't must changed by this call

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

    if(plName.empty())
        return;

    normalizePlayerName(plName);
    //CharacterDatabase.escape_string(plName);              // prevent SQL injection - normal name don't must changed by this call

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
    CHECK_PACKET_SIZE(recvPacket, 4+4+1+4*13);
    //recvPacket.hexlike();

    Guild *guild;
    std::string rankname;
    uint32 rankId;
    uint32 rights, MoneyPerDay;
    uint32 BankRights;
    uint32 BankSlotPerDay;

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
    recvPacket >> MoneyPerDay;

    for (int i = 0; i < GUILD_BANK_MAX_TABS; ++i)
    {
        recvPacket >> BankRights;
        recvPacket >> BankSlotPerDay;
        guild->SetBankRightsAndSlots(rankId, uint8(i), uint16(BankRights & 0xFF), uint16(BankSlotPerDay), true);
    }
    sLog.outDebug("WORLD: Changed RankName to %s , Rights to 0x%.4X", rankname.c_str(), rights);

    guild->SetBankMoneyPerDay(rankId, MoneyPerDay);
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

    if(guild->GetNrRanks() >= GUILD_MAX_RANKS)              // client not let create more 10 than ranks
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

    Creature *pCreature = ObjectAccessor::GetNPCIfCanInteractWith(*_player, vendorGuid,UNIT_NPC_FLAG_TABARDDESIGNER);
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

void WorldSession::HandleGuildEventLogOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug("WORLD: Received (MSG_GUILD_EVENT_LOG)"); // empty
    recvPacket.hexlike();

    uint8 count = 0,type = 0;

    WorldPacket data(MSG_GUILD_EVENT_LOG, 0);
    data << uint8(count);                                   // count, max count == 100
    for(int i = 0; i < count; ++i)
    {

        data << uint8(type);
        data << uint64(0);                                  // guid
        if( type != 2 && type != 6 )
            data << uint64(0);                              // guid

        if( type == 3 || type == 4 )
            data << uint8(0);

        data << uint32(0);                                  // time

    }
    SendPacket(&data);
    sLog.outDebug("WORLD: Sent (MSG_GUILD_EVENT_LOG)");
}

/******  GUILD BANK  *******/

void WorldSession::HandleGuildBankGetMoneyAmount( WorldPacket & /* recv_data */ )
{
    sLog.outDebug("WORLD: Received (MSG_GUILDBANK_GET_MONEY_AMOUNT)");
    //recv_data.hexlike();

    uint32 GuildId = GetPlayer()->GetGuildId();
    if (GuildId == 0)
        return;

    Guild *pGuild = objmgr.GetGuildById(GuildId);
    if(!pGuild)
        return;

    pGuild->SendMoneyInfo(this, GetPlayer()->GetGUIDLow());
}

void WorldSession::HandleGuildBankGetRights( WorldPacket& /* recv_data */ )
{
    sLog.outDebug("WORLD: Received (MSG_GUILDBANK_GET_RIGHTS)");

    uint32 GuildId = GetPlayer()->GetGuildId();
    if (GuildId == 0)
        return;

    Guild *pGuild = objmgr.GetGuildById(GuildId);
    if(!pGuild)
        return;

    uint32 rankId = GetPlayer()->GetRank();

    WorldPacket data(MSG_GUILD_BANK_GET_RIGHTS, 4*15+1);
    data << uint32(rankId);                                 // guild rank id
    data << uint32(pGuild->GetRankRights(rankId));          // rank rights
    data << uint32(pGuild->GetMemberMoneyWithdrawRem(GetPlayer()->GetGUIDLow())); // money per day left
    data << uint8(pGuild->GetPurchasedTabs());              // tabs count
    for(int i = 0; i < GUILD_BANK_MAX_TABS; ++i)
    {
        data << uint32(pGuild->GetBankRights(rankId, uint8(i)));
        data << uint32(pGuild->GetMemberSlotWithdrawRem(GetPlayer()->GetGUIDLow(), uint8(i)));
    }
    SendPacket(&data);
    sLog.outDebug("WORLD: Sent (MSG_GUILD_BANK_GET_RIGHTS)");
}

/* Called when clicking on Guild bank gameobject */
void WorldSession::HandleGuildBankQuery( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: Received (CMSG_GUILD_BANK)");
    CHECK_PACKET_SIZE(recv_data,9);
    uint64 GoGuid;
    uint8  unk;
    recv_data >> GoGuid >> unk;

    if (!objmgr.IsGuildVaultGameObject((uint32)GoGuid))
        return;

    uint32 GuildId = GetPlayer()->GetGuildId();
    if (GuildId == 0)
        return;

    Guild *pGuild = objmgr.GetGuildById(GuildId);
    if(!pGuild)
        return;

    pGuild->DisplayGuildBankTabsInfo(this);
}

/* Called when opening guild bank tab only (first one) */
void WorldSession::HandleGuildBankTabColon( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: Received (CMSG_GUILDBANK_TAB_COLON)");
    CHECK_PACKET_SIZE(recv_data,10);
    uint64 GoGuid;
    uint8 TabId,unk1;
    recv_data >> GoGuid >> TabId >> unk1;

    if (!objmgr.IsGuildVaultGameObject((uint32)GoGuid))
        return;

    uint32 GuildId = GetPlayer()->GetGuildId();
    if (GuildId == 0)
        return;

    Guild *pGuild = objmgr.GetGuildById(GuildId);
    if(!pGuild)
        return;

    // Let's update the amount of gold the player can withdraw before displaying the content
    // This is usefull if money withdraw right has changed
    pGuild->SendMoneyInfo(this, GetPlayer()->GetGUIDLow());
    pGuild->DisplayGuildBankContent(this, TabId);
}

void WorldSession::HandleGuildBankDeposit( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: Received (CMSG_GUILDBANK_DEPOSIT)");
    uint64 GoGuid;
    uint32 money;
    recv_data >> GoGuid >> money;

    if (!money)
        return;

    if (!objmgr.IsGuildVaultGameObject((uint32)GoGuid))
        return;

    uint32 GuildId = GetPlayer()->GetGuildId();
    if (GuildId == 0)
        return;
    
    Guild *pGuild = objmgr.GetGuildById(GuildId);
    if(!pGuild)
        return;

    if (GetPlayer()->GetMoney() < money)
        return;

    pGuild->SetBankMoney(pGuild->GetGuildBankMoney()+money);
    GetPlayer()->ModifyMoney(-int(money));

    // log
    pGuild->LogBankEvent(GUILD_BANK_LOG_DEPOSIT_MONEY, uint8(0), GetPlayer()->GetGUIDLow(), money);

    pGuild->DisplayGuildBankTabsInfo(this);
    pGuild->DisplayGuildBankContent(this, 0); // have to send content of tab 0 of will be blank
}

void WorldSession::HandleGuildBankWithdraw( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: Received (CMSG_GUILDBANK_WITHDRAW)");
    uint64 GoGuid;
    uint32 money;
    recv_data >> GoGuid >> money;

    if (!money)
        return;

    if (!objmgr.IsGuildVaultGameObject((uint32)GoGuid))
        return;

    uint32 GuildId = GetPlayer()->GetGuildId();
    if (GuildId == 0)
        return;
    
    Guild *pGuild = objmgr.GetGuildById(GuildId);
    if(!pGuild)
        return;

    if (pGuild->GetGuildBankMoney()<money)                  // not enough money in bank
        return;

    if (pGuild->GetRankRights(GetPlayer()->GetRank()) & GR_RIGHT_REPAIR_FROM_GUILD)
        return;

    if (!pGuild->MemberMoneyWithdraw(money, GetPlayer()->GetGUIDLow()))
        return;

    GetPlayer()->ModifyMoney(money);

    // Log
    pGuild->LogBankEvent(GUILD_BANK_LOG_WITHDRAW_MONEY, uint8(0), GetPlayer()->GetGUIDLow(), money);
    
    pGuild->SendMoneyInfo(this, GetPlayer()->GetGUIDLow());
    pGuild->DisplayGuildBankTabsInfo(this);
    pGuild->DisplayGuildBankContent(this, 0); // have to send content of tab 0 of will be blank
}

void WorldSession::HandleGuildBankDepositItem( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: Received (CMSG_GUILD_BANK_DEPOSIT_ITEM)");
    recv_data.hexlike();

    uint64 GoGuid;
    uint8 BankToBank;

    uint8 BankTab, BankTabSlot, AutoStore, AutoStoreCount, PlayerSlot, PlayerBag, SplitedAmount = 0;
    uint8 BankTabDst, BankTabSlotDst, unk2, ToChar = 1;
    uint32 ItemEntry, unk1;
    bool BankToChar = false;

    recv_data >> GoGuid >> BankToBank;
    if (BankToBank)
    {
        recv_data >> BankTabDst;
        recv_data >> BankTabSlotDst;
        recv_data >> unk1;                                  // always 0
        recv_data >> BankTab;
        recv_data >> BankTabSlot;
        recv_data >> ItemEntry;
        recv_data >> unk2;                                  // always 0
        recv_data >> SplitedAmount;

        if (BankTabSlotDst >= GUILD_BANK_MAX_SLOTS)
            return;
        if (BankTabDst == BankTab && BankTabSlotDst == BankTabSlot)
            return;
    }
    else
    {
        recv_data >> BankTab;
        recv_data >> BankTabSlot;
        recv_data >> ItemEntry;
        recv_data >> AutoStore;
        if (AutoStore)
            recv_data >> AutoStoreCount;
        recv_data >> PlayerBag;
        recv_data >> PlayerSlot;
        if (!AutoStore)
        {
            recv_data >> ToChar;
            recv_data >> SplitedAmount;
        }

        if (BankTabSlot >= GUILD_BANK_MAX_SLOTS && BankTabSlot != 0xFF)
            return;
    }

    if (!objmgr.IsGuildVaultGameObject((uint32)GoGuid))
        return;

    uint32 GuildId = GetPlayer()->GetGuildId();
    if (GuildId == 0)
        return;

    Guild *pGuild = objmgr.GetGuildById(GuildId);
    if(!pGuild)
        return;

    Player *pl = GetPlayer();

    if (BankToBank)
    {
        // empty operation
        if(BankTab==BankTabDst && BankTabSlot==BankTabSlotDst)
            return;

        Item *pItemSrc = pGuild->GetItem(BankTab, BankTabSlot);
        if (!pItemSrc)                                      // may prevent crash
            return;

        Item *pItemDst = pGuild->GetItem(BankTabDst, BankTabSlotDst);

        if(BankTab!=BankTabDst)
        {
            // check dest pos rights (if different tabs)
            if(!pGuild->CanMemberDepositTo(pl->GetGUIDLow(), BankTabDst))
                return;

            // check source pos rights (if different tabs)
            uint32 remRight = pGuild->GetMemberSlotWithdrawRem(pl->GetGUIDLow(), BankTab);
            if(remRight <= 0)
                return;
        }

        CharacterDatabase.BeginTransaction();
        if (!pItemDst)
        {
            if (SplitedAmount && SplitedAmount < pItemSrc->GetCount())
            {                                               // Bank -> Bank split to empty
                uint32 ItemEntry = pItemSrc->GetEntry();
                Item *pItemDst = NewItemOrBag(objmgr.GetItemPrototype(ItemEntry));
                uint32 NewGuid = objmgr.GenerateLowGuid(HIGHGUID_ITEM);
                pItemDst->Create(NewGuid, ItemEntry, pl);
                pItemDst->SetCount(SplitedAmount);
                pItemDst->AddToWorld();
                pGuild->StoreItem(BankTabDst, &BankTabSlotDst, pItemDst);
                pItemDst->FSetState(ITEM_NEW);
                pItemSrc->SetCount(pItemSrc->GetCount()-SplitedAmount);
                pItemSrc->FSetState(ITEM_CHANGED);
                pItemSrc->SaveToDB();
                pItemDst->SaveToDB();
                CharacterDatabase.PExecute("REPLACE INTO `guild_bank_item` (`guildid`,`TabId`,`SlotId`,`item_guid`,`item_entry`) "
                    "VALUES ('%u', '%u', '%u', '%u', '%u')", GuildId, uint32(BankTabDst), uint32(BankTabSlotDst), pItemDst->GetGUIDLow(), pItemDst->GetEntry());

                pGuild->LogBankEvent(GUILD_BANK_LOG_MOVE_ITEM, BankTab, pl->GetGUIDLow(), pGuild->GetItem(BankTabDst, BankTabSlotDst)->GetEntry(), SplitedAmount, BankTabDst);
            }
            else                                            // Bank -> Bank swap item with empty slot case (move)
            {
                pGuild->EmptyBankSlot(BankTab, BankTabSlot);// 'empty' moved to BankTabSlot

                pGuild->StoreItem(BankTabDst, &BankTabSlotDst, pItemSrc);
                // pItemSrc moved to BankTabSlotDst

                CharacterDatabase.PExecute("REPLACE INTO `guild_bank_item` (`guildid`,`TabId`,`SlotId`,`item_guid`,`item_entry`) "
                    "VALUES ('%u', '%u', '%u', '%u', '%u')", GuildId, uint32(BankTabDst), uint32(BankTabSlotDst), pItemSrc->GetGUIDLow(), pItemSrc->GetEntry());

                CharacterDatabase.CommitTransaction();
                // No need to save item instances, they did not change

                pGuild->LogBankEvent(GUILD_BANK_LOG_MOVE_ITEM, BankTab, pl->GetGUIDLow(), pItemSrc->GetEntry(), pItemSrc->GetCount(), BankTabDst);
            }
        }
        else                                                // ItemDst != NULL case
        {
            if(pItemSrc->GetEntry()==pItemDst->GetEntry() && pItemDst->GetCount() < pItemDst->GetMaxStackCount())
            {
                if(SplitedAmount + pItemDst->GetCount() >  pItemDst->GetMaxStackCount())
                    SplitedAmount = pItemDst->GetMaxStackCount() - pItemDst->GetCount();
            }

            if (SplitedAmount)                              // Bank -> Bank split to another item (partly move)
            {
                // dst right check not required

                pItemDst->SetCount(pItemDst->GetCount()+SplitedAmount);
                pItemDst->FSetState(ITEM_CHANGED);
                pItemDst->SaveToDB();

                pItemSrc->SetCount(pItemSrc->GetCount()-SplitedAmount);
                if (pItemSrc->GetCount() == 0)              // We emptied the bank slot
                {
                    pItemSrc->RemoveFromWorld();
                    pItemSrc->DeleteFromDB();
                    pGuild->EmptyBankSlot(BankTab, BankTabSlot);

                    CharacterDatabase.PExecute("DELETE FROM `guild_bank_item` WHERE `guildid`='%u' AND `TabId`='%u' AND `SlotId`='%u'",
                        GuildId, uint32(BankTab), uint32(BankTabSlot));
                }
                else
                {
                    pItemSrc->FSetState(ITEM_CHANGED);
                    pItemSrc->SaveToDB();
                }

                CharacterDatabase.PExecute("REPLACE INTO `guild_bank_item` (`guildid`,`TabId`,`SlotId`,`item_guid`,`item_entry`) "
                    "VALUES ('%u', '%u', '%u', '%u', '%u')", GuildId, uint32(BankTabDst), uint32(BankTabSlotDst), pItemDst->GetGUIDLow(), pItemDst->GetEntry());

                pGuild->LogBankEvent(GUILD_BANK_LOG_MOVE_ITEM, BankTab, pl->GetGUIDLow(), pGuild->GetItem(BankTabDst, BankTabSlotDst)->GetEntry(), SplitedAmount, BankTabDst);
            }
            else                                            // Bank to Bank swap case
            {
                if(BankTab!=BankTabDst)
                {
                    // check source pos rights (item swapped to src)
                    if(!pGuild->CanMemberDepositTo(pl->GetGUIDLow(), BankTab))
                    {
                        CharacterDatabase.RollbackTransaction();
                        return;
                    }

                    // check dest pos rights (item swapped to src)
                    uint32 remRightDst = pGuild->GetMemberSlotWithdrawRem(pl->GetGUIDLow(), BankTabDst);
                    if(remRightDst <= 0)
                    {
                        CharacterDatabase.RollbackTransaction();
                        return;
                    }
                }

                pGuild->EmptyBankSlot(BankTab, BankTabSlot);// Or next will merge stacks
                pGuild->StoreItem(BankTab, &BankTabSlot, pItemDst);
                // pItemDst moved to BankTabSlot

                CharacterDatabase.PExecute("REPLACE INTO `guild_bank_item` (`guildid`,`TabId`,`SlotId`,`item_guid`,`item_entry`) "
                    "VALUES ('%u', '%u', '%u', '%u', '%u')", GuildId, uint32(BankTab), uint32(BankTabSlot), pItemDst->GetGUIDLow(), pItemDst->GetEntry());

                pGuild->EmptyBankSlot(BankTabDst, BankTabSlotDst);// Or next will merge stacks
                pGuild->StoreItem(BankTabDst, &BankTabSlotDst, pItemSrc);
                // pItemSrc moved to BankTabSlotDst

                CharacterDatabase.PExecute("REPLACE INTO `guild_bank_item` (`guildid`,`TabId`,`SlotId`,`item_guid`,`item_entry`) "
                    "VALUES ('%u', '%u', '%u', '%u', '%u')", GuildId, uint32(BankTabDst), uint32(BankTabSlotDst), pItemSrc->GetEntry());

                CharacterDatabase.CommitTransaction();
                // No need to save item instances, they did not change

                pGuild->LogBankEvent(GUILD_BANK_LOG_MOVE_ITEM, BankTab, pl->GetGUIDLow(), pItemSrc->GetEntry(), pItemSrc->GetCount(), BankTabDst);
                pGuild->LogBankEvent(GUILD_BANK_LOG_MOVE_ITEM, BankTabDst, pl->GetGUIDLow(), pItemDst->GetEntry(), pItemDst->GetCount(), BankTab);
            }
        }

        CharacterDatabase.CommitTransaction();
        pGuild->DisplayGuildBankContent(this, BankTabDst);
        return;
    }
    else                                                    // Player <-> Bank
    {
        Item *pItemBank = pGuild->GetItem(BankTab, BankTabSlot);
        Item *pItemChar = GetPlayer()->GetItemByPos(PlayerBag, PlayerSlot);
        if (!pItemChar && !pItemBank)                       // Nothing to do
            return;

        if (!pItemChar && !ToChar)                          // Problem to get item from player
            return;

        if (!pItemBank && ToChar)                           // Problem to get bank item
            return;

        if(ToChar)
        {
            // check source pos rights (item moved to inventory)
            uint32 remRight = pGuild->GetMemberSlotWithdrawRem(pl->GetGUIDLow(), BankTab);
            if(remRight <= 0)
                return;
        }
        else
        {
            // check source pos rights (item moved to bank)
            if(!pGuild->CanMemberDepositTo(pl->GetGUIDLow(), BankTab))
                return;
        }

        if (ToChar && AutoStore)                            // bank to char autostore (with right-click)
        {
            uint32 Entry = pItemBank->GetEntry();
            uint32 Count = pItemBank->GetCount();
            uint16 Dest;
            uint8 msg;
            msg = pl->CanStoreItem(NULL_BAG, NULL_SLOT, Dest, pItemBank, false);
            if( msg != EQUIP_ERR_OK )
            {
                pl->SendEquipError( msg, pItemBank, NULL );
                return;
            }

            CharacterDatabase.BeginTransaction();

            Item *pItemStacked = pl->StoreItem(Dest, pItemBank, true); // This cause item state to change
            pGuild->EmptyBankSlot(BankTab, BankTabSlot);// Delete from bank table
            if (pItemStacked == pItemBank)              // In case stacked to player, do not execute
            {
                pItemBank->SaveToDB();
                pItemBank->AddToUpdateQueueOf(pl);
                pItemBank->SetState(ITEM_NEW);
            }
            else
            {
                pItemStacked->SetState(ITEM_CHANGED);
            }
            pl->SaveInventoryAndGoldToDB();
            pGuild->LogBankEvent(GUILD_BANK_LOG_WITHDRAW_ITEM, BankTab, pl->GetGUIDLow(), Entry, Count);
            pGuild->MemberItemWithdraw(BankTab, pl->GetGUIDLow());

            CharacterDatabase.CommitTransaction();
            pGuild->DisplayGuildBankContent(this, BankTab);
            return;
        }
        // BankToChar swap or char to bank remaining
        // Split subcase for both directions

        if (ToChar)                                         // Bank -> Char cases
        {
            CharacterDatabase.BeginTransaction();

            uint32 ItemEntry = pItemBank->GetEntry();
            if (!pItemChar)
            {
                if (SplitedAmount && SplitedAmount < pItemBank->GetCount())
                {                                           // Bank -> Char split to empty slot (patly move)
                    Item *pItemToStore = NewItemOrBag(objmgr.GetItemPrototype(ItemEntry));
                    uint32 NewGuid = objmgr.GenerateLowGuid(HIGHGUID_ITEM);
                    pItemToStore->Create(NewGuid, ItemEntry, GetPlayer());
                    pItemToStore->SetCount(SplitedAmount);

                    uint16 Dest;
                    uint8 msg;
                    msg = pl->CanStoreItem(PlayerBag, PlayerSlot, Dest, pItemToStore, false);
                    if( msg != EQUIP_ERR_OK )
                    {
                        pl->SendEquipError( msg, pItemToStore, NULL );
                        delete pItemToStore;
                        sLog.outError("GUILDBANK: Failed to store a newly create item (GUID: %u) from split case in an empty character slot!",pItemToStore->GetGUIDLow());
                        CharacterDatabase.RollbackTransaction();
                        return;
                    }

                    pItemToStore->AddToWorld();
                    pItemToStore->FSetState(ITEM_NEW);
                    pItemToStore->SaveToDB();
                    pl->StoreItem(Dest, pItemToStore, true); // Set state updated
                    pItemBank->SetCount(pItemBank->GetCount()-SplitedAmount);
                    pItemBank->FSetState(ITEM_CHANGED);
                    pItemBank->SaveToDB();
                    pItemToStore->SetState(ITEM_NEW);
                    pl->SaveInventoryAndGoldToDB();
                }
                else                                        // Bank -> Char swap with emoty slot (move)
                {
                    uint16 Dest;
                    uint8 msg;
                    msg = pl->CanStoreItem(PlayerBag, PlayerSlot, Dest, pItemBank, false);
                    if( msg == EQUIP_ERR_OK )
                    {
                        pGuild->EmptyBankSlot(BankTab, BankTabSlot);
                        pl->StoreItem(Dest, pItemBank, true); // Set state updated
                        pItemBank->SaveToDB();
                        pItemBank->SetState(ITEM_NEW);
                        pl->SaveInventoryAndGoldToDB();
                    }
                    else
                    {
                        pl->SendEquipError( msg, pItemBank, NULL );
                        sLog.outError("GUILDBANK: Failed to move this item (GUID: %u) to character!",pItemBank->GetGUIDLow());
                        CharacterDatabase.RollbackTransaction();
                        return;
                    }
                }
            }
            else                                            // To bank -> char but char as existing item
            {
                if(pItemChar->GetEntry()==pItemBank->GetEntry() && pItemChar->GetCount() < pItemChar->GetMaxStackCount())
                {
                    if(SplitedAmount + pItemChar->GetCount() >  pItemChar->GetMaxStackCount())
                        SplitedAmount = pItemChar->GetMaxStackCount() - pItemChar->GetCount();
                }

                if (SplitedAmount)                          // Bank -> Char split in existed item (partly move)
                {                                           // just update counts

                    // not right check required 

                    pItemChar->SetCount(pItemChar->GetCount()+SplitedAmount);
                    pItemBank->SetCount(pItemBank->GetCount()-SplitedAmount);
                    pItemChar->SetState(ITEM_CHANGED);
                    pItemChar->SaveToDB();
                    if (pItemBank->GetCount() == 0)         // We emptied the bank slot
                    {
                        pItemBank->RemoveFromWorld();
                        pItemBank->DeleteFromDB();
                        pGuild->EmptyBankSlot(BankTab, BankTabSlot);
                    }
                    else
                    {
                        pItemBank->FSetState(ITEM_CHANGED);
                        pItemBank->SaveToDB();
                    }
                }
                else                                        // Bank <-> Char swap items
                {
                    // check source pos rights (item swapped to bank)
                    if(!pGuild->CanMemberDepositTo(pl->GetGUIDLow(), BankTab))
                    {
                        CharacterDatabase.RollbackTransaction();
                        return;
                    }

                    // First be sure the character's item is saved to db
                    pItemChar->SaveToDB();

                    uint16 Dest;
                    uint8 msg;
                    msg = pl->CanStoreItem(PlayerBag, PlayerSlot, Dest, pItemBank, true);
                    if( msg != EQUIP_ERR_OK )
                    {
                        CharacterDatabase.RollbackTransaction();

                        sLog.outError("GUILDBANK: Could not add back item (GUID: %u) to character inventory after swap with item (GUID: %u)!",pItemBank->GetGUIDLow(),pItemChar->GetGUIDLow() );
                        return;
                    }
                        
                    // if can store in inventory 
                    // check possibility store in bank
                    
                    // prevent unexpected merge in bank item
                    pGuild->EmptyBankSlot(BankTab, BankTabSlot);
                    pGuild->StoreItem(BankTab, &BankTabSlot, pItemChar);

                    pItemChar->RemoveFromUpdateQueueOf(pl);
                    pItemChar->DestroyForPlayer(pl);
                    pl->RemoveItem(PlayerBag, PlayerSlot, true);
                    
                    CharacterDatabase.PExecute("REPLACE INTO `guild_bank_item` (`guildid`,`TabId`,`SlotId`,`item_guid`,`item_entry`) "
                        "VALUES ('%u', '%u', '%u', '%u', '%u')", GuildId, uint32(BankTab), uint32(BankTabSlot), pItemChar->GetGUIDLow(), pItemChar->GetEntry());

                    pItemChar->DeleteFromInventoryDB();
                    pItemChar->SaveToDB();                  // this item is now in bank

                    pl->StoreItem(Dest, pItemBank, true);
                    pItemBank->SaveToDB();
                    pItemBank->SetState(ITEM_NEW);
                    pl->SaveInventoryAndGoldToDB();
                }
            }
            pGuild->MemberItemWithdraw(BankTab, pl->GetGUIDLow());
            pGuild->LogBankEvent(GUILD_BANK_LOG_WITHDRAW_ITEM, BankTab, pl->GetGUIDLow(), pItemBank->GetEntry(), SplitedAmount);

            CharacterDatabase.CommitTransaction();

            pGuild->DisplayGuildBankContent(this, BankTab);
            return;
        }                                                   // End "To char" part
        else
        {                                                   // Char -> Bank cases
            CharacterDatabase.BeginTransaction();

            pl->SaveInventoryAndGoldToDB();
            if (!pItemBank)
            {
                if (SplitedAmount && SplitedAmount < pItemChar->GetCount())
                {                                           // Char -> Bank split to empty slot (partly move)
                    uint32 ItemEntry = pItemChar->GetEntry();
                    pItemChar->SetCount(pItemChar->GetCount()-SplitedAmount);
                    Item *pItemToStore = NewItemOrBag(objmgr.GetItemPrototype(ItemEntry));
                    uint32 NewGuid = objmgr.GenerateLowGuid(HIGHGUID_ITEM);
                    pItemToStore->Create(NewGuid, ItemEntry, NULL);
                    pItemToStore->SetCount(SplitedAmount);

                    pItemToStore->FSetState(ITEM_NEW);
                    pItemToStore->SaveToDB();

                    pGuild->StoreItem(BankTab, &BankTabSlot, pItemToStore);
                    CharacterDatabase.PExecute("INSERT INTO `guild_bank_item` (`guildid`,`TabId`,`SlotId`,`item_guid`,`item_entry`) "
                        "VALUES ('%u', '%u', '%u', '%u', '%u')", GuildId, uint32(BankTab), uint32(BankTabSlot), pItemToStore->GetGUIDLow(), pItemToStore->GetEntry());
                    pItemChar->SetState(ITEM_CHANGED);
                    pItemChar->SaveToDB();
                }
                else                                        // Char -> Bank swap with empty (move)
                {
                    //first be sure char item is saved
                    pItemChar->SaveToDB();
                    // autostore can be set or not
                    Item *pRetItem = pGuild->StoreItem(BankTab, &BankTabSlot, pItemChar);

                    if (pRetItem == pItemChar)
                    {
                        sLog.outDebug("GUILDBANK: Failed to put item in bank. Tab full?");
                        CharacterDatabase.RollbackTransaction();
                        return;
                    }
                    pItemChar->RemoveFromUpdateQueueOf(pl);
                    pItemChar->DestroyForPlayer(pl);
                    pl->RemoveItem(PlayerBag, PlayerSlot, true);

                    CharacterDatabase.PExecute("REPLACE INTO `guild_bank_item` (`guildid`,`TabId`,`SlotId`,`item_guid`,`item_entry`) "
                        "VALUES ('%u', '%u', '%u', '%u', '%u')", GuildId, uint32(BankTab), uint32(BankTabSlot), pItemChar->GetGUIDLow(), pItemChar->GetEntry());

                    pItemChar->DeleteFromInventoryDB();
                    pItemChar->FSetState(ITEM_CHANGED);
                    pItemChar->SaveToDB();                      // this item is now in bank
                }
                pl->SaveInventoryAndGoldToDB();
                pGuild->LogBankEvent(GUILD_BANK_LOG_DEPOSIT_ITEM, BankTab, pl->GetGUIDLow(), pItemChar->GetEntry(), SplitedAmount ? SplitedAmount : pItemChar->GetCount());
            }
            else
            {
                if(pItemChar->GetEntry()==pItemBank->GetEntry() && pItemBank->GetCount() < pItemBank->GetMaxStackCount())
                {
                    if(SplitedAmount + pItemBank->GetCount() >  pItemBank->GetMaxStackCount())
                        SplitedAmount = pItemBank->GetMaxStackCount() - pItemBank->GetCount();
                }

                if (SplitedAmount)                          // Char -> Bank split to existed item (patly move)
                {
                    // additionl right check not required

                    // No autostore here
                    uint32 ItemEntry = pItemChar->GetEntry();
                    pItemChar->SetCount(pItemChar->GetCount()-SplitedAmount);
                    pItemChar->SetState(ITEM_CHANGED);

                    if (pItemChar->GetCount() == 0)         // We emptied the inventiry slot
                    {
                        pl->DestroyItem(PlayerBag, PlayerSlot, true);
                    }
                    else
                        pItemChar->SaveToDB();

                    pItemBank->SetCount(pItemBank->GetCount()+SplitedAmount);
                    pItemBank->FSetState(ITEM_CHANGED);
                    pItemBank->SaveToDB();
                    pl->SaveInventoryAndGoldToDB();
                    pGuild->LogBankEvent(GUILD_BANK_LOG_DEPOSIT_ITEM, BankTab, pl->GetGUIDLow(), ItemEntry, SplitedAmount);
                }
                else                                        // Char <-> Bank swap items
                {
                    // check bank pos rights (item swapped with inventory)
                    uint32 remRight = pGuild->GetMemberSlotWithdrawRem(pl->GetGUIDLow(), BankTab);
                    if(remRight <= 0)
                    {
                        CharacterDatabase.RollbackTransaction();
                        return;
                    }

                    //first be sure char item is saved
                    pItemChar->SaveToDB();
                    // autostore can be set or not
                    Item *pItemBankOld = pGuild->StoreItem(BankTab, &BankTabSlot, pItemChar);
                    if (pItemBankOld == pItemChar)
                    {
                        sLog.outError("GUILDBANK: Impossible to store this item (GUID: %u) to bank!",pItemChar->GetGUIDLow());
                        CharacterDatabase.RollbackTransaction();
                        return;
                    }

                    uint16 Dest;
                    uint8 msg;
                    msg = pl->CanStoreItem(PlayerBag, PlayerSlot, Dest, pItemBank, true);
                    if( msg != EQUIP_ERR_OK )
                    {
                        GetPlayer()->SendEquipError( msg, pItemBank, NULL );
                        sLog.outError("GUILDBANK: Problem to store back bank item (GUID: %u) to char after swap with item (GUID: %u)!",pItemBank->GetGUIDLow(),pItemChar->GetGUIDLow());
                        CharacterDatabase.RollbackTransaction();
                        return;
                    }

                    pItemChar->RemoveFromUpdateQueueOf(pl);
                    pItemChar->DestroyForPlayer(pl);
                    pl->RemoveItem(PlayerBag, PlayerSlot, true);

                    CharacterDatabase.PExecute("REPLACE INTO `guild_bank_item` (`guildid`,`TabId`,`SlotId`,`item_guid`,`item_entry`) "
                        "VALUES ('%u', '%u', '%u', '%u', '%u')", GuildId, uint32(BankTab), uint32(BankTabSlot), pItemChar->GetGUIDLow(), pItemChar->GetEntry());

                    pItemChar->DeleteFromInventoryDB();
                    pItemChar->SaveToDB();                  // this item is now in bank

                    pl->StoreItem(Dest, pItemBank, true); // ITEM_CHANGED auto set auto
                    pItemBank->SaveToDB();
                    pItemBank->SetState(ITEM_NEW);
                    pGuild->MemberItemWithdraw(BankTab, pl->GetGUIDLow());
                    pGuild->LogBankEvent(GUILD_BANK_LOG_WITHDRAW_ITEM, BankTab, pl->GetGUIDLow(), pItemBank->GetEntry(), pItemBank->GetCount());
                    pl->SaveInventoryAndGoldToDB();
                    pGuild->LogBankEvent(GUILD_BANK_LOG_DEPOSIT_ITEM, BankTab, pl->GetGUIDLow(), pItemChar->GetEntry(), pItemChar->GetCount());

                }
            }
            CharacterDatabase.CommitTransaction();
            pGuild->DisplayGuildBankContent(this, BankTab);
        }
    }
}

void WorldSession::HandleGuildBankBuyTab( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: Received (CMSG_GUILD_BANK_BUY_TAB)");
    recv_data.hexlike();
    uint64 GoGuid;
    uint8 TabId;

    recv_data >> GoGuid;
    recv_data >> TabId;

    if (!objmgr.IsGuildVaultGameObject((uint32)GoGuid))
        return;

    uint32 GuildId = GetPlayer()->GetGuildId();
    if (GuildId==0)
        return;

    Guild *pGuild = objmgr.GetGuildById(GuildId);
    if(!pGuild)
        return;

    uint32 TabCost = objmgr.GetGuildBankTabPrice(TabId) * GOLD;
    if (!TabCost)
        return;

    if (pGuild->GetPurchasedTabs() >= GUILD_BANK_MAX_TABS)
        return;

    if (TabId != pGuild->GetPurchasedTabs())                // purchased_tabs = 0 when buying Tab 0, that is why this check can be made
    {
        sLog.outError("Error: trying to buy a tab non contigous to owned ones");
        return;
    }

    if (GetPlayer()->GetMoney() < TabCost)                  // Should not happen, this is checked by client
       return;

    // Go on with creating tab
    pGuild->CreateNewBankTab();
    GetPlayer()->ModifyMoney(-int(TabCost));
    pGuild->SetBankMoneyPerDay(GetPlayer()->GetRank(), WITHDRAW_MONEY_UNLIMITED);
    pGuild->SetBankRightsAndSlots(GetPlayer()->GetRank(), TabId, GUILD_BANK_RIGHT_FULL, WITHDRAW_SLOT_UNLIMITED, true);
    pGuild->Roster(this);
    pGuild->DisplayGuildBankTabsInfo(this);
}

void WorldSession::HandleGuildBankModifyTab( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: Received (CMSG_GUILD_BANK_MODIFY_TAB)");
    recv_data.hexlike();
    uint64 GoGuid;
    uint8 TabId;
    std::string Name = "";
    std::string IconIndex = "";

    recv_data >> GoGuid;
    recv_data >> TabId;
    recv_data >> Name;
    recv_data >> IconIndex;

    if (!objmgr.IsGuildVaultGameObject((uint32)GoGuid))
        return;

    uint32 GuildId = GetPlayer()->GetGuildId();
    if (GuildId==0)
        return;

    Guild *pGuild = objmgr.GetGuildById(GuildId);
    if(!pGuild)
        return;

    pGuild->SetGuildBankTabInfo(TabId, Name, IconIndex);
    pGuild->DisplayGuildBankTabsInfo(this);
    pGuild->DisplayGuildBankContent(this, TabId);
}

void WorldSession::HandleGuildBankLog( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: Received (MSG_GUILDBANK_LOG)");

    uint32 GuildId = GetPlayer()->GetGuildId();
    if (GuildId == 0)
        return;

    Guild *pGuild = objmgr.GetGuildById(GuildId);
    if(!pGuild)
        return;

    uint8 TabId;
    recv_data >> TabId;

    pGuild->DisplayGuildBankLogs(this, TabId);
}
