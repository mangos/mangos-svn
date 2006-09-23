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
#include "WorldSession.h"
#include "World.h"
#include "NameTables.h"
#include "WorldLog.h"
#include "AddonHandler.h"
#include "zlib/zlib.h"
#include "AuthCodes.h"

#include "WorldSocketMgr.h"
#include "WorldSocket.h"


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

WorldSocket::WorldSocket ()
	: flg_mask_ (ACE_Event_Handler::NULL_MASK),
	ibuf(TCP_BUFSIZE_READ), obuf(TCP_BUFSIZE_READ),
	_cmd(0), _remaining(0), _session(0), _seed(0xDEADBABE)
{
	ACE_TRACE("WorldSocket::WorldSocket ()");

}

WorldSocket::~WorldSocket ()
{
	ACE_TRACE("WorldSocket::~WorldSocket ()");

	this->reactor (0);

	for (; ;)
    {
		ACE_Time_Value tv = ACE_Time_Value::zero;
		ACE_Message_Block *mb = 0;
		if (this->getq (mb, &tv) < 0)
			break;

		ACE_Message_Block::release (mb);
    }

	if(_session)
	{
		sWorld.RemoveSession(_session->GetAccountId());
	}
}

int
WorldSocket::check_destroy (void)
{
	ACE_TRACE("WorldSocket::check_destroy (void)");

	if (flg_mask_ == ACE_Event_Handler::NULL_MASK)
		return -1;
	
	return 0;
}

int
WorldSocket::SendPacket(WorldPacket* packet)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> locker (mutex_);

	ACE_TRACE("WorldSocket::SendPacket(WorldPacket* packet)");

	WorldPacket *pck = 0;
	ACE_NEW_RETURN(pck, WorldPacket(*packet), -1);

	ACE_Time_Value tv = ACE_Time_Value::zero;
	int qcount = this->putq (pck, & tv);

	if ( qcount  <= 0 /* failed to putq */)
    {
		ACE_DEBUG ((LM_DEBUG, "%I-- %D - %M - Unable to enqueue Packet--\n"));
		ACE_Message_Block::release (pck);
		return -1;
    }
	else if ( qcount >= 50 /* control flow */)
	{
		if (this->terminate_io (ACE_Event_Handler::READ_MASK) != 0)
			return -1;
	}
	else
	{
		if (this->initiate_io (ACE_Event_Handler::READ_MASK) != 0)
			return -1;
	}

	if (this->initiate_io (ACE_Event_Handler::WRITE_MASK) != 0)
		return -1;

	return 0;
}

int
WorldSocket::open (void *_acceptor)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> locker (mutex_);
	
	int one = 1;

	ACE_TRACE("WorldSocket::open (void *_acceptor)");

	WorldSocketMgr * sWorldSocketMgr = (WorldSocketMgr *) _acceptor;

	this->reactor (sWorldSocketMgr->reactor ());

	ACE_INET_Addr addr;

	if (this->peer ().get_remote_addr (addr) == -1)
		return -1;
	
	if(this->peer().set_option (ACE_IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)) == -1)
		return -1;

	flg_mask_ = ACE_Event_Handler::NULL_MASK ;

	if (this->reactor ()->register_handler (this, flg_mask_) == -1)
		ACE_ERROR_RETURN ((LM_ERROR,
			"(%P|%t) can't register with reactor\n"),
            -1);

	if(this->initiate_io (ACE_Event_Handler::READ_MASK) != 0)
		return -1;
	
	WorldPacket packet;

    packet.Initialize( SMSG_AUTH_CHALLENGE );
    packet << _seed;

	if ( SendPacket(&packet) == -1)
		return -1;

	ACE_DEBUG ((LM_DEBUG,
              "(%P|%t) connected with %s\n",
              addr.get_host_name ()));

	return check_destroy();
}

int
WorldSocket::initiate_io (ACE_Reactor_Mask mask)
{
	ACE_TRACE("WorldSocket::initiate_io (ACE_Reactor_Mask mask)");

	if (ACE_BIT_ENABLED (flg_mask_, mask))
		return 0;

	if (this->reactor ()->schedule_wakeup  (this, mask) == -1)
		return -1;

	ACE_SET_BITS (flg_mask_, mask);
	return 0;
}

int
WorldSocket::terminate_io (ACE_Reactor_Mask mask)
{
	ACE_TRACE("WorldSocket::terminate_io (ACE_Reactor_Mask mask)");

	if (ACE_BIT_DISABLED (flg_mask_, mask))
		return 0;

	if (this->reactor ()->cancel_wakeup (this, mask) == -1)
		return -1;

	ACE_CLR_BITS (flg_mask_, mask);
	return 0;
}

int 
WorldSocket::read_input (void)
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
WorldSocket::handle_input (ACE_HANDLE handle /* = ACE_INVALID_HANDLE */)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> locker (mutex_);
	
	ACE_TRACE("WorldSocket::handle_input (ACE_HANDLE handle /* = ACE_INVALID_HANDLE */)");

	if ( read_input() == -1)
	{
		return -1;
	}

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

		if(_remaining)
			ibuf.Read((char*)packet.contents(), _remaining);
		
		/*ACE_DEBUG ((LM_DEBUG, ACE_TEXT("%I-- %D - %M - Handle: %d - Opcode %s (0x%.4X) received --\n"),
					(uint32)handle,
					LookupName(packet.GetOpcode(), g_worldOpcodeNames),
					packet.GetOpcode()));*/

		if( sWorldLog.LogWorld() )
		{
			sWorldLog.Log("CLIENT:\nSOCKET: %u\nLENGTH: %u\nOPCODE: %s (0x%.4X)\nDATA:\n",
				(uint32)handle,
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

		int result = 0;

		switch (_cmd) {
			case CMSG_PING:
				result = _HandlePing(packet);
				break;
			case CMSG_AUTH_SESSION:
				result = _HandleAuthSession(packet);
				break;
			default:
				if (_session) {
					result = _session->QueuePacket(packet);
				} else {
					ACE_DEBUG ((LM_WARNING, ACE_TEXT("%I-- %D - %M - Received out of place packet with cmdid 0x%.4X --\n"), _cmd));
					result = -1;
				}
				break;
		}
		if ( result == -1)
			return -1;
	}

	if(this->initiate_io (ACE_Event_Handler::WRITE_MASK) != 0)
		return -1;

	return check_destroy ();
}

int
WorldSocket::write_output(void)
{
	int err = 0;
	ssize_t res = 0;

	res = this->peer().send(obuf.GetStart(), (int)obuf.GetL());

	if (res < 0)
		err = errno ;

	if (err != 0  || res < 0)
	{
		return -1;
	}
	else
	{
		obuf.Remove(res);
	}

    while (obuf.Space() && m_mes.size())
    {
        ucharp_v::iterator it = m_mes.begin();
        MES *p = *it;
        if (obuf.Space() > p -> left())
        {
            obuf.Write(p -> curbuf(),p -> left());
            delete p;
            m_mes.erase(m_mes.begin());
        }
        else
        {
            size_t sz = obuf.Space();
            obuf.Write(p -> curbuf(),sz);
            p -> ptr += sz;
        }
    }

	return 0;
}

int
WorldSocket::SendBuf(const char *buf,size_t len)
{
    int n = (int)obuf.GetLength();

    /*if (!Ready())
    {
        Handler().LogError(this, "SendBuf", -1, "Attempt to write to a non-ready socket" );
        if (GetSocket() == INVALID_SOCKET)
            Handler().LogError(this, "SendBuf", 0, " * GetSocket() == INVALID_SOCKET", LOG_LEVEL_INFO);
        if (Connecting())
            Handler().LogError(this, "SendBuf", 0, " * Connecting()", LOG_LEVEL_INFO);
        if (CloseAndDelete())
            Handler().LogError(this, "SendBuf", 0, " * CloseAndDelete()", LOG_LEVEL_INFO);
        return;
    }*/

    if (m_mes.size() || len > obuf.Space())
    {
        MES *p = new MES(buf,len);
        m_mes.push_back(p);
    }
    if (m_mes.size())
    {
        while (obuf.Space() && m_mes.size())
        {
            ucharp_v::iterator it = m_mes.begin();
            MES *p = *it;
            if (obuf.Space() > p -> left())
            {
                obuf.Write(p -> curbuf(),p -> left());
                delete p;
                m_mes.erase(m_mes.begin());
            }
            else
            {
                size_t sz = obuf.Space();
                obuf.Write(p -> curbuf(),sz);
                p -> ptr += sz;
            }
        }
    }
    else
    {
		obuf.Write(buf,len);
    }
    if (!n)
    {
        return write_output();
    }
	return 0;
}

int
WorldSocket::handle_output (ACE_HANDLE handle /* = ACE_INVALID_HANDLE */)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> locker (mutex_);
	
	ACE_TRACE("WorldSocket::handle_output (ACE_HANDLE handle /* = ACE_INVALID_HANDLE */)");
	
	int qcount = 0;
	while((this->msg_queue()->is_empty() == 0))
	{
		ACE_Time_Value tv = ACE_Time_Value::zero;
		ACE_Message_Block *mb = 0;
		qcount = this->getq (mb, &tv);
		
		if ( mb != 0 )
		{
			int err = 0;
			ssize_t res = 0;

			ServerPktHeader hdr;
			WorldPacket *packet = (WorldPacket *)mb;

			hdr.size = ntohs((uint16)packet->size() + 2);
			hdr.cmd = packet->GetOpcode();

			if( sWorldLog.LogWorld() )
			{
				sWorldLog.Log("SERVER:\nSOCKET: %u\nLENGTH: %u\nOPCODE: %s (0x%.4X)\nDATA:\n",
					(uint32)handle,
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

			if( SendBuf((char*)&hdr, 4) == -1 )
			{
				ACE_Message_Block::release (mb);
				return -1;
			}

			if(packet->size())
			{
				if( SendBuf((char*)packet->contents(), packet->size()) == -1 )
				{
					ACE_Message_Block::release (mb);
					return -1;
				}
			}
			ACE_Message_Block::release (mb);
		}
	}

	if ( qcount <= 0)
	{
		if (this->terminate_io (ACE_Event_Handler::WRITE_MASK) != 0)
			return -1;

		if (this->initiate_io (ACE_Event_Handler::READ_MASK) != 0)
			return -1;
	}

	return check_destroy ();
}

int
WorldSocket::handle_timeout (const ACE_Time_Value &tv, const void *arg)
{
	ACE_DEBUG((LM_DEBUG, "WorldSocket::handle_timeout (const ACE_Time_Value &tv, const void *arg).\n"));
	return 0;
}

int
WorldSocket::handle_close (ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
	ACE_TRACE("WorldSocket::handle_close (ACE_HANDLE handle, ACE_Reactor_Mask mask).\n");

	this->reactor ()->remove_handler (this,
                             ACE_Event_Handler::ALL_EVENTS_MASK |
                             ACE_Event_Handler::DONT_CALL);  // Don't call handle_close
	this->reactor (0);
	this->destroy ();

	return 0;
}

int
WorldSocket::_HandleAuthSession(WorldPacket& recvPacket)
{
	ACE_TRACE("WorldSocket::_HandleAuthSession(WorldPacket& recvPacket).\n");

    uint8 digest[20];
    uint32 clientSeed;
    uint32 unk2;
    uint32 BuiltNumberClient;
    uint32 id, security;
    std::string account;
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
		ACE_ERROR_RETURN ((LM_DEBUG, "SOCKET: Incomplete packet.\n"), -1);
    }
	
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("account %s.\n"), account.c_str()));

    QueryResult *result = loginDatabase.PQuery("SELECT `id`,`gmlevel`,`sessionkey` FROM `account` WHERE `username` = '%s';", account.c_str());

    if ( !result )
	{

        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( AUTH_UNKNOWN_ACCOUNT );
        SendPacket( &packet );
		
		ACE_ERROR_RETURN ((LM_DEBUG, "SOCKET: Sent Auth Response (unknown account).\n"), -1);
    }

    id = (*result)[0].GetUInt32();
    security = (*result)[1].GetUInt16();
    K.SetHexStr((*result)[2].GetString());

    delete result;

    uint32 num = sWorld.GetSessionCount();
    if (sWorld.GetPlayerLimit() > 0 && num > sWorld.GetPlayerLimit() && security == 0)
    {

        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( CSTATUS_FULL);
        SendPacket( &packet );
		ACE_ERROR_RETURN ((LM_DEBUG, "SOCKET: Sent Auth Response (server full).\n"), -1);
    }

    WorldSession *session = sWorld.FindSession( id );
    if( session )
    {
        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( AUTH_ALREADY_ONLINE );
        SendPacket( &packet );

        session->LogoutPlayer(0,1);

		ACE_ERROR_RETURN ((LM_DEBUG, "SOCKET: Sent Auth Response (already connected).\n"), -1);
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

    if (ACE_OS::memcmp(sha.GetDigest(), digest, 20))
    {
        packet.Initialize( SMSG_AUTH_RESPONSE );
        packet << uint8( AUTH_FAILED );
        SendPacket( &packet );

		ACE_ERROR_RETURN ((LM_DEBUG, "SOCKET: Sent Auth Response (authentification failed).\n"), -1);
    }

    _crypt.SetKey(K.AsByteArray(), 40);
    _crypt.Init();

    //Send Auth is okey
    packet.Initialize( SMSG_AUTH_RESPONSE );
    packet << uint8( AUTH_OK );                             //0x0C
    packet << uint8( 0xB0 );
    packet << uint8( 0x09 );
    packet << uint8( 0x02 );
    packet << uint8( 0x00 );
    packet << uint8( 0x02 );
    packet << uint32( 0x0 );

	if ( this->SendPacket(&packet) == -1 )
	{
		return -1;
	}

	ACE_NEW_RETURN (_session, WorldSession(id, this) , -1);

    _session->SetSecurity(security);
    sWorld.AddSession(_session);

    ACE_DEBUG((LM_DEBUG, "SOCKET: Client '%s' authed successfully.\n", account.c_str()));
	
	//! Handled Addons
	//Create Addon Packet
    sAddOnHandler.BuildAddonPacket(&recvPacket, &SendAddonPacked, recvPacket.rpos());

    return SendPacket(&SendAddonPacked);
}

int WorldSocket::_HandlePing(WorldPacket& recvPacket)
{
    WorldPacket packet;
    uint32 ping;

	recvPacket >> ping;

    packet.Initialize( SMSG_PONG );
    packet << ping;

    return SendPacket(&packet);
}

int WorldSocket::SendAuthWaitQue(uint32 PlayersInQue)
{
    WorldPacket packet;
    packet.Initialize( SMSG_AUTH_RESPONSE );
    packet << uint8( AUTH_WAIT_QUEUE );
    packet << uint32 (0x00);                                //unknown
    packet << uint32 (0x00);                                //unknown
    packet << uint8 (0x00);                                 //unknown
    packet << uint32 (PlayersInQue);                        //amount of players in que

    return SendPacket(&packet);
}
