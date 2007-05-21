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

#include "DatabaseEnv.h"

unsigned long Database::escape_string(char *to, const char *from, unsigned long length)
{
    assert(false);
    return 0;
}

bool Database::PExecuteLog(const char * format,...)
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

	/*
	// UQ1: Disabled this for new SQL code.. Re-add it if you think it is needed.. I think its a waste of cpu time!
    if( m_logSQL )
    {
        time_t curr;
        tm local;
        time(&curr);                                            // get current time_t value
        local=*(localtime(&curr));                              // dereference and assign
        char fName[128];
        sprintf( fName, "%04d-%02d-%02d_logSQL.sql", local.tm_year+1900, local.tm_mon+1, local.tm_mday );

        std::fstream log_file ( fName, std::ios::app );

        if ( !log_file.is_open() )
        {
            // The file could not be opened
            sLog.outError("SQL-Logging is disabled - Log file for the SQL commands could not be openend: %s",fName);
        }
        else
        {
            // Safely use the file stream
            log_file << szQuery << "\n";
            log_file.close();
        }
    }*/

    return Execute(szQuery);
}

bool Database::ExecuteLog(const char * format,...)
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

	/*
	// UQ1: Disabled this for new SQL code.. Re-add it if you think it is needed.. I think its a waste of cpu time!
    if( m_logSQL )
    {
        time_t curr;
        tm local;
        time(&curr);                                            // get current time_t value
        local=*(localtime(&curr));                              // dereference and assign
        char fName[128];
        sprintf( fName, "%04d-%02d-%02d_logSQL.sql", local.tm_year+1900, local.tm_mon+1, local.tm_mday );

        std::fstream log_file ( fName, std::ios::app );

        if ( !log_file.is_open() )
        {
            // The file could not be opened
            sLog.outError("SQL-Logging is disabled - Log file for the SQL commands could not be openend: %s",fName);
        }
        else
        {
            // Safely use the file stream
            log_file << szQuery << "\n";
            log_file.close();
        }
    }*/

    return Execute(szQuery);
}

Database* CreateDatabaseInterface(DatabaseType type)
{
    switch(type)
    {

#ifdef DATABASE_SUPPORT_MYSQL

    case DATABASE_TYPE_MYSQL:
        return new MySQLDatabase;
        break;

#endif

#ifdef DATABASE_SUPPORT_PGSQL

    case DATABASE_TYPE_PGSQL:
        return new PostgreDatabase;
        break;

#endif

    }
    sLog.outError("Invalid database type specified. It has to be in the range of 1 to 3.");
    return 0;
}

void DestroyDatabaseInterface(Database * ptr)
{
    switch(ptr->GetType())
    {

#ifdef DATABASE_SUPPORT_MYSQL

    case DATABASE_TYPE_MYSQL:
        delete ((MySQLDatabase*)ptr);
        return;
        break;

#endif

#ifdef DATABASE_SUPPORT_PGSQL

    case DATABASE_TYPE_PGSQL:
        delete ((PostgreDatabase*)ptr);
        return;
        break;

#endif

    }
    sLog.outError("Invalid database type specified. It has to be in the range of 1 to 3.");
}


QueryResult::QueryResult(uint32 FieldCount, uint32 RowCount, uint32 Type)
{
    mCurrentRow = new Field[FieldCount];
    mRowCount = RowCount;
    mFieldCount = FieldCount;
    mType = Type;
}

QueryResult::~QueryResult()
{
    delete [] mCurrentRow;
    switch(mType)
    {

#ifdef DATABASE_SUPPORT_MYSQL

    case DATABASE_TYPE_MYSQL:
        ((MySQLQueryResult*)this)->Destroy();
        break;

#endif

#ifdef DATABASE_SUPPORT_PGSQL

    case DATABASE_TYPE_PGSQL:
        ((PostgreQueryResult*)this)->Destroy();
        break;

#endif

    default:
        assert(false);
        break;
    }
}
