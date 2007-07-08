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

/// \addtogroup realmd Realm Daemon
/// @{
/// \file

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "RealmList.h"

#include "Config/ConfigEnv.h"
#include "Log.h"
#include "Network/ListenSocket.h"
#include "AuthSocket.h"
#include "SystemConfig.h"

bool StartDB(std::string &dbstring);
void UnhookSignals();
void HookSignals();

bool stopEvent = false;                                     ///< Setting it to true stops the server
RealmList m_realmList;                                      ///< Holds the list of realms for this server
DatabaseMysql dbRealmServer;                                ///< Accessor to the realm server database

/// Print out the usage string for this program on the console.
void usage(const char *prog)
{
    sLog.outString("Usage: \n %s [-c config_file]",prog);
}

/// Launch the realm server
int main(int argc, char **argv)
{
    ///- Command line parsing to get the configuration file name
    char const* cfg_file = _REALMD_CONFIG;
    int c=1;
    while( c < argc )
    {
        if( strcmp(argv[c],"-c") == 0)
        {
            if( ++c >= argc )
            {
                sLog.outError("Runtime-Error: -c option requires an input argument");
                usage(argv[0]);
                return 1;
            }
            else
                cfg_file = argv[c];
        }
        else
        {
            sLog.outError("Runtime-Error: unsupported option %s",argv[c]);
            usage(argv[0]);
            return 1;
        }
        ++c;
    }

    if (!sConfig.SetSource(cfg_file))
    {
        sLog.outError("Could not find configuration file %s.", cfg_file);
        return 1;
    }
    sLog.outString("Using configuration file %s.", cfg_file);

    ///- Check the version of the configuration file
    uint32 confVersion = sConfig.GetIntDefault("ConfVersion", 0);
    if (confVersion < _REALMDCONFVERSION)
    {
        sLog.outError("*****************************************************************************");
        sLog.outError(" WARNING: Your realmd.conf version indicates your conf file is out of date!");
        sLog.outError("          Please check for updates, as your current default values may cause");
        sLog.outError("          strange behavior.");
        sLog.outError("*****************************************************************************");
        clock_t pause = 3000 + clock();
        while (pause > clock());
    }

    sLog.outString( "MaNGOS realm daemon %s", _FULLVERSION );
    sLog.outString( "<Ctrl-C> to stop.\n" );

    ///- Initialise the database connection
    std::string dbstring;
    if(!StartDB(dbstring))
        return 1;

    ///- Get the list of realms for the server
    m_realmList.GetAndAddRealms(dbstring);
    if (m_realmList.size() == 0)
    {
        sLog.outError("No valid realms specified.");
        return 1;
    }

    ///- Launch the listening network socket
    port_t rmport = sConfig.GetIntDefault( "RealmServerPort", DEFAULT_REALMSERVER_PORT );

    SocketHandler h;
    ListenSocket<AuthSocket> authListenSocket(h);
    if ( authListenSocket.Bind(rmport))
    {
        sLog.outError( "MaNGOS realmd can not bind to port %d", rmport );
        return 1;
    }

    h.Add(&authListenSocket);

    ///- Catch termination signals
    HookSignals();

    ///- Handle affinity for multiple processors and process priority on Windows
    #ifdef WIN32
    {
        HANDLE hProcess = GetCurrentProcess();

        uint32 Aff = sConfig.GetIntDefault("UseProcessors", 0);
        if(Aff > 0)
        {
            DWORD appAff;
            DWORD sysAff;

            if(GetProcessAffinityMask(hProcess,&appAff,&sysAff))
            {
                DWORD curAff = Aff & appAff;                // remove non accessible processors

                if(!curAff )
                {
                    sLog.outError("Processors marked in UseProcessors bitmask (hex) %x not accessible for realmd. Accessible processors bitmask (hex): %x",Aff,appAff);
                }
                else
                {
                    if(SetProcessAffinityMask(hProcess,curAff))
                        sLog.outString("Using processors (bitmask, hex): %x", curAff);
                    else
                        sLog.outError("Can't set used processors (hex): %x", curAff);
                }
            }
            sLog.outString("");
        }

        uint32 Prio = sConfig.GetIntDefault("ProcessPriority", 0);

        if(Prio)
        {
            if(SetPriorityClass(hProcess,HIGH_PRIORITY_CLASS))
                sLog.outString("realmd process priority class set to HIGH");
            else
                sLog.outError("ERROR: Can't set realmd process priority class.");
            sLog.outString("");
        }
    }
    #endif

    // maximum counter for next ping
    uint32 numLoops = (sConfig.GetIntDefault( "MaxPingTime", 30 ) * (MINUTE * 1000000 / 100000));
    uint32 loopCounter = 0;

    ///- Wait for termination signal
    while (!stopEvent)
    {
        h.Select(0, 100000);

        if( (++loopCounter) == numLoops )
        {
            loopCounter = 0;
            sLog.outDetail("Ping MySQL to keep connection alive");
            delete dbRealmServer.Query("SELECT 1 FROM `realmlist` LIMIT 1");
        }
    }

    ///- Remove signal handling before leaving
    UnhookSignals();

    sLog.outString( "Halting process..." );
    return 0;
}

/// Handle termination signals
/** Put the global variable stopEvent to 'true' if a termination signal is caught **/
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

/// Initialize connection to the database
bool StartDB(std::string &dbstring)
{
    if(!sConfig.GetString("LoginDatabaseInfo", &dbstring))
    {
        sLog.outError("Database not specified");
        return false;
    }

    sLog.outString("Database: %s", dbstring.c_str() );
    if(!dbRealmServer.Initialize(dbstring.c_str()))
    {
        sLog.outError("Cannot connect to database");
        return false;
    }

    return true;
}

/// Define hook 'OnSignal' for all termination signals
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

/// Unhook the signals before leaving
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

/// @}
