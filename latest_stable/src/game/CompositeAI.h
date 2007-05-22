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

#ifndef MANGOS_COMPOSITEAI_H
#define MANGOS_COMPOSITEAI_H

#include "Utilities/TypeList.h"

template<class T>
struct CreateCompositeAI
{
    static void Create(Creature &c, std::vector<CreatureAI *> &ais)
    {
        ais.push_back(new T(c));
    }
};

template<>
struct CreateCompositeAI<NullType>
{
    static void Create(Creature &, std::vector<CreatureAI *> &) {}
};

template<class H, class U> struct CreateCompositeAI<TypeList<H, U> >
{
    static void Create(Creature &c, std::vector<CreatureAI *> &v)
    {
        CreateCompositeAI<H>::Create(c, v);
        CreateCompositeAI<U>::Create(c, v);
    }
}

template<typename T>
class MANGOS_DLL_DECL CompositeAI : public CreatureAI
{
    public:

        ~CreatureAI();
        void MoveInLineOfSight(Unit *);
        void AttackStart(Unit *);
        void AttackStop(Unit *);
        void HealBy(Unit *healer, uint32 amount_healed);
        void DamageInflict(Unit *done_by, uint32 amount_damage);
        bool IsVisible(Unit *) const;
        void UpdateAI(const uint32 diff);

        inline unsigned int NumberOfAI(void) const { return i_ais.size(); }
        CreatureAI * operator()(unsigned int idx) { return i_ais[idx]; }
        const CreatureAI * operator()(unsigned int idx) const { return i_ais[idx]; }

    private:
        std::vector<CreatureAI *> i_ais;
};
#endif
