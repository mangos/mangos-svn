/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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

#include "Common.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QuestDef.h"
#include "ObjectAccessor.h"
#include "ScriptCalls.h"
#include "ScriptCalls.cpp"


void WorldSession::HandleQuestgiverStatusQueryOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Received CMSG_QUESTGIVER_STATUS_QUERY" );
    uint64 guid;
    recv_data >> guid;

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if (!pCreature)
    {
        Log::getSingleton( ).outError( "WORLD: received incorrect guid in CMSG_QUESTGIVER_STATUS_QUERY" );
        return;
    }

    uint32 questStatus = scriptCallNPCDialogStatus(GetPlayer(), pCreature );    
    GetPlayer()->PlayerTalkClass->SendQuestStatus(questStatus, guid);
}


void WorldSession::HandleQuestgiverHelloOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Received CMSG_QUESTGIVER_HELLO" );

    uint64 guid;
    recv_data >> guid;
    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if(!pCreature)
    {
        Log::getSingleton( ).outError( "WORLD: Received incorrect guid in CMSG_QUESTGIVER_HELLO" );
        return;
    }
    
    scriptCallGossipHello( GetPlayer(), pCreature );
}


void WorldSession::HandleQuestgiverAcceptQuestOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Received CMSG_QUESTGIVER_ACCEPT_QUEST" );

    uint64 guid;
    uint32 quest_id;

    recv_data >> guid >> quest_id;

    Quest *pQuest = objmgr.GetQuest(quest_id);
	if (!pQuest)
		return;

	if ( (GetPlayer()->m_timedQuest) && pQuest->HasFlag(QUEST_SPECIAL_FLAGS_TIMED))
	{
		GetPlayer()->PlayerTalkClass->SendQuestInvalid( INVALIDREASON_HAVE_TIMED_QUEST );
		return;
	}

	if (pQuest->m_qQuestItem)
	 if ( !GetPlayer()->AddItemToBackpack( pQuest->m_qQuestItem ) )
	 {
		 GetPlayer()->PlayerTalkClass->SendQuestFailed( FAILEDREASON_INV_FULL );
		 return; 
	 }

    uint16 log_slot = GetPlayer()->getOpenQuestSlot();
    if (log_slot == 0)
    {
        GetPlayer()->PlayerTalkClass->SendQuestLogFull();
        return;
    }

    GetPlayer()->SetUInt32Value(log_slot + 0, quest_id);
    GetPlayer()->SetUInt32Value(log_slot + 1, 0);
    GetPlayer()->SetUInt32Value(log_slot + 2, 0);

    Log::getSingleton( ).outDebug( "WORLD: Sent Quest Acceptance" );

    GetPlayer()->setQuestStatus(quest_id, QUEST_STATUS_INCOMPLETE, false);

	if ( GetPlayer()->checkQuestStatus(quest_id) )
	{
		GetPlayer()->PlayerTalkClass->SendQuestUpdateComplete( pQuest );
		GetPlayer()->setQuestStatus(quest_id, QUEST_STATUS_COMPLETE, false);
	}

	Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, GUID_LOPART(guid));

	if (!pCreature)
	{
		uint32 islot = GetPlayer()->GetSlotByItemGUID( guid );
		Item *pItem;

		if (islot)
			pItem = GetPlayer()->GetItemBySlot( (uint8)islot );

		if (!islot || !pItem)
		{
			GameObject *pGO = ObjectAccessor::Instance().GetGameObject(*_player, GUID_LOPART(guid));
			ASSERT(pGO);

	        scriptCallGOQuestAccept( GetPlayer(), pGO, pQuest );
			return;
		}

		scriptCallItemQuestAccept( GetPlayer(), pItem, pQuest );
		return;
	}

	scriptCallQuestAccept( GetPlayer(), pCreature, pQuest );

	GetPlayer()->SaveToDB();
}


void WorldSession::HandleQuestgiverQuestQueryOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Received CMSG_QUESTGIVER_QUERY_QUEST" );

	uint64 guid;
    uint32 quest_id = 0;
    recv_data >> guid >> quest_id;

    Quest *pQuest = objmgr.GetQuest(quest_id);

	if (!pQuest)
	{
		Log::getSingleton( ).outError("Invalid Quest ID (or not in the ObjMgr) '%d' received from player.", quest_id);
		return;
	}


	Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, GUID_LOPART(guid));
	if (!pCreature)
	{
		uint32 islot = GetPlayer()->GetSlotByItemGUID( guid );
		Item *pItem;

		if (islot)
			pItem = GetPlayer()->GetItemBySlot( (uint8)islot );

		if (!islot || !pItem)
		{
			GameObject *pGO = ObjectAccessor::Instance().GetGameObject(*_player, GUID_LOPART(guid));
			ASSERT(pGO);

	        scriptCallGOHello( GetPlayer(), pGO );
			return;
		}

		scriptCallItemHello( GetPlayer(), pItem, pQuest );
		return;
	}

	scriptCallQuestSelect( GetPlayer(), pCreature, pQuest );
}

void WorldSession::HandleQuestQueryOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Received CMSG_QUEST_QUERY" );

    uint32 quest_id = 0;
    recv_data >> quest_id;

    Quest *pQuest = objmgr.GetQuest(quest_id);
    if (!pQuest) return;
    GetPlayer()->PlayerTalkClass->SendUpdateQuestDetails( pQuest );
}



void WorldSession::HandleQuestgiverChooseRewardOpcode( WorldPacket & recv_data )
{
    
    Log::getSingleton( ).outString( "WORLD: Received CMSG_QUESTGIVER_CHOOSE_REWARD" );

	unsigned int iI;
    uint32 quest_id, rewardid;
	uint64 guid1;
    recv_data >> guid1 >> quest_id >> rewardid;

    Quest *pQuest = objmgr.GetQuest(quest_id);

    
    Player *chr = GetPlayer();

	if (!pQuest) return;

	

	if (pQuest->m_qRewMoney < 0) 
		if ( !( (chr->GetMoney() - pQuest->m_qRewMoney) >= 0) )
		{
			GetPlayer()->PlayerTalkClass->SendQuestInvalid( INVALIDREASON_DONT_HAVE_REQ_MONEY );
			return;
		}

	

	for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; iI++ )
	{
		if ( pQuest->m_qObjItemId[iI] > 0 ) 
			if (!chr->HasItemInBackpack( pQuest->m_qObjItemId[iI], pQuest->m_qObjItemCount[iI])) 
			{
				GetPlayer()->PlayerTalkClass->SendQuestInvalid( INVALIDREASON_DONT_HAVE_REQ_ITEMS );
				return;
			}
	}

	

	for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; iI++ )
		if ( pQuest->m_qObjItemId[iI] ) chr->RemoveItemFromBackpack( pQuest->m_qObjItemId[iI], pQuest->m_qObjItemCount[iI]);

	if (pQuest->m_qRewMoney < 0) 
		chr->ModifyMoney(pQuest->m_qRewMoney);

	

	if ( ( rewardid >= pQuest->m_qRewChoicesCount ) && ( pQuest->m_qRewChoicesCount > 0 ) )
	{
		Log::getSingleton().outString("WORLD: Attempt to select an unexisting rewardid !");
		return;
	}

	GetPlayer()->PlayerTalkClass->SendQuestUpdateComplete( pQuest );
	GetPlayer()->PlayerTalkClass->SendQuestComplete( pQuest );

    GetPlayer()->setQuestStatus(quest_id, QUEST_STATUS_COMPLETE, true);

    uint16 log_slot = GetPlayer()->getQuestSlot(quest_id);

	if (pQuest->m_qRewMoney > 0) chr->ModifyMoney( pQuest->m_qRewMoney );

	

	bool bkFull = false;
	for ( int iI = 0; iI < pQuest->m_qRewCount; iI++ )
		if (!GetPlayer()->AddItemToBackpack( pQuest->m_qRewItemId[iI], pQuest->m_qRewItemCount[iI] )) 
		{ 
			bkFull = true; 
			break; 
		}

	

	if ( pQuest->m_qRewChoicesCount > 0 )
	{
		if  (!bkFull) 
			GetPlayer()->AddItemToBackpack( pQuest->m_qRewChoicesItemId[rewardid], pQuest->m_qRewChoicesItemCount[rewardid] ); else
			GetPlayer()->PlayerTalkClass->SendQuestFailed( FAILEDREASON_INV_FULL );
	}

	

	if ( pQuest->m_qRewSpell > 0 ) 
	{
		WorldPacket sdata;

		GetPlayer()->addSpell( pQuest->m_qRewSpell );

		sdata.Initialize (SMSG_LEARNED_SPELL);
		sdata << pQuest->m_qRewSpell;
		SendPacket( &sdata );
	}

	
    chr->SetUInt32Value(log_slot+0, 0);
    chr->SetUInt32Value(log_slot+1, 0);
    chr->SetUInt32Value(log_slot+2, 0);

	chr->GiveXP( pQuest->XPValue( chr ), guid1 );

	chr->setQuestStatus( quest_id, QUEST_STATUS_AVAILABLE, true); 

	Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, GUID_LOPART(guid1));
	GameObject *pGameObject = ObjectAccessor::Instance().GetGameObject(*_player, GUID_LOPART(guid1));
    
	if (pCreature)
		scriptCallChooseReward(  GetPlayer(), pCreature, pQuest, rewardid ); else
		if (pGameObject)
			scriptCallGOChooseReward(  GetPlayer(), pGameObject, pQuest, rewardid );

	GetPlayer()->SaveToDB();
}


void WorldSession::HandleQuestgiverRequestRewardOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outString( "WORLD: Received CMSG_QUESTGIVER_REQUEST_REWARD" );

    uint32 quest_id;
	uint64 guid;
	recv_data >> guid >> quest_id;

	Quest *pQuest		= objmgr.GetQuest( quest_id );
	Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, GUID_LOPART(guid));

	if (!pQuest)
	{
		Log::getSingleton( ).outError("Invalid Quest ID (or not in the ObjMgr) '%d' received from player.", quest_id);
		return;
	}

	if (!pCreature)
	{
		Log::getSingleton( ).outError("Invalid NPC GUID (or not in the ObjMgr) '%d' received from player.", guid);
		return;
	}

	if ( GetPlayer()->isQuestComplete( quest_id, pCreature ) )
		GetPlayer()->PlayerTalkClass->SendQuestReward( pQuest, guid, true, NULL, 0);
}


void WorldSession::HandleQuestgiverCancel(WorldPacket& recv_data )
{
    Log::getSingleton( ).outString( "WORLD: Received CMSG_QUESTGIVER_CANCEL" );

	
	GetPlayer()->PlayerTalkClass->CloseGossip();
}

void WorldSession::HandleQuestLogSwapQuest(WorldPacket& recv_data )
{
    Log::getSingleton( ).outString( "WORLD: Received CMSG_QUESTLOG_SWAP_QUEST" );

	uint8 slot_id1, slot_id2;

	recv_data >> slot_id1 >> slot_id2;

	uint16 log_slot1 = GetPlayer()->getQuestSlotById( slot_id1 );
	uint16 log_slot2 = GetPlayer()->getQuestSlotById( slot_id2 );

	uint32 temp1, temp2;

	for (int iCx = 0; iCx < 3; iCx++ )
	{
		temp1 = GetPlayer()->GetUInt32Value(log_slot1 + iCx);
		temp2 = GetPlayer()->GetUInt32Value(log_slot2 + iCx);

		GetPlayer()->SetUInt32Value(log_slot1 + iCx, temp2);
		GetPlayer()->SetUInt32Value(log_slot2 + iCx, temp1);
	}

	GetPlayer()->SaveToDB();
}

void WorldSession::HandleQuestLogRemoveQuest(WorldPacket& recv_data)
{
    Log::getSingleton( ).outString( "WORLD: Received CMSG_QUESTLOG_REMOVE_QUEST" );

	uint8 slot_id;
	uint32 quest_id;

	recv_data >> slot_id;
	slot_id++;

	uint16 log_slot = GetPlayer()->getQuestSlotById( slot_id );
	quest_id = GetPlayer()->GetUInt32Value(log_slot + 0);

	if ( ( GetPlayer()->getQuestStatus(quest_id) != QUEST_STATUS_COMPLETE ) &&
		 ( GetPlayer()->getQuestStatus(quest_id) != QUEST_STATUS_INCOMPLETE ) )
		{
			Log::getSingleton( ).outError("Trying to remove an invalid quest '%d' from log.", quest_id);
			return;
		}
	GetPlayer()->SetUInt32Value(log_slot + 0, 0);
	GetPlayer()->SetUInt32Value(log_slot + 1, 0);
	GetPlayer()->SetUInt32Value(log_slot + 2, 0);
	GetPlayer()->setQuestStatus( quest_id, QUEST_STATUS_AVAILABLE, false); 

	GetPlayer()->SaveToDB();
}

void WorldSession::HandleQuestConfirmAccept(WorldPacket& recv_data)
{
    Log::getSingleton( ).outString( "WORLD: Received CMSG_QUEST_CONFIRM_ACCEPT" );

    uint32 quest_id;
	recv_data >> quest_id;

	Quest *pQuest = objmgr.GetQuest( quest_id );

	if (!pQuest)
	{
		Log::getSingleton( ).outError("Invalid Quest ID (or not in the ObjMgr) '%d' received from player.", quest_id);
		return;
	}

	
}

void WorldSession::HandleQuestComplete(WorldPacket& recv_data)
{
    Log::getSingleton( ).outString( "WORLD: Received CMSG_QUESTGIVER_COMPLETE_QUEST" );

    uint32 quest_id;
	uint64 guid;

	recv_data >> guid >> quest_id;

	Quest *pQuest = objmgr.GetQuest( quest_id );
	Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, GUID_LOPART(guid));

	if (!pQuest)
	{
		Log::getSingleton( ).outError("Invalid Quest ID (or not in the ObjMgr) '%d' received from player.", quest_id);
		return;
	}

	
	int points = 0;
	if( GetPlayer()->getLevel() < pQuest->m_qPlayerLevel + 6 )
	{
		points = 25;
	}
	else
	{
		int diff = GetPlayer()->getLevel() - pQuest->m_qPlayerLevel;
		points = 25 - (5*(diff-5));
		if(points < 5) points = 5; 
	}
	GetPlayer()->SetStanding(pCreature->getFaction(), points);
	

	scriptCallQuestComplete( GetPlayer(), pCreature, pQuest );
}

void WorldSession::HandleQuestAutoLaunch(WorldPacket& recvPacket)
{
	Log::getSingleton( ).outString( "WORLD: Received CMSG_QUESTGIVER_QUEST_AUTOLAUNCH (Unhandled!)" );

	
}
