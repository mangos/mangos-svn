/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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

/// \addtogroup u2w
/// @{
/// \file

#ifndef __WORLDSOCKET_H
#define __WORLDSOCKET_H

#include "Network/TcpSocket.h"
#include "Auth/AuthCrypt.h"

class WorldPacket;
class SocketHandler;
class WorldSession;

/// Handle connection with the client software
class WorldSocket : public TcpSocket
{
    public:
        WorldSocket(SocketHandler&);
        ~WorldSocket();

        void SendPacket(WorldPacket* packet);

        void OnAccept();
        void OnRead();
        void OnDelete();

        void Update(time_t diff);

    protected:
        void _HandleAuthSession(WorldPacket& recvPacket);
        void _HandlePing(WorldPacket& recvPacket);
        void SendAuthWaitQue(uint32 PlayersInQue);

    private:
        AuthCrypt _crypt;
        uint32 _seed;
        uint32 _cmd;
        uint16 _remaining;
        WorldSession* _session;

        ZThread::LockedQueue<WorldPacket*,ZThread::FastMutex> _sendQueue;

        uint32 m_LastPingMSTime;
        uint32 m_OverSpeedPings;
};
#endif
/// @}
