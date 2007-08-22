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
#include "BattleGroundNA.h"
#include "Creature.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"

BattleGroundNA::BattleGroundNA()
{

}

BattleGroundNA::~BattleGroundNA()
{
    for(uint32 i = 0; i < BG_NA_OBJECT_MAX; i++)
        delete m_bgobjects[i].object;

    m_bgobjects.clear();
}

void BattleGroundNA::Update(time_t diff)
{
    BattleGround::Update(diff);

    // after bg start we get there
    if(GetStatus() == STATUS_WAIT_JOIN && !isDoorsSpawned() && GetPlayersSize() >= 1)
    {
        for(uint32 i = 0; i < BG_NA_OBJECT_MAX; i++)
        {
            // respawn
            MapManager::Instance().GetMap(m_bgobjects[i].object->GetMapId(), m_bgobjects[i].object)->Add(m_bgobjects[i].object);
        }
        sLog.outDebug("Doors spawned...");

        SetDoorsSpawned(true);
        SetStartDelayTime(START_DELAY);

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
            for(uint32 i = BG_NA_OBJECT_DOOR_3; i < BG_NA_OBJECT_MAX; i++)
            {
                // despawn
                MapManager::Instance().GetMap(m_bgobjects[i].object->GetMapId(), m_bgobjects[i].object)->Remove(m_bgobjects[i].object, false);
            }
            sLog.outDebug("Doors despawned...");

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

    if(GetStatus() == STATUS_IN_PROGRESS)
    {
        // update something
    }
}

void BattleGroundNA::RemovePlayer(Player *plr, uint64 guid)
{
    if(plr)
        plr->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);

    if(!GetPlayersSize())
    {
        for(uint32 i = 0; i < BG_NA_OBJECT_MAX; i++)
        {
            if(m_bgobjects[i].object->IsInWorld())
            {
                // despawn
                MapManager::Instance().GetMap(m_bgobjects[i].object->GetMapId(), m_bgobjects[i].object)->Remove(m_bgobjects[i].object, false);
            }
        }
        sLog.outDebug("Objects despawned...");
    }
}

void BattleGroundNA::HandleKillPlayer(Player *player, Player *killer)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    if(!killer)
    {
        sLog.outError("Killer player not found");
        return;
    }

    uint32 killer_team_index = GetTeamIndexByTeamId(killer->GetTeam());

    m_TeamKills[killer_team_index]++;              // add kills to killer's team

    if(m_TeamKills[killer_team_index] >= GetPlayersCountByTeam(player->GetTeam()))
    {
        // all opponents killed
        EndBattleGround(killer->GetTeam());
    }
}

void BattleGroundNA::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint32 SpellId = 0;
    switch(Trigger)
    {
        case 4536:  // buff trigger?
        case 4537:  // buff trigger?
            break;
        default:
            sLog.outError("WARNING: Unhandled AreaTrigger in Battleground: %u", Trigger);
            Source->GetSession()->SendAreaTriggerMessage("Warning: Unhandled AreaTrigger in Battleground: %u", Trigger);
            break;
    }

    if(SpellId)
    {
        SpellEntry const *Entry = sSpellStore.LookupEntry(SpellId);

        if(!Entry)
        {
            sLog.outError("ERROR: Tried to cast unknown spell id %u to player.", SpellId);
            return;
        }

        Source->CastSpell(Source, Entry, true, 0);
    }
}

bool BattleGroundNA::SetupBattleGround()
{
    if(!SpawnObject(183977, BG_NA_OBJECT_DOOR_1, 4023.709, 2981.777, 10.70117, -2.648788, 0, 0, 0.9697962, -0.2439165))
        return false;
    if(!SpawnObject(183979, BG_NA_OBJECT_DOOR_2, 4090.064, 2858.438, 10.23631, 0.4928045, 0, 0, 0.2439165, 0.9697962))
        return false;
    if(!SpawnObject(183978, BG_NA_OBJECT_DOOR_3, 4031.854, 2966.833, 12.6462, -2.648788, 0, 0, 0.9697962, -0.2439165))
        return false;
    if(!SpawnObject(183980, BG_NA_OBJECT_DOOR_4, 4081.179, 2874.97, 12.39171, 0.4928045, 0, 0, 0.2439165, 0.9697962))
        return false;

    return true;
}
