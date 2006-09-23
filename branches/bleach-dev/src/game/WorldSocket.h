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


#ifndef WORLD_SOCKET_H
#define WORLD_SOCKET_H

#include "Auth/BigNumber.h"
#include "Auth/AuthCrypt.h"

#include "ace/Svc_Handler.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "ace/SOCK_Stream.h"
#include "CircularBuffer.h"

#define TCP_BUFSIZE_READ 16400

class WorldPacket;
class WorldSession;

class WorldSocket : public ACE_Svc_Handler <ACE_SOCK_STREAM, ACE_MT_SYNCH>
{

public:
	WorldSocket();
    ~WorldSocket();

	int SendPacket(WorldPacket* packet);
	int enQueuePacket(WorldPacket& packet);

	int open (void *acceptor);
	int handle_close (ACE_HANDLE handle, ACE_Reactor_Mask mask);

protected:
	int handle_input (ACE_HANDLE handle);
	int handle_output (ACE_HANDLE handle);
	int handle_timeout (const ACE_Time_Value &tv, const void *arg);

	int _HandleAuthSession(WorldPacket& recvPacket);
	int _HandlePing(WorldPacket& recvPacket);

	int SendAuthWaitQue(uint32 PlayersInQue);

private:
	AuthCrypt _crypt;
	uint32 _seed;
	uint32 _cmd;
	uint16 _remaining;
	WorldSession* _session;
	
	int read_input(void);
	int write_output(void);
	int SendBuf(const char *buf,size_t len);
	int initiate_io (ACE_Reactor_Mask mask);
	int terminate_io (ACE_Reactor_Mask mask);
	int	check_destroy (void);

	int	flg_mask_;

	ACE_Recursive_Thread_Mutex mutex_;
	CircularBuffer ibuf;
	CircularBuffer obuf;

	struct MES
    {
        MES( const char *buf_in,size_t len_in)
            :buf(new  char[len_in])
            ,len(len_in)
            ,ptr(0)
        {
            memcpy(buf,buf_in,len);
        }
        ~MES() { delete[] buf; }
        size_t left() { return len - ptr; }
        char *curbuf() { return buf + ptr; }
        char *buf;
        size_t len;
        size_t ptr;
    };
    typedef std::list<MES *> ucharp_v;
	ucharp_v m_mes;
};

#endif /* REALM_HANDLER_H */
