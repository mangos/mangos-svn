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
        //aura handlers
        void HandleNULL(bool Apply);
        void HandleBindSight(bool apply);
        void HandleModPossess(bool apply);
        void HandlePeriodicDamage(bool apply);
        void HandleAuraDummy(bool apply);
        void HandleModConfuse(bool apply);
        void HandleModCharm(bool apply);
        void HandleModFear(bool Apply);
        void HandlePeriodicHeal(bool apply);
        void HandleModAttackSpeed(bool apply);
        void HandleModThreat(bool apply);
        void HandleAuraWaterWalk(bool Apply);
        void HandleAuraFeatherFall(bool Apply);
        void HandleAddModifier(bool Apply);
        void HandleAuraModStun(bool Apply);
        void HandleModDamageDone(bool Apply);
        void HandleModDamageTaken(bool Apply);
        void HandleAuraEmpathy(bool Apply);
        void HandleAuraModRangedAttackPower(bool Apply);
        void HandleAuraModIncreaseSpeedAlways(bool Apply);
        void HandleAuraModIncreaseEnergyPercent(bool Apply);
        void HandleAuraModIncreaseHealthPercent(bool Apply);
        void HandleHaste(bool Apply);
        void HandlePeriodicTriggerSpell(bool apply);
        void HandlePeriodicEnergize(bool apply);
        void HandleAuraModResistanceExclusive(bool Apply);
        void HandleAuraSafeFall(bool Apply);
        void HandleAuraDamageShield(bool Apply);
        void HandleModStealth(bool Apply);
        void HandleModDetect(bool Apply);
        void HandleInvisibility(bool Apply);
        void HandleInvisibilityDetect(bool Apply);
        void HandleAuraModTotalHealthPercentRegen(bool Apply);
        void HandleAuraModTotalManaPercentRegen(bool Apply);
        void HandleAuraModResistance(bool Apply);
        void HandleAuraModRoot(bool Apply);
        void HandleAuraModSilence(bool Apply);
        void HandleReflectSpells(bool Apply);
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
        void HandlePeriodicLeech(bool Apply);
        void HandleModHitChance(bool Apply);
        void HandleModSpellHitChance(bool Apply);
        void HandleAuraModScale(bool Apply);
        void HandlePeriodicManaLeech(bool Apply);
        void HandleModCastingSpeed(bool Apply);
        void HandleAuraMounted(bool Apply);
        void HandleWaterBreathing(bool apply);
        void HandleModBaseResistance(bool apply);
        void HandleModRegen(bool apply);
        void HandleModPowerRegen(bool apply);
        void HandleChannelDeathItem(bool apply);
        void HandleModDamagePCTTaken(bool apply);
        void HandleModPCTRegen(bool Apply);
        void HandlePeriodicDamagePCT(bool Apply);
        void HandleAuraModAttackPower(bool Apply);
        void HandleAuraTransform(bool Apply);
        void HandleModSpellCritChance(bool Apply);
        void HandleAuraModIncreaseSwimSpeed(bool Apply);
        void HandleModDamageDoneCreature(bool Apply);
        void HandleAuraManaShield(bool Apply);
        void HandleAuraSchoolAbsorb(bool Apply);
        void HandleModSpellCritChanceShool(bool Apply);
        void HandleModPowerCost(bool Apply);
        void HandleModPowerCostSchool(bool Apply);
        void HandleReflectSpellsSchool(bool Apply);
        void HandleFarSight(bool Apply);
        void HandleModMechanicImmunity(bool Apply);
        void HandleAuraModSkill(bool Apply);
        void HandleModCreatureAttackPower(bool Apply);
        void HandleModDamagePercentDone(bool Apply);
        void HandleModPercentStat(bool Apply);
        void HandleModResistancePercent(bool Apply);
        void HandleAuraModBaseResistancePCT(bool Apply);
        void HandleModShieldBlock(bool Apply);
        void HandleModReputationGain(bool Apply);
        void HandleForceReaction(bool Apply);
        void HandleAuraModRangedHaste(bool apply);
        void HandleRangedAmmoHaste(bool Apply);
        void HandleModTotalPercentStat(bool Apply);

        Aura(SpellEntry* spellproto, uint32 eff, Unit *target, Unit *caster = NULL, Item* castItem = NULL);
        virtual ~Aura();

        void SetModifier(uint8 t, int32 a, uint32 pt, int32 miscValue, uint32 miscValue2);
        Modifier* GetModifier() {return &m_modifier;}

        SpellEntry* GetSpellProto() const { return sSpellStore.LookupEntry( m_spellId ); }
        uint32 GetId() const{ return m_spellId; }
        uint32 GetEffIndex() const{ return m_effIndex; }
        void SetEffIndex(uint32 eff) { m_effIndex = eff; }
        int32 GetAuraDuration() const { return m_duration; }
        void SetAuraDuration(int32 duration) { m_duration = duration; }
        time_t GetAuraApplyTime() { return m_applyTime; }
        void UpdateAuraDuration();

        uint64 const& GetCasterGUID() const { return m_caster_guid; }
        Unit* GetCaster() const;
        Unit* GetTarget() const { return m_target; }
        void SetTarget(Unit* target) { m_target = target; }

        uint8 GetAuraSlot() const { return m_auraSlot; }
        void SetAuraSlot(uint8 slot) { m_auraSlot = slot; }

        bool IsPositive() { return m_positive; }
        void SetNegative() { m_positive = false; }
        void SetPositive() { m_positive = true; }

        bool IsPermanent() const { return m_permanent; }
        void SetPermanent(bool value) { m_permanent = value; }
        bool IsAreaAura() const { return m_isAreaAura; }
        bool IsPeriodic() const { return m_isPeriodic; }
        bool IsTrigger() const { return m_isTrigger; }
        bool IsPassive() const { return m_isPassive; }
        bool IsPersistent() const { return m_isPersistent; }

        virtual void Update(uint32 diff);
        void ApplyModifier(bool Apply);

        void _AddAura();
        void _RemoveAura();
        uint32 CalculateDamage();

        void TriggerSpell();
        void SendCoolDownEvent();
        bool IsUpdated() { return m_updated; }
        void SetUpdated(bool val) { m_updated = val; }
        void SetRemoveOnDeath(bool rod) { m_removeOnDeath = rod; }
        void DelayPeriodicTimer(int32 delaytime);

        int32 m_procCharges;
        int32 m_absorbDmg;

    protected:
        Modifier m_modifier;
        SpellModifier *m_spellmod;
        uint32 m_spellId;
        uint32 m_effIndex;
        //SpellEntry *m_spellProto;
        uint64 m_caster_guid;
        Unit* m_target;
        int32 m_duration;
        int32 m_timeCla;
        Item* m_castItem;
        time_t m_applyTime;

        uint8 m_auraSlot;

        bool m_positive;
        bool m_permanent;
        bool m_isPeriodic;
        bool m_isTrigger;
        bool m_isAreaAura;
        bool m_isPassive;
        bool m_isPersistent;

        int32 m_periodicTimer;
        uint32 m_PeriodicEventId;
        bool m_updated;
        bool m_removeOnDeath;
};

class AreaAura : public Aura
{
    public:
        AreaAura(SpellEntry* spellproto, uint32 eff, Unit *target, Unit *caster = NULL, Item* castItem = NULL);
        ~AreaAura();
        void Update(uint32 diff);
};

class PersistentAreaAura : public Aura
{
    public:
        PersistentAreaAura(SpellEntry* spellproto, uint32 eff, Unit *target, Unit *caster = NULL, Item* castItem = NULL);
        ~PersistentAreaAura();
        void Update(uint32 diff);
};

typedef void(Aura::*pAuraHandler)(bool Apply);
#endif
