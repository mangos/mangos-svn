/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _AUTHSOCKET_H
#define _AUTHSOCKET_H

#include "Common.h"
#include "Auth/BigNumber.h"
#include "Auth/Sha1.h"
#include "Network/TcpSocket.h"

class SocketHandler;

class AuthSocket: public TcpSocket
{
    public:
        const static int N_BYTE_SIZE = 32;
        const static int s_BYTE_SIZE = 32;

        AuthSocket(SocketHandler& h);
        ~AuthSocket();

        void OnAccept();
        void OnRead();

    private:
        enum eAuthCmd
        {
            AUTH_NO_CMD                 = 0xFF,
            AUTH_LOGON_CHALLENGE        = 0x00,
            AUTH_LOGON_PROOF            = 0x01,
            AUTH_RECONNECT_CHALLENGE    = 0x02,
            AUTH_RECONNECT_PROOF        = 0x03,
            REALM_LIST                  = 0x10,
            XFER_INITIATE               = 0x30,   
            XFER_DATA                   = 0x31,   
            XFER_UNK1                   = 0x32,   
            XFER_UNK2                   = 0x33,   
            XFER_UNK3                   = 0x34    
        };

        enum eStatus
        {
            STATUS_CONNECTED = 0,
            STATUS_AUTHED
        };

        struct AuthHandler
        {
            eAuthCmd cmd;
            uint32 status;
            bool (AuthSocket::*handler)(void);
        };

        AuthHandler* _GetHandlerTable() const;

        bool _HandleLogonChallenge();
        bool _HandleLogonProof();
        bool _HandleRealmList();
        bool _HandleXferInitiate();
        bool _HandleXferData();

        BigNumber N, s, g, v;
        BigNumber b, B;
        BigNumber rs;

        
        
        
        BigNumber K;

        eAuthCmd _cmd;
        bool _authed;

        std::string _login;
};
#endif
