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

#ifndef _UTIL_H
#define _UTIL_H

#include "Common.h"

std::vector<std::string> StrSplit(const std::string &src, const std::string &sep);

// return random float from 00.000000 to 99.999999 (100000000 variants)
inline float rand_chance()
{
    // rand() result range is 0..RAND_MAX where RAND_MAX is implementation define (at 32-bit OS in most case RAND_MAX = 32767)
    // this is small number for chances writed like xx.xxxxxx (100000000 cases)
    // using combined 2 call rand() instead: xx.xxyyyy
    return float(rand() % 10000)/100.0 + float(rand() % 10000)/1000000.0;
}

inline void ApplyModUInt32Var(uint32& var, int32 val, bool apply)
{
    int32 cur = var;
    cur += (apply ? val : -val);
    if(cur < 0)
        cur = 0;
    var = cur;
}

inline void ApplyModFloatVar(float& var, float  val, bool apply)
{
    var += (apply ? val : -val);
    if(var < 0)
        var = 0;
}

inline void ApplyPercentModFloatVar(float& var, float val, bool apply)
{
    var *= (apply?(100.0f+val)/100.0f : 100.0f / (100.0f+val));
}

inline void normalizePlayerName(std::string& name)
{
    assert(name.size() > 0);
    name[0] = toupper(name[0]);
    for(size_t i = 1; i < name.size(); ++i)
        name[i] = tolower(name[i]);
}

bool IsIPAddress(char const* ipaddress);
#endif
