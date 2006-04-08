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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "Log.h"
#include "Unit.h"
#include "ObjectAccessor.h"

bool ChatHandler::HandleDebugInArcCommand(const char* args)
{
    WorldPacket data;
    Object *obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if(!(obj = (Object*)ObjectAccessor::Instance().GetPlayer(*m_session->GetPlayer(), guid)) && !(obj = (Object*)ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),guid)))
        {
            FillSystemMessageData(&data, m_session, "You should select a character or a creature.");
            m_session->SendPacket( &data );
            return true;
        }
    }
    else
        obj = (Object*)m_session->GetPlayer();

    char buf[256];

    FillSystemMessageData(&data, m_session, buf);

    return true;
}
