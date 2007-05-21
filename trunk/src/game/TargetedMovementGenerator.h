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

#ifndef MANGOS_TARGETEDMOVEMENTGENERATOR_H
#define MANGOS_TARGETEDMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
#include "DestinationHolder.h"
#include "Traveller.h"

class Unit;

class MANGOS_DLL_SPEC TargetedMovementGenerator : public MovementGenerator
{
    public:

        TargetedMovementGenerator(Unit &target) : i_target(target), i_offset(0), i_angle(0) {}
        TargetedMovementGenerator(Unit &target, float offset, float angle) : i_target(target), i_offset(offset), i_angle(angle) {}
        ~TargetedMovementGenerator() {}

        void Initialize(Creature &);
        void Reset(Creature &);
        bool Update(Creature &, const uint32 &);
        MovementGeneratorType GetMovementGeneratorType() { return TARGETED_MOTION_TYPE; }

        void spellAtack(Creature &,Unit &,uint32 spellId);

        time_t next_update_time;

    private:

        void _spellAtack(Creature &owner, SpellEntry* spellInfo);
        void _setTargetLocation(Creature &);
        Unit &i_target;
        float i_offset;
        float i_angle;
        DestinationHolder<Traveller<Creature> > i_destinationHolder;
};
#endif
