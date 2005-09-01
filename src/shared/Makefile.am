# Copyright (C) 2004 Wow Daemon
# Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

## we don't want to need this here
INCLUDES += -I$(srcdir)/../game -I$(srcdir)/../realmlist

noinst_LIBRARIES = libshared.a libauth.a libconfig.a libdatabase.a libnetwork.a

libshared_a_SOURCES = \
    Log.cpp \
    MemoryLeaks.cpp \
    ByteBuffer.h \
    Common.h \
    Errors.h \
    Log.h \
    MemoryLeaks.h \
    Singleton.h \
    Timer.h \
    Util.h \
    Util.cpp \
    WorldPacket.h


libauth_a_SOURCES = \
    Auth/BigNumber.cpp \
    Auth/BigNumber.h \
    Auth/Sha1.cpp \
    Auth/Sha1.h \
    Auth/WowCrypt.cpp \
    Auth/WowCrypt.h


libconfig_a_SOURCES = \
    Config/Config.cpp \
    Config/Config.h \
    Config/ConfigEnv.h \
    Config/dotconfpp/dotconfpp.cpp \
    Config/dotconfpp/dotconfpp.h \
    Config/dotconfpp/mempool.cpp \
    Config/dotconfpp/mempool.h \
    Config/dotconfpp

libdatabase_a_SOURCES = \
    Database/DBC.cpp \
    Database/Database.cpp \
    Database/DatabaseSqlite.cpp \
    Database/QueryResultMysql.cpp \
    Database/DBC.h \
    Database/Database.h \
    Database/DatabaseSqlite.h \
    Database/QueryResultMysql.h \
    Database/DBCStores.cpp \
    Database/DatabaseEnv.h \
    Database/Field.cpp \
    Database/QueryResultSqlite.cpp \
    Database/DBCStores.h \
    Database/DatabaseMysql.cpp \
    Database/Field.h \
    Database/QueryResultSqlite.h \
    Database/DataStore.h \
    Database/DatabaseMysql.h \
    Database/QueryResult.h 

libnetwork_a_SOURCES = \
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
