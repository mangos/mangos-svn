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

typedef void(Aura::*pAuraHandler)(bool Apply, bool Real);
// Real == true at aura add/remove
// Real == false at aura mod unapply/reapply, when this need to add/remove dependent aura/item/stat mods
//
// Code in aura handler can be guarded by if(Real) check in case if need it execution only at real add/remove of aura 
//
// MAIN RULE: Code DON'T MUST be guarded by if(Real) check if this aura mod required to unapply before add/remove dependent aura/item/stat mods
//      (percent auras, stats mods, etc)
// Second rule: Code must be guarded by if(Real) check if it modify object state (start/stop attack, send packets to client, etc)
//
// Other case choice: each code line moved under if(Real) check is mangos speedup,
//      each setting object update field code line moved under if(Real) check is significant mangos speedup, and less server->client data sends
//      each packet sending code moved under if(Real) check is _large_ mangos speedup, and lot less server->client data sends

class Aura
{
    public:
        //aura handlers
        void HandleNULL(bool Apply, bool Real);
        void HandleBindSight(bool Apply, bool Real);
        void HandleModPossess(bool Apply, bool Real);
        void HandlePeriodicDamage(bool Apply, bool Real);
        void HandleAuraDummy(bool Apply, bool Real);
        void HandleModConfuse(bool Apply, bool Real);
        void HandleModCharm(bool Apply, bool Real);
        void HandleModFear(bool Apply, bool Real);
        void HandlePeriodicHeal(bool Apply, bool Real);
        void HandleModAttackSpeed(bool Apply, bool Real);
        void HandleModThreat(bool Apply, bool Real);
        void HandleAuraWaterWalk(bool Apply, bool Real);
        void HandleAuraFeatherFall(bool Apply, bool Real);
        void HandleAuraHover(bool Apply, bool Real);
        void HandleAddModifier(bool Apply, bool Real);
        void HandleAuraModStun(bool Apply, bool Real);
        void HandleModDamageDone(bool Apply, bool Real);
        void HandleModDamageTaken(bool Apply, bool Real);
        void HandleAuraEmpathy(bool Apply, bool Real);
        void HandleAuraModRangedAttackPower(bool Apply, bool Real);
        void HandleAuraModIncreaseSpeedAlways(bool Apply, bool Real);
        void HandleAuraModIncreaseEnergyPercent(bool Apply, bool Real);
        void HandleAuraModIncreaseHealthPercent(bool Apply, bool Real);
        void HandleHaste(bool Apply, bool Real);
        void HandlePeriodicTriggerSpell(bool Apply, bool Real);
        void HandlePeriodicEnergize(bool Apply, bool Real);
        void HandleAuraModResistanceExclusive(bool Apply, bool Real);
        void HandleAuraSafeFall(bool Apply, bool Real);
        void HandleAuraDamageShield(bool Apply, bool Real);
        void HandleModStealth(bool Apply, bool Real);
        void HandleModDetect(bool Apply, bool Real);
        void HandleInvisibility(bool Apply, bool Real);
        void HandleInvisibilityDetect(bool Apply, bool Real);
        void HandleAuraModTotalHealthPercentRegen(bool Apply, bool Real);
        void HandleAuraModTotalManaPercentRegen(bool Apply, bool Real);
        void HandleAuraModResistance(bool Apply, bool Real);
        void HandleAuraModRoot(bool Apply, bool Real);
        void HandleAuraModSilence(bool Apply, bool Real);
        void HandleReflectSpells(bool Apply, bool Real);
        void HandleAuraModStat(bool Apply, bool Real);
        void HandleAuraModIncreaseSpeed(bool Apply, bool Real);
        void HandleAuraModIncreaseMountedSpeed(bool Apply, bool Real);
        void HandleAuraModDecreaseSpeed(bool Apply, bool Real);
        void HandleAuraModIncreaseHealth(bool Apply, bool Real);
        void HandleAuraModIncreaseEnergy(bool Apply, bool Real);
        void HandleAuraModShapeshift(bool Apply, bool Real);
        void HandleAuraModEffectImmunity(bool Apply, bool Real);
        void HandleAuraModStateImmunity(bool Apply, bool Real);
        void HandleAuraModSchoolImmunity(bool Apply, bool Real);
        void HandleAuraModDmgImmunity(bool Apply, bool Real);
        void HandleAuraModDispelImmunity(bool Apply, bool Real);
        void HandleAuraProcTriggerSpell(bool Apply, bool Real);
        void HandleAuraProcTriggerDamage(bool Apply, bool Real);
        void HandleAuraTracCreatures(bool Apply, bool Real);
        void HandleAuraTracResources(bool Apply, bool Real);
        void HandleAuraModParryPercent(bool Apply, bool Real);
        void HandleAuraModDodgePercent(bool Apply, bool Real);
        void HandleAuraModBlockPercent(bool Apply, bool Real);
        void HandleAuraModCritPercent(bool Apply, bool Real);
        void HandlePeriodicLeech(bool Apply, bool Real);
        void HandleModHitChance(bool Apply, bool Real);
        void HandleModSpellHitChance(bool Apply, bool Real);
        void HandleAuraModScale(bool Apply, bool Real);
        void HandlePeriodicManaLeech(bool Apply, bool Real);
        void HandleModCastingSpeed(bool Apply, bool Real);
        void HandleAuraMounted(bool Apply, bool Real);
        void HandleWaterBreathing(bool Apply, bool Real);
        void HandleModBaseResistance(bool Apply, bool Real);
        void HandleModRegen(bool Apply, bool Real);
        void HandleModPowerRegen(bool Apply, bool Real);
        void HandleChannelDeathItem(bool Apply, bool Real);
        void HandleModDamagePCTTaken(bool Apply, bool Real);
        void HandleModPCTRegen(bool Apply, bool Real);
        void HandlePeriodicDamagePCT(bool Apply, bool Real);
        void HandleAuraModAttackPower(bool Apply, bool Real);
        void HandleAuraTransform(bool Apply, bool Real);
        void HandleModSpellCritChance(bool Apply, bool Real);
        void HandleAuraModIncreaseSwimSpeed(bool Apply, bool Real);
        void HandleModDamageDoneCreature(bool Apply, bool Real);
        void HandleAuraManaShield(bool Apply, bool Real);
        void HandleAuraSchoolAbsorb(bool Apply, bool Real);
        void HandleModSpellCritChanceShool(bool Apply, bool Real);
        void HandleModPowerCost(bool Apply, bool Real);
        void HandleModPowerCostSchool(bool Apply, bool Real);
        void HandleReflectSpellsSchool(bool Apply, bool Real);
        void HandleFarSight(bool Apply, bool Real);
        void HandleModMechanicImmunity(bool Apply, bool Real);
        void HandleAuraModSkill(bool Apply, bool Real);
        void HandleModCreatureAttackPower(bool Apply, bool Real);
        void HandleModDamagePercentDone(bool Apply, bool Real);
        void HandleModPercentStat(bool Apply, bool Real);
        void HandleModResistancePercent(bool Apply, bool Real);
        void HandleAuraModBaseResistancePCT(bool Apply, bool Real);
        void HandleModShieldBlock(bool Apply, bool Real);
        void HandleModReputationGain(bool Apply, bool Real);
        void HandleForceReaction(bool Apply, bool Real);
        void HandleAuraModRangedHaste(bool Apply, bool Real);
        void HandleRangedAmmoHaste(bool Apply, bool Real);
        void HandleModTotalPercentStat(bool Apply, bool Real);

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
        void ApplyModifier(bool apply, bool Real = false);

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

#endif
