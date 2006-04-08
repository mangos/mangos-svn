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

#ifndef MANGOSSERVER_QUEST_H
#define MANGOSSERVER_QUEST_H

#include "Platform/Define.h"
#include <string>

class Player;

#define QUEST_OBJECTIVES_COUNT 4
#define QUEST_REWARD_CHOICES_COUNT 6
#define QUEST_REWARDS_COUNT 4
#define QUEST_DEPLINK_COUNT 10

enum
{
    FAILEDREASON_DUPE_ITEM_FOUND = 0x10,
    FAILEDREASON_FAILED          = 0,
    FAILEDREASON_INV_FULL        = 4,
};

enum
{
    INVALIDREASON_DONT_HAVE_REQ = 0,
    INVALIDREASON_DONT_HAVE_REQ_ITEMS = 0x13,
    INVALIDREASON_DONT_HAVE_REQ_MONEY = 0x15,
    INVALIDREASON_DONT_HAVE_RACE = 6,
    INVALIDREASON_DONT_HAVE_LEVEL = 1,
    INVALIDREASON_HAVE_QUEST = 13,
    INVALIDREASON_HAVE_TIMED_QUEST = 12,
};

enum __QuestClass
{
    QUEST_CLASS_NONE            = 0,
    QUEST_CLASS_WARRIOR         = 1,
    QUEST_CLASS_PALADIN         = 2,
    QUEST_CLASS_HUNTER          = 3,
    QUEST_CLASS_ROGUE           = 4,
    QUEST_CLASS_PRIEST          = 5,
    QUEST_CLASS_UNK0            = 6,
    QUEST_CLASS_SHAMAN          = 7,
    QUEST_CLASS_MAGE            = 8,
    QUEST_CLASS_WARLOCK         = 9,
    QUEST_CLASS_UNK1            = 10,
    QUEST_CLASS_DRUID           = 11,
};

enum __QuestRase
{
    QUEST_RACE_NONE            = 0,
    QUEST_RACE_HUMAN           = 1,
    QUEST_RACE_ORC             = 2,
    QUEST_RACE_DWARF           = 3,
    QUEST_RACE_NIGHTELF        = 4,
    QUEST_RACE_UNDEAD          = 5,
    QUEST_RACE_TAUREN          = 6,
    QUEST_RACE_GNOME           = 7,
    QUEST_RACE_TROLL           = 8,
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

enum __QuestStatus
{
    QUEST_STATUS_NONE           = 0,
    QUEST_STATUS_COMPLETE       = 1,
    QUEST_STATUS_UNAVAILABLE    = 2,
    QUEST_STATUS_INCOMPLETE     = 3,
    QUEST_STATUS_AVAILABLE      = 4,
};

enum __QuestGiverStatus
{
    DIALOG_STATUS_NONE                     = 0,
    DIALOG_STATUS_UNAVAILABLE              = 1,
    DIALOG_STATUS_CHAT                     = 2,
    DIALOG_STATUS_INCOMPLETE               = 3,
    DIALOG_STATUS_REWARD_REP               = 4,
    DIALOG_STATUS_AVAILABLE                = 5,
    DIALOG_STATUS_REWARD                   = 6,
};

enum __QuestSpecialFlags
{
    QUEST_SPECIAL_FLAGS_NONE          = 0,
    QUEST_SPECIAL_FLAGS_DELIVER       = 1,
    QUEST_SPECIAL_FLAGS_KILL          = 2,
    QUEST_SPECIAL_FLAGS_SPEAKTO       = 4,

    QUEST_SPECIAL_FLAGS_REPEATABLE    = 8,
    QUEST_SPECIAL_FLAGS_EXPLORATION   = 16,

    QUEST_SPECIAL_FLAGS_TIMED         = 32,
    QUEST_SPECIAL_FLAGS_REPUTATION    = 128,
};

class Quest
{
    public:
        Quest();

        uint32 m_qId;
        uint32 m_qCategory;
        uint32 m_qFlags;
        uint32 m_qType;

        uint32 m_qPointId, m_qPointOpt;
        float m_qPointX, m_qPointY;

        std::string m_qTitle;
        std::string m_qDetails;
        std::string m_qObjectives;
        std::string m_qEndInfo;
        std::string m_qObjectiveInfo[ QUEST_OBJECTIVES_COUNT ];

        std::string m_qCompletionInfo;
        std::string m_qIncompleteInfo;
        uint32 m_qComplexityLevel;

        uint32 m_qPlayerLevel;

        uint32 m_qRequiredQuestsCount;
        uint32 m_qRequiredQuests[ QUEST_DEPLINK_COUNT ];

        uint32 m_qRequiredAbsQuestsCount;
        uint32 m_qRequiredAbsQuests[ QUEST_DEPLINK_COUNT ];

        uint32 m_qLockerQuestsCount;
        uint32 m_qLockerQuests[ QUEST_DEPLINK_COUNT ];

        uint32 m_qRequiredRaces;
        uint32 m_qRequiredClass;
        uint32 m_qRequiredTradeskill;

        uint32 m_qObjItemId[ QUEST_OBJECTIVES_COUNT ];
        uint32 m_qObjItemCount[ QUEST_OBJECTIVES_COUNT ];

        uint32 m_qObjMobId[ QUEST_OBJECTIVES_COUNT ];
        uint32 m_qObjMobCount[ QUEST_OBJECTIVES_COUNT ];

        uint32 m_qObjRepFaction_1;
        uint32 m_qObjRepValue_1;

        uint32 m_qObjRepFaction_2;
        uint32 m_qObjRepValue_2;

        uint32 m_qObjTime;

        uint32 m_qRewChoicesCount;
        uint32 m_qRewChoicesItemId[ QUEST_REWARD_CHOICES_COUNT ];
        uint32 m_qRewChoicesItemCount[ QUEST_REWARD_CHOICES_COUNT ];

        uint32 m_qRewCount;
        uint32 m_qRewItemId[ QUEST_REWARDS_COUNT ];
        uint32 m_qRewItemCount[ QUEST_REWARDS_COUNT ];

        uint32 m_qRewMoney;
        uint32 m_qRewSpell;

        uint32 m_qQuestItem;
        uint32 m_qNextQuestId;
        Quest* m_qNextQuest;

        uint32  m_qSpecialFlags;

        uint32 XPValue                 (Player* _Player);

        uint32 GetKillObjectivesCount();

        uint32 GetDeliverObjectivesCount();

        bool CanBeTaken( Player *_Player );
        bool IsCompatible( Player *_Player );
        bool ReputationSatisfied( Player *_Player );
        bool TradeSkillSatisfied( Player *_Player );
        bool RaceSatisfied( Player *_Player );
        bool ClassSatisfied( Player *_Player );
        bool LevelSatisfied( Player *_Player );
        bool CanShowAvailable( Player *_Player );
        bool CanShowUnsatified( Player *_Player );
        bool PreReqSatisfied( Player *_Player );
        bool RewardIsTaken( Player *_Player );
        bool HasFlag( uint32 Flag )  { return (( m_qSpecialFlags & Flag ) == Flag); }

};

struct quest_status
{

    quest_status()
    {
        memset(m_questItemCount, 0, QUEST_OBJECTIVES_COUNT * sizeof(uint32));
        memset(m_questMobCount , 0, QUEST_OBJECTIVES_COUNT * sizeof(uint32));
        m_timerrel = 0;
    }

    Quest *m_quest;
    uint32 status;
    bool rewarded;
    uint32 m_questItemCount[ QUEST_OBJECTIVES_COUNT ];
    uint32 m_questMobCount [ QUEST_OBJECTIVES_COUNT ];

    uint32  m_timer;
    uint32  m_timerrel;
    bool    m_explored;
};
#endif
