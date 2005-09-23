/* Zone.cpp
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


#include "Zone.h"
#include "GameSystem/TypeContainerVisitor.h"
#include "GameSystem/TypeContainer.h"

#include "WorldPacket.h"
#include "UpdateData.h"
#include "WorldSession.h"

// VISIBILITY_RANGE = UPDATE_DISTANCE*UPDATE_DISTANCE = 155.8*155.8 = 24274
#define VISIBILITY_RANGE    24274


void
Zone::AddPlayer(Player *player)
{
    bool add_player = true;
    if( i_grid == NULL )
    {
	// double check-Lock pattern
	Guard guard(*this);
	if( i_grid == NULL )
	{
	    // create new grid
	    GridType *grid = new GridType(i_coord1.x, i_coord1.y, i_coord2.x, i_coord2.y);

	    // load grid
	    GridLoaderType loader;
	    ObjectGridLoader gloader(*grid, *player);
	    loader.Load(*grid, gloader);

	    // adds player in the grid
	    grid->AddObject(player);
	    i_grid = grid;
	    add_player = false;
	}
    }
    
    if( add_player )
    {
	Guard guard(*this); // sync...
	i_grid->AddObject(player);
    }

    // notify all objects (include creatures but not players) in the world that's within your visibility
    NotifyObject notifier_obj(*i_grid, *player);
    TypeContainerVisitor<Zone::NotifyObject, TypeMapContainer<AllObjectTypes> > notify_object(notifier_obj);
    i_grid->VisitGridObjects(notify_object);
    
    // notify surrounding player of your existence.	
    NotifyPlayer notifer_pl(*i_grid, *player);
    TypeContainerVisitor<Zone::NotifyPlayer, ContainerMapList<Player> > notify_player(notifer_pl);
    i_grid->VisitObjects(notify_player); 
}

void
Zone::RemovePlayer(Player *player)
{
    if( i_grid != NULL )
    {
	i_grid->RemoveObject(player);
	
	// remove all objects in this grid from the player
	RemoveGridObject remover(*player);
	TypeContainerVisitor<RemoveGridObject, TypeMapContainer<AllObjectTypes> > object_remover(remover);
	i_grid->VisitGridObjects(object_remover);
	
	// set grid status and schedule for unloading
    }
}

void
Zone::Update(uint32 diff)
{
    PlayerUpdater updater(*i_grid, diff);
    TypeContainerVisitor<Zone::PlayerUpdater, ContainerMapList<Player> > notify_player(updater);
    i_grid->VisitObjects(notify_player);
}


//======================================//
//         RemoveGridObject

// These method a VERY INEFFICIENT due to the player class, we need to fix
// that first
template<class T> void remove_in_range(std::map<OBJECT_HANDLE, T *> &m, Player &player)
{
    for(typename std::map<OBJECT_HANDLE, T *>::iterator iter = m.begin(); iter != m.end(); ++iter)
	player.RemoveInRangeObject(iter->second);
}


void
RemoveGridObject::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
{    
    remove_in_range(m, i_player);
}

void
RemoveGridObject::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    remove_in_range(m, i_player);
}

void
RemoveGridObject::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    remove_in_range(m, i_player);
}


//======================================//
//         NotifyPlayer
void
Zone::NotifyPlayer::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    WorldPacket my_packet;
    UpdateData my_data;

    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	assert( iter->second != &i_player );
	WorldPacket packet;
	UpdateData player_data;

	// build my data
	i_player.BuildCreateUpdateBlockForPlayer(&player_data, iter->second);
	player_data.BuildPacket(&packet);
	
	// let others know about myself
	iter->second->GetSession()->SendPacket(&packet);	

	// collect data about other players to let myself know
	iter->second->BuildCreateUpdateBlockForPlayer(&my_data, &i_player);
    }

    // send to myself about other player information and self
    sLog.outDetail("Creating player data for himself %d", i_player.GetGUID());
    i_player.BuildCreateUpdateBlockForPlayer(&my_data, &i_player);
    my_data.BuildPacket(&my_packet);
    i_player.GetSession()->SendPacket(&my_packet);
    i_player.AddToWorld();
}

//=====================================//
//        NotifyObject
void
Zone::NotifyObject::buildObjectData(UpdateData &update_data, Object *obj)
{
    if( obj->GetDistance2dSq(&i_player) <= VISIBILITY_RANGE )
    {
	obj->AddInRangeObject(&i_player);		
	sLog.outDetail("Creating object %d for player %d", obj->GetGUID(), i_player.GetGUID());
	obj->BuildCreateUpdateBlockForPlayer(&update_data, &i_player);
	i_player.AddInRangeObject(obj);
    }       
}

// do we handle GameObjects differently than creatures (yes.. but not now)
// after re-implement Creature and GameObject class.
void
Zone::NotifyObject::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
{
    UpdateData update_data;
    WorldPacket packet;
    for(std::map<OBJECT_HANDLE, GameObject *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	buildObjectData(update_data, iter->second);

    if( update_data.HasData() )
    {
	update_data.BuildPacket(&packet);
	i_player.GetSession()->SendPacket(&packet);
    }
}


void
Zone::NotifyObject::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    UpdateData update_data;
    WorldPacket packet;
    for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	buildObjectData(update_data, iter->second);

    if( update_data.HasData() )
    {
	update_data.BuildPacket(&packet);
	i_player.GetSession()->SendPacket(&packet);
    }
}

//======================================//
//         PlayerUpdater
void
Zone::PlayerUpdater::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	iter->second->Update(i_timeDiff);
}
