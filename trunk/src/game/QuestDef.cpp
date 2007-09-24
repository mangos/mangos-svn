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
    QuestId = questRecord[0].GetUInt32();
    ZoneOrSort = questRecord[1].GetInt32();
    MinLevel = questRecord[2].GetUInt32();
    QuestLevel = questRecord[3].GetUInt32();
    Type = questRecord[4].GetUInt32();
    RequiredRaces = questRecord[5].GetUInt32();
    RequiredSkillValue = questRecord[6].GetUInt32();
    RequiredMinRepFaction = questRecord[7].GetUInt32();
    RequiredMinRepValue = questRecord[8].GetUInt32();
    RequiredMaxRepFaction = questRecord[9].GetUInt32();
    RequiredMaxRepValue = questRecord[10].GetUInt32();
    SuggestedPlayers = questRecord[11].GetUInt32();
    LimitTime = questRecord[12].GetUInt32();
    SpecialFlags = questRecord[13].GetUInt32();
    PrevQuestId = questRecord[14].GetInt32();
    NextQuestId = questRecord[15].GetInt32();
    ExclusiveGroup = questRecord[16].GetInt32();
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

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ObjectiveText[i] = questRecord[27+i].GetCppString();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqItemId[i] = questRecord[31+i].GetUInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqItemCount[i] = questRecord[35+i].GetUInt32();

    for (int i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
        ReqSourceId[i] = questRecord[39+i].GetUInt32();

    for (int i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
        ReqSourceCount[i] = questRecord[43+i].GetUInt32();

    for (int i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
        ReqSourceRef[i] = questRecord[47+i].GetUInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqCreatureOrGOId[i] = questRecord[51+i].GetInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqCreatureOrGOCount[i] = questRecord[55+i].GetUInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqSpell[i] = questRecord[59+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        RewChoiceItemId[i] = questRecord[63+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        RewChoiceItemCount[i] = questRecord[69+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARDS_COUNT; ++i)
        RewItemId[i] = questRecord[75+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARDS_COUNT; ++i)
        RewItemCount[i] = questRecord[79+i].GetUInt32();

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
        RewRepFaction[i] = questRecord[83+i].GetUInt32();

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
        RewRepValue[i] = questRecord[88+i].GetInt32();

    RewOrReqMoney = questRecord[93].GetInt32();;
    RewXpOrMoney = questRecord[94].GetUInt32();
    RewSpell = questRecord[95].GetUInt32();
    PointMapId = questRecord[96].GetUInt32();
    PointX = questRecord[97].GetFloat();
    PointY = questRecord[98].GetFloat();
    PointOpt = questRecord[99].GetUInt32();

    for (int i = 0; i < QUEST_EMOTE_COUNT; ++i)
        DetailsEmote[i] = questRecord[100+i].GetUInt32();

    IncompleteEmote = questRecord[104].GetUInt32();
    CompleteEmote = questRecord[105].GetUInt32();

    for (int i = 0; i < QUEST_EMOTE_COUNT; ++i)
        OfferRewardEmote[i] = questRecord[106+i].GetInt32();

    QuestStartScript = questRecord[110].GetUInt32();
    QuestCompleteScript = questRecord[111].GetUInt32();
    Repeatable = questRecord[112].GetUInt32();

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
        if( RewXpOrMoney > 0 )
        {
            uint32 pLevel = pPlayer->getLevel();
            uint32 qLevel = QuestLevel;
            float fullxp = 0;
            if (qLevel >= 15)
                fullxp = RewXpOrMoney / 6.0;
            else if (qLevel == 14)
                fullxp = RewXpOrMoney / 4.8;
            else if (qLevel == 13)
                fullxp = RewXpOrMoney / 3.6;
            else if (qLevel == 12)
                fullxp = RewXpOrMoney / 2.4;
            else if (qLevel == 11)
                fullxp = RewXpOrMoney / 1.2;
            else if (qLevel > 0 && qLevel <= 10)
                fullxp = RewXpOrMoney / 0.6;

            if( pLevel <= qLevel +  5 )
                return (uint32)fullxp;
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
