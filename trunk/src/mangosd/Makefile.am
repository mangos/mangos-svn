# Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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
AM_CPPFLAGS = $(MYSQL_INCLUDES) -I$(top_builddir)/src/shared -I$(srcdir)/../../dep/include -I$(srcdir)/../framework -I$(srcdir)/../shared  -I$(srcdir)/../game -I$(srcdir)

## Build world list daemon as standalone program
bin_PROGRAMS = mangos-worldd
mangos_worldd_SOURCES = \
	CliRunnable.cpp \
	CliRunnable.h \
	Main.cpp \
	Master.cpp \
	Master.h \
	RASocket.cpp \
	RASocket.h \
	WorldRunnable.cpp \
	WorldRunnable.h

## Link world daemon against the shared library
mangos_worldd_LDADD = ../bindings/universal/libmangosscript.la ../game/libmangosgame.a ../shared/libmangosdatabase.a ../shared/libmangosconfig.a ../shared/libmangosauth.a ../shared/libmangosshared.a ../shared/libmangosvmaps.a ../shared/libmangosnetwork.a ../framework/libmangosframework.a ../../dep/src/zthread/libZThread.la ../../dep/src/g3dlite/libg3dlite.a
mangos_worldd_LDFLAGS = -L../../dep/src/zthread  -L../../dep/src/g3dlite -L../bindings/universal/ $(MYSQL_LIBS) -L$(libdir) $(ZLIB) $(COMPATLIB) $(SSLLIB) -export-dynamic

## Additional files to include when running 'make dist'
#  Include world daemon configuration
EXTRA_DIST = \
	mangosd.conf

## Additional files to install
sysconf_DATA = \
	mangosd.conf
