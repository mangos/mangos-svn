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

#ifndef MANGOSSERVER_COMMON_H
#define MANGOSSERVER_COMMON_H

#include "Platform/Define.h"

#if COMPILER == COMPILER_MICROSOFT

#pragma warning(disable:4996)

#ifndef __SHOW_STUPID_WARNINGS__

#pragma warning(disable:4244)

#pragma warning(disable:4267)

#pragma warning(disable:4800)

#pragma warning(disable:4018)

#pragma warning(disable:4311)

#pragma warning(disable:4305)

#pragma warning(disable:4005)
#endif                                                      // __SHOW_STUPID_WARNINGS__
#endif                                                      // __GNUC__

// we need to stick to 1 version or half of the stuff will work for someone
// others will not and opposite
// will only support WoW and WoW:TBC 2.1.3 client build 6898...

#define EXPECTED_MANGOS_CLIENT_BUILD        {6898, 0}

// must be the first thing to include for it to work
#include "MemoryLeaks.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Utilities/HashMap.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <signal.h>

#if PLATFORM == PLATFORM_WIN32
#define STRCASECMP stricmp
#else
#define STRCASECMP strcasecmp
#endif

#include <set>
#include <list>
#include <string>
#include <map>
#include <queue>
#include <sstream>
#include <algorithm>

#include <zthread/FastMutex.h>
#include <zthread/LockedQueue.h>
#include <zthread/Runnable.h>
#include <zthread/Thread.h>

#if PLATFORM == PLATFORM_WIN32
#  define FD_SETSIZE 1024
#  include <winsock2.h>
// XP winver - needed to compile with standard leak check in MemoryLeaks.h
// uncomment later if needed
//#define _WIN32_WINNT 0x0501
#  include <ws2tcpip.h>
//#undef WIN32_WINNT
#else
#  include <sys/types.h>
#  include <sys/ioctl.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
#  include <netdb.h>
#endif

#if COMPILER == COMPILER_MICROSOFT

#include <float.h>

#define I64FMT "%016I64X"
#define I64FMTD "%I64u"
#define SI64FMTD "%I64d"
#define snprintf _snprintf
#define atoll __atoi64
#define vsnprintf _vsnprintf
#define strdup _strdup
#define finite(X) _finite(X)

#else

#define stricmp strcasecmp
#define strnicmp strncasecmp
#define I64FMT "%016llX"
#define I64FMTD "%llu"
#define SI64FMTD "%lld"

#endif

#define GUID_HIPART(x)   (uint32)((x) >> 32)
#define GUID_LOPART(x)   (uint32)((x) & 0xFFFFFFFFULL)
#define MAKE_GUID(l, h)  uint64( uint32(l) | ( uint64(h) << 32 ) )

#define atol(a) strtoul( a, NULL, 10)

#define STRINGIZE(a) #a

#define for if(true) for

enum TimeConstants
{
    MINUTE = 60,
    HOUR   = MINUTE*60,
    DAY    = HOUR*24,
    MONTH  = DAY*30
};

#if PLATFORM == PLATFORM_WIN32
#  define MANGOS_DLL_SPEC __declspec(dllexport)
#else
#  define MANGOS_DLL_SPEC
#endif

// we always use stdlibc++ std::max/std::min, undefine some not C++ standart defines (Win API and some pother platforms)
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#endif

