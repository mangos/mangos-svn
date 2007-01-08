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

/** \file
    \ingroup u2w
*/

#include "Common.h"
#include "Log.h"
#include "Opcodes.h"
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
#include "../realmd/AuthCodes.h"
#include <cwctype>                                          // needs for towupper

// Only GCC 4.1.0 and later support #pragma pack(push,1) syntax
#if __GNUC__ && (GCC_MAJOR < 4 || GCC_MAJOR == 4 && GCC_MINOR < 1)
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

/// Client Packet Header
struct ClientPktHeader
{
    uint16 size;
    uint32 cmd;
};

/// Server Packet Header
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

/// WorldSocket construction and initialisation.
WorldSocket::WorldSocket(SocketHandler &sh): TcpSocket(sh), _cmd(0), _remaining(0), _session(NULL)
{
    _seed = 0xDEADBABE;
    m_LastPingMSTime = 0;                                   // first time it will counted as overspeed maybe, but this is not important
    m_OverSpeedPings = 0;
}

/// WorldSocket destructor
WorldSocket::~WorldSocket()
{
    WorldPacket *packet;

    ///- Go through the to-be-sent queue and delete remaining packets
    while(!_sendQueue.empty())
    {
        packet = _sendQueue.next();
        delete packet;
    }
}

/// Copy the packet to the to-be-sent queue
void WorldSocket::SendPacket(WorldPacket* packet)
{
    WorldPacket *pck = new WorldPacket(*packet);
    ASSERT(pck);
    _sendQueue.add(pck);
}

/// On client connection
void WorldSocket::OnAccept()
{
    ///- Add the current socket to the list of sockets to be managed (WorldSocketMgr)
    sWorldSocketMgr.AddSocket(this);

    ///- Send a AUTH_CHALLENGE packet
    WorldPacket packet( SMSG_AUTH_CHALLENGE, 4 );
    packet << _seed;

    SendPacket(&packet);
}

/// Read the client transmitted data
void WorldSocket::OnRead()
{
    TcpSocket::OnRead();

    while(1)
    {
        ///- Read the packet header and decipher it (if needed)
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

        ///- Read the remaining of the packet
        WorldPacket packet((uint16)_cmd, _remaining);

        packet.resize(_remaining);
        if(_remaining) ibuf.Read((char*)packet.contents(), _remaining);
        _remaining = 0;

        ///- If log of world packets is enable, log the incoming packet
        if( sWorldLog.LogWorld() )
        {
            sWorldLog.Log("CLIENT:\nSOCKET: %u\nLENGTH: %u\nOPCODE: %s (0x%.4X)\nDATA:\n",
                (uint32)GetSocket(),
                packet.size(),
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

        ///- If thepacket is PING or AUTH_SESSION, handle immediately
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
                ///- Else, put it in the world session queue for this user (need to be already authenticated)
                if (_session)
                    _session->QueuePacket(packet);
                else
                    sLog.outDetail("Received out of place packet with cmdid 0x%.4X", _cmd);
            }
        }
    }
}

/// On socket closing
void WorldSocket::OnDelete()
{
    ///- Stop sending remaining data through this socket
    if (_session)
    {
        _session->SetSocket(0);
        /// Session deleted from World session list at socket==0, This is only back reference from socket to session.
        _session = 0;
    }

    ///- Remove the socket from the WorldSocketMgr list
    sWorldSocketMgr.RemoveSocket(this);
}

/// Handle the client authentication packet
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

    ///- Read the content of the packet
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
    sLog.outDebug("Auth: client %u, unk2 %u, account %s, clientseed %u", BuiltNumberClient, unk2, account.c_str(), clientSeed);

    ///- Get the account information from the realmd database
    std::string safe_account=account;                       // Duplicate, else will screw the SHA hash verification below
    loginDatabase.escape_string(safe_account);
    //No SQL injection, username escaped.
    QueryResult *result = loginDatabase.PQuery("SELECT `id`,`gmlevel`,`sessionkey`,`last_ip`,`locked`, `password`, `v`, `s`, `banned` FROM `account` WHERE `username` = '%s'", safe_account.c_str());

    ///- Stop if the account is not found
    if ( !result )
    {
        packet.Initialize( SMSG_AUTH_RESPONSE, 1 );
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
        packet.Initialize( SMSG_AUTH_RESPONSE, 1 );
        packet << uint8( AUTH_UNKNOWN_ACCOUNT );
        SendPacket( &packet );
        sLog.outDetail( "SOCKET: User not logged.");
        delete result;
        return;
    }

    /// Re-check ip locking (same check as in realmd).
    if((*result)[4].GetUInt8() == 1)                        // if ip is locked
    {
        if ( strcmp((*result)[3].GetString(),GetRemoteAddress().c_str()) )
        {
            packet.Initialize( SMSG_AUTH_RESPONSE, 1 );
            packet << uint8( REALM_AUTH_ACCOUNT_FREEZED );
            SendPacket( &packet );

            sLog.outDetail( "SOCKET: Sent Auth Response (Account IP differs)." );
            delete result;
            return;
        }
    }

    /// Re-check account ban (ame check as in realmd).
    if((*result)[8].GetUInt8() == 1)                        // if account banned
    {
        packet.Initialize( SMSG_AUTH_RESPONSE, 1 );
        packet << uint8( AUTH_BANNED );
        SendPacket( &packet );

        sLog.outDetail( "SOCKET: Sent Auth Response (Account banned)." );
        delete result;
        return;
    }

    id = (*result)[0].GetUInt32();
    security = (*result)[1].GetUInt16();
    K.SetHexStr((*result)[2].GetString());

    delete result;

    ///- Check that we do not exceed the maximum number of online players in the realm
    uint32 num = sWorld.GetSessionCount();
    if (sWorld.GetPlayerLimit() > 0 && num >= sWorld.GetPlayerLimit() && security == 0)
    {
        /// \todo Handle the waiting queue when the server is full
        SendAuthWaitQue(0);
        sLog.outDetail( "SOCKET: Sent Auth Response (server full)." );
        return;
    }

    ///- kick already loaded player with same account (if any) and remove session
    if(sWorld.RemoveSession(id))
    {
        sLog.outDetail( "SOCKET: Already connected - player kicked." );
    }

    ///- Check that Key and account name are the same on client and server
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

        packet.Initialize( SMSG_AUTH_RESPONSE, 1 );
        packet << uint8( AUTH_FAILED );

        SendPacket( &packet );

        sLog.outDetail( "SOCKET: Sent Auth Response (authentification failed)." );
        return;
    }

    ///- Initialize the encryption with the Key
    _crypt.SetKey(K.AsByteArray(), 40);
    _crypt.Init();

    ///- Send 'Auth is ok'
    packet.Initialize( SMSG_AUTH_RESPONSE, 1 );
    packet << uint8( AUTH_OK );
    //packet << uint8( 0xB0 ); - redundent
    //packet << uint8( 0x09 );
    //packet << uint8( 0x02 );
    //packet << uint8( 0x00 );
    //packet << uint8( 0x02 );
    //packet << uint32( 0x0 );

    SendPacket(&packet);

    ///- Create a new WorldSession for the player and add it to the World
    _session = new WorldSession(id, this,security);
    sWorld.AddSession(_session);

    sLog.outDebug( "SOCKET: Client '%s' authenticated successfully.", account.c_str() );
    sLog.outDebug( "Account: '%s' Login.", account.c_str() );

    ///- Update the last_ip in the database
    //No SQL injection, username escaped.
    loginDatabase.PQuery("UPDATE `account` SET `last_ip` = '%s' WHERE `username` = '%s'",GetRemoteAddress().c_str(), safe_account.c_str());

    // do small delay (10ms) at accepting successful authed connection to prevent droping packets by client
    // don't must harm anyone (let login ~100 accounts in 1 sec ;) )
    #ifdef WIN32
    Sleep(10);
    #endif

    ///- Create and send the Addon packet
    sAddOnHandler.BuildAddonPacket(&recvPacket, &SendAddonPacked, recvPacket.rpos());
    SendPacket(&SendAddonPacked);

    return;
}

/// Handle the Ping packet
void WorldSocket::_HandlePing(WorldPacket& recvPacket)
{
    uint32 ping;

    ///- Get the ping packet content
    try
    {
        recvPacket >> ping;
    }
    catch(ByteBuffer::error &)
    {
        sLog.outDetail("Incomplete ping packet");
        return;
    }

    ///- check ping speed for players
    if(_session && _session->GetSecurity() == 0)
    {
        uint32 cur_mstime = getMSTime();
        uint32 diff_mstime = cur_mstime - m_LastPingMSTime;
        m_LastPingMSTime = cur_mstime;
        if(diff_mstime < 27000)                             // should be 30000 (=30 secs), add little tolerance
        {
            ++m_OverSpeedPings;

            uint32 max_count = sWorld.getConfig(CONFIG_MAX_OVERSPEED_PINGS);
            if(max_count && m_OverSpeedPings > max_count)
            {
                sLog.outBasic("Player %s from account id %u kicked for overspeed ping packets from client (non-playable connection lags or cheating) ",_session->GetPlayerName(),_session->GetAccountId());
                _session->KickPlayer();
                return;
            }
        }
        else
            m_OverSpeedPings = 0;

    }

    ///- And put the pong answer in the to-be-sent queue
    WorldPacket packet( SMSG_PONG, 4 );
    packet << ping;
    SendPacket(&packet);

    return;
}

/// Handle the update order for the socket
void WorldSocket::Update(time_t diff)
{
    WorldPacket *packet;
    ServerPktHeader hdr;

    ///- While we have packets to send
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

        ///- Encrypt (if needed) the header
        _crypt.EncryptSend((uint8*)&hdr, 4);

        ///- Send the header and body to the client
        TcpSocket::SendBuf((char*)&hdr, 4);
        if(packet->size()) TcpSocket::SendBuf((char*)packet->contents(), packet->size());

        delete packet;
    }
}

/// Handle the authentication waiting queue (to be completed)
void WorldSocket::SendAuthWaitQue(uint32 PlayersInQue)
{
    WorldPacket packet( SMSG_AUTH_RESPONSE, 5 );
    packet << uint8( AUTH_WAIT_QUEUE );
    packet << uint32 (PlayersInQue);                        //amount of players in queue
    SendPacket(&packet);
}
