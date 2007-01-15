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
#include "Language.h"
#include "SpellAuras.h"

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
            SendSysMessage(LANG_NO_HELP_CMD);
            return true;
        }

        SendSysMultilineMessage(table[i].Help.c_str());

        return true;
    }

    return false;
}

bool ChatHandler::HandleHelpCommand(const char* args)
{
    if(!*args)
        return false;

    char* cmd = strtok((char*)args, " ");
    if(!cmd)
        return false;

    if(!ShowHelpForCommand(getCommandTable(), cmd))
    {
        SendSysMessage(LANG_NO_CMD);
    }

    return true;
}

bool ChatHandler::HandleCommandsCommand(const char* args)
{
    ChatCommand *table = getCommandTable();

    SendSysMessage(LANG_AVIABLE_CMD);

    for(uint32 i = 0; table[i].Name != NULL; i++)
    {
        if(*args && !hasStringAbbr(table[i].Name, (char*)args))
            continue;

        if(m_session->GetSecurity() < table[i].SecurityLevel)
            continue;

        SendSysMessage(table[i].Name);
    }

    return true;
}

bool ChatHandler::HandleAcctCommand(const char* args)
{
    uint32 gmlevel = m_session->GetSecurity();
    PSendSysMessage(LANG_ACCOUNT_LEVEL, gmlevel);
    return true;
}

bool ChatHandler::HandleStartCommand(const char* args)
{
    Player *chr = m_session->GetPlayer();

    if(chr->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        return true;
    }

    if(chr->isInCombat())
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        return true;
    }

    // not let used .start command as free replacement for hearthstone
    if(chr->HaveSpellCooldown(8690))
    {
        SendSysMessage(LANG_YOU_USED_IT_RECENTLY);
        return true;
    }

    chr->SetUInt32Value(PLAYER_FARSIGHT, 0x01);

    PlayerInfo const *info = objmgr.GetPlayerInfo(chr->getRace(), chr->getClass());

    chr->TeleportTo(info->mapId, info->positionX, info->positionY,info->positionZ,chr->GetOrientation());

    chr->SetUInt32Value(PLAYER_FARSIGHT, 0x00);

    // set hearthstone cooldown
    chr->AddSpellCooldown(8690,time(NULL)+3600);

    return true;
}

bool ChatHandler::HandleInfoCommand(const char* args)
{
    uint32 clientsNum = sWorld.GetSessionCount();

    PSendSysMessage(LANG_CONNECTED_USERS, (int) clientsNum);

    return true;
}

bool ChatHandler::HandleDismountCommand(const char* args)
{

    //If player is not mounted, so go out :)
    if (!m_session->GetPlayer( )->IsMounted())
    {
        SendSysMessage(LANG_CHAR_NON_MOUNTED);
        return true;
    }

    if(m_session->GetPlayer( )->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        return true;
    }

    m_session->GetPlayer()->Unmount();
    m_session->GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
    return true;
}

bool ChatHandler::HandleSaveCommand(const char* args)
{
    m_session->GetPlayer()->SaveToDB();
    SendSysMessage(LANG_PLAYER_SAVED);
    return true;
}

bool ChatHandler::HandleGMListCommand(const char* args)
{
    bool first = true;

    ObjectAccessor::PlayersMapType &m(ObjectAccessor::Instance().GetPlayers());
    for(ObjectAccessor::PlayersMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if( itr->second->GetSession()->GetSecurity() && (itr->second->isGameMaster() || sWorld.getConfig(CONFIG_GM_IN_GM_LIST) ) &&
            itr->second->isVisibleFor(m_session->GetPlayer(),false) )
        {
            if(first)
            {
                SendSysMessage(LANG_GMS_ON_SRV);
                first = false;
            }

            SendSysMessage(itr->second->GetName());

        }
    }

    if(first)
        SendSysMessage(LANG_GMS_NOT_LOGGED);

    return true;
}

bool ChatHandler::HandleShowHonor(const char* args)
{
    uint32 dishonorable_kills       = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_LIFETIME_DISHONORBALE_KILLS);
    uint32 honorable_kills          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORBALE_KILLS);
    uint32 highest_rank             = (m_session->GetPlayer()->GetHonorHighestRank() < 16)? m_session->GetPlayer()->GetHonorHighestRank() : 0;
    uint32 today_honorable_kills    = (uint16)m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_SESSION_KILLS);
    uint32 today_dishonorable_kills = (uint16)(m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_SESSION_KILLS)>>16);
    uint32 yestarday_kills          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_YESTERDAY_KILLS);
    uint32 yestarday_honor          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION);
    uint32 this_week_kills          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_THIS_WEEK_KILLS);
    uint32 this_week_honor          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_THIS_WEEK_CONTRIBUTION);
    uint32 last_week_kills          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_LAST_WEEK_KILLS);
    uint32 last_week_honor          = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_LAST_WEEK_CONTRIBUTION);
    uint32 last_week_standing       = m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_LAST_WEEK_RANK);

    std::string alliance_ranks[] =
    {
        "",
        LANG_ALI_PRIVATE,
        LANG_ALI_CORPORAL,
        LANG_ALI_SERGEANT,
        LANG_ALI_MASTER_SERGEANT,
        LANG_ALI_SERGEANT_MAJOR,
        LANG_ALI_KNIGHT,
        LANG_ALI_KNIGHT_LIEUTENANT,
        LANG_ALI_KNIGHT_CAPTAIN,
        LANG_ALI_KNIGHT_CHAMPION,
        LANG_ALI_LIEUTENANT_COMMANDER,
        LANG_ALI_COMMANDER,
        LANG_ALI_MARSHAL,
        LANG_ALI_FIELD_MARSHAL,
        LANG_ALI_GRAND_MARSHAL,
        LANG_ALI_GAME_MASTER
    };
    std::string horde_ranks[] =
    {
        "",
        LANG_HRD_SCOUT,
        LANG_HRD_GRUNT,
        LANG_HRD_SERGEANT,
        LANG_HRD_SENIOR_SERGEANT,
        LANG_HRD_FIRST_SERGEANT,
        LANG_HRD_STONE_GUARD,
        LANG_HRD_BLOOD_GUARD,
        LANG_HRD_LEGIONNARE,
        LANG_HRD_CENTURION,
        LANG_HRD_CHAMPION,
        LANG_HRD_LIEUTENANT_GENERAL,
        LANG_HRD_GENERAL,
        LANG_HRD_WARLORD,
        LANG_HRD_HIGH_WARLORD,
        LANG_HRD_GAME_MASTER
    };
    std::string rank_name;
    std::string hrank_name;

    if ( m_session->GetPlayer()->GetTeam() == ALLIANCE )
    {
        rank_name = alliance_ranks[ m_session->GetPlayer()->CalculateHonorRank( m_session->GetPlayer()->GetTotalHonor() ) ];
        hrank_name = alliance_ranks[ highest_rank ];
    }
    else
    if ( m_session->GetPlayer()->GetTeam() == HORDE )
    {
        rank_name = horde_ranks[ m_session->GetPlayer()->CalculateHonorRank( m_session->GetPlayer()->GetTotalHonor() ) ];
        hrank_name = horde_ranks[ highest_rank ];
    }
    else
    {
        rank_name = LANG_NO_RANK;
    }

    PSendSysMessage(LANG_RANK, rank_name.c_str(), m_session->GetPlayer()->GetName(), m_session->GetPlayer()->CalculateHonorRank( m_session->GetPlayer()->GetTotalHonor() ));
    PSendSysMessage(LANG_HONOR_TODAY, today_honorable_kills, today_dishonorable_kills);
    PSendSysMessage(LANG_HONOR_YESTERDAY, yestarday_kills, yestarday_honor);
    PSendSysMessage(LANG_HONOR_THIS_WEEK, this_week_kills, this_week_honor);
    PSendSysMessage(LANG_HONOR_LAST_WEEK, last_week_kills, last_week_honor, last_week_standing);
    PSendSysMessage(LANG_HONOR_LIFE, honorable_kills, dishonorable_kills, highest_rank, hrank_name.c_str());

    return true;
}

bool ChatHandler::HandlePasswordCommand(const char* args)
{
    if(!*args)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    std::string password = args;
    loginDatabase.escape_string(password);

    if(loginDatabase.PExecute( "UPDATE `account` SET `password` = '%s' WHERE `id` = '%d';",password.c_str(), m_session->GetAccountId()))
    {
        PSendSysMessage("New password %s",password.c_str());
        return true;
    }

    return true;
}

bool ChatHandler::HandleLockAccountCommand(const char* args)
{
    if (!*args)
    {
        SendSysMessage("You must send parameter");
        return true;
    }

    std::string argstr = (char*)args;
    if (argstr == "on")
    {
        loginDatabase.PExecute( "UPDATE `account` SET `locked` = '1' WHERE `id` = '%d';",m_session->GetAccountId());
        PSendSysMessage("Your account now is locked");
        return true;
    }

    if (argstr == "off")
    {
        loginDatabase.PExecute( "UPDATE `account` SET `locked` = '0' WHERE `id` = '%d';",m_session->GetAccountId());
        PSendSysMessage("Your account now is unlocked");
        return true;
    }
    else
        SendSysMessage(LANG_BAD_VALUE);
    return true;
}
