/* 
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "ObjectDefines.h"

void WorldSession::HandleAttackSwingOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;

    DEBUG_LOG( "WORLD: Recvd CMSG_ATTACKSWING Message guidlow:%u guidhigh:%u", GUID_LOPART(guid), GUID_HIPART(guid) );

    Unit *pEnemy = ObjectAccessor::Instance().GetUnit(*_player, guid);

    if(!pEnemy)
    {
        if(GUID_HIPART(guid)!=HIGHGUID_PLAYER && GUID_HIPART(guid)!=HIGHGUID_UNIT)
            sLog.outError("WORLD: Object %u (TypeID: %u) isn't player or creature",GUID_LOPART(guid),GuidHigh2TypeId(GUID_HIPART(guid)));
        else
            sLog.outError( "WORLD: Enemy %s %u not found",(GUID_HIPART(guid)==HIGHGUID_PLAYER ? "player" : "creature"),GUID_LOPART(guid));

        // stop attack state at client
        WorldPacket data( SMSG_ATTACKSTOP, (4+16) );        // we guess size
        data.append(GetPlayer()->GetPackGUID());
        data.append(guid);
        SendPacket(&data);
        return;
    }

    if(_player->IsFriendlyTo(pEnemy))
    {
        sLog.outError( "WORLD: Enemy %s %u is friendly",(GUID_HIPART(guid)==HIGHGUID_PLAYER ? "player" : "creature"),GUID_LOPART(guid));

        // stop attack state at client
        WorldPacket data( SMSG_ATTACKSTOP, (4+16) );        // we guess size
        data.append(GetPlayer()->GetPackGUID());
        data.append(guid);
        SendPacket(&data);
        return;
    }

    _player->Attack(pEnemy,true);
}

void WorldSession::HandleAttackStopOpcode( WorldPacket & recv_data )
{
    GetPlayer()->AttackStop();
}

void WorldSession::HandleSetSheathedOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4);

    uint32 sheathed;
    recv_data >> sheathed;

    //sLog.outDebug( "WORLD: Recvd CMSG_SETSHEATHED Message guidlow:%u value1:%u", GetPlayer()->GetGUIDLow(), sheathed );

    GetPlayer()->SetSheath(sheathed);
}
