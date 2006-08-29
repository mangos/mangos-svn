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

#ifndef MANGOS_MOVEMENTGENERATOR_H
#define MANGOS_MOVEMENTGENERATOR_H

#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "Dynamic/ObjectRegistry.h"
#include "Dynamic/FactoryHolder.h"
#include "Common.h"

class Creature;

#define     CANNOT_HANDLE_TYPE   -1

class MANGOS_DLL_SPEC MovementGenerator
{
    public:
        enum MovementGeneratorType
        {
            IDLE_MOTION_TYPE = 0,      // IdleMovementGenerator.h
            RANDOM_MOTION_TYPE,        // RandomMovementGenerator.h
            TARGETED_MOTION_TYPE,      // TargetedMovementGenerator.h
            ANIMAL_RANDOM_MOTION_TYPE, // AnimalRandomMovementGenerator.h
            CONFUSED_MOTION_TYPE,      // ConfusedMovementGenerator.h
            TAXI_MOTION_TYPE,          // TaxiMovementGenerator.h
            WAYPOINT_MOTION_TYPE,      // WaypointMovementGenerator.h
        };

        virtual ~MovementGenerator();

        virtual void Initialize(Creature &) = 0;

        virtual void Reset(Creature &) = 0;

        virtual void Update(Creature &, const uint32 &time_diff) = 0;

        virtual MovementGeneratorType GetMovementGeneratorType() = 0;
};

struct SelectableMovement : public FactoryHolder<MovementGenerator>, public Permissible<Creature>
{
    SelectableMovement(const char *id) : FactoryHolder<MovementGenerator>(id) {}
};

template<class REAL_MOVEMENT>
struct MovementGeneratorFactory : public SelectableMovement
{
    MovementGeneratorFactory(const char *name) : SelectableMovement(name) {}

    MovementGenerator* Create(void *) const;

    int Permit(const Creature *c) const { return REAL_MOVEMENT::Permissible(c); }
};

typedef FactoryHolder<MovementGenerator> MovementGeneratorCreator;
typedef FactoryHolder<MovementGenerator>::FactoryHolderRegistry MovementGeneratorRegistry;
typedef FactoryHolder<MovementGenerator>::FactoryHolderRepository MovementGeneratorRepository;
#endif
