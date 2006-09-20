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
AM_CXXFLAGS = @COMPILER_OPTIONS@ @EXTRA_COMPILER_OPTIONS@
INCLUDES += -I$(srcdir) -I$(srcdir)/../../../dep/include  -I$(srcdir)/../../shared -I$(srcdir)/../../framework

pkglibdir = $(libdir)

lib_LTLIBRARIES = libMaNGOSScript.la

libMaNGOSScript_la_LIBADD=@LINKER_OPTIONS@ @EXTRA_LINKER_OPTIONS@
libMaNGOSScript_la_LDFLAGS= \
	-release $(LTMS_RELEASE)\
	-version-info $(LTMS_CURRENT):$(LTMS_REVISION):$(LTMS_AGE)

LIBADD=@LINKER_OPTIONS@ @EXTRA_LINKER_OPTIONS@

libMaNGOSScript_la_SOURCES = \
	config.h \
	ScriptMgr.h \
	system.cpp \
	ScriptMgr.cpp \
	script.h \
	Scripts/sc_default.cpp \
	Scripts/sc_defines.cpp \
	Scripts/sc_defines.h

