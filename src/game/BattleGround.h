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

#ifndef __BATTLEGROUND_H
#define __BATTLEGROUND_H

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "BattleGroundMgr.h"
#include "SharedDefines.h"

#define SOUND_HORDE_WINS            8454
#define SOUND_ALLIANCE_WINS         8455
#define SOUND_BG_START              3439
//TODO move these constant definitions to BG subclass
#define ITEM_WS_MARK_LOSER         24950
#define ITEM_WS_MARK_WINNER        24951
#define ITEM_AB_MARK_LOSER          24952
#define ITEM_AB_MARK_WINNER         24953
#define ITEM_AV_MARK_LOSER          24954
#define ITEM_AV_MARK_WINNER         24955

#define SPELL_WAITING_FOR_RESURRECT 2584
#define SPELL_SPIRIT_HEAL_CHANNEL   22011
#define SPELL_SPIRIT_HEAL           22012
#define SPELL_RESURRECTION_VISUAL   24171
#define SPELL_ARENA_PREPARATION     32727

#define RESURRECTION_INTERVAL       30000                   // ms
#define REMIND_INTERVAL             30000                   // ms
#define INVITE_ACCEPT_WAIT_TIME     120000                  // ms
#define TIME_TO_AUTOREMOVE          120000                  // ms
#define MAX_OFFLINE_TIME            300000                  // ms
#define START_DELAY1                60000                   // ms
#define START_DELAY2                30000                   // ms
#define RESPAWN_ONE_DAY             86400                   // secs
#define RESPAWN_IMMEDIATELY         0                       // secs
#define BUFF_RESPAWN_TIME           180                     // secs

enum BattleGroundStatus
{
    STATUS_NONE         = 0,
    STATUS_WAIT_QUEUE   = 1,
    STATUS_WAIT_JOIN    = 2,
    STATUS_IN_PROGRESS  = 3,
    STATUS_WAIT_LEAVE   = 4                                 // custom
};

struct BattleGroundPlayer
{
    uint32  LastOnlineTime;                                 // for tracking and removing offline players from queue after 5 minutes
    uint32  Team;                                           // Player's team
};

struct BattleGroundObjectInfo
{
    BattleGroundObjectInfo() : object(NULL), timer(0), spellid(0) {}

    GameObject  *object;
    int32       timer;
    uint32      spellid;
};

#define MAX_QUEUED_PLAYERS_MAP 7

enum BattleGroundTypeId
{
    BATTLEGROUND_AV     = 1,
    BATTLEGROUND_WS     = 2,
    BATTLEGROUND_AB     = 3,
    BATTLEGROUND_NA     = 4,
    BATTLEGROUND_BE     = 5,
    BATTLEGROUND_AA     = 6,
    BATTLEGROUND_EY     = 7,
    BATTLEGROUND_RL     = 8
};

enum ScoreType
{
    SCORE_KILLS             = 1,
    SCORE_FLAG_CAPTURES     = 2,
    SCORE_FLAG_RETURNS      = 3,
    SCORE_DEATHS            = 4,
    SCORE_DAMAGE_DONE       = 5,
    SCORE_HEALING_DONE      = 6,
    SCORE_BONUS_HONOR       = 7,
    SCORE_HONORABLE_KILLS   = 8
    // TODO: Add more
};

enum ArenaType
{
    ARENA_TYPE_2v2          = 2,
    ARENA_TYPE_3v3          = 3,
    ARENA_TYPE_5v5          = 5
};

enum BattleGroundType
{
    TYPE_BATTLEGROUND     = 3,
    TYPE_ARENA            = 4
};

enum BattleGroundWinner
{
    WINNER_HORDE            = 0,
    WINNER_ALLIANCE         = 1,
    WINNER_NONE             = 2
};

class BattleGroundScore
{
    public:
        BattleGroundScore() : KillingBlows(0), HonorableKills(0), Deaths(0), DamageDone(0), HealingDone(0), BonusHonor(0) {};
        virtual ~BattleGroundScore() {};    //virtual destructor is used when deleting score from scores map
        uint32 KillingBlows;
        uint32 HonorableKills;
        uint32 Deaths;
        uint32 DamageDone;
        uint32 HealingDone;
        uint32 BonusHonor;
};

/*
This class is used to:
1. Add player to battleground
2. Remove player from battleground
3. some certain cases, same for all battlegrounds
4. It has properties same for all battlegrounds
*/
class BattleGround
{
    friend class BattleGroundMgr;

    public:
        /* Construction */
        BattleGround();
        virtual ~BattleGround();
        virtual void Update(time_t diff);                   // must be implemented in BG subclass of BG specific update code, but must in begginning call parent version
        virtual bool SetupBattleGround()                    // must be implemented in BG subclass
        {
            return true;
        }

        /* Battleground */
        // Get methods:
        char const* GetName() const         { return m_Name; }
        uint32 GetTypeID() const            { return m_TypeID; }
        uint32 GetQueueType() const         { return m_Queue_type; }
        uint32 GetInstanceID() const        { return m_InstanceID; }
        uint32 GetStatus() const            { return m_Status; }
        uint32 GetStartTime() const         { return m_StartTime; }
        uint32 GetEndTime() const           { return m_EndTime; }
        uint32 GetLastResurrectTime() const { return m_LastResurrectTime; }
        uint32 GetMaxPlayers() const        { return m_MaxPlayers; }
        uint32 GetMinPlayers() const        { return m_MinPlayers; }

        uint32 GetMinLevel() const          { return m_LevelMin; }
        uint32 GetMaxLevel() const          { return m_LevelMax; }

        uint32 GetMaxPlayersPerTeam() const { return m_MaxPlayersPerTeam; }
        uint32 GetMinPlayersPerTeam() const { return m_MinPlayersPerTeam; }

        int GetStartDelayTime() const       { return m_startDelay; }  ///sure int???
        uint8 GetArenaType() const  { return m_ArenaType; }
        uint8 GetWinner() const     { return m_Winner; }

        // Set methods:
        void SetName(char const* Name)      { m_Name = Name; }
        void SetTypeID(uint32 TypeID)       { m_TypeID = TypeID; }
        void SetQueueType(uint32 ID)        { m_Queue_type = ID; }
        void SetInstanceID(uint32 InstanceID) { m_InstanceID = InstanceID; }
        void SetStatus(uint32 Status)       { m_Status = Status; }
        void SetStartTime(uint32 Time)      { m_StartTime = Time; }
        void SetEndTime(uint32 Time)        { m_EndTime = Time; }
        void SetLastResurrectTime(uint32 Time) { m_LastResurrectTime = Time; }
        void SetMaxPlayers(uint32 MaxPlayers) { m_MaxPlayers = MaxPlayers; }
        void SetMinPlayers(uint32 MinPlayers) { m_MinPlayers = MinPlayers; }
        void SetLevelRange(uint32 min, uint32 max) { m_LevelMin = min; m_LevelMax = max; }
        void SetRated(bool state)           { m_IsRated = state; }
        void SetArenaType(uint8 type)       { m_ArenaType = type; }
        void SetArenaorBGType(bool _isArena) { m_IsArena = _isArena; }
        void SetWinner(uint8 winner) { m_Winner = winner; }

        void ModifyStartDelayTime(int diff) { m_startDelay -= diff; }
        void SetStartDelayTime(int Time)    { m_startDelay = Time; }

        void SetMaxPlayersPerTeam(uint32 MaxPlayers) { m_MaxPlayersPerTeam = MaxPlayers; }
        void SetMinPlayersPerTeam(uint32 MinPlayers) { m_MinPlayersPerTeam = MinPlayers; }

        void AddToBGFreeSlotQueue();                            //this queue will be useful when more battlegrounds instances will be available
        void RemoveFromBGFreeSlotQueue();                       //this method could delete whole BG instance, if another free is available

        void DecreaseInvitedCount(uint32 team)      { (team == ALLIANCE) ? m_InvitedAlliance-- : m_InvitedHorde--; }
        void IncreaseInvitedCount(uint32 team)      { (team == ALLIANCE) ? m_InvitedAlliance++ : m_InvitedHorde++; }
        uint32 GetInvitedCount(uint32 team) const
        {
            if (team == ALLIANCE)
                return m_InvitedAlliance;
            else
                return m_InvitedHorde;
        }
        bool HasFreeSlotsForTeam(uint32 Team) const;
        bool HasFreeSlots() const;

        bool isArena() const        { return m_IsArena; }
        bool isBattleGround() const { return !m_IsArena; }
        bool isRated() const        { return m_IsRated; }

        bool isDoorsSpawned() const { return m_doorsSpawned; }
        void SetDoorsSpawned(bool state) { m_doorsSpawned = state; }

        std::map<uint64, BattleGroundPlayer> *GetPlayers() { return &m_Players; }
        uint32 GetPlayersSize() const { return m_Players.size(); }
        uint32 GetRemovedPlayersSize() const { return m_RemovedPlayers.size(); }

        std::map<uint64, BattleGroundScore*>::const_iterator GetPlayerScoresBegin() const { return m_PlayerScores.begin(); }
        std::map<uint64, BattleGroundScore*>::const_iterator GetPlayerScoresEnd() const { return m_PlayerScores.end(); }
        uint32 GetPlayerScoresSize() const { return m_PlayerScores.size(); }

        uint32 GetReviveQueueSize() const { return m_ReviveQueue.size(); }

        void AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid);
        void RemovePlayerFromResurrectQueue(uint64 player_guid);

        void StartBattleGround(time_t diff);

        /* Location */
        void SetMapId(uint32 MapID) { m_MapId = MapID; }
        uint32 GetMapId() const { return m_MapId; }

        void SetTeamStartLoc(uint32 TeamID, float X, float Y, float Z, float O);
        void GetTeamStartLoc(uint32 TeamID, float &X, float &Y, float &Z, float &O) const
        {
            uint8 idx = GetTeamIndexByTeamId(TeamID);
            X = m_TeamStartLocX[idx];
            Y = m_TeamStartLocY[idx];
            Z = m_TeamStartLocZ[idx];
            O = m_TeamStartLocO[idx];
        }

        /* Packet Transfer */
        void SendPacketToTeam(uint32 TeamID, WorldPacket *packet, Player *sender = NULL, bool self = true);
        void SendPacketToAll(WorldPacket *packet);
        void PlaySoundToTeam(uint32 SoundID, uint32 TeamID);
        void PlaySoundToAll(uint32 SoundID);
        void CastSpellOnTeam(uint32 SpellID, uint32 TeamID);
        void RewardHonorToTeam(uint32 Honor, uint32 TeamID);
        void RewardReputationToTeam(uint32 faction_id, uint32 Reputation, uint32 TeamID);
        void UpdateWorldState(uint32 Field, uint32 Value);
        void EndBattleGround(uint32 winner);
        void BlockMovement(Player *plr);

        std::list<uint64> m_SpiritGuides;
        typedef std::vector<uint64> BGObjects;
        BGObjects m_bgobjects;
        void SpawnBGObject(uint32 type, uint32 respawntime);
        bool AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0,  float rotation1,  float rotation2,  float rotation3, uint32 respawntime = 0);
        bool AddSpiritGuide(float x, float y, float z, float o, uint32 team);

        void DoorOpen(uint32 type);

        /* Raid Group */
        Group *GetBgRaid(uint32 TeamID) const { return TeamID == ALLIANCE ? m_raids[0] : m_raids[1]; }
        void SetBgRaid(uint32 TeamID, Group *bg_raid)
        {
            Group* &old_raid = TeamID == ALLIANCE ? m_raids[0] : m_raids[1];
            if(old_raid) old_raid->SetBattlegroundGroup(NULL);
            if(bg_raid) bg_raid->SetBattlegroundGroup(this);
            old_raid = bg_raid;
        }

        virtual void UpdatePlayerScore(Player *Source, uint32 type, uint32 value);

        uint8 GetTeamIndexByTeamId(uint32 Team) const { return Team == ALLIANCE ? 0 : 1; }
        uint32 GetPlayersCountByTeam(uint32 Team) const { return m_PlayersCount[GetTeamIndexByTeamId(Team)]; }
        void UpdatePlayersCountByTeam(uint32 Team, bool remove)
        {
            if(remove)
                m_PlayersCount[GetTeamIndexByTeamId(Team)]--;
            else
                m_PlayersCount[GetTeamIndexByTeamId(Team)]++;
        }

        /* Triggers handle */
        // must be implemented in BG subclass
        virtual void HandleAreaTrigger(Player* /*Source*/, uint32 /*Trigger*/) {}
        // must be implemented in BG subclass if need
        virtual void HandleKillPlayer(Player* /*player*/, Player* /*killer*/) {}
        // must be implemented in BG subclass if need
        virtual void HandleDropFlag(Player* /*player*/) {}

        virtual void AddPlayer(Player *plr);                // must be implemented in BG subclass
        virtual void RemovePlayerAtLeave(uint64 guid, bool Transport, bool SendPacket);
                                                            // can be extended in in BG subclass

    protected:
        /* Scorekeeping */
        std::map<uint64, BattleGroundScore*> m_PlayerScores;// Player scores
        // must be implemented in BG subclass
        virtual void RemovePlayer(Player * /*player*/, uint64 /*guid*/) {}

        virtual void _SendCurrentGameState(Player * /*plr*/) {}

        /* Player lists, those need to be accessible by inherited classes */
        std::map<uint64, BattleGroundPlayer> m_Players;
        std::map<uint64, std::vector<uint64> > m_ReviveQueue;// Spirit Guide guid + Player list GUIDS

    private:
        /* Battleground */
        uint32 m_TypeID;                                    //Battleground type, defined in enum BattleGroundTypeId
        uint32 m_InstanceID;                                //BattleGround Instance's GUID!
        uint32 m_Status;
        uint32 m_StartTime;
        uint32 m_EndTime;
        uint32 m_LastResurrectTime;
        uint32 m_Queue_type;
        uint8  m_ArenaType;                                 // 2=2v2, 3=3v3, 5=5v5
        // this variable is not used .... it can be found in many other ways... but to store it in BG object instance is useless
        //uint8  m_BattleGroundType;                        // 3=BG, 4=arena
        //instead of uint8 (in previous line) is bool used
        bool   m_IsArena;
        uint8  m_Winner;                                    // 0=alliance, 1=horde, 2=none
        int32  m_startDelay;
        bool   m_doorsSpawned;
        bool   m_IsRated;                                   // is this battle rated?
        char const *m_Name;

        /* Player lists */
        std::vector<uint64> m_ResurrectQueue;               // Player GUID
        std::map<uint64, uint8> m_RemovedPlayers;           // uint8 is remove type (0 - bgqueue, 1 - bg, 2 - resurrect queue)

        /* Invited counters are useful for player invitation to BG - do not allow, if BG is started to one faction to have 2 more players than another faction */
        /* Invited counters will be changed only when removing already invited player from queue, removing player from battleground and inviting player to BG */
        /* Invited players counters*/
        uint32 m_InvitedAlliance;
        uint32 m_InvitedHorde;

        /* Raid Group */
        Group *m_raids[2];                                  // 0 - alliance, 1 - horde

        /* Players count by team */
        uint32 m_PlayersCount[2];

        /* Limits */
        uint32 m_LevelMin;
        uint32 m_LevelMax;
        uint32 m_MaxPlayersPerTeam;
        uint32 m_MaxPlayers;
        uint32 m_MinPlayersPerTeam;
        uint32 m_MinPlayers;

        /* Location */
        uint32 m_MapId;
        float m_TeamStartLocX[2];
        float m_TeamStartLocY[2];
        float m_TeamStartLocZ[2];
        float m_TeamStartLocO[2];
};
#endif
