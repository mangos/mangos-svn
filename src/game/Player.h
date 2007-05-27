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

#ifndef _PLAYER_H
#define _PLAYER_H

#include "Common.h"
#include "ItemPrototype.h"
#include "Unit.h"
#include "Item.h"

#include "Database/DatabaseEnv.h"
#include "NPCHandler.h"
#include "QuestDef.h"
#include "Group.h"
#include "Bag.h"
#include "WorldSession.h"
#include "Pet.h"

struct Mail;
class Channel;
class DynamicObject;
class Creature;
class Pet;
class PlayerMenu;
class Transport;
class UpdateMask;

typedef std::deque<Mail*> PlayerMails;

enum SpellModType
{
    SPELLMOD_FLAT         = 107,
    SPELLMOD_PCT          = 108
};

enum PlayerSpellState
{
    PLAYERSPELL_UNCHANGED = 0,
    PLAYERSPELL_CHANGED   = 1,
    PLAYERSPELL_NEW       = 2,
    PLAYERSPELL_REMOVED   = 3
};

struct PlayerSpell
{
    uint16 slotId;
    uint8 active;
    PlayerSpellState state;
};

struct SpellModifier
{
    uint8 op;
    uint8 type;
    int32 value;
    uint32 mask;
    int16 charges;
    uint32 spellId;
    uint32 effectId;
};

typedef HM_NAMESPACE::hash_map<uint16, PlayerSpell*> PlayerSpellMap;
typedef std::list<SpellModifier*> SpellModList;

struct SpellCooldown
{
    time_t end;
    uint16 itemid;
};

typedef std::map<uint32, SpellCooldown> SpellCooldowns;

enum TrainerSpellState
{
    TRAINER_SPELL_GREEN = 0,
    TRAINER_SPELL_RED   = 1,
    TRAINER_SPELL_GRAY  = 2
};

enum ActionButtonUpdateState
{
    ACTIONBUTTON_UNCHANGED = 0,
    ACTIONBUTTON_CHANGED   = 1,
    ACTIONBUTTON_NEW       = 2,
    ACTIONBUTTON_DELETED   = 3
};

struct ActionButton
{
    ActionButton() : action(0), type(0), misc(0), uState( ACTIONBUTTON_NEW ) {}
    ActionButton(uint16 _action, uint8 _type, uint8 _misc) : action(_action), type(_type), misc(_misc), uState( ACTIONBUTTON_NEW ) {}

    uint16 action;
    uint8 type;
    uint8 misc;
    ActionButtonUpdateState uState;
};

enum ActionButtonType
{
    ACTION_BUTTON_SPELL = 0,
    ACTION_BUTTON_MACRO = 64,
    ACTION_BUTTON_ITEM  = 128
};

typedef std::map<uint8,ActionButton> ActionButtonList;

typedef std::pair<uint16, bool> CreateSpellPair;

struct PlayerCreateInfoItem
{
    PlayerCreateInfoItem(uint32 id, uint32 amount) : item_id(id), item_amount(amount) {}

    uint32 item_id;
    uint32 item_amount;
};

typedef std::list<PlayerCreateInfoItem> PlayerCreateInfoItems;

struct PlayerLevelInfo
{
    PlayerLevelInfo() : health(0), mana(0) { for(int i=0; i < MAX_STATS; ++i ) stats[i] = 0; }

    uint8 stats[MAX_STATS];
    uint16 health;
    uint16 mana;
};

struct PlayerInfo
{
    PlayerInfo() : displayId_m(0),displayId_f(0),levelInfo(NULL)             // existence checked by displayId != 0             // existence checked by displayId != 0
    {
    }

    uint32 mapId;
    uint32 zoneId;
    float positionX;
    float positionY;
    float positionZ;
    uint16 displayId_m;
    uint16 displayId_f;
    PlayerCreateInfoItems item;
    std::list<CreateSpellPair> spell;
    std::list<uint16> skill;
    std::list<uint16> action[4];

    PlayerLevelInfo* levelInfo;                             //[level-1] 0..MaxPlayerLevel-1
};

struct PvPInfo
{
    PvPInfo() : inHostileArea(false), endTimer(0) {}

    bool inHostileArea;
    time_t endTimer;
};

struct GroupInfo
{
    Group *group;
    Group *invite;
};

struct DuelInfo
{
    DuelInfo() : initiator(NULL), opponent(NULL), startTimer(0), startTime(0), outOfBound(0) {}

    Player *initiator;
    Player *opponent;
    time_t startTimer;
    time_t startTime;
    time_t outOfBound;
};

struct Areas
{
    uint32 areaID;
    uint32 areaFlag;
    float x1;
    float x2;
    float y1;
    float y2;
};

enum FactionState
{
    FACTION_UNCHANGED = 0,
    FACTION_CHANGED   = 1,
    FACTION_NEW       = 2
};

enum FactionFlags
{
    FACTION_FLAG_VISIBLE    = 0x01,
    FACTION_FLAG_AT_WAR     = 0x02,
    FACTION_FLAG_UNKNOWN    = 0x04,
    FACTION_FLAG_INVISIBLE  = 0x08, // unsure
    FACTION_FLAG_OWN_TEAM   = 0x10,
    FACTION_FLAG_INACTIVE   = 0x20
};

struct Faction
{
    uint32 ID;
    uint32 ReputationListID;
    uint32 Flags;
    int32  Standing;
    FactionState uState;
};

typedef std::list<Faction> FactionsList;

struct EnchantDuration
{
    EnchantDuration() : item(NULL), slot(MAX_ENCHANTMENT_SLOT), leftduration(0) {};
    EnchantDuration(Item * _item, EnchantmentSlot _slot, uint32 _leftduration) : item(_item), slot(_slot), leftduration(_leftduration) { assert(item); };

    Item * item;
    EnchantmentSlot slot;
    uint32 leftduration;
};

typedef std::list<EnchantDuration> EnchantDurationList;


struct LookingForGroupSlot
{
    LookingForGroupSlot() : entry(0), type(0) {}
    bool Empty() const { return !entry && !type; }
    void Clear() { entry = 0; type = 0; }
    void Set(uint32 _entry, uint32 _type ) { entry = _entry; type = _type; }
    bool Is(uint32 _entry, uint32 _type) const { return entry==_entry && type==_type; }
    bool canAutoJoin() const { return entry && (type == 1 || type == 5); }

    uint32 entry;
    uint32 type;
};

#define MAX_LOOKING_FOR_GROUP_SLOT 3

struct LookingForGroup
{
    LookingForGroup() {}
    bool HaveInSlot(LookingForGroupSlot const& slot) const { return HaveInSlot(slot.entry,slot.type); }
    bool HaveInSlot(uint32 _entry, uint32 _type) const
    {
        for(int i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
            if(slots[i].Is(_entry,_type))
                return true;
        return false;
    }

    bool canAutoJoin() const
    {
        for(int i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
            if(slots[i].canAutoJoin())
                return true;
        return false;
    }

    LookingForGroupSlot slots[MAX_LOOKING_FOR_GROUP_SLOT];
    LookingForGroupSlot more;
    std::string comment;
};

enum PlayerMovementType
{
    MOVE_ROOT       = 1,
    MOVE_UNROOT     = 2,
    MOVE_WATER_WALK = 3,
    MOVE_LAND_WALK  = 4
};

enum PlayerStateType
{
    /*
        PLAYER_STATE_DANCE
        PLAYER_STATE_SLEEP
        PLAYER_STATE_SIT
        PLAYER_STATE_STAND
        PLAYER_STATE_READYUNARMED
        PLAYER_STATE_WORK
        PLAYER_STATE_POINT(DNR)
        PLAYER_STATE_NONE // not used or just no state, just standing there?
        PLAYER_STATE_STUN
        PLAYER_STATE_DEAD
        PLAYER_STATE_KNEEL
        PLAYER_STATE_USESTANDING
        PLAYER_STATE_STUN_NOSHEATHE
        PLAYER_STATE_USESTANDING_NOSHEATHE
        PLAYER_STATE_WORK_NOSHEATHE
        PLAYER_STATE_SPELLPRECAST
        PLAYER_STATE_READYRIFLE
        PLAYER_STATE_WORK_NOSHEATHE_MINING
        PLAYER_STATE_WORK_NOSHEATHE_CHOPWOOD
        PLAYER_STATE_AT_EASE
        PLAYER_STATE_READY1H
        PLAYER_STATE_SPELLKNEELSTART
        PLAYER_STATE_SUBMERGED
    */

    PLAYER_STATE_NONE             = 0,
    PLAYER_STATE_SIT              = 1,
    PLAYER_STATE_SIT_CHAIR        = 2,
    PLAYER_STATE_SLEEP            = 3,
    PLAYER_STATE_SIT_LOW_CHAIR    = 4,
    PLAYER_STATE_SIT_MEDIUM_CHAIR = 5,
    PLAYER_STATE_SIT_HIGH_CHAIR   = 6,
    PLAYER_STATE_DEAD             = 7,
    PLAYER_STATE_KNEEL            = 8,

    PLAYER_STATE_FORM_ALL          = 0x00FF0000,

    PLAYER_STATE_FLAG_ALWAYS_STAND = 0x01000000,
    PLAYER_STATE_FLAG_STEALTH      = 0x02000000,
    PLAYER_STATE_FLAG_ALL          = 0xFF000000,
};

enum PlayerFlags
{
    PLAYER_FLAGS_GROUP_LEADER   = 0x00000001,
    PLAYER_FLAGS_AFK            = 0x00000002,
    PLAYER_FLAGS_DND            = 0x00000004,
    PLAYER_FLAGS_GM             = 0x00000008,
    PLAYER_FLAGS_GHOST          = 0x00000010,
    PLAYER_FLAGS_RESTING        = 0x00000020,
    PLAYER_FLAGS_FFA_PVP        = 0x00000080,
    PLAYER_FLAGS_UNK            = 0x00000100,                   // show PvP in tooltip
    PLAYER_FLAGS_IN_PVP         = 0x00000200,
    PLAYER_FLAGS_HIDE_HELM      = 0x00000400,
    PLAYER_FLAGS_HIDE_CLOAK     = 0x00000800,
    PLAYER_FLAGS_UNK1           = 0x00001000,                   // played long time
    PLAYER_FLAGS_UNK2           = 0x00002000,                   // played too long time
    PLAYER_FLAGS_UNK3           = 0x00008000,                   // strange visual effect (2.0.1), looks like PLAYER_FLAGS_GHOST flag
    PLAYER_FLAGS_UNK4           = 0x00020000,                   // taxi benchmark mode (on/off) (2.0.1)
    PLAYER_UNK                  = 0x00040000,                   // 2.0.8...
};

enum PlayerKnownTitles
{
    PLAYER_TITLE_DISABLED             = 0x00000000,
    PLAYER_TITLE_NONE                 = 0x00000001,
    PLAYER_TITLE_PRIVATE              = 0x00000002,
    PLAYER_TITLE_CORPORAL             = 0x00000004,
    PLAYER_TITLE_SERGEANT_A           = 0x00000008,
    PLAYER_TITLE_MASTER_SERGEANT      = 0x00000010,
    PLAYER_TITLE_SERGEANT_MAJOR       = 0x00000020,
    PLAYER_TITLE_KNIGHT               = 0x00000040,
    PLAYER_TITLE_KNIGHT_LIEUTENANT    = 0x00000080,
    PLAYER_TITLE_KNIGHT_CAPTAIN       = 0x00000100,
    PLAYER_TITLE_KNIGHT_CHAMPION      = 0x00000200,
    PLAYER_TITLE_LIEUTENANT_COMMANDER = 0x00000400,
    PLAYER_TITLE_COMMANDER            = 0x00000800,
    PLAYER_TITLE_MARSHAL              = 0x00001000,
    PLAYER_TITLE_FIELD_MARSHAL        = 0x00002000,
    PLAYER_TITLE_GRAND_MARSHAL        = 0x00004000,
    PLAYER_TITLE_SCOUT                = 0x00008000,
    PLAYER_TITLE_GRUNT                = 0x00010000,
    PLAYER_TITLE_SERGEANT_H           = 0x00020000,
    PLAYER_TITLE_SENIOR_SERGEANT      = 0x00040000,
    PLAYER_TITLE_FIRST_SERGEANT       = 0x00080000,
    PLAYER_TITLE_STONE_GUARD          = 0x00100000,
    PLAYER_TITLE_BLOOD_GUARD          = 0x00200000,
    PLAYER_TITLE_LEGIONNAIRE          = 0x00400000,
    PLAYER_TITLE_CENTURION            = 0x00800000,
    PLAYER_TITLE_CHAMPION             = 0x01000000,
    PLAYER_TITLE_LIEUTENANT_GENERAL   = 0x02000000,
    PLAYER_TITLE_GENERAL              = 0x04000000,
    PLAYER_TITLE_WARLORD              = 0x08000000,
    PLAYER_TITLE_HIGH_WARLORD         = 0x10000000,
    PLAYER_TITLE_UNUSED1              = 0x20000000,
    PLAYER_TITLE_UNUSED2              = 0x40000000
};

enum DungeonDifficulties
{
    DUNGEONDIFFICULTY_NORMAL = 0,
    DUNGEONDIFFICULTY_HEROIC = 1
};

enum LootType
{
    LOOT_CORPSE                 = 1,
    LOOT_SKINNING               = 2,
    LOOT_FISHING                = 3,
    LOOT_PICKPOCKETING          = 4,                        // unsupported by client, sending LOOT_SKINNING instead
    LOOT_DISENCHANTING          = 5                         // unsupported by client, sending LOOT_SKINNING instead
};

enum MirrorTimerType
{
    FATIGUE_TIMER      = 0,
    BREATH_TIMER       = 1,
    FIRE_TIMER         = 2
};

// 2^n values
enum GMFlags
{
    GM_ON              = 1,
    GM_ACCEPT_TICKETS  = 2,
    GM_ACCEPT_WHISPERS = 4,
    GM_TAXICHEAT       = 8,                                 // can be applied to non-gm players
    GM_INVISIBLE       = 16
};

typedef std::set<uint32> IgnoreList;

typedef std::map<uint32, struct quest_status> QuestStatusMap;

#define IS_BACK_SLOT(s) (s == 0xFF)

class Quest;
class Spell;
class Item;
class WorldSession;

enum PlayerSlots
{
    // first slot for item stored (in any way in player m_items data)
    PLAYER_SLOT_START           = 0,
    // last+1 slot for item stored (in any way in player m_items data)
    PLAYER_SLOT_END             = 118,
    PLAYER_SLOTS_COUNT          = (PLAYER_SLOT_END - PLAYER_SLOT_START)
};

enum EquipmentSlots
{
    EQUIPMENT_SLOT_START        = 0,
    EQUIPMENT_SLOT_HEAD         = 0,
    EQUIPMENT_SLOT_NECK         = 1,
    EQUIPMENT_SLOT_SHOULDERS    = 2,
    EQUIPMENT_SLOT_BODY         = 3,
    EQUIPMENT_SLOT_CHEST        = 4,
    EQUIPMENT_SLOT_WAIST        = 5,
    EQUIPMENT_SLOT_LEGS         = 6,
    EQUIPMENT_SLOT_FEET         = 7,
    EQUIPMENT_SLOT_WRISTS       = 8,
    EQUIPMENT_SLOT_HANDS        = 9,
    EQUIPMENT_SLOT_FINGER1      = 10,
    EQUIPMENT_SLOT_FINGER2      = 11,
    EQUIPMENT_SLOT_TRINKET1     = 12,
    EQUIPMENT_SLOT_TRINKET2     = 13,
    EQUIPMENT_SLOT_BACK         = 14,
    EQUIPMENT_SLOT_MAINHAND     = 15,
    EQUIPMENT_SLOT_OFFHAND      = 16,
    EQUIPMENT_SLOT_RANGED       = 17,
    EQUIPMENT_SLOT_TABARD       = 18,
    EQUIPMENT_SLOT_END          = 19
};

enum InventorySlots
{
    INVENTORY_SLOT_BAG_0        = 255,
    INVENTORY_SLOT_BAG_START    = 19,
    INVENTORY_SLOT_BAG_1        = 19,
    INVENTORY_SLOT_BAG_2        = 20,
    INVENTORY_SLOT_BAG_3        = 21,
    INVENTORY_SLOT_BAG_4        = 22,
    INVENTORY_SLOT_BAG_END      = 23,

    INVENTORY_SLOT_ITEM_START   = 23,
    INVENTORY_SLOT_ITEM_1       = 23,
    INVENTORY_SLOT_ITEM_2       = 24,
    INVENTORY_SLOT_ITEM_3       = 25,
    INVENTORY_SLOT_ITEM_4       = 26,
    INVENTORY_SLOT_ITEM_5       = 27,
    INVENTORY_SLOT_ITEM_6       = 28,
    INVENTORY_SLOT_ITEM_7       = 29,
    INVENTORY_SLOT_ITEM_8       = 30,
    INVENTORY_SLOT_ITEM_9       = 31,
    INVENTORY_SLOT_ITEM_10      = 32,
    INVENTORY_SLOT_ITEM_11      = 33,
    INVENTORY_SLOT_ITEM_12      = 34,
    INVENTORY_SLOT_ITEM_13      = 35,
    INVENTORY_SLOT_ITEM_14      = 36,
    INVENTORY_SLOT_ITEM_15      = 37,
    INVENTORY_SLOT_ITEM_16      = 38,
    INVENTORY_SLOT_ITEM_END     = 39
};

enum BankSlots
{
    BANK_SLOT_ITEM_START        = 39,
    BANK_SLOT_ITEM_1            = 39,
    BANK_SLOT_ITEM_2            = 40,
    BANK_SLOT_ITEM_3            = 41,
    BANK_SLOT_ITEM_4            = 42,
    BANK_SLOT_ITEM_5            = 43,
    BANK_SLOT_ITEM_6            = 44,
    BANK_SLOT_ITEM_7            = 45,
    BANK_SLOT_ITEM_8            = 46,
    BANK_SLOT_ITEM_9            = 47,
    BANK_SLOT_ITEM_10           = 48,
    BANK_SLOT_ITEM_11           = 49,
    BANK_SLOT_ITEM_12           = 50,
    BANK_SLOT_ITEM_13           = 51,
    BANK_SLOT_ITEM_14           = 52,
    BANK_SLOT_ITEM_15           = 53,
    BANK_SLOT_ITEM_16           = 54,
    BANK_SLOT_ITEM_17           = 55,
    BANK_SLOT_ITEM_18           = 56,
    BANK_SLOT_ITEM_19           = 57,
    BANK_SLOT_ITEM_20           = 58,
    BANK_SLOT_ITEM_21           = 59,
    BANK_SLOT_ITEM_22           = 60,
    BANK_SLOT_ITEM_23           = 61,
    BANK_SLOT_ITEM_24           = 62,
    BANK_SLOT_ITEM_25           = 63,
    BANK_SLOT_ITEM_26           = 64,
    BANK_SLOT_ITEM_27           = 65,
    BANK_SLOT_ITEM_28           = 66,
    BANK_SLOT_ITEM_END          = 67,

    BANK_SLOT_BAG_START         = 67,
    BANK_SLOT_BAG_1             = 67,
    BANK_SLOT_BAG_2             = 68,
    BANK_SLOT_BAG_3             = 69,
    BANK_SLOT_BAG_4             = 70,
    BANK_SLOT_BAG_5             = 71,
    BANK_SLOT_BAG_6             = 72,
    BANK_SLOT_BAG_7             = 73,
    BANK_SLOT_BAG_END           = 74
};

enum BuyBackSlots
{
    // stored in m_buybackitems
    BUYBACK_SLOT_START          = 74,
    BUYBACK_SLOT_1              = 74,
    BUYBACK_SLOT_2              = 75,
    BUYBACK_SLOT_3              = 76,
    BUYBACK_SLOT_4              = 77,
    BUYBACK_SLOT_5              = 78,
    BUYBACK_SLOT_6              = 79,
    BUYBACK_SLOT_7              = 80,
    BUYBACK_SLOT_8              = 81,
    BUYBACK_SLOT_9              = 82,
    BUYBACK_SLOT_10             = 83,
    BUYBACK_SLOT_11             = 84,
    BUYBACK_SLOT_12             = 85,
    BUYBACK_SLOT_END            = 86
};

enum KeyRingSLots
{
    KEYRING_SLOT_START          = 86,
    KEYRING_SLOT_END            = 118
};

enum SwitchWeapon
{
    DEFAULT_SWITCH_WEAPON       = 1500,                     //cooldown in ms
    ROGUE_SWITCH_WEAPON         = 1000
};

enum TradeSlots
{
    TRADE_SLOT_COUNT            = 7,
    TRADE_SLOT_TRADED_COUNT     = 6,
    TRADE_SLOT_NONTRADED        = 6
};

enum MovementFlags
{
    MOVEMENTFLAG_FORWARD        = 0x1,
    MOVEMENTFLAG_BACKWARD       = 0x2,
    MOVEMENTFLAG_STRAFE_LEFT    = 0x4,
    MOVEMENTFLAG_STRAFE_RIGHT   = 0x8,
    MOVEMENTFLAG_LEFT           = 0x10,
    MOVEMENTFLAG_RIGHT          = 0x20,
    MOVEMENTFLAG_PITCH_UP       = 0x40,
    MOVEMENTFLAG_PITCH_DOWN     = 0x80,
    MOVEMENTFLAG_WALK           = 0x100,
    MOVEMENTFLAG_ONTRANSPORT    = 0x200,
    // 0x400
    MOVEMENTFLAG_FLY_UNK1       = 0x800,
    MOVEMENTFLAG_UNK4           = 0x1000,
    MOVEMENTFLAG_JUMPING        = 0x2000,
    MOVEMENTFLAG_FALLING        = 0x4000,
    // 0x8000, 0x10000, 0x20000, 0x40000, 0x80000, 0x100000
    MOVEMENTFLAG_SWIMMING       = 0x200000, // appears with fly also, swim in air LOL
    MOVEMENTFLAG_FLY_UP         = 0x400000,
    MOVEMENTFLAG_CAN_FLY        = 0x800000,
    MOVEMENTFLAG_FLYING         = 0x1000000,
    // 0x2000000
    MOVEMENTFLAG_SPLINE         = 0x4000000,
    // 0x8000000
    MOVEMENTFLAG_WATERWALKING   = 0x10000000,
    MOVEMENTFLAG_UNK2           = 0x20000000,
    MOVEMENTFLAG_UNK3           = 0x40000000
};

typedef HM_NAMESPACE::hash_map< uint32, std::pair < uint32, uint32 > > BoundInstancesMap;

class MANGOS_DLL_SPEC Player : public Unit
{
    friend class WorldSession;
    friend void Item::AddToUpdateQueueOf(Player *player);
    friend void Item::RemoveFromUpdateQueueOf(Player *player);
    public:
        explicit Player (WorldSession *session);
        ~Player ( );

        static UpdateMask updateVisualBits;
        static void InitVisibleBits();

        void AddToWorld();
        void RemoveFromWorld();

        void TeleportTo(uint32 mapid, float x, float y, float z, float orientation, bool outofrange = true, bool ignore_transport = true, bool is_gm_command = false);

        bool Create ( uint32 guidlow, WorldPacket &data );

        void Update( uint32 time );

        void BuildEnumData( WorldPacket * p_data );

        void SetInWater(bool apply);

        bool IsInWater() const { return m_isInWater; }
        bool IsUnderWater() const;

        bool CanInteractWithNPCs(bool alive = true) const;

        bool ToggleAFK();
        bool ToggleDND();
        bool isAFK() { return this->HasFlag(PLAYER_FLAGS,PLAYER_FLAGS_AFK); };
        bool isDND() { return this->HasFlag(PLAYER_FLAGS,PLAYER_FLAGS_DND); };
        uint8 chatTag();
        std::string afkMsg;
        std::string dndMsg;

        void SendFriendlist();
        void SendIgnorelist();
        void AddToIgnoreList(uint64 guid, std::string name);
        void RemoveFromIgnoreList(uint64 guid);
        bool HasInIgnoreList(uint64 guid) const { return m_ignorelist.find(GUID_LOPART(guid)) != m_ignorelist.end(); }

        uint32 GetTaximask( uint8 index ) const { return m_taximask[index]; }
        void SetTaximask( uint8 index, uint32 value ) { m_taximask[index] = value; }
        void ClearTaxiDestinations() { m_TaxiDestinations.clear(); }
        void AddTaxiDestination(uint32 dest) { m_TaxiDestinations.push_back(dest); }
        uint32 GetTaxiSource() const { return m_TaxiDestinations.empty() ? 0 : m_TaxiDestinations.front(); }
        uint32 NextTaxiDestination()
        {
            m_TaxiDestinations.pop_front();
            return m_TaxiDestinations.empty() ? 0 : m_TaxiDestinations.front();
        }
        bool ActivateTaxiPathTo(std::vector<uint32> const& nodes );

        bool isAcceptTickets() const;
        void SetAcceptTicket(bool on) { if(on) m_GMFlags |= GM_ACCEPT_TICKETS; else m_GMFlags &= ~GM_ACCEPT_TICKETS; }
        bool isAcceptWhispers() const { return m_GMFlags & GM_ACCEPT_WHISPERS; }
        void SetAcceptWhispers(bool on) { if(on) m_GMFlags |= GM_ACCEPT_WHISPERS; else m_GMFlags &= ~GM_ACCEPT_WHISPERS; }
        bool isGameMaster() const { return m_GMFlags & GM_ON; }
        void SetGameMaster(bool on);
        bool isTaxiCheater() const { return m_GMFlags & GM_TAXICHEAT; }
        void SetTaxiCheater(bool on) { if(on) m_GMFlags |= GM_TAXICHEAT; else m_GMFlags &= ~GM_TAXICHEAT; }
        bool isGMVisible() const { return !(m_GMFlags & GM_INVISIBLE); }
        void SetGMVisible(bool on);

        void GiveXP(uint32 xp, Unit* victim);
        void GiveLevel();
        void InitStatsForLevel(uint32 level, bool sendgain = true, bool remove_mods = true);

        void setDismountCost(uint32 money) { m_dismountCost = money; };

        // Played Time Stuff
        time_t m_logintime;
        time_t m_Last_tick;
        uint32 m_Played_time[2];
        uint32 GetTotalPlayedTime() { return m_Played_time[0]; };
        uint32 GetLevelPlayedTime() { return m_Played_time[1]; };

        void setDeathState(DeathState s);                   // overwrite Unit::setDeathState

        void InnEnter (int time,float x,float y,float z)
        {
            inn_pos_x = x;
            inn_pos_y = y;
            inn_pos_z = z;
            time_inn_enter = time;
        };

        float GetRestBonus() const { return m_rest_bonus; };
        void SetRestBonus(float rest_bonus_new);

        int GetRestType() const { return rest_type; };
        void SetRestType(int n_r_type) { rest_type = n_r_type; };

        float GetInnPosX () const { return inn_pos_x; };
        float GetInnPosY () const { return inn_pos_y; };
        float GetInnPosZ () const { return inn_pos_z; };

        int GetTimeInnEter() const { return time_inn_enter; };
        void UpdateInnerTime (int time) { time_inn_enter = time; };

        void RemovePet(Pet* pet, PetSaveMode mode);
        void Uncharm();

        void Say(const std::string text, const uint32 language);
        void Yell(const std::string text, const uint32 language);
        void TextEmote(const std::string text);
        void Whisper(const uint64 receiver, const std::string text, const uint32 language);

        /*********************************************************/
        /***                    STORAGE SYSTEM                 ***/
        /*********************************************************/

        void SetVirtualItemSlot( uint8 i, Item* item);
        void SetSheath( uint32 sheathed );
        uint8 FindEquipSlot( uint32 type, uint32 slot, bool swap ) const;
        Item* CreateItem( uint32 item, uint32 count ) const;
        uint32 GetItemCount( uint32 item, Item* eItem = NULL ) const;
        uint32 GetBankItemCount( uint32 item, Item* eItem = NULL ) const;
        uint16 GetPosByGuid( uint64 guid ) const;
        Item* GetItemByPos( uint16 pos ) const;
        Item* GetItemByPos( uint8 bag, uint8 slot ) const;
        std::vector<Item *> &GetItemUpdateQueue() { return m_itemUpdateQueue; }
        static bool IsInventoryPos( uint16 pos ) { return IsInventoryPos(pos >> 8,pos & 255); }
        static bool IsInventoryPos( uint8 bag, uint8 slot );
        static bool IsEquipmentPos( uint16 pos ) { return IsEquipmentPos(pos >> 8,pos & 255); }
        static bool IsEquipmentPos( uint8 bag, uint8 slot );
        static bool IsBagPos( uint16 pos );
        static bool IsBankPos( uint16 pos ) { return IsBankPos(pos >> 8,pos & 255); }
        static bool IsBankPos( uint8 bag, uint8 slot );
        bool HasBankBagSlot( uint8 slot ) const;
        bool HasItemCount( uint32 item, uint32 count ) const;
        uint32 GetFreeSlots() const;
        uint8 CanTakeMoreSimilarItems(Item* pItem) const;
        uint8 CanStoreNewItem( uint8 bag, uint8 slot, uint16 &dest, uint32 item, uint32 count, bool swap ) const;
        uint8 CanStoreItem( uint8 bag, uint8 slot, uint16 &dest, Item *pItem, bool swap ) const;
        uint8 CanStoreItems( Item **pItem,int count) const;
        uint8 CanEquipNewItem( uint8 slot, uint16 &dest, uint32 item, uint32 count, bool swap ) const;
        uint8 CanEquipItem( uint8 slot, uint16 &dest, Item *pItem, bool swap, bool not_loading = true ) const;
        uint8 CanUnequipItem( uint16 src, bool swap ) const;
        uint8 CanBankItem( uint8 bag, uint8 slot, uint16 &dest, Item *pItem, bool swap, bool not_loading = true ) const;
        uint8 CanUseItem( Item *pItem, bool not_loading = true ) const;
        bool CanUseItem( ItemPrototype const *pItem );
        uint8 CanUseAmmo( uint32 item ) const;
        Item* StoreNewItem( uint16 pos, uint32 item, uint32 count, bool update,uint32 randomPropertyId = 0 );
        Item* StoreItem( uint16 pos, Item *pItem, bool update );
        Item* EquipNewItem( uint16 pos, uint32 item, uint32 count, bool update );
        Item* EquipItem( uint16 pos, Item *pItem, bool update );
        void SetAmmo( uint32 item );
        void QuickEquipItem( uint16 pos, Item *pItem);
        void VisualizeItem( uint16 pos, Item *pItem);
        Item* BankItem( uint16 pos, Item *pItem, bool update );
        void RemoveItem( uint8 bag, uint8 slot, bool update );
        void RemoveItemCount( uint32 item, uint32 count, bool update );
        void DestroyItem( uint8 bag, uint8 slot, bool update );
        void DestroyItemCount( uint32 item, uint32 count, bool update );
        void DestroyItemCount( Item* item, uint32& count, bool update );
        void SplitItem( uint16 src, uint16 dst, uint32 count );
        void SwapItem( uint16 src, uint16 dst );
        void AddItemToBuyBackSlot( Item *pItem );
        Item* GetItemFromBuyBackSlot( uint32 slot );
        void RemoveItemFromBuyBackSlot( uint32 slot, bool del );
        uint32 GetMaxKeyringSize() const { return KEYRING_SLOT_END-KEYRING_SLOT_START; }
        void SendEquipError( uint8 msg, Item* pItem, Item *pItem2 );
        void SendBuyError( uint8 msg, Creature* pCreature, uint32 item, uint32 param );
        void SendSellError( uint8 msg, Creature* pCreature, uint64 guid, uint32 param );
        void AddWeaponProficiency(uint32 newflag) { m_WeaponProficiency |= newflag ;}
        void AddArmorProficiency(uint32 newflag) { m_ArmorProficiency |= newflag ;}
        uint32 GetWeaponProficiency() const { return m_WeaponProficiency;}
        uint32 GetArmorProficiency() const { return m_ArmorProficiency;}
        bool IsUseEquipedWeapon() const { return m_form != FORM_CAT && m_form != FORM_BEAR && m_form != FORM_DIREBEAR; }
        void SendNewItem( Item *item, uint32 count, bool received, bool created, bool broadcast = false );
        void BuyItemFromVendor(uint64 vendorguid, uint32 item, uint8 count, uint64 bagguid, uint8 slot);

        Player* GetTrader() const { return pTrader; }
        void ClearTrade();
        void TradeCancel(bool sendback);

        void UpdateEnchantTime(uint32 time);
        void ReducePoisonCharges(uint32 enchantId);
        void AddEnchantmentDurations(Item *item);
        void RemoveEnchantmentDurations(Item *item);
        void AddEnchantmentDuration(Item *item,EnchantmentSlot slot,uint32 duration);
        void ApplyEnchantment(Item *item,EnchantmentSlot slot,bool apply, bool apply_dur = true, bool ignore_condition = false);
        void ApplyEnchantment(Item *item,bool apply);
        void SaveEnchant();
        void LoadEnchant();
        void LoadCorpse();
        void LoadPet();
        void RemoveAreaAurasFromGroup();

        uint32 m_stableSlots;

        /*********************************************************/
        /***                    QUEST SYSTEM                   ***/
        /*********************************************************/

        void PrepareQuestMenu( uint64 guid );
        void SendPreparedQuest( uint64 guid );
        Quest *GetActiveQuest( uint32 quest_id ) const;
        Quest *GetNextQuest( uint64 guid, Quest *pQuest );
        bool CanSeeStartQuest( uint32 quest_id );
        bool CanTakeQuest( Quest *pQuest, bool msg );
        bool CanAddQuest( Quest *pQuest, bool msg );
        bool CanCompleteQuest( uint32 quest_id );
        bool CanRewardQuest( Quest *pQuest, bool msg );
        bool CanRewardQuest( Quest *pQuest, uint32 reward, bool msg );
        void AddQuest( Quest *pQuest );
        void CompleteQuest( uint32 quest_id );
        void IncompleteQuest( uint32 quest_id );
        void RewardQuest( Quest *pQuest, uint32 reward, Object* questGiver );
        void FailQuest( uint32 quest_id );
        void FailTimedQuest( uint32 quest_id );
        bool SatisfyQuestClass( uint32 quest_id, bool msg );
        bool SatisfyQuestLevel( uint32 quest_id, bool msg );
        bool SatisfyQuestLog( bool msg );
        bool SatisfyQuestPreviousQuest( uint32 quest_id, bool msg );
        bool SatisfyQuestRace( uint32 quest_id, bool msg );
        bool SatisfyQuestReputation( uint32 quest_id, bool msg );
        bool SatisfyQuestSkill( uint32 quest_id, bool msg );
        bool SatisfyQuestStatus( uint32 quest_id, bool msg );
        bool SatisfyQuestTimed( uint32 quest_id, bool msg );
        bool SatisfyQuestExclusiveGroup( uint32 quest_id, bool msg );
        bool SatisfyQuestNextChain( uint32 quest_id, bool msg );
        bool SatisfyQuestPrevChain( uint32 quest_id, bool msg );
        bool GiveQuestSourceItem( uint32 quest_id );
        void TakeQuestSourceItem( uint32 quest_id );
        bool GetQuestRewardStatus( uint32 quest_id );
        QuestStatus GetQuestStatus( uint32 quest_id );
        void SetQuestStatus( uint32 quest_id, QuestStatus status );
        void AdjustQuestReqItemCount( uint32 questId );
        uint16 GetQuestSlot( uint32 quest_id );
        void AreaExplored( uint32 questId );
        void ItemAddedQuestCheck( uint32 entry, uint32 count );
        void ItemRemovedQuestCheck( uint32 entry, uint32 count );
        void KilledMonster( uint32 entry, uint64 guid );
        void CastedCreatureOrGO( uint32 entry, uint64 guid, uint32 spell_id );
        void MoneyChanged( uint32 value );
        bool HasQuestForItem( uint32 itemid );
        bool CanShareQuest(uint32 quest_id);

        void SendQuestComplete( uint32 quest_id );
        void SendQuestReward( Quest *pQuest, uint32 XP, Object* questGiver );
        void SendQuestFailed( uint32 quest_id );
        void SendQuestTimerFailed( uint32 quest_id );
        void SendCanTakeQuestResponse( uint32 msg );
        void SendPushToPartyResponse( Player *pPlayer, uint32 msg );
        void SendQuestUpdateAddItem( uint32 quest_id, uint32 item_idx, uint32 count );
        void SendQuestUpdateAddCreature( uint32 quest_id, uint64 guid, uint32 creature_idx, uint32 old_count, uint32 add_count );

        uint64 GetDivider() { return m_divider; };
        void SetDivider( uint64 guid ) { m_divider = guid; };

        uint32 GetInGameTime() { return m_ingametime; };

        void SetInGameTime( uint32 time ) { m_ingametime = time; };

        void AddTimedQuest( uint32 quest_id ) { m_timedquests.insert(quest_id); }

        /*********************************************************/
        /***                   LOAD SYSTEM                     ***/
        /*********************************************************/

        bool LoadFromDB(uint32 guid);
        bool MinimalLoadFromDB(uint32 guid);
        static bool   LoadValuesArrayFromDB(vector<string>& data,uint64 guid);
        static uint32 GetUInt32ValueFromArray(vector<string> const& data, uint16 index);
        static float  GetFloatValueFromArray(vector<string> const& data, uint16 index);
        static uint32 GetUInt32ValueFromDB(uint16 index, uint64 guid);
        static float  GetFloatValueFromDB(uint16 index, uint64 guid);
        static uint32 GetZoneIdFromDB(uint64 guid);
        static bool   LoadPositionFromDB(uint32& mapid, float& x,float& y,float& z,float& o, uint64 guid);

        /*********************************************************/
        /***                   SAVE SYSTEM                     ***/
        /*********************************************************/

        void SaveToDB();
        void SaveInventoryAndGoldToDB();                    // fast save function for item/money cheating preventing
        static bool SaveValuesArrayInDB(vector<string> const& data,uint64 guid);
        static void SetUInt32ValueInArray(vector<string>& data,uint16 index, uint32 value);
        static void SetFloatValueInArray(vector<string>& data,uint16 index, float value);
        static void SetUInt32ValueInDB(uint16 index, uint32 value, uint64 guid);
        static void SetFloatValueInDB(uint16 index, float value, uint64 guid);
        static void SavePositionInDB(uint32 mapid, float x,float y,float z,float o, uint64 guid);

        bool m_mailsLoaded;
        bool m_mailsUpdated;

        void SetBindPoint(uint64 guid);
        void SendTalentWipeConfirm(uint64 guid);
        void CalcRage( uint32 damage,bool attacker );
        void RegenerateAll();
        void Regenerate(Powers power);
        void RegenerateHealth();
        void setRegenTimer(uint32 time) {m_regenTimer = time;}
        void setWeaponChangeTimer(uint32 time) {m_weaponChangeTimer = time;}

        uint32 GetMoney() { return GetUInt32Value (PLAYER_FIELD_COINAGE); }
        void ModifyMoney( int32 d ) { SetMoney (GetMoney() + d > 0 ? GetMoney() + d : 0); }
        void SetMoney( uint32 value )
        {
            SetUInt32Value (PLAYER_FIELD_COINAGE, value);
            MoneyChanged( value );
        }

        uint32 GetTutorialInt(uint32 intId )
        {
            ASSERT( (intId < 8) );
            return m_Tutorials[intId];
        }

        void SetTutorialInt(uint32 intId, uint32 value)
        {
            ASSERT( (intId < 8) );
            m_Tutorials[intId] = value;
        }
        bool HasItemInBackpack (uint32 itemId, uint32 count = 1) { return false; }
        bool HasSpaceForItemInBackpack (uint32 itemId, uint32 count = 1) { return false; }

        QuestStatusMap& getQuestStatusMap() { return mQuestStatus; };

        const uint64& GetSelection( ) const { return m_curSelection; }
        const uint64& GetTarget( ) const { return m_curTarget; }

        void SetSelection(const uint64 &guid) { m_curSelection = guid; }
        void SetTarget(const uint64 &guid) { m_curTarget = guid; }

        void CreateMail(uint32 mailId, uint8 messageType, uint32 sender, std::string subject, uint32 itemTextId, uint32 itemGuid, uint32 item_template, time_t expire_time,time_t delivery_time, uint32 money, uint32 COD, uint32 checked, Item* pItem);
        void SendMailResult(uint32 mailId, uint32 mailAction, uint32 mailError, uint32 equipError = 0);
        void SendNewMail();
        void UpdateNextMailTimeAndUnreads();

        //void SetMail(Mail *m);
        void RemoveMail(uint32 id);

        uint32 GetMailSize() { return m_mail.size();};
        Mail* GetMail(uint32 id);

        PlayerMails::iterator GetmailBegin() { return m_mail.begin();};
        PlayerMails::iterator GetmailEnd() { return m_mail.end();};

        /*********************************************************/
        /*** MAILED ITEMS SYSTEM ***/
        /*********************************************************/

        uint8 unReadMails;
        time_t m_nextMailDelivereTime;

        typedef HM_NAMESPACE::hash_map<uint32, Item*> ItemMap;

        ItemMap mMitems;                                    //template defined in objectmgr.cpp

        Item* GetMItem(uint32 id)
        {
            ItemMap::const_iterator itr = mMitems.find(id);
            if (itr != mMitems.end())
                return itr->second;

            return NULL;
        }

        void AddMItem(Item* it)
        {
            ASSERT( it );
            //assert deleted, because items can be added before loading
            mMitems[it->GetGUIDLow()] = it;
        }

        bool RemoveMItem(uint32 id)
        {
            ItemMap::iterator i = mMitems.find(id);
            if (i == mMitems.end())
                return false;

            mMitems.erase(i);
            return true;
        }

        void PetSpellInitialize();
        bool HasSpell(uint32 spell) const;
        TrainerSpellState GetTrainerSpellState(TrainerSpell const* trainer_spell);
        void SendProficiency(uint8 pr1, uint32 pr2);
        void SendInitialSpells();
        bool addSpell(uint16 spell_id,uint8 active, PlayerSpellState state = PLAYERSPELL_NEW, uint16 slot_id=0xffff);
        bool learnSpell(uint16 spell_id);
        void removeSpell(uint16 spell_id);
        bool resetTalents(bool no_cost = false);
        uint32 resetTalentsCost() const;
        void InitTalentForLevel();

        PlayerSpellMap const& GetSpellMap() const { return m_spells; }
        PlayerSpellMap      & GetSpellMap()       { return m_spells; }

        SpellModList *getSpellModList(int op) { return &m_spellMods[op]; }
        int32 GetTotalFlatMods(uint32 spellId, uint8 op);
        int32 GetTotalPctMods(uint32 spellId, uint8 op);
        bool IsAffectedBySpellmod(SpellEntry const *spellInfo, SpellModifier *mod);
        template <class T> T ApplySpellMod(uint32 spellId, uint8 op, T &basevalue);

        bool HasSpellCooldown(uint32 spell_id) const
        {
            SpellCooldowns::const_iterator itr = m_spellCooldowns.find(spell_id);
            return itr != m_spellCooldowns.end() && itr->second.end > time(NULL);
        }
        uint32 GetSpellCooldownDelay(uint32 spell_id) const
        {
            SpellCooldowns::const_iterator itr = m_spellCooldowns.find(spell_id);
            time_t t = time(NULL);
            return itr != m_spellCooldowns.end() && itr->second.end > t ? itr->second.end - t : 0;
        }
        void AddSpellCooldown(uint32 spell_id, uint32 itemid, time_t end_time);
        void ProhibitSpellScholl(uint32 idSchool /* from SpellSchools */, uint32 unTimeMs );
        void RemoveSpellCooldown(uint32 spell_id) { m_spellCooldowns.erase(spell_id); }
        void RemoveAllSpellCooldown();
        void _LoadSpellCooldowns();
        void _SaveSpellCooldowns();

        void setResurrect(uint64 guid,float X, float Y, float Z, uint32 health, uint32 mana)
        {
            m_resurrectGUID = guid;
            m_resurrectX = X;
            m_resurrectY = Y;
            m_resurrectZ = Z;
            m_resurrectHealth = health;
            m_resurrectMana = mana;
        };

        int getCinematic()
        {
            return m_cinematic;
        }
        void setCinematic(int cine)
        {
            m_cinematic = cine;
        }

        void addActionButton(uint8 button, uint16 action, uint8 type, uint8 misc);
        void removeActionButton(uint8 button);
        void SendInitialActionButtons();

        PvPInfo pvpInfo;
        void UpdatePvP(bool state, bool ovrride=false);
        void UpdateZone(uint32 newZone);
        void UpdatePvPFlag(time_t currTime);

        /** todo: -maybe move UpdateDuelFlag+DuelComplete to independent DuelHandler.. **/
        DuelInfo *duel;
        void UpdateDuelFlag(time_t currTime);
        void CheckDuelDistance(time_t currTime);
        void DuelComplete(uint8 type);

        GroupInfo groupInfo;
        bool IsGroupVisibleFor(Player* p) const;
        bool IsInSameGroupWith(Player const* p) const;
        bool IsInSameRaidWith(Player const* p) const { return groupInfo.group != NULL && groupInfo.group == p->groupInfo.group; }
        void UninviteFromGroup();
        static void RemoveFromGroup(Group* group, uint64 guid);
        void RemoveFromGroup() { RemoveFromGroup(groupInfo.group,GetGUID()); }

        void SetInGuild(uint32 GuildId) { SetUInt32Value(PLAYER_GUILDID, GuildId); Player::SetUInt32ValueInDB(PLAYER_GUILDID, GuildId, this->GetGUID()); }
        void SetRank(uint32 rankId){ SetUInt32Value(PLAYER_GUILDRANK, rankId); Player::SetUInt32ValueInDB(PLAYER_GUILDRANK, rankId, this->GetGUID()); }
        void SetGuildIdInvited(uint32 GuildId) { m_GuildIdInvited = GuildId; }
        uint32 GetGuildId() { return GetUInt32Value(PLAYER_GUILDID);  }
        static uint32 GetGuildIdFromDB(uint64 guid);
        uint32 GetRank(){ return GetUInt32Value(PLAYER_GUILDRANK); }
        static uint32 GetRankFromDB(uint64 guid);
        int GetGuildIdInvited() { return m_GuildIdInvited; }
        static void RemovePetitionsAndSigns(uint64 guid);

        void SetDungeonDifficulty(uint32 difficulty) { m_dungeonDifficulty = difficulty; }
        uint32 GetDungeonDifficulty() { return m_dungeonDifficulty; }

        bool UpdateSkill(uint32 skill_id);

        bool UpdateSkillPro(uint16 SkillId, int32 Chance);
        bool UpdateCraftSkill(uint32 spellid);
        bool UpdateGatherSkill(uint32 SkillId, uint32 SkillValue, uint32 RedLevel, uint32 Multiplicator = 1);
        bool UpdateFishingSkill();

        uint32 GetSpellByProto(ItemPrototype *proto);

        void ApplyDefenseBonusesMod(float value, bool apply);
        void ApplyRatingMod(uint16 index, int32 value, bool apply);
        void UpdateBlockPercentage();

        const uint64& GetLootGUID() const { return m_lootGuid; }
        void SetLootGUID(const uint64 &guid) { m_lootGuid = guid; }

        WorldSession* GetSession() const { return m_session; }
        void SetSession(WorldSession *s) { m_session = s; }

        void BuildCreateUpdateBlockForPlayer( UpdateData *data, Player *target ) const;
        void DestroyForPlayer( Player *target ) const;
        void SendDelayResponse(const uint32);
        void SendLogXPGain(uint32 GivenXP,Unit* victim,uint32 RestXP);
        void SendOutOfRange(Object* obj);

        //Low Level Packets
        void PlaySound(uint32 Sound, bool OnlySelf);
        //notifiers
        void SendAttackSwingCantAttack();
        void SendAttackSwingCancelAttack();
        void SendAttackSwingDeadTarget();
        void SendAttackSwingNotStanding();
        void SendAttackSwingNotInRange();
        void SendAttackSwingBadFacingAttack();
        void SendExplorationExperience(uint32 Area, uint32 Experience);

        void SendDungeonDifficulty();
        void SendAllowMove();

        bool SetPosition(float x, float y, float z, float orientation);
        void SendMessageToSet(WorldPacket *data, bool self);// overwrite Object::SendMessageToSet
        void SendMessageToOwnTeamSet(WorldPacket *data, bool self);

        void DeleteFromDB();

        CorpsePtr& GetCorpse() const;
        void SpawnCorpseBones();
        void CreateCorpse();

        void KillPlayer();
        void ResurrectPlayer();
        void BuildPlayerRepop();
        void DurabilityLossAll(double percent);
        void DurabilityLoss(uint8 equip_pos, double percent);
        void DurabilityPointsLoss(uint8 equip_pos, uint32 points);
        void DurabilityRepairAll(bool cost, bool discount);
        void DurabilityRepair(uint16 pos, bool cost, bool discount);
        void RepopAtGraveyard();

        void SetMovement(uint8 pType);

        void JoinedChannel(Channel *c);
        void LeftChannel(Channel *c);
        void CleanupChannels();

        void BroadcastPacketToFriendListers(WorldPacket *packet);

        void UpdateDefense();
        void UpdateWeaponSkill (WeaponAttackType attType);
        void UpdateCombatSkills(Unit *pVictim, WeaponAttackType attType, MeleeHitOutcome outcome, bool defence);

        void SetSkill(uint32 id, uint16 currVal, uint16 maxVal);
        uint16 GetMaxSkillValue(uint32 skill) const;
        uint16 GetSkillValue(uint32 skill) const;
        uint16 GetPureSkillValue(uint32 skill) const;
        bool HasSkill(uint32 skill) const;

        void SetDontMove(bool dontMove);
        bool GetDontMove() const { return m_dontMove; }

        void CheckExploreSystem(void);

        static uint32 TeamForRace(uint8 race);
        uint32 GetTeam() const { return m_team; }
        static uint32 getFactionForRace(uint8 race);
        void setFactionForRace(uint8 race);

        void SetLastManaUse(time_t spellCastTime) { m_lastManaUse = spellCastTime; }

        FactionsList m_factions;
        int32 GetBaseReputation(const FactionEntry *factionEntry) const;
        int32 GetReputation(uint32 faction_id) const;
        int32 GetReputation(const FactionEntry *factionEntry) const;
        ReputationRank GetReputationRank(uint32 faction) const;
        ReputationRank GetReputationRank(const FactionEntry *factionEntry) const;
        ReputationRank GetBaseReputationRank(const FactionEntry *factionEntry) const;
        ReputationRank ReputationToRank(int32 standing) const;
        const static int32 ReputationRank_Length[MAX_REPUTATION_RANK];
        const static int32 Reputation_Cap    =  42999;
        const static int32 Reputation_Bottom = -42000;
        bool ModifyFactionReputation(uint32 FactionTemplateId, int32 DeltaReputation);
        bool ModifyFactionReputation(FactionEntry const* factionEntry, int32 standing);
        int32 CalculateReputationGain(uint32 creatureOrQuestLevel, int32 rep) const;
        void CalculateReputation(Unit *pVictim);
        void CalculateReputation(Quest *pQuest, uint64 guid);
        void SetInitialFactions();
        void UpdateReputation() const;
        void SendSetFactionStanding(const Faction* faction) const;
        void SendInitialReputations();
        void SetFactionAtWar(uint32 repListID, bool atWar);
        void SetFactionInactive(uint32 repListID, bool inactive);
        void SendSetFactionVisible(const Faction* faction) const;
        void UpdateMaxSkills();
        void UpdateSkillsToMaxSkillsForLevel();             // for .levelup
        void ModifySkillBonus(uint32 skillid,int32 val);

        /*********************************************************/
        /***                  PVP SYSTEM                       ***/
        /*********************************************************/
        void UpdateArenaFields();
        void UpdateHonorFields();
        void CalculateHonor(Unit *pVictim);
        uint32 GetHonorPoints() { return GetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY); }
        uint32 GetArenaPoints() { return GetUInt32Value(PLAYER_FIELD_ARENA_CURRENCY); }
        void SetHonorPoints(uint32 val) { SetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, val); }
        void SetArenaPoints(uint32 val) { SetUInt32Value(PLAYER_FIELD_ARENA_CURRENCY, val); }
        //End of PvP System

        void SetDrunkValue(uint16 newDrunkValue);
        uint16 GetDrunkValue() const { return m_drunk; }
        uint32 GetDeathTimer() const { return m_deathTimer; }
        uint32 GetBlockValue() const;                       // overwrite Unit version (virtual)
        bool CanParry() const { return m_canParry; }
        void SetCanParry(bool value) { m_canParry = value; }
        bool CanDualWield() const { return m_canDualWield; }
        void SetCanDualWield(bool value) { m_canDualWield = value; }

        void _ApplyItemMods(Item *item,uint8 slot,bool apply);
        void _RemoveAllItemMods();
        void _ApplyAllItemMods();
        void _ApplyItemBonuses(ItemPrototype const *proto,uint8 slot,bool apply);
        void _ApplyAmmoBonuses(bool apply);
        bool EnchantmentFitsRequirements(uint32 enchantmentcondition, int8 slot);
        void ToggleMetaGemsActive(uint16 exceptslot, bool apply);
        void CorrectMetaGemEnchants(uint8 slot, bool apply);
        void InitDataForForm();

        void CastItemEquipSpell(Item *item);
        void CastItemCombatSpell(Item *item,Unit* Target);
        bool IsItemSpellToEquip(SpellEntry const *spellInfo);
        bool IsItemSpellToCombat(SpellEntry const *spellInfo);

        void SendInitWorldStates();
        void SendUpdateWorldState(uint32 Field, uint32 Value);
        void SendDirectMessage(WorldPacket *data);

        PlayerMenu* PlayerTalkClass;
        ItemsSetEffect * ItemsSetEff[3];
        void FlightComplete(void);
        void SendLoot(uint64 guid, LootType loot_type);
        void SendNotifyLootItemRemoved(uint8 lootSlot);
        void SendNotifyLootMoneyRemoved();
        int32 FishingMinSkillForCurrentZone() const;

        /*********************************************************/
        /***               BATTLEGROUND SYSTEM                 ***/
        /*********************************************************/

        bool InBattleGround() const { return m_bgBattleGroundID != 0; }
        bool InBattleGroundQueue() const { return m_bgBattleGroundQueueID != 0; }
        uint32 GetBattleGroundId() const { return m_bgBattleGroundID; }
        uint32 GetBattleGroundQueueId() const { return m_bgBattleGroundQueueID; }
        void SetBattleGroundId(uint8 val) { m_bgBattleGroundID = val; }
        void SetBattleGroundQueueId(uint8 val) { m_bgBattleGroundQueueID = val; }

        uint32 GetBattleGroundEntryPointMap() const { return m_bgEntryPointMap; }
        float GetBattleGroundEntryPointX() const { return m_bgEntryPointX; }
        float GetBattleGroundEntryPointY() const { return m_bgEntryPointY; }
        float GetBattleGroundEntryPointZ() const { return m_bgEntryPointZ; }
        float GetBattleGroundEntryPointO() const { return m_bgEntryPointO; }
        void SetBattleGroundEntryPoint(uint32 Map, float PosX, float PosY, float PosZ, float PosO )
        { 
            m_bgEntryPointMap = Map;
            m_bgEntryPointX = PosX;
            m_bgEntryPointY = PosY;
            m_bgEntryPointZ = PosZ;
            m_bgEntryPointO = PosO;
        }

        bool DropBattleGroundFlag();

        /*********************************************************/
        /***                    REST SYSTEM                    ***/
        /*********************************************************/

        bool isRested() const { return GetRestTime() >= 10000; }
        uint32 GetXPRestBonus(uint32 xp);
        uint32 GetRestTime() const { return m_restTime;};
        void SetRestTime(uint32 v) { m_restTime = v;};

        /*********************************************************/
        /***              ENVIROMENTAL SYSTEM                  ***/
        /*********************************************************/

        /*********************************************************/
        /***                 VARIOUS SYSTEMS                   ***/
        /*********************************************************/
        uint32 GetMovementFlags() const { return m_movement_flags; }
        bool HasMovementFlags(uint32 flags) const { return m_movement_flags & flags; }
        void SetMovementFlags(uint32 Flags) { m_movement_flags = Flags;}

        bool CanFly() const { return HasMovementFlags(MOVEMENTFLAG_CAN_FLY); }
        bool IsFlying() const { return HasMovementFlags(MOVEMENTFLAG_FLYING); }

        // Transports
        Transport * GetTransport() { return m_transport; }
        void SetTransport(Transport * t) { m_transport = t; }

        void SetTransOffset(float x, float y, float z, float orientation)
            { m_transX = x; m_transY = y; m_transZ = z; m_transO = orientation; }
        void GetTransOffset( float &x, float &y, float &z, float &o ) const
            { x = m_transX; y = m_transY; z = m_transZ; o = m_transO; }
        float GetTransOffsetX() const { return m_transX; }
        float GetTransOffsetY() const { return m_transY; }
        float GetTransOffsetZ() const { return m_transZ; }
        float GetTransOffsetO() const { return m_transO; }

        uint32 GetSaveTimer() const { return m_nextSave; }
        void   SetSaveTimer(uint32 timer) { m_nextSave = timer; }

        // Recall position
        uint32 m_recallMap;
        float  m_recallX;
        float  m_recallY;
        float  m_recallZ;
        float  m_recallO;
        void   SetRecallPosition(uint32 map, float x, float y, float z, float o);

        //homebind coordinates
        uint32 m_homebindMapId;
        uint16 m_homebindZoneId;
        float m_homebindX;
        float m_homebindY;
        float m_homebindZ;

        // Invisibility and detection system
        std::vector<Player *> InvisiblePjsNear;
        Player* m_DiscoveredPj;
        uint32 m_DetectInvTimer;
        void HandleInvisiblePjs();
        bool m_enableDetect;

        void ApplySpeedMod(UnitMoveType mtype, float rate, bool forced, bool apply);
                                                            // overwrite Unit version
        uint8 m_forced_speed_changes[MAX_MOVE_TYPE];

        bool isNeedRename() const { return m_needRename; }
        void SetNeedRename(bool rename) { m_needRename = rename; }

        LookingForGroup m_lookingForGroup;

        /*********************************************************/
        /***                 INSTANCE SYSTEM                   ***/
        /*********************************************************/

        void UpdateHomebindTime(uint32 time);

        bool m_Loaded;
        uint32 m_HomebindTimer;
        bool m_InstanceValid;
        BoundInstancesMap m_BoundInstances;

    protected:

        /*********************************************************/
        /***               BATTLEGROUND SYSTEM                 ***/
        /*********************************************************/

        uint8 m_bgBattleGroundID;
        uint8 m_bgBattleGroundQueueID;
        uint32 m_bgEntryPointMap;
        float m_bgEntryPointX;
        float m_bgEntryPointY;
        float m_bgEntryPointZ;
        float m_bgEntryPointO;

        /*********************************************************/
        /***                    QUEST SYSTEM                   ***/
        /*********************************************************/

        std::set<uint32> m_timedquests;

        uint64 m_divider;
        uint32 m_ingametime;

        /*********************************************************/
        /***                   LOAD SYSTEM                     ***/
        /*********************************************************/

        void _LoadActions();
        void _LoadAuras(uint32 timediff);
        void _LoadBoundInstances();
        void _LoadInventory(uint32 timediff);
        void _LoadMail();
        void _LoadMailedItems();
        void _LoadQuestStatus();
        void _LoadGroup();
        void _LoadReputation();
        void _LoadSpells(uint32 timediff);
        void _LoadTaxiMask(const char* data);
        void _LoadTutorials();
        void LoadIgnoreList();

        /*********************************************************/
        /***                   SAVE SYSTEM                     ***/
        /*********************************************************/

        void _SaveActions();
        void _SaveAuras();
        void _SaveBoundInstances();
        void _SaveInventory();
        void _SaveMail();
        void _SaveQuestStatus();
        void _SaveReputation();
        void _SaveSpells();
        void _SaveTutorials();

        void _SetCreateBits(UpdateMask *updateMask, Player *target) const;
        void _SetUpdateBits(UpdateMask *updateMask, Player *target) const;

        FactionsList::iterator FindReputationListIdInFactionList(uint32 repListId);

        /*********************************************************/
        /***              ENVIROMENTAL SYSTEM                  ***/
        /*********************************************************/
        void HandleDrowning (uint32 UnderWaterTime);
        void HandleLava();
        void HandleSobering();
        void StartMirrorTimer(MirrorTimerType Type, uint32 MaxValue);
        void ModifyMirrorTimer(MirrorTimerType Type, uint32 MaxValue, uint32 CurrentValue, uint32 Regen);
        void StopMirrorTimer(MirrorTimerType Type);
        void EnvironmentalDamage(uint64 Guid, uint8 Type, uint32 Amount);
        uint8 m_isunderwater;
        bool m_isInWater;

        void outDebugValues() const;
        bool _removeSpell(uint16 spell_id);

        uint64 m_lootGuid;

        uint32 m_race;
        uint32 m_class;
        uint32 m_team;
        uint32 m_dismountCost;
        uint32 m_nextSave;
        uint32 m_dungeonDifficulty;
        bool m_needRename;

        Item* m_items[PLAYER_SLOTS_COUNT];
        uint32 m_currentBuybackSlot;

        std::vector<Item*> m_itemUpdateQueue;
        bool m_itemUpdateQueueBlocked;

        uint32 m_movement_flags;

        uint32 m_GMFlags;
        uint64 m_curTarget;
        uint64 m_curSelection;

        QuestStatusMap mQuestStatus;

        uint32 m_GuildIdInvited;

        PlayerMails m_mail;
        PlayerSpellMap m_spells;
        SpellCooldowns m_spellCooldowns;

        ActionButtonList m_actionButtons;

        SpellModList m_spellMods[32];                       // 32 = SPELLMOD_COUNT
        EnchantDurationList m_enchantDuration;

        uint64 m_resurrectGUID;
        float m_resurrectX, m_resurrectY, m_resurrectZ;
        uint32 m_resurrectHealth, m_resurrectMana;

        WorldSession *m_session;

        std::list<Channel*> m_channels;

        bool m_dontMove;

        TaxiMask m_taximask;
        std::deque<uint32> m_TaxiDestinations;

        int m_cinematic;

        Player *pTrader;
        bool acceptTrade;
        uint16 tradeItems[TRADE_SLOT_COUNT];
        uint32 tradeGold;

        time_t m_nextThinkTime;
        uint32 m_Tutorials[8];
        uint32 m_regenTimer;
        uint32 m_breathTimer;
        uint32 m_drunkTimer;
        uint16 m_drunk;
        time_t m_lastManaUse;
        uint32 m_weaponChangeTimer;

        uint32 m_deathTimer;
        time_t m_resurrectingSicknessExpire;

        uint32 m_restTime;

        uint32 m_WeaponProficiency;
        uint32 m_ArmorProficiency;
        bool m_canParry;
        bool m_canDualWield;
        ////////////////////Rest System/////////////////////
        int time_inn_enter;
        float inn_pos_x;
        float inn_pos_y;
        float inn_pos_z;
        float m_rest_bonus;
        int rest_type;
        ////////////////////Rest System/////////////////////

        // Transports
        float m_transX;
        float m_transY;
        float m_transZ;
        float m_transO;

        Transport * m_transport;

        uint32 m_resetTalentsCost;
        time_t m_resetTalentsTime;
        uint32 m_usedTalentCount;

        IgnoreList m_ignorelist;
};

void AddItemsSetItem(Player*player,Item *item);
void RemoveItemsSetItem(Player*player,ItemPrototype const *proto);

// "the bodies of template functions must be made available in a header file"
template <class T> T Player::ApplySpellMod(uint32 spellId, uint8 op, T &basevalue)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo) return 0;
    int32 totalpct = 0;
    int32 totalflat = 0;
    bool remove = false;
    for (SpellModList::iterator itr = m_spellMods[op].begin(); itr != m_spellMods[op].end(); ++itr)
    {
        SpellModifier *mod = *itr;

        if(!IsAffectedBySpellmod(spellInfo,mod))
            continue;

        if (mod->type == SPELLMOD_FLAT)
            totalflat += mod->value;
        else if (mod->type == SPELLMOD_PCT)
            totalpct += mod->value;
        if (mod->charges > 0)
        {
            mod->charges--;
            if (mod->charges == 0)
            {
                mod->charges = -1;
                remove = true;
            }
        }
    }

    if (remove)
    {
        for (SpellModList::iterator itr = m_spellMods[op].begin(), next; itr != m_spellMods[op].end(); itr = next)
        {
            next = itr;
            next++;
            SpellModifier *mod = *itr;
            if (!mod) continue;
            if (mod->charges == -1)
            {
                RemoveAurasDueToSpell(mod->spellId);
                if (m_spellMods[op].empty())
                    break;
                else
                    next = m_spellMods[op].begin();
            }
        }
    }

    float diff = (float)basevalue*(float)totalpct/100.0f + (float)totalflat;
    basevalue = T((float)basevalue + diff);
    return T(diff);
}
#endif
