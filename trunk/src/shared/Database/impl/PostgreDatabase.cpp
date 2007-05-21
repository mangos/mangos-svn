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

#include "../DatabaseEnv.h"

#ifdef DATABASE_SUPPORT_PGSQL

PostgreDatabase::PostgreDatabase() : Database(DATABASE_TYPE_PGSQL)
{
    Connections = NULL;
    InUseMarkers = NULL;
    QueryBuffer = NULL;
    mConnectionCount = -1;   // Not connected.
    mNextPing = getMSTime();
    mQueryThread = NULL;
}

PostgreDatabase::~PostgreDatabase()
{
    // Close connections..

    for(int32 i = 0; i < mConnectionCount; ++i)
    {
        if(Connections)
            if(Connections[i])
                Disconnect(i);

        delete [] QueryBuffer[i];
    }

    delete [] Connections;
    delete [] InUseMarkers;
    delete [] QueryBuffer;
    delete [] DelayedQueryBuffer;
}

bool PostgreDatabase::Initialize(const char* Hostname, unsigned int port, const char* Username, const char* Password, const char* DatabaseName, uint32 ConnectionCount, uint32 BufferSize)
{
    mConnectionCount = ConnectionCount;

    // Build the connection string
    stringstream ss;
    ss << "host = " << Hostname << " port = " << port << " ";
    ss << "user = '" << Username << "' ";
    if(strlen(Password) > 0)
        ss << "password = '" << Password << "' ";

    ss << "dbname = '" << DatabaseName << "'";
    mConnectionString = ss.str();

    Connections = new PGconn*[mConnectionCount];
    InUseMarkers = new bool[mConnectionCount];
    QueryBuffer = new char*[mConnectionCount];
    DelayedQueryBuffer = new char[BufferSize];

    for(int i = 0; i < mConnectionCount; ++i)
    {
        Connections[i] = NULL;
        InUseMarkers[i] = false;
        QueryBuffer[i] = new char[BufferSize];
    }

    bool result = Connect();
    if(!result) return false;

    if(result && mConnectionCount > 1)
    {
        // Spawn MySQLDatabase thread
        //ZThread::Thread t(new PostgreDatabaseThread(this));
        //sLog.outString("sql: Spawned delayed MySQLDatabase query thread...");
    }
    return result;
}

bool PostgreDatabase::Connect()
{
    sLog.outString("Connecting to PostgreSQL Database with [%s]", mConnectionString.c_str());
    for(uint32 i = 0; i < mConnectionCount; ++i)
    {
        if(!Connect(i))
            return false;
    }
    //sLog.outString("sql: %u MySQL connections established.", mConnectionCount);
    return true;
}

bool PostgreDatabase::Connect(uint32 ConnectionIndex)
{
    Connections[ConnectionIndex] = PQconnectdb(mConnectionString.c_str());
    if(Connections[ConnectionIndex] == 0) return false;
    if(PQstatus(Connections[ConnectionIndex]) != CONNECTION_OK)
    {
        // failed for some reason
        sLog.outError("PostgreSQL connection failed: %s", PQerrorMessage(Connections[ConnectionIndex]));

        // free the memory
        Disconnect(ConnectionIndex);
        return false;
    }
    return true;
}

bool PostgreDatabase::Disconnect(uint32 ConnectionIndex)
{
    if(Connections[ConnectionIndex] == 0) return false;
    PQfinish(Connections[ConnectionIndex]);
    Connections[ConnectionIndex] = 0;
    return true;
}

uint32 PostgreDatabase::GetConnection()
{
    int32 index = -1;
    while(index == -1)
    {
        for(uint32 i = 0; i < mConnectionCount; ++i)
        {
            if(Connections[i] && InUseMarkers[i] == false)
            {
                index = i;
                break;
            }
        }
        ZThread::Thread::sleep(5);
    }
    return index;
}

void PostgreDatabase::Shutdown()
{
    sLog.outString("sql: Closing all PostgreSQL connections...");
    
    for(uint32 i = 0; i < mConnectionCount; ++i)
        Disconnect(i);
    
    sLog.outString("sql: %u connections closed.", mConnectionCount);
}

PGresult * PostgreDatabase::SendQuery(uint32 ConnectionIndex, const char* Sql, bool Self)
{
    PGresult * res = PQexec(Connections[ConnectionIndex], Sql);
    return res;
}

QueryResult * PostgreDatabase::Query(const char* QueryString, ...)
{
    if(QueryString == NULL) return NULL;

    va_list vlist;
    va_start(vlist, QueryString);

    mSearchMutex.acquire();
    // Find a free connection
    uint32 i = GetConnection();

    // Mark the connection as busy
    InUseMarkers[i] = true;
    mSearchMutex.release();

    // Apply parameters
    vsprintf(QueryBuffer[i], QueryString, vlist);
    va_end(vlist);

    // Send the query
    PGresult * res = SendQuery(i, QueryBuffer[i], false);
    InUseMarkers[i] = false;

    // Get the error code
    ExecStatusType result = PQresultStatus(res);
    if(result != PGRES_TUPLES_OK)
    {
        sLog.outError("Query failed: %s", PQresultErrorMessage(res));
        // command failed.
        PQclear(res);
        return 0;
    }

    // Better check the row count.. we don't want to return an empty query..
    if(PQntuples(res) == 0)
    {
        // oh noes!
        PQclear(res);
        return 0;
    }

    // get number of columns
    uint32 FieldCount = PQnfields(res);

    // get number of rows
    uint32 RowCount = PQntuples(res);

    // Create the QueryResult
    PostgreQueryResult * qResult = new PostgreQueryResult(res, FieldCount, RowCount);
    return qResult;
}

bool PostgreDatabase::Execute(const char* QueryString, ...)
{
    if(QueryString == NULL) return false;

    va_list vlist;
    va_start(vlist, QueryString);

    if(mQueryThread == 0)
    {
        DelayedQueryBufferMutex.acquire();

        vsprintf(DelayedQueryBuffer, QueryString, vlist);
        bool res = WaitExecute(DelayedQueryBuffer);

        DelayedQueryBufferMutex.release();
        return res;
    }

    /*DelayedQueryBufferMutex.acquire();

    vsprintf(DelayedQueryBuffer, QueryString, vlist);
    mQueryThread->AddQuery(DelayedQueryBuffer);

    DelayedQueryBufferMutex.release();*/
    return false;
}

bool PostgreDatabase::WaitExecute(const char* QueryString, ...)
{
    if(QueryString == NULL) return false;

    va_list vlist;
    va_start(vlist, QueryString);

    mSearchMutex.acquire();
    uint32 Connection = GetConnection();
    InUseMarkers[Connection] = true;
    mSearchMutex.release();

    vsprintf(QueryBuffer[Connection], QueryString, vlist);

    PGresult * res = SendQuery(Connection, QueryBuffer[Connection], false);
    if(res == 0) return false;
    InUseMarkers[Connection] = false;

    ExecStatusType result = PQresultStatus(res);
    bool passed = false;

    if(result == PGRES_TUPLES_OK || result == PGRES_COMMAND_OK)
        passed = true;
    else
        sLog.outError("Execute failed because of [%s]", PQresultErrorMessage(res));

    // free up the memory
    PQclear(res);

    return passed;
}

PostgreQueryResult::PostgreQueryResult(PGresult * res, uint32 FieldCount, uint32 RowCount) : QueryResult(FieldCount, RowCount, DATABASE_TYPE_PGSQL)
{
    // set result for later deletion and use
    mResult = res;

    // starting at row 0
    mRow = 0;

    // retreieve the data
    NextRow();
}

PostgreQueryResult::~PostgreQueryResult()
{

}

void PostgreQueryResult::Destroy()
{
    PQclear(mResult);
    mResult = 0;
}

bool PostgreQueryResult::NextRow()
{
    // check if we reached the end
    if(mRow == mRowCount) return false;

    // get each field and set it in result
    char * value;
    for(uint32 i = 0; i < mFieldCount; ++i)
    {
        value = PQgetvalue(mResult, mRow, i);
        if(value == 0) return false;

        mCurrentRow[i].SetValue(value);
    }

    mRow++;
    return true;
}

void PostgreDatabase::CheckConnections()
{
    // Check every 30 seconds (TODO: MAKE CUSTOMIZABLE)
    if(getMSTime() < mNextPing)
        return;

    mNextPing = getMSTime() + 60000;
    for(uint32 i = 0; i < mConnectionCount; ++i)
    {
        if(Connections[i] != 0 && PQstatus(Connections[i]) != CONNECTION_OK)
        {
            // disconnect and reconnect
            Disconnect(i);
            Connect(i);
        }
    }
}

#endif
