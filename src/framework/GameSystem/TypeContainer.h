/* TypeContainer.h
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

#ifndef MANGOS_TYPECONTAINER_H
#define MANGOS_TYPECONTAINER_H

/*
 * @class TypeContainer contains a fixed number of types and is 
 * determined at compile time.  This is probably the most complicated
 * class and do its simplest thing, that is, holds objects
 * of different types.
 */

#include <map>
#include "TypeList.h"
#include "TypeContainerFunctions.h"
  
template<class OBJECT_TYPES>
class MANGOS_DLL_DECL TypeContainer
{
public:

  template<class SPECIFIC_TYPE> SPECIFIC_TYPE* find(OBJECT_HANDLE hdl) { return MaNGOS::Find<SPECIFIC_TYPE, OBJECT_TYPES>(i_elements, hdl); }
  template<class SPECIFIC_TYPE> const SPECIFIC_TYPE* find(OBJECT_HANDLE hdl) const { return MaNGOS::Find<SPECIFIC_TYPE, OBJECT_TYPES>(i_elements, hdl); }
  template<class SPECIFIC_TYPE> bool insert(OBJECT_HANDLE hdl, SPECIFIC_TYPE *obj) 
  {
    return MaNGOS::Insert<SPECIFIC_TYPE, OBJECT_TYPES>(i_elements, hdl, obj);
  }
  
  template<class SPECIFIC_TYPE> SPECIFIC_TYPE* remove(OBJECT_HANDLE hdl)
  {
    return MaNGOS::Remove<SPECIFIC_TYPE, OBJECT_TYPES>(i_elements, hdl);
  }


private:
  ContainerList<OBJECT_TYPES> i_elements;
};


#endif
