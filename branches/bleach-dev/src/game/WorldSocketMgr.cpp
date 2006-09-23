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

#include "WorldSocketMgr.h"

WorldSocketMgr::WorldSocketMgr (void)
{

}

WorldSocketMgr::~WorldSocketMgr (void)
{
  this->reactor (0);
}

void
WorldSocketMgr::AddSocket(WorldSocket *s)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> locker (this->mutex_);
    m_sockets.insert(s);
}

void
WorldSocketMgr::RemoveSocket(WorldSocket *s)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> locker (this->mutex_);
    m_sockets.erase(s);
}


int
WorldSocketMgr::make_svc_handler (WorldSocket *&sh)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> locker (this->mutex_);

	if( sh == 0 ) {
		ACE_NEW_RETURN (sh, WorldSocket , -1);
		return 0;
	}
	
	return -1;
}
