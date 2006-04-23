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

#include "Object.h"
#include "Player.h"
#include "BattleGround.h"

BattleGround::BattleGround()
{
    m_ID = 0;
    m_Name = "";
    m_LevelMin = 0;
    m_LevelMax = 0;
   
    m_TeamScores[0] = 0;
    m_TeamScores[1] = 0;
    m_PlayerScores.clear();

    m_Players.clear();
    m_QueuedPlayers.clear();
    m_MaxPlayersPerTeam = 0;
    m_MaxPlayers = 0;

    m_MapId = 0;
    m_TeamStartLocX[0] = 0;
    m_TeamStartLocX[1] = 0;
    
    m_TeamStartLocY[0] = 0;
    m_TeamStartLocY[1] = 0;
    
    m_TeamStartLocZ[0] = 0;
    m_TeamStartLocZ[1] = 0;
    
    m_TeamStartLocO[0] = 0;
    m_TeamStartLocO[1] = 0;
}

BattleGround::~BattleGround()
{


}

void BattleGround::SetTeamStartLoc(uint32 TeamID, float X, float Y, float Z, float O)
{
    m_TeamStartLocX[TeamID] = X;
    m_TeamStartLocY[TeamID] = Y;
    m_TeamStartLocZ[TeamID] = Z;
    m_TeamStartLocO[TeamID] = O;
}

void BattleGround::SendPacketToAll(WorldPacket *packet)
{
    for(std::list<Player*>::iterator itr=m_Players.begin();itr!=m_Players.end();++itr)
    {
        if((*itr)->GetSession())
            (*itr)->GetSession()->SendPacket(packet);
    }
}

void BattleGround::SendPacketToTeam(uint32 TeamID, WorldPacket *packet)
{
    for(std::list<Player*>::iterator itr=m_Players.begin();itr!=m_Players.end();++itr)
    {
        if((*itr)->GetSession() /* && (*itr)->m_BattleGroundTeam == TeamID */)
            (*itr)->GetSession()->SendPacket(packet);
    }
}

void BattleGround::RemovePlayer(Player *plr, bool Transport, bool SendPacket)
{
    /*std::map<uint64, BattleGroundScore>::iterator itr = m_PlayerScores.find(plr->GetGUID());
    // Remove from lists/maps
    if(itr != m_PlayerScores.end())
        m_PlayerScores.erase(itr);

    bool Removed = false;

    for(std::list<Player*>::iterator itr2=m_Players.begin();itr2!=m_Players.end();++itr2)
    {
        if((*itr2) == plr)
        {
            m_Players.erase(itr2);
            Removed = true;
            break;
        }
    }

    if(!Removed)
    {
        for(std::list<Player*>::iterator itr3 = m_QueuedPlayers.begin();itr3!=m_QueuedPlayers.end();++itr3)
        {
            if((*itr3) == plr)
            {
                m_QueuedPlayers.erase(itr3);
                Removed = true;
                break;
            }
        }
    }

    if(!Removed) sLog.outError("BATTLEGROUND: Player could not be removed from battleground completely!");

    // Let others know
    SendPacketToAll(&sBattleGroundMgr.BuildPlayerLeftBattleGroundPacket(plr));

    // Log
    sLog.outDetail("BATTLEGROUND: Player %s left the battle.", plr->GetName());

    // We're not in BG.
    plr->m_bgBattleGroundID = 0;
    plr->m_bgInBattleGround = false;
    plr->m_bgTeam = 0;

    // Packets/Movement
    //WorldPacket data;

    if(Transport)
    {
        // Needs vars added to player class and I'm too lazy to rebuild..

		plr->smsg_NewWorld(plr->m_bgEntryPointMap, plr->m_bgEntryPointX, plr->m_bgEntryPointY, plr->m_bgEntryPointZ, plr->m_bgEntryPointO);
    }

    if(SendPacket)
        plr->GetSession()->SendPacket(&sBattleGroundMgr.BuildBattleGroundStatusPacket(m_MapId, m_ID, 0, 0));

    // Log
    sLog.outDetail("BATTLEGROUND: Removed %s from BattleGround.", plr->GetName());*/
}

void BattleGround::AddPlayer(Player *plr)
{
    // Create score struct
    BattleGroundScore sc;
    sc.BonusHonor = 0;
    sc.Deaths = 0;
    sc.DishonorableKills = 0;
    sc.HonorableKills = 0;
    sc.KillingBlows = 0;
    sc.Rank = 0;
    sc.Unk = 0;
    sc.Unk1 = 0;
    sc.Unk2 = 0;
    sc.Unk3 = 0;
    sc.Unk4 = 0;

    // Add to list/maps
    m_Players.push_back(plr);
    uint64 guid = plr->GetGUID();

    m_PlayerScores[guid] = sc;

	plr->SendInitWorldStates(plr->GetMapId());

	// Let others from your team know //dono if correct if team1 only get team packages?
    SendPacketToTeam(plr->m_bgTeam,&sBattleGroundMgr.BuildPlayerJoinedBattleGroundPacket(plr));
    // Log
    sLog.outDetail("BATTLEGROUND: Player %s joined the battle.", plr->GetName());
}

void BattleGround::EventPlayerCaptureFlag(Player *Source)
{
	// TODO Event handled, trough spell system
    // TODO Use packet instead
    WorldPacket data;
    char message[100];
    sprintf(message, "%s picked up the flag!", Source->GetName());
    sChatHandler.FillSystemMessageData(&data, NULL, message);
    Source->SendMessageToSet(&data, true);
}

void BattleGround::EventPlayerDroppedFlag(Player *Source)
{
	// TODO Event handled, trough spell system
    // TODO Use packet instead
    WorldPacket data;
    char message[100];
    sprintf(message, "%s dropped the flag!", Source->GetName());
    sChatHandler.FillSystemMessageData(&data, NULL, message);
    Source->SendMessageToSet(&data, true);
}

void BattleGround::EventPlayerPassFlag(Player *Source, Player *Target)
{
	// TODO Event handled, trough spell system
    // TODO Use packet instead
    WorldPacket data;
    char message[100];
    sprintf(message, "%s passed the flag to %s!", Source->GetName(), Target->GetName());
    sChatHandler.FillSystemMessageData(&data, NULL, message);
    SendPacketToTeam(Source->m_bgTeam, &data);
}

bool BattleGround::HasFreeSlots(uint32 Team)
{
	//check if the current BG had free slots
    uint32 TeamCounts[2];
    TeamCounts[0] = 0;
    TeamCounts[1] = 0;
    for(std::list<Player*>::iterator i=m_Players.begin();i!=m_Players.end();++i)
        TeamCounts[(*i)->m_bgTeam]++;

    if(TeamCounts[Team] < m_MaxPlayersPerTeam)
        return true;
    else
        return false;
}
