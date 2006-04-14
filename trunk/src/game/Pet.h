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

#ifndef MANGOSSERVER_PET_H
#define MANGOSSERVER_PET_H

#include "Creature.h"

#define PETMAXSPELLS        4

enum PetState
{
    STATE_RA_FOLLOW         = 1,
    STATE_RA_REACTIVE       = 2,
    STATE_RA_PROACTIVE      = 4,
    STATE_RA_PASSIVE        = 8,
    STATE_RA_SPELL1         = 16,
    STATE_RA_SPELL2         = 32,
    STATE_RA_SPELL3         = 64,
    STATE_RA_SPELL4         = 128
};

class Pet : public Creature
{
    public:
        Pet();
        virtual ~Pet(){};

        uint32 GetActState() { return m_actState; }
        void SetActState(uint32 st) { m_actState=st; }
        uint32 GetFealty() { return m_fealty; }
        void SetFealty(uint32 fealty) { m_fealty=fealty; }
        uint32* GetSpells() { return m_spells; }
        void SetSpells(uint8 index, uint32 spellid)
        {
            if(index>=0 && index<PETMAXSPELLS)
                m_spells[index]=spellid;
        }
        std::string GetName() { return m_name; }
        void SetName(std::string newname) { m_name=newname; }

        void SavePetToDB();
        void LoadPetFromDB(Unit* owner, uint32 id);
        void DeletePetFromDB();

    protected:
        uint32 m_spells[PETMAXSPELLS];
        std::string m_name;
        uint32 m_actState;
        uint32 m_fealty;
};
#endif
