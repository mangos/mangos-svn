/**
 **	File ......... TcpSocket.h
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
#ifndef _TCPSOCKET_H
#define _TCPSOCKET_H

#include "Socket.h"
#include "CircularBuffer.h"
#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#ifdef _WIN32
// TODO: systray.exe??
#define RANDOM "systray.exe"
#else
#define RANDOM "/dev/urandom"
#endif
#endif

#define TCP_BUFSIZE_READ 131070

class TcpSocket : public Socket
{
    struct MES
    {
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
    typedef std::list<MES *> ucharp_v;

    public:
        TcpSocket(SocketHandler& );
        TcpSocket(SocketHandler& ,size_t isize,size_t osize);
        ~TcpSocket();

/** If you want your socket to connect to a server,
    always call Open before Add'ing a socket to the sockethandler.
    If not, the connection attempt will not be monitored by the
    socket handler... */
        bool Open(ipaddr_t,port_t,bool skip_socks = false);
        bool Open(const std::string &host,port_t port);
        bool Open6(const std::string& host,port_t port);
        int Close();

        void Send(const std::string &);
        void Sendf(char const *format, ...);

        virtual void SendBuf(const char *,size_t);
        virtual void OnRawData(const char *,size_t) {}

        size_t GetInputLength() { return ibuf.GetLength(); }
        size_t GetOutputLength() { return obuf.GetLength(); }

        void ReadLine();
        virtual void OnLine(const std::string& );

        unsigned long GetBytesReceived() { return ibuf.ByteCounter(); }
        unsigned long GetBytesSent() { return obuf.ByteCounter(); }

        void OnSocks4Connect();
        void OnSocks4ConnectFailed();
/** returns 'need_more' */
        bool OnSocks4Read();

        void Resolved(int id,ipaddr_t a,port_t port);

// SSL
        void OnSSLConnect();
        void OnSSLAccept();
        virtual void InitSSLClient();
        virtual void InitSSLServer();

    protected:
        TcpSocket(const TcpSocket& s);
        void OnRead();
        void OnWrite();
// SSL
#ifdef HAVE_OPENSSL
        void InitializeContext(SSL_METHOD * = NULL);
        void InitializeContext(const std::string& keyfile,const std::string& password,SSL_METHOD * = NULL);
        static  int password_cb(char *buf,int num,int rwflag,void *userdata);
#endif
        bool SSLNegotiate();
//
        CircularBuffer ibuf;
        CircularBuffer obuf;
        std::string m_line;
        ucharp_v m_mes;                           // overflow protection

    private:
        TcpSocket& operator=(const TcpSocket& ) { return *this; }
        int m_socks4_state;
        char m_socks4_vn;
        char m_socks4_cd;
        unsigned short m_socks4_dstport;
        unsigned long m_socks4_dstip;
        int m_resolver_id;
// SSL
#ifdef HAVE_OPENSSL
        SSL_CTX *m_context;
        SSL *m_ssl;
        BIO *m_sbio;
        static  BIO *bio_err;
        static  std::string m_password;
#endif
};
#endif                                            // _TCPSOCKET_H
