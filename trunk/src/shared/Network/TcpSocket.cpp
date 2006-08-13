/**
 **	File ......... TcpSocket.cpp
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

#include "Platform/Define.h"

#ifdef _WIN32
#pragma warning(disable:4786)
#define strcasecmp stricmp
#include <stdlib.h>
#else
#include <errno.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <stdarg.h>
#ifdef HAVE_OPENSSL
#include <openssl/rand.h>
#endif

#include "SocketHandler.h"
#include "TcpSocket.h"
#include "PoolSocket.h"

#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x)
#endif

#ifdef HAVE_OPENSSL
BIO *TcpSocket::bio_err = NULL;
std::string TcpSocket::m_password;
#endif

// thanks, q
#ifdef _WIN32
#pragma warning(disable:4355)
#endif
TcpSocket::TcpSocket(SocketHandler& h) : Socket(h)
,ibuf(*this, TCP_BUFSIZE_READ)
//,obuf(*this, 32768)
,obuf(*this, 131070)
,m_line("")
,m_socks4_state(0)
,m_resolver_id(0)
#ifdef HAVE_OPENSSL
,m_context(NULL)
,m_ssl(NULL)
,m_sbio(NULL)
#endif
{
}


#ifdef _WIN32
#pragma warning(default:4355)
#endif

#ifdef _WIN32
#pragma warning(disable:4355)
#endif
TcpSocket::TcpSocket(SocketHandler& h,size_t isize,size_t osize) : Socket(h)
,ibuf(*this, isize)
,obuf(*this, osize)
,m_line("")
,m_socks4_state(0)
,m_resolver_id(0)
#ifdef HAVE_OPENSSL
,m_context(NULL)
,m_ssl(NULL)
,m_sbio(NULL)
#endif
{
}


#ifdef _WIN32
#pragma warning(default:4355)
#endif

#if COMPILER == COMPILER_MICROSOFT
#define vsnprintf _vsnprintf
#endif

TcpSocket::~TcpSocket()
{
#ifdef HAVE_OPENSSL
    if (m_ssl)
    {
        DEB(        printf("SSL_free()\n");)
            SSL_free(m_ssl);
    }
    if (m_context)
    {
        DEB(        printf("SSL_CTX_free()\n");)
            SSL_CTX_free(m_context);
    }
#endif
}


bool TcpSocket::Open(ipaddr_t ip,port_t port,bool skip_socks)
{
    SetConnecting(false);
    SetSocks4(false);
// check for pooling
    PoolSocket *pools = Handler().FindConnection(SOCK_STREAM, "tcp", ip, port);
    if (pools)
    {
        CopyConnection( pools );
        delete pools;

        SetIsClient();
        SetCallOnConnect();                       // SocketHandler must call OnConnect
        DEB(printf("Reusing connection\n");)
            return true;
    }
// if not, create new connection
    SOCKET s = CreateSocket4(SOCK_STREAM, "tcp");
    if (s == INVALID_SOCKET)
    {
        return false;
    }
// socket must be nonblocking for async connect
    if (!SetNonblocking(true, s))
    {
        closesocket(s);
        return false;
    }
    SetIsClient();                                // client because we connect
    SetClientRemoteAddr(ip);
    SetClientRemotePort(port);
    struct sockaddr_in sa;
// size of sockaddr struct
    socklen_t sa_len = sizeof(sa);
    if (!skip_socks && GetSocks4Host() && GetSocks4Port())
    {
        memset(&sa, 0, sa_len);
        sa.sin_family = AF_INET;
        sa.sin_port = htons(GetSocks4Port());
        ipaddr_t a = GetSocks4Host();
        memcpy(&sa.sin_addr, &a, 4);
        {
            char slask[100];
            sprintf(slask,"Connecting to socks4 server @ %08x:%d",GetSocks4Host(),GetSocks4Port());
            Handler().LogError(this, "Open", 0, slask, LOG_LEVEL_INFO);
        }
        SetSocks4();
    }
    else
    {
// setup sockaddr struct
        memset(&sa,0,sa_len);
        sa.sin_family = AF_INET;                  // hp -> h_addrtype;
        sa.sin_port = htons( port );
        memcpy(&sa.sin_addr,&ip,4);
    }
// try connect
    int n = connect(s, (struct sockaddr *)&sa, sa_len);
    if (n == -1)
    {
// check error code that means a connect is in progress
#ifdef _WIN32
        if (Errno == WSAEWOULDBLOCK)
#else
            if (Errno == EINPROGRESS)
#endif
        {
            SetConnecting( true );            // this flag will control fd_set's
        }
        else
// retry
        if (Socks4() && Handler().Socks4TryDirect() )
        {
            closesocket(s);
            return Open(ip, port, true);
        }
        else
        {
            Handler().LogError(this, "connect", Errno, StrError(Errno), LOG_LEVEL_FATAL);
            closesocket(s);
            return false;
        }
    }
    else
    {
        SetCallOnConnect();                       // SocketHandler must call OnConnect
    }
    SetRemoteAddress( (struct sockaddr *)&sa,sa_len);
    Attach(s);

// 'true' means connected or connecting(not yet connected)
// 'false' means something failed
    return true;                                  //!Connecting();
}


bool TcpSocket::Open(const std::string &host,port_t port)
{
    if (!Handler().ResolverEnabled() || isip(host) )
    {
        ipaddr_t l;
        if (!u2ip(host,l))
        {
            return false;
        }
        return Open(l, port);
    }
// resolve using async resolver thread
    m_resolver_id = Resolve(host, port);
    return true;
}


void TcpSocket::Resolved(int id,ipaddr_t a,port_t port)
{
    if (id == m_resolver_id)
    {
        if (a && port)
        {
            Open(a, port);
            if (!Handler().Valid(this))
            {
                Handler().Add(this);
            }
            return;
        }
        else
        {
            Handler().LogError(this, "Resolved", 0, "Resolver failed", LOG_LEVEL_FATAL);
        }
    }
    else
    {
        Handler().LogError(this, "Resolved", id, "Resolver returned wrong job id", LOG_LEVEL_FATAL);
    }
    SetCloseAndDelete();
}


// halfbaked IPV6 code
#ifdef IPPROTO_IPV6
bool TcpSocket::Open6(const std::string &host,port_t port)
{
// check for pooling

// if not, create new connection
    SOCKET s = CreateSocket6(SOCK_STREAM, "tcp");
    if (s == INVALID_SOCKET)
    {
        return false;
    }
    struct in6_addr a;
    if (u2ip(host,a))
    {
        struct sockaddr_in6 sa;
        socklen_t sa_len = sizeof(sa);

        memset(&sa,0,sizeof(sa));
        sa.sin6_family = AF_INET6;
        sa.sin6_port = htons( port );
        sa.sin6_flowinfo = 0;
        sa.sin6_scope_id = 0;
        sa.sin6_addr = a;

        if (!SetNonblocking(true, s))
        {
            closesocket(s);
            return false;
        }
        int n = connect(s, (struct sockaddr *)&sa, sa_len);
        if (n == -1)
        {
#ifdef _WIN32
            if (Errno != WSAEWOULDBLOCK)
#else
                if (Errno != EINPROGRESS)
#endif
            {
                Handler().LogError(this, "connect", Errno, StrError(Errno), LOG_LEVEL_FATAL);
                closesocket(s);
                return false;
            }
            else
            {
                SetConnecting(true);
            }
        }
        else
        {
            SetCallOnConnect();                   // SocketHandler must call OnConnect
        }
        SetRemoteAddress((struct sockaddr *)&sa,sa_len);
        Attach(s);
        return true;                              //!Connecting();
    }
    return false;                                 // u2ip failed
}
#endif

void TcpSocket::OnRead()
{
    if (IsSSL())
    {
#ifdef HAVE_OPENSSL
        DEB(        printf("TcpSocket(SSL)::OnRead()\n");)
            if (!Ready())
            return;
        char buf[TCP_BUFSIZE_READ];
        int n = SSL_read(m_ssl, buf, TCP_BUFSIZE_READ);
        if (n == -1)
        {
            n = SSL_get_error(m_ssl, n);
            switch (n)
            {
                case SSL_ERROR_NONE:
                    break;
                case SSL_ERROR_ZERO_RETURN:
                    DEB(                printf("SSL_read() returns zero - closing socket\n");)
                        SetCloseAndDelete(true);
                    break;
                default:
                    DEB(                printf("SSL read problem, errcode = %d\n",n);)
                        SetCloseAndDelete(true);  // %!
            }
        }
        else
        if (!n)
        {
            SetCloseAndDelete(true);
            DEB(            printf("read() returns 0\n");)
        }
        else
        {
            DEB(            printf("TcpSocket(SSL) OnRead read %d bytes\n",n);)
                if (!ibuf.Write(buf,n))
            {
// overflow
                DEB(                printf(" *** overflow ibuf Write\n");)
            }
        }
        return;
#endif                                    // HAVE_OPENSSL
    }
//    DEB(printf("TcpSocket::OnRead()\n");)
        int n = (int)ibuf.Space();
    char buf[TCP_BUFSIZE_READ];
//	if (!n)
//		return; // bad
    n = TCP_BUFSIZE_READ;                         // %! patch away
    n = recv(GetSocket(),buf,(n < TCP_BUFSIZE_READ) ? n : TCP_BUFSIZE_READ,MSG_NOSIGNAL);
    if (n == -1)
    {
        Handler().LogError(this, "read", Errno, StrError(Errno), LOG_LEVEL_FATAL);
        SetCloseAndDelete(true);                  // %!
        SetLost();
    }
    else
    if (!n)
    {
        Handler().LogError(this, "read", 0, "read returns 0", LOG_LEVEL_FATAL);
        SetCloseAndDelete(true);
        SetLost();
    }
    else
    {
        OnRawData(buf,n);
        if (!ibuf.Write(buf,n))
        {
// overflow
            Handler().LogError(this, "read", 0, "ibuf overflow", LOG_LEVEL_WARNING);
        }
    }
}


void TcpSocket::OnWrite()
{
    if (IsSSL())
    {
#ifdef HAVE_OPENSSL
/*
    if (!Handler().Valid(this))
        return;
    if (!Ready())
        return;
*/
        DEB(        printf("TcpSocket(SSL)::OnWrite()\n");)
// TODO: check MES buffer
            int n = SSL_write(m_ssl,obuf.GetStart(),obuf.GetL());
        DEB(        printf("OnWrite: %d bytes sent\n",n);)
            if (n == -1)
        {
// check code
            SetCloseAndDelete(true);
            DEB(            perror("write() error");)
        }
        else
        if (!n)
        {
            SetCloseAndDelete(true);
            DEB(            printf("write() returns 0\n");)
        }
        else
        {
            DEB(            printf(" %d bytes written\n",n);)
                obuf.Remove(n);
        }
        {
            bool br;
            bool bw;
            bool bx;
            Handler().Get(GetSocket(), br, bw, bx);
            if (obuf.GetLength())
                Set(br, true);
            else
                Set(br, false);
        }
        return;
#endif                                    // HAVE_OPENSSL
    }
/*
    assert(GetSocket() != INVALID_SOCKET);
    if (obuf.GetL() <= 0)
    {
printf("OnWrite abort because: nothing to write\n");
        Set(true, false);
        return;
    }
    assert(obuf.GetL() > 0);
    if (!Handler().Valid(this))
    {
printf("OnWrite abort because: not valid\n");
return;
}
if (!Ready())
{
printf("OnWrite abort because: not ready\n");
return;
}
*/
    int n = send(GetSocket(),obuf.GetStart(),(int)obuf.GetL(),MSG_NOSIGNAL);
/*
When writing onto a connection-oriented socket that has been shut down (by the  local
or the remote end) SIGPIPE is sent to the writing process and EPIPE is returned.  The
signal is not sent when the write call specified the MSG_NOSIGNAL flag.
*/
    if (n == -1)
    {
// normal error codes:
// WSAEWOULDBLOCK
//       EAGAIN or EWOULDBLOCK
#ifdef _WIN32
        if (Errno != WSAEWOULDBLOCK)
#else
            if (Errno != EWOULDBLOCK)
#endif
        {
            Handler().LogError(this, "write", Errno, StrError(Errno), LOG_LEVEL_FATAL);
            SetCloseAndDelete(true);              // %!
            SetLost();
        }
    }
    else
    if (!n)
    {
//		SetCloseAndDelete(true);
    }
    else
    {
        obuf.Remove(n);
    }
// check m_mes
    while (obuf.Space() && m_mes.size())
    {
        ucharp_v::iterator it = m_mes.begin();
        MES *p = *it;                             //m_mes[0];
        if (obuf.Space() > p -> left())
        {
            obuf.Write(p -> curbuf(),p -> left());
            delete p;
//printf("\n m_mes erase()\n");
            m_mes.erase(m_mes.begin());
        }
        else
        {
            size_t sz = obuf.Space();
            obuf.Write(p -> curbuf(),sz);
            p -> ptr += sz;
        }
    }
    {
        bool br;
        bool bw;
        bool bx;
        Handler().Get(GetSocket(), br, bw, bx);
        if (obuf.GetLength())
            Set(br, true);
        else
        {
            Set(br, false);
//			OnWriteComplete();
        }
    }
}


void TcpSocket::Send(const std::string &str)
{
    SendBuf(str.c_str(),str.size());
}


void TcpSocket::SendBuf(const char *buf,size_t len)
{
    int n = (int)obuf.GetLength();
    if (!Ready())
    {
// warning
        Handler().LogError(this, "SendBuf", -1, "Attempt to write to a non-ready socket" );
//	if (m_socket != INVALID_SOCKET && !Connecting() && !CloseAndDelete())
//		Handler().LogError(this, "SendBuf: Data to Write", len, static_cast<std::string>(buf).substr(0,len).c_str(), LOG_LEVEL_INFO);
        if (GetSocket() == INVALID_SOCKET)
            Handler().LogError(this, "SendBuf", 0, " * GetSocket() == INVALID_SOCKET", LOG_LEVEL_INFO);
        if (Connecting())
            Handler().LogError(this, "SendBuf", 0, " * Connecting()", LOG_LEVEL_INFO);
        if (CloseAndDelete())
            Handler().LogError(this, "SendBuf", 0, " * CloseAndDelete()", LOG_LEVEL_INFO);
        return;
    }
//DEB(	printf("trying to send %d bytes;  buf before = %d bytes\n",len,n);)
    if (m_mes.size() || len > obuf.Space())
    {
        MES *p = new MES(buf,len);
        m_mes.push_back(p);
    }
    if (m_mes.size())
    {
        while (obuf.Space() && m_mes.size())
        {
            ucharp_v::iterator it = m_mes.begin();
            MES *p = *it;                         //m_mes[0];
            if (obuf.Space() > p -> left())
            {
                obuf.Write(p -> curbuf(),p -> left());
                delete p;
                m_mes.erase(m_mes.begin());
            }
            else
            {
                size_t sz = obuf.Space();
                obuf.Write(p -> curbuf(),sz);
                p -> ptr += sz;
            }
        }
    }
    else
    {
        if (!obuf.Write(buf,len))
        {
            Handler().LogError(this, "SendBuf", -1, "Send overflow" );
// overflow
        }
    }
    if (!n)
    {
        OnWrite();
    }
}


void TcpSocket::OnLine(const std::string& )
{
}


void TcpSocket::ReadLine()
{
    if (ibuf.GetLength())
    {
        size_t x = 0;
        size_t n = ibuf.GetLength();
        char tmp[TCP_BUFSIZE_READ + 1];

        n = (n >= TCP_BUFSIZE_READ) ? TCP_BUFSIZE_READ : n;
        ibuf.Read(tmp,n);
        tmp[n] = 0;

        for (size_t i = 0; i < n; i++)
        {
            while (tmp[i] == 13 || tmp[i] == 10)
            {
                char c = tmp[i];
                tmp[i] = 0;
                if (tmp[x])
                {
                    m_line += (tmp + x);
                }
                OnLine( m_line );
                i++;
                if (i < n && (tmp[i] == 13 || tmp[i] == 10) && tmp[i] != c)
                {
                    i++;
                }
                x = i;
                m_line = "";
            }
        }
        if (tmp[x])
        {
            m_line += (tmp + x);
        }
    }
}


#ifdef _WIN32
#pragma warning(disable:4355)
#endif
TcpSocket::TcpSocket(const TcpSocket& s)
:Socket(s)
,ibuf(*this,0)
,obuf(*this,0)
{
}


#ifdef _WIN32
#pragma warning(default:4355)
#endif

void TcpSocket::OnSocks4Connect()
{
    char request[1000];
    request[0] = 4;                               // socks v4
    request[1] = 1;                               // command code: CONNECT
// send port in network byte order
    unsigned short port = htons(GetClientRemotePort());
    memcpy(request + 2, &port, 2);
// ipaddr_t is already in network byte order
    memcpy(request + 4, &GetClientRemoteAddr(), 4);
    strcpy(request + 8, GetSocks4Userid().c_str());
    size_t length = GetSocks4Userid().size() + 8 + 1;
    SendBuf(request, length);
    m_socks4_state = 0;
}


void TcpSocket::OnSocks4ConnectFailed()
{
    Handler().LogError(this,"OnSocks4ConnectFailed",0,"connection to socks4 server failed, trying direct connection",LOG_LEVEL_WARNING);
    if (!Handler().Socks4TryDirect())
    {
        SetCloseAndDelete();
        OnConnectFailed();                        // just in case
    }
    else
    {
        closesocket(GetSocket());
// open directly
        Open(GetClientRemoteAddr(), GetClientRemotePort(), true);
    }
}


bool TcpSocket::OnSocks4Read()
{
    switch (m_socks4_state)
    {
        case 0:
            ibuf.Read(&m_socks4_vn, 1);
            m_socks4_state = 1;
            break;
        case 1:
            ibuf.Read(&m_socks4_cd, 1);
            m_socks4_state = 2;
            break;
        case 2:
            if (GetInputLength() > 1)
            {
                ibuf.Read( (char *)&m_socks4_dstport, 2);
                m_socks4_state = 3;
            }
            else
            {
                return true;
            }
            break;
        case 3:
            if (GetInputLength() > 3)
            {
                ibuf.Read( (char *)&m_socks4_dstip, 4);
                SetSocks4(false);

                switch (m_socks4_cd)
                {
                    case 90:
                        OnConnect();
                        break;
                    case 91:
                    case 92:
                    case 93:
                        Handler().LogError(this,"OnSocks4Read",m_socks4_cd,"socks4 server reports connect failed",LOG_LEVEL_FATAL);
                        SetCloseAndDelete();
                        OnConnectFailed();
                        break;
                    default:
                        Handler().LogError(this,"OnSocks4Read",m_socks4_cd,"socks4 server unrecognized response",LOG_LEVEL_FATAL);
                        SetCloseAndDelete();
                        break;
                }
            }
            else
            {
                return true;
            }
            break;
    }
    return false;
}


void TcpSocket::Sendf(char const *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char slask[5000];
    vsnprintf(slask, 5000, format, ap);
    va_end(ap);
    Send( slask );
}


void TcpSocket::OnSSLConnect()
{
#ifdef HAVE_OPENSSL
    DEB(    printf("TcpSocket(SSL)::OnConnect()\n");)
        SetNonblocking(true);
    {
        if (m_context)
        {
            DEB(            printf("SSL Context already initialized - closing socket\n");)
                SetCloseAndDelete(true);
            return;
        }
        DEB(        printf("InitSSLClient()\n");)
            InitSSLClient();
    }
    if (m_context)
    {
/* Connect the SSL socket */
        m_ssl = SSL_new(m_context);
        if (!m_ssl)
        {
            DEB(            printf(" m_ssl is NULL\n");)
        }
        m_sbio = BIO_new_socket(GetSocket(), BIO_NOCLOSE);
        if (!m_sbio)
        {
            DEB(            printf(" m_sbio is NULL\n");)
        }
        SSL_set_bio(m_ssl, m_sbio, m_sbio);
        if (!SSLNegotiate())
            SetSSLNegotiate();
    }
    else
    {
        SetCloseAndDelete();
    }
#endif
}


void TcpSocket::OnSSLAccept()
{
#ifdef HAVE_OPENSSL
    DEB(    printf("TcpSocket(SSL)::OnAccept()\n");)
        SetNonblocking(true);
    {
        if (m_context)
        {
            DEB(            printf("SSL Context already initialized - closing socket\n");)
                SetCloseAndDelete(true);
            return;
        }
        InitSSLServer();
        SetSSLServer();
    }
    if (m_context)
    {
        m_ssl = SSL_new(m_context);
        if (!m_ssl)
        {
            DEB(            printf(" m_ssl is NULL\n");)
        }
        m_sbio = BIO_new_socket(GetSocket(), BIO_NOCLOSE);
        if (!m_sbio)
        {
            DEB(            printf(" m_sbio is NULL\n");)
        }
        SSL_set_bio(m_ssl, m_sbio, m_sbio);
        if (!SSLNegotiate())
            SetSSLNegotiate();
    }
#endif
}


bool TcpSocket::SSLNegotiate()
{
#ifdef HAVE_OPENSSL
    if (!IsSSLServer())                           // client
    {
        DEB(        printf("TcpSocket::SSLNegotiate() is_client\n");)
            int r = SSL_connect(m_ssl);
        DEB(        printf(" SSLNegotiate is_client, SSL_connect returns %d\n",r);)
            if (r > 0)
        {
            SetSSLNegotiate(false);
// TODO: resurrect certificate check... client
//			CheckCertificateChain( "");//ServerHOST);
            SetNonblocking(false);
            DEB(            printf("TcpSocket::SSLNegotiate() init OK\n");)
                OnConnect();
            return true;
        }
        else
        if (!r)
        {
            SetSSLNegotiate(false);
            SetCloseAndDelete();
        }
        else
        {
            r = SSL_get_error(m_ssl, r);
            if (r != SSL_ERROR_WANT_READ && r != SSL_ERROR_WANT_WRITE)
            {
                DEB(                printf("SSL_connect() failed - closing socket, return code: %d\n",r);)
                    SetSSLNegotiate(false);
                SetCloseAndDelete(true);
            }
        }
    }
    else                                          // server
    {
        DEB(        printf("TcpSocket::SSLNegotiate() is_server\n");)
            int r = SSL_accept(m_ssl);
        DEB(        printf(" SSLNegotiate is_server, SSL_accept returns %d\n",r);)
            if (r > 0)
        {
            SetSSLNegotiate(false);
// TODO: resurrect certificate check... server
//			CheckCertificateChain( "");//ClientHOST);
            SetNonblocking(false);
            DEB(            printf("TcpSocket::SSLNegotiate() init OK\n");)
                OnAccept();
            return true;
        }
        else
        if (!r)
        {
            SetSSLNegotiate(false);
            SetCloseAndDelete();
        }
        else
        {
            r = SSL_get_error(m_ssl, r);
            if (r != SSL_ERROR_WANT_READ && r != SSL_ERROR_WANT_WRITE)
            {
                DEB(                printf("SSL_accept() failed - closing socket, return code: %d\n",r);)
                    SetSSLNegotiate(false);
                SetCloseAndDelete(true);
            }
        }
    }
#endif                                        // HAVE_OPENSSL
    return false;
}


void TcpSocket::InitSSLClient()
{
#ifdef HAVE_OPENSSL
//	InitializeContext();
    InitializeContext(SSLv23_method());
#endif
}


void TcpSocket::InitSSLServer()
{
}


#ifdef HAVE_OPENSSL
void TcpSocket::InitializeContext(SSL_METHOD *meth_in)
{
    SSL_METHOD *meth;

    if (!bio_err)
    {
/* An error write context */
        bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);

/* Global system initialization*/
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
    }

/* Create our context*/
    meth = meth_in ? meth_in : SSLv3_method();
    m_context = SSL_CTX_new(meth);

/* Load the CAs we trust*/
/*
    if (!(SSL_CTX_load_verify_locations(m_context, CA_LIST, 0)))
    {
DEB(		printf("Couldn't read CA list\n");)
    }
    SSL_CTX_set_verify_depth(m_context, 1);
    SSL_CTX_set_verify(m_context, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_cb);
*/

/* Load randomness */
    if (!(RAND_load_file(RANDOM, 1024*1024)))
    {
        DEB(        printf("Couldn't load randomness\n");)
    }

}


void TcpSocket::InitializeContext(const std::string& keyfile,const std::string& password,SSL_METHOD *meth_in)
{
    SSL_METHOD *meth;

    if (!bio_err)
    {
/* An error write context */
        bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);

/* Global system initialization*/
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
    }

/* Create our context*/
    meth = meth_in ? meth_in : SSLv3_method();
    m_context = SSL_CTX_new(meth);

/* Load our keys and certificates*/
    if (!(SSL_CTX_use_certificate_file(m_context, keyfile.c_str(), SSL_FILETYPE_PEM)))
    {
        DEB(        printf("Couldn't read certificate file\n");)
    }

    m_password = password;
    SSL_CTX_set_default_passwd_cb(m_context, password_cb);
    if (!(SSL_CTX_use_PrivateKey_file(m_context, keyfile.c_str(), SSL_FILETYPE_PEM)))
    {
        DEB(        printf("Couldn't read key file\n");)
    }

/* Load the CAs we trust*/
/*
    if (!(SSL_CTX_load_verify_locations(m_context, CA_LIST, 0)))
    {
DEB(		printf("Couldn't read CA list\n");)
    }
    SSL_CTX_set_verify_depth(m_context, 1);
    SSL_CTX_set_verify(m_context, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_cb);
*/

/* Load randomness */
    if (!(RAND_load_file(RANDOM, 1024*1024)))
    {
        DEB(        printf("Couldn't load randomness\n");)
    }

}


// static
int TcpSocket::password_cb(char *buf,int num,int rwflag,void *userdata)
{
    if ( (size_t)num < m_password.size() + 1)
    {
        return 0;
    }
    strcpy(buf,m_password.c_str());
    return m_password.size();
}
#endif                                            // HAVE_OPENSSL

int TcpSocket::Close()
{
#ifdef HAVE_OPENSSL
    if (IsSSL())
        SSL_shutdown(m_ssl);
#endif
    return Socket::Close();
}
