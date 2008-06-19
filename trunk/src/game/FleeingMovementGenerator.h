/* 
* Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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

#ifndef MANGOS_FLEEINGMOVEMENTGENERATOR_H
#define MANGOS_FLEEINGMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
#include "DestinationHolder.h"
#include "Traveller.h"
#include "MapManager.h"

template<class T>
class MANGOS_DLL_SPEC FleeingMovementGenerator
: public MovementGeneratorMedium< T, FleeingMovementGenerator<T> >
{
    public:
        FleeingMovementGenerator(Unit &fright, float _flee_distance = 30.0f) : i_fright(fright), flee_distance(_flee_distance), i_nextMoveTime(0), i_nextCheckTime(0) {}

        void Initialize(T &);
        void Finalize(T &);
        void Reset(T &);
        bool Update(T &, const uint32 &);

        MovementGeneratorType GetMovementGeneratorType() { return FLEEING_MOTION_TYPE; }

    private:
        void _setTargetLocation(T &owner);
        bool _getPoint(T &owner, float &x, float &y, float &z);
        bool is_water_ok;
        bool is_land_ok;
        float flee_distance;
        TimeTracker i_nextMoveTime;
        TimeTracker i_nextCheckTime;
        Unit& i_fright;
        DestinationHolder< Traveller<T> > i_destinationHolder;
};
#endif
