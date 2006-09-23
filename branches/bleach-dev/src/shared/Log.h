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

#ifndef MANGOSSERVER_LOG_H
#define MANGOSSERVER_LOG_H

#include "Common.h"
#include "Policies/Singleton.h"

//#include <ace/Singleton.h>
#include <ace/Date_Time.h>
#include <ace/Log_Msg.h>

//extern uint8 loglevel;
class Config;

class MANGOS_DLL_SPEC Log : public MaNGOS::Singleton<Log, MaNGOS::ClassLevelLockable<Log, ZThread::FastMutex> >
{
    friend class MaNGOS::OperatorNew<Log>;
	//friend class ACE_Singleton<Log, ACE_Recursive_Thread_Mutex>;
    Log() : logfile(NULL) { Initialize(); }
    ~Log()
    {
        if( logfile != NULL )
            fclose(logfile);
        logfile = NULL;
    }
    public:
        void Initialize();
        void outString( const char * str, ... );
        void outError( const char * err, ... );
        void outBasic( const char * str, ... );
        void outDetail( const char * str, ... );
        void outDebug( const char * str, ... );
		void outMenu( const char * str, ... );

		void logInfo(const char* fmt, ...)
		{
			if( fmt )
			{
				char p[128];
				va_list ap;
				va_start(ap, fmt);
				(void) vsnprintf(p, 128, fmt, ap);
				va_end(ap);
				ACE_DEBUG(( LM_INFO, ACE_TEXT("%D:%N:%M:%l -- %I\n")));
			}
		};

	private:
        FILE* logfile;
        uint32 m_logLevel;

};

/*typedef ACE_Singleton<Log, ACE_Recursive_Thread_Mutex> LogSingleton;
#define sLog LogSingleton::instance()*/

#define sLog MaNGOS::Singleton<Log>::Instance()

#ifdef MANGOS_DEBUG
	#define DEBUG_LOG MaNGOS::Singleton<Log>::Instance().outDebug
#else
	#define DEBUG_LOG
#endif

/*#define DEBUG_PREFIX		ACE_TEXT("[DEBUG]:%I")
#define INFO_PREFIX			ACE_TEXT("[INFO]:%I")
#define NOTICE_PREFIX		ACE_TEXT("[NOTICE]:%I")
#define WARNING_PREFIX		ACE_TEXT("[WARNING]:%I")
#define ERROR_PREFIX		ACE_TEXT("[ERROR]:%I")
#define CRITICAL_PREFIX		ACE_TEXT("[CRITICAL]:%I")
#define ALERT_PREFIX		ACE_TEXT("[ALERT]:%I")
#define EMERGENCY_PREFIX	ACE_TEXT("[EMERGENCY]:%I")

#ifndef MaNGOS_DEBUG
	#define MaNGOS_DEBUG(FMT, ...)	ACE_DEBUG(( LM_DEBUG, DEBUG_PREFIX FMT __VA_ARGS__))
#endif

//#define MaNGOS_INFO(FMT, ...)		ACE_DEBUG(( LM_INFO, INFO_PREFIX FMT __VA_ARGS__ ACE_TEXT("\n")))
#ifndef MaNGOS_INFO
	#define MaNGOS_INFO(FMT, ...) ACE_DEBUG(( LM_INFO, ACE_TEXT("%D:%N:%l -- [%M]:%I") FMT __VA_ARGS__ ACE_TEXT("\n")))
#endif

#define MaNGOS_NOTICE(FMT, ...)		ACE_DEBUG(( LM_NOTICE, NOTICE_PREFIX FMT __VA_ARGS__ ACE_TEXT("\n")))
#define MaNGOS_WARNING(FMT, ...)	ACE_DEBUG(( LM_WARNING, WARNING_PREFIX FMT __VA_ARGS__ ACE_TEXT("\n")))
#define MaNGOS_ERROR(FMT, ...)		ACE_DEBUG(( LM_ERROR, ERROR_PREFIX FMT __VA_ARGS__ ACE_TEXT("\n")))
#define MaNGOS_CRITICAL(FMT, ...)	ACE_DEBUG(( LM_CRITICAL, CRITICAL_PREFIX FMT __VA_ARGS__ ACE_TEXT("\n")))
#define MaNGOS_ALERT(FMT, ...)		ACE_DEBUG(( LM_ALERT, ALERT_PREFIX FMT __VA_ARGS__ ACE_TEXT("\n")))
#define MaNGOS_EMERGENCY(FMT, ...)	ACE_DEBUG(( LM_EMERGENCY, EMERGENCY_PREFIX FMT __VA_ARGS__ ACE_TEXT("\n")))*/

// primary for script library
void MANGOS_DLL_SPEC debug_log(const char * str, ...);

#include "ace/streams.h"
#include "ace/Log_Msg_Callback.h"
#include "ace/Log_Record.h"
#include "ace/SString.h"

/*#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

/*class LogManager
{
public:
  LogManager ();
  ~LogManager ();

  void redirectToDaemon(const ACE_TCHAR *prog_name = ACE_TEXT (""));
  void redirectToSyslog(const ACE_TCHAR *prog_name = ACE_TEXT (""));
  void redirectToOStream (ACE_OSTREAM_TYPE *output);
  void redirectToFile (const char *filename);
  void redirectToStderr (void);
  ACE_Log_Msg_Callback * redirectToCallback (ACE_Log_Msg_Callback *callback);

  // ...
};*/

class LogCallback : public ACE_Log_Msg_Callback
{
	public:
		LogCallback(const ACE_TCHAR * prog_name, const ACE_TCHAR * filename, 
			ACE_UINT32 debug, ACE_UINT32 verbose) : m_progname(prog_name), m_debug(debug), 
			m_verbose(verbose)
		{
			this->output = new std::ofstream (filename);
		};
		~LogCallback()
		{
			delete this->output;
		};
		void log (ACE_Log_Record &log_record)
		{
			/*
			ACE_LOG_MSG->priority_mask (0, ACE_Log_Msg::PROCESS);
			ACE_LOG_MSG->priority_mask (LM_DEBUG | LM_NOTICE, ACE_Log_Msg::THREAD);

			ACE_LOG_MSG->priority_mask (LM_DEBUG | LM_NOTICE, ACE_Log_Msg::PROCESS);
			*/
			log_record.print (m_progname, 0, *this->output);
			if (m_verbose == 0)
			{
				log_record.print (m_progname, ACE_Log_Msg::VERBOSE, *this->output);
			}
			else if(m_verbose == 1)
			{
				log_record.print (m_progname, ACE_Log_Msg::VERBOSE_LITE, *this->output);
			}
			else
			{
				if (ACE_LOG_MSG->log_priority_enabled (ACE_Log_Priority (log_record.type())))
				{
					ACE_CString data = ACE_TEXT_ALWAYS_CHAR (log_record.msg_data ());
					*this->output << data.c_str ();
					this->output->flush();
				}
			}
		}
		private:
			ACE_OSTREAM_TYPE *output;
			const ACE_TCHAR * m_progname;
			ACE_UINT32 m_debug;
			ACE_UINT32 m_verbose;
};
/*
#define LOGGER_PORT 20009

class Callback : public ACE_Log_Msg_Callback
{
public:
  Callback ()
    {
      this->logger_ = new ACE_SOCK_Stream;
      ACE_SOCK_Connector connector;
      ACE_INET_Addr addr (LOGGER_PORT, ACE_DEFAULT_SERVER_HOST);

      if (connector.connect (*(this->logger_), addr) == -1)
        {
          delete this->logger_;
          this->logger_ = 0;
        }
    }

  virtual ~Callback ()
    {
      if (this->logger_)
        {
          this->logger_->close ();
        }
      delete this->logger_;
    }

  void log (ACE_Log_Record &log_record)
    {
      if (!this->logger_)
        {
          log_record.print
            (ACE_TEXT (""), ACE_Log_Msg::VERBOSE, cerr);
          return;
        }

      size_t len = log_record.length();
      log_record.encode ();

      if (this->logger_->send_n ((char *) &log_record, len) == -1)
        {
          delete this->logger_;
          this->logger_ = 0;
        }
    }

private:
  ACE_SOCK_Stream *logger_;
};
*/
#endif
