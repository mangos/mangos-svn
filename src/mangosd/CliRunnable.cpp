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
#include "Log.h"
#include "World.h"
#include "Master.h"
#include "Timer.h"
#include "ScriptCalls.h"
#include "AddonHandler.h"
#include "GlobalEvents.h"
#include "ObjectMgr.h"
#include "WorldSession.h"

#ifdef ENABLE_CLI
#include "CliRunnable.h"

typedef int(* pPrintf)(const char*,...);
typedef void(* pCliFunc)(char *,pPrintf);

typedef struct
{
    char const * cmd;
    uint8 cmdlength;                                        //including ' ' if you want
    pCliFunc Func;
    char const * discription;
}CliCommand;

char prompt[64];

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
void CliBroadcast(char*,pPrintf);
void CliCreate(char*,pPrintf);
void CliDelete(char*,pPrintf);
void CliLoadScripts(char*,pPrintf);
void CliLoadAddons(char*,pPrintf);
void CliKick(char*,pPrintf);
void CliMotd(char*,pPrintf);
void CliCorpses(char*,pPrintf);
void CliSetLogLevel(char*,pPrintf);

#define CMD(a) a,(sizeof(a)-1)
const CliCommand Commands[]=
{
    {CMD("help"), & CliHelp,"Display this help"},
    {CMD("broadcast"), & CliBroadcast,"Announce in-game message"},
    {CMD("info"), & CliInfo,"Display Server infomation"},
    {CMD("ban"), & CliBan,"Ban account|ip"},
    {CMD("listbans"), & CliBanList,"List bans"},
    {CMD("unban"), & CliRemoveBan,"Remove ban from ip|account"},
    {CMD("setgm"), & CliSetGM,"Edit user privileges"},
    {CMD("listgm"), & CliListGM,"Display user privileges"},
    //    {CMD("quit"), & CliExit,"Shutdown server"},  //allowing aliases
    {CMD("create"), & CliCreate,"Create account"},
    {CMD("delete"), & CliDelete,"Delete account and characters"},
    {CMD("exit"), & CliExit,"Shutdown server"},
    {CMD("version"), & CliVersion,"Display server version"},
    {CMD("loadscripts"), & CliLoadScripts,"Load script library"},
    {CMD("loadaddons"), & CliLoadAddons,"Load Addons data"},
    {CMD("kick"), & CliKick,"Kick user"},
    {CMD("motd"), & CliMotd,"Change or display motd"},
    {CMD("setloglevel"), & CliSetLogLevel,"Set Log Level"},
    {CMD("corpses"), & CliCorpses,"Manually call corpses erase global even code"}
};
#define CliTotalCmds sizeof(Commands)/sizeof(CliCommand)

bool IsItIP(char* banip)
{
    if(!banip)
        return false;
    //ip looks like a.b.c.d  -- let's calc number of '.' it must be equal to 3
    //and must contain only numbers + .
    unsigned int iDotCount=0;
    unsigned int l=strlen(banip);
    for(unsigned int y=0;y<l;y++)
    {
        if(banip[y]=='.')iDotCount++;
        else
        if( (banip[y] < '0' || banip[y] > '9'))
            return false;
    }

    if(iDotCount!=3)
        return false;

    return true;
}

void CliLoadAddons(char*command,pPrintf zprintf)
{
    char *del;
    int x=0;
    while(command[x]==' ')
        x++;
    del=&command[x];
    sAddOnHandler._LoadFromDB();

    sWorld.SendWorldText("|cffff0000[System Message]:|rAddons reloaded", NULL);
}

void CliLoadScripts(char*command,pPrintf zprintf)
{

    char *del;
    int x=0;
    while(command[x]==' ')
        x++;
    del=&command[x];
    if(!LoadScriptingModule(del)) return;

    sWorld.SendWorldText("|cffff0000[System Message]:|rScripts reloaded", NULL);
}

void CliDelete(char*command,pPrintf zprintf)
{

    char *account_name;
    int x=0;
    while(command[x]==' ')
        x++;
    account_name=&command[x];
    if(!strlen(account_name))
    {
        zprintf("Syntax is: delete account\x0d\x0a");
        return;
    }
    Field *fields;
    QueryResult *result = loginDatabase.PQuery("SELECT `id` FROM `account` WHERE `username` = '%s'",account_name);

    if (!result)
    {
        zprintf("User %s does not exist\x0d\x0a",account_name);
        return;
    }

    fields = result->Fetch();
    uint32 account_id = fields[0].GetUInt32();
    delete result;

    result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `account` = '%d'",account_id);

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 guidlo = fields[0].GetUInt32();

            // kick if player currently
            if(Player* p = objmgr.GetPlayer(MAKE_GUID(guidlo,HIGHGUID_PLAYER)))
                p->GetSession()->LogoutPlayer(false);

            WorldSession acc_s(account_id,NULL);            // some invalide session
            Player acc_player(&acc_s);

            acc_player.LoadFromDB(guidlo);

            acc_player.DeleteFromDB();

            zprintf("We deleted character: %s from account %s\x0d\x0a",acc_player.GetName(),account_name);

        } while (result->NextRow());

        delete result;
    }

    sDatabase.PExecute("DELETE FROM `character` WHERE `account` = '%d'",account_id);
    loginDatabase.PExecute("DELETE FROM `account` WHERE `username` = '%s'",account_name);
    loginDatabase.PExecute("DELETE FROM `realmcharacters` WHERE `acctid` = '%d'",account_id);

    zprintf("We deleted account: %s\x0d\x0a",account_name);
}

void CliBroadcast(char *text,pPrintf zprintf)
{
    std::string str ="|cffff0000[System Message]:|r";
    str += text;
    sWorld.SendWorldText(str.c_str(), NULL);
    zprintf("Broadcasting to the world:%s\x0d\x0a",str.c_str());
}

void CliHelp(char*,pPrintf zprintf)
{
    for (unsigned int x=0;x<CliTotalCmds;x++)
        zprintf("%s - %s.\x0d\x0a",Commands[x].cmd ,Commands[x].discription);
}

void CliExit(char*,pPrintf zprintf)
{
    zprintf( "Exiting daemon...\n" );
    World::m_stopEvent = true;
}

void CliInfo(char*,pPrintf zprintf)
{
    QueryResult *resultDB = sDatabase.PQuery("SELECT `name`,`account` FROM `character` WHERE `online` > 0 AND `realm` = '%u'",realmID);

    if (!resultDB)
    {
        zprintf("NO online users\x0d\x0a");
        return;
    }

    int linesize = 1+15+2+20+3+15+2+6+3;                    // see format string
    char* buf = new char[resultDB->GetRowCount()*linesize+1];
    char* bufPos = buf;

    do
    {

        Field * fieldsDB = resultDB->Fetch();
        std::string name = fieldsDB[0].GetCppString();
        uint32 account = fieldsDB[1].GetUInt32();

        QueryResult *resultLogin = loginDatabase.PQuery(
            "SELECT `username`,`last_ip`,`gmlevel` FROM `account` WHERE `id` = '%u'",account );

        if(resultLogin)
        {
            Field *fieldsLogin = resultLogin->Fetch();
            bufPos+=sprintf(bufPos,"|%15s| %20s | %15s |%6d|\x0d\x0a",
                fieldsLogin[0].GetString(),name.c_str(),fieldsLogin[1].GetString(),fieldsLogin[2].GetUInt32());

            delete resultLogin;
        }
        else
            bufPos += sprintf(bufPos,"|<Error>        | %20s |<Error>          |<Err> |\x0d\x0a",name.c_str());

    }while(resultDB->NextRow());

    *bufPos = '\0';

    zprintf("Online users: %d\x0d\x0a",resultDB->GetRowCount());
    zprintf("=================================================================\x0d\x0a");
    zprintf("|    Account    |       Character      |       IP        |  GM  |\x0d\x0a");
    zprintf("=================================================================\x0d\x0a");
    zprintf("%s",buf);
    zprintf("=================================================================\x0d\x0a");

    delete resultDB;
    delete[] buf;
}

void CliBanList(char*,pPrintf zprintf)
{
    Field *fields;
    QueryResult *result2 = loginDatabase.Query( "SELECT `username` FROM `account` WHERE `banned` > 0" );
    if(result2)
    {
        zprintf("Banned Accounts:\x0d\x0a");
        do
        {
            fields = result2->Fetch();
            zprintf("|%5s|\x0d\x0a", fields[0].GetString());
        }while( result2->NextRow() );
        delete result2;
    }

    QueryResult *result3 = loginDatabase.Query( "SELECT `ip` FROM `ip_banned`" );
    if(result3)
    {
        zprintf("Banned IPs:\x0d\x0a");

        do
        {
            fields = result3->Fetch();
            zprintf("|%5s|\x0d\x0a", fields[0].GetString());
        }while( result3->NextRow() );
        delete result3;
    }
    if(!result2 && !result3) zprintf("We don't have banned users\x0d\x0a");
}

void CliBan(char*command,pPrintf zprintf)
{

    char *banip;
    int x=0;
    while(command[x]==' ')
        x++;
    banip=&command[x];
    if(!strlen(banip))
    {
        zprintf("Syntax is: ban account or ip\x0d\x0a");
        return;
    }

    //if(isdigit(command[4])) <- very bad check, there might be an account like '123test'

    if(IsItIP(banip))
    {

        loginDatabase.PExecute("INSERT INTO `ip_banned` VALUES ('%s')",banip);
        zprintf("We banned IP: %s\x0d\x0a",banip);
    }
    else
    {
        loginDatabase.PExecute("UPDATE `account` SET `banned` = '1' WHERE `username` = '%s'",banip);
        zprintf("We banned account (if it exists): %s\x0d\x0a",banip);
    }

}

void CliVersion(char*,pPrintf zprintf)
{
                                                            //<--maybe better append to info cmd
    zprintf( "MaNGOS daemon version is %s\x0d\x0a", _FULLVERSION );
}

void CliRemoveBan(char *command,pPrintf zprintf)
{
    char *banip;
    unsigned int x=0;
    while(command[x]==' ')x++;
    banip=&command[x];
    if(!strlen(banip))
        zprintf("Syntax is: removeban character or ip");

    //if(isdigit(command[10])) //again stupid check

    if(IsItIP(banip))
    {
        loginDatabase.PExecute("DELETE FROM `ip_banned` WHERE `ip` = '%s'",banip);
        zprintf("We removed banned IP: %s\x0d\x0a",banip);
    }

    else
    {
        loginDatabase.PExecute("UPDATE `account` SET `banned` = '0' WHERE `username` = '%s'",banip);
        zprintf("We removed ban from account: %s\x0d\x0a",banip);
    }

}

void CliListGM(char *command,pPrintf zprintf)
{

    Field *fields;

    QueryResult *result = loginDatabase.Query( "SELECT `username`,`gmlevel` FROM `account` WHERE `gmlevel` > 0" );
    if(result)
    {

        zprintf("Current gamemasters:\x0d\x0a");

        zprintf("========================\x0d\x0a");
        zprintf("|    Account    |  GM  |\x0d\x0a");
        zprintf("========================\x0d\x0a");

        do
        {
            fields = result->Fetch();
            zprintf("|%15s|", fields[0].GetString());
            zprintf("%6s|\x0d\x0a",fields[1].GetString());
        }while( result->NextRow() );

        zprintf("========================\x0d\x0a");
        delete result;
    }

    else
    {
        zprintf("NO gamemasters\x0d\x0a");
    }
}

void CliSetGM(char *command,pPrintf zprintf)
{

    unsigned int x =0;
    char *charcr;

    while (command[x]==' ') x++;
    charcr=&command[x];

    char szAcc[19];
    x=0;
    while(charcr[x]!=' ' &&charcr[x] )
    {
        szAcc[x]=charcr[x];
        x++;
    }
    szAcc[x]=0;
    if(!charcr[x])                                          //wrong syntax 'setgm name' without plevel
    {
        zprintf("Syntax is: setgm character number (0 - normal, 3 - gamemaster)\x0d\x0a");
        return;
    }
    while(charcr[x]==' ')
        x++;
    if(!charcr[x])                                          //wrong syntax 'setgm name   ' without plevel
    {
        zprintf("Syntax is: setgm character number (0 - normal, 3 - gamemaster)\x0d\x0a");
        return;
    }

    //wow it's ok,let's hope it was integer given
    int lev=atoi(&charcr[x]);                               //get int anyway

    QueryResult *result = loginDatabase.PQuery("SELECT `gmlevel` FROM `account` WHERE `username` = '%s'",szAcc );

    if (result)
    {
        loginDatabase.PExecute("UPDATE `account` SET `gmlevel` = '%d' WHERE `username` = '%s'",lev,szAcc);
        zprintf("We added %s gmlevel %d\x0d\x0a",szAcc,lev);

        delete result;
    }
    else
    {
        zprintf("No such account %s\x0d\x0a",szAcc);
    }

}

void CliCreate(char *command,pPrintf zprintf)
{
    //I see no need in this function (why would an admin personally create accounts
    //instead of using account registration page or accessing db directly?)
    //but still let it be
    char szAcc[17];
    int x=0;
    while(command[x]==' ')x++;
    if(!strlen(&command[x]))
    {
        zprintf("Syntax is: create username password\x0d\x0a");
        return;
    }
    char * ptr=&command[x];
    for(x=0;ptr[x]!=' ' && ptr[x] && x<17;x++)
        szAcc[x]=ptr[x];
    if(x==17)
    {
        zprintf("Account can't be longer then 16 characters.\x0d\x0a");
        return;
    }

    if(!ptr[x])
    {
        zprintf("Syntax is: create username password\x0d\x0a");
        return;
    }
    szAcc[x]=0;

    while(ptr[x]==' ')x++;
    if(!ptr[x])
    {
        zprintf("Syntax is: create username password\x0d\x0a");
        return;
    }

    QueryResult *result1 = loginDatabase.PQuery("SELECT `username` FROM `account` WHERE `username` = '%s'",szAcc );

    if (result1)
    {
        zprintf("User %s already exists\x0d\x0a",szAcc);
        delete result1;
        return;
    }

    if(loginDatabase.PExecute("INSERT INTO `account` (`username`,`password`,`gmlevel`,`sessionkey`,`email`,`joindate`,`banned`,`last_ip`,`failed_logins`,`locked`) VALUES ('%s','%s','0','','',NOW(),'0','0','0','0')",szAcc,&ptr[x]))
    {
        loginDatabase.PExecute("INSERT INTO `realmcharacters` (`realmid`, `acctid`, `numchars`) SELECT `realmlist`.`id`, `account`.`id`, 0 FROM `account`, `realmlist` WHERE `account`.`id` NOT IN (SELECT `acctid` FROM `realmcharacters`)");
        zprintf("User %s with password %s created successfully\x0d\x0a",szAcc,&ptr[x]);
    }
    else
        zprintf("User %s with password %s NOT created (probably sql file format was updated)\x0d\x0a",szAcc,&ptr[x]);
}

void ParseCommand( pPrintf zprintf, char*command)
{
    unsigned int x;
    unsigned int l=strlen(command);
    if(l<2)                                                 //return;//some dirty check
    {
        //zprintf(prompt);
        return;
    }
    /*
    for(unsigned int y=0;y<l;y++)
    if(command[y]==0xd||command[y]==0xa)
    {command[y]=0;break;}
    // replace '\x0d\x0a' with 0
    */
    for ( x=0;x<CliTotalCmds;x++)
        if(!memcmp(Commands[x].cmd,command,Commands[x].cmdlength ))
    {
        Commands[x].Func (&command[Commands[x].cmdlength],zprintf);
        break;
    }
    if(x==CliTotalCmds)
        zprintf( "Unknown command.\x0d\x0a" );
}

void CliRunnable::run()
{
    sDatabase.ThreadStart();                                // let thread do safe mySQL requests

    char commandbuf[256];

    //uint32 realCurrTime = 0, realPrevTime = 0;

    sLog.outString("");
    void CliHelp();
    
    if(sConfig.GetIntDefault("BeepAtStart", 1) > 0)
    {
        putchar(7);
    }

    while (!World::m_stopEvent)
    {

        printf("mangos>");
        fflush(stdout);
        char *command = fgets(commandbuf,sizeof(commandbuf),stdin);
        if (command != NULL)
        {
            for(int x=0;command[x];x++)
                if(command[x]==0xd||command[x]==0xa)
            {
                command[x]=0;
                break;
            }
            ParseCommand(&printf,command);
        }
        else if (feof(stdin))
        {
            World::m_stopEvent = true;
        }

    }

    sDatabase.ThreadEnd();                                  // free mySQL thread resources
}

void CliKick(char*command,pPrintf zprintf)
{
    char *kickName;
    int x=0;
    while(command[x]==' ')
        x++;
    kickName=&command[x];

    if (strlen(kickName) == 0)
    {
        zprintf("Syntax is: kick playername\x0d\x0a");
        return;
    }

    sWorld.KickPlayer(kickName);
}

void CliMotd(char*command,pPrintf zprintf)
{
    char const *motdText;
    int x=0;
    while(command[x]==' ')
        x++;
    motdText=&command[x];

    if (strlen(motdText) == 0)
    {
        zprintf("Current Message of the day: \x0d\x0a%s\x0d\x0a", sWorld.GetMotd());
        return;
    }
    else
    {
        sWorld.SetMotd(motdText);
        zprintf("Text changed to:\x0d\x0a%s\x0d\x0a", motdText);
    }

}

void CliCorpses(char*,pPrintf)
{
    CorpsesErase();
}

void CliSetLogLevel(char*Level,pPrintf zprintf)
{
    char *NewLevel;
    int x=0;
    while(Level[x]==' ')
        x++;
    NewLevel=&Level[x];

    if (strlen(NewLevel) == 0)
    {
        zprintf("Syntax is: Bad LogLevel\x0d\x0a");
        return;
    }
    sLog.SetLogLevel(NewLevel);
}
#endif
