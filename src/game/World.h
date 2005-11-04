/* World.h
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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
#include "Singleton.h"

class Object;
class WorldPacket;
class WorldSession;
#ifndef ENABLE_GRID_SYSTEM
class MapMgr;
#endif

enum WorldTimers
{
    WUPDATE_OBJECTS = 0,
    WUPDATE_SESSIONS = 1,
    WUPDATE_AUCTIONS = 2,
    WUPDATE_COUNT = 3
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

class World : public Singleton<World>
{
    public:
        World();
        ~World();

        WorldSession* FindSession(uint32 id) const;
        void AddSession(WorldSession *s);
        void RemoveSession(uint32 id);

        uint32 GetSessionCount() const { return m_sessions.size(); }
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

        // update the world server every frame
        void Update(time_t diff);

#ifndef ENABLE_GRID_SYSTEM
        // get map manager
        MapMgr* GetMap(uint32 id);
#endif
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

        // map text emote to spell prices
        typedef std::map< uint32, uint32> SpellPricesMap;
        SpellPricesMap mPrices;

    protected:
        // update Stuff, FIXME: use diff
        time_t _UpdateGameTime()
        {
            // Update Server time
            time_t thisTime = time(NULL);
            m_gameTime += thisTime - m_lastTick;
            m_lastTick = thisTime;

            return m_gameTime;
        }

    private:
        //! Timers
        IntervalTimer m_timers[WUPDATE_COUNT];

        typedef HM_NAMESPACE::hash_map<uint32, WorldSession*> SessionMap;
        SessionMap m_sessions;
#ifndef ENABLE_GRID_SYSTEM
        typedef HM_NAMESPACE::hash_map<uint32, MapMgr*> MapMgrMap;
        MapMgrMap m_maps;
#endif
        float regen_values[5];
        uint32 m_playerLimit;
        bool m_allowMovement;
        std::string m_motd;

        time_t m_gameTime;
        time_t m_lastTick;

        time_t m_nextThinkTime;
};

#define sWorld World::getSingleton()
#endif
