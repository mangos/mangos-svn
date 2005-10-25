/* Master.cpp
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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
#include "AuthSocket.h"
#include "WorldSocket.h"
#include "WorldSocketMgr.h"
#include "WorldRunnable.h"
#include "HttpdRunnable.h"
#include "World.h"
#include "RealmList.h"
#include "Log.h"
#include "Timer.h"
#include <signal.h>

#ifdef ENABLE_GRID_SYSTEM
#include "MapManager.h"
#endif

createFileSingleton( Master );

volatile bool Master::m_stopEvent = false;

void Master::_OnSignal(int s)
{
    switch (s)
    {
        case SIGINT:
            Master::m_stopEvent = true;
            break;
        case SIGTERM:
            Master::m_stopEvent = true;
            break;
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

#include "../shared/Database/DataStore.h"

bool Master::Run()
{
    sLog.outString( "MaNGOS daemon %s", _FULLVERSION );
    sLog.outString( "<Ctrl-C> to stop.\n" );

    _StartDB();

    loglevel = (uint8)sConfig.GetIntDefault("LogLevel", DEFAULT_LOG_LEVEL);

    std::string host;
    host = sConfig.GetStringDefault( "Host", DEFAULT_HOST );
    sLog.outString( "Server: %s\n", host.c_str() );

    new World;

    sWorld.SetPlayerLimit( sConfig.GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT) );
    sWorld.SetMotd( sConfig.GetStringDefault("Motd", "Welcome to the Massive Network Game Object Server." ).c_str() );
    sWorld.SetInitialWorldSettings();

    port_t wsport, rmport;
    rmport = sConfig.GetIntDefault( "RealmServerPort", DEFAULT_REALMSERVER_PORT );
    wsport = sConfig.GetIntDefault( "WorldServerPort", DEFAULT_WORLDSERVER_PORT );    

    // load regeneration rates.
    sWorld.setRate(RATE_HEALTH,sConfig.GetFloatDefault("Rate.Health",DEFAULT_REGEN_RATE));
    sWorld.setRate(RATE_POWER1,sConfig.GetFloatDefault("Rate.Power1",DEFAULT_REGEN_RATE));
    sWorld.setRate(RATE_POWER2,sConfig.GetFloatDefault("Rate.Power2",DEFAULT_REGEN_RATE));
    sWorld.setRate(RATE_POWER3,sConfig.GetFloatDefault("Rate.Power4",DEFAULT_REGEN_RATE));
    sWorld.setRate(RATE_DROP,sConfig.GetFloatDefault("Rate.Drop",DEFAULT_DROP_RATE));
    sWorld.setRate(RATE_XP,sConfig.GetFloatDefault("Rate.XP",DEFAULT_XP_RATE));

#ifdef ENABLE_GRID_SYSTEM
    // default Grid unload will be 5 minutes after everyone moved out.
    // Note.. minium is 1 minute. (5*60=300)
    uint32 grid_clean_up_delay = sConfig.GetIntDefault("GridCleanUpDelay", 300);    
    sLog.outDebug("Setting Grid clean up delay to %d seconds.", grid_clean_up_delay);
    grid_clean_up_delay *= 1000;
    MapManager::Instance().SetGridCleanUpDelay(grid_clean_up_delay);

    // default update is 100 milli seconds
    uint32 map_update_interval = sConfig.GetIntDefault("MapUpdateInterval", 100);
    sLog.outDebug("Setting map update interval to %d milli-seconds.", map_update_interval);
    MapManager::Instance().SetMapUpdateInterval(map_update_interval);
#endif

    sRealmList.setServerPort(wsport);
    sRealmList.GetAndAddRealms ();
    SocketHandler h;
    ListenSocket<WorldSocket> worldListenSocket(h);
    ListenSocket<AuthSocket> authListenSocket(h);

    if (worldListenSocket.Bind(wsport) || authListenSocket.Bind(rmport))
    {
        delete World::getSingletonPtr();
        _StopDB();
	sLog.outString( "MaNGOS can not bind to that port" );
	exit(1);
///        return 0;
    }

    h.Add(&worldListenSocket);
    h.Add(&authListenSocket);

    _HookSignals();

    ZThread::Thread t(new WorldRunnable);

	// UQ1: Httpd
	ZThread::Thread t2(new HttpdRunnable);

    uint32 realCurrTime, realPrevTime;
    realCurrTime = realPrevTime = getMSTime();
    while (!Master::m_stopEvent)
    {
        // uint32 exceeded
        if (realPrevTime > realCurrTime)
            realPrevTime = 0;

        realCurrTime = getMSTime();
        sWorldSocketMgr.Update( realCurrTime - realPrevTime );
        realPrevTime = realCurrTime;

        h.Select(0, 100000);                      // 100 ms
    }

    _UnhookSignals();

    t.wait();
    delete World::getSingletonPtr();

    _StopDB();

    sLog.outString( "Halting process..." );
    return 0;
}


bool Master::_StartDB()
{
    ASSERT(new DatabaseMysql);

    std::string dbstring;

    if(!sConfig.GetString("DatabaseInfo", &dbstring))
    {
        sLog.outError("Database not specified");
		exit(1);
        //return false;
    }

    sLog.outString("Database: %s", dbstring.c_str() );

    if(!sDatabase.Initialize(dbstring.c_str()))
    {
        sLog.outError("Cannot connect to database");
		exit(1);
        //return false;
    }

/// clean online table
	std::stringstream query;
        query << "UPDATE characters SET online=0";
        QueryResult *result = sDatabase.Query( query.str().c_str() );
	delete result;
    return true;
}


void Master::_StopDB()
{
    delete Database::getSingletonPtr();
}


void Master::_HookSignals()
{
    signal(SIGINT, _OnSignal);
    signal(SIGTERM, _OnSignal);
    signal(SIGABRT, _OnSignal);
#ifdef _WIN32
    signal(SIGBREAK, _OnSignal);
#endif
}


void Master::_UnhookSignals()
{
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
    signal(SIGABRT, 0);
#ifdef _WIN32
    signal(SIGBREAK, 0);
#endif

}
