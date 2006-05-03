/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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

enum Rates
{
    RATE_HEALTH=0,
    RATE_POWER1,
    RATE_POWER2,
    RATE_POWER3,
    RATE_DROP,
    RATE_XP,
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

class World
{
    public:
        World();
        ~World();

        WorldSession* FindSession(uint32 id) const;
        void AddSession(WorldSession *s);
        void RemoveSession(uint32 id);
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

        void SetInitialWorldSettings();

        void SendWorldText(const char *text, WorldSession *self = 0);
        void SendGlobalMessage(WorldPacket *packet, WorldSession *self = 0);

        void Update(time_t diff);

        void setRate(int index,float value)
        {
            if((index>=0)&&(index<MAX_RATES))
                regen_values[index]=value;
        }

        float getRate(int index)
        {
            if((index>=0)&&(index<MAX_RATES))
                return regen_values[index];
            else
                return 0;
        }

    protected:

        time_t _UpdateGameTime()
        {

            time_t thisTime = time(NULL);
            m_gameTime += thisTime - m_lastTick;
            m_lastTick = thisTime;

            return m_gameTime;
        }

    private:

        IntervalTimer m_timers[WUPDATE_COUNT];

        typedef HM_NAMESPACE::hash_map<uint32, Weather*> WeatherMap;
        WeatherMap m_weathers;
        typedef HM_NAMESPACE::hash_map<uint32, WorldSession*> SessionMap;
        SessionMap m_sessions;
        float regen_values[5];
        uint32 m_playerLimit;
        bool m_allowMovement;
        std::string m_motd;

        time_t m_gameTime;
        time_t m_lastTick;

        time_t m_nextThinkTime;
};

#define sWorld MaNGOS::Singleton<World>::Instance()
#endif
