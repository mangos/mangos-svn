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
#include "Log.h"
#include "Policies/SingletonImp.h"
#include "Config/ConfigEnv.h"

#include <stdarg.h>

INSTANTIATE_SINGLETON_1( Log );

void Log::Initialize()
{
    std::string logfn=sConfig.GetStringDefault("ServerLogFile", "Server.log");
    logfile = fopen(logfn.c_str(), "w");
    m_logLevel = sConfig.GetIntDefault("LogLevel", 0);
}

void Log::outString( const char * str, ... )
{
    if( !str ) return;
    va_list ap;
    va_start(ap, str);
    vprintf( str, ap );
    va_end(ap);
    printf( "\n" );
    if(logfile)
    {
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void Log::outError( const char * err, ... )
{
    if( !err ) return;
    va_list ap;
    va_start(ap, err);
    vfprintf( stderr, err, ap );
    fprintf( stderr, "\n" );
    if(logfile)
    {
        vfprintf(logfile, err, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stderr);
}

void Log::outBasic( const char * str, ... )
{
    if( !str ) return;
    va_list ap;

    if( m_logLevel > 0 )
    {
        va_start(ap, str);
        vprintf( str, ap );
        va_end(ap);
        printf( "\n" );
    }

    if(logfile)
    {
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void Log::outDetail( const char * str, ... )
{
    if( !str ) return;
    va_list ap;
    if( m_logLevel > 1 )
    {
        va_start(ap, str);
        vprintf( str, ap );
        va_end(ap);
        printf( "\n" );
    }
    if(logfile)
    {
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }

    fflush(stdout);
}

void Log::outDebug( const char * str, ... )
{
    if( !str ) return;
    va_list ap;
    if( m_logLevel > 2 )
    {
        va_start(ap, str);
        vprintf( str, ap );
        va_end(ap);
        printf( "\n" );
    }
    if(logfile)
    {
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void Log::outMenu( const char * str, ... )
{
    if( !str ) return;
    va_list ap;
    va_start(ap, str);
    vprintf( str, ap );
    va_end(ap);
    if(logfile)
    {
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void debug_log(const char * str, ...)
{
    if( !str ) return;

    char buf[100];
    va_list ap;
    va_start(ap, str);
    vsnprintf(buf,100, str, ap);
    va_end(ap);

    MaNGOS::Singleton<Log>::Instance().outDebug(buf);
}
