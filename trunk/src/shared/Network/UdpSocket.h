/**
 **	File ......... UdpSocket.h
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
#ifndef _UDPSOCKET_H
#define _UDPSOCKET_H

#include "Socket.h"

class UdpSocket : public Socket
{
    public:
        UdpSocket(SocketHandler& ,int ibufsz = 16384);
        ~UdpSocket();

/** new callback */
        virtual void OnRawData(const char *,size_t,struct sockaddr *,socklen_t) {}

/** to receive incoming data, call Bind to setup an incoming port */
        SOCKET Bind(port_t& port,int range);
        SOCKET Bind6(port_t& port,int range);

/** if you wish to use Send, first Open a connection */
        bool Open(ipaddr_t,port_t);
        bool Open(const std::string& host,port_t port);
        bool Open6(struct in6_addr&,port_t);
        bool Open6(const std::string& host,port_t port);

/** create before using sendto methods */
        void CreateConnection();
        void CreateConnection6();                 // ipv6

/** send to specified address */
        void SendToBuf(const std::string& ,port_t,const char *data,int len,int flags = 0);
        void SendToBuf(ipaddr_t,port_t,const char *data,int len,int flags = 0);
        void SendToBuf6(const std::string& ,port_t,const char *data,int len,int flags = 0);
        void SendTo(const std::string&,port_t,const std::string&,int flags = 0);
        void SendTo(ipaddr_t,port_t,const std::string&,int flags = 0);
        void SendTo6(const std::string&,port_t,const std::string&,int flags = 0);

/** send to connected address */
        void SendBuf(const char *data,int,int flags = 0);
        void Send(const std::string& ,int flags = 0);

/** broadcast */
        void SetBroadcast(bool b = true);
        bool IsBroadcast();

/** multicast */
        void SetMulticastTTL(int ttl = 1);
        int GetMulticastTTL();
        void SetMulticastLoop(bool = true);
        bool IsMulticastLoop();
        void AddMulticastMembership(const std::string& group,const std::string& intf = "0.0.0.0",int if_index = 0);
        void DropMulticastMembership(const std::string& group,const std::string& intf = "0.0.0.0",int if_index = 0);
/** multicast, ipv6 */
        void SetMulticastHops(int = -1);
        int GetMulticastHops();

    protected:
        UdpSocket(const UdpSocket& s) : Socket(s) {}
        void OnRead();

// int  recvfrom(int  s,  void *buf, int len, int flags, struct sockaddr *from, socklen_t *fromlen);
// int  sendto(int  s,  const  void  *msg, int len, int flags, const struct sockaddr *to, socklen_t tolen);
    private:
        UdpSocket& operator=(const UdpSocket& ) { return *this; }
        bool m_connected;
        char *m_ibuf;
        int m_ibufsz;
};
#endif                                            // _UDPSOCKET_H
