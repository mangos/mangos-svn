# Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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

INCLUDES += -I$(srcdir) -I$(srcdir)/../../dep/include
INCLUDES += -I$(top_builddir)/src/shared -I$(srcdir)/../shared
INCLUDES += -I$(srcdir)/../realmd -I$(srcdir)/../framework

bin_PROGRAMS = realmd

realmd_SOURCES = \
    AuthSocket.cpp \
    AuthSocket.h \
    Main.cpp \
    RealmList.cpp \
    RealmList.h

realmd_LDADD = $(top_builddir)/src/shared/libdatabase.a $(top_builddir)/src/shared/libconfig.a $(top_builddir)/src/shared/libauth.a $(top_builddir)/src/shared/libshared.a $(top_builddir)/src/shared/libnetwork.a $(top_builddir)/src/framework/libframework.a -lZThread

realmd_LDFLAGS = -L$(top_builddir)/dep/src/zthread

# use our configured sysconfdir
sysconfdir = @REALMD_CONFIGDIR@

# install realmd.conf to $(sysconfdir)
sysconf_DATA = \
    realmd.conf

