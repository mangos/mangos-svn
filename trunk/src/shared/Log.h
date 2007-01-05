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

enum Color
{
    BLACK,
    RED,
    GREEN,
    BROWN,
    BLUE,
    MAGENTA,
    CYAN,
    GREY,
    YELLOW,
    LRED,
    LGREEN,
    LBLUE,
    LMAGENTA,
    LCYAN,
    WHITE
};

const int Color_count = int(WHITE)+1;

class Log : public MaNGOS::Singleton<Log, MaNGOS::ClassLevelLockable<Log, ZThread::FastMutex> >
{
    friend class MaNGOS::OperatorNew<Log>;
    Log() : logfile(NULL), gmlogfile(NULL), m_colored(false) { Initialize(); }
    ~Log()
    {
        if( logfile != NULL )
            fclose(logfile);
        logfile = NULL;

        if( gmlogfile != NULL )
            fclose(gmlogfile);
        gmlogfile = NULL;

        if( dberlogfile != NULL )
            fclose(dberlogfile);
        dberlogfile = NULL;
    }
    public:
        void Initialize();
        void InitColors(std::string init_str);
        void outTitle( const char * str);
        void outCommand( const char * str, ...);
        void outString( const char * str, ... );
        void outError( const char * err, ... );
        void outBasic( const char * str, ... );
        void outDetail( const char * str, ... );
        void outDebug( const char * str, ... );
        void outMenu( const char * str, ... );
        void outErrorDb( const char * str, ... );
        void SetLogLevel(char * Level);
        void SetLogFileLevel(char * Level);
        void SetColor(bool stdout_stream, Color color);
        void ResetColor(bool stdout_stream);
        void outTimestamp(FILE* file);
    private:
        FILE* logfile;
        FILE* gmlogfile;
        FILE* dberlogfile;
        uint32 m_logLevel;
        uint32 m_logFileLevel;
        bool m_colored;
        Color m_colors[4];
};

#define sLog MaNGOS::Singleton<Log>::Instance()

#ifdef MANGOS_DEBUG
#define DEBUG_LOG MaNGOS::Singleton<Log>::Instance().outDebug
#else
#define DEBUG_LOG
#endif
#endif

// primary for script library
void MANGOS_DLL_SPEC debug_log(const char * str, ...);
void MANGOS_DLL_SPEC error_log(const char * str, ...);
