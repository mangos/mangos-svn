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

#ifndef _UTIL_H
#define _UTIL_H

#include "Common.h"

#include <string>
#include <vector>
#include "mersennetwister/MersenneTwister.h"

static MTRand mtRand;

typedef std::vector<std::string> Tokens;

Tokens StrSplit(const std::string &src, const std::string &sep);

void stripLineInvisibleChars(std::string &src);

std::string secsToTimeString(uint32 timeInSecs, bool shortText = false, bool hoursOnly = false);
uint32 TimeStringToSecs(std::string timestring);
std::string TimeToTimestampStr(time_t t);

/* Return a random number in the range min..max; (max-min) must be smaller than 32768.
 * Note: Not reentrant - if two threads call this simultaneously, they will likely
 * get the same random number. */
inline int32 irand(int32 min, int32 max)
{
    //assert((max - min) < 32768);
    return int32(mtRand.randInt(max-min))+min;
}

/* Return a random number in the range min..max (inclusive). For reliable results, the difference
* between max and min should be less than RAND32_MAX. */
inline uint32 urand(uint32 min, uint32 max)
{
    return mtRand.randInt(max-min)+ min;
}

/* Return a random number in the range 0 .. RAND32_MAX.
 * Note: Not reentrant - if two threads call this simultaneously, they will likely
 * get the same random number. */
inline int32 rand32()
{
    return mtRand.randInt();
}

/* Return a random double from 0.0 to 1.0 (exclusive). Floats support only 7 valid decimal digits.
 * A double supports up to 15 valid decimal digits and is used internally (RAND32_MAX has 10 digits).
 * With an FPU, there is usually no difference in performance between float and double. */
inline double rand_norm(void)
{
    return mtRand.randExc();
}

/* Return a random double from 0.0 to 99.9999999999999. Floats support only 7 valid decimal digits.
 * A double supports up to 15 valid decimal digits and is used internaly (RAND32_MAX has 10 digits).
 * With an FPU, there is usually no difference in performance between float and double. */
inline double rand_chance(void)
{
    return mtRand.randExc(100.0);
}

/* Return true if a random roll fits in the specified chance (range 0-100). */
inline bool roll_chance_f(float chance)
{
    return chance > rand_chance();
}

/* Return true if a random roll fits in the specified chance (range 0-100). */
inline bool roll_chance_i(int chance)
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
    if (!apply && val == -100.0f)
        val = -99.99f;
    var *= (apply?(100.0f+val)/100.0f : 100.0f / (100.0f+val));
}

inline void strToUpper(std::string& str)
{
    std::transform( str.begin(), str.end(), str.begin(), ::toupper );
}

inline void strToLower(std::string& str)
{
    std::transform( str.begin(), str.end(), str.begin(), ::tolower );
}

inline size_t utf8length(std::string utf8str)
{
    //TODO: implement correct utf8 string length check
    //Currently use string size just for cleanup related caller code
    return utf8str.size();
}

inline bool normalizePlayerName(std::string& name)
{
    if(name.empty())
        return false;

    name[0] = toupper(name[0]);
    for(size_t i = 1; i < name.size(); ++i)
        name[i] = tolower(name[i]);

    return true;
}

bool IsIPAddress(char const* ipaddress);
uint32 CreatePIDFile(std::string filename);

#endif
