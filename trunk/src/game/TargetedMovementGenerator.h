/* TargetedMovementGenerator.h
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

#ifndef MANGOS_TARGETEDMOVEMENTGENERATOR_H
#define MANGOS_TARGETEDMOVEMENTGENERATOR_H

/** TargetedMovementGenerator follows the target ensure
 * the source is close to the target
 */

#include "MovementGenerator.h"

// foward declaration
class Unit;

class MANGOS_DLL_DECL TargetedMovementGenerator : public MovementGenerator
{
public:

    TargetedMovementGenerator(Unit &target) : i_target(target) {}

    // API implementation
    void Initialize(const Creature &);
    void Reset(const Creature &);
    bool GetNext(const Creature &, float &x, float &y, float &z, float &orientation);
    void Update(Creature &, const uint32 &);

private:
    Unit &i_target;
};

#endif
