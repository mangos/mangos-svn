/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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
    public:
        typedef ObjectRegistry<FactoryHolder<T> > FactoryHolderRegistry;
        typedef MaNGOS::Singleton<FactoryHolderRegistry > FactoryHolderRepository;

        FactoryHolder(const char*s) : i_name(s) {}
        inline const char* name(void) const { return i_name; }

        void RegisterSelf(void) { FactoryHolderRepository::Instance().InsertItem(this, i_name.c_str()); }
        void DeregisterSelf(void) { FactoryHolderRepository::Instance().RemoveItem(this, false); }

        /// Abstract Factory create method
        virtual T* Create(void *data = NULL) const = 0;
    private:
        std::string i_name;
};

/** Permissible is a classic way of letting the object decide
 * whether how good they handle things.  This is not retricted
 * to factory selectors.
 */
template<class T>
class Permissible
{
    public:
        virtual ~Permissible() {}
        virtual int Permit(const T *) const = 0;
};
#endif
