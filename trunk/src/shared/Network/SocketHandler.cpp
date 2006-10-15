/**
 **	File ......... SocketHandler.cpp
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
#include <stdio.h>
#ifdef _WIN32
#pragma warning(disable:4786)
#include <stdlib.h>
#else
#include <errno.h>
#endif

#include "TcpSocket.h"
#include "StdLog.h"
#include "SocketHandler.h"
#include "UdpSocket.h"
#include "PoolSocket.h"
#include "ResolvSocket.h"
#include "ResolvServer.h"

#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x)
#endif

SocketHandler::SocketHandler(StdLog *p)
:m_stdlog(p)
,m_maxsock(0)
,m_host("")
,m_ip(0)
,m_preverror(-1)
,m_slave(false)
,m_local_resolved(false)
,m_socks4_host(0)
,m_socks4_port(0)
,m_bTryDirect(false)
,m_resolv_id(0)
,m_resolver(NULL)
{
    FD_ZERO(&m_rfds);
    FD_ZERO(&m_wfds);
    FD_ZERO(&m_efds);
}


SocketHandler::~SocketHandler()
{
    if (m_resolver)
        m_resolver -> Quit();
    if (!m_slave)
    {
        for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
        {
            Socket *p = (*it).second;
            p -> Close();
//			p -> OnDelete(); // hey, I turn this back on. what's the worst that could happen??!!
// MinionSocket breaks, calling MinderHandler methods in OnDelete -
// MinderHandler is already gone when that happens...
            if (p -> DeleteByHandler())
            {
                delete p;
            }
        }
    }
    if (m_resolver)
        delete m_resolver;
}


void SocketHandler::ResolveLocal()
{
    char h[256];

// get local hostname and translate into ip-address
    *h = 0;
    gethostname(h,255);
    {
        Socket zl(*this);
        if (zl.u2ip(h, m_ip))
        {
            zl.l2ip(m_ip, m_addr);
        }
    }
#ifdef IPPROTO_IPV6
    memset(&m_local_ip6, 0, sizeof(m_local_ip6));
    {
        Socket zl(*this);
        zl.SetIpv6();
        if (zl.u2ip(h, m_local_ip6))
        {
            zl.l2ip(m_local_ip6, m_local_addr6);
        }
    }
#endif
    m_host = h;
    m_local_resolved = true;
}


void SocketHandler::Add(Socket *p)
{
    if (p -> GetSocket() == INVALID_SOCKET)
    {
        LogError(p, "Add", -1, "Invalid socket", LOG_LEVEL_FATAL);
        return;
    }
    DEB(    printf("%s: add socket %d\n",m_slave ? "slave" : "master",p -> GetSocket());)
        m_add[p -> GetSocket()] = p;
}


void SocketHandler::Get(SOCKET s,bool& r,bool& w,bool& e)
{
    if (s >= 0)
    {
        r = FD_ISSET(s, &m_rfds) ? true : false;
        w = FD_ISSET(s, &m_wfds) ? true : false;
        e = FD_ISSET(s, &m_efds) ? true : false;
    }
}


void SocketHandler::Set(SOCKET s,bool bRead,bool bWrite,bool bException)
{
    if (s >= 0)
    {
        if (bRead)
        {
            if (!FD_ISSET(s, &m_rfds))
            {
                FD_SET(s, &m_rfds);
            }
        }
        else
        {
            FD_CLR(s, &m_rfds);
        }
        if (bWrite)
        {
            if (!FD_ISSET(s, &m_wfds))
            {
                FD_SET(s, &m_wfds);
            }
        }
        else
        {
            FD_CLR(s, &m_wfds);
        }
        if (bException)
        {
            if (!FD_ISSET(s, &m_efds))
            {
                FD_SET(s, &m_efds);
            }
        }
        else
        {
            FD_CLR(s, &m_efds);
        }
    }
}


int SocketHandler::Select(long sec,long usec)
{
    struct timeval tv;
    int n;

    while (m_add.size() && m_sockets.size() < FD_SETSIZE )
    {
        socket_m::iterator it = m_add.begin();
        SOCKET s = (*it).first;
        Socket *p = (*it).second;
// call Open before Add'ing a socket...
        if (p -> Connecting())
        {
            Set(s,false,true);
        }
        else
        {
            if (p -> IsDisableRead())
                Set(s, false, false);
            else
                Set(s,true,false);
        }
        m_maxsock = (s > m_maxsock) ? s : m_maxsock;
        m_sockets[s] = p;
        m_add.erase(it);
    }

#ifdef __APPLE_CC__
    fd_set rfds;
    fd_set wfds;
    fd_set efds;
    FD_COPY(&m_rfds, &rfds);
    FD_COPY(&m_wfds, &wfds);
    FD_COPY(&m_efds, &efds);
#else
    fd_set rfds = m_rfds;
    fd_set wfds = m_wfds;
    fd_set efds = m_efds;
#endif

    tv.tv_sec = sec;
    tv.tv_usec = usec;

    n = select( (int)(m_maxsock + 1),&rfds,&wfds,&efds,&tv);
    if (n == -1)
    {
        LogError(NULL, "select", Errno, StrError(Errno));
#ifdef _WIN32
        DEB(
            int errcode = Errno;
            if (errcode != m_preverror)
        {
            printf("  select() errcode = %d\n",errcode);
                m_preverror = errcode;
                for (size_t i = 0; i <= m_maxsock; i++)
            {
                if (FD_ISSET(i, &m_rfds))
                    printf("%4d: Read\n",i);
                    if (FD_ISSET(i, &m_wfds))
                    printf("%4d: Write\n",i);
                    if (FD_ISSET(i, &m_efds))
                    printf("%4d: Exception\n",i);
            }
        }
        )                                         // DEB
#else
        DEB(        printf("slave: %s\n",m_slave ? "YES" : "NO");
            exit(-1);)
    #endif
    }
    else
//	if (n > 0)
    {
        for (socket_m::iterator it2 = m_sockets.begin(); it2 != m_sockets.end(); it2++)
        {
            SOCKET i = (*it2).first;
            Socket *p = (*it2).second;
            if (p)
            {
                if (p -> CallOnConnect() && p -> Ready() )
                {
                    if (p -> IsSSL())             // SSL Enabled socket
                        p -> OnSSLConnect();
                    else
                    if (p -> Socks4())
                        p -> OnSocks4Connect();
                    else
                        p -> OnConnect();
                    p -> SetCallOnConnect( false );
                }
// new SSL negotiate method
                if (p -> IsSSLNegotiate())
                {
                    p -> SSLNegotiate();
                }
                else
// old SSL method...
                if (p -> SSLConnecting())
                {
                    if (p -> SSLCheckConnect())
                    {
                        p -> OnSSLInitDone();
                    }
                }
                else
                if (n > 0)
                {
                    if (FD_ISSET(i, &rfds))
                    {
                        TcpSocket *tcp = (TcpSocket *)(p);
//TcpSocket *tcp = dynamic_cast<TcpSocket *>(p);
// LockWrite (save total output buffer size)
// Sockets with write lock won't call OnWrite in SendBuf
// That will happen in UnlockWrite, if necessary
                        p -> OnRead();
                        bool need_more = false;
                        while (tcp && p -> Socks4() && tcp -> GetInputLength() && !need_more && !p -> CloseAndDelete())
                        {
                            need_more = p -> OnSocks4Read();
                        }
                        if (!p -> Socks4())
                        {
                            if (p -> LineProtocol())
                            {
                                p -> ReadLine();
                            }
//							p -> Touch();
                        }
// UnlockWrite (call OnWrite if saved size == 0 && total output buffer size > 0)
                    }
                    if (FD_ISSET(i, &wfds))
                    {
                        if (p -> Connecting())
                        {
                            if (p -> CheckConnect())
                            {
                                if (p -> IsSSL()) // SSL Enabled socket
                                    p -> OnSSLConnect();
                                else
                                if (p -> Socks4())
                                    p -> OnSocks4Connect();
                                else
                                    p -> OnConnect();
                            }
                            else
                            {
// failed
                                if (p -> Socks4())
                                {
                                    p -> OnSocks4ConnectFailed();
                                }
                                else
                                {
//									LogError(p, "connect failed", Errno, StrError(Errno), LOG_LEVEL_FATAL);
                                    p -> SetCloseAndDelete( true );
                                    p -> OnConnectFailed();
                                }
                            }
//								p -> Touch();
                        }
                        else
                        {
                            p -> OnWrite();
//								p -> Touch();
                        }
                    }
                    if (FD_ISSET(i, &efds))
                    {
                        p -> OnException();
                    }
                }
            }                                     // if (p)
        }                                         // for
    }

    bool repeat;
    do
    {
        repeat = false;
        for (socket_m::iterator it3 = m_sockets.begin(); it3 != m_sockets.end(); it3++)
        {
//			SOCKET s = (*it3).first;
            Socket *p = (*it3).second;
            if (p)
            {
                if (!m_slave && p -> IsDetach())
                {
                    Set(p -> GetSocket(), false, false, false);
                    p -> DetachSocket();
                    m_sockets.erase(it3);
                    repeat = true;
                    break;
                }
/*
                if (p && p -> Timeout() && p -> Inactive() > p -> Timeout())
                {
                    p -> SetCloseAndDelete();
                }
*/
                if (p && p -> Connecting() && p -> GetConnectTime() > p -> GetConnectTimeout() )
                {
                    LogError(p, "connect", -1, "connect timeout", LOG_LEVEL_FATAL);
                    if (p -> Socks4())
                    {
                        p -> OnSocks4ConnectFailed();
// retry direct connection
                    }
                    else
                    {
                        p -> SetCloseAndDelete(true);
                        p -> OnConnectFailed();
                    }
                }
                if (p && p -> CloseAndDelete() )
                {
//DEB(printf("%s: calling Close for socket %d\n",m_slave ? "slave" : "master",s);)
                    if (p -> Retain() && !p -> Lost())
                    {
                        PoolSocket *p2 = new PoolSocket(*this, p);
                        p2 -> SetDeleteByHandler();
                        Add(p2);
//printf("Adding PoolSocket...\n");
                    }
                    else
                    {
                        Set(p -> GetSocket(),false,false,false);
                        p -> Close();
                    }
                    p -> OnDelete();
                    if (p -> DeleteByHandler())
                    {
                        delete p;
                    }
                    m_sockets.erase(it3);
                    repeat = true;
                    break;
                }
            }                                     // if (p)
        }
        if (repeat)
        {
            m_maxsock = 0;
            for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
            {
                SOCKET s = (*it).first;
                m_maxsock = s > m_maxsock ? s : m_maxsock;
            }
            for (socket_m::iterator it3 = m_add.begin(); it3 != m_add.end(); it3++)
            {
                SOCKET s = (*it3).first;
                m_maxsock = s > m_maxsock ? s : m_maxsock;
            }
        }
    } while (repeat);
    return n;
}


const std::string& SocketHandler::GetLocalHostname()
{
    if (!m_local_resolved)
        LogError(NULL, "GetLocalHostname", 0, "local address not resolved");
    return m_host;
}


ipaddr_t SocketHandler::GetLocalIP()
{
    if (!m_local_resolved)
        LogError(NULL, "GetLocalHostname", 0, "local address not resolved");
    return m_ip;
}


const std::string& SocketHandler::GetLocalAddress()
{
    if (!m_local_resolved)
        LogError(NULL, "GetLocalHostname", 0, "local address not resolved");
    return m_addr;
}


bool SocketHandler::Valid(Socket *p0)
{
    for (socket_m::iterator it3 = m_sockets.begin(); it3 != m_sockets.end(); it3++)
    {
        Socket *p = (*it3).second;
        if (p0 == p)
            return true;
    }
    return false;
}


void SocketHandler::RegStdLog(StdLog *x)
{
    m_stdlog = x;
}


bool SocketHandler::OkToAccept()
{
    return true;
}


size_t SocketHandler::GetCount()
{
    return m_sockets.size();
}


void SocketHandler::SetSlave(bool x)
{
    m_slave = x;
}


void SocketHandler::LogError(Socket *p,const std::string& user_text,int err,const std::string& sys_err,loglevel_t t)
{
    if (m_stdlog)
    {
        m_stdlog -> error(this, p, user_text, err, sys_err, t);
    }
}


#ifdef IPPROTO_IPV6
const struct in6_addr& SocketHandler::GetLocalIP6()
{
    if (!m_local_resolved)
        LogError(NULL, "GetLocalHostname", 0, "local address not resolved");
    return m_local_ip6;
}
#endif

const std::string& SocketHandler::GetLocalAddress6()
{
    if (!m_local_resolved)
        LogError(NULL, "GetLocalHostname", 0, "local address not resolved");
    return m_local_addr6;
}


PoolSocket *SocketHandler::FindConnection(int type,const std::string& protocol,ipaddr_t a,port_t port)
{
    for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end() && m_sockets.size(); it++)
    {
        PoolSocket *pools = (PoolSocket *)((*it).second);
//PoolSocket *pools = dynamic_cast<PoolSocket *>((*it).second);
        if (pools)
        {
            if (pools -> GetSocketType() == type &&
                pools -> GetSocketProtocol() == protocol &&
                pools -> GetClientRemoteAddr() == a &&
                pools -> GetClientRemotePort() == port)
            {
                DEB(printf("FindConnection() successful\n");)
                    m_sockets.erase(it);
                pools -> SetRetain();             // avoid Close in Socket destructor
                return pools;                     // Caller is responsible that this socket is deleted
            }
        }
    }
    DEB(printf("FindConnection() NOT successful\n");)
        return NULL;
}


void SocketHandler::SetSocks4Host(ipaddr_t a)
{
    m_socks4_host = a;
}


void SocketHandler::SetSocks4Host(const std::string& host)
{
    Socket s(*this);
    s.u2ip(host, m_socks4_host);
}


void SocketHandler::SetSocks4Port(port_t port)
{
    m_socks4_port = port;
}


void SocketHandler::SetSocks4Userid(const std::string& id)
{
    m_socks4_userid = id;
}


int SocketHandler::Resolve(Socket *p,const std::string& host,port_t port)
{
// check cache
    ResolvSocket *resolv = new ResolvSocket(*this, p);
    resolv -> SetId(++m_resolv_id);
    resolv -> SetHost(host);
    resolv -> SetPort(port);
    resolv -> SetDeleteByHandler();
    ipaddr_t local;
    resolv -> u2ip("127.0.0.1", local);
    if (!resolv -> Open(local, m_resolver_port))
    {
        LogError(resolv, "Resolve", -1, "Can't connect to local resolve server", LOG_LEVEL_FATAL);
    }
    Add(resolv);
    return m_resolv_id;
}


void SocketHandler::EnableResolver(port_t port)
{
    if (!m_resolver)
    {
        m_resolver_port = port;
        m_resolver = new ResolvServer(port);
    }
}
