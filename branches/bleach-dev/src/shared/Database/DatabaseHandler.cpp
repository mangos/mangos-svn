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

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Singleton<DatabaseHandler, ACE_Recursive_Thread_Mutex>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Singleton<DatabaseHandler, ACE_Recursive_Thread_Mutex>
#elif defined (__GNUC__) && (defined (_AIX) || defined (__hpux))
template ACE_Singleton<DatabaseHandler, ACE_Recursive_Thread_Mutex> * ACE_Singleton<DatabaseHandler, ACE_Recursive_Thread_Mutex>::singleton_;
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

DatabaseHandler::DatabaseHandler()
{
}

DatabaseHandler::~DatabaseHandler()
{
	for( DatabaseConMap::iterator i = m_databases.begin(); i != m_databases.end(); i++ )
	{
		i->second->close();
        delete i->second;
	}

    m_databases.clear( );
}

int
DatabaseHandler::AddDatabase(const char *infoString, uint32 dType)
{
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

	mysqlpp::Connection* con = new mysqlpp::Connection(mysqlpp::use_exceptions);
	m_databases[dType] = con;
	con->connect(database.c_str(), host.c_str(), user.c_str(), password.c_str(), ACE_OS::atoi(port.c_str()));
	if (con) 
	{
		return 1;
	}
	return 0;
}

int
DatabaseHandler::PQuery(uint32 dbType, std::string szQuery, mysqlpp::Result &szRes)
{	
	try
	{
		mysqlpp::Query query = GetQuery(dbType);
		query << szQuery.c_str();
		ACE_DEBUG((LM_INFO , "%I-- %D - %M - SQL: %s --\n", query.preview().c_str() ));
		szRes = query.store();
		return 1;
	}
	catch (const mysqlpp::BadQuery& er /* Handle any query errors */)
	{
		ACE_DEBUG((LM_ALERT, ACE_TEXT("%I-- %D - %M - MySQL Error: %s --\n"), er.what() ));
        return 0;
    }
    catch (const mysqlpp::BadConversion& er /* Handle bad conversions */)
	{
		ACE_DEBUG((LM_ALERT, ACE_TEXT("%I-- %D - %M - MySQL Error: %s --\n"), er.what() ));
        return 0;
    }
    catch (const mysqlpp::Exception& er /* Catch all */)
	{
		ACE_DEBUG((LM_ALERT, ACE_TEXT("%I-- %D - %M - MySQL Error: %s --\n"), er.what() ));
        return 0;
    }
	return 0;
}

int
DatabaseHandler::PExecute(uint32 dbType, std::string szQuery)
{	
	try
	{
		mysqlpp::Query query = GetQuery(dbType);
		query << szQuery.c_str();
		ACE_DEBUG((LM_INFO , "%I-- %D - %M - SQL: %s --\n", query.preview().c_str() ));
		query.execute();
		return 1;
	}
	catch (const mysqlpp::BadQuery& er /* Handle any query errors */)
	{
		ACE_DEBUG((LM_ALERT, ACE_TEXT("%I-- %D - %M - MySQL Error: %s --\n"), er.what() ));
        return 0;
    }
    catch (const mysqlpp::BadConversion& er /* Handle bad conversions */)
	{
		ACE_DEBUG((LM_ALERT, ACE_TEXT("%I-- %D - %M - MySQL Error: %s --\n"), er.what() ));
        return 0;
    }
    catch (const mysqlpp::Exception& er /* Catch all */)
	{
		ACE_DEBUG((LM_ALERT, ACE_TEXT("%I-- %D - %M - MySQL Error: %s --\n"), er.what() ));
        return 0;
    }
	return 0;
}
