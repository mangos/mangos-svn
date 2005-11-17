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

AC_PREREQ(2.59)
AC_INIT([MaNGOS],
        [0.0.2],
        [https://opensvn.csie.org/traccgi/MaNGOS/],
        [mangos])
AM_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR(src/mangosd/Main.cpp)

dnl Automake is needed for the build system.
dnl
AM_INIT_AUTOMAKE([1.6 dist-bzip2])
AM_MAINTAINER_MODE

AC_PROG_CC(gcc cc)
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_LANG([C++])

dnl Checks for programs.
AM_PROG_LIBTOOL

dnl
dnl use defaults if cross-compiling, otherwise use the default path.
dnl
if test "$cross_compiling" = "yes"; then
    MYPATH=":"
else
#    MYPATH="/bin:/usr/bin:/usr/local/bin:$PATH:/sbin:/usr/sbin:/usr/local/sbin:/opt/gnu/bin"
    MYPATH=$PATH
fi

dnl
dnl Find some common programs
dnl
AC_PATH_PROG(__PYTHON, python, /usr/bin/python, $MYPATH)

dnl Set up ZThread build.
LIBZTHREAD_MAJOR_VERSION=2
LIBZTHREAD_MINOR_VERSION=3
LIBZTHREAD_MICRO_VERSION=2
LT_RELEASE=$LIBZTHREAD_MAJOR_VERSION.$LIBZTHREAD_MINOR_VERSION.$LIBZTHREAD_MICRO_VERSION
LT_CURRENT=$LIBZTHREAD_MAJOR_VERSION
LT_REVISION=$LIBZTHREAD_MINOR_VERSION
LT_AGE=$LIBZTHREAD_MICRO_VERSION

COMPILER_OPTIONS=""
LINKER_OPTIONS=""

INCLUDES="-I/usr/include/mysql -I/usr/local/include/mysql -I/usr/include/openssl -I/usr/local/include/openssl"
AC_SUBST(INCLUDES)
LDFLAGS="-L/usr/lib/mysql -L/usr/local/lib/mysql"
AC_SUBST(LDFLAGS)

dnl Checks for libraries.

dnl Checks for library functions.
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

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_CHECK_HEADERS(                                             \
   limits.h sys/ioctl.h unistd.h fcntl.h float.h mysql.h      \
   malloc.h netdb.h netinet/in.h stddef.h sys/socket.h        \
   sys/time.h sys/timeb.h opensslv.h                          \
 ,[],[AC_MSG_ERROR([Missing required header])])

CPPFLAGS=$CPPFLAGS_SAVE

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_TYPE_SIZE_T
AC_TYPE_OFF_T
AC_TYPE_SIGNAL
AC_STRUCT_TM
AC_CHECK_TYPES([ ptrdiff_t ])

dnl Checks for library functions.
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

dnl
dnl Auto-detect which python bindings should be built.
dnl
withval=auto
AC_ARG_WITH(python, [  --with-python           build python bindings ])

WITH_PYTHON_VERSION=$withval
if test $withval = auto ; then

AC_MSG_CHECKING(for python 2.4)
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <python2.4/Python.h>
main() {
  exit(strncmp("2.4", PY_VERSION, 3));
} ]])],[withval=yes],[withval=no],[withval=yes])
  AC_MSG_RESULT($withval)
  if test $withval = yes ; then
    WITH_PYTHON_VERSION="2.4"
  else
    
  AC_MSG_CHECKING(for python 2.3)
  AC_RUN_IFELSE([AC_LANG_SOURCE([[
  #include <python2.3/Python.h>
  main() {
    exit(strncmp("2.3", PY_VERSION, 3));
  } ]])],[withval=yes],[withval=no],[withval=yes])
    AC_MSG_RESULT($withval)
    if test $withval = yes ; then
      WITH_PYTHON_VERSION="2.3"
    else
    
      AC_MSG_CHECKING(for python 2.2)
      AC_RUN_IFELSE([AC_LANG_SOURCE([[
    #include <python2.2/Python.h>
    main() {
      exit(strncmp("2.2", PY_VERSION, 3));
    } ]])],[withval=yes],[withval=no],[withval=yes])
      AC_MSG_RESULT($withval)
      if test $withval = yes ; then
        WITH_PYTHON_VERSION="2.2"
      else
    
        AC_MSG_CHECKING(for python 1.5.2)
        AC_RUN_IFELSE([AC_LANG_SOURCE([[
    #include <python1.5/Python.h>
    main() {
      exit(strcmp("1.5.2", PY_VERSION));
    } ]])],[withval=yes],[withval=no],[withval=yes])
        AC_MSG_RESULT($withval)
        if test $withval = yes ; then
          WITH_PYTHON_VERSION="1.5"
        fi
      fi
    fi
  fi
fi

if test "$WITH_PYTHON_VERSION" != no ; then
  WITH_PYTHON_SUBDIR=python
  WITH_PYTHON_SUBPACKAGE=1
else
  WITH_PYTHON_SUBDIR=
  WITH_PYTHON_SUBPACKAGE=0
fi

AC_SUBST(WITH_PYTHON_SUBDIR)
AC_SUBST(WITH_PYTHON_SUBPACKAGE)
AC_SUBST(WITH_PYTHON_VERSION)

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

# check for cli system enable
AC_MSG_CHECKING(whether cli system is enabled)
MANGOSD_ENABLE_CLI=no
AC_ARG_ENABLE(enable-cli,
[  --enable-cli Turn on command console],
[
    if test "$enableval" = "yes" ; then
	CFLAGS="-DENABLE_CLI_ $CFLAGS"
	CXXFLAGS="-DENABLE_CLI_ $CXXFLAGS"
	MANGOSD_ENABLE_CLI=yes
    elif test "$withval" != "no" ; then
	AC_MSG_ERROR(Please choose yes or no)
    fi
])
AC_MSG_RESULT($MANGOSD_ENABLE_CLI)

# check for httpd system enable
AC_MSG_CHECKING(whether httpd system is enabled)
MANGOSD_ENABLE_HTTPD=no
AC_ARG_ENABLE(httpd-integrated,
[  --enable-httpd-integrated Turn on httpd integrated system],
[
    if test "$enableval" = "yes" ; then
	CFLAGS="-DENABLE_HTTPD_SYSTEM $CFLAGS"
	CXXFLAGS="-DENABLE_HTTPD_SYSTEM $CXXFLAGS"
	MANGOSD_ENABLE_HTTPD=yes
    elif test "$withval" != "no" ; then
	AC_MSG_ERROR(Please choose yes or no)
    fi
])
AC_MSG_RESULT($MANGOSD_ENABLE_HTTPD)


# check for 1.8.x protocol support
AC_MSG_CHECKING(whether 1.8.x support is enabled)
MANGOSD_ENABLE_18X=yes
AC_ARG_ENABLE(18x,
[  --enable-18x Turn on 1.8.x support],
[
    if test "$enableval" = "no" ; then
	MANGOSD_ENABLE_18X=no
    elif test "$withval" != "yes" ; then
	AC_MSG_ERROR(Please choose yes or no)
    fi
])
AC_MSG_RESULT($MANGOSD_ENABLE_18X)

if test "$MANGOSD_ENABLE_18X" != "no"; then
    CFLAGS="-D_VERSION_1_8_0_ $CFLAGS"
    CXXFLAGS="-D_VERSION_1_8_0_ $CXXFLAGS"
    MANGOSD_ENABLE_17X=no
else
    MANGOSD_ENABLE_17X=yes  
fi

# check for 1.7.x protocol support
AC_MSG_CHECKING(whether 1.7.x support is enabled)
AC_ARG_ENABLE(17x,
[  --enable-17x Turn on 1.7.x support],
[
    if test "$enableval" = "no" ; then
	MANGOSD_ENABLE_17X=no
    elif test "$withval" != "yes" ; then
	AC_MSG_ERROR(Please choose yes or no)
    fi
])
AC_MSG_RESULT($MANGOSD_ENABLE_17X)

#if 1.8 is enable, we don't allow 17x to be enable
if test "$MANGOSD_ENABLE_17X" != "no"; then
    if test "$MANGOSD_ENABLE_18X" != "no"; then
        MANGOSD_ENABLE_17X=no
    else
        CFLAGS="-D_VERSION_1_7_0_ $CFLAGS"
        CXXFLAGS="-D_VERSION_1_7_0_ $CXXFLAGS"
    fi
fi


# check for enable new framework
AC_MSG_CHECKING(whether new framework is enabled)
MANGOSD_ENABLE_NEW_FRAMEWORK=no
AC_ARG_ENABLE(new-framework,
[  --enable-new-framework Enable new framework],
[
    if test "$enableval" = "yes" ; then
	MANGOSD_ENABLE_NEW_FRAMEWORK=yes
    elif test "$withval" != "no" ; then
	AC_MSG_ERROR(Please choose yes or no)          
    fi
])
AC_MSG_RESULT($MANGOSD_ENABLE_NEW_FRAMEWORK)
AM_CONDITIONAL(NEWFRAMEWORK, test x$MANGOS_ENABLE_NEW_FRAMEWORK = x$yes)


MANGOSD_CONFIG_TEMP="$sysconfdir/mangosd.conf"
MANGOSD_CONFIGDIR_TEMP="$sysconfdir"
if test $sysconfdir = '${prefix}/etc'; then
    MANGOSD_CONFIG_TEMP="/etc/mangos/mangosd.conf"
    MANGOSD_CONFIGDIR_TEMP="/etc/mangos"
else
  if test $sysconfdir = '/'; then
	MANGOSD_CONFIG_TEMP="/mangosd.conf"
	MANGOSD_CONFIGDIR_TEMP="/"
  else
	MANGOSD_CONFIG_TEMP="$sysconfdir/mangosd.conf"
	MANGOSD_CONFIGDIR_TEMP="$sysconfdir"
  fi
fi

if test $datadir = '${prefix}/share'; then
    MANGOSD_DATA_TEMP="$prefix/share/mangos"
else
   if test $datadir = '/'; then
   	MANGOSD_DATA_TEMP="/mangos"
   else
	MANGOSD_DATA_TEMP="$datadir/mangos"
   fi
fi

eval "MANGOSD_CONFIG=$MANGOSD_CONFIG_TEMP"
eval "MANGOSD_CONFIGDIR=$MANGOSD_CONFIGDIR_TEMP"
eval "MANGOSD_DATA=$MANGOSD_DATA_TEMP"

dnl Configure the final compiler & linker options
COMPILER_OPTIONS="$COMPILER_OPTIONS $CXXFLAGS"
LINKER_OPTIONS="$LINKER_OPTIONS $LDFLAGS"

dnl Configured flags for compiling
AC_SUBST(LINKER_OPTIONS)
AC_SUBST(COMPILER_OPTIONS)

dnl Extra variables users can customize
AC_SUBST(EXTRA_LINKER_OPTIONS)
AC_SUBST(EXTRA_COMPILER_OPTIONS)

dnl Configured MaNGOS variables
AC_SUBST(MANGOSD_CONFIG)
AC_SUBST(MANGOSD_CONFIGDIR)
AC_SUBST(MANGOSD_DATA)
AC_SUBST(VERSION)
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_AGE)
AC_SUBST(LT_REVISION)

AC_CONFIG_FILES([
   ./Makefile
   doc/Makefile
   sql/Makefile
   src/Makefile
   src/shared/Makefile
   src/realmlist/Makefile
   src/mangosd/Master.h
   src/mangosd/Makefile
   src/framework/Makefile
   src/game/Makefile
   src/bindings/Makefile
   src/bindings/python/Makefile
   dep/Makefile
   dep/src/Makefile
   dep/src/zlib/Makefile
   dep/src/zthread/Makefile
])

AC_OUTPUT
