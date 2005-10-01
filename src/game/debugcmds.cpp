/* debugcmds.cpp
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

/////////////////////////////////////////////////
//  Debug Chat Commands
//

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

#ifdef ENABLE_GRID_SYSTEM
#include "ObjectAccessor.h"
#endif

bool ChatHandler::HandleDebugInArcCommand(const char* args)
{
    WorldPacket data;
    Object *obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
#ifndef ENABLE_GRID_SYSTEM
        if(!(obj = (Object*)objmgr.GetObject<Player>(guid)) && !(obj = (Object*)objmgr.GetObject<Creature>(guid)))
#else
        if(!(obj = (Object*)ObjectAccessor::Instance().GetPlayer(*m_session->GetPlayer(), guid)) && !(obj = (Object*)ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),guid)))
#endif
        {
            FillSystemMessageData(&data, m_session, "You should select a character or a creature.");
            m_session->SendPacket( &data );
            return true;
        }
    }
    else
        obj = (Object*)m_session->GetPlayer();

    char buf[256];
    // sprintf((char*)buf, "%d", m_session->GetPlayer()->inarc( 3.000f,obj->GetPositionX() ,obj->GetPositionY(), 90, m_session->GetPlayer()->GetOrientation(), m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY()));

    FillSystemMessageData(&data, m_session, buf);
    // m_session->SendPacket( &data );

    return true;
}
