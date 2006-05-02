/** \file Socket.h
 ** \date  2004-02-13
 ** \author grymse@alhem.net
**/
/*
Copyright (C) 2004,2005  Anders Hedstrom

This software is made available under the terms of the GNU GPL.

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
#ifndef _SOCKETBASE_H
#define _SOCKETBASE_H

#include <string>
#include <vector>
#include <list>

#include "socket_include.h"
#include <time.h>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


class SocketHandler;
class SocketThread;


//	typedef std::list<std::string> string_v;

/** \defgroup basic Basic sockets */
/** Socket base class.
	\ingroup basic */
class Socket
{
	friend class SocketHandler;
public:
	/** "Default" constructor */
	Socket(SocketHandler&);
	virtual ~Socket();

	/** Socket class instantiation method. Used when a "non-standard" constructor
	 * needs to be used for the socket class. Note: the socket class still needs
	 * the "default" constructor with one SocketHandler& as input parameter.
	 */
	virtual Socket *Create();

	/** CTcpSocket uses this to create its ICrypt member variable.
	 * The ICrypt member variable is created by a virtual method, therefore
	 * it can't be called directly from the CTcpSocket constructor.
	 * Also used to determine if incoming HTTP connection is normal (port 80)
	 * or ssl (port 443).
	 */
	virtual void Init();

	/** Assign this socket a file descriptor created
		by a call to socket() or otherwise. */
	void Attach(SOCKET s);
	/** Return file descriptor assigned to this socket. */
	SOCKET GetSocket();
	/** Close connection immediately - internal use.
		\sa SetCloseAndDelete */
	virtual int Close();
	/** Create a socket file descriptor.
		\param af Address family AF_INET / AF_INET6 / ...
		\param type SOCK_STREAM / SOCK_DGRAM / ...
		\param protocol "tcp" / "udp" / ... */
	SOCKET CreateSocket(int af,int type,const std::string& protocol = "");
	/** Add file descriptor to sockethandler fd_set's. */
	void Set(bool bRead,bool bWrite,bool bException = true);
	/** Returns true when socket file descriptor is valid,
		socket connection is established, and socket is not about to
		be closed. */
	bool Ready();

	/** Called when there is something to be read from the file descriptor. */
	virtual void OnRead();
	/** Called when there is room for another write on the file descriptor. */
	virtual void OnWrite();
	/** Called on socket exception. */
	virtual void OnException();
	/** Called before a socket class is deleted by the SocketHandler. */
	virtual void OnDelete();
	/** Called when a connection has completed. */
	virtual void OnConnect();
	/** Called when an incoming connection has been completed. */
	virtual void OnAccept();
	/** Called when a complete line has been read and the socket is in
	 * line protocol mode. */
	virtual void OnLine(const std::string& );
	/** Called on connect timeout (5s). */
	virtual void OnConnectFailed();
	/** Called when a socket is created, to set socket options. */
	virtual void OnOptions(int family,int type,int protocol,SOCKET);
	/** Socks4 client support internal use. \sa TcpSocket */
	virtual void OnSocks4Connect();
	/** Socks4 client support internal use. \sa TcpSocket */
	virtual void OnSocks4ConnectFailed();
	/** Socks4 client support internal use. \sa TcpSocket */
	virtual bool OnSocks4Read();
	/** Called when the last write caused the tcp output buffer to
	 * become empty. */
//	virtual void OnWriteComplete();
	/** SSL client/server support - internal use. \sa TcpSocket */
	virtual void OnSSLConnect();
	/** SSL client/server support - internal use. \sa TcpSocket */
	virtual void OnSSLAccept();
	/** Connection retry callback - return false to abort connection attempts */
	virtual bool OnConnectRetry();
	/** a reconnect has been made */
	virtual void OnReconnect();
	/** SSL negotiation failed for client connect. */
	virtual void OnSSLConnectFailed();
	/** SSL negotiation failed for server accept. */
	virtual void OnSSLAcceptFailed();
	/** When a socket is set to reconnect, and a disconnect has been detected. */
	virtual void OnDisconnect();

	/** Check whether a connection has been established. */
	virtual bool CheckConnect();
	/** Called after OnRead if socket is in line protocol mode.
		\sa SetLineProtocol */
	virtual void ReadLine();
	/** new SSL support */
	virtual bool SSLNegotiate();

	/** Enable the OnLine callback. Do not create your own OnRead
	 * callback when using this. */
	void SetLineProtocol(bool = true);
	/** Check line protocol mode.
		\return true if socket is in line protocol mode */
	bool LineProtocol();
	/** Set delete by handler true when you want the sockethandler to
		delete the socket instance after use. */
	void SetDeleteByHandler(bool = true);
	/** Check delete by handler flag.
		\return true if this instance should be deleted by the sockethandler */
	bool DeleteByHandler();
	/** Set close and delete to terminate the connection. */
	void SetCloseAndDelete(bool = true);
	/** Check close and delete flag.
		\return true if this socket should be closed and the instance removed */
	bool CloseAndDelete();
	/** Socket should call CheckConnect on next write event from select(). */
	void SetConnecting(bool = true);
	/** Check connecting flag.
		\return true if the socket is still trying to connect */
	bool Connecting();
	/** Number of seconds the socket has been connected. */
	time_t GetConnectTime();

	/** Used by ListenSocket. ipv4 and ipv6 */
	void SetRemoteAddress(struct sockaddr* sa,socklen_t);
	/** Returns address of remote end. */
	void GetRemoteSocketAddress(struct sockaddr& sa,socklen_t& sa_len);
	/** Returns address of remote end: ipv4. */
	ipaddr_t GetRemoteIP4();
	/** Returns address of remote end: ipv6. */
#ifdef IPPROTO_IPV6
	struct in6_addr GetRemoteIP6();
#endif
	/** Returns remote port number: ipv4 and ipv6. */
	port_t GetRemotePort();
	/** Returns remote ip as string? ipv4 and ipv6. */
	std::string GetRemoteAddress();
	/** ipv4 and ipv6(not implemented) */
	std::string GetRemoteHostname();

	/** Returns reference to sockethandler that owns the socket. 
	If the socket is detached, this is a reference to the slave sockethandler.
	*/
	SocketHandler& Handler() const;
	/** Returns reference to sockethandler that owns the socket. 
	This one always returns the reference to the original sockethandler,
	even if the socket is detached.
	*/
	SocketHandler& MasterHandler() const;
	/** Set socket non-block operation. */
	bool SetNonblocking(bool);
	/** Set socket non-block operation. */
	bool SetNonblocking(bool, SOCKET);

	/** Total lifetime of instance. */
	time_t Uptime();
/*
	void SetTimeout(time_t x) { m_timeout = x; }
	time_t Timeout() { return m_timeout; }
	void Touch() { m_tActive = time(NULL); }
	time_t Inactive() { return time(NULL) - m_tActive; }
*/
	/** Callback fires when a new socket thread has started and this
		socket is ready for operation again.
		\sa ResolvSocket */
	virtual void OnDetached();
	/** Internal use. */
	void SetDetach(bool x = true);
	/** Check detach flag.
		\return true if the socket should detach to its own thread */
	bool IsDetach();
	/** Internal use. */
	void SetDetached(bool x = true);
	/** Check detached flag.
		\return true if the socket runs in its own thread. */
	const bool IsDetached() const;
	/** Order this socket to start its own thread and call OnDetached
		when ready for operation. */
	bool Detach();

	/** Enable ipv6 for this socket. */
	void SetIpv6(bool x = true);
	/** Check ipv6 socket.
		\return true if this is an ipv6 socket */
	bool IsIpv6();

	/** Returns pointer to ListenSocket that created this instance
	 * on an incoming connection. */
	Socket *GetParent();
	/** Used by ListenSocket to set parent pointer of newly created
	 * socket instance. */
	void SetParent(Socket *);
	/** Get listening port from ListenSocket<>. */
	virtual port_t GetPort();

	// pooling
	/** Client = connecting TcpSocket. */
	void SetIsClient();
	/** Socket type from socket() call. */
	void SetSocketType(int x);
	/** Socket type from socket() call. */
	int GetSocketType();
	/** Protocol type from socket() call. */
	void SetSocketProtocol(const std::string& x);
	/** Protocol type from socket() call. */
	const std::string& GetSocketProtocol();
	/** Set address of last connect() call. */
	void SetClientRemoteAddr(ipaddr_t a);
#ifdef IPPROTO_IPV6
	/** IPV6: Set address of last connect() call. */
	void SetClientRemoteAddr(in6_addr a);
	/** IPV6: Returns address of last connect() call. */
	in6_addr& GetClientRemoteAddr6();
#endif
	/** Returns address of last connect() call. */
	ipaddr_t& GetClientRemoteAddr();
	/** Set port of last connect() call. */
	void SetClientRemotePort(port_t p);
	/** Returns port number of last connect() call. */
	port_t GetClientRemotePort();
	/** Instruct a client socket to stay open in the connection pool after use.
		If you have connected to a server using tcp, you can call SetRetain
		to leave the connection open after your socket instance has been deleted.
		The next connection you make to the same server will reuse the already
		opened connection, if it is still available.
	*/
	void SetRetain();
	/** Check retain flag.
		\return true if the socket should be moved to connection pool after use */
	bool Retain();
	/** Connection lost - error while reading/writing from a socket. */
	void SetLost();
	/** Check connection lost status flag.
		\return true if there was an error while r/w causing the socket to close */
	bool Lost();
	/** Instruct socket to call OnConnect callback next sockethandler cycle. */
	void SetCallOnConnect(bool x = true);
	/** Check call on connect flag.
		\return true if OnConnect() should be called a.s.a.p */
	bool CallOnConnect();

	/** Copy connection parameters from sock. */
	void CopyConnection(Socket *sock);

	/** Socket option SO_REUSEADDR.
		\sa OnOptions */
	void SetReuse(bool x);
	/** Socket option SO_KEEPALIVE.
		\sa OnOptions */
	void SetKeepalive(bool x);

	/** Request an asynchronous dns resolution.
		\param host hostname to be resolved
		\param port port number passed along for the ride
		\return Resolve ID */
	int Resolve(const std::string& host,port_t port);
	/** Callback returning a resolved address.
		\param id Resolve ID from Resolve call
		\param a resolved ip address
		\param port port number passed to Resolve */
	virtual void OnResolved(int id,ipaddr_t a,port_t port);

	/** socket still in socks4 negotiation mode */
	bool Socks4();
	/** Set flag indicating Socks4 handshaking in progress */
	void SetSocks4(bool x = true);

	/** Set socks4 server host address to use */
	void SetSocks4Host(ipaddr_t a);
	/** Set socks4 server hostname to use. */
	void SetSocks4Host(const std::string& );
	/** Socks4 server port to use. */
	void SetSocks4Port(port_t p);
	/** Provide a socks4 userid if required by the socks4 server. */
	void SetSocks4Userid(const std::string& x);
	/** Get the ip address of socks4 server to use.
		\return socks4 server host address */
	ipaddr_t GetSocks4Host();
	/** Get the socks4 server port to use.
		\return socks4 server port */
	port_t GetSocks4Port();
	/** Get socks4 userid.
		\return Socks4 userid */
	const std::string& GetSocks4Userid();

	/** Set timeout to use for connection attempt.
		\param x Timeout in seconds */
	void SetConnectTimeout(int x);
	/** Return number of seconds to wait for a connection.
		\return Connection timeout (seconds) */
	int GetConnectTimeout();

	/** Check if SSL is Enabled for this TcpSocket.
		\return true if this is a TcpSocket with SSL enabled */
	bool IsSSL();
	/** Enable SSL operation for a TcpSocket. */
	void EnableSSL(bool x = true);
	/** Still negotiating ssl connection.
		\return true if ssl negotiating is still in progress */
	bool IsSSLNegotiate();
	/** Set flag indicating ssl handshaking still in progress. */
	void SetSSLNegotiate(bool x = true);
	/** OnAccept called with SSL Enabled.
		\return true if this is a TcpSocket with an incoming SSL connection */
	bool IsSSLServer();
	/** Set flag indicating that this is a TcpSocket with incoming SSL connection. */
	void SetSSLServer(bool x = true);

	/** Ignore read events for an output only socket. */
	void DisableRead(bool x = true);
	/** Check ignore read events flag.
		\return true if read events should be ignored */
	bool IsDisableRead();

	/** Set flag to initiate a connection attempt after a connection timeout. */
	void SetRetryClientConnect(bool x = true);
	/** Check if a connection attempt should be made.
		\return true when another attempt should be made */
	bool RetryClientConnect();

	/** Common interface for SendBuf used by Tcp and Udp sockets. */
	virtual void SendBuf(const char *,size_t,int = 0);
	/** Common interface for Send used by Tcp and Udp sockets. */
	virtual void Send(const std::string&,int = 0);

	/** Set connected status. */
	void SetConnected(bool = true);
	/** Check connected status.
		\return true if connected */
	bool IsConnected();

	/** Set flush before close to make a tcp socket completely empty its
		output buffer before closing the connection. */
	void SetFlushBeforeClose(bool = true);
	/** Check flush before status.
		\return true if the socket should send all data before closing */
	bool GetFlushBeforeClose();

	/** Define number of connection retries (tcp only).
	    n = 0 - no retry
	    n > 0 - number of retries
	    n = -1 - unlimited retries */
	void SetConnectionRetry(int n);
	/** Get number of maximum connection retries (tcp only). */
	int GetConnectionRetry();
	/** Increase number of actual connection retries (tcp only). */
	void IncreaseConnectionRetries();
	/** Get number of actual connection retries (tcp only). */
	int GetConnectionRetries();
	/** Reset actual connection retries (tcp only). */
	void ResetConnectionRetries();
	/** Set flag indicating the socket is being actively deleted by the sockethandler. */
	void SetErasedByHandler(bool x = true);
	/** Get value of flag indicating socket is deleted by sockethandler. */
	bool ErasedByHandler();

	/** Store the slave sockethandler pointer. */
	void SetSlaveHandler(SocketHandler *);

	/** Return number of seconds since socket was ordered to close. */
	time_t TimeSinceClose();

	/** Set shutdown status. */
	void SetShutdown(int);
	/** Get shutdown status. */
	int GetShutdown();

protected:
	Socket(const Socket& ); ///< do not allow use of copy constructor
	/** Create new thread for this socket to run detached in. */
	void DetachSocket();

private:
	/** default constructor not available */
	Socket() : m_handler(Handler()) {}
#ifdef _WIN32
static	WSAInitializer m_winsock_init; ///< Winsock initialization singleton class
#endif
	/** assignment operator not available. */
	Socket& operator=(const Socket& ) { return *this; }
	//
	void AddList(socket_v&, bool, const std::string& src = ""); ///< Add file descriptor to specified checklist
	SocketHandler& m_handler; ///< Reference of SocketHandler in control of this socket
	SOCKET m_socket; ///< File descriptor
	bool m_bDel; ///< Delete by handler flag
	bool m_bClose; ///< Close and delete flag
	bool m_bConnecting; ///< Flag indicating connection in progress
	time_t m_tConnect; ///< Time in seconds when connection was established
	time_t m_tCreate; ///< Time in seconds when this socket was created
	bool m_line_protocol; ///< Line protocol mode flag
	bool m_ssl_connecting; ///< OLD ssl connection flag
//	time_t m_tActive;
//	time_t m_timeout;
	bool m_detach; ///< Socket ordered to detach flag
	bool m_detached; ///< Socket has been detached
	SocketThread *m_pThread; ///< Detach socket thread class pointer
	bool m_ipv6; ///< This is an ipv6 socket if this one is true
	struct sockaddr m_sa; ///< remote address, from accept() call
	socklen_t m_sa_len; ///< Length of m_sa remote address
	Socket *m_parent; ///< Pointer to ListenSocket class, valid for incoming sockets
	// pooling, ipv4
	int m_socket_type; ///< Type of socket, from socket() call
	std::string m_socket_protocol; ///< Protocol, from socket() call
	bool m_bClient; ///< only client connections are pooled
	ipaddr_t m_client_remote_addr; ///< Address used by connect()
	port_t m_client_remote_port; ///< Port number used by connect()
	bool m_bRetain; ///< keep connection on close
	bool m_bLost; ///< connection lost
	bool m_call_on_connect; ///< OnConnect will be called next SocketHandler cycle if true
	bool m_opt_reuse; ///< Socket option reuseaddr
	bool m_opt_keepalive; ///< Socket option keep-alive
	bool m_bSocks4; ///< socks4 negotiation mode (TcpSocket)
	ipaddr_t m_socks4_host; ///< socks4 server address
	port_t m_socks4_port; ///< socks4 server port number
	std::string m_socks4_userid; ///< socks4 server usedid
	int m_connect_timeout; ///< Connection timeout (seconds)
	bool m_b_enable_ssl; ///< Enable SSL for this TcpSocket
	bool m_b_ssl; ///< ssl negotiation mode (TcpSocket)
	bool m_b_ssl_server; ///< True if this is an incoming ssl TcpSocket connection
	bool m_b_disable_read; ///< Disable checking for read events
	bool m_b_retry_connect; ///< Try another connection attempt next SocketHandler cycle
#ifdef IPPROTO_IPV6
	in6_addr m_client_remote_addr6; ///< Address used by connect() ipv6
#endif
	bool m_connected; ///< Socket is connected (tcp/udp)
	bool m_flush_before_close; ///< Send all data before closing (default true)
	int m_connection_retry; ///< Maximum connection retries (tcp)
	int m_retries; ///< Actual number of connection retries (tcp)
	bool m_b_erased_by_handler; ///< Set by handler before delete
	SocketHandler *m_slave_handler; ///< Actual sockethandler while detached
	time_t m_tClose; ///< Time in seconds when ordered to close
	int m_shutdown; ///< Shutdown status
};

#ifdef SOCKETS_NAMESPACE
}
#endif


#endif // _SOCKETBASE_H
