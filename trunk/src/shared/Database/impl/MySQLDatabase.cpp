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
#include "Timer.h"

#ifdef DATABASE_SUPPORT_MYSQL

MySQLDatabase::MySQLDatabase() : Database(DATABASE_TYPE_MYSQL)
{
    Connections = NULL;
    InUseMarkers = NULL;
    QueryBuffer = NULL;
    mConnectionCount = -1;   // Not connected.
    mNormalCount = -1;
    mNextPing = getMSTime();
    mQueryThread = 0;
}

MySQLDatabase::~MySQLDatabase()
{
    // Close connections..

    for(int32 i = 0; i < mConnectionCount; ++i)
    {
        if(Connections)
            if(Connections[i])
			{
                Disconnect(i);
				delete [] Connections[i];
			}


        delete [] QueryBuffer[i];
    }

    delete [] Connections;
    delete [] InUseMarkers;
    delete [] QueryBuffer;
    delete [] DelayedQueryBuffer;
}

bool MySQLDatabase::Initialize(const char* Hostname, unsigned int Port, const char* Username, const char* Password, const char* DatabaseName, uint32 ConnectionCount, uint32 BufferSize)
{
    // Create arrays
    mConnectionCount = ConnectionCount;
    mHostname = Hostname;
    mUsername = Username;
    mPassword = Password;
    mDatabaseName = DatabaseName;
    mPort = Port;

    Connections = new MYSQL*[mConnectionCount];
    InUseMarkers = new FastMutex[mConnectionCount];
    QueryBuffer = new char*[mConnectionCount];
    DelayedQueryBuffer = new char[BufferSize];

    for(int i = 0; i < mConnectionCount; ++i)
    {
        Connections[i] = NULL;
        QueryBuffer[i] = new char[BufferSize];
    }

    bool result = Connect();
    if(!result) return false;

    result = SelectDatabase();
    mNormalCount = mConnectionCount;

    if(result && mConnectionCount > 1)
    {
        // Spawn MySQLDatabase thread
        //launch_thread(new MySQLDatabaseThread(this));
        new Thread(new MySQLDatabaseThread(this));

        // Allocate one for the query thread especially.
        mDelayedID = mConnectionCount - 1;
        mNormalCount--;

        //sLog.outString("sql: Spawned delayed MySQLDatabase query thread...");
    }
    return result;
}

bool MySQLDatabase::Connect()
{
    sLog.outString("Connecting to MySQL Database on %s with (%s : *********)", mHostname.c_str(), mUsername.c_str());
    for(uint32 i = 0; i < mConnectionCount; ++i)
    {
        if(!Connect(i))
            return false;
    }
    //sLog.outString("sql: %u MySQL connections established.", mConnectionCount);
    return true;
}

bool MySQLDatabase::Connect(uint32 ConnectionIndex)
{
    MYSQL* Descriptor = mysql_init(NULL);
    MYSQL* Descriptor2 = Descriptor;
    // Set reconnect
    #ifdef WIN32
    my_bool my_true = true;
    if (mysql_options(Descriptor, mysql_option(20)/*MYSQL_OPT_RECONNECT*/, &my_true))
        sLog.outString("sql: MYSQL_OPT_RECONNECT could not be set, connection drops may occur but will be counteracted.");
    #else
        my_bool my_true = true;
        Descriptor->reconnect = my_true;
    #endif

    Descriptor = mysql_real_connect(Descriptor2, mHostname.c_str(),
        mUsername.c_str(), mPassword.c_str(), "", mPort, NULL, 0);
    if(Descriptor == NULL)
    {
        sLog.outError("sql: Connection failed. Reason was `%s`", mysql_error(Descriptor2));
        return false;
    }

    Connections[ConnectionIndex] = Descriptor;
    return true;
}

bool MySQLDatabase::SelectDatabase()
{
    for(uint32 i = 0; i < mConnectionCount; ++i)
    {
        if(!SelectDatabase(i))
            return false;
    }
    //sLog.outString("sql: %u MySQLDatabase connections ready for use.", mConnectionCount);
    return true;
}

bool MySQLDatabase::SelectDatabase(uint32 ConnectionIndex)
{
    if(mysql_select_db(Connections[ConnectionIndex], mDatabaseName.c_str()))
    {
        sLog.outError("sql: Select of MySQLDatabase %s failed due to `%s`", mDatabaseName.c_str(),
            mysql_error(Connections[ConnectionIndex]));
        return false;
    }
    return true;
}

bool MySQLDatabase::Disconnect(uint32 ConnectionIndex)
{
    if(Connections[ConnectionIndex] != 0)
    {
        printf("sql: Closing connection %u\n", ConnectionIndex);
        mysql_close(Connections[ConnectionIndex]);
        Connections[ConnectionIndex] = NULL;
        return true;
    }
    return false;
}

void MySQLDatabase::CheckConnections()
{
    // Check every 30 seconds (TODO: MAKE CUSTOMIZABLE)
    if(getMSTime() < mNextPing)
        return;

    mNextPing = getMSTime() + 60000;
    mSearchMutex.acquire();
    for(uint32 i = 0; i < mNormalCount; ++i)
    {
        if(InUseMarkers[i].tryAcquire(1))
        {
            if(Connections[i] && mysql_ping(Connections[i]))
            {
                sLog.outString("sql: mysql_ping failed, attempting to reconnect to MySQLDatabase...");
                Disconnect(i);
                if(!Connect(i)) {
                    sLog.outError("sql: Connection %u was unable to reconnect. This could mean a failure with your MySQLDatabase.");
                    Disconnect(i);
                } else {
                    SelectDatabase(i);
                }
                InUseMarkers[i].release();
            } else if(Connections[i] == 0)
            {
                // Attempt to reconnect.
                if(Connect(i) && SelectDatabase(i))
                {
                    sLog.outString("sql: Connection %u re-established.", i);
                }
            }
            InUseMarkers[i].release();
        }
    }
    mSearchMutex.release();
}

uint32 MySQLDatabase::GetConnection()
{
    while(true)
    {
        for(uint32 i = 0; i < mNormalCount; ++i)
        {
            if(Connections[i] && InUseMarkers[i].tryAcquire(1))
            {
                return i;
            }
        }
    }
}

void MySQLDatabase::Shutdown()
{
    sLog.outString("sql: Closing all MySQLDatabase connections...");
    for(uint32 i = 0; i < mConnectionCount; ++i)
    {
        if(Connections[i])
        {
            mysql_close(Connections[i]);
            Connections[i] = 0;
        }
    }
    sLog.outString("sql: %u connections closed.", mConnectionCount);
}

bool MySQLDatabase::SendQuery(uint32 ConnectionIndex, const char* Sql, bool Self)
{
    int result = mysql_query(Connections[ConnectionIndex], Sql);
    if(result > 0)
    {
        sLog.outDetail("Sql query failed due to [%s]", mysql_error(Connections[ConnectionIndex]));
        if( Self == false && HandleError( ConnectionIndex, mysql_errno(Connections[ConnectionIndex]) ) )
        {
            // Re-send the query, the connection was successful.
            // The true on the end will prevent an endless loop here, as it will
            // stop after sending the query twice.
            SendQuery(ConnectionIndex, Sql, true);
        }
    }

    return (result == 0 ? true : false);
}

bool MySQLDatabase::HandleError(uint32 ConnectionIndex, uint32 ErrorNumber)
{
    // Handle errors that should cause a reconnect to the MySQLDatabase.
    switch(ErrorNumber)
    {
    case 2006:  // Mysql server has gone away
    case 2008:  // Client ran out of memory
    case 2013:  // Lost connection to sql server during query
    case 2055:  // Lost connection to sql server - system error
        {
            // Let's instruct a reconnect to the db when we encounter these errors.
            Disconnect(ConnectionIndex);

            // Re-send the query if our connection actually succeeds.
            bool val = Connect(ConnectionIndex);
            if(val)
                val = SelectDatabase(ConnectionIndex);

            return val;
        }break;
    }

    return false;
}

QueryResult * MySQLDatabase::Query(const char* QueryString, ...)
{
    if(QueryString == NULL) return NULL;

    va_list vlist;
    va_start(vlist, QueryString);

    mSearchMutex.acquire();
    // Find a free connection
    uint32 i = GetConnection();

    // Mark the connection as busy
    mSearchMutex.release();

    // Apply parameters
    vsprintf(QueryBuffer[i], QueryString, vlist);
    va_end(vlist);

    // Send the query
    bool Success = SendQuery(i, QueryBuffer[i], false);
    QueryResult * qResult = NULL;

    if(Success)
    {
        // We got a valid query. :)
        MYSQL_RES * Result = mysql_store_result(Connections[i]);

        // Don't think we're gonna have more than 4 billion rows......
        uint32 RowCount = mysql_affected_rows(Connections[i]);
        uint32 FieldCount = mysql_field_count(Connections[i]);

        // Check if we have no rows.
        if(!RowCount || !FieldCount) {
            mysql_free_result(Result);
        } else {
            qResult = new MySQLQueryResult( Result, FieldCount, RowCount );
            qResult->NextRow();
        }
    }

    InUseMarkers[i].release();
    return qResult;
}

bool MySQLDatabase::Execute(const char* QueryString, ...)
{
    if(QueryString == NULL) return false;

    va_list vlist;
    va_start(vlist, QueryString);

    if(mQueryThread == 0)
    {
        // No query thread.
        // Assume we're dealing with a normal query.
        mSearchMutex.acquire();
        uint32 Connection = GetConnection();
        mSearchMutex.release();

        vsprintf(QueryBuffer[Connection], QueryString, vlist);

        bool Result = SendQuery(Connection, QueryBuffer[Connection], false);
        InUseMarkers[Connection].release();

        return Result;
    }

    DelayedQueryBufferMutex.acquire();

    vsprintf(DelayedQueryBuffer, QueryString, vlist);
    mQueryThread->AddQuery(DelayedQueryBuffer);

    DelayedQueryBufferMutex.release();
    return true;
}

bool MySQLDatabase::WaitExecute(const char* QueryString, ...)
{
    if(QueryString == NULL) return false;

    va_list vlist;
    va_start(vlist, QueryString);

    mSearchMutex.acquire();
    uint32 Connection = GetConnection();
    mSearchMutex.release();

    vsprintf(QueryBuffer[Connection], QueryString, vlist);

    bool Result = SendQuery(Connection, QueryBuffer[Connection], false);
    InUseMarkers[Connection].release();

    return Result;
}

/************************************************************************/
/* Thread class                                                         */
/************************************************************************/


MySQLDatabaseThread::MySQLDatabaseThread(MySQLDatabase *db) : Runnable()
{
    //ThreadType = THREADTYPE_DATABASE;
    _db = db;
    _db->mQueryThread = this;
}

void MySQLDatabaseThread::AddQuery(std::string query)
{
    _queue.add(query);
}

void MySQLDatabaseThread::run()
{
    //THREAD_TRY_EXECUTION2
        Do();
    //THREAD_HANDLE_CRASH2
}

void MySQLDatabaseThread::Do()
{
    // set thread name
    //SetThreadName("MySQL Database Execute Thread");
    Sleep(2000);
    bool err = false;
    string query;
    MYSQL * conn = _db->Connections[_db->mDelayedID];
    assert(conn != 0);
    uint32 i = _db->mDelayedID;

    uint32 check_interval = 0;

    while(_db->Active)
    {
        ++check_interval;
        if(check_interval == 15)        // per 30 seconds
        {
            check_interval = 0;
            if(mysql_ping(_db->Connections[i]))
            {
                sLog.outError("Delayed query thread SQL connection failed!. Reconnecting.");
                // connection dropped
                _db->Disconnect(i);
                while(true)
                {
                    if(!_db->Connect(i) || !_db->SelectDatabase(i))
                    {
                        sLog.outError("Delayed query thread failed reconnection. Trying again in 2 seconds.");
                        Sleep(2000);
                    }
                    else
                    {
                        break;
                    }
                }                
            }
        }
        if(_queue.size())
        {
            while(_queue.size())
            {
                query = _queue.next();
                _db->SendQuery(i, query.c_str(), false);
            }
        }
        Sleep(200);
    }
}

MySQLDatabaseThread::~MySQLDatabaseThread()
{
    _db->mQueryThread = NULL;
}


MySQLQueryResult::MySQLQueryResult(MYSQL_RES* res, uint32 FieldCount, uint32 RowCount) : QueryResult(FieldCount, RowCount, DATABASE_TYPE_MYSQL), mResult(res)
{

}

MySQLQueryResult::~MySQLQueryResult()
{
    
}

bool MySQLQueryResult::NextRow()
{
    MYSQL_ROW row = mysql_fetch_row(mResult);
    if(row == NULL)
        return false;

    for(uint32 i = 0; i < mFieldCount; ++i)
    {
        mCurrentRow[i].SetValue(row[i]);
    }

    return true;
}

void MySQLQueryResult::Destroy()
{
    mysql_free_result(mResult);
    mResult = 0;
}

unsigned long MySQLDatabase::escape_string(char *to, const char *from, unsigned long length)
{
    mSearchMutex.acquire();
    uint32 c = GetConnection();
    mSearchMutex.release();

    if (!Connections[c] || !to || !from || !length)
        return 0;

    uint32 ret = mysql_real_escape_string(Connections[c], to, from, length);
    InUseMarkers[c].release();
    return ret;
}

#endif
