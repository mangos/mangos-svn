/** \file UdpSocket.cpp
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
#ifdef _WIN32
#pragma warning(disable:4786)
#include <stdlib.h>
#else
#include <errno.h>
#endif
#include <stdio.h>

#include "SocketHandler.h"
#include "UdpSocket.h"
#include "Utility.h"
// include this to see strange sights
//#include <linux/in6.h>


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


UdpSocket::UdpSocket(SocketHandler& h,int ibufsz,bool ipv6) : Socket(h)
,m_ibuf(new char[ibufsz])
,m_ibufsz(ibufsz)
,m_bind_ok(false)
,m_port(0)
{
#ifdef IPPROTO_IPV6
	SetIpv6(ipv6);
#endif
	CreateConnection();
}


UdpSocket::~UdpSocket()
{
	Close();
	delete[] m_ibuf;
}


int UdpSocket::Bind(port_t &port,int range)
{
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		in6_addr a;
		memset(&a, 0, sizeof(a));
		return Bind(a, port, range);
	}
#endif
	ipaddr_t l = 0;
	return Bind(l, port, range);
}


int UdpSocket::Bind(const std::string& intf,port_t &port,int range)
{
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		in6_addr a;
		if (Utility::u2ip(intf, a))
		{
			return Bind(a, port, range);
		}
		else
		{
			SetCloseAndDelete();
			return -1;
		}
	}
#endif
	ipaddr_t l = 0;
	if (Utility::u2ip(intf, l))
	{
		return Bind(l, port, range);
	}
	else
	{
		SetCloseAndDelete();
		return -1;
	}
}


int UdpSocket::Bind(ipaddr_t a,port_t &port,int range)
{
	SOCKET s = GetSocket();
	struct sockaddr_in sa;
	socklen_t sa_len = sizeof(sa);

	memset(&sa,0,sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons( port );
	memmove(&sa.sin_addr,&a,4);

	int n = bind(s, (struct sockaddr *)&sa, sa_len);
	int tries = range;
	while (n == -1 && tries--)
	{
		sa.sin_port = htons( ++port );
		n = bind(s, (struct sockaddr *)&sa, sa_len);
	}
	if (n == -1)
	{
		Handler().LogError(this, "bind", Errno, StrError(Errno), LOG_LEVEL_FATAL);
		SetCloseAndDelete();
		return -1;
	}
	m_bind_ok = true;
	m_port = port;
	return 0;
}


#ifdef IPPROTO_IPV6
int UdpSocket::Bind(in6_addr a,port_t &port,int range)
{
	SOCKET s = GetSocket();
	struct sockaddr_in6 sa;
	socklen_t sa_len = sizeof(sa);

	memset(&sa,0,sizeof(sa));
	sa.sin6_family = AF_INET6;
	sa.sin6_port = htons( port );
	sa.sin6_flowinfo = 0;
	sa.sin6_scope_id = 0;
	sa.sin6_addr = a;

	int n = bind(s, (struct sockaddr *)&sa, sa_len);
	int tries = range;
	while (n == -1 && tries--)
	{
		sa.sin6_port = htons( ++port );
		n = bind(s, (struct sockaddr *)&sa, sa_len);
	}
	if (n == -1)
	{
		Handler().LogError(this, "bind", Errno, StrError(Errno), LOG_LEVEL_FATAL);
		SetCloseAndDelete();
		return -1;
	}
	m_bind_ok = true;
	m_port = port;
	return 0;
}
#endif


/** if you wish to use Send, first Open a connection */
bool UdpSocket::Open(ipaddr_t l,port_t port)
{
	struct sockaddr_in sa;
	socklen_t sa_len = sizeof(sa);

	memset(&sa,0,sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons( port );
	memmove(&sa.sin_addr,&l,4);

	if (connect(GetSocket(), (struct sockaddr *)&sa, sa_len) == -1)
	{
		Handler().LogError(this, "connect", Errno, StrError(Errno), LOG_LEVEL_FATAL);
		SetCloseAndDelete();
		return false;
	}
	SetConnected();
	return true;
}


bool UdpSocket::Open(const std::string& host,port_t port)
{
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		struct in6_addr a;
		if (Utility::u2ip(host, a))
		{
			return Open(a, port);
		}
		return false;
	}
#endif
	ipaddr_t a;
	if (Utility::u2ip(host, a))
	{
		return Open(a, port);
	}
	return false;
}


#ifdef IPPROTO_IPV6
bool UdpSocket::Open(struct in6_addr& a,port_t port)
{
	struct sockaddr_in6 sa;
	socklen_t sa_len = sizeof(sa);

	memset(&sa,0,sizeof(sa));
	sa.sin6_family = AF_INET6;
	sa.sin6_port = htons( port );
	sa.sin6_flowinfo = 0;
	sa.sin6_scope_id = 0;
	sa.sin6_addr = a;

	if (connect(GetSocket(), (struct sockaddr *)&sa, sa_len) == -1)
	{
		Handler().LogError(this, "connect", Errno, StrError(Errno), LOG_LEVEL_FATAL);
		SetCloseAndDelete();
		return false;
	}
	SetConnected();
	return true;
}
#endif


void UdpSocket::CreateConnection()
{
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		if (GetSocket() == INVALID_SOCKET)
		{
			SOCKET s = CreateSocket(AF_INET6, SOCK_DGRAM, "udp");
			if (s == INVALID_SOCKET)
			{
				return;
			}
			SetNonblocking(true, s);
			Attach(s);
		}
		return;
	}
#endif
	if (GetSocket() == INVALID_SOCKET)
	{
		SOCKET s = CreateSocket(AF_INET, SOCK_DGRAM, "udp");
		if (s == INVALID_SOCKET)
		{
			return;
		}
		SetNonblocking(true, s);
		Attach(s);
	}
}


/** send to specified address */
void UdpSocket::SendToBuf(const std::string& h,port_t p,const char *data,int len,int flags)
{
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		struct in6_addr a;
		if (Utility::u2ip(h,a))
		{
			SendToBuf(a, p, data, len, flags);
		}
		return;
	}
#endif
	ipaddr_t a;
	if (Utility::u2ip(h,a))
	{
		SendToBuf(a, p, data, len, flags);
	}
}


/** send to specified address */
void UdpSocket::SendToBuf(ipaddr_t a,port_t p,const char *data,int len,int flags)
{
	struct sockaddr_in sa;
	socklen_t sa_len = sizeof(sa);

	memset(&sa,0,sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons( p );
	memmove(&sa.sin_addr,&a,4);

	if (sendto(GetSocket(),data,len,flags,(struct sockaddr *)&sa,sa_len) == -1)
	{
		Handler().LogError(this,"sendto",Errno,StrError(Errno),LOG_LEVEL_ERROR);
	}
}


#ifdef IPPROTO_IPV6
void UdpSocket::SendToBuf(in6_addr a,port_t p,const char *data,int len,int flags)
{
	struct sockaddr_in6 sa;
	socklen_t sa_len = sizeof(sa);

	memset(&sa,0,sizeof(sa));
	sa.sin6_family = AF_INET6;
	sa.sin6_port = htons( p );
	sa.sin6_flowinfo = 0;
	sa.sin6_scope_id = 0;
	sa.sin6_addr = a;

	if (sendto(GetSocket(),data,len,flags,(struct sockaddr *)&sa,sa_len) == -1)
	{
		Handler().LogError(this,"sendto",Errno,StrError(Errno),LOG_LEVEL_ERROR);
	}
}
#endif


void UdpSocket::SendTo(const std::string& a,port_t p,const std::string& str,int flags)
{
	SendToBuf(a,p,str.c_str(),(int)str.size(),flags);
}


void UdpSocket::SendTo(ipaddr_t a,port_t p,const std::string& str,int flags)
{
	SendToBuf(a,p,str.c_str(),(int)str.size(),flags);
}


#ifdef IPPROTO_IPV6
void UdpSocket::SendTo(in6_addr a,port_t p,const std::string& str,int flags)
{
	SendToBuf(a,p,str.c_str(),(int)str.size(),flags);
}
#endif


/** send to connected address */
void UdpSocket::SendBuf(const char *data,size_t len,int flags)
{
	if (!IsConnected())
	{
		Handler().LogError(this,"SendBuf",0,"not connected",LOG_LEVEL_ERROR);
		return;
	}
	if (send(GetSocket(),data,(int)len,flags) == -1)
	{
		Handler().LogError(this,"send",Errno,StrError(Errno),LOG_LEVEL_ERROR);
	}
}


void UdpSocket::Send(const std::string& str,int flags)
{
	SendBuf(str.c_str(),(int)str.size(),flags);
}


void UdpSocket::OnRead()
{
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		struct sockaddr_in6 sa;
		socklen_t sa_len = sizeof(sa);
		int n = recvfrom(GetSocket(), m_ibuf, m_ibufsz, 0, (struct sockaddr *)&sa, &sa_len);
		int q = 10; // receive max 10 at one cycle
		while (n > 0)
		{
			if (sa_len != sizeof(sa))
			{
				Handler().LogError(this, "recvfrom", 0, "unexpected address struct size", LOG_LEVEL_WARNING);
			}
			this -> OnRawData(m_ibuf, n, (struct sockaddr *)&sa, sa_len);
			if (!q--)
				break;
			//
			n = recvfrom(GetSocket(), m_ibuf, m_ibufsz, 0, (struct sockaddr *)&sa, &sa_len);
		}
		if (n == -1)
		{
#ifdef _WIN32
			if (Errno != WSAEWOULDBLOCK)
#else
			if (Errno != EWOULDBLOCK)
#endif
				Handler().LogError(this, "recvfrom", Errno, StrError(Errno), LOG_LEVEL_ERROR);
		}
		return;
	}
#endif
	struct sockaddr_in sa;
	socklen_t sa_len = sizeof(sa);
	int n = recvfrom(GetSocket(), m_ibuf, m_ibufsz, 0, (struct sockaddr *)&sa, &sa_len);
	int q = 10;
	while (n > 0)
	{
		if (sa_len != sizeof(sa))
		{
			Handler().LogError(this, "recvfrom", 0, "unexpected address struct size", LOG_LEVEL_WARNING);
		}
		this -> OnRawData(m_ibuf, n, (struct sockaddr *)&sa, sa_len);
		if (!q--)
			break;
		//
		n = recvfrom(GetSocket(), m_ibuf, m_ibufsz, 0, (struct sockaddr *)&sa, &sa_len);
	}
	if (n == -1)
	{
#ifdef _WIN32
		if (Errno != WSAEWOULDBLOCK)
#else
		if (Errno != EWOULDBLOCK)
#endif
			Handler().LogError(this, "recvfrom", Errno, StrError(Errno), LOG_LEVEL_ERROR);
	}
}


void UdpSocket::SetBroadcast(bool b)
{
	int one = 1;
	int zero = 0;

	if (b)
	{
		if (setsockopt(GetSocket(), SOL_SOCKET, SO_BROADCAST, (char *) &one, sizeof(one)) == -1)
		{
			Handler().LogError(this, "SetBroadcast", Errno, StrError(Errno), LOG_LEVEL_WARNING);
		}
	}
	else
	{
		if (setsockopt(GetSocket(), SOL_SOCKET, SO_BROADCAST, (char *) &zero, sizeof(zero)) == -1)
		{
			Handler().LogError(this, "SetBroadcast", Errno, StrError(Errno), LOG_LEVEL_WARNING);
		}
	}
}


bool UdpSocket::IsBroadcast()
{
	int is_broadcast = 0;
	socklen_t size;
	if (getsockopt(GetSocket(), SOL_SOCKET, SO_BROADCAST, (char *)&is_broadcast, &size) == -1)
	{
		Handler().LogError(this, "IsBroadcast", Errno, StrError(Errno), LOG_LEVEL_WARNING);
	}
	return is_broadcast != 0;
}


void UdpSocket::SetMulticastTTL(int ttl)
{
	if (setsockopt(GetSocket(), SOL_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(int)) == -1)
	{
		Handler().LogError(this, "SetMulticastTTL", Errno, StrError(Errno), LOG_LEVEL_WARNING);
	}
}


int UdpSocket::GetMulticastTTL()
{
	int ttl = 0;
	socklen_t size = sizeof(int);
	if (getsockopt(GetSocket(), SOL_IP, IP_MULTICAST_TTL, (char *)&ttl, &size) == -1)
	{
		Handler().LogError(this, "GetMulticastTTL", Errno, StrError(Errno), LOG_LEVEL_WARNING);
	}
	return ttl;
}


void UdpSocket::SetMulticastLoop(bool x)
{
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		int val = x ? 1 : 0;
		if (setsockopt(GetSocket(), IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (char *)&val, sizeof(int)) == -1)
		{
			Handler().LogError(this, "SetMulticastLoop", Errno, StrError(Errno), LOG_LEVEL_WARNING);
		}
		return;
	}
#endif
	int val = x ? 1 : 0;
	if (setsockopt(GetSocket(), SOL_IP, IP_MULTICAST_LOOP, (char *)&val, sizeof(int)) == -1)
	{
		Handler().LogError(this, "SetMulticastLoop", Errno, StrError(Errno), LOG_LEVEL_WARNING);
	}
}


bool UdpSocket::IsMulticastLoop()
{
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		int is_loop = 0;
		socklen_t size = sizeof(int);
		if (getsockopt(GetSocket(), IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (char *)&is_loop, &size) == -1)
		{
			Handler().LogError(this, "IsMulticastLoop", Errno, StrError(Errno), LOG_LEVEL_WARNING);
		}
		return is_loop ? true : false;
	}
#endif
	int is_loop = 0;
	socklen_t size = sizeof(int);
	if (getsockopt(GetSocket(), SOL_IP, IP_MULTICAST_LOOP, (char *)&is_loop, &size) == -1)
	{
		Handler().LogError(this, "IsMulticastLoop", Errno, StrError(Errno), LOG_LEVEL_WARNING);
	}
	return is_loop ? true : false;
}


void UdpSocket::AddMulticastMembership(const std::string& group,const std::string& local_if,int if_index)
{
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		struct ipv6_mreq x;
		struct in6_addr addr;
		if (Utility::u2ip( group, addr ))
		{
			x.ipv6mr_multiaddr = addr;
			x.ipv6mr_interface = if_index;
			if (setsockopt(GetSocket(), IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)&x, sizeof(struct ipv6_mreq)) == -1)
			{
				Handler().LogError(this, "AddMulticastMembership", Errno, StrError(Errno), LOG_LEVEL_WARNING);
			}
		}
		return;
	}
#endif
	struct ip_mreq x; // ip_mreqn
	ipaddr_t addr;
	if (Utility::u2ip( group, addr ))
	{
		memcpy(&x.imr_multiaddr.s_addr, &addr, sizeof(addr));
		Utility::u2ip( local_if, addr);
		memcpy(&x.imr_interface.s_addr, &addr, sizeof(addr));
//		x.imr_ifindex = if_index;
		if (setsockopt(GetSocket(), SOL_IP, IP_ADD_MEMBERSHIP, (char *)&x, sizeof(struct ip_mreq)) == -1)
		{
			Handler().LogError(this, "AddMulticastMembership", Errno, StrError(Errno), LOG_LEVEL_WARNING);
		}
	}
}


void UdpSocket::DropMulticastMembership(const std::string& group,const std::string& local_if,int if_index)
{
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		struct ipv6_mreq x;
		struct in6_addr addr;
		if (Utility::u2ip( group, addr ))
		{
			x.ipv6mr_multiaddr = addr;
			x.ipv6mr_interface = if_index;
			if (setsockopt(GetSocket(), IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, (char *)&x, sizeof(struct ipv6_mreq)) == -1)
			{
				Handler().LogError(this, "DropMulticastMembership", Errno, StrError(Errno), LOG_LEVEL_WARNING);
			}
		}
		return;
	}
#endif
	struct ip_mreq x; // ip_mreqn
	ipaddr_t addr;
	if (Utility::u2ip( group, addr ))
	{
		memcpy(&x.imr_multiaddr.s_addr, &addr, sizeof(addr));
		Utility::u2ip( local_if, addr);
		memcpy(&x.imr_interface.s_addr, &addr, sizeof(addr));
//		x.imr_ifindex = if_index;
		if (setsockopt(GetSocket(), SOL_IP, IP_DROP_MEMBERSHIP, (char *)&x, sizeof(struct ip_mreq)) == -1)
		{
			Handler().LogError(this, "DropMulticastMembership", Errno, StrError(Errno), LOG_LEVEL_WARNING);
		}
	}
}


#ifdef IPPROTO_IPV6
void UdpSocket::SetMulticastHops(int hops)
{
	if (!IsIpv6())
	{
		Handler().LogError(this, "SetMulticastHops", 0, "Ipv6 only", LOG_LEVEL_ERROR);
		return;
	}
	if (setsockopt(GetSocket(), IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (char *)&hops, sizeof(int)) == -1)
	{
		Handler().LogError(this, "SetMulticastHops", Errno, StrError(Errno), LOG_LEVEL_WARNING);
	}
}


int UdpSocket::GetMulticastHops()
{
	if (!IsIpv6())
	{
		Handler().LogError(this, "SetMulticastHops", 0, "Ipv6 only", LOG_LEVEL_ERROR);
		return -1;
	}
	int hops = 0;
	socklen_t size = sizeof(int);
	if (getsockopt(GetSocket(), IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (char *)&hops, &size) == -1)
	{
		Handler().LogError(this, "GetMulticastHops", Errno, StrError(Errno), LOG_LEVEL_WARNING);
	}
	return hops;
}
#endif // IPPROTO_IPV6


bool UdpSocket::IsBound()
{
	return m_bind_ok;
}


void UdpSocket::OnRawData(const char *buf,size_t len,struct sockaddr *sa,socklen_t sa_len)
{
}


port_t UdpSocket::GetPort()
{
	return m_port;
}


#ifdef SOCKETS_NAMESPACE
}
#endif

