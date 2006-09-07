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
#include "Util.h"

GossipMenu::GossipMenu()
{
    m_gItemsCount = 0;
}

GossipMenu::~GossipMenu()
{
    ClearMenu();
}

void GossipMenu::AddMenuItem(uint8 Icon, char const * Message, uint32 dtSender, uint32 dtAction, bool Coded)
{
    //    uint64 gtData;

    char* Text = new char[strlen(Message) + 1];
    strcpy( Text, Message );

    m_gItemsCount++;
    ASSERT( m_gItemsCount < GOSSIP_MAX_MENU_ITEMS  );

    m_gItems[m_gItemsCount - 1].m_gIcon     = Icon;
    m_gItems[m_gItemsCount - 1].m_gMessage  = Text;
    m_gItems[m_gItemsCount - 1].m_gCoded    = Coded;
    m_gItems[m_gItemsCount - 1].m_gSender   = dtSender;
    m_gItems[m_gItemsCount - 1].m_gAction   = dtAction;
}

void GossipMenu::AddMenuItem(uint8 Icon, char const * Message, bool Coded)
{
    AddMenuItem( Icon, Message, 0, 0, Coded);
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
    for (unsigned int i=0; i<m_gItemsCount; i++)
        delete[] m_gItems[i].m_gMessage;

    m_gItemsCount = 0;
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
    return pGossipMenu->MenuItemSender( Selection + pQuestMenu->MenuItemCount() );
}

uint32 PlayerMenu::GossipOptionAction( unsigned int Selection )
{
    return pGossipMenu->MenuItemAction( Selection + pQuestMenu->MenuItemCount() );
}

void PlayerMenu::SendGossipMenu( uint32 TitleTextId, uint64 npcGUID )
{
    WorldPacket data;

    data.Initialize( SMSG_GOSSIP_MESSAGE );
    data << npcGUID;
    data << uint32( TitleTextId );
    data << uint32( pGossipMenu->MenuItemCount() );

    for ( int iI = 0; iI < pGossipMenu->MenuItemCount(); iI++ )
    {
        data << uint32( iI );
        data << uint8( pGossipMenu->GetItem(iI).m_gIcon );
        data << uint8( pGossipMenu->GetItem(iI).m_gCoded);
        data << pGossipMenu->GetItem(iI).m_gMessage;
    }

    data << uint32( pQuestMenu->MenuItemCount() );

    for ( uint16 iI = 0; iI < pQuestMenu->MenuItemCount(); iI++ )
    {
        data << uint32( pQuestMenu->GetItem(iI).m_qId );
        data << uint32( pQuestMenu->GetItem(iI).m_qIcon );
        data << uint32( !(pQuestMenu->GetItem(iI).m_qAvailable) );
        data << pQuestMenu->GetItem(iI).m_qTitle;
    }

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_GOSSIP_MESSAGE NPCGuid=%u",GUID_LOPART(npcGUID) );
}

void PlayerMenu::CloseGossip()
{
    WorldPacket data;

    data.Initialize( SMSG_GOSSIP_COMPLETE );
    pSession->SendPacket( &data );

    sLog.outDebug( "WORLD: Sent SMSG_GOSSIP_COMPLETE" );
}

void PlayerMenu::SendPointOfInterest( float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, char const * locName )
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

void PlayerMenu::SendTalking( uint32 textID )
{
    WorldPacket data;

    GossipText *pGossip;
    std::string GossipStr;

    pGossip = objmgr.GetGossipText(textID);

    data.Initialize( SMSG_NPC_TEXT_UPDATE );
    data << textID;

    if (!pGossip)
    {
        data << uint32( 0 );
        data << "Greetings $N";
        data << "Greetings $N";
    } else

    for (int i=0; i<8; i++)
    {
        data << pGossip->Options[i].Probability;
        data << pGossip->Options[i].Text_0;

        if ( pGossip->Options[i].Text_1 == "" )
            data << pGossip->Options[i].Text_0; else
            data << pGossip->Options[i].Text_1;

        data << pGossip->Options[i].Language;

        data << pGossip->Options[i].Emotes[0]._Delay;
        data << pGossip->Options[i].Emotes[0]._Emote;

        data << pGossip->Options[i].Emotes[1]._Delay;
        data << pGossip->Options[i].Emotes[1]._Emote;

        data << pGossip->Options[i].Emotes[2]._Delay;
        data << pGossip->Options[i].Emotes[2]._Emote;
    }

    pSession->SendPacket( &data );

    sLog.outString( "WORLD: Sent SMSG_NPC_TEXT_UPDATE " );
}

void PlayerMenu::SendTalking( char const * title, char const * text )
{
    WorldPacket data;

    data.Initialize( SMSG_NPC_TEXT_UPDATE );
    data << uint32( 0 );
    data << uint32( 0 );
    data << title;
    data << text;

    pSession->SendPacket( &data );

    sLog.outString( "WORLD: Sent SMSG_NPC_TEXT_UPDATE " );
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/

QuestMenu::QuestMenu()
{
    m_qItemsCount = 0;
}

QuestMenu::~QuestMenu()
{
    ClearMenu();
}

void QuestMenu::AddMenuItem( uint32 QuestId, uint8 Icon, bool Available)
{
    QuestInfo const *qinfo = objmgr.GetQuestInfo(QuestId);
    if (!qinfo) return;

    char* Text = new char[strlen( qinfo->Title ) + 1];
    strcpy( Text, qinfo->Title );

    m_qItemsCount++;
    ASSERT( m_qItemsCount < GOSSIP_MAX_MENU_ITEMS  );

    m_qItems[m_qItemsCount - 1].m_qIcon      = Icon;
    m_qItems[m_qItemsCount - 1].m_qTitle     = Text;
    m_qItems[m_qItemsCount - 1].m_qId        = QuestId;
    m_qItems[m_qItemsCount - 1].m_qAvailable = Available;
}

bool QuestMenu::HasItem( uint32 questid )
{
    for(int i=0;i<m_qItemsCount;i++)
    {
        if(m_qItems[i].m_qId==questid)
        {
            return true;
        }
    }
    return false;
}

void QuestMenu::ClearMenu()
{
    for (int i=0; i<m_qItemsCount; i++)
        delete[] m_qItems[i].m_qTitle;

    m_qItemsCount = 0;
}

void PlayerMenu::SendQuestMenu( QEmote eEmote, std::string Title, uint64 npcGUID )
{
    WorldPacket data;

    data.Initialize( SMSG_QUESTGIVER_QUEST_LIST );
    data << uint64(npcGUID);
    data << Title;
    data << uint32(eEmote._Delay );                         //zyg: player emote
    data << uint32(eEmote._Emote );                         //zyg: NPC emote
    data << uint8 ( pQuestMenu->MenuItemCount() );

    for ( uint16 iI = 0; iI < pQuestMenu->MenuItemCount(); iI++ )
    {
        QuestMenuItem qmi=pQuestMenu->GetItem(iI);
        data << uint32( qmi.m_qId );
        data << uint32((qmi.m_qAvailable)?5:3);
        //data << uint32( qmi.m_qIcon );
        data << uint32(0x00);
        data << qmi.m_qTitle;
    }
    pSession->SendPacket( &data );
    uint32 fqid=pQuestMenu->GetItem(0).m_qId;
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_QUEST_LIST NPC Guid=%u, questid-0=%u",npcGUID,fqid);
}

void PlayerMenu::SendQuestStatus( uint32 questStatus, uint64 npcGUID )
{
    WorldPacket data;

    data.Initialize( SMSG_QUESTGIVER_STATUS );
    data << npcGUID << questStatus;

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_STATUS NPC Guid=%u, status=%u",GUID_LOPART(npcGUID),questStatus);
}

void PlayerMenu::SendQuestDetails( Quest *pQuest, uint64 npcGUID, bool ActivateAccept)
{
    WorldPacket data;
    data.Initialize(SMSG_QUESTGIVER_QUEST_DETAILS);

    QuestInfo const* qInfo = pQuest->GetQuestInfo();

    data << npcGUID;
    data << qInfo->QuestId << qInfo->Title << qInfo->Details;
    data << qInfo->Objectives << uint32( ActivateAccept );
    ItemPrototype const* IProto;

    data << pQuest->m_rewchoiceitemscount;
    for (uint32 i=0; i <QUEST_REWARD_CHOICES_COUNT; i++)
    {
        data << uint32(qInfo->RewChoiceItemId[i]);
        data << uint32(qInfo->RewChoiceItemCount[i]);
        IProto = objmgr.GetItemPrototype(qInfo->RewChoiceItemId[i]);
        if ( IProto )
            data << uint32(IProto->DisplayInfoID);
        else
            data << uint32( 0x00 );
    }

    data << pQuest->m_rewitemscount;
    for (uint32 i=0; i <QUEST_REWARDS_COUNT; i++)
    {
        data << qInfo->RewItemId[i];
        data << qInfo->RewItemCount[i];
        IProto = objmgr.GetItemPrototype(qInfo->RewItemId[i]);
        if ( IProto )
            data << IProto->DisplayInfoID;
        else
            data << uint32( 0x00 );
    }

    data << uint32(qInfo->RewOrReqMoney);

    data << pQuest->m_reqitemscount;
    for (uint32 i=0; i <  QUEST_OBJECTIVES_COUNT; i++)
    {
        data << qInfo->ReqItemId[i];
        data << qInfo->ReqItemCount[i];
    }

    data << pQuest->m_reqmobs_or_GO_count;
    for (uint32 i=0; i <QUEST_OBJECTIVES_COUNT; i++)
    {
        data << qInfo->ReqKillMobOrGOId[i];
        data << qInfo->ReqKillMobOrGOCount[i];
    }

    pSession->SendPacket( &data );
    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS NPCGuid=%u, questid=%u",GUID_LOPART(npcGUID),pQuest->GetQuestId());
}

void PlayerMenu::SendUpdateQuestDetails ( Quest *pQuest )
{
    WorldPacket data;

    data.Initialize( SMSG_QUEST_QUERY_RESPONSE );

    data << uint32(pQuest->GetQuestInfo()->QuestId);
    data << uint32(pQuest->GetQuestInfo()->MinLevel);
    data << uint32(pQuest->GetQuestInfo()->QuestLevel);

    if(pQuest->GetQuestInfo()->QuestSort > 0)
        data << uint32(pQuest->GetQuestInfo()->QuestSort * -1);
    else
        data << uint32(pQuest->GetQuestInfo()->ZoneId);

    data << uint32(pQuest->GetQuestInfo()->Type);
    data << uint32(pQuest->GetQuestInfo()->RequiredRepFaction);
    data << uint32(pQuest->GetQuestInfo()->RequiredRepValue);
    data << uint32(0);
    data << uint32(0);

    data << uint32(pQuest->GetQuestInfo()->NextQuestId);
    data << uint32(pQuest->GetQuestInfo()->RewOrReqMoney);
    data << uint32(pQuest->GetQuestInfo()->RewXP);

    // check if RewSpell is teaching another spell
    if(pQuest->GetQuestInfo()->RewSpell)
    {
        SpellEntry *rewspell = sSpellStore.LookupEntry(pQuest->GetQuestInfo()->RewSpell);
        if(rewspell)
        {
            if(rewspell->Effect[0] == SPELL_EFFECT_LEARN_SPELL)
                data << uint32(rewspell->EffectTriggerSpell[0]);
            else
                data << uint32(pQuest->GetQuestInfo()->RewSpell);
        }
        else
        {
            sLog.outError("Quest %u have non-existed RewSpell %u, ignored.",pQuest->GetQuestInfo()->QuestId,pQuest->GetQuestInfo()->RewSpell);
            data << uint32(0);
        }
    }
    else
        data << uint32(0);

    data << uint32(pQuest->GetQuestInfo()->SrcItemId);
    data << uint32(pQuest->GetQuestInfo()->SpecialFlags);

    int iI;

    for (iI = 0; iI < QUEST_REWARDS_COUNT; iI++)
    {
        data << uint32(pQuest->GetQuestInfo()->RewItemId[iI]);
        data << uint32(pQuest->GetQuestInfo()->RewItemCount[iI]);
    }
    for (iI = 0; iI < QUEST_REWARD_CHOICES_COUNT; iI++)
    {
        data << uint32(pQuest->GetQuestInfo()->RewChoiceItemId[iI]);
        data << uint32(pQuest->GetQuestInfo()->RewChoiceItemCount[iI]);
    }

    data << pQuest->GetQuestInfo()->PointMapId;
    data << pQuest->GetQuestInfo()->PointX;
    data << pQuest->GetQuestInfo()->PointY;
    data << pQuest->GetQuestInfo()->PointOpt;

    data << pQuest->GetQuestInfo()->Title;
    data << pQuest->GetQuestInfo()->Objectives;
    data << pQuest->GetQuestInfo()->Details;
    data << pQuest->GetQuestInfo()->EndText;

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; iI++)
    {
        data << uint32(pQuest->GetQuestInfo()->ReqKillMobOrGOId[iI])  << uint32(pQuest->GetQuestInfo()->ReqKillMobOrGOCount[iI]);
        data << uint32(pQuest->GetQuestInfo()->ReqItemId[iI]) << uint32(pQuest->GetQuestInfo()->ReqItemCount[iI]);
    }

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; iI++)
        data << pQuest->GetQuestInfo()->ObjectiveText[iI];

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUEST_QUERY_RESPONSE questid=%u",pQuest->GetQuestInfo()->QuestId );
}

void PlayerMenu::SendQuestReward( uint32 quest_id, uint64 npcGUID, bool EnbleNext, QEmote Emotes[], unsigned int EmoteCnt )
{
    QuestInfo const* qInfo = objmgr.GetQuestInfo(quest_id);
    if(!qInfo)
        return;

    WorldPacket data;
    data.Initialize( SMSG_QUESTGIVER_OFFER_REWARD );

    data << npcGUID;
    data << quest_id;
    data << qInfo->Title;
    data << qInfo->Details;

    data << uint32( EnbleNext );

    if ( EmoteCnt > 0 )
    {
        data << uint32( EmoteCnt );

        for (unsigned int iI = 0; iI < EmoteCnt; iI++ )
        {
            data << Emotes[iI]._Delay;
            data << Emotes[iI]._Emote;
        }
    } else data << uint32( 0x00 );

    Quest* pQuest =  pSession->GetPlayer()->GetActiveQuest(quest_id);

    ItemPrototype const *pItem;

    data << uint32(pQuest->m_rewchoiceitemscount);
    for (uint32 i=0; i < pQuest->m_rewchoiceitemscount; i++)
    {
        pItem = objmgr.GetItemPrototype( qInfo->RewChoiceItemId[i] );

        data << uint32(qInfo->RewChoiceItemId[i]);
        data << uint32(qInfo->RewChoiceItemCount[i]);

        if ( pItem )
            data << uint32(pItem->DisplayInfoID);
        else
            data << uint32(0);
    }

    data << uint32(pQuest->m_rewitemscount);
    for (uint16 i=0; i < pQuest->m_rewitemscount; i++)
    {
        pItem = objmgr.GetItemPrototype(qInfo->RewItemId[i]);
        data << uint32(qInfo->RewItemId[i]) << uint32(qInfo->RewItemCount[i]);

        if ( pItem )
            data << uint32(pItem->DisplayInfoID);
        else
            data << uint32(0);
    }

    data << uint32(qInfo->RewOrReqMoney);
    data << uint32(0x00);

    // check if RewSpell is teaching another spell
    if(pQuest->GetQuestInfo()->RewSpell)
    {
        SpellEntry *rewspell = sSpellStore.LookupEntry(pQuest->GetQuestInfo()->RewSpell);
        if(rewspell)
        {
            if(rewspell->Effect[0] == SPELL_EFFECT_LEARN_SPELL)
                data << uint32(rewspell->EffectTriggerSpell[0]);
            else
                data << uint32(pQuest->GetQuestInfo()->RewSpell);
        }
        else
        {
            sLog.outError("Quest %u have non-existed RewSpell %u, ignored.",pQuest->GetQuestInfo()->QuestId,pQuest->GetQuestInfo()->RewSpell);
            data << uint32(0);
        }
    }
    else
        data << uint32(0);

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_OFFER_REWARD NPCGuid=%u, questid=%u",GUID_LOPART(npcGUID),quest_id );
}

void PlayerMenu::SendRequestedItems( Quest *pQuest, uint64 npcGUID, bool Completable, bool CloseOnCancel )
{
    WorldPacket data;

    data.Initialize( SMSG_QUESTGIVER_REQUEST_ITEMS);
    data << npcGUID;
    data << pQuest->GetQuestInfo()->QuestId;
    data << pQuest->GetQuestInfo()->Title;

    if ( !Completable )
    {
        if ( strlen(pQuest->GetQuestInfo()->IncompleteText)>0 )
            data << pQuest->GetQuestInfo()->IncompleteText;
        else
            data << pQuest->GetQuestInfo()->Details;
    }
    else
    {
        if ( strlen(pQuest->GetQuestInfo()->CompletionText)>0 )
            data << pQuest->GetQuestInfo()->CompletionText;
        else
            data << pQuest->GetQuestInfo()->Details;
    }

    data << uint32(0x00);
    // Emote
    data << uint32(0x01);
    // Close Window after cancel
    if (CloseOnCancel)
        data << uint32(0x01);
    else
        data << uint32(0x00);
    // Req Money
    data << uint32(pQuest->GetQuestInfo()->RewOrReqMoney < 0 ? -pQuest->GetQuestInfo()->RewOrReqMoney : 0);

    data << uint32( pQuest->m_reqitemscount );

    ItemPrototype const *pItem;
    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if ( !pQuest->GetQuestInfo()->ReqItemId[i] ) continue;

        pItem = objmgr.GetItemPrototype(pQuest->GetQuestInfo()->ReqItemId[i]);
        data << uint32(pQuest->GetQuestInfo()->ReqItemId[i]);
        data << uint32(pQuest->GetQuestInfo()->ReqItemCount[i]);

        if ( pItem )
            data << uint32(pItem->DisplayInfoID);
        else
            data << uint32(0);
    }

    data << uint32(0x02);
    if ( !Completable )
        data << uint32(0x00);
    else
        data << uint32(0x03);
    data << uint32(0x04) << uint32(0x08) << uint32(0x10);

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS NPCGuid=%u, questid=%u",GUID_LOPART(npcGUID),pQuest->GetQuestInfo()->QuestId );
}
