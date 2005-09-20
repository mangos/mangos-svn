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
template<typename GRID_TYPE, class OBJECT, class T> void LoadHelper(GRID_TYPE &grid, OBJECT *obj, T &loader)
{
  loader.load(grid, obj); // loads a specific objects into the grid specific container
}

// Terminate condition
template<typename GRID_TYPE, class OBJECT> void LoadHelper(GRID_TYPE &grid, OBJECT *obj, ContainerList<TypeNull>)
{
}

// Recursion
template<typename GRID_TYPE, class OBJECT, class H, class T> void LoadHelper(GRID_TYPE &grid, OBJECT *obj, ContainerList<TypeList<H, T> >&loaders)
{
  loaders._element.load(grid, obj);
  LoadHelper(grid, obj, loaders._TailElements);
}


// Unload helper functions
template<typename GRID_TYPE, class T> void UnloadHelper(GRID_TYPE &grid, T &loader)
{
  loader.Unload(grid); // loads a specific objects into the grid specific container
}

// termination condition
template<typename GRID_TYPE> void UnloadHelper(GRID_TYPE &grid, ContainerList<TypeNull> &)
{
}

// Recursion
template<typename GRID_TYPE, class H, class T> void UnloadHelper(GRID_TYPE &grid, TypeList<H, T> &loaders)
{
  loaders._element.Unload(grid);
  UnloadHelper<GRID_TYPE, T>(grid, loaders._TailElements);
}

template<class OBJECT, class OBJECT_TYPES, class LOADER_TYPE>
class MANGOS_DLL_DECL GridLoader
{
public:

  /// Loads the grid
  void Load(Grid<OBJECT, OBJECT_TYPES> &grid, OBJECT *obj)
  {
      LoadHelper<OBJECT, Grid<OBJECT, OBJECT_TYPES>, LOADER_TYPES>(grid, obj, i_loaders);
  }
  
  /// Unloads the grid
  void Unload(Grid<OBJECT, OBJECT_TYPES> &grid)
  {
    UnloadHelper<Grid<OBJECT, OBJECT_TYPES>, LOADER_TYPES>(grid, i_loaders);
  }

private:
  LOADER_TYPE i_loader;
};



#endif
