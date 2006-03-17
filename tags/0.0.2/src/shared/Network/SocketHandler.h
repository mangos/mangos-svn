/**
 **	File ......... SocketHandler.h
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
#ifndef _SOCKETHANDLER_H
#define _SOCKETHANDLER_H

#include <map>
#include <string>

#include "socket_include.h"
#include "StdLog.h"

class Socket;
class PoolSocket;
class ResolvServer;

class SocketHandler
{
/** Map type for holding file descriptors/socket object pointers. */
    typedef std::map<SOCKET,Socket *> socket_m;

    public:
        SocketHandler(StdLog * = NULL);
        virtual ~SocketHandler();

/** Register StdLog object for error callback. */
        void RegStdLog(StdLog *);
        void LogError(Socket *,const std::string&,int,const std::string&,loglevel_t = LOG_LEVEL_WARNING);

        void Add(Socket *);
/** Set read/write/exception file descriptor sets (fd_set). */
        void Set(SOCKET s,bool bRead,bool bWrite,bool bException = true);
        int Select(long sec,long usec);
        bool Valid(Socket *);
/** Override and return false to deny all incoming connections. */
        virtual bool OkToAccept();
/** Get status of read/write/exception file descriptor set for a socket. */
        void Get(SOCKET s,bool& r,bool& w,bool& e);

/** ResolveLocal (hostname) - call once before calling any GetLocal method. */
        void ResolveLocal();

        const std::string& GetLocalHostname();
        ipaddr_t GetLocalIP();
        const std::string& GetLocalAddress();
        const struct in6_addr& GetLocalIP6();
        const std::string& GetLocalAddress6();

/** Return number of sockets handled by this handler.  */
        size_t GetCount();
/** Indicates that the handler runs under SocketThread. */
        void SetSlave(bool x = true);
/** Find available open connection (used by connection pool). */
        PoolSocket *FindConnection(int type,const std::string& protocol,ipaddr_t,port_t);

/** Enable transparent Socks4 client support. */
        void SetSocks4Host(ipaddr_t);
        void SetSocks4Host(const std::string& );
        void SetSocks4Port(port_t);
        void SetSocks4Userid(const std::string& );
        void SetSocks4TryDirect(bool x = true) { m_bTryDirect = x; }
        ipaddr_t GetSocks4Host() { return m_socks4_host; }
        port_t GetSocks4Port() { return m_socks4_port; }
        const std::string& GetSocks4Userid() { return m_socks4_userid; }
        bool Socks4TryDirect() { return m_bTryDirect; }

/** Enable asynchronous DNS. */
        void EnableResolver(port_t port = 16667);
        bool ResolverEnabled() { return m_resolver ? true : false; }
        int Resolve(Socket *,const std::string& host,port_t);
        port_t GetResolverPort() { return m_resolver_port; }

    protected:
        socket_m m_sockets;
        socket_m m_add;

    private:
        SocketHandler(const SocketHandler& ) {}
        SocketHandler& operator=(const SocketHandler& ) { return *this; }
        StdLog *m_stdlog;
        SOCKET m_maxsock;
        std::string m_host;                       // local
        ipaddr_t m_ip;                            // local
        std::string m_addr;                       // local
        fd_set m_rfds;
        fd_set m_wfds;
        fd_set m_efds;
        int m_preverror;
        bool m_slave;
#ifdef IPPROTO_IPV6
        struct in6_addr m_local_ip6;
#endif
        std::string m_local_addr6;
        bool m_local_resolved;
        ipaddr_t m_socks4_host;
        port_t m_socks4_port;
        std::string m_socks4_userid;
        bool m_bTryDirect;
        int m_resolv_id;
        ResolvServer *m_resolver;
        port_t m_resolver_port;
};
#endif                                            // _SOCKETHANDLER_H
