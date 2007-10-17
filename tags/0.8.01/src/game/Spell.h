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

#ifndef __SPELL_H
#define __SPELL_H

#include "GridDefines.h"
#include "Database/DBCStores.h"

class WorldSession;
class Unit;
class DynamicObj;
class Player;
class Item;
class GameObject;
class Group;
class Aura;

enum SpellCastTargetFlags
{
    /*TARGET_FLAG_NONE             = 0x0000,
    TARGET_FLAG_SWIMMER          = 0x0002,
    TARGET_FLAG_ITEM             = 0x0010,
    TARGET_FLAG_SOURCE_AREA      = 0x0020,
    TARGET_FLAG_DEST_AREA        = 0x0040,
    TARGET_FLAG_UNKNOWN          = 0x0080,
    TARGET_FLAG_SELF             = 0x0100,
    TARGET_FLAG_PVP_CORPSE       = 0x0200,
    TARGET_FLAG_MASS_SPIRIT_HEAL = 0x0400,
    TARGET_FLAG_BEAST_CORPSE     = 0x0402,
    TARGET_FLAG_OBJECT           = 0x4000,
    TARGET_FLAG_RESURRECTABLE    = 0x8000*/

    TARGET_FLAG_SELF             = 0x0000,
    TARGET_FLAG_UNIT             = 0x0002,
    TARGET_FLAG_ITEM             = 0x0010,
    TARGET_FLAG_SOURCE_LOCATION  = 0x0020,
    TARGET_FLAG_DEST_LOCATION    = 0x0040,
    TARGET_FLAG_PVP_CORPSE       = 0x0200,
    TARGET_FLAG_OBJECT           = 0x0800,
    TARGET_FLAG_TRADE_ITEM       = 0x1000,
    TARGET_FLAG_STRING           = 0x2000,
    TARGET_FLAG_CORPSE           = 0x8000
};

enum Targets
{
    TARGET_SELF                        = 1,
    TARGET_PET                         = 5,
    TARGET_CHAIN_DAMAGE                = 6,
    TARGET_AREAEFFECT_CUSTOM           = 8,
    TARGET_ALL_ENEMY_IN_AREA           = 15,
    TARGET_ALL_ENEMY_IN_AREA_INSTANT   = 16,
    TARGET_ALL_PARTY_AROUND_CASTER     = 20,
    TARGET_SINGLE_FRIEND               = 21,
    TARGET_ALL_AROUND_CASTER           = 22,                // used only in TargetA, target selection dependent from TargetB
    TARGET_GAMEOBJECT                  = 23,
    TARGET_IN_FRONT_OF_CASTER          = 24,
    TARGET_DUELVSPLAYER                = 25,
    TARGET_GAMEOBJECT_ITEM             = 26,
    TARGET_MASTER                      = 27,
    TARGET_ALL_ENEMY_IN_AREA_CHANNELED = 28,
    TARGET_MINION                      = 32,
    TARGET_ALL_PARTY                   = 33,
    TARGET_SINGLE_PARTY                = 35,
    TARGET_AREAEFFECT_PARTY            = 37,
    TARGET_SCRIPT                      = 38,
    TARGET_SELF_FISHING                = 39,
    TARGET_TOTEM_EARTH                 = 41,
    TARGET_TOTEM_WATER                 = 42,
    TARGET_TOTEM_AIR                   = 43,
    TARGET_TOTEM_FIRE                  = 44,
    TARGET_CHAIN_HEAL                  = 45,
    TARGET_DYNAMIC_OBJECT              = 47,
    TARGET_CURRENT_SELECTED_ENEMY      = 53,
    TARGET_SINGLE_FRIEND_2             = 57,
    TARGET_AREAEFFECT_PARTY_AND_CLASS  = 61,
    TARGET_SINGLE_ENEMY                = 77,
};

enum SpellCastFlags
{
    CAST_FLAG_UNKNOWN1           = 0x2,
    CAST_FLAG_UNKNOWN2           = 0x10,
    CAST_FLAG_AMMO               = 0x20,
    CAST_FLAG_UNKNOWN3           = 0x100
};

enum SpellDmgClass
{
    SPELL_DAMAGE_CLASS_NONE     = 0,
    SPELL_DAMAGE_CLASS_MAGIC    = 1,
    SPELL_DAMAGE_CLASS_MELEE    = 2,
    SPELL_DAMAGE_CLASS_RANGED   = 3
};

enum SpellNotifyPushType
{
    PUSH_IN_FRONT   = 0,
    PUSH_SELF_CENTER  = 1,
    PUSH_DEST_CENTER  = 2
};

enum Rating
{
    SPELL_RATING_SKILL                      = 0x0000001,    // 0
    SPELL_RATING_DEFENCE                    = 0x0000002,    // 1
    SPELL_RATING_DODGE                      = 0x0000004,    // 2
    SPELL_RATING_PARRY                      = 0x0000008,    // 3
    SPELL_RATING_BLOCK                      = 0x0000010,    // 4
    SPELL_RATING_MELEE_HIT                  = 0x0000020,    // 5
    SPELL_RATING_RANGED_HIT                 = 0x0000040,    // 6
    SPELL_RATING_SPELL_HIT                  = 0x0000080,    // 7
    SPELL_RATING_MELEE_CRIT_HIT             = 0x0000100,    // 8
    SPELL_RATING_RANGED_CRIT_HIT            = 0x0000200,    // 9
    SPELL_RATING_SPELL_CRIT_HIT             = 0x0000400,    // 10
    //more ratings here?
    SPELL_RATING_MELEE_HASTE                = 0x0020000,    // 17
    SPELL_RATING_RANGED_HASTE               = 0x0040000,    // 18
    SPELL_RATING_SPELL_HASTE                = 0x0080000,    // 19
    SPELL_RATING_HIT                        = 0x0100000,    // 20
    SPELL_RATING_CRIT_HIT                   = 0x0200000,    // 21
    SPELL_RATING_HIT_AVOIDANCE              = 0x0400000,    // 22
    SPELL_RATING_CRIT_AVOIDANCE             = 0x0800000,    // 23
    SPELL_RATING_RESILIENCE                 = 0x1000000     // 24

};

bool IsQuestTameSpell(uint32 spellId);

namespace MaNGOS
{
    struct SpellNotifierPlayer;
    struct SpellNotifierCreatureAndPlayer;
}

class SpellCastTargets
{

    public:
        SpellCastTargets();
        ~SpellCastTargets();

        bool read ( WorldPacket * data,Unit *caster );
        void write ( WorldPacket * data, bool forceAppend=false);

        SpellCastTargets& operator=(const SpellCastTargets &target)
        {
            m_unitTarget = target.m_unitTarget;
            m_itemTarget = target.m_itemTarget;
            m_GOTarget   = target.m_GOTarget;

            m_unitTargetGUID   = target.m_unitTargetGUID;
            m_GOTargetGUID     = target.m_GOTargetGUID;
            m_CorpseTargetGUID = target.m_CorpseTargetGUID;
            m_itemTargetGUID   = target.m_itemTargetGUID;

            m_srcX = target.m_srcX;
            m_srcY = target.m_srcY;
            m_srcZ = target.m_srcZ;

            m_destX = target.m_destX;
            m_destY = target.m_destY;
            m_destZ = target.m_destZ;

            m_strTarget = target.m_strTarget;

            m_targetMask = target.m_targetMask;

            return *this;
        }

        uint64 getUnitTargetGUID() const { return m_unitTargetGUID; }
        Unit *getUnitTarget() const { return m_unitTarget; }
        void setUnitTarget(Unit *target);

        uint64 getGOTargetGUID() const { return m_GOTargetGUID; }
        GameObject *getGOTarget() const { return m_GOTarget; }
        void setGOTarget(GameObject *target);

        uint64 getCorpseTargetGUID() const { return m_CorpseTargetGUID; }
        uint64 getItemTargetGUID() const { return m_itemTargetGUID; }
        Item* getItemTarget() const { return m_itemTarget; }
        void setItemTarget(Item* item);
        void updateTradeSlotItem()
        {
            if(m_itemTarget && (m_targetMask & TARGET_FLAG_TRADE_ITEM))
                m_itemTargetGUID = m_itemTarget->GetGUID();
        }

        bool IsEmpty() const { return m_GOTargetGUID==0 && m_unitTargetGUID==0 && m_itemTarget==0 && m_CorpseTargetGUID==0; }

        void Update(Unit* caster);

        float m_srcX, m_srcY, m_srcZ;
        float m_destX, m_destY, m_destZ;
        std::string m_strTarget;

        uint16 m_targetMask;
    private:
        // objects (can be used at spell creating and after Update at casting
        Unit *m_unitTarget;
        GameObject *m_GOTarget;
        Item *m_itemTarget;

        // object GUID, can be used always
        uint64 m_unitTargetGUID;
        uint64 m_GOTargetGUID;
        uint64 m_CorpseTargetGUID;
        uint64 m_itemTargetGUID;
};

enum SpellState
{
    SPELL_STATE_NULL      = 0,
    SPELL_STATE_PREPARING = 1,
    SPELL_STATE_CASTING   = 2,
    SPELL_STATE_FINISHED  = 3,
    SPELL_STATE_IDLE      = 4,
    SPELL_STATE_DELAYED   = 5
};

enum SpellFailedReason
{
    SPELL_FAILED_AFFECTING_COMBAT             = 0x00,
    SPELL_FAILED_ALREADY_AT_FULL_HEALTH       = 0x01,
    SPELL_FAILED_ALREADY_AT_FULL_POWER        = 0x02,
    SPELL_FAILED_ALREADY_BEING_TAMED          = 0x03,
    SPELL_FAILED_ALREADY_HAVE_CHARM           = 0x04,
    SPELL_FAILED_ALREADY_HAVE_SUMMON          = 0x05,
    SPELL_FAILED_ALREADY_OPEN                 = 0x06,
    SPELL_FAILED_AURA_BOUNCED                 = 0x07,
    SPELL_FAILED_AUTOTRACK_INTERRUPTED        = 0x08,
    SPELL_FAILED_BAD_IMPLICIT_TARGETS         = 0x09,
    SPELL_FAILED_BAD_TARGETS                  = 0x0A,
    SPELL_FAILED_CANT_BE_CHARMED              = 0x0B,
    SPELL_FAILED_CANT_BE_DISENCHANTED         = 0x0C,
    SPELL_FAILED_CANT_BE_DISENCHANTED_SKILL   = 0x0D,
    SPELL_FAILED_CANT_BE_PROSPECTED           = 0x0E,
    SPELL_FAILED_CANT_CAST_ON_TAPPED          = 0x0F,
    SPELL_FAILED_CANT_DUEL_WHILE_INVISIBLE    = 0x10,
    SPELL_FAILED_CANT_DUEL_WHILE_STEALTHED    = 0x11,
    SPELL_FAILED_CANT_STEALTH                 = 0x12,
    SPELL_FAILED_CASTER_AURASTATE             = 0x13,
    SPELL_FAILED_CASTER_DEAD                  = 0x14,
    SPELL_FAILED_CHARMED                      = 0x15,
    SPELL_FAILED_CHEST_IN_USE                 = 0x16,
    SPELL_FAILED_CONFUSED                     = 0x17,
    SPELL_FAILED_DONT_REPORT                  = 0x18,
    SPELL_FAILED_EQUIPPED_ITEM                = 0x19,
    SPELL_FAILED_EQUIPPED_ITEM_CLASS          = 0x1A,
    SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND = 0x1B,
    SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND  = 0x1C,
    SPELL_FAILED_ERROR                        = 0x1D,
    SPELL_FAILED_FIZZLE                       = 0x1E,
    SPELL_FAILED_FLEEING                      = 0x1F,
    SPELL_FAILED_FOOD_LOWLEVEL                = 0x20,
    SPELL_FAILED_HIGHLEVEL                    = 0x21,
    SPELL_FAILED_HUNGER_SATIATED              = 0x22,
    SPELL_FAILED_IMMUNE                       = 0x23,
    SPELL_FAILED_INTERRUPTED                  = 0x24,
    SPELL_FAILED_INTERRUPTED_COMBAT           = 0x25,
    SPELL_FAILED_ITEM_ALREADY_ENCHANTED       = 0x26,
    SPELL_FAILED_ITEM_GONE                    = 0x27,
    SPELL_FAILED_ITEM_NOT_FOUND               = 0x28,
    SPELL_FAILED_ITEM_NOT_READY               = 0x29,
    SPELL_FAILED_LEVEL_REQUIREMENT            = 0x2A,
    SPELL_FAILED_LINE_OF_SIGHT                = 0x2B,
    SPELL_FAILED_LOWLEVEL                     = 0x2C,
    SPELL_FAILED_LOW_CASTLEVEL                = 0x2D,
    SPELL_FAILED_MAINHAND_EMPTY               = 0x2E,
    SPELL_FAILED_MOVING                       = 0x2F,
    SPELL_FAILED_NEED_AMMO                    = 0x30,
    SPELL_FAILED_NEED_AMMO_POUCH              = 0x31,
    SPELL_FAILED_NEED_EXOTIC_AMMO             = 0x32,
    SPELL_FAILED_NOPATH                       = 0x33,
    SPELL_FAILED_NOT_BEHIND                   = 0x34,
    SPELL_FAILED_NOT_FISHABLE                 = 0x35,
    SPELL_FAILED_NOT_FLYING                   = 0x36,
    SPELL_FAILED_NOT_HERE                     = 0x37,
    SPELL_FAILED_NOT_INFRONT                  = 0x38,
    SPELL_FAILED_NOT_IN_CONTROL               = 0x39,
    SPELL_FAILED_NOT_KNOWN                    = 0x3A,
    SPELL_FAILED_NOT_MOUNTED                  = 0x3B,
    SPELL_FAILED_NOT_ON_TAXI                  = 0x3C,
    SPELL_FAILED_NOT_ON_TRANSPORT             = 0x3D,
    SPELL_FAILED_NOT_READY                    = 0x3E,
    SPELL_FAILED_NOT_SHAPESHIFT               = 0x3F,
    SPELL_FAILED_NOT_STANDING                 = 0x40,
    SPELL_FAILED_NOT_TRADEABLE                = 0x41,
    SPELL_FAILED_NOT_TRADING                  = 0x42,
    SPELL_FAILED_NOT_UNSHEATHED               = 0x43,
    SPELL_FAILED_NOT_WHILE_GHOST              = 0x44,
    SPELL_FAILED_NO_AMMO                      = 0x45,
    SPELL_FAILED_NO_CHARGES_REMAIN            = 0x46,
    SPELL_FAILED_NO_CHAMPION                  = 0x47,
    SPELL_FAILED_NO_COMBO_POINTS              = 0x48,
    SPELL_FAILED_NO_DUELING                   = 0x49,
    SPELL_FAILED_NO_ENDURANCE                 = 0x4A,
    SPELL_FAILED_NO_FISH                      = 0x4B,
    SPELL_FAILED_NO_ITEMS_WHILE_SHAPESHIFTED  = 0x4C,
    SPELL_FAILED_NO_MOUNTS_ALLOWED            = 0x4D,
    SPELL_FAILED_NO_PET                       = 0x4E,
    SPELL_FAILED_NO_POWER                     = 0x4F,
    SPELL_FAILED_NOTHING_TO_DISPEL            = 0x50,
    SPELL_FAILED_NOTHING_TO_STEAL             = 0x51,
    SPELL_FAILED_ONLY_ABOVEWATER              = 0x52,
    SPELL_FAILED_ONLY_DAYTIME                 = 0x53,
    SPELL_FAILED_ONLY_INDOORS                 = 0x54,
    SPELL_FAILED_ONLY_MOUNTED                 = 0x55,
    SPELL_FAILED_ONLY_NIGHTTIME               = 0x56,
    SPELL_FAILED_ONLY_OUTDOORS                = 0x57,
    SPELL_FAILED_ONLY_SHAPESHIFT              = 0x58,
    SPELL_FAILED_ONLY_STEALTHED               = 0x59,
    SPELL_FAILED_ONLY_UNDERWATER              = 0x5A,
    SPELL_FAILED_OUT_OF_RANGE                 = 0x5B,
    SPELL_FAILED_PACIFIED                     = 0x5C,
    SPELL_FAILED_POSSESSED                    = 0x5D,
    SPELL_FAILED_REAGENTS                     = 0x5E,
    SPELL_FAILED_REQUIRES_AREA                = 0x5F,
    SPELL_FAILED_REQUIRES_SPELL_FOCUS         = 0x60,
    SPELL_FAILED_ROOTED                       = 0x61,
    SPELL_FAILED_SILENCED                     = 0x62,
    SPELL_FAILED_SPELL_IN_PROGRESS            = 0x63,
    SPELL_FAILED_SPELL_LEARNED                = 0x64,
    SPELL_FAILED_SPELL_UNAVAILABLE            = 0x65,
    SPELL_FAILED_STUNNED                      = 0x66,
    SPELL_FAILED_TARGETS_DEAD                 = 0x67,
    SPELL_FAILED_TARGET_AFFECTING_COMBAT      = 0x68,
    SPELL_FAILED_TARGET_AURASTATE             = 0x69,
    SPELL_FAILED_TARGET_DUELING               = 0x6A,
    SPELL_FAILED_TARGET_ENEMY                 = 0x6B,
    SPELL_FAILED_TARGET_ENRAGED               = 0x6C,
    SPELL_FAILED_TARGET_FRIENDLY              = 0x6D,
    SPELL_FAILED_TARGET_IN_COMBAT             = 0x6E,
    SPELL_FAILED_TARGET_IS_PLAYER             = 0x6F,
    SPELL_FAILED_TARGET_IS_PLAYER_CONTROLLED  = 0x70,
    SPELL_FAILED_TARGET_NOT_DEAD              = 0x71,
    SPELL_FAILED_TARGET_NOT_IN_PARTY          = 0x72,
    SPELL_FAILED_TARGET_NOT_LOOTED            = 0x73,
    SPELL_FAILED_TARGET_NOT_PLAYER            = 0x74,
    SPELL_FAILED_TARGET_NO_POCKETS            = 0x75,
    SPELL_FAILED_TARGET_NO_WEAPONS            = 0x76,
    SPELL_FAILED_TARGET_UNSKINNABLE           = 0x77,
    SPELL_FAILED_THIRST_SATIATED              = 0x78,
    SPELL_FAILED_TOO_CLOSE                    = 0x79,
    SPELL_FAILED_TOO_MANY_OF_ITEM             = 0x7A,
    SPELL_FAILED_TOTEM_CATEGORY               = 0x7B,
    SPELL_FAILED_TOTEMS                       = 0x7C,
    SPELL_FAILED_TRAINING_POINTS              = 0x7D,
    SPELL_FAILED_TRY_AGAIN                    = 0x7E,
    SPELL_FAILED_UNIT_NOT_BEHIND              = 0x7F,
    SPELL_FAILED_UNIT_NOT_INFRONT             = 0x80,
    SPELL_FAILED_WRONG_PET_FOOD               = 0x81,
    SPELL_FAILED_NOT_WHILE_FATIGUED           = 0x82,
    SPELL_FAILED_TARGET_NOT_IN_INSTANCE       = 0x83,
    SPELL_FAILED_NOT_WHILE_TRADING            = 0x84,
    SPELL_FAILED_TARGET_NOT_IN_RAID           = 0x85,
    SPELL_FAILED_DISENCHANT_WHILE_LOOTING     = 0x86,
    SPELL_FAILED_PROSPECT_WHILE_LOOTING       = 0x87,
    SPELL_FAILED_PROSPECT_NEED_MORE           = 0x88,
    SPELL_FAILED_TARGET_FREEFORALL            = 0x89,
    SPELL_FAILED_NO_EDIBLE_CORPSES            = 0x8A,
    SPELL_FAILED_ONLY_BATTLEGROUNDS           = 0x8B,
    SPELL_FAILED_TARGET_NOT_GHOST             = 0x8C,
    SPELL_FAILED_TOO_MANY_SKILLS              = 0x8D,
    SPELL_FAILED_TRANSFORM_UNUSABLE           = 0x8E,
    SPELL_FAILED_WRONG_WEATHER                = 0x8F,
    SPELL_FAILED_DAMAGE_IMMUNE                = 0x90,
    SPELL_FAILED_PREVENTED_BY_MECHANIC        = 0x91,
    SPELL_FAILED_PLAY_TIME                    = 0x92,
    SPELL_FAILED_REPUTATION                   = 0x93,
    SPELL_FAILED_MIN_SKILL                    = 0x94,
    SPELL_FAILED_NOT_IN_ARENA                 = 0x95,
    SPELL_FAILED_NOT_ON_SHAPESHIFT            = 0x96,
    SPELL_FAILED_NOT_ON_STEALTHED             = 0x97,
    SPELL_FAILED_NOT_ON_DAMAGE_IMMUNE         = 0x98,
    SPELL_FAILED_NOT_ON_MOUNTED               = 0x99,
    SPELL_FAILED_TOO_SHALLOW                  = 0x9A,
    SPELL_FAILED_TARGET_NOT_IN_SANCTUARY      = 0x9B,
    SPELL_FAILED_UNKNOWN                      = 0x9C
};

#define SPELL_SPELL_CHANNEL_UPDATE_INTERVAL 1000

typedef std::multimap<uint64, uint64> SpellTargetTimeMap;

class Spell
{
    friend struct MaNGOS::SpellNotifierPlayer;
    friend struct MaNGOS::SpellNotifierCreatureAndPlayer;
    public:

        void EffectNULL(uint32 );
        void EffectDistract(uint32 i);
        void EffectPull(uint32 i);
        void EffectSchoolDMG(uint32 i);
        void EffectInstaKill(uint32 i);
        void EffectDummy(uint32 i);
        void EffectTeleportUnits(uint32 i);
        void EffectApplyAura(uint32 i);
        void EffectSendEvent(uint32 i);
        void EffectPowerBurn(uint32 i);
        void EffectManaDrain(uint32 i);
        void EffectHeal(uint32 i);
        void EffectHealthLeach(uint32 i);
        void EffectQuestComplete(uint32 i);
        void EffectCreateItem(uint32 i);
        void EffectPersistentAA(uint32 i);
        void EffectEnergize(uint32 i);
        void EffectOpenLock(uint32 i);
        void EffectSummonChangeItem(uint32 i);
        void EffectOpenSecretSafe(uint32 i);
        void EffectProficiency(uint32 i);
        void EffectApplyAA(uint32 i);
        void EffectSummon(uint32 i);
        void EffectLearnSpell(uint32 i);
        void EffectDispel(uint32 i);
        void EffectDualWield(uint32 i);
        void EffectPickPocket(uint32 i);
        void EffectAddFarsight(uint32 i);
        void EffectSummonWild(uint32 i);
        void EffectSummonGuardian(uint32 i);
        void EffectTeleUnitsFaceCaster(uint32 i);
        void EffectLearnSkill(uint32 i);
        void EffectTradeSkill(uint32 i);
        void EffectEnchantItemPerm(uint32 i);
        void EffectEnchantItemTmp(uint32 i);
        void EffectTameCreature(uint32 i);
        void EffectSummonPet(uint32 i);
        void EffectLearnPetSpell(uint32 i);
        void EffectWeaponDmg(uint32 i);
        void EffectTriggerSpell(uint32 i);
        void EffectThreat(uint32 i);
        void EffectHealMaxHealth(uint32 i);
        void EffectInterruptCast(uint32 i);
        void EffectSummonObjectWild(uint32 i);
        void EffectScriptEffect(uint32 i);
        void EffectSanctuary(uint32 i);
        void EffectAddComboPoints(uint32 i);
        void EffectDuel(uint32 i);
        void EffectStuck(uint32 i);
        void EffectSummonPlayer(uint32 i);
        void EffectSummonTotem(uint32 i);
        void EffectEnchantHeldItem(uint32 i);
        void EffectSummonObject(uint32 i);
        void EffectResurrect(uint32 i);
        void EffectParry(uint32 i);
        void EffectMomentMove(uint32 i);                    //by vendy
        void EffectTransmitted(uint32 i);
        void EffectDisEnchant(uint32 i);
        void EffectInebriate(uint32 i);
        void EffectFeedPet(uint32 i);
        void EffectDismissPet(uint32 i);
        void EffectReputation(uint32 i);
        void EffectSelfResurrect(uint32 i);
        void EffectSkinning(uint32 i);
        void EffectCharge(uint32 i);
        void EffectProspecting(uint32 i);
        void EffectSummonCritter(uint32 i);
        void EffectKnockBack(uint32 i);
        void EffectPlayerPull(uint32 i);
        void EffectDispelMechanic(uint32 i);
        void EffectSummonDeadPet(uint32 i);
        void EffectDestroyAllTotems(uint32 i);
        void EffectDurabilityDamage(uint32 i);
        void EffectSkill(uint32 i);
        void EffectAttackMe(uint32 i);
        void EffectDurabilityDamagePCT(uint32 i);
        void EffectReduceThreatPercent(uint32 i);
        void EffectResurrectNew(uint32 i);
        void EffectAddExtraAttacks(uint32 i);
        void EffectSpiritHeal(uint32 i);
        void EffectSkinPlayerCorpse(uint32 i);
        void EffectApplyPetAura(uint32 i);
        void EffectSummonDemon(uint32 i);

        Spell( Unit* Caster, SpellEntry const *info, bool triggered, Aura* Aur = NULL, uint64 originalCasterGUID = 0, Spell** triggeringContainer = NULL );
        ~Spell();

        void prepare(SpellCastTargets * targets);
        void cancel();
        void update(uint32 difftime);
        void cast(bool skipCheck = false);
        void finish(bool ok = true);
        void TakePower(uint32 mana);
        void TakeReagents();
        void TakeCastItem();
        void TriggerSpell();
        uint8 CanCast();
        int16 PetCanCast(Unit* target);
        bool CanAutoCast(Unit* target);

        // handlers
        void handle_immediate();
        uint64 handle_delayed(uint64 t_offset);
        // handler helpers
        void _handle_immediate_phase();
        void _handle_unit_phase(const uint64 targetGUID, const uint32 effectNumber, std::set<uint64>* reflectTargets);
        void _handle_go_phase(const uint64 targetGUID, const uint32 effectNumber);
        void _handle_reflection_phase(std::set<uint64>* reflectTargets);
        void _handle_finish_phase();

        uint8 CheckItems();
        uint8 CheckRange();
        uint8 CheckMana(uint32 *mana);
        int32 CalculateDamage(uint8 i);
        void HandleTeleport(uint32 id, Unit* Target);
        void Delayed(int32 delaytime);
        void DelayedChannel(int32 delaytime);
        void reflect(Unit *refunit);
        inline uint32 getState() const { return m_spellState; }
        void setState(uint32 state) { m_spellState = state; }

        void DoCreateItem(uint32 i, uint32 itemtype);

        void writeSpellGoTargets( WorldPacket * data );
        void writeAmmoToPacket( WorldPacket * data );
        void FillTargetMap();

        void SetTargetMap(uint32 i,uint32 cur,std::list<Unit*> &TagUnitMap);

        Unit* SelectMagnetTarget();
        bool CheckTarget( Unit* target, uint32 eff, bool hitPhase );

        void SendCastResult(uint8 result);
        void SendSpellStart();
        void SendSpellGo();
        void SendSpellCooldown();
        void SendLogExecute();
        void SendInterrupted(uint8 result);
        void SendChannelUpdate(uint32 time);
        void SendChannelStart(uint32 duration);
        void SendResurrectRequest(Player* target);
        void SendPlaySpellVisual(uint32 SpellID);

        void HandleEffects(Unit *pUnitTarget,Item *pItemTarget,GameObject *pGOTarget,uint32 i, float DamageMultiplier = 1.0);
        void HandleThreatSpells(uint32 spellId);
        //void HandleAddAura(Unit* Target);

        SpellEntry const* m_spellInfo;
        int32 m_currentBasePoints[3];                       // cache SpellEntry::EffectBasePoints and use for set custom base points
        Item* m_CastItem;
        SpellCastTargets m_targets;

        int32 casttime;
        bool IsAutoRepeat() const { return m_autoRepeat; }
        void SetAutoRepeat(bool rep) { m_autoRepeat = rep; }
        void ReSetTimer() { m_timer = casttime<0?0:casttime; }
        bool IsMeleeSpell() const { return m_meleeSpell; }
        bool IsChanneledSpell() const { return m_spellInfo->ChannelInterruptFlags != 0; }
        bool IsChannelActive() const { return m_caster->GetUInt32Value(UNIT_CHANNEL_SPELL) != 0; }
        bool IsMeleeAttackResetSpell() const { return !m_IsTriggeredSpell && (m_spellInfo->School != 0) && !(m_spellInfo->Attributes == 327680 && m_spellInfo->AttributesEx2 ==0); }

        bool IsDeletable() const { return m_deletable; }
        void SetDeletable(bool deletable) { m_deletable = deletable; }
        uint64 GetDelayStart() const { return m_delayStart; }
        void SetDelayStart(uint64 m_time) { m_delayStart = m_time; }
        uint64 GetDelayMoment() const { return m_delayMoment; }

        CurrentSpellTypes GetCurrentContainer();

        Unit* GetCaster() { return m_caster; }
        Unit* GetOriginalCaster() { return m_originalCaster; }

        void UpdatePointers();                              // must be used at call Spell code after time delay (non triggered spell cast/update spell call/etc)

        bool IsAffectedBy(SpellEntry const *spellInfo, uint32 effectId);

        bool CheckTargetCreatureType(Unit* target) const;

        void AddTriggeredSpell(SpellEntry const* spell) { m_TriggerSpells.push_back(spell); }
    protected:

        void SendLoot(uint64 guid, LootType loottype);
        Unit* m_caster;

        uint64 m_originalCasterGUID;                        // real source of cast (aura caster/etc), used for spell targets selection
                                                            // e.g. damage around area spell trigered by victim aura and da,age emeies of aura caster
        Unit* m_originalCaster;                             // cached pointer for m_originalCaster, updated at Spell::UpdatePointers()

        Spell** m_selfContainer;                            // pointer to our spell container (if applicable)
        Spell** m_triggeringContainer;                      // pointer to container with spell that has triggered us

        bool m_autoRepeat;
        bool m_meleeSpell;
        bool m_rangedShoot;
        bool m_needAliveTarget[3];

        // Delayed spells system
        uint64 m_delayStart;                                // time of spell delay start, filled by event handler, zero = just started
        uint64 m_delayMoment;                               // moment of next delay call, used internally
        SpellTargetTimeMap m_unitsHitList[3];               // time of units to hit by effect (used by delayed spells only)
        SpellTargetTimeMap m_objectsHitList[3];             // time of GOs to hit by effect (used by delayed spells only)
        bool m_immediateHandled;                            // were immediate actions handled? (used by delayed spells only)

        // These vars are used in both delayed spell system and modified immediate spell system
        bool m_deletable;                                   // is the spell pending deletion or must be updated till permitted to delete?
        bool m_needSpellLog;                                // need to send spell log?
        bool m_canReflect;                                  // can reflect this spell?
        bool m_applyMultiplier[3];                          // by effect: damage multiplier needed?
        float m_damageMultipliers[3];                       // by effect: damage multiplier

        // Current targets, to be used in SpellEffects (MUST BE USED ONLY IN SPELL EFFECTS)
        Unit* unitTarget;
        Item* itemTarget;
        GameObject* gameObjTarget;
        int32 damage;

        // -------------------------------------------
        GameObject* focusObject;

        // List of all Spell targets
        std::list<uint64> m_targetUnitGUIDs[3];
        std::list<Item*> m_targetItems[3];
        std::list<uint64> m_targetGameobjectGUIDs[3];
        // -------------------------------------------

        //List For Triggered Spells
        typedef std::list<SpellEntry const*> TriggerSpells;
        TriggerSpells m_TriggerSpells;

        uint32 m_spellState;
        uint32 m_timer;
        uint16 m_castFlags;

        float m_castPositionX;
        float m_castPositionY;
        float m_castPositionZ;
        float m_castOrientation;
        bool m_IsTriggeredSpell;
        Aura* m_triggeredByAura;
};

enum ReplenishType
{
    REPLENISH_UNDEFINED = 0,
    REPLENISH_HEALTH    = 20,
    REPLENISH_MANA      = 21,
    REPLENISH_RAGE      = 22
};

enum SpellTargets
{
    SPELL_TARGETS_HOSTILE,
    SPELL_TARGETS_NOT_FRIENDLY,
    SPELL_TARGETS_NOT_HOSTILE,
    SPELL_TARGETS_FRIENDLY,
    SPELL_TARGETS_AOE_DAMAGE
};

namespace MaNGOS
{
    struct MANGOS_DLL_DECL SpellNotifierPlayer
    {
        std::list<Unit*> &i_data;
        Spell &i_spell;
        const uint32& i_index;
        float i_radius;
        Unit* i_originalCaster;

        SpellNotifierPlayer(Spell &spell, std::list<Unit*> &data, const uint32 &i, float radius)
            : i_data(data), i_spell(spell), i_index(i), i_radius(radius)
        {
            i_originalCaster = i_spell.GetOriginalCaster();
        }

        void Visit(PlayerMapType &m)
        {
            if(!i_originalCaster)
                return;

            for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
            {
                Player * pPlayer = itr->getSource();
                if( !pPlayer->isAlive() )
                    continue;

                if( i_originalCaster->IsFriendlyTo(pPlayer) )
                    continue;

                if( pPlayer->GetDistance(i_spell.m_targets.m_destX, i_spell.m_targets.m_destY, i_spell.m_targets.m_destZ) < i_radius )
                    i_data.push_back(pPlayer);
            }
        }
        template<class SKIP> void Visit(GridRefManager<SKIP> &) {}
    };

    struct MANGOS_DLL_DECL SpellNotifierCreatureAndPlayer
    {
        std::list<Unit*> *i_data;
        Spell &i_spell;
        const uint32& i_push_type;
        float i_radius;
        SpellTargets i_TargetType;
        Unit* i_originalCaster;

        SpellNotifierCreatureAndPlayer(Spell &spell, std::list<Unit*> &data, float radius, const uint32 &type,
            SpellTargets TargetType = SPELL_TARGETS_NOT_FRIENDLY)
            : i_data(&data), i_spell(spell), i_push_type(type), i_radius(radius), i_TargetType(TargetType)
        {
            i_originalCaster = spell.GetOriginalCaster();
        }

        template<class T> inline void Visit(GridRefManager<T>  &m)
        {
            assert(i_data);

            if(!i_originalCaster)
                return;

            for(typename GridRefManager<T>::iterator itr = m.begin(); itr != m.end(); ++itr)
            {
                if( !itr->getSource()->isAlive() )
                    continue;

                switch (i_TargetType)
                {
                    case SPELL_TARGETS_HOSTILE:
                        if (!i_originalCaster->IsHostileTo( itr->getSource() ))
                            continue;
                        break;
                    case SPELL_TARGETS_NOT_FRIENDLY:
                        if (i_originalCaster->IsFriendlyTo( itr->getSource() ))
                            continue;
                        break;
                    case SPELL_TARGETS_NOT_HOSTILE:
                        if (i_originalCaster->IsHostileTo( itr->getSource() ))
                            continue;
                        break;
                    case SPELL_TARGETS_FRIENDLY:
                        if (!i_originalCaster->IsFriendlyTo( itr->getSource() ))
                            continue;
                        break;
                    case SPELL_TARGETS_AOE_DAMAGE:
                    {
                        if(itr->getSource()->GetTypeId()==TYPEID_UNIT && ((Creature*)itr->getSource())->isTotem())
                            continue;

                        Unit* check = i_originalCaster;
                        Unit* owner = i_originalCaster->GetCharmerOrOwner();
                        if(owner)
                            check = owner;

                        if( check->GetTypeId()==TYPEID_PLAYER )
                        {
                            if (check->IsFriendlyTo( itr->getSource() ))
                                continue;
                        }
                        else
                        {
                            if (!check->IsHostileTo( itr->getSource() ))
                                continue;
                        }
                    }
                    break;
                    default: continue;
                }

                switch(i_push_type)
                {
                    case PUSH_IN_FRONT:
                        if((i_spell.m_caster->isInFront((Unit*)(itr->getSource()), i_radius )))
                            i_data->push_back(itr->getSource());
                        break;
                    case PUSH_SELF_CENTER:
                        if(i_spell.m_caster->IsWithinDistInMap((Unit*)(itr->getSource()), i_radius))
                            i_data->push_back(itr->getSource());
                        break;
                    case PUSH_DEST_CENTER:
                        if((itr->getSource()->GetDistance(i_spell.m_targets.m_destX, i_spell.m_targets.m_destY, i_spell.m_targets.m_destZ) < i_radius ))
                            i_data->push_back(itr->getSource());
                        break;
                }
            }
        }

        #ifdef WIN32
        template<> inline void Visit(CorpseMapType & ) {}
        template<> inline void Visit(GameObjectMapType & ) {}
        template<> inline void Visit(DynamicObjectMapType & ) {}
        #endif
    };

    #ifndef WIN32
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(CorpseMapType& ) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(GameObjectMapType& ) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(DynamicObjectMapType& ) {}
    #endif
}

typedef void(Spell::*pEffect)(uint32 i);

class SpellEvent : public BasicEvent
{
    public:
        SpellEvent(Spell* spell);
        virtual ~SpellEvent();

        virtual bool Execute(uint64 e_time, uint32 p_time);
        virtual void Abort(uint64 e_time);
    protected:
        Spell* m_Spell;
};
#endif
