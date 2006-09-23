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
#include "World.h"
#include "WorldHandler.h"

WorldHandler::WorldHandler()
{

}

WorldHandler::~WorldHandler ()
{

}

int
WorldHandler::svc (void)
{
	ACE_Time_Value realCurrTime = ACE_Time_Value::zero , realPrevTime = ACE_Time_Value::zero;
	ACE_Thread_Manager *thr_mgr = ACE_Thread_Manager::instance ();
	while (thr_mgr->testcancel(ACE_OS::thr_self ()) == 0)
	{
		if (realPrevTime > realCurrTime)
            realPrevTime = ACE_Time_Value::zero;

		realCurrTime = ACE_OS::gettimeofday();
		sWorld.Update(realCurrTime.msec() - realPrevTime.msec());
		realPrevTime = realCurrTime;
		ACE_OS::sleep(ACE_Time_Value (0, 100000));
	}
	return 0;

}
