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
#include "Transports.h"

void WorldSession::HandleMoveWorldportAckOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: got MSG_MOVE_WORLDPORT_ACK." );

    GetPlayer()->RemoveFromWorld();
    MapManager::Instance().GetMap(GetPlayer()->GetMapId())->Add(GetPlayer());
    WorldPacket data(SMSG_SET_REST_START, 4);
    data << uint32(8129);
    SendPacket(&data);
    GetPlayer()->SetDontMove(false);
}

void WorldSession::HandleMovementOpcodes( WorldPacket & recv_data )
{
    if(GetPlayer()->GetDontMove())
        return;

    /* extract packet */
    uint32 flags, time, fallTime;
    float x, y, z, orientation;

    uint32 t_GUIDl, t_GUIDh;
    float  t_x, t_y, t_z, t_o;
    float  s_angle;
    float  j_unk1, j_sinAngle, j_cosAngle, j_xyspeed;

    recv_data >> flags >> time;
    recv_data >> x >> y >> z >> orientation;
    if (flags & MOVEMENTFLAG_ONTRANSPORT)
    {
        recv_data >> t_GUIDl >> t_GUIDh;
        recv_data >> t_x >> t_y >> t_z >> t_o;
    }
    if (flags & MOVEMENTFLAG_SWIMMING)
    {
        recv_data >> s_angle;                               // kind of angle, -1.55=looking down, 0=looking straight forward, +1.55=looking up
    }
    recv_data >> fallTime;                                  // duration of last jump (when in jump duration from jump begin to now)

    if (flags & MOVEMENTFLAG_JUMPING)
    {
        recv_data >> j_unk1;                                // ?constant, but different when jumping in water and on land?
        recv_data >> j_sinAngle >> j_cosAngle;              // sin + cos of angle between orientation0 and players orientation
        recv_data >> j_xyspeed;                             // speed of xy movement
    }
    /*----------------*/

    /* handle special cases */
    if (flags & MOVEMENTFLAG_ONTRANSPORT)
    {
        // if we boarded a transport, add us to it
        if (!GetPlayer()->m_transport)
        {
            for (vector<Transport*>::iterator iter = MapManager::Instance().m_Transports.begin(); iter != MapManager::Instance().m_Transports.end(); iter++)
            {
                if ((*iter)->GetGUIDLow() == t_GUIDl)
                {
                    GetPlayer()->m_transport = (*iter);
                    (*iter)->AddPassenger(GetPlayer());
                    continue;
                }
            }
        }
        GetPlayer()->m_transX = t_x;
        GetPlayer()->m_transY = t_y;
        GetPlayer()->m_transZ = t_z;
        GetPlayer()->m_transO = t_o;
    }
    else if (GetPlayer()->m_transport)                      // if we were on a transport, leave
    {
        GetPlayer()->m_transport->RemovePassenger(GetPlayer());
        GetPlayer()->m_transport = NULL;
        GetPlayer()->m_transX = 0.0f;
        GetPlayer()->m_transY = 0.0f;
        GetPlayer()->m_transZ = 0.0f;
        GetPlayer()->m_transO = 0.0f;
    }

    if (GetPlayer()->HasMovementFlags(MOVEMENTFLAG_FALLING) && !(flags&MOVEMENTFLAG_FALLING))
    {
        Player *target = GetPlayer();
        if (fallTime > 1100 && !target->isDead() && !target->isGameMaster() &&
            !target->HasAuraType(SPELL_AURA_HOVER) && !target->HasAuraType(SPELL_AURA_FEATHER_FALL))
        {
            Map *map = MapManager::Instance().GetMap(target->GetMapId());
            float posz = map->GetWaterLevel(x,y);
            float fallperc = float(fallTime)*10/11000;
            uint32 damage = (uint32)((fallperc*fallperc -1) / 9 * target->GetMaxHealth());

            if (damage > 0 && damage < 2* target->GetMaxHealth())
                target->EnvironmentalDamage(target->GetGUID(),DAMAGE_FALL, damage);
            DEBUG_LOG("!! z=%f, pz=%f FallTime=%d posz=%f damage=%d" , z, target->GetPositionZ(),fallTime, posz, damage);
        }

        //handle fall and logout at the sametime
        if (target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE))
        {
            target->SetFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT);
            // Can't move
            WorldPacket data( SMSG_FORCE_MOVE_ROOT, 12 );
            data.append(target->GetPackGUID());
            data << (uint32)2;
            SendPacket( &data );
        }
    }
    /*----------------------*/

    /* process position-change */
    WorldPacket data(recv_data.GetOpcode(), (8+recv_data.size()));
    data.append(GetPlayer()->GetPackGUID());
    data.append(recv_data.contents(), recv_data.size());
    GetPlayer()->SendMessageToSet(&data, false);

    GetPlayer()->SetPosition(x, y, z, orientation);
    GetPlayer()->SetMovementFlags(flags);
}

void WorldSession::HandleForceSpeedChangeAck(WorldPacket &recv_data)
{
    /* extract packet */
    uint64 guid;
    uint32 unk1, flags, time, fallTime;
    float x, y, z, orientation;

    uint32 t_GUIDl, t_GUIDh;
    float  t_x, t_y, t_z, t_o;
    float  s_angle;
    float  j_unk1, j_sinAngle, j_cosAngle, j_xyspeed;
    float newspeed;

    recv_data >> guid;

    // now can skip not our packet
    if(_player->GetGUID() != guid)
        return;

    // continue parse packet

    recv_data >> unk1;
    recv_data >> flags >> time;
    recv_data >> x >> y >> z >> orientation;
    if (flags & MOVEMENTFLAG_ONTRANSPORT)
    {
        recv_data >> t_GUIDl >> t_GUIDh;
        recv_data >> t_x >> t_y >> t_z >> t_o;
    }
    if (flags & MOVEMENTFLAG_SWIMMING)
    {
        recv_data >> s_angle;                               // kind of angle, -1.55=looking down, 0=looking straight forward, +1.55=looking up
    }
    recv_data >> fallTime;                                  // duration of last jump (when in jump duration from jump begin to now)

    if (flags & MOVEMENTFLAG_JUMPING)
    {
        recv_data >> j_unk1;                                // ?constant, but different when jumping in water and on land?
        recv_data >> j_sinAngle >> j_cosAngle;              // sin + cos of angle between orientation0 and players orientation
        recv_data >> j_xyspeed;                             // speed of xy movement
    }

    recv_data >> newspeed;
    /*----------------*/

    UnitMoveType move_type;

    static char const* move_type_name[MAX_MOVE_TYPE] = {  "Walk", "Run", "Walkback", "Swim", "Swimback", "Turn" };

    uint16 opcode = recv_data.GetOpcode();
    switch(opcode)
    {
        case CMSG_FORCE_WALK_SPEED_CHANGE_ACK:      move_type = MOVE_WALK;     break;
        case CMSG_FORCE_RUN_SPEED_CHANGE_ACK:       move_type = MOVE_RUN;      break;
        case CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK:  move_type = MOVE_WALKBACK; break;
        case CMSG_FORCE_SWIM_SPEED_CHANGE_ACK:      move_type = MOVE_SWIM;     break;
        case CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK: move_type = MOVE_SWIMBACK; break;
        case CMSG_FORCE_TURN_RATE_CHANGE_ACK:       move_type = MOVE_TURN;     break;
        default:
            sLog.outError("WorldSession::HandleForceSpeedChangeAck: Unknown move type opcode: %u",opcode);
            return;
    }

    if (fabs(_player->GetSpeed(move_type) - newspeed) > 0.01f)
    {
        sLog.outError("%sSpeedChange player %s is NOT correct (must be %f instead %f), force set to correct value",
            move_type_name[move_type], _player->GetName(), _player->GetSpeed(move_type), newspeed);
        _player->SetSpeed(move_type,_player->GetSpeedRate(move_type),true);
    }
}

void WorldSession::HandleSetActiveMoverOpcode(WorldPacket &recv_data)
{
    //CMSG_SET_ACTIVE_MOVER
    sLog.outDebug("WORLD: Recvd CMSG_SET_ACTIVE_MOVER");

    CHECK_PACKET_SIZE(recv_data,8);

    uint32 guild, time;
    recv_data >> guild >> time;
}

void WorldSession::HandleMountSpecialAnimOpcode(WorldPacket &recvdata)
{
    //sLog.outDebug("WORLD: Recvd CMSG_MOUNTSPECIAL_ANIM");

    WorldPacket data(SMSG_MOUNTSPECIAL_ANIM, 8);
    data << uint64(GetPlayer()->GetGUID());

    GetPlayer()->SendMessageToSet(&data, false);
}

void WorldSession::HandleMoveKnockBackAck( WorldPacket & recv_data )
{
    // Currently not used but maybe use later for recheck final player position
    // (must be at call same as into "recv_data >> x >> y >> z >> orientation;"

    /* 
    uint32 flags, time;
    float x, y, z, orientation;
    uint64 guid;
    uint32 sequence;
    uint32 ukn1;
    float xdirection,ydirection,hspeed,vspeed;

    recv_data >> guid;
    recv_data >> sequence;
    recv_data >> flags >> time;
    recv_data >> x >> y >> z >> orientation;
    recv_data >> ukn1; //unknown
    recv_data >> vspeed >> xdirection >> ydirection >> hspeed;

    // skip not personal message;
    if(GetPlayer()->GetGUID()!=guid)
        return;

    // check code
    */
}

void WorldSession::HandleMoveHoverAck( WorldPacket & recv_data )
{
}

void WorldSession::HandleMoveWaterWalkAck(WorldPacket& recv_data)
{
    // TODO
    // we receive guid,x,y,z
}
