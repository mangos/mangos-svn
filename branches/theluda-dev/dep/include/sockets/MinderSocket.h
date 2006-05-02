/*
 **	File ......... MinderSocket.h
 **	Published ....  2004-04-17
 **	Author ....... grymse@alhem.net
**/
/*
Copyright (C) 2004  Anders Hedstrom

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
#ifndef _MINDERSOCKET_H
#define _MINDERSOCKET_H
#ifdef HAVE_OPENSSL

#include "TcpSocket.h"
#include "MinionSocket.h"
//#include "Utility.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


/** Client socket for connecting to a Minder. 
	\ingroup minder */
class MinderSocket : public TcpSocket
{
public:
	MinderSocket(SocketHandler& ,const std::string& );
	~MinderSocket();

	void SetLocalIpPort(const std::string& x,port_t p) { local_ip = x; local_port = p; }

	void Function(const std::string& str) { m_function = str; }
	void SendHello();
	void SetExtraInfo(long x) { m_extra_info = x; }

	virtual MinionSocket *CreateMinionSocket(const std::string& ,ipaddr_t,port_t) = 0;

	virtual int GetMaxConnections() = 0;
	virtual void OnSwitchDatabase(const std::string& ) = 0;

static	char g_UpdateHost[256];
static	port_t g_UpdatePort;
static	char g_UpdateUrl[256];

protected:
	void OnLine(const std::string& line);
	void OnConnect();

private:
	MinderSocket& operator=(const MinderSocket& ) { return *this; }
	std::string m_app;
	std::string local_ip;
	port_t local_port;
	ipaddr_t my_ip;
	port_t my_port;
	std::string m_function;
	long m_extra_info;
};


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // HAVE_OPENSSL
#endif // _MINDERSOCKET_H
