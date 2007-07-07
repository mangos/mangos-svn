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
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;

    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_STATUS_QUERY npc = %u",uint32(GUID_LOPART(guid)) );

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if ( pCreature )
    {
        uint32 questStatus = DIALOG_STATUS_NONE;

        if( !pCreature->IsHostileTo(_player))               // not show quest status to enemies
        {
            questStatus = Script->NPCDialogStatus(_player, pCreature);
            if( questStatus > 6 )
            {
                uint32 defstatus = DIALOG_STATUS_NONE;
                questStatus = pCreature->getDialogStatus(_player, defstatus);
            }
        }
        _player->PlayerTalkClass->SendQuestGiverStatus(questStatus, guid);
    }
}

void WorldSession::HandleQuestgiverHelloOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;

    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_HELLO npc = %u",guid );

    Creature *pCreature = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_NONE);
    if (!pCreature)
    {
        sLog.outDebug( "WORLD: HandleQuestgiverHelloOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    if(Script->GossipHello( _player, pCreature ) )
        return;

    pCreature->prepareGossipMenu(_player,0);
    pCreature->sendPreparedGossip( _player );
}

void WorldSession::HandleQuestgiverAcceptQuestOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 guid;
    uint32 quest;
    recv_data >> guid >> quest;

    if(!GetPlayer()->isAlive())
        return;

    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_ACCEPT_QUEST npc = %u, quest = %u",uint32(GUID_LOPART(guid)),quest );

    Object* pObject = ObjectAccessor::Instance().GetObjectByTypeMask(*_player, guid,TYPE_UNIT|TYPE_GAMEOBJECT|TYPE_ITEM|TYPE_PLAYER);

    // no or incorrect quest giver
    if(!pObject
        || (pObject->GetTypeId()!=TYPEID_PLAYER && !pObject->hasQuest(quest))
        || (pObject->GetTypeId()==TYPEID_PLAYER && !((Player*)pObject)->CanShareQuest(quest))
      )
    {
        _player->PlayerTalkClass->CloseGossip();
        _player->SetDivider( 0 );
        return;
    }

    Quest * qInfo = objmgr.QuestTemplates[quest];
    if ( qInfo )
    {
        // prevent cheating
        if(!GetPlayer()->CanTakeQuest(qInfo,true) )
        {
            _player->PlayerTalkClass->CloseGossip();
            _player->SetDivider( 0 );
            return;
        }

        if( _player->GetDivider() != 0 )
        {
            Player *pPlayer = ObjectAccessor::Instance().FindPlayer( _player->GetDivider() );
            if( pPlayer )
            {
                pPlayer->SendPushToPartyResponse( _player, QUEST_PARTY_MSG_ACCEPT_QUEST );
                _player->SetDivider( 0 );
            }
        }

        if( _player->CanAddQuest( qInfo, true ) )
        {
            _player->AddQuest( qInfo, pObject );

            if ( _player->CanCompleteQuest( quest ) )
                _player->CompleteQuest( quest );

            switch(pObject->GetTypeId())
            {
                case TYPEID_UNIT:
                    Script->QuestAccept(_player, ((Creature*)pObject), qInfo );
                    break;
                case TYPEID_ITEM:
                {
                    Script->ItemQuestAccept(_player, ((Item*)pObject), qInfo );

                    // destroy not required for quest finish quest starting item
                    bool destroyItem = true;
                    for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                    {
                        if ((qInfo->ReqItemId[i] == ((Item*)pObject)->GetEntry()) && (((Item*)pObject)->GetProto()->MaxCount != 0))
                        {
                            destroyItem = false;
                            break;
                        }
                    }

                    if(destroyItem)
                        _player->DestroyItem(((Item*)pObject)->GetBagSlot(),((Item*)pObject)->GetSlot(),true);

                    break;
                }
                case TYPEID_GAMEOBJECT:
                    Script->GOQuestAccept(_player, ((GameObject*)pObject), qInfo );
                    break;
            }
            _player->PlayerTalkClass->CloseGossip();

            if( qInfo->GetSrcSpell() > 0 )
                _player->CastSpell( _player, qInfo->GetSrcSpell(), true);

            return;
        }
    }

    _player->PlayerTalkClass->CloseGossip();
}

void WorldSession::HandleQuestgiverQuestQueryOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 guid;
    uint32 quest;
    recv_data >> guid >> quest;
    sLog.outDebug( "WORLD: Received CMSG_QUESTGIVER_QUERY_QUEST npc = %u, quest = %u",uint32(GUID_LOPART(guid)),quest );

    // Verify that the guid is valid and is a questgiver or involved in the requested quest
    Object* pObject = ObjectAccessor::Instance().GetObjectByTypeMask(*_player, guid,TYPE_UNIT|TYPE_GAMEOBJECT|TYPE_ITEM);
    if(!pObject||!pObject->hasQuest(quest) && !pObject->hasInvolvedQuest(quest))
    {
        _player->PlayerTalkClass->CloseGossip();
        return;
    }

    Quest *pQuest = objmgr.QuestTemplates[quest];
    if ( pQuest )
    {
        _player->PlayerTalkClass->SendQuestGiverQuestDetails(pQuest, pObject->GetGUID(), true);
    }
}

void WorldSession::HandleQuestQueryOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4);

    uint32 quest;
    recv_data >> quest;
    sLog.outDebug( "WORLD: Received CMSG_QUEST_QUERY quest = %u",quest );

    Quest *pQuest = objmgr.QuestTemplates[quest];
    if ( pQuest )
    {
        _player->PlayerTalkClass->SendQuestQueryResponse( pQuest );
    }
}

void WorldSession::HandleQuestgiverChooseRewardOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4+4);

    uint32 quest, reward;
    uint64 guid;
    recv_data >> guid >> quest >> reward;

    if(!GetPlayer()->isAlive())
        return;

    sLog.outDetail( "WORLD: Received CMSG_QUESTGIVER_CHOOSE_REWARD npc = %u, quest = %u, reward = %u",uint32(GUID_LOPART(guid)),quest,reward );

    Object* pObject = ObjectAccessor::Instance().GetObjectByTypeMask(*_player, guid,TYPE_UNIT|TYPE_GAMEOBJECT|TYPE_PLAYER);
    if(!pObject)
        return;
    
    switch(pObject->GetTypeId())
    {
        case TYPEID_PLAYER:                                 // spell completed quest
            if(((Player*)pObject)==_player && !_player->IsQuestSpellComplete(quest))
                return;
             break;
        default:                                            // creature/GO completed quest
            if(!pObject->hasInvolvedQuest(quest))
                return;
            break;
    }

    Quest *pQuest = objmgr.QuestTemplates[quest];
    if( pQuest )
    {
        if( _player->CanRewardQuest( pQuest, reward, true ) )
        {
            _player->RewardQuest( pQuest, reward, pObject );

            switch(pObject->GetTypeId())
            {
                case TYPEID_UNIT:
                    if( !(Script->ChooseReward( _player, ((Creature*)pObject), pQuest, reward )) )
                    {
                        // Send next quest
                        if(Quest* nextquest = _player->GetNextQuest( guid ,pQuest ) )
                            _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextquest,guid,true);
                    }
                    break;
                case TYPEID_GAMEOBJECT:
                    if( !Script->GOChooseReward( _player, ((GameObject*)pObject), pQuest, reward ) )
                    {
                        // Send next quest
                        if(Quest* nextquest = _player->GetNextQuest( guid ,pQuest ) )
                            _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextquest,guid,true);
                    }
                    break;
                case TYPEID_PLAYER:
                    // Send next quest
                    if(Quest* nextquest = _player->GetNextQuest( guid ,pQuest ) )
                        _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextquest,guid,true);
                    break;
            }
        }
        else
            _player->PlayerTalkClass->SendQuestGiverOfferReward( quest, guid, true, NULL, 0);
    }
}

void WorldSession::HandleQuestgiverRequestRewardOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint32 quest;
    uint64 guid;
    recv_data >> guid >> quest;

    if(!GetPlayer()->isAlive())
        return;

    sLog.outDetail( "WORLD: Received CMSG_QUESTGIVER_REQUEST_REWARD npc = %u, quest = %u",uint32(GUID_LOPART(guid)),quest );

    Object* pObject = ObjectAccessor::Instance().GetObjectByTypeMask(*_player, guid,TYPE_UNIT|TYPE_GAMEOBJECT);
    if(!pObject||!pObject->hasInvolvedQuest(quest))
        return;

    Quest *pQuest = objmgr.QuestTemplates[quest];
    if( pQuest )
    {
        if ( _player->CanCompleteQuest( quest ) )
            _player->PlayerTalkClass->SendQuestGiverOfferReward( quest, guid, true, NULL, 0);
    }
}

void WorldSession::HandleQuestgiverCancel(WorldPacket& recv_data )
{
    sLog.outDetail( "WORLD: Received CMSG_QUESTGIVER_CANCEL" );

    _player->PlayerTalkClass->CloseGossip();
}

void WorldSession::HandleQuestLogSwapQuest(WorldPacket& recv_data )
{
    CHECK_PACKET_SIZE(recv_data,1+1);

    uint8 slot1, slot2;
    recv_data >> slot1 >> slot2;

    if(slot1 == slot2 || slot1 >= MAX_QUEST_LOG_SIZE || slot2 >= MAX_QUEST_LOG_SIZE)
        return;

    sLog.outDetail( "WORLD: Received CMSG_QUESTLOG_SWAP_QUEST slot 1 = %u, slot 2 = %u",slot1,slot2 );

    uint32 temp1;
    uint32 temp2;
    for (int i = 0; i < 3; i++ )
    {
        temp1 = _player->GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*slot1 + i);
        temp2 = _player->GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*slot2 + i);

        _player->SetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*slot1 + i, temp2);
        _player->SetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*slot2 + i, temp1);
    }
}

void WorldSession::HandleQuestLogRemoveQuest(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data,1);

    uint8 slot;
    uint32 quest;
    recv_data >> slot;

    sLog.outDetail( "WORLD: Received CMSG_QUESTLOG_REMOVE_QUEST slot = %u",slot );

    if( slot < MAX_QUEST_LOG_SIZE )
    {
        quest = _player->GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*slot + 0);

        if( quest )
        {
            if(!_player->TakeQuestSourceItem( quest, true ))
                return;                                     // can't un-equip some items, reject quest cancel

            _player->SetQuestStatus( quest, QUEST_STATUS_NONE);
        }
        _player->SetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*slot + 0, 0);
        _player->SetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*slot + 1, 0);
        _player->SetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*slot + 2, 0);

    }
}

void WorldSession::HandleQuestConfirmAccept(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4);

    uint32 quest;
    recv_data >> quest;

    sLog.outDetail( "WORLD: Received CMSG_QUEST_CONFIRM_ACCEPT quest = %u",quest );
}

void WorldSession::HandleQuestComplete(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint32 quest;
    uint64 guid;
    recv_data >> guid >> quest;

    if(!GetPlayer()->isAlive())
        return;

    sLog.outDetail( "WORLD: Received CMSG_QUESTGIVER_COMPLETE_QUEST npc = %u, quest = %u",uint32(GUID_LOPART(guid)),quest );

    Quest *pQuest = objmgr.QuestTemplates[quest];
    if( pQuest )
    {
        if( _player->GetQuestStatus( quest ) != QUEST_STATUS_COMPLETE )
        {
            if( pQuest->IsRepeatable() )
                _player->PlayerTalkClass->SendQuestGiverRequestItems(pQuest, guid, _player->CanCompleteRepeatableQuest(pQuest), false);
            else
                _player->PlayerTalkClass->SendQuestGiverRequestItems(pQuest, guid, false, false);
        }
        else
            _player->PlayerTalkClass->SendQuestGiverRequestItems(pQuest, guid, true, false);
    }
}

void WorldSession::HandleQuestAutoLaunch(WorldPacket& recvPacket)
{
    sLog.outDetail( "WORLD: Received CMSG_QUESTGIVER_QUEST_AUTOLAUNCH (Send your log to anakin if you see this message)" );
}

void WorldSession::HandleQuestPushToParty(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket,4);

    uint64 guid;
    uint32 quest;
    recvPacket >> quest;

    sLog.outDetail( "WORLD: Received CMSG_PUSHQUESTTOPARTY quest = %u", quest );

    Quest *pQuest = objmgr.QuestTemplates[quest];
    if( pQuest )
    {
        if( _player->groupInfo.group )
        {
            Group *pGroup = _player->groupInfo.group;
            if( pGroup )
            {
                uint32 pguid = _player->GetGUID();
                Group::MemberList const& members = pGroup->GetMembers();
                for(Group::member_citerator itr = members.begin(); itr != members.end(); ++itr)
                {
                    guid = itr->guid;
                    if( guid !=  pguid )
                    {
                        Player *pPlayer = ObjectAccessor::Instance().FindPlayer(guid);
                        if( pPlayer )
                        {
                            _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_SHARING_QUEST);

                            if( _player->GetDistanceSq( pPlayer ) > 100 )
                            {
                                _player->SendPushToPartyResponse( pPlayer, QUEST_PARTY_MSG_TOO_FAR );
                                continue;
                            }

                            if( !pPlayer->SatisfyQuestStatus( quest, false ) )
                            {
                                _player->SendPushToPartyResponse( pPlayer, QUEST_PARTY_MSG_HAVE_QUEST );
                                continue;
                            }

                            if( pPlayer->GetQuestStatus( quest ) == QUEST_STATUS_COMPLETE )
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

                            pPlayer->PlayerTalkClass->SendQuestGiverQuestDetails( pQuest, guid, true );
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
    CHECK_PACKET_SIZE(recvPacket,8+1);

    uint64 guid;
    uint8 msg;
    recvPacket >> guid >> msg;

    sLog.outDetail( "WORLD: Received MSG_QUEST_PUSH_RESULT " );

    if( _player->GetDivider() != 0 )
    {
        Player *pPlayer = ObjectAccessor::Instance().FindPlayer( _player->GetDivider() );
        if( pPlayer )
        {
            WorldPacket data( MSG_QUEST_PUSH_RESULT, (8+4+1) );
            data << guid;
            data << uint32( msg );
            data << uint8(0);
            pPlayer->GetSession()->SendPacket(&data);
            _player->SetDivider( 0 );
        }
    }
}
