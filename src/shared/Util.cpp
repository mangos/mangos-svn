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

#include "Util.h"

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

bool IsItIP(char const* ip)
{
    if(!ip)
        return false;
    //ip looks like a.b.c.d  -- let's calc number of '.' it must be equal to 3
    //and must contain only numbers + .
    unsigned int iDotCount=0;
    unsigned int l=strlen(ip);
    for(unsigned int y=0;y<l;y++)
    {
        if(ip[y]=='.')iDotCount++;
        else
        if( (ip[y] < '0' || ip[y] > '9'))
            return false;
    }

    if(iDotCount!=3)
        return false;

    return true;
}
