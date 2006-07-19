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

#ifndef _PLAYER_H
#define _PLAYER_H

#include "Common.h"
#include "ItemPrototype.h"
#include "Unit.h"
#include "Item.h"

#include "Database/DatabaseEnv.h"
#include "NPCHandler.h"
#include "QuestDef.h"
#include "Bag.h"
#include "Weather.h"

struct Mail;
class Channel;
class DynamicObject;
class Creature;
class PlayerMenu;

enum Team
{
    ALLIANCE = 469,
    HORDE = 67,
    ALLIANCE_FORCES = 891,
    HORDE_FORCES = 892,
    STEAMWHEEDLE_CARTEL = 169,
    //Duel System
    BLUE_TEAM = 1621,
    RED_TEAM = 1622,
};

enum Classes
{
    WARRIOR = 1,
    PALADIN = 2,
    HUNTER = 3,
    ROGUE = 4,
    PRIEST = 5,
    SHAMAN = 7,
    MAGE = 8,
    WARLOCK = 9,
    DRUID = 11,
};

enum Races
{
    HUMAN = 1,
    ORC = 2,
    DWARF = 3,
    NIGHTELF = 4,
    // if it needs be to official, it's actually SCOURGE acording to the story/.dbc
    UNDEAD_PLAYER = 5,
    TAUREN = 6,
    GNOME = 7,
    TROLL = 8,
    // officialy, this exists but was never taken into use.. neutral faction which could
    // learn some skills/spells from horde/alliance. maybe it'll be of some use later on.
    GOBLIN = 9,
};

struct PlayerSpell
{
    uint16 spellId;
    uint16 slotId;
    uint8 active;
};

typedef std::list<PlayerSpell*> PlayerSpellList;

struct actions
{
    uint8 button;
    uint8 type;
    uint8 misc;
    uint16 action;
};

struct PlayerCreateInfo
{
    uint8 createId;
    uint8 race;
    uint8 class_;
    uint32 mapId;
    uint32 zoneId;
    float positionX;
    float positionY;
    float positionZ;
    uint16 displayId;
    uint8 strength;
    uint8 ability;
    uint8 stamina;
    uint8 intellect;
    uint8 spirit;
    uint32 basearmor;
    uint32 health;
    uint32 mana;
    uint32 rage;
    uint32 focus;
    uint32 energy;
    uint32 attackpower;
    float mindmg;
    float maxdmg;
    float ranmindmg;
    float ranmaxdmg;
    std::list<uint32> item_id;
    std::list<uint8> item_bagIndex;
    std::list<uint8> item_slot;
    std::list<uint32> item_amount;
    std::list<uint16> spell;
    std::list<uint16> skill[3];
    std::list<uint16> action[4];
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

struct Factions
{
    uint32 ID;
    uint32 ReputationListID;
    uint32 Flags;
    uint32 Standing;
};

enum PlayerMovementType
{
    MOVE_ROOT       = 1,
    MOVE_UNROOT     = 2,
    MOVE_WATER_WALK = 3,
    MOVE_LAND_WALK  = 4,
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
    PLAYER_STATE_KNEEL            = 8
};

enum TYPE_OF_KILL
{
    HONORABLE_KILL = 1,
    DISHONORABLE_KILL = 2,
};

#define IS_BACK_SLOT(s) (s == 0xFF)

class Quest;
class Spell;
class Item;
class WorldSession;

#define EQUIPMENT_SLOT_START         0
#define EQUIPMENT_SLOT_HEAD          0
#define EQUIPMENT_SLOT_NECK          1
#define EQUIPMENT_SLOT_SHOULDERS     2
#define EQUIPMENT_SLOT_BODY          3
#define EQUIPMENT_SLOT_CHEST         4
#define EQUIPMENT_SLOT_WAIST         5
#define EQUIPMENT_SLOT_LEGS          6
#define EQUIPMENT_SLOT_FEET          7
#define EQUIPMENT_SLOT_WRISTS        8
#define EQUIPMENT_SLOT_HANDS         9
#define EQUIPMENT_SLOT_FINGER1       10
#define EQUIPMENT_SLOT_FINGER2       11
#define EQUIPMENT_SLOT_TRINKET1      12
#define EQUIPMENT_SLOT_TRINKET2      13
#define EQUIPMENT_SLOT_BACK          14
#define EQUIPMENT_SLOT_MAINHAND      15
#define EQUIPMENT_SLOT_OFFHAND       16
#define EQUIPMENT_SLOT_RANGED        17
#define EQUIPMENT_SLOT_TABARD        18
#define EQUIPMENT_SLOT_END           19

#define INVENTORY_SLOT_BAG_0         255
#define INVENTORY_SLOT_BAG_START     19
#define INVENTORY_SLOT_BAG_1         19
#define INVENTORY_SLOT_BAG_2         20
#define INVENTORY_SLOT_BAG_3         21
#define INVENTORY_SLOT_BAG_4         22
#define INVENTORY_SLOT_BAG_END       23

#define INVENTORY_SLOT_ITEM_START    23
#define INVENTORY_SLOT_ITEM_1        23
#define INVENTORY_SLOT_ITEM_2        24
#define INVENTORY_SLOT_ITEM_3        25
#define INVENTORY_SLOT_ITEM_4        26
#define INVENTORY_SLOT_ITEM_5        27
#define INVENTORY_SLOT_ITEM_6        28
#define INVENTORY_SLOT_ITEM_7        29
#define INVENTORY_SLOT_ITEM_8        30
#define INVENTORY_SLOT_ITEM_9        31
#define INVENTORY_SLOT_ITEM_10       32
#define INVENTORY_SLOT_ITEM_11       33
#define INVENTORY_SLOT_ITEM_12       34
#define INVENTORY_SLOT_ITEM_13       35
#define INVENTORY_SLOT_ITEM_14       36
#define INVENTORY_SLOT_ITEM_15       37
#define INVENTORY_SLOT_ITEM_16       38
#define INVENTORY_SLOT_ITEM_END      39

#define BANK_SLOT_ITEM_START         39
#define BANK_SLOT_ITEM_1             39
#define BANK_SLOT_ITEM_2             40
#define BANK_SLOT_ITEM_3             41
#define BANK_SLOT_ITEM_4             42
#define BANK_SLOT_ITEM_5             43
#define BANK_SLOT_ITEM_6             44
#define BANK_SLOT_ITEM_7             45
#define BANK_SLOT_ITEM_8             46
#define BANK_SLOT_ITEM_9             47
#define BANK_SLOT_ITEM_10            48
#define BANK_SLOT_ITEM_11            49
#define BANK_SLOT_ITEM_12            50
#define BANK_SLOT_ITEM_13            51
#define BANK_SLOT_ITEM_14            52
#define BANK_SLOT_ITEM_15            53
#define BANK_SLOT_ITEM_16            54
#define BANK_SLOT_ITEM_17            55
#define BANK_SLOT_ITEM_18            56
#define BANK_SLOT_ITEM_19            57
#define BANK_SLOT_ITEM_20            58
#define BANK_SLOT_ITEM_21            59
#define BANK_SLOT_ITEM_22            60
#define BANK_SLOT_ITEM_23            61
#define BANK_SLOT_ITEM_24            62
#define BANK_SLOT_ITEM_END           63

#define BANK_SLOT_BAG_START          63
#define BANK_SLOT_BAG_1              63
#define BANK_SLOT_BAG_2              64
#define BANK_SLOT_BAG_3              65
#define BANK_SLOT_BAG_4              66
#define BANK_SLOT_BAG_5              67
#define BANK_SLOT_BAG_6              68
#define BANK_SLOT_BAG_END            69

#define BUYBACK_SLOT_START           69
#define BUYBACK_SLOT_1               69
#define BUYBACK_SLOT_2               70
#define BUYBACK_SLOT_3               71
#define BUYBACK_SLOT_4               72
#define BUYBACK_SLOT_5               73
#define BUYBACK_SLOT_6               74
#define BUYBACK_SLOT_7               75
#define BUYBACK_SLOT_8               76
#define BUYBACK_SLOT_9               77
#define BUYBACK_SLOT_10              78
#define BUYBACK_SLOT_11              79
#define BUYBACK_SLOT_12              80
#define BUYBACK_SLOT_END             81

class MANGOS_DLL_SPEC Player : public Unit
{
    friend class WorldSession;
    public:
        Player (WorldSession *session);
        ~Player ( );

        void AddToWorld();
        void RemoveFromWorld();

        void TeleportTo(uint32 mapid, float x, float y, float z, float orientation);

        bool Create ( uint32 guidlow, WorldPacket &data );

        void Update( uint32 time );

        void BuildEnumData( WorldPacket * p_data );

        uint8 ToggleAFK() { m_afk = !m_afk; return m_afk; };
        bool isAcceptTickets() const;
        void SetAcceptTicket(bool on) { m_acceptTicket = on; }

        const char* GetName() { return m_name.c_str(); };
        PlayerCreateInfo* GetPlayerInfo(){return info;}

        void GiveXP(uint32 xp, Unit* victim);
        void GiveLevel();

        void BuildLvlUpStats(uint32 *STR,uint32 *STA,uint32 *AGI,uint32 *INT,uint32 *SPI);

        void setDismountCost(uint32 money) { m_dismountCost = money; };

        void setDeathState(DeathState s)
        {
            bool cur = isAlive();
            Unit::setDeathState(s);

            if(s == JUST_DIED && cur)
            {
                _RemoveAllItemMods();
                UnsummonPet(true);
            }
            if(isAlive() && !cur)
            {
                _ApplyAllItemMods();
            }
        };

        void UnsummonPet(bool remove);

        /*********************************************************/
        /***                    STORAGE SYSTEM                 ***/
        /*********************************************************/

        void SetSheath( uint32 sheathed );
        uint8 FindEquipSlot( uint32 type, uint32 slot, bool swap ) const;
        Item* CreateItem( uint32 item, uint32 count ) const;
        uint32 GetItemCount( uint32 item ) const;
        uint32 GetBankItemCount( uint32 item ) const;
        uint16 GetPosByGuid( uint64 guid ) const;
        Item* GetItemByPos( uint16 pos ) const;
        Item* GetItemByPos( uint8 bag, uint8 slot ) const;
        bool HasBankBagSlot( uint8 slot ) const;
        bool IsInventoryPos( uint16 pos ) const;
        bool IsEquipmentPos( uint16 pos ) const;
        bool IsBankPos( uint16 pos ) const;
        bool HasItemCount( uint32 item, uint32 count ) const;
        uint8 CanStoreNewItem( uint8 bag, uint8 slot, uint16 &dest, uint32 item, uint32 count, bool swap ) const;
        uint8 CanStoreItem( uint8 bag, uint8 slot, uint16 &dest, Item *pItem, bool swap ) const;
        uint8 CanEquipItem( uint8 slot, uint16 &dest, Item *pItem, bool swap, bool check_alive = true ) const;
        uint8 CanBankItem( uint8 bag, uint8 slot, uint16 &dest, Item *pItem, bool swap ) const;
        uint8 CanUseItem( Item *pItem, bool check_alive = true ) const;
        uint8 CanUseAmmo( uint32 item ) const;
        void StoreNewItem( uint16 pos, uint32 item, uint32 count, bool update );
        void StoreItem( uint16 pos, Item *pItem, bool update );
        void EquipItem( uint16 pos, Item *pItem, bool update );
        void BankItem( uint16 pos, Item *pItem, bool update );
        void RemoveItem( uint8 bag, uint8 slot, bool update );
        void RemoveItemCount( uint32 item, uint32 count, bool update );
        void DestroyItem( uint8 bag, uint8 slot, bool update );
        void DestroyItemCount( uint32 item, uint32 count, bool update );
        void SplitItem( uint16 src, uint16 dst, uint32 count );
        void SwapItem( uint16 src, uint16 dst );
        void AddItemToBuyBackSlot( uint32 slot, Item *pItem );
        Item* GetItemFromBuyBackSlot( uint32 slot );
        void RemoveItemFromBuyBackSlot( uint32 slot );
        void SendEquipError( uint8 msg, Item* pItem, Item *pItem2 );
        void SendBuyError( uint8 msg, Creature* pCreature, uint32 item, uint32 param );
        void SendSellError( uint8 msg, Creature* pCreature, uint64 guid, uint32 param );

        /*********************************************************/
        /***                    QUEST SYSTEM                   ***/
        /*********************************************************/

        void PrepareQuestMenu( uint64 guid );
        void SendPreparedQuest( uint64 guid );
        Quest *GetActiveQuest( uint32 quest_id ) const;
        Quest *GetNextQuest( uint64 guid, Quest *pQuest );
        bool CanSeeStartQuest( Quest *pQuest );
        bool CanTakeQuest( Quest *pQuest, bool msg );
        bool CanAddQuest( Quest *pQuest, bool msg );
        bool CanCompleteQuest( Quest *pQuest );
        bool CanRewardQuest( Quest *pQuest, uint32 reward, bool msg );
        void AddQuest( Quest *pQuest );
        void CompleteQuest( Quest *pQuest );
        void IncompleteQuest( Quest *pQuest );
        void RewardQuest( Quest *pQuest, uint32 reward );
        void FailQuest( Quest *pQuest );
        void FailTimedQuest( Quest *pQuest );
        bool SatisfyQuestClass( Quest *pQuest, bool msg );
        bool SatisfyQuestLevel( Quest *pQuest, bool msg );
        bool SatisfyQuestLog( bool msg );
        bool SatisfyQuestPreviousQuest( Quest *pQuest, bool msg );
        bool SatisfyQuestRace( Quest *pQuest, bool msg );
        bool SatisfyQuestReputation( Quest *pQuest, bool msg );
        bool SatisfyQuestSkill( Quest *pQuest, bool msg );
        bool SatisfyQuestStatus( Quest *pQuest, bool msg );
        bool SatisfyQuestTimed( Quest *pQuest, bool msg );
        bool GiveQuestSourceItem( Quest *pQuest );
        void TakeQuestSourceItem( Quest *pQuest );
        bool GetQuestRewardStatus( Quest *pQuest );
        uint32 GetQuestStatus( Quest *pQuest );
        void SetQuestStatus( Quest *pQuest, uint32 status );
        void AdjustQuestReqItemCount( Quest *pQuest );
        uint16 GetQuestSlot( Quest *pQuest );
        void AreaExplored( Quest *pQuest );
        void ItemAdded( uint32 entry, uint32 count );
        void ItemRemoved( uint32 entry, uint32 count );
        void KilledMonster( uint32 entry, uint64 guid );
        bool HaveQuestForItem( uint32 itemid );

        void SendQuestComplete( Quest *pQuest );
        void SendQuestReward( Quest *pQuest );
        void SendQuestFailed( Quest *pQuest );
        void SendQuestTimerFailed( Quest *pQuest );
        void SendCanTakeQuestResponse( uint32 msg );
        void SendPushToPartyResponse( Player *pPlayer, uint32 msg );
        void SendQuestUpdateAddItem( Quest *pQuest, uint32 item_idx, uint32 count );
        void SendQuestUpdateAddKill( Quest *pQuest, uint64 guid, uint32 creature_idx, uint32 old_count, uint32 add_count );

        uint64 GetDivider() { return m_divider; };
        void SetDivider( uint64 guid ) { m_divider = guid; };

        uint32 GetInGameTime() { return m_ingametime; };

        void SetInGameTime( uint32 time ) { m_ingametime = time; };

        uint32 GetTimedQuest() { return m_timedquest; };
        void SetTimedQuest( Quest *pQuest )
        {
            if( pQuest )
                m_timedquest = pQuest->GetQuestInfo()->QuestId;
            else
                m_timedquest = 0;
        }

        /*********************************************************/
        /***                   LOAD SYSTEM                     ***/
        /*********************************************************/

        bool LoadFromDB(uint32 guid);

        /*********************************************************/
        /***                   SAVE SYSTEM                     ***/
        /*********************************************************/

        void SaveToDB();
        void SavePet();

        void SetBindPoint(uint64 guid);
        void CalcRage( uint32 damage,bool attacker );
        void RegenerateAll();
        void Regenerate(uint16 field_cur, uint16 field_max);
        void setRegenTimer(uint32 time) {m_regenTimer = time;}

        uint32 GetMoney() { return GetUInt32Value (PLAYER_FIELD_COINAGE); }
        void ModifyMoney( int32 d ) { SetMoney (GetMoney() + d); }
        void SetMoney( uint32 value ) { SetUInt32Value (PLAYER_FIELD_COINAGE, value); }

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

        void AddMail(Mail *m);

        std::map<uint32, struct quest_status> getQuestStatusMap() { return mQuestStatus; };

        const uint64& GetSelection( ) const { return m_curSelection; }
        const uint64& GetTarget( ) const { return m_curTarget; }

        void SetSelection(const uint64 &guid) { m_curSelection = guid; }
        void SetTarget(const uint64 &guid) { m_curTarget = guid; }

        uint32 GetMailSize() { return m_mail.size();};
        Mail* GetMail(uint32 id);
        void RemoveMail(uint32 id);
        std::list<Mail*>::iterator GetmailBegin() { return m_mail.begin();};
        std::list<Mail*>::iterator GetmailEnd() { return m_mail.end();};
        void AddBid(bidentry *be);
        bidentry* GetBid(uint32 id);
        std::list<bidentry*>::iterator GetBidBegin() { return m_bids.begin();};
        std::list<bidentry*>::iterator GetBidEnd() { return m_bids.end();};

        bool HasSpell(uint32 spell) const;
        void SendInitialSpells();
        void addSpell(uint16 spell_id,uint8 active,uint16 slot_id=0xffff);
        void learnSpell(uint16 spell_id);
        bool removeSpell(uint16 spell_id);
        void DealWithSpellDamage(DynamicObject &);
        PlayerSpellList const& getSpellList() { return m_spells; };
        void setResurrect(uint64 guid,float X, float Y, float Z, uint32 health, uint32 mana)
        {
            m_resurrectGUID = guid;
            m_resurrectX = X;
            m_resurrectY = Y;
            m_resurrectZ = Z;
            m_resurrectHealth = health;
            m_resurrectMana = mana;
        };
        Corpse* GetCorpse() const { return this->m_pCorpse; }

        int getCinematic()
        {
            return m_cinematic;
        }
        void setCinematic(int cine)
        {
            m_cinematic = cine;
        }

        void UpdatePVPFlag(time_t currTime);

        void SetPVPCount(time_t count)
        {
            if(!m_pvp_counting)
            {
                m_pvp_count = count;
                m_pvp_counting = true;
            }
        }

        void SetPvP(bool b)
        {
            pvpOn = b;

                                                            //PvP OFF
            if(!b) SetFlag(UNIT_FIELD_FLAGS , UNIT_FLAG_NOT_IN_PVP);
                                                            //PvP ON
            else  RemoveFlag(UNIT_FIELD_FLAGS , UNIT_FLAG_NOT_IN_PVP);

            m_pvp_counting = false;
        };

        bool GetPvP()
        {
            return pvpOn;
        };

        void setFactionForRace(uint8 race);

        inline std::list<struct actions> getActionList() { return m_actions; };
        void addAction(uint8 button, uint16 action, uint8 type, uint8 misc);
        void removeAction(uint8 button);
        void SendInitialActions();

        void SetInvited() { m_isInvited = true; }
        void SetInGroup() { m_isInGroup = true; }
        void SetLeader(const uint64 &guid) { m_groupLeader = guid; }

        int  IsInGroup() { return m_isInGroup; }
        int  IsInvited() { return m_isInvited; }
        const uint64& GetGroupLeader() const { return m_groupLeader; }

        void UnSetInvited() { m_isInvited = false; }
        void UnSetInGroup() { m_isInGroup = false; }

        void SetGuildIdInvited(uint32 GuildId) { m_GuildIdInvited = GuildId; }
        void SetInGuild(uint32 GuildId) { SetUInt32Value(PLAYER_GUILDID, GuildId);  }
        void SetRank(uint32 rankId){ SetUInt32Value(PLAYER_GUILDRANK, rankId); }

        uint32 GetGuildId() { return GetUInt32Value(PLAYER_GUILDID);  }
        uint32 GetRank(){ return GetUInt32Value(PLAYER_GUILDRANK); }
        int GetGuildIdInvited() { return m_GuildIdInvited; }

        void SetDuelVs(Player *plyr) { m_pDuel = plyr; }
        void SetInDuel(bool val) { m_isInDuel = val; }
        bool isInDuel() { return m_isInDuel; }
        void SetDuelSender(Player *plyr) { m_pDuelSender = plyr; }
        void CheckDuelDistance();

        //Functions to store/restore temporary state of pvpOn
        void StorePvpState(){ pvpTemp = pvpOn; };
        void RestorePvpState(){ SetPvP(pvpTemp); };

        uint32 GetCurrentBuybackSlot() { return m_currentBuybackSlot; }
        void SetCurrentBuybackSlot( uint32 slot )
        {
            if( slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END )
                m_currentBuybackSlot = slot;
            else
                m_currentBuybackSlot = BUYBACK_SLOT_START;
        }

        bool IsGroupMember(Player *plyr);

        void UpdateSkill(uint32 skill_id);
        void UpdateSkillPro(uint32 spellid);
        uint32 GetSpellByProto(ItemPrototype *proto);

        const uint64& GetLootGUID() const { return m_lootGuid; }
        void SetLootGUID(const uint64 &guid) { m_lootGuid = guid; }

        WorldSession* GetSession() const { return m_session; }
        void SetSession(WorldSession *s) { m_session = s; }

        void CreateYourself( );
        void DestroyYourself( );

        void BuildCreateUpdateBlockForPlayer( UpdateData *data, Player *target ) const;
        void DestroyForPlayer( Player *target ) const;
        void SendDelayResponse(const uint32);
        void SendLogXPGain(uint32 GivenXP,Unit* victim);

        void SendAttackStart(Unit* pVictim);

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

        bool SetPosition(const float &x, const float &y, const float &z, const float &orientation);
        void SendMessageToSet(WorldPacket *data, bool self);// overwrite Object::SendMessageToSet

        void DeleteFromDB();
        void DeleteCorpse();

        void SpawnCorpseBones();
        void CreateCorpse();
        void KillPlayer();
        void ResurrectPlayer();
        void BuildPlayerRepop();
        void DeathDurabilityLoss(double percent);
        void RepopAtGraveyard();
        void DuelComplete();

        void SetMovement(uint8 pType);
        void SetPlayerSpeed(uint8 SpeedType, float value, bool forced=false);

        void JoinedChannel(Channel *c);
        void LeftChannel(Channel *c);
        void CleanupChannels();

        void BroadcastToFriends(std::string msg);

        void UpdateDefense();
        void UpdateSkillWeapon();

        void SetSkill(uint32 id, uint16 currVal, uint16 maxVal);
        uint16 GetSkillValue(uint32 skill) const;
        bool HasSkill(uint32 skill) const;

        void SetDontMove(bool dontMove);
        bool GetDontMove() const { return m_dontMove; }

        void CheckExploreSystem(void);

        uint32 GetTeam() const { return m_team; }
        uint32 getLevel() const { return GetUInt32Value(UNIT_FIELD_LEVEL); }

        void SetLastManaUse(time_t spellCastTime) { m_lastManaUse = spellCastTime; }
        uint32 GetReputation(uint32 faction_id) const;
        bool SetStanding(uint32 faction, int standing);
        bool ModifyFactionReputation(FactionEntry* factionEntry, int32 standing);
        void CalculateReputation(Unit *pVictim);
        void CalculateReputation(Quest *pQuest, uint64 guid);
        void SetInitialFactions();
        void UpdateReputation() const;
        void SendSetFactionStanding(const Factions* faction) const;
        void UpdateMaxSkills();
        void UpdateSkillsToMaxSkillsForLevel();             // for .levelup
        void ModifySkillBonus(uint32 skillid,int32 val);

        /*********************************************************/
        /***                  HONOR SYSTEM                     ***/
        /*********************************************************/
        void UpdateHonor();
        void CalculateHonor(Unit *pVictim);
        uint32 CalculateHonorRank(float honor) const;
        uint32 GetHonorRank() const;
        int  CalculateTotalKills(Player *pVictim) const;
        //Acessors of total honor points
        void SetTotalHonor(float total_honor_points) { m_total_honor_points = total_honor_points; };
        float GetTotalHonor(void) const { return m_total_honor_points; };
        //Acessors of righest rank
        uint32 GetHonorHighestRank() const { return m_highest_rank; }
        void SetHonorHighestRank(uint32 hr) { m_highest_rank = hr; }
        //Acessors of rating
        float GetHonorRating() const { return m_rating; }
        void SetHonorRating(float rating) { m_rating = rating; }
        //Acessors of last week standing
        int  GetHonorLastWeekStanding() const { return m_standing; }
        void SetHonorLastWeekStanding(int standing){ m_standing = standing; }
        //End of Honor System

        void SetDrunkValue(uint16 newDrunkValue);
        uint16 GetDrunkValue() const { return m_drunk; }
        uint32 GetDeathTimer() const { return m_deathTimer; }

        void ApplyItemMods(Item *item,uint8 slot,bool apply)
        {
            _ApplyItemMods(item, slot, apply);
        };
        void _ApplyItemMods(Item *item,uint8 slot,bool apply);
        void _RemoveAllItemMods();
        void _ApplyAllItemMods();

        void CastItemEquipSpell(Item *item);
        void CastItemCombatSpell(Item *item,Unit* Target);
        bool IsItemSpellToEquip(SpellEntry *spellInfo);
        bool IsItemSpellToCombat(SpellEntry *spellInfo);

        void SendInitWorldStates(uint32 MapID);
        void SendUpdateWordState(uint16 Field, uint16 Value);
        void SendDirectMessage(WorldPacket *data);

        PlayerMenu* PlayerTalkClass;
        ItemsSetEffect * ItemsSetEff[3];
        void FlightComplete(void);
        void SendLoot(uint64 guid,uint8 loot_type);
        uint8 CheckFishingAble() const;

        inline bool InBattleGround() const { return m_bgInBattleGround; }
        inline void SetInBattleGround(bool val) { m_bgInBattleGround = val; }
        inline uint32 GetBattleGroundId() const { return m_bgBattleGroundID; }
        inline void SetBattleGroundId(uint8 val) { m_bgBattleGroundID = val; }

        uint32 m_bgTeam;

        bool isRested() const { return GetRestTime() >= 10000; }
        uint32 ApplyRestBonus(uint32 xp);
        uint32 GetRestTime() const { return m_restTime;}
        void SetRestTime(uint32 v) { m_restTime = v;}

        bool IsInWater() const { return (m_isunderwater & 0x80); }

        Player* GetTrader() const { return pTrader; }

    protected:
        bool m_bgInBattleGround;
        uint8 m_bgBattleGroundID;

        uint32 m_bgEntryPointMap;
        float m_bgEntryPointX;
        float m_bgEntryPointY;
        float m_bgEntryPointZ;
        float m_bgEntryPointO;

        /*********************************************************/
        /***                    QUEST SYSTEM                   ***/
        /*********************************************************/

        uint32 m_timedquest;
        uint64 m_divider;
        uint32 m_ingametime;

        /*********************************************************/
        /***                   LOAD SYSTEM                     ***/
        /*********************************************************/

        void _LoadActions();
        void _LoadAuras();
        void _LoadBids();
        void _LoadCorpse();
        void _LoadInventory();
        void _LoadMail();
        void _LoadQuestStatus();
        void _LoadPet();
        void _LoadReputation();
        void _LoadSpells();
        void _LoadTutorials();

        /*********************************************************/
        /***                   SAVE SYSTEM                     ***/
        /*********************************************************/

        void _SaveActions();
        void _SaveAuctions();
        void _SaveAuras();
        void _SaveBids();
        void _SaveInventory();
        void _SaveMail();
        void _SaveQuestStatus();
        void _SaveReputation();
        void _SaveSpells();
        void _SaveTutorials();

        void AddWeather();
        void _SetCreateBits(UpdateMask *updateMask, Player *target) const;
        void _SetUpdateBits(UpdateMask *updateMask, Player *target) const;
        void _SetVisibleBits(UpdateMask *updateMask, Player *target) const;

        bool FactionIsInTheList(uint32 faction);

        void HandleDrowing (uint32 UnderWaterTime);
        void HandleLava();
        void HandleSobering();

        void StartMirrorTimer(uint8 Type, uint32 MaxValue);
        void ModifyMirrorTimer(uint8 Type, uint32 MaxValue, uint32 CurrentValue, uint32 Regen);
        void StopMirrorTimer(uint8 Type);
        void EnvironmentalDamage(uint64 Guid, uint8 Type, uint32 Amount);

        void outDebugValues() const;

        uint64 m_lootGuid;

        std::string m_name;
        std::string m_rank_name;

        PlayerCreateInfo *info;

        uint32 m_race;
        uint32 m_class;
        uint32 m_team;
        uint8  m_outfitId;
        uint16 m_petInfoId;
        uint16 m_petLevel;
        uint16 m_petFamilyId;
        uint32 m_dismountCost;
        uint32 m_nextSave;

        Item* m_items[BUYBACK_SLOT_END];
        Item* m_buybackitems[BUYBACK_SLOT_END - BUYBACK_SLOT_START];

        uint8 m_afk;
        bool  m_acceptTicket;
        uint64 m_curTarget;
        uint64 m_curSelection;

        typedef std::map<uint32, struct quest_status> StatusMap;
        StatusMap mQuestStatus;

        uint64 m_groupLeader;
        bool m_isInGroup;
        bool m_isInvited;

        uint32 m_GuildIdInvited;

        uint32 m_currentBuybackSlot;

        std::list<struct Factions> factions;
        std::list<bidentry*> m_bids;
        std::list<Mail*> m_mail;
        PlayerSpellList m_spells;
        std::list<struct actions> m_actions;

        uint64 m_resurrectGUID;
        float m_resurrectX, m_resurrectY, m_resurrectZ;
        uint32 m_resurrectHealth, m_resurrectMana;

        Corpse *m_pCorpse;

        WorldSession *m_session;

        std::list<Channel*> m_channels;

        bool m_dontMove;

        float m_total_honor_points;
        float m_rating;
        uint32 m_highest_rank;
        int m_standing;

        int m_cinematic;

        Player *pTrader;
        bool acceptTrade;
        int tradeItems[7];
        uint32 tradeGold;

        bool pvpOn;
        bool pvpTemp;
        Player *m_pDuel;
        bool m_isInDuel;
        Player *m_pDuelSender;

        time_t m_nextThinkTime;
        uint32 m_Tutorials[8];
        uint32 m_regenTimer;
        uint32 m_breathTimer;
        uint32 m_drunkTimer;
        uint16 m_drunk;
        time_t m_lastManaUse;

        uint32 m_deathTimer;
        uint8 m_isunderwater;

        uint32 m_restTime;
        time_t m_pvp_count;
        bool m_pvp_counting;
};

int irand(int min, int max);

inline uint32 urand(uint32 min, uint32 max)
{
    return irand(int(min), int(max));
}

void AddItemsSetItem(Player*player,Item *item);
void RemoveItemsSetItem(Player*player,ItemPrototype *proto);
#endif
