/** \file ISocketHandler.h
 **	\date  2004-02-13
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2004-2007  Anders Hedstrom

This library is made available under the terms of the GNU GPL.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

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
#include "ISocketHandler.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x) 
#endif


ISocketHandler::ISocketHandler(StdLog *log)
: m_stdlog(log)
#ifdef ENABLE_DETACH
, m_slave(false)
#endif
, m_mutex(m_mutex)
, m_b_use_mutex(false)
{
}


ISocketHandler::ISocketHandler(Mutex& mutex,StdLog *log)
: m_stdlog(log)
#ifdef ENABLE_DETACH
, m_slave(false)
#endif
, m_mutex(mutex)
, m_b_use_mutex(true)
{
}


ISocketHandler::~ISocketHandler()
{
}


Mutex& ISocketHandler::GetMutex() const
{
	return m_mutex; 
}


#ifdef ENABLE_DETACH
void ISocketHandler::SetSlave(bool x)
{
	m_slave = x;
}


bool ISocketHandler::IsSlave()
{
	return m_slave;
}
#endif


void ISocketHandler::RegStdLog(StdLog *log)
{
	m_stdlog = log;
}


void ISocketHandler::LogError(Socket *p,const std::string& user_text,int err,const std::string& sys_err,loglevel_t t)
{
	if (m_stdlog)
	{
		m_stdlog -> error(this, p, user_text, err, sys_err, t);
	}
}


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif
