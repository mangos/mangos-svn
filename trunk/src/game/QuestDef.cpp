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

#include "QuestDef.h"
#include "ObjectMgr.h"

Quest::Quest()
{
    m_quest = new QuestInfo;
    m_reqitemscount = 0;
    m_reqmobscount = 0;
    m_rewchoiceitemscount = 0;
    m_rewitemscount = 0;
}

void Quest::LoadQuest( uint32 quest )
{
    QuestInfo *pQuestInfo = objmgr.GetQuestInfo( quest );
    if( pQuestInfo )
        LoadQuest( pQuestInfo );
}

void Quest::LoadQuest( QuestInfo *pQuestInfo )
{
    if( pQuestInfo )
    {
        m_quest = pQuestInfo;
        m_reqitemscount = 0;
        m_reqmobscount = 0;
        m_rewitemscount = 0;
        m_rewchoiceitemscount = 0;

        for (int i=0; i < QUEST_OBJECTIVES_COUNT; i++)
        {
            if ( pQuestInfo->ReqItemId[i] )
                m_reqitemscount++;
            if ( pQuestInfo->ReqKillMobId[i] )
                m_reqmobscount++;
        }

        for (int i=0; i < QUEST_REWARD_CHOICES_COUNT; i++)
        {
            if ( pQuestInfo->RewChoiceItemId[i] )
                m_rewchoiceitemscount++;
        }

        for (int i=0; i < QUEST_REWARDS_COUNT; i++)
        {
            if ( pQuestInfo->RewItemId[i] )
                m_rewitemscount++;
        }
    }
}

uint32 Quest::XPValue( Player *pPlayer )
{
    if( pPlayer )
    {
        uint32 fullxp = GetQuestInfo()->RewXP;
        if( fullxp > 0 )
        {
            uint32 pLevel = pPlayer->getLevel();
            uint32 qLevel = GetQuestInfo()->QuestLevel;

            if( pLevel <= qLevel +  5 )
                return fullxp;
            else if( pLevel == qLevel +  6 )
                return (uint32)(fullxp * 0.8);
            else if( pLevel == qLevel +  7 )
                return (uint32)(fullxp * 0.6);
            else if( pLevel == qLevel +  6 )
                return (uint32)(fullxp * 0.4);
            else if( pLevel == qLevel +  6 )
                return (uint32)(fullxp * 0.2);
            else
                return (uint32)(fullxp * 0.1);
        }
    }
    return 0;
}
