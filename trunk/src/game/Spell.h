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
    TARGET_ALL_ENEMY_IN_AREA           = 15,
    TARGET_ALL_ENEMY_IN_AREA_INSTANT   = 16,
    TARGET_ALL_PARTY_AROUND_CASTER     = 20,
    TARGET_SINGLE_FRIEND               = 21,
    TARGET_ALL_ENEMIES_AROUND_CASTER   = 22,
    TARGET_GAMEOBJECT                  = 23,
    TARGET_IN_FRONT_OF_CASTER          = 24,
    TARGET_DUELVSPLAYER                = 25,
    TARGET_GAMEOBJECT_ITEM             = 26,
    TARGET_MASTER                      = 27,
    TARGET_ALL_ENEMY_IN_AREA_CHANNELED = 28,
    TARGET_MINION                      = 32,
    TARGET_SINGLE_PARTY                = 35,
    TARGET_AREAEFFECT_PARTY            = 37,
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

enum SpellChannelInterruptFlags
{
    CHANNEL_FLAG_DAMAGE      = 0x0002,
    CHANNEL_FLAG_TURNING     = 0x0010,
    CHANNEL_FLAG_DAMAGE2     = 0x0080,
    CHANNEL_FLAG_DELAY       = 0x4000
};

enum SpellAuraInterruptFlags
{
    AURA_INTERRUPT_FLAG_DAMAGE      = 0x00000002,
    AURA_INTERRUPT_FLAG_NOT_SEATED  = 0x00040000
};

enum SpellNotifyPushType
{
    PUSH_IN_FRONT   = 0,
    PUSH_SELF_CENTER  = 1,
    PUSH_DEST_CENTER  = 2
};

enum Rating
{
    SPELL_RATING_SKILL                      = 0x0000001, // 0
    SPELL_RATING_DEFENCE                    = 0x0000002, // 1
    SPELL_RATING_DODGE                      = 0x0000004, // 2
    SPELL_RATING_PARRY                      = 0x0000008, // 3
    SPELL_RATING_BLOCK                      = 0x0000010, // 4
    SPELL_RATING_MELEE_HIT                  = 0x0000020, // 5
    SPELL_RATING_RANGED_HIT                 = 0x0000040, // 6
    SPELL_RATING_SPELL_HIT                  = 0x0000080, // 7
    SPELL_RATING_MELEE_CRIT_HIT             = 0x0000100, // 8
    SPELL_RATING_RANGED_CRIT_HIT            = 0x0000200, // 9
    SPELL_RATING_SPELL_CRIT_HIT             = 0x0000400, // 10
    //more ratings here?
    SPELL_RATING_MELEE_HASTE                = 0x0020000, // 17
    SPELL_RATING_RANGED_HASTE               = 0x0040000, // 18
    SPELL_RATING_SPELL_HASTE                = 0x0080000, // 19
    SPELL_RATING_HIT                        = 0x0100000, // 20
    SPELL_RATING_CRIT_HIT                   = 0x0200000, // 21
    SPELL_RATING_HIT_AVOIDANCE              = 0x0400000, // 22
    SPELL_RATING_CRIT_AVOIDANCE             = 0x0800000, // 23
    SPELL_RATING_RESILIENCE                 = 0x1000000  // 24

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

        void read ( WorldPacket * data,Unit *caster );
        void write ( WorldPacket * data, bool forceAppend=false);

        SpellCastTargets& operator=(const SpellCastTargets &target)
        {
            m_unitTarget = target.m_unitTarget;
            m_itemTarget = target.m_itemTarget;
            m_GOTarget   = target.m_GOTarget;

            m_unitTargetGUID   = target.m_unitTargetGUID;
            m_GOTargetGUID     = target.m_GOTargetGUID;
            m_CorpseTargetGUID = target.m_CorpseTargetGUID;

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
        Unit *getUnitTarget() { return m_unitTarget; }
        void setUnitTarget(Unit *target);

        uint64 getGOTargetGUID() const { return m_GOTargetGUID; }
        GameObject *getGOTarget() const { return m_GOTarget; }
        void setGOTarget(GameObject *target);

        uint64 getCorpseTargetGUID() const { return m_CorpseTargetGUID; }

        bool IsEmpty() const { return m_GOTargetGUID==0 && m_unitTargetGUID==0 && m_itemTarget==0 && m_CorpseTargetGUID==0; }

        void Update(Unit* caster);

        Item *m_itemTarget;
        float m_srcX, m_srcY, m_srcZ;
        float m_destX, m_destY, m_destZ;
        std::string m_strTarget;

        uint16 m_targetMask;
    private:
        // objects (can be used at spell creating and after Update at casting
        Unit *m_unitTarget;
        GameObject *m_GOTarget;

        // object GUID, can be used always
        uint64 m_unitTargetGUID;
        uint64 m_GOTargetGUID;
        uint64 m_CorpseTargetGUID;
};

enum SpellState
{
    SPELL_STATE_NULL      = 0,
    SPELL_STATE_PREPARING = 1,
    SPELL_STATE_CASTING   = 2,
    SPELL_STATE_FINISHED  = 3,
    SPELL_STATE_IDLE      = 4
};

enum SpellModOp
{
    SPELLMOD_DAMAGE = 0,
    SPELLMOD_DURATION = 1,
    SPELLMOD_THREAT = 2,
    SPELLMOD_EFFECT1 = 3,
    SPELLMOD_EXTRA_BLOCKS = 4,
    SPELLMOD_RANGE = 5,
    SPELLMOD_RADIUS = 6,
    SPELLMOD_CRITICAL_CHANCE = 7,
    SPELLMOD_ALL_EFFECTS = 8,
    SPELLMOD_NOT_LOSE_CASTING_TIME = 9,
    SPELLMOD_CASTING_TIME = 10,
    SPELLMOD_COOLDOWN = 11,
    SPELLMOD_EFFECT2 = 12,
    // spellmod 13 unused
    SPELLMOD_COST = 14,
    SPELLMOD_CRIT_DAMAGE_BONUS = 15,
    SPELLMOD_RESIST_MISS_CHANCE = 16,
    SPELLMOD_JUMP_TARGETS = 17,
    SPELLMOD_CHANCE_OF_SUCCESS = 18,
    SPELLMOD_ACTIVATION_TIME = 19,
    SPELLMOD_EFFECT_PAST_FIRST = 20,
    SPELLMOD_CASTING_TIME_OLD = 21,
    SPELLMOD_DOT = 22,
    SPELLMOD_EFFECT3 = 23,
    SPELLMOD_SPELL_DAMAGE = 24,
    // spellmod 25, 26 unused
    SPELLMOD_MANA_LOST_PER_DAMAGE_TAKEN = 27,
    SPELLMOD_RESIST_DISPEL_CHANCE = 28
};

#define SPELLMOD_COUNT 32

enum SpellFailedReason
{
    SPELL_FAILED_AFFECTING_COMBAT  = 0,
    SPELL_FAILED_ALREADY_AT_FULL_HEALTH  = 1,
    SPELL_FAILED_ALREADY_AT_FULL_POWER   = 2,
    SPELL_FAILED_ALREADY_BEING_TAMED   = 3,
    SPELL_FAILED_ALREADY_HAVE_CHARM  = 4,
    SPELL_FAILED_ALREADY_HAVE_SUMMON   = 5,
    SPELL_FAILED_ALREADY_OPEN   = 6,
    SPELL_FAILED_AURA_BOUNCED   = 7,
    SPELL_FAILED_AUTOTRACK_INTERRUPTED   = 8,
    SPELL_FAILED_BAD_IMPLICIT_TARGETS   = 9,
    SPELL_FAILED_BAD_TARGETS   = 10,
    SPELL_FAILED_CANT_BE_CHARMED   = 11,
    SPELL_FAILED_CANT_BE_DISENCHANTED   = 12,
    SPELL_FAILED_CANT_BE_DISENCHANTED_SKILL  = 13,
    SPELL_FAILED_CANT_BE_PROSPECTED  = 14,
    SPELL_FAILED_CANT_CAST_ON_TAPPED   = 15,
    SPELL_FAILED_CANT_DUEL_WHILE_INVISIBLE   = 16,
    SPELL_FAILED_CANT_DUEL_WHILE_STEALTHED   = 17,
    SPELL_FAILED_CANT_STEALTH   = 18,
    SPELL_FAILED_CASTER_AURASTATE   = 19,
    SPELL_FAILED_CASTER_DEAD   = 20,
    SPELL_FAILED_CHARMED   = 21,
    SPELL_FAILED_CHEST_IN_USE   = 22,
    SPELL_FAILED_CONFUSED   = 23,
    SPELL_FAILED_DONT_REPORT   = 24,
    SPELL_FAILED_EQUIPPED_ITEM   = 25,
    SPELL_FAILED_EQUIPPED_ITEM_CLASS   = 26,
    SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND   = 27,
    SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND   = 28,
    SPELL_FAILED_ERROR   = 29,
    SPELL_FAILED_FIZZLE  = 30,
    SPELL_FAILED_FLEEING   = 31,
    SPELL_FAILED_FOOD_LOWLEVEL   = 32,
    SPELL_FAILED_HIGHLEVEL   = 33,
    SPELL_FAILED_HUNGER_SATIATED   = 34,
    SPELL_FAILED_IMMUNE  = 35,
    SPELL_FAILED_INTERRUPTED   = 36,
    SPELL_FAILED_INTERRUPTED_COMBAT  = 37,
    SPELL_FAILED_ITEM_ALREADY_ENCHANTED  = 38,
    SPELL_FAILED_ITEM_GONE   = 39,
    SPELL_FAILED_ITEM_NOT_FOUND  = 40,
    SPELL_FAILED_ITEM_NOT_READY  = 41,
    SPELL_FAILED_LEVEL_REQUIREMENT   = 42,
    SPELL_FAILED_LINE_OF_SIGHT   = 43,
    SPELL_FAILED_LOWLEVEL   = 44,
    SPELL_FAILED_LOW_CASTLEVEL   = 45,
    SPELL_FAILED_MAINHAND_EMPTY  = 46,
    SPELL_FAILED_MOVING  = 47,
    SPELL_FAILED_NEED_AMMO   = 48,
    SPELL_FAILED_NEED_AMMO_POUCH   = 49,
    SPELL_FAILED_NEED_EXOTIC_AMMO   = 50,
    SPELL_FAILED_NOPATH  = 51,
    SPELL_FAILED_NOT_BEHIND  = 52,
    SPELL_FAILED_NOT_FISHABLE   = 53,
    SPELL_FAILED_NOT_HERE   = 54,
    SPELL_FAILED_NOT_INFRONT   = 55,
    SPELL_FAILED_NOT_IN_CONTROL  = 56,
    SPELL_FAILED_NOT_KNOWN   = 57,
    SPELL_FAILED_NOT_MOUNTED   = 58,
    SPELL_FAILED_NOT_ON_TAXI   = 59,
    SPELL_FAILED_NOT_ON_TRANSPORT   = 60,
    SPELL_FAILED_NOT_READY   = 61,
    SPELL_FAILED_NOT_SHAPESHIFT  = 62,
    SPELL_FAILED_NOT_STANDING   = 63,
    SPELL_FAILED_NOT_TRADEABLE   = 64,
    SPELL_FAILED_NOT_TRADING   = 65,
    SPELL_FAILED_NOT_UNSHEATHED  = 66,
    SPELL_FAILED_NOT_WHILE_GHOST   = 67,
    SPELL_FAILED_NO_AMMO   = 68,
    SPELL_FAILED_NO_CHARGES_REMAIN   = 69,
    SPELL_FAILED_NO_CHAMPION   = 70,
    SPELL_FAILED_NO_COMBO_POINTS   = 71,
    SPELL_FAILED_NO_DUELING  = 72,
    SPELL_FAILED_NO_ENDURANCE   = 73,
    SPELL_FAILED_NO_FISH   = 74,
    SPELL_FAILED_NO_ITEMS_WHILE_SHAPESHIFTED   = 75,
    SPELL_FAILED_NO_MOUNTS_ALLOWED   = 76,
    SPELL_FAILED_NO_PET  = 77,
    SPELL_FAILED_NO_POWER   = 78,
    SPELL_FAILED_NOTHING_TO_DISPEL   = 79,
    SPELL_FAILED_NOTHING_TO_STEAL   = 80,
    SPELL_FAILED_ONLY_ABOVEWATER   = 81,
    SPELL_FAILED_ONLY_DAYTIME   = 82,
    SPELL_FAILED_ONLY_INDOORS   = 83,
    SPELL_FAILED_ONLY_MOUNTED   = 84,
    SPELL_FAILED_ONLY_NIGHTTIME  = 85,
    SPELL_FAILED_ONLY_OUTDOORS   = 86,
    SPELL_FAILED_ONLY_SHAPESHIFT   = 87,
    SPELL_FAILED_ONLY_STEALTHED  = 88,
    SPELL_FAILED_ONLY_UNDERWATER   = 89,
    SPELL_FAILED_OUT_OF_RANGE   = 90,
    SPELL_FAILED_PACIFIED   = 91,
    SPELL_FAILED_POSSESSED   = 92,
    SPELL_FAILED_REAGENTS   = 93,
    SPELL_FAILED_REQUIRES_AREA   = 94,
    SPELL_FAILED_REQUIRES_SPELL_FOCUS   = 95,
    SPELL_FAILED_ROOTED  = 96,
    SPELL_FAILED_SILENCED   = 97,
    SPELL_FAILED_SPELL_IN_PROGRESS   = 98,
    SPELL_FAILED_SPELL_LEARNED   = 99,
    SPELL_FAILED_SPELL_UNAVAILABLE   = 100,
    SPELL_FAILED_STUNNED   = 101,
    SPELL_FAILED_TARGETS_DEAD   = 102,
    SPELL_FAILED_TARGET_AFFECTING_COMBAT   = 103,
    SPELL_FAILED_TARGET_AURASTATE   = 104,
    SPELL_FAILED_TARGET_DUELING  = 105,
    SPELL_FAILED_TARGET_ENEMY   = 106,
    SPELL_FAILED_TARGET_ENRAGED  = 107,
    SPELL_FAILED_TARGET_FRIENDLY   = 108,
    SPELL_FAILED_TARGET_IN_COMBAT   = 109,
    SPELL_FAILED_TARGET_IS_PLAYER   = 110,
    SPELL_FAILED_TARGET_NOT_DEAD   = 111,
    SPELL_FAILED_TARGET_NOT_IN_PARTY   = 112,
    SPELL_FAILED_TARGET_NOT_LOOTED   = 113,
    SPELL_FAILED_TARGET_NOT_PLAYER   = 114,
    SPELL_FAILED_TARGET_NO_POCKETS   = 115,
    SPELL_FAILED_TARGET_NO_WEAPONS   = 116,
    SPELL_FAILED_TARGET_UNSKINNABLE  = 117,
    SPELL_FAILED_THIRST_SATIATED   = 118,
    SPELL_FAILED_TOO_CLOSE   = 119,
    SPELL_FAILED_TOO_MANY_OF_ITEM   = 120,
    SPELL_FAILED_TOTEM_CATEGORY  = 121,
    SPELL_FAILED_TOTEMS  = 122,
    SPELL_FAILED_TRAINING_POINTS   = 123,
    SPELL_FAILED_TRY_AGAIN   = 124,
    SPELL_FAILED_UNIT_NOT_BEHIND   = 125,
    SPELL_FAILED_UNIT_NOT_INFRONT   = 126,
    SPELL_FAILED_WRONG_PET_FOOD  = 127,
    SPELL_FAILED_NOT_WHILE_FATIGUED  = 128,
    SPELL_FAILED_TARGET_NOT_IN_INSTANCE  = 129,
    SPELL_FAILED_NOT_WHILE_TRADING   = 130,
    SPELL_FAILED_TARGET_NOT_IN_RAID  = 131,
    SPELL_FAILED_DISENCHANT_WHILE_LOOTING   = 132,
    SPELL_FAILED_PROSPECT_WHILE_LOOTING  = 133,
    SPELL_FAILED_PROSPECT_NEED_MORE  = 134,
    SPELL_FAILED_TARGET_FREEFORALL   = 135,
    SPELL_FAILED_NO_EDIBLE_CORPSES   = 136,
    SPELL_FAILED_ONLY_BATTLEGROUNDS  = 137,
    SPELL_FAILED_TARGET_NOT_GHOST   = 138,
    SPELL_FAILED_TOO_MANY_SKILLS   = 139,
    SPELL_FAILED_TRANSFORM_UNUSABLE  = 140,
    SPELL_FAILED_WRONG_WEATHER   = 141,
    SPELL_FAILED_DAMAGE_IMMUNE   = 142,
    SPELL_FAILED_PREVENTED_BY_MECHANIC   = 143,
    SPELL_FAILED_PLAY_TIME   = 144,
    SPELL_FAILED_REPUTATION  = 145,
    SPELL_FAILED_MIN_SKILL   = 146,
    SPELL_FAILED_NOT_IN_ARENA   = 147,
    SPELL_FAILED_NOT_ON_SHAPESHIFT   = 148,
    SPELL_FAILED_NOT_ON_STEALTHED   = 149,
    SPELL_FAILED_NOT_ON_DAMAGE_IMMUNE = 150,
    SPELL_FAILED_NOT_ON_MOUNTED = 151,
    SPELL_FAILED_TOO_SHALLOW = 152,
    SPELL_FAILED_TARGET_NOT_IN_SANCTUARY = 153,
    SPELL_FAILED_UNKNOWN = 154,
    SPELL_FAILED_NUMREASONS
};

#define SPELL_SPELL_CHANNEL_UPDATE_INTERVAL 1000

class Spell
{
    friend struct MaNGOS::SpellNotifierPlayer;
    friend struct MaNGOS::SpellNotifierCreatureAndPlayer;
    public:

        void EffectNULL(uint32 );
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
        void EffectSummonDeadPet(uint32 i);
        void EffectDestroyAllTotems(uint32 i);
        void EffectDurabilityDamage(uint32 i);
        void EffectSkill(uint32 i);
        void EffectAttackMe(uint32 i);
        void EffectDurabilityDamagePCT(uint32 i);
        void EffectReduceThreatPercent(uint32 i);
        void EffectResurrectNew(uint32 i);

        Spell( Unit* Caster, SpellEntry const *info, bool triggered, Aura* Aur = NULL, uint64 originalCasterGUID = 0 );
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
        Item* m_CastItem;
        SpellCastTargets m_targets;

        int32 casttime;
        bool IsAutoRepeat() const { return m_autoRepeat; }
        void SetAutoRepeat(bool rep) { m_autoRepeat = rep; }
        void ReSetTimer() { m_timer = casttime<0?0:casttime;}
        bool IsMeleeSpell() const { return m_meleeSpell; }
        bool IsChanneledSpell() const { return m_spellInfo->ChannelInterruptFlags != 0; }
        bool IsChannelActive() const { return m_caster->GetUInt32Value(UNIT_CHANNEL_SPELL) != 0; }
        bool IsMeleeAttackResetSpell() const { return !m_IsTriggeredSpell && (m_spellInfo->School != 0) && !(m_spellInfo->Attributes == 327680 && m_spellInfo->AttributesEx2 ==0); }

        Unit* GetOriginalCaster() { return m_originalCaster; }

        void UpdatePointers();                              // must be used at call Spell code after time delay (non triggered spell cast/update spell call/etc)
        bool IsAffectedBy(SpellEntry const *spellInfo, uint32 effectId);

        uint32 GetTargetCreatureTypeMask() const;
    protected:

        void SendLoot(uint64 guid, LootType loottype);

        Unit* m_caster;

        uint64 m_originalCasterGUID;                        // real source of cast (aura caster/etc), used for spell targets selection
                                                            // e.g. damage around area spell trigered by victim aura and da,age emeies of aura caster
        Unit* m_originalCaster;                             // cached pointer for m_originalCaster, updated at Spell::UpdatePointers()

        bool m_autoRepeat;
        bool m_meleeSpell;
        bool m_rangedShoot;
        bool m_needAliveTarget[3];

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
        Unit* i_originalCaster;

        SpellNotifierPlayer(Spell &spell, std::list<Unit*> &data, const uint32 &i) 
            : i_data(data), i_spell(spell), i_index(i)
        {
            i_originalCaster = i_spell.GetOriginalCaster();
        }

        void Visit(PlayerMapType &m)
        {
            float radius = GetRadius(sSpellRadiusStore.LookupEntry(i_spell.m_spellInfo->EffectRadiusIndex[i_index]));

            if(!i_originalCaster)
                return;

            for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
            {
                if( !itr->second->isAlive() )
                    continue;

                if( i_originalCaster->IsFriendlyTo(itr->second) )
                    continue;

                if( itr->second->GetDistanceSq(i_spell.m_targets.m_destX, i_spell.m_targets.m_destY, i_spell.m_targets.m_destZ) < radius * radius )
                    i_data.push_back(itr->second);
            }
        }
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, SKIP *> &) {}
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, CountedPtr<SKIP> > &) {}
    };

    struct MANGOS_DLL_DECL SpellNotifierCreatureAndPlayer
    {
        std::list<Unit*> *i_data;
        std::list<CountedPtr<Unit> > *i_dataptr;
        Spell &i_spell;
        const uint32& i_push_type;
        float i_radius;
        SpellTargets i_TargetType;
        Unit* i_originalCaster;

        SpellNotifierCreatureAndPlayer(Spell &spell, std::list<Unit*> &data, const uint32 &i, const uint32 &type,
            SpellTargets TargetType = SPELL_TARGETS_NOT_FRIENDLY)
            : i_data(&data), i_dataptr(NULL), i_spell(spell), i_push_type(type), i_TargetType(TargetType)
        {
            if (i_spell.m_spellInfo->EffectRadiusIndex[i])
                i_radius = GetRadius(sSpellRadiusStore.LookupEntry(i_spell.m_spellInfo->EffectRadiusIndex[i]));
            else
                i_radius = GetMaxRange(sSpellRangeStore.LookupEntry(i_spell.m_spellInfo->rangeIndex));
            i_originalCaster = spell.GetOriginalCaster();
        }

        SpellNotifierCreatureAndPlayer(Spell &spell, std::list<CountedPtr<Unit> > &data, const uint32 &i, const uint32 &type,
            SpellTargets TargetType = SPELL_TARGETS_NOT_FRIENDLY)
            : i_data(NULL), i_dataptr(&data), i_spell(spell), i_push_type(type), i_TargetType(TargetType)
        {
            if (i_spell.m_spellInfo->EffectRadiusIndex[i])
                i_radius = GetRadius(sSpellRadiusStore.LookupEntry(i_spell.m_spellInfo->EffectRadiusIndex[i]));
            else
                i_radius = GetMaxRange(sSpellRangeStore.LookupEntry(i_spell.m_spellInfo->rangeIndex));
            i_originalCaster = spell.GetOriginalCaster();
        }

        template<class T> inline void Visit(std::map<OBJECT_HANDLE, T *>  &m)
        {
            assert(i_data);

            if(!i_originalCaster)
                return;

            for(typename std::map<OBJECT_HANDLE, T*>::iterator itr=m.begin(); itr != m.end(); ++itr)
            {
                if( !itr->second->isAlive() )
                    continue;

                switch (i_TargetType)
                {
                    case SPELL_TARGETS_HOSTILE:
                        if (!i_originalCaster->IsHostileTo( itr->second ))
                            continue;
                        break;
                    case SPELL_TARGETS_NOT_FRIENDLY:
                        if (i_originalCaster->IsFriendlyTo( itr->second ))
                            continue;
                        break;
                    case SPELL_TARGETS_NOT_HOSTILE:
                        if (i_originalCaster->IsHostileTo( itr->second ))
                            continue;
                        break;
                    case SPELL_TARGETS_FRIENDLY:
                        if (!i_originalCaster->IsFriendlyTo( itr->second ))
                            continue;
                        break;
                    case SPELL_TARGETS_AOE_DAMAGE:
                        {
                            Unit* check = i_originalCaster;
                            Unit* owner = i_originalCaster->GetCharmerOrOwner();
                            if(owner)
                                check = owner;
                            if( check->GetTypeId()==TYPEID_PLAYER )
                            {
                                if (check->IsFriendlyTo( itr->second ))
                                    continue;
                            }
                            else
                            {
                                if (!check->IsHostileTo( itr->second ))
                                    continue;
                            }
                        }
                        break;
                    default: continue;
                }

                switch(i_push_type)
                {
                    case PUSH_IN_FRONT:
                        if((i_spell.m_caster->isInFront((Unit*)(itr->second), i_radius )))
                            i_data->push_back(itr->second);
                        break;
                    case PUSH_SELF_CENTER:
                        if(i_spell.m_caster->IsWithinDistInMap((Unit*)(itr->second), i_radius))
                            i_data->push_back(itr->second);
                        break;
                    case PUSH_DEST_CENTER:
                        if((itr->second->GetDistanceSq(i_spell.m_targets.m_destX, i_spell.m_targets.m_destY, i_spell.m_targets.m_destZ) < i_radius * i_radius ))
                            i_data->push_back(itr->second);
                        break;
                }
            }
        }

        template<class T> inline void Visit(std::map<OBJECT_HANDLE, CountedPtr<T> >  &m)
        {
            assert(i_dataptr);

            if(!i_originalCaster)
                return;

            for(typename std::map<OBJECT_HANDLE, CountedPtr<T> >::iterator itr=m.begin(); itr != m.end(); ++itr)
            {
                if( !itr->second->isAlive() )
                    continue;

                switch (i_TargetType)
                {
                    case SPELL_TARGETS_HOSTILE:
                        if (!i_originalCaster->IsHostileTo( itr->second ))
                            continue;
                        break;
                    case SPELL_TARGETS_NOT_FRIENDLY:
                        if (i_originalCaster->IsFriendlyTo( itr->second ))
                            continue;
                        break;
                    case SPELL_TARGETS_NOT_HOSTILE:
                        if (i_originalCaster->IsHostileTo( itr->second ))
                            continue;
                        break;
                    case SPELL_TARGETS_FRIENDLY:
                        if (!i_originalCaster->IsFriendlyTo( itr->second ))
                            continue;
                        break;
                    case SPELL_TARGETS_AOE_DAMAGE:
                        {
                            Unit* check = i_originalCaster;
                            Unit* owner = i_originalCaster->GetCharmerOrOwner();
                            if(owner)
                                check = owner;
                            if( check->GetTypeId()==TYPEID_PLAYER )
                            {
                                if (check->IsFriendlyTo( itr->second ))
                                    continue;
                            }
                            else
                            {
                                if (!check->IsHostileTo( itr->second ))
                                    continue;
                            }
                        }
                        break;
                    default: continue;
                }

                switch(i_push_type)
                {
                    case PUSH_IN_FRONT:
                        if((i_spell.m_caster->isInFront((Unit*)(&*itr->second), i_radius )))
                            i_dataptr->push_back(itr->second);
                        break;
                    case PUSH_SELF_CENTER:
                        if(i_spell.m_caster->IsWithinDistInMap((Unit*)(&*itr->second), i_radius))
                            i_dataptr->push_back(itr->second);
                        break;
                    case PUSH_DEST_CENTER:
                        if((itr->second->GetDistanceSq(i_spell.m_targets.m_destX, i_spell.m_targets.m_destY, i_spell.m_targets.m_destZ) < i_radius * i_radius ))
                            i_dataptr->push_back(itr->second);
                        break;
                }
            }
        }

        #ifdef WIN32
        template<> inline void Visit(CorpseMapType &m ) {}
        template<> inline void Visit(GameObjectMapType &m ) {}
        template<> inline void Visit(DynamicObjectMapType &m ) {}
        #endif
    };

    #ifndef WIN32
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(CorpseMapType &m ) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(GameObjectMapType &m ) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(DynamicObjectMapType &m ) {}
    #endif
}

typedef void(Spell::*pEffect)(uint32 i);
#endif
