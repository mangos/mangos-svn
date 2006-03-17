/**
 **	File ......... Socket.cpp
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
#include <ctype.h>
#include <fcntl.h>
#include "Parse.h"
#include "SocketHandler.h"
#include "SocketThread.h"
#include "Utility.h"

#include "Socket.h"

#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x)
#endif

// statics
#ifdef _WIN32
WSAInitializer Socket::m_winsock_init;
#endif

Socket::Socket(SocketHandler& h)
:m_handler(h)
,m_socket( INVALID_SOCKET )
,m_bDel(false)
,m_bClose(false)
,m_bConnecting(false)
,m_tCreate(time(NULL))
,m_line_protocol(false)
,m_ssl_connecting(false)
//, m_tActive(time(NULL))
//, m_timeout(0)
,m_detach(false)
,m_detached(false)
,m_pThread(NULL)
,m_ipv6(false)
,m_sa_len(0)
,m_parent(NULL)
,m_socket_type(0)
,m_bClient(false)
,m_bRetain(false)
,m_bLost(false)
,m_call_on_connect(false)
,m_opt_reuse(true)
,m_opt_keepalive(true)
,m_bSocks4(false)
,m_socks4_host(h.GetSocks4Host())
,m_socks4_port(h.GetSocks4Port())
,m_socks4_userid(h.GetSocks4Userid())
,m_connect_timeout(5)
,m_b_enable_ssl(false)
,m_b_ssl(false)
,m_b_ssl_server(false)
,m_b_disable_read(false)
{
}


Socket::~Socket()
{
    if (m_socket != INVALID_SOCKET && !m_bRetain)
    {
        Close();
    }
    if (m_pThread)
    {
        delete m_pThread;
    }
}


void Socket::Init()
{
}


void Socket::OnRead()
{
}


void Socket::OnWrite()
{
}


void Socket::OnException()
{
// errno valid here?
    int err;
    socklen_t errlen = sizeof(err);
#ifdef _WIN32
    getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char *)&err, &errlen);
#else
    getsockopt(m_socket, SOL_SOCKET, SO_ERROR, &err, &errlen);
#endif
    Handler().LogError(this, "exception on select", Errno, StrError(Errno), LOG_LEVEL_FATAL);
    SetCloseAndDelete();
}


void Socket::OnDelete()
{
}


void Socket::OnConnect()
{
}


bool Socket::CheckConnect()
{
    int err;
    socklen_t errlen = sizeof(err);
    bool r = true;
#ifdef _WIN32
    getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char *)&err, &errlen);
#else
    getsockopt(m_socket, SOL_SOCKET, SO_ERROR, &err, &errlen);
#endif
    if (err)
    {
        Handler().LogError(this, "connect failed", err, StrError(err), LOG_LEVEL_FATAL);
        r = false;
    }
    SetConnecting(false);
// %! add to read fd_set here
    if (r)                                        // ok
    {
        Set(!IsDisableRead(), false);
    }
    return r;
}


void Socket::OnAccept()
{
}


int Socket::Close()
{
    if (m_socket == INVALID_SOCKET)               // this could happen
    {
        Handler().LogError(this, "Socket::Close", 0, "file descriptor invalid", LOG_LEVEL_WARNING);
        return 0;
    }
    int n;
    SetNonblocking(true);
    if (shutdown(m_socket, SHUT_RDWR) == -1)
    {
// failed...
        Handler().LogError(this, "shutdown", Errno, StrError(Errno), LOG_LEVEL_ERROR);
    }
//
    char tmp[100];
    if ((n = recv(m_socket,tmp,100,0)) == -1)
    {
//		Handler().LogError(this, "read() after shutdown", Errno, StrError(Errno), LOG_LEVEL_WARNING);
    }
    else
    {
        if (n)
            Handler().LogError(this, "read() after shutdown", n, "bytes read", LOG_LEVEL_WARNING);
    }
    if ((n = closesocket(m_socket)) == -1)
    {
// failed...
        Handler().LogError(this, "close", Errno, StrError(Errno), LOG_LEVEL_ERROR);
    }
    m_socket = INVALID_SOCKET;
    return n;
}


SOCKET Socket::CreateSocket4(int type, const std::string& protocol)
{
    struct protoent *p = NULL;
    int optval;
    SOCKET s;

    m_socket_type = type;
    m_socket_protocol = protocol;
    if (protocol.size())
    {
        p = getprotobyname( protocol.c_str() );
        if (!p)
        {
            Handler().LogError(this, "getprotobyname", Errno, StrError(Errno), LOG_LEVEL_FATAL);
            return INVALID_SOCKET;
        }
    }
    int protno = p ? p -> p_proto : 0;

    s = socket(AF_INET, type, protno);
    if (s == INVALID_SOCKET)
    {
        Handler().LogError(this, "socket", Errno, StrError(Errno), LOG_LEVEL_FATAL);
        return INVALID_SOCKET;
    }
    OnOptions(AF_INET, type, protno, s);
    if (type == SOCK_STREAM)
    {
        optval = m_opt_reuse ? 1 : 0;
        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) == -1)
        {
            Handler().LogError(this, "setsockopt(SOL_SOCKET, SO_REUSEADDR)", Errno, StrError(Errno), LOG_LEVEL_FATAL);
            closesocket(s);
            return INVALID_SOCKET;
        }

        optval = m_opt_keepalive ? 1 : 0;
        if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&optval, sizeof(optval)) == -1)
        {
            Handler().LogError(this, "setsockopt(SOL_SOCKET, SO_KEEPALIVE)", Errno, StrError(Errno), LOG_LEVEL_FATAL);
            closesocket(s);
            return INVALID_SOCKET;
        }
	    optval = 1;
	    setsockopt(s, 0x06, TCP_NODELAY, (char *)&optval,sizeof(optval));
    }

    return s;
}


#ifdef IPPROTO_IPV6
SOCKET Socket::CreateSocket6(int type, const std::string& protocol)
{
    struct protoent *p = NULL;
    int optval;
    SOCKET s;

    m_socket_type = type;
    m_socket_protocol = protocol;
    if (protocol.size())
    {
        p = getprotobyname( protocol.c_str() );
        if (!p)
        {
            Handler().LogError(this, "getprotobyname", Errno, StrError(Errno), LOG_LEVEL_FATAL);
            return INVALID_SOCKET;
        }
    }
    int protno = p ? p -> p_proto : 0;

    s = socket(AF_INET6, type, protno);
    if (s == INVALID_SOCKET)
    {
        Handler().LogError(this, "socket", Errno, StrError(Errno), LOG_LEVEL_FATAL);
        return INVALID_SOCKET;
    }
    OnOptions(AF_INET6, type, protno, s);
    if (type == SOCK_STREAM)
    {
        optval = m_opt_reuse ? 1 : 0;
        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) == -1)
        {
            Handler().LogError(this, "setsockopt(SOL_SOCKET, SO_REUSEADDR)", Errno, StrError(Errno), LOG_LEVEL_FATAL);
            closesocket(s);
            return INVALID_SOCKET;
        }

        optval = m_opt_keepalive ? 1 : 0;
        if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&optval, sizeof(optval)) == -1)
        {
            Handler().LogError(this, "setsockopt(SOL_SOCKET, SO_KEEPALIVE)", Errno, StrError(Errno), LOG_LEVEL_FATAL);
            closesocket(s);
            return INVALID_SOCKET;
        }
    }
    m_ipv6 = true;
    return s;
}
#endif

bool Socket::isip(const std::string& str)
{
    if (m_ipv6)
    {
        size_t qc = 0;
        size_t qd = 0;
        for (size_t i = 0; i < str.size(); i++)
        {
            qc += (str[i] == ':') ? 1 : 0;
            qd += (str[i] == '.') ? 1 : 0;
        }
        if (qc > 7)
        {
            return false;
        }
        if (qd && qd != 3)
        {
            return false;
        }
        Parse pa(str,":.");
        std::string tmp = pa.getword();
        while (tmp.size())
        {
            if (tmp.size() > 4)
            {
                return false;
            }
            for (size_t i = 0; i < tmp.size(); i++)
            {
                if (tmp[i] < '0' || (tmp[i] > '9' && tmp[i] < 'A') ||
                    (tmp[i] > 'F' && tmp[i] < 'a') || tmp[i] > 'f')
                {
                    return false;
                }
            }
//
            tmp = pa.getword();
        }
        return true;
    }
    for (size_t i = 0; i < str.size(); i++)
        if (!isdigit(str[i]) && str[i] != '.')
            return false;
    return true;
}


bool Socket::u2ip(const std::string& str, ipaddr_t& l)
{
    if (m_ipv6)
    {
        Handler().LogError(this, "u2ip", 0, "converting to ipv4 on ipv6 socket", LOG_LEVEL_ERROR);
        return false;
    }
    if (isip(str))
    {
        Parse pa((char *)str.c_str(), ".");
        union
        {
            struct
            {
                unsigned char b1;
                unsigned char b2;
                unsigned char b3;
                unsigned char b4;
            } a;
            ipaddr_t l;
        } u;
        u.a.b1 = static_cast<unsigned char>(pa.getvalue());
        u.a.b2 = static_cast<unsigned char>(pa.getvalue());
        u.a.b3 = static_cast<unsigned char>(pa.getvalue());
        u.a.b4 = static_cast<unsigned char>(pa.getvalue());
        l = u.l;
        return true;
    }
    else
    {
        struct hostent *he = gethostbyname( str.c_str() );
        if (!he)
        {
            Handler().LogError(this, "gethostbyname", Errno, StrError(Errno), LOG_LEVEL_WARNING);
            return false;
        }
        memcpy(&l, he -> h_addr, 4);
        return true;
    }
    return false;
}


#ifdef IPPROTO_IPV6
bool Socket::u2ip(const std::string& str, struct in6_addr& l)
{
    if (!m_ipv6)
    {
        Handler().LogError(this, "u2ip", 0, "converting to ipv6 on ipv4 socket", LOG_LEVEL_ERROR);
        return false;
    }
    if (isip(str))
    {
        std::list<std::string> vec;
        size_t x = 0;
        char s[100];
        for (size_t i = 0; i <= str.size(); i++)
        {
            if (i == str.size() || str[i] == ':')
            {
                strncpy(s, str.substr(x,i - x).c_str(), i - x);
                s[i - x] = 0;
//
                if (strstr(s,"."))                // x.x.x.x
                {
                    Parse pa(s,".");
                    char slask[100];
                    unsigned long b0 = static_cast<unsigned long>(pa.getvalue());
                    unsigned long b1 = static_cast<unsigned long>(pa.getvalue());
                    unsigned long b2 = static_cast<unsigned long>(pa.getvalue());
                    unsigned long b3 = static_cast<unsigned long>(pa.getvalue());
                    sprintf(slask,"%lx",b0 * 256 + b1);
                    vec.push_back(slask);
                    sprintf(slask,"%lx",b2 * 256 + b3);
                    vec.push_back(slask);
                }
                else
                {
                    vec.push_back(s);
                }
//
                x = i + 1;
            }
        }
        size_t sz = vec.size();                   // number of byte pairs
        size_t i = 0;                             // index in in6_addr.in6_u.u6_addr16[] ( 0 .. 7 )
        for (std::list<std::string>::iterator it = vec.begin(); it != vec.end(); it++)
        {
            std::string bytepair = *it;
            if (bytepair.size())
            {
                l.s6_addr16[i++] = htons(Utility::hex2unsigned(bytepair));
            }
            else
            {
                l.s6_addr16[i++] = 0;
                while (sz++ < 8)
                {
                    l.s6_addr16[i++] = 0;
                }
            }
        }
        return true;
    }
    else
    {
#ifdef SOLARIS
        int errnum = 0;
        struct hostent *he = getipnodebyname( str.c_str(), AF_INET6, 0, &errnum );
#else
        struct hostent *he = gethostbyname2( str.c_str(), AF_INET6 );
#endif
        if (!he)
        {
#ifdef SOLARIS
            Handler().LogError(this, "getipnodebyname", errnum, "failed, see error code");
#else
            Handler().LogError(this, "gethostbyname2", Errno, StrError(Errno), LOG_LEVEL_WARNING);
#endif
            return false;
        }
        memcpy(&l,he -> h_addr_list[0],he -> h_length);
#ifdef SOLARIS
        free(he);
#endif
        return true;
    }
    return false;
}
#endif

void Socket::l2ip(const ipaddr_t ip, std::string& str)
{
    if (m_ipv6)
    {
        Handler().LogError(this, "l2ip", 0, "converting to ipv4 on ipv6 socket", LOG_LEVEL_ERROR);
        str = "";
        return;
    }
    union
    {
        struct
        {
            unsigned char b1;
            unsigned char b2;
            unsigned char b3;
            unsigned char b4;
        } a;
        ipaddr_t l;
    } u;
    u.l = ip;
    char tmp[100];
    sprintf(tmp, "%u.%u.%u.%u", u.a.b1, u.a.b2, u.a.b3, u.a.b4);
    str = tmp;
}


#ifdef IPPROTO_IPV6
void Socket::l2ip(const struct in6_addr& ip, std::string& str,bool mixed)
{
    if (!m_ipv6)
    {
        Handler().LogError(this, "l2ip", 0, "converting to ipv6 on ipv4 socket", LOG_LEVEL_ERROR);
        str = "";
        return;
    }
    char slask[100];
    *slask = 0;
    unsigned int prev = 0;
    if (mixed)
    {
        unsigned int x;
        for (size_t i = 0; i < 6; i++)
        {
            x = ntohs(ip.s6_addr16[i]);
            if (*slask && (x || prev))
                strcat(slask,":");
            if (x)
            {
                sprintf(slask + strlen(slask),"%X", x);
            }
            prev = x;
        }
        x = ntohs(ip.s6_addr16[6]);
        sprintf(slask + strlen(slask),":%u.%u",x / 256,x & 255);
        x = ntohs(ip.s6_addr16[7]);
        sprintf(slask + strlen(slask),".%u.%u",x / 256,x & 255);
    }
    else
    {
        for (size_t i = 0; i < 8; i++)
        {
            unsigned int x = ntohs(ip.s6_addr16[i]);
            if (*slask && (x || prev))
                strcat(slask,":");
            if (x)
            {
                sprintf(slask + strlen(slask),"%X", x);
            }
            prev = x;
        }
    }
    str = slask;
}
#endif

void Socket::Attach(SOCKET s)
{
    m_socket = s;
}


SOCKET Socket::GetSocket()
{
    return m_socket;
}


void Socket::SetDeleteByHandler(bool x)
{
    m_bDel = x;
}


bool Socket::DeleteByHandler()
{
    return m_bDel;
}


void Socket::SetCloseAndDelete(bool x)
{
    m_bClose = x;
}


bool Socket::CloseAndDelete()
{
    return m_bClose;
}


void Socket::SetConnecting(bool x)
{
    m_bConnecting = x;
    m_tConnect = time(NULL);
}


bool Socket::Connecting()
{
    return m_bConnecting;
}


void Socket::SetRemoteAddress(struct sockaddr* sa, socklen_t l)
{
    memcpy(&m_sa, sa, l);
    m_sa_len = l;
}


void Socket::GetRemoteSocketAddress(struct sockaddr& sa,socklen_t& sa_len)
{
    memcpy(&sa, &m_sa, m_sa_len);
    sa_len = m_sa_len;
}


SocketHandler& Socket::Handler() const
{
    return m_handler;
}


ipaddr_t Socket::GetRemoteIP4()
{
    ipaddr_t l = 0;
    struct sockaddr_in* saptr = (struct sockaddr_in*)&m_sa;
    if (m_ipv6)
    {
        Handler().LogError(this, "GetRemoteIP4", 0, "get ipv4 address for ipv6 socket", LOG_LEVEL_WARNING);
    }
    memcpy(&l, &saptr -> sin_addr, 4);
    return l;
}


#ifdef IPPROTO_IPV6
struct in6_addr Socket::GetRemoteIP6()
{
    struct sockaddr_in6 *p = (struct sockaddr_in6 *)&m_sa;
    if (!m_ipv6)
    {
        Handler().LogError(this, "GetRemoteIP6", 0, "get ipv6 address for ipv4 socket", LOG_LEVEL_WARNING);
    }
    return p -> sin6_addr;
}
#endif

port_t Socket::GetRemotePort()
{
#ifdef IPPROTO_IPV6
    if (m_ipv6)
    {
        struct sockaddr_in6 *p = (struct sockaddr_in6 *)&m_sa;
        return ntohs(p -> sin6_port);
    }
#endif
    struct sockaddr_in* saptr = (struct sockaddr_in*)&m_sa;
    return ntohs(saptr -> sin_port);
}


std::string Socket::GetRemoteAddress()
{
    std::string str;
#ifdef IPPROTO_IPV6
    if (m_ipv6)
    {
        l2ip(GetRemoteIP6(), str);
        return str;
    }
#endif
    l2ip(GetRemoteIP4(), str);
    return str;
}


std::string Socket::GetRemoteHostname()
{
    std::string str;
#ifdef IPPROTO_IPV6
    if (m_ipv6)
    {
        Handler().LogError(this, "GetRemoteHostname", 0, "not implemented for ipv6", LOG_LEVEL_WARNING);
        return GetRemoteAddress();
    }
#endif
    long l = GetRemoteIP4();
//#ifdef LINUX
//	struct hostent *he = gethostbyaddr(&l, sizeof(long), AF_INET);
//#else // _WIN32, MACOSX and SOLARIS
    struct hostent *he = gethostbyaddr( (char *)&l, sizeof(long), AF_INET);
//#endif
    if (!he)
    {
        return GetRemoteAddress();
    }
    str = he -> h_name;
    return str;
}


bool Socket::SetNonblocking(bool bNb)
{
#ifdef _WIN32
    unsigned long l = bNb ? 1 : 0;
    int n = ioctlsocket(m_socket, FIONBIO, &l);
    if (n != 0)
    {
        Handler().LogError(this, "ioctlsocket(FIONBIO)", Errno, "");
        return false;
    }
    return true;
#else
    if (bNb)
    {
        if (fcntl(m_socket, F_SETFL, O_NONBLOCK) == -1)
        {
            Handler().LogError(this, "fcntl(F_SETFL, O_NONBLOCK)", Errno, StrError(Errno), LOG_LEVEL_ERROR);
            return false;
        }
    }
    else
    {
        if (fcntl(m_socket, F_SETFL, 0) == -1)
        {
            Handler().LogError(this, "fcntl(F_SETFL, 0)", Errno, StrError(Errno), LOG_LEVEL_ERROR);
            return false;
        }
    }
    return true;
#endif
}


bool Socket::SetNonblocking(bool bNb, SOCKET s)
{
#ifdef _WIN32
    unsigned long l = bNb ? 1 : 0;
    int n = ioctlsocket(s, FIONBIO, &l);
    if (n != 0)
    {
        Handler().LogError(this, "ioctlsocket(FIONBIO)", Errno, "");
        return false;
    }
    return true;
#else
    if (bNb)
    {
        if (fcntl(s, F_SETFL, O_NONBLOCK) == -1)
        {
            Handler().LogError(this, "fcntl(F_SETFL, O_NONBLOCK)", Errno, StrError(Errno), LOG_LEVEL_ERROR);
            return false;
        }
    }
    else
    {
        if (fcntl(s, F_SETFL, 0) == -1)
        {
            Handler().LogError(this, "fcntl(F_SETFL, 0)", Errno, StrError(Errno), LOG_LEVEL_ERROR);
            return false;
        }
    }
    return true;
#endif
}


void Socket::Set(bool bRead, bool bWrite, bool bException)
{
    m_handler.Set(m_socket, bRead, bWrite, bException);
}


time_t Socket::GetConnectTime()
{
    return time(NULL) - m_tConnect;
}


bool Socket::Ready()
{
    if (m_socket != INVALID_SOCKET && !Connecting() && !CloseAndDelete())
        return true;
    return false;
}


bool Socket::Detach()
{
    if (!DeleteByHandler())
        return false;
    if (m_pThread)
        return false;
    if (m_detached)
        return false;
    m_detach = true;
    return true;
}


void Socket::DetachSocket()
{
    m_pThread = new SocketThread(*this);
    m_pThread -> SetRelease(true);
}


void Socket::OnLine(const std::string& )
{
}


void Socket::OnSSLInitDone()
{
}


bool Socket::SSLCheckConnect()
{
    return false;
}


void Socket::SetSSLConnecting(bool x)
{
    m_ssl_connecting = x;
}


bool Socket::SSLConnecting()
{
    return m_ssl_connecting;
}


void Socket::SetLineProtocol(bool x)
{
    m_line_protocol = x;
}


bool Socket::LineProtocol()
{
    return m_line_protocol;
}


void Socket::ReadLine()
{
}


void Socket::OnConnectFailed()
{
}


Socket::Socket(const Socket& s) : m_handler(s.Handler())
{
}


Socket *Socket::GetParent()
{
    return m_parent;
}


void Socket::SetParent(Socket *x)
{
    m_parent = x;
}


port_t Socket::GetPort()
{
    Handler().LogError(this, "GetPort", 0, "GetPort only implemented for ListenSocket", LOG_LEVEL_WARNING);
    return 0;
}


void Socket::CopyConnection(Socket *sock)
{
    Attach( sock -> GetSocket() );
    SetSocketType( sock -> GetSocketType() );
    SetSocketProtocol( sock -> GetSocketProtocol() );
    SetClientRemoteAddr( sock -> GetClientRemoteAddr() );
    SetClientRemotePort( sock -> GetClientRemotePort() );

    struct sockaddr sa;
    socklen_t sa_len;
    sock -> GetRemoteSocketAddress(sa, sa_len);
    SetRemoteAddress(&sa, sa_len);
}


void Socket::OnOptions(int family,int type,int protocol,SOCKET s)
{
/*
    Handler().LogError(this, "OnOptions", family, "Address Family", LOG_LEVEL_INFO);
    Handler().LogError(this, "OnOptions", type, "Type", LOG_LEVEL_INFO);
    Handler().LogError(this, "OnOptions", protocol, "Protocol", LOG_LEVEL_INFO);
*/
    SetReuse(true);
    SetKeepalive(true);
}


int Socket::Resolve(const std::string& host,port_t port)
{
    return Handler().Resolve(this, host, port);
}


void Socket::OnSocks4Connect()
{
    Handler().LogError(this, "OnSocks4Connect", 0, "Use with TcpSocket only");
}


void Socket::OnSocks4ConnectFailed()
{
    Handler().LogError(this, "OnSocks4ConnectFailed", 0, "Use with TcpSocket only");
}


bool Socket::OnSocks4Read()
{
    Handler().LogError(this, "OnSocks4Read", 0, "Use with TcpSocket only");
    return true;
}


void Socket::SetSocks4Host(const std::string& host)
{
    u2ip(host, m_socks4_host);
}


/*
void Socket::OnWriteComplete()
{
}
*/

void Socket::Resolved(int,ipaddr_t,port_t)
{
}
