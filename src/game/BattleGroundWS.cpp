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
#include "BattleGroundWS.h"
#include "Creature.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"

BattleGroundWS::BattleGroundWS()
{
    Reset();
}

BattleGroundWS::~BattleGroundWS()
{
    for(uint32 i = 0; i < BG_OBJECT_MAX; i++)
        delete m_bgobjects[i].object;

    m_bgobjects.clear();
}

void BattleGroundWS::Update(time_t diff)
{
    BattleGround::Update(diff);
    //If BG-Status = WAIT_JOIN and Min players in BG, we must start BG
    if(GetStatus() == STATUS_WAIT_JOIN && GetPlayersSize() >= GetMinPlayers())
    {
        for(uint32 i = 0; i < BG_OBJECT_MAX; i++)
        {
            // activate
            m_bgobjects[i].spawned = true;
            // respawn
            MapManager::Instance().GetMap(m_bgobjects[i].object->GetMapId(), m_bgobjects[i].object)->Add(m_bgobjects[i].object);
        }
        sLog.outDebug("Objects activated and respawned...");

        SetStatus(STATUS_IN_PROGRESS);
    }

    if(GetStatus() == STATUS_IN_PROGRESS)
    {
        for(uint32 i = 0; i < BG_OBJECT_MAX; i++)
        {
            m_bgobjects[i].timer -= diff;
            if(m_bgobjects[i].timer < 0)
                m_bgobjects[i].timer = 0;

            if(m_bgobjects[i].timer == 0 && !m_bgobjects[i].spawned)
            {
                if(i == BG_OBJECT_A_FLAG)
                    RespawnFlag(ALLIANCE, true);
                else if(i == BG_OBJECT_H_FLAG)
                    RespawnFlag(HORDE, true);
                else
                    MapManager::Instance().GetMap(m_bgobjects[i].object->GetMapId(), m_bgobjects[i].object)->Add(m_bgobjects[i].object);

                // mark as spawned
                m_bgobjects[i].spawned = true;
            }
        }
    }
}

void BattleGroundWS::RespawnFlag(uint32 Team, bool captured)
{
    // need delay between flag capture and it's respawn about 30-60 seconds...
    if(Team == ALLIANCE)
    {
        sLog.outDebug("Respawn Alliance flag");
        MapManager::Instance().GetMap(m_bgobjects[BG_OBJECT_A_FLAG].object->GetMapId(), m_bgobjects[BG_OBJECT_A_FLAG].object)->Add(m_bgobjects[BG_OBJECT_A_FLAG].object);
    }

    if(Team == HORDE)
    {
        sLog.outDebug("Respawn Horde flag");
        MapManager::Instance().GetMap(m_bgobjects[BG_OBJECT_H_FLAG].object->GetMapId(), m_bgobjects[BG_OBJECT_H_FLAG].object)->Add(m_bgobjects[BG_OBJECT_H_FLAG].object);
    }

    if(captured)
    {
        WorldPacket data;
        const char *message = LANG_BG_F_PLACED;
        sChatHandler.FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, message, NULL);
        SendPacketToAll(&data);

        PlaySoundToAll(8232);                               // flags respawned sound...
    }
}

void BattleGroundWS::EventPlayerCapturedFlag(Player *Source)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint8 type = 0;
    uint32 winner = 0;
    const char *message = "";

    if(Source->GetTeam() == ALLIANCE)
    {
        SetHordeFlagPicker(0);                              // must be before aura remove to prevent 2 events (drop+capture) at the same time
        m_FlagState[1] = FLAG_STATE_ON_BASE;                // horde flag in base (but not respawned yet)
        Source->RemoveAurasDueToSpell(23333);               // Drop Horde Flag from Player
        message = LANG_BG_CAPTURED_HF;
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        if(GetTeamScore(ALLIANCE) < MAX_TEAM_SCORE)
            AddPoint(ALLIANCE, 1);
        PlaySoundToAll(8173);
    }
    if(Source->GetTeam() == HORDE)
    {
        SetAllianceFlagPicker(0);                           // must be before aura remove to prevent 2 events (drop+capture) at the same time
        m_FlagState[0] = FLAG_STATE_ON_BASE;                // alliance flag in base (but not respawned yet)
        Source->RemoveAurasDueToSpell(23335);               // Drop Alliance Flag from Player
        message = LANG_BG_CAPTURED_AF;
        type = CHAT_MSG_BG_SYSTEM_HORDE;
        if(GetTeamScore(HORDE) < MAX_TEAM_SCORE)
            AddPoint(HORDE, 1);
        PlaySoundToAll(8213);
    }

    WorldPacket data;
    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    UpdateFlagState(Source->GetTeam(), 1);                  // flag state none
    UpdateTeamScore(Source->GetTeam());
    UpdatePlayerScore(Source, SCORE_KILLS, 3);              // +3 kills for flag capture...
    UpdatePlayerScore(Source, SCORE_FLAG_CAPTURES, 1);      // +1 flag captures...

    if(GetTeamScore(ALLIANCE) == MAX_TEAM_SCORE)
        winner = ALLIANCE;

    if(GetTeamScore(HORDE) == MAX_TEAM_SCORE)
        winner = HORDE;

    if(winner)
    {
        UpdateWorldState(FLAG_UNK_ALLIANCE, 0);
        UpdateWorldState(FLAG_UNK_HORDE, 0);
        UpdateWorldState(FLAG_STATE_ALLIANCE, 1);
        UpdateWorldState(FLAG_STATE_HORDE, 1);

        EndBattleGround(winner);
    }
    else
    {
        switch(Source->GetTeam())
        {
            case ALLIANCE:
                m_bgobjects[BG_OBJECT_H_FLAG].timer     = FLAG_RESPAWN_TIME;
                m_bgobjects[BG_OBJECT_H_FLAG].spawned   = false;
                //RespawnFlag(HORDE, true);
                break;
            case HORDE:
                m_bgobjects[BG_OBJECT_A_FLAG].timer     = FLAG_RESPAWN_TIME;
                m_bgobjects[BG_OBJECT_A_FLAG].spawned   = false;
                //RespawnFlag(ALLIANCE, true);
                break;
        }
    }
}

void BattleGroundWS::EventPlayerDroppedFlag(Player *Source)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    const char *message;
    uint8 type = 0;

    if(Source->GetTeam() == ALLIANCE)
    {
        SetHordeFlagPicker(0);
        m_FlagState[1] = FLAG_STATE_ON_GROUND;              // horde flag dropped
        message = LANG_BG_DROPPED_HF;
        type = CHAT_MSG_BG_SYSTEM_HORDE;
    }
    if(Source->GetTeam() == HORDE)
    {
        SetAllianceFlagPicker(0);
        m_FlagState[0] = FLAG_STATE_ON_GROUND;              // alliance flag dropped
        message = LANG_BG_DROPPED_AF;
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
    }

    UpdateFlagState(Source->GetTeam(), 1);

    WorldPacket data;
    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    if(Source->GetTeam() == ALLIANCE)
        UpdateWorldState(FLAG_UNK_HORDE, uint32(-1));
    if(Source->GetTeam() == HORDE)
        UpdateWorldState(FLAG_UNK_ALLIANCE, uint32(-1));
}

void BattleGroundWS::EventPlayerReturnedFlag(Player *Source)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    const char *message;
    uint8 type = 0;

    if(Source->GetTeam() == ALLIANCE)
    {
        m_FlagState[0] = FLAG_STATE_ON_BASE;                // alliance flag in base
        message = LANG_BG_RETURNED_AF;
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        UpdateFlagState(HORDE, 1);
        RespawnFlag(ALLIANCE, false);
    }
    if(Source->GetTeam() == HORDE)
    {
        m_FlagState[1] = FLAG_STATE_ON_BASE;                // horde flag in base
        message = LANG_BG_RETURNED_HF;
        type = CHAT_MSG_BG_SYSTEM_HORDE;
        UpdateFlagState(ALLIANCE, 1);
        RespawnFlag(HORDE, false);
    }

    PlaySoundToAll(8192);                                   // flag returned (common sound)
    UpdatePlayerScore(Source, SCORE_FLAG_RETURNS, 1);       // +1 to flag returns...

    WorldPacket data;
    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);
}

void BattleGroundWS::EventPlayerPickedUpFlag(Player *Source)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    const char *message;
    uint8 type = 0;

    if(Source->GetTeam() == ALLIANCE)
    {
        message = LANG_BG_PICKEDUP_HF;
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        PlaySoundToAll(8212);
        MapManager::Instance().GetMap(m_bgobjects[BG_OBJECT_H_FLAG].object->GetMapId(), m_bgobjects[BG_OBJECT_H_FLAG].object)->Remove(m_bgobjects[BG_OBJECT_H_FLAG].object, false);
        SetHordeFlagPicker(Source->GetGUID());              // pick up Horde Flag
        m_FlagState[1] = FLAG_STATE_ON_PLAYER;              // horde flag pickedup
    }
    if(Source->GetTeam() == HORDE)
    {
        message = LANG_BG_PICKEDUP_AF;
        type = CHAT_MSG_BG_SYSTEM_HORDE;
        PlaySoundToAll(8174);
        MapManager::Instance().GetMap(m_bgobjects[BG_OBJECT_A_FLAG].object->GetMapId(), m_bgobjects[BG_OBJECT_A_FLAG].object)->Remove(m_bgobjects[BG_OBJECT_A_FLAG].object, false);
        SetAllianceFlagPicker(Source->GetGUID());           // pick up Alliance Flag
        m_FlagState[0] = FLAG_STATE_ON_PLAYER;              // alliance flag pickedup
    }

    WorldPacket data;
    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    UpdateFlagState(Source->GetTeam(), 2);

    if(Source->GetTeam() == ALLIANCE)
        UpdateWorldState(FLAG_UNK_HORDE, 1);
    if(Source->GetTeam() == HORDE)
        UpdateWorldState(FLAG_UNK_ALLIANCE, 1);
}

void BattleGroundWS::RemovePlayer(Player *plr, uint64 guid)
{
    if(IsAllianceFlagPickedup() || IsHordeFlagPickedup())
    {
        if(m_FlagKeepers[0] == guid)
        {
            if(plr)
                plr->RemoveAurasDueToSpell(23335);
            else
            {
                //AllianceFlagSpawn[0] = 0;
                //AllianceFlagSpawn[1] = 1;
                SetAllianceFlagPicker(0);
                RespawnFlag(ALLIANCE, false);
                m_FlagState[0] = FLAG_STATE_ON_BASE;
            }
        }
        if(m_FlagKeepers[1] == guid)
        {
            if(plr)
                plr->RemoveAurasDueToSpell(23333);
            else
            {
                //HordeFlagSpawn[0] = 0;
                //HordeFlagSpawn[1] = 1;
                SetHordeFlagPicker(0);
                RespawnFlag(HORDE, false);
                m_FlagState[1] = FLAG_STATE_ON_BASE;
            }
        }
    }

    if(!GetPlayersSize())
    {
        for(uint32 i = 0; i < BG_OBJECT_MAX; i++)
        {
            // despawn
            MapManager::Instance().GetMap(m_bgobjects[i].object->GetMapId(), m_bgobjects[i].object)->Remove(m_bgobjects[i].object, false);
        }
        sLog.outDebug("Objects despawned...");

        Reset();
    }
}

void BattleGroundWS::UpdateFlagState(uint32 team, uint32 value)
{
    if(team == ALLIANCE)
        UpdateWorldState(FLAG_STATE_ALLIANCE, value);
    else if(team == HORDE)
        UpdateWorldState(FLAG_STATE_HORDE, value);
}

void BattleGroundWS::UpdateTeamScore(uint32 team)
{
    if(team == ALLIANCE)
        UpdateWorldState(FLAG_CAPTURES_ALLIANCE, GetTeamScore(team));
    if(team == HORDE)
        UpdateWorldState(FLAG_CAPTURES_HORDE, GetTeamScore(team));
}

void BattleGroundWS::HandleAreaTrigger(Player* Source, uint32 Trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint32 SpellId = 0;
    switch(Trigger)
    {
        case 3686:                                                  //Alliance elixir of speed spawn. Trigger not working, because located inside other areatrigger, can be replaced by IsWithinDist(object, dist) in BattleGround::Update().
            if(!m_bgobjects[BG_OBJECT_SPEEDBUFF_1].spawned)
                break;
            MapManager::Instance().GetMap(m_bgobjects[BG_OBJECT_SPEEDBUFF_1].object->GetMapId(), m_bgobjects[BG_OBJECT_SPEEDBUFF_1].object)->Remove(m_bgobjects[BG_OBJECT_SPEEDBUFF_1].object, false);
            m_bgobjects[BG_OBJECT_SPEEDBUFF_1].timer = BUFF_RESPAWN_TIME;   // 3 minutes
            m_bgobjects[BG_OBJECT_SPEEDBUFF_1].spawned = false;
            SpellId = m_bgobjects[BG_OBJECT_SPEEDBUFF_1].spellid;
            break;
        case 3687:                                                  //Horde elixir of speed spawn. Trigger not working, because located inside other areatrigger, can be replaced by IsWithinDist(object, dist) in BattleGround::Update().
            if(!m_bgobjects[BG_OBJECT_SPEEDBUFF_2].spawned)
                break;
            MapManager::Instance().GetMap(m_bgobjects[BG_OBJECT_SPEEDBUFF_2].object->GetMapId(), m_bgobjects[BG_OBJECT_SPEEDBUFF_2].object)->Remove(m_bgobjects[BG_OBJECT_SPEEDBUFF_2].object, false);
            m_bgobjects[BG_OBJECT_SPEEDBUFF_2].timer = BUFF_RESPAWN_TIME;   // 3 minutes
            m_bgobjects[BG_OBJECT_SPEEDBUFF_2].spawned = false;
            SpellId = m_bgobjects[BG_OBJECT_SPEEDBUFF_2].spellid;
            break;
        case 3706:                                                  //Alliance elixir of regeneration spawn
            if(!m_bgobjects[BG_OBJECT_REGENBUFF_1].spawned)
                break;
            MapManager::Instance().GetMap(m_bgobjects[BG_OBJECT_REGENBUFF_1].object->GetMapId(), m_bgobjects[BG_OBJECT_REGENBUFF_1].object)->Remove(m_bgobjects[BG_OBJECT_REGENBUFF_1].object, false);
            m_bgobjects[BG_OBJECT_REGENBUFF_1].timer = BUFF_RESPAWN_TIME;   // 3 minutes
            m_bgobjects[BG_OBJECT_REGENBUFF_1].spawned = false;
            SpellId = m_bgobjects[BG_OBJECT_REGENBUFF_1].spellid;
            break;
        case 3708:                                                  //Horde elixir of regeneration spawn
            if(!m_bgobjects[BG_OBJECT_REGENBUFF_2].spawned)
                break;
            MapManager::Instance().GetMap(m_bgobjects[BG_OBJECT_REGENBUFF_2].object->GetMapId(), m_bgobjects[BG_OBJECT_REGENBUFF_2].object)->Remove(m_bgobjects[BG_OBJECT_REGENBUFF_2].object, false);
            m_bgobjects[BG_OBJECT_REGENBUFF_2].timer = BUFF_RESPAWN_TIME;   // 3 minutes
            m_bgobjects[BG_OBJECT_REGENBUFF_2].spawned = false;
            SpellId = m_bgobjects[BG_OBJECT_REGENBUFF_2].spellid;
            break;
        case 3707:                                                  //Alliance elixir of berserk spawn
            if(!m_bgobjects[BG_OBJECT_BERSERKBUFF_1].spawned)
                break;
            MapManager::Instance().GetMap(m_bgobjects[BG_OBJECT_BERSERKBUFF_1].object->GetMapId(), m_bgobjects[BG_OBJECT_BERSERKBUFF_1].object)->Remove(m_bgobjects[BG_OBJECT_BERSERKBUFF_1].object, false);
            m_bgobjects[BG_OBJECT_BERSERKBUFF_1].timer = BUFF_RESPAWN_TIME; // 3 minutes
            m_bgobjects[BG_OBJECT_BERSERKBUFF_1].spawned = false;
            SpellId = m_bgobjects[BG_OBJECT_BERSERKBUFF_1].spellid;
            break;
        case 3709:                                                  //Horde elixir of berserk spawn
            if(!m_bgobjects[BG_OBJECT_BERSERKBUFF_2].spawned)
                break;
            MapManager::Instance().GetMap(m_bgobjects[BG_OBJECT_BERSERKBUFF_2].object->GetMapId(), m_bgobjects[BG_OBJECT_BERSERKBUFF_2].object)->Remove(m_bgobjects[BG_OBJECT_BERSERKBUFF_2].object, false);
            m_bgobjects[BG_OBJECT_BERSERKBUFF_2].timer = BUFF_RESPAWN_TIME; // 3 minutes
            m_bgobjects[BG_OBJECT_BERSERKBUFF_2].spawned = false;
            SpellId = m_bgobjects[BG_OBJECT_BERSERKBUFF_2].spellid;
            break;
        case 3646:                                                  //Alliance Flag spawn
            if(m_FlagState[1] && !m_FlagState[0])
                if(GetHordeFlagPickerGUID() == Source->GetGUID())
                    EventPlayerCapturedFlag(Source);
            break;
        case 3647:                                                  //Horde Flag spawn
            if(m_FlagState[0] && !m_FlagState[1])
                if(GetAllianceFlagPickerGUID() == Source->GetGUID())
                    EventPlayerCapturedFlag(Source);
            break;
        case 4628:                                                  // new 2.1.0?
        case 4629:                                                  // new 2.1.0?
        case 4631:                                                  // Unk1
        case 4633:                                                  // Unk2
            break;
        case 3669:                                                  // Warsong Gulch Horde Exit (removed, but trigger still exist).
        case 3671:                                                  // Warsong Gulch Alliance Exit (removed, but trigger still exist).
            break;
        default:
        {
            sLog.outError("WARNING: Unhandled AreaTrigger in Battleground: %u", Trigger);
            Source->GetSession()->SendAreaTriggerMessage("Warning: Unhandled AreaTrigger in Battleground: %u", Trigger);
            break;
        }
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

bool BattleGroundWS::SetupBattleGround()
{
    BattleGroundObjectInfo info;
    for(uint32 i = 0; i < BG_OBJECT_MAX; i++)
    {
        info.object     = new GameObject(NULL);
        info.spawned    = false;
        info.spellid    = 0;
        info.timer      = 0;
        m_bgobjects[i]  = info;
    }

    if(!m_bgobjects[BG_OBJECT_A_FLAG].object->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179830, GetMapId(), 1540.35, 1481.31, 352.635, 6.24, 0, 0, sin(6.24/2), cos(6.24/2), 0, 0))
        return false;
    if(!m_bgobjects[BG_OBJECT_H_FLAG].object->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179831, GetMapId(), 915.809, 1433.73, 346.172, 3.244, 0, 0, sin(3.244/2), cos(3.244/2), 0, 0))
        return false;

    if(!m_bgobjects[BG_OBJECT_SPEEDBUFF_1].object->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179871, GetMapId(), 1449.98, 1468.86, 342.66, 4.866, 0, 0, sin(4.866/2), cos(4.866/2), 0, 0))
        return false;
    m_bgobjects[BG_OBJECT_SPEEDBUFF_1].spellid = 23451;

    if(!m_bgobjects[BG_OBJECT_SPEEDBUFF_2].object->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179871, GetMapId(), 1006.22, 1445.98, 335.77, 1.683, 0, 0, sin(1.683/2), cos(1.683/2), 0, 0))
        return false;
    m_bgobjects[BG_OBJECT_SPEEDBUFF_2].spellid = 23451;

    if(!m_bgobjects[BG_OBJECT_REGENBUFF_1].object->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179904, GetMapId(), 1316.94, 1551.99, 313.234, 5.869, 0, 0, sin(5.869/2), cos(5.869/2), 0, 0))
        return false;
    m_bgobjects[BG_OBJECT_REGENBUFF_1].spellid = 23493;

    if(!m_bgobjects[BG_OBJECT_REGENBUFF_2].object->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179904, GetMapId(), 1110.1, 1353.24, 316.513, 5.68, 0, 0, sin(5.68/2), cos(5.68/2), 0, 0))
        return false;
    m_bgobjects[BG_OBJECT_REGENBUFF_2].spellid = 23493;

    if(!m_bgobjects[BG_OBJECT_BERSERKBUFF_1].object->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179905, GetMapId(), 1318.68, 1378.03, 314.753, 1.001, 0, 0, sin(1.001/2), cos(1.001/2), 0, 0))
        return false;
    m_bgobjects[BG_OBJECT_BERSERKBUFF_1].spellid = 23505;

    if(!m_bgobjects[BG_OBJECT_BERSERKBUFF_2].object->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179905, GetMapId(), 1141.36, 1560.99, 306.791, 3.858, 0, 0, sin(3.858/2), cos(3.858/2), 0, 0))
        return false;
    m_bgobjects[BG_OBJECT_BERSERKBUFF_2].spellid = 23505;

    return true;
}

void BattleGroundWS::Reset()
{
    m_FlagKeepers[0]    = 0;
    m_FlagKeepers[1]    = 0;
    m_FlagState[0]      = FLAG_STATE_ON_BASE;
    m_FlagState[1]      = FLAG_STATE_ON_BASE;
    m_TeamScores[0]     = 0;
    m_TeamScores[1]     = 0;

    SetStatus(STATUS_WAIT_QUEUE);
    SetStartTime(0);
    SetEndTime(0);
    SetLastResurrectTime(0);

    //m_PlayerScores.clear();
    //m_Players.clear();
    //m_ReviveQueue.clear();
    //m_RemovedPlayers.clear();
}

void BattleGroundWS::HandleKillPlayer(Player* player)
{
    if(player->GetTeam() == HORDE && IsAllianceFlagPickedup())
    {
        if(GetAllianceFlagPickerGUID() == player->GetGUID())
        {
            SetAllianceFlagPicker(0);
            player->CastSpell(player, 23336, true);   // Alliance Flag Drop
        }
    }
    if(player->GetTeam() == ALLIANCE && IsHordeFlagPickedup())
    {
        if(GetHordeFlagPickerGUID() == player->GetGUID())
        {
            SetHordeFlagPicker(0);
            player->CastSpell(player, 23334, true);   // Horde Flag Drop
        }
    }
}

void BattleGroundWS::HandleDropFlag( Player* player )
{
    if(player->GetTeam() == HORDE)
    {
        if(IsAllianceFlagPickedup())
        {
            if(GetAllianceFlagPickerGUID() == player->GetGUID())
            {
                SetAllianceFlagPicker(0);
                player->RemoveAurasDueToSpell(23335);
                player->CastSpell(player,23336,true,NULL);
            }
        }
    }
    else                                                    // ALLIANCE
    {
        if(IsHordeFlagPickedup())
        {
            if(GetHordeFlagPickerGUID() == player->GetGUID())
            {
                SetHordeFlagPicker(0);
                player->RemoveAurasDueToSpell(23333);
                player->CastSpell(player,23334,true,NULL);
            }
        }
    }
}
