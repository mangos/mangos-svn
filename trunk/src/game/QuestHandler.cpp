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
#include "Group.h"

void WorldSession::HandleQuestgiverStatusQueryOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    recv_data >> guid;

    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_STATUS_QUERY npc = %u",uint32(GUID_LOPART(guid)) );

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

    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_HELLO npc = %u",guid );

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
    uint32 quest;
    recv_data >> guid >> quest;

    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_ACCEPT_QUEST npc = %u, quest = %u",uint32(GUID_LOPART(guid)),quest );

    Quest *pQuest = objmgr.GetQuest(quest);
    if ( pQuest )
    {
        if( _player->GetDivider() != 0 )
        {
            Player *pPlayer = ObjectAccessor::Instance().FindPlayer( _player->GetDivider() );
            if( pPlayer )
            {
                pPlayer->SendPushToPartyResponse( _player, QUEST_PARTY_MSG_ACCEPT_QUEST );
                _player->SetDivider( 0 );
            }
        }

        if( _player->CanAddQuest( pQuest, true ) )
        {
            _player->AddQuest( pQuest );

            if ( _player->CanCompleteQuest( pQuest ) )
                _player->CompleteQuest( pQuest );

            Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
            if( pCreature )
                Script->QuestAccept(_player, pCreature, pQuest );
            else
            {
                Item *pItem = 0;
                //uint32 slot = _player->GetSlotByItemGUID( guid );
                //if ( slot )
                    pItem = _player->GetItemByPos( _player->GetPosByGuid( guid ));

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
    uint32 quest;
    recv_data >> guid >> quest;

    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_QUERY_QUEST npc = %u, quest = %u",uint32(GUID_LOPART(guid)),quest );

    Quest *pQuest = objmgr.GetQuest(quest);
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
            //uint32 slot = _player->GetSlotByItemGUID( guid );
            //if ( slot )
                pItem = _player->GetItemByPos( _player->GetPosByGuid( guid ));

            if( pItem )
            {
                if( !Script->ItemQuestAccept(_player, pItem, pQuest ) )
                {
                    if( status == QUEST_STATUS_NONE && _player->CanTakeQuest( pQuest, true ) )
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
    uint32 quest;
    recv_data >> quest;
    sLog.outDebug( "WORLD: Received CMSG_QUEST_QUERY quest = %u",quest );

    Quest *pQuest = objmgr.GetQuest(quest);

    if ( pQuest )
        _player->PlayerTalkClass->SendUpdateQuestDetails( pQuest );
}

void WorldSession::HandleQuestgiverChooseRewardOpcode( WorldPacket & recv_data )
{
    uint32 quest, reward;
    uint64 guid;
    recv_data >> guid >> quest >> reward;

    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_CHOOSE_REWARD npc = %u, quest = %u, reward = %u",uint32(GUID_LOPART(guid)),quest,reward );

    Quest *pQuest = objmgr.GetQuest(quest);
    if( pQuest )
    {
        if( _player->CanRewardQuest( pQuest, reward, true ) )
        {
            _player->RewardQuest( pQuest, reward );
            _player->CalculateReputation( pQuest, guid );

            Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
            if( pCreature )
            {
                if( !(Script->ChooseReward( _player, pCreature, pQuest, reward )) )
                {
                    Quest* nextquest;
                    if( nextquest = pCreature->getNextAvailableQuest(_player,pQuest) )
                        _player->PlayerTalkClass->SendQuestDetails(nextquest,pCreature->GetGUID(),true);
                }
            }
            else
            {
                GameObject *pGameObject = ObjectAccessor::Instance().GetGameObject(*_player, guid);
                if ( pGameObject )
                    Script->GOChooseReward( _player, pGameObject, pQuest, reward );
            }
        }
    }
}

void WorldSession::HandleQuestgiverRequestRewardOpcode( WorldPacket & recv_data )
{
    uint32 quest;
    uint64 guid;
    recv_data >> guid >> quest;

    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_REQUEST_REWARD npc = %u, quest = %u",uint32(GUID_LOPART(guid)),quest );

    Quest *pQuest       = objmgr.GetQuest( quest );
    if( pQuest )
    {
        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
        if( pCreature )
        {
            if ( _player->CanCompleteQuest( pQuest ) )
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
    uint8 slot1, slot2;
    recv_data >> slot1 >> slot2;

    sLog.outString( "WORLD: Received CMSG_QUESTLOG_SWAP_QUEST slot 1 = %u, slot 2 = %u",slot1,slot2 );

    if( slot1 || slot2 )
    {
        uint32 temp1;
        uint32 temp2;
        for (int i = 0; i < 3; i++ )
        {
            temp1 = _player->GetUInt32Value(3*slot1 + PLAYER_QUEST_LOG_1_1 + i);
            temp2 = _player->GetUInt32Value(3*slot2 + PLAYER_QUEST_LOG_1_1 + i);

            _player->SetUInt32Value(3*slot1 + PLAYER_QUEST_LOG_1_1 + i, temp2);
            _player->SetUInt32Value(3*slot2 + PLAYER_QUEST_LOG_1_1 + i, temp1);
        }
    }
}

void WorldSession::HandleQuestLogRemoveQuest(WorldPacket& recv_data)
{
    uint8 slot;
    uint32 quest;
    recv_data >> slot;

    sLog.outString( "WORLD: Received CMSG_QUESTLOG_REMOVE_QUEST slot = %u",slot );

    if( slot >= 0 && slot < 20 )
    {
        quest = _player->GetUInt32Value(3*slot + 200 + 0);

        _player->SetUInt32Value(3*slot + 200 + 0, 0);
        _player->SetUInt32Value(3*slot + 200 + 1, 0);
        _player->SetUInt32Value(3*slot + 200 + 2, 0);

        Quest *pQuest = objmgr.GetQuest( quest );
        if( pQuest )
        {
            _player->SetQuestStatus( pQuest, QUEST_STATUS_NONE);
            _player->TakeQuestSourceItem( pQuest );
        }
    }
}

void WorldSession::HandleQuestConfirmAccept(WorldPacket& recv_data)
{
    uint32 quest;
    recv_data >> quest;

    sLog.outString( "WORLD: Received CMSG_QUEST_CONFIRM_ACCEPT quest = %u",quest );
}

void WorldSession::HandleQuestComplete(WorldPacket& recv_data)
{
    uint32 quest;
    uint64 guid;
    recv_data >> guid >> quest;

    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_COMPLETE_QUEST npc = %u, quest = %u",uint32(GUID_LOPART(guid)),quest );

    Quest *pQuest = objmgr.GetQuest( quest );
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
    sLog.outString( "WORLD: Received CMSG_QUESTGIVER_QUEST_AUTOLAUNCH (Send your log to anakin if you see this message)" );
}

void WorldSession::HandleQuestPushToParty(WorldPacket& recvPacket)
{
    uint64 guid;
    uint32 quest;
    recvPacket >> quest;

    WorldPacket data;

    sLog.outString( "WORLD: Received CMSG_PUSHQUESTTOPARTY quest = %u", quest );

    Quest *pQuest = objmgr.GetQuest( quest );
    if( pQuest )
    {
        if( _player->IsInGroup() )
        {
            Group *pGroup = objmgr.GetGroupByLeader(_player->GetGroupLeader());
            if( pGroup )
            {
                uint32 pguid = _player->GetGUID();
                uint32 memberscount = pGroup->GetMembersCount();
                for (int i = 0; i < memberscount; i++)
                {
                    guid = pGroup->GetMemberGUID(i);
                    if( guid !=  pguid )
                    {
                        Player *pPlayer = ObjectAccessor::Instance().FindPlayer(guid);
                        if( pPlayer )
                        {
                            data.clear();
                            data.Initialize( MSG_QUEST_PUSH_RESULT );
                            data << guid;
                            data << uint32( QUEST_PARTY_MSG_SHARRING_QUEST );
                            data << uint8(0);
                            _player->GetSession()->SendPacket(&data);

                            if( _player->GetDistanceSq( pPlayer ) > 100 )
                            {
                                _player->SendPushToPartyResponse( pPlayer, QUEST_PARTY_MSG_TO_FAR );
                                continue;
                            }

                            if( !pPlayer->SatisfyQuestStatus( pQuest, false ) )
                            {
                                _player->SendPushToPartyResponse( pPlayer, QUEST_PARTY_MSG_HAVE_QUEST );
                                continue;
                            }

                            if( pPlayer->GetQuestStatus( pQuest ) == QUEST_STATUS_COMPLETE )
                            {
                                _player->SendPushToPartyResponse( pPlayer, QUEST_PARTY_MSG_FINISH_QUEST );
                                continue;
                            }

                            if( !pPlayer->CanTakeQuest( pQuest, false ) )
                            {
                                _player->SendPushToPartyResponse( pPlayer, QUEST_PARTY_MSG_CANT_TAKE_QUEST );
                                continue;
                            }

                            if( !pPlayer->SatisfyQuestLog( false ) )
                            {
                                _player->SendPushToPartyResponse( pPlayer, QUEST_PARTY_MSG_LOG_FULL );
                                continue;
                            }

                            if( pPlayer->GetDivider() != 0  )
                            {
                                _player->SendPushToPartyResponse( pPlayer, QUEST_PARTY_MSG_BUSY );
                                continue;
                            }

                            pPlayer->PlayerTalkClass->SendQuestDetails( pQuest, guid, true );
                            pPlayer->SetDivider( pguid );
                        }
                    }
                }
            }
        }
    }
}

void WorldSession::HandleQuestPushResult(WorldPacket& recvPacket)
{
    uint64 guid;
    uint8 msg;
    recvPacket >> guid >> msg;

    sLog.outString( "WORLD: Received MSG_QUEST_PUSH_RESULT " );

    if( _player->GetDivider() != 0 )
    {
        Player *pPlayer = ObjectAccessor::Instance().FindPlayer( _player->GetDivider() );
        if( pPlayer )
        {
            WorldPacket data;
            data.Initialize( MSG_QUEST_PUSH_RESULT );
            data << guid;
            data << uint32( msg );
            data << uint8(0);
            pPlayer->GetSession()->SendPacket(&data);
            _player->SetDivider( 0 );
        }
    }
}
