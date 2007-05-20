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
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "Auth/BigNumber.h"
#include "Auth/Sha1.h"
#include "UpdateData.h"
#include "LootMgr.h"
#include "Chat.h"
#include "ScriptCalls.h"
#include <zlib/zlib.h>
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Object.h"
#include "BattleGroundMgr.h"

void WorldSession::HandleBattleGroundHelloOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 8);

    uint64 guid;
    recv_data >> guid;
    sLog.outDebug( "WORLD: Recvd CMSG_BATTLEMASTER_HELLO Message from: " I64FMT, guid);

    uint32 bgid = 2; // WSG

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);
    switch(unit->GetCreatureInfo()->faction)
    {
        //AV Battlemaster
        case 1214:
        case 1216:
            bgid = 1;
            break;
        //WSG Battlemaster
        case 1641:
        case 1514:
            bgid = 2;
            break;
        //AB Battlemaster
        case 1577:
        case 412:
            bgid = 3;
            break;
        // todo: add more...
    }

    uint32 PlayerLevel = _player->getLevel();

    BattlemasterListEntry const* bl = sBattlemasterListStore.LookupEntry(bgid);

    if(!bl)
        return;

    if(PlayerLevel < bl->minlvl || PlayerLevel > bl->maxlvl)
    {
        SendNotification("You don't meet Battleground level requirements"); // temp, must be gossip message...
        return;
    }

    WorldPacket data;
    sBattleGroundMgr.BuildBattleGroundListPacket(&data, guid, _player, bgid);
    SendPacket( &data );
}

void WorldSession::HandleBattleGroundJoinOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 8+4+4+1);

    uint64 guid;
    uint32 bgid;
    uint32 instanceid;
    uint8 asgroup;

    recv_data >> guid;          // battlemaster guid
    recv_data >> bgid;          // battleground id (DBC id?)
    recv_data >> instanceid;    // instance id, 0 if First Available selected
    recv_data >> asgroup;       // join as group

    sLog.outDebug( "WORLD: Recvd CMSG_BATTLEMASTER_JOIN Message from: " I64FMT, guid);

    // ignore if we already in BG or BG queue
    if(_player->InBattleGround() || _player->InBattleGroundQueue())
        return;

    // check existence
    BattleGround *bg = sBattleGroundMgr.GetBattleGround(bgid);
    if(!bg)
        return;

    if(asgroup && _player->groupInfo.group)
    {
        Group *grp = _player->groupInfo.group;
        Group::MemberList const& members = grp->GetMembers();
        for(Group::member_citerator itr = members.begin(); itr != members.end(); ++itr)
        {
            Player *member = objmgr.GetPlayer(itr->guid);
            if(member)
            {
                member->SetBattleGroundQueueId(bgid); // add to queue

                // store entry point coords (same as leader entry point)
                member->SetBattleGroundEntryPoint(_player->GetMapId(),_player->GetPositionX(),_player->GetPositionY(),_player->GetPositionZ(),_player->GetOrientation());

                sBattleGroundMgr.SendBattleGroundStatusPacket(member, bg, STATUS_WAIT_QUEUE, 0, 0); // send status packet (in queue)
                sBattleGroundMgr.SendGroupJoinedBattlegroundPacket(member, bgid);
                bg->AddPlayerToQueue(member);
            }
        }
    }
    else
    {
        _player->SetBattleGroundQueueId(bgid); // add to queue

        // store entry point coords
        _player->SetBattleGroundEntryPoint(_player->GetMapId(),_player->GetPositionX(),_player->GetPositionY(),_player->GetPositionZ(),_player->GetOrientation());

        sBattleGroundMgr.SendBattleGroundStatusPacket(_player, bg, STATUS_WAIT_QUEUE, 0, 0); // send status packet (in queue)
        bg->AddPlayerToQueue(_player);
    }
}

void WorldSession::HandleBattleGroundPlayerPositionsOpcode( WorldPacket &recv_data )
{
    sLog.outDebug("WORLD: Recvd MSG_BATTLEGROUND_PLAYER_POSITIONS Message"); // empty opcode

    if(!_player->InBattleGround()) // can't be received if player not in battleground
        return;

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(_player->GetBattleGroundId());
    if(!bg)
        return;

    uint32 count = 0;

    Player *ap = objmgr.GetPlayer(bg->GetAllianceFlagPickerGUID());
    if(ap)
        count++;

    Player *hp = objmgr.GetPlayer(bg->GetHordeFlagPickerGUID());
    if(hp)
        count++;

    WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS, (4+4+16*count));
    data << uint32(0); // 2.0.8, unk
    data << count;
    if(ap)
    {
        data << ap->GetGUID();
        data << ap->GetPositionX();
        data << ap->GetPositionY();
    }
    if(hp)
    {
        data << hp->GetGUID();
        data << hp->GetPositionX();
        data << hp->GetPositionY();
    }

    SendPacket(&data);
}

void WorldSession::HandleBattleGroundPVPlogdataOpcode( WorldPacket &recv_data )
{
    sLog.outDebug( "WORLD: Recvd MSG_PVP_LOG_DATA Message");

    if(!_player->InBattleGround())
        return;

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(_player->GetBattleGroundId());
    if(!bg)
        return;

    sBattleGroundMgr.SendPvpLogData(_player, 2, false);

    sLog.outDebug( "WORLD: Sent MSG_PVP_LOG_DATA Message");
}

void WorldSession::HandleBattleGroundListOpcode( WorldPacket &recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 4);

    sLog.outDebug( "WORLD: Recvd CMSG_BATTLEFIELD_LIST Message");

    if(!_player->InBattleGroundQueue()) // can't be received if player not in BG queue
        return;

    uint32 bgid;
    recv_data >> bgid;          // id from DBC

    BattlemasterListEntry const* bl = sBattlemasterListStore.LookupEntry(bgid);

    if(!bl)
        return;

    WorldPacket data;
    sBattleGroundMgr.BuildBattleGroundListPacket(&data, _player->GetGUID(), _player, bgid);
    SendPacket( &data );
}

void WorldSession::HandleBattleGroundPlayerPortOpcode( WorldPacket &recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 1+1+4+2+1);

    sLog.outDebug( "WORLD: Recvd CMSG_BATTLEFIELD_PORT Message");
    recv_data.hexlike();

    uint8 unk1;
    uint8 unk2;     // unk, can be 0x0 (may be if was invited?) and 0x1 
    uint32 bgId;    // id from dbc
    uint16 unk;
    uint8 action;   // enter battle 0x1, leave queue 0x0

    recv_data >> unk1 >> unk2 >> bgId >> unk;
    recv_data >> action;

    BattleGround *bg = sBattleGroundMgr.GetBattleGround(bgId);
    if(bg)
    {
        if(_player->InBattleGroundQueue())
        {
            switch(action)
            {
                case 1:
                    bg->RemovePlayerFromQueue(_player->GetGUID());
                    _player->SetBattleGroundId(bg->GetID());
                    sBattleGroundMgr.SendToBattleGround(_player, bgId);
                    bg->AddPlayer(_player);
                    break;
                case 0:
                    bg->RemovePlayerFromQueue(_player->GetGUID());
                    sBattleGroundMgr.SendBattleGroundStatusPacket(_player, bg, STATUS_NONE, 0, 0);
                    break;
                default:
                    sLog.outError("Battleground port: unknown action %u", action);
            }
        }
    }
}

void WorldSession::HandleBattleGroundLeaveOpcode( WorldPacket &recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 1+1+4+2);

    sLog.outDebug( "WORLD: Recvd CMSG_LEAVE_BATTLEFIELD Message");

    uint8 unk1, unk2;
    uint32 bgid; // id from DBC
    uint16 unk3;

    recv_data >> unk1 >> unk2 >> bgid >> unk3;

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(_player->GetBattleGroundId());
    if(bg)
        bg->RemovePlayer(_player->GetGUID(), true, true);
}

void WorldSession::HandleBattlefieldStatusOpcode( WorldPacket & recv_data )
{
    // empty opcode
    sLog.outDebug( "WORLD: Battleground status" );

    if(_player->InBattleGround())
    {
        BattleGround* bg = sBattleGroundMgr.GetBattleGround(_player->GetBattleGroundId());
        if(bg)
        {
            uint32 time1 = getMSTime() - bg->GetStartTime();
            sBattleGroundMgr.SendBattleGroundStatusPacket(_player, bg, STATUS_INPROGRESS, 0, time1);
        }
    }
}
