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
#define DEFAULT_GOSSIP_MESSAGE              0xffffff

//POI defines
enum Poi_Icon
{
    ICON_POI_0                  =   0,
    ICON_POI_1                  =   1,
    ICON_POI_2                  =   2,
    ICON_POI_3                  =   3,
    ICON_POI_4                  =   4,                      //House
    ICON_POI_5                  =   5,
    ICON_POI_6                  =   6,                      //
    ICON_POI_7                  =   7,                      //dead
    ICON_POI_8                  =   8,
    ICON_POI_9                  =   9,
    ICON_POI_10                 =   10,
    ICON_POI_11                 =   11,
    ICON_POI_12                 =   12,
    ICON_POI_13                 =   13,
    ICON_POI_14                 =   14,
    ICON_POI_15                 =   15,
    ICON_POI_16                 =   16,
    ICON_POI_17                 =   17,
    ICON_POI_18                 =   18,
    ICON_POI_19                 =   19,
    ICON_POI_20                 =   20,
    ICON_POI_21                 =   21,
    ICON_POI_22                 =   22,
    ICON_POI_23                 =   23,
    ICON_POI_24                 =   24,
    ICON_POI_25                 =   25,
    ICON_POI_26                 =   26,
    ICON_POI_27                 =   27,
    ICON_POI_28                 =   28,
    ICON_POI_29                 =   29,
    ICON_POI_30                 =   30,
    ICON_POI_31                 =   31,
    ICON_POI_32                 =   32,
    ICON_POI_33                 =   33,
    ICON_POI_34                 =   34,
    ICON_POI_35                 =   35,
    ICON_POI_36                 =   36,
    ICON_POI_37                 =   37,
    ICON_POI_38                 =   38,
    ICON_POI_39                 =   39,
    ICON_POI_40                 =   40                      //horse
};

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

        void AddMenuItem(uint8 Icon, char const * Message, bool Coded = false);
        void AddMenuItem(uint8 Icon, char const * Message, uint32 dtSender, uint32 dtAction, bool Coded = false);

        uint8 MenuItemCount()
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

        void AddMenuItem( uint32 QuestId, uint8 Icon , bool Available);
        void ClearMenu();

        uint8 MenuItemCount()
        {
            return m_qItemsCount;
        }
        bool HasItem( uint32 questid );

        QuestMenuItem GetItem( uint16 Id )
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
        void SendPointOfInterest( float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, const char * locName );
        void SendTalking( uint32 textID );
        void SendTalking( char const * title, char const * text );
};
#endif
