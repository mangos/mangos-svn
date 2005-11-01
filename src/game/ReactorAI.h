/* ReactorAI.h
 *
 * Copyright (C) 2004 Wow Daemon
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

#ifndef MANGOS_REACTORAI_H
#define MANGOS_REACTORAI_H

/** ReactorAI reacts to the attack.  Mostly use in 
 * non aggressive animals.
 */

#include "CreatureAI.h"

class Unit;

class MANGOS_DLL_DECL ReactorAI : public CreatureAI
{
public:
    // bindind a creature to the AI.
    ReactorAI(Creature &c) : i_creature(c), i_pVictim(NULL) {}

    void MoveInLineOfSight(Creature *);
    void AttackStart(Creature *);
    void AttackStop(Creature *);
    void HealBy(Creature *healer, uint32 amount_healed);
    void DamageInflict(Creature *healer, uint32 amount_healed);
    bool IsVisible(Creature *) const;

    void MoveInLineOfSight(Player *);
    void AttackStart(Player *);
    void AttackStop(Player *);
    void HealBy(Player *healer, uint32 amount_healed);
    void DamageInflict(Player *done_by, uint32 amount);
    bool IsVisible(Player *) const;

    void UpdateAI(const uint32);
    static int Permissible(const Creature *);

private:
    Creature &i_creature;
    Unit *i_pVictim;
};

#endif
