/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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

INSTANTIATE_SINGLETON_1(DatabaseMysql);

using namespace std;

DatabaseMysql::DatabaseMysql() : Database(), mMysql(0)
{
    DatabaseRegistry::RegisterDatabase(this);
}


DatabaseMysql::~DatabaseMysql()
{
    if (mMysql)
        mysql_close(mMysql);
}


bool DatabaseMysql::Initialize(const char *infoString)
{
    MYSQL *mysqlInit;
    mysqlInit = mysql_init(NULL);
    if (!mysqlInit)
    {
        sLog.outError( "Could not initialize Mysql" );
        return false;
    }

    vector<string> tokens = StrSplit(infoString, ";");
    std::string params[4] = { "", "", "", "" };

    vector<string>::iterator iter;

    std::string host, user, password, database;
    iter = tokens.begin();

    if(iter != tokens.end())
        host = *iter++;
    if(iter != tokens.end())
        user = *iter++;
    if(iter != tokens.end())
        password = *iter++;
    if(iter != tokens.end())
        database = *iter++;

    mMysql = mysql_real_connect(mysqlInit, host.c_str(), user.c_str(),
        password.c_str(), database.c_str(), 0, 0, 0);

    if (mMysql)
        sLog.outDetail( "Connected to MySQL database at %s\n",
				       host.c_str());
    else
        sLog.outError( "Could not connect to MySQL database at %s: %s\n",
				      host.c_str(),mysql_error(mysqlInit));

    if(mMysql)
        return true;
    else
        return false;
}

QueryResult* DatabaseMysql::PQuery(const char *format,...)
{
	if( !format || !mMysql) return NULL;
    

	va_list ap;
	char szQuery [512];
	va_start(ap, format);
	vsprintf( szQuery, format, ap );
	va_end(ap);

    if (mysql_query(mMysql, szQuery))
        return NULL;

    MYSQL_RES *result = mysql_store_result(mMysql);
    uint64 rowCount = mysql_affected_rows(mMysql);
    uint32 fieldCount = mysql_field_count(mMysql);
    if (!result)
	   return NULL;
	else if( !rowCount )
       return NULL;

    QueryResultMysql *queryResult = new QueryResultMysql(result, rowCount, fieldCount);
    if(!queryResult)
        return NULL;
    
    queryResult->NextRow();
	
    DEBUG_LOG( "query = %s\n", szQuery );
    return queryResult;
}
QueryResult* DatabaseMysql::Query(const char *sql)
{
    if (!mMysql)
        return 0;

    

    if (mysql_query(mMysql, sql))
        return 0;

    MYSQL_RES *result = mysql_store_result(mMysql);

    uint64 rowCount = mysql_affected_rows(mMysql);
    uint32 fieldCount = mysql_field_count(mMysql);

	if (!result )
        return 0;
    else
    if (!rowCount)
        return 0;
    

    QueryResultMysql *queryResult = new QueryResultMysql(result, rowCount, fieldCount);
    if(!queryResult)
    {
        
        return 0;
    }

    queryResult->NextRow();

    return queryResult;
}


bool DatabaseMysql::Execute(const char *sql)
{
    if (!mMysql)
        return false;

    DEBUG_LOG( (std::string("SQL: ") + sql).c_str() );
    if (mysql_query(mMysql, sql))
        return false;

    return true;
}

bool DatabaseMysql::PExecute(const char * format,...)
{
    if (!mMysql||!format)
        return false;
	va_list ap;
	char szQuery [512];
	va_start(ap, format);
	vsprintf( szQuery, format, ap );
	va_end(ap);

    

    DEBUG_LOG( (std::string("SQL: ") + szQuery).c_str() );
    if (mysql_query(mMysql, szQuery))
        return false;

    return true;
}
