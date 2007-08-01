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
#include "Unit.h"

enum PetType
{
    SUMMON_PET              = 0,
    HUNTER_PET              = 1,
    GUARDIAN_PET            = 2,
    MINI_PET                = 3,
    MAX_PET_TYPE            = 4
};

extern char const* petTypeSuffix[MAX_PET_TYPE];

enum PetSaveMode
{
    PET_SAVE_AS_DELETED       =-1,
    PET_SAVE_AS_CURRENT       = 0,
    PET_SAVE_IN_STABLE_SLOT_1 = 1,
    PET_SAVE_IN_STABLE_SLOT_2 = 2,
    PET_SAVE_NOT_IN_SLOT      = 3
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

enum HappinessState
{
    UNHAPPY = 1,
    CONTENT = 2,
    HAPPY   = 3
};

enum LoyaltyLevel
{
    REBELLIOUS  = 1,
    UNRULY      = 2,
    SUBMISSIVE  = 3,
    DEPENDABLE  = 4,
    FAITHFUL    = 5,
    BEST_FRIEND = 6
};

enum ActiveStates
{
    ACT_ENABLED  = 0xC100,
    ACT_DISABLED = 0x8100,
    ACT_COMMAND  = 0x0700,
    ACT_REACTION = 0x0600,
    ACT_CAST     = 0x0100,
    ACT_PASSIVE  = 0x0000,
    ACT_DECIDE   = 0x0001
};

enum PetSpellState
{
    PETSPELL_UNCHANGED = 0,
    PETSPELL_CHANGED   = 1,
    PETSPELL_NEW       = 2,
    PETSPELL_REMOVED   = 3
};

struct PetSpell
{
    uint16 slotId;
    uint16 active;
    PetSpellState state;
};

enum ActionFeedback
{
    FEEDBACK_NONE            = 0,
    FEEDBACK_PET_DEAD        = 1,
    FEEDBACK_NOTHING_TO_ATT  = 2,
    FEEDBACK_CANT_ATT_TARGET = 3
};

enum PetTalk
{
    PET_TALK_SPECIAL_SPELL  = 0,
    PET_TALK_ATTACK         = 1
};

typedef HM_NAMESPACE::hash_map<uint16, PetSpell*> PetSpellMap;
typedef std::map<uint32,uint32> TeachSpellMap;
typedef std::set<uint32> AutoSpellList;

#define HAPPINESS_LEVEL_SIZE        333000

extern const uint32 LevelUpLoyalty[6];
extern const uint32 LevelStartLoyalty[6];

#define ACTIVE_SPELLS_MAX           4

#define OWNER_MAX_DISTANCE 100

#define PET_FOLLOW_DIST  1
#define PET_FOLLOW_ANGLE (M_PI/2)

class Pet : public Creature
{
    public:
        explicit Pet(WorldObject *instantiator, PetType type);
        virtual ~Pet();

        void AddToWorld();
        void RemoveFromWorld();

        PetType getPetType() const { return m_petType; }
        bool isControlled() const { return getPetType()==SUMMON_PET || getPetType()==HUNTER_PET; }
        uint32 GetPetNumber() const { return GetUInt32Value(UNIT_FIELD_PETNUMBER); }
        
        void SetCommandState(uint8 st) { m_CommandState = st; } //only for pets, charms can't be set on special modes
        uint8 GetCommandState() { return m_CommandState; }
        bool HasCommandState(CommandStates state) { return (m_CommandState == state); }
        void SetReactState(uint8 st) { m_ReactSate = st; }
        uint8 GetReactState() { return m_ReactSate; }
        bool HasReactState(ReactStates state) { return (m_ReactSate == state); }

        bool Create (uint32 guidlow, uint32 mapid, float x, float y, float z, float ang, uint32 Entry);
        bool CreateBaseAtCreature( Creature* creature );
        bool LoadPetFromDB( Unit* owner,uint32 petentry = 0,uint32 petnumber = 0, bool current = false );
        void SavePetToDB(PetSaveMode mode);
        void Remove(PetSaveMode mode);

        void setDeathState(DeathState s);                   // overwrite virtual Creature::setDeathState and Unit::setDeathState
        void Update(uint32 diff);                           // overwrite virtual Creature::Update and Unit::Update

        void RegenerateFocus();
        void LooseHappiness();
        void TickLoyaltyChange();
        void ModifyLoyalty(int32 addvalue);
        HappinessState GetHappinessState();
        uint32 GetMaxLoyaltyPoints(uint32 level);
        uint32 GetStartLoyaltyPoints(uint32 level);
        void KillLoyaltyBonus(uint32 level);
        uint32 GetLoyaltyLevel(){ return ((GetUInt32Value(UNIT_FIELD_BYTES_1) >> 8) & 0xFF);};
        void SetLoyaltyLevel(LoyaltyLevel level);
        void GivePetXP(uint32 xp);
        void GivePetLevel(uint32 level);
        bool InitStatsForLevel(uint32 level);
        bool HaveInDiet(ItemPrototype const* item) const;
        uint32 GetCurrentFoodBenefitLevel(uint32 itemlevel);
        void SetDuration(uint32 dur) { m_duration = dur; }

        int32 GetBonusDamage() { return m_bonusdamage; }
        void SetBonusDamage(int32 damage) { m_bonusdamage = damage; }

        bool UpdateStats(Stats stat);
        bool UpdateAllStats();
        void UpdateResistances(uint32 school);
        void UpdateArmor();
        void UpdateMaxHealth();
        void UpdateMaxPower(Powers power);
        void UpdateAttackPowerAndDamage(bool ranged = false);
        void UpdateDamagePhysical(WeaponAttackType attType);

        bool   CanTakeMoreActiveSpells(uint32 SpellIconID);
        void   ToggleAutocast(uint32 spellid, bool apply);
        bool   HasTPForSpell(uint32 spellid);
        int32  GetTPForSpell(uint32 spellid);
        void   SendActionFeedback(uint8 msg);
        void   SendPetTalk(PetTalk pettalk);
        void   SendCastFail(uint32 spellid, uint8 msg);

        bool HasSpell(uint32 spell);
        void SendSpellCooldown(uint32 spell_id, time_t cooltime);
        void AddTeachSpell(uint32 learned_id, uint32 source_id) { m_teachspells[learned_id] = source_id; }

        void _LoadSpellCooldowns();
        void _SaveSpellCooldowns();
        void _LoadAuras(uint32 timediff);
        void _SaveAuras();
        void _LoadSpells(uint32 timediff);
        void _SaveSpells();

        bool addSpell(uint16 spell_id,uint16 active = ACT_DECIDE, PetSpellState state = PETSPELL_NEW, uint16 slot_id=0xffff);
        bool learnSpell(uint16 spell_id);
        void removeSpell(uint16 spell_id);
        bool _removeSpell(uint16 spell_id);

        PetSpellMap     m_spells;
        TeachSpellMap   m_teachspells;
        AutoSpellList   m_autospells;

        void InitPetCreateSpells();
        void CheckLearning(uint32 spellid);
        uint32 resetTalentsCost() const;

        void  SetTP(int32 TP);
        int32 GetDispTP();

        int32   m_TrainingPoints;
        uint32  m_resetTalentsCost;
        time_t  m_resetTalentsTime;

        UnitActionBarEntry* GetActionBarEntry(uint32 index) { return &(PetActionBar[index]); }

        bool    m_removed;                                  // prevent overwrite pet state in DB at next Pet::Update if pet already removed(saved)
    protected:
        uint32  m_regenTimer;
        uint32  m_happinessTimer;
        uint32  m_loyaltyTimer;
        PetType m_petType;
        uint32  m_duration;                                 // time until unsummon (used mostly for summoned guardians and not used for controlled pets)
        int32   m_loyaltyPoints;
        int32   m_bonusdamage;
        uint8   m_CommandState;
        uint8   m_ReactSate;
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
