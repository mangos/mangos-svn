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

#ifndef MANGOSSERVER_PET_H
#define MANGOSSERVER_PET_H

#include "ObjectDefines.h"
#include "Creature.h"

enum PetType
{
    SUMMON_PET = 0,
    HUNTER_PET,
    GUARDIAN_PET,
    MINI_PET
};

#define MAX_PET_TYPE 4

extern char const* petTypeSuffix[MAX_PET_TYPE];

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

enum PetSaveMode
{
    PET_SAVE_AS_CURRENT,
    PET_SAVE_AS_STORED,
    PET_SAVE_AS_DELETED
};

// Used for values in CreatureFamilyEntry.petDietMask
enum PetDietMask
{
    PET_DIET_MEAT     = 0x01,
    PET_DIET_FISH     = 0x02,
    PET_DIET_CHEESE   = 0x04,
    PET_DIET_BREAD    = 0x08,
    PET_DIET_FUNGAS   = 0x10,
    PET_DIET_FRUIT    = 0x20,
    PET_DIET_RAW_MEAT = 0x40,
    PET_DIET_RAW_FISH = 0x80
};

#define OWNER_MAX_DISTANCE 100

#define PET_FOLLOW_DIST  1
#define PET_FOLLOW_ANGLE (M_PI/2)
#define PET_FOLLOW_START_DIST  (PET_FOLLOW_DIST*2)

class Pet : public Creature
{
    public:
        explicit Pet(PetType type);
        virtual ~Pet() {}

        uint32 GetActState() { return m_actState; }
        void SetActState(uint32 st) { m_actState=st; }
        void AddActState(uint32 st) { m_actState |= st; }
        void ClearActState(uint32 st) { m_actState &= ~st; };
        bool HasActState(uint32 st) { return m_actState & st;};
        uint32 GetFealty() { return m_fealty; }
        void SetFealty(uint32 fealty) { m_fealty=fealty; }
        PetType getPetType() const { return m_petType; }
        bool isControlled() const { return getPetType()==SUMMON_PET || getPetType()==HUNTER_PET; }

        bool CreateBaseAtCreature( Creature* creature );
        bool LoadPetFromDB( Unit* owner,uint32 petentry = 0 );
        void SavePetToDB(PetSaveMode mode);
        void Remove(PetSaveMode mode);

        void setDeathState(DeathState s);                   // overwrite virtual Creature::setDeathState and Unit::setDeathState
        void Update(uint32 diff);                           // overwrite virtual Creature::Update and Unit::Update

	void RegenerateFocus();
        void GivePetXP(uint32 xp);
        void GivePetLevel(uint32 level);
        void InitStatsForLevel(uint32 level);
        bool HaveInDiet(ItemPrototype const* item) const;
    protected:
	uint32 m_regenTimer;
        uint32 m_actState;
        uint32 m_fealty;
        PetType m_petType;
    private:
        void SaveToDB()                                     // overwrited of Creature::SaveToDB     - don't must be called
        {
            assert(false);
        }
        void DeleteFromDB()                                 // overwrited of Creature::DeleteFromDB - don't must be called
        {
            assert(false);
        }
};

class PetWithIdCheck
{
    public:
        PetWithIdCheck(Unit const* owner, uint32 entry) : i_owner(owner), i_entry(entry) {}
        bool operator()(Unit const* u) const
        {
            if(u->GetTypeId()!=TYPEID_UNIT)
                return false;

            if(!((Creature*)u)->isPet())
                return false;

            if(u->GetEntry()!=i_entry)
                return false;

            if(u->GetOwnerGUID()!=i_owner->GetGUID())
                return false;

            return true;
        }
    private:
        Unit const* i_owner;
        uint32 i_entry;
};
#endif
