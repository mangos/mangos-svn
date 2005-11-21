/* MovementGenerator.h
 *
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

/** MovementGenerator is an API that generates movement for the
 * creatures.  There are numerous types of movement and
 * its implementation specific. Note, a movement generator can
 * be mutate to different type.  For instance, a guard can
 * initially uses GuardMovementGenerator but when he is under attack
 * or move to attack stance, his movement has to collaborate
 * with his action thus mutate to TargetedMovementGenerator.
 * This is all achived by the MotionMaster class. Note
 * all API will pass on its owner to ensure information is
 * not replicated everywhere.
 */

#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "Dynamic/ObjectRegistry.h"
#include "Dynamic/FactoryHolder.h"

// foward declaration
class Creature;

typedef enum 
    {
	MOTIONLESS_TYPE = 0,
	RANDOM_MOTION_TYPE,
	TARGETED_MOTION_TYPE,
        SPECIAL_MOTION_TYPE
    } motion_t;

class MANGOS_DLL_DECL MovementGenerator 
{
public:
    //dtor
    virtual ~MovementGenerator();

    /// Initialized the movement generator
    virtual void Initialize(const Creature &) = 0;

    /// Reset the movement generator
    virtual void Reset(const Creature &) = 0;

    /// Get the next movement from the generator.  Returns false if none.
    virtual bool GetNext(const Creature &, float &x, float &y, float &z, float &orientation) = 0;

    /// Update the movement generator
    virtual void Update(Creature &, const uint32 &time_diff) = 0;
};


struct SelectableMovement : public FactoryHolder<MovementGenerator>, public Permissible<Creature>
{
    SelectableMovement(const char *id) : FactoryHolder<MovementGenerator>(id) {}
};

/** MovementGeneratorFactory responsible for creating a movement
 * generator, also delicate the actual implementation of
 * permit to is real class.
 */
template<class REAL_MOVEMENT>
struct MovementGeneratorFactory : public SelectableMovement
{
    MovementGeneratorFactory(const char *name) : SelectableMovement(name) {}

    // implement API for FactorHolder<T>
    MovementGenerator* Create(void *) const;

    // implement API for Permissible<T>
    int Permit(const Creature *c) const { return REAL_MOVEMENT::Permissible(c); }
};



#define	    MOTIONLESS_TYPE      0
#define     RANDOM_MOTION_TYPE   100
#define     TARGETED_MOTION_TYPE 200
#define     SPECIAL_MOTION_TYPE  400
#define     CUSTOM_MOTION_TYPE   800

typedef FactoryHolder<MovementGenerator> MovementGeneratorCreator;
typedef FactoryHolder<MovementGenerator>::FactoryHolderRegistry MovementGeneratorRegistry;
typedef FactoryHolder<MovementGenerator>::FactoryHolderRepository MovementGeneratorRepository;

#endif

