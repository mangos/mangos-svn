/* Quest.h
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

#ifndef WOWSERVER_QUEST_H
#define WOWSERVER_QUEST_H

enum QuestStatus
{
    QUEST_STATUS_COMPLETE       = 1,
    QUEST_STATUS_UNAVAILABLE    = 2,              // need to be higher level
    QUEST_STATUS_INCOMPLETE     = 3,
    QUEST_STATUS_AVAILABLE      = 4,
};

class Quest
{
    public:
        Quest()
        {
            m_targetGuid = 0;
            m_originalGuid = 0;
            m_zone = 0;
            memset(m_questItemId, 0, 16);
            memset(m_questItemCount, 0, 16);
            memset(m_questMobId, 0, 16);
            memset(m_questMobCount, 0, 16);

            memset(m_choiceItemId, 0, 20);
            memset(m_choiceItemCount, 0, 20);
            memset(m_rewardItemId, 0, 20);
            memset(m_rewardItemCount, 0, 20);

            m_choiceRewards = 0;
            m_itemRewards = 0;
            m_rewardGold = 0;
            m_questXp = 0;

            m_requiredLevel = 0;
            m_previousQuest = 0;
        }

        uint32 m_questId;
        uint32 m_zone;

        // String descriptions sent to Client
        std::string m_title;
        std::string m_details;
        std::string m_objectives;
        std::string m_completedText;
        std::string m_incompleteText;

        // Quest pre-requisites
        uint32 m_requiredLevel;                   // level you are required to be to do this quest
        uint32 m_previousQuest;                   // id of a previous quest that much be completed first

        // Quest Requirements
        uint64 m_originalGuid;                    // GUID of the original questgiving creature, only needed when m_targetGuid is filled out
        uint64 m_targetGuid;                      // GUID of a creature to speak with to complete the quest

        uint32 m_questItemId[4];                  // entry ID of the item type to find
        uint32 m_questItemCount[4];               // number of items to find

        uint32 m_questMobId[4];                   // entry ID of the mob to be slain for this quest
        uint32 m_questMobCount[4];                // number of mobs to slay

        // Rewards
        uint16 m_choiceRewards;                   // number of items to choose from, max 5?
        uint32 m_choiceItemId[5];                 // entry ID of the items to choose for a reward
        uint32 m_choiceItemCount[5];              // number of each item to be awarded

        uint16 m_itemRewards;                     // number of items always rewarded
        uint32 m_rewardItemId[5];                 // entry ID of the items to be awarded
        uint32 m_rewardItemCount[5];              // count of each item to be awarded

        uint32 m_rewardGold;                      // gold reward
        uint32 m_questXp;                         // XP gained from completing this quest
};
#endif
