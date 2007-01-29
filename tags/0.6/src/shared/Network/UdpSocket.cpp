/**
 **	File ......... UdpSocket.cpp
 **	Published ....  2004-02-13
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
#include <stdlib.h>
#else
#include <errno.h>
#endif
#include <stdio.h>
#include <map>

#include "StdLog.h"
#include "SocketHandler.h"
#include "UdpSocket.h"
// include this to see strange sights
//#include <linux/in6.h>

#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x)
#endif

UdpSocket::UdpSocket(SocketHandler& h,int ibufsz) : Socket(h)
,m_connected(false)
,m_ibuf(new char[ibufsz])
,m_ibufsz(ibufsz)
{
}


UdpSocket::~UdpSocket()
{
    delete[] m_ibuf;
}


SOCKET UdpSocket::Bind(port_t &port,int range)
{
    SOCKET s = GetSocket();
    if (s == INVALID_SOCKET)
    {
        s = CreateSocket4(SOCK_DGRAM, "udp");
        if (s == INVALID_SOCKET)
        {
            return s;
        }
        Attach(s);
    }
    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    ipaddr_t l = 0;

    memset(&sa,0,sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons( port );
    memmove(&sa.sin_addr,&l,4);

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
        Close();
        return INVALID_SOCKET;
    }
    return s;
}


#ifdef IPPROTO_IPV6
SOCKET UdpSocket::Bind6(port_t &port,int range)
{
    SOCKET s = GetSocket();
    if (s == INVALID_SOCKET)
    {
        s = CreateSocket6(SOCK_DGRAM, "udp");
        if (s == INVALID_SOCKET)
        {
            return s;
        }
        Attach(s);
    }
    struct sockaddr_in6 sa;
    socklen_t sa_len = sizeof(sa);

    memset(&sa,0,sizeof(sa));
    sa.sin6_family = AF_INET6;
    sa.sin6_port = htons( port );
    sa.sin6_flowinfo = 0;
    sa.sin6_scope_id = 0;
// sa.sin6_addr is all 0's

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
        Close();
        return INVALID_SOCKET;
    }
    return s;
}
#endif

/** if you wish to use Send, first Open a connection */
bool UdpSocket::Open(ipaddr_t l,port_t port)
{
    if (GetSocket() == INVALID_SOCKET)
    {
        SOCKET s = CreateSocket4(SOCK_DGRAM, "udp");
        if (s == INVALID_SOCKET)
        {
            return false;
        }
        Attach(s);
    }
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
    m_connected = true;
    return true;
}


bool UdpSocket::Open(const std::string& host,port_t port)
{
    if (GetSocket() == INVALID_SOCKET)
    {
        SOCKET s = CreateSocket4(SOCK_DGRAM, "udp");
        if (s == INVALID_SOCKET)
        {
            return false;
        }
        Attach(s);
    }
    ipaddr_t a;
    if (u2ip(host, a))
    {
        return Open(a, port);
    }
    return false;
}


#ifdef IPPROTO_IPV6
bool UdpSocket::Open6(struct in6_addr& a,port_t port)
{
    if (GetSocket() == INVALID_SOCKET)
    {
        SOCKET s = CreateSocket6(SOCK_DGRAM, "udp");
        if (s == INVALID_SOCKET)
        {
            return false;
        }
        Attach(s);
    }
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
    m_connected = true;
    return true;
}


bool UdpSocket::Open6(const std::string& host,port_t port)
{
    if (GetSocket() == INVALID_SOCKET)
    {
        SOCKET s = CreateSocket6(SOCK_DGRAM, "udp");
        if (s == INVALID_SOCKET)
        {
            return false;
        }
        Attach(s);
    }
    struct in6_addr a;
    if (u2ip(host, a))
    {
        return Open6(a, port);
    }
    return false;
}
#endif

void UdpSocket::CreateConnection()
{
    if (GetSocket() == INVALID_SOCKET)
    {
        SOCKET s = CreateSocket4(SOCK_DGRAM, "udp");
        if (s == INVALID_SOCKET)
        {
            return;
        }
        Attach(s);
    }
}


#ifdef IPPROTO_IPV6
void UdpSocket::CreateConnection6()
{
    if (GetSocket() == INVALID_SOCKET)
    {
        SOCKET s = CreateSocket6(SOCK_DGRAM, "udp");
        if (s == INVALID_SOCKET)
        {
            return;
        }
        Attach(s);
    }
}
#endif                                            // IPPROTO_IPV6

/** send to specified address */
void UdpSocket::SendToBuf(const std::string& h,port_t p,const char *data,int len,int flags)
{
    if (GetSocket() == INVALID_SOCKET)
    {
        SOCKET s = CreateSocket4(SOCK_DGRAM, "udp");
        if (s == INVALID_SOCKET)
        {
            return;
        }
        Attach(s);
    }
    ipaddr_t a;
    if (u2ip(h,a))
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
}


/** send to specified address */
void UdpSocket::SendToBuf(ipaddr_t a,port_t p,const char *data,int len,int flags)
{
    if (GetSocket() == INVALID_SOCKET)
    {
        SOCKET s = CreateSocket4(SOCK_DGRAM, "udp");
        if (s == INVALID_SOCKET)
        {
            return;
        }
        Attach(s);
    }
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
}


#ifdef IPPROTO_IPV6
void UdpSocket::SendToBuf6(const std::string& h,port_t p,const char *data,int len,int flags)
{
    if (GetSocket() == INVALID_SOCKET)
    {
        SOCKET s = CreateSocket6(SOCK_DGRAM, "udp");
        if (s == INVALID_SOCKET)
        {
            return;
        }
        Attach(s);
    }
    struct in6_addr a;
    if (u2ip(h,a))
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
void UdpSocket::SendTo6(const std::string& a,port_t p,const std::string& str,int flags)
{
    SendToBuf6(a,p,str.c_str(),(int)str.size(),flags);
}
#endif

/** send to connected address */
void UdpSocket::SendBuf(const char *data,int len,int flags)
{
    if (!m_connected)
    {
        Handler().LogError(this,"SendBuf",0,"not connected",LOG_LEVEL_ERROR);
        return;
    }
    if (send(GetSocket(),data,len,flags) == -1)
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
        if (n == -1)
        {
            Handler().LogError(this, "recvfrom", Errno, StrError(Errno), LOG_LEVEL_ERROR);
            return;
        }
        if (sa_len != sizeof(sa))
        {
            Handler().LogError(this, "recvfrom", 0, "unexpected address struct size", LOG_LEVEL_WARNING);
        }
        this -> OnRawData(m_ibuf, n, (struct sockaddr *)&sa, sa_len);
        return;
    }
#endif
    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    int n = recvfrom(GetSocket(), m_ibuf, m_ibufsz, 0, (struct sockaddr *)&sa, &sa_len);
    if (n == -1)
    {
        Handler().LogError(this, "recvfrom", Errno, StrError(Errno), LOG_LEVEL_ERROR);
        return;
    }
    if (sa_len != sizeof(sa))
    {
        Handler().LogError(this, "recvfrom", 0, "unexpected address struct size", LOG_LEVEL_WARNING);
    }
    this -> OnRawData(m_ibuf, n, (struct sockaddr *)&sa, sa_len);
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
        if (u2ip( group, addr ))
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
    struct ip_mreq x;                             // ip_mreqn
    ipaddr_t addr;
    if (u2ip( group, addr ))
    {
        memcpy(&x.imr_multiaddr.s_addr, &addr, sizeof(addr));
        u2ip( local_if, addr);
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
        if (u2ip( group, addr ))
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
    struct ip_mreq x;                             // ip_mreqn
    ipaddr_t addr;
    if (u2ip( group, addr ))
    {
        memcpy(&x.imr_multiaddr.s_addr, &addr, sizeof(addr));
        u2ip( local_if, addr);
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
#endif                                            // IPPROTO_IPV6
