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
    INVALIDREASON_DONT_HAVE_REQ        = 0,
    INVALIDREASON_DONT_HAVE_RACE       = 6,
    INVALIDREASON_HAVE_TIMED_QUEST     = 12,
    INVALIDREASON_HAVE_QUEST           = 13,
    INVALIDREASON_DONT_HAVE_REQ_ITEMS  = 19,
    INVALIDREASON_DONT_HAVE_REQ_MONEY  = 21,
};

enum
{
    QUEST_PARTY_MSG_SHARRING_QUEST  = 0,
    QUEST_PARTY_MSG_CANT_TAKE_QUEST = 1,
    QUEST_PARTY_MSG_ACCEPT_QUEST    = 2,
    QUEST_PARTY_MSG_REFUSE_QUEST    = 3,
    QUEST_PARTY_MSG_TO_FAR          = 4,
    QUEST_PARTY_MSG_BUSY            = 5,
    QUEST_PARTY_MSG_LOG_FULL        = 6,
    QUEST_PARTY_MSG_HAVE_QUEST      = 7,
    QUEST_PARTY_MSG_FINISH_QUEST    = 8,
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

enum __QuestRace
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

enum __QuestSpecialFlags                                    //according to mangos-db-11-02-2006-for_1_9_x;
{
    QUEST_SPECIAL_FLAGS_NONE          = 0,
    QUEST_SPECIAL_FLAGS_DELIVER       = 1,
    QUEST_SPECIAL_FLAGS_EXPLORATION   = 2,
    QUEST_SPECIAL_FLAGS_SPEAKTO       = 4,

    QUEST_SPECIAL_FLAGS_KILL_OR_CAST  = 8,
    QUEST_SPECIAL_FLAGS_TIMED         = 16,
    QUEST_SPECIAL_FLAGS_REPEATABLE    = 32,                 //?

    QUEST_SPECIAL_FLAGS_REPUTATION    = 64,
};

// Only GCC 4.1.0 and later support #pragma pack(push,1) syntax
#if defined( __GNUC__ ) && (GCC_MAJOR < 4 || GCC_MAJOR == 4 && GCC_MINOR < 1)
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

struct QuestInfo
{
    uint32 QuestId;
    uint32 ZoneId;
    uint32 QuestSort;
    uint32 MinLevel;
    uint32 QuestLevel;
    uint32 Type;
    uint32 RequiredRaces;
    uint32 RequiredClass;
    uint32 RequiredSkill;
    uint32 RequiredSkillValue;
    uint32 RequiredRepFaction;
    uint32 RequiredRepValue;
    uint32 LimitTime;
    uint32 SpecialFlags;
    uint32 PrevQuestId;
    uint32 NextQuestId;
    uint32 SrcItemId;
    uint32 SrcItemCount;
    char* Title;
    char* Details;
    char* Objectives;
    char* CompletionText;
    char* IncompleteText;
    char* EndText;
    char* ObjectiveText[4];
    uint32 ReqItemId[ QUEST_OBJECTIVES_COUNT ];
    uint32 ReqItemCount[ QUEST_OBJECTIVES_COUNT ];
    int32  ReqCreatureOrGOId[ QUEST_OBJECTIVES_COUNT ];     // >0 Creature <0 Gameobject
    uint32 ReqCreatureOrGOCount[ QUEST_OBJECTIVES_COUNT ];
    uint32 ReqSpell[ QUEST_OBJECTIVES_COUNT ];
    //uint32 ReqQuests[ QUEST_DEPLINK_COUNT ];
    uint32 RewChoiceItemId[ QUEST_REWARD_CHOICES_COUNT ];
    uint32 RewChoiceItemCount[ QUEST_REWARD_CHOICES_COUNT ];
    uint32 RewItemId[ QUEST_REWARDS_COUNT ];
    uint32 RewItemCount[ QUEST_REWARDS_COUNT ];
    uint32 RewRepFaction1;
    uint32 RewRepFaction2;
    int32  RewRepValue1;
    int32  RewRepValue2;
    int32  RewOrReqMoney;
    uint32 RewXP;
    uint32 RewSpell;
    uint32 PointMapId;
    float PointX;
    float PointY;
    uint32 PointOpt;

    // simple data access functions
    bool HasSpecialFlag( uint32 flag ) const { return (SpecialFlags & flag ) != 0; }
};

#if defined( __GNUC__ ) && (GCC_MAJOR < 4 || GCC_MAJOR == 4 && GCC_MINOR < 1)
#pragma pack()
#else
#pragma pack(pop)
#endif

class Quest
{
    public:
        Quest();

        QuestInfo const* GetQuestInfo() const;
        uint32 GetQuestId() const { return m_quest_id; }

        uint32 m_reqitemscount;
        uint32 m_reqmobs_or_GO_count;
        uint32 m_rewchoiceitemscount;
        uint32 m_rewitemscount;

        bool LoadQuest( uint32 quest );
        uint32 XPValue( Player *pPlayer );

    private:
        uint32 m_quest_id;
};

struct quest_status
{
    quest_status()
    {
        memset(m_itemcount, 0, QUEST_OBJECTIVES_COUNT * sizeof(uint32));
        memset(m_mobcount, 0, QUEST_OBJECTIVES_COUNT * sizeof(uint32));
    }

    Quest *m_quest;

    uint32 m_status;
    bool m_rewarded;
    bool m_explored;
    uint32 m_timer;

    uint32 m_itemcount[ QUEST_OBJECTIVES_COUNT ];
    uint32 m_mobcount[ QUEST_OBJECTIVES_COUNT ];
};
#endif
