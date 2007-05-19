/** \file ResolvSocket.cpp
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
#ifdef _WIN32
#pragma warning(disable:4786)
#else
#include <netdb.h>
#endif
#include "ResolvSocket.h"
#ifdef ENABLE_RESOLVER
#include "Utility.h"
#include "Parse.h"
#include "ISocketHandler.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x)
#endif


ResolvSocket::ResolvSocket(ISocketHandler& h,Socket *parent)
:TcpSocket(h)
,m_bServer(false)
,m_parent(parent)
#ifdef ENABLE_IPV6
,m_resolve_ipv6(false)
#endif
{
	SetLineProtocol();
}


ResolvSocket::~ResolvSocket()
{
}


void ResolvSocket::OnLine(const std::string& line)
{
	Parse pa(line, ":");
	if (m_bServer)
	{
		m_query = pa.getword();
		m_data = pa.getrest();
DEB(		fprintf(stderr, "ResolvSocket server; query=%s, data=%s\n", m_query.c_str(), m_data.c_str());)
		if (!Detach()) // detach failed?
		{
			SetCloseAndDelete();
		}
		return;
	}
	std::string key = pa.getword();
	std::string value = pa.getrest();

	if (key == "Failed" && m_parent)
	{
DEB(		fprintf(stderr, "************ Resolve failed\n");)
		if (Handler().Valid(m_parent))
			m_parent -> OnResolveFailed(m_resolv_id);
		m_parent = NULL;
	}
	else
	if (key == "Name" && !m_resolv_host.size() && m_parent)
	{
		if (Handler().Valid(m_parent))
			m_parent -> OnReverseResolved(m_resolv_id, value);
		m_parent = NULL;
	}
	else
	if (key == "A" && m_parent)
	{
		ipaddr_t l;
		Utility::u2ip(value, l); // ip2ipaddr_t
		if (Handler().Valid(m_parent))
			m_parent -> OnResolved(m_resolv_id, l, m_resolv_port);
		m_parent = NULL; // always use first ip in case there are several
	}
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
	else
	if (key == "AAAA" && m_parent)
	{
		in6_addr a;
		Utility::u2ip(value, a);
		if (Handler().Valid(m_parent))
			m_parent -> OnResolved(m_resolv_id, a, m_resolv_port);
		m_parent = NULL;
	}
#endif
#endif
}


void ResolvSocket::OnDetached()
{
DEB(	fprintf(stderr, "ResolvSocket::OnDetached(); query=%s, data=%s\n", m_query.c_str(), m_data.c_str());)
	if (m_query == "gethostbyname")
	{
		struct sockaddr_in sa;
		if (Utility::u2ip(m_data, sa))
		{
			std::string ip;
			Utility::l2ip(sa.sin_addr, ip);
			Send("A: " + ip + "\n");
		}
		else
		{
			Send("Failed\n");
		}
		Send("\n");
	}
	else
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
	if (m_query == "gethostbyname2")
	{
		struct sockaddr_in6 sa;
		if (Utility::u2ip(m_data, sa))
		{
			std::string ip;
			Utility::l2ip(sa.sin6_addr, ip);
			Send("AAAA: " + ip + "\n");
		}
		else
		{
			Send("Failed\n");
		}
		Send("\n");
	}
	else
#endif
#endif
	if (m_query == "gethostbyaddr")
	{
		if (Utility::isipv4( m_data ))
		{
			struct sockaddr_in sa;
			if (!Utility::u2ip(m_data, sa, AI_NUMERICHOST))
			{
				Send("Failed: convert to sockaddr_in failed\n");
			}
			else
			{
				std::string name;
				if (!Utility::reverse( (struct sockaddr *)&sa, sizeof(sa), name))
				{
					Send("Failed: ipv4 reverse lookup of " + m_data + "\n");
				}
				else
				{
					Send("Name: " + name + "\n");
				}
			}
		}
		else
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
		if (Utility::isipv6( m_data ))
		{
			struct sockaddr_in6 sa;
			if (!Utility::u2ip(m_data, sa, AI_NUMERICHOST))
			{
				Send("Failed: convert to sockaddr_in6 failed\n");
			}
			else
			{
				std::string name;
				if (!Utility::reverse( (struct sockaddr *)&sa, sizeof(sa), name))
				{
					Send("Failed: ipv6 reverse lookup of " + m_data + "\n");
				}
				else
				{
					Send("Name: " + name + "\n");
				}
			}
		}
		else
#endif
#endif
		{
			Send("Failed: malformed address\n");
		}
		Send("\n");
	}
	else
	{
		std::string msg = "Unknown query type: " + m_query;
		Handler().LogError(this, "OnDetached", 0, msg);
		Send("Unknown\n\n");
	}
	SetCloseAndDelete();
}


void ResolvSocket::OnConnect()
{
	if (m_resolv_host.size())
	{
#ifdef ENABLE_IPV6
		std::string msg = (m_resolve_ipv6 ? "gethostbyname2 " : "gethostbyname ") + m_resolv_host + "\n";
#else
		std::string msg = "gethostbyname " + m_resolv_host + "\n";
#endif
		Send( msg );
		return;
	}
#ifdef ENABLE_IPV6
	if (m_resolve_ipv6)
	{
		std::string tmp;
		Utility::l2ip(m_resolv_address6, tmp);
		std::string msg = "gethostbyaddr " + tmp + "\n";
		Send( msg );
	}
#endif
	std::string tmp;
	Utility::l2ip(m_resolv_address, tmp);
	std::string msg = "gethostbyaddr " + tmp + "\n";
	Send( msg );
}


void ResolvSocket::OnDelete()
{
	if (m_parent)
	{
		if (Handler().Valid(m_parent))
			m_parent -> OnResolveFailed(m_resolv_id);
		m_parent = NULL;
	}
}


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // ENABLE_RESOLVER
