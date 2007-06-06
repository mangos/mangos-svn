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
    RequiredRepFaction = questRecord[7].GetUInt32();
    RequiredRepValue = questRecord[8].GetUInt32();
    SuggestedPlayers = questRecord[9].GetUInt32();
    LimitTime = questRecord[10].GetUInt32();
    SpecialFlags = questRecord[11].GetUInt32();
    PrevQuestId = questRecord[12].GetInt32();
    NextQuestId = questRecord[13].GetInt32();
    ExclusiveGroup = questRecord[14].GetUInt32();
    NextQuestInChain = questRecord[15].GetUInt32();
    SrcItemId = questRecord[16].GetUInt32();
    SrcItemCount = questRecord[17].GetUInt32();
    SrcSpell = questRecord[18].GetUInt32();
    Title = questRecord[19].GetCppString();
    Details = questRecord[20].GetCppString();
    Objectives = questRecord[21].GetCppString();
    OfferRewardText = questRecord[22].GetCppString();
    RequestItemsText = questRecord[23].GetCppString();
    EndText = questRecord[24].GetCppString();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ObjectiveText[i] = questRecord[25+i].GetCppString();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqItemId[i] = questRecord[29+i].GetUInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqItemCount[i] = questRecord[33+i].GetUInt32();

    for (int i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
        ReqSourceId[i] = questRecord[37+i].GetUInt32();

    for (int i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
        ReqSourceCount[i] = questRecord[41+i].GetUInt32();

    for (int i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
        ReqSourceRef[i] = questRecord[45+i].GetUInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqCreatureOrGOId[i] = questRecord[49+i].GetInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqCreatureOrGOCount[i] = questRecord[53+i].GetUInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqSpell[i] = questRecord[57+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        RewChoiceItemId[i] = questRecord[61+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        RewChoiceItemCount[i] = questRecord[67+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARDS_COUNT; ++i)
        RewItemId[i] = questRecord[73+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARDS_COUNT; ++i)
        RewItemCount[i] = questRecord[77+i].GetUInt32();

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
        RewRepFaction[i] = questRecord[81+i].GetUInt32();

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
        RewRepValue[i] = questRecord[86+i].GetInt32();

    RewOrReqMoney = questRecord[91].GetInt32();;
    RewXpOrMoney = questRecord[92].GetUInt32();
    RewSpell = questRecord[93].GetUInt32();
    PointMapId = questRecord[94].GetUInt32();
    PointX = questRecord[95].GetFloat();
    PointY = questRecord[96].GetFloat();
    PointOpt = questRecord[97].GetUInt32();

    for (int i = 0; i < QUEST_EMOTE_COUNT; ++i)
        DetailsEmote[i] = questRecord[98+i].GetUInt32();

    IncompleteEmote = questRecord[102].GetUInt32();
    CompleteEmote = questRecord[103].GetUInt32();

    for (int i = 0; i < QUEST_EMOTE_COUNT; ++i)
        OfferRewardEmote[i] = questRecord[104+i].GetInt32();

    QuestCompleteScript = questRecord[108].GetUInt32();
    Repeatable = questRecord[109].GetUInt32();

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
            uint32 fullxp = 0;
            if (qLevel >= 15)
                fullxp = RewXpOrMoney / 6.0;
            if (qLevel == 14)
                fullxp = RewXpOrMoney / 4.8;
            if (qLevel == 13)
                fullxp = RewXpOrMoney / 3.6;
            if (qLevel == 12)
                fullxp = RewXpOrMoney / 2.4;
            if (qLevel == 11)
                fullxp = RewXpOrMoney / 1.2;
            if (qLevel > 0 && qLevel <= 10)
                fullxp = RewXpOrMoney / 0.6;

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
