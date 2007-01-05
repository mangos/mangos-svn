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
#include "Player.h"
#include "BattleGroundMgr.h"
#include "SharedDefines.h"
#include "Policies/SingletonImp.h"

INSTANTIATE_SINGLETON_1( BattleGroundMgr );

BattleGroundMgr::BattleGroundMgr()
{
    m_BattleGrounds.clear();
}

BattleGroundMgr::~BattleGroundMgr()
{
    for(std::map<uint32, BattleGround*>::iterator itr=m_BattleGrounds.begin();itr!=m_BattleGrounds.end();++itr)
        delete itr->second;
    m_BattleGrounds.clear();
}

void BattleGroundMgr::SendBattleGroundStatusPacket(Player *pl, uint32 MapID, uint8 InstanceID, uint8 StatusID, uint32 Time)
{
    WorldPacket data(SMSG_BATTLEFIELD_STATUS, (5*4+1));     //0x2D4
    data << uint32(0x0);                                    // Unknown 1
    data << uint32(MapID);                                  // MapID
    data << uint8(0);                                       // Unknown
    data << uint32(InstanceID);                             // Instance ID
    data << uint32(StatusID);                               // Status ID
    data << uint32(Time);                                   // Time
    pl->GetSession()->SendPacket( &data );
}

void BattleGroundMgr::BuildPlayerLeftBattleGroundPacket(WorldPacket *data, Player *plr)
{
    // "player" Has left the battle.
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_LEFT);        //0x2EE
    (*data) << plr->GetGUID();
}

void BattleGroundMgr::BuildPlayerJoinedBattleGroundPacket(WorldPacket* data, Player *plr)
{
    // "player" Has joined the battle.
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_JOINED);      //0x2ED
    (*data) << plr->GetGUID();
}

uint32 BattleGroundMgr::CreateBattleGround(uint32 MaxPlayersPerTeam, uint32 LevelMin, uint32 LevelMax, std::string BattleGroundName, uint32 MapID, float Team1StartLocX, float Team1StartLocY, float Team1StartLocZ, float Team1StartLocO, float Team2StartLocX, float Team2StartLocY, float Team2StartLocZ, float Team2StartLocO)
{
    // Create the BG
    BattleGround *bg = new BattleGround;

    bg->SetMapId(MapID);
    bg->SetMaxPlayersPerTeam(MaxPlayersPerTeam);
    bg->SetMaxPlayers(MaxPlayersPerTeam*2);
    bg->SetName(BattleGroundName);
    bg->SetTeamStartLoc(ALLIANCE, Team1StartLocX, Team1StartLocY, Team1StartLocZ, Team1StartLocO);
    bg->SetTeamStartLoc(HORDE,    Team2StartLocX, Team2StartLocY, Team2StartLocZ, Team2StartLocO);
    bg->SetLevelRange(LevelMin, LevelMax);

    uint32 BattleGroundID = m_BattleGrounds.size();         // this will be replaced with instance ID later.
    if(BattleGroundID == 0) BattleGroundID = 1;

    bg->SetID(BattleGroundID);

    AddBattleGround(BattleGroundID, bg);
    sLog.outDetail("BattleGroundMgr: Created new battleground: %u %s (Map %u, %u players per team, Levels %u-%u)", BattleGroundID, bg->m_Name.c_str(), bg->m_MapId, bg->m_MaxPlayersPerTeam, bg->m_LevelMin, bg->m_LevelMax);
    return BattleGroundID;
}

void BattleGroundMgr::CreateInitialBattleGrounds()
{
    std::string bg_Name;
    uint32 MaxPlayersPerTeam;
    uint32 MapId;
    float AStartLoc[4];
    float HStartLoc[4];
    uint32 LevMin;
    uint32 LevMax;

    // Create BG, Warsong Gulch, Levels 1-60, 10 per team
    bg_Name = "Warsong Gulch";
    MapId = 489;
    MaxPlayersPerTeam = 10;
    AStartLoc[0] = 1519.530273f;
    AStartLoc[1] = 1481.868408f;
    AStartLoc[2] = 352.023743f;
    AStartLoc[3] = 3.141593f;

    HStartLoc[0] = 933.989685f;
    HStartLoc[1] = 1430.735840f;
    HStartLoc[2] = 345.537140f;
    HStartLoc[3] = 3.141593f;

    LevMin = 1;
    LevMax = 60;
    sLog.outDetail("Creating battleground %s, %u-%u", bg_Name.c_str(), LevMin, LevMax);
    CreateBattleGround(MaxPlayersPerTeam, LevMin, LevMax, bg_Name, MapId, AStartLoc[0], AStartLoc[1], AStartLoc[2], AStartLoc[3], HStartLoc[0], HStartLoc[1], HStartLoc[2], HStartLoc[3]);

    sLog.outDetail("Created initial battlegrounds.");
}

void BattleGroundMgr::BuildBattleGroundListPacket(WorldPacket* data, uint64 guid, Player* plr)
{
    // We assume for now all BG NPC's are map 489(Warsong gulch)
    uint32 MapId = 489;
    uint32 PlayerLevel = 10;

    if(plr)
        PlayerLevel = plr->getLevel();

    // TODO Lookup npc entry code and find mapid
    // Gossip related

    data->Initialize(SMSG_BATTLEFIELD_LIST);
    (*data) << guid;
    (*data) << MapId;

    std::list<uint32> SendList;
    for(std::map<uint32, BattleGround*>::iterator itr=m_BattleGrounds.begin();itr!=m_BattleGrounds.end();itr++)
        if(itr->second->GetMapId() == MapId && (PlayerLevel >= itr->second->GetMinLevel()) && (PlayerLevel <= itr->second->GetMaxLevel()))
            SendList.push_back(itr->second->GetID());
    uint32 size = SendList.size();

    (*data) << uint32(size << 8);

    uint32 count = 1;

    for(std::list<uint32>::iterator i = SendList.begin();i!=SendList.end();++i)
    {
        (*data) << uint32(count << 8);
        count++;
    }
}

void BattleGroundMgr::AddPlayerToBattleGround(Player *pl, uint32 bgId)
{
    sLog.outDetail("BATTLEGROUND: Added %s to BattleGround.", pl->GetName());
    BattleGround *bg = GetBattleGround(bgId);
    bg->AddPlayer(pl);

}

void BattleGroundMgr::SendToBattleGround(Player *pl, uint32 bgId)
{
    uint32 mapid = GetBattleGround(bgId)->GetMapId();
    float x,y,z,O;
    GetBattleGround(bgId)->GetTeamStartLoc(pl->GetTeam(),x,y,z,O);

    sLog.outDetail("BATTLEGROUND: Sending %s to %f,%f,%f,%f", pl->GetName(), x,y,z,O);
    pl->TeleportTo(mapid, x, y, z, O);
    pl->SendInitWorldStates(mapid);
}
