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
void CliKick(char*,pPrintf);

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
    //	{CMD("quit"), & CliExit,"Shutdown server"},  //allowing aliases
    {CMD("create"), & CliCreate,"Create account"},
    {CMD("delete"), & CliDelete,"Delete account and characters"},
    {CMD("exit"), & CliExit,"Shutdown server"},
    {CMD("version"), & CliVersion,"Display server version"},
    {CMD("loadscripts"), & CliLoadScripts,"Load script library"},
    {CMD("kick"), & CliKick,"Kick user"}
};
#define CliTotalCmds sizeof(Commands)/sizeof(CliCommand)

bool IsItIP(char* banip)
{
    //ip looks like a.b.c.d  -- let's calc number of '.' it must be equal to 3
    //and must contain only numbers + .
    unsigned int iDotCount=0;
    unsigned int l=strlen(banip);
    for(int y=0;y<l;y++)
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

    char *del;
    int x=0;
    while(command[x]==' ')
        x++;
    del=&command[x];
    if(!strlen(del))
    {
        zprintf("Syntax is: delete account\x0d\x0a");
        return;
    }
    Field *fields;
    QueryResult *result = sDatabase.PQuery("SELECT `id` FROM `account` WHERE `username` = '%s';",del);

    if (!result)
    {
        zprintf("User %s not exists\x0d\x0a",del);
        return;
    }

    fields = result->Fetch();
    int guid = fields[0].GetUInt32();
    sDatabase.PExecute("DELETE FROM `character` WHERE `account` = '%d'",guid);
    sDatabase.PExecute("DELETE FROM `account` WHERE `username` = '%s'",del);
    delete result;

    zprintf("We deleted : %s\x0d\x0a",del);
}

void CliBroadcast(char *text,pPrintf zprintf)
{
    char tmp[512]="|cffff0000[System Message]:|r";
    strcat(tmp,text);
    sWorld.SendWorldText(tmp, NULL);
    zprintf("Broadcasting to the world:%s\x0d\x0a",text);
}

void CliHelp(char*,pPrintf zprintf)
{
    for (unsigned int x=0;x<CliTotalCmds;x++)
        zprintf("%s - %s.\x0d\x0a",Commands[x].cmd ,Commands[x].discription);
}

void CliExit(char*,pPrintf zprintf)
{
    zprintf( "Exiting daemon...\n" );
    Master::m_stopEvent = true;
}

void CliInfo(char*,pPrintf zprintf)
{
    Field *fields;
    QueryResult *result = sDatabase.PQuery("SELECT COUNT(*) FROM `character` WHERE `online` > 0;");

    if (result)
    {
        fields = result->Fetch();
        int cnt = fields[0].GetUInt32();
        delete result;
        if ( cnt > 0 )
        {
            zprintf("Online users: %d\x0d\x0a",cnt);
            result = sDatabase.PQuery( "SELECT `character`.`name`,`character`.`account`,`account`.`gmlevel`,`account`.`username`,`account`.`last_ip` FROM `character` LEFT JOIN `account` ON `account`.`id` = `character`.`account` WHERE `character`.`online` > 0;" );

            zprintf("========================================================\x0d\x0a");
            zprintf("|    Account    |   Character   |      IP       |  GM  |\x0d\x0a");
            zprintf("========================================================\x0d\x0a");

            do
            {
                fields = result->Fetch();
                zprintf("|%15s|", fields[3].GetString());
                zprintf("%15s|",fields[0].GetString());
                zprintf("%15s|",fields[4].GetString());
                zprintf("%6d|\x0d\x0a", fields[2].GetUInt32());
            }while( result->NextRow() );

            zprintf("========================================================\x0d\x0a");
            delete result;
        }
        else
            zprintf("NO online users\x0d\x0a");

    }

}

void CliBanList(char*,pPrintf zprintf)
{
    Field *fields;
    QueryResult *result2 = sDatabase.PQuery( "SELECT `username` FROM `account` WHERE `banned` > 0;" );
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

    QueryResult *result3 = sDatabase.PQuery( "SELECT `ip` FROM `ip_banned`;" );
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

        sDatabase.PExecute("INSERT INTO `ip_banned` VALUES ('%s')",banip);
        zprintf("We banned IP: %s\x0d\x0a",banip);
    }
    else
    {
        sDatabase.PExecute("UPDATE `account` SET `banned` = '1' WHERE `username` = '%s'",banip);
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
        sDatabase.PExecute("DELETE FROM `ip_banned` WHERE `ip` = '%s'",banip);
        zprintf("We removed banned IP: %s\x0d\x0a",banip);
    }

    else
    {
        sDatabase.PExecute("UPDATE `account` SET `banned` = '0' WHERE `username` = '%s'",banip);
        zprintf("We removed ban from account: %s\x0d\x0a",banip);
    }

}

void CliListGM(char *command,pPrintf zprintf)
{

    Field *fields;

    QueryResult *result = sDatabase.PQuery( "SELECT `username`,`gmlevel` FROM `account` WHERE `gmlevel` > 0;" );
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

    QueryResult *result = sDatabase.PQuery("SELECT `gmlevel` FROM `account` WHERE `username` = '%s';",szAcc );

    if (result)
    {
        sDatabase.PExecute("UPDATE `account` SET `gmlevel` = '%d' WHERE `username` = '%s'",lev,szAcc);
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

    QueryResult *result1 = sDatabase.PQuery("SELECT `username` FROM `account` WHERE `username` = '%s';",szAcc );

    if (result1)
    {
        zprintf("User %s already exists\x0d\x0a",szAcc);
        delete result1;
        return;
    }

    if(sDatabase.PExecute("INSERT into `account` (`username`,`password`,`gmlevel`,`sessionkey`,`email`,`joindate`,`banned`,`last_ip`,`failed_logins`,`locked`) VALUES ('%s','%s','0','','',NOW(),'0','0','0','0')",szAcc,&ptr[x]))
        zprintf("User %s with password %s created successfully\x0d\x0a",szAcc,&ptr[x]);
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

    char commandbuf[256];

    uint32 realCurrTime = 0, realPrevTime = 0;

    sLog.outString("");
    void CliHelp();
    putchar(7);

    while (!Master::m_stopEvent)
    {

        printf("mangos>");
        fflush(stdout);
        char *command = fgets(commandbuf,sizeof(commandbuf),stdin);
        if (command != NULL)
        {
            for(int x=0;true;x++)
                if(command[x]==0xd||command[x]==0xa)
            {
                command[x]=0;
                break;
            }
            ParseCommand(&printf,command);
        }
        else if (feof(stdin))
        {
            Master::m_stopEvent = true;
        }

    }

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
#endif
