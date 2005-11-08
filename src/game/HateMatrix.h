/* HateMatrix.h
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

#ifndef MANGOS_HATEMATRIX_H
#define MANGOS_HATEMATRIX_H

/** HateMatrix is a storage of hate values used by CreatureAI.
 */

#include "Utilities/HashMap.h"

#ifdef ENABLE_GRID_SYSTEM
#include "MapManager.h"
#include "ObjectAccessor.h"
#endif

// UQ1: warning C4005: 'assert' : macro redefinition
#pragma warning(disable:4005)

#define assert(exp) ((void)0)

// forward declaration
class Unit;

struct MANGOS_DLL_DECL HateMatrix
{
    typedef hash_map<Unit *, uint32> HateMatrixMapType;
    
    // accessor
    inline uint32 operator[](Unit *unit) const
    {
	HateMatrixMapType::const_iterator iter = i_hateValues.find(unit);
	return (iter == i_hateValues.end() ? 0 : iter->second);
    }
    
    // force the unit's hate values to exist in the matrix
    inline uint32& operator[](Unit *unit)
    {
	HateMatrixMapType::iterator iter = i_hateValues.find(unit);
	if( iter == i_hateValues.end() )
	{
	    std::pair<HateMatrixMapType::iterator, bool> p = i_hateValues.insert( HateMatrixMapType::value_type(unit, 0) );
	    assert(p.second);
	    iter = p.first;
	}
	
	return iter->second;
    }
    
    
    // clear all hate values
    inline void ClearMatrix(void) { i_hateValues.clear(); }
    
    // remove a unit's hate value
    inline void RemoveValue(Unit *unit)
    {
	HateMatrixMapType::iterator iter = i_hateValues.find(unit);
	if( iter != i_hateValues.end() )
	    i_hateValues.erase( iter );
    }
    
    inline void AddValue(Unit *unit, uint32 val)
    {
	(*this)[unit] += val;
    }
    
private:
    HateMatrixMapType i_hateValues;
};

/** HateBinder binds a creature to the hater..this avoids searching.
 * The trick of rebinding uses the placement new.
 */
struct HateBinder
{
    static uint32 si_noHateValue;
    uint32 &i_hateValue;
    Unit *i_unit;
    HateBinder(uint32 &val, Unit *u) : i_hateValue(val), i_unit(u) {}
    HateBinder() : i_hateValue(si_noHateValue), i_unit(NULL) {}
    HateBinder(const HateBinder &obj) : i_hateValue(obj.i_hateValue), i_unit(obj.i_unit) {}

    HateBinder& operator=(const HateBinder &obj)
    {
	this->~HateBinder();
	new (this) HateBinder(obj);
	return *this;
    }
};

#endif
