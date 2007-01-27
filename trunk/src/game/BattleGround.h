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

struct BattleGroundScore
{
    uint32 Unk;
    uint32 KillingBlows;
    uint32 HonorableKills;
    uint32 Deaths;
    uint32 DishonorableKills;
    uint32 BonusHonor;
    uint32 Rank;
    uint32 Unk1;
    uint32 Unk2;
    uint16 Unk3;
    uint8  Unk4;
};

class BattleGround
{
    friend class BattleGroundMgr;

    public:
        /* Construction */
        BattleGround();
        ~BattleGround();

        /* Battleground */
        inline void SetName(std::string Name) { m_Name = Name; };
        inline std::string GetName() { return m_Name; };
        inline void SetID(uint32 ID) { m_ID = ID; };
        inline uint32 GetID() { return m_ID; };
        inline void SetMaxPlayers(uint32 MaxPlayers) { m_MaxPlayers = MaxPlayers; };
        inline uint32 GetMaxPlayers() { return m_MaxPlayers; };
        inline void SetLevelRange(uint32 min, uint32 max)
        {
            m_LevelMin = min;
            m_LevelMax = max;
        }
        inline uint32 GetMinLevel() { return m_LevelMin; };
        inline uint32 GetMaxLevel() { return m_LevelMax; };

        inline void SetMaxPlayersPerTeam(uint32 MaxPlayers) { m_MaxPlayersPerTeam = MaxPlayers; };
        inline uint32 GetMaxPlayersPerTeam() { return m_MaxPlayersPerTeam; };

        bool HasFreeSlots(uint32 Team);

        void HandleAreaTrigger(Player* Source, uint32 Trigger);

        /* Player lists */
        inline std::list<Player*>::iterator GetPlayersBegin() { return m_Players.begin(); };
        inline std::list<Player*>::iterator GetPlayersEnd() { return m_Players.end(); };
        inline uint32 GetPlayersSize() { return m_Players.size(); };

        inline std::list<Player*>::iterator GetQueuedPlayersBegin() { return m_QueuedPlayers.begin(); };
        inline std::list<Player*>::iterator GetQueuedPlayersEnd() { return m_QueuedPlayers.end(); };

        inline std::map<uint64, BattleGroundScore>::iterator GetPlayerScoresBegin() { return m_PlayerScores.begin(); };
        inline std::map<uint64, BattleGroundScore>::iterator GetPlayerScoresEnd() { return m_PlayerScores.end(); };

        inline uint32 GetPlayerScoresSize() { return m_PlayerScores.size(); };

        void AddPlayer(Player* plr);
        void RemovePlayer(Player *plr, bool Transport = false, bool SendPacket = false);

        /* Battleground Events */
        void EventPlayerCaptureFlag(Player* Source);
        void EventPlayerDroppedFlag(Player* Source);
        void EventPlayerPassFlag(Player* Source, Player* Target);

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

        /* Scorekeeping */
        inline uint32 GetTeamScore(uint32 TeamID)
        {
            return m_TeamScores[GetTeamIndexByTeamId(TeamID)];
        }

        inline void AddPoint(uint32 TeamID, uint32 Points = 1)
        {
            m_TeamScores[GetTeamIndexByTeamId(TeamID)] += Points;

            // TODO: Update all players
        }

        inline void RemovePoint(uint32 TeamID, uint32 Points = 1)
        {
            m_TeamScores[GetTeamIndexByTeamId(TeamID)] -= Points;

            // TODO: Update all players
        }

    private:
        inline uint8 GetTeamIndexByTeamId(uint32 Team) { return Team==HORDE ? 0 : 1; }

        /* Battleground */
        uint32 m_ID;
        std::string m_Name;
        uint32 m_LevelMin;
        uint32 m_LevelMax;

        /* Scorekeeping */
        uint32 m_TeamScores[2];                             // Usually Alliance/Horde
        std::map<uint64, BattleGroundScore> m_PlayerScores; // Player scores.. usually I would say by Player* pointer, but it's easier to build with guid

        /* Player lists */
        std::list<Player*> m_Players;
        std::list<Player*> m_QueuedPlayers;
        uint32 m_MaxPlayersPerTeam;
        uint32 m_MaxPlayers;

        /* Location */
        uint32 m_MapId;
        float m_TeamStartLocX[2];
        float m_TeamStartLocY[2];
        float m_TeamStartLocZ[2];
        float m_TeamStartLocO[2];
};
#endif
