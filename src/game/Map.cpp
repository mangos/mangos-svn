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
#include "GridStates.h"
#include "RedZoneDistrict.h"

// 3 minutes
#define DEFAULT_GRID_EXPIRY     300
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
        i_grids[p.x_coord][p.y_coord] = new GridType(p.x_coord*MAX_NUMBER_OF_GRIDS + p.y_coord);
        i_info[p.x_coord][p.y_coord] = new GridInfo(i_gridExpiry);
        i_gridMask[p.x_coord] |= mask; // sets the mask on
    }    
    }

    return mask;
}

void Map::EnsurePlayerInGrid(const GridPair &p, Player *player, bool add_player)
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

        if( add_player )
        grid->AddObject(player, player->GetGUID());
        i_gridStatus[p.x_coord] |= mask;
    }
    }
    else if( add_player )
    {
    WriteGuard guard(i_info[p.x_coord][p.y_coord]->i_lock);
    grid->AddObject(player, player->GetGUID());

    }
}

void Map::NotifyPlayerInRange(const GridPair &p, Player *player)
{
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



void Map::Add(Player *player)
{
    GridPair p = CalculateGrid(player->GetPositionX(), player->GetPositionY());
    
    assert( p.x_coord >= 0 && p.x_coord < MAX_NUMBER_OF_GRIDS &&
        p.y_coord >= 0 && p.y_coord < MAX_NUMBER_OF_GRIDS );

    sLog.outDebug("Player %s enters grid[%d, %d]", player->GetName(), p.x_coord, p.y_coord);
    EnsurePlayerInGrid(p, player, true);
    NotifyPlayerInRange(p, player);
}


template<class T>
void Map::AddType(T *obj)
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

void Map::MessageBoardcast(Player *player, WorldPacket *msg, bool to_self)
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

void Map::MessageBoardcast(Object *obj, WorldPacket *msg)
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

void Map::Add(Creature *creature)
{
    AddType<Creature>(creature);    
}

void Map::Add(DynamicObject *obj)
{
    AddType<DynamicObject>(obj);    
}

void Map::Add(GameObject *obj)
{
    AddType<GameObject>(obj);    
}

void Map::Add(Corpse *obj)
{
    AddType<Corpse>(obj);    
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
        GridType &grid(*i_grids[i][j]);
        GridInfo &info(*i_info[i][j]);
        si_GridStates[grid.GetGridState()]->Update(*this, grid, info, i, j, t_diff);
        }

        mask <<= 1;
    }
    }
}

void Map::Remove(Player *player, bool remove)
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
void Map::RemoveType(T *obj, bool remove)
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

void Map::RemoveFromMap(Player *player)
{
    RemoveType<Player>(player, false);
}

void Map::Remove(Corpse *obj, bool remove)
{
    RemoveType<Corpse>(obj, remove);
}

void Map::Remove(GameObject *obj, bool remove)
{
    RemoveType<GameObject>(obj, remove);
}

void Map::Remove(Creature *obj, bool remove)
{
    RemoveType<Creature>(obj, remove);
}

void Map::Remove(DynamicObject *obj, bool remove)
{
    RemoveType<DynamicObject>(obj, remove);
}

void Map::PlayerRelocation(Player *player, const float &x, const float &y, const float &z, const float &orientation)
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
    EnsurePlayerInGrid(new_grid, player, true);
    }
    else
        player->Relocate(x, y, z, orientation);

    // now inform all others in the grid of your activity
    GridType *grid = i_grids[new_grid.x_coord][new_grid.y_coord];
    assert(grid != NULL);    

    {
    ReadGuard guard(i_info[new_grid.x_coord][new_grid.y_coord]->i_lock);
    MaNGOS::PlayerRelocationNotifier notifier(*player);
    TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, ContainerMapList<Player> > player_notifier(notifier);
    grid->VisitObjects(player_notifier);
    TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, TypeMapContainer<AllObjectTypes> > object_notifier(notifier);
    grid->VisitGridObjects(object_notifier);
    }
    grid->SetGridState(GRID_STATE_ACTIVE);
}


template<class T>
void Map::ObjectRelocation(T *obj, const float &x, const float &y, const float &z, const float &ang)
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

bool Map::UnloadGrid(const uint32 &x, const uint32 &y)
{
    GridType *grid = i_grids[x][y];
    assert( grid != NULL && i_info[x][y] != NULL );
    sLog.outDebug("Unloading grid[%d,%d] for map %d", x,y, i_id);
    {
    WriteGuard guard(i_info[x][y]->i_lock); // prevent ALL activities until this is done..

    /** Before we unload.. ensure no player has referenced to the grid...
     */
    ObjectAccessor::PlayerMapType& players(ObjectAccessor::Instance().GetPlayers());
    MaNGOS::ValidateGridUnload validater(players, *this, x, y);
    TypeContainerVisitor<MaNGOS::ValidateGridUnload, TypeMapContainer<AllObjectTypes> > object_validater(validater);
    grid->VisitGridObjects(object_validater);
    
    if( !validater.IsOkToUnload() )
        return false;

    ObjectGridUnloader unloader(*grid);
    TypeContainerVisitor<ObjectGridUnloader, TypeMapContainer<AllObjectTypes> > object_unload(unloader);
    uint64 mask = CalculateGridMask(y);
    i_gridMask[x] &= ~mask; // clears the bit
    i_gridStatus[y] &= ~mask;
    grid->VisitGridObjects(object_unload);
    delete i_grids[x][y];
    i_grids[x][y] = NULL;
    }
    
    delete i_info[x][y];
    i_info[x][y] = NULL;
    return true;
}

void Map::ZoneAlert(Player &player, const GridPair &p, const uint8 &mask)
{
    if( mask == RedZoneDistrict::si_UpperLeftCorner )
    {
    GridPair p_1(p.x_coord-1, p.y_coord-1);
    GridPair p_2(p.x_coord, p.y_coord-1);
    GridPair p_3(p.x_coord-1, p.y_coord);
    }
    else if ( mask == RedZoneDistrict::si_UpperRightCorner )
    {
    GridPair p_1(p.x_coord, p.y_coord-1);
    GridPair p_2(p.x_coord+1, p.y_coord-1);
    GridPair p_3(p.x_coord+1, p.y_coord);
    }
    else if( mask == RedZoneDistrict::si_LowerLeftCorner )
    {
    GridPair p_1(p.x_coord-1, p.y_coord);
    GridPair p_2(p.x_coord-1, p.y_coord+1);
    GridPair p_3(p.x_coord, p.y_coord+1);
    }
    else if( mask == RedZoneDistrict::si_LowerRightCorner )
    {
    GridPair p_1(p.x_coord, p.y_coord+1);
    GridPair p_2(p.x_coord+1, p.y_coord+1);
    GridPair p_3(p.x_coord+1, p.y_coord);
    }
    else if( mask == RedZoneDistrict::si_LeftCenter )
    {
    GridPair p_1(p.x_coord-1, p.y_coord);
    }
    else if( mask == RedZoneDistrict::si_RightCenter )
    {
    GridPair p_1(p.x_coord+1, p.y_coord);
    }
    else if( mask == RedZoneDistrict::si_UpperCenter )
    {
    GridPair p_1(p.x_coord, p.y_coord-1);
    }
    else if( mask == RedZoneDistrict::si_LowerCenter )
    {
    GridPair p_1(p.x_coord, p.y_coord+1);
    }
}

// explicit template instantiation
template void Map::ObjectRelocation<Creature>(Creature *, const float &, const float &, const float &, const float &);
template void Map::ObjectRelocation<GameObject>(GameObject *, const float &, const float &, const float &, const float &);
template void Map::ObjectRelocation<DynamicObject>(DynamicObject *, const float &, const float &, const float &, const float &);
template void Map::ObjectRelocation<Corpse>(Corpse *, const float &, const float &, const float &, const float &);


