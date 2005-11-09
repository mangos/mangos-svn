/* QuestDef.cpp
 *
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

#include "QuestDef.h"
#include "ObjectMgr.h"
#include "ItemPrototype.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"

Quest::Quest()
{
	// No initialization required as
	// all the quests will be read from the DB anyway.

	m_qId = 0;
}

//--------------------------------------------------------------------
// Returns the amount of XP a player _Player would get by completing this quest.
uint32 Quest::XPValue(Player* _Player)
{
	//
	// TODO: Add a nice formula for this function.
	//

	return 100;
}

//------------------------------------------------------------------
// Returns the number of Kill Objectives
uint32 Quest::GetKillObjectivesCount()
{
	uint32 i;

	for (i=0; i < QUEST_OBJECTIVES_COUNT; i++) 
		if (!m_qObjMobId[i]) return i;

	return QUEST_OBJECTIVES_COUNT;
}

//-------------------------------------------------------------------
// Returns the number of Kill Objectives
uint32 Quest::GetDeliverObjectivesCount()
{
	uint32 i;

	for (i=0; i < QUEST_OBJECTIVES_COUNT; i++) 
		if (!m_qObjItemId[i]) return i;

	return QUEST_OBJECTIVES_COUNT;
}

//--------------------------------------------------------------------
// Send the Quest's details to _Player from senderGUID
// Packet Structure:
//   UINT64 <senderGUID>
//   UINT32 <QuestID>
//   TEXT   <QuestTitle>
//   TEXT   <QuestObjectives>
//   TEXT   <QuestDetails>
//   UINT32 <ActivateAcceptButton> (Not sure if all 32 bits are for that)
//   UINT32 <QuestChoiceRewardCount>
//   |  UINT32 <ItemId>
//   |  UINT32 <ItemCount>
//   |  UINT32 <ItemModel>
//   UINT32 <QuestRewardCount>
//   |  UINT32 <ItemId>
//   |  UINT32 <ItemCount>
//   |  UINT32 <ItemModel>
//   UINT32 <RewardGold>
//   UINT32 <DeliverObjectiveCount>
//   |  UINT32 <ItemId>
//   |  UINT32 <ItemCount>
//   UINT32 <KillObjectiveCount>
//   |  UINT32 <MobId>
//   |  UINT32 <MobCount>
//   ----------------------------- END

void Quest::SendDetails(Player* _Player, uint64 SenderGUID, bool ActivateAccept)
{
	WorldPacket data;
    data.Initialize(SMSG_QUESTGIVER_QUEST_DETAILS);

    data << SenderGUID;
    data << m_qId << m_qTitle << m_qObjectives << m_qDetails << uint32( ActivateAccept );

	//
	// Adding Rewards to the Packet
	//

	ItemPrototype* IProto;
	int i;

	data << m_qRewChoicesCount;
    for (i=0; i < m_qRewChoicesCount; i++)
        {
            data << uint32(m_qRewChoicesItemId[i]);
			data << uint32(m_qRewChoicesItemCount[i]);
			IProto = objmgr.GetItemPrototype(m_qRewChoicesItemId[i]);
			if ( IProto ) data << uint32(IProto->DisplayInfoID); else
						  data << uint32( 0x00 );  // Send NULL model.
        }

	data << m_qRewCount;
    for (i=0; i < m_qRewCount; i++)
        {
            data << m_qRewItemId[i];
			data << m_qRewItemCount[i];
			IProto = objmgr.GetItemPrototype(m_qRewItemId[i]);
			if ( IProto ) data << IProto->DisplayInfoID; else
						  data << uint32( 0x00 );  // Send NULL model.
        }

	data << m_qRewMoney;


	//
	// Sending Deliver Objectives
	//

	uint32 Objs = GetDeliverObjectivesCount();
	data << Objs;

    for (i=0; i < QUEST_OBJECTIVES_COUNT; i++)
        {
			data << m_qObjItemId[i];
			data << m_qObjItemCount[i];
		}

	//
	// Sending Kill Objectives
	//

	Objs = GetKillObjectivesCount();
	data << Objs;
    for (i=0; i < QUEST_OBJECTIVES_COUNT; i++)
        {
			data << m_qObjMobId[i];
			data << m_qObjMobCount[i];
		}

	_Player->GetSession()->SendPacket( &data ); 
    Log::getSingleton( ).outDebug( "WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS" );
}
