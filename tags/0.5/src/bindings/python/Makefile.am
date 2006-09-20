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

# Note: we yet do not care for multiple architectures, will have to change
# pylibdir to pylibdir = ${prefix}/lib@MARK64@/python@WITH_PYTHON_VERSION@ when
# we get x86_64 support done.
pylibdir = ${prefix}/lib/python@WITH_PYTHON_VERSION@
pyincdir = $(prefix)/include/python@WITH_PYTHON_VERSION@

INCLUDES += -I$(srcdir) -I$(srcdir)/../../../dep/include -I$(srcdir)/../../shared -I$(srcdir)/../../game -I$(srcdir)/../../framework -I$(pyincdir)

#noinst_LIBRARIES = libmangospython.a

#libmangospython_a_SOURCES = \
#	AddSourcesHere.cpp

#libmangospython_a_LDFLAGS = -L(pylibdir)

#if NEWFRAMEWORK
#libmangospython_a_LIBADD = \
#	AddNewFrameworkRelatedSourcesHere.cpp
#endif
