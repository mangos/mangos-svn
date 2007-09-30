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
    m_bgobjects.resize(BG_WS_OBJECT_MAX);
    Reset();
}

BattleGroundWS::~BattleGroundWS()
{
}

void BattleGroundWS::Update(time_t diff)
{
    BattleGround::Update(diff);

    // after bg start we get there (once)
    if((GetStatus() == STATUS_WAIT_JOIN) && !isDoorsSpawned() && GetPlayersSize() >= 1)
    {
        for(uint32 i = BG_WS_OBJECT_DOOR_A_1; i <= BG_WS_OBJECT_DOOR_H_4; i++)
        {
            SpawnBGObject(i, RESPAWN_IMMEDIATELY);
        }
        sLog.outDebug("Doors spawned...");

        for(uint32 i = BG_WS_OBJECT_A_FLAG; i <= BG_WS_OBJECT_BERSERKBUFF_2; i++)
        {
            SpawnBGObject(i, RESPAWN_ONE_DAY);
        }
        sLog.outDebug("Objects despawned...");

        SetDoorsSpawned(true);
        SetStartDelayTime(START_DELAY1);

        WorldPacket data;
        const char *message = LANG_BG_WS_ONE_MINUTE;
        sChatHandler.FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, message, NULL);
        SendPacketToAll(&data);
    }

    // after bg start and doors spawn we get there
    if((GetStatus() == STATUS_WAIT_JOIN) && isDoorsSpawned())
    {
        ModifyStartDelayTime(diff);

        // delay expired (1 minute)
        if(GetStartDelayTime() < 0)
        {
            for(uint32 i = BG_WS_OBJECT_DOOR_A_1; i <= BG_WS_OBJECT_DOOR_H_4; i++)
            {
                SpawnBGObject(i, RESPAWN_ONE_DAY);
            }
            sLog.outDebug("Doors despawned...");

            for(uint32 i = BG_WS_OBJECT_A_FLAG; i <= BG_WS_OBJECT_BERSERKBUFF_2; i++)
            {
                SpawnBGObject(i, RESPAWN_IMMEDIATELY);
            }
            sLog.outDebug("Objects spawned...");

            WorldPacket data;
            const char *message = LANG_BG_WS_BEGIN;
            sChatHandler.FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, message, NULL);
            SendPacketToAll(&data);

            PlaySoundToAll(SOUND_BG_START);

            SetStatus(STATUS_IN_PROGRESS);

            SetDoorsSpawned(false);
            SetStartDelayTime(0);
        }
    }

    if(GetStatus() == STATUS_IN_PROGRESS)
    {
        if(m_FlagState[0] == BG_WS_FLAG_STATE_WAIT_RESPAWN)
        {
            m_FlagsTimer[0] -= diff;

            if(m_FlagsTimer[0] < 0)
                m_FlagsTimer[0] = 0;

            if(m_FlagsTimer[0] == 0)
                RespawnFlag(ALLIANCE, true);
        }
        if(m_FlagState[1] == BG_WS_FLAG_STATE_WAIT_RESPAWN)
        {
            m_FlagsTimer[1] -= diff;

            if(m_FlagsTimer[1] < 0)
                m_FlagsTimer[1] = 0;

            if(m_FlagsTimer[1] == 0)
                RespawnFlag(HORDE, true);
        }
    }
}

void BattleGroundWS::RespawnFlag(uint32 Team, bool captured)
{
    if(Team == ALLIANCE)
    {
        sLog.outDebug("Respawn Alliance flag");
        m_FlagState[0] = BG_WS_FLAG_STATE_ON_BASE;
    }
    else
    {
        sLog.outDebug("Respawn Horde flag");
        m_FlagState[1] = BG_WS_FLAG_STATE_ON_BASE;
    }

    if(captured)
    {
        WorldPacket data;
        const char *message = LANG_BG_WS_F_PLACED;
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
        m_FlagState[1] = BG_WS_FLAG_STATE_WAIT_RESPAWN;           // horde flag in base (but not respawned yet)
        Source->RemoveAurasDueToSpell(23333);               // Drop Horde Flag from Player
        message = LANG_BG_WS_CAPTURED_HF;
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        if(GetTeamScore(ALLIANCE) < BG_WS_MAX_TEAM_SCORE)
            AddPoint(ALLIANCE, 1);
        PlaySoundToAll(8173);
        SpawnBGObject(BG_WS_OBJECT_H_FLAG, BG_WS_FLAG_RESPAWN_TIME/1000);
    }
    else
    {
        SetAllianceFlagPicker(0);                           // must be before aura remove to prevent 2 events (drop+capture) at the same time
        m_FlagState[0] = BG_WS_FLAG_STATE_WAIT_RESPAWN;           // alliance flag in base (but not respawned yet)
        Source->RemoveAurasDueToSpell(23335);               // Drop Alliance Flag from Player
        message = LANG_BG_WS_CAPTURED_AF;
        type = CHAT_MSG_BG_SYSTEM_HORDE;
        if(GetTeamScore(HORDE) < BG_WS_MAX_TEAM_SCORE)
            AddPoint(HORDE, 1);
        PlaySoundToAll(8213);
        SpawnBGObject(BG_WS_OBJECT_A_FLAG, BG_WS_FLAG_RESPAWN_TIME/1000);
    }

    WorldPacket data;
    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    UpdateFlagState(Source->GetTeam(), 1);                  // flag state none
    UpdateTeamScore(Source->GetTeam());
    UpdatePlayerScore(Source, SCORE_KILLS, 3);              // +3 kills for flag capture...
    UpdatePlayerScore(Source, SCORE_FLAG_CAPTURES, 1);      // +1 flag captures...

    if(GetTeamScore(ALLIANCE) == BG_WS_MAX_TEAM_SCORE)
        winner = ALLIANCE;

    if(GetTeamScore(HORDE) == BG_WS_MAX_TEAM_SCORE)
        winner = HORDE;

    if(winner)
    {
        UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, 0);
        UpdateWorldState(BG_WS_FLAG_UNK_HORDE, 0);
        UpdateWorldState(BG_WS_FLAG_STATE_ALLIANCE, 1);
        UpdateWorldState(BG_WS_FLAG_STATE_HORDE, 1);

        EndBattleGround(winner);
    }
    else
    {
        m_FlagsTimer[GetTeamIndexByTeamId(Source->GetTeam()) ? 0 : 1] = BG_WS_FLAG_RESPAWN_TIME;
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
        if(IsHordeFlagPickedup())
        {
            if(GetHordeFlagPickerGUID() == Source->GetGUID())
            {
                SetHordeFlagPicker(0);
                Source->RemoveAurasDueToSpell(23333);
                m_FlagState[1] = BG_WS_FLAG_STATE_ON_GROUND;      // horde flag dropped
                message = LANG_BG_WS_DROPPED_HF;
                type = CHAT_MSG_BG_SYSTEM_HORDE;
                Source->CastSpell(Source, 23334, true);
            }
        }
    }
    else
    {
        if(IsAllianceFlagPickedup())
        {
            if(GetAllianceFlagPickerGUID() == Source->GetGUID())
            {
                SetAllianceFlagPicker(0);
                Source->RemoveAurasDueToSpell(23335);
                m_FlagState[0] = BG_WS_FLAG_STATE_ON_GROUND;      // alliance flag dropped
                message = LANG_BG_WS_DROPPED_AF;
                type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
                Source->CastSpell(Source, 23336, true);
            }
        }
    }

    UpdateFlagState(Source->GetTeam(), 1);

    WorldPacket data;
    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    if(Source->GetTeam() == ALLIANCE)
        UpdateWorldState(BG_WS_FLAG_UNK_HORDE, uint32(-1));
    else
        UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, uint32(-1));
}

void BattleGroundWS::EventPlayerReturnedFlag(Player *Source)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    const char *message;
    uint8 type = 0;

    if(Source->GetTeam() == ALLIANCE)
    {
        message = LANG_BG_WS_RETURNED_AF;
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        UpdateFlagState(HORDE, 1);
        RespawnFlag(ALLIANCE, false);
        SpawnBGObject(BG_WS_OBJECT_A_FLAG, RESPAWN_IMMEDIATELY);
    }
    else
    {
        message = LANG_BG_WS_RETURNED_HF;
        type = CHAT_MSG_BG_SYSTEM_HORDE;
        UpdateFlagState(ALLIANCE, 1);
        RespawnFlag(HORDE, false);
        SpawnBGObject(BG_WS_OBJECT_H_FLAG, RESPAWN_IMMEDIATELY);
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
        message = LANG_BG_WS_PICKEDUP_HF;
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        PlaySoundToAll(8212);
        SpawnBGObject(BG_WS_OBJECT_H_FLAG, RESPAWN_ONE_DAY);
        SetHordeFlagPicker(Source->GetGUID());              // pick up Horde Flag
        m_FlagState[1] = BG_WS_FLAG_STATE_ON_PLAYER;              // horde flag pickedup
    }
    else
    {
        message = LANG_BG_WS_PICKEDUP_AF;
        type = CHAT_MSG_BG_SYSTEM_HORDE;
        PlaySoundToAll(8174);
        SpawnBGObject(BG_WS_OBJECT_A_FLAG, RESPAWN_ONE_DAY);
        SetAllianceFlagPicker(Source->GetGUID());           // pick up Alliance Flag
        m_FlagState[0] = BG_WS_FLAG_STATE_ON_PLAYER;              // alliance flag pickedup
    }

    WorldPacket data;
    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    UpdateFlagState(Source->GetTeam(), 2);

    if(Source->GetTeam() == ALLIANCE)
        UpdateWorldState(BG_WS_FLAG_UNK_HORDE, 1);
    else
        UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, 1);
}

void BattleGroundWS::RemovePlayer(Player *plr, uint64 guid)
{
    // sometimes flag aura not removed :(
    if(IsAllianceFlagPickedup() || IsHordeFlagPickedup())
    {
        if(m_FlagKeepers[0] == guid)
        {
            if(plr)
                plr->RemoveAurasDueToSpell(23335);
            else
            {
                SetAllianceFlagPicker(0);
                RespawnFlag(ALLIANCE, false);
            }
        }
        if(m_FlagKeepers[1] == guid)
        {
            if(plr)
                plr->RemoveAurasDueToSpell(23333);
            else
            {
                SetHordeFlagPicker(0);
                RespawnFlag(HORDE, false);
            }
        }
    }

    if(!GetPlayersSize())
    {
        Reset();
    }
}

void BattleGroundWS::UpdateFlagState(uint32 team, uint32 value)
{
    if(team == ALLIANCE)
        UpdateWorldState(BG_WS_FLAG_STATE_ALLIANCE, value);
    else
        UpdateWorldState(BG_WS_FLAG_STATE_HORDE, value);
}

void BattleGroundWS::UpdateTeamScore(uint32 team)
{
    if(team == ALLIANCE)
        UpdateWorldState(BG_WS_FLAG_CAPTURES_ALLIANCE, GetTeamScore(team));
    else
        UpdateWorldState(BG_WS_FLAG_CAPTURES_HORDE, GetTeamScore(team));
}

void BattleGroundWS::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint32 SpellId = 0;
    uint64 buff_guid = 0;
    switch(Trigger)
    {
        case 3686:                                          // Alliance elixir of speed spawn. Trigger not working, because located inside other areatrigger, can be replaced by IsWithinDist(object, dist) in BattleGround::Update().
            buff_guid = m_bgobjects[BG_WS_OBJECT_SPEEDBUFF_1];
            break;
        case 3687:                                          // Horde elixir of speed spawn. Trigger not working, because located inside other areatrigger, can be replaced by IsWithinDist(object, dist) in BattleGround::Update().
            buff_guid = m_bgobjects[BG_WS_OBJECT_SPEEDBUFF_2];
            break;
        case 3706:                                          // Alliance elixir of regeneration spawn
            buff_guid = m_bgobjects[BG_WS_OBJECT_REGENBUFF_1];
            break;
        case 3708:                                          // Horde elixir of regeneration spawn
            buff_guid = m_bgobjects[BG_WS_OBJECT_REGENBUFF_2];
            break;
        case 3707:                                          // Alliance elixir of berserk spawn
            buff_guid = m_bgobjects[BG_WS_OBJECT_BERSERKBUFF_1];
            break;
        case 3709:                                          // Horde elixir of berserk spawn
            buff_guid = m_bgobjects[BG_WS_OBJECT_BERSERKBUFF_2];
            break;
        case 3646:                                          // Alliance Flag spawn
            if(m_FlagState[1] && !m_FlagState[0])
                if(GetHordeFlagPickerGUID() == Source->GetGUID())
                    EventPlayerCapturedFlag(Source);
            break;
        case 3647:                                          // Horde Flag spawn
            if(m_FlagState[0] && !m_FlagState[1])
                if(GetAllianceFlagPickerGUID() == Source->GetGUID())
                    EventPlayerCapturedFlag(Source);
            break;
        case 3649:                                          // unk1
        case 3688:                                          // unk2
        case 4628:                                          // unk3
        case 4629:                                          // unk4
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
            SpellId = obj->GetGOInfo()->sound3;
            if(SpellId)
                Source->CastSpell(Source, SpellId, true);
        }
    }
}

bool BattleGroundWS::SetupBattleGround()
{
    // flags
    if(!AddObject(BG_WS_OBJECT_A_FLAG, 179830, 1540.423, 1481.325, 351.8284, 3.089233, 0, 0, 0.9996573, 0.02617699, BG_WS_FLAG_RESPAWN_TIME/1000))
        return false;
    if(!AddObject(BG_WS_OBJECT_H_FLAG, 179831, 916.0226, 1434.405, 345.413, 0.01745329, 0, 0, 0.008726535, 0.9999619, BG_WS_FLAG_RESPAWN_TIME/1000))
        return false;
    // buffs
    if(!AddObject(BG_WS_OBJECT_SPEEDBUFF_1, 179871, 1449.93, 1470.71, 342.6346, -1.64061, 0, 0, 0.7313537, -0.6819983, BUFF_RESPAWN_TIME))
        return false;
    if(!AddObject(BG_WS_OBJECT_SPEEDBUFF_2, 179899, 1005.171, 1447.946, 335.9032, 1.64061, 0, 0, 0.7313537, 0.6819984, BUFF_RESPAWN_TIME))
        return false;
    if(!AddObject(BG_WS_OBJECT_REGENBUFF_1, 179904, 1317.506, 1550.851, 313.2344, -0.2617996, 0, 0, 0.1305263, -0.9914448, BUFF_RESPAWN_TIME))
        return false;
    if(!AddObject(BG_WS_OBJECT_REGENBUFF_2, 179906, 1110.451, 1353.656, 316.5181, -0.6806787, 0, 0, 0.333807, -0.9426414, BUFF_RESPAWN_TIME))
        return false;
    if(!AddObject(BG_WS_OBJECT_BERSERKBUFF_1, 179905, 1320.09, 1378.79, 314.7532, 1.186824, 0, 0, 0.5591929, 0.8290376, BUFF_RESPAWN_TIME))
        return false;
    if(!AddObject(BG_WS_OBJECT_BERSERKBUFF_2, 179907, 1139.688, 1560.288, 306.8432, -2.443461, 0, 0, 0.9396926, -0.3420201, BUFF_RESPAWN_TIME))
        return false;
    // alliance gates
    if(!AddObject(BG_WS_OBJECT_DOOR_A_1, 179918, 1503.335, 1493.466, 352.1888, 3.115414, 0, 0, 0.9999143, 0.01308903, 0))
        return false;
    if(!AddObject(BG_WS_OBJECT_DOOR_A_2, 179919, 1492.478, 1457.912, 342.9689, 3.115414, 0, 0, 0.9999143, 0.01308903, 0))
        return false;
    if(!AddObject(BG_WS_OBJECT_DOOR_A_3, 179920, 1468.503, 1494.357, 351.8618, 3.115414, 0, 0, 0.9999143, 0.01308903, 0))
        return false;
    if(!AddObject(BG_WS_OBJECT_DOOR_A_4, 179921, 1471.555, 1458.778, 362.6332, 3.115414, 0, 0, 0.9999143, 0.01308903, 0))
        return false;
    if(!AddObject(BG_WS_OBJECT_DOOR_A_5, 180322, 1492.347, 1458.34, 342.3712, -0.03490669, 0, 0, 0.01745246, -0.9998477, 0))
        return false;
    if(!AddObject(BG_WS_OBJECT_DOOR_A_6, 180322, 1503.466, 1493.367, 351.7352, -0.03490669, 0, 0, 0.01745246, -0.9998477, 0))
        return false;
    // horde gates
    if(!AddObject(BG_WS_OBJECT_DOOR_H_1, 179916, 949.1663, 1423.772, 345.6241, -0.5756807, -0.01673368, -0.004956111, -0.2839723, 0.9586737, 0))
        return false;
    if(!AddObject(BG_WS_OBJECT_DOOR_H_2, 179917, 953.0507, 1459.842, 340.6526, -1.99662, -0.1971825, 0.1575096, -0.8239487, 0.5073641, 0))
        return false;
    if(!AddObject(BG_WS_OBJECT_DOOR_H_3, 180322, 949.9523, 1422.751, 344.9273, 0, 0, 0, 0, 1, 0))
        return false;
    if(!AddObject(BG_WS_OBJECT_DOOR_H_4, 180322, 950.7952, 1459.583, 342.1523, 0.05235988, 0, 0, 0.02617695, 0.9996573, 0))
        return false;

    WorldSafeLocsEntry const *sg = sWorldSafeLocsStore.LookupEntry(771);
    if(!sg || !AddSpiritGuide(sg->x, sg->y, sg->z, 3.124139, ALLIANCE))
    {
        sLog.outErrorDb("Failed to spawn spirit guide! BattleGround not created!");
        return false;
    }

    sg = sWorldSafeLocsStore.LookupEntry(772);
    if(!sg || !AddSpiritGuide(sg->x, sg->y, sg->z, 3.193953, HORDE))
    {
        sLog.outErrorDb("Failed to spawn spirit guide! BattleGround not created!");
        return false;
    }

    return true;
}

void BattleGroundWS::Reset()
{
    m_FlagKeepers[0]    = 0;
    m_FlagKeepers[1]    = 0;
    m_FlagState[0]      = BG_WS_FLAG_STATE_ON_BASE;
    m_FlagState[1]      = BG_WS_FLAG_STATE_ON_BASE;
    m_TeamScores[0]     = 0;
    m_TeamScores[1]     = 0;

    SetWinner(2);
    SetStatus(STATUS_WAIT_QUEUE);
    SetStartTime(0);
    SetEndTime(0);
    SetLastResurrectTime(0);
}

void BattleGroundWS::HandleKillPlayer(Player *player, Player *killer)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    // i have no idea, why on earth we call this
    // i made a small change but i commented it, because i'm not sure if it is correct :
    //this->AddPlayerToResurrectQueue(0, player->GetGUID()); in spite of :
    EventPlayerDroppedFlag(player);
}

void BattleGroundWS::HandleDropFlag(Player *player)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    EventPlayerDroppedFlag(player);
}
