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
    TARGET_SINGLE_ENEMY                = 6,
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
    TARGET_CHAIN                       = 45,
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
struct TeleportCoords
{
    uint32 id;
    uint32 mapId;
    float x;
    float y;
    float z;
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
        Unit *getUnitTarget() { return m_unitTarget;};
        void setUnitTarget(Unit *target)
        {
            m_destX = target->GetPositionX();
            m_destY = target->GetPositionY();
            m_destZ = target->GetPositionZ();
            m_unitTarget = target;
            m_targetMask |= TARGET_FLAG_UNIT | TARGET_FLAG_DEST_LOCATION;
        }

        Item *m_itemTarget;
        GameObject *m_GOTarget;
        float m_srcX, m_srcY, m_srcZ;
        float m_destX, m_destY, m_destZ;
        std::string m_strTarget;

        uint16 m_targetMask;
    private:
        Unit *m_unitTarget;
};

enum SpellState
{
    SPELL_STATE_NULL      = 0,
    SPELL_STATE_PREPARING = 1,
    SPELL_STATE_CASTING   = 2,
    SPELL_STATE_FINISHED  = 3,
    SPELL_STATE_IDLE      = 4
};

enum ShapeshiftForm
{
    FORM_CAT              = 1,
    FORM_TREE             = 2,
    FORM_TRAVEL           = 3,
    FORM_AQUA             = 4,
    FORM_BEAR             = 5,
    FORM_AMBIENT          = 6,
    FORM_GHOUL            = 7,
    FORM_DIREBEAR         = 8,
    FORM_CREATUREBEAR     = 14,
    FORM_GHOSTWOLF        = 16,
    FORM_BATTLESTANCE     = 17,
    FORM_DEFENSIVESTANCE  = 18,
    FORM_BERSERKERSTANCE  = 19,
    FORM_SHADOW           = 28,
    FORM_STEALTH          = 30,
    FORM_MOONKIN          = 31
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

enum SpellFailedReason
{
    CAST_FAIL_IN_COMBAT = 0,
    CAST_FAIL_ALREADY_FULL_HEALTH = 1,
    CAST_FAIL_ALREADY_FULL_MANA = 2,
    //CAST_FAIL_ALREADY_FULL_RAGE = 2,
    CAST_FAIL_CREATURE_ALREADY_TAMING = 3,
    CAST_FAIL_ALREADY_HAVE_CHARMED = 4,
    CAST_FAIL_ALREADY_HAVE_SUMMON = 5,
    CAST_FAIL_ALREADY_OPEN = 6,
    CAST_FAIL_MORE_POWERFUL_SPELL_ACTIVE = 7,
    //CAST_FAIL_FAILED = 8,-> 29
    CAST_FAIL_NO_TARGET = 9,
    CAST_FAIL_INVALID_TARGET = 10,
    CAST_FAIL_CANT_BE_CHARMED = 11,
    CAST_FAIL_CANT_BE_DISENCHANTED = 12,                    //decompose
    // 13 SPELL_FAILED_CANT_BE_PROSPECTED
    CAST_FAIL_TARGET_IS_TAPPED = 13+1,
    CAST_FAIL_CANT_START_DUEL_INVISIBLE = 14+1,
    CAST_FAIL_CANT_START_DUEL_STEALTHED = 15+1,
    CAST_FAIL_TOO_CLOSE_TO_ENEMY = 16+1,
    CAST_FAIL_CANT_DO_THAT_YET = 17+1,
    CAST_FAIL_YOU_ARE_DEAD = 18+1,
    CAST_FAIL_CANT_DO_WHILE_XXXX =19,                       //NONE
    CAST_FAIL_CANT_DO_WHILE_CHARMED =19+1,                  //SPELL_FAILED_CHARMED
    CAST_FAIL_OBJECT_ALREADY_BEING_USED = 20+1,             //SPELL_FAILED_CHEST_IN_USE
    CAST_FAIL_CANT_DO_WHILE_CONFUSED = 21+1,                //SPELL_FAILED_CONFUSED
    //23 is gone.
    CAST_FAIL_MUST_HAVE_ITEM_EQUIPPED = 22 + 1 + 1,
    CAST_FAIL_MUST_HAVE_XXXX_EQUIPPED = 23 + 1 + 1,         //SPELL_FAILED_EQUIPPED_ITEM_CLASS
    CAST_FAIL_MUST_HAVE_XXXX_IN_MAINHAND = 24 + 1 + 1,      //SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND
    CAST_FAIL_MUST_HAVE_XXXX_IN_OFFHAND = 25 + 1 + 1,       //SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND
    CAST_FAIL_INTERNAL_ERROR = 26 + 1 + 1,
    CAST_FAIL_FAILED = 29,                                  // Doesn't exist anymore? Used Fizzle value atm.
    CAST_FAIL_FIZZLED = 27 + 1 + 1,                         // changed (+2) 12.1.1
    CAST_FAIL_YOU_ARE_FLEEING = 28 + 1 + 1,
    CAST_FAIL_FOOD_TOO_LOWLEVEL_FOR_PET = 29 + 1 + 1,
    CAST_FAIL_TARGET_IS_TOO_HIGH = 30 + 1 + 1,
    //32 + 1 is gone.
    CAST_FAIL_IMMUNE = 32 + 1 + 1,                          //SPELL_FAILED_IMMUNE
    CAST_FAIL_INTERRUPTED = 33 + 1 + 1,                     //SPELL_FAILED_INTERRUPTED
    CAST_FAIL_INTERRUPTED1 = 34 + 1 + 1,                    //SPELL_FAILED_INTERRUPTED_COMBAT
    CAST_FAIL_INTERRUPTED_COMBAT = 36,                      //just 36 SPELL_FAILED_INTERRUPTED_COMBAT
    CAST_FAIL_ITEM_ALREADY_ENCHANTED = 35 + 1 + 1,
    CAST_FAIL_ITEM_NOT_EXIST = 36 + 1 + 1,
    CAST_FAIL_ENCHANT_NOT_EXISTING_ITEM = 37 + 1 + 1,
    CAST_FAIL_ITEM_NOT_READY = 38 + 1 + 1,
    CAST_FAIL_YOU_ARE_NOT_HIGH_ENOUGH = 39 + 1 + 1,
    CAST_FAIL_NOT_IN_LINE_OF_SIGHT = 40 + 1 + 1,
    CAST_FAIL_TARGET_TOO_LOW = 41 + 1 + 1,
    CAST_FAIL_SKILL_NOT_HIGH_ENOUGH = 42 + 1 + 1,
    CAST_FAIL_WEAPON_HAND_IS_EMPTY = 43 + 1 + 1,
    CAST_FAIL_CANT_DO_WHILE_MOVING = 44 + 1 + 1,
    CAST_FAIL_NEED_AMMO_IN_PAPERDOLL_SLOT = 45 + 1 + 1,
    CAST_FAIL_REQUIRES_SOMETHING = 46 + 1 + 1,
    CAST_FAIL_NEED_EXOTIC_AMMO = 47 + 1 + 1,
    CAST_FAIL_NO_PATH_AVAILABLE = 48 + 1 + 1,
    CAST_FAIL_NOT_BEHIND_TARGET = 49 + 1 + 1,
    CAST_FAIL_DIDNT_LAND_IN_FISHABLE_WATER = 50 + 1 + 1,
    CAST_FAIL_CANT_BE_CAST_HERE = 51 + 1 + 1,
    CAST_FAIL_NOT_IN_FRONT_OF_TARGET = 52 + 1 + 1,
    CAST_FAIL_NOT_IN_CONTROL_OF_ACTIONS = 53 + 1 + 1,
    CAST_FAIL_SPELL_NOT_LEARNED = 54 + 1 + 1,
    CAST_FAIL_CANT_USE_WHEN_MOUNTED = 55 + 1 + 1,
    CAST_FAIL_YOU_ARE_IN_FLIGHT = 56 + 1 + 1,
    CAST_FAIL_YOU_ARE_ON_TRANSPORT = 57 + 1 + 1,
    CAST_FAIL_SPELL_NOT_READY_YET = 58 + 1 + 1,
    CAST_FAIL_CANT_DO_IN_SHAPESHIFT = 59 + 1 + 1,
    CAST_FAIL_HAVE_TO_BE_STANDING = 60 + 1 + 1,
    CAST_FAIL_CAN_USE_ONLY_ON_OWN_OBJECT = 61 + 1 + 1,      // rogues trying "enchant" other's weapon with poison
    //CAST_FAIL_ALREADY_OPEN1 = 62,

    CAST_FAIL_CANT_ENCHANT_TRADE_ITEM = 63 + 1,
    CAST_FAIL_HAVE_TO_BE_UNSHEATHED = 63 + 1 + 1,           // yellow text SPELL_FAILED_NOT_UNSHEATHED
    CAST_FAIL_CANT_CAST_AS_GHOST = 64 + 1 + 1,
    CAST_FAIL_NO_AMMO = 65 + 1 + 1,
    CAST_FAIL_NO_CHARGES_REMAIN = 66 + 1 + 1,
    CAST_FAIL_NOT_SELECT = 67 + 1 + 1,
    CAST_FAIL_COMBO_POINTS_REQUIRED = 68 + 1 + 1,
    CAST_FAIL_NO_DUELING_HERE = 69 + 1 + 1,
    CAST_FAIL_NOT_ENOUGH_ENDURANCE = 70 + 1 + 1,
    CAST_FAIL_THERE_ARENT_ANY_FISH_HERE = 71 + 1 + 1,
    CAST_FAIL_CANT_USE_WHILE_SHAPESHIFTED = 72 + 1 + 1,
    CAST_FAIL_CANT_MOUNT_HERE = 73 + 1 + 1,
    CAST_FAIL_YOU_DO_NOT_HAVE_PET = 74 + 1 + 1,
    CAST_FAIL_NOT_ENOUGH_MANA = 75 + 1 + 1,
    CAST_FAIL_NOT_AURA_TO_QUSHAN = 76 + 1 + 1,
    //= 79, CAST_FAIL_NOT_ITEM_TO_STEAL = 111 + 1 + 2   //(SPELL_FAILED_NOTHING_TO_STEAL)
    CAST_FAIL_CANT_USE_WHILE_SWIMMING = 77 + 1 + 2,
    CAST_FAIL_CAN_ONLY_USE_AT_DAY = 78 + 1 + 2,
    CAST_FAIL_CAN_ONLY_USE_INDOORS = 79 + 1 + 2,
    CAST_FAIL_CAN_ONLY_USE_MOUNTED = 80 + 1 + 2,
    CAST_FAIL_CAN_ONLY_USE_AT_NIGHT = 81 + 1 + 2,
    CAST_FAIL_CAN_ONLY_USE_OUTDOORS = 82 + 1 + 2,
    //CAST_FAIL_ONLY_SHAPESHIFTED = 83                  // didn't display
    // 86 none
    CAST_FAIL_CAN_ONLY_USE_STEALTHED  = 85 + 2,
    CAST_FAIL_CAN_ONLY_USE_WHILE_SWIMMING = 86 + 2,
    CAST_FAIL_OUT_OF_RANGE = 87 + 2,
    CAST_FAIL_CANT_USE_WHILE_PACIFIED = 87 + 1 + 2,
    CAST_FAIL_YOU_ARE_POSSESSED = 88 + 1 + 2,
    CAST_FAIL_YOU_NEED_TO_BE_IN_XXX = 90 + 1 + 2,
    CAST_FAIL_REQUIRES_XXX = 91 + 1 + 2,
    CAST_FAIL_UNABLE_TO_MOVE = 92 + 1 + 2,
    CAST_FAIL_SILENCED = 93 + 1 + 2,
    CAST_FAIL_ANOTHER_ACTION_IS_IN_PROGRESS = 94 + 1 + 2,
    CAST_FAIL_ALREADY_LEARNED_THAT_SPELL = 95 + 1 + 2,
    CAST_FAIL_SPELL_NOT_AVAILABLE_TO_YOU = 96 + 1 + 2,
    CAST_FAIL_CANT_DO_WHILE_STUNNED = 97 + 1 + 2,
    CAST_FAIL_YOUR_TARGET_IS_DEAD = 98 + 1 + 2,
    CAST_FAIL_TARGET_IS_IN_COMBAT = 99 + 1 + 2,
    CAST_FAIL_CANT_DO_THAT_YET_2 = 100 + 1 + 2,
    CAST_FAIL_TARGET_IS_DUELING = 101 + 1 + 2,
    CAST_FAIL_TARGET_IS_HOSTILE = 102 + 1 + 2,
    CAST_FAIL_TARGET_IS_TOO_ENRAGED_TO_CHARM = 103 + 1 + 2,
    CAST_FAIL_TARGET_IS_FRIENDLY = 104 + 1 + 2,
    CAST_FAIL_TARGET_CANT_BE_IN_COMBAT = 105 + 1 + 2,
    CAST_FAIL_CANT_TARGET_PLAYERS = 106 + 1 + 2,
    CAST_FAIL_TARGET_IS_ALIVE = 107 + 1 + 2,
    CAST_FAIL_TARGET_NOT_IN_YOUR_PARTY = 108 + 1 + 2,
    CAST_FAIL_CREATURE_MUST_BE_LOOTED_FIRST = 109 + 1 + 2,
    CAST_FAIL_AUCTION_HAVE_CANCEL = 110 + 1 + 2,
    CAST_FAIL_NOT_ITEM_TO_STEAL = 111 + 1 + 2,
    //CAST_FAIL_TARGET_IS_NOT_A_PLAYER = 107,
    //CAST_FAIL_NO_POCKETS_TO_PICK = 108,
    CAST_FAIL_TARGET_HAS_NO_WEAPONS_EQUIPPED = 112 + 1 + 2,
    CAST_FAIL_NOT_SKINNABLE = 113 + 1 + 2,
    CAST_FAIL_TOO_CLOSE = 115 + 1 + 2,
    CAST_FAIL_TOO_MANY_OF_THAT_ITEM_ALREADY = 116 + 1 + 2,
    CAST_FAIL_NOT_ENOUGH_TRAINING_POINTS = 118 + 1 + 2,
    CAST_FAIL_FAILED_ATTEMPT = 119 + 1 + 2,
    CAST_FAIL_TARGET_NEED_TO_BE_BEHIND = 120 + 1 + 2,
    CAST_FAIL_TARGET_NEED_TO_BE_INFRONT = 121 + 1 + 2,
    CAST_FAIL_PET_DOESNT_LIKE_THAT_FOOD = 122 + 1 + 2,
    CAST_FAIL_CANT_CAST_WHILE_FATIGUED = 123 + 1 + 2,
    CAST_FAIL_TARGET_MUST_BE_IN_THIS_INSTANCE = 124 + 1 + 2,
    CAST_FAIL_CANT_CAST_WHILE_TRADING = 125 + 1 + 2,
    CAST_FAIL_TARGET_IS_NOT_PARTY_OR_RAID = 126 + 1 + 2,
    CAST_FAIL_CANT_DISENCHANT_WHILE_LOOTING = 127 + 1 + 2,
    CAST_FAIL_TARGET_IS_IN_FFA_PVP_COMBAT = 133,
    //CAST_FAIL_TARGET_IS_NOT_A_GHOST = 128,    //SPELL_FAILED_TARGET_NOT_GHOST
    CAST_FAIL_NO_NEARBY_CORPSES_TO_EAT = 129 + 1 + 4,
    CAST_FAIL_CAN_ONLY_USE_IN_BATTLEGROUNDS = 130 + 1 + 4,
    CAST_FAIL_CANT_EQUIP_ON_LOW_RANK = 131 + 1 + 4,
    CAST_FAIL_YOUR_PET_CANT_LEARN_MORE_SKILLS = 132 + 1 + 4,
    CAST_FAIL_CANT_USE_NEW_ITEM = 133 + 1 + 4,
    CAST_FAIL_CANT_DO_IN_THIS_WEATHER = 134 + 1 + 4,
    CAST_FAIL_CANT_DO_IN_IMMUNE = 135 + 1 + 4,
    CAST_FAIL_CANT_DO_IN_XXX = 136 + 1 + 4,
    CAST_FAIL_GAME_TIME_OVER = 137 + 1 + 4,
    CAST_FAIL_NOT_ENOUGH_RANK = 138 + 1 + 4,
    CAST_FAIL_UNKNOWN_REASON = 139 + 1 + 4,
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
        void EffectAddComboPoints(uint32 i);
        void EffectDuel(uint32 i);
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
        uint32 CalculateDamage(uint8 i);
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

        void HandleEffects(Unit *pUnitTarget,Item *pItemTarget,GameObject *pGOTarget,uint32 i);
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
    protected:

        Unit* m_caster;
        bool m_autoRepeat;
        bool m_meleeSpell;
        bool m_rangedShoot;
        bool m_needAliveTarget[3];

        // Current targets, to be used in SpellEffects
        Unit* unitTarget;
        Item* itemTarget;
        GameObject* gameObjTarget;
        // -------------------------------------------
        GameObject* focusObject;

        uint32 damage;

        // List of all Spell targets
        std::list<Unit*> m_targetUnits[3];
        std::list<Item*> m_targetItems[3];
        std::list<GameObject*> m_targetGOs[3];
        // -------------------------------------------

        // List of all targets that arent repeated. (Unique)
        uint8 m_targetCount;
        std::list<Unit*> UniqueTargets;
        std::list<GameObject*> UniqueGOsTargets;
        // -------------------------------------------

        uint32 m_spellState;
        uint32 m_timer;
        int32 m_delayedTime;
        SpellEntry const* m_TriggerSpell;
        uint16 m_castFlags;

        float m_castPositionX;
        float m_castPositionY;
        float m_castPositionZ;
        float m_castOrientation;
        bool m_IsTriggeredSpell;
        Aura* m_triggeredByAura;
        //bool m_AreaAura;

        // List of all Objects to be Deleted in spell Finish
        //std::list<DynamicObject*> m_dynObjToDel;
        //std::list<GameObject*> m_ObjToDel;
        // -------------------------------------------
        uint8 up_skillvalue;
};

enum ReplenishType
{
    REPLENISH_UNDEFINED = 0,
    REPLENISH_HEALTH = 20,
    REPLENISH_MANA = 21,
    REPLENISH_RAGE = 22
};

namespace MaNGOS
{
    struct MANGOS_DLL_DECL SpellNotifierPlayer
    {
        std::list<Unit*> &i_data;
        Spell &i_spell;
        const uint32& i_index;
        SpellNotifierPlayer(Spell &spell, std::list<Unit*> &data, const uint32 &i) : i_data(data), i_spell(spell), i_index(i) {}
        inline void Visit(PlayerMapType &m)
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
    };

    struct MANGOS_DLL_DECL SpellNotifierCreatureAndPlayer
    {
        std::list<Unit*> &i_data;
        Spell &i_spell;
        const uint32& i_index;
        const uint32& i_push_type;
        SpellNotifierCreatureAndPlayer(Spell &spell, std::list<Unit*> &data, const uint32 &i,const uint32 &type)
            : i_data(data), i_spell(spell), i_index(i), i_push_type(type){}

        template<class T> inline void Visit(std::map<OBJECT_HANDLE, T *>  &m)
        {
            float radius = GetRadius(sSpellRadiusStore.LookupEntry(i_spell.m_spellInfo->EffectRadiusIndex[i_index]));
            for(typename std::map<OBJECT_HANDLE, T*>::iterator itr=m.begin(); itr != m.end(); ++itr)
            {
                if( !itr->second->isAlive() )
                    continue;

                if (i_spell.m_caster->IsFriendlyTo( itr->second ))
                    continue;

                switch(i_push_type)
                {
                    case PUSH_IN_FRONT:
                        if((i_spell.m_caster->isInFront((Unit*)(itr->second), radius )))
                            i_data.push_back(itr->second);
                        break;
                    case PUSH_SELF_CENTER:
                        if(i_spell.m_caster->IsWithinDist((Unit*)(itr->second), radius))
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
