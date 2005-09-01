/* WorldSocket.cpp
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

#include "Common.h"
#include "Log.h"
#include "Opcodes.h"
#include "Config/ConfigEnv.h"
#include "Database/DatabaseEnv.h"
#include "Auth/Sha1.h"
#include "WorldPacket.h"
#include "WorldSocket.h"
#include "WorldSession.h"
#include "World.h"
#include "WorldSocketMgr.h"

#pragma pack(push, 1)
struct ClientPktHeader
{
    uint16 size;
    uint32 cmd;
};

struct ServerPktHeader
{
    uint16 size;
    uint16 cmd;
};
#pragma pack(pop)

WorldSocket::WorldSocket(SocketHandler &sh): TcpSocket(sh), _remaining(0), _session(0), _cmd(0)
{
    _seed = _GetSeed();
}


WorldSocket::~WorldSocket()
{
    WorldPacket *packet;

    while(!_sendQueue.empty())
    {
        packet = _sendQueue.next();
        delete packet;
    }
}


uint32 WorldSocket::_GetSeed()
{
    return 0xDEADBABE;
}


void WorldSocket::SendPacket(WorldPacket* packet)
{
    WorldPacket *pck = new WorldPacket(*packet);
    ASSERT(pck);

// guard
    _sendQueue.add(pck);
}


void WorldSocket::OnAccept()
{
    WorldPacket packet;

    sWorldSocketMgr.AddSocket(this);

    packet.Initialize( SMSG_AUTH_CHALLENGE );
    packet << _seed;

    SendPacket(&packet);
}


void WorldSocket::OnRead()
{
    TcpSocket::OnRead();

    while(1)
    {
        if (!_remaining)
        {
            if (ibuf.GetLength() < 6)
                break;                            // no header

            ClientPktHeader hdr;

            ibuf.Read((char *)&hdr, 6);
            _crypt.DecryptRecv((uint8 *)&hdr, 6);

            _remaining = ntohs(hdr.size) - 4;     // we've already read cmd
            _cmd = hdr.cmd;
        }

// wait until there's full packet
        if (ibuf.GetLength() < _remaining)
            break;

        WorldPacket packet;

        packet.resize(_remaining);
        packet.SetOpcode((uint16)_cmd);
        ibuf.Read((char*)packet.contents(), _remaining);

// World Logger
        if (sConfig.GetBoolDefault("LogWorld", false))
        {
            FILE *pFile = fopen("world.log", "a");
            fprintf(pFile,
                "CLIENT:\nSOCKET: %d\nLENGTH: %d\nOPCODE: %.4X\nDATA:\n",
                (uint32)GetSocket(),
                _remaining,
                _cmd);

            uint32 p = 0;
            while (p < packet.size())
            {
                for (uint32 j = 0; j < 16 && p < packet.size(); j++)
                    fprintf(pFile, "%.2X ", packet[p++]);

                fprintf(pFile, "\n");
            }

            fprintf(pFile, "\n\n");
            fclose(pFile);
        }

        _remaining = 0;

// packets that we handle ourself
        switch (_cmd)
        {
            case CMSG_PING:
            {
                _HandlePing(packet);
                break;
            }
            case CMSG_AUTH_SESSION:
            {
                _HandleAuthSession(packet);
                break;
            }
            default:
            {
                if (_session)
                    _session->QueuePacket(packet);
                else
                    Log::getSingleton( ).outDetail("Received out of place packet with cmdid 0x%.4X", _cmd);
            }
        }
    }
}


void WorldSocket::OnDelete()
{
    if (_session)
    {
        _session->SetSocket(0);
        _session = 0;
    }

    sWorldSocketMgr.RemoveSocket(this);
}


void WorldSocket::_HandleAuthSession(WorldPacket& recvPacket)
{
    uint8 digest[20];
    uint32 clientSeed;
    std::string account;
    uint32 unk1, unk2;
    uint32 id, security;
    WorldPacket packet;
    BigNumber K;

    try
    {
        recvPacket >> unk1;
        recvPacket >> unk2;
        recvPacket >> account;
        recvPacket >> clientSeed;
        recvPacket.read(digest, 20);
    }
    catch(ByteBuffer::error &)
    {
        Log::getSingleton( ).outDetail("Incomplete packet");
        return;
    }

    std::stringstream ss;
    ss << "SELECT acct, gm, sessionkey FROM accounts WHERE login='" << account << "'";

    QueryResult *result = sDatabase.Query(ss.str().c_str());

// Checking if account exists
    if ( !result )
    {
// Send Bad Account
        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( 21 );                    // TODO: use enums
        SendPacket( &packet );

        Log::getSingleton( ).outDetail( "SOCKET: Sent Auth Response (unknown account)." );
        return;
    }

    id = (*result)[0].GetUInt32();
    security = (*result)[1].GetUInt16();
    K.SetHexStr((*result)[2].GetString());

    delete result;

// TODO: check if server is full
    uint32 num = sWorld.GetSessionCount();
    if (sWorld.GetPlayerLimit() > 0 && num > sWorld.GetPlayerLimit() && security == 0)
    {
// Should Send Server Full Error Code
        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( 21 );                    // TODO: use enums
        SendPacket( &packet );

        Log::getSingleton( ).outBasic( "SOCKET: Sent Auth Response (server full)." );
        return;
    }

//checking if player is already connected
    WorldSession *session = sWorld.FindSession( id );
    if( session )
    {
        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( 13 );                    // TODO: use enums
        SendPacket( &packet );

        Log::getSingleton( ).outDetail( "SOCKET: Sent Auth Response (already connected)." );

        session->LogoutPlayer(false);
//session->SetSocket(0);
//session = 0;
//sWorldSocketMgr.RemoveSocket(this);
//sWorld.RemoveSession(id);
        return;
    }

    Sha1Hash sha;

    uint32 t = 0;
    uint32 seed = _seed;

    sha.UpdateData(account);
    sha.UpdateData((uint8 *)&t, 4);
    sha.UpdateData((uint8 *)&clientSeed, 4);
    sha.UpdateData((uint8 *)&seed, 4);
    sha.UpdateBigNumbers(&K, NULL);
    sha.Finalize();

    if (memcmp(sha.GetDigest(), digest, 20))
    {
// Sending Authentification Failed
        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( 21 );                    // TODO: use enums
        SendPacket( &packet );

        Log::getSingleton( ).outDetail( "SOCKET: Sent Auth Response (authentification failed)." );
        return;
    }

    _crypt.SetKey(K.AsByteArray(), 40);
    _crypt.Init();

    packet.Initialize( SMSG_AUTH_RESPONSE );
    packet << uint8( 0x0C );                      // auth succesfull, TODO: use enums
    packet << uint8( 0xcf );                      // sometime of counter.. increased by 1 at every auth
    packet << uint8( 0xD2 );                      // always d2
    packet << uint8( 0x07 );                      // always 07
    packet << uint8( 0x00 );                      // always 00
    packet << uint8( 0x00 );                      // always 00

    SendPacket(&packet);

// FIXME: allocating memory in one thread and transferring ownership  to another.
    _session = new WorldSession(id, this);
    ASSERT(_session);
    _session->SetSecurity(security);
    sWorld.AddSession(_session);

    Log::getSingleton( ).outBasic( "SOCKET: Client '%s' authed successfully.", account.c_str() );

    return;
}


void WorldSocket::_HandlePing(WorldPacket& recvPacket)
{
    WorldPacket packet;
    uint32 ping;

    try
    {
        recvPacket >> ping;
    }
    catch(ByteBuffer::error &)
    {
        Log::getSingleton( ).outDetail("Incomplete packet");
        return;
    }

    packet.Initialize( SMSG_PONG );
    packet << ping;
    SendPacket(&packet);

    return;
}


void WorldSocket::Update(time_t diff)
{
    WorldPacket *packet;
    ServerPktHeader hdr;

    while(!_sendQueue.empty())
    {
        packet = _sendQueue.next();

        hdr.size = ntohs((uint16)packet->size() + 2);
        hdr.cmd = packet->GetOpcode();

// World Logger
        if (sConfig.GetBoolDefault("LogWorld", false))
        {
            FILE *pFile = fopen("world.log", "a");
            fprintf(pFile,
                "SERVER:\nSOCKET: %d\nLENGTH: %d\nOPCODE: %.4X\nDATA:\n",
                (uint32)GetSocket(),
                packet->size(),
                packet->GetOpcode());

            uint32 p = 0;
            while (p < packet->size())
            {
                for (uint32 j = 0; j < 16 && p < packet->size(); j++)
                    fprintf(pFile, "%.2X ", (*packet)[p++]);

                fprintf(pFile, "\n");
            }

            fprintf(pFile, "\n\n");
            fclose(pFile);
        }

        _crypt.EncryptSend((uint8*)&hdr, 4);

        TcpSocket::SendBuf((char*)&hdr, 4);
        TcpSocket::SendBuf((char*)packet->contents(), packet->size());

        delete packet;
    }
}
