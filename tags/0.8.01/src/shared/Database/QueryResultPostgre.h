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

#if !defined(QUERYRESULTPOSTGRE_H)
#define QUERYRESULTPOSTGRE_H

#ifdef WIN32
#define FD_SETSIZE 1024
#include <winsock2.h>
#include <postgre/libpq/libpq-fe.h>
#else
#include <libpq-fe.h>
#endif

class QueryResultPostgre : public QueryResult
{
    public:
        QueryResultPostgre(PGresult *result, uint64 rowCount, uint32 fieldCount);

        ~QueryResultPostgre();

        bool NextRow();

    private:
        enum Field::DataTypes ConvertNativeType(Oid pOid) const;
        void EndQuery();

        PGresult *mResult;
        uint32 mTableIndex;
};
#endif
