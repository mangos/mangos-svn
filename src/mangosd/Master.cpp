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

/** \file
    \ingroup mangosd
*/

#include "Master.h"
#include "Network/SocketHandler.h"
#include "Network/ListenSocket.h"
#include "WorldSocket.h"
#include "WorldSocketMgr.h"
#include "WorldRunnable.h"
#include "World.h"
#include "Log.h"
#include "Timer.h"
#include <signal.h>
#include "Policies/SingletonImp.h"
#include "SystemConfig.h"
#include "Config/ConfigEnv.h"
#include "Database/DatabaseEnv.h"
#include "CliRunnable.h"
#include "RASocket.h"

#include "Network/TcpSocket.h"
#include "Network/Utility.h"
#include "Network/Parse.h"
#include "Network/Socket.h"

/// \todo Warning disabling not useful under VC++2005. Can somebody say on which compiler it is useful?
#pragma warning(disable:4305)

#if defined( WIN32 ) && (_DEBUG)
#include <windows.h>
#include <tchar.h>
#include <process.h>
#include <mmsystem.h>

DWORD cthread1 = 0;
DWORD cthread2 = 0;

void __cdecl RunCLI ( void * )
{
    CliRunnable *client(new CliRunnable);
    client->run();
}

void __cdecl RunWorld ( void * )
{
    WorldRunnable *world(new WorldRunnable);
    world->run();
}
#endif //defined( WIN32 ) && (_DEBUG)

INSTANTIATE_SINGLETON_1( Master );

Master::Master()
{
}

Master::~Master()
{
}

/// Main function
void Master::Run()
{
    sLog.outString( "MaNGOS daemon %s", _FULLVERSION );
    sLog.outString( "<Ctrl-C> to stop.\n\n" );

    sLog.outTitle( "MM   MM         MM   MM  MMMMM   MMMM   MMMMM");
    sLog.outTitle( "MM   MM         MM   MM MMM MMM MM  MM MMM MMM");
    sLog.outTitle( "MMM MMM         MMM  MM MMM MMM MM  MM MMM");
    sLog.outTitle( "MM M MM         MMMM MM MMM     MM  MM  MMM");
    sLog.outTitle( "MM M MM  MMMMM  MM MMMM MMM     MM  MM   MMM");
    sLog.outTitle( "MM M MM M   MMM MM  MMM MMMMMMM MM  MM    MMM");
    sLog.outTitle( "MM   MM     MMM MM   MM MM  MMM MM  MM     MMM");
    sLog.outTitle( "MM   MM MMMMMMM MM   MM MMM MMM MM  MM MMM MMM");
    sLog.outTitle( "MM   MM MM  MMM MM   MM  MMMMMM  MMMM   MMMMM");
    sLog.outTitle( "        MM  MMM http://www.mangosproject.org");
    sLog.outTitle( "        MMMMMM\n\n");

    ///- Start the databases
    if (!_StartDB())
        return;

    ///- Initialize the World
    sWorld.SetInitialWorldSettings();

    ///- Launch the world listener socket
    port_t wsport = sWorld.getConfig(CONFIG_PORT_WORLD);

	SocketHandler h;

    ListenSocket<WorldSocket> worldListenSocket(h);
    if (worldListenSocket.Bind(wsport))
    {
        clearOnlineAccounts();
        sLog.outError("MaNGOS cannot bind to port %d", wsport);
        return;
    }

    h.Add(&worldListenSocket);

    ///- Catch termination signals
    _HookSignals();

    #if defined( WIN32 ) && (_DEBUG)
        ///- Launch WorldRunnable thread - Use windows threads for debugging!
        cthread2 = _beginthread( RunWorld, 0, LOWORD( 0) );
    #else //!defined( WIN32 ) && (_DEBUG)
        ///- Launch WorldRunnable thread
        ZThread::Thread t(new WorldRunnable);
        t.setPriority ((ZThread::Priority )2);
    #endif //!defined( WIN32 ) && (_DEBUG)

    if (sConfig.GetBoolDefault("Console.Enable", 1))
    {
        #if defined( WIN32 ) && (_DEBUG)
    	    cthread1 = _beginthread( RunCLI, 0, LOWORD( 0) );
        #else //!defined( WIN32 ) && (_DEBUG)
            ///- Launch CliRunnable thread
            ZThread::Thread td1(new CliRunnable);
        #endif //!defined( WIN32 ) && (_DEBUG)
    }

    ///- Launch the RA listener socket
    ListenSocket<RASocket> RAListenSocket(h);
    if (sConfig.GetBoolDefault("Ra.Enable", 0))
    {
        port_t raport = sConfig.GetIntDefault( "Ra.Port", 3443 );

        if (RAListenSocket.Bind(raport))
            sLog.outError( "MaNGOS RA can not bind to port %d", raport );
        else
            h.Add(&RAListenSocket);

        sLog.outString("Starting Remote access listner on port %d", raport);
    }

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
                    sLog.outError("Processors marked in UseProcessors bitmask (hex) %x not accessible for mangosd. Accessible processors bitmask (hex): %x",Aff,appAff);
                }
                else
                {
                    if(SetProcessAffinityMask(hProcess,curAff))
                        sLog.outString("Using processors (bitmask, hex): %x", curAff);
                    else
                        sLog.outError("Can't set used processors (hex): %x",curAff);
                }
            }
            sLog.outString("");
        }

        uint32 Prio = sConfig.GetIntDefault("ProcessPriority", 0);

        if(Prio)
        {
            if(SetPriorityClass(hProcess,HIGH_PRIORITY_CLASS))
                sLog.outString("mangosd process priority class set to HIGH");
            else
                sLog.outError("ERROR: Can't set mangosd process priority class.");
            sLog.outString("");
        }
    }
    #endif

    uint32 realCurrTime, realPrevTime;
    realCurrTime = realPrevTime = getMSTime();

    uint32 socketSelecttime = sWorld.getConfig(CONFIG_SOCKET_SELECTTIME);

    // maximum counter for next ping
    uint32 numLoops = (sConfig.GetIntDefault( "MaxPingTime", 30 ) * (MINUTE * 1000000 / socketSelecttime));
    uint32 loopCounter = 0;

    ///- Wait for termination signal
    while (!World::m_stopEvent)
    {

        if (realPrevTime > realCurrTime)
            realPrevTime = 0;

        realCurrTime = getMSTime();
        sWorldSocketMgr.Update( realCurrTime - realPrevTime );
        realPrevTime = realCurrTime;

        h.Select(0, socketSelecttime);

        // ping if need
        if( (++loopCounter) == numLoops )
        {
            loopCounter = 0;
            sLog.outDetail("Ping MySQL to keep connection alive");
            delete sDatabase.Query("SELECT 1 FROM `command` LIMIT 1");
            delete loginDatabase.Query("SELECT 1 FROM `realmlist` LIMIT 1");
        }
    }

    ///- Remove signal handling before leaving
    _UnhookSignals();

#if defined( WIN32 ) && (_DEBUG)
    //t.wait(); // UQ1: I don't know why this is needed.. If it is, FIXME!
#else //!defined( WIN32 ) && (_DEBUG)
    t.wait();
#endif //!defined( WIN32 ) && (_DEBUG)

    ///- Clean database before leaving
    clearOnlineAccounts();

    sLog.outString( "Halting process..." );

    #ifdef WIN32
        // this only way to terminate CLI thread exist at Win32 (alt. way exist only in Windows Vista API)
        //_exit(1);
        // send keyboard input to safely unblock the CLI thread
        keybd_event('X',0x2d,0,0);
        keybd_event('X',0x2d,KEYEVENTF_KEYUP,0);
        keybd_event(VK_RETURN,0x1c,0,0);
        keybd_event(VK_RETURN,0x1c,KEYEVENTF_KEYUP,0);
    #endif

    return;
}

/// Initialize connection to the databases
bool Master::_StartDB()
{
    ///- Get world database info from configuration file
    std::string dbstring;
    if(!sConfig.GetString("WorldDatabaseInfo", &dbstring))
    {
        sLog.outError("Database not specified in configuration file");
        return false;
    }
    sLog.outString("World Database: %s", dbstring.c_str());

    ///- Initialise the world database
    if(!sDatabase.Initialize(dbstring.c_str()))
    {
        sLog.outError("Cannot connect to world database %s",dbstring.c_str());
        return false;
    }

    ///- Get login database info from configuration file
    if(!sConfig.GetString("LoginDatabaseInfo", &dbstring))
    {
        sLog.outError("Login database not specified in configuration file");
        return false;
    }

    ///- Initialise the login database
    sLog.outString("Login Database: %s", dbstring.c_str() );
    if(!loginDatabase.Initialize(dbstring.c_str()))
    {
        sLog.outError("Cannot connect to login database %s",dbstring.c_str());
        return false;
    }

    ///- Get the realm Id from the configuration file
    realmID = sConfig.GetIntDefault("RealmID", 0);
    if(!realmID)
    {
        sLog.outError("Realm ID not defined in configuration file");
        return false;
    }
    sLog.outString("Realm running as realm ID %d", realmID);

    ///- Clean the database before starting
    clearOnlineAccounts();

    return true;
}

/// Clear 'online' status for all accounts with characters in this realm
void Master::clearOnlineAccounts()
{
    // Cleanup online status for characters hosted at current realm
    /// \todo Only accounts with characters logged on *this* realm should have online status reset. Move the online column from 'account' to 'realmcharacters'?
    loginDatabase.PExecute(
        "UPDATE `account`,`realmcharacters` SET `account`.`online` = 0 "
        "WHERE `account`.`online` > 0 AND `account`.`id` = `realmcharacters`.`acctid` "
        "  AND `realmcharacters`.`realmid` = '%d'",realmID);

    sDatabase.Execute("UPDATE `character` SET `online` = 0");
}

/// Handle termination signals
/** Put the World::m_stopEvent to 'true' if a termination signal is caught **/
void Master::_OnSignal(int s)
{
    switch (s)
    {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
        case SIGABRT:
        #ifdef _WIN32
        case SIGBREAK:
        #endif
            World::m_stopEvent = true;
            break;
    }

    signal(s, _OnSignal);
}

/// Define hook '_OnSignal' for all termination signals
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

/// Unhook the signals before leaving
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
