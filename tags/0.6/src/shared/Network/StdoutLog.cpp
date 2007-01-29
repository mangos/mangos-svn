/**
 **	File ......... StdoutLog.cpp
 **	Published ....  2004-06-01
 **	Author ....... grymse@alhem.net
 **/
/*
Copyright (C) 2004,2005  Anders Hedstrom

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include <stdio.h>
#include <time.h>
#include "SocketHandler.h"
#include "Socket.h"
#include "StdoutLog.h"

void StdoutLog::error(SocketHandler *,Socket *,const std::string& call,int err,const std::string& sys_err,loglevel_t lvl)
{
    time_t t = time(NULL);
    struct tm *tp = localtime(&t);
    std::string level;

    switch (lvl)
    {
        case LOG_LEVEL_WARNING:
            level = "Warning";
            break;
        case LOG_LEVEL_ERROR:
            level = "Error";
            break;
        case LOG_LEVEL_FATAL:
            level = "Fatal";
            break;
        case LOG_LEVEL_INFO:
            level = "Info";
            break;
    }

    printf("%d-%02d-%02d %02d:%02d:%02d :: %s: %d %s (%s)\n",
        tp -> tm_year + 1900,
        tp -> tm_mon + 1,
        tp -> tm_mday,
        tp -> tm_hour,tp -> tm_min,tp -> tm_sec,
        call.c_str(),err,sys_err.c_str(),level.c_str());
}
