/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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
#include "BattleGroundEY.h"
#include "Creature.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"

BattleGroundEY::BattleGroundEY()
{
    m_BgObjects.resize(BG_EY_OBJECT_MAX);
    m_BgCreatures.resize(BG_EY_CREATURES_MAX);
    m_Buff_Entry[0] = BG_OBJECTID_SPEEDBUFF_ENTRY;
    m_Buff_Entry[1] = BG_OBJECTID_REGENBUFF_ENTRY;
    m_Buff_Entry[2] = BG_OBJECTID_BERSERKERBUFF_ENTRY;
    m_Points_Trigger[FEL_REALVER] = TR_FEL_REALVER_BUFF;
    m_Points_Trigger[BLOOD_ELF] = TR_BLOOD_ELF_BUFF;
    m_Points_Trigger[DRAENEI_RUINS] = TR_DRAENEI_RUINS_BUFF;
    m_Points_Trigger[MAGE_TOWER] = TR_MAGE_TOWER_BUFF;
}

BattleGroundEY::~BattleGroundEY()
{
}

void BattleGroundEY::Update(time_t diff)
{
    BattleGround::Update(diff);
    // after bg start we get there (once)
    if (GetStatus() == STATUS_WAIT_JOIN && GetPlayersSize())
    {
        ModifyStartDelayTime(diff);

        if(!(m_Events & 0x01))
        {
            m_Events |= 0x01;

            SpawnBGObject(BG_EY_OBJECT_DOOR_A, RESPAWN_IMMEDIATELY);
            SpawnBGObject(BG_EY_OBJECT_DOOR_H, RESPAWN_IMMEDIATELY);

            for(uint32 i = BG_EY_OBJECT_A_BANNER_FEL_REALVER_CENTER; i <= BG_EY_OBJECT_FLAG_MAGE_TOWER; ++i)
                SpawnBGObject(i, RESPAWN_ONE_DAY);

            SetStartDelayTime(START_DELAY0);
        }
        // After 1 minute, warning is signalled
        else if(GetStartDelayTime() <= START_DELAY1 && !(m_Events & 0x04))
        {
            m_Events |= 0x04;
            SendMessageToAll(GetMangosString(LANG_BG_EY_ONE_MINUTE));
        }
        // After 1,5 minute, warning is signalled
        else if(GetStartDelayTime() <= START_DELAY2 && !(m_Events & 0x08))
        {
            m_Events |= 0x08;
            SendMessageToAll(GetMangosString(LANG_BG_EY_HALF_MINUTE));
        }
        // After 2 minutes, gates OPEN ! x)
        else if(GetStartDelayTime() < 0 && !(m_Events & 0x10))
        {
            m_Events |= 0x10;
            SpawnBGObject(BG_EY_OBJECT_DOOR_A, RESPAWN_ONE_DAY);
            SpawnBGObject(BG_EY_OBJECT_DOOR_H, RESPAWN_ONE_DAY);

            for(uint32 i = BG_EY_OBJECT_N_BANNER_FEL_REALVER_CENTER; i <= BG_EY_OBJECT_FLAG_NETHERSTORM; ++i)
                SpawnBGObject(i, RESPAWN_IMMEDIATELY);

            SendMessageToAll(GetMangosString(LANG_BG_EY_BEGIN));

            PlaySoundToAll(SOUND_BG_START);
            SetStatus(STATUS_IN_PROGRESS);

            for(std::map<uint64, BattleGroundPlayer>::const_iterator itr = GetPlayers()->begin(); itr != GetPlayers()->end(); ++itr)
            {
                Player *plr = objmgr.GetPlayer(itr->first);
                if(plr)
                    plr->RemoveAurasDueToSpell(SPELL_ARENA_PREPARATION);
            }
        }
    }
    else if(GetStatus() == STATUS_IN_PROGRESS)
    {
        m_PointAddingTimer -= diff;
        if(m_PointAddingTimer <= 0)
        {
            m_PointAddingTimer = BG_EY_FPOINTS_TICK_TIME;
            if (m_TeamPointsCount[BG_TEAM_ALLIANCE] > 0)
                AddPoints(ALLIANCE, BG_EY_TickPoints[m_TeamPointsCount[BG_TEAM_ALLIANCE] - 1]);
            if (m_TeamPointsCount[BG_TEAM_HORDE] > 0)
                AddPoints(HORDE, BG_EY_TickPoints[m_TeamPointsCount[BG_TEAM_HORDE] - 1]);
        }

        if(m_FlagState == BG_EY_FLAG_STATE_WAIT_RESPAWN || m_FlagState == BG_EY_FLAG_STATE_ON_GROUND)
        {
            m_FlagsTimer -= diff;

            if(m_FlagsTimer < 0)
            {
                m_FlagsTimer = 0;
                if (m_FlagState == BG_EY_FLAG_STATE_WAIT_RESPAWN)
                    RespawnFlag(true);
                else
                    RespawnFlagAfterDrop();
            }
        }

        m_TowerCapCheckTimer -= diff;
        if(m_TowerCapCheckTimer <= 0)
        {
            //check if player joined point
            /*I used this order of calls, because although we will check if one player is in gameobject's distance 2 times
              but we can count of players on current point in CheckSomeoneLeftPoint
            */
            this->CheckSomeoneJoinedPoint();
            //check if player left point
            this->CheckSomeoneLeftPoint();
            this->UpdatePointStatuses();
            m_TowerCapCheckTimer = BG_EY_FPOINTS_TICK_TIME;
        }
    }
}

void BattleGroundEY::AddPoints(uint32 Team, uint32 Points)
{
    uint8 team_index = GetTeamIndexByTeamId(Team);
    m_TeamScores[team_index] += Points;
    m_HonorScoreTics[team_index] += Points;
    if (m_HonorScoreTics[team_index] >= BG_HONOR_SCORE_TICKS)
    {
        RewardHonorToTeam(20, Team);
        m_HonorScoreTics[team_index] -= BG_HONOR_SCORE_TICKS;
    }
    UpdateTeamScore(Team);
}

void BattleGroundEY::CheckSomeoneJoinedPoint()
{
    GameObject *obj = NULL;
    for (uint8 i = 0; i < EY_POINTS_MAX; ++i)
    {
        obj = HashMapHolder<GameObject>::Find(m_BgObjects[BG_EY_OBJECT_TOWER_CAP_FEL_REALVER + i]);
        if (obj)
        {
            uint8 j = 0;
            while (j < m_PlayersNearPoint[EY_POINTS_MAX].size())
            {
                Player *plr = objmgr.GetPlayer(m_PlayersNearPoint[EY_POINTS_MAX][j]);
                if(!plr)
                {
                    sLog.outError("BattleGroundEY: Player " I64FMTD " not found!", m_PlayersNearPoint[EY_POINTS_MAX][j]);
                    ++j;
                    continue;
                }
                if (plr->isAlive() && plr->IsWithinDistInMap(obj, BG_EY_POINT_RADIUS))
                {
                    //player joined point!
                    //show progress bar
                    UpdateWorldStateForPlayer(PROGRESS_BAR_PERCENT_GREY, BG_EY_PROGRESS_BAR_PERCENT_GREY, plr);
                    UpdateWorldStateForPlayer(PROGRESS_BAR_STATUS, m_PointBarStatus[i], plr);
                    UpdateWorldStateForPlayer(PROGRESS_BAR_SHOW, BG_EY_PROGRESS_BAR_SHOW, plr);
                    //add player to point
                    m_PlayersNearPoint[i].push_back(m_PlayersNearPoint[EY_POINTS_MAX][j]);
                    //remove player from "free space"
                    m_PlayersNearPoint[EY_POINTS_MAX].erase(m_PlayersNearPoint[EY_POINTS_MAX].begin() + j);
                }
                else
                    ++j;
            }
        }
    }    
}

void BattleGroundEY::CheckSomeoneLeftPoint()
{
    //reset current point counts
    for (uint8 i = 0; i < 2*EY_POINTS_MAX; ++i)
        m_CurrentPointPlayersCount[i] = 0;
    GameObject *obj = NULL;
    for(uint8 i = 0; i < EY_POINTS_MAX; ++i)
    {
        obj = HashMapHolder<GameObject>::Find(m_BgObjects[BG_EY_OBJECT_TOWER_CAP_FEL_REALVER + i]);
        if(obj)
        {
            uint8 j = 0;
            while (j < m_PlayersNearPoint[i].size())
            {
                Player *plr = objmgr.GetPlayer(m_PlayersNearPoint[i][j]);
                if (!plr)
                {
                    sLog.outError("BattleGroundEY: Player " I64FMTD " not found!", m_PlayersNearPoint[i][j]);
                    //move not existed player to "free space" - this will cause many error showing in log, but it is a very important bug
                    m_PlayersNearPoint[EY_POINTS_MAX].push_back(m_PlayersNearPoint[i][j]);
                    m_PlayersNearPoint[i].erase(m_PlayersNearPoint[i].begin() + j);
                    ++j;
                    continue;
                }
                if (!plr->isAlive() || !plr->IsWithinDistInMap(obj, BG_EY_POINT_RADIUS))
                    //move player out of point (add him to players that are out of points
                {
                    m_PlayersNearPoint[EY_POINTS_MAX].push_back(m_PlayersNearPoint[i][j]);
                    m_PlayersNearPoint[i].erase(m_PlayersNearPoint[i].begin() + j);                    
                    this->UpdateWorldStateForPlayer(PROGRESS_BAR_SHOW, BG_EY_PROGRESS_BAR_DONT_SHOW, plr);
                }
                else
                {
                    //player is neat flag, so update count:
                    m_CurrentPointPlayersCount[2 * i + GetTeamIndexByTeamId(plr->GetTeam())]++;
                    ++j;
                }
            }
        }
    }
}

void BattleGroundEY::UpdatePointStatuses()
{
    for(uint8 point = 0; point < EY_POINTS_MAX; ++point)
    {
        if (m_PlayersNearPoint[point].empty())
            continue;
        //count new point bar status:
        m_PointBarStatus[point] += (m_CurrentPointPlayersCount[2 * point] - m_CurrentPointPlayersCount[2 * point + 1] < BG_EY_POINT_MAX_CAPTURERS_COUNT) ? m_CurrentPointPlayersCount[2 * point] - m_CurrentPointPlayersCount[2 * point + 1] : BG_EY_POINT_MAX_CAPTURERS_COUNT;
        
        if (m_PointBarStatus[point] > BG_EY_PROGRESS_BAR_ALI_CONTROLLED)
            //point is fully alliance's
            m_PointBarStatus[point] = BG_EY_PROGRESS_BAR_ALI_CONTROLLED;
        if (m_PointBarStatus[point] < BG_EY_PROGRESS_BAR_HORDE_CONTROLLED)
            //point is fully horde's
            m_PointBarStatus[point] = BG_EY_PROGRESS_BAR_HORDE_CONTROLLED;

        uint32 pointOwnerTeamId = 0;
        //find which team should own this point
        if (m_PointBarStatus[point] <= BG_EY_PROGRESS_BAR_NEUTRAL_LOW)
            pointOwnerTeamId = HORDE;
        else if (m_PointBarStatus[point] >= BG_EY_PROGRESS_BAR_NEUTRAL_HIGH)
            pointOwnerTeamId = ALLIANCE;
        else
            pointOwnerTeamId = EY_POINT_NO_OWNER;

        for (uint8 i = 0; i < m_PlayersNearPoint[point].size(); ++i)
        {
            Player *plr = objmgr.GetPlayer(m_PlayersNearPoint[point][i]);
            if (plr)
            {
                this->UpdateWorldStateForPlayer(PROGRESS_BAR_STATUS, m_PointBarStatus[point], plr);
                if (pointOwnerTeamId != m_PointOwnedByTeam[point]) //if point owner changed we must evoke event!
                {
                    //point was uncontrolled and player is from team which captured point
                    if (m_PointState[point] == EY_POINT_STATE_UNCONTROLLED && plr->GetTeam() == pointOwnerTeamId)
                        this->EventTeamCapturedPoint(plr, point);
                    
                    //point was under control and player isn't from team which controlled it
                    if (m_PointState[point] == EY_POINT_UNDER_CONTROL && plr->GetTeam() != m_PointOwnedByTeam[point])
                        this->EventTeamLostPoint(plr, point);
                }
            }
        }
    }
}

void BattleGroundEY::UpdateTeamScore(uint32 Team)
{
    uint32 score = GetTeamScore(Team);
    if(score >= EY_MAX_TEAM_SCORE)
    {
        score = EY_MAX_TEAM_SCORE;
        EndBattleGround(Team);
    }

    if(Team == ALLIANCE)
        UpdateWorldState(EY_ALLIANCE_RESOURCES, score);
    else
        UpdateWorldState(EY_HORDE_RESOURCES, score);
}

void BattleGroundEY::UpdatePointsCount(uint32 Team)
{
    if(Team == ALLIANCE)
        UpdateWorldState(EY_ALLIANCE_BASE, m_TeamPointsCount[BG_TEAM_ALLIANCE]);
    else
        UpdateWorldState(EY_HORDE_BASE, m_TeamPointsCount[BG_TEAM_HORDE]);
}

void BattleGroundEY::UpdatePointsIcons(uint32 Team, uint32 Point)
{
    //we MUST firstly send 0, after that we can send 1!!!
    if (m_PointState[Point] == EY_POINT_UNDER_CONTROL)
    {
        UpdateWorldState(m_PointsIconStruct[Point].WorldStateControlIndex, 0);
        if(Team == ALLIANCE)
            UpdateWorldState(m_PointsIconStruct[Point].WorldStateAllianceControlledIndex, 1);
        else
            UpdateWorldState(m_PointsIconStruct[Point].WorldStateHordeControlledIndex, 1);
    }
    else
    {
        if(Team == ALLIANCE)
            UpdateWorldState(m_PointsIconStruct[Point].WorldStateAllianceControlledIndex, 0);
        else
            UpdateWorldState(m_PointsIconStruct[Point].WorldStateHordeControlledIndex, 0);
        UpdateWorldState(m_PointsIconStruct[Point].WorldStateControlIndex, 1);
    }
}

void BattleGroundEY::AddPlayer(Player *plr)
{
    BattleGround::AddPlayer(plr);
    //create score and add it to map
    BattleGroundEYScore* sc = new BattleGroundEYScore;

    m_PlayersNearPoint[EY_POINTS_MAX].push_back(plr->GetGUID());

    m_PlayerScores[plr->GetGUID()] = sc;
}

void BattleGroundEY::RemovePlayer(Player *plr, uint64 guid)
{
    // sometimes flag aura not removed :(
    for (int j = EY_POINTS_MAX; j >= 0; --j)
    {
        for(int i = 0; i < m_PlayersNearPoint[j].size(); ++i)
            if(m_PlayersNearPoint[j][i] == guid)
                m_PlayersNearPoint[j].erase(m_PlayersNearPoint[j].begin() + i);
    }
    if(IsFlagPickedup())
    {
        if(m_FlagKeeper == guid)
        {
            if(plr)
                this->EventPlayerDroppedFlag(plr);
            else
            {
                SetFlagPicker(0);
                RespawnFlag(true);
            }
        }
    }
}

void BattleGroundEY::HandleBuffUse(uint64 const& buff_guid)
{
    if(m_BgObjects[BG_EY_OBJECT_BUFF_FEL_REALVER] == buff_guid)
    {
        DelObject(BG_EY_OBJECT_BUFF_FEL_REALVER);
        SpawnBuff(BG_EY_OBJECT_BUFF_FEL_REALVER,FEL_REALVER,BUFF_RESPAWN_TIME);
    }
    else if(m_BgObjects[BG_EY_OBJECT_BUFF_BLOOD_ELF] == buff_guid)
    {
        DelObject(BG_EY_OBJECT_BUFF_BLOOD_ELF);
        SpawnBuff(BG_EY_OBJECT_BUFF_BLOOD_ELF,BLOOD_ELF,BUFF_RESPAWN_TIME);
    }
    else if(m_BgObjects[BG_EY_OBJECT_BUFF_DRAENEI_RUINS] == buff_guid)
    {
        DelObject(BG_EY_OBJECT_BUFF_DRAENEI_RUINS);
        SpawnBuff(BG_EY_OBJECT_BUFF_DRAENEI_RUINS,DRAENEI_RUINS,BUFF_RESPAWN_TIME);
    }
    else if(m_BgObjects[BG_EY_OBJECT_BUFF_MAGE_TOWER] == buff_guid)
    {
        DelObject(BG_EY_OBJECT_BUFF_MAGE_TOWER);
        SpawnBuff(BG_EY_OBJECT_BUFF_MAGE_TOWER,MAGE_TOWER,BUFF_RESPAWN_TIME);
    }
    else
       sLog.outError("unknown buff on bg");
}

void BattleGroundEY::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    if(!Source->isAlive())                                                      //hack code, must be removed later
        return;

    switch(Trigger)
    {
        case TR_BLOOD_ELF_POINT:
            if(m_PointState[BLOOD_ELF] == EY_POINT_UNDER_CONTROL && m_PointOwnedByTeam[BLOOD_ELF] == Source->GetTeam())
                if(m_FlagState && GetFlagPickerGUID() == Source->GetGUID())
                    EventPlayerCapturedFlag(Source, BG_EY_OBJECT_FLAG_BLOOD_ELF);
            break;
        case TR_FEL_REALVER_POINT:
            if(m_PointState[FEL_REALVER] == EY_POINT_UNDER_CONTROL && m_PointOwnedByTeam[FEL_REALVER] == Source->GetTeam())
                if(m_FlagState && GetFlagPickerGUID() == Source->GetGUID())
                    EventPlayerCapturedFlag(Source, BG_EY_OBJECT_FLAG_FEL_REALVER);
            break;
        case TR_MAGE_TOWER_POINT:
            if(m_PointState[MAGE_TOWER] == EY_POINT_UNDER_CONTROL && m_PointOwnedByTeam[MAGE_TOWER] == Source->GetTeam())
                if(m_FlagState && GetFlagPickerGUID() == Source->GetGUID())
                    EventPlayerCapturedFlag(Source, BG_EY_OBJECT_FLAG_MAGE_TOWER);
            break;
        case TR_DRAENEI_RUINS_POINT:
            if(m_PointState[DRAENEI_RUINS] == EY_POINT_UNDER_CONTROL && m_PointOwnedByTeam[DRAENEI_RUINS] == Source->GetTeam())
                if(m_FlagState && GetFlagPickerGUID() == Source->GetGUID())
                    EventPlayerCapturedFlag(Source, BG_EY_OBJECT_FLAG_DRAENEI_RUINS);
            break;
        case 4512:
        case 4515:
        case 4517:
        case 4519:
        case 4530:
        case 4531:
        case 4568:
        case 4569:
        case 4570:
        case 4571:
            break;
        default:
            sLog.outError("WARNING: Unhandled AreaTrigger in Battleground: %u", Trigger);
            Source->GetSession()->SendAreaTriggerMessage("Warning: Unhandled AreaTrigger in Battleground: %u", Trigger);
            break;
    }
}

bool BattleGroundEY::SetupBattleGround()
{
        // doors
    if(    !AddObject(BG_EY_OBJECT_DOOR_A, BG_OBJECT_A_DOOR_EY_ENTRY, 2527.6, 1596.91, 1262.13, -3.12414, -0.173642, -0.001515, 0.98477, -0.008594, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_EY_OBJECT_DOOR_H, BG_OBJECT_H_DOOR_EY_ENTRY, 1803.21, 1539.49, 1261.09, 3.14159, 0.173648, 0, 0.984808, 0, RESPAWN_IMMEDIATELY)
        // banners (alliance)
        || !AddObject(BG_EY_OBJECT_A_BANNER_FEL_REALVER_CENTER, BG_OBJECT_A_BANNER_EY_ENTRY, 2057.46, 1735.07, 1187.91, -0.925024, 0, 0, 0.446198, -0.894934, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_FEL_REALVER_LEFT, BG_OBJECT_A_BANNER_EY_ENTRY, 2032.25, 1729.53, 1190.33, 1.8675, 0, 0, 0.803857, 0.594823, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_FEL_REALVER_RIGHT, BG_OBJECT_A_BANNER_EY_ENTRY, 2092.35, 1775.46, 1187.08, -0.401426, 0, 0, 0.199368, -0.979925, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_BLOOD_ELF_CENTER, BG_OBJECT_A_BANNER_EY_ENTRY, 2047.19, 1349.19, 1189, -1.62316, 0, 0, 0.725374, -0.688354, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_BLOOD_ELF_LEFT, BG_OBJECT_A_BANNER_EY_ENTRY, 2074.32, 1385.78, 1194.72, 0.488692, 0, 0, 0.241922, 0.970296, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_BLOOD_ELF_RIGHT, BG_OBJECT_A_BANNER_EY_ENTRY, 2025.13, 1386.12, 1192.74, 2.3911, 0, 0, 0.930418, 0.366501, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_DRAENEI_RUINS_CENTER, BG_OBJECT_A_BANNER_EY_ENTRY, 2276.8, 1400.41, 1196.33, 2.44346, 0, 0, 0.939693, 0.34202, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_DRAENEI_RUINS_LEFT, BG_OBJECT_A_BANNER_EY_ENTRY, 2305.78, 1404.56, 1199.38, 1.74533, 0, 0, 0.766044, 0.642788, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_DRAENEI_RUINS_RIGHT, BG_OBJECT_A_BANNER_EY_ENTRY, 2245.4, 1366.41, 1195.28, 2.21657, 0, 0, 0.894934, 0.446198, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_MAGE_TOWER_CENTER, BG_OBJECT_A_BANNER_EY_ENTRY, 2270.84, 1784.08, 1186.76, 2.42601, 0, 0, 0.936672, 0.350207, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_MAGE_TOWER_LEFT, BG_OBJECT_A_BANNER_EY_ENTRY, 2269.13, 1737.7, 1186.66, 0.994838, 0, 0, 0.477159, 0.878817, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_MAGE_TOWER_RIGHT, BG_OBJECT_A_BANNER_EY_ENTRY, 2300.86, 1741.25, 1187.7, -0.785398, 0, 0, 0.382683, -0.92388, RESPAWN_ONE_DAY)
        // banners (horde)
        || !AddObject(BG_EY_OBJECT_H_BANNER_FEL_REALVER_CENTER, BG_OBJECT_H_BANNER_EY_ENTRY, 2057.46, 1735.07, 1187.91, -0.925024, 0, 0, 0.446198, -0.894934, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_FEL_REALVER_LEFT, BG_OBJECT_H_BANNER_EY_ENTRY, 2032.25, 1729.53, 1190.33, 1.8675, 0, 0, 0.803857, 0.594823, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_FEL_REALVER_RIGHT, BG_OBJECT_H_BANNER_EY_ENTRY, 2092.35, 1775.46, 1187.08, -0.401426, 0, 0, 0.199368, -0.979925, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_BLOOD_ELF_CENTER, BG_OBJECT_H_BANNER_EY_ENTRY, 2047.19, 1349.19, 1189, -1.62316, 0, 0, 0.725374, -0.688354, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_BLOOD_ELF_LEFT, BG_OBJECT_H_BANNER_EY_ENTRY, 2074.32, 1385.78, 1194.72, 0.488692, 0, 0, 0.241922, 0.970296, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_BLOOD_ELF_RIGHT, BG_OBJECT_H_BANNER_EY_ENTRY, 2025.13, 1386.12, 1192.74, 2.3911, 0, 0, 0.930418, 0.366501, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_DRAENEI_RUINS_CENTER, BG_OBJECT_H_BANNER_EY_ENTRY, 2276.8, 1400.41, 1196.33, 2.44346, 0, 0, 0.939693, 0.34202, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_DRAENEI_RUINS_LEFT, BG_OBJECT_H_BANNER_EY_ENTRY, 2305.78, 1404.56, 1199.38, 1.74533, 0, 0, 0.766044, 0.642788, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_DRAENEI_RUINS_RIGHT, BG_OBJECT_H_BANNER_EY_ENTRY, 2245.4, 1366.41, 1195.28, 2.21657, 0, 0, 0.894934, 0.446198, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_MAGE_TOWER_CENTER, BG_OBJECT_H_BANNER_EY_ENTRY, 2270.84, 1784.08, 1186.76, 2.42601, 0, 0, 0.936672, 0.350207, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_MAGE_TOWER_LEFT, BG_OBJECT_H_BANNER_EY_ENTRY, 2269.13, 1737.7, 1186.66, 0.994838, 0, 0, 0.477159, 0.878817, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_MAGE_TOWER_RIGHT, BG_OBJECT_H_BANNER_EY_ENTRY, 2300.86, 1741.25, 1187.7, -0.785398, 0, 0, 0.382683, -0.92388, RESPAWN_ONE_DAY)
        // banners (natural)
        || !AddObject(BG_EY_OBJECT_N_BANNER_FEL_REALVER_CENTER, BG_OBJECT_N_BANNER_EY_ENTRY, 2057.46, 1735.07, 1187.91, -0.925024, 0, 0, 0.446198, -0.894934, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_FEL_REALVER_LEFT, BG_OBJECT_N_BANNER_EY_ENTRY, 2032.25, 1729.53, 1190.33, 1.8675, 0, 0, 0.803857, 0.594823, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_FEL_REALVER_RIGHT, BG_OBJECT_N_BANNER_EY_ENTRY, 2092.35, 1775.46, 1187.08, -0.401426, 0, 0, 0.199368, -0.979925, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_BLOOD_ELF_CENTER, BG_OBJECT_N_BANNER_EY_ENTRY, 2047.19, 1349.19, 1189, -1.62316, 0, 0, 0.725374, -0.688354, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_BLOOD_ELF_LEFT, BG_OBJECT_N_BANNER_EY_ENTRY, 2074.32, 1385.78, 1194.72, 0.488692, 0, 0, 0.241922, 0.970296, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_BLOOD_ELF_RIGHT, BG_OBJECT_N_BANNER_EY_ENTRY, 2025.13, 1386.12, 1192.74, 2.3911, 0, 0, 0.930418, 0.366501, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_DRAENEI_RUINS_CENTER, BG_OBJECT_N_BANNER_EY_ENTRY, 2276.8, 1400.41, 1196.33, 2.44346, 0, 0, 0.939693, 0.34202, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_DRAENEI_RUINS_LEFT, BG_OBJECT_N_BANNER_EY_ENTRY, 2305.78, 1404.56, 1199.38, 1.74533, 0, 0, 0.766044, 0.642788, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_DRAENEI_RUINS_RIGHT, BG_OBJECT_N_BANNER_EY_ENTRY, 2245.4, 1366.41, 1195.28, 2.21657, 0, 0, 0.894934, 0.446198, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_MAGE_TOWER_CENTER, BG_OBJECT_N_BANNER_EY_ENTRY, 2270.84, 1784.08, 1186.76, 2.42601, 0, 0, 0.936672, 0.350207, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_MAGE_TOWER_LEFT, BG_OBJECT_N_BANNER_EY_ENTRY, 2269.13, 1737.7, 1186.66, 0.994838, 0, 0, 0.477159, 0.878817, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_MAGE_TOWER_RIGHT, BG_OBJECT_N_BANNER_EY_ENTRY, 2300.86, 1741.25, 1187.7, -0.785398, 0, 0, 0.382683, -0.92388, RESPAWN_ONE_DAY)
        // flags
        || !AddObject(BG_EY_OBJECT_FLAG_NETHERSTORM, BG_OBJECT_FLAG2_EY_ENTRY, 2174.782227, 1569.054688, 1160.361938, -1.448624, 0, 0, 0.662620, -0.748956, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_FLAG_FEL_REALVER, BG_OBJECT_FLAG1_EY_ENTRY, 2044.28, 1729.68, 1189.96, -0.017453, 0, 0, 0.008727, -0.999962, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_FLAG_BLOOD_ELF, BG_OBJECT_FLAG1_EY_ENTRY, 2048.83, 1393.65, 1194.49, 0.20944, 0, 0, 0.104528, 0.994522, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_FLAG_DRAENEI_RUINS, BG_OBJECT_FLAG1_EY_ENTRY, 2286.56, 1402.36, 1197.11, 3.72381, 0, 0, 0.957926, -0.287016, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_FLAG_MAGE_TOWER, BG_OBJECT_FLAG1_EY_ENTRY, 2284.48, 1731.23, 1189.99, 2.89725, 0, 0, 0.992546, 0.121869, RESPAWN_ONE_DAY)
        // tower cap
        || !AddObject(BG_EY_OBJECT_TOWER_CAP_FEL_REALVER, BG_OBJECT_FR_TOWER_CAP_EY_ENTRY, 2024.600708, 1742.819580, 1195.157715, 2.443461, 0, 0, 0.939693, 0.342020, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_TOWER_CAP_BLOOD_ELF, BG_OBJECT_BE_TOWER_CAP_EY_ENTRY, 2050.493164, 1372.235962, 1194.563477, 1.710423, 0, 0, 0.754710, 0.656059, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_TOWER_CAP_DRAENEI_RUINS, BG_OBJECT_DR_TOWER_CAP_EY_ENTRY, 2301.010498, 1386.931641, 1197.183472, 1.570796, 0, 0, 0.707107, 0.707107, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_TOWER_CAP_MAGE_TOWER, BG_OBJECT_HU_TOWER_CAP_EY_ENTRY, 2282.121582, 1760.006958, 1189.707153, 1.919862, 0, 0, 0.819152, 0.573576, RESPAWN_ONE_DAY)
        )
    {
        sLog.outErrorDb("BatteGroundAB: Failed to spawn some object BattleGround not created!");
        return false;
    }

    WorldSafeLocsEntry const *sg = NULL;
    sg = sWorldSafeLocsStore.LookupEntry(EY_GRAVEYARD_MAIN_ALLIANCE);
    if(!sg || !AddSpiritGuide(EY_SPIRIT_MAIN_ALLIANCE, sg->x, sg->y, sg->z, 3.124139, ALLIANCE))
    {
        sLog.outErrorDb("Failed to spawn spirit guide! BattleGround not created!");
        return false;
    }

    sg = sWorldSafeLocsStore.LookupEntry(EY_GRAVEYARD_MAIN_HORDE);
    if(!sg || !AddSpiritGuide(EY_SPIRIT_MAIN_HORDE, sg->x, sg->y, sg->z, 3.193953, HORDE))
    {
        sLog.outErrorDb("Failed to spawn spirit guide! BattleGround not created!");
        return false;
    }

    return true;
}

void BattleGroundEY::ResetBGSubclass()
{
    m_TeamScores[BG_TEAM_ALLIANCE] = 0;
    m_TeamScores[BG_TEAM_HORDE] = 0;
    m_TeamPointsCount[BG_TEAM_ALLIANCE] = 0;
    m_TeamPointsCount[BG_TEAM_HORDE] = 0;
    m_HonorScoreTics[BG_TEAM_ALLIANCE] = 0;
    m_HonorScoreTics[BG_TEAM_HORDE] = 0;
    m_FlagState = BG_EY_FLAG_STATE_ON_BASE;
    m_FlagCapturedBgObjectType = 0;
    m_FlagKeeper = 0;
    m_DroppedFlagGUID = 0;
    m_PointAddingTimer = 0;
    m_TowerCapCheckTimer = 0;
    
    for(uint8 i = 0; i < EY_POINTS_MAX; ++i)
    {
        m_PointOwnedByTeam[i] = EY_POINT_NO_OWNER;
        m_PointState[i] = EY_POINT_STATE_UNCONTROLLED;
        m_PointBarStatus[i] = BG_EY_PROGRESS_BAR_STATE_MIDDLE;
        m_PlayersNearPoint[i].clear();
        m_PlayersNearPoint[i].reserve(15);                   //tip size
    }
    m_PlayersNearPoint[EY_PLAYERS_OUT_OF_POINTS].clear();
    m_PlayersNearPoint[EY_PLAYERS_OUT_OF_POINTS].reserve(30);
}

bool BattleGroundEY::SpawnBuff(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 spawntime)
{
    GameObjectInfo const* goinfo = objmgr.GetGameObjectInfo(entry);
    if(!goinfo)
    {
        sLog.outErrorDb("Gameobject template %u not found in database! BattleGround not created!", entry);
        return false;
    }

    uint32 guid = objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT);

    GameObject* pGameObj = new GameObject(NULL);
    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), entry, GetMapId(), x, y, z, o, rotation0, rotation1, rotation2, rotation3, 0, 1))
    {
        sLog.outError("Can't create creature entry: %u",entry);
        delete pGameObj;
        return false;
    }
    pGameObj->SetMapId(GetMapId());
    //pGameObj->SetInstanceId(GetInstanceID());
    pGameObj->SetRespawnTime(spawntime);
    if(spawntime)
        pGameObj->SetLootState(GO_JUST_DEACTIVATED);
    MapManager::Instance().GetMap(pGameObj->GetMapId(), pGameObj)->Add(pGameObj);
    m_BgObjects[type] = pGameObj->GetGUID();

    return true;
}

void BattleGroundEY::SpawnBuff(uint32 type, uint8 point, uint32 respawntime)
{
    AreaTriggerEntry const* at = sAreaTriggerStore.LookupEntry(m_Points_Trigger[point]);
    if(!at)
    {
        sLog.outError("unknown trigger: %u",m_Points_Trigger[point]);
        return;
    }
    uint8 buff = urand(0, 2);
    if(!SpawnBuff(type, m_Buff_Entry[buff], at->x, at->y, at->z, 0.907571, 0, 0, 0.438371, 0.898794, respawntime))
    {
        sLog.outError("Can't create SpawnBuff");
        return;
    }
}

void BattleGroundEY::RespawnFlag(bool send_message)
{
    if (m_FlagCapturedBgObjectType > 0)
        SpawnBGObject(m_FlagCapturedBgObjectType, RESPAWN_ONE_DAY);

    m_FlagCapturedBgObjectType = 0;
    m_FlagState = BG_EY_FLAG_STATE_ON_BASE;
    SpawnBGObject(BG_EY_OBJECT_FLAG_NETHERSTORM, RESPAWN_IMMEDIATELY);

    if(send_message)
    {
        SendMessageToAll(GetMangosString(LANG_BG_EY_RESETED_FLAG));
        PlaySoundToAll(BG_EY_SOUND_FLAG_RESET);            // flags respawned sound...
    }

    UpdateWorldState(NETHERSTORM_FLAG, 1);
}

void BattleGroundEY::RespawnFlagAfterDrop()
{
    RespawnFlag(true);

    GameObject *obj = HashMapHolder<GameObject>::Find(GetDroppedFlagGUID());
    if(obj)
        obj->Delete();
    else
        sLog.outError("BattleGroundEY: Unknown dropped flag guid: %u",GetDroppedFlagGUID());

    SetDroppedFlagGUID(0);
}

void BattleGroundEY::HandleKillPlayer(Player *player, Player *killer)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    BattleGround::HandleKillPlayer(player, killer);
    EventPlayerDroppedFlag(player);
}

void BattleGroundEY::EventPlayerDroppedFlag(Player *Source)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    const char *message = "";
    uint8 type = 0;

    if(IsFlagPickedup())
    {
        if(GetFlagPickerGUID() == Source->GetGUID())
        {
            SetFlagPicker(0);
            Source->RemoveAurasDueToSpell(BG_EY_NETHERSTORM_FLAG_SPELL);
            m_FlagState = BG_EY_FLAG_STATE_ON_GROUND;
            m_FlagsTimer = BG_EY_FLAG_RESPAWN_TIME;
            Source->CastSpell(Source, BG_EY_PLAYER_CANNOT_PICK_FLAG, true);
            Source->CastSpell(Source, BG_EY_PLAYER_DROPPED_FLAG_SPELL, true);
            if(Source->GetTeam() == ALLIANCE)
            {
                message = GetMangosString(LANG_BG_EY_DROPPED_FLAG);
                type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
            }
            else
            {
                message = GetMangosString(LANG_BG_EY_DROPPED_FLAG);
                type = CHAT_MSG_BG_SYSTEM_HORDE;
            }
            //this does not work correctly :( (it should remove flag carrier name)
            UpdateWorldState(NETHERSTORM_FLAG_STATE_HORDE, BG_EY_FLAG_STATE_WAIT_RESPAWN);
            UpdateWorldState(NETHERSTORM_FLAG_STATE_ALLIANCE, BG_EY_FLAG_STATE_WAIT_RESPAWN);

            WorldPacket data;
            ChatHandler::FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
            SendPacketToAll(&data);
        }
    }
}

void BattleGroundEY::EventPlayerClickedOnFlag(Player *Source, GameObject* target_obj)
{
    if(GetStatus() != STATUS_IN_PROGRESS || this->IsFlagPickedup() || !Source->IsWithinDistInMap(target_obj, 10))
        return;

    const char *message;
    uint8 type = 0;
    message = GetMangosString(LANG_BG_EY_HAS_TAKEN_FLAG);
    
    if(Source->GetTeam() == ALLIANCE)
    {
        UpdateWorldState(NETHERSTORM_FLAG_STATE_ALLIANCE, BG_EY_FLAG_STATE_ON_PLAYER);
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        PlaySoundToAll(BG_EY_SOUND_FLAG_PICKED_UP_ALLIANCE);
    }
    else
    {
        UpdateWorldState(NETHERSTORM_FLAG_STATE_HORDE, BG_EY_FLAG_STATE_ON_PLAYER);
        type = CHAT_MSG_BG_SYSTEM_HORDE;
        PlaySoundToAll(BG_EY_SOUND_FLAG_PICKED_UP_HORDE);
    }
    
    if (m_FlagState == BG_EY_FLAG_STATE_ON_BASE)
        UpdateWorldState(NETHERSTORM_FLAG, 0);
    m_FlagState = BG_EY_FLAG_STATE_ON_PLAYER;

    SpawnBGObject(BG_EY_OBJECT_FLAG_NETHERSTORM, RESPAWN_ONE_DAY);
    SetFlagPicker(Source->GetGUID());
    //get flag aura on player
    Source->CastSpell(Source, 34976, true);

    WorldPacket data;
    ChatHandler::FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);
}

void BattleGroundEY::EventTeamLostPoint(Player *Source, uint32 Point)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    //Natural point
    uint8 message_type = 0;
    const char *message = "";
    uint32 Team = m_PointOwnedByTeam[Point];

    if(!Team)
        return;

    if (Team == ALLIANCE)
    {
        m_TeamPointsCount[BG_TEAM_ALLIANCE]--;
        message_type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        message = GetMangosString(m_LoosingPointTypes[Point].MessageIdAlliance);
        SpawnBGObject(m_LoosingPointTypes[Point].DespawnObjectTypeAlliance, RESPAWN_ONE_DAY);
        SpawnBGObject(m_LoosingPointTypes[Point].DespawnObjectTypeAlliance + 1, RESPAWN_ONE_DAY);
        SpawnBGObject(m_LoosingPointTypes[Point].DespawnObjectTypeAlliance + 2, RESPAWN_ONE_DAY);
    }
    else
    {
        m_TeamPointsCount[BG_TEAM_HORDE]--;
        message_type = CHAT_MSG_BG_SYSTEM_HORDE;
        message = GetMangosString(m_LoosingPointTypes[Point].MessageIdHorde);
        SpawnBGObject(m_LoosingPointTypes[Point].DespawnObjectTypeHorde, RESPAWN_ONE_DAY);
        SpawnBGObject(m_LoosingPointTypes[Point].DespawnObjectTypeHorde + 1, RESPAWN_ONE_DAY);
        SpawnBGObject(m_LoosingPointTypes[Point].DespawnObjectTypeHorde + 2, RESPAWN_ONE_DAY);
    }
    
    SpawnBGObject(m_LoosingPointTypes[Point].SpawnNeutralObjectType, RESPAWN_IMMEDIATELY);
    SpawnBGObject(m_LoosingPointTypes[Point].SpawnNeutralObjectType + 1, RESPAWN_IMMEDIATELY);
    SpawnBGObject(m_LoosingPointTypes[Point].SpawnNeutralObjectType + 2, RESPAWN_IMMEDIATELY);

    uint32 type = BG_EY_OBJECT_BUFF_FEL_REALVER + Point;
    //buff despawn
    if(m_BgObjects[type])
        SpawnBGObject(type, RESPAWN_ONE_DAY);

    m_PointOwnedByTeam[Point] = EY_POINT_NO_OWNER;
    m_PointState[Point] = EY_POINT_NO_OWNER;

    WorldPacket data;
    ChatHandler::FillMessageData(&data, Source->GetSession(), message_type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    UpdatePointsIcons(Team, Point);
    UpdatePointsCount(Team);
}

void BattleGroundEY::EventTeamCapturedPoint(Player *Source, uint32 Point)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint8 type = 0;
    const char *message = "";
    uint32 Team = Source->GetTeam();

    SpawnBGObject(m_CapturingPointTypes[Point].DespawnNeutralObjectType, RESPAWN_ONE_DAY);
    SpawnBGObject(m_CapturingPointTypes[Point].DespawnNeutralObjectType + 1, RESPAWN_ONE_DAY);
    SpawnBGObject(m_CapturingPointTypes[Point].DespawnNeutralObjectType + 2, RESPAWN_ONE_DAY);

    if (Team == ALLIANCE)
    {
        m_TeamPointsCount[BG_TEAM_ALLIANCE]++;
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        message = GetMangosString(m_CapturingPointTypes[Point].MessageIdAlliance);
        SpawnBGObject(m_CapturingPointTypes[Point].SpawnObjectTypeAlliance, RESPAWN_IMMEDIATELY);
        SpawnBGObject(m_CapturingPointTypes[Point].SpawnObjectTypeAlliance + 1, RESPAWN_IMMEDIATELY);
        SpawnBGObject(m_CapturingPointTypes[Point].SpawnObjectTypeAlliance + 2, RESPAWN_IMMEDIATELY);
    }
    else
    {
        m_TeamPointsCount[BG_TEAM_HORDE]++;
        type = CHAT_MSG_BG_SYSTEM_HORDE;
        message = GetMangosString(m_CapturingPointTypes[Point].MessageIdHorde);
        SpawnBGObject(m_CapturingPointTypes[Point].SpawnObjectTypeHorde, RESPAWN_IMMEDIATELY);
        SpawnBGObject(m_CapturingPointTypes[Point].SpawnObjectTypeHorde + 1, RESPAWN_IMMEDIATELY);
        SpawnBGObject(m_CapturingPointTypes[Point].SpawnObjectTypeHorde + 2, RESPAWN_IMMEDIATELY);
    }

    m_PointOwnedByTeam[Point] = Team;
    m_PointState[Point] = EY_POINT_UNDER_CONTROL;

    WorldPacket data;
    ChatHandler::FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);


    if(m_BgCreatures[Point])
        DelCreature(Point);

    WorldSafeLocsEntry const *sg = NULL;
    sg = sWorldSafeLocsStore.LookupEntry(m_CapturingPointTypes[Point].GraveYardId);
    if(!sg || !AddSpiritGuide(Point, sg->x, sg->y, sg->z, 3.124139, Team))
        sLog.outError("Failed to spawn spirit guide! point: %u, team: u, graveyard_id: %u", Point, Team, m_CapturingPointTypes[Point].GraveYardId);
    // Spawn buffs
    type = BG_EY_OBJECT_BUFF_FEL_REALVER + Point;
    if(!m_BgObjects[type])
    {
        //buff not spawned, spawn it
        SpawnBuff(type, Point, RESPAWN_IMMEDIATELY);
    }
    else
        //buff spawned ... respawn it
        SpawnBGObject(type, RESPAWN_IMMEDIATELY);

    UpdatePointsIcons(Team, Point);
    UpdatePointsCount(Team);
}

void BattleGroundEY::EventPlayerCapturedFlag(Player *Source, uint32 BgObjectType)
{
    if(GetStatus() != STATUS_IN_PROGRESS || this->GetFlagPickerGUID() != Source->GetGUID())
        return;

    uint8 type = 0;
    uint8 team_id = 0;
    const char *message = "";

    SetFlagPicker(0);
    m_FlagState = BG_EY_FLAG_STATE_WAIT_RESPAWN;
    Source->RemoveAurasDueToSpell(BG_EY_NETHERSTORM_FLAG_SPELL);

    if(Source->GetTeam() == ALLIANCE)
    {
        PlaySoundToAll(BG_EY_SOUND_FLAG_CAPTURED_ALLIANCE);
        team_id = BG_TEAM_ALLIANCE;
        message = GetMangosString(LANG_BG_EY_CAPTURED_FLAG_A);
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
    }
    else
    {
        PlaySoundToAll(BG_EY_SOUND_FLAG_CAPTURED_HORDE);
        team_id = BG_TEAM_HORDE;
        message = GetMangosString(LANG_BG_EY_CAPTURED_FLAG_H);
        type = CHAT_MSG_BG_SYSTEM_HORDE;
    }

    SpawnBGObject(BgObjectType, RESPAWN_IMMEDIATELY);

    m_FlagsTimer = BG_EY_FLAG_RESPAWN_TIME;
    m_FlagCapturedBgObjectType = BgObjectType;

    WorldPacket data;
    ChatHandler::FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    if(m_TeamPointsCount[team_id] > 0)
        AddPoints(Source->GetTeam(), BG_EY_FlagPoints[m_TeamPointsCount[team_id] - 1]);

    UpdatePlayerScore(Source, SCORE_FLAG_CAPTURES, 1);
}

void BattleGroundEY::UpdatePlayerScore(Player *Source, uint32 type, uint32 value)
{
    std::map<uint64, BattleGroundScore*>::iterator itr = m_PlayerScores.find(Source->GetGUID());

    if(itr == m_PlayerScores.end())                         // player not found
        return;

    switch(type)
    {
        case SCORE_FLAG_CAPTURES:                           // flags captured
            ((BattleGroundEYScore*)itr->second)->FlagCaptures += value;
            break;
        default:
            BattleGround::UpdatePlayerScore(Source, type, value);
            break;
    }
}

void BattleGroundEY::FillInitialWorldStates(WorldPacket& data)
{
    data << uint32(EY_HORDE_BASE) << uint32(m_TeamPointsCount[BG_TEAM_HORDE]);
    data << uint32(EY_ALLIANCE_BASE) << uint32(m_TeamPointsCount[BG_TEAM_ALLIANCE]);
    data << uint32(0xab6) << uint32(0x0);
    data << uint32(0xab5) << uint32(0x0);
    data << uint32(0xab4) << uint32(0x0);
    data << uint32(0xab3) << uint32(0x0);
    data << uint32(0xab2) << uint32(0x0);
    data << uint32(0xab1) << uint32(0x0);
    data << uint32(0xab0) << uint32(0x0);
    data << uint32(0xaaf) << uint32(0x0);

    data << uint32(DRAENEI_RUINS_HORDE_CONTROL)     << uint32(m_PointOwnedByTeam[DRAENEI_RUINS] == HORDE && m_PointState[DRAENEI_RUINS] == EY_POINT_UNDER_CONTROL);

    data << uint32(DRAENEI_RUINS_ALLIANCE_CONTROL)  << uint32(m_PointOwnedByTeam[DRAENEI_RUINS] == ALLIANCE && m_PointState[DRAENEI_RUINS] == EY_POINT_UNDER_CONTROL);

    data << uint32(DRAENEI_RUINS_UNCONTROL)         << uint32(m_PointState[DRAENEI_RUINS] != EY_POINT_UNDER_CONTROL);

    data << uint32(MAGE_TOWER_ALLIANCE_CONTROL)     << uint32(m_PointOwnedByTeam[MAGE_TOWER] == ALLIANCE && m_PointState[MAGE_TOWER] == EY_POINT_UNDER_CONTROL);
        
    data << uint32(MAGE_TOWER_HORDE_CONTROL)        << uint32(m_PointOwnedByTeam[MAGE_TOWER] == HORDE && m_PointState[MAGE_TOWER] == EY_POINT_UNDER_CONTROL);
        
    data << uint32(MAGE_TOWER_UNCONTROL)            << uint32(m_PointState[MAGE_TOWER] != EY_POINT_UNDER_CONTROL);

    data << uint32(FEL_REAVER_HORDE_CONTROL)        << uint32(m_PointOwnedByTeam[FEL_REALVER] == HORDE && m_PointState[FEL_REALVER] == EY_POINT_UNDER_CONTROL);

    data << uint32(FEL_REAVER_ALLIANCE_CONTROL)     << uint32(m_PointOwnedByTeam[FEL_REALVER] == ALLIANCE && m_PointState[FEL_REALVER] == EY_POINT_UNDER_CONTROL);

    data << uint32(FEL_REAVER_UNCONTROL)            << uint32(m_PointState[FEL_REALVER] != EY_POINT_UNDER_CONTROL);
        
    data << uint32(BLOOD_ELF_HORDE_CONTROL)         << uint32(m_PointOwnedByTeam[BLOOD_ELF] == HORDE && m_PointState[BLOOD_ELF] == EY_POINT_UNDER_CONTROL);

    data << uint32(BLOOD_ELF_ALLIANCE_CONTROL)      << uint32(m_PointOwnedByTeam[BLOOD_ELF] == ALLIANCE && m_PointState[BLOOD_ELF] == EY_POINT_UNDER_CONTROL);
        
    data << uint32(BLOOD_ELF_UNCONTROL)             << uint32(m_PointState[BLOOD_ELF] != EY_POINT_UNDER_CONTROL);

    data << uint32(NETHERSTORM_FLAG)                << uint32(m_FlagState == BG_EY_FLAG_STATE_ON_BASE);

    data << uint32(0xad2) << uint32(0x1);
    data << uint32(0xad1) << uint32(0x1);
    data << uint32(0xabe) << uint32(GetTeamScore(HORDE));
    data << uint32(0xabd) << uint32(GetTeamScore(ALLIANCE));
    data << uint32(0xa05) << uint32(0x8e);
    data << uint32(0xaa0) << uint32(0x0);
    data << uint32(0xa9f) << uint32(0x0);
    data << uint32(0xa9e) << uint32(0x0);
    data << uint32(0xc0d) << uint32(0x17b);
}

WorldSafeLocsEntry const *BattleGroundEY::GetClosestGraveYard(float x, float y, float z, uint32 MapId, uint32 team)
{
    uint32 g_id = 0;

    if(team == ALLIANCE)
        g_id = EY_GRAVEYARD_MAIN_ALLIANCE;
    else if(team == HORDE)
        g_id = EY_GRAVEYARD_MAIN_HORDE;
    else
        return NULL;

    float distance, nearestDistance;

    WorldSafeLocsEntry const* entry = NULL;
    WorldSafeLocsEntry const* nearestEntry = NULL;
    entry = sWorldSafeLocsStore.LookupEntry(g_id);
    nearestEntry = entry;

    if(!entry)
    {
        sLog.outError("BattleGroundEY: Not found the main team graveyard. Graveyard system isn't working!");
        return NULL;
    }

    distance = (entry->x - x)*(entry->x - x) + (entry->y - y)*(entry->y - y) + (entry->z - z)*(entry->z - z);
    nearestDistance = distance;

    for(uint8 i = 0; i < EY_POINTS_MAX; ++i)
    {
        if(m_PointOwnedByTeam[i]==team && m_PointState[i]==EY_POINT_UNDER_CONTROL)
        {
            entry = sWorldSafeLocsStore.LookupEntry(m_CapturingPointTypes[i].GraveYardId);
            if(!entry)
                sLog.outError("BattleGroundEY: Not found graveyard: %u",m_CapturingPointTypes[i].GraveYardId);
            else
            {
                distance = (entry->x - x)*(entry->x - x) + (entry->y - y)*(entry->y - y) + (entry->z - z)*(entry->z - z);
                if(distance < nearestDistance)
                {
                    nearestDistance = distance;
                    nearestEntry = entry;
                }
            }
        }
    }

    return nearestEntry;
}
