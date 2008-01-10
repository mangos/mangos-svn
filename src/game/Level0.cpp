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
#include "AccountMgr.h"

bool ChatHandler::ShowHelpForCommand(ChatCommand *table, const char* cmd)
{
    for(uint32 i = 0; table[i].Name != NULL; ++i)
    {
        if(!hasStringAbbr(table[i].Name, cmd))
            continue;

        if(table[i].ChildCommands != NULL)
        {
            char* subcmd = strtok(NULL, " ");
            if(subcmd)
            {
                if(!ShowHelpForCommand(table[i].ChildCommands, subcmd))
                    SendSysMessage(LANG_NO_SUBCMD);

                return true;
            }

            // no help for command with subcommands, show list subcommands with permissions
            if(table[i].Help== "")
            {
                std::string list;
                for(uint32 j = 0; table[i].ChildCommands[j].Name != NULL; ++j)
                    if(m_session->GetSecurity() >= table[i].ChildCommands[j].SecurityLevel)
                        (list += "\r\n    ") += table[i].ChildCommands[j].Name;

                if(list.empty())
                    return false;

                PSendSysMessage(LANG_SUBCMDS_LIST,cmd,list.c_str());
                return true;
            }

            // if no permission show error
            if(m_session->GetSecurity() < table[i].SecurityLevel)
                return false;

            // accessible command with subcommands with help
            SendSysMultilineMessage(table[i].Help.c_str());
            return true;
        }

        // in case simple command skip to next if not permissions
        if(m_session->GetSecurity() < table[i].SecurityLevel)
            continue;

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

    // cast spell Stuck
    chr->CastSpell(chr,7355,false);
    return true;
}

bool ChatHandler::HandleInfoCommand(const char* args)
{
    uint32 clientsNum = sWorld.GetActiveSessionCount();
    uint32 maxClientsNum = sWorld.GetMaxSessionCount();
    std::string str = secsToTimeString(sWorld.GetUptime());

    PSendSysMessage(LANG_CONNECTED_USERS, clientsNum, maxClientsNum);
    PSendSysMessage(LANG_UPTIME, str.c_str());

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
    Player *player=m_session->GetPlayer();

    // save GM account without delay and output message (testing, etc)
    if(m_session->GetSecurity())
    {
        player->SaveToDB();
        SendSysMessage(LANG_PLAYER_SAVED);
        return true;
    }

    // save or plan save after 20 sec (logout delay) if current next save time more this value and _not_ output any messages to prevent cheat planning
    uint32 save_interval = sWorld.getConfig(CONFIG_INTERVAL_SAVE);
    if(save_interval==0 || save_interval > 20*1000 && player->GetSaveTimer() <= save_interval - 20*1000)
        player->SaveToDB();

    return true;
}

bool ChatHandler::HandleGMListCommand(const char* args)
{
    bool first = true;

    HashMapHolder<Player>::MapType &m = HashMapHolder<Player>::GetContainer();
    HashMapHolder<Player>::MapType::iterator itr = m.begin();
    for(; itr != m.end(); ++itr)
    {
        if( itr->second->GetSession()->GetSecurity() && (itr->second->isGameMaster() || sWorld.getConfig(CONFIG_GM_IN_GM_LIST) ) &&
            itr->second->IsVisibleGloballyFor(m_session->GetPlayer()) )
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

bool ChatHandler::HandlePasswordCommand(const char* args)
{
    if(!*args)
        return false;

    char *old_pass = strtok ((char*)args, " ");
    char *new_pass = strtok (NULL, " ");
    char *new_pass_c  = strtok (NULL, " ");

    if( !old_pass || !new_pass || !new_pass_c )
        return false;

    std::string password_old = old_pass;
    std::string password_new = new_pass;
    std::string password_new_c = new_pass_c;

    if (password_new.size() > 16)
    {
        SendSysMessage(LANG_COMMAND_NOTCHANGEPASSWORD);
        return true;
    }

    loginDatabase.escape_string(password_old);
    loginDatabase.escape_string(password_new);
    loginDatabase.escape_string(password_new_c);
    QueryResult *result = loginDatabase.PQuery("SELECT 1 FROM `account` WHERE `id`='%d' AND `I`=SHA1(CONCAT(UPPER(`username`),':',UPPER('%s')))", m_session->GetAccountId(), password_old.c_str());
    if(!result || password_new != password_new_c)
        SendSysMessage(LANG_COMMAND_WRONGOLDPASSWORD);
    else if(accmgr.ChangePassword(m_session->GetAccountId(), password_new) == 0)
        SendSysMessage(LANG_COMMAND_PASSWORD);

    if(result)
        delete result;

    return true;
}

bool ChatHandler::HandleLockAccountCommand(const char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_COMMAND_ACCLOCKPARAMETER);
        return true;
    }

    std::string argstr = (char*)args;
    if (argstr == "on")
    {
        loginDatabase.PExecute( "UPDATE `account` SET `locked` = '1' WHERE `id` = '%d'",m_session->GetAccountId());
        PSendSysMessage(LANG_COMMAND_ACCLOCKLOCKED);
        return true;
    }

    if (argstr == "off")
    {
        loginDatabase.PExecute( "UPDATE `account` SET `locked` = '0' WHERE `id` = '%d'",m_session->GetAccountId());
        PSendSysMessage(LANG_COMMAND_ACCLOCKUNLOCKED);
        return true;
    }
    else
        SendSysMessage(LANG_BAD_VALUE);
    return true;
}
