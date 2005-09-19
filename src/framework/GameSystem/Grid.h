/* Grid.h
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

#ifndef MANGOS_GRID_H
#define MANGOS_GRID_H

/*
  @class Grid
  Grid is a logical segment of the game world represented inside MaNGOS.
  Grid is bind at compile time to a particular type of object which
  we call it the object of interested.  There are many types of loader,
  specially, dynamic loader, static loader, or on-demand loader.  There's
  a subtle difference between dynamic loader and on-demand loader but
  this is implementation specific to the loader class.  From the
  Grid's perspective, the loader meets its API requirement is suffice.
*/

#include "Define.h"
#include "TypeContainer.h"
#include "Policies/ThreadingModel.h"

template
<
  class OBJECT,
  class OBJECT_TYPES,
  class ThreadModel = MaNGOS::SingleThreaded<OBJECT>
>
class MANGOS_DLL_DECL Grid
{
public:

  /// dtor
  ~Grid();

  /// Object enters the grid
  void AddObject(OBJECT *obj);

  /// Object move out of the grid
  void RemoveObject(OBJECT *obj);

  /// Refreshes the grid
  void RefreshGrid(void);

  /// Locks a grid.  Any object enters must wait until the grid is unlock
  void LockGrid(void);

  /// Unlocks the grid.
  void UnlockGrid(void);

  /// Returns the number of object within the grid.
  unsigned int ObjectsInGrid(void) const;


  /// Accessors: Returns a specific type of object in the OBJECT_TYPES
  template<class SPECIFIC_OBJECT> const SPECIFIC_OBJECT* GetObject(OBJECT_HANDLE hdl) const { return i_container.template find<SPECIFIC_OBJECT>(hdl); }
  template<class SPECIFIC_OBJECT> SPECIFIC_OBJECT* GetObject(OBJECT_HANDLE hdl) { return i_container.template find<SPECIFIC_OBJECT>(hdl); }

  // Mutators
  template<class SPECIFIC_OBJECT> bool AddObject(SPECIFIC_OBJECT *obj, OBJECT_HANDLE hdl) { return i_container.template insert<SPECIFIC_OBJECT>(obj, hdl); }

private:

  typedef typename ThreadModel::Lock Guard;
  typedef typename ThreadModel::VolatileType VolatileType;

  TypeMapContainer<OBJECT_TYPES> i_container;
  std::map<OBJECT_HANDLE, OBJECT *> i_objects;
};


#endif
