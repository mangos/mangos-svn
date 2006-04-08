/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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

void Guild::create(uint64 lGuid, std::string gname)
{
    Player *pl;
    MemberSlot *newmember;
    std::string rname;

    pl = ObjectAccessor::Instance().FindPlayer(lGuid);
    if(!pl) return;

    leaderGuid = lGuid;
    name = gname;
    MOTD = "No message set.";

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

    sLog.outDebug("GUILD: creating guild %s to leader:%d", gname.c_str(), leaderGuid);

    QueryResult *result = sDatabase.PQuery( "SELECT MAX(guildid) FROM guilds;" );
    if( result )
    {
        Id = (*result)[0].GetUInt32()+1;
        delete result;
    }
    else Id = 1;

    pl->SetInGuild(Id);
    pl->SetRank( GR_GUILDMASTER );

    newmember = new MemberSlot;
    newmember->guid = leaderGuid;
    newmember->RankId = GR_GUILDMASTER;
    newmember->Pnote = "";
    newmember->OFFnote = "";
    AddMember(newmember);
    SaveGuildToDB();

}

void Guild::LoadGuildFromDB(uint32 GuildId)
{

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `guilds` where guildid = '%u';", GuildId);

    if(!result)
        return;

    Field *fields = result->Fetch();

    Id = fields[0].GetUInt32();
    name = fields[1].GetString();
    leaderGuid = fields[2].GetUInt64();
    EmblemStyle = fields[3].GetUInt32();
    EmblemColor = fields[4].GetUInt32();
    BorderStyle = fields[5].GetUInt32();
    BorderColor = fields[6].GetUInt32();
    BackgroundColor = fields[7].GetUInt32();
    MOTD = fields[8].GetString();

    delete result;

    QueryResult *result1 = sDatabase.PQuery("SELECT DATE_FORMAT(createdate,\"%d\") FROM guilds WHERE guildid = '%u';", GuildId);
    if(!result1) return;
    fields = result1->Fetch();
    CreatedDay = fields[0].GetUInt32();

    delete result1;

    QueryResult *result2 = sDatabase.PQuery("SELECT DATE_FORMAT(createdate,\"%m\") FROM guilds WHERE guildid = '%u';", GuildId);
    if(!result2) return;
    fields = result2->Fetch();
    CreatedMonth = fields[0].GetUInt32();

    delete result2;

    QueryResult *result3 = sDatabase.PQuery("SELECT DATE_FORMAT(createdate,\"%Y\") FROM guilds WHERE guildid = '%u';", GuildId);
    if(!result3) return;
    fields = result3->Fetch();
    CreatedYear = fields[0].GetUInt32();

    delete result3;

    LoadRanksFromDB(GuildId);
    LoadMembersFromDB(GuildId);
}

void Guild::LoadRanksFromDB(uint32 GuildId)
{
    Field *fields;
    QueryResult *result = sDatabase.PQuery("SELECT * FROM `guilds_ranks` where guildid = '%u';", GuildId);

    if(!result) return;

    do
    {
        fields = result->Fetch();
        CreateRank(fields[1].GetString(),fields[2].GetUInt32());

    }while( result->NextRow() );
    delete result;
}

void Guild::LoadMembersFromDB(uint32 GuildId)
{
    Field *fields;
    Player *pl;
    MemberSlot *newmember;

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `guilds_members` where guildid = '%u';", GuildId);

    if(!result)
        return;

    do
    {
        fields = result->Fetch();
        newmember = new MemberSlot;
        newmember->guid = fields[1].GetUInt64();
        newmember->RankId = fields[2].GetUInt32();
        pl = ObjectAccessor::Instance().FindPlayer(newmember->guid);
        if(!pl || !pl->IsInWorld()) Loadplayerstats(newmember);
        newmember->Pnote = fields[3].GetString();
        newmember->OFFnote = fields[4].GetString();
        AddMember(newmember);

    }while( result->NextRow() );
    delete result;
}

void Guild::Loadplayerstats(MemberSlot *memslot)
{
    Field *fields;

    // row 'level' doesn't exist in characters table
    QueryResult *result = sDatabase.PQuery("SELECT (name, level, class, zoneId) FROM characters WHERE guid = '%lu';", (unsigned long)memslot->guid);

    if(!result) return;

    fields = result->Fetch();

    memslot->name  = fields[0].GetString();
    memslot->level = fields[1].GetUInt8();
    memslot->Class = fields[2].GetUInt8();
    memslot->zoneId = fields[3].GetUInt32();
    delete result;
}

void Guild::Loadplayerstatsbyguid(uint64 guid)
{
    Player *pl;

    std::list<MemberSlot*>::iterator itr;
    for (itr = members.begin(); itr != members.end();itr++)
    {
        if ((*itr)->guid == guid)
        {
            pl = ObjectAccessor::Instance().FindPlayer((*itr)->guid);
            if(!pl)break;
            (*itr)->name  = pl->GetName();
            (*itr)->level = pl->getLevel();
            (*itr)->Class = pl->getClass();
        }
    }

}

void Guild::DelMember(uint64 guid)
{
    std::list<MemberSlot*>::iterator itr;
    for (itr = members.begin(); itr != members.end();itr++)
    {
        if ((*itr)->guid == guid)
        {
            members.erase(itr);
            break;
        }
    }
}

void Guild::SetPNOTE(uint64 guid,std::string pnote)
{
    std::list<MemberSlot*>::iterator itr;
    for (itr = members.begin(); itr != members.end();itr++)
    {
        if ((*itr)->guid == guid)
        {
            (*itr)->Pnote = pnote;
            break;
        }
    }
}

void Guild::SetOFFNOTE(uint64 guid,std::string offnote)
{
    std::list<MemberSlot*>::iterator itr;
    for (itr = members.begin(); itr != members.end();itr++)
    {
        if ((*itr)->guid == guid)
        {
            (*itr)->OFFnote = offnote;
            break;
        }
    }
}

void Guild::SaveGuildToDB()
{
    sDatabase.PExecute("DELETE FROM guilds WHERE guildid = '%u'",Id);
    sDatabase.PExecute("INSERT INTO guilds VALUES('%u','%s','%u', '%u', '%u', '%u', '%u', '%u', '%s', 'NOW()');", Id, name.c_str(), leaderGuid, EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor, MOTD.c_str());
    SaveRanksToDB();
    SaveGuildMembersToDB();
}

void Guild::SaveRanksToDB()
{
    std::stringstream ss;

    sDatabase.PExecute("DELETE FROM guilds_ranks WHERE guildid = '%u';",Id);

    std::list<RankInfo*>::iterator itr;

    for (itr = ranks.begin(); itr != ranks.end();itr++)
    {
        sDatabase.PExecute("INSERT INTO guilds_ranks VALUES ('%u', '%s', '%u');", Id, (*itr)->name.c_str(), (*itr)->rights);
        sLog.outDebug( "query rank: %s", ss.str( ).c_str( ) );
    }
}

void Guild::SaveGuildMembersToDB()
{
    std::list<MemberSlot*>::iterator itr;

    for (itr = members.begin(); itr != members.end(); itr++)
    {
        SaveMemberToDB(*itr);
    }
}

void Guild::SaveMemberToDB(MemberSlot *memslot)
{
    if(!memslot) return;

    sDatabase.PExecute("DELETE FROM guilds_members WHERE guid = '%u';",memslot->guid);
    sDatabase.PExecute("INSERT INTO guilds_members VALUES ('%u', '%u', '%u', '%s', '%s');", Id, memslot->guid, memslot->RankId, memslot->Pnote.c_str(), memslot->OFFnote.c_str());
}

void Guild::DelGuildFromDB()
{
    sDatabase.PExecute("DELETE FROM guilds WHERE guildid = '%u';",Id);
    sDatabase.PExecute("DELETE FROM guilds_ranks WHERE guildid = '%u';",Id);
}

void Guild::DelGuildMembersFromDB()
{
    std::list<MemberSlot*>::iterator itr;

    for (itr = members.begin(); itr != members.end(); itr++)
    {
        DelMemberFromDB((*itr)->guid);
    }
}

void Guild::DelMemberFromDB(uint64 guid)
{
    sDatabase.PExecute("DELETE FROM guilds_members WHERE guid = '%d';",guid);
}

void Guild::BroadcastToGuild(WorldSession *session, std::string msg)
{
    if (session && session->GetPlayer() && HasRankRight(session->GetPlayer()->GetRank(),GR_RIGHT_GCHATSPEAK))
    {
        Player *pl;
        std::list<MemberSlot*>::iterator itr;

        for (itr = members.begin(); itr != members.end(); itr++)
        {
            WorldPacket data;
            sChatHandler.FillMessageData(&data, session, CHAT_MSG_GUILD, LANG_UNIVERSAL, NULL, msg.c_str());

            pl = ObjectAccessor::Instance().FindPlayer((*itr)->guid);

            if (pl && pl->GetSession() && HasRankRight(pl->GetRank(),GR_RIGHT_GCHATLISTEN))
                pl->GetSession()->SendPacket(&data);
        }
    }
}

void Guild::BroadcastToOfficers(WorldSession *session, std::string msg)
{
    if (session && session->GetPlayer() && HasRankRight(session->GetPlayer()->GetRank(),GR_RIGHT_OFFCHATSPEAK))
    {
        Player *pl;
        std::list<MemberSlot*>::iterator itr;

        for (itr = members.begin(); itr != members.end(); itr++)
        {
            WorldPacket data;
            sChatHandler.FillMessageData(&data, session, CHAT_MSG_GUILD, LANG_UNIVERSAL, NULL, msg.c_str());

            pl = ObjectAccessor::Instance().FindPlayer((*itr)->guid);

            if (pl && pl->GetSession() && HasRankRight(pl->GetRank(),GR_RIGHT_OFFCHATLISTEN))
                pl->GetSession()->SendPacket(&data);
        }
    }
}

void Guild::CreateRank(std::string name,uint32 rights)
{
    RankInfo *newrank;

    newrank = new RankInfo;
    newrank->name = name;
    newrank->rights = rights;
    ranks.push_back(newrank);
}

std::string Guild::GetRankName(uint32 rankId)
{
    std::list<RankInfo*>::iterator itr;

    if(rankId > ranks.size()-1) return NULL;
    int i=0;
    for (itr = ranks.begin(); itr != ranks.end();itr++)
    {
        if (i == rankId)
            return (*itr)->name;
        i++;
    }
    return 0;
}

uint32 Guild::GetRankRights(uint32 rankId)
{
    std::list<RankInfo*>::iterator itr;

    if(rankId > ranks.size()-1) return 0;
    int i=0;
    for (itr = ranks.begin(); itr != ranks.end();itr++)
    {
        if (i == rankId)
            return (*itr)->rights;
        i++;
    }
    return 0;
}

void Guild::SetRankName(uint32 rankId, std::string name)
{
    std::list<RankInfo*>::iterator itr;

    if(rankId > ranks.size()-1) return;
    int i=0;
    for (itr = ranks.begin(); itr != ranks.end();itr++)
    {
        if (i == rankId)
        {
            (*itr)->name = name;
            break;
        }
        i++;
    }
}

void Guild::SetRankRights(uint32 rankId, uint32 rights)
{
    std::list<RankInfo*>::iterator itr;

    if(rankId > ranks.size()-1) return;
    int i=0;
    for (itr = ranks.begin(); itr != ranks.end();itr++)
    {
        if (i == rankId)
        {
            (*itr)->rights = rights;
            break;
        }
        i++;
    }
}

void Guild::Disband()
{
    Player * pl;
    std::list<MemberSlot*>::iterator itr;

    for (itr = members.begin(); itr != members.end();)
    {
        pl = ObjectAccessor::Instance().FindPlayer((*itr)->guid);
        if(pl)
        {
            WorldPacket data;
            pl->SetInGuild(0);

            data.Initialize(SMSG_GUILD_EVENT);
            data << (uint8)GE_DISBANDED;

            pl->GetSession()->SendPacket(&data);
        }
        DelMemberFromDB((*itr)->guid);
        members.erase(itr++);
    }
    DelGuildFromDB();
    objmgr.RemoveGuild(this);
}

void Guild::Roster(WorldSession *session)
{
    WorldPacket data;
    Player *pl;

    data.Initialize(SMSG_GUILD_ROSTER);
    data << (uint32)members.size();
    data << MOTD;
    data << GINFO;
    data << (uint32)ranks.size();

    std::list<RankInfo*>::iterator ritr;
    for (ritr = ranks.begin(); ritr != ranks.end();ritr++)
        data << (*ritr)->rights;

    std::list<MemberSlot*>::iterator itr;

    for (itr = members.begin(); itr != members.end(); itr++)
    {
        pl = ObjectAccessor::Instance().FindPlayer((*itr)->guid);

        if (pl)
        {

            data << pl->GetGUID();
            data << (uint8)1;
            data << pl->GetName();
            data << pl->GetRank();
            data << pl->getLevel();
            data << pl->getClass();
            data << pl->GetZoneId ();
            data << (uint8)0;
            data << (*itr)->Pnote;
            data << (*itr)->OFFnote;
        }
        else
        {

            data << (*itr)->guid;
            data << (uint8)0;
            data << (*itr)->name;
            data << (*itr)->RankId;
            data << (*itr)->level;
            data << (*itr)->Class;
            data << (*itr)->zoneId;
            data << (uint32)1;
            data << (*itr)->Pnote;
            data << (*itr)->OFFnote;
        }
    }

    session->SendPacket(&data);
    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_ROSTER)" );
}

void Guild::Query(WorldSession *session)
{
    WorldPacket data;

    data.Initialize( SMSG_GUILD_QUERY_RESPONSE );
    data << Id;
    data << name;
    std::list<RankInfo*>::iterator itr;
    for (itr = ranks.begin(); itr != ranks.end();itr++)
        data << (*itr)->name;
    data << EmblemStyle;
    data << EmblemColor;
    data << BorderStyle;
    data << BorderColor;
    data << BackgroundColor;

    session->SendPacket( &data );

    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_QUERY_RESPONSE)" );
}
