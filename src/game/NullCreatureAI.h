/* NullCreatureAI.h
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

#ifndef MANGOS_NULLCREATUREAI_H
#define MANGOS_NULLCREATUREAI_H

/** NullCreatureAI are creatures that don't attack.  This is
 * a classic Null Object pattern.
 */

#include "CreatureAI.h"

class MANGOS_DLL_DECL NullCreatureAI : public CreatureAI 
{
public:
    
    void MoveInLineOfSight(Creature *) {}
    void AttackStart(Creature *) {}
    void AttackStop(Creature *) {}
    void HealBy(Creature *healer, uint32 amount_healed) {}
    void DamageInflict(Creature *healer, uint32 amount_healed) {}
    bool IsVisible(Creature *) const { return false; /* inactive creature don't care */ }

    void MoveInLineOfSight(Player *) {}
    void AttackStart(Player *) {}
    void AttackStop(Player *) {}
    void HealBy(Player *healer, uint32 amount_healed) {}
    void DamageInflict(Player *done_by, uint32 amount) {}
    bool IsVisible(Player *) const { return false; /* inactive creature don't care */ }
    
    void UpdateAI(const uint32) {}
    static int Permissible(const Creature *) { return 0; /* that's the best I can do dude */ }
};



#endif
