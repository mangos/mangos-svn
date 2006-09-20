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

#ifndef MANGOS_MOTIONMASTER_H
#define MANGOS_MOTIONMASTER_H

#include "IdleMovementGenerator.h"
#include <stack>

class MANGOS_DLL_DECL MotionMaster : private std::stack<MovementGenerator *>
{
    static IdleMovementGenerator si_idleMovement;
    public:

        MotionMaster() : i_owner(NULL) {}

        void Initialize(Creature *creature);

        MovementGenerator* operator->(void) { return top(); }

        using std::stack<MovementGenerator *>::top;

        void UpdateMotion(const uint32 &diff);

        void Clear(void);

        void MovementExpired(void);

        void Idle(void)
        {
            if( !isStatic( top() ) )
                push( &si_idleMovement );
        }

        void Mutate(MovementGenerator *m)
        {
            m->Initialize(*i_owner);
            push(m);
        }

    private:

        inline bool isStatic(MovementGenerator *mv) const { return (mv == &si_idleMovement); }
        Creature *i_owner;
};
#endif
