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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "MapManager.h"
#include "ObjectAccessor.h"

bool ChatHandler::ShowHelpForCommand(ChatCommand *table, const char* cmd)
{
    for(uint32 i = 0; table[i].Name != NULL; i++)
    {
        if(!hasStringAbbr(table[i].Name, cmd))
            continue;

        if(m_session->GetSecurity() < table[i].SecurityLevel)
            continue;

        if(table[i].ChildCommands != NULL)
        {
            cmd = strtok(NULL, " ");
            if(cmd && ShowHelpForCommand(table[i].ChildCommands, cmd))
                return true;
        }

        if(table[i].Help == "")
        {
            WorldPacket data;
            FillSystemMessageData(&data, m_session, "There is no help for that command");
            m_session->SendPacket(&data);
            return true;
        }

        SendMultilineMessage(table[i].Help.c_str());

        return true;
    }

    return false;
}

bool ChatHandler::HandleHelpCommand(const char* args)
{
    ChatCommand *table = getCommandTable();
    WorldPacket data;

    if(!*args)
        return false;

    char* cmd = strtok((char*)args, " ");
    if(!cmd)
        return false;

    if(!ShowHelpForCommand(getCommandTable(), cmd))
    {
        FillSystemMessageData(&data, m_session, "There is no such command");
        m_session->SendPacket( &data );
    }

    return true;
}

bool ChatHandler::HandleCommandsCommand(const char* args)
{
    ChatCommand *table = getCommandTable();
    WorldPacket data;

    FillSystemMessageData(&data, m_session, "Commands aviable to you:");
    m_session->SendPacket(&data);

    for(uint32 i = 0; table[i].Name != NULL; i++)
    {
        if(*args && !hasStringAbbr(table[i].Name, (char*)args))
            continue;

        if(m_session->GetSecurity() < table[i].SecurityLevel)
            continue;

        FillSystemMessageData(&data, m_session, table[i].Name);
        m_session->SendPacket(&data);
    }

    return true;
}

bool ChatHandler::HandleAcctCommand(const char* args)
{
    WorldPacket data;

    uint32 gmlevel = m_session->GetSecurity();
    char buf[256];
    sprintf(buf, "Your account level is: %i", gmlevel);
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    return true;
}

bool ChatHandler::HandleStartCommand(const char* args)
{
    Player *chr = m_session->GetPlayer();
    chr->SetUInt32Value(PLAYER_FARSIGHT, 0x01);

    PlayerCreateInfo *info = objmgr.GetPlayerCreateInfo(
        m_session->GetPlayer()->getRace(), m_session->GetPlayer()->getClass());
    ASSERT(info);

    m_session->GetPlayer()->smsg_NewWorld(info->mapId, info->positionX, info->positionY,info->positionZ,0.0f);

    return true;
}

bool ChatHandler::HandleInfoCommand(const char* args)
{
    WorldPacket data;

    uint32 clientsNum = sWorld.GetSessionCount();
    char buf[256];

    sprintf((char*)buf,"Number of users connected: %i", (int) clientsNum);
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    return true;
}

bool ChatHandler::HandleDismountCommand(const char* args)
{
    WorldPacket data;

    m_session->GetPlayer( )->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
    m_session->GetPlayer( )->RemoveFlag( UNIT_FIELD_FLAGS, 0x002000 );

    if (m_session->GetPlayer( )->GetUInt32Value(UNIT_FIELD_FLAGS) & 0x000004 )
        m_session->GetPlayer( )->RemoveFlag( UNIT_FIELD_FLAGS, 0x000004 );

    m_session->GetPlayer( )->SetPlayerSpeed(MOVE_RUN, 7.5, true);
    return true;
}

bool ChatHandler::HandleSaveCommand(const char* args)
{
    WorldPacket data;

    m_session->GetPlayer()->SaveToDB();
    FillSystemMessageData(&data, m_session, "Player saved.");
    m_session->SendPacket( &data );
    return true;
}

bool ChatHandler::HandleGMListCommand(const char* args)
{
    WorldPacket data;
    bool first = true;

    ObjectAccessor::PlayersMapType &m(ObjectAccessor::Instance().GetPlayers());
    for(ObjectAccessor::PlayersMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if(itr->second->GetSession()->GetSecurity())
        {
            if(first)
            {
                FillSystemMessageData(&data, m_session, "There are following active GMs on this server:");
                m_session->SendPacket( &data );
            }

            FillSystemMessageData(&data, m_session, itr->second->GetName());
            m_session->SendPacket( &data );

            first = false;
        }
    }

    if(first)
    {
        FillSystemMessageData(&data, m_session, "There are no GMs currently logged in on this server.");
        m_session->SendPacket( &data );
    }

    return true;
}

bool ChatHandler::HandleShowHonor(const char* args)
{
    WorldPacket data;

    uint32 dishonorable_kills       = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_LIFETIME_DISHONORABLE_KILLS);
    uint32 honorable_kills          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);
    uint32 highest_rank             = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_PVP_MEDALS);
    uint32 today_honorable_kills    = (uint16)m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_SESSION_KILLS);
    uint32 today_dishonorable_kills = (uint16)(m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_SESSION_KILLS)>>16);
    uint32 yestarday_kills          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_YESTERDAY_KILLS);
    uint32 yestarday_honor          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION);
    uint32 this_week_kills          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_THIS_WEEK_KILLS);
    uint32 this_week_honor          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_THIS_WEEK_CONTRIBUTION);
    uint32 last_week_kills          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_LAST_WEEK_KILLS);
    uint32 last_week_honor          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_LAST_WEEK_CONTRIBUTION);
    uint32 last_week_standing       = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_LAST_WEEK_RANK);

    std::string alliance_ranks[] = {"", "Private ", "Corporal ", "Sergeant ", "Master Sergeant ", "Sergeant Major ", "Knight ", "Knight-Lieutenant ", "Knight-Captain ", "Knight-Champion ", "Lieutenant Commander ", "Commander ", "Marshal ", "Field Marshal ", "Grand Marshal ", "Game Master "};
    std::string horde_ranks[] = {"", "Scout ", "Grunt ", "Sergeant ", "Senior Sergeant ", "First Sergeant ", "Stone Guard ", "Blood Guard ", "Legionnare ", "Centurion ", "Champion ", "Lieutenant General ", "General ", "Warlord ", "High Warlord ", "Game Master "};
    std::string rank_name;

    if ( m_session->GetPlayer()->GetTeam() == ALLIANCE )
    {
        rank_name = alliance_ranks[ m_session->GetPlayer()->CalculateHonorRank( m_session->GetPlayer()->GetTotalHonor() ) ];
    }
    else
    if ( m_session->GetPlayer()->GetTeam() == HORDE )
    {
        rank_name = horde_ranks[ m_session->GetPlayer()->CalculateHonorRank( m_session->GetPlayer()->GetTotalHonor() ) ];
    }
    else
    {
        rank_name = "No Rank ";
    }

    char msg[256];
    sprintf(msg, "%s%s (Rank %u)", rank_name.c_str(), m_session->GetPlayer()->GetName(), m_session->GetPlayer()->CalculateHonorRank( m_session->GetPlayer()->GetTotalHonor() ));
    FillSystemMessageData(&data, m_session, msg);
    m_session->SendPacket( &data );

    sprintf(msg, "Today: [Honorable Kills: |c0000ff00%u|r] [Dishonorable Kills: |c00ff0000%u|r]", today_honorable_kills, today_dishonorable_kills);
    FillSystemMessageData(&data, m_session, msg);
    m_session->SendPacket( &data );

    sprintf(msg, "Yestarday: [Kills: |c0000ff00%u|r] [Honor: %u]", yestarday_kills, yestarday_honor);
    FillSystemMessageData(&data, m_session, msg);
    m_session->SendPacket( &data );

    sprintf(msg, "This Week: [Kills: |c0000ff00%u|r] [Honor: %u]", this_week_kills, this_week_honor);
    FillSystemMessageData(&data, m_session, msg);
    m_session->SendPacket( &data );

    sprintf(msg, "Last Week: [Kills: |c0000ff00%u|r] [Honor: %u] [Standing: %u]", last_week_kills, last_week_honor, last_week_standing);
    FillSystemMessageData(&data, m_session, msg);
    m_session->SendPacket( &data );

    sprintf(msg, "Life Time: [Honorable Kills: |c0000ff00%u|r] [Dishonorable Kills: |c00ff0000%u|r] [Highest Rank: %u]", honorable_kills, dishonorable_kills, highest_rank);
    FillSystemMessageData(&data, m_session, msg);
    m_session->SendPacket( &data );

    return true;
}
