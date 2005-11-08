/* Common.h
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

#pragma warning(disable:4996)

#ifndef __SHOW_STUPID_WARNINGS__
// UQ1: Remove variable conversion warnings...
// UQ1: warning C4244: 'argument' : conversion from X to X, possible loss of data
#pragma warning(disable:4244)
// UQ1: warning C4267: '=' : conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable:4267)
// UQ1: warning C4800: forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable:4800)
// UQ1: warning C4018: '<' : signed/unsigned mismatch
#pragma warning(disable:4018)
// UQ1: warning C4311: 'type cast' : pointer truncation
#pragma warning(disable:4311)
// UQ1: warning C4305: 'argument' : truncation from 'double' to 'float'
#pragma warning(disable:4305)
// UQ1: warning C4005: #define X macro redefinition
#pragma warning(disable:4005)
#endif //__SHOW_STUPID_WARNINGS__

// Only clients with this version will be allowed to view the realmlist.
// Higher versions will be rejected, lower versions will be patched if possible.

#define EXPECTED_MANGOS_CLIENT_BUILD        {4544,4500,4565/*1.6.x*/,4671/*1.7.0*/,4735/*1.8.0*/, 4769/*1.8.1*/, 4784/*1.8.2*/}
// 1.6.0 > 4500,4449,4442,4375,4364,4341,4284,4279,4222,4150,4125,4115,4062,4044,3989,3988,3925,3892,3810,3807,0
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Utilities/HashMap.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <errno.h>

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
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <sys/types.h>
#  include <sys/ioctl.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
#  include <signal.h>
#  include <netdb.h>
#endif


#include "MemoryLeaks.h"


#if COMPILER == COMPILER_MICROSOFT

#define I64FMT "%016I64X"
#define I64FMTD "%I64u"
#define SI64FMTD "%I64d"
#define snprintf _snprintf
#define atoll __atoi64

#else

#define stricmp strcasecmp
#define strnicmp strncasecmp
#define I64FMT "%016llX"
#define I64FMTD "%llu"
#define SI64FMTD "%lld"
#endif

#define GUID_HIPART(x) (*(((uint32*)&(x))+1))
#define GUID_LOPART(x) (*((uint32*)&(x)))

#define atol(a) strtoul( a, NULL, 10)

#define STRINGIZE(a) #a

// fix buggy MSVC's for variable scoping to be reliable =S
#define for if(true) for

#define LOGOUTDELAY 600 // ~ 1 min

#endif
