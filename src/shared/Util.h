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

#ifndef _UTIL_H
#define _UTIL_H

#include "Common.h"

std::vector<std::string> StrSplit(const std::string &src, const std::string &sep);


// return random float from 00.000 to 99.999 (100000 variants)
inline float rand_chance() { 
    // rand() result range is 0..RAND_MAX where RAND_MAX is implementation define (at 32-bit OS in most case RAND_MAX = 32767)
    // this is small number for chances writed like xx.xxx (100000 cases)
    // using combined 2 call rand() instead: xx.yyy
    return float(rand() % 100) + float(rand() % 1000)/1000.0;
}

#endif
