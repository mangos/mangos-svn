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
	
	

	m_qId = 0;
}



uint32 Quest::XPValue(Player* _Player)
{
	
	
	

	return 100;
}



uint32 Quest::GetKillObjectivesCount()
{
	uint32 i, count = 0;

	for (i=0; i < QUEST_OBJECTIVES_COUNT; i++) 
		if (m_qObjMobId[i]) count++;

	return count;
}



uint32 Quest::GetDeliverObjectivesCount()
{
	uint32 i, count = 0;

	for (i=0; i < QUEST_OBJECTIVES_COUNT; i++) 
		if (m_qObjItemId[i]) count++;

	return count;
}




bool Quest::CanBeTaken( Player *_Player )
{
	bool result = ( !RewardIsTaken( _Player ) && IsCompatible( _Player ) && 
					 PreReqSatisfied( _Player ) );

	return result;
}


bool Quest::RewardIsTaken( Player *_Player )
{
	bool bResult = false;
	if ( !HasFlag(QUEST_SPECIAL_FLAGS_REPEATABLE) ) 
		bResult = ( _Player->getQuestRewardStatus( m_qId ) );

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
	if ( m_qRequiredRaces == QUEST_RACE_NONE ) return true;
	return (((m_qRequiredRaces >> (_Player->getRace() - 1)) & 0x01) == 0x01);
}




bool Quest::ClassSatisfied( Player *_Player )
{
	if ( m_qRequiredClass == QUEST_CLASS_NONE ) return true;
	return (m_qRequiredClass == _Player->getClass());
}




bool Quest::LevelSatisfied( Player *_Player )
{
	return ( _Player->getLevel() >= m_qPlayerLevel );
}




bool Quest::CanShowAvailable( Player *_Player )
{
	uint8 iPLevel;
	iPLevel = _Player->getLevel();

	if ( iPLevel < m_qPlayerLevel ) return false;
	return ( (iPLevel - m_qPlayerLevel) <= 7 );
}




bool Quest::CanShowUnsatified( Player *_Player )
{
	uint8 iPLevel;
	iPLevel = _Player->getLevel();

	if ( iPLevel > m_qPlayerLevel ) return false;
	return ( (m_qPlayerLevel - iPLevel) <= 7 );
}




bool Quest::PreReqSatisfied( Player *_Player )
{
	bool bResult = true;

	if ( m_qRequiredQuestsCount > 0 )
	{
		bResult = false;
		for (uint32 iI = 0; iI < m_qRequiredQuestsCount; iI++ )
			bResult |= _Player->getQuestRewardStatus( m_qRequiredQuests[iI] );
	}

	if (!bResult) return false;

	if ( m_qRequiredAbsQuestsCount > 0 )
	{
		for (uint32 iI = 0; iI < m_qRequiredAbsQuestsCount; iI++ )
			bResult &= _Player->getQuestRewardStatus( m_qRequiredAbsQuests[iI] );
	}

	if (!bResult) return false;

	if ( m_qLockerQuestsCount > 0 )
	{
		for (uint32 iI = 0; iI < m_qLockerQuestsCount; iI++ )
		{
			bResult &= (!_Player->getQuestRewardStatus( m_qLockerQuests[iI] ));
			bResult &= ( _Player->getQuestStatus( m_qLockerQuests[iI] ) != QUEST_STATUS_COMPLETE);
			bResult &= ( _Player->getQuestStatus( m_qLockerQuests[iI] ) != QUEST_STATUS_INCOMPLETE);
			
		}
	}

	return bResult;
}
