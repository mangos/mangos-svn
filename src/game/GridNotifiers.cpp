/* GridNotifiers.cpp
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



#include "GridNotifiers.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "Item.h"

using namespace MaNGOS;

// VISIBILITY_RANGE = UPDATE_DISTANCE*UPDATE_DISTANCE = 155.8*155.8 = 24274
#define VISIBILITY_RANGE    24274

/* common methods used by all.  This method is simple,
 * for a given object and player, if the object is in range,
 * build an update package otherwise if the object is not in
 * range but was in range, pretty much means the object just
 * move out of sight, so build an out of range packet.  Note,
 * this builds the object's data for the player NOT player's
 * data for the object.
 */
static
void buildObjectData(Player &player, UpdateData &update_data, Object *obj)
{
    if( obj->GetDistance2dSq(&player) <= VISIBILITY_RANGE )
    {
	sLog.outDetail("Creating object %d for player %d", obj->GetGUID(), player.GetGUID());
	obj->BuildCreateUpdateBlockForPlayer(&update_data, &player);
#ifdef ENABLE_GRID_SYSTEM
	player.AddInRangeObject(obj);
    }       
    else if( player.RemoveInRangeObject(obj) )
    {
	obj->BuildOutOfRangeUpdateBlock(&update_data);
#endif
    }
}

//======================================//
//         NotifyPlayer
/*
 * couple of things happend here
 * (1) in inform every player of my existence (if they are still in range)
 * (2) inform myself of every player that's within range
 */
void
PlayerNotifier::Visit(PlayerMapType &m)
{
    WorldPacket my_packet;
    UpdateData my_data;

    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	assert( iter->second != &i_player );
	WorldPacket packet;
	UpdateData player_data;

	// build my data for the other player.. 
	// and inform them
	buildObjectData(*iter->second, player_data, &i_player);
	player_data.BuildPacket(&packet);
	iter->second->GetSession()->SendPacket(&packet);
	
	// collect data about other players to let myself know
	buildObjectData(i_player, my_data, iter->second);
    }

    // send to myself about other player information and self
    // note, don't need to build about self data since already handle in there
    sLog.outDetail("Creating player data for himself %d", i_player.GetGUID());
    my_data.BuildPacket(&my_packet);
    i_player.GetSession()->SendPacket(&my_packet);
    i_player.AddToWorld();
}


// do we handle GameObjects differently than creatures (yes.. but not now)
// after re-implement Creature and GameObject class.
void
PlayerNotifier::Visit(GameObjectMapType &m)
{
    UpdateData update_data;
    WorldPacket packet;
    for(std::map<OBJECT_HANDLE, GameObject *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	buildObjectData(i_player, update_data, iter->second);
    
    if( update_data.HasData() )
    {
	update_data.BuildPacket(&packet);
	i_player.GetSession()->SendPacket(&packet);
    }
}


void
PlayerNotifier::Visit(CreatureMapType &m)
{
    UpdateData update_data;
    WorldPacket packet;
    for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	buildObjectData(i_player, update_data, iter->second);

    if( update_data.HasData() )
    {
	update_data.BuildPacket(&packet);
	i_player.GetSession()->SendPacket(&packet);
    }
}

void
PlayerNotifier::Visit(DynamicObjectMapType &m)
{
    UpdateData update_data;
    WorldPacket packet;
    for(std::map<OBJECT_HANDLE, DynamicObject *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	buildObjectData(i_player, update_data, iter->second);

    if( update_data.HasData() )
    {
	update_data.BuildPacket(&packet);
	i_player.GetSession()->SendPacket(&packet);
    }
}


//=====================================//
//         MessageDeliverer
void
MessageDeliverer::Visit(PlayerMapType &m)
{
  Player *player = &i_player;
  for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
  {
      if( iter->second != player &&  iter->second->GetDistance2dSq(player) <= VISIBILITY_RANGE )
      {
	  iter->second->GetSession()->SendPacket(i_message);
      }
  }
}

//========================================//
//       ExitNotifier
void
ExitNotifier::Visit(PlayerMapType &m)
{
#ifdef ENABLE_GRID_SYSTEM
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	if( i_player.RemoveInRangeObject(iter->second) )
	{
	    WorldPacket packet;
	    UpdateData update_data;    
	    i_player->BuildOutOfRangeUpdateBlock(&update_data);
	    update_data.BuildPacket(&packet);
	    iter->second->GetSession()->SendPacket(&packet);
	}
    }
#endif
}

//========================================//
//        InRangeRemove
void
InRangeRemover::Visit(PlayerMapType &m)
{
#ifdef ENABLE_GRID_SYSTEM
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter
	    iter->second->RemoveInRangeObject(i_object);
#endif    
}
