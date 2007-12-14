/** \file TcpSocket.h
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
#ifndef _SOCKETS_TcpSocket_H
#define _SOCKETS_TcpSocket_H
#include "sockets-config.h"
#include "Socket.h"
#include "CircularBuffer.h"
#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#include "SSLInitializer.h"
#endif


#define TCP_BUFSIZE_READ 16400


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

class SocketAddress;


/** Socket implementation for TCP. 
	\ingroup basic */
class TcpSocket : public Socket
{
	/** Dynamic output buffer storage struct. 
		\ingroup internal */
	struct MES {
		MES( const char *buf_in,size_t len_in)
		:buf(new  char[len_in])
		,len(len_in)
		,ptr(0)
		{
			memcpy(buf,buf_in,len);
		}
		~MES() { delete[] buf; }
		size_t left() { return len - ptr; }
		 char *curbuf() { return buf + ptr; }
		 char *buf;
		size_t len;
		size_t ptr;
	};
	/** Dynamic output buffer list. */
	typedef std::list<MES *> ucharp_v;
public:
	/** Constructor with standard values on input/output buffers. */
	TcpSocket(ISocketHandler& );
	/** Constructor with custom values for i/o buffer. 
		\param h ISocketHandler reference
		\param isize Input buffer size
		\param osize Output buffer size */
	TcpSocket(ISocketHandler& h,size_t isize,size_t osize);
	~TcpSocket();

	/** Open a connection to a remote server.
	    If you want your socket to connect to a server, 
	    always call Open before Add'ing a socket to the sockethandler.
	    If not, the connection attempt will not be monitored by the
	    socket handler... 
		\param ip IP address
		\param port Port number
		\param skip_socks Do not use socks4 even if configured */
	bool Open(ipaddr_t ip,port_t port,bool skip_socks = false);
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
	/** Open connection. 
		\param ip Ipv6 address
		\param port Port number
		\param skip_socks Do not use socks4 even if configured */
	bool Open(in6_addr ip,port_t port,bool skip_socks = false);
#endif
#endif
	bool Open(SocketAddress&,bool skip_socks = false);
	bool Open(SocketAddress&,SocketAddress& bind_address,bool skip_socks = false);
	/** Open connection. 
		\param host Hostname
		\param port Port number */
	bool Open(const std::string &host,port_t port);
	/** Close file descriptor - internal use only. 
		\sa SetCloseAndDelete */
	int Close();

	/** Send a string. 
		\param s String to send 
		\param f Dummy flags -- not used */
	void Send(const std::string &s,int f = 0);
	/** Send string using printf formatting. */
	void Sendf(const char *format, ...);
	/** Send buffer of bytes.
		\param buf Buffer pointer
		\param len Length of data
		\param f Dummy flags -- not used */
	void SendBuf(const char *buf,size_t len,int f = 0);
	/** This callback is executed after a successful read from the socket.
		\param buf Pointer to the data
		\param len Length of the data */
	virtual void OnRawData(const char *buf,size_t len);

	/** Number of bytes in input buffer. */
	size_t GetInputLength();
	/** Number of bytes in output buffer. */
	size_t GetOutputLength();

	/** Callback fires when a socket in line protocol has read one full line. 
		\param line Line read */
	void OnLine(const std::string& line);
	/** Get counter of number of bytes received. */
	uint64_t GetBytesReceived(bool clear = false);
	/** Get counter of number of bytes sent. */
	uint64_t GetBytesSent(bool clear = false);

	/** Socks4 specific callback. */
	void OnSocks4Connect();
	/** Socks4 specific callback. */
	void OnSocks4ConnectFailed();
	/** Socks4 specific callback.
		\return 'need_more' */
	bool OnSocks4Read();

#ifdef ENABLE_RESOLVER
	/** Callback executed when resolver thread has finished a resolve request. */
	void OnResolved(int id,ipaddr_t a,port_t port);
#ifdef ENABLE_IPV6
	void OnResolved(int id,in6_addr& a,port_t port);
#endif
#endif
#ifdef HAVE_OPENSSL
	/** Callback for 'New' ssl support - replaces SSLSocket. Internal use. */
	void OnSSLConnect();
	/** Callback for 'New' ssl support - replaces SSLSocket. Internal use. */
	void OnSSLAccept();
	/** This method must be implemented to initialize 
		the ssl context for an outgoing connection. */
	virtual void InitSSLClient();
	/** This method must be implemented to initialize 
		the ssl context for an incoming connection. */
	virtual void InitSSLServer();
#endif

#ifdef ENABLE_RECONNECT
	/** Flag that says a broken connection will try to reconnect. */
	void SetReconnect(bool = true);
	/** Check reconnect on lost connection flag status. */
	bool Reconnect();
	/** Flag to determine if a reconnect is in progress. */
	void SetIsReconnect(bool x = true);
	/** Socket is reconnecting. */
	bool IsReconnect();
#endif

	void DisableInputBuffer(bool = true);

	void OnOptions(int,int,int,SOCKET);

	void SetLineProtocol(bool = true);

protected:
	TcpSocket(const TcpSocket& s);
	void OnRead();
	void OnWrite();
#ifdef HAVE_OPENSSL
	/** SSL; Initialize ssl context for a client socket. 
		\param meth_in SSL method */
	void InitializeContext(const std::string& context, SSL_METHOD *meth_in = NULL);
	/** SSL; Initialize ssl context for a server socket. 
		\param keyfile Combined private key/certificate file 
		\param password Password for private key 
		\param meth_in SSL method */
	void InitializeContext(const std::string& context, const std::string& keyfile, const std::string& password, SSL_METHOD *meth_in = NULL);
	/** SSL; Password callback method. */
static	int SSL_password_cb(char *buf,int num,int rwflag,void *userdata);
	/** SSL; Get pointer to ssl context structure. */
	virtual SSL_CTX *GetSslContext();
	/** SSL; Get pointer to ssl structure. */
	virtual SSL *GetSsl();
	/** ssl; still negotiating connection. */
	bool SSLNegotiate();
	/** SSL; Get ssl password. */
	const std::string& GetPassword();
#endif

	CircularBuffer ibuf; ///< Circular input buffer
	CircularBuffer obuf; ///< Circular output buffer

private:
	TcpSocket& operator=(const TcpSocket& ) { return *this; }
	bool m_b_input_buffer_disabled;
	uint64_t m_bytes_sent;
	uint64_t m_bytes_received;
	bool m_skip_c; ///< Skip second char of CRLF or LFCR sequence in OnRead
	char m_c; ///< First char in CRLF or LFCR sequence
	std::string m_line; ///< Current line in line protocol mode
	ucharp_v m_mes; ///< overflow protection, dynamic output buffer
#ifdef SOCKETS_DYNAMIC_TEMP
	char *m_buf; ///< temporary read buffer
#endif

#ifdef HAVE_OPENSSL
static	SSLInitializer m_ssl_init;
	SSL_CTX *m_ssl_ctx; ///< ssl context
	SSL *m_ssl; ///< ssl 'socket'
	BIO *m_sbio; ///< ssl bio
	std::string m_password; ///< ssl password
#endif

#ifdef ENABLE_SOCKS4
	int m_socks4_state; ///< socks4 support
	char m_socks4_vn; ///< socks4 support, temporary variable
	char m_socks4_cd; ///< socks4 support, temporary variable
	unsigned short m_socks4_dstport; ///< socks4 support
	unsigned long m_socks4_dstip; ///< socks4 support
#endif

#ifdef ENABLE_RESOLVER
	int m_resolver_id; ///< Resolver id (if any) for current Open call
#endif

#ifdef ENABLE_RECONNECT
	bool m_b_reconnect; ///< Reconnect on lost connection flag
	bool m_b_is_reconnect; ///< Trying to reconnect
#endif

};


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _SOCKETS_TcpSocket_H
