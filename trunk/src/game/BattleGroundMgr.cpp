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
#include "Player.h"
#include "BattleGroundMgr.h"
#include "SharedDefines.h"
#include "Policies/SingletonImp.h"
#include "MapManager.h"
#include "ObjectMgr.h"

INSTANTIATE_SINGLETON_1( BattleGroundMgr );

BattleGroundMgr::BattleGroundMgr()
{
    m_BattleGrounds.clear();
}

BattleGroundMgr::~BattleGroundMgr()
{
    for(std::map<uint32, BattleGround*>::iterator itr = m_BattleGrounds.begin(); itr != m_BattleGrounds.end(); ++itr)
        delete itr->second;
    m_BattleGrounds.clear();
}

void BattleGroundMgr::Update(time_t diff)
{
    for(BattleGroundSet::iterator itr = m_BattleGrounds.begin(); itr != m_BattleGrounds.end(); ++itr)
    {
        itr->second->Update(diff);
    }
}

void BattleGroundMgr::SendBattleGroundStatusPacket(Player *pl, BattleGround * bg, uint8 StatusID, uint32 Time1, uint32 Time2)
{
    // we can be in 3 queues in same time...
    if(StatusID == 0)
    {
        WorldPacket data(SMSG_BATTLEFIELD_STATUS, 4*3);
        data << uint32(0);    // queue id (0...2)
        data << uint32(0);
        data << uint32(0);
        pl->GetSession()->SendPacket( &data );
        return;
    }
    // queue sounds: 8458, 8459, 8462, 8463, is it used on official?
    WorldPacket data(SMSG_BATTLEFIELD_STATUS, (4+1+1+4+2+4+1+4+4+4));
    data << uint32(0x0);                // queue id (0...2)
    data << uint8(0x0);                 // team type (2=2x2, 3=3x3, 5=5x5), for arenas
    switch(bg->GetID()) // value depends on bg id
    {
        case 1: // AV
            data << uint8(0);
            break;
        case 2: // WSG
            data << uint8(2);
            break;
        case 3: // AB
            data << uint8(1);
            break;
        case 6: // All Arenas
            data << uint8(4);
            break;
        default: // unknown
            data << uint8(0);
    }
    data << bg->GetID();                // id from DBC
    data << uint16(0x1F90);             // unk value 8080
    data << bg->GetInstanceID();        // instance id
    data << uint8(0x0);                 // unk
    data << uint32(StatusID);           // status
    switch(StatusID)
    {
        case STATUS_WAIT_QUEUE:         // status_in_queue
            data << Time1;              // wait time, milliseconds
            data << Time2;              // time in queue, updated every minute?
            break;
        case STATUS_WAIT_JOIN:          // status_invite
            data << bg->GetMapId();     // map id
            data << Time1;              // time to remove from queue, milliseconds
            break;
        case STATUS_INPROGRESS:         // status_in_progress
            data << bg->GetMapId();     // map id
            data << Time1;              // 0 at bg start, 120000 after bg end, time to bg auto leave, milliseconds
            data << Time2;              // time from bg start, milliseconds
            data << uint8(0x1);         // unk
            break;
        default:
            sLog.outError("Unknown BG status!");
            break;
    }
    pl->GetSession()->SendPacket( &data );
}

void BattleGroundMgr::SendPvpLogData(Player *Source, uint8 winner, bool to_all)
{
    // winner can be:
    // 0 = horde
    // 1 = alliance
    // 2 = no winner yet

    if(!Source->InBattleGround())
        return;

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(Source->GetBattleGroundId());
    if(!bg)
        return;

    WorldPacket data(MSG_PVP_LOG_DATA, (1+1+4+40*bg->GetPlayerScoresSize())); // checked on 2.0.8
    data << uint8(0x00);        // seems to be type (battleground=0/arena=1)
    if(winner < 2)
    {
        data << uint8(0x01);    // may be team id, because there some problems with final scoreboard...
        data << winner;
    }
    else
    {
        data << uint8(0x00);    // not enough players flag?
    }
    data << uint32(bg->GetPlayerScoresSize());

    for(std::map<uint64, BattleGroundScore>::const_iterator itr = bg->GetPlayerScoresBegin(); itr != bg->GetPlayerScoresEnd(); ++itr)
    {
        data << (uint64)itr->first;
        data << (uint32)itr->second.KillingBlows;
        data << (uint32)itr->second.HonorableKills;
        data << (uint32)itr->second.Deaths;
        data << (uint32)itr->second.BonusHonor;
        data << (uint32)itr->second.HealingDone;
        data << (uint32)itr->second.DamageDone;
        switch(bg->GetID())
        {
            case 1:                                         // AV
                data << (uint32)0x00000005;                 // count of next fields
                data << (uint32)itr->second.FlagCaptures;   // unk
                data << (uint32)itr->second.FlagReturns;    // unk
                data << (uint32)itr->second.FlagCaptures;   // unk
                data << (uint32)itr->second.FlagReturns;    // unk
                data << (uint32)itr->second.FlagCaptures;   // unk
                break;
            case 2:                                         // WSG
                data << (uint32)0x00000002;                 // count of next fields
                data << (uint32)itr->second.FlagCaptures;   // flag captures
                data << (uint32)itr->second.FlagReturns;    // flag returns
                break;
            case 3:                                         // AB
                data << (uint32)0x00000002;                 // count of next fields
                data << (uint32)itr->second.FlagCaptures;   // unk
                data << (uint32)itr->second.FlagReturns;    // unk
                break;
            default:
                sLog.outDebug("Unhandled MSG_PVP_LOG_DATA for BG id %u", bg->GetID());
                return;
        }
    }
    if(to_all)
        bg->SendPacketToAll(&data);
    else
        Source->GetSession()->SendPacket(&data);
}

void BattleGroundMgr::SendGroupJoinedBattlegroundPacket(Player *Source, uint32 bgid)
{
    /*
    bgid is:
    0 - Your group has joined a battleground queue, but you are not iligible
    1 - Your group has joined the queue for AV
    2 - Your group has joined the queue for WSG
    3 - Your group has joined the queue for AB
    4 - Your group has joined the queue for NA
    5 - Your group has joined the queue for BE Arena
    6 - Your group has joined the queue for All Arenas
    7 - Your group has joined the queue for EotS
    */
    WorldPacket data(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
    data << bgid;
    Source->GetSession()->SendPacket(&data);
}

void BattleGroundMgr::BuildPlayerLeftBattleGroundPacket(WorldPacket *data, Player *plr)
{
    // "player" Has left the battle.
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_LEFT);
    (*data) << plr->GetGUID();
}

void BattleGroundMgr::BuildPlayerJoinedBattleGroundPacket(WorldPacket* data, Player *plr)
{
    // "player" Has joined the battle.
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_JOINED);
    (*data) << plr->GetGUID();
}

uint32 BattleGroundMgr::CreateBattleGround(uint32 bg_ID, uint32 MinPlayersPerTeam, uint32 MaxPlayersPerTeam, uint32 LevelMin, uint32 LevelMax, char* BattleGroundName, uint32 MapID, float Team1StartLocX, float Team1StartLocY, float Team1StartLocZ, float Team1StartLocO, float Team2StartLocX, float Team2StartLocY, float Team2StartLocZ, float Team2StartLocO)
{
    // Create the BG
    BattleGround *bg = new BattleGround;

    bg->SetInstanceID(bg_ID);   // temporary
    bg->SetMapId(MapID);
    bg->SetMinPlayersPerTeam(MinPlayersPerTeam);
    bg->SetMaxPlayersPerTeam(MaxPlayersPerTeam);
    bg->SetMaxPlayers(MaxPlayersPerTeam*2);
    bg->SetName(BattleGroundName);
    bg->SetTeamStartLoc(ALLIANCE, Team1StartLocX, Team1StartLocY, Team1StartLocZ, Team1StartLocO);
    bg->SetTeamStartLoc(HORDE,    Team2StartLocX, Team2StartLocY, Team2StartLocZ, Team2StartLocO);
    bg->SetLevelRange(LevelMin, LevelMax);

    if(bg_ID == 2) // only WSG
    {
        GameObject* AllianceFlag = new GameObject(NULL);
        GameObject* HordeFlag = new GameObject(NULL);
        GameObject* SpeedBonus1 = new GameObject(NULL);
        GameObject* SpeedBonus2 = new GameObject(NULL);
        GameObject* RegenBonus1 = new GameObject(NULL);
        GameObject* RegenBonus2 = new GameObject(NULL);
        GameObject* BerserkBonus1 = new GameObject(NULL);
        GameObject* BerserkBonus2 = new GameObject(NULL);

        AllianceFlag->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179830, bg->GetMapId(), 1540.35, 1481.31, 352.635, 6.24, 0, 0, sin(6.24/2), cos(6.24/2), 0, 0);
        HordeFlag->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179831, bg->GetMapId(), 915.809, 1433.73, 346.172, 3.244, 0, 0, sin(3.244/2), cos(3.244/2), 0, 0);
        SpeedBonus1->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179871, bg->GetMapId(), 1449.98, 1468.86, 342.66, 4.866, 0, 0, sin(4.866/2), cos(4.866/2), 0, 0);
        SpeedBonus2->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179871, bg->GetMapId(), 1006.22, 1445.98, 335.77, 1.683, 0, 0, sin(1.683/2), cos(1.683/2), 0, 0);
        RegenBonus1->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179904, bg->GetMapId(), 1316.94, 1551.99, 313.234, 5.869, 0, 0, sin(5.869/2), cos(5.869/2), 0, 0);
        RegenBonus2->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179904, bg->GetMapId(), 1110.1, 1353.24, 316.513, 5.68, 0, 0, sin(5.68/2), cos(5.68/2), 0, 0);
        BerserkBonus1->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179905, bg->GetMapId(), 1318.68, 1378.03, 314.753, 1.001, 0, 0, sin(1.001/2), cos(1.001/2), 0, 0);
        BerserkBonus2->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179905, bg->GetMapId(), 1141.36, 1560.99, 306.791, 3.858, 0, 0, sin(3.858/2), cos(3.858/2), 0, 0);

        bg->af = AllianceFlag;
        bg->hf = HordeFlag;
        bg->SpeedBonus1 = SpeedBonus1;
        bg->SpeedBonus2 = SpeedBonus2;
        bg->RegenBonus1 = RegenBonus1;
        bg->RegenBonus2 = RegenBonus2;
        bg->BerserkBonus1 = BerserkBonus1;
        bg->BerserkBonus2 = BerserkBonus2;
    }

    bg->SetID(bg_ID);

    AddBattleGround(bg_ID, bg);
    sLog.outDetail("BattleGroundMgr: Created new battleground: %u %s (Map %u, %u players per team, Levels %u-%u)", bg_ID, bg->m_Name, bg->m_MapId, bg->m_MaxPlayersPerTeam, bg->m_LevelMin, bg->m_LevelMax);
    return bg_ID;
}

void BattleGroundMgr::CreateInitialBattleGrounds()
{
    float AStartLoc[4];
    float HStartLoc[4];
    uint32 bg_ID;
    BattlemasterListEntry const* bl;

    // Create BG, AV
    bg_ID = 1;
    bl = sBattlemasterListStore.LookupEntry(bg_ID);
    AStartLoc[0] = 750.134f;    // todo: find correct coords
    AStartLoc[1] = -485.181f;   // todo: find correct coords
    AStartLoc[2] = 91.1357f;    // todo: find correct coords
    AStartLoc[3] = 2.72532f;    // todo: find correct coords

    HStartLoc[0] = -888.637f;   // todo: find correct coords
    HStartLoc[1] = -534.253f;   // todo: find correct coords
    HStartLoc[2] = 54.8074f;    // todo: find correct coords
    HStartLoc[3] = 2.27452f;    // todo: find correct coords

    sLog.outDetail("Creating battleground %s, %u-%u", bl->name, bl->minlvl, bl->maxlvl);
    CreateBattleGround(bg_ID, bl->minplayers, bl->maxplayers, bl->minlvl, bl->maxlvl, bl->name, bl->mapid1, AStartLoc[0], AStartLoc[1], AStartLoc[2], AStartLoc[3], HStartLoc[0], HStartLoc[1], HStartLoc[2], HStartLoc[3]);

    // Create BG, WSG
    bg_ID = 2;
    bl = sBattlemasterListStore.LookupEntry(bg_ID);
    AStartLoc[0] = 1519.530273f;
    AStartLoc[1] = 1481.868408f;
    AStartLoc[2] = 352.023743f;
    AStartLoc[3] = 3.141593f;

    HStartLoc[0] = 933.989685f;
    HStartLoc[1] = 1430.735840f;
    HStartLoc[2] = 345.537140f;
    HStartLoc[3] = 3.141593f;

    sLog.outDetail("Creating battleground %s, %u-%u", bl->name, bl->minlvl, bl->maxlvl);
    CreateBattleGround(bg_ID, bl->minplayers, bl->maxplayers, bl->minlvl, bl->maxlvl, bl->name, bl->mapid1, AStartLoc[0], AStartLoc[1], AStartLoc[2], AStartLoc[3], HStartLoc[0], HStartLoc[1], HStartLoc[2], HStartLoc[3]);

    // Create BG, AB
    bg_ID = 3;
    bl = sBattlemasterListStore.LookupEntry(bg_ID);
    AStartLoc[0] = 1351.55f;    // todo: find correct coords
    AStartLoc[1] = 1289.99f;    // todo: find correct coords
    AStartLoc[2] = -12.1946f;   // todo: find correct coords
    AStartLoc[3] = 3.40156f;    // todo: find correct coords
 
    HStartLoc[0] = 665.942f;    // todo: find correct coords
    HStartLoc[1] = 706.139f;    // todo: find correct coords
    HStartLoc[2] = -14.4749f;   // todo: find correct coords
    HStartLoc[3] = 0.263892f;   // todo: find correct coords

    sLog.outDetail("Creating battleground %s, %u-%u", bl->name, bl->minlvl, bl->maxlvl);
    CreateBattleGround(bg_ID, bl->minplayers, bl->maxplayers, bl->minlvl, bl->maxlvl, bl->name, bl->mapid1, AStartLoc[0], AStartLoc[1], AStartLoc[2], AStartLoc[3], HStartLoc[0], HStartLoc[1], HStartLoc[2], HStartLoc[3]);

    // Create BG, EotS
    bg_ID = 7;
    bl = sBattlemasterListStore.LookupEntry(bg_ID);
    AStartLoc[0] = 1351.55f;    // todo: find coords
    AStartLoc[1] = 1289.99f;    // todo: find coords
    AStartLoc[2] = -12.1946f;   // todo: find coords
    AStartLoc[3] = 3.40156f;    // todo: find coords
 
    HStartLoc[0] = 665.942f;    // todo: find coords
    HStartLoc[1] = 706.139f;    // todo: find coords
    HStartLoc[2] = -14.4749f;   // todo: find coords
    HStartLoc[3] = 0.263892f;   // todo: find coords

    sLog.outDetail("Creating battleground %s, %u-%u", bl->name, bl->minlvl, bl->maxlvl);
    CreateBattleGround(bg_ID, bl->minplayers, bl->maxplayers, bl->minlvl, bl->maxlvl, bl->name, bl->mapid1, AStartLoc[0], AStartLoc[1], AStartLoc[2], AStartLoc[3], HStartLoc[0], HStartLoc[1], HStartLoc[2], HStartLoc[3]);

    sLog.outDetail("Created initial battlegrounds.");
}

void BattleGroundMgr::BuildBattleGroundListPacket(WorldPacket* data, uint64 guid, Player* plr, uint32 bgId)
{
    uint32 PlayerLevel = 10;

    if(plr)
        PlayerLevel = plr->getLevel();

    data->Initialize(SMSG_BATTLEFIELD_LIST);
    *data << guid;
    *data << bgid;
    *data << uint8(0x00);

    std::list<uint32> SendList;
    for(std::map<uint32, BattleGround*>::iterator itr = m_BattleGrounds.begin(); itr != m_BattleGrounds.end(); ++itr)
        if(itr->second->GetID() == bgid && (PlayerLevel >= itr->second->GetMinLevel()) && (PlayerLevel <= itr->second->GetMaxLevel()))
            SendList.push_back(itr->second->GetInstanceID());

    *data << uint32(SendList.size());

    for(std::list<uint32>::iterator i = SendList.begin(); i != SendList.end(); ++i)
    {
        *data << uint32(*i);
    }
    SendList.clear();
}

void BattleGroundMgr::SendToBattleGround(Player *pl, uint32 bgId)
{
    BattleGround *bg = GetBattleGround(bgId);
    if(bg)
    {
        uint32 mapid = bg->GetMapId();
        float x, y, z, O;
        bg->GetTeamStartLoc(pl->GetTeam(), x, y, z, O);

        sLog.outDetail("BATTLEGROUND: Sending %s to %f, %f, %f, %f", pl->GetName(), x, y, z, O);
        pl->TeleportTo(mapid, x, y, z, O);
    }
}
