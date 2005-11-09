/* GossipDef.h
 *
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

#ifndef MANGOSSERVER_GOSSIP_H
#define MANGOSSERVER_GOSSIP_H

#define GOSSIP_MAX_MENU_ITEMS 15

/*
 * GossipMenuItem Structure defines an Item
 * that will be sent to the player by
 * sending the menu.
 */
struct GossipMenuItem {
	uint8		m_gIcon;
	bool		m_gCoded;
	char*		m_gMessage;
};

/*
 * QuestMenuItem Structure defines an Item
 * that will be sent to the player by
 * sending the quest menu.
 */
struct QuestMenuItem {
	uint32		m_qId;
	uint8		m_qIcon;
	bool		m_qAvailable;
	char*		m_qTitle;
};

/*
 * Basic class for the Gossip Menus
 */
class GossipMenu
{
public:
	GossipMenu();
	~GossipMenu();

	void MenuItem(uint8 Icon, std::string Message, bool Coded = false);
	void ClearMenu();

protected:
	int m_gItemsCount;
	GossipMenuItem m_gItems[GOSSIP_MAX_MENU_ITEMS];
};

/*
 * Basic class for the Gossip Menus
 */
class QuestMenu
{
public:
	QuestMenu();
	~QuestMenu();

	void QuestItem( uint32 QuestId, uint8 Icon , bool Available);
	void ClearMenu();

protected:
	int m_qItemsCount;
	QuestMenuItem m_qItems[GOSSIP_MAX_MENU_ITEMS];
};

/*
 * Basic Class for PlayerMenu
 *  -> Will contain all needed methods related to NPC to Player relations.
 */
class PlayerMenu
{
public:

	//
	// Basic methods, contructors and destructors
	//

	PlayerMenu()
	{
		pGossipMenu = new GossipMenu();
		pQuestMenu  = new QuestMenu();
	}

	~PlayerMenu()
	{
		delete pGossipMenu;
		delete pQuestMenu;
	}

	GossipMenu* GetGossipMenu() { return pGossipMenu; }
	QuestMenu* GetQuestMenu() { return pQuestMenu; }

	//
	// Player communication methods
	// TODO: Needs implementation

protected:
	GossipMenu* pGossipMenu;
	QuestMenu* pQuestMenu;
};

#endif
