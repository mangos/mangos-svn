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

#ifndef MANGOS_FLIGHTMASERT_H
#define MANGOS_FLIGHTMASERT_H

/** @page FlightMaster keeps all current flight that's in progress so
 * that every small time interval, it updates its location
 * to generate ground and air activities.  When the activities occur
 * player update himself and other players within its visibility
 * range so that both ground player and in the air player
 * see each other's movement.
 */

#include "Policies/Singleton.h"
#include "WaypointMovementGenerator.h"
#include "Log.h"

#include "zthread/FastMutex.h"
#include <functional>
#include <vector>
#include <cassert>

class MANGOS_DLL_DECL FlightMaster : public MaNGOS::Singleton<FlightMaster, MaNGOS::ClassLevelLockable<FlightMaster, ZThread::FastMutex> >
{
    friend class MaNGOS::OperatorNew<FlightMaster>;
    typedef ZThread::FastMutex LockType;
    typedef MaNGOS::ClassLevelLockable<FlightMaster, ZThread::FastMutex>::Lock Guard;
    typedef std::map<Player *, FlightPathMovementGenerator *> FlightMapType;
    FlightMapType i_flights;

    public:

        /** ReportFlight reports a certain flight just started
         * so that the flight master can keep track of the flight.
         */
        inline void ReportFlight(Player *pl, FlightPathMovementGenerator *gen)
        {
            Guard guard(*this);
            i_flights[pl] = gen;
        }

        /** FlightReportUpdate updates each flight and if the flight has arrived
         * to its destination, it will report to its player that the flight has
         * finish and the flight path will be remove.
         */
        inline void FlightReportUpdate(const uint32 &diff)
        {
            Guard guard(*this);
            for(FlightMapType::iterator iter=i_flights.begin(); iter != i_flights.end();)
            {
                //DEBUG_LOG("id=%d IsInWorld=%d", iter->first,iter->first->IsInWorld());
                if( iter->first->IsInWorld() != 1 )
                {
                    // do not use any func like "getname()" if player is offline, or it crash server
                    DEBUG_LOG("Removing player id=%d flight from flight master. (player offline)", iter->first);
                    delete iter->second;
                    i_flights.erase(iter++);
                }
                else if( iter->second->CheckFlight(diff) )
                {
                    DEBUG_LOG("Removing player %s flight from flight master.", iter->first->GetName());
                    delete iter->second;
                    i_flights.erase(iter++);
                }
                else
                    ++iter;
            }
        }

};
#endif
