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
AM_CPPFLAGS = -I$(srcdir) -I$(srcdir)/../../dep/include -I$(srcdir)/../framework $(MYSQL_INCLUDES)
##AM_CPPFLAGS += -I$(srcdir)/../game -I$(srcdir)/../realmd 

## Build MaNGOS shared library and its parts as convenience library.
#  All libraries will be convenience libraries. Might be changed to shared
#  later.
noinst_LIBRARIES = libmangosshared.a libmangosauth.a libmangosconfig.a libmangosdatabase.a libmangosnetwork.a

#  libmangosshared library will later be reused by ...
libmangosshared_a_SOURCES = \
	Base.cpp \
	Base.h \
	ByteBuffer.h \
	Common.h \
	Errors.h \
	EventSystem.cpp \
	EventSystem.h \
	Log.cpp \
	Log.h \
	MemoryLeaks.cpp \
	MemoryLeaks.h \
	Mthread.cpp \
	Mthread.h \
	ProgressBar.cpp \
	ProgressBar.h \
	Singleton.h \
	Timer.h \
	Util.cpp \
	Util.h \
	WorldPacket.h

libmangosauth_a_SOURCES = \
	Auth/AuthCrypt.cpp \
	Auth/AuthCrypt.h \
	Auth/BigNumber.cpp \
	Auth/BigNumber.h \
	Auth/Sha1.cpp \
	Auth/Sha1.h

libmangosconfig_a_SOURCES = \
	Config/Config.cpp \
	Config/ConfigEnv.h \
	Config/Config.h \
	Config/dotconfpp \
	Config/dotconfpp/dotconfpp.cpp \
	Config/dotconfpp/dotconfpp.h \
	Config/dotconfpp/mempool.cpp \
	Config/dotconfpp/mempool.h

libmangosdatabase_a_SOURCES = \
	Database/Database.cpp \
	Database/DatabaseEnv.h \
	Database/Database.h \
	Database/DatabaseMysql.cpp \
	Database/DatabaseMysql.h \
	Database/DatabaseSqlite.cpp \
	Database/DatabaseSqlite.h \
	Database/DataStore.h \
	Database/dbcfile.cpp \
	Database/dbcfile.h \
	Database/DBCStores.cpp \
	Database/DBCStores.cpp \
	Database/DBCStores.h \
	Database/DBCStores.h \
	Database/DBCStructure.h \
	Database/Field.cpp \
	Database/Field.h \
	Database/QueryResult.h \
	Database/QueryResultMysql.cpp \
	Database/QueryResultMysql.h \
	Database/QueryResultSqlite.cpp \
	Database/QueryResultSqlite.h \
	Database/SQLStorage.cpp \
	Database/SQLStorage.h

libmangosnetwork_a_SOURCES = \
	Network/Base64.cpp \
	Network/Base64.h \
	Network/CircularBuffer.cpp \
	Network/CircularBuffer.h \
	Network/ListenSocket.h \
	Network/Parse.cpp \
	Network/Parse.h \
	Network/PoolSocket.cpp \
	Network/PoolSocket.h \
	Network/ResolvServer.cpp \
	Network/ResolvServer.h \
	Network/ResolvSocket.cpp \
	Network/ResolvSocket.h \
	Network/Socket.cpp \
	Network/Socket.h \
	Network/SocketHandler.cpp \
	Network/SocketHandler.h \
	Network/socket_include.cpp \
	Network/socket_include.h \
	Network/SocketThread.cpp \
	Network/SocketThread.h \
	Network/StdLog.h \
	Network/StdoutLog.cpp \
	Network/StdoutLog.h \
	Network/TcpSocket.cpp \
	Network/TcpSocket.h \
	Network/Thread.cpp \
	Network/Thread.h \
	Network/UdpSocket.cpp \
	Network/UdpSocket.h \
	Network/Utility.cpp \
	Network/Utility.h

## Additional files to include when running 'make dist'
#  Auth sources and headers.
EXTRA_DIST = \
	Auth/AuthCrypt.cpp \
	Auth/AuthCrypt.h \
	Auth/BigNumber.cpp \
	Auth/BigNumber.h \
	Auth/Sha1.cpp \
	Auth/Sha1.h

#  Configuration sources and headers.
EXTRA_DIST += \
	Config/Config.cpp \
	Config/ConfigEnv.h \
	Config/Config.h \
	Config/dotconfpp \
	Config/dotconfpp/dotconfpp.cpp \
	Config/dotconfpp/dotconfpp.h \
	Config/dotconfpp/mempool.cpp \
	Config/dotconfpp/mempool.h

#  Database sources and headers.
EXTRA_DIST += \
	Database/Database.cpp \
	Database/DatabaseEnv.h \
	Database/Database.h \
	Database/DatabaseMysql.cpp \
	Database/DatabaseMysql.h \
	Database/DatabaseSqlite.cpp \
	Database/DatabaseSqlite.h \
	Database/DataStore.h \
	Database/dbcfile.cpp \
	Database/dbcfile.h \
	Database/DBCStores.cpp \
	Database/DBCStores.cpp \
	Database/DBCStores.h \
	Database/DBCStores.h \
	Database/DBCStructure.h \
	Database/Field.cpp \
	Database/Field.h \
	Database/QueryResult.h \
	Database/QueryResultMysql.cpp \
	Database/QueryResultMysql.h \
	Database/QueryResultSqlite.cpp \
	Database/QueryResultSqlite.h \
	Database/SQLStorage.cpp \
	Database/SQLStorage.h

#  Network sources and headers.
EXTRA_DIST += \
	Network/Base64.cpp \
	Network/Base64.h \
	Network/CircularBuffer.cpp \
	Network/CircularBuffer.h \
	Network/ListenSocket.h \
	Network/Parse.cpp \
	Network/Parse.h \
	Network/PoolSocket.cpp \
	Network/PoolSocket.h \
	Network/ResolvServer.cpp \
	Network/ResolvServer.h \
	Network/ResolvSocket.cpp \
	Network/ResolvSocket.h \
	Network/Socket.cpp \
	Network/Socket.h \
	Network/SocketHandler.cpp \
	Network/SocketHandler.h \
	Network/socket_include.cpp \
	Network/socket_include.h \
	Network/SocketThread.cpp \
	Network/SocketThread.h \
	Network/StdLog.h \
	Network/StdoutLog.cpp \
	Network/StdoutLog.h \
	Network/TcpSocket.cpp \
	Network/TcpSocket.h \
	Network/Thread.cpp \
	Network/Thread.h \
	Network/UdpSocket.cpp \
	Network/UdpSocket.h \
	Network/Utility.cpp \
	Network/Utility.h
