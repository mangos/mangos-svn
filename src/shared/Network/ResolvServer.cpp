/**
 **	File ......... ResolvServer.cpp
 **	Published ....  2005-03-24
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
//#include <stdio.h>
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include "StdoutLog.h"
#include "SocketHandler.h"
#include "ListenSocket.h"
#include "ResolvSocket.h"
#include "ResolvServer.h"

ResolvServer::ResolvServer(port_t port)
:Thread()
,m_quit(false)
,m_port(port)
{
}


ResolvServer::~ResolvServer()
{
}


void ResolvServer::Run()
{
//	StdoutLog log;
    SocketHandler h;
    ListenSocket<ResolvSocket> l(h);

    if (l.Bind("127.0.0.1", m_port))
    {
        return;
    }
    h.Add(&l);

    while (!m_quit && IsRunning() )
    {
        h.Select(1,0);
    }
    SetRunning(false);
}


void ResolvServer::Quit()
{
    m_quit = true;
}
