/**
 **	File ......... Socket.h
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
#ifndef _SOCKETBASE_H
#define _SOCKETBASE_H

#include <string>
#include <vector>
#include <list>

#include "socket_include.h"
#include <time.h>

#ifndef _WIN32
#include <netinet/tcp.h>
#else
#define TCP_NODELAY 0x0001
#endif

class SocketHandler;
class SocketThread;

//	typedef std::list<std::string> string_v;

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
        virtual Socket *Create() { return NULL; }
/** Init: CTcpSocket uses this to create its ICrypt member variable.
 * The ICrypt member variable is created by a virtual method, therefore
 * it can't be called directly from the CTcpSocket constructor.
 * Also used to determine if incoming HTTP connection is normal (port 80)
 * or ssl (port 443).
 */
        virtual void Init();

        void Attach(SOCKET s);
        SOCKET GetSocket();
        virtual int Close();
        SOCKET CreateSocket4(int type,const std::string& protocol = "");
        SOCKET CreateSocket6(int type,const std::string& protocol = "");
        void Set(bool bRead,bool bWrite,bool bException = true);
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
/** Used internally by SSLSocket. */
        virtual void OnSSLInitDone();
/** Called on connect timeout (5s). */
        virtual void OnConnectFailed();
/** Called when a socket is created, to set socket options. */
        virtual void OnOptions(int family,int type,int protocol,SOCKET);
/** Socks4 client support internal use. @see TcpSocket */
        virtual void OnSocks4Connect();
/** Socks4 client support internal use. @see TcpSocket */
        virtual void OnSocks4ConnectFailed();
/** Socks4 client support internal use. @see TcpSocket */
        virtual bool OnSocks4Read();
/** Called when the last write caused the tcp output buffer to
 * become empty. */
//	virtual void OnWriteComplete();
/** SSL client/server support - internal use. @see TcpSocket */
        virtual void OnSSLConnect() {}
/** SSL client/server support - internal use. @see TcpSocket */
        virtual void OnSSLAccept() {}

        virtual bool CheckConnect();
        virtual void ReadLine();
/** OLD SSL support. */
        virtual bool SSLCheckConnect();
/** new SSL support */
        virtual bool SSLNegotiate() { return false; }

        void SetSSLConnecting(bool = true);
        bool SSLConnecting();
/** Enable the OnLine callback. Do not create your own OnRead
 * callback when using this. */
        void SetLineProtocol(bool = true);
        bool LineProtocol();
        void SetDeleteByHandler(bool = true);
        bool DeleteByHandler();
        void SetCloseAndDelete(bool = true);
        bool CloseAndDelete();
        void SetConnecting(bool = true);
        bool Connecting();
        time_t GetConnectTime();

/** ipv4 and ipv6 */
        bool isip(const std::string&);
/** ipv4 */
        bool u2ip(const std::string&, ipaddr_t&);
/** ipv6 */
#ifdef IPPROTO_IPV6
        bool u2ip(const std::string&, struct in6_addr&);
#endif
/** ipv4 */
        void l2ip(const ipaddr_t,std::string& );
/** ipv6 */
#ifdef IPPROTO_IPV6
        void l2ip(const struct in6_addr&,std::string& ,bool mixed = false);
#endif

/** ipv4 and ipv6 */
        void SetRemoteAddress(struct sockaddr* sa,socklen_t);
        void GetRemoteSocketAddress(struct sockaddr& sa,socklen_t& sa_len);
/** ipv4 */
        ipaddr_t GetRemoteIP4();
/** ipv6 */
#ifdef IPPROTO_IPV6
        struct in6_addr GetRemoteIP6();
#endif
/** ipv4 and ipv6 */
        port_t GetRemotePort();
/** ipv4 and ipv6 */
        std::string GetRemoteAddress();
/** ipv4 and ipv6(not implemented) */
        std::string GetRemoteHostname();

        SocketHandler& Handler() const;
        bool SetNonblocking(bool);
        bool SetNonblocking(bool, SOCKET);

        time_t Uptime() { return time(NULL) - m_tCreate; }

/*
    void SetTimeout(time_t x) { m_timeout = x; }
    time_t Timeout() { return m_timeout; }
    void Touch() { m_tActive = time(NULL); }
    time_t Inactive() { return time(NULL) - m_tActive; }
*/
        virtual void OnDetached()                 // Threading
        {
        }
        void SetDetach(bool x = true) { m_detach = x; }
        bool IsDetach() { return m_detach; }
        void SetDetached(bool x = true) { m_detached = x; }
        bool IsDetached() { return m_detached; }
        bool Detach();

        void SetIpv6(bool x = true) { m_ipv6 = x; }
        bool IsIpv6() { return m_ipv6; }

/** Returns pointer to ListenSocket that created this instance
 * on an incoming connections. */
        Socket *GetParent();
/** Used by ListenSocket to set parent pointer of newly created
 * socket instance. */
        void SetParent(Socket *);
/** Get listening port from ListenSocket<>. */
        virtual port_t GetPort();

// pooling
        void SetIsClient() { m_bClient = true; }
        void SetSocketType(int x) { m_socket_type = x; }
        int GetSocketType() { return m_socket_type; }
        void SetSocketProtocol(const std::string& x) { m_socket_protocol = x; }
        const std::string& GetSocketProtocol() { return m_socket_protocol; }
        void SetClientRemoteAddr(ipaddr_t a) { m_client_remote_addr = a; }
        ipaddr_t& GetClientRemoteAddr() { return m_client_remote_addr; }
        void SetClientRemotePort(port_t p) { m_client_remote_port = p; }
        port_t GetClientRemotePort() { return m_client_remote_port; }
        void SetRetain() { if (m_bClient) m_bRetain = true; }
        bool Retain() { return m_bRetain; }
        void SetLost() { m_bLost = true; }
        bool Lost() { return m_bLost; }
        void SetCallOnConnect(bool x = true) { m_call_on_connect = x; }
        bool CallOnConnect() { return m_call_on_connect; }

// copy connection parameters from sock
        void CopyConnection(Socket *sock);

        void SetReuse(bool x) { m_opt_reuse = x; }
        void SetKeepalive(bool x) { m_opt_keepalive = x; }

// dns
        int Resolve(const std::string& host,port_t port);
        virtual void Resolved(int id,ipaddr_t,port_t);

/** socket still in socks4 negotiation mode */
        bool Socks4() { return m_bSocks4; }
        void SetSocks4(bool x = true) { m_bSocks4 = x; }

        void SetSocks4Host(ipaddr_t a) { m_socks4_host = a; }
        void SetSocks4Host(const std::string& );
        void SetSocks4Port(port_t p) { m_socks4_port = p; }
        void SetSocks4Userid(const std::string& x) { m_socks4_userid = x; }
        ipaddr_t GetSocks4Host() { return m_socks4_host; }
        port_t GetSocks4Port() { return m_socks4_port; }
        const std::string& GetSocks4Userid() { return m_socks4_userid; }

        void SetConnectTimeout(int x) { m_connect_timeout = x; }
        int GetConnectTimeout() { return m_connect_timeout; }

/** SSL Enabled */
        bool IsSSL() { return m_b_enable_ssl; }
        void EnableSSL(bool x = true) { m_b_enable_ssl = x; }
/** Still negotiating ssl connection */
        bool IsSSLNegotiate() { return m_b_ssl; }
        void SetSSLNegotiate(bool x = true) { m_b_ssl = x; }
/** OnAccept called with SSL Enabled */
        bool IsSSLServer() { return m_b_ssl_server; }
        void SetSSLServer(bool x = true) { m_b_ssl_server = x; }

        void DisableRead(bool x = true) { m_b_disable_read = x; }
        bool IsDisableRead() { return m_b_disable_read; }

    protected:
        Socket(const Socket& );                   // do not allow use of copy constructor
        void DetachSocket();                      // protected, friend class SocketHandler;

    private:
/** default constructor not available */
        Socket() : m_handler(Handler()) {}
#ifdef _WIN32
        static  WSAInitializer m_winsock_init;
#endif
        Socket& operator=(const Socket& ) { return *this; }
//
        SocketHandler& m_handler;
        SOCKET m_socket;
        bool m_bDel;
        bool m_bClose;
        bool m_bConnecting;
        time_t m_tConnect;
        time_t m_tCreate;
        bool m_line_protocol;
        bool m_ssl_connecting;
//	time_t m_tActive;
//	time_t m_timeout;
        bool m_detach;
        bool m_detached;
        SocketThread *m_pThread;
        bool m_ipv6;
        struct sockaddr m_sa;                     // remote, from accept
        socklen_t m_sa_len;
        Socket *m_parent;
// pooling, ipv4
        int m_socket_type;
        std::string m_socket_protocol;
        bool m_bClient;                           // only client connections are pooled
        ipaddr_t m_client_remote_addr;
        port_t m_client_remote_port;
        bool m_bRetain;                           // keep connection on close
        bool m_bLost;                             // connection lost
        bool m_call_on_connect;
        bool m_opt_reuse;
        bool m_opt_keepalive;
        bool m_bSocks4;                           // socks4 negotiation mode (TcpSocket)
        ipaddr_t m_socks4_host;
        port_t m_socks4_port;
        std::string m_socks4_userid;
        int m_connect_timeout;
        bool m_b_enable_ssl;
        bool m_b_ssl;                             // ssl negotiation mode (TcpSocket)
        bool m_b_ssl_server;
        bool m_b_disable_read;
};
#endif                                            // _SOCKETBASE_H
