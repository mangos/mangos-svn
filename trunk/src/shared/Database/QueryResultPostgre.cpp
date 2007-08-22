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

#include "DatabaseEnv.h"

QueryResultPostgre::QueryResultPostgre(PGresult *result, uint64 rowCount, uint32 fieldCount) :
QueryResult(rowCount, fieldCount), mResult(result)
{

    mCurrentRow = new Field[mFieldCount];
    ASSERT(mCurrentRow);

    for (uint32 i = 0; i < mFieldCount; i++)
    {
        mCurrentRow[i].SetName(PQfname(result, i));
        //mCurrentRow[i].SetType(ConvertNativeType(PQftype( result, i )));
        mCurrentRow[i].SetType(Field::DB_TYPE_UNKNOWN);
    }
}

QueryResultPostgre::~QueryResultPostgre()
{
    EndQuery();
}

bool QueryResultPostgre::NextRow()
{
    if (!mResult)
        return false;

    if (mTableIndex >= mRowCount)
    {
        EndQuery();
        return false;
    }

    for (int j = 0; j < mFieldCount; j++)
    {
        mCurrentRow[j].SetValue(PQgetvalue(mResult, mTableIndex, j));
    }

    return true;
}

void QueryResultPostgre::EndQuery()
{
    if (mCurrentRow)
    {
        delete [] mCurrentRow;
        mCurrentRow = 0;
    }

    if (mResult)
    {
        PQclear(mResult);
        mResult = 0;
    }
}

// dummy -> all is set to "uknown"
enum Field::DataTypes QueryResultPostgre::ConvertNativeType(Oid  pOid ) const
{
    switch (pOid)
    {

        // temporarry commented out until proper resolution found
        //        case FIELD_TYPE_DATE:
        //        case FIELD_TYPE_TIME:
        //        case FIELD_TYPE_DATETIME:
        //        case FIELD_TYPE_YEAR:
        //        case FIELD_TYPE_STRING:
        //        case FIELD_TYPE_VAR_STRING:
        //        case FIELD_TYPE_BLOB:
        //        case FIELD_TYPE_SET:
        //        case FIELD_TYPE_NULL:
        //            return Field::DB_TYPE_STRING;
        //        case FIELD_TYPE_TINY:

        //        case FIELD_TYPE_SHORT:
        //        case FIELD_TYPE_LONG:
        //        case FIELD_TYPE_INT24:
        //        case FIELD_TYPE_LONGLONG:
        //        case FIELD_TYPE_ENUM:
        //           return Field::DB_TYPE_INTEGER;
        //        case FIELD_TYPE_DECIMAL:
        //        case FIELD_TYPE_FLOAT:
        //        case FIELD_TYPE_DOUBLE:
        //            return Field::DB_TYPE_FLOAT;
        default:
            return Field::DB_TYPE_UNKNOWN;
    }
}
#endif
