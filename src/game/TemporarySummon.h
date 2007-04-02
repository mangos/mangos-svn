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

#ifndef MANGOSSERVER_TEMPSUMMON_H
#define MANGOSSERVER_TEMPSUMMON_H

#include "Creature.h"

enum TempSummonType
{
    TEMPSUMMON_REMOVE_DEAD   = 1,
    TEMPSUMMON_REMOVE_CORPSE = 2,
};

class TemporarySummon : public Creature
{
    public:
        TemporarySummon( WorldObject *instantiator );
        virtual ~TemporarySummon(){};
        void Update(uint32 time);
        void Summon(TempSummonType type, uint32 lifetime);
        void UnSummon();
        void setDeathState(DeathState s);
        void SaveToDB();
    private:
        TempSummonType m_type;
        uint32 m_timer;
        uint32 m_lifetime;
};
#endif
