/* TaxiMovementGenerator.h
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

#ifndef MANGOS_TAXIMOVEMENTGENERATOR_H
#define MANGOS_TAXIMOVEMENTGENERATOR_H

/** @page TaxiMovementGenerator is a special class that its not only
 * inherited from MovementGenerator but is the only MovementGenerator
 * family of object that can be use by the player.  The sole purpose
 * of this class is to generate movement (like the client side) during
 * a taxi ride so that the server updates its surrounding grid cells which
 * gives activities on the ground during a taxi ride. During a taxi ride,
 * when a player exits, this class can use to handle the position of the
 * player when he re-enters which indeed still in his taxi ride (except
 * that the surrounding environment might be a little different than
 * that when he exits (if during taxi ride allows the player to exit).
 * When use on creatures, this gives identical results except that the subscribers
 * of the messages is only the players surrounding the grid cell, not
 * the creature itself.
 */

#include "MovementGenerator.h"

class MANGOS_DLL_DECL TaxiMovementGenerator : public MovementGenerator
{
public:

    void Initialize(const Creature &);
    void Reset(const Creature &);
    bool GetNext(const Creature &, float &x, float &y, float &z, float &orientation);
    void Update(Creature &, const uint32 &);
};

#endif
