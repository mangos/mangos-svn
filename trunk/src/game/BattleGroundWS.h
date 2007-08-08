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

#ifndef __BATTLEGROUNDWS_H
#define __BATTLEGROUNDWS_H

#include "BattleGround.h"

#define MAX_TEAM_SCORE      3
#define FLAG_RESPAWN_TIME   60000
#define BUFF_RESPAWN_TIME   180000

#define SOUND_FLAG_CAPTURED_ALLIANCE 8173
#define SOUND_FLAG_CAPTURED_HORDE    8213
#define SOUND_FLAG_PLACED            8232

#define SPELL_WARSONG_FLAG      23333
#define SPELL_SILVERWING_FLAG   23335

// WorldStates
#define FLAG_UNK_ALLIANCE       1545
#define FLAG_UNK_HORDE          1546
//#define FLAG_UNK                1547
#define FLAG_CAPTURES_ALLIANCE  1581
#define FLAG_CAPTURES_HORDE     1582
#define FLAG_CAPTURES_MAX       1601
#define FLAG_STATE_HORDE        2338
#define FLAG_STATE_ALLIANCE     2339

enum BattleGroundObjectTypes
{
    BG_OBJECT_A_FLAG        = 0,
    BG_OBJECT_H_FLAG        = 1,
    BG_OBJECT_SPEEDBUFF_1   = 2,
    BG_OBJECT_SPEEDBUFF_2   = 3,
    BG_OBJECT_REGENBUFF_1   = 4,
    BG_OBJECT_REGENBUFF_2   = 5,
    BG_OBJECT_BERSERKBUFF_1 = 6,
    BG_OBJECT_BERSERKBUFF_2 = 7,
    // TODO: Add and handle gates
    BG_OBJECT_MAX           = 8
};

enum FlagState
{
    FLAG_STATE_ON_BASE      = 0,
    FLAG_STATE_ON_PLAYER    = 1,
    FLAG_STATE_ON_GROUND    = 2
};

struct BattleGroundObjectInfo
{
    GameObject  *object;
    bool        spawned;
    int32       timer;
    uint32      spellid;
};

class BattleGroundWS : public BattleGround
{
    friend class BattleGroundMgr;

    public:
        /* Construction */
        BattleGroundWS();
        ~BattleGroundWS();
        void Update(time_t diff);

        /* BG Flags */
        uint64 GetAllianceFlagPickerGUID() const { return m_FlagKeepers[0]; }
        uint64 GetHordeFlagPickerGUID() const { return m_FlagKeepers[1]; }
        void SetAllianceFlagPicker(uint64 guid) { m_FlagKeepers[0] = guid; }
        void SetHordeFlagPicker(uint64 guid) { m_FlagKeepers[1] = guid; }
        bool IsAllianceFlagPickedup() const { return m_FlagKeepers[0] != 0; }
        bool IsHordeFlagPickedup() const { return m_FlagKeepers[1] != 0; }
        void RespawnFlag(uint32 Team, bool captured);
        uint8 GetFlagState(uint32 team) { return m_FlagState[GetTeamIndexByTeamId(team)]; }

        /* Battleground Events */
        void EventPlayerCapturedFlag(Player *Source);
        void EventPlayerDroppedFlag(Player *Source);
        void EventPlayerReturnedFlag(Player *Source);
        void EventPlayerPickedUpFlag(Player *Source);

        void RemovePlayer(Player *plr, uint64 guid);
        void HandleAreaTrigger(Player *Source, uint32 Trigger);
        void HandleKillPlayer(Player* player);
        void HandleDropFlag(Player* player);
        bool SetupBattleGround();
        void Reset();

        void UpdateFlagState(uint32 team, uint32 value);
        void UpdateTeamScore(uint32 team);

        /* Scorekeeping */
        uint32 GetTeamScore(uint32 TeamID) const { return m_TeamScores[GetTeamIndexByTeamId(TeamID)]; }
        void AddPoint(uint32 TeamID, uint32 Points = 1) { m_TeamScores[GetTeamIndexByTeamId(TeamID)] += Points; }
        void SetTeamPoint(uint32 TeamID, uint32 Points = 0) { m_TeamScores[GetTeamIndexByTeamId(TeamID)] = Points; }
        void RemovePoint(uint32 TeamID, uint32 Points = 1) { m_TeamScores[GetTeamIndexByTeamId(TeamID)] -= Points; }

    private:
        uint64 m_FlagKeepers[2];                              // 0 - alliance, 1 - horde
        uint8 m_FlagState[2];                                 // for checking flag state
        uint32 m_TeamScores[2];

        std::map<uint32, BattleGroundObjectInfo> m_bgobjects;
        //std::set<uint32, BattleGroundObjectInfo> m_bgobjects2;
};
#endif
