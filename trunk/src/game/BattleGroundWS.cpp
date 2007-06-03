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
#include "Spell.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"

BattleGroundWS::BattleGroundWS()
{
    m_AllianceFlagPickerGUID = 0;
    m_HordeFlagPickerGUID = 0;

    m_HordeFlag = NULL;
    m_AllianceFlag = NULL;
    SpeedBonus1 = NULL;
    SpeedBonus2 = NULL;
    RegenBonus1 = NULL;
    RegenBonus2 = NULL;
    BerserkBonus1 = NULL;
    BerserkBonus2 = NULL;
    SpeedBonus1Spawn[0] = 0;
    SpeedBonus1Spawn[1] = 0;
    SpeedBonus2Spawn[0] = 0;
    SpeedBonus2Spawn[1] = 0;
    RegenBonus1Spawn[0] = 0;
    RegenBonus1Spawn[1] = 0;
    RegenBonus2Spawn[0] = 0;
    RegenBonus2Spawn[1] = 0;
    BerserkBonus1Spawn[0] = 0;
    BerserkBonus1Spawn[1] = 0;
    BerserkBonus2Spawn[0] = 0;
    BerserkBonus2Spawn[1] = 0;
    AllianceFlagSpawn[0] = 0;
    AllianceFlagSpawn[1] = 0;
    HordeFlagSpawn[0] = 0;
    HordeFlagSpawn[1] = 0;
}

BattleGroundWS::~BattleGroundWS()
{
    delete m_HordeFlag;
    delete m_AllianceFlag;
    delete SpeedBonus1;
    delete SpeedBonus2;
    delete RegenBonus1;
    delete RegenBonus2;
    delete BerserkBonus1;
    delete BerserkBonus2;
}

void BattleGroundWS::Update(time_t diff)
{
    BattleGround::Update(diff);
    //If BG-Status = WAIT_JOIN and Min players in BG, we must start BG
    if(GetStatus() == STATUS_WAIT_JOIN && GetPlayersSize() >= GetMinPlayers() )
    {
        AllianceFlagSpawn[1] = 1;
        HordeFlagSpawn[1] = 1;
        sLog.outDebug("Flags activated...");
        MapManager::Instance().GetMap(m_AllianceFlag->GetMapId(), m_AllianceFlag)->Add(m_AllianceFlag);
        MapManager::Instance().GetMap(m_HordeFlag->GetMapId(), m_HordeFlag)->Add(m_HordeFlag);
        sLog.outDebug("Flags respawned...");
        SpeedBonus1Spawn[1] = 1;
        SpeedBonus2Spawn[1] = 1;
        RegenBonus1Spawn[1] = 1;
        RegenBonus2Spawn[1] = 1;
        BerserkBonus1Spawn[1] = 1;
        BerserkBonus2Spawn[1] = 1;
        sLog.outDebug("Bonuses activated...");
        MapManager::Instance().GetMap(SpeedBonus1->GetMapId(), SpeedBonus1)->Add(SpeedBonus1);
        MapManager::Instance().GetMap(SpeedBonus2->GetMapId(), SpeedBonus2)->Add(SpeedBonus2);
        MapManager::Instance().GetMap(RegenBonus1->GetMapId(), RegenBonus1)->Add(RegenBonus1);
        MapManager::Instance().GetMap(RegenBonus2->GetMapId(), RegenBonus2)->Add(RegenBonus2);
        MapManager::Instance().GetMap(BerserkBonus1->GetMapId(), BerserkBonus1)->Add(BerserkBonus1);
        MapManager::Instance().GetMap(BerserkBonus2->GetMapId(), BerserkBonus2)->Add(BerserkBonus2);
        sLog.outDebug("Bonuses respawned...");
        SetStatus(STATUS_IN_PROGRESS);
    }

    if(GetStatus() == STATUS_IN_PROGRESS)
    {
        // Flags timers
        AllianceFlagSpawn[0] -= diff;
        HordeFlagSpawn[0] -= diff;
        if(AllianceFlagSpawn[0] < 0)
            AllianceFlagSpawn[0] = 0;
        if(HordeFlagSpawn[0] < 0)
            HordeFlagSpawn[0] = 0;
        if(AllianceFlagSpawn[0] == 0 && AllianceFlagSpawn[1] == 0)
        {
            //MapManager::Instance().GetMap(m_AllianceFlag->GetMapId(), m_AllianceFlag)->Add(m_AllianceFlag);
            RespawnFlag(ALLIANCE, true);
            AllianceFlagSpawn[1] = 1;                       // spawned
        }
        if(HordeFlagSpawn[0] == 0 && HordeFlagSpawn[1] == 0)
        {
            //MapManager::Instance().GetMap(m_HordeFlag->GetMapId(), m_HordeFlag)->Add(m_HordeFlag);
            RespawnFlag(HORDE, true);
            HordeFlagSpawn[1] = 1;                          // spawned
        }
        //Bonuses timers
        SpeedBonus1Spawn[0] -= diff;
        SpeedBonus2Spawn[0] -= diff;
        RegenBonus1Spawn[0] -= diff;
        RegenBonus2Spawn[0] -= diff;
        BerserkBonus1Spawn[0] -= diff;
        BerserkBonus2Spawn[0] -= diff;
        if(SpeedBonus1Spawn[0] < 0)
            SpeedBonus1Spawn[0] = 0;
        if(SpeedBonus2Spawn[0] < 0)
            SpeedBonus2Spawn[0] = 0;
        if(RegenBonus1Spawn[0] < 0)
            RegenBonus1Spawn[0] = 0;
        if(RegenBonus2Spawn[0] < 0)
            RegenBonus2Spawn[0] = 0;
        if(BerserkBonus1Spawn[0] < 0)
            BerserkBonus1Spawn[0] = 0;
        if(BerserkBonus2Spawn[0] < 0)
            BerserkBonus2Spawn[0] = 0;
        //If Timer==0 && SpawnStatus==0 then respawn
        if(SpeedBonus1Spawn[0] == 0 && SpeedBonus1Spawn[1] == 0)
        {
            MapManager::Instance().GetMap(SpeedBonus1->GetMapId(), SpeedBonus1)->Add(SpeedBonus1);
            SpeedBonus1Spawn[1] = 1;
        }
        if(SpeedBonus2Spawn[0] == 0 && SpeedBonus2Spawn[1] == 0)
        {
            MapManager::Instance().GetMap(SpeedBonus2->GetMapId(), SpeedBonus2)->Add(SpeedBonus2);
            SpeedBonus2Spawn[1] = 1;
        }
        if(RegenBonus1Spawn[0] == 0 && RegenBonus1Spawn[1] == 0)
        {
            MapManager::Instance().GetMap(RegenBonus1->GetMapId(), RegenBonus1)->Add(RegenBonus1);
            RegenBonus1Spawn[1] = 1;
        }
        if(RegenBonus2Spawn[0] == 0 && RegenBonus2Spawn[1] == 0)
        {
            MapManager::Instance().GetMap(RegenBonus2->GetMapId(), RegenBonus2)->Add(RegenBonus2);
            RegenBonus2Spawn[1] = 1;
        }
        if(BerserkBonus1Spawn[0] == 0 && BerserkBonus1Spawn[1] == 0)
        {
            MapManager::Instance().GetMap(BerserkBonus1->GetMapId(), BerserkBonus1)->Add(BerserkBonus1);
            BerserkBonus1Spawn[1] = 1;
        }
        if(BerserkBonus2Spawn[0] == 0 && BerserkBonus2Spawn[1] == 0)
        {
            MapManager::Instance().GetMap(BerserkBonus2->GetMapId(), BerserkBonus2)->Add(BerserkBonus2);
            BerserkBonus2Spawn[1] = 1;
        }
    }
}

void BattleGroundWS::RespawnFlag(uint32 Team, bool captured)
{
    // need delay between flag capture and it's respawn about 30-60 seconds...
    if(Team == ALLIANCE)
    {
        sLog.outDebug("Respawn Alliance flag");
        MapManager::Instance().GetMap(m_AllianceFlag->GetMapId(), m_AllianceFlag)->Add(m_AllianceFlag);
    }

    if(Team == HORDE)
    {
        sLog.outDebug("Respawn Horde flag");
        MapManager::Instance().GetMap(m_HordeFlag->GetMapId(), m_HordeFlag)->Add(m_HordeFlag);
    }

    if(captured)
    {
        WorldPacket data;
        const char *message = LANG_BG_F_PLACED;
        sChatHandler.FillMessageData(&data, NULL, CHAT_MSG_BATTLEGROUND, LANG_UNIVERSAL, NULL, 0, message, NULL);
        SendPacketToAll(&data);

        PlaySoundToAll(8232);                               // flags respawned sound...
    }
}

void BattleGroundWS::EventPlayerCapturedFlag(Player *Source)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint8 type = 0;
    bool win = false;
    uint32 winner = 0;
    const char *message = "";

    if(Source->GetTeam() == ALLIANCE)
    {
        SetHordeFlagPicker(0);                              // must be before aura remove to prevent 2 events (drop+capture) at the same time
        Source->RemoveAurasDueToSpell(23333);               // Drop Horde Flag from Player
        message = LANG_BG_CAPTURED_HF;
        type = CHAT_MSG_BATTLEGROUND_HORDE;
        if(GetTeamScore(ALLIANCE) < 3)
            AddPoint(ALLIANCE, 1);
        PlaySoundToAll(8173);
    }
    if(Source->GetTeam() == HORDE)
    {
        SetAllianceFlagPicker(0);                           // must be before aura remove to prevent 2 events (drop+capture) at the same time
        Source->RemoveAurasDueToSpell(23335);               // Drop Alliance Flag from Player
        message = LANG_BG_CAPTURED_AF;
        type = CHAT_MSG_BATTLEGROUND_ALLIANCE;
        if(GetTeamScore(HORDE) < 3)
            AddPoint(HORDE, 1);
        PlaySoundToAll(8213);
    }

    WorldPacket data;
    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    UpdateFlagState(Source->GetTeam(), 1);
    UpdateTeamScore(Source->GetTeam());
    UpdatePlayerScore(Source, 1, 3);                        // +3 kills for flag capture...
    UpdatePlayerScore(Source, 2, 1);                        // +1 flag captures...

    if(GetTeamScore(ALLIANCE) == 3)
    {
        win = true;
        winner = ALLIANCE;
    }

    if(GetTeamScore(HORDE) == 3)
    {
        win = true;
        winner = HORDE;
    }

    if(win)
        EndBattleGround(winner);
    else
    {
        switch(Source->GetTeam())
        {
            case ALLIANCE:
                HordeFlagSpawn[0] = 1*60*1000;
                HordeFlagSpawn[1] = 0;
                //RespawnFlag(HORDE, true);
                break;
            case HORDE:
                AllianceFlagSpawn[0] = 1*60*1000;
                AllianceFlagSpawn[1] = 0;
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
        message = LANG_BG_DROPPED_HF;
        type = CHAT_MSG_BATTLEGROUND_ALLIANCE;
    }
    if(Source->GetTeam() == HORDE)
    {
        SetAllianceFlagPicker(0);
        message = LANG_BG_DROPPED_AF;
        type = CHAT_MSG_BATTLEGROUND_HORDE;
    }

    UpdateFlagState(Source->GetTeam(), 1);

    WorldPacket data;
    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    if(Source->GetTeam() == ALLIANCE)
        UpdateWorldState(1546, uint32(-1));
    if(Source->GetTeam() == HORDE)
        UpdateWorldState(1545, uint32(-1));
}

void BattleGroundWS::EventPlayerReturnedFlag(Player *Source)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    const char *message;
    uint8 type = 0;

    if(Source->GetTeam() == ALLIANCE)
    {
        message = LANG_BG_RETURNED_AF;
        type = CHAT_MSG_BATTLEGROUND_HORDE;
        UpdateFlagState(HORDE, 1);
        RespawnFlag(ALLIANCE, false);
    }
    if(Source->GetTeam() == HORDE)
    {
        message = LANG_BG_RETURNED_HF;
        type = CHAT_MSG_BATTLEGROUND_ALLIANCE;
        UpdateFlagState(ALLIANCE, 1);
        RespawnFlag(HORDE, false);
    }

    PlaySoundToAll(8192);                                   // flag returned (common sound)
    UpdatePlayerScore(Source, 3, 1);                        // +1 to flag returns...

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
        type = CHAT_MSG_BATTLEGROUND_HORDE;
        PlaySoundToAll(8212);
        MapManager::Instance().GetMap(m_HordeFlag->GetMapId(), m_HordeFlag)->Remove(m_HordeFlag, false);
        SetHordeFlagPicker(Source->GetGUID());              // pick up Horde Flag
    }
    if(Source->GetTeam() == HORDE)
    {
        message = LANG_BG_PICKEDUP_AF;
        type = CHAT_MSG_BATTLEGROUND_ALLIANCE;
        PlaySoundToAll(8174);
        MapManager::Instance().GetMap(m_AllianceFlag->GetMapId(), m_AllianceFlag)->Remove(m_AllianceFlag, false);
        SetAllianceFlagPicker(Source->GetGUID());           // pick up Alliance Flag
    }

    WorldPacket data;
    sChatHandler.FillMessageData(&data, Source->GetSession(), type, LANG_UNIVERSAL, NULL, Source->GetGUID(), message, NULL);
    SendPacketToAll(&data);

    UpdateFlagState(Source->GetTeam(), 2);

    if(Source->GetTeam() == ALLIANCE)
        UpdateWorldState(1546, 1);
    if(Source->GetTeam() == HORDE)
        UpdateWorldState(1545, 1);
}

void BattleGroundWS::RemovePlayer(Player *plr,uint64 guid)
{
    if(plr)
    {
        if(IsAllianceFlagPickedup() || IsHordeFlagPickedup())
        {
            // drop flag...
            if(m_AllianceFlagPickerGUID == guid)
                plr->RemoveAurasDueToSpell(23335);
            if(m_HordeFlagPickerGUID == guid)
                plr->RemoveAurasDueToSpell(23333);
        }
    }
    else
    {
        // check this case...
        if(m_AllianceFlagPickerGUID == guid)
        {
            //AllianceFlagSpawn[0] = 0;
            //AllianceFlagSpawn[1] = 1;
            SetAllianceFlagPicker(0);
            RespawnFlag(ALLIANCE, false);
        }
        if(m_HordeFlagPickerGUID == guid)
        {
            //HordeFlagSpawn[0] = 0;
            //HordeFlagSpawn[1] = 1;
            SetHordeFlagPicker(0);
            RespawnFlag(HORDE, false);
        }
    }
    if(!GetPlayersSize())
    {
        MapManager::Instance().GetMap(m_AllianceFlag->GetMapId(), m_AllianceFlag)->Remove(m_AllianceFlag, false);
        MapManager::Instance().GetMap(m_HordeFlag->GetMapId(), m_HordeFlag)->Remove(m_HordeFlag, false);
        sLog.outDebug("Flags despawned...");
        MapManager::Instance().GetMap(SpeedBonus1->GetMapId(), SpeedBonus1)->Remove(SpeedBonus1, false);
        MapManager::Instance().GetMap(SpeedBonus2->GetMapId(), SpeedBonus2)->Remove(SpeedBonus2, false);
        MapManager::Instance().GetMap(RegenBonus1->GetMapId(), RegenBonus1)->Remove(RegenBonus1, false);
        MapManager::Instance().GetMap(RegenBonus2->GetMapId(), RegenBonus2)->Remove(RegenBonus2, false);
        MapManager::Instance().GetMap(BerserkBonus1->GetMapId(), BerserkBonus1)->Remove(BerserkBonus1, false);
        MapManager::Instance().GetMap(BerserkBonus2->GetMapId(), BerserkBonus2)->Remove(BerserkBonus2, false);
        sLog.outDebug("Bonuses despawned...");
    }
}

void BattleGroundWS::HandleAreaTrigger(Player* Source, uint32 Trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint32 SpellId = 0;
    switch(Trigger)
    {
        case 3686:                                          //Alliance elixir of speed spawn. Trigger not working, because located inside other areatrigger, can be replaced by IsWithinDist(object, dist) in BattleGround::Update().
            sLog.outDebug("SpeedBonus1SpawnState = %i, SpeedBonus1SpawnTimer = %i", SpeedBonus1Spawn[1], SpeedBonus1Spawn[0]);
            if(SpeedBonus1Spawn[1] == 0)
            {
                break;
            }
            MapManager::Instance().GetMap(SpeedBonus1->GetMapId(), SpeedBonus1)->Remove(SpeedBonus1, false);
            SpeedBonus1Spawn[0] = 3*60*1000;                // 3 minutes
            SpeedBonus1Spawn[1] = 0;
            SpellId = 23451;
            break;
        case 3687:                                          //Horde elixir of speed spawn. Trigger not working, because located inside other areatrigger, can be replaced by IsWithinDist(object, dist) in BattleGround::Update().
            sLog.outDebug("SpeedBonus2SpawnState = %i, SpeedBonus2SpawnTimer = %i", SpeedBonus2Spawn[1], SpeedBonus2Spawn[0]);
            if(SpeedBonus2Spawn[1] == 0)
            {
                break;
            }
            MapManager::Instance().GetMap(SpeedBonus2->GetMapId(), SpeedBonus2)->Remove(SpeedBonus2, false);
            SpeedBonus2Spawn[0] = 3*60*1000;                // 3 minutes
            SpeedBonus2Spawn[1] = 0;
            SpellId = 23451;
            break;
        case 3706:                                          //Alliance elixir of regeneration spawn
            sLog.outDebug("RegenBonus1SpawnState = %u, RegenBonus1SpawnTimer = %u", RegenBonus1Spawn[1], RegenBonus1Spawn[0]);
            if(RegenBonus1Spawn[1] == 0)
            {
                break;
            }
            MapManager::Instance().GetMap(RegenBonus1->GetMapId(), RegenBonus1)->Remove(RegenBonus1, false);
            RegenBonus1Spawn[0] = 3*60*1000;                // 3 minutes
            RegenBonus1Spawn[1] = 0;
            SpellId = 23493;
            break;
        case 3708:                                          //Horde elixir of regeneration spawn
            sLog.outDebug("RegenBonus2SpawnState = %u, RegenBonus2SpawnTimer = %u", RegenBonus2Spawn[1], RegenBonus2Spawn[0]);
            if(RegenBonus2Spawn[1] == 0)
            {
                break;
            }
            MapManager::Instance().GetMap(RegenBonus2->GetMapId(), RegenBonus2)->Remove(RegenBonus2, false);
            RegenBonus2Spawn[0] = 3*60*1000;                // 3 minutes
            RegenBonus2Spawn[1] = 0;
            SpellId = 23493;
            break;
        case 3707:                                          //Alliance elixir of berserk spawn
            sLog.outDebug("BerserkBonus1SpawnState = %u, BerserkBonus1SpawnTimer = %u", BerserkBonus1Spawn[1], BerserkBonus1Spawn[0]);
            if(BerserkBonus1Spawn[1] == 0)
            {
                break;
            }
            MapManager::Instance().GetMap(BerserkBonus1->GetMapId(), BerserkBonus1)->Remove(BerserkBonus1, false);
            BerserkBonus1Spawn[0] = 3*60*1000;              // 3 minutes
            BerserkBonus1Spawn[1] = 0;
            SpellId = 23505;
            break;
        case 3709:                                          //Horde elixir of berserk spawn
            sLog.outDebug("BerserkBonus2SpawnState = %u, BerserkBonus2SpawnTimer = %u", BerserkBonus2Spawn[1], BerserkBonus2Spawn[0]);
            if(BerserkBonus2Spawn[1] == 0)
            {
                break;
            }
            MapManager::Instance().GetMap(BerserkBonus2->GetMapId(), BerserkBonus2)->Remove(BerserkBonus2, false);
            BerserkBonus2Spawn[0] = 3*60*1000;              // 3 minutes
            BerserkBonus2Spawn[1] = 0;
            SpellId = 23505;
            break;
        case 3646:                                          //Alliance Flag spawn
            if(IsHordeFlagPickedup() && !IsAllianceFlagPickedup())
            {
                if(GetHordeFlagPickerGUID() == Source->GetGUID())
                {
                    EventPlayerCapturedFlag(Source);
                }
            }
            break;
        case 3647:                                          //Horde Flag spawn
            if(IsAllianceFlagPickedup() && !IsHordeFlagPickedup())
            {
                if(GetAllianceFlagPickerGUID() == Source->GetGUID())
                {
                    EventPlayerCapturedFlag(Source);
                }
            }
            break;
        case 4628:                                          // new 2.1.0?
        case 4629:                                          // new 2.1.0?
        case 4631:                                          // Unk1
        case 4633:                                          // Unk2
            break;
        case 3669:                                          // Warsong Gulch Horde Exit (removed, but trigger still exist).
        case 3671:                                          // Warsong Gulch Alliance Exit (removed, but trigger still exist).
            break;
        default:
        {
            sLog.outError("WARNING: Unhandled AreaTrigger in Battleground: %d", Trigger);
            Source->GetSession()->SendAreaTriggerMessage("Warning: Unhandled AreaTrigger in Battleground: %u", Trigger);
            break;
        }
    }

    if(SpellId)
    {
        SpellEntry const *Entry = sSpellStore.LookupEntry(SpellId);

        if(!Entry)
        {
            sLog.outError("ERROR: Tried to add unknown spell id %d to plr.", SpellId);
            return;
        }

        Source->CastSpell(Source, Entry, true, 0);
    }
}

void BattleGroundWS::SetupBattleGround()
{
    m_AllianceFlag = new GameObject(NULL);
    m_HordeFlag = new GameObject(NULL);
    SpeedBonus1 = new GameObject(NULL);
    SpeedBonus2 = new GameObject(NULL);
    RegenBonus1 = new GameObject(NULL);
    RegenBonus2 = new GameObject(NULL);
    BerserkBonus1 = new GameObject(NULL);
    BerserkBonus2 = new GameObject(NULL);

    m_AllianceFlag->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179830, GetMapId(), 1540.35, 1481.31, 352.635, 6.24, 0, 0, sin(6.24/2), cos(6.24/2), 0, 0);
    m_HordeFlag->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179831, GetMapId(), 915.809, 1433.73, 346.172, 3.244, 0, 0, sin(3.244/2), cos(3.244/2), 0, 0);
    SpeedBonus1->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179871, GetMapId(), 1449.98, 1468.86, 342.66, 4.866, 0, 0, sin(4.866/2), cos(4.866/2), 0, 0);
    SpeedBonus2->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179871, GetMapId(), 1006.22, 1445.98, 335.77, 1.683, 0, 0, sin(1.683/2), cos(1.683/2), 0, 0);
    RegenBonus1->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179904, GetMapId(), 1316.94, 1551.99, 313.234, 5.869, 0, 0, sin(5.869/2), cos(5.869/2), 0, 0);
    RegenBonus2->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179904, GetMapId(), 1110.1, 1353.24, 316.513, 5.68, 0, 0, sin(5.68/2), cos(5.68/2), 0, 0);
    BerserkBonus1->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179905, GetMapId(), 1318.68, 1378.03, 314.753, 1.001, 0, 0, sin(1.001/2), cos(1.001/2), 0, 0);
    BerserkBonus2->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 179905, GetMapId(), 1141.36, 1560.99, 306.791, 3.858, 0, 0, sin(3.858/2), cos(3.858/2), 0, 0);
}
