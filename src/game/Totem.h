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

#ifndef MANGOSSERVER_TOTEM_H
#define MANGOSSERVER_TOTEM_H

#include "Creature.h"

enum TotemType
{
    TOTEM_PASSIVE    = 0,
    TOTEM_ACTIVE     = 1,
    TOTEM_LAST_BURST = 2,
};

class Totem : public Creature
{
    public:
        Totem( WorldObject *instantiator );
        virtual ~Totem(){};
        void Update( uint32 time );
        void Summon();
        void UnSummon();
        uint32 GetSpell() const { return m_spell; }
        uint32 GetTotemDuration() const { return m_duration; }
        Unit *GetOwner();
        TotemType GetTotemType() const { return m_type; }
        void SetSpell(uint32 spellId);
        void SetDuration(uint32 dur) { m_duration = dur; }
        void SetOwner(uint64 guid);

    protected:
        TotemType m_type;
        uint32 m_duration;
        uint32 m_spell;
};
#endif
