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


#include "Database/DatabaseEnv.h"
#include "ByteBuffer.h"
#include "Log.h"
#include "RealmList.h"
#include "AuthCodes.h"

#include "RealmAcceptor.h"
#include "RealmHandler.h"


RealmHandler::RealmHandler ()
	: flg_mask_ (ACE_Event_Handler::NULL_MASK), 
	ibuf(TCP_BUFSIZE_READ),
	obuf(TCP_BUFSIZE_READ),
	pPatch(NULL),
	_authed(false)
{
	ACE_DEBUG((LM_DEBUG, "RealmHandler\n"));

	N.SetHexStr("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
    g.SetDword(7);
}

RealmHandler::~RealmHandler ()
{
	ACE_DEBUG((LM_DEBUG, "~RealmHandler\n"));
	this->reactor (0);

	if(pPatch)
		fclose(pPatch);
}

int
RealmHandler::check_destroy (void)
{
  if (flg_mask_ == ACE_Event_Handler::NULL_MASK)
    return -1;

  return 0;
}

int
RealmHandler::open (void *_acceptor)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> locker (mutex_);

	ACE_DEBUG((LM_DEBUG, "open \n"));

	RealmAcceptor *acceptor = (RealmAcceptor *) _acceptor;

	this->reactor (acceptor->reactor ());

	ACE_INET_Addr addr;

	if (this->peer ().get_remote_addr (addr) == -1)
		return -1;
	
	flg_mask_ = ACE_Event_Handler::NULL_MASK ;

	s.SetRand(s_BYTE_SIZE * 8);

	if (this->reactor ()->register_handler (this, flg_mask_) == -1)
		ACE_ERROR_RETURN ((LM_ERROR,
			"(%P|%t) can't register with reactor\n"),
            -1);

	initiate_io (ACE_Event_Handler::READ_MASK);

	ACE_DEBUG ((LM_DEBUG,
              "(%P|%t) connected with %s\n",
              addr.get_host_name ()));

	return check_destroy();
}

int
RealmHandler::initiate_io (ACE_Reactor_Mask mask)
{
  if (ACE_BIT_ENABLED (flg_mask_, mask))
    return 0;

  if (this->reactor ()->schedule_wakeup  (this, mask) == -1)
    return -1;

  ACE_SET_BITS (flg_mask_, mask);
  return 0;
}

int
RealmHandler::terminate_io (ACE_Reactor_Mask mask)
{
  if (ACE_BIT_DISABLED (flg_mask_, mask))
    return 0;

  if (this->reactor ()->cancel_wakeup (this, mask) == -1)
    return -1;

  ACE_CLR_BITS (flg_mask_, mask);
  return 0;
}

int 
RealmHandler::read_input (void)
{
	int err = 0;
	char buf[TCP_BUFSIZE_READ];
	ssize_t res = this->peer ().recv(buf, TCP_BUFSIZE_READ);
	
	if (res >= 0)
    {
		this->ibuf.Write(buf, res);
    }
	else
		err = errno ;

	if (err == EWOULDBLOCK)
    {
		err=0;
		res=0;
		return check_destroy ();
    }

	if (err !=0  || res <= 0)
    {
		return -1;
    }
	return 0;
}


int
RealmHandler::handle_input (ACE_HANDLE handle /* = ACE_INVALID_HANDLE */)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> locker (mutex_);

	ACE_DEBUG((LM_DEBUG, "handle_input \n"));

	if( read_input() == -1)
	{
		return -1;
	}

	uint8 _cmd;
	while(1)
	{
		int result = 0;
		if (!ibuf.GetLength())
			break;

		ibuf.SoftRead((char*)&_cmd,1);
		ACE_DEBUG((LM_DEBUG, "[AuthChallenge]: got data for cmd %u ibuf length %u\n",_cmd, ibuf.GetLength()));

		switch(_authed) {
			case STATUS_NOTAUTHED:
				switch(_cmd) {
					case AUTH_LOGON_CHALLENGE:
						result = _HandleLogonChallenge();
						break;
					case AUTH_LOGON_PROOF:
						result = _HandleLogonProof();
						break;
					case XFER_ACCEPT:
						_HandleXferAccept();
						break;
					case XFER_RESUME:
						_HandleXferResume();
						break;
					case XFER_CANCEL:
						result= _HandleXferCancel();
						break;
					default:
						ACE_DEBUG((LM_DEBUG, "[AuthChallenge]: unknow packet.\n"));
						result = -1;
						break;
				}
				break;
			case STATUS_AUTHED:
				switch(_cmd) {
					case REALM_LIST:
						result = _HandleRealmList();
						break;
					default:
						ACE_DEBUG((LM_DEBUG, "[AuthChallenge]: unknow packet.\n"));
						result = -1;
						break;
				}
				break;
			default:
				break;
		}

		if (result == -1)
		{
			return -1 ;
		}
	}
	return check_destroy ();
}

int
RealmHandler::handle_close (ACE_HANDLE handle,
                              ACE_Reactor_Mask mask)
{
	this->reactor ()->remove_handler (this,
                             ACE_Event_Handler::ALL_EVENTS_MASK |
                             ACE_Event_Handler::DONT_CALL);  // Don't call handle_close
	this->reactor (0);
	this->destroy ();
	return 0;
}

int
RealmHandler::_HandleLogonChallenge()
{
	ACE_TRACE("ReadHandler::_HandleLogonChallenge()");

	ACE_DEBUG((LM_DEBUG, "HandleLogonChallenge\n"));

	if (ibuf.GetLength() < 4)
        return 0;

    std::vector<uint8> buf;
    buf.resize(4);
	ibuf.Read((char*)&buf[0],4);
    uint16 remaining = ((sAuthLogonChallenge_C *)&buf[0])->size;
    ACE_DEBUG((LM_DEBUG, "[AuthChallenge]: got header, body is %#04x bytes\n", remaining));

    if (ibuf.GetLength() < remaining)
        return 0;

    buf.resize(remaining + buf.size() + 1);
    
    buf[buf.size() - 1] = 0;
    sAuthLogonChallenge_C *ch = (sAuthLogonChallenge_C*)&buf[0];
	ibuf.Read((char*)&buf[4], remaining);
    ACE_DEBUG((LM_DEBUG, "[AuthChallenge]: got full packet, %#04x bytes\n", ch->size));
	ACE_DEBUG((LM_DEBUG, "    I(%d): '%s'\n", ch->I_len, ch->I));
	ACE_DEBUG((LM_DEBUG, "    Build: '%d'\n", ch->build));

	ByteBuffer pkt;

    _login = (const char*)ch->I;

    
					
    bool valid_version = false;

    int accepted_versions[] = EXPECTED_MANGOS_CLIENT_BUILD;

    for(int i=0; accepted_versions[i]; i++)
	{
        if(ch->build == accepted_versions[i])
		{
            valid_version = true;
			break;
		}
	}
	
	if(valid_version)
    {
		pkt << (uint8) AUTH_LOGON_CHALLENGE;
        pkt << (uint8) 0x00;
		
		ACE_INET_Addr addr;
		this->peer().get_remote_addr(addr);

        QueryResult *result = stDatabaseMysql.PQuery( "SELECT * FROM `ip_banned` WHERE `ip` = '%s'", addr.get_host_addr());

        if(result)
        {
	        pkt << (uint8)AUTH_BANNED;
            ACE_DEBUG((LM_DEBUG, "[AuthChallenge]: Banned ip %s try to login!\n.", addr.get_host_addr()));
			delete result;
        }
        else
        {
            QueryResult *result = stDatabaseMysql.PQuery("SELECT `password`,`gmlevel`,`banned`,`locked`,`last_ip` FROM `account` WHERE `username` = '%s'",_login.c_str ());

            if( result )
            {
				if((*result)[3].GetUInt8() == 1) // if ip is locked
				{
					ACE_DEBUG((LM_DEBUG, "[AuthChallenge]: Account '%s' is locked to IP - '%s'.\n", _login.c_str(), (*result)[4].GetString()));
					ACE_DEBUG((LM_DEBUG, "[AuthChallenge]: Player address is '%s'.\n", addr.get_host_addr()));

					if ( strcmp((*result)[4].GetString(),addr.get_host_addr()) )
					{
						ACE_DEBUG((LM_DEBUG, "[AuthChallenge]: Account IP differs.\n"));
						pkt << (uint8) AUTH_SUSPENDED;
					} else {
						ACE_DEBUG((LM_DEBUG, "[AuthChallenge]: Account IP matches.\n"));
					}

				} else {
					ACE_DEBUG((LM_DEBUG, "[AuthChallenge]: Account '%s' is not locked to ip\n", _login.c_str()));
				}

                if((*result)[2].GetUInt8())//if banned
                {
					pkt << (uint8) AUTH_BANNED;
				    ACE_DEBUG((LM_DEBUG, "[AuthChallenge]: Banned account %s try to login!\n",_login.c_str ()));
		           
                }
                else
                {
                    std::string password = (*result)[0].GetString();

					/*QueryResult *result = sDatabase.PQuery("SELECT COUNT(*) FROM characters c,accounts a WHERE c.online > 0 and a.gm = 0;");
					uint32 cnt=0;
					if(result)
					{
						Field *fields = result->Fetch();
						cnt = fields[0].GetUInt32();
						delete result;
						
					}
					if(cnt >= sConfig->GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT))//number of not-gm players online
					{
						ACE_DEBUG((LM_DEBUG, ACE_TEXT("SERVER is full.\n")));
						pkt << (uint8)CE_SERVER_FULL;

						add to queue ;
						void WorldSocket::SendAuthWaitQue(uint32 PlayersInQue){ WorldPacket packet; packet.Initialize( SMSG_AUTH_RESPONSE ); packet << uint8( AUTH_WAIT_QUEUE ); packet << uint32 (0x00); //unknown packet << uint32 (0x00); //unknown packet << uint8 (0x00); //unknown packet << uint32 (PlayersInQue); //amount of players in que SendPacket(&packet);}
					}
					else 
					{*/	//if server is not full
						
						ACE_DEBUG((LM_DEBUG, ACE_TEXT("[AuthChallenge]: SERVER is not full.\n")));

                        uint32 acct;
                        QueryResult *resultAcct = stDatabaseMysql.PQuery("SELECT `id` FROM `account` WHERE `username` = '%s';", _login.c_str ());
						if(resultAcct)
						{
							Field *fields = resultAcct->Fetch();
							acct = fields[0].GetUInt32();
							delete resultAcct;
						    
							QueryResult *result = stDatabaseMysql.PQuery("SELECT * FROM `account` WHERE `online` > 0 AND `id` = '%u';",acct);
						    if(result)
						    {
								ACE_DEBUG((LM_DEBUG, ACE_TEXT("[AuthChallenge]: Account in use.\n")));
							    delete result;
							   //TODO FIX THIS THIS IS NOT CORRECT
								pkt << (uint8)AUTH_ALREADY_LOGGING_IN;
						    }
						    else
						    {
								ACE_DEBUG((LM_DEBUG, ACE_TEXT("[AuthChallenge]: Account not in use.\n")));

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
						//}
					}
                }
				delete result;
            }
            else //no account
            {
				ACE_DEBUG((LM_DEBUG, ACE_TEXT("[AuthChallenge]: ACCOUNT '%s' doesn't exist.\n"), _login.c_str ()));
				pkt << (uint8) AUTH_UNKNOWN_ACCOUNT;
            }
		}
		//ip is not banned
	}else {		   
		//Check if we have apropriate patch
		char tmp[64];
		sprintf(tmp,"./patches/%d%c%c%c%c.mpq",ch->build,ch->country[3],
			ch->country[2],ch->country[1],ch->country[0]);
		FILE *pFile = fopen(tmp,"rb");
		if(!pFile) {
			
			ACE_DEBUG((LM_DEBUG, ACE_TEXT("[AuthChallenge]: patch %s not found.\n"), tmp));

			ACE_DEBUG((LM_INFO, ACE_TEXT("[AuthChallenge] %u is not a valid client version!"), ch->build));

			pkt << (uint8) AUTH_LOGON_CHALLENGE;
			pkt << (uint8) 0x00;
			pkt << (uint8) AUTH_VERSION_MISMATCH;
			

		} else {

			pPatch = pFile;
			XFER_INIT xferh;

			if(sPatcher->GetHash(tmp,(uint8*)&xferh .md5 ))
			{
				ACE_DEBUG((LM_DEBUG, "Found precached patch info.\n"));
			
			}
			else
			{
				ACE_DEBUG((LM_DEBUG, ACE_TEXT("Patch info for %s was not cached.\n"),tmp));

				sPatcher->LoadPatchMD5(tmp);
				sPatcher->GetHash(tmp,(uint8*)&xferh .md5 );
			}

			uint8 data[2] = { AUTH_LOGON_PROOF, CSTATUS_NEGOTIATION_FAILED };
			int send = this->peer ().send ((char*)data, sizeof(data));
			if(send == -1)
			{
				return -1;
			}

			ACE_OS::memcpy(&xferh,"0\x05Patch",7);
			xferh.cmd = XFER_INITIATE;
			fseek(pPatch,0,2);
			xferh.file_size = ftell(pPatch);
			return this->peer ().send ((char*)&xferh, sizeof(xferh));
		}
	}
	return this->peer ().send ((char *)pkt.contents(), pkt.size());
}

int
RealmHandler::_HandleLogonProof()
{
	ACE_DEBUG((LM_DEBUG, "_HandleLogonProof\n"));

	if (ibuf.GetLength() < sizeof(sAuthLogonProof_C))
        return 0;
	
	ACE_DEBUG((LM_DEBUG, "[AuthLogonProof] checking...\n"));

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
    ACE_OS::memcpy(t, S.AsByteArray(), 32);
    for (int i = 0; i < 16; i++) {
        t1[i] = t[i*2];
    }
    sha.Initialize();
    sha.UpdateData(t1, 16);
    sha.Finalize();
    for (int i = 0; i < 20; i++) {
        vK[i*2] = sha.GetDigest()[i];
    }
    for (int i = 0; i < 16; i++) {
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
    ACE_OS::memcpy(hash, sha.GetDigest(), 20);
    sha.Initialize();
    sha.UpdateBigNumbers(&g, NULL);
    sha.Finalize();
    for (int i = 0; i < 20; i++) {
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

    if (!ACE_OS::memcmp(M.AsByteArray(), lp.M1, 20)) {

        ACE_DEBUG((LM_DEBUG, "[AuthLogonProof]: User '%s' successfully authed.\n", _login.c_str()));
		
		ACE_INET_Addr addr;
		this->peer().get_remote_addr(addr);
		uint32 sqlEx = stDatabaseMysql.PExecute("UPDATE `account` SET `sessionkey` = '%s', `last_ip` = '%s', `last_login` = NOW() WHERE `username` = '%s';",
			K.AsHexStr(), 
			addr.get_host_addr(), 
			_login.c_str() );
		if ( sqlEx ) {
			sha.Initialize();
			sha.UpdateBigNumbers(&A, &M, &K, NULL);
			sha.Finalize();

			sAuthLogonProof_S proof;
			memcpy(proof.M2, sha.GetDigest(), 20);
			proof.cmd = AUTH_LOGON_PROOF;
			proof.error = 0;
			proof.unk2 = 0;

			_authed = true;
		
			return this->peer ().send ((char *)&proof, sizeof(proof));

		} else {
			_authed = false;

			ByteBuffer pkt;
			pkt << (uint8) AUTH_LOGON_CHALLENGE;
			pkt << (uint8) 0x00;
			pkt << (uint8) AUTH_DB_BUSY;
			
			return this->peer ().send ((char*)pkt.contents(), pkt.size());

		}
    }
   
	char data[2] = { AUTH_LOGON_PROOF, 4 };
	
	return this->peer ().send (data, sizeof(data));
}

int 
RealmHandler::_HandleRealmList()
{
	ACE_TRACE("ReadHandler::_HandleRealmList()");

	if (ibuf.GetLength() < 5)
		return 0;

    ibuf.Remove(5);

    QueryResult *result = stDatabaseMysql.PQuery("SELECT `id` FROM `account` WHERE `username` = '%s'", _login.c_str());
    if(!result) {
        ACE_DEBUG((LM_DEBUG, "[Realmd]: user %s tried to login and we cant find him in the database.\n", _login.c_str()));
        return -1;
    }                          

    uint32 id = (*result)[0].GetUInt32();
    delete result;

    uint8 AmountOfCharacters = 0;

    ByteBuffer pkt;
    pkt << (uint32) 0;
    pkt << (uint8) sRealmList->size();
    RealmList::RealmMap::const_iterator i;
    for( i = sRealmList->begin( ); i != sRealmList->end( ); i++ ) {
        pkt << (uint32) i->second->icon;
        pkt << (uint8) i->second->color;
        pkt << i->first;
        pkt << i->second->address;
        //TODO FIX THIS
        pkt << (float) 1.0;                                 //this is population 0.5 = low 1.0 = medium 2.0 high     (float)(maxplayers / players)*2
        //result = i->second->dbRealm.PQuery( "SELECT COUNT(*) FROM `character` WHERE `account` = %d",id);
        result = stDatabaseMysql.PQuery( "SELECT `numchars` FROM `realmcharacters` WHERE `acctid` = %d AND `realmid` = %d", id, i->second->m_ID);
        if( result ) {
            Field *fields = result->Fetch();
            AmountOfCharacters = fields[0].GetUInt8();
           
        } else {
            AmountOfCharacters = 0;
        }
		delete result;
        pkt << (uint8) AmountOfCharacters;
        pkt << (uint8) i->second->timezone;
        pkt << (uint8) 0;                      
    }
    pkt << (uint8) 0;                             
    pkt << (uint8) 0x2;                           

    ByteBuffer hdr;
    hdr << (uint8) REALM_LIST;
    hdr << (uint16)pkt.size();
    hdr.append(pkt);

	return this->peer ().send ((char *)hdr.contents() , hdr.size());
}

int
RealmHandler::_HandleXferResume()
{
	ACE_DEBUG((LM_DEBUG, "_HandleXferResume\n"));

    if (ibuf.GetLength () < 9 )
    {
        return 0;
    }

    uint64 start;
    ibuf.Remove(1);
    ibuf.Read((char*)&start,sizeof(start));
    fseek(pPatch,start,0);

	//Patcher patcher(this);
	//patcher.start(1);
    //ZThread::Thread u(new PatcherRunnable(this));
	return 0;
}

int
RealmHandler::_HandleXferCancel()
{
	ACE_DEBUG((LM_DEBUG, "_HandleXferResume\n"));

    ibuf.Remove(1);
    return -1;
}

void
RealmHandler:: _HandleXferAccept()
{
	ACE_DEBUG((LM_DEBUG, "_HandleXferResume\n"));
	ibuf.Remove(1);
    fseek(pPatch,0,0);

    //ZThread::Thread u(new PatcherRunnable(this));
}

/*
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
            pkt << (uint8)AUTH_BANNED;
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
                        pkt << (uint8) AUTH_SUSPENDED;
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
                    pkt << (uint8) AUTH_BANNED;
                    sLog.outBasic("[AuthChallenge] Banned account %s try to login!",_login.c_str ());
                }
                else
                {
                    password = (*result)[0].GetCppString();
                    /*
                    QueryResult *result =  .PQuery("SELECT COUNT(*) FROM `account` WHERE `account`.`online` > 0 AND `login`.`gmlevel` = 0;");
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
                    {
                    */

                    //if server is not full

                   /* uint32 acct;
                    QueryResult *resultAcct = dbRealmServer.PQuery("SELECT `id` FROM `account` WHERE `username` = '%s';", _login.c_str ());
                    if(resultAcct)
                    {
                        Field *fields = resultAcct->Fetch();
                        acct=fields[0].GetUInt32();
                        delete resultAcct;

                        QueryResult *result = dbRealmServer.PQuery("SELECT * FROM `account` WHERE `online` > 0 AND `id` = '%u';",acct);
                        if(result)
                        {
                            delete result;
                            //TODO FIX THIS THIS IS NOT CORRECT
                            pkt << (uint8)AUTH_ALREADY_LOGGING_IN;
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
                pkt<< (uint8) AUTH_UNKNOWN_ACCOUNT;
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
            pkt << (uint8) AUTH_VERSION_MISMATCH;
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

            uint8 data[2]={AUTH_LOGON_PROOF,CSTATUS_NEGOTIATION_FAILED};
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

*/
