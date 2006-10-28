/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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

#include "DatabaseEnv.h"
#include "Util.h"
#include "Policies/SingletonImp.h"

using namespace std;

void DatabaseMysql::ThreadStart()
{
    mysql_thread_init();
}

void DatabaseMysql::ThreadEnd()
{
    mysql_thread_end();
}

size_t DatabaseMysql::db_count = 0;


DatabaseMysql::DatabaseMysql() : Database(), mMysql(0)
{
    // before first connection
    if( db_count++ == 0 )
    {
        // Mysql Library Init
        mysql_library_init(-1, NULL, NULL);

        if (!mysql_thread_safe())
        {
            sLog.outError("FATAL ERROR: Used MySQL library isn't thread-safe.");
            exit(1);
        }
    }
}

DatabaseMysql::~DatabaseMysql()
{
    if (mMysql)
        mysql_close(mMysql);
    
    //Free Mysql library pointers for last ~DB
    if(--db_count == 0)
        mysql_library_end();
}

bool DatabaseMysql::Initialize(const char *infoString)
{
    MYSQL *mysqlInit;
    mysqlInit = mysql_init(NULL);
    if (!mysqlInit)
    {
        sLog.outError( "Could not initialize Mysql connection" );
        return false;
    }

    vector<string> tokens = StrSplit(infoString, ";");

    vector<string>::iterator iter;

    std::string host, port, user, password, database;
    iter = tokens.begin();

    if(iter != tokens.end())
        host = *iter++;
    if(iter != tokens.end())
        port = *iter++;
    if(iter != tokens.end())
        user = *iter++;
    if(iter != tokens.end())
        password = *iter++;
    if(iter != tokens.end())
        database = *iter++;

    mysql_options(mysqlInit,MYSQL_SET_CHARSET_NAME,"utf8");
    mMysql = mysql_real_connect(mysqlInit, host.c_str(), user.c_str(),
        password.c_str(), database.c_str(), atoi(port.c_str()), 0, 0);

    if (mMysql)
    {
        sLog.outDetail( "Connected to MySQL database at %s\n",
            host.c_str());
        return true;
    }
    else
    {
        sLog.outError( "Could not connect to MySQL database at %s: %s\n",
            host.c_str(),mysql_error(mysqlInit));
        mysql_close(mysqlInit);
        return false;
    }
}

QueryResult* DatabaseMysql::PQuery(const char *format,...)
{
    if(!format) return NULL;

    va_list ap;
    char szQuery [1024];
    va_start(ap, format);
    int res = vsnprintf( szQuery, 1024, format, ap );
    va_end(ap);

    if(res==-1)
    {
        sLog.outError("SQL Query truncated (and not execute) for format: %s",format);
        return false;
    }

    return Query(szQuery);
}

QueryResult* DatabaseMysql::Query(const char *sql)
{
    if (!mMysql)
        return 0;

    MYSQL_RES *result = 0;
    uint64 rowCount = 0;
    uint32 fieldCount = 0;

    {
        // guarded block for thread-safe mySQL request
        ZThread::Guard<ZThread::FastMutex> query_connection_guard(mMutex);

        if(mysql_query(mMysql, sql))
        {
            ERROR_LOG( "SQL: %s\n", sql );
            ERROR_LOG( (std::string("query ERROR: ") + mysql_error(mMysql)).c_str() );
            return NULL;
        }

        result = mysql_store_result(mMysql);

        rowCount = mysql_affected_rows(mMysql);
        fieldCount = mysql_field_count(mMysql);
        // end guarded block
    }

    if (!result )
        return NULL;

    if (!rowCount)
    {
        mysql_free_result(result);
        return NULL;
    }

    QueryResultMysql *queryResult = new QueryResultMysql(result, rowCount, fieldCount);

    queryResult->NextRow();

    DEBUG_LOG( "SQL: %s", sql );
    return queryResult;
}

bool DatabaseMysql::Execute(const char *sql)
{
    if (!mMysql)
        return false;

    {
        // guarded block for thread-safe mySQL request
        ZThread::Guard<ZThread::FastMutex> query_connection_guard(mMutex);

        DEBUG_LOG( (std::string("SQL: ") + sql).c_str() );
        if(mysql_query(mMysql, sql))
        {
            ERROR_LOG( (std::string("SQL ERROR: ") + mysql_error(mMysql)).c_str() );
            return false;
        }
        // end guarded block
    }

    return true;
}

bool DatabaseMysql::PExecute(const char * format,...)
{
    if (!format)
        return false;

    va_list ap;
    char szQuery [1024];
    va_start(ap, format);
    int res = vsnprintf( szQuery, 1024, format, ap );
    va_end(ap);

    if(res==-1)
    {
        sLog.outError("SQL Query truncated (and not execute) for format: %s",format);
        return false;
    }

    return Execute(szQuery);
}
