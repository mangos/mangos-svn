/** \file ResolvSocket.h
 **	\date  2005-03-24
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
#ifndef _SOCKETS_ResolvSocket_H
#define _SOCKETS_ResolvSocket_H
#include "sockets-config.h"
#ifdef ENABLE_RESOLVER
#include "TcpSocket.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

/** Async DNS resolver socket. 
	\ingroup async */
class ResolvSocket : public TcpSocket
{
public:
	ResolvSocket(ISocketHandler&,Socket *parent = NULL);
	~ResolvSocket();

	void OnAccept() { m_bServer = true; }
	void OnLine(const std::string& line);
	void OnDetached();
	void OnDelete();

	void SetId(int x) { m_resolv_id = x; }
	void SetHost(const std::string& x) { m_resolv_host = x; }
	void SetAddress(ipaddr_t x) { m_resolv_address = x; }
#ifdef ENABLE_IPV6
	void SetAddress(in6_addr& a) { m_resolv_address6 = a; m_resolve_ipv6 = true; }
#endif
	void SetPort(port_t x) { m_resolv_port = x; }
	void OnConnect();

#ifdef ENABLE_IPV6
	void SetResolveIpv6(bool x = true) { m_resolve_ipv6 = x; }
#endif

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
	ipaddr_t m_resolv_address;
#ifdef ENABLE_IPV6
	bool m_resolve_ipv6;
	in6_addr m_resolv_address6;
#endif
};




#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // ENABLE_RESOLVER
#endif // _SOCKETS_ResolvSocket_H
