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
#include "Utilities.h"

using namespace MaNGOS;


/* common methods used by all.  This method is simple,
 * for a given object and player, if the object is in range,
 * build an update package otherwise if the object is not in
 * range but was in range, pretty much means the object just
 * move out of sight, so build an out of range packet.  Note,
 * this builds the object's data for the player NOT player's
 * data for the object.
 */
static
void buildUnitData(Player &player, UpdateData &update_data, Unit *obj)
{
#ifdef ENABLE_GRID_SYSTEM
    if( obj->IsInWorld() )
    {
	if( Utilities::is_in_range(obj, &player) )
	{
	    //sLog.outDetail("Creating object %d for player %d", obj->GetGUID(), player.GetGUID());
	    obj->BuildCreateUpdateBlockForPlayer(&update_data, &player);
	    player.AddInRangeObject(obj);
	}       
	else if( player.RemoveInRangeObject(obj) )
	{
	    obj->BuildOutOfRangeUpdateBlock(&update_data);
	}
    }
#endif
}

static
void buildObjectData(Player &player, UpdateData &update_data, Object *obj)
{
    if( Utilities::is_in_range(obj, &player) )
    {
	//sLog.outDetail("Creating object %d for player %d", obj->GetGUID(), player.GetGUID());
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
	if( iter->second == &i_player )
	    continue; // this is me dude

	WorldPacket packet;
	UpdateData player_data;

	// build my data for the other player.. 
	// and inform them
	buildUnitData(*iter->second, player_data, &i_player);
	player_data.BuildPacket(&packet);
	iter->second->GetSession()->SendPacket(&packet);
	
	// collect data about other players to let myself know
	buildUnitData(i_player, my_data, iter->second);
    }

    // send to myself about other player information and self
    sLog.outDetail("Creating player data for himself %d", i_player.GetGUID());
    i_player.BuildCreateUpdateBlockForPlayer(&my_data, &i_player);
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
	buildUnitData(i_player, update_data, iter->second);

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

void
PlayerNotifier::Visit(CorpseMapType &m)
{
    UpdateData update_data;
    WorldPacket packet;
    for(CorpseMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
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
      if( iter->second != player &&  Utilities::is_in_range(iter->second, player) )
      {
	  iter->second->GetSession()->SendPacket(i_message);
      }
  }
}

//========================================//
//        UnitMessageDeliverer
void
ObjectMessageDeliverer::Visit(PlayerMapType &m)
{
#ifdef ENABLE_GRID_SYSTEM
    uint64 guid = i_object.GetGUID();
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	if( iter->second->isInRangeUnit(guid) || iter->second->isInRangeObject(guid) )
	    iter->second->GetSession()->SendPacket(i_message);
    }
#endif
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
	    i_player.BuildOutOfRangeUpdateBlock(&update_data);
	    update_data.BuildPacket(&packet);
	    iter->second->GetSession()->SendPacket(&packet);
	}
    }
#endif
}

//========================================//
//       InRangeRemover
void
InRangeRemover::Visit(PlayerMapType &m)
{
#ifdef ENABLE_GRID_SYSTEM
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	iter->second->RemoveInRangeObject(&i_object);
	i_object.DestroyForPlayer(iter->second);
    }
#endif    
}

//============================================//
//       PlayerRelocation
void
PlayerRelocationNotifier::Visit(PlayerMapType  &m)
{
#ifdef ENABLE_GRID_SYSTEM
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	if( iter->second == &i_player )
	    continue; // its myself

	// for each of my in range player if they are out of range.. 
	// tell them I am out of range
	if( i_player.isInRangeUnit(iter->second->GetGUID()) )
	{
	    // let's check to see if he's still in my range after the relocation?
	    if( !MaNGOS::Utilities::is_in_range(&i_player, iter->second) )
	    {
		// ok.. we moved out of range
		iter->second->BuildOutOfRangeUpdateBlock(&i_data);
		iter->second->MoveOutOfRange(i_player);
	    }

	    // else player still in range so...
	}
	else if( Utilities::is_in_range(iter->second, &i_player) )
	{
	    // Ok.. this guys wasn't in my range before but now moved in my range
	    sLog.outDebug("Creating in range packet for both player %d and %d", i_player.GetGUID(), iter->second->GetGUID());

	    // build this guy for me
	    iter->second->BuildCreateUpdateBlockForPlayer(&i_data, &i_player);
	    i_player.AddInRangeObject(iter->second);
	    iter->second->MoveInRange(i_player);
	}
    }
#endif
}


template<class T>
void
PlayerRelocationNotifier::Visit(std::map<OBJECT_HANDLE, T*> &m)
{
#ifdef ENABLE_GRID_SYSTEM
    for(typename std::map<OBJECT_HANDLE, T*>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	if( iter->second->IsInWorld() )
	{
	    if( i_player.isInRange(iter->second) )
	    {
		// let's check to see if the object moved out of my range after relocation.
		if( !MaNGOS::Utilities::is_in_range(&i_player, iter->second) )
		{
		    // build out of range for this object
		    iter->second->BuildOutOfRangeUpdateBlock(&i_data);		
		    i_player.RemoveInRangeObject(iter->second);
		}
	    }
	    else if( MaNGOS::Utilities::is_in_range(&i_player, iter->second) )
	    {
		// new object moved in range... due to player movement
		// build in range packet and put it in my pocket..
		iter->second->BuildCreateUpdateBlockForPlayer(&i_data, &i_player);
		i_player.AddInRangeObject(iter->second);
	    }
	}
    }
#endif
}

PlayerRelocationNotifier::~PlayerRelocationNotifier()
{
    if( i_data.HasData() )
    {
	WorldPacket packet;
	i_data.BuildPacket(&packet);
	i_player.GetSession()->SendPacket(&packet);
    }
}


//===============================================//
//        ObjectRelocationNotifier
template<>
void
ObjectRelocationNotifier<Creature>::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
	if( Utilities::is_in_range(&i_object, iter->second) )
	    iter->second->AddInRangeObject(&i_object);
}

template<class T>
void
ObjectRelocationNotifier<T>::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
	if( Utilities::is_in_range(&i_object, iter->second) )
	    iter->second->AddInRangeObject(&i_object);
}

//====================================================//
//           ObjectEnterNotifier
template<class T>
void
ObjectEnterNotifier<T>::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	UpdateData data;
	buildObjectData(*iter->second, data, &i_object);
	if( data.HasData() )
	{
	    WorldPacket packet;
	    data.BuildPacket(&packet);
	    iter->second->GetSession()->SendPacket(&packet);
	}
    }
}

template<> void ObjectEnterNotifier<Creature>::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	UpdateData data;
	buildUnitData(*iter->second, data, &i_object);
	if( data.HasData() )
	{
	    WorldPacket packet;
	    data.BuildPacket(&packet);
	    iter->second->GetSession()->SendPacket(&packet);
	}
    }
}


//==========================================//
//          PlayerUpdatePacket 
void
PlayerUpdatePacket::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	MaNGOS::PlayerRelocationNotifier notifier(*iter->second);
	TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, ContainerMapList<Player> > player_update(notifier);
	i_grid.VisitObjects(player_update);
	TypeContainerVisitor<MaNGOS::PlayerRelocationNotifier, TypeMapContainer<AllObjectTypes> > object_update(notifier);
	i_grid.VisitGridObjects(object_update);
    }
}



// specialization....
template void PlayerRelocationNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &);
template void PlayerRelocationNotifier::Visit(std::map<OBJECT_HANDLE, GameObject *> &);
template void PlayerRelocationNotifier::Visit(std::map<OBJECT_HANDLE, DynamicObject *> &);
template void PlayerRelocationNotifier::Visit(std::map<OBJECT_HANDLE, Corpse *> &);

template void ObjectEnterNotifier<GameObject>::Visit(PlayerMapType &);
template void ObjectEnterNotifier<Corpse>::Visit(PlayerMapType &);
template void ObjectEnterNotifier<DynamicObject>::Visit(PlayerMapType &);

template void ObjectRelocationNotifier<GameObject>::Visit(PlayerMapType &);
template void ObjectRelocationNotifier<Corpse>::Visit(PlayerMapType &);
template void ObjectRelocationNotifier<DynamicObject>::Visit(PlayerMapType &);

