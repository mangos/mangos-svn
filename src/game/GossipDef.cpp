/* GossipDef.cpp
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

#include "QuestDef.h"
#include "GossipDef.h"
#include "ObjectMgr.h"
#include <string.h>


/*
 *  Gossip Menu Implementation
 */

GossipMenu::GossipMenu()
{
	m_gItemsCount = 0;
}

GossipMenu::~GossipMenu()
{
	ClearMenu();
}

void GossipMenu::MenuItem(uint8 Icon, std::string Message, bool Coded)
{
	char* Text = new char[strlen(Message.c_str()) + 1];
	strcpy( Text, Message.c_str() );

	m_gItemsCount++;
	ASSERT( m_gItemsCount < GOSSIP_MAX_MENU_ITEMS  );

	m_gItems[m_gItemsCount - 1].m_gIcon		 = Icon;
	m_gItems[m_gItemsCount - 1].m_gMessage   = Text;
	m_gItems[m_gItemsCount - 1].m_gCoded     = Coded;
}

void GossipMenu::ClearMenu()
{
	for (int i=0; i<m_gItemsCount; i++)
		delete m_gItems[i].m_gMessage;

	m_gItemsCount = 0;
}

/*
 *  Quest Menu Implementation
 */

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

	m_qItems[m_qItemsCount - 1].m_qIcon		 = Icon;
	m_qItems[m_qItemsCount - 1].m_qTitle	 = Text;
	m_qItems[m_qItemsCount - 1].m_qId		 = QuestId;
	m_qItems[m_qItemsCount - 1].m_qAvailable = Available;
}

void QuestMenu::ClearMenu()
{
	for (int i=0; i<m_qItemsCount; i++)
		delete m_qItems[i].m_qTitle;

	m_qItemsCount = 0;
}
