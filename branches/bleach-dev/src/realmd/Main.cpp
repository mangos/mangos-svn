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
#include "SystemConfig.h"

#include <ace/Event_Handler.h>
#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <ace/Thread_Manager.h>
#include <ace/TP_Reactor.h>
#include <ace/Get_Opt.h>
#include <ace/Service_Config.h>
#include <ace/ARGV.h>

#include "RealmAcceptor.h"

DatabaseMysql stDatabaseMysql;

/**
 * This is the function Handles any signal that has happened.
 *
 * @param arg is expected to be of type (int signum)
 */ 
static void external_handler (int signum)
{
	ACE_DEBUG ((LM_DEBUG, "\nsignal %S occurred in external handler!", signum));
}

/**
 * This is the function initialize the Database.
 *
 * @param arg is expected to be of type (std::string &dbstring)
 */

static int StartDB(std::string &dbstring)
{
	ACE_TRACE("StartDB(std::string &dbstring)");

    if(!sConfig->GetString("LoginDatabaseInfo", &dbstring))
    {
		ACE_ERROR_RETURN ((LM_ERROR, "Database not specified.\n"), -1);
    }
	ACE_DEBUG ((LM_DEBUG, ACE_TEXT("Database: %s\n"), dbstring.c_str()));
    if(!stDatabaseMysql.Initialize(dbstring.c_str()))
    {
		ACE_ERROR_RETURN ((LM_ERROR, "Cannot connect to database.\n"), -1);
    }
    // Right now we just clear all logged in accounts on boot up
    // We should query the realms and ask them.
    stDatabaseMysql.PExecute("UPDATE `account` SET `online` = 0;");
    ACE_RETURN(0);
}

/**
 * This is variable is used to show realmd usage.
 *
 */
const ACE_TCHAR usage_[] =
ACE_TEXT( "usage: realmd [options]\n" )
ACE_TEXT( "options:\n" )
ACE_TEXT( "    -f , --foreground\n" )
ACE_TEXT( "        Runs daemon on foreground.\n" )
ACE_TEXT( "        Output is directed to console. Default output is to log file.\n" )
ACE_TEXT( "    -c , --config\n" )
ACE_TEXT( "        Sets configuration file to use. Default configuration file is used.\n" )
ACE_TEXT( "    -d, --debug\n" )
ACE_TEXT( "        Output debug messages.\n" )
ACE_TEXT( "    -v V, --verbose=V\n" )
ACE_TEXT( "        Sets verbose level. Default is 2.\n" )
ACE_TEXT( "        0 - Prepends timestamp and message priority to each message.\n" )
ACE_TEXT( "        1 - Prepends timestamp and message priority to each message.\n" )
ACE_TEXT( "        2 - Verbose is disable.\n" );

/**
 * This is the function run by all threads in the thread pool.
 *
 * @param arg is expected to be of type (ACE_Reactor *)
 */
ACE_THR_FUNC_RETURN RealmdthreadFunc(void *arg)
{
    ACE_TRACE("RealmdthreadFunc(void *)");

	ACE_Reactor *reactor = (ACE_Reactor *) arg;
	reactor->run_reactor_event_loop();

    return 0;
}

/**
 * The main function sets up the TP reactor.
 */
int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	ACE_Get_Opt get_opt (argc, argv, ACE_TEXT ("f:"));

    get_opt.long_option( ACE_TEXT( "foreground" ),	'f', ACE_Get_Opt::NO_ARG );
	/*get_opt.long_option( ACE_TEXT( "debug" ),		'd', ACE_Get_Opt::NO_ARG );
    get_opt.long_option( ACE_TEXT( "verbose" ),		'v',ACE_Get_Opt::ARG_REQUIRED );
	get_opt.long_option( ACE_TEXT( "config" ),		'c',ACE_Get_Opt::ARG_REQUIRED );*/

	ACE_UINT32 debug = 0;
	ACE_UINT32 foreground = 0;
	ACE_UINT32 verbose = 2;

	ACE_TString cfg = _REALMD_CONFIG;
	ACE_TString longopt;

	int c;
	while ((c = get_opt()) != -1)
	{
		switch (c)
		{
			case 'f': foreground = 1; break;
			/*case 'd': debug = 1; break;
			case 'v': verbose = ACE_OS::atoi(get_opt.opt_arg()); break;
			case 'c': cfg = get_opt.opt_arg(); break;*/
			case 0:	longopt = get_opt.long_option();
				if( longopt == "foreground" )
				{
					foreground = 1;
				} 
				/*else if( longopt == "debug" )
				{
					debug = 1;
				}
				else if ( longopt == "verbose" )
				{
					verbose = ACE_OS::atoi(get_opt.opt_arg());
				}
				else if ( longopt == "config" )
				{
					cfg = get_opt.opt_arg();
				}
				else if (longopt == "help")
				{
					ACE_ERROR_RETURN(( LM_ERROR, usage_ ), 0 );
				}*/
				else
					ACE_ERROR_RETURN(( LM_ERROR, usage_ ), 0 );
				break;
			default:  ACE_ERROR_RETURN(( LM_ERROR, usage_ ), 0 );
		}
	}
    if (!sConfig->SetSource(cfg.c_str()))
    {
		ACE_ERROR_RETURN ((LM_ERROR, ACE_TEXT ("Could not find configuration file %s\n"), cfg.c_str()), -1);
    }
	
	std::string logfile = sConfig->GetStringDefault("RealmServerLogFile", "realmd.log");

	/*LogCallback *logcallback = 0;
	ACE_NEW_NORETURN (logcallback, LogCallback(argv[0], logfile.c_str(), debug, verbose));
	if (logcallback == 0)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("[%D]:%M:%N:%l: ") ACE_TEXT("Failed to allocate logcallback.") ACE_TEXT ("(errno = %i: %m)\n"), errno), -1);
	}
	ACE_LOG_MSG->set_flags (ACE_Log_Msg::MSG_CALLBACK);
	ACE_LOG_MSG->msg_callback (logcallback);

	if(foreground == 0)
	{
		ACE_LOG_MSG->clr_flags (ACE_Log_Msg::STDERR);
		ACE::daemonize(ACE_TEXT("/"), 0, argv[0]);
	}*/
	ACE_DEBUG ( (LM_INFO, ACE_TEXT("MaNGOS realm daemon %s\n"), _FULLVERSION) );
	ACE_DEBUG ( (LM_INFO, ACE_TEXT("Using configuration file %s.\n"), cfg.c_str() ));
    
    // Non-critical warning about conf file version
    uint32 confVersion = sConfig->GetIntDefault("ConfVersion", 0);
    if (confVersion < _REALMDCONFVERSION)
    {
        ACE_DEBUG ( (LM_WARNING, ACE_TEXT ("*****************************************************************************\n") ));
        ACE_DEBUG ( (LM_WARNING, ACE_TEXT (" WARNING: Your realmd.conf version indicates your conf file is out of date!\n") ));
        ACE_DEBUG ( (LM_WARNING, ACE_TEXT ("          Please check for updates, as your current default values may cause\n") ));
        ACE_DEBUG ( (LM_WARNING, ACE_TEXT ("          strange behavior.\n") ));
        ACE_DEBUG ( (LM_WARNING, ACE_TEXT ("*****************************************************************************\n") ));
        clock_t pause = 3000 + clock();
        while (pause > clock());
    }

	// create a reactor from a TP reactor
    ACE_TP_Reactor tpReactor;
    ACE_Reactor reactor(&tpReactor);

	RealmAcceptor peer_acceptor;
	
	if (peer_acceptor.open (ACE_INET_Addr (3724),
                          &reactor, ACE_NONBLOCK) == -1)
    ACE_ERROR_RETURN ((LM_ERROR,
                       "%p\n",
                       "open"),
                      -1);

	std::string dbstring;
    if(StartDB(dbstring))
	{
		ACE_ERROR_RETURN ((LM_ERROR, ACE_TEXT("[%D]:%M:%N:%l: ") ACE_TEXT("Couldn't Start Database.\n")), -1);
	}
	sRealmList->GetAndAddRealms(dbstring);
    if (sRealmList->size() == 0)
    {
		ACE_ERROR_RETURN ((LM_ERROR, ACE_TEXT("[%D]:%M:%N:%l: ") ACE_TEXT("No valid realms specified.\n")), -1);
    }

    ACE_Thread_Manager::instance()->spawn_n(sConfig->GetIntDefault("RealmNumberOfThreads", 5), RealmdthreadFunc, &reactor);
	
	ACE_Thread_Manager::instance()->wait();
	return 0;
}

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Acceptor <RealmHandler, ACE_SOCK_ACCEPTOR>;
template class ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Acceptor <RealmHandler, ACE_SOCK_ACCEPTOR>
#pragma instantiate ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */
