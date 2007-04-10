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
#include "Log.h"
#include "World.h"
#include "ScriptCalls.h"
#include "GlobalEvents.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "SystemConfig.h"
#include "Config/ConfigEnv.h"
#include "Util.h"

#ifdef ENABLE_CLI
#include "CliRunnable.h"

typedef int(* pPrintf)(const char*,...);
typedef void(* pCliFunc)(char *,pPrintf);

/// Storage structure for commands
typedef struct
{
    char const * cmd;
    pCliFunc Func;
    char const * description;
}CliCommand;

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
void CliMotd(char*,pPrintf);
void CliCorpses(char*,pPrintf);
void CliSetLogLevel(char*,pPrintf);
void CliUpTime(char*,pPrintf);

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
    {"listgm", & CliListGM,"Display user privileges"},
    {"loadscripts", & CliLoadScripts,"Load script library"},
    {"setloglevel", & CliSetLogLevel,"Set Log Level"},
    {"corpses", & CliCorpses,"Manually call corpses erase global even code"},
    {"version", & CliVersion,"Display server version"},
    {"idleshutdown", & CliIdleShutdown,"Shutdown server with some delay when not active connections at server"},
    {"shutdown", & CliShutdown,"Shutdown server with some delay"},
    {"exit", & CliExit,"Shutdown server NOW"}
};
/// \todo Need some pragma pack? Else explain why in a comment.
#define CliTotalCmds sizeof(Commands)/sizeof(CliCommand)

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
        zprintf("Syntax is: delete <account>\r\n");
        return;
    }

    ///- Escape account name to allow quotes in names
    std::string safe_account_name=account_name;
    loginDatabase.escape_string(safe_account_name);

    ///- Get the account ID from the database
    Field *fields;
    // No SQL injection (account_name escaped)
    QueryResult *result = loginDatabase.PQuery("SELECT `id` FROM `account` WHERE `username` = '%s'",safe_account_name.c_str());

    if (!result)
    {
        zprintf("User %s does not exist\r\n",account_name);
        return;
    }

    fields = result->Fetch();
    uint32 account_id = fields[0].GetUInt32();
    delete result;

    ///- Circle through characters belonging to this account ID and remove all characters related data (items, quests, ...) from the database
    // No SQL injection (account_id is db internal)
    result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `account` = '%d'",account_id);

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 guidlo = fields[0].GetUInt32();

            // kick if player currently
            if(Player* p = objmgr.GetPlayer(MAKE_GUID(guidlo,HIGHGUID_PLAYER)))
                p->GetSession()->KickPlayer();

            WorldSession acc_s(account_id,NULL,0);          // some invalid session
            Player acc_player(&acc_s);

            acc_player.LoadFromDB(guidlo);

            acc_player.DeleteFromDB();

            zprintf("We deleted character: %s from account %s\r\n",acc_player.GetName(),account_name);

        } while (result->NextRow());

        delete result;
    }

    ///- Remove characters and account from the databases
    sDatabase.BeginTransaction();

    bool done = sDatabase.PExecute("DELETE FROM `character` WHERE `account` = '%d'",account_id) &&
        loginDatabase.PExecute("DELETE FROM `account` WHERE `username` = '%s'",safe_account_name.c_str()) &&
        loginDatabase.PExecute("DELETE FROM `realmcharacters` WHERE `acctid` = '%d'",account_id);

    sDatabase.CommitTransaction();

    if (done)
        zprintf("We deleted account: %s\r\n",account_name);
}

/// Broadcast a message to the World
void CliBroadcast(char *text,pPrintf zprintf)
{
    std::string str ="|cffff0000[System Message]:|r";
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
        zprintf("Syntax is: idleshutdown <seconds|cancel>\r\n");
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
            zprintf("Syntax is: idleshutdown <seconds|cancel>\r\n");
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
        zprintf("Syntax is: shutdown <seconds|cancel>\r\n");
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
            zprintf("Syntax is: shutdown <seconds|cancel>\r\n");
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

    int linesize = 1+15+2+20+3+15+2+6+3;                    // see format string
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
        QueryResult *resultLogin = loginDatabase.PQuery(
            "SELECT `username`,`last_ip`,`gmlevel` FROM `account` WHERE `id` = '%u'",account);

        if(resultLogin)
        {
            Field *fieldsLogin = resultLogin->Fetch();
            bufPos+=sprintf(bufPos,"|%15s| %20s | %15s |%6d|\r\n",
                fieldsLogin[0].GetString(),name.c_str(),fieldsLogin[1].GetString(),fieldsLogin[2].GetUInt32());

            delete resultLogin;
        }
        else
            bufPos += sprintf(bufPos,"|<Error>        | %20s |<Error>          |<Err> |\r\n",name.c_str());

    }while(resultDB->NextRow());

    *bufPos = '\0';

    ///- Display the list of account/characters online
    std::string timeStr = secsToTimeString(sWorld.GetUptime(),true);
    uint32 maxUsers = sWorld.GetMaxSessionCount();
    zprintf("Online users: %u (max: %u) Uptime: %s\r\n",uint32(resultDB->GetRowCount()),maxUsers,timeStr.c_str());
    zprintf("=================================================================\r\n");
    zprintf("|    Account    |       Character      |       IP        |  GM  |\r\n");
    zprintf("=================================================================\r\n");
    zprintf("%s",buf);
    zprintf("=================================================================\r\n");

    delete resultDB;
    delete[] buf;
}

/// Display a list of banned accounts and ip addresses
void CliBanList(char*,pPrintf zprintf)
{
    ///- Get the list of banned accounts and display them
    Field *fields;
    QueryResult *result2 = loginDatabase.Query( "SELECT `username` FROM `account` WHERE `banned` > 0" );
    if(result2)
    {
        zprintf("Banned Accounts:\r\n");
        do
        {
            fields = result2->Fetch();
            zprintf("|%15s|\r\n", fields[0].GetString());
        }while( result2->NextRow() );
        delete result2;
    }

    ///- Get the list of banned IP addresses and display them
    QueryResult *result3 = loginDatabase.Query( "SELECT `ip` FROM `ip_banned`" );
    if(result3)
    {
        zprintf("Banned IPs:\r\n");
        do
        {
            fields = result3->Fetch();
            zprintf("|%15s|\r\n", fields[0].GetString());
        }while( result3->NextRow() );
        delete result3;
    }

    if(!result2 && !result3) zprintf("We do not have banned users\r\n");
}

/// Ban an IP address or a user account
void CliBan(char*command,pPrintf zprintf)
{
    ///- Get the command parameter
    char *banip = strtok(command," ");
    if(!banip)
    {
        zprintf("Syntax is: ban <account|ip>\r\n");
        return;
    }

    // Is this IP address or account name?
    bool is_ip = IsIPAddress(banip);

    if(sWorld.BanAccount(banip))
    {
        if(is_ip)
            zprintf("We banned IP: %s\r\n",banip);
        else
            zprintf("We banned account: %s\r\n",banip);
    }
    else
    {
        zprintf("Account %s not found and not banned.\r\n",banip);
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
    char *banip = strtok(command," ");
    if(!banip)
        zprintf("Syntax is: removeban <account|ip>\r\n");

    sWorld.RemoveBanAccount(banip);

    ///- If this is an IP address
    if(IsIPAddress(banip))
        zprintf("We removed banned IP: %s\r\n",banip);
    else
        zprintf("We removed ban from account: %s\r\n",banip);
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

    if(!szAcc)                                              //wrong syntax 'setgm' without name
    {
        zprintf("Syntax is: setgm <character> <number (0 - normal, 3 - gamemaster)>\r\n");
        return;
    }

    char *szLevel =  strtok(NULL," ");

    if(!szLevel)                                            //wrong syntax 'setgm' without plevel
    {
        zprintf("Syntax is: setgm <character> <number (0 - normal, 3 - gamemaster)>\r\n");
        return;
    }

    //wow it's ok,let's hope it was integer given
    int lev=atoi(szLevel);                                  //get int anyway (0 if error)

    ///- Escape the account name to allow quotes in names
    std::string safe_account_name=szAcc;
    loginDatabase.escape_string(safe_account_name);

    ///- Try to find the account, then update the GM level
    // No SQL injection (account name is escaped)
    QueryResult *result = loginDatabase.PQuery("SELECT 1 FROM `account` WHERE `username` = '%s'",safe_account_name.c_str());

    if (result)
    {
        // No SQL injection (account name is escaped)
        loginDatabase.PExecute("UPDATE `account` SET `gmlevel` = '%d' WHERE `username` = '%s'",lev,safe_account_name.c_str());
        zprintf("We added %s gmlevel %d\r\n",szAcc,lev);

        delete result;
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
        zprintf("Syntax is: create <username> <password>\r\n");
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
        zprintf("Syntax is: create <username> <password>\r\n");
        return;
    }

    ///- Escape the account name to allow quotes in names
    std::string safe_account_name=szAcc;
    loginDatabase.escape_string(safe_account_name);

    ///- Check that the account does not exist yet
    QueryResult *result1 = loginDatabase.PQuery("SELECT 1 FROM `account` WHERE `username` = '%s'",safe_account_name.c_str());

    if (result1)
    {
        zprintf("User %s already exists\r\n",szAcc);
        delete result1;
        return;
    }

    ///- Also escape the password
    std::string safe_password=szPassword;
    loginDatabase.escape_string(safe_password);

    ///- Insert the account in the database (account table)
    // No SQL injection (escaped account name and password)
    sDatabase.BeginTransaction();
    if(loginDatabase.PExecute("INSERT INTO `account` (`username`,`password`,`gmlevel`,`sessionkey`,`email`,`joindate`,`banned`,`last_ip`,`failed_logins`,`locked`) VALUES ('%s','%s','0','','',NOW(),'0','0','0','0')",safe_account_name.c_str(),safe_password.c_str()))
    {
        ///- Make sure that the realmcharacters table is up-to-date
        loginDatabase.Execute("INSERT INTO `realmcharacters` (`realmid`, `acctid`, `numchars`) SELECT `realmlist`.`id`, `account`.`id`, 0 FROM `account`, `realmlist` WHERE `account`.`id` NOT IN (SELECT `acctid` FROM `realmcharacters`)");
        zprintf("User %s with password %s created successfully\r\n",szAcc,szPassword);
    }
    else
        zprintf("User %s with password %s NOT created (probably sql file format was updated)\r\n",szAcc,szPassword);
    sDatabase.CommitTransaction();
}

/// Command parser and dispatcher
void ParseCommand( pPrintf zprintf, char* input)
{
    unsigned int x;
    if (!input)
        return;

    unsigned int l=strlen(input);
    char *supposedCommand=NULL,* arguments=(char*)("");
    if(!l)
        return;

    ///- Get the command and the arguments
    supposedCommand = strtok(input," ");
    if (l>strlen(supposedCommand))
        arguments=&input[strlen(supposedCommand)+1];

    ///- Circle through the command table and invoke the appropriate handler
    for ( x=0;x<CliTotalCmds;x++)
        if(!strcmp(Commands[x].cmd,supposedCommand))
    {
        Commands[x].Func(arguments,zprintf);
        break;
    }

    ///- Display an error message if the command is unknown
    if(x==CliTotalCmds)
        zprintf("Unknown command: %s\r\n", input);
}

/// Kick a character out of the realm
void CliKick(char*command,pPrintf zprintf)
{
    char *kickName = strtok(command, " ");

    if (!kickName)
    {
        zprintf("Syntax is: kick <charactername>\r\n");
        return;
    }

    std::string name = kickName;
    normalizePlayerName(name);

    sWorld.KickPlayer(name);
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
        zprintf("Syntax is: setloglevel <loglevel>\r\n");
        return;
    }
    sLog.SetLogLevel(NewLevel);
}

void CliUpTime(char*,pPrintf zprintf)
{    
    uint32 uptime = sWorld.GetUptime();
    std::string suptime = secsToTimeString(uptime,true,(uptime > 86400));
    sLog.outBasic("Server has been up for : %s",suptime.c_str());
}

/// @}

/// %Thread start
void CliRunnable::run()
{
    ///- Init new SQL thread for the world database (one connection call enough)
    sDatabase.ThreadStart();                                // let thread do safe mySQL requests

    char commandbuf[256];

    ///- Display the list of available CLI functions then beep
    sLog.outString("");
    /// \todo Shoudn't we use here also the sLog singleton?
    CliHelp(NULL,&printf);

    if(sConfig.GetIntDefault("BeepAtStart", 1) > 0)
    {
        printf("\a");                                       // \a = Alert
    }

    ///- As long as the World is running (no World::m_stopEvent), get the command line and handle it
    while (!World::m_stopEvent)
    {
        printf("mangos>");
        fflush(stdout);
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
#endif
