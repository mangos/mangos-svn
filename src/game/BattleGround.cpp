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
#include "Creature.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"                                       // for chat messages
#include "Chat.h"
#include "SpellAuras.h"

BattleGround::BattleGround()
{
    m_TypeID            = 0;
    m_InstanceID        = 0;
    m_Status            = 0;
    m_StartTime         = 0;
    m_EndTime           = 0;
    m_LastResurrectTime = 0;
    m_Queue_type        = MAX_BATTLEGROUND_QUEUES;
    m_InvitedAlliance   = 0;
    m_InvitedHorde      = 0;
    m_ArenaType         = 0;
    m_IsArena           = false;
    m_Winner            = 2;
    m_startDelay        = 0;
    m_doorsSpawned      = false;
    m_IsRated           = false;
    m_Name              = "";
    m_LevelMin          = 0;
    m_LevelMax          = 0;

    m_MaxPlayersPerTeam = 0;
    m_MaxPlayers        = 0;
    m_MinPlayersPerTeam = 0;
    m_MinPlayers        = 0;

    m_MapId             = 0;

    m_TeamStartLocX[0]  = 0;
    m_TeamStartLocX[1]  = 0;

    m_TeamStartLocY[0]  = 0;
    m_TeamStartLocY[1]  = 0;

    m_TeamStartLocZ[0]  = 0;
    m_TeamStartLocZ[1]  = 0;

    m_TeamStartLocO[0]  = 0;
    m_TeamStartLocO[1]  = 0;

    m_raids[0]          = NULL;
    m_raids[1]          = NULL;

    m_PlayersCount[0]   = 0;
    m_PlayersCount[1]   = 0;
}

BattleGround::~BattleGround()
{

}

void BattleGround::Update(time_t diff)
{
    bool found = false;

    if(GetPlayersSize() || GetRemovedPlayersSize() || GetReviveQueueSize())
        found = true;

    if(!found)                                              // BG is empty
        return;

    WorldPacket data;

    if(GetRemovedPlayersSize())
    {
        for(std::map<uint64, uint8>::iterator itr = m_RemovedPlayers.begin(); itr != m_RemovedPlayers.end(); ++itr)
        {
            Player *plr = objmgr.GetPlayer(itr->first);
            switch(itr->second)
            {
                //following code is handled by event:
                /*case 0:
                    sBattleGroundMgr.m_BattleGroundQueues[GetTypeID()].RemovePlayer(itr->first);
                    //RemovePlayerFromQueue(itr->first);
                    if(plr)
                    {
                        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetTeam(), plr->GetBattleGroundQueueIndex(m_TypeID), STATUS_NONE, 0, 0);
                        plr->GetSession()->SendPacket(&data);
                    }
                    break;*/
                case 1:                                     // currently in bg and was removed from bg
                    if(plr)
                        RemovePlayer(itr->first, true, true);
                    else
                        RemovePlayer(itr->first, false, false);
                    break;
                case 2:                                     // revive queue
                    RemovePlayerFromResurrectQueue(itr->first);
                    break;
            }
        }
        m_RemovedPlayers.clear();
    }

    // remove offline players from bg after ~5 minutes
    if(GetPlayersSize())
    {
        for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player *plr = objmgr.GetPlayer(itr->first);
            itr->second.LastOnlineTime += diff;

            if(plr)
                itr->second.LastOnlineTime = 0;   // update last online time
            else
                if(itr->second.LastOnlineTime >= MAX_OFFLINE_TIME)                   // 5 minutes
                    m_RemovedPlayers[itr->first] = 1;       // add to remove list (BG)
        }
    }

    m_LastResurrectTime += diff;
    if (m_LastResurrectTime >= RESURRECTION_INTERVAL)
    {
        if(GetReviveQueueSize())
        {
            for(std::map<uint64, std::vector<uint64> >::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
            {
                Creature *sh = NULL;
                for(std::vector<uint64>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
                {
                    Player *plr = objmgr.GetPlayer(*itr2);
                    if(!plr)
                        continue;

                    if (!sh)
                    {
                        sh = ObjectAccessor::Instance().GetCreature(*plr, itr->first);
                        // only for visual effect
                        if (sh)
                            sh->CastSpell(sh, SPELL_SPIRIT_HEAL, true);   // Spirit Heal, effect 117
                    }

                    plr->CastSpell(plr, SPELL_RESURRECTION_VISUAL, true);   // Resurrection visual
                    m_ResurrectQueue.push_back(*itr2);
                }
                (itr->second).clear();
            }

            m_ReviveQueue.clear();
            m_LastResurrectTime = 0;
        }
        else
            // queue is clear and time passed, just update last resurrection time
            m_LastResurrectTime = 0;
    }
    else if (m_LastResurrectTime > 500)    // Resurrect players only half a second later, to see spirit heal effect on NPC
    {
        for(std::vector<uint64>::iterator itr = m_ResurrectQueue.begin(); itr != m_ResurrectQueue.end(); ++itr)
        {
            Player *plr = objmgr.GetPlayer(*itr);
            if(!plr)
                continue;
            plr->ResurrectPlayer(1.0f);
            ObjectAccessor::Instance().ConvertCorpseForPlayer(*itr);
        }
        m_ResurrectQueue.clear();
    }

    if(GetStatus() == STATUS_WAIT_LEAVE)
    {
        // remove all players from battleground after 2 minutes
        m_EndTime += diff;
        if(m_EndTime >= TIME_TO_AUTOREMOVE)                         // 2 minutes
        {
            for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            {
                m_RemovedPlayers[itr->first] = 1;           // add to remove list (BG)
            }
            SetStatus(STATUS_WAIT_QUEUE);
            SetQueueType(MAX_BATTLEGROUND_QUEUES);
            SetWinner(2);

            if(isArena())
            {
                BattleGround *bg = sBattleGroundMgr.GetBattleGround(BATTLEGROUND_AA);
                if(!bg)
                    return;

                bg->SetStatus(STATUS_WAIT_QUEUE);
            }
        }
    }
}

void BattleGround::SetTeamStartLoc(uint32 TeamID, float X, float Y, float Z, float O)
{
    uint8 idx = GetTeamIndexByTeamId(TeamID);
    m_TeamStartLocX[idx] = X;
    m_TeamStartLocY[idx] = Y;
    m_TeamStartLocZ[idx] = Z;
    m_TeamStartLocO[idx] = O;
}

void BattleGround::SendPacketToAll(WorldPacket *packet)
{
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(plr)
            plr->GetSession()->SendPacket(packet);
        else
            sLog.outError("Player " I64FMTD " not found!", itr->first);
    }
}

void BattleGround::SendPacketToTeam(uint32 TeamID, WorldPacket *packet, Player *sender, bool self)
{
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);

        if(!plr)
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
            continue;
        }

        if(!self && sender && (plr->GetGUID() == sender->GetGUID()))
            continue;

        if(plr->GetTeam() == TeamID)
            plr->GetSession()->SendPacket(packet);
    }
}

void BattleGround::PlaySoundToAll(uint32 SoundID)
{
    WorldPacket data;
    sBattleGroundMgr.BuildPlaySoundPacket(&data, SoundID);
    SendPacketToAll(&data);
}

void BattleGround::PlaySoundToTeam(uint32 SoundID, uint32 TeamID)
{
    WorldPacket data;

    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);

        if(!plr)
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
            continue;
        }

        if(plr->GetTeam() == TeamID)
        {
            sBattleGroundMgr.BuildPlaySoundPacket(&data, SoundID);
            plr->GetSession()->SendPacket(&data);
        }
    }
}

void BattleGround::CastSpellOnTeam(uint32 SpellID, uint32 TeamID)
{
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);

        if(!plr)
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
            continue;
        }

        if(plr->GetTeam() == TeamID)
            plr->CastSpell(plr, SpellID, true);
    }
}

void BattleGround::UpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data;
    sBattleGroundMgr.BuildUpdateWorldStatePacket(&data, Field, Value);
    SendPacketToAll(&data);
}

void BattleGround::EndBattleGround(uint32 winner)
{
    WorldPacket data;
    Player *Source = NULL;
    const char *winmsg = "";

    if(winner == ALLIANCE)
    {
        winmsg = LANG_BG_A_WINS;

        PlaySoundToAll(SOUND_ALLIANCE_WINS);                // alliance wins sound...

        SetWinner(1);
    }
    else
    {
        winmsg = LANG_BG_H_WINS;

        PlaySoundToAll(SOUND_HORDE_WINS);                   // horde wins sound...

        SetWinner(0);
    }

    SetStatus(STATUS_WAIT_LEAVE);
    m_EndTime = 0;

    uint32 mark = 0;

    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = objmgr.GetPlayer(itr->first);
        if(!plr)
        {
            sLog.outError("Player " I64FMTD " not found!", itr->first);
            continue;
        }

        if(!plr->isAlive())
        {
            plr->ResurrectPlayer(1.0f);
            plr->SpawnCorpseBones();
        }

        BlockMovement(plr);

        sBattleGroundMgr.BuildPvpLogDataPacket(&data, this);
        plr->GetSession()->SendPacket(&data);

        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetTeam(), plr->GetBattleGroundQueueIndex(m_TypeID), STATUS_IN_PROGRESS, TIME_TO_AUTOREMOVE, GetStartTime());
        plr->GetSession()->SendPacket(&data);

        if(plr->GetTeam() == winner)
        {
            if(!Source)
                Source = plr;
            switch(GetTypeID())
            {
                case BATTLEGROUND_AV:
                    mark = ITEM_AV_MARK_WINNER;
                    break;
                case BATTLEGROUND_WS:
                    mark = ITEM_WSG_MARK_WINNER;
                    break;
                case BATTLEGROUND_AB:
                    mark = ITEM_AB_MARK_WINNER;
                    break;
            }
        }
        else
        {
            switch(GetTypeID())
            {
                case BATTLEGROUND_AV:
                    mark = ITEM_AV_MARK_LOSER;
                    break;
                case BATTLEGROUND_WS:
                    mark = ITEM_WSG_MARK_LOSER;
                    break;
                case BATTLEGROUND_AB:
                    mark = ITEM_AB_MARK_LOSER;
                    break;
            }
        }
        if(mark)
        {
            plr->CastSpell(plr, mark, true);
        }
    }

    if(Source)
    {
        sChatHandler.FillMessageData(&data, Source->GetSession(), CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, Source->GetGUID(), winmsg, NULL);
        SendPacketToAll(&data);
    }
}

void BattleGround::BlockMovement(Player *plr)
{
    WorldPacket data(SMSG_UNKNOWN_794, 8);                  // this must block movement...
    data.append(plr->GetPackGUID());
    plr->GetSession()->SendPacket(&data);
}

void BattleGround::RemovePlayer(uint64 guid, bool Transport, bool SendPacket)
{
    // Remove from lists/maps
    std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.find(guid);
    if(itr != m_Players.end())
    {
        UpdatePlayersCountByTeam(itr->second.Team, true);   // -1 player
        m_Players.erase(itr);
    }

    std::map<uint64, BattleGroundScore*>::iterator itr2 = m_PlayerScores.find(guid);
    if(itr2 != m_PlayerScores.end())
    {
        delete itr2->second;                                 // delete player's score
        m_PlayerScores.erase(itr2);
    }

    for(std::map<uint64, std::vector<uint64> >::iterator itr3 = m_ReviveQueue.begin(); itr3 != m_ReviveQueue.end(); ++itr3)
    {
        std::vector<uint64>::iterator itr4 = (itr3->second).begin();
        for(; itr4 != (itr3->second).end(); ++itr4)
            if (*itr4 == guid)
                break;
        if (itr4 != (itr3->second).end())
        {
            (itr3->second).erase(itr4);
            break;
        }
    }

    Player *plr = objmgr.GetPlayer(guid);

    if(plr && !plr->isAlive())                              // resurrect on exit
    {
        plr->ResurrectPlayer(1.0f);
        plr->SpawnCorpseBones();
    }

    RemovePlayer(plr, guid);                                // BG subclass specific code

    if(plr)
    {
        if(isArena())
        {
            plr->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);
            plr->RemoveAurasDueToSpell(SPELL_ARENA_PREPARATION);
        }

        WorldPacket data;
        if(SendPacket)
        {
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetTeam(), plr->GetBattleGroundQueueIndex(m_TypeID), STATUS_NONE, 0, 0);
            plr->GetSession()->SendPacket(&data);
        }

        // this call is important, because player, when joins to battleground, this method is not called, so it must be called when leaving bg
        plr->RemoveBattleGroundQueueId(m_TypeID);

        if(!plr->GetBattleGroundId())
            return;

        Group * group = plr->GetGroup();

        // remove from raid group if exist
        if(group && group == GetBgRaid(plr->GetTeam()))
        {
            if(!group->RemoveMember(guid, 0))               // group was disbanded
            {
                SetBgRaid(plr->GetTeam(), NULL);
                delete group;
            }
        }

        // Do next only if found in battleground
        plr->SetBattleGroundId(0);                          // We're not in BG.

        // Let others know
        sBattleGroundMgr.BuildPlayerLeftBattleGroundPacket(&data, plr);
        SendPacketToTeam(plr->GetTeam(), &data, plr, false);

        // Log
        sLog.outDetail("BATTLEGROUND: Player %s left the battle.", plr->GetName());

        if(Transport)
        {
            plr->TeleportTo(plr->GetBattleGroundEntryPointMap(), plr->GetBattleGroundEntryPointX(), plr->GetBattleGroundEntryPointY(), plr->GetBattleGroundEntryPointZ(), plr->GetBattleGroundEntryPointO(), true, false);
            //sLog.outDetail("BATTLEGROUND: Sending %s to %f,%f,%f,%f", pl->GetName(), x,y,z,O);
        }

        // Log
        sLog.outDetail("BATTLEGROUND: Removed %s from BattleGround.", plr->GetName());
    }

    /// there will be code which will add battleground to BGFreeSlotQueue , when battleground instance will exist
    // we always should check if BG is in that queue before adding..

    if(!GetPlayersSize())
    {
        SetStatus(STATUS_WAIT_QUEUE);
        if(isArena())
        {
            BattleGround *bg = sBattleGroundMgr.GetBattleGround(BATTLEGROUND_AA);
            if(!bg)
                return;

            bg->SetStatus(STATUS_WAIT_QUEUE);
        }
        SetWinner(2);
        m_Players.clear();
        m_PlayerScores.clear();

        for(uint32 i = 0; i < m_bgobjects.size(); i++)
        {
            SpawnBGObject(i, RESPAWN_ONE_DAY);
        }
        sLog.outDebug("BG: Objects despawned...");
    }
}

void BattleGround::StartBattleGround(time_t diff)
{
    SetStartTime(0);

    SetLastResurrectTime(0);

    WorldPacket data;
    BattleGround *bg = NULL;

    if(GetTypeID() == BATTLEGROUND_AA)
    {
        // we must select one of running arena
        uint8 arenas[3] = { BATTLEGROUND_NA, BATTLEGROUND_BE, BATTLEGROUND_RL };
        uint8 arena_type = arenas[urand(0, 2)];

        bg = sBattleGroundMgr.GetBattleGround(arena_type);
        if(!bg)
            return;

        bg->SetStatus(STATUS_WAIT_JOIN);

        bg->SetStartTime(0);
        bg->SetLastResurrectTime(0);
    }
}

void BattleGround::AddPlayer(Player *plr)
{
    // score struct must be created in inherited class

    uint64 guid = plr->GetGUID();

    BattleGroundPlayer bp;
    bp.LastOnlineTime = 0;
    bp.Team = plr->GetTeam();

    // Add to list/maps
    m_Players[guid] = bp;

    if (m_TypeID == 3)    // Only implemented for AB (also have a look at Player::SendInitWorldSettings() !
        _SendCurrentGameState(plr);
    UpdatePlayersCountByTeam(plr->GetTeam(), false);        // +1 player

    WorldPacket data;
    sBattleGroundMgr.BuildPlayerJoinedBattleGroundPacket(&data, plr);
    SendPacketToTeam(plr->GetTeam(), &data, plr, false);

    if(isArena())
    {
        if(GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
        {
            // temporary disabled
            //plr->CastSpell(plr, SPELL_ARENA_PREPARATION, true);
        }
        plr->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);
    }

    // Log
    sLog.outDetail("BATTLEGROUND: Player %s joined the battle.", plr->GetName());
}

/* This method should be called only once ... it adds pointer to queue */
void BattleGround::AddToBGFreeSlotQueue() const
{
    sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].push_front((BattleGround*) this);
}

/* This method removes this battleground from free queue - it must be called when deleting battleground - not used now*/
void BattleGround::RemoveFromBGFreeSlotQueue()
{
    /* uncomment this code when battlegrounds will work like instances
    for (std::deque<BattleGround*>::iterator itr = sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].begin(); itr != sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].end(); ++itr)
    {
        if ((*itr)->GetInstanceID() == m_InstanceID)
        {
            sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].erase(itr);
            return;
        }
    }*/
}

/*
this method should decide, if we can invite new player of certain team to BG, it is based on BATTLEGROUND_STATUS
*/
bool BattleGround::HasFreeSlotsForTeam(uint32 Team) const
{
    //if BG is starting ... invite anyone:
    if (GetStatus() == STATUS_WAIT_JOIN)
        return GetInvitedCount(Team) < GetMaxPlayersPerTeam();
    //if BG is already started .. do not allow to join too much players of one faction
    uint32 otherTeam;
    if (Team == ALLIANCE)
        otherTeam = GetInvitedCount(HORDE);
    else
        otherTeam = GetInvitedCount(ALLIANCE);
    if (GetStatus() > STATUS_WAIT_JOIN && GetStatus() < STATUS_WAIT_LEAVE)
        return (GetInvitedCount(Team) <= otherTeam && GetInvitedCount(Team) < GetMaxPlayersPerTeam());

    return false;
}

/* this method isn't called already, it will be useful when more battlegrounds of one type will be available */
bool BattleGround::HasFreeSlots() const
{
    return GetPlayersSize() < GetMaxPlayers();
}

void BattleGround::UpdatePlayerScore(Player* Source, uint32 type, uint32 value)
{
    std::map<uint64, BattleGroundScore*>::iterator itr = m_PlayerScores.find(Source->GetGUID());

    if(itr == m_PlayerScores.end())                         // player not found...
        return;

    switch(type)
    {
        case SCORE_KILLS:                                   // kills
            itr->second->KillingBlows += value;
            break;
            //this code is temporarily commented, we should move it into BG subclass
        /*case SCORE_FLAG_CAPTURES:                       // flags captured
            itr->second->FlagCaptures += value;
            break;
        case SCORE_FLAG_RETURNS:                            // flags returned
            itr->second->FlagReturns += value;
            break;*/
        case SCORE_DEATHS:                                  // deaths
            itr->second->Deaths += value;
            break;
        default:
            sLog.outDebug("Unknown player score type %u", type);
            break;
    }
}

void BattleGround::AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid)
{
    m_ReviveQueue[npc_guid].push_back(player_guid);

    Player *plr = objmgr.GetPlayer(player_guid);
    if(!plr)
        return;

    plr->CastSpell(plr, SPELL_WAITING_FOR_RESURRECT, true);
    SpellEntry const *spellInfo = sSpellStore.LookupEntry( SPELL_WAITING_FOR_RESURRECT );
    if(spellInfo)
    {
        Aura *Aur = new Aura(spellInfo, 0, NULL, plr);
        plr->AddAura(Aur);
    }
}

void BattleGround::RemovePlayerFromResurrectQueue(uint64 player_guid)
{
    for(std::map<uint64, std::vector<uint64> >::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
        for(std::vector<uint64>::iterator itr2 =(itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
        {
            if(*itr2 == player_guid)
            {
                (itr->second).erase(itr2);

                Player *plr = objmgr.GetPlayer(player_guid);
                if(!plr)
                    return;

                plr->RemoveAurasDueToSpell(SPELL_WAITING_FOR_RESURRECT);

                return;
            }
        }
}

bool BattleGround::AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 spawntime)
{
    GameObjectInfo const* goinfo = objmgr.GetGameObjectInfo(entry);
    if(!goinfo)
    {
        sLog.outErrorDb("Gameobject template %u not found in database! BattleGround not created!", entry);
        return false;
    }

    uint32 guid = objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT);

    GameObjectData& data = objmgr.NewGOData(guid);

    data.id             = entry;
    data.mapid          = GetMapId();
    data.posX           = x;
    data.posY           = y;
    data.posZ           = z;
    data.orientation    = o;
    data.rotation0      = rotation0;
    data.rotation1      = rotation1;
    data.rotation2      = rotation2;
    data.rotation3      = rotation3;
    data.lootid         = 0;
    data.spawntimesecs  = spawntime;
    data.animprogress   = 100;
    data.dynflags       = 0;
    objmgr.AddGameobjectToGrid(guid, &data);

    m_bgobjects[type] = MAKE_GUID(guid, HIGHGUID_GAMEOBJECT);

    return true;
}

void BattleGround::SpawnBGObject(uint32 type, uint32 respawntime)
{
    if(respawntime == 0)
    {
        GameObject *obj = HashMapHolder<GameObject>::Find(m_bgobjects[type]);
        if(obj)
        {
            //obj->Respawn();                               // bugged
            obj->SetRespawnTime(0);
            objmgr.SaveGORespawnTime(obj->GetGUIDLow(), 0, 0);
        }
        else
            objmgr.SaveGORespawnTime(GUID_LOPART(m_bgobjects[type]), 0, 0);
    }
    else
    {
        GameObject *obj = HashMapHolder<GameObject>::Find(m_bgobjects[type]);
        if(obj)
        {
            obj->SetRespawnTime(respawntime);
            obj->SetLootState(GO_LOOTED);
        }
        else
            objmgr.SaveGORespawnTime(GUID_LOPART(m_bgobjects[type]), 0, time(NULL) + respawntime);
    }
}

bool BattleGround::AddSpiritGuide(float x, float y, float z, float o, uint32 team)
{
    uint32 entry = 0;

    if(team == ALLIANCE)
        entry = 13116;
    else
        entry = 13117;

    CreatureInfo const* cinfo = objmgr.GetCreatureTemplate(entry);
    if(!cinfo)
    {
        sLog.outErrorDb("Creature template %u not found. BattleGround not created!", entry);
        return false;
    }

    uint32 guid = objmgr.GenerateLowGuid(HIGHGUID_UNIT);

    CreatureData& data = objmgr.NewCreatureData(guid);

    data.id             = entry;
    data.mapid          = GetMapId();
    data.posX           = x;
    data.posY           = y;
    data.posZ           = z;
    data.orientation    = o;
    data.spawntimesecs  = 0;
    data.spawndist      = 0;
    data.currentwaypoint= 0;
    data.spawn_posX     = x;
    data.spawn_posY     = y;
    data.spawn_posZ     = z;
    data.spawn_orientation = o;
    data.curhealth      = cinfo->maxhealth;
    data.curmana        = cinfo->maxmana;
    data.deathState     = 0;
    data.movementType   = cinfo->MovementType;
    data.auras          = "";
    objmgr.AddCreatureToGrid(guid, &data);

    m_SpiritGuides.push_back(MAKE_GUID(guid, HIGHGUID_UNIT));
    /*
        // set some fields to off like values
        pCreature->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, pCreature->GetGUID());
        // aura
        pCreature->SetUInt32Value(UNIT_FIELD_AURA, SPELL_SPIRIT_HEAL_CHANNEL);
        pCreature->SetUInt32Value(UNIT_FIELD_AURAFLAGS, 0x00000009);
        pCreature->SetUInt32Value(UNIT_FIELD_AURALEVELS, 0x0000003C);
        pCreature->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS, 0x000000FF);
        // casting visual effect
        pCreature->SetUInt32Value(UNIT_CHANNEL_SPELL, SPELL_SPIRIT_HEAL_CHANNEL);
        // correct cast speed
        pCreature->SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);
    */
    return true;
}
