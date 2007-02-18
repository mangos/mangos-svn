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

## Sub-directories to parse
SUBDIRS = doc dep src sql

## Additional files to include when running 'make dist'
#  Add guidelines for working with the sources.
EXTRA_DIST = HACKING

#  Add installation instructions, and a special ReadMe for Windows users,
#  these are convenience files, using DOS linefeeds.
EXTRA_DIST = HACKING.Win32.txt INSTALL.Win32.txt README.Win32.txt

# Win32 project workspace for Visual C++ 2003 Toolkit and Visual C++ 2003
EXTRA_DIST += \
	win32/mangosdVC71.sln \
	win32/VC71/framework.vcproj \
	win32/VC71/game.vcproj \
	win32/VC71/mangosd.vcproj \
	win32/VC71/realmd.vcproj \
	win32/VC71/shared.vcproj

# Win32 project workspace for Visual Studio .NET 2005
EXTRA_DIST += \
	win32/mangosdVC80.sln \
	win32/VC80/framework.vcproj \
	win32/VC80/game.vcproj \
	win32/VC80/mangosd.vcproj \
	win32/VC80/realmd.vcproj \
	win32/VC80/shared.vcproj
