/* Quest.h
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

#include "QuestDef.h"
#include "GossipDef.h"
#include <string.h>


GossipMenu::GossipMenu()
{
	m_gItemsCount = 0;
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
