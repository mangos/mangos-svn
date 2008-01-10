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
#include "BattleGroundNA.h"
#include "Creature.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"

BattleGroundNA::BattleGroundNA()
{
    m_BgObjects.resize(BG_NA_OBJECT_MAX);
}

BattleGroundNA::~BattleGroundNA()
{

}

void BattleGroundNA::Update(time_t diff)
{
    BattleGround::Update(diff);

    // after bg start we get there
    if (GetStatus() == STATUS_WAIT_JOIN && GetPlayersSize())
    {
        ModifyStartDelayTime(diff);

        if (!(m_Events & 0x01))
        {
            m_Events |= 0x01;
            for(uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_4; i++)
                SpawnBGObject(i, RESPAWN_IMMEDIATELY);

            SetStartDelayTime(START_DELAY1);
            SendMessageToAll(LANG_ARENA_ONE_MINUTE);
        }
        // After 30 seconds, warning is signalled
        else if (GetStartDelayTime() <= START_DELAY2 && !(m_Events & 0x04))
        {
            m_Events |= 0x04;
            SendMessageToAll(LANG_ARENA_THIRTY_SECONDS);
        }
        // After 15 seconds, warning is signalled
        else if (GetStartDelayTime() <= START_DELAY3 && !(m_Events & 0x08))
        {
            m_Events |= 0x08;
            SendMessageToAll(LANG_ARENA_FIFTEEN_SECONDS);
        }
        // delay expired (1 minute)
        else if (GetStartDelayTime() <= 0 && !(m_Events & 0x10))
        {
            m_Events |= 0x10;

            for(uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_2; i++)
                DoorOpen(i);

            SendMessageToAll(LANG_ARENA_BEGUN);
            SetStatus(STATUS_IN_PROGRESS);
            SetStartDelayTime(0);

            for(std::map<uint64, BattleGroundPlayer>::const_iterator itr = GetPlayers()->begin(); itr != GetPlayers()->end(); ++itr)
            {
                Player *plr = objmgr.GetPlayer(itr->first);
                if(plr)
                    plr->RemoveAurasDueToSpell(SPELL_ARENA_PREPARATION);
            }
        }
    }

    /*if(GetStatus() == STATUS_IN_PROGRESS)
    {
        // update something
    }*/
}

void BattleGroundNA::AddPlayer(Player *plr)
{
    BattleGround::AddPlayer(plr);
    //create score and add it to map, default values are set in constructor
    BattleGroundNAScore* sc = new BattleGroundNAScore;

    m_PlayerScores[plr->GetGUID()] = sc;
}

void BattleGroundNA::RemovePlayer(Player *plr, uint64 guid)
{

}

void BattleGroundNA::HandleKillPlayer(Player *player, Player *killer)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    if(!killer)
    {
        sLog.outError("BattleGroundNA: Killer player not found");
        return;
    }

    uint32 killer_team_index = GetTeamIndexByTeamId(killer->GetTeam());

    m_TeamKills[killer_team_index]++;                       // add kills to killer's team

    if(m_TeamKills[killer_team_index] >= GetPlayersCountByTeam(player->GetTeam()))
    {
        // all opponents killed
        EndBattleGround(killer->GetTeam());
    }
}

void BattleGroundNA::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint32 SpellId = 0;
    uint64 buff_guid = 0;
    switch(Trigger)
    {
        case 4536:                                          // buff trigger?
        case 4537:                                          // buff trigger?
            break;
        default:
            sLog.outError("WARNING: Unhandled AreaTrigger in Battleground: %u", Trigger);
            Source->GetSession()->SendAreaTriggerMessage("Warning: Unhandled AreaTrigger in Battleground: %u", Trigger);
            break;
    }

    if(buff_guid)
    {
        GameObject *obj = HashMapHolder<GameObject>::Find(buff_guid);
        if(obj)
        {
            if(!obj->isSpawned())
                return;                                     // buff not spawned yet
            obj->SetRespawnTime(BUFF_RESPAWN_TIME);
            obj->SetLootState(GO_LOOTED);
            SpellId = obj->GetGOInfo()->data3;
            if(SpellId)
                Source->CastSpell(Source, SpellId, true);
        }
    }
}

void BattleGroundNA::ResetBGSubclass()
{
    m_TeamKills[BG_TEAM_ALLIANCE] = 0;
    m_TeamKills[BG_TEAM_HORDE]    = 0;
}

bool BattleGroundNA::SetupBattleGround()
{
    // gates
    if(    !AddObject(BG_NA_OBJECT_DOOR_1, BG_NA_OBJECT_TYPE_DOOR_1, 4031.854, 2966.833, 12.6462, -2.648788, 0, 0, 0.9697962, -0.2439165, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_NA_OBJECT_DOOR_2, BG_NA_OBJECT_TYPE_DOOR_2, 4081.179, 2874.97, 12.39171, 0.4928045, 0, 0, 0.2439165, 0.9697962, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_NA_OBJECT_DOOR_3, BG_NA_OBJECT_TYPE_DOOR_3, 4023.709, 2981.777, 10.70117, -2.648788, 0, 0, 0.9697962, -0.2439165, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_NA_OBJECT_DOOR_4, BG_NA_OBJECT_TYPE_DOOR_4, 4090.064, 2858.438, 10.23631, 0.4928045, 0, 0, 0.2439165, 0.9697962, RESPAWN_IMMEDIATELY))
    {
        sLog.outErrorDb("BatteGroundNA: Failed to spawn some object!");
        return false;
    }

    return true;
}

/*
20:12:14 id:036668 [S2C] SMSG_INIT_WORLD_STATES (706 = 0x02C2) len: 86
0000: 2f 02 00 00 72 0e 00 00 00 00 00 00 09 00 11 0a  |  /...r...........
0010: 00 00 01 00 00 00 0f 0a 00 00 00 00 00 00 10 0a  |  ................
0020: 00 00 00 00 00 00 d4 08 00 00 00 00 00 00 d8 08  |  ................
0030: 00 00 00 00 00 00 d7 08 00 00 00 00 00 00 d6 08  |  ................
0040: 00 00 00 00 00 00 d5 08 00 00 00 00 00 00 d3 08  |  ................
0050: 00 00 00 00 00 00                                |  ......
*/
