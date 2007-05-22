/** \file ISocketHandler.h
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
#ifndef _SOCKETS_ISocketHandler_H
#define _SOCKETS_ISocketHandler_H
#include "sockets-config.h"

#include <map>
#include <list>

#include "socket_include.h"
#include "StdLog.h"
#include "Mutex.h"
#include "Socket.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

typedef enum {
	LIST_CALLONCONNECT = 0,
#ifdef ENABLE_DETACH
	LIST_DETACH,
#endif
	LIST_CONNECTING,
	LIST_RETRY,
	LIST_CLOSE
} list_t;

class SocketAddress;


/** Socket container class, event generator. 
	\ingroup basic */
class ISocketHandler
{
	friend class Socket;

public:
	/** Connection pool class for internal use by the ISocketHandler. 
		\ingroup internal */
#ifdef ENABLE_POOL
	class PoolSocket : public Socket
	{
	public:
		PoolSocket(ISocketHandler& h,Socket *src) : Socket(h) {
			CopyConnection( src );
			SetIsClient();
		}

		void OnRead() {
			Handler().LogError(this, "OnRead", 0, "data on hibernating socket", LOG_LEVEL_FATAL);
			SetCloseAndDelete();
		}
		void OnOptions(int,int,int,SOCKET) {}

	};
#endif

public:
	/** ISocketHandler constructor.
		\param log Optional log class pointer */
	ISocketHandler(StdLog *log);
	/** ISocketHandler threadsafe constructor.
		\param mutex Externally declared mutex variable
		\param log Optional log class pointer */
	ISocketHandler(Mutex& mutex,StdLog *log);
	virtual ~ISocketHandler();

	/** Get mutex reference for threadsafe operations. */
	Mutex& GetMutex() const;

#ifdef ENABLE_DETACH
	/** Indicates that the handler runs under SocketThread. */
	void SetSlave(bool x = true);
	/** Indicates that the handler runs under SocketThread. */
	bool IsSlave();
#endif

	/** Register StdLog object for error callback. 
		\param log Pointer to log class */
	void RegStdLog(StdLog *log);
	/** Log error to log class for print out / storage. */
	void LogError(Socket *p,const std::string& user_text,int err,const std::string& sys_err,loglevel_t t = LOG_LEVEL_WARNING);

	// -------------------------------------------------------------------------
	// Socket stuff
	// -------------------------------------------------------------------------
	/** Add socket instance to socket map. Removal is always automatic. */
	virtual void Add(Socket *) = 0;
private:
	/** Remove socket from socket map, used by Socket class. */
	virtual void Remove(Socket *) = 0;
public:
	/** Get status of read/write/exception file descriptor set for a socket. */
	virtual void Get(SOCKET s,bool& r,bool& w,bool& e) = 0;
	/** Set read/write/exception file descriptor sets (fd_set). */
	virtual void Set(SOCKET s,bool bRead,bool bWrite,bool bException = true) = 0;

	/** Wait for events, generate callbacks. */
	virtual int Select(long sec,long usec) = 0;
	/** This method will not return until an event has been detected. */
	virtual int Select() = 0;
	/** Wait for events, generate callbacks. */
	virtual int Select(struct timeval *tsel) = 0;

	/** Check that a socket really is handled by this socket handler. */
	virtual bool Valid(Socket *) = 0;
	/** Return number of sockets handled by this handler.  */
	virtual size_t GetCount() = 0;

	/** Override and return false to deny all incoming connections. 
		\param p ListenSocket class pointer (use GetPort to identify which one) */
	virtual bool OkToAccept(Socket *p) = 0;

	/** Called by Socket when a socket changes state. */
	virtual void AddList(SOCKET s,list_t which_one,bool add) = 0;

	// -------------------------------------------------------------------------
	// Connection pool
	// -------------------------------------------------------------------------
#ifdef ENABLE_POOL
	/** Find available open connection (used by connection pool). */
	virtual ISocketHandler::PoolSocket *FindConnection(int type,const std::string& protocol,SocketAddress&) {
		return NULL;
	}
	/** Enable connection pool (by default disabled). */
	virtual void EnablePool(bool = true) {}
	/** Check pool status. 
		\return true if connection pool is enabled */
	virtual bool PoolEnabled() {
		return false;
	}
#endif // ENABLE_POOL

	// -------------------------------------------------------------------------
	// Socks4
	// -------------------------------------------------------------------------
#ifdef ENABLE_SOCKS4
	/** Set socks4 server ip that all new tcp sockets should use. */
	virtual void SetSocks4Host(ipaddr_t) {}
	/** Set socks4 server hostname that all new tcp sockets should use. */
	virtual void SetSocks4Host(const std::string& ) {}
	/** Set socks4 server port number that all new tcp sockets should use. */
	virtual void SetSocks4Port(port_t) {};
	/** Set optional socks4 userid. */
	virtual void SetSocks4Userid(const std::string& ) {}
	/** If connection to socks4 server fails, immediately try direct connection to final host. */
	virtual void SetSocks4TryDirect(bool = true) {}
	/** Get socks4 server ip. 
		\return socks4 server ip */
	virtual ipaddr_t GetSocks4Host() {
		return (ipaddr_t)0;
	}
	/** Get socks4 port number.
		\return socks4 port number */
	virtual port_t GetSocks4Port() {
		return 0;
	}
	/** Get socks4 userid (optional).
		\return socks4 userid */
	virtual const std::string& GetSocks4Userid() = 0;
	/** Check status of socks4 try direct flag.
		\return true if direct connection should be tried if connection to socks4 server fails */
	virtual bool Socks4TryDirect() {
		return false;
	}
#endif // ENABLE_SOCKS4

	// -------------------------------------------------------------------------
	// DNS resolve server
	// -------------------------------------------------------------------------
#ifdef ENABLE_RESOLVER
	/** Enable asynchronous DNS. 
		\param port Listen port of asynchronous dns server */
	virtual void EnableResolver(port_t = 16667) {}
	/** Check resolver status.
		\return true if resolver is enabled */
	virtual bool ResolverEnabled() {
		return false;
	}
	/** Queue a dns request.
		\param host Hostname to be resolved
		\param port Port number will be echoed in Socket::OnResolved callback */
	virtual int Resolve(Socket *,const std::string& host,port_t port) {
		return -1;
	}
#ifdef ENABLE_IPV6
	virtual int Resolve6(Socket *,const std::string& host,port_t port) {
		return -1;
	}
#endif
	/** Do a reverse dns lookup. */
	virtual int Resolve(Socket *,ipaddr_t a) {
		return -1;
	}
#ifdef ENABLE_IPV6
	virtual int Resolve(Socket *,in6_addr& a) {
		return -1;
	}
#endif
	/** Get listen port of asynchronous dns server. */
	virtual port_t GetResolverPort() {
		return 0;
	}
	/** Resolver thread ready for queries. */
	virtual bool ResolverReady() {
		return false;
	}
#endif

protected:
	StdLog *m_stdlog; ///< Registered log class, or NULL
#ifdef ENABLE_DETACH
	bool m_slave; ///< Indicates that this is a ISocketHandler run in SocketThread
#endif
	Mutex& m_mutex; ///< Thread safety mutex
	bool m_b_use_mutex; ///< Mutex correctly initialized
};


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _SOCKETS_ISocketHandler_H
