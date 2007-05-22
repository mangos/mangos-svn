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

#ifndef MANGOS_HOMEMOVEMENTGENERATOR_H
#define MANGOS_HOMEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class MANGOS_DLL_SPEC HomeMovementGenerator : public MovementGenerator
{
    public:

        HomeMovementGenerator() {}
        ~HomeMovementGenerator() {}

        void Initialize(Creature &);
        void Reset(Creature &);
        bool Update(Creature &, const uint32 &);
        void modifyTravelTime(uint32 travel_time) { i_travel_timer = travel_time; }
        MovementGeneratorType GetMovementGeneratorType() { return HOME_MOTION_TYPE; }

    private:
        void _setTargetLocation(Creature &);

        uint32 i_travel_timer;
        void _reLocate(Creature &);
};
#endif
