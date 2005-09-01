/* QueryResult.h
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

#if !defined(QUERYRESULT_H)
#define QUERYRESULT_H

class QueryResult
{
    public:
        QueryResult(uint64 rowCount, uint32 fieldCount) : mRowCount(rowCount),
            mFieldCount(fieldCount) {}

//! Frees resources used by QueryResult.
        virtual ~QueryResult() {}

//! Selects the next row in the result of the current query.
/*
This will update any references to fields of the previous row, so use Field's copy constructor to keep a persistant field.
@return 1 if the next row was successfully selected, else 0.
*/
        virtual bool NextRow() = 0;

//! Returns a pointer to an array of fields of the currently selected row.
/* Will be updated when NextRow() is called. */
        Field *Fetch() const { return mCurrentRow; }

//! Returns a single field reference.
/* Will be updated when NextRow() is called. */
        const Field & operator [] (int index) const { return mCurrentRow[index]; }

//! Returns the number of fields in the current row.
        int GetFieldCount() const { return mFieldCount; }

//! Returns the rows affected or the number of rows selected, depending on the last query.
        uint64 GetRowCount() const { return mRowCount; }

    protected:
        Field *mCurrentRow;
        uint32 mFieldCount;
        uint64 mRowCount;
};
#endif
