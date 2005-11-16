/* QuestDef.h
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

 /*
   Basic Quest Type. Fields include standard Quest info as the
   modifiers needed by the server to manage the quests properly.
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

enum {
	FAILEDREASON_DUPE_ITEM_FOUND = 0x10,
    FAILEDREASON_FAILED			 = 0,
    FAILEDREASON_INV_FULL		 = 4,
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
    QUEST_STATUS_NONE           = 0,   // Player has no status for this quest.
    QUEST_STATUS_COMPLETE       = 1,   // Quest is complete ( rewarded or not ).
    QUEST_STATUS_UNAVAILABLE    = 2,   // Quest requirements not satisfied.
    QUEST_STATUS_INCOMPLETE     = 3,   // Quest is incomplete.
    QUEST_STATUS_AVAILABLE      = 4,   // Quest is available to be taken.
};

enum __QuestGiverStatus
{
    DIALOG_STATUS_NONE					   = 0,   // Questgiver has nothing to tell you.
    DIALOG_STATUS_UNAVAILABLE			   = 1,   // Has quests, but you require a higher level.
    DIALOG_STATUS_CHAT					   = 2,   // Can chat with you.
    DIALOG_STATUS_INCOMPLETE			   = 3,   // You haven't completed the quest.
    DIALOG_STATUS_REWARD_REP			   = 4,   // Has reward to give (repeatable).
    DIALOG_STATUS_AVAILABLE				   = 5,   // Has quests to give now.
    DIALOG_STATUS_REWARD				   = 6,   // Has reward to give.
};


enum __QuestSpecialFlags
{
	QUEST_SPECIAL_FLAGS_NONE          = 0,   // Has no special flags.
	QUEST_SPECIAL_FLAGS_DELIVER       = 1,   // Has Delivery objectives.
	QUEST_SPECIAL_FLAGS_KILL          = 2,   // Has Kill Objectives.
	QUEST_SPECIAL_FLAGS_SPEAKTO       = 4,   // Has SpeakTo Objectives.

	QUEST_SPECIAL_FLAGS_REPEATABLE    = 8,   // It's repeatable
	QUEST_SPECIAL_FLAGS_EXPLORATION   = 16,  // It's an exploration quest.

	QUEST_SPECIAL_FLAGS_TIMED         = 32,  // Timer quest.
	QUEST_SPECIAL_FLAGS_REPUTATION    = 128, // Repuation Objectives.
};

struct quest_status{

    quest_status(){
        memset(m_questItemCount, 0, QUEST_OBJECTIVES_COUNT * sizeof(uint32));
        memset(m_questMobCount , 0, QUEST_OBJECTIVES_COUNT * sizeof(uint32));
		m_timerrel = 0;
    }

    uint32 quest_id;
    uint32 status;
	bool rewarded;
    uint32 m_questItemCount[ QUEST_OBJECTIVES_COUNT ]; // number of items collected. Ignored
    uint32 m_questMobCount [ QUEST_OBJECTIVES_COUNT ];  // number of monsters slain

	uint32	m_timer;
	uint32	m_timerrel;
	bool	m_explored;
};

class Quest
{
public:
	Quest();

	/*
     *  Basic data sent to the client. It is not used internally.
	 */

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
	
	/*
     *  Internal and client used data.
	 */

    std::string m_qCompletionInfo;
    std::string m_qIncompleteInfo;
	uint32 m_qComplexityLevel;

	/*
     *  Quest Pre-requirements.
	 */

    uint32 m_qPlayerLevel;


	/*
	 * LudMilla Defined Fields. Could be done otherwise.
	 */

    uint32 m_qRequiredQuestsCount;
    uint32 m_qRequiredQuests[ QUEST_DEPLINK_COUNT ];

    uint32 m_qRequiredAbsQuestsCount;
    uint32 m_qRequiredAbsQuests[ QUEST_DEPLINK_COUNT ];

    uint32 m_qLockerQuestsCount;
    uint32 m_qLockerQuests[ QUEST_DEPLINK_COUNT ];

	uint32 m_qRequiredRaces;
	uint32 m_qRequiredClass;
	uint32 m_qRequiredTradeskill;

	/*
	 * Objective Fields.
	 */

    uint32 m_qObjItemId[ QUEST_OBJECTIVES_COUNT ];
    uint32 m_qObjItemCount[ QUEST_OBJECTIVES_COUNT ];

    uint32 m_qObjMobId[ QUEST_OBJECTIVES_COUNT ];
    uint32 m_qObjMobCount[ QUEST_OBJECTIVES_COUNT ];

	uint32 m_qObjRepFaction_1;
	uint32 m_qObjRepValue_1;

	uint32 m_qObjRepFaction_2;
	uint32 m_qObjRepValue_2;

	uint32 m_qObjTime;

	/*
	 * Rewards.
	 */

    uint32 m_qRewChoicesCount;
    uint32 m_qRewChoicesItemId[ QUEST_REWARD_CHOICES_COUNT ];
    uint32 m_qRewChoicesItemCount[ QUEST_REWARD_CHOICES_COUNT ];

    uint32 m_qRewCount;
    uint32 m_qRewItemId[ QUEST_REWARDS_COUNT ];
    uint32 m_qRewItemCount[ QUEST_REWARDS_COUNT ];

    uint32 m_qRewMoney;
	uint32 m_qRewSpell;

	/*
	 * Other fields.
	 */


    uint32 m_qQuestItem;
    uint32 m_qNextQuest;

	uint32  m_qSpecialFlags;

	/*
	 * Functions to be used by external modules.
	 */

	//-----------------------------------------------
	// Returns the amount of XP a player _Player would
	// get by completing this quest.
	uint32 XPValue                 (Player* _Player);

	//-----------------------------------------------
	// Returns the number of Kill Objectives
	uint32 GetKillObjectivesCount();

	//-----------------------------------------------
	// Returns the number of Deliver Objectives
	uint32 GetDeliverObjectivesCount();

	//------------------------------------------------
	// Helper Functions
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


#endif
