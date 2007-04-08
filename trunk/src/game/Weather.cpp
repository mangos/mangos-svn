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

#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "World.h"
#include "Weather.h"
#include "EventSystem.h"
#include "Config/ConfigEnv.h"
#include "Chat.h"

uint32 Weather::GetSound()
{
    uint32 sound;
    switch(m_type)
    {
        case 1:                                             //rain
            if(m_grade<0.33333334f)
                sound = WEATHER_RAINLIGHT;
            else if(m_grade<0.6666667f)
                sound = WEATHER_RAINMEDIUM;
            else
                sound = WEATHER_RAINHEAVY;
            break;
        case 2:                                             //snow
            if(m_grade<0.33333334f)
                sound = WEATHER_SNOWLIGHT;
            else if(m_grade<0.6666667f)
                sound = WEATHER_SNOWMEDIUM;
            else
                sound = WEATHER_SNOWHEAVY;
            break;
        case 3:                                             //storm
            if(m_grade<0.33333334f)
                sound = WEATHER_SANDSTORMLIGHT;
            else if(m_grade<0.6666667f)
                sound = WEATHER_SANDSTORMMEDIUM;
            else
                sound = WEATHER_SANDSTORMHEAVY;
            break;
        case 0:                                             //fine
        default:
            sound = WEATHER_NOSOUND;
            break;
    }
    return sound;
}

Weather::Weather(uint32 zone) : m_zone(zone)
{
    m_weatherChances = objmgr.GetWeatherChances(zone);
    m_interval = sWorld.getConfig(CONFIG_INTERVAL_CHANGEWEATHER);
    m_type = 0;
    m_grade = 0;

    sLog.outString( "WORLD: Starting weather system for zone %u (change per %u minutes).",m_zone, (uint32)(m_interval / 60000) );
    m_timer = m_interval;
}

bool Weather::Update(uint32 diff)
{
    if(m_timer > 0)
    {
        if(diff > m_timer)
            m_timer = 0;
        else
            m_timer -= diff;
    }

    // not required update
    if(m_timer != 0 )
        return true;

    // update only if Regenerate change weather
    if(ReGenerate())
    {
        // will be removed if not updated (no players)
        if(!UpdateWeather())
            return false;
    }

    m_timer = m_interval;
    return true;
}

bool Weather::ReGenerate()
{
    if (!m_weatherChances)
    {
        m_type = 0;
        m_grade = 0.0;
        return false;
    }

    // Weather statistics:
    // 30% - no change
    // 30% - weather worsens
    // 30% - weather gets better
    // 10% - radical change
    uint32 u = urand(0, 99);

    if (u < 30)
        return false;

    // remember old values
    uint32 old_type = m_type;
    float old_grade = m_grade;

    m_grade = rand_norm();
    time_t gtime = sWorld.GetGameTime();
    uint32 season = (gtime / (91 * 360)) % 4;
    
    static char const* seasonName[WEATHER_SEASONS] = { "spring", "summer", "fall", "winter" };

    sLog.outDebug("Generating a change in %s weather for zone %u.", seasonName[season], m_zone);

    if ((u < 60) && (m_grade < 0.33333334f))                // Get fair
    {
        m_type = 0;
    }

    if (m_type == 0)
    {
        m_grade = 0.0;
    }

    if ((u < 60) && (m_type != 0))                          // Get better
    {
        m_grade -= 0.33333334f;
        return true;
    }

    if ((u < 90) && (m_type != 0))                          // Get worse
    {
        m_grade += 0.33333334f;
        return true;
    }

    if (m_type != 0)
    {
        // Severe change, and already doing something
        if (m_grade < 0.33333334f)
        {
            m_grade = 0.9999;                               // go nuts
            return true;
        }
        else
        {
            if (m_grade > 0.6666667f)
            {
                                                            // Severe change, but how severe?
                uint32 rnd = urand(0,99);
                if (rnd < 50)
                {
                    m_grade -= 0.6666667;
                    return true;
                }
            }
            m_type = 0;                                     // clear up
            m_grade = 0;
        }
    }

    // At this point, only weather that isn't doing anything remains but that have weather data
    uint32 chance1 =          m_weatherChances->data[season].rainChance;
    uint32 chance2 = chance1+ m_weatherChances->data[season].snowChance;
    uint32 chance3 = chance2+ m_weatherChances->data[season].stormChance;

    uint32 rnd = urand(0, 99);
    if(rnd <= chance1)
        m_type = 1;
    else if(rnd <= chance2)
        m_type = 2;
    else if(rnd <= chance3)
        m_type = 3;
    else
        m_type = 0;

    if (m_type == 0)
    {
        m_grade = 0.0;
    }
    else if (u < 90)
    {
        m_grade = rand_norm() * 0.3333;
    }
    else
    {
        // Severe change, but how severe?
        rnd = urand(0, 99);
        if (rnd < 50)
            m_grade = rand_norm() * 0.3333 + 0.3334;
        else
            m_grade = rand_norm() * 0.3333 + 0.6667;
    }

    // return true only in case weather changes
    return m_type != old_type || m_grade != old_grade;
}

void Weather::SendWeatherUpdateToPlayer(Player *player)
{
    uint32 sound = GetSound();
    WorldPacket data( SMSG_WEATHER, (4+4+4) );

    data << (uint32)m_type << (float)m_grade << (uint32)sound;
    player->GetSession()->SendPacket( &data );
}

bool Weather::UpdateWeather()
{
    Player* player = sWorld.FindPlayerInZone(m_zone);
    if(!player)
        return false;

    uint32 sound = GetSound();
    if (m_grade >= 1)
        m_grade = 0.9999;
    else if (m_grade < 0)
        m_grade = 0.0001;

    WorldPacket data( SMSG_WEATHER, (4+4+4) );
    data << (uint32)m_type << (float)m_grade << (uint32)sound;
    player->SendMessageToSet( &data, true );

    char const* wthstr;
    switch(sound)
    {
        case WEATHER_RAINLIGHT:
            wthstr = "light rain";
            break;
        case WEATHER_RAINMEDIUM:
            wthstr = "medium rain";
            break;
        case WEATHER_RAINHEAVY:
            wthstr = "heavy rain";
            break;
        case WEATHER_SNOWLIGHT:
            wthstr = "light snow";
            break;
        case WEATHER_SNOWMEDIUM:
            wthstr = "medium snow";
            break;
        case WEATHER_SNOWHEAVY:
            wthstr = "heavy snow";
            break;
        case WEATHER_SANDSTORMLIGHT:
            wthstr = "light storm";
            break;
        case WEATHER_SANDSTORMMEDIUM:
            wthstr = "medium storm";
            break;
        case WEATHER_SANDSTORMHEAVY:
            wthstr = "heavy storm";
            break;
        case WEATHER_NOSOUND:
        default:
            wthstr = "fine";
            break;
    }
    sLog.outDetail("Change the weather of zone %u to %s.", m_zone, wthstr);

    return true;
}

void Weather::SetWeather(uint32 type, float grade)
{
    if(m_type == type && m_grade == grade)
        return;

    m_type = type;
    m_grade = grade;
    UpdateWeather();
}
