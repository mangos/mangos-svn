/**
 **	File ......... ResolvSocket.h
 **	Published ....  2005-03-24
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
#ifndef _RESOLVSOCKET_H
#define _RESOLVSOCKET_H

#include "SocketHandler.h"
#include "TcpSocket.h"

class ResolvSocket : public TcpSocket
{
    public:
        ResolvSocket(SocketHandler&,Socket *parent = NULL);
        ~ResolvSocket();

        void OnAccept() { m_bServer = true; }
        void OnLine(const std::string& );
        void OnDetached();

        void SetId(int x) { m_resolv_id = x; }
        void SetHost(const std::string& x) { m_resolv_host = x; }
        void SetPort(port_t x) { m_resolv_port = x; }
        void OnConnect();

    private:
// copy constructor
        ResolvSocket(const ResolvSocket& s) : TcpSocket(s)
        {
        }
// assignment operator
        ResolvSocket& operator=(const ResolvSocket& )
        {
            return *this;
        }

        std::string m_query;
        std::string m_data;
        bool m_bServer;
        Socket *m_parent;
        int m_resolv_id;
        std::string m_resolv_host;
        port_t m_resolv_port;
};
#endif                                            // _RESOLVSOCKET_H
