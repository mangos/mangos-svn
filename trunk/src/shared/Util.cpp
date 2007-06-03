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

#include "Util.h"

#include "Network/socket_include.h"

using namespace std;

vector<string> StrSplit(const string &src, const string &sep)
{
    vector<string> r;
    string s;
    for (string::const_iterator i = src.begin(); i != src.end(); i++)
    {
        if (sep.find(*i) != string::npos)
        {
            if (s.length()) r.push_back(s);
            s = "";
        }
        else
        {
            s += *i;
        }
    }
    if (s.length()) r.push_back(s);
    return r;
}

std::string secsToTimeString(uint32 timeInSecs, bool shortText, bool hoursOnly)
{
    uint32 secs    = timeInSecs % MINUTE;
    uint32 minutes = timeInSecs % HOUR / MINUTE;
    uint32 hours   = timeInSecs % DAY  / HOUR;
    uint32 days    = timeInSecs / DAY;

    std::ostringstream ss;
    if(days)
        ss << days << (shortText ? "d" : " Day(s) ");
    if(hours)
        ss << hours << (shortText ? "h" : " Hour(s) ");
    if(!hoursOnly)
    {
        if(minutes)
            ss << minutes << (shortText ? "m" : " Minute(s) ");
        if(secs)
            ss << secs << (shortText ? "s" : " Second(s).");
    }

    return ss.str();
}

uint32 TimeStringToSecs(std::string timestring)
{
    uint32 secs       = 0;
    uint32 buffer     = 0;
    uint32 multiplier = 0;

    for(std::string::iterator itr = timestring.begin(); itr != timestring.end(); itr++ )
    {
        if(isdigit(*itr))
        {
            std::string str;            //very complicated typecast char->const char*; is there no better way?
            str += *itr;
            const char* tmp = str.c_str();

            buffer*=10;
            buffer+=atoi(tmp);
       }
        else
        {
            switch(*itr)
            {
                case 'd': multiplier = 60*60*24; break;
                case 'h': multiplier = 60*60;    break;
                case 'm': multiplier = 60;       break;
                case 's': multiplier = 1;        break;
                default : return 0;       //bad format
            }
            buffer*=multiplier;
            secs+=buffer;
            buffer=0;
        }

    }

    return secs;
}

/// Check if the string is a valid ip address representation
bool IsIPAddress(char const* ipaddress)
{
    if(!ipaddress)
        return false;

    // Let the big boys do it.
    // Drawback: all valid ip address formats are recognized e.g.: 12.23,121234,0xABCD)
    return inet_addr(ipaddress) != INADDR_NONE;
}

// internal status if the irand() random number generator
static uint32 holdrand = 0x89abcdef;

// initialize the irand() random number generator
void Rand_Init(uint32 seed)
{
    holdrand = seed;
}

/* Return a random number in the range min .. max.
 * max-min must be smaller than 32768. */
int32 irand(int32 min, int32 max)
{
    assert((max - min) < 32768);

    ++max;
    holdrand = (holdrand * 214013) + 2531011;

    return (((holdrand >> 17) * (max - min)) >> 15) + min;
}

// current state of the random number generator
static int32 rand32_state = 1;

/* Return a pseudo-random number in the range 0 .. RAND32_MAX.
 * Note: Not reentrant - if two threads call this simultaneously, they will likely
 * get the same random number. */
int32 rand32(void)
{
    #   define m   2147483647
    #   define a   48271
    #   define q   (m / a)
    #   define r   (m % a)

    const int32 hi = rand32_state / q;
    const int32 lo = rand32_state % q;
    const int32 test = a * lo - r * hi;

    if (test > 0)
        rand32_state = test;
    else
        rand32_state = test + m;
    return rand32_state - 1;
}
