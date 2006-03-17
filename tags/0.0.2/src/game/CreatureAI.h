/* CreatureAI.h
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

#ifndef MANGOS_CREATUREAI_H
#define MANGOS_CREATUREAI_H

#include "Platform/Define.h"

/** This is an API for Creatures AI
 */

class Unit;

class MANGOS_DLL_DECL CreatureAI 
{
public:
    
    // dtor
    virtual ~CreatureAI();

    /** Call on a creature/Player move in line of sight of creature.
     * The line of sight of the creature depends on the distance
     * of the target plus the angle the creature is looking.
     */
    virtual void MoveInLineOfSight(Unit *) = 0;

    /// Call when a player/Creature starts to attack the creature
    virtual void AttackStart(Unit *) = 0;

    /// Call when the creature/Player stop attacking
    virtual void AttackStop(Unit *) = 0;

    /// Call to update the status of the attacker.  Only call when the victim exist
    virtual void Update(uint32, Unit *) = 0;

    /// Call when the victim of the creature heals by another creature/player
    virtual void HealBy(Unit *healer, uint32 amount_healed) = 0;
};



#endif
