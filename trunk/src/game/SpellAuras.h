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
#ifndef MANGOS_SPELLAURAS_H
#define MANGOS_SPELLAURAS_H

#include "Spell.h"
#include "SpellAuraDefines.h"

struct DamageManaShield
{
    uint32 m_spellId;
    uint32 m_modType;
    int32 m_schoolType;
    uint32 m_totalAbsorb;
    uint32 m_currAbsorb;
};

struct Modifier
{
    uint8 m_auraname;
    int32 m_amount;
    int32 m_miscvalue;
    uint32 m_miscvalue2;
    uint32 periodictime;
};

class Unit;
struct SpellEntry;
struct ProcTriggerSpell;

class Aura
{
    public:
        typedef std::list<Modifier*> ModList;

        //aura handlers
        void HandleNULL(bool Apply);
        void HandlePeriodicDamage(bool apply);
        void HandleModConfuse(bool apply);
        void HandleModFear(bool Apply);
        void HandlePeriodicHeal(bool apply);
        void HandleModAttackSpeed(bool apply);
        void HandleModThreat(bool apply);
        void HandleAuraWaterWalk(bool Apply);
        void HandleAuraFeatherFall(bool Apply);
        void HandleAddModifier(bool Apply);
        void HandleAuraModStun(bool Apply);
        void HandleAuraModRangedAttackPower(bool Apply);
        void HandleAuraModIncreaseSpeedAlways(bool Apply);
        void HandleAuraModIncreaseEnergyPercent(bool Apply);
        void HandleAuraModIncreaseHealthPercent(bool Apply);
        void HandlePeriodicTriggerSpell(bool apply);
        void HandleAuraModResistanceExclusive(bool Apply);
        void HandleAuraSafeFall(bool Apply);
        void HandleAuraDamageShield(bool Apply);
        void HandleModStealth(bool Apply);
        void HandleModDetect(bool Apply);
        void HandleInvisibility(bool Apply);
        void HandleInvisibilityDetect(bool Apply);
        void HandleAuraModResistance(bool Apply);
        void HandleAuraModRoot(bool Apply);
        void HandleAuraModSilence(bool Apply);
        void HandleAuraModStat(bool Apply);
        void HandleAuraModIncreaseSpeed(bool Apply);
        void HandleAuraModIncreaseMountedSpeed(bool Apply);
        void HandleAuraModDecreaseSpeed(bool Apply);
        void HandleAuraModIncreaseHealth(bool Apply);
        void HandleAuraModIncreaseEnergy(bool Apply);
        void HandleAuraModShapeshift(bool Apply);
        void HandleAuraModEffectImmunity(bool Apply);
        void HandleAuraModStateImmunity(bool Apply);
        void HandleAuraModSchoolImmunity(bool Apply);
        void HandleAuraModDmgImmunity(bool Apply);
        void HandleAuraModDispelImmunity(bool Apply);
        void HandleAuraProcTriggerSpell(bool Apply);
        void HandleAuraProcTriggerDamage(bool Apply);
        void HandleAuraTracCreatures(bool Apply);
        void HandleAuraTracResources(bool Apply);
        void HandleAuraModParryPercent(bool Apply);
        void HandleAuraModDodgePercent(bool Apply);
        void HandleAuraModBlockPercent(bool Apply);
        void HandleAuraModCritPercent(bool Apply);
        void HandleAuraModScale(bool Apply);
        void HandleAuraMounted(bool Apply);
        void HandleWaterBreathing(bool apply);
        void HandleModBaseResistance(bool apply);
        void HandleModRegen(bool apply);
        void HandleModPowerRegen(bool apply);
        void HandleChannelDeathItem(bool apply);
        void HandleAuraModAttackPower(bool Apply);
        void HandleAuraTransform(bool Apply);
        void HandleAuraModIncreaseSwimSpeed(bool Apply);
        void HandleAuraManaShield(bool Apply);
        void HandleAuraSchoolAbsorb(bool Apply);
        void HandleReflectSpellsSchool(bool Apply);
        void HandleModMechanicImmunity(bool Apply);
        void HandleAuraModSkill(bool Apply);
        void HandleModDamagePercentDone(bool Apply);
        void HandleModPercentStat(bool Apply);
        void HandleModResistancePercent(bool Apply);
        void HandleAuraModBaseResistancePCT(bool Apply);
        void HandleRangedAmmoHaste(bool Apply);

        Aura() : m_spellId(0), m_effIndex(0xFFFF), m_caster(NULL), m_target(NULL), m_duration(0), m_auraSlot(0xFF), m_positive(false), m_permanent(false), m_isPeriodic(false), m_procSpell(NULL), m_isTrigger(false) {}
        Aura(SpellEntry* spellproto, uint32 eff, Unit *caster, Unit *target);

        void SetModifier(uint8 t, int32 a, uint32 pt, int32 miscValue, uint32 miscValue2);
        void SetModifier(Modifier* mod) {m_modifier=mod;}
        Modifier* GetModifier() {return m_modifier;}

        SpellEntry* GetSpellProto() const { return sSpellStore.LookupEntry( m_spellId ); }
        uint32 GetId() const{ return m_spellId; }
        uint32 GetEffIndex() const{ return m_effIndex; }
        void SetEffIndex(uint32 eff) { m_effIndex = eff; }
        int32 GetAuraDuration() const { return m_duration; }
        void SetAuraDuration(int32 duration) { m_duration = duration; }
        void UpdateAuraDuration();

        Unit* GetCaster() const { return m_caster; }
        Unit* GetTarget() const { return m_target; }
        void SetCaster(Unit* caster) { m_caster = caster; }
        void SetTarget(Unit* target) { m_target = target; }

        uint8 GetAuraSlot() const { return m_auraSlot; }
        void SetAuraSlot(uint8 slot) { m_auraSlot = slot; }

        bool IsPositive() { return m_positive; }
        void SetNegative() { m_positive = false; }
        void SetPositive() { m_positive = true; }

        bool IsPermanent() { return m_permanent; }
        void SetPermanent(bool value) { m_permanent = value; }

        void Update(uint32 diff);
        void ApplyModifier(bool Apply);

        void _AddAura();
        void _RemoveAura();
        uint32 CalculateDamage();

        ProcTriggerSpell* GetProcSpell() { return m_procSpell; }
        void TriggerSpell();
        void SendCoolDownEvent();

    private:

        ProcTriggerSpell *m_procSpell;
        Modifier *m_modifier;
        uint32 m_spellId;
        uint32 m_effIndex;
        //SpellEntry *m_spellProto;
        Unit* m_caster;
        Unit* m_target;
        int32 m_duration;

        uint8 m_auraSlot;

        bool m_positive;
        bool m_permanent;
        bool m_isPeriodic;
        bool m_isTrigger;

        uint32 m_periodicTimer;
        uint32 m_PeriodicEventId;
};

typedef void(Aura::*pAuraHandler)(bool Apply);
#endif
