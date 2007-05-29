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
    STATUS_WAIT_LEAVE   = 4                                 // custom
};

enum BattleGroundABPoints
{
    STABLES_POINT      = 0,
    GOLD_MINE_POINT    = 1,
    FARM_POINT         = 2,
    LUMBER_MILL_POINT  = 3
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
    uint32  InviteTime;                                     // first invite time
    uint32  LastInviteTime;                                 // last invite time
    bool    IsInvited;                                      // was invited flag
    uint32  LastOnlineTime;                                 // for tracking and removing offline players from queue after 5 minutes
};

struct BattleGroundPlayer
{
    uint32  LastOnlineTime;                                 // for tracking and removing offline players from queue after 5 minutes
    uint32  Team;                                           // Player's team
};

#define MAX_QUEUED_PLAYERS_MAP 6

enum BattleGroundId
{
    BATTLEGROUND_AV_ID = 1,
    BATTLEGROUND_WS_ID = 2,
    BATTLEGROUND_AB_ID = 3,
    BATTLEGROUND_ARENA_ID = 6,
    BATTLEGROUND_EY_ID = 7
};

class BattleGround
{
    friend class BattleGroundMgr;

    public:
        /* Construction */
        BattleGround();
        ~BattleGround();
        virtual void Update(time_t diff);                   // must be implemented in BG subclass of BG specific update code, but must in begginning call parent version
        virtual void SetupBattleGround() {}                 // must be implemented in BG subclass

        /* Battleground */
        void SetName(char const* Name) { m_Name = Name; };
        char const* GetName() const { return m_Name; };
        void SetID(uint32 ID) { m_ID = ID; };
        uint32 GetID() const { return m_ID; };
        void SetQueueType(uint32 ID) { m_Queue_type = ID; };
        uint32 GetQueueType() const { return m_Queue_type; };
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

        uint32 GetPlayersSize() const { return m_Players.size(); };
        uint32 GetQueuedPlayersSize(uint32 level) const;
        uint32 GetRemovedPlayersSize() const { return m_RemovedPlayers.size(); };

        std::map<uint64, BattleGroundScore>::const_iterator GetPlayerScoresBegin() const { return m_PlayerScores.begin(); };
        std::map<uint64, BattleGroundScore>::const_iterator GetPlayerScoresEnd() const { return m_PlayerScores.end(); };
        uint32 GetPlayerScoresSize() { return m_PlayerScores.size(); };

        void AddPlayer(Player* plr);
        void AddPlayerToQueue(Player* plr);
        void RemovePlayerFromQueue(uint64 guid);
        bool CanStartBattleGround();
        void StartBattleGround();
        void RemovePlayer(uint64 guid, bool Transport = false, bool SendPacket = false);

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

        /* Triggers handle */
        virtual void HandleAreaTrigger(Player* Source, uint32 Trigger) {}
                                                            // must be implemented in BG subclass
    protected:
        virtual void RemovePlayer(Player *plr,uint64 guid){}// must be implemented in BG subclass
    private:
        uint8 GetTeamIndexByTeamId(uint32 Team) const { return Team==HORDE ? 0 : 1; }

        /* Battleground */
        uint32 m_ID;
        uint32 m_InstanceID;
        uint32 m_Status;
        uint32 m_StartTime;
        uint32 m_EndTime;
        uint32 m_Queue_type;
        char const* m_Name;

        /* Scorekeeping */
        uint32 m_TeamScores[2];                             // Usually Alliance/Horde
        std::map<uint64, BattleGroundScore> m_PlayerScores; // Player scores

        /* Player lists */
        std::map<uint64, BattleGroundPlayer> m_Players;
        std::map<uint64, bool> m_RemovedPlayers;

        typedef std::map<uint64, BattleGroundQueue> QueuedPlayersMap;
        QueuedPlayersMap m_QueuedPlayers[MAX_QUEUED_PLAYERS_MAP];

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

};
#endif
