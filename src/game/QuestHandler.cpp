/* QuestHandler.cpp
 *
 * Copyright (C) 2004 Wow Daemon
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

#include "Common.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Quest.h"

#ifdef ENABLE_GRID_SYSTEM
#include "ObjectAccessor.h"
#endif

void WorldSession::HandleQuestgiverStatusQueryOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_QUESTGIVER_STATUS_QUERY" );

    uint64 guid;
    WorldPacket data;

    recv_data >> guid;

#ifndef ENABLE_GRID_SYSTEM
    Creature *pCreature = objmgr.GetObject<Creature>(guid);
#else
    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, GUID_LOPART(guid));
#endif
    if (!pCreature)
    {
        Log::getSingleton( ).outError( "WORLD: received incorrect guid in CMSG_QUESTGIVER_STATUS_QUERY" );
        return;
    }

    // uint32 quest_status = sWorld.mCreatures[guid1]->getQuestStatus(GetPlayer());

    uint32 questStatus = pCreature->getQuestStatus(GetPlayer());

    data.Initialize( SMSG_QUESTGIVER_STATUS );
    data << guid << questStatus;
#ifdef _VERSION_1_7_0_
    data << uint32(0);
#endif //_VERSION_1_7_0_
    SendPacket( &data );

    Log::getSingleton( ).outDebug( "WORLD: Sent SMSG_QUESTGIVER_STATUS" );
}


void WorldSession::HandleQuestgiverHelloOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_QUESTGIVER_HELLO" );

    uint64 guid;
    Creature *pCreature;
    WorldPacket data;

    recv_data >> guid;
#ifndef ENABLE_GRID_SYSTEM
    pCreature = objmgr.GetObject<Creature>(guid);
#else
    pCreature = ObjectAccessor::Instance().GetCreature(*_player, GUID_LOPART(guid));
#endif
    if(!pCreature)
    {
        Log::getSingleton( ).outError( "WORLD: received incorrect guid in SMSG_QUESTGIVER_REQUEST_ITEMS" );
        return;
    }

    uint32 qg_status = pCreature->getQuestStatus(GetPlayer());
    uint32 quest_id = pCreature->getCurrentQuest(GetPlayer());

    if (qg_status == 0)
        return;

    Quest *pQuest = objmgr.GetQuest(quest_id);

    if(qg_status == QUEST_STATUS_INCOMPLETE)
    {
        if (GetPlayer()->checkQuestStatus(quest_id) || pQuest->m_targetGuid == guid)
        {
            const char *title = pCreature->getQuestTitle(quest_id);
            const char *details = pCreature->getQuestCompleteText(quest_id);

            data.Initialize( SMSG_QUESTGIVER_OFFER_REWARD );
            data << guid;
            data << quest_id;
            data << title;
            data << details;

            data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);
            data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);
            data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
            data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);

            data << uint32(pQuest->m_choiceRewards);
            for (uint16 i=0; i < pQuest->m_choiceRewards; i++)
            {
                data << uint32(pQuest->m_choiceItemId[i]) << uint32(pQuest->m_choiceItemCount[i]);
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
            }

            data << uint32(pQuest->m_itemRewards);
            for (uint16 i=0; i < pQuest->m_itemRewards; i++)
            {
                data << uint32(pQuest->m_rewardItemId[i]) << uint32(pQuest->m_rewardItemCount[i]);
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
            }

            data << uint32(pQuest->m_rewardGold);
            data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);

            WPAssert(data.size() == 8+4+strlen(title)+1 + strlen(details)+1 + 32 + pQuest->m_choiceRewards*12 + pQuest->m_itemRewards*12);
            SendPacket( &data );
            Log::getSingleton( ).outDebug( "WORLD: Sent SMSG_QUESTGIVER_OFFER_REWARD" );
        }
        else
        {
            // Incomplete Quest
            const char *title = pCreature->getQuestTitle(quest_id);
            const char *incompleteText = pCreature->getQuestIncompleteText(quest_id);

            data.Initialize( SMSG_QUESTGIVER_REQUEST_ITEMS);
            data << guid;
            data << quest_id;
            data << title;
            data << incompleteText;

            data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
            data << uint8(0x06) << uint8(0x00) << uint8(0x00) << uint8(0x00);
            data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);
            data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
            // setting this to anything...
            data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);
            // with this set to anything, enables "continue" to comlete quest
            data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
            data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);

            WPAssert(data.size() == 8 + 4 + strlen(title)+1 + strlen(incompleteText)+1 + 28);
            SendPacket( &data );
            // GetPlayer()->setQuestStatus(quest_id, QUEST_STATUS_CHOOSE_REWARD);
            Log::getSingleton( ).outDebug( "WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS" );
        }
    }
    else if(qg_status == QUEST_STATUS_AVAILABLE)
    {
        // Send quest details
        const char *title = pCreature->getQuestTitle(quest_id);
        const char *details = pCreature->getQuestDetails(quest_id);
        const char *objectives = pCreature->getQuestObjectives(quest_id);
        Quest *pQuest = objmgr.GetQuest(quest_id);

        uint16 rewardSize = 52;
        rewardSize += pQuest->m_choiceRewards*12;
        rewardSize += pQuest->m_itemRewards*12;

        data.Initialize( SMSG_QUESTGIVER_QUEST_DETAILS );
        data << guid;
        data << quest_id;
        data << title;
        data << details;
        data << objectives;

        data << uint32(1);
        data << uint32(pQuest->m_choiceRewards);

        for (int i=0; i < pQuest->m_choiceRewards; i++)
        {
            data << pQuest->m_choiceItemId[i] << pQuest->m_choiceItemCount[i];
            data << uint32(0);
        }

        data << uint32(pQuest->m_itemRewards);
        for (int i=0; i < pQuest->m_itemRewards; i++)
        {
            data << uint32(pQuest->m_rewardItemId[i]) << uint32(pQuest->m_rewardItemCount[i]);
            data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
        }

        data << uint32(pQuest->m_rewardGold);
        data << uint32(0) << uint32(0);
        data << uint32(0) << uint32(0);
        data << uint32(0) << uint32(0);
        data << uint32(0) << uint32(0) << uint32(0);

        WPAssert(data.size() == 8 + 4 + strlen(title)+1 + strlen(details)+1 + strlen(objectives)+1 + rewardSize);
        SendPacket( &data );
        Log::getSingleton( ).outDebug( "WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS" );
    }
}


void WorldSession::HandleQuestgiverAcceptQuestOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_QUESTGIVER_ACCEPT_QUEST" );

    uint64 guid;
    uint32 quest_id;
    WorldPacket data;

    recv_data >> guid >> quest_id;

    Quest *pQuest = objmgr.GetQuest(quest_id);
    if(!pQuest)
    {
        Log::getSingleton( ).outError( "WORLD: received incorrect quest id in CMSG_QUESTGIVER_ACCEPT_QUEST" );
        return;
    }

    uint16 log_slot = GetPlayer()->getOpenQuestSlot();
    if (log_slot == 0)
    {
        // TODO:  Send log full message
        return;
    }

    GetPlayer()->SetUInt32Value(log_slot, quest_id);
    GetPlayer()->SetUInt32Value(log_slot+1, uint32(0x337));

    // SendPacket( &data );
    Log::getSingleton( ).outDebug( "WORLD: Sent Quest Acceptance 0xA9" );

    if (pQuest->m_targetGuid != 0)
    {
        SetNpcFlagsForTalkToQuest(guid, pQuest->m_targetGuid);
    }

    GetPlayer()->setQuestStatus(quest_id, QUEST_STATUS_INCOMPLETE);
}


void WorldSession::HandleQuestQueryOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_QUEST_QUERY" );

    uint32 quest_id = 0;
    recv_data >> quest_id;

    if (quest_id > 10)                            // assuming there are no more than 10 quests for now
    {
        Log::getSingleton().outDebug("Bad Quest Query requested\n");
        return;
    }

    Creature *pCreature = NULL;
#ifndef ENABLE_GRID_SYSTEM
    for( ObjectMgr::CreatureMap::const_iterator i = objmgr.Begin<Creature>();
        i != objmgr.End<Creature>(); ++ i )
    {
        if(i->second != NULL)
        {
            if(i->second->hasQuest(quest_id))
                pCreature = i->second;            // FIXME: break?
        }
    }
#else
    for(Player::InRangeUnitsMapType::iterator iter=_player->InRangeUnitsBegin(); iter != _player->InRangeUnitsEnd(); ++iter)
    {
    pCreature = dynamic_cast<Creature *>(iter->second);
    if( pCreature != NULL && pCreature->hasQuest(quest_id) )
        break;
    }
#endif

    WPAssert( pCreature != NULL );

    const char *title = pCreature->getQuestTitle(quest_id);
    const char *details = pCreature->getQuestDetails(quest_id);
    const char *objectives = pCreature->getQuestObjectives(quest_id);
    Quest *pQuest = objmgr.GetQuest(quest_id);

    WorldPacket data;
    data.Initialize( SMSG_QUEST_QUERY_RESPONSE );
    data << quest_id;

    // tdata
    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
    data << uint32(pQuest->m_zone);

    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
    // reputation faction?
    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);

    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
    data << uint32(pQuest->m_rewardGold);
    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);

    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);

    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);

    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);

    for (int i=0; i < 5; i++)
        data << uint32(pQuest->m_choiceItemId[i]) << uint32(pQuest->m_choiceItemCount[i]);

    data << uint32(0);
    data << uint32(0x01);
    data << uint32(0xFF);
    data << uint32(0);
    data << uint32(0);

    data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);

    data << title;
    data << objectives;
    data << details;

    // quest requirements
    data << uint8(0);
    data << uint32(pQuest->m_questMobId[0])  << uint32(pQuest->m_questMobCount[0]);
    data << uint32(pQuest->m_questItemId[0]) << uint32(pQuest->m_questItemCount[0]);
    data << uint32(pQuest->m_questMobId[1])  << uint32(pQuest->m_questMobCount[1]);
    data << uint32(pQuest->m_questItemId[1]) << uint32(pQuest->m_questItemCount[1]);
    data << uint32(pQuest->m_questMobId[2])  << uint32(pQuest->m_questMobCount[2]);
    data << uint32(pQuest->m_questItemId[2]) << uint32(pQuest->m_questItemCount[2]);
    data << uint32(pQuest->m_questMobId[3])  << uint32(pQuest->m_questMobCount[3]);
    data << uint32(pQuest->m_questItemId[3]) << uint32(pQuest->m_questItemCount[3]);
    data << uint32(0);

    // not sure
    WPAssert(data.size() == 140 + strlen(title)+1 + strlen(details)+1 + strlen(objectives)+1 + 69 + 4);
    SendPacket( &data );
    Log::getSingleton( ).outDebug( "WORLD: Sent SMSG_QUEST_QUERY_RESPONSE" );
}


void WorldSession::HandleQuestgiverChooseRewardOpcode( WorldPacket & recv_data )
{
/*
    Log::getSingleton( ).outString( "WORLD: Recieved CMSG_QUESTGIVER_CHOOSE_REWARD" );

    uint32 guid1,guid2,quest_id,rewardid;
    WorldPacket data;

    recv_data >> guid1 >> guid2 >> quest_id >> rewardid;

    Quest *pQuest = objmgr.getQuest(quest_id);

    data.Initialize( SMSG_QUESTGIVER_QUEST_COMPLETE );
    data << quest_id;
    data.append(uint32(0x03));  // unsure
    data.append(uint32(pQuest->m_questXp));
    data.append(uint32(pQuest->m_rewardGold));
    data << uint32(0x00);
    WPAssert(data.size() == 20);
    SendPacket( &data );

    GetPlayer()->setQuestStatus(quest_id, QUEST_STATUS_COMPLETE);

    if (pQuest->m_targetGuid != 0 && pQuest->m_originalGuid != 0)
    {
        // Do some special actions for "Talk to..." quests
        UpdateMask npcMask;
        npcMask.setCount(UNIT_END);
        npcMask.setBit(OBJECT_FIELD_GUID );
        npcMask.setBit(OBJECT_FIELD_GUID+1 );
        npcMask.setBit(UNIT_NPC_FLAGS );
        // added code to buffer flags and set back so other players don't see the change -RG
        // note that this buffering is *not* thread safe and should be only temporary

        Creature *unit = objmgr.GetObject<Creature>(pQuest->m_originalGuid);
        WPAssert(unit);

        uint32 valuebuffer = unit->GetUInt32Value( UNIT_NPC_FLAGS  );
        unit->SetUInt32Value(UNIT_NPC_FLAGS , uint32(2));
        unit->UpdateObject(&npcMask, &data);
        SendPacket(&data);
        unit->SetUInt32Value(UNIT_NPC_FLAGS , valuebuffer);

        unit = objmgr.GetObject<Creature>( pQuest->m_targetGuid );
        WPAssert(unit);

        valuebuffer = unit->GetUInt32Value(UNIT_NPC_FLAGS );
        unit->SetUInt32Value(UNIT_NPC_FLAGS , uint32(2));
        unit->UpdateObject(&npcMask, &data);
        SendPacket(&data);
        unit->SetUInt32Value(UNIT_NPC_FLAGS , valuebuffer);
    }

    Log::getSingleton( ).outString( "WORLD: Sent SMSG_QUESTGIVER_QUEST_COMPLETE" );

    uint16 log_slot = GetPlayer()->getQuestSlot(quest_id);

    // Set player object with rewards!
    Player *chr = GetPlayer();

    uint32 guid = chr->GetGUIDLow();

    chr->giveXP(pQuest->m_questXp);
    if (pQuest->m_rewardGold > 0)
    {
        uint32 newCoinage = chr->GetUInt32Value(PLAYER_FIELD_COINAGE ) + pQuest->m_rewardGold;
        chr->SetUInt32Value(PLAYER_FIELD_COINAGE , newCoinage);
    }

    chr->SetUInt32Value(log_slot, 0);
    chr->SetUInt32Value(log_slot+1, 0);
    chr->SetUInt32Value(log_slot+2, 0);
    chr->SetUInt32Value(log_slot+3, 0);
*/
}


void WorldSession::HandleQuestgiverRequestRewardOpcode( WorldPacket & recv_data )
{
/*
    // Not really sure what this is all about.  Sent from a SMSG_QUESTGIVER_REQUEST_ITEMS

    uint32 guid1, guid2, quest_id;
    recv_data >> guid1 >> guid2 >> quest_id;

    char *title = sWorld.mCreatures[guid1]->getQuestTitle(quest_id);
    char *details = sWorld.mCreatures[guid1]->getQuestCompleteText(quest_id);

    unsigned char tdata[] =
    {
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
    };

    data.Initialize( 8+4+strlen(title)+1 + strlen(details)+1 +sizeof(tdata), SMSG_QUESTGIVER_OFFER_REWARD );
    data << guid1 << guid2 << uint32( 0x1 );
    data.append( title, strlen(title)+1 );
    data.append( details, strlen(details)+1 );
    data.append( tdata, sizeof(tdata) );
    SendPacket( &data );
    Log::getSingleton( ).outString( "WORLD: Sent SMSG_QUESTGIVER_OFFER_REWARD" );
    GetPlayer()->setQuestStatus(quest_id, QUEST_STATUS_CHOOSE_REWARD);
*/
}


void WorldSession::SetNpcFlagsForTalkToQuest(const uint64 &guid, const uint64 &targetGuid)
{
/*
    // Do some special actions for "Talk to..." quests
    WorldPacket data;
    UpdateMask npcMask;

    npcMask.setCount(UNIT_END);
    npcMask.setBit(UNIT_NPC_FLAGS );

    Creature* pGiver = objmgr.GetObject<Creature>(guid1);
    uint32 valuebuffer = pGiver->GetUInt32Value( UNIT_NPC_FLAGS  );
    pGiver->SetUInt32Value(UNIT_NPC_FLAGS , uint32(0));
    pGiver->UpdateObject(&npcMask, &data);
    SendPacket(&data);
    pGiver->SetUInt32Value(UNIT_NPC_FLAGS , valuebuffer);

    Creature* pTarget = objmgr.GetObject<Creature>(targetGuid);

    valuebuffer = pTarget->GetUInt32Value(UNIT_NPC_FLAGS );
    pTarget->SetUInt32Value(UNIT_NPC_FLAGS , uint32(2));
    pTarget->UpdateObject(&npcMask, &data);
    SendPacket(&data);
    pTarget->SetUInt32Value(UNIT_NPC_FLAGS , valuebuffer);
*/
}


/*  Query Response Header
                uint8 tdata[] =
                {
                    // Flags of some sort?
                    0x02, 0x00, 0x00, 0x00,
                    0x04, 0x00, 0x00, 0x00,
                    0x09, 0x00, 0x00, 0x00,

                    0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, //<-- Reputation Faction!?  If I set above 0, CRASH!  Probably have to set factions first
                    0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x05, 0x00, 0x00, 0x00, // Oooh, gold reward?

0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,

0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

// Item Reward list
0xb0, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, // Item Entry ID and reward count
0xcc, 0x15, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
0x89, 0x04, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
0xcb, 0x15, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
0x87, 0x04, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

0x00, 0x00, 0x00, 0x00, // setting any of these bytes to 1 makes it ignore the Item rewards Above. Ooook then.

0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
*/
/* another Query Response Header
                // rewards?
                uint8 tdata[] =
                {
                    // Flags of some sort?
                    0x2B, 0x02, 0x00, 0x00,
                    0x3E, 0x03, 0x00, 0x00,
                    0x90, 0x00, 0x00, 0x00, // zone id to store quest in log

                    0x02, 0x00, 0x00, 0x00,

0x00, 0x00, 0x00, 0x00, //<-- Reputation Faction!?  If I set above 0, CRASH!  Probably have to set factions first

0x0B, 0x01, 0x00, 0x00,

0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, // Oooh, gold reward
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,

0xF4, 0x01, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,

0x99, 0x0E, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, // Item rewards you always get, no choosing
0x91, 0x0E, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,

0x00, 0x00, 0x00, 0x00,

// Item Reward Choice list
0x82, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, // Item Entry ID and reward count
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

0x00, 0x00, 0x00, 0x00, // setting any of these bytes to 1 makes it ignore the Item rewards Above. Ooook then.

0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
*/

/*
SMSG_QUESTGIVER_OFFER_REWARD
4 bytes
4 bytes
4 bytes
4 bytes
4 bytes - # of items to choose from
  for itemCount
    4 bytes - item entry id
    4 bytes - # in stack
    4 bytes - Item icon (?)

4 bytes - # of items being rewarded
for itemCount
4 bytes - item entry id
4 bytes - # in stack
4 bytes - Item icon (?)
4 bytes - Gold rewarded
4 bytes
*/

/*
SMSG_QUESTGIVER_QUEST_DETAILS

8 bytes - questgiver guid
4 bytes - quest id
string - quest title
string - quest description
string - quest objectives
4 bytes - ?
4 bytes - number of rewards to choose from
  for each item
4 bytes - item name entry id
4 bytes - # in stack
4 bytes - picture id
4 bytes - number of item rewards always awarded
for each item
4 bytes - item name entry id
4 bytes - # in stack
4 bytes - picture id
4 bytes - reward gold
4 bytes ?
4 bytes ?
4 bytes ?
4 bytes ?
4 bytes ?
4 bytes ?
4 bytes ?
4 bytes ?

*/

/*
SMSG_QUEST_QUERY_RESPONSE

4 bytes - quest id
4 bytes - ?
4 bytes - ?
4 bytes - Quest Zone
4 bytes - ?
4 bytes - Faction (?)
4 bytes - ?
4 bytes - ?
4 bytes - ?
4 bytes - ?
4 bytes - Gold rewarded
4 bytes - ?
4 bytes - ?

4 bytes - ?
4 bytes - ?

4 bytes - ?
4 bytes - ?
4 bytes - ?
4 bytes - ?

4 bytes - ?

loop 5 times
4 bytes - Item name entry id
4 bytes - items tack count

4 bytes - ?
4 bytes - ?
4 bytes - ?
4 bytes - ?
4 bytes - ?
4 bytes - ?
string - quest title
string - objectives
string - decription
1 byte - ? some kind of flag ?
4 bytes - Creature to slay entry ID
4 bytes - # of those creature to slay
4 bytes - Item ID to collect
4 bytes - # of those items to collect
4 bytes - Creature to slay entry ID
4 bytes - # of those creature to slay
4 bytes - Item ID to collect
4 bytes - # of those items to collect
4 bytes - Creature to slay entry ID
4 bytes - # of those creature to slay
4 bytes - Item ID to collect
4 bytes - # of those items to collect
4 bytes - Creature to slay entry ID
4 bytes - # of those creature to slay
4 bytes - Item ID to collect
4 bytes - # of those items to collect
4 bytes - ?

*/

/*
SMSG_QUESTUPDATE_ADD_KILL[00000194]

4 bytes - Quest ID
4 bytes - Entry ID of Monster Killed
4 bytes - Number of kills to add?
4 bytes - Total kills required for quest?
8 bytes - Killed Monster GUID
*/
