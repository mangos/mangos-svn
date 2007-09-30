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

#ifndef __BATTLEGROUNDMGR_H
#define __BATTLEGROUNDMGR_H

#include "BattleGround.h"
#include "Policies/Singleton.h"

class BattleGround;

typedef std::map<uint32, BattleGround*> BattleGroundSet;

class BattleGroundMgr
{
    public:
        /* Construction */
        BattleGroundMgr();
        ~BattleGroundMgr();
        void Update(time_t diff);

        /* Packet Building */
        void BuildPlayerJoinedBattleGroundPacket(WorldPacket *data, Player *plr);
        void BuildPlayerLeftBattleGroundPacket(WorldPacket *data, Player *plr);
        void BuildBattleGroundListPacket(WorldPacket *data, uint64 guid, Player *plr, uint32 bgId);
        void BuildGroupJoinedBattlegroundPacket(WorldPacket *data, uint32 bgid);
        void BuildUpdateWorldStatePacket(WorldPacket *data, uint32 field, uint32 value);
        void BuildPvpLogDataPacket(WorldPacket *data, BattleGround *bg);
        void BuildBattleGroundStatusPacket(WorldPacket *data, BattleGround *bg, uint32 team, uint8 StatusID, uint32 Time1, uint32 Time2);
        void BuildPlaySoundPacket(WorldPacket *data, uint32 soundid);

        /* Battlegrounds */
        BattleGroundSet::iterator GetBattleGroundsBegin() { return m_BattleGrounds.begin(); };
        BattleGroundSet::iterator GetBattleGroundsEnd() { return m_BattleGrounds.end(); };

        BattleGround* GetBattleGround(uint8 ID)
        {
            BattleGroundSet::iterator i = m_BattleGrounds.find(ID);
            if(i != m_BattleGrounds.end())
                return i->second;
            else
                return NULL;
        };

        uint32 CreateBattleGround(uint32 bg_ID, uint32 MaxPlayersPerTeam, uint32 LevelMin, uint32 LevelMax, char* BattleGroundName, uint32 MapID, float Team1StartLocX, float Team1StartLocY, float Team1StartLocZ, float Team1StartLocO, float Team2StartLocX, float Team2StartLocY, float Team2StartLocZ, float Team2StartLocO, uint8 type);

        inline void AddBattleGround(uint32 ID, BattleGround* BG) { m_BattleGrounds[ID] = BG; };

        void CreateInitialBattleGrounds();

        void SendToBattleGround(Player *pl, uint32 bgId);

        void SendAreaSpiritHealerQueryOpcode(Player *pl, BattleGround *bg, uint64 guid);

    private:

        /* Battlegrounds */
        BattleGroundSet m_BattleGrounds;
};

#define sBattleGroundMgr MaNGOS::Singleton<BattleGroundMgr>::Instance()
#endif
