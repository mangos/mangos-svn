/**
 **	File ......... StdLog.h
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
#ifndef _STDLOG_H
#define _STDLOG_H

#include <string>

typedef enum
{
    LOG_LEVEL_WARNING = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_INFO
} loglevel_t;

class SocketHandler;
class Socket;

class StdLog
{
    public:
        virtual void error(SocketHandler *,Socket *,
            const std::string& user_text,
            int err,
            const std::string& sys_err,
            loglevel_t = LOG_LEVEL_WARNING) = 0;
};
#endif                                            // _STDLOG_H
