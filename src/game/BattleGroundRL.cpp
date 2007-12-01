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

#include "Object.h"
#include "Player.h"
#include "BattleGround.h"
#include "BattleGroundRL.h"
#include "Creature.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"

BattleGroundRL::BattleGroundRL()
{
    m_bgobjects.resize(BG_RL_OBJECT_MAX);
}

BattleGroundRL::~BattleGroundRL()
{

}

void BattleGroundRL::Update(time_t diff)
{
    BattleGround::Update(diff);

    // after bg start we get there
    if(GetStatus() == STATUS_WAIT_JOIN && !isDoorsSpawned() && GetPlayersSize() >= 1)
    {
        for(uint32 i = BG_RL_OBJECT_DOOR_1; i <= BG_RL_OBJECT_DOOR_2; i++)
        {
            SpawnBGObject(i, RESPAWN_IMMEDIATELY);
        }
        sLog.outDebug("Doors spawned...");

        SetDoorsSpawned(true);
        SetStartDelayTime(START_DELAY1);

        WorldPacket data;
        const char *message = LANG_ARENA_ONE_MINUTE;
        sChatHandler.FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, message, NULL);
        SendPacketToAll(&data);
    }

    // after bg start and doors spawn we get there
    if(GetStatus() == STATUS_WAIT_JOIN && isDoorsSpawned())
    {
        ModifyStartDelayTime(diff);

        // delay expired (1 minute)
        if(GetStartDelayTime() < 0)
        {
            for(uint32 i = BG_RL_OBJECT_DOOR_1; i <= BG_RL_OBJECT_DOOR_2; i++)
            {
                DoorOpen(i);
            }
            sLog.outDebug("Doors opened...");

            WorldPacket data;
            const char *message = LANG_ARENA_BEGUN;
            sChatHandler.FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, message, NULL);
            SendPacketToAll(&data);

            SetStatus(STATUS_IN_PROGRESS);

            SetDoorsSpawned(false);
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

void BattleGroundRL::AddPlayer(Player *plr)
{
    BattleGround::AddPlayer(plr);
    //create score and add it to map, default values are set in constructor
    BattleGroundRLScore* sc = new BattleGroundRLScore;

    m_PlayerScores[plr->GetGUID()] = sc;
}

void BattleGroundRL::RemovePlayer(Player *plr, uint64 guid)
{

}

void BattleGroundRL::HandleKillPlayer(Player *player, Player *killer)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    if(!killer)
    {
        sLog.outError("Killer player not found");
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

void BattleGroundRL::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint32 SpellId = 0;
    uint64 buff_guid = 0;
    switch(Trigger)
    {
        case 4696:                                          // buff trigger?
        case 4697:                                          // buff trigger?
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

bool BattleGroundRL::SetupBattleGround()
{
    // gates
    if(!AddObject(BG_RL_OBJECT_DOOR_1, 185918, 1293.561, 1601.938, 31.60557, -1.457349, 0, 0, -0.6658813, 0.7460576, 0))
        return false;
    if(!AddObject(BG_RL_OBJECT_DOOR_2, 185917, 1278.648, 1730.557, 31.60557, 1.684245, 0, 0, 0.7460582, 0.6658807, 0))
        return false;

    return true;
}

/*
Packet S->C, id 600, SMSG_INIT_WORLD_STATES (706), len 86
0000: 3C 02 00 00 80 0F 00 00 00 00 00 00 09 00 BA 0B | <...............
0010: 00 00 01 00 00 00 B9 0B 00 00 02 00 00 00 B8 0B | ................
0020: 00 00 00 00 00 00 D8 08 00 00 00 00 00 00 D7 08 | ................
0030: 00 00 00 00 00 00 D6 08 00 00 00 00 00 00 D5 08 | ................
0040: 00 00 00 00 00 00 D3 08 00 00 00 00 00 00 D4 08 | ................
0050: 00 00 00 00 00 00                               | ......
*/
