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

enum PetState
{
    STATE_RA_STAY           = 1,
    STATE_RA_FOLLOW         = 2,
    STATE_RA_REACTIVE       = 4,
    STATE_RA_PROACTIVE      = 8,
    STATE_RA_PASSIVE        = 16,
    STATE_RA_SPELL1         = 32,
    STATE_RA_SPELL2         = 64,
    STATE_RA_SPELL3         = 128,
    STATE_RA_SPELL4         = 256,
    STATE_RA_AUTOSPELL      = STATE_RA_SPELL1 | STATE_RA_SPELL2 | STATE_RA_SPELL3 | STATE_RA_SPELL4
};

#define OWNER_MAX_DISTANCE 100

#define PET_FOLLOW_DIST  1
#define PET_FOLLOW_ANGLE (M_PI/2)

class Pet : public Creature
{
    public:
        Pet();
        virtual ~Pet(){};

        uint32 GetActState() { return m_actState; }
        void SetActState(uint32 st) { m_actState=st; }
        void AddActState(uint32 st) { m_actState |= st; }
        void ClearActState(uint32 st) { m_actState &= ~st; };
        bool HasActState(uint32 st) { return m_actState & st;};
        uint32 GetFealty() { return m_fealty; }
        void SetFealty(uint32 fealty) { m_fealty=fealty; }
        char const* GetName() const { return m_name.c_str(); }
                                                            // overwrite Cerature::GetName
        void SetName(std::string newname) { m_name=newname; }
        bool LoadPetFromDB( Unit* owner,uint32 petentry = 0 );
        void SaveToDB();                                    // overwrited of Creature::SaveToDB
        void DeleteFromDB();                                // overwrited of Creature::DeleteFromDB
        void Unsummon();

        void setDeathState(DeathState s);                   // overwrite virtual Creature::setDeathState and Unit::setDeathState

    protected:
        std::string m_name;
        uint32 m_actState;
        uint32 m_fealty;
};
#endif
