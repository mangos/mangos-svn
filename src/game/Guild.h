/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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

#ifndef MANGOSSERVER_GUILD_H
#define MANGOSSERVER_GUILD_H

#define WITHDRAW_MONEY_UNLIMITED    0xFFFFFFFF
#define WITHDRAW_SLOT_UNLIMITED     0xFFFFFFFF

#include "Item.h"

enum GuildDefaultRanks
{
    GR_GUILDMASTER  = 0,
    GR_OFFICER      = 1,
    //GR_VETERAN      = 2, -- not used anywhere and possible incorrect in modified rank list
    //GR_MEMBER       = 3,
    //GR_INITIATE     = 4, -- use Guild::GetLowestRank() instead for lowest rank
};

enum GuildRankRights
{
    GR_RIGHT_EMPTY              = 0x00000040,
    GR_RIGHT_GCHATLISTEN        = 0x00000041,
    GR_RIGHT_GCHATSPEAK         = 0x00000042,
    GR_RIGHT_OFFCHATLISTEN      = 0x00000044,
    GR_RIGHT_OFFCHATSPEAK       = 0x00000048,
    GR_RIGHT_PROMOTE            = 0x000000C0,
    GR_RIGHT_DEMOTE             = 0x00000140,
    GR_RIGHT_INVITE             = 0x00000050,
    GR_RIGHT_REMOVE             = 0x00000060,
    GR_RIGHT_SETMOTD            = 0x00001040,
    GR_RIGHT_EPNOTE             = 0x00002040,
    GR_RIGHT_VIEWOFFNOTE        = 0x00004040,
    GR_RIGHT_EOFFNOTE           = 0x00008040,
    GR_RIGHT_MODIFY_GUILD_INFO  = 0x00010040,
    GR_RIGHT_REPAIR_FROM_GUILD  = 0x00020000,               // Remove money withdraw capacity
    GR_RIGHT_ALL                = 0x0001F1FF
};

enum Typecommand
{
    GUILD_CREATE_S  = 0x00,
    GUILD_INVITE_S  = 0x01,
    GUILD_QUIT_S    = 0x03,
    GUILD_FOUNDER_S = 0x0E,
    GUILD_UNK1      = 0x10,
    GUILD_UNK2      = 0x15,                                 // guild bank?
    GUILD_UNK3      = 0x16
};

enum CommandErrors
{
    GUILD_PLAYER_NO_MORE_IN_GUILD   = 0x00,
    GUILD_INTERNAL                  = 0x01,
    GUILD_ALREADY_IN_GUILD          = 0x02,
    ALREADY_IN_GUILD                = 0x03,
    INVITED_TO_GUILD                = 0x04,
    ALREADY_INVITED_TO_GUILD        = 0x05,
    GUILD_NAME_INVALID              = 0x06,
    GUILD_NAME_EXISTS               = 0x07,
    GUILD_LEADER_LEAVE              = 0x08,
    GUILD_PERMISSIONS               = 0x08,
    GUILD_PLAYER_NOT_IN_GUILD       = 0x09,
    GUILD_PLAYER_NOT_IN_GUILD_S     = 0x0A,
    GUILD_PLAYER_NOT_FOUND          = 0x0B,
    GUILD_NOT_ALLIED                = 0x0C,
    GUILD_RANK_TOO_HIGH_S           = 0x0D,
    GUILD_ALREADY_LOWEST_RANK_S     = 0x0E,
    GUILD_TEMP_ERROR                = 0x11,
    GUILD_RANK_IN_USE               = 0x12,
    GUILD_IGNORE                    = 0x13,
    GUILD_ERR_UNK1                  = 0x17,
    GUILD_WITHDRAW_TOO_MUCH         = 0x18,
    GUILD_BANK_NO_MONEY             = 0x19,
    GUILD_BANK_TAB_IS_FULL          = 0x1B,
    GUILD_BANK_ITEM_NOT_FOUND       = 0x1C
};

enum GuildEvents
{
    GE_PROMOTION        = 0x00,
    GE_DEMOTION         = 0x01,
    GE_MOTD             = 0x02,
    GE_JOINED           = 0x03,
    GE_LEFT             = 0x04,
    GE_REMOVED          = 0x05,
    GE_LEADER_IS        = 0x06,
    GE_LEADER_CHANGED   = 0x07,
    GE_DISBANDED        = 0x08,
    GE_TABARDCHANGE     = 0x09,
    GE_UNK1             = 0x0A,                             // string, string
    GE_UNK2             = 0x0B,
    GE_SIGNED_ON        = 0x0C,
    GE_SIGNED_OFF       = 0x0D,
    GE_UNK3             = 0x0E,
    GE_BANKTAB_PURCHASED= 0x0F,
    GE_UNK5             = 0x10,
    GE_UNK6             = 0x11,                             // string 0000000000002710 is 1 gold
    GE_UNK7             = 0x12
};

enum PetitionTurns
{
    PETITION_TURN_OK                    = 0,
    PETITION_TURN_ALREADY_IN_GUILD      = 2,
    PETITION_TURN_NEED_MORE_SIGNATURES  = 4,
};

enum PetitionSigns
{
    PETITION_SIGN_OK                = 0,
    PETITION_SIGN_ALREADY_SIGNED    = 1,
    PETITION_SIGN_ALREADY_IN_GUILD  = 2,
    PETITION_SIGN_CANT_SIGN_OWN     = 3,
    PETITION_SIGN_NOT_SERVER        = 4,
};

enum GuildBankRights
{
    GUILD_BANK_RIGHT_VIEW_TAB       = 0x01,
    GUILD_BANK_RIGHT_PUT_ITEM       = 0x02,

    GUILD_BANK_RIGHT_DEPOSIT_ITEM   = GUILD_BANK_RIGHT_VIEW_TAB | GUILD_BANK_RIGHT_PUT_ITEM,
    GUILD_BANK_RIGHT_FULL           = 0xFF,
};

enum GuildBankLogEntries
{
    GUILD_BANK_LOG_DEPOSIT_ITEM     = 1,
    GUILD_BANK_LOG_WITHDRAW_ITEM    = 2,
    GUILD_BANK_LOG_MOVE_ITEM        = 3,
    GUILD_BANK_LOG_DEPOSIT_MONEY    = 4,
    GUILD_BANK_LOG_WITHDRAW_MONEY   = 5,
    GUILD_BANK_LOG_REPAIR_MONEY     = 6,
    GUILD_BANK_LOG_MOVE_ITEM2       = 7,
};

struct GuildBankEvent
{
    uint32 LogGuid;
    uint8  LogEntry;
    uint8  TabId;
    uint32 PlayerGuid;
    uint32 ItemOrMoney;
    uint8  ItemStackCount;
    uint8  DestTabId;
    uint64 TimeStamp;
};

struct GuildBankTab
{
    Item* Slots[GUILD_BANK_MAX_SLOTS];
    std::string Name;
    std::string Icon;
};

struct MemberSlot
{
    uint64 logout_time;
    std::string name; 
    std::string Pnote;
    std::string OFFnote;
    uint32 RankId;
    uint32 zoneId;
    uint8 level;
    uint8 Class;
    uint32 BankResetTimeMoney;
    uint32 BankRemMoney;
    uint32 BankResetTimeTab[GUILD_BANK_MAX_TABS];
    uint32 BankRemSlotsTab[GUILD_BANK_MAX_TABS];
};

struct RankInfo
{
    RankInfo(std::string _name, uint32 _rights, uint32 _money) : name(_name), rights(_rights), BankMoneyPerDay(_money) {}

    std::string name;
    uint32 rights;
    uint32 BankMoneyPerDay;
    uint32 TabRight[GUILD_BANK_MAX_TABS];
    uint32 TabSlotPerDay[GUILD_BANK_MAX_TABS];
};

class Guild
{
    public:
        Guild();
        ~Guild();

        bool create(uint64 lGuid, std::string gname);
        void Disband();

        typedef std::map<uint32, MemberSlot> MemberList;
        typedef std::vector<RankInfo> RankList;

        uint32 GetId(){ return Id; }
        const uint64& GetLeader(){ return leaderGuid; }
        std::string GetName(){ return name; }
        std::string GetMOTD(){ return MOTD; }
        std::string GetGINFO(){ return GINFO; }

        uint32 GetCreatedYear(){ return CreatedYear; }
        uint32 GetCreatedMonth(){ return CreatedMonth; }
        uint32 GetCreatedDay(){ return CreatedDay; }

        uint32 GetEmblemStyle(){ return EmblemStyle; }
        uint32 GetEmblemColor(){ return EmblemColor; }
        uint32 GetBorderStyle(){ return BorderStyle; }
        uint32 GetBorderColor(){ return BorderColor; }
        uint32 GetBackgroundColor(){ return BackgroundColor; }

        void SetLeader(uint64 guid);
        bool AddMember(uint64 plGuid, uint32 plRank);
        void ChangeRank(uint64 guid, uint32 newRank);
        void DelMember(uint64 guid, bool isDisbanding=false);
        uint32 GetLowestRank() const { return GetNrRanks()-1; }

        void SetMOTD(std::string motd);
        void SetGINFO(std::string ginfo);
        void SetPNOTE(uint64 guid,std::string pnote);
        void SetOFFNOTE(uint64 guid,std::string offnote);
        void SetEmblem(uint32 emblemStyle, uint32 emblemColor, uint32 borderStyle, uint32 borderColor, uint32 backgroundColor);

        uint32 GetMemberSize() const { return members.size(); }

        bool LoadGuildFromDB(uint32 GuildId);
        bool LoadRanksFromDB(uint32 GuildId);
        bool LoadMembersFromDB(uint32 GuildId);

        bool FillPlayerData(uint64 guid, MemberSlot* memslot);
        void LoadPlayerStatsByGuid(uint64 guid);

        void BroadcastToGuild(WorldSession *session, std::string msg, uint32 language = LANG_UNIVERSAL);
        void BroadcastToOfficers(WorldSession *session, std::string msg, uint32 language = LANG_UNIVERSAL);
        void BroadcastPacketToRank(WorldPacket *packet, uint32 rankId);
        void BroadcastPacket(WorldPacket *packet);

        void CreateRank(std::string name,uint32 rights);
        void DelRank();
        std::string GetRankName(uint32 rankId);
        uint32 GetRankRights(uint32 rankId);
        uint32 GetNrRanks() const { return m_ranks.size(); }

        void SetRankName(uint32 rankId, std::string name);
        void SetRankRights(uint32 rankId, uint32 rights);
        bool HasRankRight(uint32 rankId, uint32 right)
        {
            return ((GetRankRights(rankId) & right) != GR_RIGHT_EMPTY) ? true : false;
        }

        void Roster(WorldSession *session);
        void Query(WorldSession *session);

        void UpdateLogoutTime(uint64 guid);

        // ** Guild bank **
        // Content & item deposit/withdraw
        void   DisplayGuildBankContent(WorldSession *session, uint8 TabId);
        Item*  StoreItem(uint8 TabId, uint8* SlotId, Item* pItem);
        Item*  GetItem(uint8 TabId, uint8 SlotId);
        void   EmptyBankSlot(uint8 TabId, uint8 SlotId);
        // Tabs
        void   DisplayGuildBankTabsInfo(WorldSession *session);
        void   CreateNewBankTab();
        void   SetGuildBankTabInfo(uint8 TabId, std::string name, std::string icon);
        void   CreateBankRightForTab(uint32 rankid, uint8 TabId);
        const  GuildBankTab *GetBankTab(uint8 index) { if(index >= m_TabListMap.size()) return NULL; return m_TabListMap[index]; }
        const  uint8 GetPurchasedTabs() const { return purchased_tabs; }
        uint8  GetBankRights(uint32 rankId, uint8 TabId) const;
        bool   CanMemberDepositTo(uint32 LowGuid, uint8 TabId) const;
        // Load/unload
        void   LoadGuildBankFromDB();
        void   UnloadGuildBank();
        void   IncOnlineMemberCount() { m_onlinemembers++; }
        // Money deposit/withdraw
        void   SendMoneyInfo(WorldSession *session, uint32 LowGuid);
        bool   MemberMoneyWithdraw(uint32 amount, uint32 LowGuid);
        uint64 GetGuildBankMoney() { return guildbank_money; }
        void   SetBankMoney(int64 money);
        // per days
        bool   MemberItemWithdraw(uint8 TabId, uint32 LowGuid);
        uint32 GetMemberSlotWithdrawRem(uint32 LowGuid, uint8 TabId);
        uint32 GetMemberMoneyWithdrawRem(uint32 LowGuid);
        void   SetBankMoneyPerDay(uint32 rankId, uint32 money);
        void   SetBankRightsAndSlots(uint32 rankId, uint8 TabId, uint32 right, uint32 SlotPerDay, bool db);
        uint32 GetBankMoneyPerDay(uint32 rankId);
        uint32 GetBankSlotPerDay(uint32 rankId, uint8 TabId);
        // rights per day
        void   LoadBankRightsFromDB(uint32 GuildId);
        // logs
        void   LoadGuildBankEventLogFromDB();
        void   UnloadGuildBankEventLog();
        void   DisplayGuildBankLogs(WorldSession *session, uint8 TabId);
        void   LogBankEvent(uint8 LogEntry, uint8 TabId, uint32 PlayerGuidLow, uint32 ItemOrMoney, uint8 ItemStackCount=0, uint8 DestTabId=0);
        void   RenumBankLogs();

    protected:
        void AddRank(std::string name,uint32 rights,uint32 money);

        uint32 Id;
        std::string name;
        uint64 leaderGuid;
        std::string MOTD;
        std::string GINFO;
        uint32 CreatedYear;
        uint32 CreatedMonth;
        uint32 CreatedDay;

        uint32 EmblemStyle;
        uint32 EmblemColor;
        uint32 BorderStyle;
        uint32 BorderColor;
        uint32 BackgroundColor;

        RankList m_ranks;
        uint32 maxrank;

        MemberList members;

        typedef std::vector<GuildBankTab*> TabListMap;
        TabListMap m_TabListMap;
        typedef std::list<GuildBankEvent*> GuildBankEventLog;
        GuildBankEventLog m_GuildBankEventLog_Money;
        GuildBankEventLog m_GuildBankEventLog_Item[GUILD_BANK_MAX_TABS];

        bool m_bankloaded;
        uint32 m_onlinemembers;
        uint64 guildbank_money;
        uint8 purchased_tabs;

        uint32 LogMaxGuid;
};
#endif
