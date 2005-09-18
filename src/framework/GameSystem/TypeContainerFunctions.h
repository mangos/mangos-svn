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

template<class OBJECT_TYPES> struct ContainerList
{
};

template<> struct ContainerList<TypeNull> {}; /* nothing is in type null */
template<class H, class T> struct ContainerList<TypeList<H, T> >
{
  std::map<OBJECT_HANDLE, H*> _elements;
  ContainerList<T> _TailElements;
};
  

namespace MaNGOS
{
  // non-const find functions
  template<class SPECIFIC_TYPE, class OBJECT_TYPES> SPECIFIC_TYPE* Find(ContainerList<OBJECT_TYPES> &elements, OBJECT_HANDLE hdl)
  {
    return NULL;
  };

  template<class SPECIFIC_TYPE, class T> SPECIFIC_TYPE* Find(ContainerList<TypeList<SPECIFIC_TYPE, T> > &elements, OBJECT_HANDLE hdl)
  {
    typename SPECIFIC_TYPE::iterator iter = elements._elements.find(hdl);
    return (hdl == elements._elements.end() ? NULL : iter->second);
  }
  
  template<class SPECIFIC_TYPE, class H, class T> SPECIFIC_TYPE* Find(ContainerList<TypeList<H, T> >&elements, OBJECT_HANDLE hdl)
  {
    return Find<T>(elements.TailElement, hdl);
  }

  // const find functions
  template<class SPECIFIC_TYPE, class OBJECT_TYPES> SPECIFIC_TYPE* Find(const ContainerList<OBJECT_TYPES> &elements, OBJECT_HANDLE hdl)
  {
    return NULL;
  };

  template<class SPECIFIC_TYPE, class T> const SPECIFIC_TYPE* Find(const ContainerList<TypeList<SPECIFIC_TYPE, T> > &elements, OBJECT_HANDLE hdl)
  {
    typename SPECIFIC_TYPE::const_iterator iter = elements._elements.find(hdl);
    return (hdl == elements._elements.end() ? NULL : iter->second);
  }
  
  template<class SPECIFIC_TYPE, class H, class T> SPECIFIC_TYPE* Find(const ContainerList<TypeList<H, T> >&elements, OBJECT_HANDLE hdl)
  {
    return Find<T>(elements.TailElement, hdl);
  }

  // non-const insert functions
  template<class SPECIFIC_TYPE, class OBJECT_TYPES> bool Insert(ContainerList<OBJECT_TYPES> &elements, SPECIFIC_TYPE *obj, OBJECT_HANDLE hdl)
  {
    return false;
    // should be a compile time error
  };
  
  // Bingo.. we have a match
  template<class SPECIFIC_TYPE, class T> bool Insert(ContainerList<TypeList<SPECIFIC_TYPE, T> > &elements, SPECIFIC_TYPE *obj, OBJECT_HANDLE hdl)
  {
    typename SPECIFIC_TYPE::iterator iter = elements._elements.find(hdl);
    if( iter != elements._elements.end() )
      return false;

    elements._elements[hdl] = obj;
    return true;
  }
  
  // Recursion
  template<class SPECIFIC_TYPE, class H, class T> bool Insert(ContainerList<TypeList<H, T> >&elements, SPECIFIC_TYPE *obj, OBJECT_HANDLE hdl)
  {
    return Insert<T>(elements.TailElement, obj, hdl);
  }

  // non-const remove method
  template<class SPECIFIC_TYPE, class OBJECT_TYPES> bool Remove(ContainerList<OBJECT_TYPES> &, OBJECT_HANDLE hdl)
  {
  }

  template<class SPECIFIC_TYPE, class T> SPECIFIC_TYPE* Remove(ContainerList<TypeList<SPECIFIC_TYPE, T> > &elements, OBJECT_HANDLE hdl)
  {
    typename SPECIFIC_TYPE::iterater iter = elements._elements.find(hdl);
    return (iter == elements._elements.end() ? NULL : iter->second);
  }


  template<class SPECIFIC_TYPE, class T, class H> SPECIFIC_TYPE* Remove(ContainerList<TypeList<H, T> > &elements, OBJECT_HANDLE hdl)
  {
    return Remove(elements.TailElements, hdl);
  }
}

#endif
