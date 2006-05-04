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
#include "NameTables.h"
#include "Policies/SingletonImp.h"
#include "WorldLog.h"

// Only GCC 4.1.0 and later support #pragma pack(push,1) syntax
#if __GNUC__ && (GCC_MAJOR < 4 || GCC_MAJOR == 4 && GCC_MINOR < 1)
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

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

// Only GCC 4.1.0 and later support #pragma pack(pop) syntax
#if __GNUC__ && (GCC_MAJOR < 4 || GCC_MAJOR == 4 && GCC_MINOR < 1)
#pragma pack()
#else
#pragma pack(pop)
#endif

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
                break;

            ClientPktHeader hdr;

            ibuf.Read((char *)&hdr, 6);
            _crypt.DecryptRecv((uint8 *)&hdr, 6);

            _remaining = ntohs(hdr.size) - 4;
            _cmd = hdr.cmd;
        }

        if (ibuf.GetLength() < _remaining)
            break;

        WorldPacket packet;

        packet.resize(_remaining);
        packet.SetOpcode((uint16)_cmd);
        if(_remaining) ibuf.Read((char*)packet.contents(), _remaining);

        if( sWorldLog.LogWorld() )
        {
            sWorldLog.Log("CLIENT:\nSOCKET: %u\nLENGTH: %u\nOPCODE: %s (0x%.4X)\nDATA:\n",
                (uint32)GetSocket(),
                _remaining,
                LookupName(packet.GetOpcode(), g_worldOpcodeNames),
                packet.GetOpcode());

            uint32 p = 0;
            while (p < packet.size())
            {
                for (uint32 j = 0; j < 16 && p < packet.size(); j++)
                    sWorldLog.Log("%.2X ", packet[p++]);

                sWorldLog.Log("\n");
            }

            sWorldLog.Log("\n\n");
        }

        _remaining = 0;

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
                    sLog.outDetail("Received out of place packet with cmdid 0x%.4X", _cmd);
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
    uint32 ADDONCount;
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
        recvPacket >> ADDONCount;
    }
    catch(ByteBuffer::error &)
    {
        sLog.outDetail("Incomplete packet");
        return;
    }

    QueryResult *result = sDatabase.PQuery("SELECT `id`,`gmlevel`,`sessionkey` FROM `account` WHERE `username` = '%s';", account.c_str());

    if ( !result )
    {

        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( 21 );
        SendPacket( &packet );

        sLog.outDetail( "SOCKET: Sent Auth Response (unknown account)." );
        return;
    }

    id = (*result)[0].GetUInt32();
    security = (*result)[1].GetUInt16();
    K.SetHexStr((*result)[2].GetString());

    delete result;

    uint32 num = sWorld.GetSessionCount();
    if (sWorld.GetPlayerLimit() > 0 && num > sWorld.GetPlayerLimit() && security == 0)
    {

        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( 21 );
        SendPacket( &packet );

        sLog.outBasic( "SOCKET: Sent Auth Response (server full)." );
        return;
    }

    WorldSession *session = sWorld.FindSession( id );
    if( session )
    {
        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( 13 );
        SendPacket( &packet );

        sLog.outDetail( "SOCKET: Sent Auth Response (already connected)." );

        session->LogoutPlayer(false);

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

        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( 21 );
        SendPacket( &packet );

        sLog.outDetail( "SOCKET: Sent Auth Response (authentification failed)." );
        return;
    }

    _crypt.SetKey(K.AsByteArray(), 40);
    _crypt.Init();

    uint8 size = 12;
    packet.Initialize( SMSG_AUTH_RESPONSE );
    packet << uint8( size );                                //0x0C
    packet << uint8( 0xB0 );
    packet << uint8( 0x09 );
    packet << uint8( 0x02 );
    packet << uint8( 0x00 );
    packet << uint8( 0x02 );
    packet << uint32( 0x0 );

    SendPacket(&packet);

    //! Enable ADDON's Thanks to Burlex
    //! this is a fast hack, real fix is comming

    packet.Initialize(0x2EF);                               // SMSG_ADDON_INFO
    packet << uint8(0x00);
    for(int i = 0; i < ADDONCount; i++)
        packet << uint8(0x01);

    packet << uint8(0x00);

    SendPacket(&packet);

    _session = new WorldSession(id, this);

    ASSERT(_session);
    _session->SetSecurity(security);
    sWorld.AddSession(_session);

    sLog.outBasic( "SOCKET: Client '%s' authed successfully.", account.c_str() );

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
        sLog.outDetail("Incomplete packet");
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

        if( sWorldLog.LogWorld() )
        {
            sWorldLog.Log("SERVER:\nSOCKET: %u\nLENGTH: %u\nOPCODE: %s (0x%.4X)\nDATA:\n",
                (uint32)GetSocket(),
                packet->size(),
                LookupName(packet->GetOpcode(), g_worldOpcodeNames),
                packet->GetOpcode());

            uint32 p = 0;
            while (p < packet->size())
            {
                for (uint32 j = 0; j < 16 && p < packet->size(); j++)
                    sWorldLog.Log("%.2X ", (*packet)[p++]);

                sWorldLog.Log("\n");
            }

            sWorldLog.Log("\n\n");
        }

        _crypt.EncryptSend((uint8*)&hdr, 4);

        TcpSocket::SendBuf((char*)&hdr, 4);
        if(packet->size()) TcpSocket::SendBuf((char*)packet->contents(), packet->size());

        delete packet;
    }
}
