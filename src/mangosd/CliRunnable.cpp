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

/// \addtogroup mangosd
/// @{
/// \file

#include "Common.h"
#include "Language.h"
#include "Log.h"
#include "World.h"
#include "ScriptCalls.h"
#include "GlobalEvents.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "SystemConfig.h"
#include "Config/ConfigEnv.h"
#include "Util.h"
#include "AccountMgr.h"
#include "CliRunnable.h"
#include "MapManager.h"

//CliCommand and CliCommandHolder are defined in World.h to avoid cyclic deps

//func prototypes must be defined

void CliHelp(char*,pPrintf);
void CliInfo(char*,pPrintf);
void CliBan(char*,pPrintf);
void CliBanList(char*,pPrintf);
void CliRemoveBan(char*,pPrintf);
void CliSetGM(char*,pPrintf);
void CliListGM(char*,pPrintf);
void CliVersion(char*,pPrintf);
void CliExit(char*,pPrintf);
void CliIdleShutdown(char*,pPrintf zprintf);
void CliShutdown(char*,pPrintf zprintf);
void CliBroadcast(char*,pPrintf);
void CliCreate(char*,pPrintf);
void CliDelete(char*,pPrintf);
void CliLoadScripts(char*,pPrintf);
void CliKick(char*,pPrintf);
void CliTele(char*,pPrintf);
void CliMotd(char*,pPrintf);
void CliCorpses(char*,pPrintf);
void CliSetLogLevel(char*,pPrintf);
void CliUpTime(char*,pPrintf);
void CliSetTBC(char*,pPrintf);
void CliWritePlayerDump(char*,pPrintf);
void CliLoadPlayerDump(char*,pPrintf);
void CliSave(char*,pPrintf);
void CliSend(char*,pPrintf);
void CliPLimit(char*,pPrintf);
/// Table of known commands
const CliCommand Commands[]=
{
    {"help", & CliHelp,"Display this help message"},
    {"broadcast", & CliBroadcast,"Announce in-game message"},
    {"create", & CliCreate,"Create account"},
    {"delete", & CliDelete,"Delete account and characters"},
    {"info", & CliInfo,"Display Server infomation"},
    {"uptime", & CliUpTime, "Displays the server uptime"},
    {"motd", & CliMotd,"Change or display motd"},
    {"kick", & CliKick,"Kick user"},
    {"ban", & CliBan,"Ban account|ip"},
    {"listbans", & CliBanList,"List bans"},
    {"unban", & CliRemoveBan,"Remove ban from account|ip"},
    {"setgm", & CliSetGM,"Edit user privileges"},
    {"setbc", & CliSetTBC,"Set user expansion allowed"},
    {"listgm", & CliListGM,"Display user privileges"},
    {"loadscripts", & CliLoadScripts,"Load script library"},
    {"setloglevel", & CliSetLogLevel,"Set Log Level"},
    {"corpses", & CliCorpses,"Manually call corpses erase global even code"},
    {"version", & CliVersion,"Display server version"},
    {"idleshutdown", & CliIdleShutdown,"Shutdown server with some delay when not active connections at server"},
    {"shutdown", & CliShutdown,"Shutdown server with some delay"},
    {"exit", & CliExit,"Shutdown server NOW"},
    {"writepdump", &CliWritePlayerDump,"Write a player dump to a file"},
    {"loadpdump", &CliLoadPlayerDump,"Load a player dump from a file"},
    {"saveall", &CliSave,"Save all players"},
    {"send", &CliSend,"Send message to a player"},
    {"tele", &CliTele,"Teleport player to location"},
    {"plimit", &CliPLimit,"Show or set player login limitations"}
};
/// \todo Need some pragma pack? Else explain why in a comment.
#define CliTotalCmds sizeof(Commands)/sizeof(CliCommand)

/// Create a character dump file
void CliWritePlayerDump(char*command,pPrintf zprintf)
{
    char * file = strtok(command, " ");
    char * p2 = strtok(NULL, " ");
    if(!file || !p2)
    {
        zprintf("Syntax is: writepdump $filename $playerGUID\r\n");
        return;
    }
    objmgr.WritePlayerDump(file, atoi(p2));
}

/// Load a character from a dump file
void CliLoadPlayerDump(char*command,pPrintf zprintf)
{
    char * file = strtok(command, " ");
    char * acc = strtok(NULL, " ");
    if (!file ||!acc)
    {
        zprintf("Syntax is: loadpdump $filename $account ($newname) ($newguid)\r\n");
        return;
    }
    char * name = strtok(NULL, " ");
    char * guid = name ? strtok(NULL, " ") : NULL;
    if(objmgr.LoadPlayerDump(file, atoi(acc), name ? name : "", guid ? atoi(guid) : 0))
        zprintf("Character loaded successfully!");
    else
        zprintf("Failed to load the character!");
}

/// Reload the scripts and notify the players
void CliLoadScripts(char*command,pPrintf zprintf)
{
    char const *del=strtok(command," ");
    if (!del)
        del="";
    if(!LoadScriptingModule(del))                           // Error report is already done by LoadScriptingModule
        return;

    sWorld.SendWorldText("|cffff0000[System Message]:|rScripts reloaded", NULL);
}

/// Delete a user account and all associated characters in this realm
/// \todo This function has to be enhanced to respect the login/realm split (delete char, delete account chars in realm, delete account chars in realm then delete account
void CliDelete(char*command,pPrintf zprintf)
{
    ///- Get the account name from the command line
    char *account_name=strtok(command," ");
    if(!account_name)
    {
        // \r\n is used because this function can also be called from RA
        zprintf("Syntax is: delete $account\r\n");
        return;
    }

    int result = accmgr.DeleteAccount(accmgr.GetId(account_name));
    if(result == -1)
        zprintf("User %s NOT deleted (probably sql file format was updated)\r\n",account_name);
    if(result == 1)
        zprintf("User %s does not exist\r\n",account_name);
    else if(result == 0)
        zprintf("We deleted account: %s\r\n",account_name);
}

/// Broadcast a message to the World
void CliBroadcast(char *text,pPrintf zprintf)
{
    std::string str = LANG_SYSTEMMESSAGE;
    str += text;
    sWorld.SendWorldText(str.c_str(), NULL);
    zprintf("Broadcasting to the world:%s\r\n",str.c_str());
}

/// Print the list of commands and associated description
void CliHelp(char*,pPrintf zprintf)
{
    for (unsigned int x=0;x<CliTotalCmds;x++)
        zprintf("%-13s - %s.\r\n",Commands[x].cmd ,Commands[x].description);
}

/// Exit the realm
void CliExit(char*,pPrintf zprintf)
{
    zprintf( "Exiting daemon...\r\n" );
    World::m_stopEvent = true;
}

/// Shutdown the server (with some delay) as soon as no active connections remain on the server
void CliIdleShutdown(char* command,pPrintf zprintf)
{
    char *args = strtok(command," ");

    if(!args)
    {
        zprintf("Syntax is: idleshutdown $seconds|cancel\r\n");
        return;
    }

    if(std::string(args)=="cancel")
    {
        sWorld.ShutdownCancel();
    }
    else
    {

        uint32 time = atoi(args);

        ///- Prevent interpret wrong arg value as 0 secs shutdown time
        if(time==0 && (args[0]!='0' || args[1]!='\0') || time < 0)
        {
            zprintf("Syntax is: idleshutdown $seconds|cancel\r\n");
            return;
        }

        sWorld.ShutdownServ(time,true);
    }
}

/// Shutdown the server with some delay
void CliShutdown(char* command,pPrintf zprintf)
{
    char *args = strtok(command," ");

    if(!args)
    {
        zprintf("Syntax is: shutdown $seconds|cancel\r\n");
        return;
    }

    if(std::string(args)=="cancel")
    {
        sWorld.ShutdownCancel();
    }
    else
    {
        int32 time = atoi(args);

        ///- Prevent interpret wrong arg value as 0 secs shutdown time
        if(time==0 && (args[0]!='0' || args[1]!='\0') || time < 0)
        {
            zprintf("Syntax is: shutdown $seconds|cancel\r\n");
            return;
        }

        sWorld.ShutdownServ(time);
    }
}

/// Display info on users currently in the realm
void CliInfo(char*,pPrintf zprintf)
{
    ///- Get the list of accounts ID logged to the realm
    QueryResult *resultDB = sDatabase.Query("SELECT `name`,`account` FROM `character` WHERE `online` > 0");

    if (!resultDB)
    {
        int maxUsers = sWorld.GetMaxSessionCount();
        std::string timeStr = secsToTimeString(sWorld.GetUptime(),true);
        zprintf("Online users: 0 (max: %d) Uptime: %s\r\n",maxUsers,timeStr.c_str());
        return;
    }

    int linesize = 1+15+2+20+3+15+2+4+1+5+3;                // see format string
    char* buf = new char[resultDB->GetRowCount()*linesize+1];
    char* bufPos = buf;

    ///- Circle through accounts
    do
    {
        Field *fieldsDB = resultDB->Fetch();
        std::string name = fieldsDB[0].GetCppString();
        uint32 account = fieldsDB[1].GetUInt32();

        ///- Get the username, last IP and GM level of each account
        // No SQL injection. account is uint32.
        //                                                      0          1         2         3
        QueryResult *resultLogin = loginDatabase.PQuery("SELECT `username`,`last_ip`,`gmlevel`,`tbc` FROM `account` WHERE `id` = '%u'",account);

        if(resultLogin)
        {
            Field *fieldsLogin = resultLogin->Fetch();
            bufPos+=sprintf(bufPos,"|%15s| %20s | %15s |%4d|%5d|\r\n",
                fieldsLogin[0].GetString(),name.c_str(),fieldsLogin[1].GetString(),fieldsLogin[2].GetUInt32(),fieldsLogin[3].GetUInt32());

            delete resultLogin;
        }
        else
            bufPos += sprintf(bufPos,"|<Error>        | %20s |<Error>          |<Er>|<Err>|\r\n",name.c_str());

    }while(resultDB->NextRow());

    *bufPos = '\0';

    ///- Display the list of account/characters online
    std::string timeStr = secsToTimeString(sWorld.GetUptime(),true);
    uint32 maxUsers = sWorld.GetMaxSessionCount();
    zprintf("Online users: %u (max: %u) Uptime: %s\r\n",uint32(resultDB->GetRowCount()),maxUsers,timeStr.c_str());
    zprintf("=====================================================================\r\n");
    zprintf("|    Account    |       Character      |       IP        | GM | TBC |\r\n");
    zprintf("=====================================================================\r\n");
    zprintf("%s",buf);
    zprintf("=====================================================================\r\n");

    delete resultDB;
    delete[] buf;
}

/// Display a list of banned accounts and ip addresses
void CliBanList(char*,pPrintf zprintf)
{
    ///- Get the list of banned accounts and display them
    Field *fields;
    QueryResult *result = loginDatabase.Query("SELECT `id`,`username` FROM `account` WHERE `id` IN (SELECT `id` FROM `account_banned` WHERE `active` = 1)");
    if(result)
    {
        zprintf("Actual Banned Accounts:\r\n");
        zprintf("===============================================================================\r\n");
        zprintf("|    Account    |   BanDate    |   UnbanDate  |  Banned By    | Banned reason |\r\n");
        Field *fields2;
        do
        {
            zprintf("-------------------------------------------------------------------------------\r\n");
            fields = result->Fetch();
            // No SQL injection. id is uint32.
            QueryResult *banInfo = loginDatabase.PQuery("SELECT `bandate`,`unbandate`,`bannedby`,`banreason` FROM `account_banned` WHERE `id` = %u AND `active` = 1 ORDER BY `unbandate`", fields[0].GetUInt32());
            if (banInfo)
            {
                fields2 = banInfo->Fetch();
                do
                {
                    time_t t_ban = fields2[0].GetUInt64();
                    tm* aTm_ban = localtime(&t_ban);
                    zprintf("|%-15.15s|", fields[1].GetString());
                    zprintf("%02d-%02d-%02d %02d:%02d|", aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min);
                    if ( fields2[0].GetUInt64() == fields2[1].GetUInt64() )
                        zprintf("   permanent  |");
                    else
                    {
                        time_t t_unban = fields2[1].GetUInt64();
                        tm* aTm_unban = localtime(&t_unban);
                        zprintf("%02d-%02d-%02d %02d:%02d|",aTm_unban->tm_year%100, aTm_unban->tm_mon+1, aTm_unban->tm_mday, aTm_unban->tm_hour, aTm_unban->tm_min);
                    }
                    zprintf("%-15.15s|%-15.15s|\r\n",fields2[2].GetString(),fields2[3].GetString());
                }while ( banInfo->NextRow() );
                delete banInfo;
            }
        }while( result->NextRow() );
        zprintf("===============================================================================\r\n");
        delete result;
    }

    ///- Get the list of banned IP addresses and display them
    result = loginDatabase.Query( "SELECT `ip`,`bandate`,`unbandate`,`bannedby`,`banreason` FROM `ip_banned` WHERE (`bandate`=`unbandate` OR `unbandate`>UNIX_TIMESTAMP()) ORDER BY `unbandate`" );
    if(result)
    {
        zprintf("Actual Banned IPs:\r\n");
        zprintf("===============================================================================\r\n");
        zprintf("|      IP       |   BanDate    |   UnbanDate  |  Banned By    | Banned reason |\r\n");
        do
        {
            zprintf("-------------------------------------------------------------------------------\r\n");
            fields = result->Fetch();
            time_t t_ban = fields[1].GetUInt64();
            tm* aTm_ban = localtime(&t_ban);
            zprintf("|%-15.15s|", fields[0].GetString());
            zprintf("%02d-%02d-%02d %02d:%02d|", aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min);
            if ( fields[1].GetUInt64() == fields[2].GetUInt64() )
                zprintf("   permanent  |");
            else
            {
                time_t t_unban = fields[2].GetUInt64();
                tm* aTm_unban = localtime(&t_unban);
                zprintf("%02d-%02d-%02d %02d:%02d|", aTm_unban->tm_year%100, aTm_unban->tm_mon+1, aTm_unban->tm_mday, aTm_unban->tm_hour, aTm_unban->tm_min);
            }
            zprintf("%-15.15s|%-15.15s|\r\n", fields[3].GetString(), fields[4].GetString());
        }while( result->NextRow() );
        zprintf("===============================================================================\r\n");
        delete result;
    }
    //there is result already deleted pointer! , do not use it
    if(!result) zprintf("We do not have banned users\r\n");
}

/// Ban an IP address or a user account
void CliBan(char*command,pPrintf zprintf)
{
    ///- Get the command parameter
    char* type = strtok((char*)command, " ");
    char* nameOrIP = strtok(NULL, " ");
    char* reason = strtok(NULL," ");
    char* duration = strtok(NULL," ");

    if(!type||!nameOrIP||!reason)                           // ?!? input of single char "0"-"9" wouldn't detect when with: || !atoi(duration)
    {
        zprintf("Syntax: ban account|ip|character $AccountOrIpOrCharacter $reason ($duration[s|m|h|d]) \r\n");
        return;
    }

    if(!duration)
        duration="0";

    switch (sWorld.BanAccount(type, nameOrIP, duration, reason, "Set by console."))
    {
        case BAN_SUCCESS:
            if(atoi(duration)>0)
                zprintf("%s is banned for %s. Reason: %s.\r\n",nameOrIP,secsToTimeString(TimeStringToSecs(duration),true,false).c_str(),reason);
            else
                zprintf("%s is banned permanently. Reason: %s.\r\n",nameOrIP,reason);
            break;
        case BAN_NOTFOUND:
            zprintf("%s %s not found\r\n", type, nameOrIP);
            break;
        case BAN_SYNTAX_ERROR:
            zprintf("Syntax: ban account|ip|character $AccountOrIpOrCharacter $reason ($duration[s|m|h|d]) \r\n");
            break;
    }
}

/// Display %MaNGOS version
void CliVersion(char*,pPrintf zprintf)
{
                                                            //<--maybe better append to info cmd
    zprintf( "MaNGOS daemon version is %s\r\n", _FULLVERSION );
}

/// Unban an IP adress or a user account
void CliRemoveBan(char *command,pPrintf zprintf)
{
    ///- Get the command parameter
    char *type = strtok(command," ");
    char *nameorip = strtok(NULL," ");
    if(!nameorip||!type)
    {
        zprintf("Syntax is: unban account|ip|character $nameorip\r\n");
        return;
    }

    if (!sWorld.RemoveBanAccount(type, nameorip))
        zprintf("%s %s not found\r\n", type, nameorip);
    else
        zprintf("We removed ban from %s: %s\r\n",type,nameorip);
}

/// Display the list of GMs
void CliListGM(char*,pPrintf zprintf)
{

    ///- Get the accounts with GM Level >0
    Field *fields;

    QueryResult *result = loginDatabase.Query( "SELECT `username`,`gmlevel` FROM `account` WHERE `gmlevel` > 0" );
    if(result)
    {

        zprintf("Current gamemasters:\r\n");
        zprintf("========================\r\n");
        zprintf("|    Account    |  GM  |\r\n");
        zprintf("========================\r\n");

        ///- Circle through them. Display username and GM level
        do
        {
            fields = result->Fetch();
            zprintf("|%15s|", fields[0].GetString());
            zprintf("%6s|\r\n",fields[1].GetString());
        }while( result->NextRow() );

        zprintf("========================\r\n");
        delete result;
    }
    else
    {
        zprintf("NO gamemasters\r\n");
    }
}

/// Set the GM level of an account
void CliSetGM(char *command,pPrintf zprintf)
{
    ///- Get the command line arguments
    char *szAcc = strtok(command," ");
    char *szLevel =  strtok(NULL," ");

    if(!szAcc||!szLevel)                                    //wrong syntax 'setgm' without name
    {
        zprintf("Syntax is: setgm $character $number (0 - normal, 3 - gamemaster)>\r\n");
        return;
    }

    //wow it's ok,let's hope it was integer given
    int lev=atoi(szLevel);                                  //get int anyway (0 if error)

    ///- Escape the account name to allow quotes in names
    std::string safe_account_name=szAcc;
    loginDatabase.escape_string(safe_account_name);

    ///- Try to find the account, then update the GM level
    // No SQL injection (account name is escaped)
    QueryResult *result = loginDatabase.PQuery("SELECT `id` FROM `account` WHERE `username` = '%s'",safe_account_name.c_str());

    if (result)
    {
        Field *fields = result->Fetch();
        uint32 account_id = fields[0].GetUInt32();
        delete result;

        WorldSession* session = sWorld.FindSession(account_id);
        if(session)
            session->SetSecurity(lev);

        // No SQL injection (account name is escaped)
        loginDatabase.PExecute("UPDATE `account` SET `gmlevel` = '%d' WHERE `username` = '%s'",lev,safe_account_name.c_str());
        zprintf("We added %s gmlevel %d\r\n",szAcc,lev);
    }
    else
    {
        zprintf("No account %s found\r\n",szAcc);
    }
}

/// Create an account
void CliCreate(char *command,pPrintf zprintf)
{
    //I see no need in this function (why would an admin personally create accounts
    //instead of using account registration page or accessing db directly?)
    //but still let it be

    ///- %Parse the command line arguments
    char *szAcc = strtok(command, " ");
    if(!szAcc)
    {
        zprintf("Syntax is: create $username $password\r\n");
        return;
    }

    if(strlen(szAcc)>16)
    {
        zprintf("Account cannot be longer than 16 characters.\r\n");
        return;
    }

    char *szPassword = strtok(NULL, " ");

    if(!szPassword)
    {
        zprintf("Syntax is: create $username $password\r\n");
        return;
    }

    int result = accmgr.CreateAccount(szAcc, szPassword);
    if(result == -1)
        zprintf("User %s with password %s NOT created (probably sql file format was updated)\r\n",szAcc,szPassword);
    else if(result == 1)
        zprintf("Username %s is too long\r\n", szAcc);
    else if(result == 2)
        zprintf("User %s already exists\r\n",szAcc);
    else if(result == 0)
        zprintf("User %s with password %s created successfully\r\n",szAcc,szPassword);
}

/// Command parser and dispatcher
void ParseCommand( pPrintf zprintf, char* input)
{
    unsigned int x;
    bool bSuccess=false;
    if (!input)
        return;

    unsigned int l=strlen(input);
    char *supposedCommand=NULL,* arguments=(char*)("");
    if(l)
    {
        ///- Get the command and the arguments
        supposedCommand = strtok(input," ");
        if (supposedCommand)
        {
            if (l>strlen(supposedCommand))
                arguments=&input[strlen(supposedCommand)+1];

            ///- Circle through the command table and, if found, put the command in the queue
            for ( x=0;x<CliTotalCmds;x++)
                if(!strcmp(Commands[x].cmd,supposedCommand))
            {
                sWorld.QueueCliCommand(new CliCommandHolder(&Commands[x], arguments, zprintf));
                bSuccess=true;
                break;
            }

            ///- Display an error message if the command is unknown
            if(x==CliTotalCmds)
                zprintf("Unknown command: %s\r\n", input);
        }
    }
    if (!bSuccess)
        zprintf("mangos>");
}

/// Kick a character out of the realm
void CliKick(char*command,pPrintf zprintf)
{
    char *kickName = strtok(command, " ");

    if (!kickName)
    {
        zprintf("Syntax is: kick $charactername\r\n");
        return;
    }

    std::string name = kickName;
    normalizePlayerName(name);

    sWorld.KickPlayer(name);
}

/// Teleport a character to location
void CliTele(char*command,pPrintf zprintf)
{
    char *charName = strtok(command, " ");
    char *locName = strtok(NULL, " ");

    if (!charName || !locName)
    {
        zprintf("Syntax is: tele $charactername $location\r\n");
        return;
    }

    std::string name = charName;
    normalizePlayerName(name);

    std::string location = locName;

    sDatabase.escape_string(location);
    QueryResult *result = sDatabase.PQuery("SELECT `position_x`,`position_y`,`position_z`,`orientation`,`map` FROM `game_tele` WHERE `name` = '%s'",location.c_str());
    if (!result)
    {
        zprintf(LANG_COMMAND_TELE_NOTFOUND "\r\n");
        return;
    }

    Field *fields = result->Fetch();
    float x = fields[0].GetFloat();
    float y = fields[1].GetFloat();
    float z = fields[2].GetFloat();
    float ort = fields[3].GetFloat();
    int mapid = fields[4].GetUInt16();
    delete result;

    if(!MapManager::IsValidMapCoord(mapid,x,y))
    {
        zprintf(LANG_INVALID_TARGET_COORD "\r\n",x,y,mapid);
        return;
    }

    Player *chr = objmgr.GetPlayer(name.c_str());
    if (chr)
    {

        if(chr->IsBeingTeleported()==true)
        {
            zprintf(LANG_IS_TELEPORTED "\r\n",chr->GetName());
            return;
        }

        if(chr->isInFlight())
        {
            zprintf(LANG_CHAR_IN_FLIGHT "\r\n",chr->GetName());
            return;
        }

        zprintf(LANG_TELEPORTING_TO "\r\n",chr->GetName(),"", location.c_str());

        chr->SetRecallPosition(chr->GetMapId(),chr->GetPositionX(),chr->GetPositionY(),chr->GetPositionZ(),chr->GetOrientation());

        chr->TeleportTo(mapid,x,y,z,chr->GetOrientation());
    }
    else if (uint64 guid = objmgr.GetPlayerGUIDByName(name.c_str()))
    {
        zprintf(LANG_TELEPORTING_TO "\r\n",name.c_str(), LANG_OFFLINE, location.c_str());
        Player::SavePositionInDB(mapid,x,y,z,ort,MapManager::Instance().GetZoneId(mapid,x,y),guid);
    }
    else
        zprintf(LANG_NO_PLAYER "\r\n",name.c_str());
}

/// Display/Define the 'Message of the day' for the realm
void CliMotd(char*command,pPrintf zprintf)
{

    if (strlen(command) == 0)
    {
        zprintf("Current Message of the day: \r\n%s\r\n", sWorld.GetMotd());
        return;
    }
    else
    {
        sWorld.SetMotd(command);
        zprintf("Message of the day changed to:\r\n%s\r\n", command);
    }
}

/// Comment me
/// \todo What is CorpsesErase for?
void CliCorpses(char*,pPrintf)
{
    CorpsesErase();
}

/// Set the level of logging
void CliSetLogLevel(char*command,pPrintf zprintf)
{
    char *NewLevel = strtok(command, " ");
    if (!NewLevel)
    {
        zprintf("Syntax is: setloglevel $loglevel\r\n");
        return;
    }
    sLog.SetLogLevel(NewLevel);
}

/// Display the server uptime
void CliUpTime(char*,pPrintf zprintf)
{
    uint32 uptime = sWorld.GetUptime();
    std::string suptime = secsToTimeString(uptime,true,(uptime > 86400));
    zprintf("Server has been up for: %s\r\n", suptime.c_str());
}

/// Set/Unset the TBC flag for an account
void CliSetTBC(char *command,pPrintf zprintf)
{
    ///- Get the command line arguments
    char *szAcc = strtok(command," ");
    char *szTBC =  strtok(NULL," ");

    if(!szAcc||!szTBC)
    {
        zprintf("Syntax is: setbc $account $number (0 - normal, 1 - tbc)>\r\n");
        return;
    }

    int lev=atoi(szTBC);                                    //get int anyway (0 if error)

    if((lev > 1)|| (lev < 0))
    {
        zprintf("Syntax is: setbc $account $number (0 - normal, 1 - tbc)>\r\n");
        return;
    }

    ///- Escape the account name to allow quotes in names
    std::string safe_account_name=szAcc;
    loginDatabase.escape_string(safe_account_name);

    // No SQL injection (account name is escaped)
    QueryResult *result = loginDatabase.PQuery("SELECT 1 FROM `account` WHERE `username` = '%s'",safe_account_name.c_str());

    if (result)
    {
        // No SQL injection (account name is escaped)
        loginDatabase.PExecute("UPDATE `account` SET `tbc` = '%d' WHERE `username` = '%s'",lev,safe_account_name.c_str());
        zprintf("We added %s to expansion allowed %d\r\n",szAcc,lev);

        delete result;
    }
    else
    {
        zprintf("No account %s found\r\n",szAcc);
    }
}

/// Save all players
void CliSave(char*,pPrintf zprintf)
{
    ///- Save players
    ObjectAccessor::Instance().SaveAllPlayers();
    zprintf( "All Players Saved \r\n" );

    ///- Send a message
    sWorld.SendWorldText("Players saved!", NULL);
}

/// Send a message to a player in game
void CliSend(char *playerN,pPrintf zprintf)
{
    ///- Get the command line arguments
    char* plr = strtok((char*)playerN, " ");
    char* msg = strtok(NULL, "");

    if(!plr || !msg)
    {
        zprintf("Syntax: send $player $message (Player names case sensitive)\r\n");
        return;
    }

    ///- Find the player and check that he is not logging out.
    Player *rPlayer = objmgr.GetPlayer(plr);
    if(!rPlayer)
    {
        zprintf("Player %s not found!\r\n", plr);
        return;
    }

    if (rPlayer->GetSession()->isLogingOut())
    {
        zprintf("Cannot send message while player %s is logging out!\r\n",plr);
        return;
    }

    ///- Send the message
    //Use SendAreaTriggerMessage for fastest delivery.
    rPlayer->GetSession()->SendAreaTriggerMessage("%s", msg);
    rPlayer->GetSession()->SendAreaTriggerMessage("|cffff0000[Message from administrator]:|r");

    //Confirmation message
    zprintf("Message '%s' sent to %s\r\n",msg , plr);
}

void CliPLimit(char *args,pPrintf zprintf)
{
    if(*args)
    {
        char* param = strtok((char*)args, " ");
        if(!param || !*param)
            return;

        int l = strlen(param);

        if(     strncmp(param,"player",l) == 0 )
            sWorld.SetPlayerLimit(-SEC_PLAYER);
        else if(strncmp(param,"moderator",l) == 0 )
            sWorld.SetPlayerLimit(-SEC_MODERATOR);
        else if(strncmp(param,"gamemaster",l) == 0 )
            sWorld.SetPlayerLimit(-SEC_GAMEMASTER);
        else if(strncmp(param,"administrator",l) == 0 )
            sWorld.SetPlayerLimit(-SEC_ADMINISTRATOR);
        else if(strncmp(param,"default",l) == 0 )
            sWorld.SetPlayerLimit(DEFAULT_PLAYER_LIMIT);
        else
        {
            int val = atoi(param);
            if(val < -SEC_ADMINISTRATOR) val = -SEC_ADMINISTRATOR;

            sWorld.SetPlayerLimit(val);
        }

        // kick all low security level players
        if(sWorld.GetPlayerAmountLimit() > SEC_PLAYER)
            sWorld.KickAllLess(sWorld.GetPlayerSecurityLimit());
    }

    uint32 pLimit = sWorld.GetPlayerAmountLimit();
    AccountTypes allowedAccountType = sWorld.GetPlayerSecurityLimit();
    char const* secName = "";
    switch(allowedAccountType)
    {
    case SEC_PLAYER:        secName = "Player";        break;
    case SEC_MODERATOR:     secName = "Moderator";     break;
    case SEC_GAMEMASTER:    secName = "Gamemaster";    break;
    case SEC_ADMINISTRATOR: secName = "Administrator"; break;
    default:                secName = "<???>";         break;
    }

    zprintf("Player limits: amount %u, min. security level %s.\r\n",pLimit,secName); 
}

/// @}

#ifdef linux
// Non-blocking keypress detector, when return pressed, return 1, else always return 0
int kb_hit_return()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}
#endif

/// %Thread start
void CliRunnable::run()
{
    ///- Init new SQL thread for the world database (one connection call enough)
    sDatabase.ThreadStart();                                // let thread do safe mySQL requests

    char commandbuf[256];

    ///- Display the list of available CLI functions then beep
    sLog.outString();
    /// \todo Shoudn't we use here also the sLog singleton?
    CliHelp(NULL,&printf);

    if(sConfig.GetIntDefault("BeepAtStart", 1) > 0)
    {
        printf("\a");                                       // \a = Alert
    }

    // print this here the first time
    // later it will be printed after command queue updates
    printf("mangos>");

    ///- As long as the World is running (no World::m_stopEvent), get the command line and handle it
    while (!World::m_stopEvent)
    {
        fflush(stdout);
        #ifdef linux
        while (!kb_hit_return() && !World::m_stopEvent)
            // With this, we limit CLI to 10commands/second
            usleep(100);
        if (World::m_stopEvent)
            break;
        #endif
        char *command = fgets(commandbuf,sizeof(commandbuf),stdin);
        if (command != NULL)
        {
            for(int x=0;command[x];x++)
                if(command[x]=='\r'||command[x]=='\n')
            {
                command[x]=0;
                break;
            }
            //// \todo Shoudn't we use here also the sLog singleton?
            ParseCommand(&printf,command);
        }
        else if (feof(stdin))
        {
            World::m_stopEvent = true;
        }
    }

    ///- End the database thread
    sDatabase.ThreadEnd();                                  // free mySQL thread resources
}
