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
#include "AddonHandler.h"
#include "zlib/zlib.h"
#include "../realmd/AuthCodes.h"
#include <cwctype>                                          // needs for towupper

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

WorldSocket::WorldSocket(SocketHandler &sh): TcpSocket(sh), _cmd(0), _remaining(0), _session(NULL)
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
    uint32 unk2;
    uint32 BuiltNumberClient;
    uint32 id, security;
    std::string account;
    Sha1Hash I;
    Sha1Hash sha1;
    BigNumber v, s, g, N, x;
    std::string password;
    WorldPacket packet, SendAddonPacked;

    BigNumber K;

    try
    {
        recvPacket >> BuiltNumberClient;                    // for now no use
        recvPacket >> unk2;
        recvPacket >> account;
        recvPacket >> clientSeed;
        recvPacket.read(digest, 20);
    }
    catch(ByteBuffer::error &)
    {
        sLog.outError("WorldSocket::_HandleAuthSession Get Incomplete packet");
        return;
    }
    sLog.outDebug("Auth: client %u, unk2 %u, account %s, clientseed %u, digest %u", BuiltNumberClient, unk2, account.c_str(), clientSeed, digest);
    loginDatabase.escape_string(account);

    QueryResult *result = loginDatabase.PQuery("SELECT `id`,`gmlevel`,`sessionkey`,`last_ip`,`locked`, `password`, `v`, `s`, `banned` FROM `account` WHERE `username` = '%s'", account.c_str());

    if ( !result )
    {

        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( AUTH_UNKNOWN_ACCOUNT );
        SendPacket( &packet );

        sLog.outDetail( "SOCKET: Sent Auth Response (unknown account)." );
        return;
    }

    N.SetHexStr("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
    g.SetDword(7);
    password = (*result)[5].GetString();
    std::transform(password.begin(), password.end(), password.begin(), std::towupper);

    s.SetHexStr((*result)[7].GetString());
    std::string sI = account + ":" + password;
    I.UpdateData(sI);
    I.Finalize();
    sha1.UpdateData(s.AsByteArray(), s.GetNumBytes());
    sha1.UpdateData(I.GetDigest(), 20);
    sha1.Finalize();
    x.SetBinary(sha1.GetDigest(), sha1.GetLength());
    v = g.ModExp(x, N);

    sLog.outDebug("SOCKET: (s,v) check s: %s v_old: %s v_new: %s", s.AsHexStr(), (*result)[6].GetString(), v.AsHexStr() );
    loginDatabase.PQuery("UPDATE `account` SET `v` = '0', `s` = '0' WHERE `username` = '%s'", account.c_str());
    if ( strcmp(v.AsHexStr(),(*result)[6].GetString() ) )
    {
        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( AUTH_UNKNOWN_ACCOUNT );
        SendPacket( &packet );
        sLog.outDetail( "SOCKET: User not logged.");
        delete result;
        return;
    }

    if((*result)[4].GetUInt8() == 1)                        // if ip is locked
    {
        if ( strcmp((*result)[3].GetString(),GetRemoteAddress().c_str()) )
        {
            packet.Initialize( SMSG_AUTH_RESPONSE );
            packet << uint8( REALM_AUTH_ACCOUNT_FREEZED );
            SendPacket( &packet );

            sLog.outDetail( "SOCKET: Sent Auth Response (Account IP differs)." );
            return;
        }
    }
    if((*result)[8].GetUInt8() == 1)                        // if account banned
    {
        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( AUTH_BANNED );
        SendPacket( &packet );

        sLog.outDetail( "SOCKET: Sent Auth Response (Account banned)." );
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
        //packet << uint8( 21 );
        packet << uint8( CSTATUS_FULL);
        SendPacket( &packet );
        sLog.outBasic( "SOCKET: Sent Auth Response (server full)." );
        return;
    }

    WorldSession *session = sWorld.FindSession( id );
    if( session )
    {
        packet.Initialize( SMSG_AUTH_RESPONSE );
        //packet << uint8( 13 );
        packet << uint8( AUTH_ALREADY_ONLINE );
        SendPacket( &packet );

        sLog.outDetail( "SOCKET: Sent Auth Response (already connected)." );

        session->LogoutPlayer(true);

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
        //packet << uint8( 21 );
        packet << uint8( AUTH_FAILED );

        SendPacket( &packet );

        sLog.outDetail( "SOCKET: Sent Auth Response (authentification failed)." );
        return;
    }

    _crypt.SetKey(K.AsByteArray(), 40);
    _crypt.Init();

    //Send Auth is okey
    packet.Initialize( SMSG_AUTH_RESPONSE );
    packet << uint8( AUTH_OK );                             //0x0C
    //packet << uint8( 0xB0 ); - redundent
    //packet << uint8( 0x09 );
    //packet << uint8( 0x02 );
    //packet << uint8( 0x00 );
    //packet << uint8( 0x02 );
    //packet << uint32( 0x0 );

    SendPacket(&packet);

    _session = new WorldSession(id, this);

    ASSERT(_session);
    _session->SetSecurity(security);
    sWorld.AddSession(_session);

    sLog.outBasic( "SOCKET: Client '%s' authed successfully.", account.c_str() );
    sLog.outString( "Account: '%s' Login.", account.c_str() );
    loginDatabase.PQuery("UPDATE `account` SET `last_ip` = '%s' WHERE `username` = '%s'",GetRemoteAddress().c_str(), account.c_str());

    // do small delay (10ms) at accepting successful authed connection to prevent droping packets by client
    // don't must harm anyone (let login ~100 accounts in 1 sec ;) )
    #ifdef WIN32
    Sleep(10);
    #endif
    //! Handled Addons

    //Create Addon Packet
    sAddOnHandler.BuildAddonPacket(&recvPacket, &SendAddonPacked, recvPacket.rpos());
    SendPacket(&SendAddonPacked);

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

void WorldSocket::SendAuthWaitQue(uint32 PlayersInQue)
{
    WorldPacket packet;
    packet.Initialize( SMSG_AUTH_RESPONSE );
    packet << uint8( AUTH_WAIT_QUEUE );
    //packet << uint32 (0x00);                                //unknown
    //packet << uint32 (0x00);                                //unknown
    //packet << uint8 (0x00);                                 //unknown
    packet << uint32 (PlayersInQue);                        //amount of players in que
    SendPacket(&packet);
}
