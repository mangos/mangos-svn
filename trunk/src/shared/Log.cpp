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

#include <stdarg.h>

INSTANTIATE_SINGLETON_1( Log );

void Log::Initialize()
{
	//std::string logfn=sConfig.GetStringDefault("LogFile", "Server.log");
	logfile = fopen("Server.log", "w");//logfn.c_str()
}

void Log::outString( const char * str, ... )
{
    if( !str ) return;
    va_list ap;
    va_start(ap, str);
    vprintf( str, ap );
    printf( "\n" );
	vfprintf(logfile, str, ap);
	fprintf(logfile, "\n" );
    va_end(ap);
    fflush(stdout);
	fflush(logfile);
}


void Log::outError( const char * err, ... )
{
    if( !err ) return;
    va_list ap;
    va_start(ap, err);
    vfprintf( stderr, err, ap );
    fprintf( stderr, "\n" );
	vfprintf(logfile, err, ap);
	fprintf(logfile, "\n" );
    va_end(ap);
    fflush(stderr);
	fflush(logfile);
}


void Log::outBasic( const char * str, ... )
{
    if( !str ) return;
    va_list ap;
    va_start(ap, str);
	vfprintf(logfile, str, ap);
	fprintf(logfile, "\n" );
    if( loglevel > 0 )
    {
        vprintf( str, ap );
        printf( "\n" );
    }
    va_end(ap);
	fflush(stdout);
	fflush(logfile);
}


void Log::outDetail( const char * str, ... )
{
    if( !str ) return;
	va_list ap;
	va_start(ap, str);
	vfprintf(logfile, str, ap);
	fprintf(logfile, "\n" );
    if( loglevel > 1 )
    {
        vprintf( str, ap );
        printf( "\n" );
	}
	va_end(ap);
	fflush(stdout);
	fflush(logfile);
}


void Log::outDebug( const char * str, ... )
{
    if( !str ) return;
	va_list ap;
	va_start(ap, str);
	vfprintf(logfile, str, ap);
	fprintf(logfile, "\n" );
    if( loglevel > 2 )
    {
        vprintf( str, ap );
        printf( "\n" );
	}
	va_end(ap);
	fflush(stdout);
	fflush(logfile);
}


void Log::outMenu( const char * str, ... )
{
    if( !str ) return;
    va_list ap;
    va_start(ap, str);
    vprintf( str, ap );
	vfprintf(logfile, str, ap);
	fprintf(logfile, "\n" );
    va_end(ap);
    fflush(stdout);
	fflush(logfile);
}
