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
#include "WorldSocket.h"
#include "WorldSocketMgr.h"
#include "World.h"
#include "Log.h"
#include "Timer.h"
#include <signal.h>
#include "MapManager.h"
#include "AddonHandler.h"
#include "WorldHandler.h"

#include <ace/Event_Handler.h>
#include <ace/Thread_Manager.h>
#include <ace/TP_Reactor.h>

/*#ifdef ENABLE_CLI
#include "CliRunnable.h"

INSTANTIATE_SINGLETON_1( CliRunnable );
#endif*/

//#pragma warning(disable:4305)

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Singleton<Master, ACE_Recursive_Thread_Mutex>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Singleton<Master, ACE_Recursive_Thread_Mutex>
#elif defined (__GNUC__) && (defined (_AIX) || defined (__hpux))
template ACE_Singleton<Master, ACE_Recursive_Thread_Mutex> * ACE_Singleton<Master, ACE_Recursive_Thread_Mutex>::singleton_;
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

volatile bool Master::m_stopEvent = false;

static int m_thrgrp[THR_GRP_SIZE];

static ACE_THR_FUNC_RETURN MapManagerWorker (void *)
{
	ACE_TRACE("MapManagerWorker (void *)");

	ACE_DEBUG((LM_DEBUG, "(%t) Spawning MapManager thread...\n"));
	ACE_Time_Value realCurrTime = ACE_Time_Value::zero , realPrevTime = ACE_Time_Value::zero;
	ACE_Thread_Manager *thr_mgr = ACE_Thread_Manager::instance ();
	while (thr_mgr->testcancel(ACE_OS::thr_self ()) == 0)
	{
		if (realPrevTime > realCurrTime)
			realPrevTime = ACE_Time_Value::zero;

		realCurrTime = ACE_OS::gettimeofday();
		MapManager::Instance().Update(realCurrTime.msec() - realPrevTime.msec());
		realPrevTime = realCurrTime;
		ACE_OS::sleep(ACE_Time_Value (0, 100000));
	}
	ACE_DEBUG ((LM_DEBUG, "(%t) MapManager Thread Closed\n"));
	return 0;
}

static ACE_THR_FUNC_RETURN MangosdthreadFunc(void *arg)
{
    ACE_TRACE("MangosdthreadFunc(void *)");

	ACE_Reactor *reactor = (ACE_Reactor *) arg;
	reactor->run_reactor_event_loop();

	ACE_DEBUG ((LM_DEBUG, "(%t) Reactor Thread Closed\n"));

    return 0;
}


void Master::_OnSignal(int s)
{
	//ACE_DEBUG ((LM_DEBUG, "(%t) received signal %d\n", s));

    /*switch (s)
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
    }*/

    ACE_OS::signal(s, (ACE_SignalHandler)_OnSignal);
	
	ACE_Thread_Manager *thr_mgr = ACE_Thread_Manager::instance ();

	//ACE_DEBUG ((LM_DEBUG, "(%t) signaling group %d \n", m_thrgrp[THR_GRP_MAPMANAGER]));
	/*if (thr_mgr->kill_grp (m_thrgrp[THR_GRP_MAPMANAGER], SIGINT) == -1)
		ACE_ERROR ((LM_DEBUG, "(%t) %p\n", "kill_grp"));*/

	/*if (thr_mgr->cancel_grp (m_thrgrp[THR_GRP_MAPMANAGER]) == -1)
		ACE_ERROR ((LM_DEBUG, "(%t) %p\n", "cancel_grp"));
	
	ACE_Thread_Manager::instance()->wait_grp(m_thrgrp[THR_GRP_MAPMANAGER]);*/

	ACE_DEBUG ((LM_DEBUG, "(%t) signaling group %d \n", m_thrgrp[THR_GRP_REACTOR]));
	
	/*if (thr_mgr->kill_grp (m_thrgrp[THR_GRP_REACTOR], SIGINT) == -1)
		ACE_ERROR ((LM_DEBUG, "(%t) %p\n", "kill_grp"));*/

	if (thr_mgr->cancel_grp (m_thrgrp[THR_GRP_REACTOR]) == -1)
		ACE_ERROR ((LM_DEBUG, "(%t) %p\n", "cancel_grp"));
	
	ACE_Thread_Manager::instance()->wait_grp(m_thrgrp[THR_GRP_REACTOR]);
}

Master::Master()
{
}

Master::~Master()
{
}

bool Master::Run()
{
	// create a reactor from a TP reactor
	// Register a signal handler.
	_HookSignals();

    ACE_TP_Reactor tpReactor;
    ACE_Reactor new_reactor(&tpReactor);
	ACE_Reactor::instance (&new_reactor);

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
	
	if(_StartDB())
		ACE_ERROR_RETURN ((LM_ERROR, ACE_TEXT("[%D]:%M:%N:%l: ") ACE_TEXT("Couldn't Start Database.\n")), -1);

    sWorld.SetPlayerLimit( sConfig->GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT) );
    sWorld.SetMotd( sConfig->GetStringDefault("Motd", "Welcome to the Massive Network Game Object Server.").c_str() );

    sWorld.SetInitialWorldSettings();

    uint32 rmport = sWorld.getConfig(CONFIG_PORT_REALM);           
    uint32 wsport = sWorld.getConfig(CONFIG_PORT_WORLD);

	WorldSocketMgr peer_acceptor;
	
	if (peer_acceptor.open (ACE_INET_Addr (wsport), &new_reactor, ACE_NONBLOCK) == -1)
		ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "open"), -1);
	
	WorldHandler *wh = 0;
	ACE_NEW_RETURN(wh,WorldHandler, -1);
	m_thrgrp[THR_GRP_SESSION] = wh->activate(THR_NEW_LWP | THR_DETACHED);

    m_thrgrp[THR_GRP_REACTOR] = ACE_Thread_Manager::instance()->spawn_n(5 , ACE_THR_FUNC(MangosdthreadFunc), &new_reactor);

	//m_thrgrp[THR_GRP_MAPMANAGER] = ACE_Thread_Manager::instance()->spawn(ACE_THR_FUNC(MapManagerWorker), 0, THR_NEW_LWP | THR_DETACHED);

	ACE_Thread_Manager::instance()->wait();

	return 0;
}

int
Master::_StartDB()
{
    ACE_TRACE("StartDB()");
    std::string dbstring;
    if(!sConfig->GetString("WorldDatabaseInfo", &dbstring))
       ACE_ERROR_RETURN ((LM_ERROR, "World Database not specified.\n"), -1);

    sLog.outString("World Database: %s", dbstring.c_str() );
    if(!sDatabase.Initialize(dbstring.c_str()))
       ACE_ERROR_RETURN ((LM_ERROR, "Cannot connect to World database.\n"), -1);

    if(!sConfig->GetString("LoginDatabaseInfo", &dbstring))
        ACE_ERROR_RETURN ((LM_ERROR, "Login Database not specified.\n"), -1);

    sLog.outString("Login Database: %s", dbstring.c_str() );
    if(!loginDatabase.Initialize(dbstring.c_str()))
        ACE_ERROR_RETURN ((LM_ERROR, "Cannot connect to login database.\n"), -1);

    realmID = sConfig->GetIntDefault("RealmID", 0);
    if(!realmID)
		ACE_ERROR_RETURN ((LM_ERROR, "Realm ID not defined.\n"), -1);

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("Realm running as realm ID %d.\n"), realmID));
   
    sDatabase.PExecute("UPDATE `character` SET `online` = 0;");
	
	sConfig->GetString("WorldDatabaseInfo", &dbstring);

	if(sDatabaseMysql->AddDatabase(dbstring.c_str(), DATABASE_WORLD) == 0)
	{
		ACE_ERROR_RETURN ((LM_ERROR, "Cannot connect to World database: %s.\n", sDatabaseMysql->GetDatabaseError(DATABASE_WORLD)), -1);
	}
	sConfig->GetString("LoginDatabaseInfo", &dbstring);
	if(sDatabaseMysql->AddDatabase(dbstring.c_str(), DATABASE_LOGIN) == 0)
	{
		ACE_ERROR_RETURN ((LM_ERROR, "Cannot connect to Login database: %s.\n", sDatabaseMysql->GetDatabaseError(DATABASE_LOGIN)), -1);
	}

    clearOnlineAccounts();
    return 0;
}

void Master::_StopDB()
{
    clearOnlineAccounts();
}

void Master::clearOnlineAccounts()
{
    // Cleanup online status for characters hosted at current realm
    loginDatabase.PExecute(
        "UPDATE `account`,`realmcharacters` SET `account`.`online` = 0 "
        "WHERE `account`.`online` > 0 AND `account`.`id` = `realmcharacters`.`acctid` "
        "  AND `realmcharacters`.`realmid` = %d;",realmID);
}

void Master::_HookSignals()
{
	ACE_Sig_Action sa ((ACE_SignalHandler) _OnSignal, SIGINT);
	ACE_UNUSED_ARG (sa);

  /*  signal(SIGINT, );
    signal(SIGQUIT, _OnSignal);
    signal(SIGTERM, _OnSignal);
    signal(SIGABRT, _OnSignal);
    #ifdef _WIN32
    signal(SIGBREAK, _OnSignal);
    #endif*/
}

/*
void Master::_UnhookSignals()
{
    signal(SIGINT, 0);
    signal(SIGQUIT, 0);
    signal(SIGTERM, 0);
    signal(SIGABRT, 0);
    #ifdef _WIN32
    signal(SIGBREAK, 0);
    #endif

}*/

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Acceptor <WorldSocket, ACE_SOCK_ACCEPTOR>;
template class ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Acceptor <WorldSocket, ACE_SOCK_ACCEPTOR>
#pragma instantiate ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */
