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

/*
#include "NameTables.h"
extern NameTableEntry g_worldOpcodeNames[];
*/

void WorldSession::HandleMovementOpcodes( WorldPacket & recv_data )
{
    uint32 flags, time;
    float x, y, z, orientation;
//    uint32 nothing;
    // float unk1, unk2, unk3, unk4, unk5;

    if(GetPlayer()->GetDontMove())
        return;

#ifdef _VERSION_1_7_0_
    recv_data >> flags >> time;
    recv_data >> x >> y >> z >> orientation >> nothing;
#else //!_VERSION_1_7_0_
    recv_data >> flags >> time;
    recv_data >> x >> y >> z >> orientation;
#endif //_VERSION_1_7_0_

    if( GetPlayer() && !GetPlayer( )->SetPosition(x, y, z, orientation) )
    {
        WorldPacket movedata;
        GetPlayer( )->BuildTeleportAckMsg(&movedata, GetPlayer()->GetPositionX(),
            GetPlayer()->GetPositionY(), GetPlayer()->GetPositionZ(), GetPlayer()->GetOrientation() );

        SendPacket(&movedata);
    }

    WorldPacket data;
    data.Initialize( recv_data.GetOpcode() );

#ifdef _VERSION_1_7_0_
/*
#ifdef _DEBUG
    std::stringstream ss1;
    ss1.rdbuf()->str("");
    ss1 << x;
    float x2 = (float)atof(ss1.str().c_str());

    std::stringstream ss2;
    ss2.rdbuf()->str("");
    ss2 << y;
    float y2 = (float)atof(ss2.str().c_str());

    std::stringstream ss3;
    ss3.rdbuf()->str("");
    ss3 << z;
    float z2 = (float)atof(ss3.str().c_str());

    std::stringstream ss4;
    ss4.rdbuf()->str("");
    ss4 << orientation;
    float orientation2 = (float)atof(ss4.str().c_str());

    std::stringstream ss5;
    ss5.rdbuf()->str("");
    ss5 << nothing;
    float nothingb = (float)atof(ss5.str().c_str());
    uint32 nothingb2 = (uint32)atoi(ss5.str().c_str());
    uint16 nothingb3 = (uint16)atoi(ss5.str().c_str());
    uint8 nothingb4 = (uint8)atoi(ss5.str().c_str());

    Log::getSingleton().outDebug( "SESSION: recieved opcode %s (0x%.4X) - Origin: %f %f %f (%f).",
            LookupName(recv_data.GetOpcode(), g_worldOpcodeNames),
            recv_data.GetOpcode(), x2, y2, z2, orientation2);

//    Log::getSingleton().outDebug( "Nothings are [%f] [%u] [%u] [%u].",
//            nothingb, nothingb2, nothingb3, nothingb4);

    Log::getSingleton().outDebug( "Nothings: [%f] [%f].",
            nothingb, nothingb2);
#endif //_DEBUG*/
    data << GetPlayer()->GetGUID();
    data << flags << time;
    data << x << y << z << orientation;
    data  << nothing << uint32(0) << uint32(0) << uint32(0);
#else //!_VERSION_1_7_0_
    data << GetPlayer()->GetGUID();
    data << flags << time;
    data << x << y << z << orientation;
#endif //_VERSION_1_7_0_

    GetPlayer()->SendMessageToSet(&data, false);
}

void
WorldSession::HandleSetActiveMoverOpcode(WorldPacket &recv_data)
{
    uint32 guild, time;
    recv_data >> guild >> time;
}
