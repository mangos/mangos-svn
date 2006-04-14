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
#include "ItemPrototype.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"

Quest::Quest()
{
	m_quest=new QuestInfo;
	m_qReqItemsCount=0;
	m_qReqMobsCount=0;
	m_qRewChoiceItemsCount=0;
	m_qRewItemsCount=0;
}

void Quest::LoadQuest(uint32 quest_id)
{
	QuestInfo *qi=objmgr.GetQuestInfo(quest_id);
	if(qi!=NULL)
		LoadQuest(qi);
}

void Quest::LoadQuest(QuestInfo *questinfo)
{
	if(!questinfo)
		return;
	m_quest=questinfo;
	m_qReqItemsCount=0;
	m_qReqMobsCount=0;
	m_qRewItemsCount=0;
	m_qRewChoiceItemsCount=0;
	for (int i=0; i < QUEST_REWARD_CHOICES_COUNT; i++) 
	{
		if(i<QUEST_OBJECTIVES_COUNT)
		{
			if (questinfo->ReqItemId[i]) 
				m_qReqItemsCount++;
			if (questinfo->ReqKillMobId[i]) 
				m_qReqMobsCount++;
			if (questinfo->RewItemId[i]) 
				m_qRewItemsCount++;
		}
		if ( questinfo->RewChoiceItemId[i]) 
			m_qRewChoiceItemsCount++;
	}
}


uint32 Quest::XPValue(Player* _Player)
{
	if(GetQuestInfo()->RewXP<=0)
		return 100;
	return (uint32)(GetQuestInfo()->RewXP*(1.0+((double)GetQuestInfo()->MaxLevel+(double)GetQuestInfo()->MinLevel-(double)_Player->getLevel()*2.0)*0.05));
}

bool Quest::CanBeTaken( Player *_Player )
{
	bool result = ( !RewardIsTaken( _Player ) && IsCompatible( _Player ) &&
					LevelSatisfied( _Player ) && PreReqSatisfied( _Player ) );
	return result;
}


bool Quest::RewardIsTaken( Player *_Player )
{
	bool bResult = false;
	if ( !HasFlag(QUEST_SPECIAL_FLAGS_REPEATABLE) ) 
		bResult = ( _Player->getQuestRewardStatus( GetQuestInfo()->QuestId ) );
	return bResult;
}

bool Quest::IsCompatible( Player *_Player )
{
	return ( ReputationSatisfied ( _Player ) &&
		     RaceSatisfied       ( _Player ) &&
			 ClassSatisfied      ( _Player ) &&
			 TradeSkillSatisfied ( _Player ) );
}

bool Quest::ReputationSatisfied( Player *_Player )
{
	return true;
}

bool Quest::TradeSkillSatisfied( Player *_Player )
{
	return true;
}

bool Quest::RaceSatisfied( Player *_Player )
{
	if ( GetQuestInfo()->RequiredRaces == QUEST_RACE_NONE ) return true;
	return (((GetQuestInfo()->RequiredRaces >> (_Player->getRace() - 1)) & 0x01) == 0x01);
}

bool Quest::ClassSatisfied( Player *_Player )
{
	if ( GetQuestInfo()->RequiredClass == QUEST_CLASS_NONE ) return true;
	return (GetQuestInfo()->RequiredClass == _Player->getClass());
}

bool Quest::LevelSatisfied( Player *_Player )
{
	uint32 plevel=_Player->getLevel();
	return ( plevel >= GetQuestInfo()->MinLevel && plevel <= GetQuestInfo()->MaxLevel );
}

bool Quest::CanShowUnsatified( Player *_Player )
{
	uint8 iPLevel;
	iPLevel = _Player->getLevel();
	return iPLevel>=GetQuestInfo()->MinLevel -7 && iPLevel<GetQuestInfo()->MinLevel ;
}

bool Quest::PreReqSatisfied( Player *_Player )
{
	if (GetQuestInfo()->PrevQuestId > 0  && !_Player->getQuestRewardStatus(  GetQuestInfo()->PrevQuestId ))
		return false;
	return true;
}
