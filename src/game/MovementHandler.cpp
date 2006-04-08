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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "MapManager.h"

void WorldSession::HandleMoveWorldportAckOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: got MSG_MOVE_WORLDPORT_ACK." );
    _player->RemoveFromWorld();
    MapManager::Instance().GetMap(_player->GetMapId())->Add(_player);
    WorldPacket data;
    data.Initialize(SMSG_SET_REST_START);
    data << uint32(8129);
    SendPacket(&data);
    _player->SetDontMove(false);
}

void WorldSession::HandleFallOpcode( WorldPacket & recv_data )
{

    // TODO Add Watercheck when fall into water no damage

    uint32 flags, time;
    float x, y, z, orientation;

    uint32 FallTime;

    uint64 guid;
    //    uint8 type;
    uint32 damage;

    if(GetPlayer()->GetDontMove())
        return;

    recv_data >> flags >> time;
    recv_data >> x >> y >> z >> orientation;
    recv_data >> FallTime;
    if ( FallTime > 1100 && !GetPlayer()->isDead())
    {
        uint32 MapID = GetPlayer()->GetMapId();
        Map* Map = MapManager::Instance().GetMap(MapID);
        float posz = Map->GetWaterLevel(x,y);
        if (z < (posz - (float) 1))
        {
            guid = GetPlayer()->GetGUID();
            damage = (uint32)((FallTime - 1100)/100)+1;
            GetPlayer()->EnvironmentalDamage(guid,DAMAGE_FALL, damage);
        }

    }
}

void WorldSession::HandleMovementOpcodes( WorldPacket & recv_data )
{
    uint32 flags, time;
    float x, y, z, orientation;

    if(GetPlayer()->GetDontMove())
        return;

    recv_data >> flags >> time;
    recv_data >> x >> y >> z >> orientation;

    bool isSet = GetPlayer( )->SetPosition(x, y, z, orientation);
    if(recv_data.GetOpcode() != MSG_MOVE_JUMP)
    {
        WorldPacket data;
        data.Initialize( recv_data.GetOpcode() );
        data << uint8(0xFF) << GetPlayer()->GetGUID();
        data << flags << time;
        data << x << y << z << orientation;
        GetPlayer()->SendMessageToSet(&data, false);
    }

    uint32 MapID = GetPlayer()->GetMapId();
    Map* Map = MapManager::Instance().GetMap(MapID);
    float posz = Map->GetWaterLevel(x,y);
    uint8 flag1 = Map->GetTerrainType(x,y);

    //!Underwater check
    if ((z < (posz - (float)2)) && (flag1 & 0x01))
        GetPlayer()->m_isunderwater|= 0x01;
    else if (z > (posz - (float)2))
        GetPlayer()->m_isunderwater&= 0x7A;
    //!in lava check
    if ((z < (posz - (float)2)) && (flag1 & 0x02))
        GetPlayer()->m_isunderwater|= 0x80;
}

void WorldSession::HandleSetActiveMoverOpcode(WorldPacket &recv_data)
{
    //CMSG_SET_ACTIVE_MOVER

    uint32 guild, time;
    recv_data >> guild >> time;
}
