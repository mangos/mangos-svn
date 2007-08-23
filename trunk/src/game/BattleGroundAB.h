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
#ifndef __BATTLEGROUNDAB_H
#define __BATTLEGROUNDAB_H

class BattleGround;

enum BattleGroundABPoints
{
    STABLES      = 0,
    GOLD_MINE    = 1,
    FARM         = 2,
    LUMBER_MILL  = 3,
    BLACKSMITH   = 4
};

class BattleGroundAB : public BattleGround
{
    friend class BattleGroundMgr;

    public:
        BattleGroundAB();
        ~BattleGroundAB();
        void Update(time_t diff);
        void RemovePlayer(Player *plr,uint64 guid);
        void HandleAreaTrigger(Player *Source, uint32 Trigger);
        //bool SetupBattleGround();

        /* Scorekeeping */
        uint32 GetTeamScore(uint32 TeamID) const { return m_TeamScores[GetTeamIndexByTeamId(TeamID)]; }
        void AddPoint(uint32 TeamID, uint32 Points = 1) { m_TeamScores[GetTeamIndexByTeamId(TeamID)] += Points; }
        void SetTeamPoint(uint32 TeamID, uint32 Points = 0) { m_TeamScores[GetTeamIndexByTeamId(TeamID)] = Points; }
        void RemovePoint(uint32 TeamID, uint32 Points = 1) { m_TeamScores[GetTeamIndexByTeamId(TeamID)] -= Points; }

    private:
        uint32 m_Points[5];
        uint32 m_TeamScores[2];
};
#endif
