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

#ifndef MANGOS_DEFINE_H
#define MANGOS_DEFINE_H

#include "Platform/CompilerDefs.h"
#include <sys/types.h>

#ifdef WIN32
#define MANGOS_EXPORT __declspec(dllexport)
#define MANGOS_LIBRARY_HANDLE HMODULE
#define MANGOS_LOAD_LIBRARY(a) LoadLibrary(a)
#define MANGOS_CLOSE_LIBRARY FreeLibrary
#define MANGOS_GET_PROC_ADDR GetProcAddress
#define MANGOS_IMPORT __cdecl
#define MANGOS_SCRIPT_EXT ".dll"
#define MANGOS_SCRIPT_NAME "MaNGOSScript"
#define SIGQUIT 3
#else
#define MANGOS_LIBRARY_HANDLE void*
#define MANGOS_EXPORT export
#define MANGOS_LOAD_LIBRARY(a) dlopen(a,RTLD_NOW)
#define MANGOS_CLOSE_LIBRARY dlclose
#define MANGOS_GET_PROC_ADDR dlsym

#if defined(__APPLE_CC__) && defined(BIG_ENDIAN)
#define MANGOS_IMPORT __attribute__ ((longcall))
#else
#define MANGOS_IMPORT __attribute__ ((cdecl))
#endif

#define MANGOS_SCRIPT_EXT ".so"
#define MANGOS_SCRIPT_NAME "libmangosscript"
#endif

#ifdef WIN32
#ifdef MANGOS_WIN32_DLL_IMPORT

#define MANGOS_DLL_DECL __declspec(dllimport)
#else
#ifdef MANGOS_WIND_DLL_EXPORT
#define MANGOS_DLL_DECL __declspec(dllexport)
#else
#define MANGOS_DLL_DECL
#endif
#endif

#else
#define MANGOS_DLL_DECL
#endif

#ifndef DEBUG
#define MANGOS_INLINE inline
#else
#ifndef MANGOS_DEBUG
#define MANGOS_DEBUG
#endif
#define MANGOS_INLINE
#endif

#if COMPILER == COMPILER_MICROSOFT
typedef __int64            int64;
typedef long               int32;
typedef short              int16;
typedef char               int8;
typedef unsigned __int64   uint64;
typedef unsigned long      uint32;
typedef unsigned short     uint16;
typedef unsigned char      uint8;
#else
//typedef long long int64;
typedef __int64_t   int64;
typedef __int32_t   int32;
typedef __int16_t   int16;
typedef __int8_t    int8;
typedef __uint64_t  uint64;
typedef __uint32_t  uint32;
typedef __uint16_t  uint16;
typedef __uint8_t   uint8;
typedef uint16      WORD;
typedef uint32      DWORD;
#endif
typedef uint64      OBJECT_HANDLE;

#if PLATFORM == PLATFORM_WIN32
#  ifndef DECLSPEC_NORETURN
#    define DECLSPEC_NORETURN __declspec(noreturn)
#  endif
#else
#  define DECLSPEC_NORETURN
#endif

#if COMPILER == COMPILER_GNU
#  define ATTR_NORETURN __attribute__((noreturn))
#  define ATTR_PRINTF(F,V) __attribute__ ((format (printf, F, V)))
#else
#  define ATTR_NORETURN
#  define ATTR_PRINTF(F,V)
#endif
#endif
