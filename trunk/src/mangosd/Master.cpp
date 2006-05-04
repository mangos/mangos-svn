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

#include "Master.h"
#include "Network/SocketHandler.h"
#include "Network/ListenSocket.h"
#include "Network/TcpSocket.h"
//#include "AuthSocket.h"
#include "WorldSocket.h"
#include "RASocket.h"
#include "WorldSocketMgr.h"
#include "WorldRunnable.h"
#include "World.h"
//#include "RealmList.h"
#include "Log.h"
#include "Timer.h"
#include <signal.h>
#include "MapManager.h"
#include "Policies/SingletonImp.h"

#ifdef ENABLE_CLI
#include "CliRunnable.h"

INSTANTIATE_SINGLETON_1( CliRunnable );
#endif

#pragma warning(disable:4305)

INSTANTIATE_SINGLETON_1( Master );

volatile bool Master::m_stopEvent = false;

void Master::_OnSignal(int s)
{
    switch (s)
    {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
        case SIGABRT:
            Master::m_stopEvent = true;
            break;
        #ifdef _WIN32
        case SIGBREAK:
            Master::m_stopEvent = true;
            break;
        #endif
    }

    signal(s, _OnSignal);
}

Master::Master()
{
}

Master::~Master()
{
}

bool Master::Run()
{
    sLog.outString( "MaNGOS daemon %s", _FULLVERSION );
    sLog.outString( "<Ctrl-C> to stop.\n\n" );

    sLog.outString( "MM   MM         MM   MM  MMMMM   MMMM   MMMMM");
    sLog.outString( "MM   MM         MM   MM MMM MMM MM  MM MMM MMM");
    sLog.outString( "MMM MMM         MMM  MM MMM MMM MM  MM MMM");
    sLog.outString( "MM M MM         MMMM MM MMM     MM  MM  MMM");
    sLog.outString( "MM M MM  MMMMM  MM MMMM MMM     MM  MM   MMM");
    sLog.outString( "MM M MM M   MMM MM  MMM MMMMMMM MM  MM    MMM");
    sLog.outString( "MM   MM     MMM MM   MM MM  MMM MM  MM     MMM");
    sLog.outString( "MM   MM MMMMMMM MM   MM MMM MMM MM  MM MMM MMM");
    sLog.outString( "MM   MM MM  MMM MM   MM  MMMMMM  MMMM   MMMMM");
    sLog.outString( "        MM  MMM http://www.mangosproject.org");
    sLog.outString( "        MMMMMM\n\n");

    _StartDB();

    //loglevel = (uint8)sConfig.GetIntDefault("LogLevel", DEFAULT_LOG_LEVEL);

    std::string host;
    host = sConfig.GetStringDefault( "Host", DEFAULT_HOST );
    sLog.outString( "Server: %s\n", host.c_str() );

    sWorld.SetPlayerLimit( sConfig.GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT) );
    sWorld.SetMotd( sConfig.GetStringDefault("Motd", "Welcome to the Massive Network Game Object Server." ).c_str() );
    sWorld.SetInitialWorldSettings();

    port_t wsport, rmport;
    rmport = sWorld.getConfig(CONFIG_PORT_REALM);//sConfig.GetIntDefault( "RealmServerPort", DEFAULT_REALMSERVER_PORT );
    wsport = sWorld.getConfig(CONFIG_PORT_WORLD);//sConfig.GetIntDefault( "WorldServerPort", DEFAULT_WORLDSERVER_PORT );

    uint32 socketSelecttime;
    socketSelecttime = sWorld.getConfig(CONFIG_SOCKET_SELECTTIME);//sConfig.GetIntDefault( "SocketSelectTime", DEFAULT_SOCKET_SELECT_TIME );

    //sWorld.setRate(RATE_HEALTH,sConfig.GetFloatDefault("Rate.Health",DEFAULT_REGEN_RATE));
    //sWorld.setRate(RATE_POWER1,sConfig.GetFloatDefault("Rate.Power1",DEFAULT_REGEN_RATE));
    //sWorld.setRate(RATE_POWER2,sConfig.GetFloatDefault("Rate.Power2",DEFAULT_REGEN_RATE));
    //sWorld.setRate(RATE_POWER3,sConfig.GetFloatDefault("Rate.Power4",DEFAULT_REGEN_RATE));
    //sWorld.setRate(RATE_DROP,sConfig.GetFloatDefault("Rate.Drop",DEFAULT_DROP_RATE));
    //sWorld.setRate(RATE_XP,sConfig.GetFloatDefault("Rate.XP",DEFAULT_XP_RATE));

    //uint32 grid_clean_up_delay = sConfig.GetIntDefault("GridCleanUpDelay", 300);
    //sLog.outDebug("Setting Grid clean up delay to %d seconds.", grid_clean_up_delay);
    //grid_clean_up_delay *= 1000;
    //MapManager::Instance().SetGridCleanUpDelay(grid_clean_up_delay);

    //uint32 map_update_interval = sConfig.GetIntDefault("MapUpdateInterval", 100);
    //sLog.outDebug("Setting map update interval to %d milli-seconds.", map_update_interval);
    //MapManager::Instance().SetMapUpdateInterval(map_update_interval);

    //    sRealmList.setServerPort(wsport);
    //    sRealmList.GetAndAddRealms ();
    SocketHandler h;
    ListenSocket<WorldSocket> worldListenSocket(h);
    //    ListenSocket<AuthSocket> authListenSocket(h);

    if (worldListenSocket.Bind(wsport))
    {
        _StopDB();
        sLog.outString( "MaNGOS can not bind to that port" );
        exit(1);
    }

    h.Add(&worldListenSocket);
    //    h.Add(&authListenSocket);

    _HookSignals();

    ZThread::Thread t(new WorldRunnable);

    //#ifndef WIN32
    t.setPriority ((ZThread::Priority )2);
    //#endif

    #ifdef ENABLE_CLI
    ZThread::Thread td1(new CliRunnable);
    #endif

    #ifdef ENABLE_RA

    ListenSocket<RASocket> RAListenSocket(h);

    if (RAListenSocket.Bind(sConfig.GetIntDefault( "RA.Port", 3443 )))
    {

        sLog.outString( "MaNGOS can not bind to that port" );
        // exit(1); go on with no RA

    }

    h.Add(&RAListenSocket);
    #endif
    uint32 realCurrTime, realPrevTime;
    realCurrTime = realPrevTime = getMSTime();
    while (!Master::m_stopEvent)
    {

        if (realPrevTime > realCurrTime)
            realPrevTime = 0;

        realCurrTime = getMSTime();
        sWorldSocketMgr.Update( realCurrTime - realPrevTime );
        realPrevTime = realCurrTime;

        //h.Select(0, 100000);
        h.Select(0, socketSelecttime);
    }

    _UnhookSignals();

    t.wait();

    _StopDB();

    sLog.outString( "Halting process..." );
    return 0;
}

bool Master::_StartDB()
{
    std::string dbstring;
    if(!sConfig.GetString("DatabaseInfo", &dbstring))
    {
        sLog.outError("Database not specified");
        exit(1);

    }

    sLog.outString("Database: %s", dbstring.c_str() );
    if(!sMySqlDatabase.Initialize(dbstring.c_str()))
    {
        sLog.outError("Cannot connect to database");
        exit(1);

    }

    sDatabase.PExecute("UPDATE `character` SET `online` = 0;");
    return true;
}

void Master::_StopDB()
{
}

void Master::_HookSignals()
{
    signal(SIGINT, _OnSignal);
    signal(SIGQUIT, _OnSignal);
    signal(SIGTERM, _OnSignal);
    signal(SIGABRT, _OnSignal);
    #ifdef _WIN32
    signal(SIGBREAK, _OnSignal);
    #endif
}

void Master::_UnhookSignals()
{
    signal(SIGINT, 0);
    signal(SIGQUIT, 0);
    signal(SIGTERM, 0);
    signal(SIGABRT, 0);
    #ifdef _WIN32
    signal(SIGBREAK, 0);
    #endif

}
