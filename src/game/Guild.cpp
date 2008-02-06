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
    guildbank_money = 0;
    purchased_tabs = 0;

    QueryResult *result = CharacterDatabase.Query( "SELECT MAX(`guildid`) FROM `guild`" );
    if( result )
    {
        Id = (*result)[0].GetUInt32()+1;
        delete result;
    }
    else Id = 1;

    // gname already assigned to Guild::name, use it to encode string for DB
    CharacterDatabase.escape_string(gname);

    std::string dbGINFO = GINFO;
    std::string dbMOTD = MOTD;
    CharacterDatabase.escape_string(dbGINFO);
    CharacterDatabase.escape_string(dbMOTD);

    CharacterDatabase.BeginTransaction();
    // CharacterDatabase.PExecute("DELETE FROM `guild` WHERE `guildid`='%u'", Id); - MAX(`guildid`)+1 not exist
    CharacterDatabase.PExecute("DELETE FROM `guild_rank` WHERE `guildid`='%u'", Id);
    CharacterDatabase.PExecute("DELETE FROM `guild_member` WHERE `guildid`='%u'", Id);
    CharacterDatabase.PExecute("INSERT INTO `guild` (`guildid`,`name`,`leaderguid`,`info`,`MOTD`,`createdate`,`EmblemStyle`,`EmblemColor`,`BorderStyle`,`BorderColor`,`BackgroundColor`,`BankMoney`) "
        "VALUES('%u','%s','%u', '%s', '%s', NOW(),'%u','%u','%u','%u','%u','" I64FMTD "')",
        Id, gname.c_str(), GUID_LOPART(leaderGuid), dbGINFO.c_str(), dbMOTD.c_str(), EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor, guildbank_money);
    CharacterDatabase.CommitTransaction();

    maxrank = 0;
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

    return AddMember(lGuid, (uint32)GR_GUILDMASTER);
}

bool Guild::AddMember(uint64 plGuid, uint32 plRank)
{
    if(Player::GetGuildIdFromDB(plGuid) != 0)               // player already in guild
        return false;

    // remove all player signs from another petitions
    // this will be prevent attempt joining player to many guilds and corrupt guild data integrity
    Player::RemovePetitionsAndSigns(plGuid, 9);

    // fill player data
    MemberSlot newmember;

    if(!FillPlayerData(plGuid, &newmember))                 // problems with player data collection
        return false;

    newmember.RankId = plRank;
    newmember.OFFnote = (std::string)"";
    newmember.Pnote = (std::string)"";
    newmember.logout_time = time(NULL);
    newmember.BankResetTimeMoney = 0;                       // this will force update at first query
    for (int i = 0; i < GUILD_BANK_MAX_TABS; ++i)
        newmember.BankResetTimeTab[i] = 0;
    members[GUID_LOPART(plGuid)] = newmember;

    std::string dbPnote = newmember.Pnote;
    std::string dbOFFnote = newmember.OFFnote;
    CharacterDatabase.escape_string(dbPnote);
    CharacterDatabase.escape_string(dbOFFnote);

    CharacterDatabase.PExecute("INSERT INTO `guild_member` (`guildid`,`guid`,`rank`,`Pnote`,`OFFnote`) VALUES ('%u', '%u', '%u','%s','%s')",
        Id, GUID_LOPART(plGuid), newmember.RankId, dbPnote.c_str(), dbOFFnote.c_str());

    Player* pl = objmgr.GetPlayer(plGuid);
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
    return true;
}

void Guild::SetMOTD(std::string motd)
{
    MOTD = motd;

    // motd now can be used for encoding to DB
    CharacterDatabase.escape_string(motd);
    CharacterDatabase.PExecute("UPDATE `guild` SET `MOTD`='%s' WHERE `guildid`='%u'", motd.c_str(), Id);
}

void Guild::SetGINFO(std::string ginfo)
{
    GINFO = ginfo;

    // ginfo now can be used for encoding to DB
    CharacterDatabase.escape_string(ginfo);
    CharacterDatabase.PExecute("UPDATE `guild` SET `info`='%s' WHERE `guildid`='%u'", ginfo.c_str(), Id);
}

bool Guild::LoadGuildFromDB(uint32 GuildId)
{
    if(!LoadRanksFromDB(GuildId))
        return false;

    if(!LoadMembersFromDB(GuildId))
        return false;

    QueryResult *result = CharacterDatabase.PQuery("SELECT MAX(`TabId`) FROM `guild_bank_tab` WHERE `guildid`='%u'", GuildId);
    if(result)
    {
        Field *fields = result->Fetch();
        purchased_tabs = fields[0].GetUInt8()+1;            // Because TabId begins at 0
        delete result;
    }
    else
        purchased_tabs = 0;

    LoadBankRightsFromDB(GuildId);                          // Must be after LoadRanksFromDB because it populates rank struct

    //                                                     0         1      2            3             4             5             6
    result = CharacterDatabase.PQuery("SELECT `guildid`,`name`,`leaderguid`,`EmblemStyle`,`EmblemColor`,`BorderStyle`,`BorderColor`,"
    //   7                 8      9      10           11
        "`BackgroundColor`,`info`,`MOTD`,`createdate`,`BankMoney` FROM `guild` WHERE `guildid` = '%u'", GuildId);

    if(!result)
        return false;

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
    guildbank_money = fields[11].GetUInt64();

    delete result;

    uint64 dTime = time /1000000;
    CreatedDay   = dTime%100;
    CreatedMonth = (dTime/100)%100;
    CreatedYear  = (dTime/10000)%10000;

    // If the leader does not exist attempt to promote another member
    if(!objmgr.GetPlayerAccountIdByGUID(leaderGuid ))
    {
        DelMember(leaderGuid);

        // check no members case (disbanded)
        if(members.empty())
            return false;
    }

    sLog.outDebug("Guild %u Creation time Loaded day: %u, month: %u, year: %u", GuildId, CreatedDay, CreatedMonth, CreatedYear);
    m_bankloaded = false;
    m_onlinemembers = 0;
    RenumBankLogs();
    return true;
}

bool Guild::LoadRanksFromDB(uint32 GuildId)
{
    Field *fields;
    QueryResult *result = CharacterDatabase.PQuery("SELECT `rname`,`rights`,`BankMoneyPerDay`,`rid` FROM `guild_rank` WHERE `guildid` = '%u' ORDER BY `rid` ASC", GuildId);

    if(!result)
        return false;

    do
    {
        fields = result->Fetch();
        AddRank(fields[0].GetCppString(),fields[1].GetUInt32(),fields[2].GetUInt32());
        maxrank=fields[3].GetUInt32();

    }while( result->NextRow() );
    delete result;

    return true;
}

bool Guild::LoadMembersFromDB(uint32 GuildId)
{
    //                                                                    0      1      2       3         4                    5
    QueryResult *result = CharacterDatabase.PQuery("SELECT `guild_member`.`guid`,`rank`,`Pnote`,`OFFnote`,`BankResetTimeMoney`,`BankRemMoney`,"
    //   6                    7                 8                   9                  10                  11
        "`BankResetTimeTab0`,`BankRemSlotsTab0`,`BankResetTimeTab1`,`BankRemSlotsTab1`,`BankResetTimeTab2`,`BankRemSlotsTab2`,"
    //   12                   13                14                  15                 16                  17
        "`BankResetTimeTab3`,`BankRemSlotsTab3`,`BankResetTimeTab4`,`BankRemSlotsTab4`,`BankResetTimeTab5`,`BankRemSlotsTab5`,"
    //   18
        "`logout_time` FROM `guild_member` LEFT JOIN `character` ON `character`.`guid` = `guild_member`.`guid` WHERE `guildid` = '%u'", GuildId);

    if(!result)
        return false;

    do
    {
        Field *fields = result->Fetch();
        MemberSlot newmember;
        newmember.RankId = fields[1].GetUInt32();
        uint64 guid = MAKE_GUID(fields[0].GetUInt32(),HIGHGUID_PLAYER);

        // Player does not exist
        if(!FillPlayerData(guid, &newmember))
            continue;

        newmember.Pnote                 = fields[2].GetCppString();
        newmember.OFFnote               = fields[3].GetCppString();
        newmember.BankResetTimeMoney    = fields[4].GetUInt32();
        newmember.BankRemMoney          = fields[5].GetUInt32();
        for (int i = 0; i < GUILD_BANK_MAX_TABS; ++i)
        {
            newmember.BankResetTimeTab[i] = fields[6+(2*i)].GetUInt32();
            newmember.BankRemSlotsTab[i]  = fields[7+(2*i)].GetUInt32();
        }
        newmember.logout_time           = fields[18].GetUInt64();
        members[GUID_LOPART(guid)]      = newmember;

    }while( result->NextRow() );
    delete result;

    if(members.empty())
        return false;

    return true;
}

bool Guild::FillPlayerData(uint64 guid, MemberSlot* memslot)
{
    std::string plName;
    uint32 plLevel;
    uint32 plClass;
    uint32 plZone;

    Player* pl = objmgr.GetPlayer(guid);
    if(pl)
    {
        plName  = pl->GetName();
        plLevel = pl->getLevel();
        plClass = pl->getClass();
        plZone  = pl->GetZoneId();
    }
    else
    {
        if(!objmgr.GetPlayerNameByGUID(guid, plName))       // player doesn't exist
            return false;

        plLevel = Player::GetUInt32ValueFromDB(UNIT_FIELD_LEVEL, guid);
        if(plLevel<1||plLevel>255)                          // can be at broken `data` field
        {
            sLog.outError("Player (GUID: %u) has a broken data in field `character`.`data`.",GUID_LOPART(guid));
            return false;
        }
        plZone = Player::GetZoneIdFromDB(guid);

        QueryResult *result = CharacterDatabase.PQuery("SELECT `class` FROM `character` WHERE `guid`='%u'", GUID_LOPART(guid));
        if(!result)
            return false;
        plClass = (*result)[0].GetUInt32();
        if(plClass<CLASS_WARRIOR||plClass>=MAX_CLASSES)     // can be at broken `class` field
        {
            sLog.outError("Player (GUID: %u) has a broken data in field `character`.`class`.",GUID_LOPART(guid));
            return false;
        }

        delete result;
    }

    memslot->name = plName;
    memslot->level = plLevel;
    memslot->Class = plClass;
    memslot->zoneId = plZone;

    return(true);
}

void Guild::LoadPlayerStatsByGuid(uint64 guid)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if (itr == members.end() )
        return;

    Player *pl = ObjectAccessor::FindPlayer(guid);
    if(!pl)
        return;
    itr->second.name  = pl->GetName();
    itr->second.level = pl->getLevel();
    itr->second.Class = pl->getClass();
}

void Guild::SetLeader(uint64 guid)
{
    leaderGuid = guid;
    this->ChangeRank(guid, GR_GUILDMASTER);

    CharacterDatabase.PExecute("UPDATE `guild` SET `leaderguid`='%u' WHERE `guildid`='%u'", GUID_LOPART(guid), Id);
}

void Guild::DelMember(uint64 guid, bool isDisbanding)
{
    if(this->leaderGuid == guid && !isDisbanding)
    {
        std::ostringstream ss;
        ss<<"SELECT `guid` FROM `guild_member` WHERE `guildid`='"<<Id<<"' AND `guid`!='"<<this->leaderGuid<<"' ORDER BY `rank` ASC LIMIT 1";
        QueryResult *result = CharacterDatabase.Query( ss.str().c_str() );
        if( result )
        {
            uint64 newLeaderGUID;
            Player *newLeader;
            std::string newLeaderName, oldLeaderName;

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

            // when leader non-exist (at guild load with deleted leader only) not send broadcasts
            if(objmgr.GetPlayerNameByGUID(guid, oldLeaderName))
            {
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
            }

            sLog.outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
        }
        else
        {
            this->Disband();
            return;
        }
    }

    members.erase(GUID_LOPART(guid));

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

    CharacterDatabase.PExecute("DELETE FROM `guild_member` WHERE `guid` = '%u'", GUID_LOPART(guid));
}

void Guild::ChangeRank(uint64 guid, uint32 newRank)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if( itr != members.end() )
        itr->second.RankId = newRank;

    Player *player = objmgr.GetPlayer(guid);
    if(player)
        player->SetRank(newRank);
    else
        Player::SetUInt32ValueInDB(PLAYER_GUILDRANK, newRank, guid);

    CharacterDatabase.PExecute( "UPDATE `guild_member` SET `rank`='%u' WHERE `guid`='%u'", newRank, GUID_LOPART(guid) );
}

void Guild::SetPNOTE(uint64 guid,std::string pnote)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if( itr == members.end() )
        return;

    itr->second.Pnote = pnote;

    // pnote now can be used for encoding to DB
    CharacterDatabase.escape_string(pnote);
    CharacterDatabase.PExecute("UPDATE `guild_member` SET `Pnote` = '%s' WHERE `guid` = '%u'", pnote.c_str(), itr->first);
}

void Guild::SetOFFNOTE(uint64 guid,std::string offnote)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if( itr == members.end() )
        return;
    itr->second.OFFnote = offnote;
    // offnote now can be used for encoding to DB
    CharacterDatabase.escape_string(offnote);
    CharacterDatabase.PExecute("UPDATE `guild_member` SET `OFFnote` = '%s' WHERE `guid` = '%u'", offnote.c_str(), itr->first);
}

void Guild::BroadcastToGuild(WorldSession *session, std::string msg, uint32 language)
{
    if (session && session->GetPlayer() && HasRankRight(session->GetPlayer()->GetRank(),GR_RIGHT_GCHATSPEAK))
    {
        WorldPacket data;
        ChatHandler(session).FillMessageData(&data, CHAT_MSG_GUILD, language, 0, msg.c_str());

        for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
        {
            Player *pl = ObjectAccessor::FindPlayer(MAKE_GUID(itr->first,HIGHGUID_PLAYER));

            if (pl && pl->GetSession() && HasRankRight(pl->GetRank(),GR_RIGHT_GCHATLISTEN) && !pl->HasInIgnoreList(session->GetPlayer()->GetGUID()) )
                pl->GetSession()->SendPacket(&data);
        }
    }
}

void Guild::BroadcastToOfficers(WorldSession *session, std::string msg, uint32 language)
{
    if (session && session->GetPlayer() && HasRankRight(session->GetPlayer()->GetRank(),GR_RIGHT_OFFCHATSPEAK))
    {
        for(MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
        {
            WorldPacket data;
            ChatHandler::FillMessageData(&data, session, CHAT_MSG_OFFICER, language, NULL, 0, msg.c_str(),NULL);

            Player *pl = ObjectAccessor::FindPlayer(MAKE_GUID(itr->first,HIGHGUID_PLAYER));

            if (pl && pl->GetSession() && HasRankRight(pl->GetRank(),GR_RIGHT_OFFCHATLISTEN) && !pl->HasInIgnoreList(session->GetPlayer()->GetGUID()))
                pl->GetSession()->SendPacket(&data);
        }
    }
}

void Guild::BroadcastPacket(WorldPacket *packet)
{
    for(MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        Player *player = ObjectAccessor::FindPlayer(MAKE_GUID(itr->first,HIGHGUID_PLAYER));
        if(player)
            player->GetSession()->SendPacket(packet);
    }
}

void Guild::BroadcastPacketToRank(WorldPacket *packet, uint32 rankId)
{
    for(MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        if (itr->second.RankId == rankId)
        {
            Player *player = ObjectAccessor::FindPlayer(MAKE_GUID(itr->first,HIGHGUID_PLAYER));
            if(player)
                player->GetSession()->SendPacket(packet);
        }
    }
}

void Guild::CreateRank(std::string name_,uint32 rights)
{
    if(m_ranks.size() >= GUILD_MAX_RANKS)
        return;

    AddRank(name_,rights,0);

    for (int i = 0; i < purchased_tabs; ++i)
    {
        CreateBankRightForTab(maxrank, uint8(i));
    }

    // name now can be used for encoding to DB
    CharacterDatabase.escape_string(name_);
    CharacterDatabase.PExecute( "INSERT INTO `guild_rank` (`guildid`,`rid`,`rname`,`rights`) VALUES ('%u', '%u', '%s', '%u')", Id, (++maxrank), name_.c_str(), rights );
}

void Guild::AddRank(std::string name_,uint32 rights, uint32 money)
{
    m_ranks.push_back(RankInfo(name_,rights,money));
}

void Guild::DelRank()
{
    if(m_ranks.empty())
        return;

    uint32 rank = m_ranks.size()-1;
    CharacterDatabase.PExecute("DELETE FROM `guild_rank` WHERE `rid`>='%u' AND `guildid`='%u'", (rank+1), Id);

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

void Guild::SetRankName(uint32 rankId, std::string name_)
{
    if(rankId >= m_ranks.size())
        return;

    m_ranks[rankId].name = name_;

    // name now can be used for encoding to DB
    CharacterDatabase.escape_string(name_);
    CharacterDatabase.PExecute("UPDATE `guild_rank` SET `rname`='%s' WHERE `rid`='%u' AND `guildid`='%u'", name_.c_str(), (rankId+1), Id);
}

void Guild::SetRankRights(uint32 rankId, uint32 rights)
{
    if(rankId >= m_ranks.size())
        return;

    m_ranks[rankId].rights = rights;

    CharacterDatabase.PExecute("UPDATE `guild_rank` SET `rights`='%u' WHERE `rid`='%u' AND `guildid`='%u'", rights, (rankId+1), Id);
}

void Guild::Disband()
{
    WorldPacket data(SMSG_GUILD_EVENT, 1);
    data << (uint8)GE_DISBANDED;
    this->BroadcastPacket(&data);

    while (!members.empty())
    {
        MemberList::iterator itr = members.begin();
        DelMember(MAKE_GUID(itr->first,HIGHGUID_PLAYER), true);
    }

    CharacterDatabase.BeginTransaction();
    CharacterDatabase.PExecute("DELETE FROM `guild` WHERE `guildid` = '%u'",Id);
    CharacterDatabase.PExecute("DELETE FROM `guild_rank` WHERE `guildid` = '%u'",Id);
    CharacterDatabase.PExecute("DELETE FROM `guild_bank_tab` WHERE `guildid` = '%u'",Id);
    CharacterDatabase.PExecute("DELETE FROM `guild_bank_item` WHERE `guildid` = '%u'",Id);
    CharacterDatabase.PExecute("DELETE FROM `guild_bank_right` WHERE `guildid` = '%u'",Id);
    CharacterDatabase.PExecute("DELETE FROM `guild_bank_eventlog` WHERE `guildid` = '%u'",Id);
    CharacterDatabase.CommitTransaction();
    objmgr.RemoveGuild(this);
}

void Guild::Roster(WorldSession *session)
{
    Player *pl;
                                                            // we can only guess size
    WorldPacket data(SMSG_GUILD_ROSTER, (4+MOTD.length()+1+GINFO.length()+1+4+6*8+m_ranks.size()*4+members.size()*50));
    data << (uint32)members.size();
    data << MOTD;
    data << GINFO;

    data << (uint32)m_ranks.size();
    for (RankList::iterator ritr = m_ranks.begin(); ritr != m_ranks.end();++ritr)
    {
        data << ritr->rights;
        data << ritr->BankMoneyPerDay;                      // count of: withdraw gold(gold/day) Note: in game set gold, in packet set bronze.
        for (int i = 0; i < GUILD_BANK_MAX_TABS; ++i)
        {
            data << ritr->TabRight[i];                      // for TAB_i rights: view tabs = 0x01, deposit items =0x02
            data << ritr->TabSlotPerDay[i];                 // for TAB_i count of: withdraw items(stack/day)
        }
    }
    for (MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        pl = ObjectAccessor::FindPlayer(MAKE_GUID(itr->first,HIGHGUID_PLAYER));
        if (pl)
        {
            data << pl->GetGUID();
            data << (uint8)1;
            data << (std::string)pl->GetName();
            data << itr->second.RankId;
            data << (uint8)pl->getLevel();
            data << pl->getClass();
            data << pl->GetZoneId();
            data << itr->second.Pnote;
            data << itr->second.OFFnote;
        }
        else
        {
            data << uint64(MAKE_GUID(itr->first,HIGHGUID_PLAYER));
            data << (uint8)0;
            data << itr->second.name;
            data << (uint32)itr->second.RankId;
            data << (uint8)itr->second.level;
            data << (uint8)itr->second.Class;
            data << (uint32)itr->second.zoneId;
            data << (float(time(NULL)-itr->second.logout_time) / DAY);
            data << itr->second.Pnote;
            data << itr->second.OFFnote;
        }
    }
    BroadcastPacket(&data);
    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_ROSTER)" );
}

void Guild::Query(WorldSession *session)
{
    WorldPacket data(SMSG_GUILD_QUERY_RESPONSE, (8*32+200));// we can only guess size

    data << Id;
    data << name;
    RankList::iterator itr;
    for (size_t i = 0 ; i < 10; ++i)                        // show always 10 ranks
    {
        if(i < m_ranks.size())
            data << m_ranks[i].name;
        else
            data << (uint8)0;                               // null string
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

    CharacterDatabase.PExecute("UPDATE `guild` SET EmblemStyle=%u, EmblemColor=%u, BorderStyle=%u, BorderColor=%u, BackgroundColor=%u WHERE guildid = %u", EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor, Id);
}

void Guild::UpdateLogoutTime(uint64 guid)
{
    MemberList::iterator itr = members.find(GUID_LOPART(guid));
    if (itr == members.end() )
        return;

    itr->second.logout_time = time(NULL);

    if (m_onlinemembers > 0)
        m_onlinemembers--;
    else
        UnloadGuildBank();
}

// *************************************************
// Guild Bank part
// *************************************************
// Bank content related

void Guild::DisplayGuildBankContent(WorldSession *session, uint8 TabId)
{
    WorldPacket data(SMSG_GUILD_BANK_LIST,1200);

    if (!GetBankTab(TabId))
        return;

    data << uint64(GetGuildBankMoney());
    data << uint8(TabId);
                                                            // remaining slots for today
    data << uint32(GetMemberSlotWithdrawRem(session->GetPlayer()->GetGUIDLow(), TabId));
    data << uint8(0);                                       // Tell client this is a tab content packet

    data << uint8(GUILD_BANK_MAX_SLOTS);

    for (int i=0; i<GUILD_BANK_MAX_SLOTS; ++i)
    {
        Item *pItem = GetBankTab(TabId)->Slots[i];
        uint32 entry;
        if (pItem)
            entry = pItem->GetEntry();
        else
            entry = 0;
        data << uint8(i);
        data << entry;
        if (entry)
        {
            data << (uint32) pItem->GetItemRandomPropertyId();  // random item property id +8
            if (pItem->GetItemRandomPropertyId())
                data << (uint32) pItem->GetItemSuffixFactor();  // SuffixFactor +4
            data << (uint8)GetBankTab(TabId)->Slots[i]->GetCount(); // +12 // ITEM_FIELD_STACK_COUNT
            data << uint32(0);                                  // +16 // Unknown value
            //data << uint8(0);                                 
            uint32 Enchant0 = pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + PERM_ENCHANTMENT_SLOT + ENCHANTMENT_ID_OFFSET);
            if (Enchant0)
            {
                data << uint8(1);                               // nb of enchantements (max 3)
                data << uint8(0);                               // enchantment slot (range: 0:2)
                data << (uint32)Enchant0;                       // enchantment id
            }
            else
                data << uint8(0);
        }
    }
    session->SendPacket(&data);
    sLog.outDebug("WORLD: Sent (SMSG_GUILD_BANK_LIST)");
}

// If same is return: failed or just modified, if different: player has to store, null: stored
Item* Guild::StoreItem(uint8 TabId, uint8* SlotId, Item* pItem) //, uint8 StackAmount)
{
    if (TabId >= m_TabListMap.size() || ((*SlotId) >= GUILD_BANK_MAX_SLOTS && (*SlotId) != 0xFF))
        return pItem;

    if (!pItem)                                             // Clear slot and return item if any
    {
        if (*SlotId >= GUILD_BANK_MAX_SLOTS)                // Should never happen
            return pItem;
        Item* BankItem = m_TabListMap[TabId]->Slots[*SlotId];
        m_TabListMap[TabId]->Slots[*SlotId] = NULL;
        return BankItem;
    }

    // Find a compatible slot when right-click (slot 255) and stack/store or return fail
    uint32 StackNeed = pItem->GetMaxStackCount() > pItem->GetCount() ? pItem->GetCount() : 0;
    if (*SlotId == 0xFF)
    {
        uint8 firstFree = 0xFF, firstStack = 0xFF;
        for (int i = GUILD_BANK_MAX_SLOTS-1; i >= 0; --i)
        {
            if (!m_TabListMap[TabId]->Slots[i])
                firstFree = i;
            if (StackNeed && m_TabListMap[TabId]->Slots[i] && m_TabListMap[TabId]->Slots[i]->GetEntry() == pItem->GetEntry() && StackNeed <= m_TabListMap[TabId]->Slots[i]->GetMaxStackCount() - m_TabListMap[TabId]->Slots[i]->GetCount())
                firstStack = i;
        }
        if (firstStack != 0xFF)
        {
            m_TabListMap[TabId]->Slots[firstStack]->SetCount(m_TabListMap[TabId]->Slots[firstStack]->GetCount() + StackNeed);
            *SlotId = firstStack;
            return NULL;                                    // Stacked
        }
        else if (firstFree != 0xFF)
        {
            m_TabListMap[TabId]->Slots[firstFree] = pItem;
            *SlotId = firstFree;
            pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, 0);
            pItem->SetUInt64Value(ITEM_FIELD_OWNER, 0);
            return NULL;                                    // Stored
        }
        else
            return pItem;                                   // Tab full
    }

    if (!m_TabListMap[TabId]->Slots[*SlotId])               // Empty slot? Then just store
    {
        m_TabListMap[TabId]->Slots[*SlotId] = pItem;
        pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, 0);
        pItem->SetUInt64Value(ITEM_FIELD_OWNER, 0);
        return NULL;
    }

    Item *BankItem = m_TabListMap[TabId]->Slots[*SlotId];   // Just get a shorter name for it

    if (pItem->GetEntry() != BankItem->GetEntry() || BankItem->GetCount() >= BankItem->GetMaxStackCount())
    {                                                       // Item swap because different entry or full stack
        m_TabListMap[TabId]->Slots[*SlotId] = pItem;
        pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, 0);
        pItem->SetUInt64Value(ITEM_FIELD_OWNER, 0);
        return BankItem;
    }
    
    // Same entry now with not full stack
    {                                                       // there is a stack possibility
        uint32 StackSpace = BankItem->GetMaxStackCount() - BankItem->GetCount();
        if (StackSpace >= StackNeed)                        // Fully stackable
        {
            BankItem->SetCount(BankItem->GetCount() + StackNeed);
            return NULL;
        }
        else                                                // No place
            return pItem;
    }

    return pItem;                                           // Fail
}

Item* Guild::GetItem(uint8 TabId, uint8 SlotId)
{
    if (TabId >= m_TabListMap.size() || SlotId >= GUILD_BANK_MAX_SLOTS)
        return NULL;
    if (m_TabListMap[TabId]->Slots[SlotId]!=0)
        return m_TabListMap[TabId]->Slots[SlotId];
    return NULL;
}

void Guild::EmptyBankSlot(uint8 TabId, uint8 SlotId)
{
    m_TabListMap[TabId]->Slots[SlotId]=0;
    CharacterDatabase.PExecute("DELETE FROM `guild_bank_item` WHERE `guildid`='%u' AND `TabId`='%u' AND `SlotId`='%u'",
        Id, uint32(TabId), uint32(SlotId));
}

// *************************************************
// Tab related

void Guild::DisplayGuildBankTabsInfo(WorldSession *session)
{
    // Time to load bank if not already done
    if (!m_bankloaded)
        LoadGuildBankFromDB();

    WorldPacket data(SMSG_GUILD_BANK_LIST, 500);

    data << uint64(GetGuildBankMoney());
    data << uint8(0);                                       // TabInfo packet must be for TabId 0
    data << uint32(0xFFFFFFFF);                             // bit 9 must be set for this packet to work
    data << uint8(1);                                       // Tell Client this is a TabInfo packet

    data << uint8(purchased_tabs);                          // here is the number of tabs

    for(int i = 0; i < purchased_tabs; ++i)
    {
        data << m_TabListMap[i]->Name.c_str();
        data << m_TabListMap[i]->Icon.c_str();
    }
    data << uint8(0);                                       // Do not send tab content
    session->SendPacket(&data);
    sLog.outDebug("WORLD: Sent (SMSG_GUILD_BANK_LIST)");
}

void Guild::CreateNewBankTab()
{
    if (purchased_tabs >= GUILD_BANK_MAX_TABS)
        return;

    purchased_tabs++;

    GuildBankTab* AnotherTab = new GuildBankTab;
    memset(AnotherTab->Slots, 0, GUILD_BANK_MAX_SLOTS * sizeof(Item*));
    m_TabListMap.resize(purchased_tabs);
    m_TabListMap[purchased_tabs-1] = AnotherTab;

    CharacterDatabase.BeginTransaction();
    CharacterDatabase.PExecute("DELETE FROM `guild_bank_tab` WHERE `guildid`='%u' AND `TabId`='%u'", Id, uint32(purchased_tabs-1));
    CharacterDatabase.PExecute("INSERT INTO `guild_bank_tab` (`guildid`,`TabId`) VALUES ('%u','%u')", Id, uint32(purchased_tabs-1));
    CharacterDatabase.CommitTransaction();
}

void Guild::SetGuildBankTabInfo(uint8 TabId, std::string Name, std::string Icon)
{
    if (TabId >= GUILD_BANK_MAX_TABS)
        return;
    if (TabId >= m_TabListMap.size())
        return;

    if (!m_TabListMap[TabId])
        return;

    m_TabListMap[TabId]->Name = Name;
    m_TabListMap[TabId]->Icon = Icon;
    CharacterDatabase.PExecute("UPDATE `guild_bank_tab` SET `TabName`='%s',`TabIcon`='%s' WHERE `guildid`='%u' AND `TabId`='%u'", Name.c_str(), Icon.c_str(), Id, uint32(TabId));
}

void Guild::CreateBankRightForTab(uint32 rankId, uint8 TabId)
{
    sLog.outErrorDb("CreateBankRightForTab. rank: %u, TabId: %u", rankId, uint32(TabId));
    if (rankId >= m_ranks.size() || TabId >= GUILD_BANK_MAX_TABS)
        return;
  
    m_ranks[rankId].TabRight[TabId]=0;
    m_ranks[rankId].TabSlotPerDay[TabId]=0;

    CharacterDatabase.PExecute("REPLACE INTO `guild_bank_right` (`guildid`,`TabId`,`rid`) VALUES ('%u','%u','%u')", Id, uint32(TabId), rankId);
}

uint8 Guild::GetBankRights(uint32 rankId, uint8 TabId)
{
    if(rankId >= m_ranks.size() || TabId >= GUILD_BANK_MAX_TABS)
        return 0;

    return m_ranks[rankId].TabRight[TabId];
}

// *************************************************
// Guild bank loading/unloading related

// This load should be called when the bank is first accessed by a guild member
void Guild::LoadGuildBankFromDB()
{
    if (m_bankloaded)
        return;

    m_bankloaded = true;
    LoadGuildBankEventLogFromDB();

    //                                                     0       1         2
    QueryResult *result = CharacterDatabase.PQuery("SELECT `TabId`,`TabName`,`TabIcon` FROM `guild_bank_tab` WHERE `guildid`='%u' ORDER BY `TabId`", Id);
    if(!result)
    {
        purchased_tabs = 0;     
        return;
    }

    m_TabListMap.resize(purchased_tabs);
    do
    {
        Field *fields = result->Fetch();
        uint8 TabId = fields[0].GetUInt8();

        GuildBankTab *NewTab = new GuildBankTab;
        memset(NewTab->Slots, 0, GUILD_BANK_MAX_SLOTS * sizeof(Item*));
    
        NewTab->Name = fields[1].GetCppString();
        NewTab->Icon = fields[2].GetCppString();

        m_TabListMap[TabId] = NewTab;
    }while( result->NextRow() );

    delete result;

    //                                        0       1        2
    result = CharacterDatabase.PQuery("SELECT `TabId`,`SlotId`,`item_guid`,`item_entry` FROM `guild_bank_item` WHERE `guildid`='%u' ORDER BY `TabId`", Id);
    if(!result)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint8 TabId = fields[0].GetUInt8();
        uint8 SlotId = fields[1].GetUInt8();
        uint32 ItemGuid = fields[2].GetUInt32();
        uint32 ItemEntry = fields[3].GetUInt32();

        if (TabId >= purchased_tabs || TabId >= GUILD_BANK_MAX_TABS)
        {
            sLog.outError( "Guild::LoadGuildBankFromDB: Invalid tab for item (GUID: %u id: #%u) in guild bank, skipped.", ItemGuid,ItemEntry);
            continue;
        }

        if (SlotId >= GUILD_BANK_MAX_SLOTS)
        {
            sLog.outError( "Guild::LoadGuildBankFromDB: Invalid slot for item (GUID: %u id: #%u) in guild bank, skipped.", ItemGuid,ItemEntry);
            continue;
        }

        ItemPrototype const *proto = objmgr.GetItemPrototype(ItemEntry);

        if(!proto)
        {
            sLog.outError( "Guild::LoadGuildBankFromDB: Unknown item (GUID: %u id: #%u) in guild bank, skipped.", ItemGuid,ItemEntry);
            continue;
        }

        Item *pItem = NewItemOrBag(proto);
        if (pItem->LoadFromDB(ItemGuid, 0))
        {
            pItem->AddToWorld();
            m_TabListMap[TabId]->Slots[SlotId] = pItem;
        }
        else
        {
            CharacterDatabase.PExecute("DELETE FROM `guild_bank_item` WHERE `guildid`='%u' AND `TabId`='%u' AND `SlotId`='%u'", Id, uint32(TabId), uint32(SlotId));
            sLog.outError("Item GUID %u not found in `item_instance`, deleting from Guild Bank!", ItemGuid);
        }
    }while( result->NextRow() );

    delete result;
}

// This unload should be called when the last member of the guild gets offline
void Guild::UnloadGuildBank()
{
    if (!m_bankloaded)
        return;
    for (uint8 i = 0 ; i < purchased_tabs ; ++i )
    {
        for (uint8 j = 0 ; j < GUILD_BANK_MAX_SLOTS ; ++j)
        {
            if (m_TabListMap[i]->Slots[j])
            {
                m_TabListMap[i]->Slots[j]->RemoveFromWorld();
                delete m_TabListMap[i]->Slots[j];
            }
        }
        delete m_TabListMap[i];
    }
    m_TabListMap.clear();
    
    UnloadGuildBankEventLog();
    m_bankloaded = false;
}

// *************************************************
// Money deposit/withdraw related

void Guild::SendMoneyInfo(WorldSession *session, uint32 LowGuid)
{
    WorldPacket data(MSG_GUILD_BANK_GET_MONEY_AMOUNT, 4);
    data << uint32(GetMemberMoneyWithdrawRem(LowGuid));
    session->SendPacket(&data);
    sLog.outDebug("WORLD: Sent (MSG_GUILD_BANK_GET_MONEY_AMOUNT)");
}

bool Guild::MemberMoneyWithdraw(uint32 amount, uint32 LowGuid)
{
    uint32 MoneyWithDrawRight = GetMemberMoneyWithdrawRem(LowGuid);

    if (MoneyWithDrawRight < amount || GetGuildBankMoney() < amount)
        return false;

    SetBankMoney(GetGuildBankMoney()-amount);

    if (MoneyWithDrawRight < WITHDRAW_MONEY_UNLIMITED)
    {
        MemberList::iterator itr = members.find(LowGuid);
        if (itr == members.end() )
            return false;
        itr->second.BankRemMoney -= amount;
        CharacterDatabase.PExecute("UPDATE `guild_member` SET `BankRemMoney`='%u' WHERE `guildid`='%u' AND `guid`='%u'",
            itr->second.BankRemMoney, Id, LowGuid);
    }
    return true;
}

void Guild::SetBankMoney(int64 money)
{
    if (money < 0)                          // I don't know how this happens, it does!!
        money = 0;
    guildbank_money = money;

    CharacterDatabase.PExecute("UPDATE `guild` SET `BankMoney`='" I64FMTD "' WHERE `guildid`='%u'", money, Id);
}

// *************************************************
// Item per day and money per day related

bool Guild::MemberItemWithdraw(uint8 TabId, uint32 LowGuid)
{
    uint32 SlotsWithDrawRight = GetMemberSlotWithdrawRem(LowGuid, TabId);

    if (SlotsWithDrawRight == 0)
        return false;

    if (SlotsWithDrawRight < WITHDRAW_SLOT_UNLIMITED)
    {
        MemberList::iterator itr = members.find(LowGuid);
        if (itr == members.end() )
            return false;
        itr->second.BankRemSlotsTab[TabId]--;
        CharacterDatabase.PExecute("UPDATE `guild_member` SET `BankRemSlotsTab%u`='%u' WHERE `guildid`='%u' AND `guid`='%u'",
            uint32(TabId), itr->second.BankRemSlotsTab[TabId], Id, LowGuid);
    }
    return true;
}

uint32 Guild::GetMemberSlotWithdrawRem(uint32 LowGuid, uint8 TabId)
{
    MemberList::iterator itr = members.find(LowGuid);
    if (itr == members.end() )
        return 0;

    if (itr->second.RankId == GR_GUILDMASTER)
        return WITHDRAW_SLOT_UNLIMITED;

    uint32 curTime = uint32(time(NULL)/60);                     // minutes
    if (curTime - itr->second.BankResetTimeTab[TabId] >= 1440)  // 24 hours
    {
        itr->second.BankResetTimeTab[TabId] = curTime;
        itr->second.BankRemSlotsTab[TabId] = GetBankSlotPerDay(itr->second.RankId, TabId);
        CharacterDatabase.PExecute("UPDATE `guild_member` SET `BankResetTimeTab%u`='%u',`BankRemSlotsTab%u`='%u' WHERE `guildid`='%u' AND `guid`='%u'",
            uint32(TabId), itr->second.BankResetTimeTab[TabId], uint32(TabId), itr->second.BankRemSlotsTab[TabId], Id, LowGuid);
    }
    return itr->second.BankRemSlotsTab[TabId];
}

uint32 Guild::GetMemberMoneyWithdrawRem(uint32 LowGuid)
{
    MemberList::iterator itr = members.find(LowGuid);
    if (itr == members.end() )
        return 0;


    if (itr->second.RankId == GR_GUILDMASTER)
        return WITHDRAW_MONEY_UNLIMITED;

    uint32 curTime = uint32(time(NULL)/MINUTE);             // minutes
    if (curTime > itr->second.BankResetTimeMoney + 24*HOUR/MINUTE) // 24 hours
    {
        itr->second.BankResetTimeMoney = curTime;
        itr->second.BankRemMoney = GetBankMoneyPerDay(itr->second.RankId);
        CharacterDatabase.PExecute("UPDATE `guild_member` SET `BankResetTimeMoney`='%u',`BankRemMoney`='%u' WHERE `guildid`='%u' AND `guid`='%u'",
            itr->second.BankResetTimeMoney, itr->second.BankRemMoney, Id, LowGuid);
    }
    return itr->second.BankRemMoney;
}

void Guild::SetBankMoneyPerDay(uint32 rankId, uint32 money)
{
    if (rankId >= m_ranks.size())
        return;

    if (rankId == GR_GUILDMASTER)
        money = WITHDRAW_MONEY_UNLIMITED;

    m_ranks[rankId].BankMoneyPerDay = money;

    for (MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
        if (itr->second.RankId == rankId)
            itr->second.BankResetTimeMoney = 0;

    CharacterDatabase.PExecute("UPDATE `guild_rank` SET `BankMoneyPerDay`='%u' WHERE `rid`='%u' AND `guildid`='%u'", money, (rankId+1), Id);
    CharacterDatabase.PExecute("UPDATE `guild_member` SET `BankResetTimeMoney`='0' WHERE `guildid`='%u' AND `rank`='%u'", Id, rankId);
}

void Guild::SetBankRightsAndSlots(uint32 rankId, uint8 TabId, uint32 right, uint32 nbSlots, bool db)
{
    if(rankId >= m_ranks.size() ||
        TabId >= GUILD_BANK_MAX_TABS ||
        TabId >= purchased_tabs)
        return;

    if (rankId == GR_GUILDMASTER)
    {
        nbSlots = WITHDRAW_SLOT_UNLIMITED;
        right = GUILD_BANK_RIGHT_FULL;
    }

    m_ranks[rankId].TabSlotPerDay[TabId]=nbSlots;
    m_ranks[rankId].TabRight[TabId]=right;

    if (db)
    {
        for (MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
            if (itr->second.RankId == rankId)
                for (int i = 0; i < GUILD_BANK_MAX_TABS; ++i)
                    itr->second.BankResetTimeTab[i] = 0;
        CharacterDatabase.PExecute("REPLACE INTO `guild_bank_right` SET `guildid`='%u',`TabId`='%u',`rid`='%u',`Right`='%u',`SlotPerDay`='%u'", Id, uint32(TabId), rankId, m_ranks[rankId].TabRight[TabId], m_ranks[rankId].TabSlotPerDay[TabId]);
        CharacterDatabase.PExecute("UPDATE `guild_member` SET `BankResetTimeTab%u`='0' WHERE `guildid`='%u' AND `rank`='%u'", uint32(TabId), Id, rankId);
    }
}

uint32 Guild::GetBankMoneyPerDay(uint32 rankId)
{
    if(rankId >= m_ranks.size())
        return 0;

    if (rankId == GR_GUILDMASTER)
        return WITHDRAW_MONEY_UNLIMITED;
    return m_ranks[rankId].BankMoneyPerDay;
}

uint32 Guild::GetBankSlotPerDay(uint32 rankId, uint8 TabId)
{
    if(rankId >= m_ranks.size() || TabId >= GUILD_BANK_MAX_TABS)
        return 0;

    if (rankId == GR_GUILDMASTER)
        return WITHDRAW_SLOT_UNLIMITED;
    return m_ranks[rankId].TabSlotPerDay[TabId];
}

// *************************************************
// Rights per day related

void Guild::LoadBankRightsFromDB(uint32 GuildId)
{
    //                                                     0       1      2       3
    QueryResult *result = CharacterDatabase.PQuery("SELECT `TabId`,`rid`,`Right`,`SlotPerDay` FROM `guild_bank_right` WHERE `guildid` = '%u' ORDER BY `TabId`", GuildId);

    if(!result)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint8 TabId = fields[0].GetUInt8();
        uint32 rankId = fields[1].GetUInt32();
        uint16 right = fields[2].GetUInt16();
        uint16 SlotPerDay = fields[3].GetUInt16();
        SetBankRightsAndSlots(rankId, TabId, right, SlotPerDay, false);
        
    }while( result->NextRow() );
    delete result;

    return;
}

// *************************************************
// Bank log related

void Guild::LoadGuildBankEventLogFromDB()
{
    //                                                     0         1          2       3            4             5                6           7
    QueryResult *result = CharacterDatabase.PQuery("SELECT `LogGuid`,`LogEntry`,`TabId`,`PlayerGuid`,`ItemOrMoney`,`ItemStackCount`,`DestTabId`,`TimeStamp` FROM `guild_bank_eventlog` WHERE `guildid`='%u' ORDER BY `TimeStamp` DESC", Id);
    if(!result)
        return;

    do
    {
        Field *fields = result->Fetch();
        GuildBankEvent *NewEvent = new GuildBankEvent;
        
        NewEvent->LogGuid = fields[0].GetUInt32();
        NewEvent->LogEntry = fields[1].GetUInt8();
        uint8 TabId = fields[2].GetUInt8();
        NewEvent->PlayerGuid = fields[3].GetUInt32();
        NewEvent->ItemOrMoney = fields[4].GetUInt32();
        NewEvent->ItemStackCount = fields[5].GetUInt8();
        NewEvent->DestTabId = fields[6].GetUInt8();
        NewEvent->TimeStamp = fields[7].GetUInt64();
        if (NewEvent->LogEntry == GUILD_BANK_LOG_DEPOSIT_MONEY || 
            NewEvent->LogEntry == GUILD_BANK_LOG_WITHDRAW_MONEY || 
            NewEvent->LogEntry == GUILD_BANK_LOG_REPAIR_MONEY)
            m_GuildBankEventLog_Money.push_front(NewEvent);
        else
            m_GuildBankEventLog_Item[TabId].push_front(NewEvent);

    }while( result->NextRow() );
    delete result;

    // Check lists size in case to many event entries in db for a tab or for money
    // This cases can happen only if a crash occured somewhere and table has too many log entries
    if (m_GuildBankEventLog_Money.size() > GUILD_BANK_MAX_LOGS)
    {
        do
        {
            GuildBankEvent *EventLogEntry = *(m_GuildBankEventLog_Money.begin());
            m_GuildBankEventLog_Money.pop_front();
            CharacterDatabase.PExecute("DELETE FROM `guild_bank_eventlog` WHERE `guildid`='%u' AND `LogGuid`='%u'", 
                Id, uint32(EventLogEntry->LogEntry), EventLogEntry->LogGuid);
            delete EventLogEntry;
        }while( m_GuildBankEventLog_Money.size() > GUILD_BANK_MAX_LOGS );
    }
    for (int i = 0; i < GUILD_BANK_MAX_TABS; ++i)
    {
        if (m_GuildBankEventLog_Item[i].size() > GUILD_BANK_MAX_LOGS)
        {
            do
            {
                GuildBankEvent *EventLogEntry = *(m_GuildBankEventLog_Item[i].begin());
                m_GuildBankEventLog_Item[i].pop_front();
                CharacterDatabase.PExecute("DELETE FROM `guild_bank_eventlog` WHERE `guildid`='%u' AND `LogGuid`='%u'", 
                    Id, uint32(EventLogEntry->LogEntry), EventLogEntry->LogGuid);
                delete EventLogEntry;
            }while( m_GuildBankEventLog_Item[i].size() > GUILD_BANK_MAX_LOGS );
        }
    }
}

void Guild::UnloadGuildBankEventLog()
{
    GuildBankEvent *EventLogEntry;
    if (m_GuildBankEventLog_Money.size()>0)
    {
        do
        {
            EventLogEntry = *(m_GuildBankEventLog_Money.begin());
            m_GuildBankEventLog_Money.pop_front();
            delete EventLogEntry;
        }while( m_GuildBankEventLog_Money.size()>0 );
    }
    for (int i = 0; i < GUILD_BANK_MAX_TABS; ++i)
    {
        if (m_GuildBankEventLog_Item[i].size()>0)
        {
            do
            {
                EventLogEntry = *(m_GuildBankEventLog_Item[i].begin());
                m_GuildBankEventLog_Item[i].pop_front();
                delete EventLogEntry;
            }while( m_GuildBankEventLog_Item[i].size()>0 );
        }
    }
}

void Guild::DisplayGuildBankLogs(WorldSession *session, uint8 TabId)
{
    if (TabId > GUILD_BANK_MAX_TABS)
        return;

    if (TabId == GUILD_BANK_MAX_TABS)
    {
        // Here we display money logs
        WorldPacket data(MSG_GUILD_BANK_LOG, m_GuildBankEventLog_Money.size()*(4*4+1)+1+1);
        data << uint8(TabId);                               // Here GUILD_BANK_MAX_TABS
        data << uint8(m_GuildBankEventLog_Money.size());    // number of log entries
        for (GuildBankEventLog::const_iterator itr = m_GuildBankEventLog_Money.begin(); itr != m_GuildBankEventLog_Money.end(); ++itr)
        {
            data << uint8((*itr)->LogEntry);
            data << uint32((*itr)->PlayerGuid);
            data << uint32(0);                              // second part of 64bits guid
            data << uint32((*itr)->ItemOrMoney);
            data << uint32(time(NULL)-(*itr)->TimeStamp);
        }
        session->SendPacket(&data);
    }
    else
    {
        // here we display current tab logs
        WorldPacket data(MSG_GUILD_BANK_LOG, m_GuildBankEventLog_Item[TabId].size()*(4*4+1+1)+1+1);
        data << uint8(TabId);                               // Here a real Tab Id
        data << uint8(m_GuildBankEventLog_Item[TabId].size());     // number of log entries
        for (GuildBankEventLog::const_iterator itr = m_GuildBankEventLog_Item[TabId].begin(); itr != m_GuildBankEventLog_Item[TabId].end(); ++itr)
        {
            data << uint8((*itr)->LogEntry);
            data << uint32((*itr)->PlayerGuid);
            data << uint32(0);                              // second part of 64 bits guid
            data << uint32((*itr)->ItemOrMoney);
            data << uint8((*itr)->ItemStackCount);
            if ((*itr)->LogEntry == GUILD_BANK_LOG_MOVE_ITEM || (*itr)->LogEntry == GUILD_BANK_LOG_MOVE_ITEM2)
                data << uint8((*itr)->DestTabId);           // moved tab
            data << uint32(time(NULL)-(*itr)->TimeStamp);
        }
        session->SendPacket(&data);
    }
    sLog.outDebug("WORLD: Sent (MSG_GUILD_BANK_LOG)");
}

void Guild::LogBankEvent(uint8 LogEntry, uint8 TabId, uint32 PlayerGuidLow, uint32 ItemOrMoney, uint8 ItemStackCount, uint8 DestTabId)
{
    GuildBankEvent *NewEvent = new GuildBankEvent;

    NewEvent->LogGuid = LogMaxGuid++;
    NewEvent->LogEntry = LogEntry;
    NewEvent->PlayerGuid = PlayerGuidLow;
    NewEvent->ItemOrMoney = ItemOrMoney;
    NewEvent->ItemStackCount = ItemStackCount;
    NewEvent->DestTabId = DestTabId;
    NewEvent->TimeStamp = uint32(time(NULL));

    if (LogEntry == GUILD_BANK_LOG_DEPOSIT_MONEY || 
        LogEntry == GUILD_BANK_LOG_WITHDRAW_MONEY ||
        LogEntry == GUILD_BANK_LOG_REPAIR_MONEY)
    {
        if (m_GuildBankEventLog_Money.size() > GUILD_BANK_MAX_LOGS)
        {
            GuildBankEvent *OldEvent = *(m_GuildBankEventLog_Money.begin());
            m_GuildBankEventLog_Money.pop_front();
            CharacterDatabase.PExecute("DELETE FROM `guild_bank_eventlog` WHERE `guildid`='%u' AND `LogGuid`='%u'", Id, OldEvent->LogGuid);
            delete OldEvent;
        }
        m_GuildBankEventLog_Money.push_back(NewEvent);
    }
    else
    {
        if (m_GuildBankEventLog_Item[TabId].size() > GUILD_BANK_MAX_LOGS)
        {
            GuildBankEvent *OldEvent = *(m_GuildBankEventLog_Item[TabId].begin());
            m_GuildBankEventLog_Item[TabId].pop_front();
            CharacterDatabase.PExecute("DELETE FROM `guild_bank_eventlog` WHERE `guildid`='%u' AND `LogGuid`='%u'", Id, OldEvent->LogGuid);         
            delete OldEvent;
        }
        m_GuildBankEventLog_Item[TabId].push_back(NewEvent);
    }
    CharacterDatabase.PExecute("INSERT INTO `guild_bank_eventlog` (`guildid`,`LogGuid`,`LogEntry`,`TabId`,`PlayerGuid`,`ItemOrMoney`,`ItemStackCount`,`DestTabId`,`TimeStamp`) VALUES ('%u','%u','%u','%u','%u','%u','%u','%u','" I64FMTD "')",
        Id, NewEvent->LogGuid, uint32(NewEvent->LogEntry), uint32(TabId), NewEvent->PlayerGuid, NewEvent->ItemOrMoney, uint32(NewEvent->ItemStackCount), uint32(NewEvent->DestTabId), NewEvent->TimeStamp);
}

// This will renum guids used at load to prevent always going up until infinit
void Guild::RenumBankLogs()
{
    QueryResult *result = CharacterDatabase.PQuery("SELECT `LogGuid` FROM `guild_bank_eventlog` WHERE `guildid` = '%u' ORDER BY `LogGuid`", Id);

    if(!result)
        return;

    LogMaxGuid = 1;
    CharacterDatabase.BeginTransaction();
    do
    {
        Field *fields = result->Fetch();
        uint32 OldGuid = fields[0].GetUInt32();
        CharacterDatabase.PExecute("UPDATE `guild_bank_eventlog` SET `LogGuid`='%u' WHERE `guildid`='%u' AND `LogGuid`='%u'", LogMaxGuid, Id, OldGuid);
        ++LogMaxGuid;
    }while( result->NextRow() );
    delete result;
    CharacterDatabase.CommitTransaction();
}











