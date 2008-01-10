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

#ifndef MANGOS_TIMER_H
#define MANGOS_TIMER_H

#include "Platform/CompilerDefs.h"

#if PLATFORM == PLATFORM_WIN32
#   include <windows.h>
#   include <mmsystem.h>
#   include <time.h>
#else
# if defined(__APPLE_CC__)
#   include <time.h>
# endif
#   include <sys/time.h>
#   include <sys/timeb.h>
#endif

#if PLATFORM == PLATFORM_WIN32
inline uint32 getMSTime() { return GetTickCount(); }
#else
inline uint32 getMSTime()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
#endif

class IntervalTimer
{
    public:
        IntervalTimer() : _interval(0), _current(0) {}

        void Update(time_t diff) { _current += diff; if(_current<0) _current=0;}
        bool Passed() { return _current >= _interval; }
        void Reset() { if(_current >= _interval) _current -= _interval;  }

        void SetCurrent(time_t current) { _current = current; }
        void SetInterval(time_t interval) { _interval = interval; }
        time_t GetInterval() const { return _interval; }
        time_t GetCurrent() const { return _current; }

    private:
        time_t _interval;
        time_t _current;
};

struct TimeTracker
{

    TimeTracker(time_t expiry) : i_expiryTime(expiry) {}
    void Update(time_t diff) { i_expiryTime -= diff; }
    bool Passed(void) const { return (i_expiryTime <= 0); }
    void Reset(time_t interval) { i_expiryTime = interval; }
    time_t GetExpiry(void) const { return i_expiryTime; }
    time_t i_expiryTime;
};
#endif
