/*
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _MYSQLDATABASE_H
#define _MYSQLDATABASE_H

#include <string>
#include "../Database.h"

using namespace std;
using namespace ZThread;
class MySQLQueryResult;
class MySQLDatabaseThread;

class MySQLDatabase : public Database
{
    friend class MySQLDatabaseThread;
public:
    MySQLDatabase();
    ~MySQLDatabase();

    bool Initialize(const char* Hostname, unsigned int port,
        const char* Username, const char* Password, const char* DatabaseName,
        uint32 ConnectionCount, uint32 BufferSize);

    void Shutdown();

    QueryResult* Query(const char* QueryString, ...);
    bool WaitExecute(const char* QueryString, ...);
    bool Execute(const char* QueryString, ...);

    void CheckConnections();
    bool Active;
    unsigned long escape_string(char *to, const char *from, unsigned long length);

protected:

    bool Connect();

    bool Connect(uint32 ConnectionIndex);
    bool Disconnect(uint32 ConnectionIndex);

    bool SelectDatabase();
    bool SelectDatabase(uint32 ConnectionIndex);
    bool HandleError(uint32 ConnectionIndex, uint32 ErrorNumber);

    bool SendQuery(uint32 ConnectionIndex, const char* Sql, bool Self = false);

    uint32 GetConnection();

    MYSQL ** Connections;
    char ** QueryBuffer;

    FastMutex DelayedQueryBufferMutex;
    char * DelayedQueryBuffer;

    FastMutex * InUseMarkers;

    int32 mConnectionCount;
    int32 mNormalCount;
    int32 mDelayedID;

    // For reconnecting a broken connection
    string mHostname;
    string mUsername;
    string mPassword;
    string mDatabaseName;
    uint32 mPort;

    uint32 mNextPing;

    FastMutex mSearchMutex;
    MySQLDatabaseThread* mQueryThread;
};

class MySQLQueryResult : public QueryResult
{
public:
    MySQLQueryResult(MYSQL_RES* res, uint32 FieldCount, uint32 RowCount);
    ~MySQLQueryResult();

    bool NextRow();
    void Destroy();

protected:
    MYSQL_RES* mResult;
};

class MySQLDatabaseThread : public Runnable
{
public:
    MySQLDatabaseThread(MySQLDatabase *db);
    ~MySQLDatabaseThread();

    void run();
    void Do();
    void AddQuery(std::string query);

protected:
    LockedQueue<string, FastMutex> _queue;
    MySQLDatabase *_db;
};

#endif
