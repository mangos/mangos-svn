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


/* Return a random number in the range min..max; (max-min) must be smaller than 32768. 
 * Note: Not reentrant - if two threads call this simultaneously, they will likely 
 * get the same random number. */
extern int32 irand(int32 min, int32 max);

/* Return a random number in the range min..max; (max-min) must be smaller than 32768. 
 * Note: Not reentrant - if two threads call this simultaneously, they will likely 
 * get the same random number. */
inline uint32 urand(uint32 min, uint32 max)
{
    return irand(int32(min), int32(max));
}

/* maximum number that can come out of the rand32 generator */
#define RAND32_MAX  2147483645

/* Return a random number in the range 0 .. RAND32_MAX. 
 * Note: Not reentrant - if two threads call this simultaneously, they will likely 
 * get the same random number. */
extern int32 rand32(void);

/* Return a random double from 0.0 to 1.0 (exclusive). Floats support only 7 valid decimal digits.
 * A double supports up to 15 valid decimal digits and is used internaly (RAND32_MAX has 10 digits).
 * With an FPU, there is usually no difference in performance between float and double. */
inline double rand_norm(void)
{
    return double(rand32()) / double(RAND32_MAX+1);
}

/* Return a random number in the range min..max (inclusive). For reliable results, the difference 
 * between max and min should be less than RAND32_MAX. */
inline uint32 rand32(const uint32 min, const uint32 max) 
{
    return (uint32)rand_norm() * (max-min+1) + min;
}

/* Return a random double from 0.0 to 99.9999999999999. Floats support only 7 valid decimal digits.
 * A double supports up to 15 valid decimal digits and is used internaly (RAND32_MAX has 10 digits).
 * With an FPU, there is usually no difference in performance between float and double. */
inline double rand_chance(void)
{
    return double(rand32()) / (double(RAND32_MAX+1) / 100.0);
}

/* Return true if a random roll fits in the specified chance (range 0-100). */
inline bool roll_chance_f(const float& chance)
{
    return chance > rand_chance();
}

/* Return true if a random roll fits in the specified chance (range 0-100). */
inline bool roll_chance_i(int const& chance)
{
    return chance > irand(0, 99);
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
