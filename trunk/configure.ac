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
#  VERSION:            0.9.0 (trunk)
#  BUG-REPORT-ADDRESS: mangos-devs@lists.sourceforge.net
AC_INIT( [mangos], [0.9.0], [mangos-devs@lists.sourceforge.net])
AC_CONFIG_SRCDIR([src/shared/Base.cpp])

## Prelude, basic settings for Automake
#  Turn on all warnings and error messages, and enforce GNU
#  standards for the package.
AM_INIT_AUTOMAKE([-Wall -Werror])
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
AC_CHECK_LIB( compat, ftime, [COMPATLIB=-lcompat] )
AC_CHECK_LIB( crypto, SHA1_Init, [SSLLIB=-lssl], [AC_MSG_ERROR([Missing openssl])])

AC_ARG_WITH(postgresql,
[  --with-postgresql       Use PostgreSQL as a backend (default: no)],
[case "${withval}" in
	yes) DO_POSTGRESQL=yes ;;
	no)  DO_POSTGRESQL=no ;;
	maybe) DO_POSTGRESQL=maybe ;;
	*)   AC_MSG_ERROR(Bad value ${withval} for --with-postgresql) ;;
    esac],
[DO_POSTGRESQL=no])
				 
AC_ARG_WITH(mysql,
[  --with-mysql            Use MySQL as a backend (default: yes)],
[case "${withval}" in
	yes) DO_MYSQL=yes ;;
	no)  DO_MYSQL=no ;;
	maybe) DO_MYSQL=maybe ;;
	*)   AC_MSG_ERROR(Bad value ${withval} for --with-mysql) ;;
    esac],
[DO_MYSQL=yes])

# here Postgre
AC_MSG_CHECKING(whether to build/link POSTGRESQL)
if test "x$DO_POSTGRESQL" = "xyes"; then
DO_MYSQL=no
POSTGRE_INCLUDES="-I/usr/include/postgresql $POSTGRE_INCLUDES"
POSTGRE_LIBS="-L/usr/lib/postresql -lpq -lz -lpthread -lcrypt -lnsl -lm -lpthread -L/usr/lib -lssl -lcrypto $POSTGRE_LIBS "
CXXFLAGS="-DDO_POSTGRESQL $CXXFLAGS"
fi
AC_MSG_RESULT($DO_POSTGRESQL)
	
# here mysql
AC_MSG_CHECKING(whether to build/link MYSQL)
if test "x$DO_MYSQL" = "xyes"; then
AC_MSG_RESULT($DO_MYSQL)
AC_PATH_PROGS(MYSQL_CONFIG, mysql_config, mysql_config, $PATH)
    if test -x "$MYSQL_CONFIG"
        then
    # MySQL v4 uses --include while v3 uses --cflags
        MYSQL_INCLUDES="`$MYSQL_CONFIG --include`" || \
	    MYSQL_INCLUDES="`$MYSQL_CONFIG --cflags`"
        MYSQL_LIBS="`$MYSQL_CONFIG --libs_r`"
	CXXFLAGS="-DDO_MYSQL $CXXFLAGS"
    fi
else
AC_MSG_RESULT($DO_MYSQL)
fi

## Check for options
#  Include debug info in library?
AC_MSG_CHECKING(whether to include debug info in library)
MANGOSD_DEBUG_INFO=no
AC_ARG_WITH(debug-info,
[
Debugging options:

  --with-debug-info       Include debug info in library],
[
    if test "$withval" = "yes" ; then
	CFLAGS="-g -DMANGOS_DEBUG $CFLAGS"
	CXXFLAGS="-g -DMANGOS_DEBUG $CXXFLAGS"
	MANGOSD_DEBUG_INFO=yes
    elif test "$withval" != "no" ; then
	AC_MSG_ERROR(Please choose yes or no)
    fi
])
AC_MSG_RESULT($MANGOSD_DEBUG_INFO)


# Enable CLI console?
AC_MSG_CHECKING(whether cli console is enabled)
MANGOSD_ENABLE_CLI=no
AC_ARG_ENABLE(cli,
[  --enable-cli Turn on command console system],
[
    if test "$enableval" = "yes" ; then
	CFLAGS="-DENABLE_CLI $CFLAGS"
	CXXFLAGS="-DENABLE_CLI $CXXFLAGS"
	MANGOSD_ENABLE_CLI=yes
    elif test "$withval" != "no" ; then
	AC_MSG_ERROR(Please choose yes or no)
    fi
])
AC_MSG_RESULT($MANGOSD_ENABLE_CLI)

# Enable remote console?
AC_MSG_CHECKING(whether remote console is enabled)
MANGOSD_ENABLE_RA=no
AC_ARG_ENABLE(ra,
[  --enable-ra Turn on remote console system],
[
    if test "$enableval" = "yes" ; then
	CFLAGS="-DENABLE_RA $CFLAGS"
	CXXFLAGS="-DENABLE_RA $CXXFLAGS"
	MANGOSD_ENABLE_RA=yes
    elif test "$withval" != "no" ; then
	AC_MSG_ERROR(Please choose yes or no)
    fi
])
AC_MSG_RESULT($MANGOSD_ENABLE_RA)

## Check for required header files.
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS([ arpa/inet.h fcntl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/socket.h sys/timeb.h sys/time.h termios.h unistd.h  ])

AC_CHECK_HEADERS([pthread.h])
AC_CHECK_HEADERS([openssl/md5.h openssl/rand.h openssl/ssl.h openssl/sha.h openssl/bn.h])
AC_CHECK_HEADERS([mysql.h mysql/mysql.h])
AC_CHECK_HEADERS([libpq-fe.h])
AC_CHECK_HEADERS([zlib.h])

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
AC_SUBST(POSTGRE_INCLUDES)
AC_SUBST(POSTGRE_LIBS)
AC_SUBST(DOXYGENPATH)
AC_SUBST(MANGOSD_DEBUG_INFO)
AC_SUBST(MANGOSD_ENABLE_CLI)
AC_SUBST(MANGOSD_ENABLE_RA)

## Set output files.
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
   dep/include/Makefile
   dep/lib/Makefile
   dep/src/Makefile
   dep/src/g3dlite/Makefile
   dep/src/zlib/Makefile
   dep/src/zthread/Makefile
   dep/Makefile
   doc/Doxyfile
   doc/Makefile
   Makefile
   sql/Makefile
   sql/updates/Makefile
   src/Makefile
   src/framework/Makefile
   src/shared/Makefile
   src/shared/Auth/Makefile
   src/shared/Config/Makefile
   src/shared/Database/Makefile
   src/shared/Network/Makefile
   src/shared/vmap/Makefile
   src/shared/SystemConfig.h
   src/game/Makefile
   src/realmd/Makefile
   src/realmd/realmd.conf
   src/mangosd/Makefile
   src/mangosd/mangosd.conf
   src/bindings/Makefile
   src/bindings/universal/Makefile
])

## Disabled Makefiles, until they are ready for a successful make and
#  make dist run.

## Output files.
AC_OUTPUT
