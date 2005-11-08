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
#include "Player.h"
#include <string>

#define QUEST_OBJECTIVES_COUNT 4
#define QUEST_DEPLINK_COUNT 10

enum __QuestStatus
{
    QUEST_STATUS_NONE           = 0,   // Player has no status for this quest.
    QUEST_STATUS_COMPLETE       = 1,   // Quest is complete ( rewarded or not ).
    QUEST_STATUS_UNAVAILABLE    = 2,   // Quest requirements not satisfied.
    QUEST_STATUS_INCOMPLETE     = 3,   // Quest is incomplete.
    QUEST_STATUS_AVAILABLE      = 4,   // Quest is available to be taken.
	QUEST_STATUS_REWARDED		= 5,   // A reward was taken by player.
};

enum __QuestGiverStatus
{
    QUESTGIVER_STATUS_NONE				   = 0,   // Questgiver has nothing to tell you.
    QUESTGIVER_STATUS_UNAVAILABLE		   = 1,   // Has quests, but you require a higher level.
    QUESTGIVER_STATUS_CHAT				   = 2,   // Can chat with you.
    QUESTGIVER_STATUS_INCOMPLETE		   = 3,   // You haven't completed the quest.
    QUESTGIVER_STATUS_REPEATABLE_HASREWARD = 4,   // Has reward to give (repeatable).
    QUESTGIVER_STATUS_AVAILABLE			   = 5,   // Has quests to give now.
    QUESTGIVER_STATUS_HASREWARD			   = 6,   // Has reward to give.
};

enum __QuestSpecialFlags
{
	QUEST_SPECIAL_FLAGS_NONE          = 0,   // Has no special flags
	QUEST_SPECIAL_FLAGS_REPEATABLE    = 1,   // It's repeatable
	QUEST_SPECIAL_FLAGS_EXPLORATION   = 2,   // It's an exploration quest.
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

	uint32 m_qPointId;
	 float m_qPointX, m_qPointY, m_qPointZ;

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

    uint16 m_qRewChoicesCount;
    uint32 m_qRewChoicesItemId[6];
    uint32 m_qRewChoicesItemCount[6];

    uint16 m_qRewCount;
    uint32 m_qRewCountItemId[4];
    uint32 m_qRewCountItemCount[4];

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
};


#endif
