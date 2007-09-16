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

#ifndef MANGOS_MOTIONMASTER_H
#define MANGOS_MOTIONMASTER_H

#include "Common.h"
#include <stack>

class MovementGenerator;
class Unit;

#define     CANNOT_HANDLE_TYPE   -1

// values 0 ... MAX_DB_MOTION_TYPE-1 used in DB
enum MovementGeneratorType
{
    IDLE_MOTION_TYPE      = 0,                              // IdleMovementGenerator.h
    RANDOM_MOTION_TYPE    = 1,                              // RandomMovementGenerator.h
    WAYPOINT_MOTION_TYPE  = 2,                              // WaypointMovementGenerator.h
    MAX_DB_MOTION_TYPE    = 3,                              // *** this and below motion types can't be set in DB.
    ANIMAL_RANDOM_MOTION_TYPE = MAX_DB_MOTION_TYPE,         // AnimalRandomMovementGenerator.h
    CONFUSED_MOTION_TYPE  = 4,                              // ConfusedMovementGenerator.h
    TARGETED_MOTION_TYPE  = 5,                              // TargetedMovementGenerator.h
    HOME_MOTION_TYPE      = 6,                              // HomeMovementGenerator.h
    FLIGHT_MOTION_TYPE    = 7                               // WaypointMovementGenerator.h
};

class MANGOS_DLL_SPEC MotionMaster : private std::stack<MovementGenerator *>
{
    private:
        typedef std::stack<MovementGenerator *> Impl;
    public:

        explicit MotionMaster(Unit *unit) : i_owner(unit) {}
        ~MotionMaster();

        void Initialize();

        MovementGenerator* operator->(void) { return top(); }

        using Impl::top;
        using Impl::empty;

        typedef Impl::container_type::const_iterator const_iterator;
        const_iterator begin() const { return Impl::c.begin(); }
        const_iterator end() const { return Impl::c.end(); }

        void UpdateMotion(const uint32 &diff);

        void Clear(bool reset = true);

        void MovementExpired(bool reset = true);

        void Idle(void);

        void TargetedHome();

        void Mutate(MovementGenerator *m);

    private:
        Unit *i_owner;
};
#endif
