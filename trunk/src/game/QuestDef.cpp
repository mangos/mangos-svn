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
    RequiredMinRepValue = questRecord[8].GetInt32();
    RequiredMaxRepFaction = questRecord[9].GetUInt32();
    RequiredMaxRepValue = questRecord[10].GetInt32();
    SuggestedPlayers = questRecord[11].GetUInt32();
    LimitTime = questRecord[12].GetUInt32();
    QuestFlags = questRecord[13].GetUInt16();
    uint32 SpecialFlags = questRecord[14].GetUInt16();
    PrevQuestId = questRecord[15].GetInt32();
    NextQuestId = questRecord[16].GetInt32();
    ExclusiveGroup = questRecord[17].GetInt32();
    NextQuestInChain = questRecord[18].GetUInt32();
    SrcItemId = questRecord[19].GetUInt32();
    SrcItemCount = questRecord[20].GetUInt32();
    SrcSpell = questRecord[21].GetUInt32();
    Title = questRecord[22].GetCppString();
    Details = questRecord[23].GetCppString();
    Objectives = questRecord[24].GetCppString();
    OfferRewardText = questRecord[25].GetCppString();
    RequestItemsText = questRecord[26].GetCppString();
    EndText = questRecord[27].GetCppString();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ObjectiveText[i] = questRecord[28+i].GetCppString();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqItemId[i] = questRecord[32+i].GetUInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqItemCount[i] = questRecord[36+i].GetUInt32();

    for (int i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
        ReqSourceId[i] = questRecord[40+i].GetUInt32();

    for (int i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
        ReqSourceCount[i] = questRecord[44+i].GetUInt32();

    for (int i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
        ReqSourceRef[i] = questRecord[48+i].GetUInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqCreatureOrGOId[i] = questRecord[52+i].GetInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqCreatureOrGOCount[i] = questRecord[56+i].GetUInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqSpell[i] = questRecord[60+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        RewChoiceItemId[i] = questRecord[64+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        RewChoiceItemCount[i] = questRecord[70+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARDS_COUNT; ++i)
        RewItemId[i] = questRecord[76+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARDS_COUNT; ++i)
        RewItemCount[i] = questRecord[80+i].GetUInt32();

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
        RewRepFaction[i] = questRecord[84+i].GetUInt32();

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
        RewRepValue[i] = questRecord[89+i].GetInt32();

    RewOrReqMoney = questRecord[94].GetInt32();
    RewXpOrMoney = questRecord[95].GetUInt32();
    RewSpell = questRecord[96].GetUInt32();
    PointMapId = questRecord[97].GetUInt32();
    PointX = questRecord[98].GetFloat();
    PointY = questRecord[99].GetFloat();
    PointOpt = questRecord[100].GetUInt32();

    for (int i = 0; i < QUEST_EMOTE_COUNT; ++i)
        DetailsEmote[i] = questRecord[101+i].GetUInt32();

    IncompleteEmote = questRecord[105].GetUInt32();
    CompleteEmote = questRecord[106].GetUInt32();

    for (int i = 0; i < QUEST_EMOTE_COUNT; ++i)
        OfferRewardEmote[i] = questRecord[107+i].GetInt32();

    QuestStartScript = questRecord[111].GetUInt32();
    QuestCompleteScript = questRecord[112].GetUInt32();

    QuestFlags |= SpecialFlags << 16;

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

uint32 Quest::XPValue( Player *pPlayer ) const
{
    if( pPlayer )
    {
        if( RewXpOrMoney > 0 )
        {
            uint32 pLevel = pPlayer->getLevel();
            uint32 qLevel = QuestLevel;
            float fullxp = 0;
            if (qLevel >= 15)
                fullxp = RewXpOrMoney / 6.0f;
            else if (qLevel == 14)
                fullxp = RewXpOrMoney / 4.8f;
            else if (qLevel == 13)
                fullxp = RewXpOrMoney / 3.6f;
            else if (qLevel == 12)
                fullxp = RewXpOrMoney / 2.4f;
            else if (qLevel == 11)
                fullxp = RewXpOrMoney / 1.2f;
            else if (qLevel > 0 && qLevel <= 10)
                fullxp = RewXpOrMoney / 0.6f;

            if( pLevel <= qLevel +  5 )
                return (uint32)fullxp;
            else if( pLevel == qLevel +  6 )
                return (uint32)(fullxp * 0.8f);
            else if( pLevel == qLevel +  7 )
                return (uint32)(fullxp * 0.6f);
            else if( pLevel == qLevel +  8 )
                return (uint32)(fullxp * 0.4f);
            else if( pLevel == qLevel +  9 )
                return (uint32)(fullxp * 0.2f);
            else
                return (uint32)(fullxp * 0.1f);
        }
    }
    return 0;
}
