/* QueryResultSqlite.h
 *
 * Copyright (C) 2004 Wow Daemon
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

#if !defined(QUERYRESULTSQLITE_H)
#define QUERYRESULTSQLITE_H

// Required for SQLite DBM
#include <sqlite/sqlite.h>

class QueryResultSqlite : public QueryResult
{
    public:
        QueryResultSqlite(char **tableData, uint32 rowCount, uint32 fieldCount);

//! Frees resources used by QueryResult.
        ~QueryResultSqlite();

//! Selects the next row in the result of the current query.
/*
This will update any references to fields of the previous row, so use Field's copy constructor to keep a persistant field.
@return 1 if the next row was successfully selected, else 0.
*/
        bool NextRow();

    private:
        enum Field::DataTypes ConvertNativeType(const char* sqliteType) const;
        void EndQuery();

        char **mTableData;
        uint32 mTableIndex;
};
#endif
