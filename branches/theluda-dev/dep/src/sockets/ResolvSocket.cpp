/** \file ResolvSocket.cpp
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
#include <stdio.h>
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
/*
#if defined(_WIN32) || defined(__CYGWIN__)
#pragma warning(disable:4786)
#include <winsock.h>
#else
#include <netdb.h>
#endif
*/
#include "ResolvSocket.h"
#include "Utility.h"
#include "Parse.h"
#include "SocketHandler.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif




ResolvSocket::ResolvSocket(SocketHandler& h,Socket *parent)
:TcpSocket(h)
,m_bServer(false)
,m_parent(parent)
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
		if (!Detach()) // detach failed?
		{
			SetCloseAndDelete();
		}
		return;
	}
	std::string key = pa.getword();
	std::string value = pa.getrest();

	if (key == "A" && m_parent)
	{
		ipaddr_t l;
		Utility::u2ip(value, l); // ip2ipaddr_t
		m_parent -> OnResolved(m_resolv_id, l, m_resolv_port);
		m_parent = NULL; // always use first ip in case there are several
	}
}


void ResolvSocket::OnDetached()
{
#ifndef _WIN32
	if (m_query == "getaddrinfo")
	{
		struct addrinfo hints;
		struct addrinfo *res;
		memset(&hints, 0, sizeof(hints));
		hints.ai_flags |= AI_CANONNAME;
		int n = getaddrinfo(m_data.c_str(), NULL, &hints, &res);
		if (!n)
		{
/*
       struct addrinfo {
           int     ai_flags;
           int     ai_family; // PF_INET, PF_INET6
           int     ai_socktype;
           int     ai_protocol;
           size_t  ai_addrlen;
           struct sockaddr *ai_addr;
           char   *ai_canonname;
           struct addrinfo *ai_next;
       };


*/
			while (res)
			{
				Send("Flags: " + Utility::l2string(res -> ai_flags) + "\n");
				Send("Family: " + Utility::l2string(res -> ai_family) + "\n");
				Send("Socktype: " + Utility::l2string(res -> ai_socktype) + "\n");
				Send("Protocol: " + Utility::l2string(res -> ai_protocol) + "\n");
				Send("Addrlen: " + Utility::l2string(res -> ai_addrlen) + "\n");
				std::string tmp;
				Base64 bb;
				bb.encode( (unsigned char *)res -> ai_addr, res -> ai_addrlen, tmp, false);
				Send("Address: " + tmp + "\n");
				// base64-encoded sockaddr
				Send("Canonname: ");
				Send( res -> ai_canonname );
				Send("\n");
				Send("\n");
				//
				res = res -> ai_next;
			}
			freeaddrinfo(res);
		}
		else
		{
			std::string error = "Error: ";
			error += gai_strerror(n);
			Send( error + "\n" );
			Send("\n");
		}
	}
	else
#endif // _WIN32
	if (m_query == "gethostbyname")
	{
		struct hostent *h = gethostbyname(m_data.c_str());
		if (h)
		{
//			sprintf(slask, "Name: %s\n", h -> h_name);
//			Send( slask );
			Send("Name: " + (std::string)h -> h_name + "\n");
			size_t i = 0;
			while (h -> h_aliases[i])
			{
//				sprintf(slask, "Alias: %s\n", h -> h_aliases[i]);
//				Send( slask );
				Send("Alias: " + (std::string)h -> h_aliases[i] + "\n");
				i++;
			}
//			sprintf(slask, "AddrType: %d\n", h -> h_addrtype);
//			Send( slask );
			Send("AddrType: " + Utility::l2string(h -> h_addrtype) + "\n");
//			sprintf(slask, "Length: %d\n", h -> h_length);
//			Send( slask );
			Send("Length: " + Utility::l2string(h -> h_length) + "\n");
			i = 0;
			while (h -> h_addr_list[i])
			{
				// let's assume 4 byte IPv4 addresses
				char ip[40];
				*ip = 0;
				for (int j = 0; j < 4; j++)
				{
					if (*ip)
						strcat(ip,".");
					sprintf(ip + strlen(ip),"%u",(unsigned char)h -> h_addr_list[i][j]);
				}
//				sprintf(slask, "A: %s\n", ip);
//				Send( slask );
				Send("A: " + (std::string)ip + "\n");
				i++;
			}
		}
		else
		{
			Send("Failed\n");
		}
		Send( "\n" );
	}
	else
	if (m_query == "gethostbyaddr")
	{
		Send("Not Implemented\n\n");
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
	std::string msg = "gethostbyname " + m_resolv_host + "\n";
	Send( msg );
}


#ifdef SOCKETS_NAMESPACE
}
#endif

