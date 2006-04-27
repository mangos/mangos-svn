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

void WorldSession::HandleQuestgiverStatusQueryOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    recv_data >> guid;
    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_STATUS_QUERY NpcGUID=%u",uint32(GUID_LOPART(guid)) );

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if (!pCreature)
    {
        sLog.outError( "WORLD: received incorrect guid in CMSG_QUESTGIVER_STATUS_QUERY" );
        return;
    }

    uint32 questStatus = Script->NPCDialogStatus(_player, pCreature );
    if( questStatus > 6 )
    {
        uint32 defstatus=DIALOG_STATUS_CHAT;
        if(pCreature->isQuestGiver())
            defstatus=DIALOG_STATUS_NONE;
        questStatus = pCreature->getDialogStatus(_player, defstatus);
    }
    _player->PlayerTalkClass->SendQuestStatus(questStatus, guid);
}

void WorldSession::HandleQuestgiverHelloOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    recv_data >> guid;
    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_HELLO guid=%u",guid );
    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if(!pCreature)
    {
        sLog.outError( "WORLD: Received incorrect guid in CMSG_QUESTGIVER_HELLO" );
        return;
    }

    if(!(Script->GossipHello( _player, pCreature )))
    {
        pCreature->prepareQuestMenu( _player );
        pCreature->sendPreparedQuest( _player );
    }
}

void WorldSession::HandleQuestgiverAcceptQuestOpcode( WorldPacket & recv_data )
{

    uint64 guid;
    uint32 quest_id;

    recv_data >> guid >> quest_id;
    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_ACCEPT_QUEST guid=%u, questid=%u",uint32(GUID_LOPART(guid)),quest_id );
    Quest *pQuest = objmgr.GetQuest(quest_id);
    if (!pQuest)
        return;

    if ( (_player->m_timedQuest) && pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_TIMED))
    {
        _player->PlayerTalkClass->SendQuestInvalid( INVALIDREASON_HAVE_TIMED_QUEST );
        return;
    }

    if(_player->getQuestStatus(pQuest->GetQuestInfo()->QuestId)==QUEST_STATUS_NONE)
        _player->addNewQuest(pQuest,QUEST_STATUS_INCOMPLETE);
    uint16 log_slot = _player->getOpenQuestSlot();
    if (log_slot == 0)
    {
        _player->PlayerTalkClass->SendQuestLogFull();
        return;
    }
    _player->SetUInt32Value(log_slot + 0, quest_id);
    _player->SetUInt32Value(log_slot + 1, 0);
    _player->SetUInt32Value(log_slot + 2, 0);

    sLog.outDebug( "WORLD: Sent Quest Acceptance" );

    _player->setQuestStatus(quest_id, QUEST_STATUS_INCOMPLETE, false);

    if ( _player->isQuestComplete(pQuest) )
        _player->PlayerTalkClass->SendQuestCompleteToLog( pQuest );

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (!pCreature)
    {
        uint32 islot = _player->GetSlotByItemGUID( guid );
        Item *pItem;

        if (islot)
            pItem = _player->GetItemBySlot( (uint8)islot );

        if (!islot || !pItem)
        {
            GameObject *pGO = ObjectAccessor::Instance().GetGameObject(*_player, guid);
            if(pGO)
                Script->GOQuestAccept(_player, pGO, pQuest );
        }
        else
            Script->ItemQuestAccept(_player, pItem, pQuest );
    }
    else
        Script->QuestAccept(_player, pCreature, pQuest );
    _player->PlayerTalkClass->CloseGossip();
    if(!(Script->GossipHello( _player, pCreature )))
    {
        pCreature->prepareQuestMenu( _player );
        pCreature->sendPreparedQuest( _player );
    }
    //_player->SaveToDB();
}

void WorldSession::HandleQuestgiverQuestQueryOpcode( WorldPacket & recv_data )
{

    uint64 guid,guid1;
    uint32 quest_id = 0;
    recv_data >> guid >> quest_id;
    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_QUERY_QUEST guid=%u, quest_id=%u",uint32(GUID_LOPART(guid)),quest_id );

    Quest *pQuest = objmgr.GetQuest(quest_id);

    if (!pQuest)
    {
        sLog.outError("Invalid Quest ID (or not in the ObjMgr) '%d' received from _player.", quest_id);
        return;
    }

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
    bool usesc=false;
    if (!pCreature)
    {
        uint32 islot = _player->GetSlotByItemGUID( guid );
        Item *pItem;
        if (islot)
            pItem = _player->GetItemBySlot( (uint8)islot );

        if (!islot || !pItem)
        {
            GameObject *pGO = ObjectAccessor::Instance().GetGameObject(*_player, guid);

            if(pGO)
            {
                if(!(usesc=Script->GOHello(_player, pGO )))
                    guid1=pGO->GetGUID();
            }
        }
        if(!(usesc=Script->ItemHello(_player, pItem, pQuest )))
            guid1=pItem->GetGUID();
    }
    else
    {
        if(!(usesc=Script->QuestSelect(_player, pCreature, pQuest )))
            guid1=pCreature->GetGUID();
    }
    if(!usesc)
    {
        uint32 status=_player->getQuestStatus(quest_id);
        if(status==QUEST_STATUS_COMPLETE && !_player->getQuestRewardStatus(quest_id))
            _player->PlayerTalkClass->SendQuestReward( pQuest, guid1, true, NULL, 0 );
        else if(status==QUEST_STATUS_INCOMPLETE)
            _player->PlayerTalkClass->SendUpdateQuestDetails( pQuest );
        else
            _player->PlayerTalkClass->SendQuestDetails(pQuest,guid1,true);
    }
}

void WorldSession::HandleQuestQueryOpcode( WorldPacket & recv_data )
{
    uint32 quest_id = 0;
    recv_data >> quest_id;
    sLog.outDebug( "WORLD: Received CMSG_QUEST_QUERY questid=%u",quest_id );

    Quest *pQuest = objmgr.GetQuest(quest_id);

    if (!pQuest) return;
    _player->PlayerTalkClass->SendUpdateQuestDetails( pQuest );
}

void WorldSession::HandleQuestgiverChooseRewardOpcode( WorldPacket & recv_data )
{
    unsigned int iI;
    uint32 quest_id, rewardid;
    uint64 guid1;
    recv_data >> guid1 >> quest_id >> rewardid;
    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_CHOOSE_REWARD guid=%u, questid=%u, rewardid=%u",uint32(GUID_LOPART(guid1)),quest_id,rewardid );

    Quest *pQuest = objmgr.GetQuest(quest_id);

    if (!pQuest) return;

    if (pQuest->GetQuestInfo()->RewMoney < 0)
        if ( !( (_player->GetMoney() - pQuest->GetQuestInfo()->RewMoney) >= 0) )
    {
        _player->PlayerTalkClass->SendQuestInvalid( INVALIDREASON_DONT_HAVE_REQ_MONEY );
        return;
    }

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; iI++ )
    {
        if ( pQuest->GetQuestInfo()->ReqItemId[iI] > 0 )
        {
            if (_player->GetItemCount( pQuest->GetQuestInfo()->ReqItemId[iI], false)< pQuest->GetQuestInfo()->ReqItemCount[iI])
            {
                _player->PlayerTalkClass->SendQuestInvalid( INVALIDREASON_DONT_HAVE_REQ_ITEMS );
                return;
            }
        }
    }

    if ( ( rewardid >= pQuest->m_qRewChoiceItemsCount ) && ( pQuest->m_qRewChoiceItemsCount > 0 ) )
    {
        sLog.outString("WORLD: Attempt to select an unexisting rewardid !");
        return;
    }

    if ( pQuest->m_qRewChoiceItemsCount > 0 )
    {
        if  (_player->CanAddItemCount(pQuest->GetQuestInfo()->RewChoiceItemId[rewardid], pQuest->GetQuestInfo()->RewChoiceItemCount[rewardid])>=(int)pQuest->m_qRewChoiceItemsCount)
            _player->AddNewItem(pQuest->GetQuestInfo()->RewChoiceItemId[rewardid],pQuest->GetQuestInfo()->RewChoiceItemCount[rewardid], true);
        else
        {
            _player->PlayerTalkClass->SendQuestFailed( FAILEDREASON_INV_FULL );
            return;
        }
    }

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; iI++ )
        if ( pQuest->GetQuestInfo()->ReqItemId[iI] )
            _player->RemovItemFromBag( pQuest->GetQuestInfo()->ReqItemId[iI], pQuest->GetQuestInfo()->ReqItemCount[iI]);

    if ( pQuest->GetQuestInfo()->RewSpell > 0 )
    {
        WorldPacket sdata;

        _player->addSpell( pQuest->GetQuestInfo()->RewSpell );

        sdata.Initialize (SMSG_LEARNED_SPELL);
        sdata << pQuest->GetQuestInfo()->RewSpell;
        SendPacket( &sdata );
    }

    _player->PlayerTalkClass->SendQuestUpdateComplete( pQuest );
    _player->PlayerTalkClass->SendQuestComplete( pQuest );
    uint16 log_slot = _player->getQuestSlot(quest_id);
    _player->SetUInt32Value(log_slot+0, 0);
    _player->SetUInt32Value(log_slot+1, 0);
    _player->SetUInt32Value(log_slot+2, 0);
    _player->GiveXP( pQuest->XPValue( _player ), guid1 );
    _player->ModifyMoney( pQuest->GetQuestInfo()->RewMoney );

    _player->setQuestStatus(quest_id, QUEST_STATUS_COMPLETE, true);
    //_player->SaveToDB();

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid1);
    GameObject *pGameObject = ObjectAccessor::Instance().GetGameObject(*_player, guid1);

    if (pCreature)
    {
        if(!(Script->ChooseReward( _player, pCreature, pQuest, rewardid )))
        {
            Quest* nextquest;
            if(nextquest=pCreature->getNextAvailableQuest(_player,pQuest))
                _player->PlayerTalkClass->SendQuestDetails(nextquest,pCreature->GetGUID(),true);
            else
                _player->PlayerTalkClass->CloseGossip();
        }
    }
    else if (pGameObject)
        Script->GOChooseReward( _player, pGameObject, pQuest, rewardid );
}

void WorldSession::HandleQuestgiverRequestRewardOpcode( WorldPacket & recv_data )
{
    uint32 quest_id;
    uint64 guid;
    recv_data >> guid >> quest_id;
    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_REQUEST_REWARD guid=%u, questid=%u",uint32(GUID_LOPART(guid)),quest_id );

    Quest *pQuest       = objmgr.GetQuest( quest_id );
    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (!pQuest)
    {
        sLog.outError("Invalid Quest ID (or not in the ObjMgr) '%d' received from _player.", quest_id);
        return;
    }

    if (!pCreature)
    {
        sLog.outError("Invalid NPC GUID (or not in the ObjMgr) '%d' received from _player.", guid);
        return;
    }

    if ( _player->isQuestComplete( pQuest ) )
        _player->PlayerTalkClass->SendQuestReward( pQuest, guid, true, NULL, 0);
}

void WorldSession::HandleQuestgiverCancel(WorldPacket& recv_data )
{
    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_CANCEL" );

    _player->PlayerTalkClass->CloseGossip();
}

void WorldSession::HandleQuestLogSwapQuest(WorldPacket& recv_data )
{
    uint8 slot_id1, slot_id2;

    recv_data >> slot_id1 >> slot_id2;
    sLog.outString( "WORLD: Received CMSG_QUESTLOG_SWAP_QUEST slotid1=%u,slotid2=%u",slot_id1,slot_id2 );

    uint16 log_slot1 = _player->getQuestSlotById( slot_id1 );
    uint16 log_slot2 = _player->getQuestSlotById( slot_id2 );

    uint32 temp1, temp2;

    for (int iCx = 0; iCx < 3; iCx++ )
    {
        temp1 = _player->GetUInt32Value(log_slot1 + iCx);
        temp2 = _player->GetUInt32Value(log_slot2 + iCx);

        _player->SetUInt32Value(log_slot1 + iCx, temp2);
        _player->SetUInt32Value(log_slot2 + iCx, temp1);
    }

    //_player->SaveToDB();
}

void WorldSession::HandleQuestLogRemoveQuest(WorldPacket& recv_data)
{
    uint8 slot_id;
    uint32 quest_id;

    recv_data >> slot_id;
    sLog.outString( "WORLD: Received CMSG_QUESTLOG_REMOVE_QUEST slotid=%u",slot_id );
    slot_id++;

    uint16 log_slot = _player->getQuestSlotById( slot_id );
    quest_id = _player->GetUInt32Value(log_slot + 0);

    if ( ( _player->getQuestStatus(quest_id) != QUEST_STATUS_COMPLETE ) &&
        ( _player->getQuestStatus(quest_id) != QUEST_STATUS_INCOMPLETE ) )
    {
        _player->SetUInt32Value(log_slot + 0, 0);
        _player->SetUInt32Value(log_slot + 1, 0);
        _player->SetUInt32Value(log_slot + 2, 0);
        sLog.outError("Trying to remove an invalid quest '%d' from log.", quest_id);
        return;
    }
    _player->SetUInt32Value(log_slot + 0, 0);
    _player->SetUInt32Value(log_slot + 1, 0);
    _player->SetUInt32Value(log_slot + 2, 0);
    _player->setQuestStatus( quest_id, QUEST_STATUS_AVAILABLE, false);

    //_player->SaveToDB();
}

void WorldSession::HandleQuestConfirmAccept(WorldPacket& recv_data)
{
    uint32 quest_id;
    recv_data >> quest_id;
    sLog.outString( "WORLD: Received CMSG_QUEST_CONFIRM_ACCEPT questid=%u",quest_id );

    Quest *pQuest = objmgr.GetQuest( quest_id );

    if (!pQuest)
    {
        sLog.outError("Invalid Quest ID (or not in the ObjMgr) '%d' received from _player.", quest_id);
        return;
    }
}

void WorldSession::HandleQuestComplete(WorldPacket& recv_data)
{
    uint32 quest_id;
    uint64 guid;

    recv_data >> guid >> quest_id;
    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_COMPLETE_QUEST guid=%u, quest_id=%u",uint32(GUID_LOPART(guid)),quest_id );

    Quest *pQuest = objmgr.GetQuest( quest_id );
    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (!pQuest)
    {
        sLog.outError("Invalid Quest ID (or not in the ObjMgr) '%d' received from _player.", quest_id);
        return;
    }
    if(_player->getQuestStatus(quest_id)!=QUEST_STATUS_COMPLETE)
    {
        _player->PlayerTalkClass->SendUpdateQuestDetails( pQuest );
        return;
    }

    int points = 0;
    if( _player->getLevel() < pQuest->GetQuestInfo()->MinLevel + 6 )
    {
        points = 25;
    }
    else
    {
        int diff = _player->getLevel() - pQuest->GetQuestInfo()->MinLevel;
        points = 25 - (5*(diff-5));
        if(points < 5) points = 5;
    }
    _player->SetStanding(pCreature->getFaction(), points);

    if(!(Script->QuestComplete(_player, pCreature, pQuest )))
    {
        _player->PlayerTalkClass->SendQuestComplete(pQuest);
        _player->setQuestStatus(quest_id, QUEST_STATUS_COMPLETE, true);
        _player->ModifyMoney( pQuest->GetQuestInfo()->RewMoney );
        _player->GiveXP( pQuest->XPValue( _player ), guid );
        if ( pQuest->GetQuestInfo()->RewSpell > 0 )
        {
            WorldPacket sdata;

            _player->addSpell( pQuest->GetQuestInfo()->RewSpell );

            sdata.Initialize (SMSG_LEARNED_SPELL);
            sdata << pQuest->GetQuestInfo()->RewSpell;
            SendPacket( &sdata );
        }

        uint16 log_slot = _player->getQuestSlot(quest_id);
        _player->SetUInt32Value(log_slot+0, 0);
        _player->SetUInt32Value(log_slot+1, 0);
        _player->SetUInt32Value(log_slot+2, 0);
        //_player->SaveToDB();
        Quest* nextquest;
        if(nextquest=pCreature->getNextAvailableQuest(_player,pQuest))
            _player->PlayerTalkClass->SendQuestDetails(nextquest,pCreature->GetGUID(),true);
        else
            _player->PlayerTalkClass->CloseGossip();
    }
}

void WorldSession::HandleQuestAutoLaunch(WorldPacket& recvPacket)
{
    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_QUEST_AUTOLAUNCH (Unhandled!)" );

}
