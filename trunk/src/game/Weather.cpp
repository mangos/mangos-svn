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

#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "World.h"
#include "Weather.h"
#include "EventSystem.h"
#include "Config/ConfigEnv.h"

/*void HandleWeather(void *weather)
{
    Weather *wth = ((Weather*)weather);
    wth->ReGenerate();
}*/

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
        case 3:                                             //snow
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

Weather::Weather(Player *player) : m_player(player), m_zone( player->GetZoneId())
{
	m_interval = sConfig.GetIntDefault("ChangeWeatherInterval", WEATHER_CHANGE_TIME);
	ReGenerate();
    sLog.outString( "WORLD: Starting weather system(change per %u minutes).", (uint32)(m_interval / 60000) );
	m_timer = m_interval;
	//AddEvent(&HandleWeather, NULL, 420000, false, true);
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
	if(m_timer == 0 )
	{
		ReGenerate();
		m_timer = m_interval;
	}
	if(!m_player)
		return false;
	return true;
}

void Weather::ReGenerate()
{
    bool notchange = !(rand() % 30);
    if(notchange)
        return;
    m_type = 0;
    m_grade = (float)rand() / (float)RAND_MAX;
    uint32 gtime = sWorld.GetGameTime();
    uint32 season = (gtime / (91 * 360)) % 4;
    sLog.outDebug("Generate random weather for season %u.", season);
    QueryResult *result;
    result = sDatabase.PQuery("SELECT * FROM `game_weather` WHERE `zone` = '%u';", m_zone);
    if (!result)
    {
        return;
    }

    uint32 chance1, chance2, chance3;
    Field *fields = result->Fetch();
    chance1 = fields[season+1].GetUInt32();
    chance2 = fields[season+2].GetUInt32();
    chance3 = fields[season+3].GetUInt32();
    delete result;
    if(chance1 > 100)
        chance1 =25;
    if(chance2 > 100)
        chance2 =25;
    if(chance3 > 100)
        chance3 =25;

    chance2 = chance1 + chance2;
    chance3 = chance2 + chance3;

    uint32 rnd = rand() % 100;
    if(rnd < chance1)
        m_type = 1;
    else if(rnd < chance2)
        m_type = 2;
    else if(rnd < chance3)
        m_type = 3;
    else
        m_type =0;
    ChangeWeather();
}

void Weather::ChangeWeather()
{	
	//if(!m_player || !m_player->IsInWorld() || m_player->GetZoneId() != m_zone)
	//{
		m_player = sWorld.FindPlayerInZone(m_zone);
		if(!m_player)
			return;
	//}

    WorldPacket data;
    uint32 sound = GetSound();
    data.Initialize( SMSG_WEATHER );
    data << (uint32)m_type << (float)m_grade << (uint32)sound;
    m_player->GetSession()->SendPacket( &data );
    char* wthstr;
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
    sLog.outString("Change the weather of zone %u to %s.", m_zone, wthstr);
}
