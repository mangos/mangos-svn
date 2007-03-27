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

#include "Object.h"
#include "Player.h"
#include "BattleGround.h"
#include "Creature.h"
#include "Chat.h"
#include "Spell.h"

// TODO: Make a warper for all this type of opcodes, and add this one to it
// becouse this is a relative universal opcode
void SendAreaTriggerMessage(Player* Target, const char* Text, ...)
{
    va_list ap;                                             //
    char str [1024];                                        //1024 seems to be rather large
    va_start(ap, Text);
    vsnprintf(str,1024,Text, ap );
    va_end(ap);

    WorldPacket data;
    data.Initialize(SMSG_AREA_TRIGGER_MESSAGE);
    data << uint32(0);
    data << str;
    data << uint8(0);
    Target->GetSession()->SendPacket(&data);
}

BattleGround::BattleGround()
{
    m_ID = 0;
    m_Name = "";
    m_LevelMin = 0;
    m_LevelMax = 0;

    m_TeamScores[0] = 0;
    m_TeamScores[1] = 0;

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
    uint8 idx = GetTeamIndexByTeamId(TeamID);
    m_TeamStartLocX[idx] = X;
    m_TeamStartLocY[idx] = Y;
    m_TeamStartLocZ[idx] = Z;
    m_TeamStartLocO[idx] = O;
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
        if((*itr)->GetSession() && (*itr)->GetTeam() == TeamID )
            (*itr)->GetSession()->SendPacket(packet);
    }
}

void BattleGround::RemovePlayer(Player *plr, bool Transport, bool SendPacket)
{
    std::map<uint64, BattleGroundScore>::iterator itr = m_PlayerScores.find(plr->GetGUID());
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

    for(std::list<Player*>::iterator itr3 = m_QueuedPlayers.begin();itr3!=m_QueuedPlayers.end();++itr3)
    {
        if((*itr3) == plr)
        {
            m_QueuedPlayers.erase(itr3);
            Removed = true;
            break;
        }
    }

    if(!Removed)
        sLog.outError("BATTLEGROUND: Player could not be removed from battleground completely!");

    if(!plr->GetBattleGroundId())
        return;

    // Do next only if found in battleground
    // We're not in BG.
    plr->SetBattleGroundId(0);

    // Let others know
    WorldPacket data;
    sBattleGroundMgr.BuildPlayerLeftBattleGroundPacket(&data,plr);
    SendPacketToAll(&data);

    // Log
    sLog.outDetail("BATTLEGROUND: Player %s left the battle.", plr->GetName());

    // Packets/Movement
    //WorldPacket data;

    if(Transport)
    {
        // Needs vars added to player class and I'm too lazy to rebuild..

        plr->TeleportTo(plr->GetBattleGroundEntryPointMap(), plr->GetBattleGroundEntryPointX(), plr->GetBattleGroundEntryPointY(), plr->GetBattleGroundEntryPointZ(), plr->GetBattleGroundEntryPointO());
        plr->SendInitWorldStates(plr->GetBattleGroundEntryPointMap());
        //sLog.outDetail("BATTLEGROUND: Sending %s to %f,%f,%f,%f", pl->GetName(), x,y,z,O);
    }

    if(SendPacket)
        sBattleGroundMgr.SendBattleGroundStatusPacket(plr, m_MapId, m_ID, 0, 0);

    // Log
    sLog.outDetail("BATTLEGROUND: Removed %s from BattleGround.", plr->GetName());
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

    WorldPacket data;
    sBattleGroundMgr.BuildPlayerJoinedBattleGroundPacket(&data,plr);

    // Let others from your team know //dono if correct if team1 only get team packages?
    SendPacketToTeam(plr->GetTeam(), &data);
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
    SendPacketToTeam(Source->GetTeam(), &data);
}

bool BattleGround::HasFreeSlots(uint32 Team)
{
    //check if the current BG had free slots
    uint32 TeamCounts = 0;
    for(std::list<Player*>::iterator i=m_Players.begin();i!=m_Players.end();++i)
        if((*i)->GetTeam() == Team)
            ++TeamCounts;

    return (TeamCounts < m_MaxPlayersPerTeam);
}

//
void BattleGround::HandleAreaTrigger(Player* Source, uint32 Trigger)
{
    //I thank a neutral friend for the SpellID's
    uint32 SpellId = 0;
    switch(Trigger)
    {
        case 3686:                                          // Speed
        case 3687:                                          // Speed (Horde)
            SpellId=23451;
            break;
        case 3706:                                          // Restoration
        case 3708:                                          // Restoration (Horde)
            SpellId=23493;
            break;
        case 3707:                                          // Berserking
        case 3709:                                          // Berserking (Horde)
            SpellId=23505;
            break;
        case 3669:
        case 3671:
        {
            // Exit BG*/
            if(Source->InBattleGround())
            {
                BattleGround* TempBattlegrounds = sBattleGroundMgr.GetBattleGround(Source->GetBattleGroundId());
                if(TempBattlegrounds)
                    TempBattlegrounds->RemovePlayer(Source,true,true);
                return;
            }
            RemovePlayer(Source, true, true);
            return;
        }break;
        case 3646:
        case 3647:
        {
            // Flag capture points
            return;
        }break;
        default:
        {
            sLog.outError("WARNING: Unhandled AreaTrigger in Battleground: %d", Trigger);
            SendAreaTriggerMessage(Source, "Warning: Unhandled AreaTrigger in Battleground: %d", Trigger);
        }break;
    }

    if(SpellId)
    {
        SpellEntry const *Entry = sSpellStore.LookupEntry(SpellId);

        if(!Entry)
            sLog.outError("WARNING: Tried to add unknown spell id %d to plr.", SpellId);

        Spell spell(Source, Entry, true,0);
        SpellCastTargets targets;
        targets.setUnitTarget(Source);
        targets.m_targetMask = TARGET_FLAG_UNIT;
        spell.prepare(&targets);
    }
}
