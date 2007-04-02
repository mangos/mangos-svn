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

#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "MapManager.h"
#include "Player.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "Guild.h"
#include "Chat.h"

Guild::Guild()
{
    Id = 0;
    name = "";
    leaderGuid = 0;
    GINFO = MOTD = "";
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

bool Guild::create(uint64 lGuid, std::string gname)
{
    std::string rname;
    std::string lName;

    if(!objmgr.GetPlayerNameByGUID(lGuid, lName))
        return false;
    if(objmgr.GetGuildByName(gname))
        return false;

    sLog.outDebug("GUILD: creating guild %s to leader: %u", gname.c_str(), GUID_LOPART(lGuid));

    leaderGuid = lGuid;
    name = gname;
    GINFO = "";
    MOTD = "No message set.";

    QueryResult *result = sDatabase.Query( "SELECT MAX(`guildid`) FROM `guild`" );
    if( result )
    {
        Id = (*result)[0].GetUInt32()+1;
        delete result;
    }
    else Id = 1;

    // gname already assigned to Guild::name, use it to encode string for DB
    sDatabase.escape_string(gname);

    std::string dbGINFO = GINFO;
    std::string dbMOTD = MOTD;
    sDatabase.escape_string(dbGINFO);
    sDatabase.escape_string(dbMOTD);

    sDatabase.BeginTransaction();
    // sDatabase.PExecute("DELETE FROM `guild` WHERE `guildid`='%u'", Id); - MAX(`guildid`)+1 not exist
    sDatabase.PExecute("DELETE FROM `guild_rank` WHERE `guildid`='%u'", Id);
    sDatabase.PExecute("DELETE FROM `guild_member` WHERE `guildid`='%u'", Id);
    sDatabase.PExecute("INSERT INTO `guild` (`guildid`,`name`,`leaderguid`,`info`,`MOTD`,`createdate`,`EmblemStyle`,`EmblemColor`,`BorderStyle`,`BorderColor`,`BackgroundColor`) "
        "VALUES('%u','%s','%u', '%s', '%s', NOW(),'%u','%u','%u','%u','%u')",
        Id, gname.c_str(), GUID_LOPART(leaderGuid), dbGINFO.c_str(), dbMOTD.c_str(), EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor);
    sDatabase.CommitTransaction();

    rname = "Guild Master";
    CreateRank(rname,GR_RIGHT_ALL);
    rname = "Officer";
    CreateRank(rname,GR_RIGHT_ALL);
    rname = "Veteran";
    CreateRank(rname,GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
    rname = "Member";
    CreateRank(rname,GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
    rname = "Initiate";
    CreateRank(rname,GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);

    AddMember(lGuid, (uint32)GR_GUILDMASTER);
    return true;
}

void Guild::AddMember(uint64 plGuid, uint32 plRank)
{
    std::string plName;
    uint8 plLevel, plClass;
    uint32 plZone;

    if(!objmgr.GetPlayerNameByGUID(plGuid, plName))         // player doesnt exist
        return;
    if(Player::GetGuildIdFromDB(plGuid) != 0)               // player already in guild
        return;

    // remove all player signs from another petitions
    // this will be prevent attempt joining player to many guilds and corrupt guild data integrity
    Player::RemovePetitionsAndSigns(plGuid);

    Player* pl = objmgr.GetPlayer(plGuid);
    if(pl)
    {
        plLevel = (uint8)pl->getLevel();
        plClass = (uint8)pl->getClass();

        AreaTableEntry  const* area = GetAreaEntryByAreaFlag(MapManager::Instance().GetMap(pl->GetMapId(), pl)->GetAreaFlag(pl->GetPositionX(),pl->GetPositionY()));
        if (area)                                           // For example: .worldport -2313 478 48 1    Zone will be 0(unkonown), even though it's a usual cave
            plZone = area->zone;                            // would cause null pointer exception
        else
            plZone = 0;
    }
    else
    {
        plLevel = (uint8)Player::GetUInt32ValueFromDB(UNIT_FIELD_LEVEL, plGuid);
        plZone = Player::GetZoneIdFromDB(plGuid);

        QueryResult *result = sDatabase.PQuery("SELECT `class` FROM `character` WHERE `guid`='%u'", GUID_LOPART(plGuid));
        if(!result)
            return;
        plClass = (*result)[0].GetUInt8();
        delete result;
    }

    MemberSlot newmember;
    newmember.name = plName;
    newmember.guid = plGuid;
    newmember.RankId = plRank;
    newmember.OFFnote = (std::string)"";
    newmember.Pnote = (std::string)"";
    newmember.level = plLevel;
    newmember.Class = plClass;
    newmember.zoneId = plZone;
    members.push_back(newmember);

    std::string dbPnote = newmember.Pnote;
    std::string dbOFFnote = newmember.OFFnote;
    sDatabase.escape_string(dbPnote);
    sDatabase.escape_string(dbOFFnote);

    sDatabase.PExecute("INSERT INTO `guild_member` (`guildid`,`guid`,`rank`,`Pnote`,`OFFnote`) VALUES ('%u', '%u', '%u','%s','%s')",
        Id, GUID_LOPART(newmember.guid), newmember.RankId, dbPnote.c_str(), dbOFFnote.c_str());

    if(pl)
    {
        pl->SetInGuild(Id);
        pl->SetRank(newmember.RankId);
        pl->SetGuildIdInvited(0);
    }
    else
    {
        Player::SetUInt32ValueInDB(PLAYER_GUILDID, Id, plGuid);
        Player::SetUInt32ValueInDB(PLAYER_GUILDRANK, newmember.RankId, plGuid);
    }
}

void Guild::SetMOTD(std::string motd)
{
    MOTD = motd;

    // motd now can be used for encoding to DB
    sDatabase.escape_string(motd);
    sDatabase.PExecute("UPDATE `guild` SET `MOTD`='%s' WHERE `guildid`='%u'", motd.c_str(), Id);
}

void Guild::SetGINFO(std::string ginfo)
{
    GINFO = ginfo;

    // ginfo now can be used for encoding to DB
    sDatabase.escape_string(ginfo);
    sDatabase.PExecute("UPDATE `guild` SET `info`='%s' WHERE `guildid`='%u'", ginfo.c_str(), Id);
}

void Guild::LoadGuildFromDB(uint32 GuildId)
{
    LoadRanksFromDB(GuildId);
    LoadMembersFromDB(GuildId);

    QueryResult *result = sDatabase.PQuery("SELECT `guildid`,`name`,`leaderguid`,`EmblemStyle`,`EmblemColor`,`BorderStyle`,`BorderColor`,`BackgroundColor`,`info`,`MOTD`,`createdate` FROM `guild` WHERE `guildid` = '%u'", GuildId);

    if(!result)
        return;

    Field *fields = result->Fetch();

    Id = fields[0].GetUInt32();
    name = fields[1].GetCppString();
    leaderGuid  = MAKE_GUID(fields[2].GetUInt32(),HIGHGUID_PLAYER);
    EmblemStyle = fields[3].GetUInt32();
    EmblemColor = fields[4].GetUInt32();
    BorderStyle = fields[5].GetUInt32();
    BorderColor = fields[6].GetUInt32();
    BackgroundColor = fields[7].GetUInt32();
    GINFO = fields[8].GetCppString();
    MOTD = fields[9].GetCppString();
    uint64 time = fields[10].GetUInt64();                   //datetime is uint64 type ... YYYYmmdd:hh:mm:ss

    delete result;

    uint64 dTime = time /1000000;
    CreatedDay   = dTime%100;
    CreatedMonth = (dTime/100)%100;
    CreatedYear  = (dTime/10000)%10000;
    sLog.outDebug("Guild %u Creation time Loaded day: %u, month: %u, year: %u", GuildId, CreatedDay, CreatedMonth, CreatedYear);
}

void Guild::LoadRanksFromDB(uint32 GuildId)
{
    Field *fields;
    QueryResult *result = sDatabase.PQuery("SELECT `rname`,`rights` FROM `guild_rank` WHERE `guildid` = '%u' ORDER BY `rid` ASC", GuildId);

    if(!result) return;

    do
    {
        fields = result->Fetch();
        AddRank(fields[0].GetCppString(),fields[1].GetUInt32());

    }while( result->NextRow() );
    delete result;
}

void Guild::LoadMembersFromDB(uint32 GuildId)
{
    Field *fields;
    Player *pl;

    QueryResult *result = sDatabase.PQuery("SELECT `guid`,`rank`,`Pnote`,`OFFnote` FROM `guild_member` WHERE `guildid` = '%u'", GuildId);

    if(!result)
        return;

    do
    {
        fields = result->Fetch();
        MemberSlot newmember;
        newmember.guid = MAKE_GUID(fields[0].GetUInt32(),HIGHGUID_PLAYER);
        newmember.RankId = fields[1].GetUInt32();
        pl = ObjectAccessor::Instance().FindPlayer(newmember.guid);
        if(!pl || !pl->IsInWorld()) LoadPlayerStats(&newmember);
        newmember.Pnote = fields[2].GetCppString();
        newmember.OFFnote = fields[3].GetCppString();
        members.push_back(newmember);

    }while( result->NextRow() );
    delete result;
}

void Guild::LoadPlayerStats(MemberSlot* memslot)
{
    Field *fields;

    QueryResult *result = sDatabase.PQuery("SELECT `name`,`class`,`map`,`position_x`,`position_y`,`data` FROM `character` WHERE `guid` = '%u'", GUID_LOPART(memslot->guid));
    if(!result) return;
    fields = result->Fetch();
    memslot->name  = fields[0].GetCppString();
    memslot->Class = fields[1].GetUInt8();

    vector<string> tokens = StrSplit(fields[5].GetCppString(), " ");
    memslot->level = Player::GetUInt32ValueFromArray(tokens,UNIT_FIELD_LEVEL);

    AreaTableEntry const* area = GetAreaEntryByAreaFlag(MapManager::Instance().GetAreaFlag(fields[2].GetUInt32(), fields[3].GetFloat(),fields[4].GetFloat()));
    if (area)                                               // For example: .worldport -2313 478 48 1    Zone will be 0(unkonown), even though it's a usual cave
        memslot->zoneId = area->zone;                       // would cause null pointer exception
    else
        memslot->zoneId = 0;

    delete result;
}

void Guild::LoadPlayerStatsByGuid(uint64 guid)
{
    Player *pl;

    MemberList::iterator itr;
    for (itr = members.begin(); itr != members.end();itr++)
    {
        if (itr->guid == guid)
        {
            pl = ObjectAccessor::Instance().FindPlayer(itr->guid);
            if(!pl)break;
            itr->name  = pl->GetName();
            itr->level = pl->getLevel();
            itr->Class = pl->getClass();
        }
    }

}

void Guild::SetLeader(uint64 guid)
{
    leaderGuid = guid;
    this->ChangeRank(guid, GR_GUILDMASTER);

    sDatabase.PExecute("UPDATE `guild` SET `leaderguid`='%u' WHERE `guildid`='%u'", GUID_LOPART(guid), Id);
}

void Guild::DelMember(uint64 guid, bool isDisbanding)
{
    if(this->leaderGuid == guid && !isDisbanding)
    {
        std::ostringstream ss;
        ss<<"SELECT `guid` FROM `guild_member` WHERE `guildid`='"<<Id<<"' AND `guid`!='"<<this->leaderGuid<<"' ORDER BY `rank` ASC LIMIT 1";
        QueryResult *result = sDatabase.Query( ss.str().c_str() );
        if( result )
        {
            uint64 newLeaderGUID;
            Player *newLeader;
            string newLeaderName, oldLeaderName;

            newLeaderGUID = (*result)[0].GetUInt64();
            delete result;

            this->SetLeader(newLeaderGUID);

            newLeader = objmgr.GetPlayer(newLeaderGUID);
            if(newLeader)
            {
                newLeader->SetRank(GR_GUILDMASTER);
                newLeaderName = newLeader->GetName();
            }
            else
            {
                Player::SetUInt32ValueInDB(PLAYER_GUILDRANK, GR_GUILDMASTER, newLeaderGUID);
                objmgr.GetPlayerNameByGUID(newLeaderGUID, newLeaderName);
            }
            objmgr.GetPlayerNameByGUID(guid, oldLeaderName);

            WorldPacket data(SMSG_GUILD_EVENT, (1+1+oldLeaderName.size()+1+newLeaderName.size()+1));
            data << (uint8)GE_LEADER_CHANGED;
            data << (uint8)2;
            data << oldLeaderName;
            data << newLeaderName;
            this->BroadcastPacket(&data);

            data.Initialize(SMSG_GUILD_EVENT, (1+1+oldLeaderName.size()+1));
            data << (uint8)GE_LEFT;
            data << (uint8)1;
            data << oldLeaderName;
            this->BroadcastPacket(&data);

            sLog.outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
        }
        else
        {
            this->Disband();
            return;
        }
    }

    MemberList::iterator itr;
    for (itr = members.begin(); itr != members.end(); itr++)
    {
        if (itr->guid == guid)
        {
            members.erase(itr);
            break;
        }
    }

    Player *player = objmgr.GetPlayer(guid);
    if(player)
    {
        player->SetInGuild(0);
        player->SetRank(0);
    }
    else
    {
        Player::SetUInt32ValueInDB(PLAYER_GUILDID, 0, guid);
        Player::SetUInt32ValueInDB(PLAYER_GUILDRANK, GR_GUILDMASTER, guid);
    }

    sDatabase.PExecute("DELETE FROM `guild_member` WHERE `guid` = '%u'", GUID_LOPART(guid));
}

void Guild::ChangeRank(uint64 guid, uint32 newRank)
{
    MemberList::iterator itr;
    for (itr = members.begin(); itr != members.end();itr++)
    {
        if(itr->guid == guid)
            itr->RankId = newRank;
    }

    Player *player = objmgr.GetPlayer(guid);
    if(player)
        player->SetRank(newRank);
    else
        Player::SetUInt32ValueInDB(PLAYER_GUILDRANK, newRank, guid);

    sDatabase.PExecute( "UPDATE `guild_member` SET `rank`='%u' WHERE `guid`='%u'", newRank, guid );
}

void Guild::SetPNOTE(uint64 guid,std::string pnote)
{
    MemberList::iterator itr;
    for (itr = members.begin(); itr != members.end();itr++)
    {
        if (itr->guid == guid)
        {
            itr->Pnote = pnote;

            // pnote now can be used for encoding to DB
            sDatabase.escape_string(pnote);
            sDatabase.PExecute("UPDATE `guild_member` SET `Pnote` = '%s' WHERE `guid` = '%u'", pnote.c_str(), GUID_LOPART(itr->guid));
            break;
        }
    }
}

void Guild::SetOFFNOTE(uint64 guid,std::string offnote)
{
    MemberList::iterator itr;
    for (itr = members.begin(); itr != members.end();itr++)
    {
        if (itr->guid == guid)
        {
            itr->OFFnote = offnote;
            // offnote now can be used for encoding to DB
            sDatabase.escape_string(offnote);
            sDatabase.PExecute("UPDATE `guild_member` SET `OFFnote` = '%s' WHERE `guid` = '%u'", offnote.c_str(), GUID_LOPART(itr->guid));
            break;
        }
    }
}

void Guild::BroadcastToGuild(WorldSession *session, std::string msg)
{
    if (session && session->GetPlayer() && HasRankRight(session->GetPlayer()->GetRank(),GR_RIGHT_GCHATSPEAK))
    {
        Player *pl;
        MemberList::iterator itr;

        for (itr = members.begin(); itr != members.end(); itr++)
        {
            WorldPacket data;
            sChatHandler.FillMessageData(&data, session, CHAT_MSG_GUILD, LANG_UNIVERSAL, NULL, 0, msg.c_str());

            pl = ObjectAccessor::Instance().FindPlayer(itr->guid);

            if (pl && pl->GetSession() && HasRankRight(pl->GetRank(),GR_RIGHT_GCHATLISTEN) && !pl->HasInIgnoreList(session->GetPlayer()->GetGUID()) )
                pl->GetSession()->SendPacket(&data);
        }
    }
}

void Guild::BroadcastToOfficers(WorldSession *session, std::string msg)
{
    if (session && session->GetPlayer() && HasRankRight(session->GetPlayer()->GetRank(),GR_RIGHT_OFFCHATSPEAK))
    {
        Player *pl;
        MemberList::iterator itr;

        for (itr = members.begin(); itr != members.end(); itr++)
        {
            WorldPacket data;
            sChatHandler.FillMessageData(&data, session, CHAT_MSG_OFFICER, LANG_UNIVERSAL, NULL, 0, msg.c_str());

            pl = ObjectAccessor::Instance().FindPlayer(itr->guid);

            if (pl && pl->GetSession() && HasRankRight(pl->GetRank(),GR_RIGHT_OFFCHATLISTEN) && !pl->HasInIgnoreList(session->GetPlayer()->GetGUID()))
                pl->GetSession()->SendPacket(&data);
        }
    }
}

void Guild::BroadcastPacket(WorldPacket *packet)
{
    MemberList::iterator itr;

    for (itr = members.begin(); itr != members.end(); itr++)
    {
        Player *player = ObjectAccessor::Instance().FindPlayer(itr->guid);
        if(player)
            player->GetSession()->SendPacket(packet);
    }
}

void Guild::CreateRank(std::string name,uint32 rights)
{
    uint32 rank;

    QueryResult *result = sDatabase.PQuery( "SELECT MAX(`rid`) FROM `guild_rank` WHERE `guildid`='%u'",Id);
    if( result )
    {
        rank = (*result)[0].GetUInt32();                    // rank always = rid-1
        delete result;
    }
    else
        rank = 0;

    AddRank(name,rights);

    // name now can be used for encoding to DB
    sDatabase.escape_string(name);
    sDatabase.PExecute( "INSERT INTO `guild_rank` (`guildid`,`rid`,`rname`,`rights`) VALUES ('%u', '%u', '%s', '%u')", Id, (rank+1), name.c_str(), rights );
}

void Guild::AddRank(std::string name,uint32 rights)
{
    m_ranks.push_back(RankInfo(name,rights));
}

void Guild::DelRank()
{
    if(m_ranks.empty())
        return;

    uint32 rank = m_ranks.size()-1;
    sDatabase.PExecute("DELETE FROM `guild_rank` WHERE `rid`>='%u' AND `guildid`='%u'", (rank+1), Id);

    m_ranks.pop_back();
}

std::string Guild::GetRankName(uint32 rankId)
{
    if(rankId >= m_ranks.size())
        return "<unknown>";

    return m_ranks[rankId].name;
}

uint32 Guild::GetRankRights(uint32 rankId)
{
    if(rankId >= m_ranks.size())
        return 0;

    return m_ranks[rankId].rights;
}

void Guild::SetRankName(uint32 rankId, std::string name)
{
    if(rankId >= m_ranks.size())
        return;

    m_ranks[rankId].name = name;

    // name now can be used for encoding to DB
    sDatabase.escape_string(name);
    sDatabase.PExecute("UPDATE `guild_rank` SET `rname`='%s' WHERE `rid`='%u' AND `guildid`='%u'", name.c_str(), (rankId+1), Id);
}

void Guild::SetRankRights(uint32 rankId, uint32 rights)
{
    if(rankId >= m_ranks.size())
        return;

    m_ranks[rankId].rights = rights;

    sDatabase.PExecute("UPDATE `guild_rank` SET `rights`='%u' WHERE `rid`='%u' AND `guildid`='%u'", rights, (rankId+1), Id);
}

void Guild::Disband()
{
    WorldPacket data(SMSG_GUILD_EVENT, 1);
    data << (uint8)GE_DISBANDED;
    this->BroadcastPacket(&data);

    uint32 count = members.size();
    uint64 *memberGuids = new uint64[count];

    MemberList::iterator itr;
    uint32 i=0;
    for (itr = members.begin(); itr != members.end(); itr++)
    {
        memberGuids[i] = itr->guid;
        i++;
    }

    for(uint32 i=0; i<count; i++)
        this->DelMember(memberGuids[i], true);
    delete[] memberGuids;

    sDatabase.BeginTransaction();
    sDatabase.PExecute("DELETE FROM `guild` WHERE `guildid` = '%u'",Id);
    sDatabase.PExecute("DELETE FROM `guild_rank` WHERE `guildid` = '%u'",Id);
    sDatabase.CommitTransaction();
    objmgr.RemoveGuild(this);
}

void Guild::Roster(WorldSession *session)
{
    Player *pl;

                                                            // we can only guess size
    WorldPacket data(SMSG_GUILD_ROSTER, (4+MOTD.length()+1+GINFO.length()+1+4+m_ranks.size()*4+members.size()*50));
    data << (uint32)members.size();
    data << MOTD;
    data << GINFO;

    data << (uint32)m_ranks.size();
    for (RankList::iterator ritr = m_ranks.begin(); ritr != m_ranks.end();++ritr)
        data << ritr->rights;

    for (MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        pl = ObjectAccessor::Instance().FindPlayer(itr->guid);
        if (pl)
        {
            data << pl->GetGUID();
            data << (uint8)1;
            data << (std::string)pl->GetName();
            data << itr->RankId;
            data << (uint8)pl->getLevel();
            data << pl->getClass();
            data << pl->GetZoneId();
            data << itr->Pnote;
            data << itr->OFFnote;
        }
        else
        {
            uint64 logout_time = 0;
            QueryResult *result = sDatabase.PQuery("SELECT `logout_time` FROM `character` WHERE `guid`='%u'", GUID_LOPART(itr->guid));
            if(result)
            {
                logout_time = (*result)[0].GetUInt64();
                delete result;
            }

            data << itr->guid;
            data << (uint8)0;
            data << itr->name;
            data << itr->RankId;
            data << itr->level;
            data << itr->Class;
            data << itr->zoneId;
            data << (float(time(NULL)-logout_time) / DAY);
            /*data << (uint8)0;
            data << (uint8)1;
            data << (uint8)1;
            data << (uint8)1;*/
            data << itr->Pnote;
            data << itr->OFFnote;
        }
    }
    session->SendPacket(&data);
    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_ROSTER)" );
}

void Guild::Query(WorldSession *session)
{
    WorldPacket data(SMSG_GUILD_QUERY_RESPONSE, (8*32+200));// we can only guess size

    data << Id;
    data << name;
    RankList::iterator itr;
    for (int i = 0 ; i < 10; ++i)                           // show always 10 ranks
    {
        if(i < m_ranks.size())
            data << m_ranks[i].name;
        else
            data << "Unused";
    }

    data << uint32(EmblemStyle);
    data << uint32(EmblemColor);
    data << uint32(BorderStyle);
    data << uint32(BorderColor);
    data << uint32(BackgroundColor);

    session->SendPacket( &data );

    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_QUERY_RESPONSE)" );
}

void Guild::SetEmblem(uint32 emblemStyle, uint32 emblemColor, uint32 borderStyle, uint32 borderColor, uint32 backgroundColor)
{
    this->EmblemStyle = emblemStyle;
    this->EmblemColor = emblemColor;
    this->BorderStyle = borderStyle;
    this->BorderColor = borderColor;
    this->BackgroundColor = backgroundColor;

    sDatabase.PExecute("UPDATE `guild` SET EmblemStyle=%u, EmblemColor=%u, BorderStyle=%u, BorderColor=%u, BackgroundColor=%u WHERE guildid = %u", EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor, Id);
}
