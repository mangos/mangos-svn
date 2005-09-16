/* AuthSocket.cpp
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
#include "Database/DatabaseEnv.h"
#include "ByteBuffer.h"
#include "Log.h"
#include "RealmList.h"
#include "AuthSocket.h"
#include "World.h"

#pragma pack(push, 1)

typedef struct
{
    uint8   cmd;
    uint8   error;                                // 0x00
    uint16  size;                                 // 0x0026
    uint8   gamename[4];                          // 'MaNGOS'
    uint8   version1;                             // 0x00
    uint8   version2;                             // 0x00 (0.0.1)
    uint8   version3;                             // 0x01
    uint16  build;                                // 3734
    uint8   platform[4];                          // 'x86'
    uint8   os[4];                                // 'Win'
    uint8   country[4];                           // 'enUS'
    uint32  timezone_bias;                        // -419
    uint32  ip;                                   // client ip
    uint8   I_len;                                // length of account name
    uint8   I[1];                                 // account name
} sAuthLogonChallenge_C;

typedef sAuthLogonChallenge_C sAuthReconnectChallenge_C;

typedef struct
{
    uint8   cmd;                                  // 0x00 CMD_AUTH_LOGON_CHALLENGE
    uint8   error;                                // 0 - ok
    uint8   unk2;                                 // 0x00
    uint8   B[32];
    uint8   g_len;                                // 0x01
    uint8   g[1];
    uint8   N_len;                                // 0x20
    uint8   N[32];
    uint8   s[32];
    uint8   unk3[16];
} sAuthLogonChallenge_S;

typedef struct
{
    uint8   cmd;                                  // 0x01
    uint8   A[32];
    uint8   M1[20];
    uint8   crc_hash[20];
    uint8   number_of_keys;
} sAuthLogonProof_C;

typedef struct
{
    uint16  unk1;
    uint32  unk2;
    uint8   unk3[4];
    uint16  unk4[20];                             // sha1(A,g,?)
}  sAuthLogonProofKey_C;

typedef struct
{
    uint8   cmd;                                  // 0x01 CMD_AUTH_LOGON_PROOF
    uint8   error;
    uint8   M2[20];
    uint32  unk2;
} sAuthLogonProof_S;

#pragma pack(pop)

AuthSocket::AuthSocket(SocketHandler &h) : TcpSocket(h)
{
    N.SetHexStr("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
    g.SetDword(7);
    _authed = false;
}


AuthSocket::~AuthSocket()
{
}


void AuthSocket::OnAccept()
{
    TcpSocket::OnAccept();

    sLog.outBasic("Accepting connection from '%s:%d'", GetRemoteAddress().c_str(), GetRemotePort());
    _cmd = AUTH_NO_CMD;

    s.SetRand(s_BYTE_SIZE * 8);
}


void AuthSocket::OnRead()
{
    TcpSocket::OnRead();

    while (1)
    {
        if (!ibuf.GetLength())
        {
            sLog.outDebug("No data");
            return;
        }

        // are we waiting for a new packet?
        if (_cmd == AUTH_NO_CMD)
            ibuf.SoftRead((char *)&_cmd, 1);
        else
            return;

        AuthHandler *table = _GetHandlerTable();
        int i;

        for (i = 0; table[i].handler != 0; i++)
        {
            if ((uint8)table[i].cmd == _cmd &&
                (table[i].status == STATUS_CONNECTED ||
                (_authed && table[i].status == STATUS_AUTHED)))
            {
                sLog.outDebug("[Auth] got data for cmd %u ibuf length %u", (uint32)_cmd, ibuf.GetLength());

                if(!(*this.*table[i].handler)())
                    return;

                break;
            }
        }

        if (table[i].handler == 0)
        {
            sLog.outDebug("[Auth] got unknown packet %u", (uint32)_cmd);
            _cmd = AUTH_NO_CMD;
            return;
        }
    }
}


bool AuthSocket::_HandleLogonChallenge()
{
    // not even header..
    if (ibuf.GetLength() < 4)
        return false;

    std::vector<uint8> buf;
    buf.resize(4);
    // got only packet header
    ibuf.Read((char *)&buf[0], 4);
    uint16 remaining = ((sAuthLogonChallenge_C *)&buf[0])->size;
    sLog.outDetail("[AuthChallenge] got header, body is %#04x bytes", remaining);

    if (ibuf.GetLength() < remaining)
        return false;

    // got full packet
    buf.resize(remaining + buf.size() + 1);
    // we want I zero-terminated
    buf[buf.size() - 1] = 0;
    sAuthLogonChallenge_C *ch = (sAuthLogonChallenge_C*)&buf[0];
    ibuf.Read((char *)&buf[4], remaining);
    sLog.outDebug("[AuthChallenge] got full packet, %#04x bytes", ch->size);
    sLog.outDebug("    I(%d): '%s'", ch->I_len, ch->I);

    // AuthChallenge
    ByteBuffer pkt;

    _login = (const char*)ch->I;

    enum _errors
    {
        CE_SUCCESS = 0,
        CE_WRONG_BUILDNUMBER,
        CE_SERVER_FULL,
        CE_NO_ACCOUNT,
        CE_ACCOUNT_CLOSED,
        CE_ACCOUNT_IN_USE,
        CE_PREORDER_TIME_LIMIT,
        CE_UPDATE_CLIENT,
        CE_ACCOUNT_FREEZED,
        CE_IPBAN
    } res;

    std::string password;

    // check for an accepted client version
    bool valid_version=false;
    int accepted_versions[]=EXPECTED_MANGOS_CLIENT_BUILD;
    for(int i=0;accepted_versions[i]&&(!valid_version);i++)
        if(ch->build==accepted_versions[i])
            valid_version=true;

    if(!valid_version)
    {
        res = CE_WRONG_BUILDNUMBER;
        pkt << (uint8) AUTH_LOGON_CHALLENGE;
        pkt << (uint8) 0x00;
        pkt << (uint8) 0x09;
    }
    else
    {
        // TODO: in the far future should check if the account is expired too
        std::stringstream sb;
        std::string ipb;
        ipb=GetRemoteAddress().c_str();
        sb << "SELECT * FROM `ipbantable` where ip='" << ipb << "'";
        QueryResult *result = sDatabase.Query( sb.str().c_str() );
        if(result)
        {
            res = CE_IPBAN;
            //Add ip here
            sLog.outBasic("Banned ip try to login!");
        }
        else
        {
            std::stringstream ss;
            // Deadknight Fix Start
            // ss << "SELECT password, gm FROM accounts WHERE login='" << _login << "'"; // password
            // password,banned
            ss << "SELECT password, gm, banned FROM accounts WHERE login='" << _login << "'";
            QueryResult *result = sDatabase.Query(ss.str().c_str());

            if( result )
            {
                int banned;
                banned = (*result)[2].GetUInt8(); //database banned field
                if(banned == 1)
                {
                    res = CE_ACCOUNT_CLOSED;
                    delete result;
                }
                else
                {
                    password = (*result)[0].GetString();

                    uint32 num = sWorld.GetSessionCount();
                    uint16 security = (*result)[1].GetUInt16();

                    if (sWorld.GetPlayerLimit() > 0 &&
                        num > sWorld.GetPlayerLimit() &&
                        security == 0)
                    {
                        res = CE_SERVER_FULL;
                    }
                    else
                    {
                        res = CE_SUCCESS;
                    }

                    delete result;
                }
            }
            else
            {
                res = CE_NO_ACCOUNT;
            }
        }
        // Finish fix

        pkt << (uint8) AUTH_LOGON_CHALLENGE;

        if (res != CE_SUCCESS)
        {
            switch (res)
            {
                case CE_ACCOUNT_CLOSED:
                    pkt << (uint8) 0x00;
                    pkt << (uint8) 0x03;
                    break;
                case CE_NO_ACCOUNT:
                    pkt << (uint8) 0x00;
                    pkt << (uint8) 0x04;
                    break;
                case CE_ACCOUNT_IN_USE:
                    pkt << (uint8) 0x00;
                    pkt << (uint8) 0x06;
                    break;
                case CE_PREORDER_TIME_LIMIT:
                    pkt << (uint8) 0x00;
                    pkt << (uint8) 0x07;
                    break;
                case CE_SERVER_FULL:
                    pkt << (uint8) 0x00;
                    pkt << (uint8) 0x08;
                    break;
                case CE_UPDATE_CLIENT:
                    pkt << (uint8) 0x00;
                    pkt << (uint8) 0x0A;
                    break;
                case CE_ACCOUNT_FREEZED:
                    pkt << (uint8) 0x00;
                    pkt << (uint8) 0x0C;
                    break;
                case CE_IPBAN:
                    pkt << (uint8) 0x00;
                    pkt << (uint8) 0x01;
                    break;
                default:
                    ASSERT(0);
            }
        }
        else
        {
            std::transform(password.begin(), password.end(), password.begin(), towupper);

            Sha1Hash I;
            std::string sI = _login + ":" + password;
            I.UpdateData(sI);
            I.Finalize();
            Sha1Hash sha;
            sha.UpdateData(s.AsByteArray(), s.GetNumBytes());
            sha.UpdateData(I.GetDigest(), 20);
            sha.Finalize();
            BigNumber x;
            x.SetBinary(sha.GetDigest(), sha.GetLength());
            v = g.ModExp(x, N);
            b.SetRand(19 * 8);
            BigNumber gmod=g.ModExp(b, N);
            B = ((v * 3) + gmod) % N;
            ASSERT(gmod.GetNumBytes() <= 32);

            BigNumber unk3;
            unk3.SetRand(16*8);

            pkt << (uint8)0;                      // no error
            pkt << (uint8)0;
            pkt.append(B.AsByteArray(), 32);
            pkt                                   // g_len, g
                << (uint8)1;
            pkt.append(g.AsByteArray(), 1);
            // N_len, N
            pkt << (uint8)32;
            pkt.append(N.AsByteArray(), 32);

            pkt.append(s.AsByteArray(), s.GetNumBytes());
            pkt.append(unk3.AsByteArray(), 16);
        }
    }

    _cmd = AUTH_NO_CMD;
    SendBuf((char *)pkt.contents(), pkt.size());

    if (res != CE_SUCCESS)
    {
        return false;
    }

    return true;
}


bool AuthSocket::_HandleLogonProof()
{
    if (ibuf.GetLength() < sizeof(sAuthLogonProof_C))
        return false;

    sLog.outDetail("[AuthLogonProof] checking...");

    sAuthLogonProof_C lp;
    ibuf.Read((char *)&lp, sizeof(sAuthLogonProof_C));

    BigNumber A;
    A.SetBinary(lp.A, 32);

    Sha1Hash sha;
    sha.UpdateBigNumbers(&A, &B, NULL);
    sha.Finalize();
    BigNumber u;
    u.SetBinary(sha.GetDigest(), 20);
    BigNumber S = (A * (v.ModExp(u, N))).ModExp(b, N);

    uint8 t[32];
    uint8 t1[16];
    uint8 vK[40];
    memcpy(t, S.AsByteArray(), 32);
    for (int i = 0; i < 16; i++)
    {
        t1[i] = t[i*2];
    }
    sha.Initialize();
    sha.UpdateData(t1, 16);
    sha.Finalize();
    for (int i = 0; i < 20; i++)
    {
        vK[i*2] = sha.GetDigest()[i];
    }
    for (int i = 0; i < 16; i++)
    {
        t1[i] = t[i*2+1];
    }
    sha.Initialize();
    sha.UpdateData(t1, 16);
    sha.Finalize();
    for (int i = 0; i < 20; i++)
    {
        vK[i*2+1] = sha.GetDigest()[i];
    }
    K.SetBinary(vK, 40);

    uint8 hash[20];

    sha.Initialize();
    sha.UpdateBigNumbers(&N, NULL);
    sha.Finalize();
    memcpy(hash, sha.GetDigest(), 20);
    sha.Initialize();
    sha.UpdateBigNumbers(&g, NULL);
    sha.Finalize();
    for (int i = 0; i < 20; i++)
    {
        hash[i] ^= sha.GetDigest()[i];
    }
    BigNumber t3;
    t3.SetBinary(hash, 20);

    sha.Initialize();
    sha.UpdateData(_login);
    sha.Finalize();
    BigNumber t4;
    t4.SetBinary(sha.GetDigest(), 20);

    sha.Initialize();
    sha.UpdateBigNumbers(&t3, &t4, &s, &A, &B, &K, NULL);
    sha.Finalize();
    BigNumber M;
    M.SetBinary(sha.GetDigest(), 20);

    if (!memcmp(M.AsByteArray(), lp.M1, 20))
    {
        sLog.outBasic("User '%s' successfully authed", _login.c_str());

        //saving session key to database
        std::stringstream ss;
        ss << "UPDATE accounts SET sessionkey = '";
        ss << K.AsHexStr();
        ss<< "' WHERE login='" << _login << "'";
        sDatabase.Execute( ss.str().c_str() );

        sha.Initialize();
        sha.UpdateBigNumbers(&A, &M, &K, NULL);
        sha.Finalize();

        sAuthLogonProof_S proof;
        memcpy(proof.M2, sha.GetDigest(), 20);
        proof.cmd = AUTH_LOGON_PROOF;
        proof.error = 0;
        proof.unk2 = 0;

        SendBuf((char *)&proof, sizeof(proof));

        _authed = true;

        _cmd = AUTH_NO_CMD;
        return true;
    }
    else
    {
        ByteBuffer pkt;

        pkt << (uint8) AUTH_LOGON_PROOF;
        pkt << (uint8) 4;                         // bad password

        SendBuf((char *)pkt.contents(), pkt.size());
        _cmd = AUTH_NO_CMD;

        sLog.outBasic("User '%s': invalid password", _login.c_str());

        return false;
    }
}


bool AuthSocket::_HandleRealmList()
{
    if (ibuf.GetLength() < 5)
        return false;

    sLog.outDetail("[RealmList]");

    ibuf.Remove(5);

    ByteBuffer pkt;

    std::stringstream ss;
    // password
    ss << "SELECT acct FROM accounts WHERE login='" << _login << "'";
    QueryResult *result = sDatabase.Query(ss.str().c_str());
    if(!result)                                   // we got an error.
    {
        Log::getSingleton().outError("[ERROR] user %s tried to login and we cant find him in the database.",_login.c_str());
        this->Close();
        return false;
    }
    ASSERT (result);                              // should be fine when we get here now, just incase.

    uint32 id = (*result)[0].GetUInt32();
    delete result;

    uint8 chars = 0;

    ss.rdbuf()->str("");
    ss << "SELECT guid FROM characters WHERE acct=" << id;
    result = sDatabase.Query( ss.str().c_str() );
    if( result )
    {
        do
        {
            chars++;
        }
        while( result->NextRow() );

        delete result;
    }

    pkt << (uint32) 0;
    pkt << (uint8) sRealmList.size();
    RealmList::RealmMap::const_iterator i;
    for( i = sRealmList.begin( ); i != sRealmList.end( ); i++ )
    {
        pkt << (uint32) i->second->icon;          // icon
        pkt << (uint8) i->second->color;
        pkt << i->first;
        pkt << i->second->address;
        pkt << (float) 1.6;                       // population value. lower == lower population and vice versa
        pkt << (uint8) chars;                     // number of characters on this server
        pkt << (uint8) i->second->timezone;
        pkt << (uint8) 0;                         // unknown
    }
    pkt << (uint8) 0;                             // unknown
    pkt << (uint8) 0x2;                           // unknown

    ByteBuffer hdr;
    hdr << (uint8) REALM_LIST;
    hdr << (uint16)pkt.size();
    hdr.append(pkt);

    SendBuf((char *)hdr.contents(), hdr.size());

    _cmd = AUTH_NO_CMD;

    return true;
}


bool AuthSocket::_HandleXferInitiate()
{
    sLog.outDebug("[XferInitiate]");
    _cmd = AUTH_NO_CMD;
    return true;
}


bool AuthSocket::_HandleXferData()
{
    sLog.outDebug("[XferData]");
    _cmd = AUTH_NO_CMD;
    return true;
}


AuthSocket::AuthHandler* AuthSocket::_GetHandlerTable() const
{
    static AuthHandler table[] =
    {
        { AUTH_LOGON_CHALLENGE,     STATUS_CONNECTED, &AuthSocket::_HandleLogonChallenge          },
        { AUTH_LOGON_PROOF,         STATUS_CONNECTED, &AuthSocket::_HandleLogonProof              },
        { REALM_LIST,               STATUS_AUTHED,    &AuthSocket::_HandleRealmList               },
        { XFER_INITIATE,            STATUS_AUTHED,    &AuthSocket::_HandleXferInitiate            },
        { XFER_DATA,                STATUS_AUTHED,    &AuthSocket::_HandleXferData                },
        { AUTH_NO_CMD,              0,                0                                           }
    };

    return table;
}
