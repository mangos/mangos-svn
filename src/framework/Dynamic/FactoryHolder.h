/* FactoryHolder.h
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

#ifndef MANGOS_FACTORY_HOLDER
#define MANGOS_FACTORY_HOLDER

#include "Utiltities/Type.h"

/** FactoryHolder holds a factory object of a specific type
 */
template<class T, class INPUT_DATA = TypeNull >
class MANGOS_DLL_DECL FactoryHolder 
{
public:
    FactoryHolder(const char*, Factory);
    inline const char* name(void) const { return i_name; }

    T* Create(const INPUT_DATA &data) { return (new T(data)); }
};

// Specialization
template<class T> T*
FatoryHolder<T, TypeNull>::Create(const TypeNull &)
{
    return (new T);
}


#endif
