# Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

## Process this file with automake to produce Makefile.in

## CPP flags for includes, defines, etc.
AM_CPPFLAGS = $(MYSQL_INCLUDES) $(POSTGRE_INCLUDES) -I$(top_builddir)/src/shared -I$(srcdir)/../../dep/include -I$(srcdir)/../framework -I$(srcdir)/../shared -I$(srcdir)

## Build realm list daemon as standalone program
bin_PROGRAMS = mangos-realmd
mangos_realmd_SOURCES = \
	AuthCodes.h \
	AuthSocket.cpp \
	AuthSocket.h \
	Main.cpp \
	RealmList.cpp \
	RealmList.h

## Link realm list daemon against the shared library
mangos_realmd_LDADD = ../shared/Database/libmangosdatabase.a ../shared/Config/libmangosconfig.a ../shared/Auth/libmangosauth.a ../shared/libmangosshared.a ../framework/libmangosframework.a ../../dep/src/sockets/libmangossockets.a ../../dep/src/zthread/libZThread.la
mangos_realmd_LDFLAGS = -L../../dep/src/sockets -L../../dep/src/zthread $(MYSQL_LIBS) $(POSTGRE_LIBS) -L$(libdir) $(ZLIB) $(COMPATLIB) $(SSLLIB)

## Additional files to include when running 'make dist'
#  Include realm list daemon configuration
EXTRA_DIST = \
	realmd.conf

## Additional files to install
sysconf_DATA = \
	realmd.conf
