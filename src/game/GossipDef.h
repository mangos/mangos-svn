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

#ifndef MANGOSSERVER_GOSSIP_H
#define MANGOSSERVER_GOSSIP_H

#include "Common.h"
#include "QuestDef.h"
#include "NPCHandler.h"

class WorldSession;

#define GOSSIP_MAX_MENU_ITEMS 15

struct GossipMenuItem
{
    uint8       m_gIcon;
    bool        m_gCoded;
    char*       m_gMessage;
    uint32  m_gSender;
    uint32  m_gAction;
};

struct QuestMenuItem
{
    uint32      m_qId;
    uint8       m_qIcon;
    bool        m_qAvailable;
    char*       m_qTitle;
};

class MANGOS_DLL_SPEC GossipMenu
{
    public:
        GossipMenu();
        ~GossipMenu();

        void MenuItem(uint8 Icon, std::string Message, bool Coded = false);
        void MenuItem(uint8 Icon, std::string Message, uint32 dtSender, uint32 dtAction, bool Coded = false);

        uint8 ItemsInMenu()
        {
            return m_gItemsCount;
        }

        GossipMenuItem GetItem( unsigned int Id )
        {
            return m_gItems[ Id ];
        }

        uint32 MenuItemSender( unsigned int ItemId );
        uint32 MenuItemAction( unsigned int ItemId );

        void ClearMenu();

    protected:
        int m_gItemsCount;
        GossipMenuItem m_gItems[GOSSIP_MAX_MENU_ITEMS];
};

class QuestMenu
{
    public:
        QuestMenu();
        ~QuestMenu();

        void QuestItem( uint32 QuestId, uint8 Icon , bool Available);
        void ClearMenu();

        uint8 QuestsInMenu()
        {
            return m_qItemsCount;
        }

        QuestMenuItem GetItem( unsigned int Id )
        {
            return m_qItems[ Id ];
        }

    protected:
        int m_qItemsCount;
        QuestMenuItem m_qItems[GOSSIP_MAX_MENU_ITEMS];
};

class MANGOS_DLL_SPEC PlayerMenu
{
    private:
        GossipMenu* pGossipMenu;
        QuestMenu* pQuestMenu;
        WorldSession* pSession;

    public:

        PlayerMenu( WorldSession *Session );
        ~PlayerMenu();

        GossipMenu* GetGossipMenu() { return pGossipMenu; }
        QuestMenu* GetQuestMenu() { return pQuestMenu; }

        void ClearMenus();
        uint32 GossipOptionSender( unsigned int Selection );
        uint32 GossipOptionAction( unsigned int Selection );

        void SendGossipMenu( uint32 TitleTextId, uint64 npcGUID );
        void SendQuestMenu ( QEmote eEmote, std::string Title, uint64 npcGUID );
        void SendQuestStatus( uint32 questStatus, uint64 npcGUID );
        void SendQuestReward( Quest *pQuest, uint64 npcGUID, bool EnbleNext, QEmote Emotes[], unsigned int EmoteCnt );
        void SendQuestDetails( Quest *pQuest, uint64 npcGUID, bool ActivateAccept);
        void SendUpdateQuestDetails ( Quest *pQuest );
        void SendRequestedItems( Quest *pQuest, uint64 npcGUID, bool Completable );
        void SendQuestComplete( Quest *pQuest );
        void SendQuestUpdateComplete( Quest *pQuest );
        void CloseGossip();
        void SendQuestUpdateAddItem( Quest *pQuest, uint32 iLogItem, uint32 iLogNr);
        void SendQuestLogFull();
        void SendQuestIncompleteToLog( Quest *pQuest );
        void SendQuestCompleteToLog( Quest *pQuest );
        void SendQuestUpdateAddKill( Quest *pQuest, uint64 mobGUID, uint32 iNrMob, uint32 iLogMob );
        void SendQuestUpdateFailedTimer( Quest *pQuest );
        void SendQuestUpdateFailed( Quest *pQuest );
        void SendQuestUpdateSetTimer( Quest *pQuest, uint32 TimerValue);
        void SendQuestFailed( uint32 iReason );
        void SendQuestInvalid( uint32 iReason );
        void SendPointOfInterest( float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, const std::string locName );
};
#endif
