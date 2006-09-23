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

#ifndef __DATABASEHANDLER_H
#define __DATABASEHANDLER_H

//#include "Common.h"

#include <ace/Singleton.h>

#include <mysql++/mysql++.h>
#include <iostream>
#include <vector>

enum eDatabaseType
{
	DATABASE_LOGIN = 0,
	DATABASE_WORLD,
	DATABASE_REALM
};

class DatabaseHandler
{
    public:
		 DatabaseHandler();
		~DatabaseHandler();

		typedef std::map<uint32, mysqlpp::Connection*> DatabaseConMap;

		int AddDatabase(const char *infoString, uint32 dType);

		int PQuery(uint32 dbType, std::string szQuery, mysqlpp::Result &szRes);
		int PExecute(uint32 dbType, std::string szQuery);
		
		ACE_INLINE
		mysqlpp::Connection* GetDatabase(uint32 dType)
		{
			return m_databases[dType];
		};

		ACE_INLINE
		const char* GetDatabaseError(uint32 dType)
		{
			return m_databases[dType]->error();
		};
		
		ACE_INLINE
		std::string GetQueryError(uint32 dType)
		{
			return m_databases[dType]->query().error();
		};
		
		ACE_INLINE
		mysqlpp::Query GetQuery(uint32 dType)
		{
			return m_databases[dType]->query();
		};
		
		

        /*virtual ~Database() {}

        virtual int Initialize(const char *infoString) = 0;

        virtual QueryResult* Query(const char *sql) = 0;
        virtual QueryResult* PQuery(const char *format,...) = 0;

        virtual bool Execute(const char *sql) = 0;
        virtual bool PExecute(const char *format,...) = 0;

        virtual operator bool () const = 0;*/
		
		ACE_INLINE
        const char *GetString(const char* value)
		{ 
			return value; 
		}
        std::string GetCppString(const char* value)
        {
            return std::string(value);
        }
        float GetFloat(const char* mValue)
		{ 
			return mValue ? ACE_static_cast(float, atof(mValue)) : 0;
		}
        bool GetBool(const char* mValue)
		{
			return mValue ? atoi(mValue) > 0 : false;
		}
        uint8 GetUInt8(const char* mValue)
		{
			return mValue ? ACE_static_cast(uint8, atol(mValue)) : 0;
		}
        uint16 GetUInt16(const char* mValue)
		{
			return mValue ? ACE_static_cast(uint16, atol(mValue)) : 0;
		}
        uint32 GetUInt32(const char* mValue)
		{
			return mValue ? ACE_static_cast(uint32, atol(mValue)) : 0;
		}
        uint64 GetUInt64(const char* mValue)
        {
            if(mValue)
            {
                uint64 value;
                sscanf(mValue,I64FMTD,&value);
                return value;
            }
            else
                return 0;
        }

	private:
		
		DatabaseConMap m_databases;
};

typedef ACE_Singleton<DatabaseHandler, ACE_Recursive_Thread_Mutex> DatabaseHandlerSingleton;
#define sDatabaseMysql DatabaseHandlerSingleton::instance()

#endif /* __DATABASEHANDLER_H */
