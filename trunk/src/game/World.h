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

/// \addtogroup world The World
/// @{
/// \file

#ifndef __WORLD_H
#define __WORLD_H

#include "Common.h"
#include "Timer.h"
#include "Policies/Singleton.h"

#include <map>
#include <set>
#include <list>

class Object;
class WorldPacket;
class WorldSession;
class Player;
class Weather;
struct ScriptAction;
struct ScriptInfo;
class CliCommandHolder;
class SqlResultQueue;
class QueryResult;
class WorldSocket;

/// Timers for different object refresh rates
enum WorldTimers
{
    WUPDATE_OBJECTS     = 0,
    WUPDATE_SESSIONS    = 1,
    WUPDATE_AUCTIONS    = 2,
    WUPDATE_WEATHERS    = 3,
    WUPDATE_UPTIME      = 4,
    WUPDATE_CORPSES     = 5,
    WUPDATE_EVENTS      = 6,
    WUPDATE_COUNT       = 7
};

/// Configuration elements
enum WorldConfigs
{
    CONFIG_COMPRESSION = 0,
    CONFIG_GRID_UNLOAD,
    CONFIG_INTERVAL_SAVE,
    CONFIG_INTERVAL_GRIDCLEAN,
    CONFIG_INTERVAL_MAPUPDATE,
    CONFIG_INTERVAL_CHANGEWEATHER,
    CONFIG_PORT_WORLD,
    CONFIG_SOCKET_SELECTTIME,
    CONFIG_GROUP_XP_DISTANCE,
    CONFIG_SIGHT_MONSTER,
    CONFIG_SIGHT_GUARDER,
    CONFIG_GAME_TYPE,
    CONFIG_ALLOW_TWO_SIDE_ACCOUNTS,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_TRADE,
    CONFIG_ALLOW_TWO_SIDE_WHO_LIST,
    CONFIG_MAX_PLAYER_LEVEL,
    CONFIG_INSTANCE_IGNORE_LEVEL,
    CONFIG_INSTANCE_IGNORE_RAID,
    CONFIG_BATTLEGROUND_CAST_DESERTER,
    CONFIG_MAX_PRIMARY_TRADE_SKILL,
    CONFIG_MIN_PETITION_SIGNS,
    CONFIG_GM_WISPERING_TO,
    CONFIG_GM_IN_GM_LIST,
    CONFIG_GM_IN_WHO_LIST,
    CONFIG_GM_LOGIN_STATE,
    CONFIG_GM_LOG_TRADE,
    CONFIG_GROUP_VISIBILITY,
    CONFIG_MAIL_DELIVERY_DELAY,
    CONFIG_SKILL_CHANCE_ORANGE,
    CONFIG_SKILL_CHANCE_YELLOW,
    CONFIG_SKILL_CHANCE_GREEN,
    CONFIG_SKILL_CHANCE_GREY,
    CONFIG_SKILL_CHANCE_MINING_STEPS,
    CONFIG_SKILL_CHANCE_SKINNING_STEPS,
    CONFIG_MAX_OVERSPEED_PINGS,
    CONFIG_SAVE_RESPAWN_TIME_IMMEDIATLY,
    CONFIG_WEATHER,
    CONFIG_EXPANSION,
    CONFIG_CHATFLOOD_MESSAGE_COUNT,
    CONFIG_CHATFLOOD_MESSAGE_DELAY,
    CONFIG_CHATFLOOD_MUTE_TIME,
    CONFIG_CREATURE_FAMILY_ASSISTEMCE_RADIUS,
    CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF,
    CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF,
    CONFIG_HONOR_KILL_LIMIT,
    CONFIG_DETECT_POS_COLLISION,
    CONFIG_VALUE_COUNT
};

/// Server rates
enum Rates
{
    RATE_HEALTH=0,
    RATE_POWER_MANA,
    RATE_POWER_RAGE_INCOME,
    RATE_POWER_RAGE_LOSS,
    RATE_POWER_FOCUS,
    RATE_DROP_ITEMS,
    RATE_DROP_MONEY,
    RATE_XP_KILL,
    RATE_XP_QUEST,
    RATE_XP_EXPLORE,
    RATE_REPUTATION_GAIN,
    RATE_CREATURE_NORMAL_HP,
    RATE_CREATURE_ELITE_ELITE_HP,
    RATE_CREATURE_ELITE_RAREELITE_HP,
    RATE_CREATURE_ELITE_WORLDBOSS_HP,
    RATE_CREATURE_ELITE_RARE_HP,
    RATE_CREATURE_NORMAL_DAMAGE,
    RATE_CREATURE_ELITE_ELITE_DAMAGE,
    RATE_CREATURE_ELITE_RAREELITE_DAMAGE,
    RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE,
    RATE_CREATURE_ELITE_RARE_DAMAGE,
    RATE_CREATURE_AGGRO,
    RATE_REST_INGAME,
    RATE_REST_OFFLINE_IN_TAVERN_OR_CITY,
    RATE_REST_OFFLINE_IN_WILDERNESS,
    RATE_DAMAGE_FALL,
    RATE_AUCTION_TIME,
    RATE_AUCTION_DEPOSIT,
    RATE_AUCTION_CUT,
    RATE_HONOR,
    RATE_MINING_AMOUNT,
    RATE_MINING_NEXT,
    RATE_TALENT,
    RATE_LOYALTY,
    MAX_RATES
};

/// Type of environmental damages
enum EnviromentalDamage
{
    DAMAGE_EXHAUSTED = 0,
    DAMAGE_DROWNING = 1,
    DAMAGE_FALL = 2,
    DAMAGE_LAVA = 3,
    DAMAGE_SLIME = 4,
    DAMAGE_FIRE = 5
};

/// Type of server
enum RealmType
{
    REALM_NORMAL = 0,
    REALM_PVP = 1,
    REALM_NORMAL2 = 4,
    REALM_RP = 6,
    REALM_RPPVP = 8
};

/// Ban function return codes
enum BanReturn
{
    BAN_SUCCESS,
    BAN_SYNTAX_ERROR,
    BAN_NOTFOUND
};

// DB scripting commands
#define SCRIPT_COMMAND_SAY                   0
#define SCRIPT_COMMAND_EMOTE                 1
#define SCRIPT_COMMAND_FIELD_SET             2
#define SCRIPT_COMMAND_MOVE_TO               3
#define SCRIPT_COMMAND_FLAG_SET              4
#define SCRIPT_COMMAND_FLAG_REMOVE           5
#define SCRIPT_COMMAND_TELEPORT_TO           6
#define SCRIPT_COMMAND_QUEST_EXPLORED        7
#define SCRIPT_COMMAND_RESPAWN_GAMEOBJECT    9
#define SCRIPT_COMMAND_TEMP_SUMMON_CREATURE 10
#define SCRIPT_COMMAND_OPEN_DOOR            11

/// CLI related stuff, define here to prevent cyclic dependancies

typedef int(* pPrintf)(const char*,...);
typedef void(* pCliFunc)(char *,pPrintf);

/// Command Template class
struct CliCommand
{
    char const * cmd;
    pCliFunc Func;
    char const * description;
};

/// Storage class for commands issued for delayed execution
class CliCommandHolder
{
    private:
        const CliCommand *cmd;
        char *args;
        pPrintf m_zprintf;
    public:
        CliCommandHolder(const CliCommand *command, const char *arguments, pPrintf p_zprintf)
            : cmd(command), m_zprintf(p_zprintf)
        {
            size_t len = strlen(arguments)+1;
            args = new char[len];
            memcpy(args, arguments, len);
        }
        ~CliCommandHolder() { delete[] args; }
        void Execute() const { cmd->Func(args, m_zprintf); }
        pPrintf GetOutputMethod() const {return (m_zprintf);}
};

/// The World
class World
{
    public:
        static volatile bool m_stopEvent;

        World();
        ~World();

        WorldSession* FindSession(uint32 id) const;
        void AddSession(WorldSession *s);
        bool RemoveSession(uint32 id);
        /// Get the number of current active sessions
        uint32 GetActiveAndQueuedSessionCount() const { return m_sessions.size(); }
		uint32 GetActiveSessionCount() const { return m_sessions.size() - m_QueuedPlayer.size(); }
        /// Get the maximum number of parallel sessions on the server since last reboot
        uint32 GetMaxSessionCount() const { return m_maxSessionsCount; }
        Player* FindPlayerInZone(uint32 zone);

        Weather* FindWeather(uint32 id) const;
        Weather* AddWeather(uint32 zone_id);
        void RemoveWeather(uint32 zone_id);

        /// Get the active session server limit (or security level limitations)
        uint32 GetPlayerAmountLimit() const { return m_playerLimit >= 0 ? m_playerLimit : 0; }
        AccountTypes GetPlayerSecurityLimit() const { return m_playerLimit <= 0 ? AccountTypes(-m_playerLimit) : SEC_PLAYER; }

        /// Set the active session server limit (or security level limitation)
        void SetPlayerLimit(int32 limit, bool needUpdate = false);

		//player Queue
		typedef std::list<WorldSocket*> Queue;
		void AddQueuedPlayer(WorldSocket* Socket);
		void RemoveQueuedPlayer(WorldSocket* Socket);
		int32 GetQueuePos(WorldSocket* Socket);
		uint32 GetQueueSize() const { return m_QueuedPlayer.size(); }

        /// \todo Actions on m_allowMovement still to be implemented
        /// Is movement allowed?
        bool getAllowMovement() const { return m_allowMovement; }
        /// Allow/Disallow object movements
        void SetAllowMovement(bool allow) { m_allowMovement = allow; }

        /// Set a new Message of the Day
        void SetMotd(const char *motd) { m_motd = motd; }
        /// Get the current Message of the Day
        const char* GetMotd() const { return m_motd.c_str(); }

        uint32 GetDBClang() const { return m_langid; }

        /// Get the path where data (dbc, maps) are stored on disk
        std::string GetDataPath() const { return m_dataPath; }

        /// When server started?
        time_t const& GetStartTime() const { return m_startTime; }
        /// What time is it?
        time_t const& GetGameTime() const { return m_gameTime; }
        /// Uptime (in secs)
        uint32 GetUptime() const { return uint32(m_gameTime - m_startTime); }

        /// Get the maximum skill level a player can reach
        uint16 GetConfigMaxSkillValue() const
        {
            uint32 lvl = getConfig(CONFIG_MAX_PLAYER_LEVEL);
            return lvl > 60 ? 300 + ((lvl - 60) * 75) / 10 : lvl*5;
        }

        void SetInitialWorldSettings();

        void SendWorldText(const char *text, WorldSession *self = 0);
        void SendGlobalMessage(WorldPacket *packet, WorldSession *self = 0, uint32 team = 0);
        void SendZoneMessage(uint32 zone, WorldPacket *packet, WorldSession *self = 0, uint32 team = 0);
        void SendZoneText(uint32 zone, const char *text, WorldSession *self = 0, uint32 team = 0);
        void SendServerMessage(uint32 type, const char *text = "", Player* player = NULL);

        /// Are we in the middle of a shutdown?
        bool IsShutdowning() const { return m_ShutdownTimer > 0; }
        void ShutdownServ(uint32 time, bool idle = false);
        void ShutdownCancel();
        void ShutdownMsg(bool show = false, Player* player = NULL);

        void Update(time_t diff);

        void UpdateSessions( time_t diff );
        /// Set a server rate (see #Rates)
        void setRate(Rates rate,float value) { rate_values[rate]=value; }
        /// Get a server rate (see #Rates)
        float getRate(Rates rate) const { return rate_values[rate]; }

        /// Set a server configuration element (see #WorldConfigs)
        void setConfig(uint32 index,uint32 value)
        {
            if(index<CONFIG_VALUE_COUNT)
                m_configs[index]=value;
        }

        /// Get a server configuration element (see #WorldConfigs)
        uint32 getConfig(uint32 index) const
        {
            if(index<CONFIG_VALUE_COUNT)
                return m_configs[index];
            else
                return 0;
        }

        /// Are we on a "Player versus Player" server?
        bool IsPvPRealm() { return (getConfig(CONFIG_GAME_TYPE) == REALM_PVP || getConfig(CONFIG_GAME_TYPE) == REALM_RPPVP); }

        bool KickPlayer(std::string playerName);
        void KickAll();
        void KickAllLess(AccountTypes sec);
        void KickAllQueued();
        uint8 BanAccount(std::string type, std::string nameOrIP, std::string duration, std::string reason, std::string author);
        bool RemoveBanAccount(std::string type, std::string nameOrIP);

        void ScriptsStart(std::map<uint32, std::multimap<uint32, ScriptInfo> > const& scripts, uint32 id, Object* source, Object* target);
        bool IsScriptScheduled() const { return !m_scriptSchedule.empty(); }

        // for max speed access
        static float GetMaxVisibleDistanceForCreature() { return m_MaxVisibleDistanceForCreature; }
        static float GetMaxVisibleDistanceForPlayer()   { return m_MaxVisibleDistanceForPlayer;   }
        static float GetMaxVisibleDistanceForObject()   { return m_MaxVisibleDistanceForObject;   }
        static float GetMaxVisibleDistanceInFlight()    { return m_MaxVisibleDistanceInFlight;    }
        static float GetVisibleUnitGreyDistance()       { return m_VisibleUnitGreyDistance;       }
        static float GetVisibleObjectGreyDistance()     { return m_VisibleObjectGreyDistance;     }

        void ProcessCliCommands();
        void QueueCliCommand(CliCommandHolder* command) { cliCmdQueue.add(command); }

        void UpdateResultQueue();
        void InitResultQueue();

        void UpdateRealmCharCount(uint32 accid);
    protected:
        void _UpdateGameTime();
        void ScriptsProcess();
        // callback for UpdateRealmCharacters
        void _UpdateRealmCharCount(QueryResult *resultCharCount, uint32 accountId);

        void InitDailyQuestResetTime();
        void ResetDailyQuests();
    private:
        time_t m_startTime;
        time_t m_gameTime;
        IntervalTimer m_timers[WUPDATE_COUNT];
        uint32 mail_timer;
        uint32 mail_timer_expires;

        typedef HM_NAMESPACE::hash_map<uint32, Weather*> WeatherMap;
        WeatherMap m_weathers;
        typedef HM_NAMESPACE::hash_map<uint32, WorldSession*> SessionMap;
        SessionMap m_sessions;
        uint32 m_maxSessionsCount;
        std::set<WorldSession*> m_kicked_sessions;

        std::multimap<time_t, ScriptAction> m_scriptSchedule;

        float rate_values[MAX_RATES];
        uint32 m_configs[CONFIG_VALUE_COUNT];
        int32 m_playerLimit;
        uint32 m_langid;
        void DetectDBCLang();
        bool m_allowMovement;
        std::string m_motd;
        std::string m_dataPath;

        uint32 m_ShutdownIdleMode;
        uint32 m_ShutdownTimer;

        // for max speed access
        static float m_MaxVisibleDistanceForCreature;
        static float m_MaxVisibleDistanceForPlayer;
        static float m_MaxVisibleDistanceForObject;
        static float m_MaxVisibleDistanceInFlight;
        static float m_VisibleUnitGreyDistance;
        static float m_VisibleObjectGreyDistance;

        // CLI command holder to be thread safe
        ZThread::LockedQueue<CliCommandHolder*, ZThread::FastMutex> cliCmdQueue;
        SqlResultQueue *m_resultQueue;

        // next daily quests reset time
        time_t m_NextDailyQuestReset;

		//Player Queue
		Queue m_QueuedPlayer;
};

extern uint32 realmID;

#define sWorld MaNGOS::Singleton<World>::Instance()
#endif
/// @}
