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
#include "GossipDef.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include <string.h>
#include "WorldPacket.h"
#include "WorldSession.h"

GossipMenu::GossipMenu()
{
    m_gItemsCount = 0;
}

GossipMenu::~GossipMenu()
{
    ClearMenu();
}

void GossipMenu::MenuItem(uint8 Icon, char const* Message, uint32 dtSender, uint32 dtAction, bool Coded)
{
    //	uint64 gtData;

    char* Text = new char[strlen(Message) + 1];
    strcpy( Text, Message );

    m_gItemsCount++;
    ASSERT( m_gItemsCount < GOSSIP_MAX_MENU_ITEMS  );

    m_gItems[m_gItemsCount - 1].m_gIcon       = Icon;
    m_gItems[m_gItemsCount - 1].m_gMessage  = Text;
    m_gItems[m_gItemsCount - 1].m_gCoded    = Coded;
    m_gItems[m_gItemsCount - 1].m_gSender   = dtSender;
    m_gItems[m_gItemsCount - 1].m_gAction   = dtAction;
}

void GossipMenu::MenuItem(uint8 Icon, char const * Message, bool Coded)
{
    MenuItem( Icon, Message, 0, 0, Coded);
}

uint32 GossipMenu::MenuItemSender( unsigned int ItemId )
{
    if ( ItemId >= m_gItemsCount ) return 0;

    return m_gItems[ ItemId ].m_gSender;
}

uint32 GossipMenu::MenuItemAction( unsigned int ItemId )
{
    if ( ItemId >= m_gItemsCount ) return 0;

    return m_gItems[ ItemId ].m_gAction;
}

void GossipMenu::ClearMenu()
{
    for (int i=0; i<m_gItemsCount; i++)
        delete m_gItems[i].m_gMessage;

    m_gItemsCount = 0;
}

QuestMenu::QuestMenu()
{
    m_qItemsCount = 0;
}

QuestMenu::~QuestMenu()
{
    ClearMenu();
}

void QuestMenu::QuestItem( uint32 QuestId, uint8 Icon, bool Available)
{
    Quest *pQuest = objmgr.GetQuest(QuestId);
    if (!pQuest) return;

    char* Text = new char[strlen( pQuest->m_qTitle.c_str() ) + 1];
    strcpy( Text, pQuest->m_qTitle.c_str() );

    m_qItemsCount++;
    ASSERT( m_qItemsCount < GOSSIP_MAX_MENU_ITEMS  );

    m_qItems[m_qItemsCount - 1].m_qIcon      = Icon;
    m_qItems[m_qItemsCount - 1].m_qTitle     = Text;
    m_qItems[m_qItemsCount - 1].m_qId        = QuestId;
    m_qItems[m_qItemsCount - 1].m_qAvailable = Available;
}

void QuestMenu::ClearMenu()
{
    for (int i=0; i<m_qItemsCount; i++)
        delete m_qItems[i].m_qTitle;

    m_qItemsCount = 0;
}

PlayerMenu::PlayerMenu( WorldSession *Session )
{
    pGossipMenu = new GossipMenu();
    pQuestMenu  = new QuestMenu();
    pSession    = Session;
}

PlayerMenu::~PlayerMenu()
{
    delete pGossipMenu;
    delete pQuestMenu;
}

void PlayerMenu::ClearMenus()
{
    pGossipMenu->ClearMenu();
    pQuestMenu->ClearMenu();
}

uint32 PlayerMenu::GossipOptionSender( unsigned int Selection )
{
    return pGossipMenu->MenuItemSender( Selection + pQuestMenu->QuestsInMenu() );
}

uint32 PlayerMenu::GossipOptionAction( unsigned int Selection )
{
    return pGossipMenu->MenuItemAction( Selection + pQuestMenu->QuestsInMenu() );
}

void PlayerMenu::SendGossipMenu( uint32 TitleTextId, uint64 npcGUID )
{
    WorldPacket data;

    data.Initialize( SMSG_GOSSIP_MESSAGE );
    data << npcGUID;
    data << uint32( TitleTextId );
    data << uint32( pGossipMenu->ItemsInMenu() );

    for ( int iI = 0; iI < pGossipMenu->ItemsInMenu(); iI++ )
    {
        data << uint32( iI );
        data << uint8( pGossipMenu->GetItem(iI).m_gIcon );
        data << uint8( pGossipMenu->GetItem(iI).m_gCoded);
        data << pGossipMenu->GetItem(iI).m_gMessage;
    }

    data << uint32( pQuestMenu->QuestsInMenu() );

    for ( int iI = 0; iI < pQuestMenu->QuestsInMenu(); iI++ )
    {
        data << uint32( pQuestMenu->GetItem(iI).m_qId );
        data << uint32( pQuestMenu->GetItem(iI).m_qIcon );
        data << uint32( !(pQuestMenu->GetItem(iI).m_qAvailable) );
        data << pQuestMenu->GetItem(iI).m_qTitle;
    }

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_GOSSIP_MESSAGE" );
}

void PlayerMenu::SendQuestMenu( QEmote eEmote, std::string Title, uint64 npcGUID )
{
    WorldPacket data;

    data.Initialize( SMSG_QUESTGIVER_QUEST_LIST );
    data << npcGUID;
    data << Title;
    data << uint32( eEmote._Delay );
    data << uint32( eEmote._Emote );
    data << uint8 ( pQuestMenu->QuestsInMenu() );

    for ( int iI = 0; iI < pQuestMenu->QuestsInMenu(); iI++ )
    {
        data << uint32(  pQuestMenu->GetItem(iI).m_qId );
        data << uint32( (pQuestMenu->GetItem(iI).m_qAvailable)?0x05:0x03 );
        data << uint32(0x00);
        data << pQuestMenu->GetItem(iI).m_qTitle;
    }

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_QUEST_LIST" );
}

void PlayerMenu::CloseGossip()
{
    WorldPacket data;

    data.Initialize( SMSG_GOSSIP_COMPLETE );
    pSession->SendPacket( &data );

    sLog.outDebug( "WORLD: Sent SMSG_GOSSIP_COMPLETE" );
}

void PlayerMenu::SendQuestStatus( uint32 questStatus, uint64 npcGUID )
{
    WorldPacket data;

    data.Initialize( SMSG_QUESTGIVER_STATUS );
    data << npcGUID << questStatus;

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_STATUS");
}

void PlayerMenu::SendQuestDetails( Quest *pQuest, uint64 npcGUID, bool ActivateAccept)
{
    WorldPacket data;
    data.Initialize(SMSG_QUESTGIVER_QUEST_DETAILS);

    data << npcGUID;
    data << pQuest->m_qId << pQuest->m_qTitle << pQuest->m_qObjectives;
    data << pQuest->m_qDetails << uint32( ActivateAccept );

    ItemPrototype* IProto;
    int i;

    data << pQuest->m_qRewChoicesCount;
    for (i=0; i < pQuest->m_qRewChoicesCount; i++)
    {
        data << uint32(pQuest->m_qRewChoicesItemId[i]);
        data << uint32(pQuest->m_qRewChoicesItemCount[i]);
        IProto = objmgr.GetItemPrototype(pQuest->m_qRewChoicesItemId[i]);
        if ( IProto ) data << uint32(IProto->DisplayInfoID); else
            data << uint32( 0x00 );
    }

    data << pQuest->m_qRewCount;
    for (i=0; i < pQuest->m_qRewCount; i++)
    {
        data << pQuest->m_qRewItemId[i];
        data << pQuest->m_qRewItemCount[i];
        IProto = objmgr.GetItemPrototype(pQuest->m_qRewItemId[i]);
        if ( IProto ) data << IProto->DisplayInfoID; else
            data << uint32( 0x00 );
    }

    data << pQuest->m_qRewMoney;

    uint32 Objs = pQuest->GetDeliverObjectivesCount();
    data << Objs;

    for (i=0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        data << pQuest->m_qObjItemId[i];
        data << pQuest->m_qObjItemCount[i];
    }

    Objs = pQuest->GetKillObjectivesCount();
    data << Objs;
    for (i=0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        data << pQuest->m_qObjMobId[i];
        data << pQuest->m_qObjMobCount[i];
    }

    pSession->SendPacket( &data );
    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS");
}

void PlayerMenu::SendQuestReward( Quest *pQuest, uint64 npcGUID, bool EnbleNext, QEmote Emotes[], unsigned int EmoteCnt )
{
    WorldPacket data;
    data.Initialize( SMSG_QUESTGIVER_OFFER_REWARD );

    data << npcGUID;
    data << pQuest->m_qId;
    data << pQuest->m_qTitle;
    data << pQuest->m_qDetails;

    data << uint32( EnbleNext );

    if ( EmoteCnt > 0 )
    {
        data << uint32( EmoteCnt );

        for (int iI = 0; iI < EmoteCnt; iI++ )
        {
            data << Emotes[iI]._Delay;
            data << Emotes[iI]._Emote;
        }
    } else data << uint32( 0x00 );

    ItemPrototype *pItem;

    data << uint32(pQuest->m_qRewChoicesCount);
    for (uint16 i=0; i < pQuest->m_qRewChoicesCount; i++)
    {
        pItem = objmgr.GetItemPrototype( pQuest->m_qRewChoicesItemId[i] );

        data << uint32(pQuest->m_qRewChoicesItemId[i]);
        data << uint32(pQuest->m_qRewChoicesItemCount[i]);

        if ( pItem )
            data << uint32(pItem->DisplayInfoID); else
            data << uint32(0);
    }

    data << uint32(pQuest->m_qRewCount);
    for (uint16 i=0; i < pQuest->m_qRewCount; i++)
    {
        pItem = objmgr.GetItemPrototype(pQuest->m_qRewItemId[i]);
        data << uint32(pQuest->m_qRewItemId[i]) << uint32(pQuest->m_qRewItemCount[i]);

        if ( pItem )
            data << uint32(pItem->DisplayInfoID); else
            data << uint32(0);
    }

    data << uint32(pQuest->m_qRewMoney);
    data << uint32(0x00);
    data << uint32(pQuest->m_qRewSpell);

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_OFFER_REWARD" );
}

void PlayerMenu::SendRequestedItems( Quest *pQuest, uint64 npcGUID, bool Completable )
{
    WorldPacket data;

    data.Initialize( SMSG_QUESTGIVER_REQUEST_ITEMS);
    data << npcGUID;
    data << pQuest->m_qId;
    data << pQuest->m_qTitle;

    if ( !Completable )
    {
        if ( pQuest->m_qIncompleteInfo != "" )
            data << pQuest->m_qIncompleteInfo; else
            data << pQuest->m_qDetails;
    } else
    {
        if ( pQuest->m_qCompletionInfo != "" )
            data << pQuest->m_qCompletionInfo; else
            data << pQuest->m_qDetails;
    }

    data << uint32(0x00) << uint32(0x01);
    data << uint32(0x01) << uint32(0x00);

    data << uint32( pQuest->GetDeliverObjectivesCount() );

    ItemPrototype *pItem;
    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if ( !pQuest->m_qObjItemId[i] ) continue;

        pItem = objmgr.GetItemPrototype(pQuest->m_qObjItemId[i]);
        data << uint32(pQuest->m_qObjItemId[i]);
        data << uint32(pQuest->m_qObjItemCount[i]);

        if ( pItem )
            data << uint32(pItem->DisplayInfoID); else
            data << uint32(0);
    }

    data << uint32(0x02) << uint32(0x00);
    data << uint32(0x04) << uint32(0x08) << uint32(0x10);

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS" );
}

void PlayerMenu::SendUpdateQuestDetails ( Quest *pQuest )
{
    WorldPacket data;

    data.Initialize( SMSG_QUEST_QUERY_RESPONSE );

    data << uint32(pQuest->m_qId);
    data << uint32(pQuest->m_qPlayerLevel);
    data << uint32(pQuest->m_qComplexityLevel);
    data << uint32(pQuest->m_qCategory);

    data << uint32(pQuest->m_qType);
    data << uint32(pQuest->m_qObjRepFaction_1);
    data << uint32(pQuest->m_qObjRepValue_1);
    data << uint32(pQuest->m_qObjRepFaction_2);
    data << uint32(pQuest->m_qObjRepValue_2);

    data << uint32(pQuest->m_qNextQuest);
    data << uint32(pQuest->m_qRewMoney);

    data << uint32(pQuest->m_qRewSpell);

    data << uint32(pQuest->m_qQuestItem);
    data << uint32(pQuest->m_qFlags);

    int iTotals = 8;
    int iI;

    for (iI = 0; iI < pQuest->m_qRewCount; iI++)
    {
        data << uint32(pQuest->m_qRewItemId[iI]);
        data << uint32(pQuest->m_qRewItemCount[iI]);
        iTotals -= 2;
    }

    for (iI = 0; iI < iTotals; iI++) data << uint32(0x00);

    iTotals = 12;

    for (iI = 0; iI < pQuest->m_qRewChoicesCount; iI++)
    {
        data << uint32(pQuest->m_qRewChoicesItemId[iI]);
        data << uint32(pQuest->m_qRewChoicesItemCount[iI]);
        iTotals -= 2;
    }

    for (iI = 0; iI < iTotals; iI++) data << uint32(0x00);

    data << pQuest->m_qPointId;
    data << pQuest->m_qPointX;
    data << pQuest->m_qPointY;
    data << pQuest->m_qPointOpt;

    data << pQuest->m_qTitle;
    data << pQuest->m_qObjectives;
    data << pQuest->m_qDetails;
    data << pQuest->m_qEndInfo;

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; iI++)
    {
        data << uint32(pQuest->m_qObjMobId[iI])  << uint32(pQuest->m_qObjMobCount[iI]);
        data << uint32(pQuest->m_qObjItemId[iI]) << uint32(pQuest->m_qObjItemCount[iI]);
    }

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; iI++)
        data << pQuest->m_qObjectiveInfo[iI];

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUEST_QUERY_RESPONSE" );
}

void PlayerMenu::SendQuestComplete( Quest *pQuest )
{
    WorldPacket data;

    data.Initialize( SMSG_QUESTGIVER_QUEST_COMPLETE );
    data << pQuest->m_qId;
    data << uint32(0x03);
    data << pQuest->XPValue( pSession->GetPlayer() );
    data << pQuest->m_qRewMoney;

    uint32 iNr = 0;
    for (int iI=0; iI< QUEST_REWARDS_COUNT; iI++)
        if (pQuest->m_qRewItemId[iI] > 0) iNr++;

    data << uint32( iNr );

    for (int iI = 0; iI < QUEST_REWARDS_COUNT; iI++)
        if (pQuest->m_qRewItemId[iI] > 0)
    {
        data << pQuest->m_qRewItemId[iI] << pQuest->m_qRewItemCount[iI];
    }

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_QUEST_COMPLETE" );
}

void PlayerMenu::SendQuestUpdateComplete( Quest *pQuest )
{
    WorldPacket data;
    data.Initialize( SMSG_QUESTUPDATE_COMPLETE );

    data << pQuest->m_qId;
    pSession->SendPacket( &data );

    sLog.outDebug( "WORLD: Sent SMSG_QUESTUPDATE_COMPLETE" );
}

void PlayerMenu::SendQuestCompleteToLog( Quest *pQuest )
{
    uint16 log_slot   = pSession->GetPlayer()->getQuestSlot( pQuest->m_qId );
    uint32 updt       = pSession->GetPlayer()->GetUInt32Value( log_slot + 1 );
    updt             |= 0x01000000;

    pSession->GetPlayer()->SetUInt32Value( log_slot + 1, updt );
}

void PlayerMenu::SendQuestIncompleteToLog( Quest *pQuest )
{
    uint16 log_slot   = pSession->GetPlayer()->getQuestSlot( pQuest->m_qId );
    uint32 vle1       = pSession->GetPlayer()->GetUInt32Value( log_slot + 0 );

    pSession->GetPlayer()->SetUInt32Value( log_slot + 0 , vle1 );
    pSession->GetPlayer()->SetUInt32Value( log_slot + 1 , 0 );
    pSession->GetPlayer()->SetUInt32Value( log_slot + 2 , 0 );
}

void PlayerMenu::SendQuestLogFull()
{
    WorldPacket data;

    data.Initialize( SMSG_QUESTLOG_FULL );
    pSession->SendPacket( &data );

    sLog.outDebug( "WORLD: Sent QUEST_LOG_FULL_MESSAGE" );
}

void PlayerMenu::SendQuestUpdateAddItem( Quest *pQuest, uint32 iLogItem, uint32 iLogNr)
{
    WorldPacket data;

    data.Initialize( SMSG_QUESTUPDATE_ADD_ITEM );

    data << pQuest->m_qObjItemId[iLogItem];
    data << uint32(iLogNr);

    pSession->SendPacket( &data );
}

void PlayerMenu::SendQuestUpdateAddKill( Quest *pQuest, uint64 mobGUID, uint32 iNrMob, uint32 iLogMob )
{

    WorldPacket data;
    data.Initialize( SMSG_QUESTUPDATE_ADD_KILL );

    data << pQuest->m_qId;
    data << pQuest->m_qObjMobId[ iLogMob ];
    data << uint32(iNrMob);
    data << pQuest->m_qObjMobCount[ iLogMob ];
    data << mobGUID;

    pSession->SendPacket(&data);
    sLog.outDebug( "WORLD: Sent SMSG_QUESTUPDATE_ADD_KILL" );

    if (pSession->GetPlayer() != NULL)
    {
        uint16 log_slot   = pSession->GetPlayer()->getQuestSlot( pQuest->m_qId );
        uint32 kills      = pSession->GetPlayer()->GetUInt32Value( log_slot + 1 );
        kills             = kills + (1 << ( 6 * iLogMob ));
        pSession->GetPlayer()->SetUInt32Value( log_slot + 1, kills );
    }
}

void PlayerMenu::SendQuestUpdateSetTimer( Quest *pQuest, uint32 TimerValue)
{
    uint16 log_slot   = pSession->GetPlayer()->getQuestSlot( pQuest->m_qId );
    time_t pk         = time(NULL);
    pk += (TimerValue * 60);
    pSession->GetPlayer()->SetUInt32Value( log_slot + 2, pk );
}

void PlayerMenu::SendQuestUpdateFailed( Quest *pQuest )
{
    WorldPacket data;
    data.Initialize( SMSG_QUESTUPDATE_FAILED );

    data << uint32(pQuest->m_qId);
    pSession->SendPacket( &data );

    sLog.outDebug( "WORLD: Sent SMSG_QUESTUPDATE_FAILED" );
}

void PlayerMenu::SendQuestUpdateFailedTimer( Quest *pQuest )
{
    WorldPacket data;

    data.Initialize( SMSG_QUESTUPDATE_FAILEDTIMER );

    data << uint32(pQuest->m_qId);
    pSession->SendPacket( &data );

    sLog.outDebug( "WORLD: Sent SMSG_QUESTUPDATE_FAILEDTIMER" );

}

void PlayerMenu::SendPointOfInterest( float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, const char *locName )
{
    WorldPacket data;

    data.Initialize( SMSG_GOSSIP_POI );
    data << Flags;
    data << X << Y;
    data << uint32(Icon);
    data << uint32(Data);
    data << locName;

    pSession->SendPacket( &data );
    sLog.outDebug("WORLD: Sent SMSG_GOSSIP_POI");
}

void PlayerMenu::SendQuestFailed( uint32 iReason )
{
    WorldPacket data;

    data.Initialize( SMSG_QUESTGIVER_QUEST_FAILED );
    data << iReason;

    pSession->SendPacket( &data );

    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_FAILED");
}

void PlayerMenu::SendQuestInvalid( uint32 iReason )
{
    WorldPacket data;

    data.Initialize( SMSG_QUESTGIVER_QUEST_INVALID );
    data << iReason;

    pSession->SendPacket( &data );

    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_INVALID");
}
