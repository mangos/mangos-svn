/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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
#include "ObjectAccessor.h"

#include <list>
#define UF_TARGET_DIED  1
#define UF_ATTACKING    2                         

class Affect;
class Modifier;
class Spell;
class DynamicObject;

enum DeathState
{
    ALIVE = 0,                                    
    JUST_DIED,                                    
    CORPSE,                                       
    DEAD                                          
};





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

        
        inline void addStateFlag(uint32 f) { m_state |= f; };
        inline void clearStateFlag(uint32 f) { m_state &= ~f; };

        
        inline uint8 getLevel() { return (uint8)m_uint32Values[ UNIT_FIELD_LEVEL ]; };
        inline uint8 getRace() { return (uint8)m_uint32Values[ UNIT_FIELD_BYTES_0 ] & 0xFF; };
        inline uint8 getClass() { return (uint8)(m_uint32Values[ UNIT_FIELD_BYTES_0 ] >> 8) & 0xFF; };
        inline uint8 getGender() { return (uint8)(m_uint32Values[ UNIT_FIELD_BYTES_0 ] >> 16) & 0xFF; };
        inline uint8 getStandState() { return (uint8)m_uint32Values[ UNIT_FIELD_BYTES_1 ] & 0xFF; };

        
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
    {
    return m_attackTimer == 0;
        };
        void PeriodicAuraLog(Unit *pVictim, uint32 spellID, uint32 damage, uint32 damageType);
        void SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage);
        
        void CastSpell(Unit* caster,Unit* Victim, uint32 spellId, bool triggered);
        void CalcRage( uint32 damage );
        void RegenerateAll();
        void Regenerate(uint16 field_cur, uint16 field_max, bool switch_);  void setRegenTimer(uint32 time) {m_regenTimer = time;};
        
        void DeMorph();   
    
    void smsg_AttackStop(uint64 victimGuid);
    bool isPlayer();
    bool isUnit();

    virtual void DealWithSpellDamage(DynamicObject &);
    virtual void MoveOutOfRange(Player &) {  }

        
        bool isAlive() { return m_deathState == ALIVE; };
        bool isDead() { return ( m_deathState == DEAD || m_deathState == CORPSE ); };
        virtual void setDeathState(DeathState s)
        {
            m_deathState = s;
        };
        DeathState getDeathState() { return m_deathState; }

        
        bool AddAffect(Affect *aff, bool uniq = false);
        
        void RemoveAffect(Affect *aff);
        
        bool RemoveAffect(uint32 type);
        void RemoveAffectById(uint32 spellId);
        
        void RemoveAllAffects();
        void SetAura(Affect* aff){ m_aura = aff; }
        bool SetAffDuration(uint32 spellId,Unit* caster,uint32 duration);
        uint32 GetAffDuration(uint32 spellId,Unit* caster);
        Affect* tmpAffect;

        
        virtual void ApplyModifier(const Modifier *mod, bool apply, Affect* parent);

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

        
        bool isInFront(Unit* target,float distance);
        bool setInFront(Unit* target, float distance);

        
        bool m_silenced;
        std::list<struct DamageShield> m_damageShields;
        std::list<struct ProcTriggerSpell> m_procSpells;

    protected:
        Unit ( );

        float m_speed;
        
        void _RemoveAllAffectMods();
        
        void _ApplyAllAffectMods();

        void _AddAura(Affect *aff);
        void _RemoveAura(Affect *aff);
        Affect* FindAff(uint32 spellId);

        void _UpdateSpells(uint32 time);
        void _UpdateAura();

        Affect* m_aura;
        uint32 m_auraCheck, m_removeAuraTimer;

        
        bool _IsAffectPositive(Affect *aff) { return true; }

        uint32 m_regenTimer;
        uint32 m_state;                           
        uint32 m_attackTimer;                     

        
        AttackerSet m_attackers;
        DeathState m_deathState;

        typedef std::list<Affect*> AffectList;
        AffectList m_affects;

        
        Spell * m_currentSpell;

        

        float geteasyangle( float angle );        

        

        
        bool inarc( float radius,  float xM, float yM, float fov, float orientation, float xP, float yP );
        float getangle( float xe, float ye, float xz, float yz );
        float getdistance( float xe, float ye, float xz, float yz );
};
#endif
