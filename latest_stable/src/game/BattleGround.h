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

#ifndef __BATTLEGROUND_H
#define __BATTLEGROUND_H

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Chat.h"
#include "BattleGroundMgr.h"
#include "SharedDefines.h"

enum BattleGroundStatus
{
    STATUS_NONE         = 0,
    STATUS_WAIT_QUEUE   = 1,
    STATUS_WAIT_JOIN    = 2,
    STATUS_INPROGRESS   = 3,
    STATUS_WAIT_LEAVE   = 4     // custom
};

struct BattleGroundScore
{
    uint32 KillingBlows;
    uint32 HonorableKills;
    uint32 Deaths;
    uint32 DamageDone;
    uint32 HealingDone;
    uint32 FlagCaptures;
    uint32 FlagReturns;
    uint32 BonusHonor;
    uint32 Unk2;
};

struct BattleGroundQueue
{
    uint32  InviteTime;         // first invite time
    uint32  LastInviteTime;     // last invite time
    bool    IsInvited;          // was invited flag
    uint32  LastOnlineTime;     // for tracking and removing offline players from queue after 5 minutes
};

struct BattleGroundPlayer
{
    uint32  LastOnlineTime;     // for tracking and removing offline players from queue after 5 minutes
    uint32  Team;               // Player's team
};

class BattleGround
{
    friend class BattleGroundMgr;

    public:
        /* Construction */
        BattleGround();
        ~BattleGround();
        void Update(time_t diff);

        /* Battleground */
        void SetName(char const* Name) { m_Name = Name; };
        char const* GetName() const { return m_Name; };
        void SetID(uint32 ID) { m_ID = ID; };
        uint32 GetID() const { return m_ID; };
        void SetInstanceID(uint32 InstanceID) { m_InstanceID = InstanceID; };
        uint32 GetInstanceID() const { return m_InstanceID; };
        void SetStatus(uint32 Status) { m_Status = Status; };
        uint32 GetStatus() const { return m_Status; };
        void SetStartTime(uint32 Time) { m_StartTime = Time; };
        uint32 GetStartTime() const { return m_StartTime; };
        void SetEndTime(uint32 Time) { m_EndTime = Time; };
        uint32 GetEndTime() const { return m_EndTime; };
        void SetMaxPlayers(uint32 MaxPlayers) { m_MaxPlayers = MaxPlayers; };
        uint32 GetMaxPlayers() const { return m_MaxPlayers; };
        void SetMinPlayers(uint32 MinPlayers) { m_MinPlayers = MinPlayers; };
        uint32 GetMinPlayers() const { return m_MinPlayers; };
        void SetLevelRange(uint32 min, uint32 max)
        {
            m_LevelMin = min;
            m_LevelMax = max;
        }
        uint32 GetMinLevel() const { return m_LevelMin; };
        uint32 GetMaxLevel() const { return m_LevelMax; };

        void SetMaxPlayersPerTeam(uint32 MaxPlayers) { m_MaxPlayersPerTeam = MaxPlayers; };
        uint32 GetMaxPlayersPerTeam() const { return m_MaxPlayersPerTeam; };
        void SetMinPlayersPerTeam(uint32 MinPlayers) { m_MinPlayersPerTeam = MinPlayers; };
        uint32 GetMinPlayersPerTeam() const { return m_MinPlayersPerTeam; };

        bool HasFreeSlots(uint32 Team) const;

        void HandleAreaTrigger(Player* Source, uint32 Trigger);

        uint32 GetPlayersSize() const { return m_Players.size(); };
        uint32 GetQueuedPlayersSize() const { return m_QueuedPlayers.size(); };
        uint32 GetRemovedPlayersSize() const { return m_RemovedPlayers.size(); };

        std::map<uint64, BattleGroundScore>::const_iterator GetPlayerScoresBegin() const { return m_PlayerScores.begin(); };
        std::map<uint64, BattleGroundScore>::const_iterator GetPlayerScoresEnd() const { return m_PlayerScores.end(); };
        uint32 GetPlayerScoresSize() { return m_PlayerScores.size(); };

        void AddPlayer(Player* plr);
        void AddPlayerToQueue(uint64 guid);
        void RemovePlayerFromQueue(uint64 guid);
        bool CanStartBattleGround();
        void StartBattleGround();
        void RemovePlayer(uint64 guid, bool Transport = false, bool SendPacket = false);

        /* Battleground Events */
        void EventPlayerCapturedFlag(Player* Source);
        void EventPlayerDroppedFlag(Player* Source);
        void EventPlayerReturnedFlag(Player* Source);
        void EventPlayerPickedUpFlag(Player* Source);

        /* Location */
        void SetMapId(uint32 MapID) { m_MapId = MapID; };
        uint32 GetMapId() const { return m_MapId; };

        void SetTeamStartLoc(uint32 TeamID, float X, float Y, float Z, float O);
        void GetTeamStartLoc(uint32 TeamID, float &X, float &Y, float &Z, float &O) const
        {
            uint8 idx = GetTeamIndexByTeamId(TeamID);
            X = m_TeamStartLocX[idx];
            Y = m_TeamStartLocY[idx];
            Z = m_TeamStartLocZ[idx];
            O = m_TeamStartLocO[idx];
        }

        /* Packet Transfer */
        void SendPacketToTeam(uint32 TeamID, WorldPacket *packet);
        void SendPacketToAll(WorldPacket *packet);
        void PlaySoundToTeam(uint32 SoundID, uint32 TeamID);
        void PlaySoundToAll(uint32 SoundID);
        void CastSpellOnTeam(uint32 SpellID, uint32 TeamID);
        void UpdateWorldState(uint32 Field, uint32 Value);
        void EndBattleGround(uint32 winner);
        void BlockMovement(Player *plr);

        /* Raid Group */
        Group *GetBgRaid(uint32 TeamID) const;
        void SetBgRaid(uint32 TeamID, Group *bg_raid);

        /* BG Flags */
        uint64 GetAllianceFlagPickerGUID() const { return m_AllianceFlagPickerGUID; }
        uint64 GetHordeFlagPickerGUID() const { return m_HordeFlagPickerGUID; }
        void SetAllianceFlagPicker(uint64 guid) { m_AllianceFlagPickerGUID = guid; }
        void SetHordeFlagPicker(uint64 guid) { m_HordeFlagPickerGUID = guid; }
        bool IsAllianceFlagPickedup() const { return m_AllianceFlagPickerGUID != 0; }
        bool IsHordeFlagPickedup() const { return m_HordeFlagPickerGUID != 0; }
        void RespawnFlag(uint32 Team, bool captured);

        /* Scorekeeping */
        uint32 GetTeamScore(uint32 TeamID) const { return m_TeamScores[GetTeamIndexByTeamId(TeamID)]; }

        void AddPoint(uint32 TeamID, uint32 Points = 1)
        {
            m_TeamScores[GetTeamIndexByTeamId(TeamID)] += Points;
        }

        void SetTeamPoint(uint32 TeamID, uint32 Points = 0)
        {
            m_TeamScores[GetTeamIndexByTeamId(TeamID)] = Points;
        }

        void RemovePoint(uint32 TeamID, uint32 Points = 1)
        {
            m_TeamScores[GetTeamIndexByTeamId(TeamID)] -= Points;
        }

        void UpdateTeamScore(uint32 team);
        void UpdatePlayerScore(Player* Source, uint32 type, uint32 value);
        void UpdateFlagState(uint32 team, uint32 value);

    private:
        uint8 GetTeamIndexByTeamId(uint32 Team) const { return Team==HORDE ? 0 : 1; }

        /* Battleground */
        uint32 m_ID;
        uint32 m_Type;
        uint32 m_InstanceID;
        uint32 m_Status;
        uint32 m_StartTime;
        uint32 m_EndTime;
        char const* m_Name;

        /* Scorekeeping */
        uint32 m_TeamScores[2];                             // Usually Alliance/Horde
        std::map<uint64, BattleGroundScore> m_PlayerScores; // Player scores

        /* Player lists */
        std::map<uint64, BattleGroundPlayer> m_Players;
        std::map<uint64, BattleGroundQueue> m_QueuedPlayers;
        std::map<uint64, bool> m_RemovedPlayers;

        /* Raid Group */
        Group *m_HordeRaid;
        Group *m_AllianceRaid;

        /* Limits */
        uint32 m_LevelMin;
        uint32 m_LevelMax;
        uint32 m_MaxPlayersPerTeam;
        uint32 m_MaxPlayers;
        uint32 m_MinPlayersPerTeam;
        uint32 m_MinPlayers;

        /* Location */
        uint32 m_MapId;
        float m_TeamStartLocX[2];
        float m_TeamStartLocY[2];
        float m_TeamStartLocZ[2];
        float m_TeamStartLocO[2];

        /* Flags */
        uint64 m_AllianceFlagPickerGUID;
        uint64 m_HordeFlagPickerGUID;
        GameObject *m_HordeFlag;
        GameObject *m_AllianceFlag;
        GameObject *SpeedBonus1;
        GameObject *SpeedBonus2;
        GameObject *RegenBonus1;
        GameObject *RegenBonus2;
        GameObject *BerserkBonus1;
        GameObject *BerserkBonus2;
        int32 SpeedBonus1Spawn[2];
        int32 SpeedBonus2Spawn[2];
        int32 RegenBonus1Spawn[2];
        int32 RegenBonus2Spawn[2];
        int32 BerserkBonus1Spawn[2];
        int32 BerserkBonus2Spawn[2];
        int32 AllianceFlagSpawn[2];
        int32 HordeFlagSpawn[2];
};
#endif
