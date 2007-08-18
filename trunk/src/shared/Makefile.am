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
AM_CPPFLAGS = $(MYSQL_INCLUDES) $(POSTGRE_INCLUDES) -I$(top_builddir)/src/shared -I$(srcdir) -I$(srcdir)/../../dep/include -I$(srcdir)/../framework -I$(srcdir)/../shared -I$(srcdir)/../../dep/include/g3dlite
## AM_CPPFLAGS += -I$(srcdir)/../game -I$(srcdir)/../realmd

## Build MaNGOS shared library and its parts as convenience library.
#  All libraries will be convenience libraries. Might be changed to shared
#  later.
noinst_LIBRARIES = libmangosshared.a libmangosauth.a libmangosconfig.a libmangosdatabase.a libmangosnetwork.a libmangosvmaps.a

#  libmangosshared library will later be reused by ...
libmangosshared_a_SOURCES = \
	Base.cpp \
	Base.h \
	ByteBuffer.h \
	Common.h \
	Errors.h \
	Log.cpp \
	Log.h \
	MemoryLeaks.cpp \
	MemoryLeaks.h \
	Mthread.cpp \
	Mthread.h \
	ProgressBar.cpp \
	ProgressBar.h \
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
	Auth/Sha1.h \
	Auth/md5.c \
	Auth/md5.h

libmangosconfig_a_SOURCES = \
	Config/dotconfpp/dotconfpp.cpp \
	Config/dotconfpp/dotconfpp.h \
	Config/dotconfpp/mempool.cpp \
	Config/dotconfpp/mempool.h \
	Config/Config.cpp \
	Config/Config.h \
	Config/ConfigEnv.h

libmangosdatabase_a_SOURCES = \
	Database/DBCStores.cpp \
	Database/DBCStores.h \
	Database/DBCStructure.h \
	Database/DBCfmt.cpp \
	Database/Database.cpp \
	Database/Database.h \
	Database/DatabaseEnv.h \
	Database/DatabaseImpl.h \
	Database/DatabaseMysql.cpp \
	Database/DatabasePostgre.cpp \
	Database/DatabaseMysql.h \
	Database/DatabasePostgre.h \
	Database/DatabaseSqlite.cpp \
	Database/DatabaseSqlite.h \
	Database/Field.cpp \
	Database/Field.h \
	Database/MySQLDelayThread.h \
	Database/QueryResult.h \
	Database/QueryResultMysql.cpp \
	Database/QueryResultMysql.h \
	Database/QueryResultPostgre.cpp \
	Database/QueryResultPostgre.h \
	Database/QueryResultSqlite.cpp \
	Database/QueryResultSqlite.h \
	Database/SQLStorage.cpp \
	Database/SQLStorage.h \
	Database/SqlDelayThread.cpp \
	Database/SqlDelayThread.h \
	Database/SqlOperations.cpp \
	Database/SqlOperations.h \
	Database/dbcfile.cpp \
	Database/dbcfile.h

libmangosnetwork_a_SOURCES = \
	Network/Base64.cpp \
	Network/Base64.h \
	Network/CircularBuffer.cpp \
	Network/CircularBuffer.h \
	Network/IFile.h \
	Network/ISocketHandler.cpp \
	Network/ISocketHandler.h \
	Network/Ipv4Address.cpp \
	Network/Ipv4Address.h \
	Network/Ipv6Address.cpp \
	Network/Ipv6Address.h \
	Network/ListenSocket.h \
	Network/Mutex.cpp \
	Network/Mutex.h \
	Network/Parse.cpp \
	Network/Parse.h \
	Network/RandomNumber.cpp \
	Network/RandomNumber.h \
	Network/ResolvServer.cpp \
	Network/ResolvServer.h \
	Network/ResolvSocket.cpp \
	Network/ResolvSocket.h \
	Network/SctpSocket.h \
	Network/Socket.cpp \
	Network/Socket.h \
	Network/SocketAddress.h \
	Network/SocketHandler.cpp \
	Network/SocketHandler.h \
	Network/StdLog.h \
	Network/StdoutLog.cpp \
	Network/StdoutLog.h \
	Network/TcpSocket.cpp \
	Network/TcpSocket.h \
	Network/Thread.cpp \
	Network/Thread.h \
	Network/UdpSocket.cpp \
	Network/UdpSocket.h \
	Network/Uid.h \
	Network/Utility.cpp \
	Network/Utility.h \
	Network/socket_include.cpp \
	Network/socket_include.h \
	Network/sockets-config.h

libmangosvmaps_a_SOURCES = \
	vmap/AABSPTree.h \
	vmap/BaseModel.cpp \
	vmap/BaseModel.h \
	vmap/CoordModelMapping.cpp \
	vmap/CoordModelMapping.h \
	vmap/DebugCmdLogger.cpp \
	vmap/DebugCmdLogger.h \
	vmap/IVMapManager.h \
	vmap/ManagedModelContainer.cpp \
	vmap/ManagedModelContainer.h \
	vmap/ModelContainer.cpp \
	vmap/ModelContainer.h \
	vmap/NodeValueAccess.h \
	vmap/RayIntersectionIterator.h \
	vmap/ShortBox.h \
	vmap/ShortVector.h \
	vmap/SubModel.cpp \
	vmap/SubModel.h \
	vmap/TileAssembler.cpp \
	vmap/TileAssembler.h \
	vmap/TreeNode.cpp \
	vmap/TreeNode.h \
	vmap/VMapDefinitions.h \
	vmap/VMapFactory.cpp \
	vmap/VMapFactory.h \
	vmap/VMapManager.cpp \
	vmap/VMapManager.h

## Additional files to include when running 'make dist'
#  Disabled packet logger
EXTRA_DIST = \
	PacketLog.cpp \
	PacketLog.h

# System configuration
EXTRA_DIST += \
	SystemConfig.h

#  VC++ project workspace for dotconfpp
EXTRA_DIST += \
	Config/ConfigLibrary.vcproj
