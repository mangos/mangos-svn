/* IdleMovement.h
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

#ifndef MANGOS_IDLEMOVEMENTGENERATOR_H
#define MANGOS_IDLEMOVEMENTGENERATOR_H

/** IdleMovementGenerator uses by creatures that don't move.
 */

#include "MovementGenerator.h"

class MANGOS_DLL_DECL IdleMovementGenerator : public MovementGenerator
{
public:
    
    // overide API
    void Initialize(const Creature &) { /* don't need to implement */ }
    void Reset(const Creature &) {}
    bool GetNext(const Creature &, float &x, float &y, float &z, float &orientation) { return false; }
    void Update(Creature &, const uint32 &) {}
};

#endif
