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

#ifndef __UNIT_H
#define __UNIT_H

#include "Common.h"
#include "Object.h"
#include "ObjectAccessor.h"
#include "Opcodes.h"
#include "Mthread.h"

#include <list>

#define UNIT_MAX_SPELLS         4
#define PLAYER_MAX_SKILLS       127
#define PLAYER_SKILL(x)         (PLAYER_SKILL_INFO_START + (x*3))
// DWORD definitions gathered from windows api
#define SKILL_VALUE(x)          uint16(x)
#define SKILL_MAX(x)            uint16((uint32(x) >> 16))
#define MAKE_SKILL_VALUE(v, m) ((uint32)(((uint16)(v)) | (((uint32)((uint16)(m))) << 16)))

#define NULL_SLOT 255

struct SpellEntry;
struct Modifier;

class Aura;
class Spell;
class DynamicObject;

struct DamageShield
{
    uint32 m_spellId;
    uint32 m_damage;
    Unit *m_caster;
};

struct ProcTriggerSpell
{
    uint32 trigger;
    uint32 spellId;
    uint64 caster;
    uint32 procChance;
    uint32 procFlags;
    uint32 procCharges;
};

enum DeathState
{
    ALIVE = 0,
    JUST_DIED,
    CORPSE,
    DEAD
};

enum UnitState
{
    UNIT_STAT_STOPPED       = 0,
    UNIT_STAT_DIED          = 1,
    UNIT_STAT_ATTACKING     = 2,                            // player is attacking someone
    UNIT_STAT_ATTACK_BY     = 4,                            // player is attack by someone
                                                            // player is in combat mode
    UNIT_STAT_IN_COMBAT     = (UNIT_STAT_ATTACKING | UNIT_STAT_ATTACK_BY),
    UNIT_STAT_STUNDED       = 8,
    UNIT_STAT_ROAMING       = 16,
    UNIT_STAT_CHASE         = 32,
    UNIT_STAT_SEARCHING     = 64,
    UNIT_STAT_FLEEING       = 128,
    UNIT_STAT_MOVING        = (UNIT_STAT_ROAMING | UNIT_STAT_CHASE | UNIT_STAT_SEARCHING | UNIT_STAT_FLEEING),
    UNIT_STAT_IN_FLIGHT     = 256,                          // player is i n flight mode
    UNIT_STAT_FOLLOW        = 512,
	UNIT_STAT_ROOT			= 1024,
	UNIT_STAT_CONFUSED		= 2048,
    UNIT_STAT_ALL_STATE     = 0xffff                        //(UNIT_STAT_STOPPED | UNIT_STAT_MOVING | UNIT_STAT_IN_COMBAT | UNIT_STAT_IN_FLIGHT)
};

struct Hostil
{
    uint64 UnitGuid;
    float Hostility;
    bool operator <(Hostil item)
    {
        if(Hostility < item.Hostility)
            return true;
        else
            return false;
    };
};

class MANGOS_DLL_SPEC Unit : public Object
{
    public:
        typedef std::set<Unit*> AttackerSet;
        typedef std::list<Aura*> AuraList;
        virtual ~Unit ( );

        virtual void Update( uint32 time );

        void setAttackTimer(uint32 time, bool rangeattack = false);
        bool isAttackReady() const { return m_attackTimer == 0; }
        bool canReachWithAttack(Unit *pVictim) const;
        SpellEntry *reachWithSpellAttack(Unit *pVictim);

        inline AttackerSet getAttackerSet( ) {  return m_attackers; }
        inline void addAttacker(Unit *pAttacker)
        {
            AttackerSet::iterator itr = m_attackers.find(pAttacker);
            if(itr == m_attackers.end())
                m_attackers.insert(pAttacker);
        }
        inline void removeAttacker(Unit *pAttacker)
        {
            AttackerSet::iterator itr = m_attackers.find(pAttacker);
            if(itr != m_attackers.end())
                m_attackers.erase(itr);
        }

        inline void addUnitState(uint32 f) { m_state |= f; };
        inline bool hasUnitState(const uint32 f) const { return (m_state & f); }
        inline void clearUnitState(uint32 f) { m_state &= ~f; };

        inline uint32 getLevel() { return (GetUInt32Value(UNIT_FIELD_LEVEL)); };
        inline uint8 getRace() { return (uint8)m_uint32Values[ UNIT_FIELD_BYTES_0 ] & 0xFF; };
        inline uint8 getClass() { return (uint8)(m_uint32Values[ UNIT_FIELD_BYTES_0 ] >> 8) & 0xFF; };
        inline uint8 getGender() { return (uint8)(m_uint32Values[ UNIT_FIELD_BYTES_0 ] >> 16) & 0xFF; };
        inline uint8 getStandState() { return (uint8)m_uint32Values[ UNIT_FIELD_BYTES_1 ] & 0xFF; };

        void DealDamage(Unit *pVictim, uint32 damage, uint32 procFlag);
        void DoAttackDamage(Unit *pVictim, uint32 *damage, uint32 *blocked_amount, uint32 *damageType, uint32 *hitInfo, uint32 *victimState,uint32 *absorbDamage,uint32 *turn);
        uint32 CalDamageAbsorb(Unit *pVictim,uint32 School,const uint32 damage);
        void HandleEmoteCommand(uint32 anim_id);
        void AttackerStateUpdate (Unit *pVictim, uint32 damage);

        float GetUnitDodgeChance(){ return m_floatValues[ PLAYER_DODGE_PERCENTAGE ]; }
        float GetUnitParryChance(){ return m_floatValues[ PLAYER_PARRY_PERCENTAGE ]; }
        float GetUnitBlockChance(){ return m_floatValues[ PLAYER_BLOCK_PERCENTAGE ]; }
        float GetUnitCriticalChance(){ return m_floatValues[ PLAYER_CRIT_PERCENTAGE ]; }

        uint32 GetUnitBlockValue() { return (uint32)m_uint32Values[ UNIT_FIELD_ARMOR ]; }
        uint32 GetUnitStrength() { return (uint32)m_uint32Values[ UNIT_FIELD_STR ]; }
        uint32 GetUnitMeleeSkill(){ return (uint32)m_uint32Values[ UNIT_FIELD_ATTACK_POWER ]; }

        bool isVendor()       { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR ); }
        bool isTrainer()      { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER ); }
        bool isQuestGiver()   { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER ); }
        bool isGossip()       { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP ); }
        bool isTaxi()         { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TAXIVENDOR ); }
        bool isGuildMaster()  { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PETITIONER ); }
        bool isBattleMaster() { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BATTLEFIELDPERSON ); }
        bool isBanker()       { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER ); }
        bool isInnkeeper()    { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER ); }
        bool isSpiritHealer() { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER ); }
        bool isTabardVendor() { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDVENDOR ); }
        bool isAuctioner()    { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER ); }
        bool isArmorer()      { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_ARMORER ); }
        //Need fix or use this
        bool isGuard() const  { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GUARD); }

        bool isStunned() { return m_attackTimer == 0;};

        void PeriodicAuraLog(Unit *pVictim, SpellEntry *spellProto, Modifier *mod);
        void SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage);
        void CastSpell(Unit* caster,Unit* Victim, uint32 spellId, bool triggered);

        void DeMorph();

        void smsg_AttackStop(uint64 victimGuid);

        virtual void DealWithSpellDamage(DynamicObject &);
        virtual void MoveOutOfRange(Player &) {  }

        bool isAlive() { return (m_deathState == ALIVE); };
        bool isDead() { return ( m_deathState == DEAD || m_deathState == CORPSE ); };
        virtual void setDeathState(DeathState s)
        {
            m_deathState = s;
        };
        DeathState getDeathState() { return m_deathState; }

        bool AddAura(Aura *aur, bool uniq = false);

        void RemoveFirstAuraByCategory(uint32 category);
        void RemoveAura(AuraList::iterator i);
        void RemoveAura(uint32 spellId, uint32 effindex);
        void RemoveAura(uint32 spellId);

        void RemoveAllAuras();
        //void SetAura(Aura* Aur){ m_Auras = Aur; }
        bool SetAurDuration(uint32 spellId,Unit* caster,uint32 duration);
        uint32 GetAurDuration(uint32 spellId,Unit* caster);
        Aura* tmpAura;

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
        uint32 m_immuneToEffect;
        uint32 m_immuneToState;
        uint32 m_immuneToSchool;
        uint32 m_immuneToDmg;
        uint32 m_immuneToDispel;
        uint32 m_stealth;
        uint32 m_ReflectSpellSchool;
        uint32 m_ReflectSpellPerc;
        float m_speed;

        bool isInFront(Unit* target,float distance);
        bool setInFront(Unit* target);

        bool m_silenced;
        bool waterbreath;
        std::list<struct DamageShield> m_damageShields;

        struct DamageManaShield* m_damageManaShield;

        uint32 m_spells[UNIT_MAX_SPELLS];
        Spell * m_currentSpell;

        float GetHostility(uint64 guid);
        Hostil* GetHostil(uint64 guid);
        std::list<Hostil*> GetHostilList() { return m_hostilList; }
        void AddHostil(uint64 guid, float hostility);
        Aura* GetAura(uint32 spellId, uint32 effindex);
        AuraList GetAuras( ) {return m_Auras;}
        void SendMoveToPacket(float x, float y, float z, bool run);
		void AddItemEnchant(uint32 enchant_id);
		void setTransForm(uint32 spellid) { m_transform = spellid;}
		uint32 getTransForm() { return m_transform;}

    protected:
        Unit ( );

        void _RemoveStatsMods();
        void _ApplyStatsMods();
        void ApplyStats(bool apply);

        void _RemoveAllAuraMods();
        void _ApplyAllAuraMods();

        void _UpdateSpells(uint32 time);
        void _UpdateHostil( uint32 time );
        //void _UpdateAura();

        //Aura* m_aura;
        //uint32 m_auraCheck, m_removeAuraTimer;

        uint32 m_state;
        uint32 m_attackTimer;

        AttackerSet m_attackers;
        DeathState m_deathState;

        AuraList m_Auras;

        std::list<Hostil*> m_hostilList;
		uint32 m_transform;

};
#endif
