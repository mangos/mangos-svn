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

#ifdef DO_POSTGRESQL

#include "Util.h"
#include "Policies/SingletonImp.h"
#include "Platform/Define.h"
#include "../src/zthread/ThreadImpl.h"
#include "DatabaseEnv.h"
#include "Database/SqlOperations.h"

using namespace std;

void DatabasePostgre::ThreadStart()
{
}

void DatabasePostgre::ThreadEnd()
{
}

size_t DatabasePostgre::db_count = 0;

DatabasePostgre::DatabasePostgre() : Database()
{
}

DatabasePostgre::~DatabasePostgre()
{
    if( mPGconn )
    {
        PQfinish(mPGconn);
        mPGconn = NULL;
    }
}

bool DatabasePostgre::Initialize(const char *infoString)
{
    if(!Database::Initialize(infoString))
        return false;

    tranThread = NULL;

    vector<string> tokens = StrSplit(infoString, ";");

    vector<string>::iterator iter;

    std::string host, port_or_socket, user, password, database;

    iter = tokens.begin();

    if(iter != tokens.end())
        host = *iter++;
    if(iter != tokens.end())
        port_or_socket = *iter++;
    if(iter != tokens.end())
        user = *iter++;
    if(iter != tokens.end())
        password = *iter++;
    if(iter != tokens.end())
        database = *iter++;

    mPGconn = PQsetdbLogin(host.c_str(), port_or_socket.c_str(), NULL, NULL, database.c_str(), user.c_str(), password.c_str());

    /* check to see that the backend connection was successfully made */
    if (PQstatus(mPGconn) != CONNECTION_OK)
    {
        sLog.outError( "Could not connect to Postgre database at %s: %s\n",
            host.c_str(), PQerrorMessage(mPGconn));
        PQfinish(mPGconn);
        return false;
    }
    else
    {
        sLog.outDetail( "Connected to Postgre database at %s\n",
            host.c_str());
        return true;
    }

}

QueryResult* DatabasePostgre::Query(const char *sql)
{
    if (!mPGconn)
        return 0;

    uint64 rowCount = 0;
    uint32 fieldCount = 0;

    // guarded block for thread-safe mySQL request
    ZThread::Guard<ZThread::FastMutex> query_connection_guard((ZThread::ThreadImpl::current()==tranThread?tranMutex:mMutex));

    // Send the query
    PGresult * result = PQexec(mPGconn, sql);

    if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
        sLog.outErrorDb( "SQL: %s", sql );
        sLog.outErrorDb("query ERROR: %s", PQerrorMessage(mPGconn));
        PQclear(result);
        return NULL;
    }
    else
    {
        DEBUG_LOG( "SQL: %s", sql );
    }

    rowCount = PQntuples(result);
    fieldCount = PQnfields(result);
    // end guarded block

    if (!result )
    {
        PQclear(result);
        return NULL;
    }

    if (!rowCount)
    {
        PQclear(result);
        return NULL;
    }

    QueryResultPostgre * queryResult = new QueryResultPostgre(result, fieldCount, rowCount);
    return queryResult;

    queryResult->NextRow();

    return queryResult;
}

bool DatabasePostgre::Execute(const char *sql)
{
    if (!mPGconn)
        return false;

    {
        // guarded block for thread-safe mySQL request
        ZThread::Guard<ZThread::FastMutex> query_connection_guard((ZThread::ThreadImpl::current()==tranThread?tranMutex:mMutex));

        PGresult *res = PQexec(mPGconn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            sLog.outErrorDb("SQL: %s", sql);
            sLog.outErrorDb("SQL ERROR: %s", PQerrorMessage(mPGconn));
            return false;
        }
        else
        {
            DEBUG_LOG("SQL: %s", sql);
        }
        PQclear(res);

        // end guarded block
    }

    return true;
}

bool DatabasePostgre::DirectExecute(const char* sql)
{
    return true;
}

bool DatabasePostgre::_TransactionCmd(const char *sql)
{
    if (!mPGconn)
        return false;

    PGresult *res = PQexec(mPGconn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        sLog.outError("SQL: %s", sql);
        sLog.outError("SQL ERROR: %s", PQerrorMessage(mPGconn));
        return false;
    }
    else
    {
        DEBUG_LOG("SQL: %s", sql);
    }
    return true;
}

bool DatabasePostgre::BeginTransaction()
{
    if (!mPGconn)
        return false;
    if (tranThread==ZThread::ThreadImpl::current())
        return false;                                       // huh? this thread already started transaction
    mMutex.acquire();
    if (!_TransactionCmd("START TRANSACTION"))
    {
        mMutex.release();                                   // can't start transaction
        return false;
    }
    // transaction started
    tranThread = ZThread::ThreadImpl::current();            // owner of this transaction
    return true;
}

bool DatabasePostgre::CommitTransaction()
{
    if (!mPGconn)
        return false;
    if (tranThread!=ZThread::ThreadImpl::current())
        return false;
    bool _res = _TransactionCmd("COMMIT");
    tranThread = NULL;
    mMutex.release();
    return _res;
}

bool DatabasePostgre::RollbackTransaction()
{
    if (!mPGconn)
        return false;
    if (tranThread!=ZThread::ThreadImpl::current())
        return false;
    bool _res = _TransactionCmd("ROLLBACK");
    tranThread = NULL;
    mMutex.release();
    return _res;
}

unsigned long DatabasePostgre::escape_string(char *to, const char *from, unsigned long length)
{
    if (!mPGconn || !to || !from || !length)
        return 0;

    return PQescapeString(to, from, length);
}

void DatabasePostgre::InitDelayThread()
{
}

void DatabasePostgre::HaltDelayThread()
{
}
#endif
