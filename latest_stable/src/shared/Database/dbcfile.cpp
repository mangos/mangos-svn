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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbcfile.h"

DBCFile::DBCFile()
{
    data = NULL;
    fieldsOffset = NULL;
}

bool DBCFile::Load(const char *filename, const char *fmt)
{

    uint32 header;
    if(data)
    {
        delete [] data;
        data=NULL;
    }
    FILE * f=fopen(filename,"rb");
    if(!f)return false;

    fread(&header,4,1,f);                                   // Number of records
    if(header!=0x43424457)
    {
        //printf("not dbc file");
        return false;                                       //'WDBC'
    }
    fread(&recordCount,4,1,f);                              // Number of records
    fread(&fieldCount,4,1,f);                               // Number of fields
    fread(&recordSize,4,1,f);                               // Size of a record
    fread(&stringSize,4,1,f);                               // String size

    fieldsOffset = new uint32[fieldCount];
    fieldsOffset[0] = 0;
    for(uint32 i = 1; i < fieldCount; i++)
    {
        fieldsOffset[i] = fieldsOffset[i - 1];
        if (fmt[i - 1] == 'b' || fmt[i - 1] == 'X')         // byte fields
            fieldsOffset[i] += 1;
        else                                                // 4 byte fields (int32/float/strings)
            fieldsOffset[i] += 4;
    }

    data = new unsigned char[recordSize*recordCount+stringSize];
    stringTable = data + recordSize*recordCount;
    fread(data,recordSize*recordCount+stringSize,1,f);
    fclose(f);
    return true;
}

DBCFile::~DBCFile()
{
    if(data)
        delete [] data;
    if(fieldsOffset)
        delete [] fieldsOffset;
}

DBCFile::Record DBCFile::getRecord(size_t id)
{
    assert(data);
    return Record(*this, data + id*recordSize);
}

uint32 DBCFile::GetFormatRecordSize(const char * format,int32* index_pos)
{
    uint32 recordsize = 0;
    int32 i = -1;
    for(uint32 x=0; format[x];++x)
        switch(format[x])
        {
            case FT_FLOAT:
            case FT_INT:
                recordsize+=4;
                break;
            case FT_STRING:
                recordsize+=sizeof(char*);
                break;
            case FT_SORT:
                i=x;
                break;
            case FT_IND:
                i=x;
                recordsize+=4;
                break;
            case FT_BYTE:
                recordsize += 1;
                break;
        }

    if(index_pos)
        *index_pos = i;

    return recordsize;
}

void * DBCFile::AutoProduceData(const char * format, uint32 * records, char *&_data)
{
    /*
    format STRING, NA, FLOAT,NA,INT <=>
    struct{
    char* field0,
    float field1,
    int field2
    }entry;

    this func will generate  entry[rows] data;
    */

    typedef char * ptr;
    //char * _data;
    ptr* table;
    uint32 offset=0;

    if(strlen(format)!=fieldCount)
        return NULL;

    //get struct size and index pos
    int32 i;
    uint32 recordsize=GetFormatRecordSize(format,&i);

    if(i>=0)
    {
        uint32 maxi=0;
        //find max index
        for(uint32 y=0;y<recordCount;y++)
        {
            uint32 ind=getRecord(y).getUInt (i);
            if(ind>maxi)maxi=ind;
        }

        maxi++;
        *records=maxi;
        table=new ptr[maxi];
        memset(table,0,maxi*sizeof(ptr));
    }else
    {
        *records = recordCount;
        table = new ptr [recordCount];
    }

    _data= new char[recordCount*recordsize + stringSize];
    char *stringData = _data+recordCount*recordsize;
    memcpy(stringData,stringTable,stringSize);

    for(uint32 y =0;y<recordCount;y++)
    {
        //offset=0;
        if(i>=0)
        {
            table[getRecord(y).getUInt (i)]=&_data[offset];
        }else table[y]=&_data[offset];

        for(uint32 x=0;x<fieldCount;x++)
            switch(format[x])
            {
                case FT_FLOAT:
                    *((float*)(&_data[offset]))=getRecord(y).getFloat (x);
                    offset+=4;
                    break;
                case FT_IND:
                case FT_INT:
                    *((uint32*)(&_data[offset]))=getRecord(y).getUInt (x);
                    offset+=4;
                    break;
                case FT_BYTE:
                    *((uint8*)(&_data[offset]))=getRecord(y).getUInt8 (x);
                    offset+=1;
                    break;
                case FT_STRING:
                    const char * st = getRecord(y).getString(x);
                    *((char**)(&_data[offset]))=stringData+(st-(const char*)stringTable);
                    offset+=sizeof(char*);
                    break;
            }
    }

    return table;
}
