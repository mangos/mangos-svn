# Copyright (C) 2007 MaNGOS project <http://www.mangosproject.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

## Process this file with autoconf to produce a configure script.

## Prelude, basic settings for Autoconf
#  PACKAGE:            mangos
#  VERSION:            0.7.01 (trunk)
#  BUG-REPORT-ADDRESS: mangos-devs@lists.sourceforge.net
AC_INIT( [mangos], [0.7.01], [mangos-devs@lists.sourceforge.net])
AC_CONFIG_SRCDIR([src/mangosd/Main.cpp])

## Prelude, basic settings for Automake
#  Turn on all warnings and error messages, and enforce GNU
#  standards for the package.
AM_INIT_AUTOMAKE([-Wall -Werror dist-bzip2])
AM_MAINTAINER_MODE

## Check for required dependencies.

## Check for a valid build environment.
#  Valid equals having:
#  - a C++ compiler compliant with the ISO98 C++ specification.
#  - a working library tool for creating convenience libraries.
#  - a working linker for creating static and shared libraries.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_INSTALL

AC_PATH_PROG(DOXYGENPATH, doxygen)
if test "_$DOXYGENPATH" = _ ; then
    AC_MSG_WARN([doxygen is required to build the API documentation.])
fi
AM_CONDITIONAL([USE_DOXYGEN], [test "_$DOXYGENPATH" != _])

## Check for required libraries.
AC_CHECK_LIB( pthread, pthread_create, [], 
    [LDFLAGS="-pthread $LDFLAGS"
     AC_TRY_LINK([char pthread_create();],
         pthread_create();,
         [], [AC_MSG_ERROR([Missing pthread])])
    ])
AC_CHECK_LIB( z, compress, [ZLIB=-lz],[AC_MSG_ERROR([Missing zlib])] )
#AC_CHECK_LIB( mysqlclient, mysql_init, [],[AC_CHECK_LIB(mysql, mysql_init,[],[AC_MSG_ERROR([Missing mysql])])])
AC_CHECK_LIB( compat, ftime, [COMPATLIB=-lcompat] )
AC_CHECK_LIB( ssl, BN_cmp, [SSLLIB=-lssl], [AC_MSG_ERROR([Missing openssl])])

AC_ARG_WITH(mysql-libs,
[  --with-mysql-libs=DIR     Look for mysql libs in this dir ],
    MYSQL_LIBS="-L$withval -lmysqlclient"
)
AC_ARG_WITH(mysql-includes,
[  --with-mysql-includes=DIR Look for mysql includes in this dir ],
    MYSQL_INCLUDES="-I$withval"
)

AC_PATH_PROGS(MYSQL_CONFIG, mysql_config, mysql_config, $PATH)
if test -x "$MYSQL_CONFIG"
then
    # MySQL v4 uses --include while v3 uses --cflags
    MYSQL_INCLUDES="`$MYSQL_CONFIG --include`" || \
        MYSQL_INCLUDES="`$MYSQL_CONFIG --cflags`"
    MYSQL_LIBS="`$MYSQL_CONFIG --libs`"
fi

## Check for required header files.
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS([ arpa/inet.h fcntl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/socket.h sys/timeb.h sys/time.h termios.h unistd.h  ])

## Check for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT64_T
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])

## Check for required library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit ftime gethostbyaddr gethostbyname gethostname gettimeofday memmove memset pow realpath select socket sqrt strchr strdup strerror strstr])

## Export defined variables
AC_SUBST(ZLIB)
AC_SUBST(COMPATLIB)
AC_SUBST(SSLLIB)
AC_SUBST(MYSQL_INCLUDES)
AC_SUBST(MYSQL_LIBS)
AC_SUBST(DOXYGENPATH)

## Set output files.
AC_CONFIG_HEADERS( [config.h] )
AC_CONFIG_FILES([
   ./Makefile
   doc/Doxyfile
   doc/Makefile
   sql/Makefile
   src/Makefile
   src/shared/Makefile
   src/shared/SystemConfig.h
   src/realmd/Makefile
   src/realmd/realmd.conf
   src/framework/Makefile
   src/game/Makefile
   src/bindings/Makefile
   src/bindings/universal/Makefile
   src/mangosd/Makefile
   src/mangosd/mangosd.conf
   dep/Makefile
   dep/src/Makefile
   dep/src/zlib/Makefile
   dep/src/zthread/Makefile
	])

## Output files.
AC_OUTPUT
