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

#ifndef MANGOS_MOTIONMASTER_H
#define MANGOS_MOTIONMASTER_H

#include "MovementGenerator.h"
#include <stack>

class MANGOS_DLL_SPEC MotionMaster : private std::stack<MovementGenerator *>
{
    public:

        explicit MotionMaster(Creature *creature) : i_owner(creature) {}
        ~MotionMaster();

        void Initialize();

        MovementGenerator* operator->(void) { return top(); }

        using std::stack<MovementGenerator *>::top;
        using std::stack<MovementGenerator *>::empty;

        void UpdateMotion(const uint32 &diff);

        void Clear(bool reset = true);

        void MovementExpired(bool reset = true);

        void Idle(void);

        void TargetedHome();

        void Mutate(MovementGenerator *m)
        {
            // HomeMovement is not that important, delete it if meanwhile a new comes
            if (!empty() && top()->GetMovementGeneratorType() == HOME_MOTION_TYPE)
                MovementExpired(false);
            m->Initialize(*i_owner);
            push(m);
        }

    private:
        Creature *i_owner;
};
#endif
