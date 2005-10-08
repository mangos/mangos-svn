/* ObjectRegistry.h
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

#ifndef MANGOS_OBJECTREGISTRY_H
#define MANGOS_OBJECTREGISTRY_H

#include "Platform/Define.h"

/** ObjectRegistry holds all registry item of the same type
 */
template<class T>
class MANGOS_DLL_DECL ObjectRegistry
{
    typedef std::map<std::string, T *> RegistryMapType;
    RegistryMapType i_registeredObjects;


    // protected for friend use since it should be a singleton
    ObjectRegistry() {}
    ~ObjectRegistry() 
    {
	for(RegistryMapType::iterator iter=i_registeredObjects.begin(); iter != i_registeredObjects.end(); ++iter)
	    delete iter->second;
	i_registeredObjects.clear();
    }

public:

    /// Returns a registry item
    const T* GetRegistryItem(const char *name) const
    {
	for(RegistryMapType::const_iterator iter = i_registeredObjects.begin(); iter != i_registeredObjects.end(); ++iter)
	    if( iter->first == std::string(name) )
		return iter->second;
	return NULL;
    }

    /// Inserts a registry item
    bool InsertItem(T *obj, const char *name, bool override = false)
    {
	RegistryMapType::iterator iter = i_registeredObjects.find(name);
	if( iter != i_registeredObjects.end() )
	{
	    if( !override )
		return false;
	    delete iter->second;
	    i_registeredObjects.erase(iter);
	}

	i_registeredObjects[name] = obj;
	return true;
    }

    /// Removes a registry item
    void RemoveItem(const char *name, bool delete_object = true)
    {
	RegistryMapType::iterator iter = i_registeredObjects.find(name);
	if( iter != i_registeredObjects.end() )
	{
	    if( delete_object )
		delete iter->second;
	    i_registeredObjects.erase(iter);
	}
    }

    /// Returns true if registry contains an item
    bool HasItem(const char *name) const
    {
	return (i_registeredObjects.find(name) != i_registeredObjects.end());
    }

    /// Return a list of registered items
    unsinged int GetRegisteredItems(std::vector<std::string> &l) const
    {
	unsigned int sz = l.size();
	l.resize(sz + i_registeredObjects.size());
	for(RegistryMapType::const_iterator iter = i_registeredObjects.begin(); iter != i_registeredObjects.end(); ++iter)
	    l[sz] = iter->frist;
    }
};

#endif
