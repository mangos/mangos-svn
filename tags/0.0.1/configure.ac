# Copyright (C) 2004 WoW Daemon
# Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

## Process this file with autoconf to produce a configure script.

AC_INIT([MaNGOS],
        [0.0.1],
        [Massive Network Game Object Server],
        [mangos])

AC_PREREQ(2.5)

if test -d obj; then rm -rf obj; fi

AM_CONFIG_HEADER( config.h )
INCLUDES="-I/usr/include/mysql -I/usr/local/include/mysql -I/usr/include/openssl -I/usr/local/include/openssl"
AC_SUBST(INCLUDES)
LDFLAGS="-L/usr/lib/mysql -L/usr/local/lib/mysql"
AC_SUBST(LDFLAGS)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB( pthread, pthread_create, [],
    [LDFLAGS="-pthread $LDFLAGS"
     AC_TRY_LINK([char pthread_create();],
        pthread_create();,
        [], [AC_MSG_ERROR([Missing pthread])])
    ])
AC_CHECK_LIB( z, compress, [],[AC_MSG_ERROR([Missing zlib])] )
AC_CHECK_LIB( compat, ftime )
AC_CHECK_LIB( mysqlclient, mysql_init, [],[AC_CHECK_LIB(mysql, mysql_init,[],[AC_MSG_ERROR([Missing mysql])])])
AC_CHECK_LIB( ssl, SHA1_Init, [], [AC_CHECK_LIB(ssl, SHA1_Init,[],[AC_MSG_ERROR([Missing openssl])])])

CPPFLAGS_SAVE=$CPPFLAGS
CPPFLAGS=$INCLUDES $CPPFLAGS

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_CHECK_HEADERS(                                             \
   limits.h sys/ioctl.h unistd.h fcntl.h float.h mysql.h      \
   malloc.h netdb.h netinet/in.h stddef.h sys/socket.h        \
   sys/time.h sys/timeb.h opensslv.h                          \
 ,[],[AC_MSG_ERROR([Missing required header])])

CPPFLAGS=$CPPFLAGS_SAVE

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_TYPE_SIZE_T
AC_TYPE_OFF_T
AC_TYPE_SIGNAL
AC_STRUCT_TM
AC_CHECK_TYPES([ ptrdiff_t ])

# Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_STRCOLL
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STRFTIME
AC_FUNC_STRNLEN
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(                                               \
   ftime strcspn strtoul atexit bzero floor gethostbyname     \
   gettimeofday localtime_r memset rint select socket         \
   sqrt stpcpy strchr strpbrk strrchr strstr strtol strtoul   \
)

if test $sysconfdir = '${prefix}/etc'; then
	MANGOSD_CONFIG_TEMP="/etc/mangos/mangosd.conf"
else
  if test $sysconfdir = '/'; then
	MANGOSD_CONFIG_TEMP="/mangosd.conf"
  else
	MANGOSD_CONFIG_TEMP="$sysconfdir/mangosd.conf"
  fi
fi

if test $datadir = '${prefix}/share'; then
	MANGOSD_DATA_TEMP="/etc/mangos"
else
   if test $datadir = '/'; then
   	MANGOSD_DATA_TEMP="/mangos"
   else
	MANGOSD_DATA_TEMP="$datadir/mangos"
   fi
fi

eval "MANGOSD_CONFIG=$MANGOSD_CONFIG_TEMP"
eval "MANGOSD_DATA=$MANGOSD_DATA_TEMP"

AC_SUBST(MANGOSD_CONFIG)
AC_SUBST(MANGOSD_DATA)
AC_SUBST(VERSION)

AC_CONFIG_FILES([
   ./Makefile
   doc/Makefile
   src/Makefile
   src/mangosd/Master.h
   src/mangosd/Makefile
   src/shared/Makefile
   src/realmlist/Makefile
   src/game/Makefile
   dep/Makefile
   dep/src/Makefile
   dep/src/zlib/Makefile
   dep/src/zthread/Makefile
])

AC_OUTPUT
