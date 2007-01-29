/**
 **	File ......... PoolSocket.cpp
 **	Published ....  2004-11-14
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
#include "SocketHandler.h"
#include "PoolSocket.h"

#define DEB(x)

PoolSocket::PoolSocket(SocketHandler& h,Socket *src)
:Socket(h)
{
    CopyConnection( src );
    DEB(printf("PoolSocket()\n");)
        SetIsClient();
}


PoolSocket::~PoolSocket()
{
    DEB(printf("~PoolSocket()\n");)
}


void PoolSocket::OnRead()
{
    Handler().LogError(this, "OnRead", 0, "data on hibernating socket", LOG_LEVEL_FATAL);
    SetCloseAndDelete();
}
