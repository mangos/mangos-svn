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

#ifndef MANGOS_TYPECONTAINER_H
#define MANGOS_TYPECONTAINER_H

/*
 * Here, you'll find a series of containers that allow you to hold multiple
 * types of object at the same time.
 */

#include <map>
#include <vector>
#include <zthread/CountedPtr.h>
#include "Platform/Define.h"
#include "Utilities/TypeList.h"

using ZThread::CountedPtr;
class Corpse;

template<typename T>
CountedPtr<T>&  NullPtr(T *fake)
{
    static CountedPtr<T> _p;
    _p.reset();
    return _p;
};

/*
 * @class ContainerMapList is a mulit-type container for map elements
 * By itself its meaningless but collaborate along with TypeContainers,
 * it become the most powerfully container in the whole system.
 */
template<class OBJECT> struct ContainerMapList
{
    std::map<OBJECT_HANDLE, OBJECT *> _element;
};

template<> struct ContainerMapList<Corpse>
{
    std::map<OBJECT_HANDLE, CountedPtr<Corpse> > _element;
};

template<> struct ContainerMapList<TypeNull>                /* nothing is in type null */
{
};
template<class H, class T> struct ContainerMapList<TypeList<H, T> >
{
    ContainerMapList<H> _elements;
    ContainerMapList<T> _TailElements;
};

/*
 * @class ContaierArrayList is a multi-type container for
 * array of elements.
 */
template<class OBJECT> struct ContainerArrayList
{
    std::vector<OBJECT> _element;
};

// termination condition
template<> struct ContainerArrayList<TypeNull> {};
// recursion
template<class H, class T> struct ContainerArrayList<TypeList<H, T> >
{
    ContainerArrayList<H> _elements;
    ContainerArrayList<T> _TailElements;
};

/*
 * @class ContainerList is a simple list of different types of elements
 *
 */
template<class OBJECT> struct ContainerList
{
    OBJECT _element;
};

/* TypeNull is underfined */
template<> struct ContainerList<TypeNull> {};
template<class H, class T> struct ContainerList<TypeList<H, T> >
{
    ContainerList<H> _elements;
    ContainerMapList<T> _TailElements;
};

#include "TypeContainerFunctions.h"
#include "TypeContainerFunctionsPtr.h"

/*
 * @class TypeMapContainer contains a fixed number of types and is
 * determined at compile time.  This is probably the most complicated
 * class and do its simplest thing, that is, holds objects
 * of different types.
 */

template<class OBJECT_TYPES>
class MANGOS_DLL_DECL TypeMapContainer
{
    public:
        template<class SPECIFIC_TYPE> size_t Count() const { return MaNGOS::Count(i_elements, (SPECIFIC_TYPE*)NULL); }

        template<class SPECIFIC_TYPE> SPECIFIC_TYPE* find(OBJECT_HANDLE hdl, SPECIFIC_TYPE *fake) { return MaNGOS::Find(i_elements, hdl,fake); }
        template<class SPECIFIC_TYPE> CountedPtr<SPECIFIC_TYPE>& find(OBJECT_HANDLE hdl) { return MaNGOS::Find(i_elements, hdl,(CountedPtr<SPECIFIC_TYPE>*)NULL); }

        /// find a specific type of object in the container
        template<class SPECIFIC_TYPE> const SPECIFIC_TYPE* find(OBJECT_HANDLE hdl, SPECIFIC_TYPE *fake) const { return MaNGOS::Find(i_elements, hdl,fake); }
        template<class SPECIFIC_TYPE> const CountedPtr<SPECIFIC_TYPE>& find(OBJECT_HANDLE hdl) const { return MaNGOS::Find(i_elements, hdl,(CountedPtr<SPECIFIC_TYPE>*)NULL); }

        /// inserts a specific object into the container
        template<class SPECIFIC_TYPE> bool insert(OBJECT_HANDLE hdl, SPECIFIC_TYPE *obj)
        {
            SPECIFIC_TYPE* t = MaNGOS::Insert(i_elements, obj, hdl);
            return (t != NULL);
        }

        template<class SPECIFIC_TYPE> bool insert(OBJECT_HANDLE hdl, CountedPtr<SPECIFIC_TYPE> &obj)
        {
            return  MaNGOS::Insert(i_elements, obj, hdl);
        }

        ///  Removes the object from the container, and returns the removed object
        template<class SPECIFIC_TYPE> bool remove(SPECIFIC_TYPE* obj, OBJECT_HANDLE hdl)
        {
            SPECIFIC_TYPE* t = MaNGOS::Remove(i_elements, obj, hdl);
            return (t != NULL);
        }

        template<class SPECIFIC_TYPE> bool remove(CountedPtr<SPECIFIC_TYPE>& obj, OBJECT_HANDLE hdl)
        {
            return MaNGOS::Remove(i_elements, obj, hdl);
        }

        ContainerMapList<OBJECT_TYPES> & GetElements(void) { return i_elements; }
        const ContainerMapList<OBJECT_TYPES> & GetElements(void) const { return i_elements;}

    private:
        ContainerMapList<OBJECT_TYPES> i_elements;
};
#endif
