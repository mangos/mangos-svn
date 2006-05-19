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

    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_STATUS_QUERY creature = %u",uint32(GUID_LOPART(guid)) );

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if ( pCreature )
    {
        uint32 questStatus = Script->NPCDialogStatus(_player, pCreature);
        if( questStatus > 6 )
        {
            //uint32 defstatus = DIALOG_STATUS_CHAT;
            uint32 defstatus = DIALOG_STATUS_NONE;
            questStatus = pCreature->getDialogStatus(_player, defstatus);
        }
        _player->PlayerTalkClass->SendQuestStatus(questStatus, guid);
    }
}
void WorldSession::HandleQuestgiverHelloOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    recv_data >> guid;

    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_HELLO creature = %u",guid );

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if( pCreature )
    {
        if( !(Script->GossipHello( _player, pCreature )) )
        {
            pCreature->prepareQuestMenu( _player );
            pCreature->sendPreparedQuest( _player );
        }
    }
}
void WorldSession::HandleQuestgiverAcceptQuestOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    uint32 quest_id;
    recv_data >> guid >> quest_id;

    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_ACCEPT_QUEST creature = %u, quest = %u",uint32(GUID_LOPART(guid)),quest_id );

    Quest *pQuest = objmgr.GetQuest(quest_id);
    if ( pQuest )
    {
        uint16 log_slot = _player->getOpenQuestSlot();
        if (log_slot != 0)
        {
            if ( (_player->m_timedQuest) && pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_TIMED) )
            {
                _player->PlayerTalkClass->SendQuestInvalid( INVALIDREASON_HAVE_TIMED_QUEST );
                return;
            }

            _player->GiveQuestSourceItem( pQuest );
            _player->AddQuest( pQuest );

            _player->SetUInt32Value(log_slot + 0, quest_id);
            _player->SetUInt32Value(log_slot + 1, 0);
            _player->SetUInt32Value(log_slot + 2, 0);

            if ( _player->IsQuestComplete(pQuest) )
            {
                _player->PlayerTalkClass->SendQuestCompleteToLog( pQuest );
                _player->SetQuestStatus(pQuest, QUEST_STATUS_COMPLETE);
            }

            Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
            if( pCreature )
                Script->QuestAccept(_player, pCreature, pQuest );
            else
            {
                Item *pItem;
                uint32 slot = _player->GetSlotByItemGUID( guid );
                if ( slot )
                    pItem = _player->GetItemBySlot( (uint8)slot );

                if( pItem )
                    Script->ItemQuestAccept(_player, pItem, pQuest );
                else
                {
                    GameObject *pGO = ObjectAccessor::Instance().GetGameObject(*_player, guid);
                    if( pGO )
                        Script->GOQuestAccept(_player, pGO, pQuest );
                }
            }
        }
    }
    _player->PlayerTalkClass->CloseGossip();
}
void WorldSession::HandleQuestgiverQuestQueryOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    uint32 quest_id;
    recv_data >> guid >> quest_id;

    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_QUERY_QUEST guid = %u, quest = %u",uint32(GUID_LOPART(guid)),quest_id );

    Quest *pQuest = objmgr.GetQuest(quest_id);
    if ( pQuest )
    {
        uint32 status = _player->GetQuestStatus( pQuest );
        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
        if( pCreature )
        {
            if( !Script->QuestSelect(_player, pCreature, pQuest ) )
            {
                if( status == QUEST_STATUS_COMPLETE && !_player->GetQuestRewardStatus( pQuest ) )
                    _player->PlayerTalkClass->SendQuestReward( pQuest, pCreature->GetGUID(), true, NULL, 0 );
                else if( status == QUEST_STATUS_INCOMPLETE )
                    _player->PlayerTalkClass->SendUpdateQuestDetails( pQuest );
                else
                    _player->PlayerTalkClass->SendQuestDetails(pQuest, pCreature->GetGUID(), true);
            }
        }
        else
        {
            Item *pItem;
            uint32 slot = _player->GetSlotByItemGUID( guid );
            if ( slot )
                pItem = _player->GetItemBySlot( (uint8)slot );

            if( pItem )
            {
                if( !Script->ItemQuestAccept(_player, pItem, pQuest ) )
                {
                    if( status == QUEST_STATUS_NONE )
                        _player->PlayerTalkClass->SendQuestDetails(pQuest, pItem->GetGUID(), true);
                }
            }
            else
            {
                GameObject *pGO = ObjectAccessor::Instance().GetGameObject(*_player, guid);
                if( pGO )
                {
                    if( !Script->GOQuestAccept(_player, pGO, pQuest ) )
                    {
                        if( status == QUEST_STATUS_COMPLETE && !_player->GetQuestRewardStatus( pQuest ) )
                            _player->PlayerTalkClass->SendQuestReward( pQuest, pCreature->GetGUID(), true, NULL, 0 );
                        else if( status == QUEST_STATUS_INCOMPLETE )
                            _player->PlayerTalkClass->SendUpdateQuestDetails( pQuest );
                        else
                            _player->PlayerTalkClass->SendQuestDetails(pQuest, pCreature->GetGUID(), true);
                    }
                }
            }
        }
    }
    _player->PlayerTalkClass->CloseGossip();
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
    uint32 quest_id, rewardid;
    uint64 guid1;
    recv_data >> guid1 >> quest_id >> rewardid;

    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_CHOOSE_REWARD guid = %u, quest = %u, rewardid=%u",uint32(GUID_LOPART(guid1)),quest_id,rewardid );

    Quest *pQuest = objmgr.GetQuest(quest_id);
    if( pQuest )
    {
        if ( pQuest->GetQuestInfo()->RewMoney < 0 )
        {
            if ( _player->GetMoney() - pQuest->GetQuestInfo()->RewMoney < 0 )
            {
                _player->PlayerTalkClass->SendQuestInvalid( INVALIDREASON_DONT_HAVE_REQ_MONEY );
                return;
            }
        }

        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++ )
        {
            if ( pQuest->GetQuestInfo()->ReqItemId[i] > 0 )
            {
                if ( _player->GetItemCount( pQuest->GetQuestInfo()->ReqItemId[i], false) < pQuest->GetQuestInfo()->ReqItemCount[i] )
                {
                    _player->PlayerTalkClass->SendQuestInvalid( INVALIDREASON_DONT_HAVE_REQ_ITEMS );
                    return;
                }
            }
        }

        if ( pQuest->m_qRewChoiceItemsCount > 0 && rewardid >= pQuest->m_qRewChoiceItemsCount )
        {
            sLog.outString("WORLD: Attempt to select an unexisting rewardid !");
            return;
        }

        if ( pQuest->m_qRewChoiceItemsCount > 0 )
        {
            if  ( _player->CanAddItemCount(pQuest->GetQuestInfo()->RewChoiceItemId[rewardid], pQuest->GetQuestInfo()->RewChoiceItemCount[rewardid]) >= (int)pQuest->m_qRewChoiceItemsCount )
                _player->AddNewItem(pQuest->GetQuestInfo()->RewChoiceItemId[rewardid],pQuest->GetQuestInfo()->RewChoiceItemCount[rewardid], true);
            else
            {
                _player->PlayerTalkClass->SendQuestFailed( FAILEDREASON_INV_FULL );
                return;
            }
        }

        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++ )
        {
            if ( pQuest->GetQuestInfo()->ReqItemId[i] )
                _player->RemoveItemFromInventory( pQuest->GetQuestInfo()->ReqItemId[i], pQuest->GetQuestInfo()->ReqItemCount[i]);
        }

        if ( pQuest->GetQuestInfo()->RewSpell > 0 )
        {
            WorldPacket sdata;

            sdata.Initialize (SMSG_LEARNED_SPELL);
            sdata << pQuest->GetQuestInfo()->RewSpell;
            SendPacket( &sdata );
            _player->addSpell( (uint16)pQuest->GetQuestInfo()->RewSpell );
        }

        _player->PlayerTalkClass->SendQuestUpdateComplete( pQuest );
        _player->PlayerTalkClass->SendQuestComplete( pQuest );
        uint16 log_slot = _player->getQuestSlot(quest_id);
        _player->SetUInt32Value(log_slot+0, 0);
        _player->SetUInt32Value(log_slot+1, 0);
        _player->SetUInt32Value(log_slot+2, 0);

        if ( _player->getLevel() < 60 )
        {
            _player->GiveXP( pQuest->XPValue( _player ), guid1 );
            _player->ModifyMoney( pQuest->GetQuestInfo()->RewMoney );
        }
        else
            _player->ModifyMoney( pQuest->GetQuestInfo()->RewMoney + pQuest->XPValue( _player ) );

        if ( !pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_REPEATABLE ) )
            _player->mQuestStatus[quest_id].rewarded = true;
        else
            _player->SetQuestStatus(pQuest, QUEST_STATUS_NONE);

        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid1);
        if( pCreature )
        {
            if( !(Script->ChooseReward( _player, pCreature, pQuest, rewardid )) )
            {
                pCreature->RemoveFlag(UNIT_DYNAMIC_FLAGS, 2);
                Quest* nextquest;
                if( nextquest = pCreature->getNextAvailableQuest(_player,pQuest) )
                    _player->PlayerTalkClass->SendQuestDetails(nextquest,pCreature->GetGUID(),true);
                else
                    _player->PlayerTalkClass->CloseGossip();
            }
        }
        else
        {
            GameObject *pGameObject = ObjectAccessor::Instance().GetGameObject(*_player, guid1);
            if (pGameObject)
                Script->GOChooseReward( _player, pGameObject, pQuest, rewardid );
        }
    }
}
void WorldSession::HandleQuestgiverRequestRewardOpcode( WorldPacket & recv_data )
{
    uint32 quest_id;
    uint64 guid;
    recv_data >> guid >> quest_id;

    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_REQUEST_REWARD guid = %u, questid = %u",uint32(GUID_LOPART(guid)),quest_id );

    Quest *pQuest       = objmgr.GetQuest( quest_id );
    if( pQuest )
    {
        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
        if( pCreature )
        {
            if ( _player->IsQuestComplete( pQuest ) )
                _player->PlayerTalkClass->SendQuestReward( pQuest, guid, true, NULL, 0);
        }
    }
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

    sLog.outString( "WORLD: Received CMSG_QUESTLOG_SWAP_QUEST slot 1 = %u, slot 2 = %u",slot_id1,slot_id2 );

    uint16 log_slot1 = _player->getQuestSlotById( slot_id1 );
    if( log_slot1 )
    {
        uint16 log_slot2 = _player->getQuestSlotById( slot_id2 );
        if( log_slot2 )
        {
            uint32 temp1, temp2;

            for (int iCx = 0; iCx < 3; iCx++ )
            {
                temp1 = _player->GetUInt32Value(log_slot1 + iCx);
                temp2 = _player->GetUInt32Value(log_slot2 + iCx);

                _player->SetUInt32Value(log_slot1 + iCx, temp2);
                _player->SetUInt32Value(log_slot2 + iCx, temp1);
            }
        }
    }
}
void WorldSession::HandleQuestLogRemoveQuest(WorldPacket& recv_data)
{
    uint8 slot_id;
    uint32 quest_id;
    recv_data >> slot_id;

    sLog.outString( "WORLD: Received CMSG_QUESTLOG_REMOVE_QUEST slot = %u",slot_id );

    slot_id++;
    uint16 log_slot = _player->getQuestSlotById( slot_id );
    if( log_slot )
    {
        quest_id = _player->GetUInt32Value(log_slot + 0);

        _player->SetUInt32Value(log_slot + 0, 0);
        _player->SetUInt32Value(log_slot + 1, 0);
        _player->SetUInt32Value(log_slot + 2, 0);

        Quest *pQuest = objmgr.GetQuest( quest_id );
        if( pQuest )
        {
            _player->SetQuestStatus( pQuest, QUEST_STATUS_NONE);
            _player->TakeQuestSourceItem( pQuest );
        }
    }
}
void WorldSession::HandleQuestConfirmAccept(WorldPacket& recv_data)
{
    uint32 quest_id;
    recv_data >> quest_id;

    sLog.outString( "WORLD: Received CMSG_QUEST_CONFIRM_ACCEPT quest = %u",quest_id );
}
void WorldSession::HandleQuestComplete(WorldPacket& recv_data)
{
    uint32 quest_id;
    uint64 guid;
    recv_data >> guid >> quest_id;

    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_COMPLETE_QUEST guid = %u, quest = %u",uint32(GUID_LOPART(guid)),quest_id );

    Quest *pQuest = objmgr.GetQuest( quest_id );
    if( pQuest )
    {
        if( _player->GetQuestStatus( pQuest ) != QUEST_STATUS_COMPLETE )
            _player->PlayerTalkClass->SendRequestedItems(pQuest, guid, false);
        else
            _player->PlayerTalkClass->SendRequestedItems(pQuest, guid, true);
    }
}
void WorldSession::HandleQuestAutoLaunch(WorldPacket& recvPacket)
{
    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_QUEST_AUTOLAUNCH (Unhandled!)" );
}