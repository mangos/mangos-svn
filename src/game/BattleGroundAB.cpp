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
#include "BattleGroundAB.h"
#include "Creature.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"

BattleGroundAB::BattleGroundAB()
{
    /* code commented because, it isn't implemented for AB yet
    m_BgObjects.resize(BG_AB_OBJECT_MAX);
    //used to store actual spirit guides GUIDs
    m_BgCreatures.resize(BG_AB_NODES_COUNT+2);*/
}

BattleGroundAB::~BattleGroundAB()
{
}

void BattleGroundAB::Update(time_t diff)
{
    BattleGround::Update(diff);

    if (GetStatus() == STATUS_WAIT_JOIN && GetPlayersSize())
    {
        ModifyStartDelayTime(diff);

        if (!(m_Events & 0x01))
        {
            m_Events |= 0x01;

            sLog.outDebug("Arathi Basin: entering state STATUS_WAIT_JOIN ...");

            // Starting doors
            MapManager::Instance().GetMap(GetMapId(), m_bgobjects[BG_AB_OBJECT_GATE_A])->Add(m_bgobjects[BG_AB_OBJECT_GATE_A]);
            MapManager::Instance().GetMap(GetMapId(), m_bgobjects[BG_AB_OBJECT_GATE_H])->Add(m_bgobjects[BG_AB_OBJECT_GATE_H]);
            // Starting base spirit guides
            _NodeOccupied(5);
            _NodeOccupied(6);

            SetStartDelayTime(START_DELAY0);
        }
        // After 1 minute, warning is signalled
        else if (GetStartDelayTime() <= START_DELAY1 && !(m_Events & 0x04))
        {
            m_Events |= 0x04;
            SendMessageToAll(LANG_BG_AB_ONEMINTOSTART);
        }
        // After 1,5 minute, warning is signalled
        else if (GetStartDelayTime() <= START_DELAY2 && !(m_Events & 0x08))
        {
            m_Events |= 0x08;
            SendMessageToAll(LANG_BG_AB_HALFMINTOSTART);
        }
        // After 2 minutes, gates OPEN ! x)
        else if (GetStartDelayTime() <= 0 && !(m_Events & 0x10))
        {
            m_Events |= 0x10;
            SendMessageToAll(LANG_BG_AB_STARTED);

            // Neutral banners
            for (int banner = BG_AB_OBJECT_BANNER_NEUTRAL, i = 0; i < 5; banner += 8, ++i)
                MapManager::Instance().GetMap(GetMapId(), m_bgobjects[banner])->Add(m_bgobjects[banner]);

            m_bgobjects[BG_AB_OBJECT_GATE_A]->SetUInt32Value(GAMEOBJECT_FLAGS,33);
            m_bgobjects[BG_AB_OBJECT_GATE_A]->SetUInt32Value(GAMEOBJECT_STATE,0);
            m_bgobjects[BG_AB_OBJECT_GATE_H]->SetUInt32Value(GAMEOBJECT_FLAGS,33);
            m_bgobjects[BG_AB_OBJECT_GATE_H]->SetUInt32Value(GAMEOBJECT_STATE,0);

            PlaySoundToAll(SOUND_BG_START);
            SetStatus(STATUS_IN_PROGRESS);
        }

    }
    else if(GetStatus() == STATUS_IN_PROGRESS)
    {
        // 3 sec delay to spawn new banner instead previous despawned one
        for (int node = 0; node < 5; ++node)
            if (m_BannerTimers[node].timer)
                if (m_BannerTimers[node].timer > diff)
                    m_BannerTimers[node].timer -= diff;
                else
                {
                    m_BannerTimers[node].timer = 0;
                    _CreateBanner(node, m_BannerTimers[node].type, m_BannerTimers[node].teamIndex, false);
                }

        // 1-minute to occupy a node from contested state
        for (int node = 0; node < 5; ++node)
            if (m_NodeTimers[node])
                if (m_NodeTimers[node] > diff)
                    m_NodeTimers[node] -= diff;
                else
                {
                    m_NodeTimers[node] = 0;
                    // Change from contested to occupied !
                    uint8 teamIndex = m_Nodes[node]-1;
                    m_prevNodes[node] = m_Nodes[node];
                    m_Nodes[node] += 2;
                    // burn current contested banner
                    _DelBanner(node, BG_AB_NODE_TYPE_CONTESTED, teamIndex);
                    // create new occupied banner
                    _CreateBanner(node, BG_AB_BODE_TYPE_OCCUPIED, teamIndex, true);
                    _SendNodeUpdate(node);
                    _NodeOccupied(node);
                    // Message to chatlog
                    char buf[256];
                    uint8 type = (teamIndex == 0) ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE;
                    sprintf(buf, LANG_BG_AB_NODE_TAKEN, (teamIndex == 0) ? LANG_BG_AB_ALLY : LANG_BG_AB_HORDE, _GetNodeName(node));
                    WorldPacket data;
                    ChatHandler::FillMessageData(&data, NULL, type, LANG_UNIVERSAL, NULL, 0, buf, NULL);
                    SendPacketToAll(&data);
                    PlaySoundToAll((teamIndex == 0) ? SOUND_NODE_CAPTURED_ALLIANCE : SOUND_NODE_CAPTURED_HORDE);
                }

        // Accumulate points
        for (int team = 0; team < 2; ++team)
        {
            int points = 0;
            // Go throught all nodes
            for (int node = 0; node < BG_AB_NODES_COUNT; ++node)
                if (m_Nodes[node] == team + 3)
                    ++points;
            if (!points)
                continue;
            m_lastTick[team] += diff;
            if (m_lastTick[team] > BG_AB_TickIntervals[points])
            {
                m_lastTick[team] -= BG_AB_TickIntervals[points];
                m_TeamScores[team] += BG_AB_TickPoints[points];
                m_HonorScoreTics[team] += BG_AB_TickPoints[points];
                m_ReputationScoreTics[team] += BG_AB_TickPoints[points];
                if(m_ReputationScoreTics[team] >= 200)
                {
                    (team == BG_TEAM_ALLIANCE) ? RewardReputationToTeam(509, 10, ALLIANCE) : RewardReputationToTeam(510, 10, HORDE);
                    m_ReputationScoreTics[team] -= 200;
                }
                if(m_HonorScoreTics[team] >= 330)
                {
                    RewardHonorToTeam(20, team);
                    m_HonorScoreTics[team] = -330;
                }
                if (!m_IsInformedNearVictory && m_TeamScores[team] > 1800)
                {
                    if(team == BG_TEAM_ALLIANCE)
                        SendMessageToAll(LANG_BG_AB_A_NEAR_VICTORY);
                    else
                        SendMessageToAll(LANG_BG_AB_H_NEAR_VICTORY);
                    PlaySoundToAll(SOUND_NEAR_VICTORY);
                    m_IsInformedNearVictory = true;
                }

                if (m_TeamScores[team] > 2000)
                    m_TeamScores[team] = 2000;
                if(team == BG_TEAM_ALLIANCE)
                    UpdateWorldState(BG_AB_OP_RESOURCES_ALLY, m_TeamScores[team]);
                if(team == BG_TEAM_HORDE)
                    UpdateWorldState(BG_AB_OP_RESOURCES_HORDE, m_TeamScores[team]);
            }
        }

        // Test win condition
        if (m_TeamScores[BG_TEAM_ALLIANCE] >= 2000)
            EndBattleGround(ALLIANCE);
        if (m_TeamScores[BG_TEAM_HORDE] >= 2000)
            EndBattleGround(HORDE);

        // Buff objects checking
        m_buffchecktimer += diff;
        if (m_buffchecktimer > BG_AB_BUFFCHECK_TIME)
        {
            for (uint8 node = 0; node < BG_AB_NODES_COUNT; ++node)
            {
                // respawn timer caring
                if (m_bgbuffobjects[node].timer > 0)
                {
                    if (m_bgbuffobjects[node].timer < m_buffchecktimer)
                        m_bgbuffobjects[node].timer = 0;
                    else
                        m_bgbuffobjects[node].timer -= m_buffchecktimer;

                // actual respawning
                } else if (m_Nodes[node] && !m_bgbuffobjects[node].object)    // only spawn buff on half- or fully occupied nodes
                {
                    uint8 buff = urand(0, 2);
                    m_bgbuffobjects[node].object = new GameObject(NULL);
                    if (!m_bgbuffobjects[node].object->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), BG_AB_BuffObjects[buff][0], GetMapId(), BG_AB_BuffPositions[node][0], BG_AB_BuffPositions[node][1], BG_AB_BuffPositions[node][2], BG_AB_BuffPositions[node][3], 0, 0, sin(BG_AB_BuffPositions[node][3]/2), cos(BG_AB_BuffPositions[node][3]/2), 0, 0))
                        ASSERT(0);
                    m_bgbuffobjects[node].spellId = BG_AB_BuffObjects[buff][1];
                    MapManager::Instance().GetMap(GetMapId(), m_bgbuffobjects[node].object)->Add(m_bgbuffobjects[node].object);

                // pickup checking
                } else if (m_bgbuffobjects[node].object)
                    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
                    {
                        Player *plr = objmgr.GetPlayer(itr->first);
                        if(!plr)
                        {
                            sLog.outError("Player " I64FMTD " not found!", itr->first);
                            continue;
                        }
                        if (plr->isAlive() && plr->IsWithinDistInMap(m_bgbuffobjects[node].object , 2))    // 2 yard radius to pick it up
                        {
                            SpellEntry const *spell = sSpellStore.LookupEntry(m_bgbuffobjects[node].spellId);
                            if (spell)
                                plr->CastSpell(plr, spell, true, 0);
                            m_bgbuffobjects[node].timer = BG_AB_BUFF_RESPAWN_TIME;
                            m_bgbuffobjects[node].object->Delete();
                            m_bgbuffobjects[node].object = NULL;
                            break;
                        }
                    }
            }
            m_buffchecktimer = 0;
        }
    }
}

void BattleGroundAB::AddPlayer(Player *plr)
{
    BattleGround::AddPlayer(plr);
    //create score and add it to map, default values are set in the constructor
    BattleGroundABScore* sc = new BattleGroundABScore;

    m_PlayerScores[plr->GetGUID()] = sc;
}

void BattleGroundAB::RemovePlayer(Player * /*plr*/, uint64 /*guid*/)
{

    if(GetPlayersSize()==0)
    {
        sLog.outDebug("Arathi Basin: BG ended, objects despawning...");

        // delete everything (for times, when BG will instanced)
        for(uint32 i = 0; i < BG_AB_OBJECT_MAX; i++)
            if (m_bgobjects[i])
            {
                m_bgobjects[i]->Delete();
                m_bgobjects[i] = NULL;
            }

        for(uint32 node = 0; node < BG_AB_NODES_COUNT; node++)
            if (m_bgbuffobjects[node].object)
            {
                m_bgbuffobjects[node].object->Delete();
                m_bgbuffobjects[node].object = NULL;
            }

        for(uint32 node = 0; node < 7; node++)
            if (m_spiritGuides[node])
            {
                m_spiritGuides[node]->CleanupsBeforeDelete();
                ObjectAccessor::Instance().AddObjectToRemoveList(m_spiritGuides[node]);
                m_spiritGuides[node] = NULL;
            }

        // re-setup
        Reset();
        SetupBattleGround();
                    // Only change back to STATUS_WAIT_QUEUE if successfully re-setuped
    }
}

void BattleGroundAB::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    switch(Trigger)
    {
        case 3948:                                          // Arathi Basin Alliance Exit.
            if(Source->GetTeam() != ALLIANCE)
                Source->GetSession()->SendAreaTriggerMessage("Only The Alliance can use that portal");
            else
                Source->LeaveBattleground();
            break;
        case 3949:                                          // Arathi Basin Horde Exit.
            if(Source->GetTeam() != HORDE)
                Source->GetSession()->SendAreaTriggerMessage("Only The Horde can use that portal");
            else
                Source->LeaveBattleground();
            break;
        case 3866:                                          // Stables
        case 3869:                                          // Gold Mine
        case 3867:                                          // Farm
        case 3868:                                          // Lumber Mill
        case 3870:                                          // Black Smith
        case 4020:                                          // Unk1
        case 4021:                                          // Unk2
            //break;
        default:
            //sLog.outError("WARNING: Unhandled AreaTrigger in Battleground: %u", Trigger);
            //Source->GetSession()->SendAreaTriggerMessage("Warning: Unhandled AreaTrigger in Battleground: %u", Trigger);
            break;
    }
}

/*  type: 0-neutral, 1-contested, 3-occupied
    teamIndex: 0-ally, 1-horde                        */
void BattleGroundAB::_CreateBanner(uint8 node, uint8 type, uint8 teamIndex, bool delay)
{
    // Just put it into the queue
    if (delay)
    {
        m_BannerTimers[node].timer = 2000;
        m_BannerTimers[node].type = type;
        m_BannerTimers[node].teamIndex = teamIndex;
        return;
    }

    uint8 obj = node*8 + type + teamIndex;

    // create this object
    uint32 goId;
    if (type==1) {
        if (teamIndex==0) goId = BG_AB_OBJECTID_BANNER_CONT_A;
        else goId = BG_AB_OBJECTID_BANNER_CONT_H;
    } else
        if (teamIndex==0) goId = BG_AB_OBJECTID_BANNER_A;
        else goId = BG_AB_OBJECTID_BANNER_H;

    m_bgobjects[obj] = new GameObject(NULL);
    m_bgobjects[obj]->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), goId, GetMapId(), BG_AB_NodePositions[node][0], BG_AB_NodePositions[node][1], BG_AB_NodePositions[node][2], BG_AB_NodePositions[node][3], 0, 0, sin(BG_AB_NodePositions[node][3]/2), cos(BG_AB_NodePositions[node][3]/2), 100, 1);
    MapManager::Instance().GetMap(GetMapId(), m_bgobjects[obj])->Add(m_bgobjects[obj]);

    // handle aura with banner
    if (!type)
        return;
    obj = node*8 + ((type == 3) ? (5 + teamIndex) : 7);
    MapManager::Instance().GetMap(GetMapId(), m_bgobjects[obj])->Add(m_bgobjects[obj]);
}

void BattleGroundAB::_DelBanner(uint8 node, uint8 type, uint8 teamIndex)
{
    uint8 obj = node*8 + type + teamIndex;
    if (!m_bgobjects[obj])
        return;
    m_bgobjects[obj]->Delete();
    m_bgobjects[obj] = NULL;

    // handle aura with banner
    if (!type) return;
    obj = node*8 + ((type == 3) ? (5 + teamIndex) : 7);
    MapManager::Instance().GetMap(GetMapId(), m_bgobjects[obj])->Remove(m_bgobjects[obj], false);
}

const char* BattleGroundAB::_GetNodeName(uint8 node)
{
    switch (node)
    {
        case BG_AB_NODE_STABLES:
            return LANG_BG_AB_NODE_STABLES;
        case BG_AB_NODE_BLACKSMITH:
            return LANG_BG_AB_NODE_BLACKSMITH;
        case BG_AB_NODE_FARM:
            return LANG_BG_AB_NODE_FARM;
        case BG_AB_NODE_LUMBER_MILL:
            return LANG_BG_AB_NODE_LUMBER_MILL;
        case BG_AB_NODE_GOLD_MINE:
            return LANG_BG_AB_NODE_GOLD_MINE;
        default:
            ASSERT(0);
    }
    return "";
}

void BattleGroundAB::FillInitialWorldStates(WorldPacket& data)
{
    const uint8 plusArray[] = {0, 2, 3, 0, 1};

    /* this values should be already in data variable
    WorldPacket data(SMSG_INIT_WORLD_STATES, (4+4+4+2+(38*8)));
    data << uint32(plr->GetMapId());
    data << uint32(plr->GetZoneId());
    data << uint32(plr->GetAreaId());
    data << uint16(38);                                     // count of uint64 blocks
    data << uint32(0x8d8) << uint32(0x0);                   // 1
    data << uint32(0x8d7) << uint32(0x0);                   // 2
    data << uint32(0x8d6) << uint32(0x0);                   // 3
    data << uint32(0x8d5) << uint32(0x0);                   // 4
    data << uint32(0x8d4) << uint32(0x0);                   // 5
    data << uint32(0x8d3) << uint32(0x0);                   // 6*/

    // Node icons
    for (uint8 node = 0; node < BG_AB_NODES_COUNT; ++node)
        data << uint32(BG_AB_OP_NODEICONS[node]) << uint32((m_Nodes[node]==0)?1:0);

    // Node occupied states
    for (uint8 node = 0; node < BG_AB_NODES_COUNT; ++node)
        for (uint8 i = 1; i < BG_AB_NODES_COUNT; ++i)
            data << uint32(BG_AB_OP_NODESTATES[node] + plusArray[i]) << uint32((m_Nodes[node]==i)?1:0);

    // How many bases each team owns
    uint8 ally = 0, horde = 0;
    for (uint8 node = 0; node < BG_AB_NODES_COUNT; ++node)
        if (m_Nodes[node] == BG_AB_NODE_STATUS_ALLY_OCCUPIED)
            ++ally;
        else if (m_Nodes[node] == BG_AB_NODE_STATUS_HORDE_OCCUPIED)
            ++horde;

    data << uint32(BG_AB_OP_OCCUPIED_BASES_ALLY)  << uint32(ally);
    data << uint32(BG_AB_OP_OCCUPIED_BASES_HORDE) << uint32(horde);

    // Team scores
    data << uint32(BG_AB_OP_RESOURCES_MAX)      << uint32(BG_AB_MAX_TEAM_SCORE);
    data << uint32(BG_AB_OP_RESOURCES_WARNING)  << uint32(BG_AB_WARNING_SCORE);
    data << uint32(BG_AB_OP_RESOURCES_ALLY)     << uint32(m_TeamScores[BG_TEAM_ALLIANCE]);
    data << uint32(BG_AB_OP_RESOURCES_HORDE)    << uint32(m_TeamScores[BG_TEAM_HORDE]);

    // other unknown
    data << uint32(0x745) << uint32(0x2);           // 37 1861 unk
}

void BattleGroundAB::_SendNodeUpdate(uint8 node)
{
    // Send node owner state update to refresh map icons on client
    const uint8 plusArray[] = {0, 2, 3, 0, 1};

    if (m_prevNodes[node])
        UpdateWorldState(BG_AB_OP_NODESTATES[node] + plusArray[m_prevNodes[node]], 0);
    else
        UpdateWorldState(BG_AB_OP_NODEICONS[node], 0);

    UpdateWorldState(BG_AB_OP_NODESTATES[node] + plusArray[m_Nodes[node]], 1);

    // How many bases each team owns
    uint8 ally = 0, horde = 0;
    for (uint8 i = 0; i < BG_AB_NODES_COUNT; ++i)
        if (m_Nodes[i] == BG_AB_NODE_STATUS_ALLY_OCCUPIED)
            ++ally;
        else if (m_Nodes[i] == BG_AB_NODE_STATUS_HORDE_OCCUPIED)
            ++horde;

    UpdateWorldState(BG_AB_OP_OCCUPIED_BASES_ALLY, ally);
    UpdateWorldState(BG_AB_OP_OCCUPIED_BASES_HORDE, horde);
}

void BattleGroundAB::_NodeOccupied(uint8 node)
{
    // spawn spirit guide
    if (m_spiritGuides[node])
        return;
    m_spiritGuides[node] = new Creature(NULL);
    if (!m_spiritGuides[node]->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), 529, BG_AB_SpiritGuidePos[node][0], BG_AB_SpiritGuidePos[node][1], BG_AB_SpiritGuidePos[node][2], BG_AB_SpiritGuidePos[node][3], (node>4)?BG_AB_SpiritGuideId[node-5]:BG_AB_SpiritGuideId[m_Nodes[node]-3], (node>4)?BG_AB_SpiritGuideTeamId[node-5]:BG_AB_SpiritGuideTeamId[m_Nodes[node]-3]))
        ASSERT(0);
    m_spiritGuides[node]->setDeathState(DEAD);
    m_spiritGuides[node]->AIM_Initialize();

    MapManager::Instance().GetMap(GetMapId(), m_spiritGuides[node])->Add(m_spiritGuides[node]);
    m_spiritGuides[node]->CastSpell(m_spiritGuides[node], SPELL_SPIRIT_HEAL_CHANNEL, true);
}

void BattleGroundAB::_NodeDeOccupied(uint8 node)
{
    if (node >= BG_AB_NODES_COUNT)
        return;

    // Those who are waiting to resurrect at this node are taken to the closest own node's graveyard
    std::vector<uint64> ghost_list = m_ReviveQueue[m_spiritGuides[node]->GetGUID()];
    if (!ghost_list.empty())
    {
        WorldSafeLocsEntry const *ClosestGrave = NULL;
        Player *plr;
        for (std::vector<uint64>::iterator itr = ghost_list.begin(); itr != ghost_list.end(); ++itr)
        {
            plr = objmgr.GetPlayer(*ghost_list.begin());
            if (!plr)
                continue;
            if (!ClosestGrave)
                ClosestGrave = SelectGraveYard(plr);

            plr->TeleportTo(GetMapId(), ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, plr->GetOrientation());
        }
    }

    // despawn spirit guide
    if (m_spiritGuides[node])
    {
        m_spiritGuides[node]->CleanupsBeforeDelete();
        ObjectAccessor::Instance().AddObjectToRemoveList(m_spiritGuides[node]);
        m_spiritGuides[node] = NULL;
    }

    // despawn buff object
    if ((node < BG_AB_NODES_COUNT) && (m_bgbuffobjects[node].object))
    {
        m_bgbuffobjects[node].object->Delete();
        m_bgbuffobjects[node].object = NULL;
    }
}

/* Invoked if a player used a banner as a gameobject */
void BattleGroundAB::EventPlayerCapturedBanner(Player *source)
{
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint8 node = BG_AB_NODE_STABLES;
    while ( (node < BG_AB_NODES_COUNT) && !(source->IsWithinDistInMap(m_bgobjects[node*8+7], 10)) )        // player and nodes distance
        ++node;

    if (node == BG_AB_NODES_COUNT)
    {
        // this means our player isn't close to any of banners - maybe cheater ??
        return;
    }

    uint8 teamIndex = GetTeamIndexByTeamId(source->GetTeam());

    // Message to chatlog
    char buf[256];
    uint8 type = (teamIndex == 0) ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE;

    // Check if player really could use this banner, not cheated
    if ( !(m_Nodes[node] == 0 || teamIndex == m_Nodes[node]%2) )
        return;

    uint32 sound = 0;
    // If node is neutral, change to contested
    if (m_Nodes[node] == BG_AB_NODE_TYPE_NEUTRAL)
    {
        m_prevNodes[node] = m_Nodes[node];
        m_Nodes[node] = teamIndex + 1;
        // burn current neutral banner
        _DelBanner(node, BG_AB_NODE_TYPE_NEUTRAL, 0);
        // create new contested banner
        _CreateBanner(node, BG_AB_NODE_TYPE_CONTESTED, teamIndex, true);
        _SendNodeUpdate(node);
        m_NodeTimers[node] = 60000;
        sprintf(buf, LANG_BG_AB_NODE_CLAIMED, _GetNodeName(node), (teamIndex == 0) ? LANG_BG_AB_ALLY : LANG_BG_AB_HORDE);
        sound = SOUND_NODE_CLAIMED;
    }
    // If node is contested
    else if ((m_Nodes[node] == 1) || (m_Nodes[node] == 2))
    {
        // If last state is NOT occupied, change node to enemy-contested
        if (m_prevNodes[node] < BG_AB_BODE_TYPE_OCCUPIED)
        {
            m_prevNodes[node] = m_Nodes[node];
            m_Nodes[node] = teamIndex + 1;
            // burn current contested banner
            _DelBanner(node, BG_AB_NODE_TYPE_CONTESTED, !teamIndex);
            // create new contested banner
            _CreateBanner(node, BG_AB_NODE_TYPE_CONTESTED, teamIndex, true);
            _SendNodeUpdate(node);
            m_NodeTimers[node] = 60000;
            sprintf(buf, LANG_BG_AB_NODE_ASSAULTED, _GetNodeName(node));
        }
        // If contested, change back to occupied
        else
        {
            m_prevNodes[node] = m_Nodes[node];
            m_Nodes[node] = teamIndex + 3;
            // burn current contested banner
            _DelBanner(node, BG_AB_NODE_TYPE_CONTESTED, !teamIndex);
            // create new occupied banner
            _CreateBanner(node, BG_AB_BODE_TYPE_OCCUPIED, teamIndex, true);
            _SendNodeUpdate(node);
            m_NodeTimers[node] = 0;
            _NodeOccupied(node);
            sprintf(buf, LANG_BG_AB_NODE_DEFENDED, _GetNodeName(node));
        }
        (teamIndex == 0) ? SOUND_NODE_ASSAULTED_ALLIANCE : SOUND_NODE_ASSAULTED_HORDE;
    }
    // If node is occupied, change to enemy-contested
    else
    {
        m_prevNodes[node] = m_Nodes[node];
        m_Nodes[node] = teamIndex + 1;
        // burn current occupied banner
        _DelBanner(node, BG_AB_BODE_TYPE_OCCUPIED, !teamIndex);
        // create new contested banner
        _CreateBanner(node, BG_AB_NODE_TYPE_CONTESTED, teamIndex, true);
        _SendNodeUpdate(node);
        _NodeDeOccupied(node);
        m_NodeTimers[node] = 60000;
        sprintf(buf, LANG_BG_AB_NODE_ASSAULTED, _GetNodeName(node));
        (teamIndex == 0) ? SOUND_NODE_ASSAULTED_ALLIANCE : SOUND_NODE_ASSAULTED_HORDE;
    }
    UpdatePlayerScore(source, SCORE_HONORABLE_KILLS, 1);
    UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
    WorldPacket data;
    ChatHandler::FillMessageData(&data, source->GetSession(), type, LANG_UNIVERSAL, NULL, source->GetGUID(), buf, NULL);
    SendPacketToAll(&data);
    // If node is occupied again, send "XY has taken the XY" msg.
    if (m_Nodes[node] > 2)
    {
        sprintf(buf, LANG_BG_AB_NODE_TAKEN, (teamIndex == 0) ? LANG_BG_AB_ALLY : LANG_BG_AB_HORDE, _GetNodeName(node));
        ChatHandler::FillMessageData(&data, NULL, type, LANG_UNIVERSAL, NULL, 0, buf, NULL);
        SendPacketToAll(&data);
    }
    PlaySoundToAll(sound);
}

bool BattleGroundAB::SetupBattleGround()
{
    // Initialize objects table
    for (uint8 i = 0; i < BG_AB_OBJECT_MAX; ++i)
        m_bgobjects[i] = NULL;

    // Create starting objects

    // neutral banners
    for (int banner = BG_AB_OBJECT_BANNER_NEUTRAL, i = 0; i < 5; banner += 8, ++i)
    {
        m_bgobjects[banner] = new GameObject(NULL);
        if(!m_bgobjects[banner]->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), BG_AB_OBJECTID_NODE_BANNER_0+i, GetMapId(), BG_AB_NodePositions[i][0], BG_AB_NodePositions[i][1], BG_AB_NodePositions[i][2], BG_AB_NodePositions[i][3], 0, 0, sin(BG_AB_NodePositions[i][3]/2), cos(BG_AB_NodePositions[i][3]/2), 100, 1))
           return false;
    }

    // ALL banner auras
    for (int banner = BG_AB_OBJECT_AURA_ALLY, i = 0; i < 5; banner += 8, ++i)
    {
        m_bgobjects[banner] = new GameObject(NULL);
        if(!m_bgobjects[banner]->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), BG_AB_OBJECTID_AURA_A, GetMapId(), BG_AB_NodePositions[i][0], BG_AB_NodePositions[i][1], BG_AB_NodePositions[i][2], BG_AB_NodePositions[i][3], 0, 0, sin(BG_AB_NodePositions[i][3]/2), cos(BG_AB_NodePositions[i][3]/2), 0, 0))
           return false;
    }
    for (int banner = BG_AB_OBJECT_AURA_HORDE, i = 0; i < 5; banner += 8, ++i)
    {
        m_bgobjects[banner] = new GameObject(NULL);
        if(!m_bgobjects[banner]->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), BG_AB_OBJECTID_AURA_H, GetMapId(), BG_AB_NodePositions[i][0], BG_AB_NodePositions[i][1], BG_AB_NodePositions[i][2], BG_AB_NodePositions[i][3], 0, 0, sin(BG_AB_NodePositions[i][3]/2), cos(BG_AB_NodePositions[i][3]/2), 0, 0))
           return false;
    }
    for (int banner = BG_AB_OBJECT_AURA_CONTESTED, i = 0; i < 5; banner += 8, ++i)
    {
        m_bgobjects[banner] = new GameObject(NULL);
        if(!m_bgobjects[banner]->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), BG_AB_OBJECTID_AURA_C, GetMapId(), BG_AB_NodePositions[i][0], BG_AB_NodePositions[i][1], BG_AB_NodePositions[i][2], BG_AB_NodePositions[i][3], 0, 0, sin(BG_AB_NodePositions[i][3]/2), cos(BG_AB_NodePositions[i][3]/2), 0, 0))
           return false;
    }

    // Starting gates
    m_bgobjects[BG_AB_OBJECT_GATE_A] = new GameObject(NULL);
    if(!m_bgobjects[BG_AB_OBJECT_GATE_A]->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), BG_AB_OBJECTID_GATE_A, GetMapId(), BG_AB_DoorPositions[0][0], BG_AB_DoorPositions[0][1], BG_AB_DoorPositions[0][2], BG_AB_DoorPositions[0][3], BG_AB_DoorPositions[0][4], BG_AB_DoorPositions[0][5], BG_AB_DoorPositions[0][6], BG_AB_DoorPositions[0][7], 0, 0))
        return false;
    m_bgobjects[BG_AB_OBJECT_GATE_H] = new GameObject(NULL);
    if(!m_bgobjects[BG_AB_OBJECT_GATE_H]->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), BG_AB_OBJECTID_GATE_H, GetMapId(), BG_AB_DoorPositions[1][0], BG_AB_DoorPositions[1][1], BG_AB_DoorPositions[1][2], BG_AB_DoorPositions[1][3], BG_AB_DoorPositions[1][4], BG_AB_DoorPositions[1][5], BG_AB_DoorPositions[1][6], BG_AB_DoorPositions[1][7], 0, 0))
        return false;

    // Initialize pickups table
    for (uint8 i = 0; i < 5; ++i)
    {
        BG_AB_BuffObjectInfo info;
        info.object     = NULL;
        info.spellId    = 0;
        info.timer      = 0;
        m_bgbuffobjects[i]  = info;
    }

    // OK
    return true;
}

void BattleGroundAB::ResetBGSubclass()
{
    m_TeamScores[BG_TEAM_ALLIANCE]          = 0;
    m_TeamScores[BG_TEAM_HORDE]             = 0;
    m_lastTick[BG_TEAM_ALLIANCE]            = 0;
    m_lastTick[BG_TEAM_HORDE]               = 0;
    m_HonorScoreTics[BG_TEAM_ALLIANCE]      = 0;
    m_HonorScoreTics[BG_TEAM_HORDE]         = 0;
    m_ReputationScoreTics[BG_TEAM_ALLIANCE] = 0;
    m_ReputationScoreTics[BG_TEAM_HORDE]    = 0;
    m_IsInformedNearVictory                 = false;
    for (uint8 i = 0; i < BG_AB_NODES_COUNT; ++i)
    {
        m_Nodes[i] = 0;
        m_prevNodes[i] = 0;
        m_NodeTimers[i] = 0;
        m_BannerTimers[i].timer = 0;
    }
    for (uint8 i = 0; i < 7; ++i)
        m_spiritGuides[i] = NULL;
    m_buffchecktimer = 0;
}

WorldSafeLocsEntry const* BattleGroundAB::SelectGraveYard(Player* player)
{
    uint8 teamIndex = GetTeamIndexByTeamId(player->GetTeam());

    // Is there any occupied node for this team?
    std::vector<uint8> nodes;
    for (uint8 i = 0; i < BG_AB_NODES_COUNT; ++i)
        if (m_Nodes[i] == teamIndex+3)
            nodes.push_back(i);

    WorldSafeLocsEntry const* good_entry = NULL;
    // If so, select the closest node to place ghost on
    if (!nodes.empty())
    {
        float x = player->GetPositionX();
        float y = player->GetPositionY();    // let alone z coordinate...
        float mindist = 999999.0f;
        for (uint8 i = 0; i < nodes.size(); ++i)
        {
            WorldSafeLocsEntry const*entry = sWorldSafeLocsStore.LookupEntry( BG_AB_GraveyardIds[nodes[i]] );
            if(!entry) continue;
            float dist = (entry->x - x)*(entry->x - x)+(entry->y - y)*(entry->y - y);
            if (mindist > dist)
            {
                mindist = dist;
                good_entry = entry;
            }
        }
        nodes.clear();
    }
    // If not, place ghost on starting location
    if(!good_entry)
        good_entry = sWorldSafeLocsStore.LookupEntry( BG_AB_GraveyardIds[teamIndex+5] );

    return good_entry;
}

void BattleGroundAB::UpdatePlayerScore(Player* Source, uint32 type, uint32 value)
{

    std::map<uint64, BattleGroundScore*>::iterator itr = m_PlayerScores.find(Source->GetGUID());

    if(itr == m_PlayerScores.end())                         // player not found...
        return;

    switch(type)
    {
        case SCORE_BASES_ASSAULTED:
            ((BattleGroundABScore*)itr->second)->BasesAssaulted += value;
            break;
        case SCORE_BASES_DEFENDED:
            ((BattleGroundABScore*)itr->second)->BasesDefended += value;
            break;
        default:
            BattleGround::UpdatePlayerScore(Source,type,value);
            break;
    }
}


