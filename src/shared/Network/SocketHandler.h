/** \file SocketHandler.h
 **	\date  2004-02-13
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
#ifndef _SOCKETS_SocketHandler_H
#define _SOCKETS_SocketHandler_H

#include "sockets-config.h"
#include <map>
#include <list>

#include "socket_include.h"
#include "StdLog.h"
#include "Mutex.h"
#include "ISocketHandler.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


class Socket;
#ifdef ENABLE_RESOLVER
class ResolvServer;
#endif
class Mutex;

/** Socket container class, event generator. 
	\ingroup basic */
class SocketHandler : public ISocketHandler
{
protected:
	/** Map type for holding file descriptors/socket object pointers. */
	typedef std::map<SOCKET,Socket *> socket_m;

public:
	/** SocketHandler constructor.
		\param log Optional log class pointer */
	SocketHandler(StdLog *log = NULL);
	/** SocketHandler threadsafe constructor.
		\param mutex Externally declared mutex variable
		\param log Optional log class pointer */
	SocketHandler(Mutex& mutex,StdLog *log = NULL);
	~SocketHandler();

	/** Add socket instance to socket map. Removal is always automatic. */
	void Add(Socket *);
private:
	/** Remove socket from socket map, used by Socket class. */
	void Remove(Socket *);
public:
	/** Get status of read/write/exception file descriptor set for a socket. */
	void Get(SOCKET s,bool& r,bool& w,bool& e);
	/** Set read/write/exception file descriptor sets (fd_set). */
	void Set(SOCKET s,bool bRead,bool bWrite,bool bException = true);

	/** Wait for events, generate callbacks. */
	int Select(long sec,long usec);
	/** This method will not return until an event has been detected. */
	int Select();
	/** Wait for events, generate callbacks. */
	int Select(struct timeval *tsel);

	/** Check that a socket really is handled by this socket handler. */
	bool Valid(Socket *);
	/** Return number of sockets handled by this handler.  */
	size_t GetCount();

	/** Override and return false to deny all incoming connections. 
		\param p ListenSocket class pointer (use GetPort to identify which one) */
	bool OkToAccept(Socket *p);

	/** Called by Socket when a socket changes state. */
	void AddList(SOCKET s,list_t which_one,bool add);

	// Connection pool
#ifdef ENABLE_POOL
	/** Find available open connection (used by connection pool). */
	ISocketHandler::PoolSocket *FindConnection(int type,const std::string& protocol,SocketAddress&);
	/** Enable connection pool (by default disabled). */
	void EnablePool(bool x = true);
	/** Check pool status. 
		\return true if connection pool is enabled */
	bool PoolEnabled();
#endif // ENABLE_POOL

	// Socks4
#ifdef ENABLE_SOCKS4
	/** Set socks4 server ip that all new tcp sockets should use. */
	void SetSocks4Host(ipaddr_t);
	/** Set socks4 server hostname that all new tcp sockets should use. */
	void SetSocks4Host(const std::string& );
	/** Set socks4 server port number that all new tcp sockets should use. */
	void SetSocks4Port(port_t);
	/** Set optional socks4 userid. */
	void SetSocks4Userid(const std::string& );
	/** If connection to socks4 server fails, immediately try direct connection to final host. */
	void SetSocks4TryDirect(bool x = true);
	/** Get socks4 server ip. 
		\return socks4 server ip */
	ipaddr_t GetSocks4Host();
	/** Get socks4 port number.
		\return socks4 port number */
	port_t GetSocks4Port();
	/** Get socks4 userid (optional).
		\return socks4 userid */
	const std::string& GetSocks4Userid();
	/** Check status of socks4 try direct flag.
		\return true if direct connection should be tried if connection to socks4 server fails */
	bool Socks4TryDirect();
#endif // ENABLE_SOCKS4

	// DNS resolve server
#ifdef ENABLE_RESOLVER
	/** Enable asynchronous DNS. 
		\param port Listen port of asynchronous dns server */
	void EnableResolver(port_t port = 16667);
	/** Check resolver status.
		\return true if resolver is enabled */
	bool ResolverEnabled();
	/** Queue a dns request.
		\param host Hostname to be resolved
		\param port Port number will be echoed in Socket::OnResolved callback */
	int Resolve(Socket *,const std::string& host,port_t port);
#ifdef ENABLE_IPV6
	int Resolve6(Socket *,const std::string& host,port_t port);
#endif
	/** Do a reverse dns lookup. */
	int Resolve(Socket *,ipaddr_t a);
#ifdef ENABLE_IPV6
	int Resolve(Socket *,in6_addr& a);
#endif
	/** Get listen port of asynchronous dns server. */
	port_t GetResolverPort();
	/** Resolver thread ready for queries. */
	bool ResolverReady();
#endif

	/** Sanity check of those accursed lists. */
	void CheckSanity();

protected:
	socket_m m_sockets; ///< Active sockets list
	socket_m m_add; ///< Sockets to be added to sockets list
	std::list<Socket *> m_delete; ///< Sockets to be deleted (failed when Add)

private:
	void CheckList(socket_v&,const std::string&); ///< Used by CheckSanity
	SOCKET m_maxsock; ///< Highest file descriptor + 1 in active sockets list
	fd_set m_rfds; ///< file descriptor set monitored for read events
	fd_set m_wfds; ///< file descriptor set monitored for write events
	fd_set m_efds; ///< file descriptor set monitored for exceptions
	int m_preverror; ///< debug select() error
	int m_errcnt;
#ifdef ENABLE_SOCKS4
	ipaddr_t m_socks4_host; ///< Socks4 server host ip
	port_t m_socks4_port; ///< Socks4 server port number
	std::string m_socks4_userid; ///< Socks4 userid
	bool m_bTryDirect; ///< Try direct connection if socks4 server fails
#endif
#ifdef ENABLE_RESOLVER
	int m_resolv_id; ///< Resolver id counter
	ResolvServer *m_resolver; ///< Resolver thread pointer
	port_t m_resolver_port; ///< Resolver listen port
#endif
#ifdef ENABLE_POOL
	bool m_b_enable_pool; ///< Connection pool enabled if true
#endif
	socket_v m_fds; ///< Active file descriptor list
	socket_v m_fds_erase; ///< File descriptors that are to be erased from m_sockets
	socket_v m_fds_callonconnect; ///< checklist CallOnConnect
#ifdef ENABLE_DETACH
	socket_v m_fds_detach; ///< checklist Detach
#endif
	socket_v m_fds_connecting; ///< checklist Connecting
	socket_v m_fds_retry; ///< checklist retry client connect
	socket_v m_fds_close; ///< checklist close and delete
};


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _SOCKETS_SocketHandler_H
