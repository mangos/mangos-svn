/* Unit.h
 *
 * Copyright (C) 2004 Wow Daemon
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

#ifndef __UNIT_H
#define __UNIT_H

#include "Object.h"
#ifdef ENABLE_GRID_SYSTEM
#include "ObjectAccessor.h"
#endif

#define UF_TARGET_DIED  1
#define UF_ATTACKING    2                         // this unit is attacking it's selection

class Affect;
class Modifier;
class Spell;
class DynamicObject;

enum Factions
{
    // Fields
    Alliance = 0x73,
    ArgentDawn = 0x16,
    BattlegroundNeutral = 0x17,
    Beast = 0x20,
    BlacksmithingArmorSmithing = 13,
    BlacksmithingAxeSmithing = 14,
    BlacksmithingDragonScaleSmithing = 0x13,
    BlacksmithingElementalSmithing = 20,
    BlacksmithingGnomeSmithing = 0x11,
    BlacksmithingGoblinSmithing = 0x12,
    BlacksmithingHammerSmithing = 15,
    BlacksmithingSwordSmithing = 0x10,
    BlacksmithingTribalSmithing = 0x15,
    BloodsailBuccaneers = 0x18,
    BootyBay = 10,
    CaerDarrow = 0x19,
    DarkspearTrolls = 5,
    Darnasus = 1,
    Everlook = 11,
    EvilBeast = 0x2c,
    Friend = 0x23,
    FrostwolfClan = 0x1a,
    Gadgetzan = 12,
    GellisClanCentaur = 0x1b,
    GnomereganExiles = 2,
    Horde = 6,
    HydraxianWaterlords = 0x2a,
    IronForge = 3,
    MagranClanCentaure = 0x1d,
    Monster = 60,
    MoroGai = 30,
    NoFaction = 0,
    Ogrimmar = 6,
    Prey = 0x1f,
    Ratchet = 13,
    RavasawrTrainers = 0x24,
    Ravenholdt = 0x2b,
    ShatterspearTrolls = 0x21,
    Shendralar = 0x22,
    SteamWheedleCartel = 9,
    StormpikeGuard = 0x29,
    Stormwind = 4,
    Syndicate = 0x1c,
    ThoriumBrotherhood = 0x25,
    ThunderBluff = 7,
    ThundermawFurbolgs = 0x26,
    Undercity = 8,
    WildHammerClan = 0x27,
    WinterSaberTrainers = 40
};

enum DeathState
{
    ALIVE = 0,                                    // Unit is alive and well
    JUST_DIED,                                    // Unit has JUST died
    CORPSE,                                       // Unit has died but remains in the world as a corpse
    DEAD                                          // Unit is dead and his corpse is gone from the world
};

//====================================================================
//  Unit
//  Base object for Players and Creatures
//====================================================================
class Unit : public Object
{
    public:
        typedef std::set<Unit*> AttackerSet;
        virtual ~Unit ( );

        virtual void Update( uint32 time );

        void setAttackTimer(uint32 time);
        bool isAttackReady() const { return m_attackTimer == 0; }
        bool canReachWithAttack(Unit *pVictim) const;

        inline void removeAttacker(Unit *pAttacker)
        {
            AttackerSet::iterator itr = m_attackers.find(pAttacker);
            if(itr != m_attackers.end())
                m_attackers.erase(itr);
        }

        /// State flags are server-only flags to help me know when to do stuff, like die, or attack
        inline void addStateFlag(uint32 f) { m_state |= f; };
        inline void clearStateFlag(uint32 f) { m_state &= ~f; };

        /// Stats
        inline uint8 getLevel() { return (uint8)m_uint32Values[ UNIT_FIELD_LEVEL ]; };
        inline uint8 getRace() { return (uint8)m_uint32Values[ UNIT_FIELD_BYTES_0 ] & 0xFF; };
        inline uint8 getClass() { return (uint8)(m_uint32Values[ UNIT_FIELD_BYTES_0 ] >> 8) & 0xFF; };
        inline uint8 getGender() { return (uint8)(m_uint32Values[ UNIT_FIELD_BYTES_0 ] >> 16) & 0xFF; };
        inline uint8 getStandState() { return (uint8)m_uint32Values[ UNIT_FIELD_BYTES_1 ] & 0xFF; };

        //// Combat
        void DealDamage(Unit *pVictim, uint32 damage, uint32 procFlag);
		void DoAttackDamage(Unit *pVictim, uint32 damage, uint32 was_blocked, uint32 damageType, uint32 hitInfo, uint32 victimInfo);
		void HandleEmoteCommand(uint32 anim_id);
        void AttackerStateUpdate (Unit *pVictim, uint32 damage);
    float GetUnitDodgeChance();
    float GetUnitParryChance();
    float GetUnitBlockChance();
    float GetUnitCriticalChance();

    uint32 GetUnitBlockValue() 
    { 
	return (uint32)m_uint32Values[ UNIT_FIELD_ARMOR ]; 
    };
    uint32 GetUnitStrength() 
    { 
	return (uint32)m_uint32Values[ UNIT_FIELD_STR ]; 
    };
		uint32 GetUnitMeleeSkill()
    {
	return (uint32)m_uint32Values[ UNIT_FIELD_ATTACKPOWER ]; 
    };

    bool isStunned() 
    {// UQ1: FIXME - Add stun...
	return m_attackTimer == 0;
		};
        void PeriodicAuraLog(Unit *pVictim, uint32 spellID, uint32 damage, uint32 damageType);
        void SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage);
        // void HandleProc(ProcTriggerSpell *pts, uint32 flag) {};
        void CastSpell(Unit* caster,Unit* Victim, uint32 spellId, bool triggered);
        void CalcRage( uint32 damage );
        void RegenerateAll();
        void Regenerate(uint16 field_cur, uint16 field_max, bool switch_);  void setRegenTimer(uint32 time) {m_regenTimer = time;};
        // Morph stuff  - morph into something, and out of something
        void DeMorph();   
    
    // overloaded method add cuz most of the time, an initial swing is by the player
    // and we do not need another hash search for the player
    void smsg_AttackStart(Unit *pVictim, Player *p);
    void smsg_AttackStart(Unit* pVictim);
    void smsg_AttackStop(uint64 victimGuid);
    bool isPlayer();
    bool isUnit();

#ifndef ENABLE_GRID_SYSTEM
        virtual void RemoveInRangeObject(Object* pObj)
        {
            if(pObj->GetTypeId() == TYPEID_PLAYER || pObj->GetTypeId() == TYPEID_UNIT)
            {
                AttackerSet::iterator i = m_attackers.find((Unit*)pObj);
                if(i != m_attackers.end())
                    m_attackers.erase(i);
            }
            Object::RemoveInRangeObject(pObj);
        }
#else
    virtual void DealWithSpellDamage(DynamicObject &);
    virtual void MoveOutOfRange(Player &) { /* the player just moved out of my range */ }
#endif

        /// Combat / Death Status
        bool isAlive() { return m_deathState == ALIVE; };
        bool isDead() { return ( m_deathState == DEAD || m_deathState == CORPSE ); };
        virtual void setDeathState(DeathState s)
        {
            m_deathState = s;
        };
        DeathState getDeathState() { return m_deathState; }

        //! Add affect to unit
        bool AddAffect(Affect *aff, bool uniq = false);
        //! Remove affect from unit
        void RemoveAffect(Affect *aff);
        //! Remove all affects with specified type
        bool RemoveAffect(uint32 type);
        void RemoveAffectById(uint32 spellId);
        //! Remove all affects
        void RemoveAllAffects();
        void SetAura(Affect* aff){ m_aura = aff; }
        bool SetAffDuration(uint32 spellId,Unit* caster,uint32 duration);
        uint32 GetAffDuration(uint32 spellId,Unit* caster);
        Affect* tmpAffect;

        //! Player should override it to use POS/NEG fields
        virtual void ApplyModifier(const Modifier *mod, bool apply, Affect* parent);

/*
        // Distance Calculation
        float CalcDistance(Unit *Ua, Unit *Ub);
        float CalcDistance(Unit *Ua, float PaX, float PaY, float PaZ);
        float CalcDistance(float PaX, float PaY, float PaZ, float PbX, float PbY, float PbZ);
*/

        void castSpell(Spell * pSpell);
        void InterruptSpell();
        bool m_meleeSpell;
        uint32 m_addDmgOnce;
        uint64 m_TotemSlot1;
        uint64 m_TotemSlot2;
        uint64 m_TotemSlot3;
        uint64 m_TotemSlot4;
        uint32 m_triggerSpell;
        uint32 m_triggerDamage;
        uint32 m_canMove;

        // Use it to Check if a Unit is in front of another one
        bool isInFront(Unit* target,float distance);
        bool setInFront(Unit* target, float distance);

        // Spell Effect Variables
        bool m_silenced;
        std::list<struct DamageShield> m_damageShields;
        std::list<struct ProcTriggerSpell> m_procSpells;

    protected:
        Unit ( );

        float m_speed;
        //! Temporary remove all affects
        void _RemoveAllAffectMods();
        //! Place all mods back
        void _ApplyAllAffectMods();

        void _AddAura(Affect *aff);
        void _RemoveAura(Affect *aff);
        Affect* FindAff(uint32 spellId);

        void _UpdateSpells(uint32 time);
        void _UpdateAura();

        Affect* m_aura;
        uint32 m_auraCheck, m_removeAuraTimer;

        // FIXME: implement it
        bool _IsAffectPositive(Affect *aff) { return true; }

        uint32 m_regenTimer;
        uint32 m_state;                           // flags for keeping track of some states
        uint32 m_attackTimer;                     // timer for attack

        /// Combat
        AttackerSet m_attackers;
        DeathState m_deathState;

        typedef std::list<Affect*> AffectList;
        AffectList m_affects;

        // Spell currently casting
        Spell * m_currentSpell;

        /* Some Functions for isInFront Calculation ( thanks to emperor and undefined for the formula ) */

        float geteasyangle( float angle );        /* converts to 360 > x > 0 */

        /* float calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y ); */

        /* Main Function called by isInFront(); */
        bool inarc( float radius,  float xM, float yM, float fov, float orientation, float xP, float yP );
        float getangle( float xe, float ye, float xz, float yz );
        float getdistance( float xe, float ye, float xz, float yz );
};
#endif
