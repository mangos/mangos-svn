/* DataStore.h
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

#ifndef __DATASTORE_H
#define __DATASTORE_H

#include "Common.h"
#include "Singleton.h"
#include "DBC.h"

template <class T>
class DataStore : public Singleton<DataStore<T> >
{
    protected:
        DBC d;
    public:
        DataStore(const char* filename)
        {
            d.Load(filename);
        }
        ~DataStore() {}

        virtual T *LookupEntry(const uint32 row)
        {
            if(!d.IsLoaded() || (uint32)d.GetRows() < row) return NULL;
            return (T*)d.GetRow(row);
        }
        const char* LookupString(const uint32 offset)
        {
            if(!d.IsLoaded() || (uint32)d.GetDBSize() < offset) return NULL;
            return d.LookupString(offset);
        }
        uint32 GetNumRows()
        {
            return d.GetRows();
        }
};

template <class T>
class IndexedDataStore : public DataStore<T>
{
    protected:
        std::map<uint32,uint32> indexMap;
    public:
        IndexedDataStore(const char* filename) : DataStore<T>(filename)
        {
            for(uint32 row=0;row<(uint32)DataStore<T>::d.GetRows();row++)
                indexMap[*(int*)DataStore<T>::d.GetRow(row)] = row;
        }
        ~IndexedDataStore() {}

        virtual T *LookupEntry(const uint32 row)
        {
            if(!DataStore<T>::d.IsLoaded()) return NULL;
            return (T*)DataStore<T>::d.GetRow(indexMap[row]);
        }
};

#define defineDBCStore(name,structname) \
class name : public DataStore<structname> \
{ \
    public: \
        name(const char* filename); \
        ~name(); \
    }

#define implementDBCStore(name,structname) \
initialiseSingleton(name); \
initialiseSingleton(DataStore< structname >); \
name::name(const char* filename) : DataStore<structname>(filename) {} \
name::~name() {} \

#define defineIndexedDBCStore(name,structname) \
class name : public IndexedDataStore<structname> \
{ \
    public: \
        name(const char* filename); \
        ~name(); \
    }

#define implementIndexedDBCStore(name,structname) \
initialiseSingleton(name); \
initialiseSingleton(DataStore< structname >); \
name::name(const char* filename) : IndexedDataStore<structname>(filename) {} \
name::~name() {}
#endif
