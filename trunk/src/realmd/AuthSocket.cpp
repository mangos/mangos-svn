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
#include "Database/DatabaseEnv.h"
#include "ByteBuffer.h"
#include "Log.h"
#include "RealmList.h"
#include "AuthSocket.h"
#include <cwctype>                                          // needs for towupper

const AuthHandler table[] =
{
    { AUTH_LOGON_CHALLENGE,     STATUS_CONNECTED, &AuthSocket::_HandleLogonChallenge},
    { AUTH_LOGON_PROOF,         STATUS_CONNECTED, &AuthSocket::_HandleLogonProof    },
    { REALM_LIST,               STATUS_AUTHED,    &AuthSocket::_HandleRealmList     },
    { XFER_ACCEPT,              STATUS_CONNECTED, &AuthSocket::_HandleXferAccept    },
    { XFER_RESUME,              STATUS_CONNECTED, &AuthSocket::_HandleXferResume    },
    { XFER_CANCEL,              STATUS_CONNECTED, &AuthSocket::_HandleXferCancel    }
};

#define AUTH_TOTAL_COMMANDS sizeof(table)/sizeof(AuthHandler)

Patcher PatchesCache;

PatcherRunnable::PatcherRunnable(class AuthSocket * as)
{
    mySocket=as;
}

AuthSocket::AuthSocket(SocketHandler &h) : TcpSocket(h)
{
    N.SetHexStr("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
    g.SetDword(7);
    _authed = false;
    pPatch=NULL;
}

AuthSocket::~AuthSocket()
{

    if(pPatch)
        fclose(pPatch);

}

void AuthSocket::OnAccept()
{
    sLog.outBasic("Accepting connection from '%s:%d'",
        GetRemoteAddress().c_str(), GetRemotePort());

    s.SetRand(s_BYTE_SIZE * 8);
}

void AuthSocket::OnRead()
{
    TcpSocket::OnRead();
    uint8 _cmd;
    while (1)
    {
        if (!ibuf.GetLength())
            return;

        ibuf.SoftRead((char *)&_cmd, 1);
        int i;

        for (i=0;i<AUTH_TOTAL_COMMANDS; i++)
        {
            if ((uint8)table[i].cmd == _cmd &&
                (table[i].status == STATUS_CONNECTED ||
                (_authed && table[i].status == STATUS_AUTHED)))
            {
                DEBUG_LOG("[Auth] got data for cmd %u ibuf length %u", (uint32)_cmd, ibuf.GetLength());

                (*this.*table[i].handler)();
                break;
                // return;
            }
        }

        if (i==AUTH_TOTAL_COMMANDS)
        {

            DEBUG_LOG("[Auth] got unknown packet %u", (uint32)_cmd);
            return;
        }

    }
}

void AuthSocket::_HandleLogonChallenge()
{

    if (ibuf.GetLength() < 4)
        return ;

    std::vector<uint8> buf;
    buf.resize(4);

    ibuf.Read((char *)&buf[0], 4);
    uint16 remaining = ((sAuthLogonChallenge_C *)&buf[0])->size;
    DEBUG_LOG("[AuthChallenge] got header, body is %#04x bytes", remaining);

    if (ibuf.GetLength() < remaining)
        return ;

    buf.resize(remaining + buf.size() + 1);

    buf[buf.size() - 1] = 0;
    sAuthLogonChallenge_C *ch = (sAuthLogonChallenge_C*)&buf[0];
    ibuf.Read((char *)&buf[4], remaining);
    DEBUG_LOG("[AuthChallenge] got full packet, %#04x bytes", ch->size);
    DEBUG_LOG("[AuthChallenge] name(%d): '%s'", ch->I_len, ch->I);

    ByteBuffer pkt;

    _login = (const char*)ch->I;

    enum _errors
    {
        CE_SUCCESS = 0,
        CE_IPBAN=0x01,                                      //2bd -- unable to connect (some internal problem)
        CE_ACCOUNT_CLOSED=0x03,                             // "This account has been closed and is no longer in service -- Please check the registered email address of this account for further information.";
        CE_NO_ACCOUNT=0x04,                                 //(5)The information you have entered is not valid.  Please check the spelling of the account name and password.  If you need help in retrieving a lost or stolen password and account
        CE_ACCOUNT_IN_USE=0x06,                             //This account is already logged in.  Please check the spelling and try again.
        CE_PREORDER_TIME_LIMIT=0x07,
        CE_SERVER_FULL=0x08,                                //Could not log in at this time.  Please try again later.
        CE_WRONG_BUILD_NUMBER=0x09,                         //Unable to validate game version.  This may be caused by file corruption or the interference of another program.
        CE_UPDATE_CLIENT=0x0a,
        CE_ACCOUNT_FREEZED=0x0c
    } ;

    std::string password;

    bool valid_version=false;
    int accepted_versions[]=EXPECTED_MANGOS_CLIENT_BUILD;
    for(int i=0;accepted_versions[i];i++)
        if(ch->build==accepted_versions[i])
    {
        valid_version=true;
        break;
    }

    if(valid_version)
    {
        pkt << (uint8) AUTH_LOGON_CHALLENGE;
        pkt << (uint8) 0x00;

        QueryResult *result = dbRealmServer.PQuery(  "SELECT * FROM `ip_banned` WHERE `ip` = '%s'",GetRemoteAddress().c_str());
        if(result)
        {
            pkt << (uint8)CE_IPBAN;
            sLog.outBasic("[AuthChallenge] Banned ip %s try to login!",GetRemoteAddress().c_str ());
            delete result;
        }
        else
        {
            QueryResult *result = dbRealmServer.PQuery("SELECT `password`,`gmlevel`,`banned`,`locked`,`last_ip` FROM `account` WHERE `username` = '%s'",_login.c_str ());
            if( result )
            {
                if((*result)[3].GetUInt8() == 1)            // if ip is locked
                {
                    DEBUG_LOG("[AuthChallenge] Account '%s' is locked to IP - '%s'", _login.c_str(), (*result)[4].GetString());
                    DEBUG_LOG("[AuthChallenge] Player address is '%s'", GetRemoteAddress().c_str());
                    if ( strcmp((*result)[4].GetString(),GetRemoteAddress().c_str()) )
                    {
                        DEBUG_LOG("[AuthChallenge] Account IP differs");
                        pkt << (uint8) CE_ACCOUNT_CLOSED;
                    }
                    else
                    {
                        DEBUG_LOG("[AuthChallenge] Account IP matches");
                    }

                }
                else
                {
                    DEBUG_LOG("[AuthChallenge] Account '%s' is not locked to ip", _login.c_str());
                }

                if((*result)[2].GetUInt8())                 //if banned
                {
                    pkt << (uint8) CE_ACCOUNT_CLOSED;
                    sLog.outBasic("[AuthChallenge] Banned account %s try to login!",_login.c_str ());
                }
                else
                {
                    password = (*result)[0].GetString();
 /*                   QueryResult *result =  .PQuery("SELECT COUNT(*) FROM `account` WHERE `account`.`online` > 0 AND `login`.`gmlevel` = 0;");
                    uint32 cnt=0;
                    if(result)
                    {
                        Field *fields = result->Fetch();
                        cnt = fields[0].GetUInt32();
                        delete result;

                    }
                                                            //number of not-gm players online
                    if(cnt>=sConfig.GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT))
                        pkt << (uint8)CE_SERVER_FULL;

                    else 
                    {    */                                   //if server is not full

                        uint32 acct;
                        QueryResult *resultAcct = dbRealmServer.PQuery("SELECT `id` FROM `account` WHERE `username` = '%s';", _login.c_str ());
                        if(resultAcct)
                        {
                            Field *fields = resultAcct->Fetch();
                            acct=fields[0].GetUInt32();
                            delete resultAcct;

                            QueryResult *result = dbRealmServer.PQuery("SELECT * FROM `account` WHERE `online` > 0 AND `account` = '%u';",acct);
                            if(result)
                            {
                                delete result;
                                pkt << (uint8)CE_ACCOUNT_IN_USE;

                            }
                            else
                            {

                                std::transform(password.begin(), password.end(), password.begin(), std::towupper);

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

                                //pkt << (uint8)0;
                                pkt << (uint8)0;
                                pkt.append(B.AsByteArray(), 32);
                                pkt << (uint8)1;
                                pkt.append(g.AsByteArray(), 1);

                                pkt << (uint8)32;
                                pkt.append(N.AsByteArray(), 32);

                                pkt.append(s.AsByteArray(), s.GetNumBytes());
                                pkt.append(unk3.AsByteArray(), 16);
                            }
                        }
                //    }
                }
                delete result;
            }
            else                                            //no account
            {
                pkt<< (uint8) CE_NO_ACCOUNT;
            }
        }                                                   //ip is not banned

    }else
    {

        //Check if we have apropriate patch
        char tmp[64];
        sprintf(tmp,"./patches/%d%c%c%c%c.mpq",ch->build,ch->country[3],
            ch->country[2],ch->country[1],ch->country[0]);
        FILE *pFile=fopen(tmp,"rb");
        if(!pFile)
        {
            //printf("......patch not found");
            pkt << (uint8) AUTH_LOGON_CHALLENGE;
            pkt << (uint8) 0x00;
            pkt << (uint8) CE_WRONG_BUILD_NUMBER;
            DEBUG_LOG("[AuthChallenge] %u is not a valid client version!", ch->build);
        }else
        {                                                   //have patch
            pPatch=pFile;
            XFER_INIT xferh;

            if(PatchesCache .GetHash(tmp,(uint8*)&xferh .md5 ))
            {
                DEBUG_LOG("\n[AuthChallenge] Found precached patch info.");

            }
            else
            {                                               //calculate patch md5
                printf("\n[AuthChallenge] Patch info for %s was not cached.",tmp);
                PatchesCache.LoadPatchMD5(tmp);
                PatchesCache.GetHash(tmp,(uint8*)&xferh .md5 );
            }

            uint8 data[2]={AUTH_LOGON_PROOF,CE_UPDATE_CLIENT};
            SendBuf((const char*)data,sizeof(data));

            memcpy(&xferh,"0\x05Patch",7);
            xferh.cmd=XFER_INITIATE;
            fseek(pPatch,0,2);
            xferh.file_size=ftell(pPatch);

            SendBuf((const char*)&xferh,sizeof(xferh));
            return;
        }
    }
    SendBuf((char *)pkt.contents(), pkt.size());

}

void AuthSocket::_HandleLogonProof()
{
    if (ibuf.GetLength() < sizeof(sAuthLogonProof_C))
        return ;

    DEBUG_LOG("[AuthLogonProof] checking...");

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

        dbRealmServer.PExecute("UPDATE `account` SET `sessionkey` = '%s', `last_ip` = '%s', `last_login` = NOW() WHERE `username` = '%s'",K.AsHexStr(), GetRemoteAddress().c_str(), _login.c_str() );
        
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

    }
    else
    {
        char data[2]={AUTH_LOGON_PROOF,4};

        SendBuf(data,sizeof(data));
    }
}

void AuthSocket::_HandleRealmList()
{
    if (ibuf.GetLength() < 5)
        return ;

    sLog.outDetail("[RealmList]");

    ibuf.Remove(5);

    ByteBuffer pkt;

    QueryResult *result = dbRealmServer.PQuery("SELECT `id` FROM `account` WHERE `username` = '%s'",_login.c_str());
    if(!result)
    {
        sLog.outError("[ERROR] user %s tried to login and we cant find him in the database.",_login.c_str());
        this->Close();
        return;
    }
    ASSERT (result);

    uint32 id = (*result)[0].GetUInt32();
    delete result;

    uint8 chars = 0;

    pkt << (uint32) 0;
    pkt << (uint8) m_realmList.size();
    RealmList::RealmMap::const_iterator i;
    for( i = m_realmList.begin( ); i != m_realmList.end( ); i++ )
    {
        pkt << (uint32) i->second->icon;
        pkt << (uint8) i->second->color;
        pkt << i->first;
        pkt << i->second->address;
        pkt << (float) 1.6;
        //result = i->second->dbRealm.PQuery( "SELECT COUNT(*) FROM `character` WHERE `account` = %d",id);
        result = dbRealmServer.PQuery( "SELECT `numchars` FROM `realmcharacters` WHERE `acctid` = %d AND `realmid` = %d",id,i->second->m_ID);
        if( result )
        {
            Field *fields = result->Fetch();
            chars = fields[0].GetUInt8();

            delete result;
        } else {
            chars = 0;
        }
        pkt << (uint8) chars;
        pkt << (uint8) i->second->timezone;
        pkt << (uint8) 0;
    }
    pkt << (uint8) 0;
    pkt << (uint8) 0x2;

    ByteBuffer hdr;
    hdr << (uint8) REALM_LIST;
    hdr << (uint16)pkt.size();
    hdr.append(pkt);

    SendBuf((char *)hdr.contents(), hdr.size());

}

void AuthSocket::_HandleXferResume()
{
    //	printf("\ngot xfer resume");
    if (ibuf.GetLength ()<9)
    {
        //printf("Wrong packet XFER_RESUME");
        return;
    }

    uint64 start;
    ibuf.Remove(1);
    ibuf.Read((char*)&start,sizeof(start));
    fseek(pPatch,start,0);

    ZThread::Thread u(new PatcherRunnable(this));

}

void AuthSocket::_HandleXferCancel()
{

    ibuf.Remove(1);                                         //clear input buffer

    //printf("\ngot xfer cancel");

    //ZThread::Thread::sleep(15);
    SetCloseAndDelete ();

}

void AuthSocket::_HandleXferAccept()
{
    ibuf.Remove(1);                                         //clear input buffer
    //printf("\ngot xfer accept");
    fseek(pPatch,0,0);

    ZThread::Thread u(new PatcherRunnable(this));

}

bool AuthSocket::IsLag()
{
    return (TCP_BUFSIZE_READ-obuf.GetLength ()< 2*ChunkSize);

}

void PatcherRunnable::run()
{
    XFER_DATA_STRUCT xfdata;                                //=new XFER_DATA_STRUCT;
    xfdata.opcode =XFER_DATA;

    while(!feof(mySocket->pPatch) && mySocket->Ready () )
    {
        while(mySocket->Ready() && mySocket->IsLag ())
        {
            //Sleep(1);
            ZThread::Thread::sleep(1);
        }
        xfdata.data_size=fread(&xfdata.data,1,ChunkSize,mySocket->pPatch);
        mySocket->SendBuf((const char*)&xfdata,xfdata.data_size +(sizeof(XFER_DATA_STRUCT)-ChunkSize));

    }

    //delete [] xfdata;

}

#ifndef _WIN32
#include <sys/dir.h>
#include <errno.h>
void Patcher ::LoadPatchesInfo()
{
    DIR * dirp;
    //int errno;
    struct dirent * dp;
    dirp = opendir("./patches/");
    if(!dirp)
        return;
    while (dirp)
    {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL)
        {
            int l=strlen(dp->d_name);
            if(l<8)continue;
            if(!memcmp(&dp->d_name[l-4],".mpq",4))
                LoadPatchMD5(dp->d_name);
        }
        else
        {
            if(errno != 0)
            {
                closedir(dirp);
                return;
            }
            break;
        }
    }

    if(dirp)
        closedir(dirp);
}

#else

void Patcher :: LoadPatchesInfo()
{
    WIN32_FIND_DATA fil;
    HANDLE hFil=FindFirstFile("./patches/*.mpq",&fil);
    if(hFil==INVALID_HANDLE_VALUE)return;                   //no pathces were found
    LoadPatchMD5(fil.cFileName);

    while(FindNextFile(hFil,&fil))
        LoadPatchMD5(fil.cFileName);
}
#endif

void Patcher::LoadPatchMD5(char * szFileName)
{
    char path[128]="./patches/";
    strcat(path,szFileName);
    FILE * pPatch=fopen(path,"rb");
    printf("\nLoading patch info from %s\n",path);
    if(!pPatch)
    {
        printf("Error loading patch.");
        return;
    }
    MD5_CTX ctx;
    MD5_Init(&ctx);
    uint8 * buf= new uint8 [512*1024];

    while (!feof(pPatch))
    {
        size_t read = fread(buf, 1, 512*1024, pPatch);
        MD5_Update(&ctx, buf, read);
    }
    delete [] buf;
    fclose(pPatch);
    _patches[ path] = new PATCH_INFO;
    MD5_Final((uint8 *)&_patches[path]->md5 , &ctx);
}

bool Patcher::GetHash(char * pat,uint8 mymd5[16])
{
    for( Patches::iterator i = _patches.begin(); i != _patches.end(); i++ )
        if(!stricmp(pat,i->first.c_str () ))
    {
        memcpy(mymd5,i->second->md5,16);
        return true;
    }

    return false;
}

Patcher::Patcher()
{
    LoadPatchesInfo();
}

Patcher::~Patcher()
{
    for(Patches::iterator i = _patches.begin(); i != _patches.end(); i++ )
        delete i->second;

}
