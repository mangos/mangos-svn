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

#ifndef MANGOS_RANDOMMOTIONGENERATOR_H
#define MANGOS_RANDOMMOTIONGENERATOR_H

#include "MovementGenerator.h"
#include "DestinationHolder.h"
#include "Traveller.h"

#define MAX_CONF_WAYPOINTS 24

class MANGOS_DLL_DECL ConfusedMovementGenerator : public MovementGenerator
{
    public:
        ConfusedMovementGenerator(const Creature &) : i_nextMoveTime(0) {}

        void Initialize(Creature &);
        void Reset(Creature &);
        bool Update(Creature &, const uint32 &);
        MovementGeneratorType GetMovementGeneratorType() { return CONFUSED_MOTION_TYPE; }

        static int Permissible(const Creature *);
    private:
        TimeTracker i_nextMoveTime;
        float i_waypoints[MAX_CONF_WAYPOINTS+1][3];
        DestinationHolder<CreatureTraveller> i_destinationHolder;
        uint32 i_nextMove;
};
#endif
