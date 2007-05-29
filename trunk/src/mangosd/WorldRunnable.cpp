/* 
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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
    \ingroup mangosd
*/

#include "Common.h"
#include "World.h"
#include "WorldRunnable.h"
#include "Timer.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "EventSystem.h"

#include "Database/DatabaseEnv.h"

/// Heartbeat for the World
void WorldRunnable::run()
{
    ///- Init new SQL thread for the world database
    sDatabase.ThreadStart();                                // let thread do safe mySQL requests (one connection call enough)

    uint32 realCurrTime = 0;
    uint32 realPrevTime = getMSTime();

    ///- While we have not World::m_stopEvent, update the world
    while (!World::m_stopEvent)
    {
        realCurrTime = getMSTime();

        uint32 diff;
        if (realPrevTime > realCurrTime)                    // getMSTime() have limited data range and this is case when it overflow in this tick
            diff = 0xFFFFFFFF - (realPrevTime - realCurrTime);
        else
            diff = realCurrTime - realPrevTime;

        sWorld.Update( diff );
        realPrevTime = realCurrTime;

        #ifdef WIN32
        ZThread::Thread::sleep(50);
        #else
        ZThread::Thread::sleep(100);
        #endif
    }

    sWorld.KickAll();                                       // save and kick all players

    MapManager::Instance().UnloadAll();                     // unload all grids (including locked in memory)

    ///- End the database thread    
    sDatabase.ThreadEnd();                                  // free mySQL thread resources

    StopEventSystem();
}
