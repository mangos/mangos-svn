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
    uint32 Unk1;
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
        inline void SetName(char* Name) { m_Name = Name; };
        inline char* GetName() { return m_Name; };
        inline void SetID(uint32 ID) { m_ID = ID; };
        inline uint32 GetID() { return m_ID; };
        inline void SetInstanceID(uint32 InstanceID) { m_InstanceID = InstanceID; };
        inline uint32 GetInstanceID() { return m_InstanceID; };
        inline void SetStatus(uint32 Status) { m_Status = Status; };
        inline uint32 GetStatus() { return m_Status; };
        inline void SetStartTime(uint32 Time) { m_StartTime = Time; };
        inline uint32 GetStartTime() { return m_StartTime; };
        inline void SetEndTime(uint32 Time) { m_EndTime = Time; };
        inline uint32 GetEndTime() { return m_EndTime; };
        inline void SetMaxPlayers(uint32 MaxPlayers) { m_MaxPlayers = MaxPlayers; };
        inline uint32 GetMaxPlayers() { return m_MaxPlayers; };
        inline void SetMinPlayers(uint32 MinPlayers) { m_MinPlayers = MinPlayers; };
        inline uint32 GetMinPlayers() { return m_MinPlayers; };
        inline void SetLevelRange(uint32 min, uint32 max)
        {
            m_LevelMin = min;
            m_LevelMax = max;
        }
        inline uint32 GetMinLevel() { return m_LevelMin; };
        inline uint32 GetMaxLevel() { return m_LevelMax; };

        inline void SetMaxPlayersPerTeam(uint32 MaxPlayers) { m_MaxPlayersPerTeam = MaxPlayers; };
        inline uint32 GetMaxPlayersPerTeam() { return m_MaxPlayersPerTeam; };
        inline void SetMinPlayersPerTeam(uint32 MinPlayers) { m_MinPlayersPerTeam = MinPlayers; };
        inline uint32 GetMinPlayersPerTeam() { return m_MinPlayersPerTeam; };

        bool HasFreeSlots(uint32 Team);

        void HandleAreaTrigger(Player* Source, uint32 Trigger);

        /* Player lists */
        inline std::map<uint64, BattleGroundPlayer>::iterator GetPlayersBegin() { return m_Players.begin(); };
        inline std::map<uint64, BattleGroundPlayer>::iterator GetPlayersEnd() { return m_Players.end(); };
        inline uint32 GetPlayersSize() { return m_Players.size(); };

        inline std::map<uint64, BattleGroundQueue>::iterator GetQueuedPlayersBegin() { return m_QueuedPlayers.begin(); };
        inline std::map<uint64, BattleGroundQueue>::iterator GetQueuedPlayersEnd() { return m_QueuedPlayers.end(); };
        inline uint32 GetQueuedPlayersSize() { return m_QueuedPlayers.size(); };

        inline uint32 GetRemovedPlayersSize() { return m_RemovedPlayers.size(); };

        inline std::map<uint64, BattleGroundScore>::iterator GetPlayerScoresBegin() { return m_PlayerScores.begin(); };
        inline std::map<uint64, BattleGroundScore>::iterator GetPlayerScoresEnd() { return m_PlayerScores.end(); };
        inline uint32 GetPlayerScoresSize() { return m_PlayerScores.size(); };

        void AddPlayer(Player* plr);
        void AddPlayerToQueue(Player* plr);
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
        inline void SetMapId(uint32 MapID) { m_MapId = MapID; };
        inline uint32 GetMapId() { return m_MapId; };

        void SetTeamStartLoc(uint32 TeamID, float X, float Y, float Z, float O);
        void GetTeamStartLoc(uint32 TeamID, float &X, float &Y, float &Z, float &O)
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
        Group *GetBgRaid(uint32 TeamID);
        void SetBgRaid(uint32 TeamID, Group *bg_raid);

        /* BG Flags */
        uint64 GetAllianceFlagPicker() {return AllianceFlagPicker;}
        uint64 GetHordeFlagPicker() {return HordeFlagPicker;}
        void SetAllianceFlag(uint64 guid, bool state) { AllianceFlagPickedUp = state; AllianceFlagPicker = guid; }
        void SetHordeFlag(uint64 guid, bool state) { HordeFlagPickedUp = state; HordeFlagPicker = guid; }
        bool GetAllianceFlagState() {return AllianceFlagPickedUp;}
        bool GetHordeFlagState() {return HordeFlagPickedUp;}
        void RespawnFlag(uint32 Team, bool captured);

        /* Scorekeeping */
        inline uint32 GetTeamScore(uint32 TeamID)
        {
            return m_TeamScores[GetTeamIndexByTeamId(TeamID)];
        }

        inline void AddPoint(uint32 TeamID, uint32 Points = 1)
        {
            m_TeamScores[GetTeamIndexByTeamId(TeamID)] += Points;
        }

        inline void SetTeamPoint(uint32 TeamID, uint32 Points = 0)
        {
            m_TeamScores[GetTeamIndexByTeamId(TeamID)] = Points;
        }

        inline void RemovePoint(uint32 TeamID, uint32 Points = 1)
        {
            m_TeamScores[GetTeamIndexByTeamId(TeamID)] -= Points;
        }

        void UpdateTeamScore(uint32 team);
        void UpdatePlayerScore(Player* Source, uint32 type, uint32 value);
        void UpdateFlagState(uint32 team, uint32 value);

    private:
        inline uint8 GetTeamIndexByTeamId(uint32 Team) { return Team==HORDE ? 0 : 1; }

        /* Battleground */
        uint32 m_ID;
        uint32 m_InstanceID;
        uint32 m_Status;
        uint32 m_StartTime;
        uint32 m_EndTime;
        char* m_Name;

        /* Scorekeeping */
        uint32 m_TeamScores[2];                             // Usually Alliance/Horde
        std::map<uint64, BattleGroundScore> m_PlayerScores; // Player scores

        /* Player lists */
        std::map<uint64, BattleGroundPlayer> m_Players;
        std::map<uint64, BattleGroundQueue> m_QueuedPlayers;
        std::map<uint64, bool> m_RemovedPlayers;

        /* Raid Group */
        Group *bg_raid_a;
        Group *bg_raid_h;

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
        bool AllianceFlagPickedUp;
        bool HordeFlagPickedUp;
        uint64 AllianceFlagPicker;
        uint64 HordeFlagPicker;
        GameObject *hf;
        GameObject *af;
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
};
#endif
