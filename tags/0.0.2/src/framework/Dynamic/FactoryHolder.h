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

#include "Platform/Define.h"
#include "Utilities/TypeList.h"
#include "ObjectRegistry.h"
#include "Policies/SingletonImp.h"

/** FactoryHolder holds a factory object of a specific type
 */
template<class T>
class MANGOS_DLL_DECL FactoryHolder 
{
    typedef MaNGOS::Singleton<FactoryHolder<T> > FactoryHolderRegistry;
public:
    FactoryHolder(const char*s) : i_name(s) {}
    inline const char* name(void) const { return i_name; }

    T* Create(void * data = NULL) { return (new T(data)); }
    void RegisterSelf(void) { FactoryHolderRegistry::Instance().InsertItem(i_name.c_str(), this); }
    void DeregisterSelf(void) { FactoryHolderRegistry::Instance().RemoveItem(this, false); }

private:
    std::string i_name;
};


#endif
