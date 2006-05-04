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

//extern uint8 loglevel;
class Config;

class Log : public MaNGOS::Singleton<Log, MaNGOS::ClassLevelLockable<Log, ZThread::FastMutex> >
{
    friend class MaNGOS::OperatorNew<Log>;
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
    private:
        FILE* logfile;
        uint32 m_logLevel;

};

#define sLog MaNGOS::Singleton<Log>::Instance()

#ifdef MANGOS_DEBUG
#define DEBUG_LOG MaNGOS::Singleton<Log>::Instance().outDebug
#else
#define DEBUG_LOG
#endif
#endif
