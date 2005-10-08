/* MovementHandler.h
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

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"

#ifdef ENABLE_GRID_SYSTEM
#include "MapManager.h"
#endif

class MovementInfo
{
    uint32 flags, time;
    uint64 unk1;
    float unk2, unk3, unk4, unk5;
    float unk6;
    uint32 FallTime;
    float unk8, unk9, unk10, unk11, unk12;
    public:
        float x, y, z, orientation;

        MovementInfo(WorldPacket &data)
        {
            data >> flags >> time;
            data >> x >> y >> z >> orientation;

            if (flags & 0x2000000)                // Transport
            {
                data >> unk1 >> unk2 >> unk3 >> unk4 >> unk5;
            }
            if (flags & 0x200000)                 // Swimming
            {
                data >> unk6;
            }
            if (flags & 0x2000)                   // Falling
            {
                data >> FallTime >> unk8 >> unk9 >> unk10 >> unk11;
            }
            if (flags & 0x4000000)
            {
                data >> unk12;
            }
        }

        MovementInfo &operator >>(WorldPacket &data)
        {
            data << flags << time;
            data << x << y << z << orientation;

            if (flags & 0x2000000)                // Transport
            {
                data << unk1 << unk2 << unk3 << unk4 << unk5;
            }
            if (flags & 0x200000)                 // Swimming
            {
                data << unk6;
            }
            if (flags & 0x2000)                   // Falling
            {
                data << FallTime << unk8 << unk9 << unk10 << unk11;
            }
            if (flags & 0x4000000)
            {
                data << unk12;
            }
            return *this;
        }
};

void WorldSession::HandleMoveHeartbeatOpcode( WorldPacket & recv_data )
{
    uint32 flags, time;
    float x, y, z, orientation;

    if(GetPlayer()->GetDontMove())
        return;

    recv_data >> flags >> time;
    recv_data >> x >> y >> z >> orientation;

    if( GetPlayer() && !GetPlayer( )->SetPosition(x, y, z, orientation) )
    {
        WorldPacket movedata;

        GetPlayer( )->BuildTeleportAckMsg(&movedata, GetPlayer()->GetPositionX(),
            GetPlayer()->GetPositionY(), GetPlayer()->GetPositionZ(), GetPlayer()->GetOrientation() );

        SendPacket(&movedata);
    }

    WorldPacket data;
    data.Initialize( MSG_MOVE_HEARTBEAT );

    data << GetPlayer()->GetGUID();
    data << flags << time;
    data << x << y << z << orientation;

    GetPlayer( )->SendMessageToSet(&data, false);
}


void WorldSession::HandleMoveWorldportAckOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: got MSG_MOVE_WORLDPORT_ACK." );
#ifndef ENABLE_GRID_SYSTEM
    GetPlayer()->PlaceOnMap();
#else
    MapManager::Instance().GetMap(_player->GetMapId())->Add(_player);
#endif
}


void WorldSession::HandleFallOpcode( WorldPacket & recv_data )
{

    uint32 flags, time;
    float x, y, z, orientation;
    // float unk1, unk2, unk3, unk4;
    uint32 FallTime;

    uint64 guid;
    uint8 type;
    uint32 damage;

    if(GetPlayer()->GetDontMove())
        return;

    recv_data >> flags >> time;
    recv_data >> x >> y >> z >> orientation;
    recv_data >> FallTime;
    if ( FallTime > 1100 && !GetPlayer()->isDead() )
    {
        // need a better formula
        guid = GetPlayer()->GetGUID();
        type = DAMAGE_FALL;
        damage = (uint32)((FallTime - 1100)/100)+1;
        // Log::getSingleton().outError("Falling for %d time and %d damage taken.",FallTime,damage);
        WorldPacket data;
        data.Initialize(SMSG_ENVIRONMENTALDAMAGELOG);
        data << guid;
        data << type;
        data << damage;
        SendPacket(&data);
        GetPlayer()->DealDamage(GetPlayer(),damage,0);
    }
}

void WorldSession::HandleMovementOpcodes( WorldPacket & recv_data )
{
    uint32 flags, time;
    float x, y, z, orientation;
    // float unk1, unk2, unk3, unk4, unk5;

    if(GetPlayer()->GetDontMove())
        return;

    recv_data >> flags >> time;
    recv_data >> x >> y >> z >> orientation;

    if( GetPlayer() && !GetPlayer( )->SetPosition(x, y, z, orientation) )
    {
        WorldPacket movedata;
        GetPlayer( )->BuildTeleportAckMsg(&movedata, GetPlayer()->GetPositionX(),
            GetPlayer()->GetPositionY(), GetPlayer()->GetPositionZ(), GetPlayer()->GetOrientation() );

        SendPacket(&movedata);
    }

    WorldPacket data;
    data.Initialize( recv_data.GetOpcode() );

    data << GetPlayer()->GetGUID();
    data << flags << time;
    data << x << y << z << orientation;

    GetPlayer()->SendMessageToSet(&data, false);
}

void
WorldSession::HandleSetActiveMoverOpcode(WorldPacket &recv_data)
{
    uint32 guild, time;
    recv_data >> guild >> time;
}
