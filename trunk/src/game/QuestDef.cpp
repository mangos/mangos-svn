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

#include "QuestDef.h"
#include "ObjectMgr.h"

Quest::Quest(Field * questRecord)
{
    /*m_quest_id = 0;
    m_reqitemscount = 0;
    m_reqCreatureOrGOcount = 0;
    m_rewchoiceitemscount = 0;
    m_rewitemscount = 0;

    m_offerRewardEmote = 0;
    m_requestItemsEmote = 0;*/

    QuestId = questRecord[0].GetUInt32();
    ZoneId = questRecord[1].GetUInt32();
    QuestSort = questRecord[2].GetUInt32();
    MinLevel = questRecord[3].GetUInt32();
    QuestLevel = questRecord[4].GetUInt32();
    Type = questRecord[5].GetUInt32();
    RequiredRaces = questRecord[6].GetUInt32();
    RequiredClass = questRecord[7].GetUInt32();
    RequiredSkill = questRecord[8].GetUInt32();
    RequiredSkillValue = questRecord[9].GetUInt32();
    RequiredRepFaction = questRecord[10].GetUInt32();
    RequiredRepValue = questRecord[11].GetUInt32();
    LimitTime = questRecord[12].GetUInt32();
    SpecialFlags = questRecord[13].GetUInt32();
    PrevQuestId = questRecord[14].GetInt32();
    NextQuestId = questRecord[15].GetInt32();
    ExclusiveGroup = questRecord[16].GetUInt32();
    NextQuestInChain = questRecord[17].GetUInt32();
    SrcItemId = questRecord[18].GetUInt32();
    SrcItemCount = questRecord[19].GetUInt32();
    SrcSpell = questRecord[20].GetUInt32();
    Title = questRecord[21].GetCppString();
    Details = questRecord[22].GetCppString();
    Objectives = questRecord[23].GetCppString();
    OfferRewardText = questRecord[24].GetCppString();
    RequestItemsText = questRecord[25].GetCppString();
    EndText = questRecord[26].GetCppString();
    for (int i = 27; i < 27 + QUEST_OBJECTIVES_COUNT; i++)
    {
        //if (questRecord[i].GetCppString().size() > 0)
        ObjectiveText.push_back(questRecord[i].GetCppString());
    }
    for (int i = 31; i < 31 + QUEST_OBJECTIVES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqItemId.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 35; i < 35 + QUEST_OBJECTIVES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqItemCount.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 39; i < 39 + QUEST_SOURCE_ITEM_IDS_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqSourceId.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 43; i < 43 + QUEST_SOURCE_ITEM_IDS_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqSourceRef.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 47; i < 47 + QUEST_OBJECTIVES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqCreatureOrGOId.push_back(questRecord[i].GetInt32());
    }
    for (int i = 51; i < 51 + QUEST_OBJECTIVES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqCreatureOrGOCount.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 55; i < 55 + QUEST_OBJECTIVES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqSpell.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 59; i < 59 + QUEST_REWARD_CHOICES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        RewChoiceItemId.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 65; i < 65 + QUEST_REWARD_CHOICES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        RewChoiceItemCount.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 71; i < 71 + QUEST_REWARDS_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        RewItemId.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 75; i < 75 + QUEST_REWARDS_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        RewItemCount.push_back(questRecord[i].GetUInt32());
    }
    RewRepFaction1 = questRecord[79].GetUInt32();
    RewRepFaction2 = questRecord[80].GetUInt32();
    RewRepValue1 = questRecord[81].GetInt32();
    RewRepValue2 = questRecord[82].GetInt32();
    RewOrReqMoney = questRecord[83].GetInt32();;
    RewXP = questRecord[84].GetUInt32();
    RewSpell = questRecord[85].GetUInt32();
    PointMapId = questRecord[86].GetUInt32();
    PointX = questRecord[87].GetFloat();
    PointY = questRecord[88].GetFloat();
    PointOpt = questRecord[89].GetUInt32();
    OfferRewardEmote = questRecord[90].GetUInt32();
    RequestItemsEmote = questRecord[91].GetUInt32();
    QuestCompleteScript = questRecord[92].GetUInt32();
    Repeatable = questRecord[93].GetUInt32();

    m_reqitemscount = 0;
    m_reqCreatureOrGOcount = 0;
    m_rewitemscount = 0;
    m_rewchoiceitemscount = 0;

    for (int i=0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if ( ReqItemId[i] )
            m_reqitemscount++;
        if ( ReqCreatureOrGOId[i] )
            m_reqCreatureOrGOcount++;
    }

    for (int i=0; i < QUEST_REWARDS_COUNT; i++)
    {
        if ( RewItemId[i] )
            m_rewitemscount++;
    }

    for (int i=0; i < QUEST_REWARD_CHOICES_COUNT; i++)
    {
        if (RewChoiceItemId[i])
            m_rewchoiceitemscount++;
    }
}

uint32 Quest::XPValue( Player *pPlayer )
{
    if( pPlayer )
    {
        uint32 fullxp = RewXP;
        if( fullxp > 0 )
        {
            uint32 pLevel = pPlayer->getLevel();
            uint32 qLevel = QuestLevel;

            if( pLevel <= qLevel +  5 )
                return fullxp;
            else if( pLevel == qLevel +  6 )
                return (uint32)(fullxp * 0.8);
            else if( pLevel == qLevel +  7 )
                return (uint32)(fullxp * 0.6);
            else if( pLevel == qLevel +  8 )
                return (uint32)(fullxp * 0.4);
            else if( pLevel == qLevel +  9 )
                return (uint32)(fullxp * 0.2);
            else
                return (uint32)(fullxp * 0.1);
        }
    }
    return 0;
}
