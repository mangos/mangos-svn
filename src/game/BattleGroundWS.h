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
        uint64 GetAllianceFlagPickerGUID() const { return FlagKeepers[0]; }
        uint64 GetHordeFlagPickerGUID() const { return FlagKeepers[1]; }
        void SetAllianceFlagPicker(uint64 guid) { FlagKeepers[0] = guid; }
        void SetHordeFlagPicker(uint64 guid) { FlagKeepers[1] = guid; }
        bool IsAllianceFlagPickedup() const { return FlagKeepers[0] != 0; }
        bool IsHordeFlagPickedup() const { return FlagKeepers[1] != 0; }
        void RespawnFlag(uint32 Team, bool captured);

        /* Battleground Events */
        void EventPlayerCapturedFlag(Player *Source);
        void EventPlayerDroppedFlag(Player *Source);
        void EventPlayerReturnedFlag(Player *Source);
        void EventPlayerPickedUpFlag(Player *Source);

        void RemovePlayer(Player *plr, uint64 guid);
        void HandleAreaTrigger(Player *Source, uint32 Trigger);
        void SetupBattleGround();

        void UpdateFlagState(uint32 team, uint32 value);
        void UpdateTeamScore(uint32 team);

        /* Scorekeeping */
        uint32 GetTeamScore(uint32 TeamID) const { return m_TeamScores[GetTeamIndexByTeamId(TeamID)]; }
        void AddPoint(uint32 TeamID, uint32 Points = 1) { m_TeamScores[GetTeamIndexByTeamId(TeamID)] += Points; }
        void SetTeamPoint(uint32 TeamID, uint32 Points = 0) { m_TeamScores[GetTeamIndexByTeamId(TeamID)] = Points; }
        void RemovePoint(uint32 TeamID, uint32 Points = 1) { m_TeamScores[GetTeamIndexByTeamId(TeamID)] -= Points; }

    private:
        uint64 FlagKeepers[2];                              // 0 - alliance, 1 - horde
        bool FlagState[2];                                  // for checking in base/dropped state
        uint32 m_TeamScores[2];

        std::map<uint32, BattleGroundObjectInfo> m_bgobjects;
        //std::set<uint32, BattleGroundObjectInfo> m_bgobjects2;
};
#endif
