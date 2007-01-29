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

#ifndef DBCFILE_H
#define DBCFILE_H
#include "Platform/Define.h"
#include <cassert>

enum
{
    FT_NA='x',                                              //not used or unknown
    FT_STRING='s',                                          //char*
    FT_FLOAT='f',                                           //float
    FT_INT='i',                                             //uint32
    FT_SORT='d',                                            //sorted by this field, field is not included
    FT_IND='n'                                              //the same,but parsed to data
};

class DBCFile
{
    public:
        DBCFile();
        ~DBCFile();

        bool Load(const char *filename);

        class Record
        {
            public:
                float getFloat(size_t field) const
                {
                    assert(field < file.fieldCount);
                    return *reinterpret_cast<float*>(offset+field*4);
                }
                uint32 getUInt(size_t field) const
                {
                    assert(field < file.fieldCount);
                    return *reinterpret_cast<uint32*>(offset+field*4);
                }

                const char *getString(size_t field) const
                {
                    assert(field < file.fieldCount);
                    size_t stringOffset = getUInt(field);
                    assert(stringOffset < file.stringSize);
                    return reinterpret_cast<char*>(file.stringTable + stringOffset);
                }

            private:
                Record(DBCFile &file, unsigned char *offset): offset(offset), file(file) {}
                unsigned char *offset;
                DBCFile &file;

                friend class DBCFile;

        };

        // Get record by id
        Record getRecord(size_t id);
        /// Get begin iterator over records

        uint32 GetNumRows() const { return recordCount;}
        uint32 GetCols() const { return fieldCount; }
        bool IsLoaded() {return (data!=NULL);}
        void *AutoProduceData(const char*, uint32 *);
        static uint32 GetFormatRecordSize(const char * format, int32 * index_pos = NULL);
    private:

        uint32 recordSize;
        uint32 recordCount;
        uint32 fieldCount;
        uint32 stringSize;
        unsigned char *data;
        unsigned char *stringTable;
};
#endif
