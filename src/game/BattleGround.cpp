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
#include "Language.h"                                       // for chat messages

BattleGround::BattleGround()
{
    m_ID = 0;
    m_InstanceID = 0;
    m_Status = 0;
    m_StartTime = 0;
    m_EndTime = 0;
    m_LastResurrectTime = 0;
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

    m_HordeRaid = NULL;
    m_AllianceRaid = NULL;

    m_Queue_type = 1;
}

BattleGround::~BattleGround()
{
}

void BattleGround::Update(time_t diff)
{
    bool found = false;

    if(GetPlayersSize())
        found = true;
    else
    {
        for(int i = 0; i < MAX_QUEUED_PLAYERS_MAP; ++i)
        {
            if(m_QueuedPlayers[i].size())
            {
                found = true;
                break;
            }
        }
    }

    if(!found)                                              // BG is empty
    //if(!GetPlayersSize() && !GetQueuedPlayersSize() && !GetRemovedPlayersSize()) // BG is empty
        return;

    if(CanStartBattleGround())
        StartBattleGround();                                //Queue is full, we must invite to BG

    if(GetRemovedPlayersSize())
    {
        for(std::map<uint64, uint8>::iterator itr = m_RemovedPlayers.begin(); itr != m_RemovedPlayers.end(); ++itr)
        {
            Player *plr = objmgr.GetPlayer(itr->first);
            switch(itr->second)
            {
                case 0: // currently in queue and was removed from queue
                    RemovePlayerFromQueue(itr->first);
                    if(plr)
                    {
                        WorldPacket data;
                        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetTeam(), STATUS_NONE, 0, 0);
                        plr->GetSession()->SendPacket(&data);
                    }
                    break;
                case 1: // currently in bg and was removed from bg
                    if(plr)
                        RemovePlayer(itr->first, true, true);
                    else
                        RemovePlayer(itr->first, false, false);
                    break;
                case 2: // revive queue
                    RemovePlayerFromResurrectQueue(itr->first);
                    break;
            }
        }
        m_RemovedPlayers.clear();
    }

    // Invite reminder and idle remover (from queue only)
    QueuedPlayersMap& pQueue = m_QueuedPlayers[GetQueueType()];

    if(GetQueuedPlayersSize((GetQueueType()+2)*10))
    {
        for(QueuedPlayersMap::iterator itr = pQueue.begin(); itr != pQueue.end(); ++itr)
        {
            Player *plr = objmgr.GetPlayer(itr->first);
            if(plr)
            {
                // update last online time
                itr->second.LastOnlineTime = getMSTime();

                if(GetStatus() == STATUS_INPROGRESS)
                {
                    if(!itr->second.IsInvited)              // not invited yet
                    {
                        if(HasFreeSlots(plr->GetTeam()))
                        {
                            plr->SaveToDB();
                            WorldPacket data;
                            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetTeam(), STATUS_WAIT_JOIN, 120000, 0);
                            plr->GetSession()->SendPacket(&data);
                            itr->second.IsInvited = true;
                            itr->second.InviteTime = getMSTime();
                            itr->second.LastInviteTime = getMSTime();
                        }
                    }
                }

                if(itr->second.IsInvited)                   // already was invited
                {
                    uint32 t = getMSTime() - itr->second.LastInviteTime;
                    uint32 tt = getMSTime() - itr->second.InviteTime;
                    if (tt >= 120000)                       // remove idle player from queue
                    {
                        m_RemovedPlayers[itr->first] = 0;
                    }
                    else if(t >= 30000)                     // remind every 30 seconds
                    {
                        WorldPacket data;
                        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetTeam(), STATUS_WAIT_JOIN, 120000 - tt, 0);
                        plr->GetSession()->SendPacket(&data);
                        itr->second.LastInviteTime = getMSTime();
                    }
                }
            }
            else
            {
                uint32 t = getMSTime() - itr->second.LastOnlineTime;
                if(t >= 300000)                             // 5 minutes
                {
                    m_RemovedPlayers[itr->first] = 0;       // add to remove list (queue)
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
                if(t >= 300000)                             // 5 minutes
                {
                    m_RemovedPlayers[itr->first] = 1;       // add to remove list (BG)
                }
            }
        }
    }

    bool need_res = (getMSTime() - m_LastResurrectTime) >= 30000;   // 30 seconds passed
    if(GetReviveQueueSize() && need_res)
    {
        for(std::map<uint64, uint64>::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
        {
            Player *plr = objmgr.GetPlayer(itr->second);
            if(!plr)
            {
                continue;
            }

            Creature *sh = ObjectAccessor::Instance().GetCreature(*plr, itr->first);
            if(!sh)
            {
                continue;
            }

            if(plr->IsWithinDistInMap(sh, 10.0f))           // 20 yards radius
            {
                // spell not working, only visual effect :(
                sh->CastSpell(plr, 22012, true, 0, 0, 0);   // Spirit Heal, effect 117

                plr->ResurrectPlayer();                     // temp
                plr->SpawnCorpseBones();                    // temp
            }
        }

        m_ReviveQueue.clear();
        m_LastResurrectTime = getMSTime();
    }
    else if(need_res)
    {
        // queue is clear and time passed, just update last resurrection time
        m_LastResurrectTime = getMSTime();
    }

    if(GetStatus() == STATUS_WAIT_LEAVE)
    {
        // remove all players from battleground after 2 minutes
        uint32 d = getMSTime() - GetEndTime();
        if(d >= 120000)                                     // 2 minutes
        {
            for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            {
                m_RemovedPlayers[itr->first] = false;       // add to remove list (BG)
            }
            SetStatus(STATUS_WAIT_QUEUE);
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

        if(!plr)
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
            continue;
        }

        if(plr->GetTeam() == TeamID)
            plr->GetSession()->SendPacket(packet);
    }
}

void BattleGround::PlaySoundToAll(uint32 SoundID)
{
    WorldPacket data;
    sBattleGroundMgr.BuildPlaySoundPacket(&data, SoundID);
    SendPacketToAll(&data);
}

void BattleGround::PlaySoundToTeam(uint32 SoundID, uint32 TeamID)
{
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);

        if(!plr)
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
            continue;
        }

        if(plr->GetTeam() == TeamID)
        {
            WorldPacket data;
            sBattleGroundMgr.BuildPlaySoundPacket(&data, SoundID);
            plr->GetSession()->SendPacket(&data);
        }
    }
}

void BattleGround::CastSpellOnTeam(uint32 SpellID, uint32 TeamID)
{
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);

        if(!plr)
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
            continue;
        }

        if(plr->GetTeam() == TeamID)
        {
            plr->CastSpell(plr, SpellID, true, 0);
        }
    }
}

void BattleGround::UpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data;
    sBattleGroundMgr.BuildUpdateWorldStatePacket(&data, Field, Value);
    SendPacketToAll(&data);
}

void BattleGround::EndBattleGround(uint32 winner)
{
    Player *Source = NULL;
    const char *winmsg = "";
    if(winner == ALLIANCE)
    {
        winmsg = LANG_BG_A_WINS;

        PlaySoundToAll(8455);                               // alliance wins sound...

        WorldPacket data2;
        sBattleGroundMgr.BuildPvpLogDataPacket(&data2, this, 1);
        SendPacketToAll(&data2);
    }
    if(winner == HORDE)
    {
        winmsg = LANG_BG_H_WINS;

        PlaySoundToAll(8454);                               // horde wins sound...

        WorldPacket data2;
        sBattleGroundMgr.BuildPvpLogDataPacket(&data2, this, 0);
        SendPacketToAll(&data2);
    }

    SetStatus(STATUS_WAIT_LEAVE);
    SetEndTime(getMSTime());

    uint32 mark = 0;

    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(!plr)
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
            continue;
        }

        if(!plr->isAlive())
        {
            plr->ResurrectPlayer();
            plr->SpawnCorpseBones();
        }

        BlockMovement(plr);

        uint32 time1 = getMSTime() - GetStartTime();

        WorldPacket data;
        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetTeam(), STATUS_INPROGRESS, 120000, time1); // 2 minutes to auto leave BG
        plr->GetSession()->SendPacket(&data);

        if(plr->GetTeam() == winner)
        {
            if(!Source)
                Source = plr;
            switch(GetID())
            {
                case BATTLEGROUND_AV_ID:
                    // Create AV Mark of Honor (Winner)
                    mark = 24955;
                    break;
                case BATTLEGROUND_WS_ID:
                    // Create WSG Mark of Honor (Winner)
                    mark = 24951;
                    break;
                case BATTLEGROUND_AB_ID:
                    // Create AB Mark of Honor (Winner)
                    mark = 24953;
                    break;
            }
        }
        else
        {
            switch(GetID())
            {
                case BATTLEGROUND_AV_ID:
                    mark = 24954;                           //Create AV Mark of Honor (Loser)
                    break;
                case BATTLEGROUND_WS_ID:
                    mark = 24950;                           //Create WSG Mark of Honor (Loser)
                    break;
                case BATTLEGROUND_AB_ID:
                    mark = 24952;                           //Create AB Mark of Honor (Loser)
                    break;
            }
        }
        if(mark)
        {
            plr->CastSpell(plr, mark, true, 0);
        }
    }

    if(Source)
    {
        WorldPacket data;
        sChatHandler.FillMessageData(&data, Source->GetSession(), CHAT_MSG_BATTLEGROUND, LANG_UNIVERSAL, NULL, Source->GetGUID(), winmsg, NULL);
        SendPacketToAll(&data);
    }

    SetTeamPoint(ALLIANCE, 0);
    SetTeamPoint(HORDE, 0);
}

Group *BattleGround::GetBgRaid(uint32 TeamId) const
{
    switch(TeamId)
    {
        case ALLIANCE:
            return m_AllianceRaid;
        case HORDE:
            return m_HordeRaid;
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
            m_AllianceRaid = bg_raid;
            break;
        case HORDE:
            m_HordeRaid = bg_raid;
            break;
        default:
            sLog.outDebug("unknown teamid in BattleGround::SetBgRaid(): %u", TeamId);
            break;
    }
}

void BattleGround::BlockMovement(Player *plr)
{
    WorldPacket data(SMSG_UNKNOWN_794, 8);                  // this must block movement...
    data.append(plr->GetPackGUID());
    plr->GetSession()->SendPacket(&data);
}

void BattleGround::RemovePlayer(uint64 guid, bool Transport, bool SendPacket)
{
    // Remove from lists/maps
    std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.find(guid);
    if(itr != m_Players.end())
        m_Players.erase(itr);

    std::map<uint64, BattleGroundScore>::iterator itr2 = m_PlayerScores.find(guid);
    if(itr2 != m_PlayerScores.end())
        m_PlayerScores.erase(itr2);

    for(std::map<uint64, uint64>::iterator itr3 = m_ReviveQueue.begin(); itr3 != m_ReviveQueue.end(); ++itr3)
    {
        if(itr3->second == guid)
        {
            m_ReviveQueue.erase(itr3);
            break;
        }
    }

    Player *plr = objmgr.GetPlayer(guid);

    if(plr && !plr->isAlive())                              // resurrect on exit
    {
        plr->ResurrectPlayer();
        plr->SpawnCorpseBones();
    }

    RemovePlayer(plr, guid);                                // BG subclass specific code

    if(plr)
    {
        if(SendPacket)
        {
            WorldPacket data;
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetTeam(), STATUS_NONE, 0, 0);
            plr->GetSession()->SendPacket(&data);
        }

        if(!plr->GetBattleGroundId())
            return;

        // remove from raid group if exist
        if(plr->groupInfo.group && plr->groupInfo.group == GetBgRaid(plr->GetTeam()))
                                                            // group was disbanded
            if(!plr->groupInfo.group->RemoveMember(guid, 0))
                SetBgRaid(plr->GetTeam(), NULL);

        //plr->RemoveFromGroup();

        // Do next only if found in battleground
        plr->SetBattleGroundId(0);                          // We're not in BG.

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
    }
}

void BattleGround::AddPlayerToQueue(uint64 guid, uint32 level)
{
    if(GetQueuedPlayersSize(level) < GetMaxPlayers())
    {
        BattleGroundQueue q;
        q.InviteTime = 0;
        q.LastInviteTime = 0;
        q.IsInvited = false;
        q.LastOnlineTime = 0;

        if(level >= 10 && level <= 19)
            m_QueuedPlayers[0].insert(pair<uint64, BattleGroundQueue>(guid, q));
        else if(level >= 20 && level <= 29)
            m_QueuedPlayers[1].insert(pair<uint64, BattleGroundQueue>(guid, q));
        else if(level >= 30 && level <= 39)
            m_QueuedPlayers[2].insert(pair<uint64, BattleGroundQueue>(guid, q));
        else if(level >= 40 && level <= 49)
            m_QueuedPlayers[3].insert(pair<uint64, BattleGroundQueue>(guid, q));
        else if(level >= 50 && level <= 59)
            m_QueuedPlayers[4].insert(pair<uint64, BattleGroundQueue>(guid, q));
        else if(level >= 60 && level <= 69)
            m_QueuedPlayers[5].insert(pair<uint64, BattleGroundQueue>(guid, q));
        else
            m_QueuedPlayers[6].insert(pair<uint64, BattleGroundQueue>(guid, q));
    }
}

void BattleGround::RemovePlayerFromQueue(uint64 guid)
{
    for(int i = 0; i < MAX_QUEUED_PLAYERS_MAP; ++i)
    {
        QueuedPlayersMap::iterator itr = m_QueuedPlayers[i].find(guid);
        if(itr != m_QueuedPlayers[i].end())
            m_QueuedPlayers[i].erase(itr);
    }

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
    if(GetStatus() >= STATUS_WAIT_JOIN)                     // already started or ended
        return false;

    bool ready = false;
    for(int i = 0; i < MAX_QUEUED_PLAYERS_MAP; ++i)
    {
        if(m_QueuedPlayers[i].size() >= GetMinPlayers())
        {
            ready = true;
            break;
        }
    }

    if(!ready)                                              // queue is not ready yet
        return false;

    for(int i = 0; i < MAX_QUEUED_PLAYERS_MAP; ++i)
    {
        uint8 hordes = 0;
        uint8 allies = 0;

        QueuedPlayersMap& pQueue = m_QueuedPlayers[i];

        for(QueuedPlayersMap::iterator itr = pQueue.begin(); itr != pQueue.end(); ++itr)
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
                sLog.outError("Player " I64FMTD " not found!", itr->first);
        }
        if(allies >= GetMinPlayersPerTeam() && hordes >= GetMinPlayersPerTeam())
        {
            SetStatus(STATUS_WAIT_JOIN);
            SetQueueType(i);
            return true;
        }
    }

    return false;
}

void BattleGround::StartBattleGround()
{
    SetStartTime(getMSTime());
    SetLastResurrectTime(getMSTime() + 30000);

    QueuedPlayersMap& pQueue = m_QueuedPlayers[GetQueueType()];

    for(QueuedPlayersMap::iterator itr = pQueue.begin(); itr != pQueue.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(plr)
        {
            // Save before join (player must loaded out of bg, if disconnected at bg,etc), it's not blizz like...
            plr->SaveToDB();
            WorldPacket data;
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetTeam(), STATUS_WAIT_JOIN, 120000, 0);  // 2 minutes to remove from queue
            plr->GetSession()->SendPacket(&data);
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
    sc.KillingBlows     = 0;                                // killing blows
    sc.HonorableKills   = 0;                                // honorable kills
    sc.Deaths           = 0;                                // deaths
    sc.HealingDone      = 0;                                // healing done
    sc.DamageDone       = 0;                                // damage done
    sc.FlagCaptures     = 0;                                // flag captures
    sc.FlagReturns      = 0;                                // flag returns
    sc.BonusHonor       = 0;                                // unk
    sc.Unk2             = 0;                                // unk

    uint64 guid = plr->GetGUID();

    BattleGroundPlayer bp;
    bp.LastOnlineTime = getMSTime();
    bp.Team = plr->GetTeam();

    // Add to list/maps
    m_Players.insert(pair<uint64, BattleGroundPlayer>(guid, bp));
    m_PlayerScores.insert(pair<uint64, BattleGroundScore>(guid, sc));

    plr->SendInitWorldStates();

    plr->RemoveFromGroup();                                 // leave old group before join battleground raid group, not blizz like (old group must be restored after leave BG)...

    if(!GetBgRaid(plr->GetTeam()))                          // first player joined
    {
        Group *group = new Group;
        group->SetBattlegroundGroup(true);
        group->ConvertToRaid();
        group->AddMember(guid, plr->GetName());
        group->ChangeLeader(plr->GetGUID());
        SetBgRaid(plr->GetTeam(), group);
    }
    else                                                    // raid already exist
    {
        GetBgRaid(plr->GetTeam())->AddMember(guid, plr->GetName());
    }

    WorldPacket data;
    sBattleGroundMgr.BuildPlayerJoinedBattleGroundPacket(&data, plr);
    SendPacketToTeam(plr->GetTeam(), &data);

    // Log
    sLog.outDetail("BATTLEGROUND: Player %s joined the battle.", plr->GetName());
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

    if(itr != m_PlayerScores.end())                         // player found...
    {
        switch(type)
        {
            case 1:                                         // kills
                itr->second.KillingBlows += value;
                break;
            case 2:                                         // flags captured
                itr->second.FlagCaptures += value;
                break;
            case 3:                                         // flags returned
                itr->second.FlagReturns += value;
                break;
            case 4:                                         // deaths
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
    else if(team == HORDE)
        UpdateWorldState(2338, value);
}

uint32 BattleGround::GetQueuedPlayersSize(uint32 level) const
{
     if(level >= 10 && level <= 19)
         return m_QueuedPlayers[0].size();
     else if(level >= 20 && level <= 29)
         return m_QueuedPlayers[1].size();
     else if(level >= 30 && level <= 39)
         return m_QueuedPlayers[2].size();
     else if(level >= 40 && level <= 49)
         return m_QueuedPlayers[3].size();
     else if(level >= 50 && level <= 59)
         return m_QueuedPlayers[4].size();
     else if(level >= 60 && level <= 69)
         return m_QueuedPlayers[5].size();
     else
         return m_QueuedPlayers[6].size();
}

void BattleGround::AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid)
{
    m_ReviveQueue[npc_guid] = player_guid;

    Player *plr = objmgr.GetPlayer(player_guid);
    if(!plr)
        return;

    plr->CastSpell(plr, 2584, true, 0, 0, 0); // "Waiting to Resurrect"
}

void BattleGround::RemovePlayerFromResurrectQueue(uint64 player_guid)
{
    for(std::map<uint64, uint64>::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
    {
        if(itr->second == player_guid)
        {
            m_ReviveQueue.erase(itr);
            Player *plr = objmgr.GetPlayer(player_guid);
            if(!plr)
                return;

            plr->RemoveAurasDueToSpell(2584);

            break;
        }
    }
}
