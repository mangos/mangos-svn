/** \file ListenSocket.h
 **	\date  2004-02-13
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
#ifndef _LISTENSOCKET_H
#define _LISTENSOCKET_H

#ifdef _WIN32
#include <stdlib.h>
#else
#include <errno.h>
#endif

#include "SocketHandler.h"
#include "Socket.h"
#include "Utility.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


/** Binds incoming port number to new Socket class X. 
	\ingroup basic */
template <class X>
class ListenSocket : public Socket
{
public:
	/** Constructor.
		\param h SocketHandler reference
		\param use_creator Optional use of creator (default true) */
	ListenSocket(SocketHandler& h,bool use_creator = true) : Socket(h), m_port(0), m_depth(0), m_creator(NULL)
	,m_bHasCreate(false)
	{
		if (use_creator)
		{
			m_creator = new X(h);
			Socket *tmp = m_creator -> Create();
			if (tmp && dynamic_cast<X *>(tmp))
			{
				m_bHasCreate = true;
			}
			if (tmp)
			{
				delete tmp;
			}
		}
	}
	~ListenSocket() {
		if (m_creator)
		{
			delete m_creator;
		}
	}

	/** Close file descriptor. */
	int Close() {
		if (GetSocket() != INVALID_SOCKET)
		{
			closesocket(GetSocket());
		}
		return 0;
	}

	/** Bind and listen to any interface.
		\param port Port (0 is random)
		\param depth Listen queue depth */
	int Bind(port_t port,int depth = 20) {
#ifdef IPPROTO_IPV6
		if (IsIpv6())
		{
			in6_addr a;
			memset(&a, 0, sizeof(in6_addr));
			return Bind(a, port, depth);
		}
		else
#endif
		{
			ipaddr_t a = 0;
			return Bind(a, port, depth);
		}
	}

	/** Bind and listen to specific interface.
		\param intf Interface hostname
		\param port Port (0 is random)
		\param depth Listen queue depth */
	int Bind(const std::string& intf,port_t port,int depth = 20) {
#ifdef IPPROTO_IPV6
		if (IsIpv6())
		{
			in6_addr a;
			if (Utility::u2ip(intf, a))
			{
				return Bind(a, port, depth);
			}
			Handler().LogError(this, "Bind", 0, "name resolution of interface name failed", LOG_LEVEL_FATAL);
			return -1;
		}
		else
#endif
		{
			ipaddr_t a;
			if (Utility::u2ip(intf, a))
			{
				return Bind(a, port, depth);
			}
			Handler().LogError(this, "Bind", 0, "name resolution of interface name failed", LOG_LEVEL_FATAL);
			return -1;
		}
	}

	/** Bind and listen to ipv4 interface.
		\param a Ipv4 interface address
		\param port Port (0 is random)
		\param depth Listen queue depth */
	int Bind(ipaddr_t a,port_t port,int depth = 20) {
		struct sockaddr_in sa;
		SOCKET s;

		if ( (s = CreateSocket(AF_INET, SOCK_STREAM)) == INVALID_SOCKET)
		{
			return -1;
		}
		memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		sa.sin_port = htons( port );
		memcpy(&sa.sin_addr, &a, 4);
		if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) == -1)
		{
			Handler().LogError(this, "bind", Errno, StrError(Errno), LOG_LEVEL_FATAL);
			closesocket(s);
			return -1;
		}
		if (listen(s, depth) == -1)
		{
			Handler().LogError(this, "listen", Errno, StrError(Errno), LOG_LEVEL_FATAL);
			closesocket(s);
			return -1;
		}
		int sockaddr_length = sizeof(sockaddr);
		getsockname(s, (struct sockaddr *)&sa, (socklen_t*)&sockaddr_length);
		m_port = ntohs(sa.sin_port);
		m_depth = depth;
		Attach(s);
		return 0;
	}

#ifdef IPPROTO_IPV6
	/** Bind and listen to ipv6 interface.
		\param a Ipv6 interface address
		\param port Port (0 is random)
		\param depth Listen queue depth */
	int Bind(in6_addr a,port_t port,int depth = 20) {
		struct sockaddr_in6 sa;
		SOCKET s;

		if ( (s = CreateSocket(AF_INET6, SOCK_STREAM)) == INVALID_SOCKET)
		{
			return -1;
		}
		memset(&sa, 0, sizeof(sa));
		sa.sin6_family = AF_INET6;
		sa.sin6_port = htons( port );
		sa.sin6_flowinfo = 0;
		sa.sin6_scope_id = 0;
		sa.sin6_addr = a;
		if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) == -1)
		{
			Handler().LogError(this, "bind", Errno, StrError(Errno), LOG_LEVEL_FATAL);
			closesocket(s);
			return -1;
		}
		if (listen(s, depth) == -1)
		{
			Handler().LogError(this, "listen", Errno, StrError(Errno), LOG_LEVEL_FATAL);
			closesocket(s);
			return -1;
		}
		int sockaddr_length = sizeof(sockaddr);
		getsockname(s, (struct sockaddr *)&sa, (socklen_t*)&sockaddr_length);
		m_port = ntohs(sa.sin6_port);
		m_depth = depth;
		Attach(s);
		return 0;
	}
#endif

	/** Return assigned port number. */
	port_t GetPort()
	{
		return m_port;
	}

	/** Return listen queue depth. */
	int GetDepth()
	{
		return m_depth;
	}

	/** OnRead on a ListenSocket receives an incoming connection. */
	void OnRead()
	{
		struct sockaddr sa;
		socklen_t sa_len = sizeof(sa);
		SOCKET a_s = accept(GetSocket(), &sa, &sa_len);

		if (a_s == INVALID_SOCKET)
		{
			Handler().LogError(this, "accept", Errno, StrError(Errno), LOG_LEVEL_ERROR);
			return;
		}
		if (!Handler().OkToAccept(this))
		{
			Handler().LogError(this, "accept", -1, "Not OK to accept", LOG_LEVEL_WARNING);
			closesocket(a_s);
			return;
		}
		if (Handler().GetCount() >= FD_SETSIZE)
		{
			Handler().LogError(this, "accept", (int)Handler().GetCount(), "SocketHandler fd_set limit reached", LOG_LEVEL_FATAL);
			closesocket(a_s);
			return;
		}
		Socket *tmp = m_bHasCreate ? m_creator -> Create() : new X(Handler());
		tmp -> SetIpv6( IsIpv6() );
		tmp -> SetParent(this);
		tmp -> Attach(a_s);
		tmp -> SetNonblocking(true);
		tmp -> SetRemoteAddress( &sa, sa_len);
		tmp -> SetConnected(true);
		tmp -> Init();
		Handler().Add(tmp);
		tmp -> SetDeleteByHandler(true);
		if (tmp -> IsSSL()) // SSL Enabled socket
			tmp -> OnSSLAccept();
		else
			tmp -> OnAccept();
	}

	/** This method is not supposed to be used, because accept() is
	    handled automatically in the OnRead() method. */
        virtual SOCKET Accept(SOCKET socket, struct sockaddr *saptr, socklen_t *lenptr)
        {
                return accept(socket, saptr, lenptr);
        }

protected:
	ListenSocket(const ListenSocket& ) {}
private:
	ListenSocket& operator=(const ListenSocket& ) { return *this; }
	port_t m_port;
	int m_depth;
	X *m_creator;
	bool m_bHasCreate;
};



#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _LISTENSOCKET_H
