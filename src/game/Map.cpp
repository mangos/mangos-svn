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
#include "Player.h"
#include "WorldSession.h"
#include "Log.h"

// 3 minutes
#define DEFAULT_GRID_EXPIRY     300


Map::Map(uint32 id, time_t expiry) : i_id(id), i_gridExpiry(expiry)
{
    for(unsigned int idx=0; idx < MAX_NUMBER_OF_GRIDS; ++idx)
    {
	i_gridMask[idx] = 0;
	i_gridStatus[idx] = 0;
	for(unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
	{
	    i_grids[idx][j] = NULL;
	    i_info[idx][j] = NULL;
	}
    }
}

uint64
Map::EnsureGridCreated(const GridPair &p)
{
    uint64 mask = CalculateGridMask(p.y_coord);

    if( !(i_gridMask[p.x_coord] & mask) )
    {
	Guard guard(*this);
	if( !(i_gridMask[p.x_coord] & mask) )
	{
	    i_grids[p.x_coord][p.y_coord] = new GridType(p.x_coord*MAX_NUMBER_OF_GRIDS + p.y_coord);
	    i_info[p.x_coord][p.y_coord] = new GridInfo(i_gridExpiry);
	    i_gridMask[p.x_coord] |= mask; // sets the mask on
	}	
    }

    return mask;
}

void
Map::EnsurePlayerInGrid(const GridPair &p, Player *player)
{
    uint64 mask = EnsureGridCreated(p);
    GridType *grid = i_grids[p.x_coord][p.y_coord];
    assert(grid != NULL);
    if( !(i_gridStatus[p.x_coord] & mask) )
    {
	WriteGuard guard(i_info[p.x_coord][p.y_coord]->i_lock);
	if( !(i_gridStatus[p.x_coord] & mask) )
	{
	    sLog.outDebug("Player %s triggers of loading grid [%d,%d] on map %d", player->GetName(), p.x_coord, p.y_coord, i_id);
	    GridLoaderType loader;
	    ObjectGridLoader gloader(*grid, *player, i_id);
	    loader.Load(*grid, gloader);
	    grid->AddObject(player, player->GetGUID());
	    i_gridStatus[p.x_coord] |= mask;
	}
    }
    else
    {
	WriteGuard guard(i_info[p.x_coord][p.y_coord]->i_lock);
	grid->AddObject(player, player->GetGUID());

    }
}

void
Map::Add(Player *player)
{
    GridPair p = CalculateGrid(player->GetPositionX(), player->GetPositionY());
    
    assert( p.x_coord >= 0 && p.x_coord < MAX_NUMBER_OF_GRIDS &&
	    p.y_coord >= 0 && p.y_coord < MAX_NUMBER_OF_GRIDS );

    EnsurePlayerInGrid(p, player);
   
    sLog.outDebug("Player %s enters grid[%d, %d]", player->GetName(), p.x_coord, p.y_coord);
    GridType &grid(*i_grids[p.x_coord][p.y_coord]);
    MaNGOS::PlayerNotifier notifier(grid, *player);

    {   // don't remove the scope braces.. its for performance..
	ReadGuard guard(i_info[p.x_coord][p.y_coord]->i_lock);
	
	// notify players that a new player is in
	TypeContainerVisitor<MaNGOS::PlayerNotifier, ContainerMapList<Player> > player_notifier(notifier);
	grid.VisitObjects(player_notifier);
	TypeContainerVisitor<MaNGOS::PlayerNotifier, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
	grid.VisitGridObjects(object_notifier);
    }
}


template<class T>
void
Map::AddType(T *obj)
{
    GridPair p = CalculateGrid(obj->GetPositionX(), obj->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < MAX_NUMBER_OF_GRIDS &&
	    p.y_coord >= 0 && p.y_coord < MAX_NUMBER_OF_GRIDS );

    EnsureGridCreated(p);
    GridType *grid = i_grids[p.x_coord][p.y_coord];
    assert( grid != NULL );

    {
	WriteGuard guard(i_info[p.x_coord][p.y_coord]->i_lock);
	(*grid).template AddGridObject<T>(obj, obj->GetGUID());
    }

    sLog.outDebug("Object %d enters grid[%d,%d]", obj->GetGUID(), p.x_coord, p.y_coord);
    // now update things.. only required a read lock
    ReadGuard guard(i_info[p.x_coord][p.y_coord]->i_lock);
    MaNGOS::ObjectEnterNotifier<T> notifier(*grid, *obj);
    TypeContainerVisitor<MaNGOS::ObjectEnterNotifier<T>, ContainerMapList<Player> > player_notifier(notifier);
    grid->VisitObjects(player_notifier);	
}

void
Map::MessageBoardcast(Player *player, WorldPacket *msg, bool to_self)
{
    if( to_self )
	player->GetSession()->SendPacket(msg);

    GridPair p = CalculateGrid(player->GetPositionX(), player->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < MAX_NUMBER_OF_GRIDS &&
	    p.y_coord >= 0 && p.y_coord < MAX_NUMBER_OF_GRIDS );

    if( !loaded(p) )
	return; // dude.. how did I end up in a grid that's no loaded...
    
    GridType *grid = i_grids[p.x_coord][p.y_coord];
    assert( grid != NULL );
    if( grid->ObjectsInGrid() > 0 )
    {
	ReadGuard guard(i_info[p.x_coord][p.y_coord]->i_lock);
	MaNGOS::MessageDeliverer post_man(*player, msg);
	TypeContainerVisitor<MaNGOS::MessageDeliverer, ContainerMapList<Player> > message(post_man);
	grid->VisitObjects(message);
    }
}

void
Map::MessageBoardcast(Object *obj, WorldPacket *msg)
{
    GridPair p = CalculateGrid(obj->GetPositionX(), obj->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < MAX_NUMBER_OF_GRIDS &&
	    p.y_coord >= 0 && p.y_coord < MAX_NUMBER_OF_GRIDS );

    if( !loaded(p) )
	return; // ignore the creature.. no one there to hear him
    
    GridType *grid = i_grids[p.x_coord][p.y_coord];
    assert( grid != NULL );
    if( grid->ObjectsInGrid() > 0 )
    {
	ReadGuard guard(i_info[p.x_coord][p.y_coord]->i_lock);
	MaNGOS::ObjectMessageDeliverer post_man(*obj, msg);
	TypeContainerVisitor<MaNGOS::ObjectMessageDeliverer, ContainerMapList<Player> > message(post_man);
	grid->VisitObjects(message);
    }
}

void
Map::Add(Creature *creature)
{
    AddType<Creature>(creature);    
}

void
Map::Add(DynamicObject *obj)
{
    AddType<DynamicObject>(obj);    
}

void
Map::Add(GameObject *obj)
{
    AddType<GameObject>(obj);    
}

void
Map::Add(Corpse *obj)
{
    AddType<Corpse>(obj);    
}

bool
Map::loaded(const GridPair &p) const
{
    uint64 mask = CalculateGridMask(p.y_coord);
    return ( (i_gridMask[p.x_coord] & mask)  && (i_gridStatus[p.x_coord] & mask) );
}

// update map
void
Map::Update(uint32 t_diff)
{
	if (m_nextThinkTime > time(NULL))
		return; // Think once every 5 secs only for GameObject updates...

	m_nextThinkTime = time(NULL) + 5;

    for(unsigned int i=0; i < MAX_NUMBER_OF_GRIDS; ++i)
    {
	uint64 mask = 1;
	for(unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
	{
	    uint64 mask2 = CalculateGridMask(j);
	    assert(mask2 == mask);
	    if( i_gridMask[i] & mask )
	    {
		assert(i_grids[i][j] != NULL && i_info[i][j] != NULL);
		GridType &grid(*i_grids[i][j]);
		GridInfo &info(*i_info[i][j]);
		if( grid.ObjectsInGrid() > 0 )
		{       
		    {
			// update the object
			grid.SetGridStatus(GRID_STATUS_ACTIVE);
			MaNGOS::GridUpdater updater(grid, t_diff);
			TypeContainerVisitor<MaNGOS::GridUpdater, ContainerMapList<Player> > player_notifier(updater);
			grid.VisitObjects(player_notifier);
			TypeContainerVisitor<MaNGOS::GridUpdater, TypeMapContainer<AllObjectTypes> > object_notifier(updater);
			grid.VisitGridObjects(object_notifier);
		    }

		    // now update packets.. it uses the same notifier as the player moves..
		    // (1) update its inrange objects (remove the out of range)
		    // (2) update the newly in range
		    //MaNGOS::PlayerUpdatePacket update_packet(*i_grids[i][j]);
		    //TypeContainerVisitor<MaNGOS::PlayerUpdatePacket, ContainerMapList<Player> > player_update(update_packet);
		    //graid.VisitObjects(player_update);
		}
		else
		{
		    if( grid.GetGridStatus() == GRID_STATUS_REMOVAL )
		    {
			// unloading the grid....
			info.i_timer.Update(t_diff);
			if( info.i_timer.Passed() )
			{
			    sLog.outDebug("Unloading grid[%d,%d] for map %d", i,j, i_id);
			    {
				WriteGuard guard(i_info[i][j]->i_lock);
				ObjectGridUnloader unloader(grid);
				TypeContainerVisitor<ObjectGridUnloader, TypeMapContainer<AllObjectTypes> > object_unload(unloader);
				i_gridMask[i] &= ~mask; // clears the bit
				i_gridStatus[i] &= ~mask;
				grid.VisitGridObjects(object_unload);
				delete i_grids[i][j];
				i_grids[i][j] = NULL;
			    }
			    
			    delete i_info[i][j];
			    i_info[i][j] = NULL;
			}
		    }
		    else if( grid.GetGridStatus() == GRID_STATUS_IDLE )
		    {
			// schedule for removale
			grid.SetGridStatus(GRID_STATUS_REMOVAL);
			info.i_timer.Reset(i_gridExpiry); // starts the count count of removal
		       
		    }
		    else if( grid.GetGridStatus() == GRID_STATUS_ACTIVE )
		    {
			grid.SetGridStatus(GRID_STATUS_IDLE);
		    }
		    else
		    {
			// invalid status don't triggers unload..
		    }
		}
	    }
	    mask <<= 1;
	}
    }
}

void
Map::Remove(Player *player, bool remove)
{   
    GridPair p = CalculateGrid(player->GetPositionX(), player->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < MAX_NUMBER_OF_GRIDS &&
	    p.y_coord >= 0 && p.y_coord < MAX_NUMBER_OF_GRIDS );
    
    uint64 mask = CalculateGridMask(p.y_coord);

    if( !(i_gridMask[p.x_coord] & mask) )
    {
	assert( false );	
	return; // hmm...  how can we end up here in an unloaded grid
    }

    sLog.outDebug("Remove player %s from grid[%d,%d]", player->GetName(), p.x_coord, p.y_coord);
    GridType *grid = i_grids[p.x_coord][p.y_coord];
    assert(grid != NULL);
    
    {  // do not remove the scope braces... fro performances
	WriteGuard guard(i_info[p.x_coord][p.y_coord]->i_lock);
	grid->RemoveObject(player, player->GetGUID());
	player->RemoveFromWorld();
    }
    

    if( remove )
	delete player;
}

template<class T>
void
Map::RemoveType(T *obj, bool remove)
{
    GridPair p = CalculateGrid(obj->GetPositionX(), obj->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < MAX_NUMBER_OF_GRIDS &&
	    p.y_coord >= 0 && p.y_coord < MAX_NUMBER_OF_GRIDS );

    if( !loaded(p) )
	return; // doesn't make sense to remove a creature in a location where its nothing

    sLog.outDebug("Remove object % from grid[%d,%d]", obj->GetGUID(), p.x_coord, p.y_coord);
    GridType *grid = i_grids[p.x_coord][p.y_coord];
    assert( grid != NULL );

    {  // do not remove the scope braces... fro performances
	WriteGuard guard(i_info[p.x_coord][p.y_coord]->i_lock);
	(*grid).template RemoveGridObject<T>(obj, obj->GetGUID());
    }

    // now notify the players this creature is gone..
    {
	ReadGuard guard(i_info[p.x_coord][p.y_coord]->i_lock);
	MaNGOS::InRangeRemover range_remover(*grid, *obj);
	TypeContainerVisitor<MaNGOS::InRangeRemover, ContainerMapList<Player> > player_notifier(range_remover);
	grid->VisitObjects(player_notifier);
    }
    
    if( remove )
	delete obj;
}

void
Map::RemoveFromMap(Player *player)
{
    RemoveType<Player>(player, false);
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

void
Map::PlayerRelocation(Player *player, const float &x, const float &y, const float &z, const float &orientation)
{
    GridPair old_grid = CalculateGrid(player->GetPositionX(), player->GetPositionY());
    GridPair new_grid = CalculateGrid(x, y);

    if( old_grid != new_grid )
    {
	sLog.outDebug("Player %s relocation grid[%d,%d]->grid[%d,%d]", player->GetName(), old_grid.x_coord, old_grid.y_coord,new_grid.x_coord, new_grid.y_coord);       	
	
	// Remove in range objects for the old grid
	GridType &grid(*i_grids[old_grid.x_coord][old_grid.y_coord]);

	// inorder to figure out the objects are still in range from the old grid.. the player has to
	// be in a new position
	player->Relocate(x, y, z, orientation);
	MaNGOS::PlayerRelocationNotifier notifier(*player);
	TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, ContainerMapList<Player> > player_notifier(notifier);
	grid.VisitObjects(player_notifier);
	TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
	grid.VisitGridObjects(object_notifier);

	{
	    WriteGuard guard(i_info[old_grid.x_coord][old_grid.y_coord]->i_lock);
	    grid.RemoveObject(player, player->GetGUID());	    
	}
	EnsurePlayerInGrid(new_grid, player);
    }
    else
        player->Relocate(x, y, z, orientation);

    // now inform all others in the grid of your activity
    GridType *grid = i_grids[new_grid.x_coord][new_grid.y_coord];
    assert(grid != NULL);    

    MaNGOS::PlayerRelocationNotifier notifier(*player);
    TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, ContainerMapList<Player> > player_notifier(notifier);
    grid->VisitObjects(player_notifier);
    TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
    grid->VisitGridObjects(object_notifier);
}


template<class T>
void
Map::ObjectRelocation(T *obj, const float &x, const float &y, const float &z, const float &ang)
{
    GridPair old_grid = CalculateGrid(obj->GetPositionX(), obj->GetPositionY());
    GridPair new_grid = CalculateGrid(x, y);

    if( old_grid != new_grid )
    {
	sLog.outDebug("This creature "I64FMT" moved from grid[%d,%d] to grid[%d,%d].", obj->GetGUID(), old_grid.x_coord, old_grid.y_coord, new_grid.x_coord, new_grid.y_coord);

	{
	    assert(i_info[old_grid.x_coord][old_grid.y_coord] != NULL);
	    WriteGuard guard(i_info[old_grid.x_coord][old_grid.y_coord]->i_lock);
	    (*i_grids[old_grid.x_coord][old_grid.y_coord]).template RemoveGridObject<T>(obj, obj->GetGUID());
	}
  
	EnsureGridCreated(new_grid);    
	{
	    WriteGuard guard(i_info[new_grid.x_coord][new_grid.y_coord]->i_lock);
	    (*i_grids[new_grid.x_coord][new_grid.y_coord]).template AddGridObject<T>(obj, obj->GetGUID());
	}
    }

    // we still need to update the players to see if this guy
    // is move out of the player's range OR has moved in
    // the player's range.
    GridType &grid(*i_grids[new_grid.x_coord][new_grid.y_coord]);
    ReadGuard guard(i_info[new_grid.x_coord][new_grid.y_coord]->i_lock);
    obj->Relocate(x, y, z, ang);
    MaNGOS::ObjectRelocationNotifier<T> notifier(*obj);
    TypeContainerVisitor<MaNGOS::ObjectRelocationNotifier<T>, ContainerMapList<Player> > player_notifier(notifier);
    grid.VisitObjects(player_notifier);    
    
    // what about move into other creatures location and start an attack?
}

// explicit template instantiation
template void Map::ObjectRelocation<Creature>(Creature *, const float &, const float &, const float &, const float &);
template void Map::ObjectRelocation<GameObject>(GameObject *, const float &, const float &, const float &, const float &);
template void Map::ObjectRelocation<DynamicObject>(DynamicObject *, const float &, const float &, const float &, const float &);
template void Map::ObjectRelocation<Corpse>(Corpse *, const float &, const float &, const float &, const float &);


