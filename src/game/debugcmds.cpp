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
#include "GossipDef.h"
#include "Language.h"

bool ChatHandler::HandleDebugInArcCommand(const char* args)
{
    Object *obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if(!(obj = (Object*)ObjectAccessor::Instance().GetPlayer(*m_session->GetPlayer(), guid)) && !(obj = (Object*)ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),guid)))
        {
            SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            return true;
        }
    }
    else
        obj = m_session->GetPlayer();

    SendSysMessage(LANG_NOT_IMPLEMENTED);

    return true;
}

bool ChatHandler::HandleDebugSpellFailCommand(const char* args)
{
    WorldPacket data;

    if(!args || args ==" ")
        return false;
    char* px = strtok((char*)args, " ");

    uint8 failnum = (uint8)atoi(px);

    data.Initialize(SMSG_CAST_RESULT);
    data << (uint32)133;
    data << uint8(2);
    data << failnum;

    m_session->SendPacket(&data);
    //char buf[256];
    //FillSystemMessageData(&data, m_session, buf);

    return true;
}

bool ChatHandler::HandleSetPoiCommand(const char* args)
{
    Player *  pPlayer = m_session->GetPlayer();
    Unit* target = ObjectAccessor::Instance().GetCreature(*pPlayer, pPlayer->GetSelection());
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }
    uint32 icon = atol((char*)args);
    if ( icon < 0 )
        icon = 0;
    sLog.outDetail("Command : POI, NPC = %u, icon = %u", target->GetGUID(), icon);
    pPlayer->PlayerTalkClass->SendPointOfInterest(target->GetPositionX(), target->GetPositionY(), icon, 6, 30, "Test POI");
    return true;
}

bool ChatHandler::HandleSendItemErrorMsg(const char* args)
{
    uint8 error_msg = atol((char*)args);
    if ( error_msg > 0 )
    {
        WorldPacket data;
        data.Initialize(SMSG_INVENTORY_CHANGE_FAILURE);
        data << error_msg;
        data << uint64(0);
        data << uint64(0);
        data << uint8(0);
        m_session->SendPacket( &data );
        return true;
    }
    return false;
}

bool ChatHandler::HandleSendQuestPartyMsgCommand(const char* args)
{
    uint32 msg = atol((char*)args);
    if ( msg >= 0 )
    {
        WorldPacket data;
        data.Initialize( MSG_QUEST_PUSH_RESULT );
        data << m_session->GetPlayer()->GetGUID();
        data << uint32(msg);
        data << uint8(0);
        m_session->SendPacket(&data);
    }
    return true;
}

bool ChatHandler::HandleSendQuestInvalidMsgCommand(const char* args)
{
    Player *  pPlayer = m_session->GetPlayer();
    uint32 msg = atol((char*)args);
    if ( msg >= 0 )
        pPlayer->SendCanTakeQuestResponse( msg );
    return true;
}
