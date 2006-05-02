/** \file TcpSocket.h
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
#ifndef _TCPSOCKET_H
#define _TCPSOCKET_H

#include "Socket.h"
#include "CircularBuffer.h"
#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#else // !HAVE_OPENSSL
typedef void * BIO;
typedef void * SSL;
typedef void * SSL_CTX;
typedef void * SSL_METHOD;
#endif

#define TCP_BUFSIZE_READ 16400

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


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
	/** Removes ssl .rnd file */
	class SSLInitializer {
	public:
		SSLInitializer() {
		}
		~SSLInitializer() {
#ifdef HAVE_OPENSSL
			TcpSocket::DeleteRandFile();
#endif
		}
	};
public:
	/** Contructor with standard values on input/output buffers. */
	TcpSocket(SocketHandler& );
	/** Constructor with custom values for i/o buffer. 
		\param h SocketHandler reference
		\param isize Input buffer size
		\param osize Output buffer size */
	TcpSocket(SocketHandler& h,size_t isize,size_t osize);
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
#ifdef IPPROTO_IPV6
	/** Open connection. 
		\param ip Ipv6 address
		\param port Port number
		\param skip_socks Do not use socks4 even if configured */
	bool Open(in6_addr ip,port_t port,bool skip_socks = false);
#endif
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
	void Sendf(char *format, ...);
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

	/** Callback used when socket is in line protocol mode.
		\sa SetLineProtocol */
	void ReadLine();
	/** Callback fires when a socket in line protocol has read one full line. 
		\param line Line read */
	void OnLine(const std::string& line);
	/** Get counter of number of bytes received. */
	unsigned long GetBytesReceived();
	/** Get counter of number of bytes sent. */
	unsigned long GetBytesSent();

	/** Socks4 specific callback. */
	void OnSocks4Connect();
	/** Socks4 specific callback. */
	void OnSocks4ConnectFailed();
	/** Socks4 specific callback.
		\return 'need_more' */
	bool OnSocks4Read();

	/** Callback executed when resolver thread has finished a resolve request. */
	void OnResolved(int id,ipaddr_t a,port_t port);

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

	/** Flag that says a broken connection will try to reconnect. */
	void SetReconnect(bool = true);
	/** Check reconnect on lost connection flag status. */
	bool Reconnect();
	/** Flag to determine if a reconnect is in progress. */
	void SetIsReconnect(bool x = true);
	/** Socket is reconnecting. */
	bool IsReconnect();

	/** SSL; Get ssl password. */
	const std::string& GetPassword();
	/** SSL; Set random filename + size to be used. */
	void SetRandFile(const std::string& file,size_t size);
	/** SSL; delete random file when shutting down. */
static	void DeleteRandFile();

protected:
	TcpSocket(const TcpSocket& s);
	void OnRead();
	void OnWrite();
	/** SSL; Initialize ssl context for a client socket. 
		\param meth_in SSL method */
	void InitializeContext(SSL_METHOD *meth_in = NULL);
	/** SSL; Initialize ssl context for a server socket. 
		\param keyfile Combined private key/certificate file 
		\param password Password for private key 
		\param meth_in SSL method */
	void InitializeContext(const std::string& keyfile,const std::string& password,SSL_METHOD *meth_in = NULL);
	/** SSL; Password callback method. */
static	int password_cb(char *buf,int num,int rwflag,void *userdata);
	/** SSL; Get pointer to ssl context structure. */
	virtual SSL_CTX *GetSslContext();
	/** SSL; Get pointer to ssl structure. */
	virtual SSL *GetSsl();
	/** ssl; still negotiating connection. */
	bool SSLNegotiate();
	//
	CircularBuffer ibuf; ///< Circular input buffer
	CircularBuffer obuf; ///< Circular output buffer
	std::string m_line; ///< Current line in line protocol mode
	ucharp_v m_mes; ///< overflow protection, dynamic output buffer

private:
	TcpSocket& operator=(const TcpSocket& ) { return *this; }
	int m_socks4_state; ///< socks4 support
	char m_socks4_vn; ///< socks4 support, temporary variable
	char m_socks4_cd; ///< socks4 support, temporary variable
	unsigned short m_socks4_dstport; ///< socks4 support
	unsigned long m_socks4_dstip; ///< socks4 support
	int m_resolver_id; ///< Resolver id (if any) for current Open call
	// SSL
	SSL_CTX *m_context; ///< ssl context
	SSL *m_ssl; ///< ssl 'socket'
	BIO *m_sbio; ///< ssl bio
static	BIO *bio_err; ///< ssl bio err
	std::string m_password; ///< ssl password
static	bool m_b_rand_file_generated; ///< rand_file is generated once
static	std::string m_rand_file;
static	size_t m_rand_size;
	// state flags
	bool m_b_reconnect; ///< Reconnect on lost connection flag
	bool m_b_is_reconnect; ///< Trying to reconnect
static	SSLInitializer m_ssl_init;
};


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _TCPSOCKET_H
