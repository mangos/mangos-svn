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
#include "RealmList.h"

#include "Config/ConfigEnv.h"
#include "Log.h"
#include "Network/SocketHandler.h"
#include "Network/ListenSocket.h"
#include "AuthSocket.h"
#include "SystemConfig.h"

#include <signal.h>
#include <iostream>
bool StartDB(std::string &dbstring);
void UnhookSignals();
void HookSignals();
//uint8 loglevel = DEFAULT_LOG_LEVEL;

bool stopEvent = false;
RealmList m_realmList;
DatabaseMysql dbRealmServer;

int usage(const char *prog)
{
    sLog.outString("Usage: \n %s -c config_file [%s]",prog,_MANGOSD_CONFIG);
    exit(1);
}

int main(int argc, char **argv)
{
    std::string cfg_file = _REALMD_CONFIG;
    int c=1;
    while( c < argc )
    {
        const char *tmp = argv[c];
        if( *tmp == '-' && std::string(tmp +1) == "c" )
        {
            if( ++c >= argc )
            {
                std::cerr << "Runtime-Error: -c option requires an input argument" << std::endl;
                usage(argv[0]);
            }
            else
                cfg_file = argv[c];
        }
        else
        {
            std::cerr << "Runtime-Error: unsupported option " << tmp << std::endl;

        }
        ++c;
    }

    if (!sConfig.SetSource(cfg_file.c_str()))
    {
        sLog.outError("\nCould not find configuration file %s.", cfg_file.c_str());
    } else {
        sLog.outString("\nUsing configuration file %s.", cfg_file.c_str());
    }

    // Non-critical warning about conf file version
    uint32 confVersion = sConfig.GetIntDefault("ConfVersion", 0);
    if (confVersion < _REALMDCONFVERSION)
    {
        sLog.outString("*****************************************************************************");
        sLog.outString(" WARNING: Your realmd.conf version indicates your conf file is out of date!");
        sLog.outString("          Please check for updates, as your current default values may cause");
        sLog.outString("          strange behavior.");
        sLog.outString("*****************************************************************************");
        clock_t pause = 3000 + clock();
        while (pause > clock());
    }

    sLog.outString( "MaNGOS realm daemon %s", _FULLVERSION );
    sLog.outString( "<Ctrl-C> to stop.\n" );

    std::string dbstring;
    StartDB(dbstring);

    //loglevel = (uint8)sConfig.GetIntDefault("LogLevel", DEFAULT_LOG_LEVEL);

    port_t rmport = sConfig.GetIntDefault( "RealmServerPort", DEFAULT_REALMSERVER_PORT );
    
    m_realmList.GetAndAddRealms(dbstring);
    if (m_realmList.size() == 0) {
        sLog.outError("No valid realms specified.");
        exit(1);
    }

    SocketHandler h;
    ListenSocket<AuthSocket> authListenSocket(h);
    if ( authListenSocket.Bind(rmport))
    {
        sLog.outString( "MaNGOS realmd can not bind to port %d", rmport );
        exit(1);
    }

    h.Add(&authListenSocket);

    HookSignals();

    while (!stopEvent)
        h.Select(0, 100000);

    UnhookSignals();

    sLog.outString( "Halting process..." );
    return 0;
}

void OnSignal(int s)
{
    switch (s)
    {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
        case SIGABRT:
            stopEvent = true;
            break;
        #ifdef _WIN32
        case SIGBREAK:
            stopEvent = true;
            break;
        #endif
    }

    signal(s, OnSignal);
}

bool StartDB(std::string &dbstring)
{
    if(!sConfig.GetString("LoginDatabaseInfo", &dbstring))
    {
        sLog.outError("Database not specified");
        exit(1);
    }

    sLog.outString("Database: %s", dbstring.c_str() );
    if(!dbRealmServer.Initialize(dbstring.c_str()))
    {
        sLog.outError("Cannot connect to database");
        exit(1);

    }
    
    // Right now we just clear all logged in accounts on boot up
    // We should query the realms and ask them.
    dbRealmServer.PExecute( "UPDATE `account` SET `online` = 0;" );
    return true;
}

void HookSignals()
{
    signal(SIGINT, OnSignal);
    signal(SIGQUIT, OnSignal);
    signal(SIGTERM, OnSignal);
    signal(SIGABRT, OnSignal);
    #ifdef _WIN32
    signal(SIGBREAK, OnSignal);
    #endif
}

void UnhookSignals()
{
    signal(SIGINT, 0);
    signal(SIGQUIT, 0);
    signal(SIGTERM, 0);
    signal(SIGABRT, 0);
    #ifdef _WIN32
    signal(SIGBREAK, 0);
    #endif

}
