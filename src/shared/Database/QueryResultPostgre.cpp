/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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
QueryResult(rowCount, fieldCount), mResult(result),  mTableIndex(0)
{

    mCurrentRow = new Field[mFieldCount];
    ASSERT(mCurrentRow);

    for (uint32 i = 0; i < mFieldCount; i++)
    {
        mCurrentRow[i].SetName(PQfname(result, i));
        mCurrentRow[i].SetType(ConvertNativeType(PQftype( result, i )));
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

    char* pPQgetvalue;
    for (int j = 0; j < mFieldCount; j++)
    {
        pPQgetvalue = PQgetvalue(mResult, mTableIndex, j);
        if(pPQgetvalue && !(*pPQgetvalue))
            pPQgetvalue = NULL;

        mCurrentRow[j].SetValue(pPQgetvalue);
    }
    ++mTableIndex;

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

    /// TODO: need Fix This!!!
    /*
    switch (pOid)
    {
        // see types in #include <postgre/pg_type.h>
        case BOOLOID:
        case NUMERICOID:
        case INT8OID:
        case INT4OID:
        case INT2OID:
        case OIDOID:
        case TEXTOID:
        case BYTEAOID:
        case CHAROID:
        case NAMEOID:
        case CSTRINGARRAYOID:
        case BPCHAROID:
        case VARCHAROID:
        case FLOAT4OID:
        case FLOAT8OID:
        case DATEOID:
        case TIMEOID:
        case TIMESTAMPOID:
        case TIMESTAMPTZOID:
        case UNKNOWNOID:
        default:
            return Field::DB_TYPE_UNKNOWN;

    }
    */
    return Field::DB_TYPE_UNKNOWN;
}
#endif
