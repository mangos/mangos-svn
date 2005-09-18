/* GridLoader.h
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

#ifndef MANGOS_GRIDLOADER_H
#define MANGOS_GRIDLOADER_H

/*
  @class GridLoader
  The GridLoader is working in conjuction with the Grid and responsible
  for loading and unloading object-types (one or more) when objects
  enters a grid.  Unloading is scheduled and might be canceled if
  an interested object re-enters.
 */

#include "Define.h"
#include "Grid.h"

// load helper functions
template<class OBJECT, typename GRID_TYPE, class T> LoadHelper<OBJECT, GRID_TYPE, T>(GRID_TYPE &grid, OBJECT *obj)
{
  T::Load(grid, obj); // loads a specific objects into the grid specific container
}

// termination condition
template<class OBJECT, typename GRID_TYPE> LoadHelper<OBJECT, GRID_TYPE, TypeNull>(GRID_TYPE &grid, OBJECT *obj)
{
}

// Recursion
template<class OBJECT, typename GRID_TYPE, class H, class T> LoadHelper<OBJECT, GRID_TYPE, TypeList<H,T> >(GRID_TYPE &grid, OBJECT *obj)
{
  LoadHelper<OBJECT, GRID_TYPE, H>::Load(grid, obj);
  LoadHelper<OBJBECT, GRID_TYPE, T>::Load(grid, obj);
}


// Unload helper functions
template<typename GRID_TYPE, class T> UnloadHelper<OBJECT, GRID_TYPE, T>(GRID_TYPE &grid)
{
  T::Unload(grid); // loads a specific objects into the grid specific container
}

// termination condition
template<typename GRID_TYPE> UnloadHelper<OBJECT, GRID_TYPE, TypeNull>(GRID_TYPE &grid)
{
}

// Recursion
template<typename GRID_TYPE, class H, class T> UoadHelper<GRID_TYPE, TypeList<H,T> >(GRID_TYPE &grid)
{
  LoadHelper<GRID_TYPE, H>::Unload(grid, obj);
  LoadHelper<GRID_TYPE, T>::Unload(grid, obj);
}

template<class OBJECT, class OBJECT_TYPES, class LOADER_TYPES>
class MANGOS_DLL_DECL GridLoader
{
  typedef typename Grid<OBJECT, OBJECT_TYPES> GridType;
public:

  /// Loads the grid
  void Load(GridType &, OBJECT *obj)
  {
    LoadHelper<OBJECT, GridType, LOADER_TYPES>(grid, obj);
  }
  
  /// Unloads the grid
  void Unload(GridType &)
  {
    UnloaderHelper<GridType, LOADER_TYPES>(grid);
  }
};



#endif
