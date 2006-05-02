/** \file SocketHandler.h
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
#ifndef _SOCKETHANDLER_H
#define _SOCKETHANDLER_H

#include <map>
#include <list>

#include "socket_include.h"
#include "StdLog.h"
#include "Mutex.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


class Socket;
class PoolSocket;
class ResolvServer;
class Mutex;

/** Socket container class, event generator. 
	\ingroup basic */
class SocketHandler
{
	friend class Socket;

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
	virtual ~SocketHandler();

	/** Register StdLog object for error callback. 
		\param log Pointer to log class */
	void RegStdLog(StdLog *log);
	/** Log error to log class for print out / storage. */
	void LogError(Socket *,const std::string&,int,const std::string&,loglevel_t = LOG_LEVEL_WARNING);

	/** Add socket instance to socket map. */
	void Add(Socket *);
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
	/** Override and return false to deny all incoming connections. 
		\param p ListenSocket class pointer (use GetPort to identify which one) */
	virtual bool OkToAccept(Socket *p);
	/** Get status of read/write/exception file descriptor set for a socket. */
	void Get(SOCKET s,bool& r,bool& w,bool& e);

	/** Return number of sockets handled by this handler.  */
	size_t GetCount();
	/** Indicates that the handler runs under SocketThread. */
	void SetSlave(bool x = true);
	/** Find available open connection (used by connection pool). */
	PoolSocket *FindConnection(int type,const std::string& protocol,ipaddr_t,port_t);
#ifdef IPPROTO_IPV6
	/** Find available open connection (used by connection pool). */
	PoolSocket *FindConnection(int type,const std::string& protocol,in6_addr,port_t);
#endif

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

	/** Enable asynchronous DNS. 
		\param port Listen port of asynchronous dns server */
	void EnableResolver(port_t port = 16667);
	/** Check resolver status.
		\return true if resolver is enabled */
	bool ResolverEnabled();
	/** Queue a dns request. */
	int Resolve(Socket *,const std::string& host,port_t);
	/** Get listen port of asynchronous dns server. */
	port_t GetResolverPort();
	/** Resolver thread ready for queries. */
	bool ResolverReady();

	/** Enable connection pool (by default disabled). */
	void EnablePool(bool x = true);
	/** Check pool status. 
		\return true if connection pool is enabled */
	bool PoolEnabled();
private:
	/** Remove socket from socket map, used by Socket class. */
	void Remove(Socket *);
public:
	/** Get checklist: callonconnect */
	socket_v& GetFdsCallOnConnect();
	/** Get checklist: Detach */
	socket_v& GetFdsDetach();
	/** Get checklist: Connecting */
	socket_v& GetFdsConnecting();
	/** Get checklist: Retry client connect */
	socket_v& GetFdsRetry();
	/** Get checklist: Close and delete */
	socket_v& GetFdsClose();

	/** Get mutex reference for threadsafe operations. */
	Mutex& GetMutex() const;

	/** Get socket from active list. */
	Socket *GetSocket(SOCKET );

protected:
	socket_m m_sockets; ///< Active sockets list
	socket_m m_add; ///< Sockets to be added to sockets list
	std::list<Socket *> m_delete; ///< Sockets to be deleted (failed when Add)

private:
	SocketHandler(const SocketHandler& h) : m_mutex(h.GetMutex()) {}
	SocketHandler& operator=(const SocketHandler& ) { return *this; }
	StdLog *m_stdlog; ///< Registered log class, or NULL
	SOCKET m_maxsock; ///< Highest file descriptor + 1 in active sockets list
	fd_set m_rfds; ///< file descriptor set monitored for read events
	fd_set m_wfds; ///< file descriptor set monitored for write events
	fd_set m_efds; ///< file descriptor set monitored for exceptions
#ifdef _WIN32
	int m_preverror; ///< debug select() error
#endif
	bool m_slave; ///< Indicates that this is a SocketHandler run in SocketThread
	ipaddr_t m_socks4_host; ///< Socks4 server host ip
	port_t m_socks4_port; ///< Socks4 server port number
	std::string m_socks4_userid; ///< Socks4 userid
	bool m_bTryDirect; ///< Try direct connection if socks4 server fails
	int m_resolv_id; ///< Resolver id counter
	ResolvServer *m_resolver; ///< Resolver thread pointer
	port_t m_resolver_port; ///< Resolver listen port
	bool m_b_enable_pool; ///< Connection pool enabled if true
	socket_v m_fds; ///< Active file descriptor list
	socket_v m_fds_erase; ///< File descriptors that are to be erased from m_sockets
	socket_v m_fds_callonconnect; ///< checklist CallOnConnect
	socket_v m_fds_detach; ///< checklist Detach
	socket_v m_fds_connecting; ///< checklist Connecting
	socket_v m_fds_retry; ///< checklist retry client connect
	socket_v m_fds_close; ///< checklist close and delete
	Mutex& m_mutex; ///< Thread safety mutex
	bool m_b_use_mutex; ///< Mutex correctly initialized
};


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _SOCKETHANDLER_H
