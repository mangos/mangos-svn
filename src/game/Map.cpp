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

#include "Player.h"
#include "GridNotifiers.h"
#include "WorldSession.h"
#include "Log.h"
#include "GridStates.h"
#include "RedZoneDistrict.h"
#include "CellImpl.h"
#include "Map.h"
#include "GridNotifiersImpl.h"

// 3 minutes
#define DEFAULT_GRID_EXPIRY     300

// we give 50 milliseconds to load a grid
#define MAX_GRID_LOAD_TIME      50 
static GridState* si_GridStates[MAX_GRID_STATE];

// static initialization
void Map::InitStateMachine()
{
    si_GridStates[GRID_STATE_INVALID] = new InvalidState;
    si_GridStates[GRID_STATE_ACTIVE] = new ActiveState;
    si_GridStates[GRID_STATE_IDLE] = new IdleState;
    si_GridStates[GRID_STATE_REMOVAL] = new RemovalState;
}


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
	    i_grids[p.x_coord][p.y_coord] = new NGridType(p.x_coord*MAX_NUMBER_OF_GRIDS + p.y_coord);
	    i_info[p.x_coord][p.y_coord] = new GridInfo(i_gridExpiry);
	    i_gridMask[p.x_coord] |= mask; // sets the mask on
	}	
    }

    return mask;
}

void
Map::EnsureGridLoadedForPlayer(const Cell &cell, Player *player, bool add_player)
{
    uint64 mask = EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];

    assert(grid != NULL);
    if( !(i_gridStatus[cell.GridX()] & mask) )
    {
	WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
	if( !(i_gridStatus[cell.GridX()] & mask) )
	{
	    if( player != NULL )
	    {
		player->SendDelayResponse(MAX_GRID_LOAD_TIME);
		DEBUG_LOG("Player %s enter cell[%d,%d] triggers of loading grid[%d,%d] on map %d", player->GetName(), cell.CellX(), cell.CellY(), cell.GridX(), cell.GridY(), i_id);
	    }
	    else
		DEBUG_LOG("Player nearby triggers of loading grid [%d,%d] on map %d", cell.GridX(), cell.GridY(), i_id); 

	    ObjectGridLoader loader(*grid, i_id, cell);
	    loader.LoadN();
	    grid->SetGridState(GRID_STATE_ACTIVE);

	    if( add_player && player != NULL )
		(*grid)(cell.CellX(), cell.CellY()).AddObject(player, player->GetGUID());		
	    i_gridStatus[cell.GridX()] |= mask;
	}
    }
    else if( add_player )
    {
	WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
	(*grid)(cell.CellX(), cell.CellY()).AddObject(player, player->GetGUID());
    }
}

void
Map::NotifyPlayerVisibility(const Cell &cell, const CellPair &cell_pair, Player *player)
{
    MaNGOS::PlayerNotifier pl_notifier(*player);
    MaNGOS::VisibleNotifier obj_notifier(*player);
    TypeContainerVisitor<MaNGOS::PlayerNotifier, ContainerMapList<Player> > player_notifier(pl_notifier);
    TypeContainerVisitor<MaNGOS::VisibleNotifier, TypeMapContainer<AllObjectTypes> > object_notifier(obj_notifier);
    
    CellLock<ReadGuard> cell_lock(cell, cell_pair);
    cell_lock->Visit(cell_lock, player_notifier, *this);
    pl_notifier.Notify(); // tell the dudes :-)
    cell_lock->Visit(cell_lock, object_notifier, *this);
    obj_notifier.Notify();
}


void Map::Add(Player *player)
{
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    
    assert( p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
	    p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );

    Cell cell = RedZone::GetZone(p);
    EnsureGridLoadedForPlayer(cell, player, true);
    cell.data.Part.reserved = ALL_DISTRICT; // ensure handle all district
    NotifyPlayerVisibility(cell, p, player);
}


template<class T>
void
Map::Add(T *obj)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
	    p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );

    Cell cell = RedZone::GetZone(p);
    EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );

    {
	WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
	(*grid)(cell.CellX(), cell.CellY()).template AddGridObject<T>(obj, obj->GetGUID());
    }

    DEBUG_LOG("Object %d enters grid[%d,%d]", obj->GetGUID(), cell.GridX(), cell.GridY());
    cell.data.Part.reserved = ALL_DISTRICT;

    // now update things.. only required a read lock
    MaNGOS::ObjectVisibleNotifier notifier(*static_cast<Object *>(obj));
    TypeContainerVisitor<MaNGOS::ObjectVisibleNotifier, ContainerMapList<Player> > player_notifier(notifier);
    
    CellLock<ReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, player_notifier, *this);
}

void Map::MessageBoardcast(Player *player, WorldPacket *msg, bool to_self)
{
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
	    p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );

    Cell cell = RedZone::GetZone(p);
    cell.data.Part.reserved = ALL_DISTRICT;

    if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
	return; // dude.. how did I end up in a grid that's no loaded...   

    MaNGOS::MessageDeliverer post_man(*player, msg, to_self);
    TypeContainerVisitor<MaNGOS::MessageDeliverer, ContainerMapList<Player> > message(post_man);
    CellLock<ReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, message, *this);
}

void Map::MessageBoardcast(Object *obj, WorldPacket *msg)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
	    p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );

    Cell cell = RedZone::GetZone(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate(); // ensure the grid will not create if no player is nearby.

    if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
    return; // ignore the creature.. no one there to hear him

    
    MaNGOS::ObjectMessageDeliverer post_man(*obj, msg);
    TypeContainerVisitor<MaNGOS::ObjectMessageDeliverer, ContainerMapList<Player> > message(post_man);
    CellLock<ReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, message, *this); 
}

bool Map::loaded(const GridPair &p) const
{
    uint64 mask = CalculateGridMask(p.y_coord);
    return ( (i_gridMask[p.x_coord] & mask)  && (i_gridStatus[p.x_coord] & mask) );
}

// update map
void Map::Update(const uint32 &t_diff)
{
    for(unsigned int i=0; i < MAX_NUMBER_OF_GRIDS; ++i)
    {
	uint64 mask = 1;
	for(unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
	{
	    if( i_gridMask[i] & mask )
	    {
		assert(i_grids[i][j] != NULL && i_info[i][j] != NULL);
		NGridType &grid(*i_grids[i][j]);
		GridInfo &info(*i_info[i][j]);
		si_GridStates[grid.GetGridState()]->Update(*this, grid, info, i, j, t_diff);
	    }
	    mask <<= 1;
	}
    }
}

void Map::Remove(Player *player, bool remove)
{   
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
	    p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );
    
    Cell cell = RedZone::GetZone(p);
    uint64 mask = CalculateGridMask(cell.data.Part.grid_y);

    if( !(i_gridMask[cell.data.Part.grid_x] & mask) )
    {
	assert( false );    
	return; // hmm...  how can we end up here in an unloaded grid
    }

    DEBUG_LOG("Remove player %s from grid[%d,%d]", player->GetName(), cell.GridX(), cell.GridY());
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert(grid != NULL);
    
    {  // do not remove the scope braces... fro performances
	WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
	grid->RemoveObject(cell.CellX(), cell.CellY(), player, player->GetGUID());
	player->RemoveFromWorld();
    }

    cell.data.Part.reserved = ALL_DISTRICT;
    MaNGOS::NotVisibleNotifier notifier(*player);
    TypeContainerVisitor<MaNGOS::NotVisibleNotifier, ContainerMapList<Player> > player_notifier(notifier);
    CellLock<ReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, player_notifier, *this);
    notifier.Notify();

    if( remove )
	delete player;
}

template<class T>
void
Map::Remove(T *obj, bool remove)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    assert( p.x_coord >= 0 && p.x_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP &&
	    p.y_coord >= 0 && p.y_coord < TOTAL_NUMBER_OF_CELLS_PER_MAP );

    Cell cell = RedZone::GetZone(p);
    if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
	return; // doesn't make sense to remove a creature in a location where its nothing

    DEBUG_LOG("Remove object % from grid[%d,%d]", obj->GetGUID(), cell.data.Part.grid_x, cell.data.Part.grid_y);
    NGridType *grid = i_grids[cell.GridX()][cell.GridY()];
    assert( grid != NULL );

    {  // do not remove the scope braces... fro performances
	WriteGuard guard(i_info[cell.GridX()][cell.GridY()]->i_lock);
	(*grid)(cell.CellX(), cell.CellY()).template RemoveGridObject<T>(obj, obj->GetGUID());
    }

    // now notify the players this creature is gone..
    {
	Cell cell = RedZone::GetZone(p);
	CellLock<ReadGuard> cell_lock(cell, p);
	MaNGOS::ObjectNotVisibleNotifier notifier(*obj);
	TypeContainerVisitor<MaNGOS::ObjectNotVisibleNotifier, ContainerMapList<Player> > player_notifier(notifier);
	cell_lock->Visit(cell_lock, player_notifier, *this);
    }
    
    if( remove )
	delete obj;
}


void
Map::PlayerRelocation(Player *player, const float &x, const float &y, const float &z, const float &orientation)
{
    CellPair old_val = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    CellPair new_val = MaNGOS::ComputeCellPair(x, y);
    
    Cell old_cell = RedZone::GetZone(old_val);
    Cell new_cell = RedZone::GetZone(new_val);
    new_cell |= old_cell;
    player->Relocate(x, y, z, orientation);

    if( old_cell.DiffGrid(new_cell) || old_cell.DiffCell(new_cell) )
    {
	DEBUG_LOG("Player %s relocation grid[%d,%d]cell[%d,%d]->grid[%d,%d]cell[%d,%d]", player->GetName(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY()); 
	
	NGridType &grid(*i_grids[old_cell.GridX()][old_cell.GridY()]);
	
	{
	    WriteGuard guard(i_info[old_cell.GridX()][old_cell.GridY()]->i_lock);
	    grid(old_cell.CellX(),old_cell.CellY()).RemoveObject(player, player->GetGUID());

	    // NOTE, this will hold a write lock once..
	    if( !old_cell.DiffGrid(new_cell) )
		grid(new_cell.CellX(),new_cell.CellY()).AddObject(player, player->GetGUID());
	}

	if( old_cell.DiffGrid(new_cell) )
	    EnsureGridLoadedForPlayer(new_cell, player, true);
    }

    // now inform all others in the grid of your activity
    // (1) first within your visibility..
    CellLock<ReadGuard> cell_lock(new_cell, new_val);

    if( old_cell == new_cell )
    {
	// even if we move a little.. gotta let other player see my update...
	MaNGOS::VisibleNotifier notifier(*player);
	TypeContainerVisitor<MaNGOS::VisibleNotifier, ContainerMapList<Player> > player_notifier(notifier);
	new_cell.data.Part.reserved = ALL_DISTRICT;
	cell_lock->Visit(cell_lock, player_notifier, *this);
	notifier.Notify();
	return;
    }

    MaNGOS::VisibleNotifier notifier(*player);
    TypeContainerVisitor<MaNGOS::VisibleNotifier, ContainerMapList<Player> > player_notifier(notifier);
    TypeContainerVisitor<MaNGOS::VisibleNotifier, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
    cell_lock->Visit(cell_lock, object_notifier, *this);
    notifier.Notify();

    // (2) out of your visibility
    MaNGOS::NotVisibleNotifier notifier2(*player);
    TypeContainerVisitor<MaNGOS::NotVisibleNotifier, ContainerMapList<Player> > player_notifier2(notifier2);
    TypeContainerVisitor<MaNGOS::NotVisibleNotifier, TypeMapContainer<AllObjectTypes> > object_notifier2(notifier2);
    cell_lock = CellLock<ReadGuard>(old_cell, old_val);
    cell_lock->Visit(cell_lock, player_notifier2, *this);
    cell_lock->Visit(cell_lock, object_notifier2, *this);
    i_grids[new_cell.GridX()][new_cell.GridY()]->SetGridState(GRID_STATE_ACTIVE);
    notifier2.Notify();
}


void
Map::CreatureRelocation(Creature *creature, const float &x, const float &y, const float &z, const float &ang)
{ 
    CellPair old_val = MaNGOS::ComputeCellPair(creature->GetPositionX(), creature->GetPositionY());
    CellPair new_val = MaNGOS::ComputeCellPair(x, y);
    
    Cell old_cell = RedZone::GetZone(old_val);
    Cell new_cell = RedZone::GetZone(new_val);
    creature->Relocate(x, y, z, ang);

    if( creature->GetCreatureState() == ATTACKING /* TBD */ )
    {
	if( old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell) )
	{
	    DEBUG_LOG("Creature "I64FMT" moved from grid[%d,%d]cell[%d,%d] to grid[%d,%d]cell[%d,%d].", creature->GetGUID(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
	    
	    {
		assert(i_info[old_cell.GridX()][old_cell.GridY()] != NULL);
		WriteGuard guard(i_info[old_cell.GridX()][old_cell.GridY()]->i_lock);
		(*i_grids[old_cell.GridX()][old_cell.GridY()])(old_cell.CellX(), old_cell.CellY()).RemoveGridObject<Creature>(creature, creature->GetGUID());
		if( !old_cell.DiffGrid(new_cell) )
		    (*i_grids[new_cell.GridX()][new_cell.GridY()])(new_cell.CellX(), new_cell.CellY()).AddGridObject<Creature>(creature, creature->GetGUID());
	    }
	    
	    if( old_cell.DiffGrid(new_cell) )
	    {
		EnsureGridCreated(GridPair(new_cell.GridX(), new_cell.GridY()));    
		WriteGuard guard(i_info[new_cell.GridX()][new_cell.GridY()]->i_lock);
		(*i_grids[new_cell.GridX()][new_cell.GridY()])(new_cell.CellX(), new_cell.CellY()).AddGridObject<Creature>(creature, creature->GetGUID());
	    }
	}
	
	new_cell |= old_cell;
	new_cell.SetNoCreate();
	old_cell.SetNoCreate();

	// move in visibility.. the creature can chase all it wants to
	CellLock<ReadGuard> cell_lock(new_cell, new_val);
	if( new_cell == old_cell )
	{
	    new_cell.data.Part.reserved = ALL_DISTRICT;

	    MaNGOS::CreatureVisibleMovementNotifier notifier(*creature);
	    TypeContainerVisitor<MaNGOS::CreatureVisibleMovementNotifier, ContainerMapList<Player> > player_notifier(notifier);
	    cell_lock->Visit(cell_lock, player_notifier, *this);
	    return;
	}
	
	// move out of visibity
	MaNGOS::CreatureNotVisibleMovementNotifier notifier2(*creature);
	TypeContainerVisitor<MaNGOS::CreatureNotVisibleMovementNotifier, ContainerMapList<Player> > player_notifier2(notifier2);
	cell_lock = CellLock<ReadGuard>(old_cell, old_val);
	cell_lock->Visit(cell_lock, player_notifier2, *this);	
    }
    else
    {
	// minimum.. let other players know my movement
	CellLock<ReadGuard> cell_lock(new_cell, new_val);
	new_cell.data.Part.reserved = ALL_DISTRICT;
	new_cell.SetNoCreate();
	MaNGOS::CreatureVisibleMovementNotifier notifier(*creature);
	TypeContainerVisitor<MaNGOS::CreatureVisibleMovementNotifier, ContainerMapList<Player> > player_notifier(notifier);
	cell_lock->Visit(cell_lock, player_notifier, *this);	
    }
}


bool Map::UnloadGrid(const uint32 &x, const uint32 &y)
{
    NGridType *grid = i_grids[x][y];
    assert( grid != NULL && i_info[x][y] != NULL );

    {
	
	if( ObjectAccessor::Instance().PlayersNearGrid(x, y, i_id) )
	    return false;
	
	WriteGuard guard(i_info[x][y]->i_lock); // prevent ALL activities until this is done..
	DEBUG_LOG("Unloading grid[%d,%d] for map %d", x,y, i_id);
	ObjectGridUnloader unloader(*grid);
	uint64 mask = CalculateGridMask(y);
	i_gridMask[x] &= ~mask; // clears the bit
	i_gridStatus[x] &= ~mask;
	unloader.UnloadN();
	delete i_grids[x][y];
	i_grids[x][y] = NULL;
    }
    
    delete i_info[x][y];
    i_info[x][y] = NULL;
    return true;
}


// explicit template instantiation
template void Map::Add(Creature *);
template void Map::Add(GameObject *);
template void Map::Add(DynamicObject *);
template void Map::Add(Corpse *);

template void Map::Remove(Creature *,bool);
template void Map::Remove(GameObject *, bool);
template void Map::Remove(DynamicObject *, bool);
template void Map::Remove(Corpse *, bool);



