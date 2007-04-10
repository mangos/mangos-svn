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

/// \addtogroup world
/// @{
/// \file

#ifndef __WEATHER_H
#define __WEATHER_H

#include "Common.h"
#include "ObjectMgr.h"
#include "Timer.h"

class Player;

/// Weather for one zone
class Weather
{
    public:
        Weather(uint32 zone, WeatherZoneChances const* weatherChances);
        ~Weather() { };
        bool ReGenerate();
        bool UpdateWeather();
        void SendWeatherUpdateToPlayer(Player *player);
        void SetWeather(uint32 type, float grade);
        /// For which zone is this weather?
        uint32 GetZone() { return m_zone; };
        bool Update(time_t diff);
    private:
        uint32 GetSound();
        uint32 m_zone;
        uint32 m_type;
        float m_grade;
        IntervalTimer m_timer;
        WeatherZoneChances const* m_weatherChances;
};
#endif
