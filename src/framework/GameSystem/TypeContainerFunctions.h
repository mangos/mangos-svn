/* TypeContainerFunctions.h
 *
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

#ifndef TYPECONTAINER_FUNCTIONS_H
#define TYPECONTAINER_FUNCTIONS_H

/*
 * Here you'll find a list of helper functions to make
 * the TypeContainer usefull.  Without it, its hard
 * to access or mutate the container.
 */

#include "Define.h"
#include "TypeList.h"
#include <map>

namespace MaNGOS
{
    /* ContainerMapList Helpers */
    // non-const find functions
    template<class SPECIFIC_TYPE> SPECIFIC_TYPE* Find(ContainerMapList<SPECIFIC_TYPE> &elements, OBJECT_HANDLE hdl)
    {
	typename SPECIFIC_TYPE::iterator iter = elements._element.find(hdl);
	return (hdl == elements._element.end() ? NULL : iter->second);
    };
    
    template<class SPECIFIC_TYPE> SPECIFIC_TYPE* Find(ContainerMapList<TypeNull> &elements, OBJECT_HANDLE hdl)
    {
	return NULL; // terminate recursion
    }
  
    template<class SPECIFIC_TYPE, class T> SPECIFIC_TYPE* Find(ContainerMapList<T> &elements, OBJECT_HANDLE hdl)
    {
	return NULL; // wrong container
    }

    template<class SPECIFIC_TYPE, class H, class T> SPECIFIC_TYPE* Find(ContainerMapList<TypeList<H, T> >&elements, OBJECT_HANDLE hdl)
    {
	if( !Find(elements._elements, hdl) )
	    return Find(elements.TailElement, hdl);
    }
    
    // const find functions
    template<class SPECIFIC_TYPE> const SPECIFIC_TYPE* Find(const ContainerMapList<SPECIFIC_TYPE> &elements, OBJECT_HANDLE hdl)
    {
	typename SPECIFIC_TYPE::iterator iter = elements._element.find(hdl);
	return (hdl == elements._element.end() ? NULL : iter->second);
    };

    template<class SPECIFIC_TYPE> const SPECIFIC_TYPE* Find(const ContainerMapList<TypeNull> &elements, OBJECT_HANDLE hdl)
    {
	return NULL;
    }

    template<class SPECIFIC_TYPE, class T> const SPECIFIC_TYPE* Find(const ContainerMapList<T> &elements, OBJECT_HANDLE hdl)
    {
	return NULL;
    }


    template<class SPECIFIC_TYPE, class H, class T> SPECIFIC_TYPE* Find(const ContainerMapList<TypeList<H, T> >&elements, OBJECT_HANDLE hdl)
    {
	if( !Find(elements._elements, hdl) )
	    return Find(elements._TailElement, hdl);
    }

    // non-const insert functions
    template<class SPECIFIC_TYPE> bool Insert(ContainerMapList<SPECIFIC_TYPE> &elements, SPECIFIC_TYPE *obj, OBJECT_HANDLE hdl)
    {
	elements._element[hdl] = obj;
	return true;
    };
    
    template<class SPECIFIC_TYPE> bool Insert(ContainerMapList<TypeNull> &elements, SPECIFIC_TYPE *obj, OBJECT_HANDLE hdl)
    {
	return false;
    }

    template<class SPECIFIC_TYPE, class T> bool Insert(ContainerMapList<T> &elements, SPECIFIC_TYPE *obj, OBJECT_HANDLE hdl)
    {
	return false; // wrong type...
    }
  
    // Recursion
    template<class SPECIFIC_TYPE, class H, class T> bool Insert(ContainerMapList<TypeList<H, T> >&elements, SPECIFIC_TYPE *obj, OBJECT_HANDLE hdl)
    {
	if( !Insert(elements._elements, obj, hdl) )
	    return Insert(elements._TailElement, obj, hdl);
    }

    // non-const remove method
    template<class SPECIFIC_TYPE> bool Remove(ContainerMapList<SPECIFIC_TYPE> &elements, OBJECT_HANDLE hdl)
    {
	typename std::map<OBJECT_HANDLE, SPECIFIC_TYPE *>::iterator iter = elements._element.find(hdl);
	if( iter != elements._element.end() )
	    elements._element.erase(iter);
	return true; // found... terminate the search
    }
    
    template<class SPECIFIC_TYPE> bool Remove(ContainerMapList<TypeNull> &elements, OBJECT_HANDLE hdl)
    {
	return false;
    }

    template<class SPECIFIC_TYPE, class T> bool Remove(ContainerMapList<T> &elements, OBJECT_HANDLE hdl)
    {
	return false; // bad hit
    }

    template<class SPECIFIC_TYPE, class T, class H> SPECIFIC_TYPE* Remove(ContainerMapList<TypeList<H, T> > &elements, OBJECT_HANDLE hdl)
    {
	if( !Remove(elements._elements, hdl) )
	    return Remove(elements._TailElements, hdl);
    }

}

#endif
