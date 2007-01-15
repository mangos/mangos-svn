# Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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
        [0.6-SVN],
        [http://www.mangosproject.org/],
        [mangos])
AM_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR(src/mangosd/Main.cpp)

dnl Automake is needed for the build system.
dnl
AM_INIT_AUTOMAKE([1.6 dist-bzip2])
AM_MAINTAINER_MODE

AC_PROG_CC(gcc cc)
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_LANG([C++])
AC_EXEEXT
AC_OBJEXT

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

dnl Set up Scripting build.
LIBMANGOSSCRIPT_MAJOR_VERSION=0
LIBMANGOSSCRIPT_MINOR_VERSION=2
LIBMANGOSSCRIPT_MICRO_VERSION=0
LTMS_RELEASE=$LIBMANGOSSCRIPT_MAJOR_VERSION.$LIBMANGOSSCRIPT_MINOR_VERSION.$LIBMANGOSSCRIPT_MICRO_VERSION
LTMS_CURRENT=$LIBMANGOSSCRIPT_MAJOR_VERSION
LTMS_REVISION=$LIBMANGOSSCRIPT_MINOR_VERSION
LTMS_AGE=$LIBMANGOSSCRIPT_MICRO_VERSION

COMPILER_OPTIONS=""
LINKER_OPTIONS=""

# check for openssl path given
AC_ARG_WITH(openssl,
[  --with-openssl=/path/to/openssl/dir    OpenSSL directory],
[
  AC_CHECK_FILE($withval/include/openssl/ssl.h,
  [
    OPENSSL_INCLUDES="-I$withval/include"
    OPENSSL_LDFLAGS="-L$withval/lib"],
  [AC_MSG_ERROR([Directory '$withval' does not exist, or does not contain OpenSSL])]) ],
 [
  OPENSSL_INCLUDES=""
  OPENSSL_LDFLAGS="" ])

AC_SUBST(OPENSSL_INCLUDES)
AC_SUBST(OPENSSL_LDFLAGS)

# check for mysql path given
AC_ARG_WITH(mysql,
[  --with-mysql=/path/to/mysql/dir        MySQL directory]],
[
  AC_CHECK_FILE($withval/include/mysql.h,
    [MYSQL_INCLUDES="-I$withval/include"
    MYSQL_LDFLAGS="-L$withval/lib"],
    [AC_MSG_ERROR([Directory '$withval' does not exist, or does not contain MySQL])]) ],
[
  MYSQL_INCLUDES="-I/usr/include/mysql -I/usr/local/include/mysql"
  MYSQL_LDFLAGS="-L/usr/lib/mysql -L/usr/local/lib/mysql"])

AC_SUBST(MYSQL_INCLUDES)
AC_SUBST(MYSQL_LDFLAGS)

INCLUDES="$MYSQL_INCLUDES $OPENSSL_INCLUDES"
LDFLAGS="$MYSQL_LDFLAGS $OPENSSL_LDFLAGS"
AC_SUBST(INCLUDES)
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
AC_CHECK_LIB( mysqlclient_r, mysql_init, [],[AC_CHECK_LIB(mysql, mysql_init,[],[AC_MSG_ERROR([Missing mysql])])])
AC_CHECK_LIB( ssl, SHA1_Init, [], [AC_CHECK_LIB(ssl, SHA1_Init,[],[AC_MSG_ERROR([Missing openssl])])])

CPPFLAGS_SAVE=$CPPFLAGS
CPPFLAGS=$INCLUDES $CPPFLAGS

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_CHECK_HEADERS(                                             \
   limits.h sys/ioctl.h unistd.h fcntl.h float.h mysql.h      \
   netdb.h netinet/in.h stddef.h sys/socket.h                 \
   sys/time.h sys/timeb.h openssl/opensslv.h                          \
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

AC_PATH_PROG(__DOXYGEN, doxygen, no, $PATH)
dnl
dnl Auto-detect whether doxygen generated API docs should be included.
dnl
withval=auto
AC_ARG_WITH(apidocs, [  --with-apidocs          build MaNGOS API docs ])

if test $withval = auto -a $__DOXYGEN != no ; then
    withval=yes
elif test $withval = yes -a $__DOXYGEN = no ; then
    AC_MSG_ERROR(--> MaNGOS API docs needs doxygen in PATH)
fi
    
if test $withval = yes; then
    WITH_APIDOCS_TARGET=apidocs
    WITH_APIDOCS=1
else
    WITH_APIDOCS=0
fi
AC_SUBST(WITH_APIDOCS_TARGET)
AC_SUBST(WITH_APIDOCS)
      
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


# check for cli console enable
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

# check for remote console enable
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


MANGOSD_CONFIG_TEMP="$sysconfdir/mangosd.conf"
MANGOSD_CONFIGDIR_TEMP="$sysconfdir"
REALMD_CONFIG_TEMP="$sysconfdir/realmd.conf"
REALMD_CONFIGDIR_TEMP="$sysconfdir"
if test $sysconfdir = '${prefix}/etc'; then
    MANGOSD_CONFIG_TEMP="${prefix}/mangos/mangosd.conf"
    MANGOSD_CONFIGDIR_TEMP="${prefix}/mangos"
    REALMD_CONFIG_TEMP="${prefix}/mangos/realmd.conf"
    REALMD_CONFIGDIR_TEMP="${prefix}/mangos"
else
  if test $sysconfdir = '/'; then
	MANGOSD_CONFIG_TEMP="/mangosd.conf"
	MANGOSD_CONFIGDIR_TEMP="/"
	REALMD_CONFIG_TEMP="/realmd.conf"
	REALMD_CONFIGDIR_TEMP="/"
  else
	MANGOSD_CONFIG_TEMP="$sysconfdir/mangosd.conf"
	MANGOSD_CONFIGDIR_TEMP="$sysconfdir"
	REALMD_CONFIG_TEMP="$sysconfdir/realmd.conf"
	REALMD_CONFIGDIR_TEMP="$sysconfdir"
  fi
fi

if test $datadir = '${prefix}/share'; then
    MANGOSD_DATA_TEMP="${prefix}/share/mangos"
else
   if test $datadir = '/'; then
   	MANGOSD_DATA_TEMP="/mangos"
   else
	MANGOSD_DATA_TEMP="$datadir/mangos"
   fi
fi

eval "MANGOSD_CONFIG=$MANGOSD_CONFIG_TEMP"
eval "MANGOSD_CONFIGDIR=$MANGOSD_CONFIGDIR_TEMP"
eval "REALMD_CONFIG=$REALMD_CONFIG_TEMP"
eval "REALMD_CONFIGDIR=$REALMD_CONFIGDIR_TEMP"
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
AC_SUBST(REALMD_CONFIG)
AC_SUBST(REALMD_CONFIGDIR)
AC_SUBST(MANGOSD_DATA)
AC_SUBST(MANGOSD_ENABLE_CLI)
AC_SUBST(MANGOSD_ENABLE_RA)
AC_SUBST(VERSION)
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_AGE)
AC_SUBST(LT_REVISION)
AC_SUBST(LTMS_RELEASE)
AC_SUBST(LTMS_CURRENT)
AC_SUBST(LTMS_AGE)
AC_SUBST(LTMS_REVISION)

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
   src/bindings/python/Makefile
   src/bindings/universal/Makefile
   src/mangosd/Makefile
   src/mangosd/mangosd.conf
   dep/Makefile
   dep/src/Makefile
   dep/src/zlib/Makefile
   dep/src/zthread/Makefile
])

AC_OUTPUT
