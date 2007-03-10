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

#ifndef __WEATHER_H
#define __WEATHER_H

#include "Common.h"

// weather difines
enum WeatherSounds
{
     WEATHER_NOSOUND                = 0,
     WEATHER_RAINLIGHT              = 8533,
     WEATHER_RAINMEDIUM             = 8534,
     WEATHER_RAINHEAVY              = 8535,
     WEATHER_SNOWLIGHT              = 8536,
     WEATHER_SNOWMEDIUM             = 8537,
     WEATHER_SNOWHEAVY              = 8538,
     WEATHER_SANDSTORMLIGHT         = 8556,
     WEATHER_SANDSTORMMEDIUM        = 8557,
     WEATHER_SANDSTORMHEAVY         = 8558
};

class Player;

class Weather
{
    public:
        Weather(Player *player);
        ~Weather() { m_player = NULL;};
        void ReGenerate();
        void UpdateWeather();
        void SetWeather(uint32 type, float grade);
        uint32 GetZone() { return m_zone; };
        bool Update(uint32 diff);
    private:
        uint32 GetSound();
        uint32 m_zone;
        uint32 m_type;
        float m_grade;
        Player *m_player;
        uint32 m_interval;
        uint32 m_timer;
};
#endif
