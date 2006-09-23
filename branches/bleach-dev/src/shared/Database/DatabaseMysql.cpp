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

DatabaseMysql::DatabaseMysql() : Database(), mMysql(0)
{
	ACE_TRACE("DatabaseMysql::DatabaseMysql() : Database(), mMysql(0), m_error(0)");

    DatabaseRegistry::RegisterDatabase(this);
}

DatabaseMysql::~DatabaseMysql()
{
	ACE_TRACE("DatabaseMysql::~DatabaseMysql()");

    if (mMysql)
        mysql_close(mMysql);

}

int
DatabaseMysql::Initialize(const char *infoString)
{
	ACE_TRACE("DatabaseMysql::Initialize(const char *infoString)");

    MYSQL *mysqlInit;
    mysqlInit = mysql_init(NULL);
    if (!mysqlInit)
    {
        ACE_ERROR_RETURN ((LM_ERROR, "Could not initialize Mysql.\n"), 0);
    }

    std::vector<std::string> tokens = StrSplit(infoString, ";");

    std::vector<std::string>::iterator iter;

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

    mysql_options(mysqlInit, MYSQL_SET_CHARSET_NAME, "utf8");

    mMysql = mysql_real_connect(mysqlInit, host.c_str(), user.c_str(), password.c_str(), database.c_str(), atoi(port.c_str()), 0, 0);

    if (mMysql)
	{
		ACE_DEBUG( (LM_INFO, "Connected to MySQL database at %s.\n", host.c_str()));
		return 1;
	}
	ACE_ERROR_RETURN ((LM_ERROR, "Could not connect to MySQL database at %s: %s.\n", host.c_str(), mysql_error(mysqlInit)), 0);
}

QueryResult* DatabaseMysql::PQuery(const char *format, ...)
{
	ACE_TRACE("DatabaseMysql::PQuery(const char *format, ...)");

    if(!format)
		return 0;

    va_list ap;
    char szQuery [1024];
    va_start(ap, format);
    vsprintf( szQuery, format, ap );
    va_end(ap);

    return Query(szQuery);

}

QueryResult* DatabaseMysql::Query(const char *sql)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> locker (mutex_);
	
	ACE_TRACE("DatabaseMysql::Query(const char *sql)");
	
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("SQL: %s\n"), sql ));

    if (!mMysql)
        return 0;

    MYSQL_RES *result = 0;
    uint64 rowCount = 0;
    uint32 fieldCount = 0;

	if(mysql_query(mMysql, sql))
	{
		ACE_DEBUG((LM_ALERT, ACE_TEXT("%I-- %D - %M - MySQL Error(%s) -- %N:%l --\n"), mysql_error(mMysql) ));
		return NULL;
	}

	result = mysql_store_result(mMysql);

	rowCount = mysql_affected_rows(mMysql);
	fieldCount = mysql_field_count(mMysql);

    if (!result )
        return 0;

    if (!rowCount)
	{
        mysql_free_result(result);
        return 0;
    }

    QueryResultMysql *queryResult = new QueryResultMysql(result, rowCount, fieldCount);

    queryResult->NextRow();

    return queryResult;
}

bool DatabaseMysql::Execute(const char *sql)
{
	ACE_TRACE("DatabaseMysql::Execute(const char *sql)");
	
	ACE_Guard<ACE_Recursive_Thread_Mutex> locker (mutex_);

    if (!mMysql)
        return false;

	if(mysql_query(mMysql, sql))
	{
		ACE_DEBUG( (LM_DEBUG, ACE_TEXT("SQL: %s\n"), sql ));
		ACE_DEBUG( (LM_DEBUG, ACE_TEXT("query ERROR: %s\n"), mysql_error(mMysql) ));
		return false;
	}

	ACE_DEBUG( (LM_DEBUG, "SQL: %s.\n", sql ));

    return true;
}

bool DatabaseMysql::PExecute(const char * format, ...)
{
	ACE_TRACE("DatabaseMysql::PExecute(const char *format, ...)");
    if (!format)
        return false;
    va_list ap;
    char szQuery [1024];
    va_start(ap, format);
    vsprintf( szQuery, format, ap );
    va_end(ap);

    return Execute(szQuery);
}
