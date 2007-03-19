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
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;
    sLog.outDebug( "WORLD: Recvd CMSG_BATTLEMASTER_HELLO Message from: " I64FMT, guid);

    // For now we'll assume all battlefield npcs are mapid 489
    // gossip related
    uint32 MapId = 489;
    uint32 PlayerLevel = 1;

    PlayerLevel = GetPlayer()->getLevel();

    // TODO: Lookup npc entry code and find mapid

    WorldPacket data;
    data.Initialize(SMSG_BATTLEFIELD_LIST);
    data << uint64(guid);
    data << uint32(MapId);

    std::list<uint32> SendList;

    for(std::map<uint32, BattleGround*>::iterator itr = sBattleGroundMgr.GetBattleGroundsBegin();itr!= sBattleGroundMgr.GetBattleGroundsEnd();++itr)
    {
        if(itr->second->GetMapId() == MapId && (PlayerLevel >= itr->second->GetMinLevel()) && (PlayerLevel <= itr->second->GetMaxLevel()))
            SendList.push_back(itr->second->GetID());
    }

    uint32 size = SendList.size();

    data << uint32(size << 8);

    uint32 count = 1;

    for(std::list<uint32>::iterator i = SendList.begin();i!=SendList.end();++i)
    {
        data << uint32(count << 8);
        count++;
    }
    SendList.clear();
    data << uint8(0x00);
    SendPacket( &data );
}

void WorldSession::HandleBattleGroundJoinOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;                                      // >> MapID >> Instance;
    sLog.outDebug( "WORLD: Recvd CMSG_BATTLEMASTER_JOIN Message from: " I64FMT, guid);

    // ignore if we already in BG
    if(GetPlayer()->InBattleGround())
        return;

    // TODO: select bg ID base at bg map_id or use map id as bg ID
    uint8 bgID = 1;

    // check existance
    BattleGround *bg = sBattleGroundMgr.GetBattleGround(bgID);
    if(!bg)
        return;

    // We're in BG.
    GetPlayer()->SetBattleGroundId(bgID);

    //sBattleGroundMgr.SendBattleGroundStatusPacket(GetPlayer(), sBattleGroundMgr.GetBattleGround(1)->GetMapId(), sBattleGroundMgr.GetBattleGround(1)->GetID(),3);

    GetPlayer()->SetBattleGroundEntryPointMap(GetPlayer()->GetMapId());
    GetPlayer()->SetBattleGroundEntryPointO(GetPlayer()->GetOrientation());
    GetPlayer()->SetBattleGroundEntryPointX(GetPlayer()->GetPositionX());
    GetPlayer()->SetBattleGroundEntryPointY(GetPlayer()->GetPositionY());
    GetPlayer()->SetBattleGroundEntryPointZ(GetPlayer()->GetPositionZ());

    sBattleGroundMgr.SendToBattleGround(GetPlayer(),bgID);

    //sBattleGroundMgr.SendBattleGroundStatusPacket(GetPlayer(), sBattleGroundMgr.GetBattleGround(1)->GetMapId(), sBattleGroundMgr.GetBattleGround(1)->GetID(), 0x03, 0x001D2A00);

    sBattleGroundMgr.SendBattleGroundStatusPacket(GetPlayer(), sBattleGroundMgr.GetBattleGround(1)->GetMapId(), sBattleGroundMgr.GetBattleGround(bgID)->GetID(), 0x03, 0x00);

    // Adding Player to BattleGround id = 0
    sLog.outDetail("BATTLEGROUND: Added %s to BattleGround.", GetPlayer()->GetName());
    bg->AddPlayer(GetPlayer());

    // Bur: Not sure if we would have to send a position/score update.. maybe the client requests this automatically we'll have to see
}

void WorldSession::HandleBattleGroundPlayerPositionsOpcode( WorldPacket &recv_data )
{

    if(!GetPlayer()->InBattleGround())
        return;

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(GetPlayer()->GetBattleGroundId());
    if(!bg)
        return;

    std::list<Player*> ListToSend;

    for(std::list<Player*>::iterator i = bg->GetPlayersBegin(); i != bg->GetPlayersEnd(); ++i)
    {
        if((*i) != GetPlayer())
            ListToSend.push_back(*i);
    }

    WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS, (4+16*ListToSend.size()+1));
    data << uint32(ListToSend.size());
    for(std::list<Player*>::iterator itr=ListToSend.begin();itr!=ListToSend.end();++itr)
    {
        data << (uint64)(*itr)->GetGUID();
        data << (float)(*itr)->GetPositionX();
        data << (float)(*itr)->GetPositionY();
    }
    data << uint8(0);
    SendPacket(&data);
}

void WorldSession::HandleBattleGroundPVPlogdataOpcode( WorldPacket &recv_data )
{
    sLog.outDebug( "WORLD: Recvd MSG_PVP_LOG_DATA Message");

    if(!GetPlayer()->InBattleGround())
        return;

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(GetPlayer()->GetBattleGroundId());
    if(!bg)
        return;

    WorldPacket data(MSG_PVP_LOG_DATA, (1+4+40*bg->GetPlayerScoresSize()));
    data << uint8(0x0);
    data << uint32(bg->GetPlayerScoresSize());

    for(std::map<uint64, BattleGroundScore>::iterator itr=bg->GetPlayerScoresBegin();itr!=bg->GetPlayerScoresEnd();++itr)
    {
        data << (uint64)itr->first;                         //8
        data << (uint32)itr->second.Rank;                   //4                    //Rank
        data << (uint32)itr->second.KillingBlows;           //4
        data << (uint32)itr->second.Deaths;                 //4
        data << (uint32)itr->second.HonorableKills;         //4
        data << (uint32)itr->second.DishonorableKills;      //4
        data << (uint32)itr->second.BonusHonor;             //4
        data << (uint32)0;
        data << (uint32)0;
        /*data << itr->second.Rank;
        data << itr->second.Unk1;
        data << itr->second.Unk2;
        data << itr->second.Unk3;
        data << itr->second.Unk4;*/
    }
    SendPacket(&data);

    sLog.outDebug( "WORLD: Send MSG_PVP_LOG_DATA Message players:%u", bg->GetPlayerScoresSize());

    //data << (uint8)0; ////Warsong Gulch
    /*data << (uint8)1; //
    data << (uint8)1; //

    //strangest thing 2 different
    data << (uint32)NumberofPlayers;
    for (uint8 i = 0; i < NumberofPlayers; i++)
    {
        data << (uint64)8;//GUID     //8
        data << (uint32)0;//rank
        data << (uint32)0;//killing blows
        data << (uint32)0;//honorable kills
        data << (uint32)0;//deaths
        data << (uint32)0;//Bonus Honor
        data << (uint32)0;//I think Instance
        data << (uint32)0;
        data << (uint32)0;            //8*4 = 32+8=40

        //1 player is 40 bytes
    }
    data.hexlike();
    SendPacket(&data);*/
}

void WorldSession::HandleBattleGroundListOpcode( WorldPacket &recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4);

    sLog.outDebug( "WORLD: Recvd CMSG_BATTLEFIELD_LIST Message");

    uint32 MapID;
    uint32 NumberOfInstances;

    //recv_data.hexlike();

    recv_data >> MapID;

    //CMSG_BATTLEFIELD_LIST (0x023C);

    //MapID = 489;

    NumberOfInstances = 8;
    WorldPacket data(SMSG_BATTLEFIELD_LIST, (8+4+4+4*NumberOfInstances+1));
    data << uint64(GetPlayer()->GetGUID());
    data << uint32(MapID);
    data << uint32(NumberOfInstances << 8);
    for (uint8 i = 0; i < NumberOfInstances; i++)
    {
        data << uint32(i << 8);
    }
    data << uint8(0x00);

    SendPacket(&data);

}

void WorldSession::HandleBattleGroundPlayerPortOpcode( WorldPacket &recv_data )
{
    //CHECK_PACKET_SIZE(recv_data,?);

    sLog.outDebug( "WORLD: Recvd CMSG_BATTLEFIELD_PORT Message");
    //CMSG_BATTLEFIELD_PORT

    /*uint64 guid = 0;
    uint32 MapID = 0;
    uint32 Instance = 0;
    uint8 Enter = 0;

    WorldPacket data;
    recv_data.hexlike();

    //if que = 0 player logs in
    if (GetPlayer()->GetInstanceQue() == 0)
    {
        //recv_data >> guid >> MapID >> Instance;

        recv_data >> (uint32) MapID >> (uint8)Enter;

        if (Enter == 1)
        {
            //Enter the battleground
            //we can do some type of system. BattleGround GetPlayer(guid)->Getcurrentinstance..... enzzz

            Instance = GetPlayer()->GetCurrentInstance();
            //MapID = GetPlayer()->GetInstanceMapId();

            sLog.outDebug( "BATTLEGROUND: Sending Player:%u to Map:%u Instance %u",(uint32)guid, MapID, Instance);

            //data << uint32(0x000001E9) << float(1519.530273f) << float(1481.868408f) << float(352.023743f) << float(3.141593f);
            //Map* Map = MapManager::Instance().GetMap(GetPlayer()->GetInstanceMapId());
            Map* Map = MapManager::Instance().GetMap(MapID);
            float posz = Map->GetHeight(1021.24,1418.05);
            GetPlayer()->SendNewWorld(MapID, 1021.24, 1418.05,posz+0.5,0.0);

            SendBattleFieldStatus(GetPlayer(),MapID, Instance, 3, 0);
            //GetPlayer()->SendInitWorldStates(MapID);

        }
        else
        {

            //remove the battleground sign
            SendBattleFieldStatus(GetPlayer(),MapID, Instance, 0, 0);
        }

    }

    //if player que = in progress and this function is called again its becouse of it quits que
    //MapID and instance maybe not needed for removing
    if (GetPlayer()->GetInstanceQue() > 0)
    {
        sLog.outDebug( "BATTLEGROUND: Player is quiting que");

        SendBattleFieldStatus(GetPlayer(),GetPlayer()->GetInstanceMapId(), GetPlayer()->GetCurrentInstance(), 0, 0);

        GetPlayer()->SetInstanceQue(0);
        GetPlayer()->SetInstanceMapId(0);
        GetPlayer()->SetInstanceType(0);
    }

    //HORDE Coordinates
    //x:1021.24
    //y:1418.05
    //z:341.56

    //uint32 MapID = GetPlayer()->GetMapId();
    */
}

void WorldSession::HandleBattleGroundLeaveOpcode( WorldPacket &recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4);
                                                            //0x2E1
    sLog.outDebug( "WORLD: Recvd CMSG_LEAVE_BATTLEFIELD Message");
    uint32 MapID;
    WorldPacket data;

    recv_data >> MapID;

    /* Send Back to old xyz
    Map* Map = MapManager::Instance().GetMap(GetPlayer()->GetInstanceMapId());
    float posz = Map->GetHeight(1021.24,1418.05);
    GetPlayer()->SendNewWorld(MapID, 1021.24, 1418.05,posz+0.5,0.0);
    */

}
