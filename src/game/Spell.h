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
    TARGET_FLAG_OBJECT           = 0x0800,
    TARGET_FLAG_ITEM             = 0x1010,
    TARGET_FLAG_SOURCE_LOCATION  = 0x0020,
    TARGET_FLAG_DEST_LOCATION    = 0x0040,
    TARGET_FLAG_STRING           = 0x2000
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

enum SpellNotifyPushType
{
    PUSH_IN_FRONT   = 0,
    PUSH_SELF_CENTER  = 1,
    PUSH_DEST_CENTER  = 2
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

            m_unitTargetGUID = target.m_unitTargetGUID;
            m_GOTargetGUID   = target.m_GOTargetGUID;

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
    SPELLMOD_ATTACK_POWER_BONUS = 3,
    SPELLMOD_EXTRA_BLOCKS = 4,
    SPELLMOD_RANGE = 5,
    SPELLMOD_RADIUS = 6,
    SPELLMOD_CRITICAL_CHANCE = 7,
    SPELLMOD_ALL_EFFECTS = 8,
    SPELLMOD_NOT_LOSE_CASTING_TIME = 9,
    SPELLMOD_CASTING_TIME = 10,
    SPELLMOD_COOLDOWN = 11,
    SPELLMOD_MOVEMENT_SPEED = 12,
    SPELLMOD_COST = 14,
    SPELLMOD_CRIT_DAMAGE_BONUS = 15,
    SPELLMOD_RESIST_MISS_CHANCE = 16,
    SPELLMOD_JUMP_TARGETS = 17,
    SPELLMOD_CHANCE_OF_SUCCESS = 18,
    SPELLMOD_ACTIVATION_TIME = 19,
    SPELLMOD_EFFECT_PAST_FIRST = 20,
    SPELLMOD_CASTING_TIME_OLD = 21,
    SPELLMOD_DOT = 22,
    SPELLMOD_PENALTY1 = 23,
    SPELLMOD_PENALTY2 = 24,
    SPELLMOD_MANA_LOST_PER_DAMAGE_TAKEN = 27,
};

#define SPELLMOD_COUNT 32

enum SpellFailedReason
{
    CAST_FAIL_IN_COMBAT                       = 0,
    CAST_FAIL_ALREADY_FULL_HEALTH             = 1,
    CAST_FAIL_ALREADY_FULL_MANA               = 2,
    CAST_FAIL_CREATURE_ALREADY_TAMING         = 3,
    CAST_FAIL_ALREADY_HAVE_CHARMED            = 4,
    CAST_FAIL_ALREADY_HAVE_SUMMON             = 5,
    CAST_FAIL_ALREADY_OPEN                    = 6,
    CAST_FAIL_MORE_POWERFUL_SPELL_ACTIVE      = 7,
    CAST_FAIL_AUTOTRACK_INTERRUPTED           = 8,
    CAST_FAIL_NO_TARGET                       = 9,
    CAST_FAIL_INVALID_TARGET                  = 10,
    CAST_FAIL_CANT_BE_CHARMED                 = 11,
    CAST_FAIL_CANT_BE_DISENCHANTED            = 12,
    CAST_FAIL_CANT_BE_DISENCHANTED_SKILL      = 13,
    CAST_FAIL_CANT_BE_PROSPECTED              = 14, // There no gems in this
    CAST_FAIL_TARGET_IS_TAPPED                = 15,
    CAST_FAIL_CANT_START_DUEL_INVISIBLE       = 16,
    CAST_FAIL_CANT_START_DUEL_STEALTHED       = 17,
    CAST_FAIL_TOO_CLOSE_TO_ENEMY              = 18,
    CAST_FAIL_CANT_DO_THAT_YET                = 19,
    CAST_FAIL_YOU_ARE_DEAD                    = 20,
    CAST_FAIL_CANT_DO_WHILE_CHARMED           = 21,
    CAST_FAIL_OBJECT_ALREADY_BEING_USED       = 22,
    CAST_FAIL_CANT_DO_WHILE_CONFUSED          = 23,
    CAST_FAIL_DONT_REPORT                     = 24,
    CAST_FAIL_MUST_HAVE_ITEM_EQUIPPED         = 25,
    CAST_FAIL_MUST_HAVE_XXXX_EQUIPPED         = 26,
    CAST_FAIL_MUST_HAVE_XXXX_IN_MAINHAND      = 27,
    CAST_FAIL_MUST_HAVE_XXXX_IN_OFFHAND       = 28,
    CAST_FAIL_INTERNAL_ERROR                  = 29,
    CAST_FAIL_FIZZLED                         = 30,
    CAST_FAIL_YOU_ARE_FLEEING                 = 31,
    CAST_FAIL_FOOD_TOO_LOWLEVEL_FOR_PET       = 32,
    CAST_FAIL_TARGET_IS_TOO_HIGH              = 33,
    CAST_FAIL_HUNGER_SATIATED                 = 34,
    CAST_FAIL_IMMUNE                          = 35,
    CAST_FAIL_INTERRUPTED                     = 36,
    CAST_FAIL_INTERRUPTED_COMBAT              = 37,
    CAST_FAIL_ITEM_ALREADY_ENCHANTED          = 38,
    CAST_FAIL_ITEM_NOT_EXIST                  = 39,
    CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM       = 40,
    CAST_FAIL_ITEM_NOT_READY                  = 41,
    CAST_FAIL_YOU_ARE_NOT_HIGH_ENOUGH         = 42,
    CAST_FAIL_NOT_IN_LINE_OF_SIGHT            = 43,
    CAST_FAIL_TARGET_TOO_LOW                  = 44,
    CAST_FAIL_SKILL_NOT_HIGH_ENOUGH           = 45,
    CAST_FAIL_WEAPON_HAND_IS_EMPTY            = 46,
    CAST_FAIL_CANT_DO_WHILE_MOVING            = 47,
    CAST_FAIL_NEED_AMMO_IN_PAPERDOLL_SLOT     = 48,
    CAST_FAIL_REQUIRES_SOMETHING              = 49,
    CAST_FAIL_NEED_EXOTIC_AMMO                = 50,
    CAST_FAIL_NO_PATH_AVAILABLE               = 51,
    CAST_FAIL_NOT_BEHIND_TARGET               = 52,
    CAST_FAIL_DIDNT_LAND_IN_FISHABLE_WATER    = 53,
    CAST_FAIL_CANT_BE_CAST_HERE               = 54,
    CAST_FAIL_NOT_IN_FRONT_OF_TARGET          = 55,
    CAST_FAIL_NOT_IN_CONTROL_OF_ACTIONS       = 56,
    CAST_FAIL_SPELL_NOT_LEARNED               = 57,
    CAST_FAIL_CANT_USE_WHEN_MOUNTED           = 58,
    CAST_FAIL_YOU_ARE_IN_FLIGHT               = 59,
    CAST_FAIL_YOU_ARE_ON_TRANSPORT            = 60,
    CAST_FAIL_SPELL_NOT_READY_YET             = 61,
    CAST_FAIL_CANT_DO_IN_SHAPESHIFT           = 62,
    CAST_FAIL_HAVE_TO_BE_STANDING             = 63,
    CAST_FAIL_CAN_USE_ONLY_ON_OWN_OBJECT      = 64,
    CAST_FAIL_CANT_ENCHANT_TRADE_ITEM         = 65,
    CAST_FAIL_HAVE_TO_BE_UNSHEATHED           = 66,
    CAST_FAIL_CANT_CAST_AS_GHOST              = 67,
    CAST_FAIL_NO_AMMO                         = 68,
    CAST_FAIL_NO_CHARGES_REMAIN               = 69,
    CAST_FAIL_NOT_SELECT                      = 70,
    CAST_FAIL_COMBO_POINTS_REQUIRED           = 71,
    CAST_FAIL_NO_DUELING_HERE                 = 72,
    CAST_FAIL_NOT_ENOUGH_ENDURANCE            = 73,
    CAST_FAIL_THERE_ARENT_ANY_FISH_HERE       = 74,
    CAST_FAIL_CANT_USE_WHILE_SHAPESHIFTED     = 75,
    CAST_FAIL_CANT_MOUNT_HERE                 = 76,
    CAST_FAIL_YOU_DO_NOT_HAVE_PET             = 77,
    CAST_FAIL_NOT_ENOUGH_MANA                 = 78,
    CAST_FAIL_NOTHING_TO_DISPEL               = 79,
    CAST_FAIL_NOTHING_TO_STEAL                = 80,
    CAST_FAIL_CANT_USE_WHILE_SWIMMING         = 81,
    CAST_FAIL_CAN_ONLY_USE_AT_DAY             = 82,
    CAST_FAIL_CAN_ONLY_USE_INDOORS            = 83,
    CAST_FAIL_CAN_ONLY_USE_MOUNTED            = 84,
    CAST_FAIL_CAN_ONLY_USE_AT_NIGHT           = 85,
    CAST_FAIL_CAN_ONLY_USE_OUTDOORS           = 86,
    CAST_FAIL_CAN_ONLY_USE_SHAPESHIFT         = 87,
    CAST_FAIL_CAN_ONLY_USE_STEALTHED          = 88,
    CAST_FAIL_CAN_ONLY_USE_WHILE_SWIMMING     = 89,
    CAST_FAIL_OUT_OF_RANGE                    = 90,
    CAST_FAIL_CANT_USE_WHILE_PACIFIED         = 91,
    CAST_FAIL_YOU_ARE_POSSESSED               = 92,
    CAST_FAIL_REAGENTS                        = 93,
    CAST_FAIL_YOU_NEED_TO_BE_IN_XXX           = 94,
    CAST_FAIL_REQUIRES_XXX                    = 95,
    CAST_FAIL_UNABLE_TO_MOVE                  = 96,
    CAST_FAIL_SILENCED                        = 97,
    CAST_FAIL_ANOTHER_ACTION_IS_IN_PROGRESS   = 98,
    CAST_FAIL_ALREADY_LEARNED_THAT_SPELL      = 99,
    CAST_FAIL_SPELL_NOT_AVAILABLE_TO_YOU      = 100,
    CAST_FAIL_CANT_DO_WHILE_STUNNED           = 101,
    CAST_FAIL_YOUR_TARGET_IS_DEAD             = 102,
    CAST_FAIL_TARGET_IS_IN_COMBAT             = 103,
    CAST_FAIL_CANT_DO_THAT_YET_2              = 104, // SPELL_FAILED_TARGET_AURASTATE
    CAST_FAIL_TARGET_IS_DUELING               = 105,
    CAST_FAIL_TARGET_IS_HOSTILE               = 106,
    CAST_FAIL_TARGET_IS_TOO_ENRAGED_TO_CHARM  = 107,
    CAST_FAIL_TARGET_IS_FRIENDLY              = 108,
    CAST_FAIL_TARGET_CANT_BE_IN_COMBAT        = 109,
    CAST_FAIL_CANT_TARGET_PLAYERS             = 110,
    CAST_FAIL_TARGET_IS_ALIVE                 = 111,
    CAST_FAIL_TARGET_NOT_IN_YOUR_PARTY        = 112,
    CAST_FAIL_CREATURE_MUST_BE_LOOTED_FIRST   = 113,
    CAST_FAIL_TARGET_IS_NOT_PLAYER            = 114,
    CAST_FAIL_NOT_ITEM_TO_STEAL               = 115,
    CAST_FAIL_TARGET_HAS_NO_WEAPONS_EQUIPPED  = 116,
    CAST_FAIL_NOT_SKINNABLE                   = 117,
    CAST_FAIL_THIRST_SATIATED                 = 118,
    CAST_FAIL_TOO_CLOSE                       = 119,
    CAST_FAIL_TOO_MANY_OF_THAT_ITEM_ALREADY   = 120,
    CAST_FAIL_TOTEM_CATEGORY                  = 121,
    CAST_FAIL_TOTEMS                          = 122,
    CAST_FAIL_NOT_ENOUGH_TRAINING_POINTS      = 123,
    CAST_FAIL_FAILED_ATTEMPT                  = 124,
    CAST_FAIL_TARGET_NEED_TO_BE_BEHIND        = 125,
    CAST_FAIL_TARGET_NEED_TO_BE_INFRONT       = 126,
    CAST_FAIL_PET_DOESNT_LIKE_THAT_FOOD       = 127,
    CAST_FAIL_CANT_CAST_WHILE_FATIGUED        = 128,
    CAST_FAIL_TARGET_MUST_BE_IN_THIS_INSTANCE = 129,
    CAST_FAIL_CANT_CAST_WHILE_TRADING         = 130,
    CAST_FAIL_TARGET_IS_NOT_PARTY_OR_RAID     = 131,
    CAST_FAIL_CANT_DISENCHANT_WHILE_LOOTING   = 132,
    CAST_FAIL_CANT_PROSPECT_WHILE_LOOTING     = 133,
    CAST_FAIL_CANT_PROSPECT_NEED_MORE         = 134,
    CAST_FAIL_TARGET_IS_IN_FFA_PVP_COMBAT     = 135,
    CAST_FAIL_NO_NEARBY_CORPSES_TO_EAT        = 136,
    CAST_FAIL_CAN_ONLY_USE_IN_BATTLEGROUNDS   = 137,
    CAST_FAIL_TARGET_NOT_GHOST                = 138,
    CAST_FAIL_YOUR_PET_CANT_LEARN_MORE_SKILLS = 139,
    CAST_FAIL_TRANSFORM_UNUSABLE              = 140,
    CAST_FAIL_WRONG_WEATHER                   = 141,
    CAST_FAIL_DAMAGE_IMMUNE                   = 142,
    CAST_FAIL_PREVENTED_BY_MECHANIC           = 143,
    CAST_FAIL_PLAY_TIME                       = 144,
    CAST_FAIL_REPUTATION                      = 145,
    CAST_FAIL_MIN_SKILL                       = 146,
    CAST_FAIL_NOT_IN_ARENA                    = 147,
    CAST_FAIL_NOT_ON_SHAPESHIFT               = 148,
    CAST_FAIL_NOT_ON_STEALTHED                = 149,
    CAST_FAIL_NOT_ON_DAMAGE_IMMUNE            = 150,
    CAST_FAIL_NOT_ON_MOUNTED                  = 151,
    CAST_FAIL_TOO_SHALLOW                     = 152,
    CAST_FAIL_TARGET_NOT_IN_SANCTUARY         = 153,
    CAST_FAIL_UNKNOWN_REASON                  = 154,
    CAST_FAIL_NUMREASONS
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
        void EffectPowerDrain(uint32 i);
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
        void EffectSummonCritter(uint32 i);
        void EffectKnockBack(uint32 i);
        void EffectSummonDeadPet(uint32 i);
        void EffectSkill(uint32 i);
        void EffectAttackMe(uint32 i);
        void EffectResurrectNew(uint32 i);

        Spell( Unit* Caster, SpellEntry const *info, bool triggered, Aura* Aur );
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
        void SetTargetMap(uint32 i,uint32 cur,std::list<Unit*> &TagUnitMap,std::list<Item*> &TagItemMap,std::list<GameObject*> &TagGOMap);

        void SendCastResult(uint8 result);
        void SendSpellStart();
        void SendSpellGo();
        void SendSpellCooldown();
        void SendLogExecute();
        void SendInterrupted(uint8 result);
        void SendChannelUpdate(uint32 time);
        void SendChannelStart(uint32 duration);
        void SendResurrectRequest(Player* target);
        void SendHealSpellOnPlayer(Player* target, uint32 SpellID, uint32 Damage, bool CriticalHeal = false);
        void SendHealSpellOnPlayerPet(Player* target, uint32 SpellID, uint32 Damage, bool CriticalHeal = false);
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
    protected:

        Unit* m_caster;
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
        std::list<SpellEntry const*> m_TriggerSpell;

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
    SPELL_TARGETS_FRIENDLY
};

namespace MaNGOS
{
    struct MANGOS_DLL_DECL SpellNotifierPlayer
    {
        std::list<Unit*> &i_data;
        Spell &i_spell;
        const uint32& i_index;
        SpellNotifierPlayer(Spell &spell, std::list<Unit*> &data, const uint32 &i) : i_data(data), i_spell(spell), i_index(i) {}
        void Visit(PlayerMapType &m)
        {
            float radius = GetRadius(sSpellRadiusStore.LookupEntry(i_spell.m_spellInfo->EffectRadiusIndex[i_index]));

            for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
            {
                if( !itr->second->isAlive() )
                    continue;

                if( i_spell.m_caster->IsFriendlyTo(itr->second) )
                    continue;

                if( itr->second->GetDistanceSq(i_spell.m_targets.m_destX, i_spell.m_targets.m_destY, i_spell.m_targets.m_destZ) < radius * radius )
                    i_data.push_back(itr->second);
            }
        }
        template<class SKIP> void Visit(std::map<OBJECT_HANDLE, SKIP *> &) {}
    };

    struct MANGOS_DLL_DECL SpellNotifierCreatureAndPlayer
    {
        std::list<Unit*> &i_data;
        Spell &i_spell;
        const uint32& i_push_type;
        float radius;
        SpellTargets i_TargetType;

        SpellNotifierCreatureAndPlayer(Spell &spell, std::list<Unit*> &data, const uint32 &i, const uint32 &type,
            SpellTargets TargetType = SPELL_TARGETS_NOT_FRIENDLY)
            : i_data(data), i_spell(spell), i_push_type(type), i_TargetType(TargetType)
        {
            if (i_spell.m_spellInfo->EffectRadiusIndex[i])
                radius = GetRadius(sSpellRadiusStore.LookupEntry(i_spell.m_spellInfo->EffectRadiusIndex[i]));
            else
                radius = GetMaxRange(sSpellRangeStore.LookupEntry(i_spell.m_spellInfo->rangeIndex));
        }

        template<class T> inline void Visit(std::map<OBJECT_HANDLE, T *>  &m)
        {
            for(typename std::map<OBJECT_HANDLE, T*>::iterator itr=m.begin(); itr != m.end(); ++itr)
            {
                if( !itr->second->isAlive() )
                    continue;

                switch (i_TargetType)
                {
                    case SPELL_TARGETS_HOSTILE:
                        if (!i_spell.m_caster->IsHostileTo( itr->second ))
                            continue;
                        break;
                    case SPELL_TARGETS_NOT_FRIENDLY:
                        if (i_spell.m_caster->IsFriendlyTo( itr->second ))
                            continue;
                        break;
                    case SPELL_TARGETS_NOT_HOSTILE:
                        if (i_spell.m_caster->IsHostileTo( itr->second ))
                            continue;
                        break;
                    case SPELL_TARGETS_FRIENDLY:
                        if (!i_spell.m_caster->IsFriendlyTo( itr->second ))
                            continue;
                        break;
                    default: continue;
                }

                switch(i_push_type)
                {
                    case PUSH_IN_FRONT:
                        if((i_spell.m_caster->isInFront((Unit*)(itr->second), radius )))
                            i_data.push_back(itr->second);
                        break;
                    case PUSH_SELF_CENTER:
                        if(i_spell.m_caster->IsWithinDistInMap((Unit*)(itr->second), radius))
                            i_data.push_back(itr->second);
                        break;
                    case PUSH_DEST_CENTER:
                        if((itr->second->GetDistanceSq(i_spell.m_targets.m_destX, i_spell.m_targets.m_destY, i_spell.m_targets.m_destZ) < radius * radius ))
                            i_data.push_back(itr->second);
                        break;
                }
            }
        }

        #ifdef WIN32
        template<> inline void Visit(std::map<OBJECT_HANDLE, Corpse *> &m ) {}
        template<> inline void Visit(std::map<OBJECT_HANDLE, GameObject *> &m ) {}
        template<> inline void Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m ) {}
        #endif
    };

    #ifndef WIN32
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(std::map<OBJECT_HANDLE, Corpse *> &m ) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(std::map<OBJECT_HANDLE, GameObject *> &m ) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m ) {}
    #endif

}

typedef void(Spell::*pEffect)(uint32 i);
#endif
