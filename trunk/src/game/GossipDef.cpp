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
    WorldPacket data( SMSG_GOSSIP_MESSAGE, (100) );         // guess size
    data << npcGUID;
    data << uint32( TitleTextId );
    data << uint32( pGossipMenu->MenuItemCount() );

    for ( unsigned int iI = 0; iI < pGossipMenu->MenuItemCount(); iI++ )
    {
        data << uint32( iI );
        data << uint8( pGossipMenu->GetItem(iI).m_gIcon );
        data << uint8( pGossipMenu->GetItem(iI).m_gCoded);
        data << pGossipMenu->GetItem(iI).m_gMessage;
    }

    data << uint32( pQuestMenu->MenuItemCount() );

    for ( uint16 iI = 0; iI < pQuestMenu->MenuItemCount(); iI++ )
    {
        uint32 questID = pQuestMenu->GetItem(iI).m_qId;
        data << questID;
        data << uint32( pQuestMenu->GetItem(iI).m_qIcon );
        data << uint32( objmgr.QuestTemplates[questID]->GetQuestLevel() );
        data << objmgr.QuestTemplates[questID]->GetTitle();
    }

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_GOSSIP_MESSAGE NPCGuid=%u",GUID_LOPART(npcGUID) );
}

void PlayerMenu::CloseGossip()
{
    WorldPacket data( SMSG_GOSSIP_COMPLETE, 0 );
    pSession->SendPacket( &data );

    sLog.outDebug( "WORLD: Sent SMSG_GOSSIP_COMPLETE" );
}

void PlayerMenu::SendPointOfInterest( float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, char const * locName )
{
    WorldPacket data( SMSG_GOSSIP_POI, (4+4+4+4+4+10) );    // guess size
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
    GossipText *pGossip;
    std::string GossipStr;

    pGossip = objmgr.GetGossipText(textID);

    WorldPacket data( SMSG_NPC_TEXT_UPDATE, 100 );          // guess size
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

    sLog.outDetail( "WORLD: Sent SMSG_NPC_TEXT_UPDATE " );
}

void PlayerMenu::SendTalking( char const * title, char const * text )
{
    WorldPacket data( SMSG_NPC_TEXT_UPDATE, 50 );           // guess size
    data << uint32( 0 );
    data << uint32( 0 );
    data << title;
    data << text;

    pSession->SendPacket( &data );

    sLog.outDetail( "WORLD: Sent SMSG_NPC_TEXT_UPDATE " );
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

void QuestMenu::AddMenuItem( uint32 QuestId, uint8 Icon)
{
    Quest * qinfo = objmgr.QuestTemplates[QuestId];
    if (!qinfo) return;

    m_qItemsCount++;
    ASSERT( m_qItemsCount < GOSSIP_MAX_MENU_ITEMS  );

    m_qItems[m_qItemsCount - 1].m_qId        = QuestId;
    m_qItems[m_qItemsCount - 1].m_qIcon      = Icon;
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
    m_qItemsCount = 0;
}

void PlayerMenu::SendQuestGiverQuestList( QEmote eEmote, std::string Title, uint64 npcGUID )
{
    WorldPacket data( SMSG_QUESTGIVER_QUEST_LIST, 100 );    // guess size
    data << uint64(npcGUID);
    data << Title;
    data << uint32(eEmote._Delay );                         //zyg: player emote
    data << uint32(eEmote._Emote );                         //zyg: NPC emote
    data << uint8 ( pQuestMenu->MenuItemCount() );

    for ( uint16 iI = 0; iI < pQuestMenu->MenuItemCount(); iI++ )
    {
        QuestMenuItem qmi=pQuestMenu->GetItem(iI);
        uint32 questID = qmi.m_qId;
        data << questID;
        data << uint32( qmi.m_qIcon );
        data << uint32( objmgr.QuestTemplates[questID]->GetQuestLevel() );
        data << objmgr.QuestTemplates[questID]->GetTitle();
    }
    pSession->SendPacket( &data );
    uint32 fqid=pQuestMenu->GetItem(0).m_qId;
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_QUEST_LIST NPC Guid=%u, questid-0=%u",npcGUID,fqid);
}

void PlayerMenu::SendQuestGiverStatus( uint32 questStatus, uint64 npcGUID )
{
    WorldPacket data( SMSG_QUESTGIVER_STATUS, 12 );
    data << npcGUID << questStatus;

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_STATUS NPC Guid=%u, status=%u",GUID_LOPART(npcGUID),questStatus);
}

void PlayerMenu::SendQuestGiverQuestDetails( Quest *pQuest, uint64 npcGUID, bool ActivateAccept)
{
    WorldPacket data(SMSG_QUESTGIVER_QUEST_DETAILS, 100);   // guess size

    data << npcGUID;
    data << pQuest->GetQuestId() << pQuest->GetTitle() << pQuest->GetDetails();
    data << pQuest->GetObjectives() << uint32( ActivateAccept );
    ItemPrototype const* IProto;

    data << pQuest->GetRewChoiceItemsCount();
    for (uint32 i=0; i < QUEST_REWARD_CHOICES_COUNT; i++)
    {
        data << uint32(pQuest->RewChoiceItemId[i]);
        data << uint32(pQuest->RewChoiceItemCount[i]);
        IProto = objmgr.GetItemPrototype(pQuest->RewChoiceItemId[i]);
        if ( IProto )
            data << uint32(IProto->DisplayInfoID);
        else
            data << uint32( 0x00 );
    }

    data << pQuest->GetRewItemsCount();
    for (uint32 i=0; i < QUEST_REWARDS_COUNT; i++)
    {
        data << pQuest->RewItemId[i];
        data << pQuest->RewItemCount[i];
        IProto = objmgr.GetItemPrototype(pQuest->RewItemId[i]);
        if ( IProto )
            data << IProto->DisplayInfoID;
        else
            data << uint32( 0x00 );
    }

    data << uint32(pQuest->GetRewOrReqMoney());

    data << pQuest->GetReqItemsCount();
    for (uint32 i=0; i <  QUEST_OBJECTIVES_COUNT; i++)
    {
        data << pQuest->ReqItemId[i];
        data << pQuest->ReqItemCount[i];
    }

    data << pQuest->GetReqCreatureOrGOcount();
    for (uint32 i=0; i <QUEST_OBJECTIVES_COUNT; i++)
    {
        data << uint32(pQuest->ReqCreatureOrGOId[i]);
        data << pQuest->ReqCreatureOrGOCount[i];
    }

    pSession->SendPacket( &data );

    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS NPCGuid=%u, questid=%u",GUID_LOPART(npcGUID),pQuest->GetQuestId());
}

void PlayerMenu::SendQuestQueryResponse( Quest *pQuest )
{
    WorldPacket data( SMSG_QUEST_QUERY_RESPONSE, 100 );     // guess size

    data << uint32(pQuest->GetQuestId());
    data << uint32(pQuest->GetMinLevel());
    data << uint32(pQuest->GetQuestLevel());

    if(pQuest->GetQuestSort() > 0)
        data << uint32(pQuest->GetQuestSort() * -1);
    else
        data << uint32(pQuest->GetZoneId());

    data << uint32(pQuest->GetType());
    data << uint32(pQuest->GetRequiredRepFaction());
    data << uint32(pQuest->GetRequiredRepValue());
    data << uint32(0);
    data << uint32(0);

    data << uint32(pQuest->GetNextQuestId());
    data << uint32(pQuest->GetRewOrReqMoney());
    data << uint32(pQuest->GetRewXP());

    // check if RewSpell is teaching another spell
    if(pQuest->GetRewSpell())
    {
        SpellEntry const *rewspell = sSpellStore.LookupEntry(pQuest->GetRewSpell());
        if(rewspell)
        {
            if(rewspell->Effect[0] == SPELL_EFFECT_LEARN_SPELL)
                data << uint32(rewspell->EffectTriggerSpell[0]);
            else
                data << uint32(pQuest->GetRewSpell());
        }
        else
        {
            sLog.outError("Quest %u have non-existed RewSpell %u, ignored.",pQuest->GetQuestId(),pQuest->GetRewSpell());
            data << uint32(0);
        }
    }
    else
        data << uint32(0);

    data << uint32(pQuest->GetSrcItemId());
    data << uint32(pQuest->GetSpecialFlags());

    int iI;

    for (iI = 0; iI < QUEST_REWARDS_COUNT; iI++)
    {
        data << uint32(pQuest->RewItemId[iI]);
        data << uint32(pQuest->RewItemCount[iI]);
    }
    for (iI = 0; iI < QUEST_REWARD_CHOICES_COUNT; iI++)
    {
        data << uint32(pQuest->RewChoiceItemId[iI]);
        data << uint32(pQuest->RewChoiceItemCount[iI]);
    }

    data << pQuest->GetPointMapId();
    data << pQuest->GetPointX();
    data << pQuest->GetPointY();
    data << pQuest->GetPointOpt();

    data << pQuest->GetTitle();
    data << pQuest->GetObjectives();
    data << pQuest->GetDetails();
    data << pQuest->GetEndText();

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; iI++)
    {
        data << uint32(pQuest->ReqCreatureOrGOId[iI])  << uint32(pQuest->ReqCreatureOrGOCount[iI]);
        data << uint32(pQuest->ReqItemId[iI]) << uint32(pQuest->ReqItemCount[iI]);
    }

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; iI++)
        data << pQuest->ObjectiveText[iI];

    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUEST_QUERY_RESPONSE questid=%u",pQuest->GetQuestId() );
}

void PlayerMenu::SendQuestGiverOfferReward( uint32 quest_id, uint64 npcGUID, bool EnbleNext, QEmote Emotes[], unsigned int EmoteCnt )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if(!qInfo)
        return;

    WorldPacket data( SMSG_QUESTGIVER_OFFER_REWARD, 50 );   // guess size

    data << npcGUID;
    data << quest_id;
    data << qInfo->GetTitle();
    data << qInfo->GetOfferRewardText();

    data << uint32( EnbleNext );

    /*if ( EmoteCnt > 0 )
    {
        data << uint32( EmoteCnt );

        for (unsigned int iI = 0; iI < EmoteCnt; iI++ )
        {
            data << Emotes[iI]._Delay;
            data << Emotes[iI]._Emote;
        }
    } else data << uint32( 0x00 ); */
    // Must be extended later, but for now:
    if (qInfo->GetOfferRewardEmote() == 0)
    {
        data << uint32(0x00);
    }
    else
    {
        data << uint32(0x1);
        data << uint32(0x0);
        data << qInfo->GetOfferRewardEmote();
    }

    ItemPrototype const *pItem;

    data << uint32(qInfo->GetRewChoiceItemsCount());
    for (uint32 i=0; i < qInfo->GetRewChoiceItemsCount(); i++)
    {
        pItem = objmgr.GetItemPrototype( qInfo->RewChoiceItemId[i] );

        data << uint32(qInfo->RewChoiceItemId[i]);
        data << uint32(qInfo->RewChoiceItemCount[i]);

        if ( pItem )
            data << uint32(pItem->DisplayInfoID);
        else
            data << uint32(0);
    }

    data << uint32(qInfo->GetRewItemsCount());
    for (uint16 i=0; i < qInfo->GetRewItemsCount(); i++)
    {
        pItem = objmgr.GetItemPrototype(qInfo->RewItemId[i]);
        data << uint32(qInfo->RewItemId[i]) << uint32(qInfo->RewItemCount[i]);

        if ( pItem )
            data << uint32(pItem->DisplayInfoID);
        else
            data << uint32(0);
    }

    data << uint32(qInfo->GetRewOrReqMoney());
    data << uint32(0x00);

    // check if RewSpell is teaching another spell
    if(qInfo->GetRewSpell())
    {
        SpellEntry const *rewspell = sSpellStore.LookupEntry(qInfo->GetRewSpell());
        if(rewspell)
        {
            if(rewspell->Effect[0] == SPELL_EFFECT_LEARN_SPELL)
                data << uint32(rewspell->EffectTriggerSpell[0]);
            else
                data << uint32(qInfo->GetRewSpell());
        }
        else
        {
            sLog.outError("Quest %u have non-existed RewSpell %u, ignored.",qInfo->GetQuestId(),qInfo->GetRewSpell());
            data << uint32(0);
        }
    }
    else
        data << uint32(0);

    // more data here--zoneid uint32 + 0x00000000?
    pSession->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_OFFER_REWARD NPCGuid=%u, questid=%u",GUID_LOPART(npcGUID),quest_id );
}

void PlayerMenu::SendQuestGiverRequestItems( Quest *pQuest, uint64 npcGUID, bool Completable, bool CloseOnCancel )
{
    // We can always call to RequestItems, but this packet only goes out if there are actually
    // items.  Otherwise, we'll skip straight to the OfferReward

    // We may wish a better check, perhaps checking the real quest requirements
    if (strlen(pQuest->GetRequestItemsText()) == 0)
    {
        SendQuestGiverOfferReward(pQuest->GetQuestId(), npcGUID, true, NULL, 0);
        return;
    }

    WorldPacket data( SMSG_QUESTGIVER_REQUEST_ITEMS, 50 );  // guess size
    data << npcGUID;
    data << pQuest->GetQuestId();
    data << pQuest->GetTitle();

    data << pQuest->GetRequestItemsText();

    data << uint32(0x00);
    data << pQuest->GetRequestItemsEmote();

    // Close Window after cancel
    if (CloseOnCancel)
        data << uint32(0x01);
    else
        data << uint32(0x00);

    // Req Money
    data << uint32(pQuest->GetRewOrReqMoney() < 0 ? -pQuest->GetRewOrReqMoney() : 0);

    data << uint32( pQuest->GetReqItemsCount() );

    ItemPrototype const *pItem;
    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if ( !pQuest->ReqItemId[i] ) continue;

        pItem = objmgr.GetItemPrototype(pQuest->ReqItemId[i]);
        data << uint32(pQuest->ReqItemId[i]);
        data << uint32(pQuest->ReqItemCount[i]);

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
    sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS NPCGuid=%u, questid=%u",GUID_LOPART(npcGUID),pQuest->GetQuestId() );
}
