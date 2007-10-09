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

#ifndef MANGOSSERVER_QUEST_H
#define MANGOSSERVER_QUEST_H

#include "Platform/Define.h"
#include "Database/DatabaseEnv.h"

#include <string>
#include <vector>

class Player;

class ObjectMgr;

// (PLAYER_VISIBLE_ITEM_1_CREATOR - PLAYER_QUEST_LOG_1_1)/3
#define MAX_QUEST_LOG_SIZE 25

#define QUEST_OBJECTIVES_COUNT 4
#define QUEST_SOURCE_ITEM_IDS_COUNT 4
#define QUEST_REWARD_CHOICES_COUNT 6
#define QUEST_REWARDS_COUNT 4
#define QUEST_DEPLINK_COUNT 10
#define QUEST_REPUTATIONS_COUNT 5
#define QUEST_EMOTE_COUNT 4

enum QuestFailedReasons
{
    INVALIDREASON_DONT_HAVE_REQ        = 0,
    INVALIDREASON_DONT_HAVE_REQLEVEL   = 1,
    INVALIDREASON_DONT_HAVE_RACE       = 6,
    INVALIDREASON_UNKNOWN7             = 7,                 //quest completed
    INVALIDREASON_HAVE_TIMED_QUEST     = 12,
    INVALIDREASON_HAVE_QUEST           = 13,
    INVALIDREASON_UNKNOWN16            = 16,                //need expantion account
    INVALIDREASON_DONT_HAVE_REQ_ITEMS  = 21,                //2.1.3
    INVALIDREASON_DONT_HAVE_REQ_MONEY  = 23,                //2.1.3
    INVALIDREASON_UNKNOWN26            = 26,                //"you have completed 10 daily quests today"
    INVALIDREASON_UNKNOWN27            = 27,                //"You cannot completed quests once you have reached tired time"
};

enum QuestShareMessages
{
    QUEST_PARTY_MSG_SHARING_QUEST   = 0,
    QUEST_PARTY_MSG_CANT_TAKE_QUEST = 1,
    QUEST_PARTY_MSG_ACCEPT_QUEST    = 2,
    QUEST_PARTY_MSG_REFUSE_QUEST    = 3,
    QUEST_PARTY_MSG_TOO_FAR         = 4,
    QUEST_PARTY_MSG_BUSY            = 5,
    QUEST_PARTY_MSG_LOG_FULL        = 6,
    QUEST_PARTY_MSG_HAVE_QUEST      = 7,
    QUEST_PARTY_MSG_FINISH_QUEST    = 8,
};

enum __QuestTradeSkill
{
    QUEST_TRSKILL_NONE           = 0,
    QUEST_TRSKILL_ALCHEMY        = 1,
    QUEST_TRSKILL_BLACKSMITHING  = 2,
    QUEST_TRSKILL_COOKING        = 3,
    QUEST_TRSKILL_ENCHANTING     = 4,
    QUEST_TRSKILL_ENGINEERING    = 5,
    QUEST_TRSKILL_FIRSTAID       = 6,
    QUEST_TRSKILL_HERBALISM      = 7,
    QUEST_TRSKILL_LEATHERWORKING = 8,
    QUEST_TRSKILL_POISONS        = 9,
    QUEST_TRSKILL_TAILORING      = 10,
    QUEST_TRSKILL_MINING         = 11,
    QUEST_TRSKILL_FISHING        = 12,
    QUEST_TRSKILL_SKINNING       = 13,
    QUEST_TRSKILL_JEWELCRAFTING  = 14,
};

enum QuestStatus
{
    QUEST_STATUS_NONE           = 0,
    QUEST_STATUS_COMPLETE       = 1,
    QUEST_STATUS_UNAVAILABLE    = 2,
    QUEST_STATUS_INCOMPLETE     = 3,
    QUEST_STATUS_AVAILABLE      = 4,
    MAX_QUEST_STATUS
};

enum __QuestGiverStatus
{
    DIALOG_STATUS_NONE                     = 0,
    DIALOG_STATUS_UNAVAILABLE              = 1,
    DIALOG_STATUS_CHAT                     = 2,
    DIALOG_STATUS_INCOMPLETE               = 3,
    DIALOG_STATUS_REWARD_REP               = 4,
    DIALOG_STATUS_AVAILABLE                = 5,
    DIALOG_STATUS_REWARD_OLD               = 6,             // red dot on minimap
    DIALOG_STATUS_REWARD                   = 7,             // yellow dot on minimap
};

enum __QuestSpecialFlags                                    //according to mangos-db-11-02-2006-for_1_9_x;
{
    QUEST_SPECIAL_FLAGS_NONE          = 0,
    QUEST_SPECIAL_FLAGS_DELIVER       = 1,
    QUEST_SPECIAL_FLAGS_EXPLORATION   = 2,
    QUEST_SPECIAL_FLAGS_SPEAKTO       = 4,
    QUEST_SPECIAL_FLAGS_KILL_OR_CAST  = 8,
    QUEST_SPECIAL_FLAGS_TIMED         = 16,
    //QUEST_SPECIAL_FLAGS_REPEATABLE    = 32,               // meaning of flag 32 unknown
    QUEST_SPECIAL_FLAGS_REPUTATION    = 64,
    //QUEST_SPECIAL_FLAGS_UNK1          = 128,              // unknown tbc
    //QUEST_SPECIAL_FLAGS_UNK2          = 256,              // unknown tbc, bring items?
    //QUEST_SPECIAL_FLAGS_UNK3          = 512,              // unknown tbc
    //QUEST_SPECIAL_FLAGS_UNK4          = 1024,             // unknown tbc
    //QUEST_SPECIAL_FLAGS_UNK5          = 2048,             // unknown tbc
};

// This Quest class provides a convenient way to access a few pretotaled (cached) quest details,
// all base quest information, and any utility functions such as generating the amount of
// xp to give
class Quest
{
    friend class ObjectMgr;
    public:
        Quest(Field * questRecord);
        uint32 XPValue( Player *pPlayer ) const;

        bool HasSpecialFlag( uint32 flag ) const { return (SpecialFlags & flag ) != 0; }

        // table data accessors:
        uint32 GetQuestId() const { return QuestId; }
        int32  GetZoneOrSort() const { return ZoneOrSort; }
        uint32 GetMinLevel() const { return MinLevel; }
        uint32 GetQuestLevel() const { return QuestLevel; }
        uint32 GetType() const { return Type; }
        uint32 GetRequiredRaces() const { return RequiredRaces; }
        uint32 GetRequiredSkillValue() const { return RequiredSkillValue; }
        uint32 GetRequiredMinRepFaction() const { return RequiredMinRepFaction; }
         int32 GetRequiredMinRepValue() const { return RequiredMinRepValue; }
        uint32 GetRequiredMaxRepFaction() const { return RequiredMaxRepFaction; }
         int32 GetRequiredMaxRepValue() const { return RequiredMaxRepValue; }
        uint32 GetSuggestedPlayers() const { return SuggestedPlayers; }
        uint32 GetLimitTime() const { return LimitTime; }
        int32  GetNextQuestId() const { return NextQuestId; }
        int32  GetExclusiveGroup() const { return ExclusiveGroup; }
        uint32 GetNextQuestInChain() const { return NextQuestInChain; }
        uint32 GetSrcItemId() const { return SrcItemId; }
        uint32 GetSrcItemCount() const { return SrcItemCount; }
        uint32 GetSrcSpell() const { return SrcSpell; }
        const char* GetTitle() const { return Title.c_str(); }
        const char* GetDetails() const { return Details.c_str(); }
        const char* GetObjectives() const { return Objectives.c_str(); }
        const char* GetOfferRewardText() const { return OfferRewardText.c_str(); }
        const char* GetRequestItemsText() const { return RequestItemsText.c_str(); }
        const char* GetEndText() const { return EndText.c_str(); }
        int32  GetRewOrReqMoney() const { return RewOrReqMoney; }
        uint32 GetRewXpOrMoney() const { return RewXpOrMoney; }
        uint32 GetRewSpell() const { return RewSpell; }
        uint32 GetPointMapId() const { return PointMapId; }
        float  GetPointX() const { return PointX; }
        float  GetPointY() const { return PointY; }
        uint32 GetPointOpt() const { return PointOpt; }
        uint32 GetIncompleteEmote() const { return IncompleteEmote; }
        uint32 GetCompleteEmote() const { return CompleteEmote; }
        uint32 GetQuestStartScript() const { return QuestStartScript; }
        uint32 GetQuestCompleteScript() const { return QuestCompleteScript; }
        bool   IsRepeatable() const { return bool(Repeatable); }
        bool   IsAutoComplete() const { return Objectives.empty(); }
        uint32 GetSpecialFlags() const { return SpecialFlags; }

        // multiple values
        std::string ObjectiveText[QUEST_OBJECTIVES_COUNT];
        uint32 ReqItemId[QUEST_OBJECTIVES_COUNT];
        uint32 ReqItemCount[QUEST_OBJECTIVES_COUNT];
        uint32 ReqSourceId[QUEST_SOURCE_ITEM_IDS_COUNT];
        uint32 ReqSourceCount[QUEST_SOURCE_ITEM_IDS_COUNT];
        uint32 ReqSourceRef[QUEST_SOURCE_ITEM_IDS_COUNT];
        int32  ReqCreatureOrGOId[QUEST_OBJECTIVES_COUNT];   // >0 Creature <0 Gameobject
        uint32 ReqCreatureOrGOCount[QUEST_OBJECTIVES_COUNT];
        uint32 ReqSpell[QUEST_OBJECTIVES_COUNT];
        uint32 RewChoiceItemId[QUEST_REWARD_CHOICES_COUNT];
        uint32 RewChoiceItemCount[QUEST_REWARD_CHOICES_COUNT];
        uint32 RewItemId[QUEST_REWARDS_COUNT];
        uint32 RewItemCount[QUEST_REWARDS_COUNT];
        uint32 RewRepFaction[QUEST_REPUTATIONS_COUNT];
        int32  RewRepValue[QUEST_REPUTATIONS_COUNT];
        uint32 DetailsEmote[QUEST_EMOTE_COUNT];
        uint32 OfferRewardEmote[QUEST_EMOTE_COUNT];

        uint32 GetReqItemsCount() const { return m_reqitemscount; }
        uint32 GetReqCreatureOrGOcount() const { return m_reqCreatureOrGOcount; }
        uint32 GetRewChoiceItemsCount() const { return m_rewchoiceitemscount; }
        uint32 GetRewItemsCount() const { return m_rewitemscount; }

        typedef std::vector<int32> PrevQuests;
        PrevQuests prevQuests;
        typedef std::vector<uint32> PrevChainQuests;
        PrevChainQuests prevChainQuests;

        // cached data
    private:
        uint32 m_reqitemscount;
        uint32 m_reqCreatureOrGOcount;
        uint32 m_rewchoiceitemscount;
        uint32 m_rewitemscount;

        // table data
    protected:
        uint32 QuestId;
        int32  ZoneOrSort;
        uint32 MinLevel;
        uint32 QuestLevel;
        uint32 Type;
        uint32 RequiredRaces;
        uint32 RequiredSkillValue;
        uint32 RequiredMinRepFaction;
        int32  RequiredMinRepValue;
        uint32 RequiredMaxRepFaction;
        int32  RequiredMaxRepValue;
        uint32 SuggestedPlayers;
        uint32 LimitTime;
        uint32 SpecialFlags;
        int32  PrevQuestId;
        int32  NextQuestId;
        int32  ExclusiveGroup;
        uint32 NextQuestInChain;
        uint32 SrcItemId;
        uint32 SrcItemCount;
        uint32 SrcSpell;
        std::string Title;
        std::string Details;
        std::string Objectives;
        std::string OfferRewardText;
        std::string RequestItemsText;
        std::string EndText;
        int32  RewOrReqMoney;
        uint32 RewXpOrMoney;
        uint32 RewSpell;
        uint32 PointMapId;
        float  PointX;
        float  PointY;
        uint32 PointOpt;
        uint32 IncompleteEmote;
        uint32 CompleteEmote;
        uint32 QuestStartScript;
        uint32 QuestCompleteScript;
        uint32 Repeatable;
};

enum QuestUpdateState
{
    QUEST_UNCHANGED = 0,
    QUEST_CHANGED = 1,
    QUEST_NEW = 2
};

struct QuestStatusData
{
    QuestStatusData()
        : m_status(QUEST_STATUS_NONE),m_rewarded(false),
        m_explored(false), m_timer(0), uState(QUEST_NEW)
    {
        memset(m_itemcount,    0, QUEST_OBJECTIVES_COUNT * sizeof(uint32));
        memset(m_creatureOrGOcount, 0, QUEST_OBJECTIVES_COUNT * sizeof(uint32));
    }

    QuestStatus m_status;
    bool m_rewarded;
    bool m_explored;
    uint32 m_timer;
    QuestUpdateState uState;

    uint32 m_itemcount[ QUEST_OBJECTIVES_COUNT ];
    uint32 m_creatureOrGOcount[ QUEST_OBJECTIVES_COUNT ];
};
#endif
