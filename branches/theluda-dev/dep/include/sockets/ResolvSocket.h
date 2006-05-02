/** \file ResolvSocket.h
 **	\date  2005-03-24
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2004,2005  Anders Hedstrom

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
#ifndef _RESOLVSOCKET_H
#define _RESOLVSOCKET_H

#include "TcpSocket.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

/** Async DNS resolver socket. 
	\ingroup async */
class ResolvSocket : public TcpSocket
{
public:
	ResolvSocket(SocketHandler&,Socket *parent = NULL);
	~ResolvSocket();

	void OnAccept() { m_bServer = true; }
	void OnLine(const std::string& line);
	void OnDetached();

	void SetId(int x) { m_resolv_id = x; }
	void SetHost(const std::string& x) { m_resolv_host = x; }
	void SetPort(port_t x) { m_resolv_port = x; }
	void OnConnect();

private:
	ResolvSocket(const ResolvSocket& s) : TcpSocket(s) {} // copy constructor
	ResolvSocket& operator=(const ResolvSocket& ) { return *this; } // assignment operator

	std::string m_query;
	std::string m_data;
	bool m_bServer;
	Socket *m_parent;
	int m_resolv_id;
	std::string m_resolv_host;
	port_t m_resolv_port;
};




#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _RESOLVSOCKET_H
