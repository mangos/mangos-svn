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

/**
  @class GridLoader
  The GridLoader is working in conjuction with the Grid and responsible
  for loading and unloading object-types (one or more) when objects
  enters a grid.  Unloading is scheduled and might be canceled if
  an interested object re-enters.  GridLoader does not do the actuall
  loading and unloading but implements as a template pattern that
  delicate its loading and unloading for the actualy loader and unloader.
  GridLoader manages the grid (both local and remote).
 */

#include "Platform/Define.h"
#include "Grid.h"
#include "TypeContainerVisitor.h"

template
<
    class OBJECT,
    class OBJECT_TYPES,
    class LOADER_TYPE,
    class UNLOADER_TYPE
>
class MANGOS_DLL_DECL GridLoader
{
public:
    
    /** Loads the grid
     */
    void Load(Grid<OBJECT, OBJECT_TYPES> &grid)
    {
	grid.LockGrid();
	LOADER_TYPE loader(grid);
	TypeContainerVisitor<LOADER_TYPE, TypeMapContainer<OBJECT_TYPES> > visitor(loader);
	visitor.Visit(grid.i_container);
	grid.UnlockGrid();
    }
    
    /** Unloads the grid
     */
    void Unload(Grid<OBJECT, OBJECT_TYPES> &grid)
    {
	grid.LockGrid();
	UNLOADER_TYPE unloader(grid);
	TypeContainerVisitor<UNLOADER_TYPE, TypeMapContainer<OBJECT_TYPES> > visitor(unloader);
	visitor.Visit(grid.i_container);
	grid.UnlockGrid();
    }
};



#endif
