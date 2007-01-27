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
    SrcItemId = questRecord[17].GetUInt32();
    SrcItemCount = questRecord[18].GetUInt32();
    SrcSpell = questRecord[19].GetUInt32();
    Title = questRecord[20].GetCppString();
    Details = questRecord[21].GetCppString();
    Objectives = questRecord[22].GetCppString();
    OfferRewardText = questRecord[23].GetCppString();
    RequestItemsText = questRecord[24].GetCppString();
    EndText = questRecord[25].GetCppString();
    for (int i = 26; i < 26 + QUEST_OBJECTIVES_COUNT; i++)
    {
        //if (questRecord[i].GetCppString().size() > 0)
        ObjectiveText.push_back(questRecord[i].GetCppString());
    }
    for (int i = 30; i < 30 + QUEST_OBJECTIVES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqItemId.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 34; i < 34 + QUEST_OBJECTIVES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqItemCount.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 38; i < 38 + QUEST_SOURCE_ITEM_IDS_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqSourceId.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 42; i < 42 + QUEST_SOURCE_ITEM_IDS_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqSourceRef.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 46; i < 46 + QUEST_OBJECTIVES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqCreatureOrGOId.push_back(questRecord[i].GetInt32());
    }
    for (int i = 50; i < 50 + QUEST_OBJECTIVES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqCreatureOrGOCount.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 54; i < 54 + QUEST_OBJECTIVES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        ReqSpell.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 58; i < 58 + QUEST_REWARD_CHOICES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        RewChoiceItemId.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 64; i < 64 + QUEST_REWARD_CHOICES_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        RewChoiceItemCount.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 70; i < 70 + QUEST_REWARDS_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        RewItemId.push_back(questRecord[i].GetUInt32());
    }
    for (int i = 74; i < 74 + QUEST_REWARDS_COUNT; i++)
    {
        //if (questRecord[i].GetUInt32() != 0)
        RewItemCount.push_back(questRecord[i].GetUInt32());
    }
    RewRepFaction1 = questRecord[78].GetUInt32();
    RewRepFaction2 = questRecord[79].GetUInt32();
    RewRepValue1 = questRecord[80].GetInt32();
    RewRepValue2 = questRecord[81].GetInt32();
    RewOrReqMoney = questRecord[82].GetInt32();;
    RewXP = questRecord[83].GetUInt32();
    RewSpell = questRecord[84].GetUInt32();
    PointMapId = questRecord[85].GetUInt32();
    PointX = questRecord[86].GetFloat();
    PointY = questRecord[87].GetFloat();
    PointOpt = questRecord[88].GetUInt32();
    OfferRewardEmote = questRecord[89].GetUInt32();
    RequestItemsEmote = questRecord[90].GetUInt32();
    QuestCompleteScript = questRecord[91].GetUInt32();
    HaveQuestId = questRecord[92].GetUInt32();
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
