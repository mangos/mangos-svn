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

#ifndef __WORLD_H
#define __WORLD_H

#include "Timer.h"
#include "Policies/Singleton.h"
#include "SharedDefines.h"
#include <string>
using namespace std;

class Object;
class WorldPacket;
class WorldSession;
class Player;
class Weather;

enum WorldTimers
{
    WUPDATE_OBJECTS = 0,
    WUPDATE_SESSIONS = 1,
    WUPDATE_AUCTIONS = 2,
    WUPDATE_WEATHERS = 3,
    WUPDATE_COUNT = 4
};

enum WorldConfigs
{
    CONFIG_LOG_LEVEL = 0,
    CONFIG_LOG_WORLD,
    CONFIG_COMPRESSION,
    CONFIG_GRID_UNLOAD,
    CONFIG_INTERVAL_SAVE,
    CONFIG_INTERVAL_GRIDCLEAN,
    CONFIG_INTERVAL_MAPUPDATE,
    CONFIG_INTERVAL_CHANGEWEATHER,
    CONFIG_PORT_WORLD,
    CONFIG_PORT_REALM,
    CONFIG_SOCKET_SELECTTIME,
    CONFIG_GROUP_XP_DISTANCE,
    CONFIG_GROUP_XP_LEVELDIFF,
    CONFIG_SIGHT_MONSTER,
    CONFIG_SIGHT_GUARDER,
    CONFIG_GAME_TYPE,
    CONFIG_ALLOW_TWO_SIDE_ACCOUNTS,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION,
    CONFIG_ALLOW_TWO_SIDE_WHO_LIST,
    CONFIG_MAX_PLAYER_LEVEL,
    CONFIG_IGNORE_AT_LEVEL_REQUIREMENT,
    CONFIG_MAX_PRIMARY_TRADE_SKILL,
    CONFIG_MIN_PETITION_SIGNS,
    CONFIG_GM_WISPERING_TO,
    CONFIG_GM_IN_GM_LIST,
    CONFIG_GM_IN_WHO_LIST,
    CONFIG_GM_LOGIN_STATE,
    CONFIG_GM_LOG_TRADE,
    CONFIG_GROUP_VISIBILITY,
    CONFIG_SKILL_CHANCE_ORANGE,
    CONFIG_SKILL_CHANCE_YELLOW,
    CONFIG_SKILL_CHANCE_GREEN,
    CONFIG_SKILL_CHANCE_GREY,
    CONFIG_MAX_OVERSPEED_PINGS,
    CONFIG_VALUE_COUNT
};

// bitmask
enum LogFilters
{
    LOG_FILTER_TRANSPORT_MOVES = 1,
    LOG_FILTER_CREATURE_MOVES  = 2
};

enum Rates
{
    RATE_HEALTH=0,
    RATE_POWER_MANA,
    RATE_POWER_RAGE,
    RATE_POWER_FOCUS,
    RATE_DROP_ITEMS,
    RATE_DROP_MONEY,
    RATE_XP_KILL,
    RATE_XP_QUEST,
    RATE_XP_EXPLORE,
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
    MAX_RATES
};

enum EnviromentalDamage
{
    DAMAGE_EXHAUSTED = 0,
    DAMAGE_DROWNING = 1,
    DAMAGE_FALL = 2,
    DAMAGE_LAVA = 3,
    DAMAGE_SLIME = 4,
    DAMAGE_FIRE = 5
};

// ServerMessages.dbc
enum ServerMessageType
{
    SERVER_MSG_SHUTDOWN_TIME      = 1,
    SERVER_MSG_RESTART_TIME       = 2,
    SERVER_MSG_STRING             = 3,
    SERVER_MSG_SHUTDOWN_CANCELLED = 4,
    SERVER_MSG_RESTART_CANCELLED  = 5
};

struct ScriptInfo;
struct ScriptAction
{
    Object* source;
    Object* target;
    ScriptInfo* script;
};

class World
{
    public:
        static volatile bool m_stopEvent;

        World();
        ~World();

        WorldSession* FindSession(uint32 id) const;
        void AddSession(WorldSession *s);
        bool RemoveSession(uint32 id);
        uint32 GetSessionCount() const { return m_sessions.size(); }
        Player* FindPlayerInZone(uint32 zone);

        Weather* FindWeather(uint32 id) const;
        void AddWeather(Weather *w);
        void RemoveWeather(uint32 id);

        uint32 GetPlayerLimit() const { return m_playerLimit; }
        void SetPlayerLimit(uint32 limit) { m_playerLimit = limit; }

        bool getAllowMovement() const { return m_allowMovement; }
        void SetAllowMovement(bool allow) { m_allowMovement = allow; }

        void SetMotd(const char *motd) { m_motd = motd; }
        const char* GetMotd() const { return m_motd.c_str(); }

        time_t GetGameTime() const { return m_gameTime; }

        uint16 GetConfigMaxSkillValue() const { return getConfig(CONFIG_MAX_PLAYER_LEVEL)*5; }

        void SetInitialWorldSettings();

        void SendWorldText(const char *text, WorldSession *self = 0);
        void SendGlobalMessage(WorldPacket *packet, WorldSession *self = 0);
        void SendZoneMessage(uint32 zone, WorldPacket *packet, WorldSession *self = 0);
        void SendZoneText(uint32 zone, const char *text, WorldSession *self = 0);
        void SendServerMessage(ServerMessageType type, const char *text = "", Player* player = NULL);

        bool IsShutdowning() const { return m_ShutdownTimer > 0; }
        void ShutdownServ(uint32 time, bool idle = false);
        void ShutdownCancel();
        void ShutdownMsg(bool show = false, Player* player = NULL);

        void Update(time_t diff);
        time_t GetLastTickTime() const { return m_Last_tick; }

        void setRate(Rates rate,float value) { rate_values[rate]=value; }
        float getRate(Rates rate) const { return rate_values[rate]; }

        void setConfig(uint32 index,uint32 value)
        {
            if(index<CONFIG_VALUE_COUNT)
                m_configs[index]=value;
        }

        uint32 getConfig(uint32 index) const
        {
            if(index<CONFIG_VALUE_COUNT)
                return m_configs[index];
            else
                return 0;
        }
        uint32 getLogFilter() const { return m_logFilter; }

        bool IsPvPRealm() { return (MaNGOS::Singleton<World>::Instance().getConfig(CONFIG_GAME_TYPE) == 1 || MaNGOS::Singleton<World>::Instance().getConfig(CONFIG_GAME_TYPE) == 8); }

        bool KickPlayer(std::string playerName);
        void KickAll();
        bool BanAccount(std::string nameOrIP);
        bool RemoveBanAccount(std::string nameOrIP);

        multimap<uint64, ScriptAction> scriptSchedule;
        void ScriptsStart(map<uint32, multimap<uint32, ScriptInfo> > scripts, uint32 id, Object* source, Object* target);
        void ScriptsProcess();
        uint64 internalGameTime;

    protected:

        time_t _UpdateGameTime();

    private:

        IntervalTimer m_timers[WUPDATE_COUNT];
        uint32 mail_timer;
        uint32 mail_timer_expires;

        typedef HM_NAMESPACE::hash_map<uint32, Weather*> WeatherMap;
        WeatherMap m_weathers;
        typedef HM_NAMESPACE::hash_map<uint32, WorldSession*> SessionMap;
        SessionMap m_sessions;
        std::set<WorldSession*> m_kicked_sessions;
        float rate_values[MAX_RATES];
        uint32 m_playerLimit;
        bool m_allowMovement;
        std::string m_motd;

        time_t m_gameTime;

        time_t m_nextThinkTime;
        uint32 m_configs[CONFIG_VALUE_COUNT];
        uint32 m_logFilter;
        time_t m_Last_tick;
        uint32 m_ShutdownIdleMode;
        uint32 m_ShutdownTimer;
};

extern uint32 realmID;

#define sWorld MaNGOS::Singleton<World>::Instance()
#endif
