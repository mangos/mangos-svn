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

## CPP flags for includes, defines, etc.
AM_CPPFLAGS = $(MYSQL_INCLUDES) $(POSTGRE_INCLUDES) -I$(top_builddir)/src/shared -I$(srcdir) -I$(srcdir)/../../../dep/include -I$(srcdir)/../../framework -I$(srcdir)/../shared -I$(srcdir)/../../../dep/include/g3dlite

## Build MaNGOS shared library and its parts as convenience library.
#  All libraries will be convenience libraries. Might be changed to shared
#  later.
noinst_LIBRARIES = libmangosnetwork.a

libmangosnetwork_a_SOURCES = \
	Base64.cpp \
	Base64.h \
	CircularBuffer.cpp \
	CircularBuffer.h \
	IFile.h \
	ISocketHandler.cpp \
	ISocketHandler.h \
	Ipv4Address.cpp \
	Ipv4Address.h \
	Ipv6Address.cpp \
	Ipv6Address.h \
	ListenSocket.h \
	Mutex.cpp \
	Mutex.h \
	Parse.cpp \
	Parse.h \
	RandomNumber.cpp \
	RandomNumber.h \
	ResolvServer.cpp \
	ResolvServer.h \
	ResolvSocket.cpp \
	ResolvSocket.h \
	SctpSocket.h \
	Socket.cpp \
	Socket.h \
	SocketAddress.h \
	SocketHandler.cpp \
	SocketHandler.h \
	StdLog.h \
	StdoutLog.cpp \
	StdoutLog.h \
	TcpSocket.cpp \
	TcpSocket.h \
	Thread.cpp \
	Thread.h \
	UdpSocket.cpp \
	UdpSocket.h \
	Uid.h \
	Utility.cpp \
	Utility.h \
	socket_include.cpp \
	socket_include.h \
	sockets-config.h
