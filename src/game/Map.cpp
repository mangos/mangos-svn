/* Map.cpp
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

#include "Map.h"
#include "GridNotifiers.h"

Map::Map(uint32 id) : i_id(id), i_gridMask(0), i_gridStatus(0)
{
    for(unsigned int idx=0; idx < MAX_NUMBER_OF_GRIDS; ++idx)
	for(unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
	{
	    i_grids[idx][j] = NULL;
	    i_guards[idx][j] = NULL;
	}
}


void
Map::Add(Player *player)
{
    GridPair p = CalculateGrid(player->GetPositionX(), player->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < MAX_NUMBER_OF_GRIDS &&
	    p.y_coord >= 0 && p.y_coord < MAX_NUMBER_OF_GRIDS );
    
    uint64 mask = CalculateGridMask(p.x_coord, p.y_coord);

    if( !(i_gridMask & mask) )
    {
	Guard guard(*this);
	if( !(i_gridMask & mask) )
	{
	    i_grids[p.x_coord][p.y_coord] = new GridType(p.x_coord*MAX_NUMBER_OF_GRIDS + p.y_coord);
	    i_guards[p.x_coord][p.y_coord] = new GridRWLock();
	    i_gridMask &= mask;
	}	
    }

    GridType *grid = i_grids[p.x_coord][p.y_coord];
    assert(grid != NULL);
    if( !(i_gridStatus & mask) )
    {
	GridLockType &lock = i_guards[p.x_coord][p.y_coord]->getWriteLock();
	GridGuard guard(lock);
	if( !(i_gridStatus & mask) )
	{
	    sLog.outDebug("Player %s triggers of loading grid [%d,%d] on map %d", player->GetGUID(), p.x_coord, p.y_coord, i_id);
	    GridLoaderType loader;
	    ObjectGridLoader gloader(*grid, *player);
	    loader.Load(*grid, gloader);
	    grid->AddObject(player, player->GetGUID());
	    i_gridStatus &= mask;
	}
    }
    else
    {
	GridLockType &lock = i_guards[p.x_coord][p.y_coord]->getWriteLock();
	GridGuard guard(lock);
	grid->AddObject(player, player->GetGUID());

    }
   
    grid->SetGridStatus(GRID_STATUS_ACTIVE);
    // check to see if the grid status is loaded...
}

template<class T>
GridType*
Map::AddType(T *obj)
{
    GridPair p = CalculateGrid(obj->GetPositionX(), obj->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < MAX_NUMBER_OF_GRIDS &&
	    p.y_coord >= 0 && p.y_coord < MAX_NUMBER_OF_GRIDS );

    if( !loaded(p) )
	return NULL; // doesn't make sense to add a creature to a location where there's no one

    GridType *grid = i_grids[p.x_coord][p.y_coord];
    assert( grid != NULL );

    {
	GridLockType &lock = i_guards[p.x_coord][p.y_coord]->getWriteLock();
	GridGuard guard(lock);       
	(*grid).template AddGridObject<T>(obj, obj->GetGUID());
	grid->SetGridStatus(GRID_STATUS_ACTIVE);
    }
	
    // now update things.. only required a read lock
    
}

void
Map::Add(Creature *creature)
{
    GridType *grid = AddType<Creature>(creature);
    
}

void
Map::Add(DynamicObject *obj)
{
    GridType *grid = AddType<DynamicObject>(obj);
    
}

void
Map::Add(GameObject *obj)
{
    GridType *grid = AddType<GameObject>(obj);
    
}

void
Map::Add(Corpse *obj)
{
    GridType *grid = AddType<Corpse>(obj);
    
}

bool
Map::loaded(const GridPair &p) const
{
    uint64 mask = CalculateGridMask(p.x_coord, p.y_coord);
    return ( !(i_gridMask & mask) || !(i_gridStatus & mask) );
}

// update map
void
Map::Update(uint32 t_diff)
{
    uint64 mask = 1;
    for(unsigned int i=0; i < MAX_NUMBER_OF_GRIDS; ++i)
    {
	for(unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
	{
	    if( i_gridMask & mask )
	    {
		assert(i_grids[i][j] != NULL);
		GridType &grid(*i_grids[i][j]);
		if( grid.ObjectsInGrid() > 0 )
		{
		    MaNGOS::GridUpdater updater(grid, t_diff);
		    TypeContainerVisitor<MaNGOS::GridUpdater, ContainerMapList<Player> > player_notifier(updater);
		    grid.VisitObjects(player_notifier);
		    TypeContainerVisitor<MaNGOS::GridUpdater, TypeMapContainer<AllObjectTypes> > object_notifier(updater);
		    grid.VisitGridObjects(object_notifier);

		    // now update packets..
		}
		else
		{
		    if( grid.GetGridStatus() == GRID_STATUS_REMOVAL )
		    {
			// unloading the grid....
			sLog.outDebug("Unloading grid %d for map %d", grid.GetGridId(), i_id);
			ObjectGridUnloader unloader(grid);
			TypeContainerVisitor<ObjectGridUnloader, TypeMapContainer<AllObjectTypes> > object_unload(unloader);
			grid.VisitGridObjects(object_unload);
			i_gridMask &= ~mask;
		    }
		    else
		    {
			// schedule for removale
			grid.SetGridStatus(GRID_STATUS_REMOVAL);
		    }
		}
	    }
	}
    }
}

void
Map::Remove(Player *player, bool remove)
{   
    GridPair p = CalculateGrid(player->GetPositionX(), player->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < MAX_NUMBER_OF_GRIDS &&
	    p.y_coord >= 0 && p.y_coord < MAX_NUMBER_OF_GRIDS );
    
    uint64 mask = CalculateGridMask(p.x_coord, p.y_coord);

    if( !(i_gridMask & mask) )
    {
	assert( false );	
	return; // hmm...  how can we end up here
    }

    GridType *grid = i_grids[p.x_coord][p.y_coord];
    assert(grid != NULL);
    
    {  // do not remove the scope braces... fro performances
	GridLockType &lock = i_guards[p.x_coord][p.y_coord]->getWriteLock();
	GridGuard guard(lock);
	grid->RemoveObject(player, player->GetGUID());
    }
    
    {
	GridLockType &lock = i_guards[p.x_coord][p.y_coord]->getReadLock();
	MaNGOS::ExitNotifier notifier(*grid, *player);
	TypeContainerVisitor<MaNGOS::ExitNotifier, ContainerMapList<Player> > player_notifier(notifier);
	grid->VisitObjects(player_notifier);
    }

    if( remove )
	delete player;
}

template<class T>
GridType*
Map::RemoveType(T *obj, bool remove)
{
    GridPair p = CalculateGrid(obj->GetPositionX(), obj->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < MAX_NUMBER_OF_GRIDS &&
	    p.y_coord >= 0 && p.y_coord < MAX_NUMBER_OF_GRIDS );

    if( !loaded(p) )
	return NULL; // doesn't make sense to remove a creature in a location where its nothing

    GridType *grid = i_grids[p.x_coord][p.y_coord];
    assert( grid != NULL );

    {  // do not remove the scope braces... fro performances
	GridLockType &lock = i_guards[p.x_coord][p.y_coord]->getWriteLock();
	GridGuard guard(lock);
	(*grid).template RemoveGridObject<T>(obj, obj->GetGUID());
    }

    // now notify the players this creature is gone..
    {
	GridLockType &lock = i_guards[p.x_coord][p.y_coord]->getReadLock();
	GridGuard guard(lock);
	MaNGOS::InRangeRemover range_remover(*grid, obj);
	TypeContainerVisitor<MaNGOS::InRangeRemover, ContainerMapList<Player> > player_notifier(range_remover);
	grid->VisitObjects(player_notifier);
    }
    
    if( remove )
	delete obj;
}

void
Map::Remove(Corpse *obj, bool remove)
{
    RemoveType<Corpse>(obj, remove);
}

void
Map::Remove(GameObject *obj, bool remove)
{
    RemoveType<GameObject>(obj, remove);
}

void
Map::Remove(Creature *obj, bool remove)
{
    RemoveType<Creature>(obj, remove);
}

void
Map::Remove(DynamicObject *obj, bool remove)
{
    RemoveType<DynamicObject>(obj, remove);
}


