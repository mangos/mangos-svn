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


#ifndef WORLDSOCKET_MGR_H
#define WORLDSOCKET_MGR_H

#include "WorldSocket.h"
#include "ace/Acceptor.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "ace/SOCK_Acceptor.h"

class WorldSocketMgr : public ACE_Acceptor<WorldSocket,ACE_SOCK_ACCEPTOR>
{

public:
	WorldSocketMgr (void);
	virtual ~WorldSocketMgr (void);

	void AddSocket(WorldSocket *s);
	void RemoveSocket(WorldSocket *s);

	virtual int make_svc_handler (WorldSocket * & sh);

private:
	typedef std::set<WorldSocket*> SocketSet;
	SocketSet m_sockets;

	ACE_Recursive_Thread_Mutex mutex_;
};

#endif /* WORLDSOCKET_MGR_H */
