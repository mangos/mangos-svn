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
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"       // for chat messages

BattleGround::BattleGround()
{
    m_ID = 0;
    m_InstanceID = 0;
    m_Status = 0;
    m_StartTime = 0;
    m_EndTime = 0;
    m_Name = "";
    m_LevelMin = 0;
    m_LevelMax = 0;

    m_TeamScores[0] = 0;
    m_TeamScores[1] = 0;

    m_MaxPlayersPerTeam = 0;
    m_MaxPlayers = 0;
    m_MinPlayersPerTeam = 0;
    m_MinPlayers = 0;

    m_MapId = 0;
    m_TeamStartLocX[0] = 0;
    m_TeamStartLocX[1] = 0;

    m_TeamStartLocY[0] = 0;
    m_TeamStartLocY[1] = 0;

    m_TeamStartLocZ[0] = 0;
    m_TeamStartLocZ[1] = 0;

    m_TeamStartLocO[0] = 0;
    m_TeamStartLocO[1] = 0;

    m_AllianceFlagPickerGUID = 0;
    m_HordeFlagPickerGUID = 0;

    hf = NULL;
    af = NULL;
    bg_raid_a = NULL;
    bg_raid_h = NULL;
    SpeedBonus1 = NULL;
    SpeedBonus2 = NULL;
    RegenBonus1 = NULL;
    RegenBonus2 = NULL;
    BerserkBonus1 = NULL;
    BerserkBonus2 = NULL;
    SpeedBonus1Spawn[0] = 0;
    SpeedBonus1Spawn[1] = 0;
    SpeedBonus2Spawn[0] = 0;
    SpeedBonus2Spawn[1] = 0;
    RegenBonus1Spawn[0] = 0;
    RegenBonus1Spawn[1] = 0;
    RegenBonus2Spawn[0] = 0;
    RegenBonus2Spawn[1] = 0;
    BerserkBonus1Spawn[0] = 0;
    BerserkBonus1Spawn[1] = 0;
    BerserkBonus2Spawn[0] = 0;
    BerserkBonus2Spawn[1] = 0;
    AllianceFlagSpawn[0] = 0;
    AllianceFlagSpawn[1] = 0;
    HordeFlagSpawn[0] = 0;
    HordeFlagSpawn[1] = 0;
}

BattleGround::~BattleGround()
{

}

void BattleGround::Update(time_t diff)
{
    if(!GetPlayersSize() && !GetQueuedPlayersSize() && !GetRemovedPlayersSize()) // BG is empty
        return;

    if(CanStartBattleGround())
        StartBattleGround();  //Queue is full, we must invite to BG

    if(GetRemovedPlayersSize())
    {
        for(std::map<uint64, bool>::iterator itr = m_RemovedPlayers.begin(); itr != m_RemovedPlayers.end(); ++itr)
        {
            Player *plr = objmgr.GetPlayer(itr->first);
            if(plr) // online player
            {
                if(plr->InBattleGround() && !itr->second) // currently in bg and was removed from bg
                {
                    RemovePlayer(itr->first, true, true);
                }
                else if(plr->InBattleGroundQueue() && itr->second) // currently in queue and was removed from queue
                {
                    RemovePlayerFromQueue(itr->first);
                    sBattleGroundMgr.SendBattleGroundStatusPacket(plr, this, STATUS_NONE, 0, 0);
                }
            }
            else // player currently offline
            {
                if(itr->second)
                    RemovePlayerFromQueue(itr->first);
                else
                    RemovePlayer(itr->first, false, false);
            }
        }
        m_RemovedPlayers.clear();
    }

    // Invite reminder and idle remover (from queue only)
    if(GetQueuedPlayersSize())
    {
        for(std::map<uint64, BattleGroundQueue>::iterator itr = m_QueuedPlayers.begin(); itr != m_QueuedPlayers.end(); ++itr)
        {
            Player *plr = objmgr.GetPlayer(itr->first);
            if(plr)
            {
                // update last online time
                itr->second.LastOnlineTime = getMSTime();

                if(GetStatus() == STATUS_INPROGRESS)
                {
                    if(!itr->second.IsInvited)  // not invited yet
                    {
                        if(HasFreeSlots(plr->GetTeam()))
                        {
                            plr->SaveToDB();
                            sBattleGroundMgr.SendBattleGroundStatusPacket(plr, this, STATUS_WAIT_JOIN, 120000, 0);
                            itr->second.IsInvited = true;
                            itr->second.InviteTime = getMSTime();
                            itr->second.LastInviteTime = getMSTime();
                        }
                    }
                }

                if(itr->second.IsInvited)       // already was invited
                {
                    uint32 t = getMSTime() - itr->second.LastInviteTime;
                    uint32 tt = getMSTime() - itr->second.InviteTime;
                    if (tt >= 120000)           // remove idle player from queue
                    {
                        m_RemovedPlayers[itr->first] = true;
                    }
                    else if(t >= 30000)         // remind every 30 seconds
                    {
                        sBattleGroundMgr.SendBattleGroundStatusPacket(plr, this, STATUS_WAIT_JOIN, 120000 - tt, 0);
                        itr->second.LastInviteTime = getMSTime();
                    }
                    else
                    {
                        ; // do nothing
                    }
                }
            }
            else
            {
                uint32 t = getMSTime() - itr->second.LastOnlineTime;
                if(t >= 300000)                 // 5 minutes
                {
                    m_RemovedPlayers[itr->first] = true;    // add to remove list (queue)
                }
            }
        }
    }

    // remove offline players from bg after ~5 minutes
    if(GetPlayersSize())
    {
        for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player *plr = objmgr.GetPlayer(itr->first);
            if(plr)
            {
                itr->second.LastOnlineTime = getMSTime();
            }
            else
            {
                uint32 t = getMSTime() - itr->second.LastOnlineTime;
                if(t >= 300000) // 5 minutes
                {
                    m_RemovedPlayers[itr->first] = false;   // add to remove list (BG)
                }
            }
        }
    }

    if(GetStatus() == STATUS_WAIT_LEAVE)
    {
        // remove all players from battleground after 2 minutes
        uint32 d = getMSTime() - GetEndTime();
        if(d >= 120000) // 2 minutes
        {
            for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            {
                m_RemovedPlayers[itr->first] = false;       // add to remove list (BG)
            }
            SetStatus(STATUS_WAIT_QUEUE);
        }
    }

    //If Queue is empty and BG-Status = WAIT_JOIN, we must start BG
    if(GetStatus() == STATUS_WAIT_JOIN && !GetQueuedPlayersSize())
    {
        // AV
        if(GetID() == 1)
        {
        }
        // WSG
        if(GetID() == 2)
        {
            AllianceFlagSpawn[1] = 1;
            HordeFlagSpawn[1] = 1;
            sLog.outError("Flags activated...");
            MapManager::Instance().GetMap(af->GetMapId(), af)->Add(af);
            MapManager::Instance().GetMap(hf->GetMapId(), hf)->Add(hf);
            sLog.outError("Flags respawned...");
            SpeedBonus1Spawn[1] = 1;
            SpeedBonus2Spawn[1] = 1;
            RegenBonus1Spawn[1] = 1;
            RegenBonus2Spawn[1] = 1;
            BerserkBonus1Spawn[1] = 1;
            BerserkBonus2Spawn[1] = 1;
            sLog.outError("Bonuses activated...");
            MapManager::Instance().GetMap(SpeedBonus1->GetMapId(), SpeedBonus1)->Add(SpeedBonus1);
            MapManager::Instance().GetMap(SpeedBonus2->GetMapId(), SpeedBonus2)->Add(SpeedBonus2);
            MapManager::Instance().GetMap(RegenBonus1->GetMapId(), RegenBonus1)->Add(RegenBonus1);
            MapManager::Instance().GetMap(RegenBonus2->GetMapId(), RegenBonus2)->Add(RegenBonus2);
            MapManager::Instance().GetMap(BerserkBonus1->GetMapId(), BerserkBonus1)->Add(BerserkBonus1);
            MapManager::Instance().GetMap(BerserkBonus2->GetMapId(), BerserkBonus2)->Add(BerserkBonus2);
            sLog.outError("Bonuses respawned...");
        }
        // AB
        if(GetID() == 3)
        {
        }
        // EotS
        if(GetID() == 7)
        {
        }
        SetStatus(STATUS_INPROGRESS);
    }

    if(GetStatus() == STATUS_INPROGRESS)
    {
        // AV
        if(GetID() == 1)
        {
        }
        // WSG
        if(GetID() == 2)
        {
            // Flags timers
            AllianceFlagSpawn[0] -= diff;
            HordeFlagSpawn[0] -= diff;
            if(AllianceFlagSpawn[0] < 0)
                AllianceFlagSpawn[0] = 0;
            if(HordeFlagSpawn[0] < 0)
                HordeFlagSpawn[0] = 0;
            if(AllianceFlagSpawn[0] == 0 && AllianceFlagSpawn[1] == 0)
            {
                //MapManager::Instance().GetMap(af->GetMapId(), af)->Add(af);
                RespawnFlag(ALLIANCE, true);
                AllianceFlagSpawn[1] = 1;   // spawned
            }
            if(HordeFlagSpawn[0] == 0 && HordeFlagSpawn[1] == 0)
            {
                //MapManager::Instance().GetMap(hf->GetMapId(), hf)->Add(hf);
                RespawnFlag(HORDE, true);
                HordeFlagSpawn[1] = 1;      // spawned
            }
            //Bonuses timers
            SpeedBonus1Spawn[0] -= diff;
            SpeedBonus2Spawn[0] -= diff;
            RegenBonus1Spawn[0] -= diff;
            RegenBonus2Spawn[0] -= diff;
            BerserkBonus1Spawn[0] -= diff;
            BerserkBonus2Spawn[0] -= diff;
            if(SpeedBonus1Spawn[0] < 0)
                SpeedBonus1Spawn[0] = 0;
            if(SpeedBonus2Spawn[0] < 0)
                SpeedBonus2Spawn[0] = 0;
            if(RegenBonus1Spawn[0] < 0)
                RegenBonus1Spawn[0] = 0;
            if(RegenBonus2Spawn[0] < 0)
                RegenBonus2Spawn[0] = 0;
            if(BerserkBonus1Spawn[0] < 0)
                BerserkBonus1Spawn[0] = 0;
            if(BerserkBonus2Spawn[0] < 0)
                BerserkBonus2Spawn[0] = 0;
            //If Timer==0 && SpawnStatus==0 then respawn
            if(SpeedBonus1Spawn[0] == 0 && SpeedBonus1Spawn[1] == 0)
            {
                MapManager::Instance().GetMap(SpeedBonus1->GetMapId(), SpeedBonus1)->Add(SpeedBonus1);
                SpeedBonus1Spawn[1] = 1;
            }
            if(SpeedBonus2Spawn[0] == 0 && SpeedBonus2Spawn[1] == 0)
            {
                MapManager::Instance().GetMap(SpeedBonus2->GetMapId(), SpeedBonus2)->Add(SpeedBonus2);
                SpeedBonus2Spawn[1] = 1;
            }
            if(RegenBonus1Spawn[0] == 0 && RegenBonus1Spawn[1] == 0)
            {
                MapManager::Instance().GetMap(RegenBonus1->GetMapId(), RegenBonus1)->Add(RegenBonus1);
                RegenBonus1Spawn[1] = 1;
            }
            if(RegenBonus2Spawn[0] == 0 && RegenBonus2Spawn[1] == 0)
            {
                MapManager::Instance().GetMap(RegenBonus2->GetMapId(), RegenBonus2)->Add(RegenBonus2);
                RegenBonus2Spawn[1] = 1;
            }
            if(BerserkBonus1Spawn[0] == 0 && BerserkBonus1Spawn[1] == 0)
            {
                MapManager::Instance().GetMap(BerserkBonus1->GetMapId(), BerserkBonus1)->Add(BerserkBonus1);
                BerserkBonus1Spawn[1] = 1;
            }
            if(BerserkBonus2Spawn[0] == 0 && BerserkBonus2Spawn[1] == 0)
            {
                MapManager::Instance().GetMap(BerserkBonus2->GetMapId(), BerserkBonus2)->Add(BerserkBonus2);
                BerserkBonus2Spawn[1] = 1;
            }
        }
        // AB
        if(GetID() == 3)
        {
        }
        // EotS
        if(GetID() == 7)
        {
        }
    }
    //sLog.outError("m_Status=%u diff=%u", m_Status, diff);
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
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(plr)
        {
            plr->GetSession()->SendPacket(packet);
        }
        else
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
        }
    }
}

void BattleGround::SendPacketToTeam(uint32 TeamID, WorldPacket *packet)
{
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(plr && plr->GetTeam() == TeamID)
        {
            plr->GetSession()->SendPacket(packet);
        }
        else
        {
            sLog.outError("Player " I64FMTD " not found or other team!", itr->first);
        }
    }
}

void BattleGround::PlaySoundToAll(uint32 SoundID)
{
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(plr)
        {
            WorldPacket packet(SMSG_PLAY_SOUND, 4);
            packet << SoundID;
            plr->GetSession()->SendPacket(&packet);
        }
        else
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
        }
    }
}

void BattleGround::PlaySoundToTeam(uint32 SoundID, uint32 TeamID)
{
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(plr && plr->GetTeam() == TeamID)
        {
            WorldPacket packet(SMSG_PLAY_SOUND, 4);
            packet << SoundID;
            plr->GetSession()->SendPacket(&packet);
        }
        else
        {
            sLog.outError("Player " I64FMTD " not found or other team!", itr->first);
        }
    }
}

void BattleGround::CastSpellOnTeam(uint32 SpellID, uint32 TeamID)
{
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(plr && plr->GetTeam() == TeamID)
        {
            plr->CastSpell(plr, SpellID, true, 0);
        }
        else
        {
            sLog.outError("Player " I64FMTD " not found or other team!", itr->first);
        }
    }
}

void BattleGround::UpdateWorldState(uint32 Field, uint32 Value)
{
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(plr)
        {
            WorldPacket data(SMSG_UPDATE_WORLD_STATE, 4+4);
            data << Field;
            data << Value;
            plr->GetSession()->SendPacket(&data);
        }
        else
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
        }
    }
}

void BattleGround::EndBattleGround(uint32 winner)
{
    SetStatus(STATUS_WAIT_LEAVE);
    SetEndTime(getMSTime());

    uint32 mark = 0, reputation = 0;

    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(!plr)
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
            continue;
        }

        if(!plr->isAlive())
            plr->ResurrectPlayer();

        BlockMovement(plr);

        uint32 time1 = getMSTime() - GetStartTime();
        sBattleGroundMgr.SendBattleGroundStatusPacket(plr, this, STATUS_INPROGRESS, 120000, time1); // 2 minutes to auto leave BG

        if(plr->GetTeam() == winner)
        {
            switch(GetID())
            {
                case 1:
                    //Create AV Mark of Honor (Winner)
                    mark = 24955;
                    if(plr->GetTeam() == ALLIANCE)
                        reputation = 23534; //need test on offi
                    else
                        reputation = 23529; //need test on offi
                    break;
                case 2:
                    //Create WSG Mark of Honor (Winner)
                    mark = 24951;
                    if(plr->GetTeam() == ALLIANCE)
                        reputation = 23524; //need test on offi
                    else
                        reputation = 23526; //need test on offi
                    break;
                case 3:
                    //Create AB Mark of Honor (Winner)
                    mark = 24953;
                    if(plr->GetTeam() == ALLIANCE)
                        reputation = 24182; //need test on offi
                    else
                        reputation = 24181; //need test on offi
                    break;
            }           
        }
        else
        {
            switch(GetID())
            {
                case 1:
                    mark = 24954; //Create AV Mark of Honor (Loser)
                    break;
                case 2:
                    mark = 24950; //Create WSG Mark of Honor (Loser)
                    break;
                case 3:
                    mark = 24952; //Create AB Mark of Honor (Loser)
                    break;
            }
        }
        if(mark)
        {
            plr->CastSpell(plr, mark, true, 0);
        }

        if(reputation)
        {
            plr->CastSpell(plr, reputation, true, 0);
        }
    }
    SetTeamPoint(ALLIANCE, 0);
    SetTeamPoint(HORDE, 0);
}

Group *BattleGround::GetBgRaid(uint32 TeamId) const
{
    switch(TeamId)
    {
        case ALLIANCE:
            return bg_raid_a;
        case HORDE:
            return bg_raid_h;
        default:
            sLog.outDebug("unknown teamid in BattleGround::SetBgRaid(): %u", TeamId);
            return NULL;
    }
}

void BattleGround::SetBgRaid(uint32 TeamId, Group *bg_raid)
{
    switch(TeamId)
    {
        case ALLIANCE:
            bg_raid_a = bg_raid;
            break;
        case HORDE:
            bg_raid_h = bg_raid;
            break;
        default:
            sLog.outDebug("unknown teamid in BattleGround::SetBgRaid(): %u", TeamId);
            break;
    }
}

void BattleGround::BlockMovement(Player *plr)
{
    WorldPacket data(SMSG_UNKNOWN_794, 8);  // this must block movement...
    data.append(plr->GetPackGUID());
    plr->GetSession()->SendPacket(&data);
}

void BattleGround::RemovePlayer(uint64 guid, bool Transport, bool SendPacket)
{
    Player *plr = objmgr.GetPlayer(guid);

    // Remove from lists/maps
    std::map<uint64, BattleGroundScore>::iterator itr = m_PlayerScores.find(guid);
    if(itr != m_PlayerScores.end())
        m_PlayerScores.erase(itr);

    if(plr && !plr->isAlive())      // resurrect on exit
        plr->ResurrectPlayer();

    if(plr)
    {
        if(IsAllianceFlagPickedup() || IsHordeFlagPickedup())
        {
            // drop flag...
            if(m_AllianceFlagPickerGUID == guid)
                plr->RemoveAurasDueToSpell(23335);
            if(m_HordeFlagPickerGUID == guid)
                plr->RemoveAurasDueToSpell(23333);
        }
    }
    else
    {
        // check this case...
        if(m_AllianceFlagPickerGUID == guid)
        {
            //AllianceFlagSpawn[0] = 0;
            //AllianceFlagSpawn[1] = 1;
            SetAllianceFlagPicker(0);
            RespawnFlag(ALLIANCE, false);
        }
        if(m_HordeFlagPickerGUID == guid)
        {
            //HordeFlagSpawn[0] = 0;
            //HordeFlagSpawn[1] = 1;
            SetHordeFlagPicker(0);
            RespawnFlag(HORDE, false);
        }
    }

    std::map<uint64, BattleGroundPlayer>::iterator itr2 = m_Players.find(guid);
    if(itr2 != m_Players.end())
        m_Players.erase(itr2);

    if(plr)
    {
        if(SendPacket)
            sBattleGroundMgr.SendBattleGroundStatusPacket(plr, this, STATUS_NONE, 0, 0);

        if(!plr->GetBattleGroundId())
            return;

        // remove from raid group if exist
        if(GetBgRaid(plr->GetTeam()))
            if(!GetBgRaid(plr->GetTeam())->RemoveMember(guid, 0))   // group was disbanded
                SetBgRaid(plr->GetTeam(), NULL);

        // Do next only if found in battleground
        plr->SetBattleGroundId(0);      // We're not in BG.

        // Let others know
        WorldPacket data;
        sBattleGroundMgr.BuildPlayerLeftBattleGroundPacket(&data, plr);
        SendPacketToAll(&data);

        // Log
        sLog.outDetail("BATTLEGROUND: Player %s left the battle.", plr->GetName());

        if(Transport)
        {
            plr->TeleportTo(plr->GetBattleGroundEntryPointMap(), plr->GetBattleGroundEntryPointX(), plr->GetBattleGroundEntryPointY(), plr->GetBattleGroundEntryPointZ(), plr->GetBattleGroundEntryPointO(), true, false);
            //sLog.outDetail("BATTLEGROUND: Sending %s to %f,%f,%f,%f", pl->GetName(), x,y,z,O);
        }

        // Log
        sLog.outDetail("BATTLEGROUND: Removed %s from BattleGround.", plr->GetName());
    }

    if(!GetPlayersSize())
    {
        SetStatus(STATUS_WAIT_QUEUE);
        m_Players.clear();
        m_PlayerScores.clear();
        m_TeamScores[0] = 0;
        m_TeamScores[1] = 0;
        if(GetID() == 2)
        {
            MapManager::Instance().GetMap(af->GetMapId(), af)->Remove(af, false);
            MapManager::Instance().GetMap(hf->GetMapId(), hf)->Remove(hf, false);
            sLog.outDebug("Flags despawned...");
        }
    }
}

void BattleGround::AddPlayerToQueue(Player *plr)
{
    if(GetQueuedPlayersSize() < GetMaxPlayers())
    {
        BattleGroundQueue q;
        q.InviteTime = 0;
        q.LastInviteTime = 0;
        q.IsInvited = false;
        q.LastOnlineTime = 0;
        m_QueuedPlayers.insert(pair<uint64, BattleGroundQueue>(plr->GetGUID(),q));
    }
}

void BattleGround::RemovePlayerFromQueue(uint64 guid)
{
    std::map<uint64, BattleGroundQueue>::iterator itr = m_QueuedPlayers.find(guid);
    if(itr != m_QueuedPlayers.end())
        m_QueuedPlayers.erase(itr);

    Player *plr = objmgr.GetPlayer(guid);
    if(plr)
    {
        if(plr->GetBattleGroundQueueId())
        {
            plr->SetBattleGroundQueueId(0);
        }
    }
}

bool BattleGround::CanStartBattleGround()
{
    if(GetStatus() >= STATUS_WAIT_JOIN)     // already started or ended
        return false;

    if(GetQueuedPlayersSize() < GetMaxPlayers())
        return false;

    uint8 hordes = 0;
    uint8 allies = 0;

    for(std::map<uint64, BattleGroundQueue>::iterator itr = m_QueuedPlayers.begin(); itr != m_QueuedPlayers.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(plr)
        {
            if(plr->GetTeam() == ALLIANCE)
                allies++;
            if(plr->GetTeam() == HORDE)
                hordes++;
        }
        else
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
        }
    }

    if(allies >= GetMinPlayersPerTeam() && hordes >= GetMinPlayersPerTeam())
    {
        SetStatus(STATUS_WAIT_JOIN);
        return true;
    }

    return false;
}

void BattleGround::StartBattleGround()
{
    SetStartTime(getMSTime());

    for(std::map<uint64, BattleGroundQueue>::iterator itr = m_QueuedPlayers.begin(); itr != m_QueuedPlayers.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(plr)
        {
            // Save before join (player must loaded out of bg, if disconnected at bg,etc), it's not blizz like...
            plr->SaveToDB();
            sBattleGroundMgr.SendBattleGroundStatusPacket(plr, this, STATUS_WAIT_JOIN, 120000, 0);  // 2 minutes to remove from queue
            itr->second.IsInvited = true;
            itr->second.InviteTime = getMSTime();
            itr->second.LastInviteTime = getMSTime();
        }
        else
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
        }
    }
}

void BattleGround::AddPlayer(Player *plr)
{
    // Create score struct
    BattleGroundScore sc;
    sc.KillingBlows     = 0;    // killing blows
    sc.HonorableKills   = 0;    // honorable kills
    sc.Deaths           = 0;    // deaths
    sc.HealingDone      = 0;    // healing done
    sc.DamageDone       = 0;    // damage done
    sc.FlagCaptures     = 0;    // flag captures
    sc.FlagReturns      = 0;    // flag returns
    sc.BonusHonor       = 0;    // unk
    sc.Unk2             = 0;    // unk

    uint64 guid = plr->GetGUID();

    BattleGroundPlayer bp;
    bp.LastOnlineTime = getMSTime();
    bp.Team = plr->GetTeam();

    // Add to list/maps
    m_Players.insert(pair<uint64, BattleGroundPlayer>(guid, bp));
    m_PlayerScores.insert(pair<uint64, BattleGroundScore>(guid, sc));

    plr->SendInitWorldStates();

    plr->RemoveFromGroup();                                 // leave old group before join battleground raid group, not blizz like (old group must be restored after leave BG)...

    if(!GetBgRaid(plr->GetTeam()))      // first player joined
    {
        Group *group = new Group;
        group->SetBattlegroundGroup(true);
        group->ConvertToRaid();
        group->AddMember(guid, plr->GetName());
        group->ChangeLeader(plr->GetGUID());
        SetBgRaid(plr->GetTeam(), group);
    }
    else                                // raid already exist
    {
        GetBgRaid(plr->GetTeam())->AddMember(guid, plr->GetName());
    }

    WorldPacket data;
    sBattleGroundMgr.BuildPlayerJoinedBattleGroundPacket(&data, plr);

    // Let others from your team know //dono if correct if team1 only get team packages?
    SendPacketToTeam(plr->GetTeam(), &data);
    // Log
    sLog.outDetail("BATTLEGROUND: Player %s joined the battle.", plr->GetName());
}

void BattleGround::EventPlayerCapturedFlag(Player *Source)
{
    if(GetStatus() != STATUS_INPROGRESS)
        return;

    WorldPacket data;
    uint8 type = 0;
    bool win = false;
    uint32 winner = 0;
    const char *message = "";

    if(Source->GetTeam() == ALLIANCE)
    {
        SetHordeFlagPicker(0);                  // must be before aura remove to prevent 2 events (drop+capture) at the same time
        Source->RemoveAurasDueToSpell(23333);   // Drop Horde Flag from Player
        message = LANG_BG_CAPTURED_HF;
        type = CHAT_MSG_BATTLEGROUND_HORDE;
        if(GetTeamScore(ALLIANCE) < 3)
            AddPoint(ALLIANCE, 1);
        PlaySoundToAll(8173);
        if(GetID() == 2) // WSG
        {
            CastSpellOnTeam(23523, ALLIANCE);   // team gain +35 reputation to WSG for each flag capture
        }
        //Source->CastSpell(Source, 23523, true, 0);
    }
    if(Source->GetTeam() == HORDE)
    {
        SetAllianceFlagPicker(0);               // must be before aura remove to prevent 2 events (drop+capture) at the same time
        Source->RemoveAurasDueToSpell(23335);   // Drop Alliance Flag from Player
        message = LANG_BG_CAPTURED_AF;
        type = CHAT_MSG_BATTLEGROUND_ALLIANCE;
        if(GetTeamScore(HORDE) < 3)
            AddPoint(HORDE, 1);
        PlaySoundToAll(8213);
        if(GetID() == 2) // WSG
        {
            CastSpellOnTeam(23525, HORDE);      // team gain +35 reputation to WSG for each flag capture
        }
        //Source->CastSpell(Source, 23525, true, 0);
    }

    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    UpdateFlagState(Source->GetTeam(), 1);
    UpdateTeamScore(Source->GetTeam());
    UpdatePlayerScore(Source, 1, 3);        // +3 kills for flag capture...
    UpdatePlayerScore(Source, 2, 1);        // +1 flag captures...

    const char *winmsg = "";
    if(GetTeamScore(ALLIANCE) == 3)
    {
        winmsg = LANG_BG_A_WINS;
        win = true;

        PlaySoundToAll(8455);                 // alliance wins sound...
        winner = ALLIANCE;

        sBattleGroundMgr.SendPvpLogData(Source, 1, true);
    }
    if(GetTeamScore(HORDE) == 3)
    {
        winmsg = LANG_BG_H_WINS;
        win = true;

        PlaySoundToAll(8454);                 // horde wins sound...
        winner = HORDE;

        sBattleGroundMgr.SendPvpLogData(Source, 0, true);
    }

    sChatHandler.FillMessageData(&data, Source->GetSession(), CHAT_MSG_BATTLEGROUND, LANG_UNIVERSAL, NULL, Source->GetGUID(), winmsg, NULL);
    SendPacketToAll(&data);

    if(win)
        EndBattleGround(winner);
    else
    {
        switch(Source->GetTeam())
        {
            case ALLIANCE:
                HordeFlagSpawn[0] = 1*60*1000;
                HordeFlagSpawn[1] = 0;
                //RespawnFlag(HORDE, true);
                break;
            case HORDE:
                AllianceFlagSpawn[0] = 1*60*1000;
                AllianceFlagSpawn[1] = 0;
                //RespawnFlag(ALLIANCE, true);
                break;
        }
    }
}

void BattleGround::EventPlayerDroppedFlag(Player *Source)
{
    if(GetStatus() != STATUS_INPROGRESS)
        return;

    WorldPacket data;
    const char *message;
    uint8 type = 0;

    if(Source->GetTeam() == ALLIANCE)
    {
        SetHordeFlagPicker(0);
        message = LANG_BG_DROPPED_HF;
        type = CHAT_MSG_BATTLEGROUND_ALLIANCE;
    }
    if(Source->GetTeam() == HORDE)
    {
        SetAllianceFlagPicker(0);
        message = LANG_BG_DROPPED_AF;
        type = CHAT_MSG_BATTLEGROUND_HORDE;
    }

    UpdateFlagState(Source->GetTeam(), 1);

    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    if(Source->GetTeam() == ALLIANCE)
        UpdateWorldState(1546, uint32(-1));
    if(Source->GetTeam() == HORDE)
        UpdateWorldState(1545, uint32(-1));
}

void BattleGround::EventPlayerReturnedFlag(Player *Source)
{
    if(GetStatus() != STATUS_INPROGRESS)
        return;

    WorldPacket data;
    const char *message;
    uint8 type = 0;

    if(Source->GetTeam() == ALLIANCE)
    {
        message = LANG_BG_RETURNED_AF;
        type = CHAT_MSG_BATTLEGROUND_HORDE;
        UpdateFlagState(HORDE, 1);
        RespawnFlag(ALLIANCE, false);
    }
    if(Source->GetTeam() == HORDE)
    {
        message = LANG_BG_RETURNED_HF;
        type = CHAT_MSG_BATTLEGROUND_ALLIANCE;
        UpdateFlagState(ALLIANCE, 1);
        RespawnFlag(HORDE, false);
    }

    PlaySoundToAll(8192);               // flag returned (common sound)
    UpdatePlayerScore(Source, 3, 1);    // +1 to flag returns...

    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);
}

void BattleGround::EventPlayerPickedUpFlag(Player *Source)
{
    if(GetStatus() != STATUS_INPROGRESS)
        return;

    WorldPacket data;
    const char *message;
    uint8 type = 0;

    if(Source->GetTeam() == ALLIANCE)
    {
        message = LANG_BG_PICKEDUP_HF;
        type = CHAT_MSG_BATTLEGROUND_HORDE;
        PlaySoundToAll(8212);
        MapManager::Instance().GetMap(hf->GetMapId(), hf)->Remove(hf, false);
        SetHordeFlagPicker(Source->GetGUID());              // pick up Horde Flag
    }
    if(Source->GetTeam() == HORDE)
    {
        message = LANG_BG_PICKEDUP_AF;
        type = CHAT_MSG_BATTLEGROUND_ALLIANCE;
        PlaySoundToAll(8174);
        MapManager::Instance().GetMap(af->GetMapId(), af)->Remove(af, false);
        SetAllianceFlagPicker(Source->GetGUID());           // pick up Alliance Flag
    }

    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    UpdateFlagState(Source->GetTeam(), 2);

    if(Source->GetTeam() == ALLIANCE)
        UpdateWorldState(1546, 1);
    if(Source->GetTeam() == HORDE)
        UpdateWorldState(1545, 1);
}

bool BattleGround::HasFreeSlots(uint32 Team) const
{
    // queue is unlimited, so we can check free slots only if bg is started...
    uint8 free = 0;
    for(std::map<uint64, BattleGroundPlayer>::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(plr && plr->GetTeam() == Team)
        {
            free++;
        }
    }

    return (free < GetMaxPlayersPerTeam());
}

void BattleGround::UpdateTeamScore(uint32 team)
{
    if(team == ALLIANCE)
        UpdateWorldState(1581, GetTeamScore(team));
    if(team == HORDE)
        UpdateWorldState(1582, GetTeamScore(team));
}

void BattleGround::UpdatePlayerScore(Player* Source, uint32 type, uint32 value)
{
    std::map<uint64, BattleGroundScore>::iterator itr = m_PlayerScores.find(Source->GetGUID());

    if(itr != m_PlayerScores.end())     // player found...
    {
        switch(type)
        {
            case 1: // kills
                itr->second.KillingBlows += value;
                break;
            case 2: // flags captured
                itr->second.FlagCaptures += value;
                break;
            case 3: // flags returned
                itr->second.FlagReturns += value;
                break;
            case 4: // deaths
                itr->second.Deaths += value;
                break;
            default:
                sLog.outDebug("Unknown player score type %u", type);
                break;
        }
    }
}

void BattleGround::UpdateFlagState(uint32 team, uint32 value)
{
    if(team == ALLIANCE)
        UpdateWorldState(2339, value);
    if(team == HORDE)
        UpdateWorldState(2338, value);
}

void BattleGround::RespawnFlag(uint32 Team, bool captured)
{
    // need delay between flag capture and it's respawn about 30-60 seconds...
    if(Team == ALLIANCE)
    {
        sLog.outDebug("Respawn Alliance flag");
        MapManager::Instance().GetMap(af->GetMapId(), af)->Add(af);
    }

    if(Team == HORDE)
    {
        sLog.outDebug("Respawn Horde flag");
        MapManager::Instance().GetMap(hf->GetMapId(), hf)->Add(hf);
    }

    if(captured)
    {
        WorldPacket data;
        const char *message = LANG_BG_F_PLACED;
        sChatHandler.FillMessageData(&data, NULL, CHAT_MSG_BATTLEGROUND, LANG_UNIVERSAL, NULL, 0, message, NULL);
        SendPacketToAll(&data);

        PlaySoundToAll(8232); // flags respawned sound...
    }
}

void BattleGround::HandleAreaTrigger(Player* Source, uint32 Trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if(GetStatus() != STATUS_INPROGRESS)
        return;

    uint32 SpellId = 0;
    switch(Trigger)
    {
        //WSG
        case 3686:                                          //Alliance elixir of speed spawn. Trigger not working, because located inside other areatrigger, can be replaced by IsWithinDist(object, dist) in BattleGround::Update().
            sLog.outError("SpeedBonus1SpawnState = %i, SpeedBonus1SpawnTimer = %i", SpeedBonus1Spawn[1], SpeedBonus1Spawn[0]);
            if(SpeedBonus1Spawn[1] == 0)
            {
                break;
            }
            MapManager::Instance().GetMap(SpeedBonus1->GetMapId(), SpeedBonus1)->Remove(SpeedBonus1, false);
            SpeedBonus1Spawn[0] = 3*60*1000;                // 3 minutes
            SpeedBonus1Spawn[1] = 0;
            SpellId = 23451;
            break;
        case 3687:                                          //Horde elixir of speed spawn. Trigger not working, because located inside other areatrigger, can be replaced by IsWithinDist(object, dist) in BattleGround::Update().
            sLog.outError("SpeedBonus2SpawnState = %i, SpeedBonus2SpawnTimer = %i", SpeedBonus2Spawn[1], SpeedBonus2Spawn[0]);
            if(SpeedBonus2Spawn[1] == 0)
            {
                break;
            }
            MapManager::Instance().GetMap(SpeedBonus2->GetMapId(), SpeedBonus2)->Remove(SpeedBonus2, false);
            SpeedBonus2Spawn[0] = 3*60*1000;                // 3 minutes
            SpeedBonus2Spawn[1] = 0;
            SpellId = 23451;
            break;
        case 3706:                                          //Alliance elixir of regeneration spawn
            sLog.outError("RegenBonus1SpawnState = %u, RegenBonus1SpawnTimer = %u", RegenBonus1Spawn[1], RegenBonus1Spawn[0]);
            if(RegenBonus1Spawn[1] == 0)
            {
                break;
            }
            MapManager::Instance().GetMap(RegenBonus1->GetMapId(), RegenBonus1)->Remove(RegenBonus1, false);
            RegenBonus1Spawn[0] = 3*60*1000; // 3 minutes
            RegenBonus1Spawn[1] = 0;
            SpellId = 23493;
            break;
        case 3708:                                          //Horde elixir of regeneration spawn
            sLog.outError("RegenBonus2SpawnState = %u, RegenBonus2SpawnTimer = %u", RegenBonus2Spawn[1], RegenBonus2Spawn[0]);
            if(RegenBonus2Spawn[1] == 0)
            {
                break;
            }
            MapManager::Instance().GetMap(RegenBonus2->GetMapId(), RegenBonus2)->Remove(RegenBonus2, false);
            RegenBonus2Spawn[0] = 3*60*1000; // 3 minutes
            RegenBonus2Spawn[1] = 0;
            SpellId = 23493;
            break;
        case 3707:                                          //Alliance elixir of berserk spawn
            sLog.outError("BerserkBonus1SpawnState = %u, BerserkBonus1SpawnTimer = %u", BerserkBonus1Spawn[1], BerserkBonus1Spawn[0]);
            if(BerserkBonus1Spawn[1] == 0)
            {
                break;
            }
            MapManager::Instance().GetMap(BerserkBonus1->GetMapId(), BerserkBonus1)->Remove(BerserkBonus1, false);
            BerserkBonus1Spawn[0] = 3*60*1000; // 3 minutes
            BerserkBonus1Spawn[1] = 0;
            SpellId = 23505;
            break;
        case 3709:                                          //Horde elixir of berserk spawn
            sLog.outError("BerserkBonus2SpawnState = %u, BerserkBonus2SpawnTimer = %u", BerserkBonus2Spawn[1], BerserkBonus2Spawn[0]);
            if(BerserkBonus2Spawn[1] == 0)
            {
                break;
            }
            MapManager::Instance().GetMap(BerserkBonus2->GetMapId(), BerserkBonus2)->Remove(BerserkBonus2, false);
            BerserkBonus2Spawn[0] = 3*60*1000; // 3 minutes
            BerserkBonus2Spawn[1] = 0;
            SpellId = 23505;
            break;
        case 3646:                                          //Alliance Flag spawn
            if(IsHordeFlagPickedup() && !IsAllianceFlagPickedup())
            {
                if(GetHordeFlagPickerGUID() == Source->GetGUID())
                {
                    //SpellId = 23389; // strange, not working...
                    EventPlayerCapturedFlag(Source);
                }
            }
            break;
        case 3647:                                          //Horde Flag spawn
            if(IsAllianceFlagPickedup() && !IsHordeFlagPickedup())
            {
                if(GetAllianceFlagPickerGUID() == Source->GetGUID())
                {
                    //SpellId = 23390; // strange, not working...
                    EventPlayerCapturedFlag(Source);
                }
            }
            break;
        case 4631:                                          //Unk1
        case 4633:                                          //Unk2
            break;
        //AB
        case 3866:                                          //Stables
        case 3869:                                          //Gold Mine
        case 3867:                                          //Farm
        case 3868:                                          //Lumber Mill
        case 4020:                                          //Unk1
        case 4021:                                          //Unk2
            break;
        //Exits
        case 3669:                                          //Warsong Gulch Horde Exit (removed, but trigger still exist).
        case 3671:                                          //Warsong Gulch Alliance Exit (removed, but trigger still exist).
            break;
        case 3948:                                          //Arathi Basin Alliance Exit.
            if(Source->GetTeam() != ALLIANCE)
                Source->GetSession()->SendAreaTriggerMessage("Only The Alliance can use that portal");
            else
                RemovePlayer(Source->GetGUID(), true, true);
            break;
        case 3949:                                          //Arathi Basin Horde Exit.
            if(Source->GetTeam() != ALLIANCE)
                Source->GetSession()->SendAreaTriggerMessage("Only The Horde can use that portal");
            else
                RemovePlayer(Source->GetGUID(), true, true);
            break;
        default:
            sLog.outError("WARNING: Unhandled AreaTrigger in Battleground: %d", Trigger);
            Source->GetSession()->SendAreaTriggerMessage("Warning: Unhandled AreaTrigger in Battleground: %u", Trigger);
            break;
    }

    if(SpellId)
    {
        SpellEntry const *Entry = sSpellStore.LookupEntry(SpellId);

        if(!Entry)
        {
            sLog.outError("ERROR: Tried to add unknown spell id %d to plr.", SpellId);
            return;
        }

        Source->CastSpell(Source, Entry, true, 0);
    }
}
