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
#include "Map.h"

using namespace MaNGOS;


//======================================//
//         NotifyPlayer
/*
 * couple of things happend here
 * (1) in inform every player of my existence (if they are still in range)
 * (2) inform myself of every player that's within range
 */
void PlayerNotifier::Visit(PlayerMapType &m)
{
    return;
    Player *player = &i_player;
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	if( iter->second == player )
	    continue; // this is me dude

	WorldPacket packet;
	UpdateData update_data;

	// build my data for the other player.. 
	// and inform them
	i_player.BuildCreateUpdateBlockForPlayer(&update_data, iter->second);
	update_data.BuildPacket(&packet);
	iter->second->GetSession()->SendPacket(&packet);
	
	// collect data about other players to let myself know
	iter->second->BuildCreateUpdateBlockForPlayer(&i_data, &i_player);
    }
    
}



void
PlayerNotifier::Notify()
{
    WorldPacket packet;

    // send to myself about other player information and self
    sLog.outDetail("Creating player data for himself %d", i_player.GetGUID());
    i_player.BuildCreateUpdateBlockForPlayer(&i_data, &i_player);
    i_data.BuildPacket(&packet);
    i_player.GetSession()->SendPacket(&packet);
    i_player.AddToWorld();
}



//===============================================//
//         VisibleNotifier
void
VisibleNotifier::Notify()
{
    WorldPacket packet;

    if( i_data.HasData() )
    {
	i_data.BuildPacket(&packet);
	i_player.GetSession()->SendPacket(&packet);
    }
}


template<class T>
void
VisibleNotifier::Visit(std::map<OBJECT_HANDLE, T *> &m)
{
    for(typename std::map<OBJECT_HANDLE, T *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	iter->second->BuildCreateUpdateBlockForPlayer(&i_data, &i_player);
}



    
//===============================================//
//         NotVisibleNotifier
void
NotVisibleNotifier::Notify()
{
    WorldPacket packet;
    if( i_data.HasData() )
    {
	i_data.BuildPacket(&packet);
	i_player.GetSession()->SendPacket(&packet);
    }

}


template<class T>
void
NotVisibleNotifier::Visit(std::map<OBJECT_HANDLE, T *> &m)
{
    for(typename std::map<OBJECT_HANDLE, T *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	iter->second->BuildOutOfRangeUpdateBlock(&i_data);
}



    
//=====================================//
//     ObjectVisibleNotifer
void
ObjectVisibleNotifier::Visit(PlayerMapType &m)
{
    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	UpdateData update_data;
	WorldPacket packet;   
	i_object.BuildCreateUpdateBlockForPlayer(&update_data, iter->second);
	update_data.BuildPacket(&packet);
	iter->second->GetSession()->SendPacket(&packet);
    }    
}    

//=====================================//
//     ObjectVisibleNotifer
void
ObjectNotVisibleNotifier::Visit(PlayerMapType &m)
{
    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	UpdateData update_data;
	WorldPacket packet;   
	i_object.BuildOutOfRangeUpdateBlock(&update_data);
	update_data.BuildPacket(&packet);
	iter->second->GetSession()->SendPacket(&packet);
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
      if( iter->second != player || i_toSelf )
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
    uint64 guid = i_object.GetGUID();
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	iter->second->GetSession()->SendPacket(i_message);
    }
}

//=========================================//
//  CreatureVisibleMovementNotifier
void
CreatureVisibleMovementNotifier::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	if( iter->second->isAlive() )
	{
	    UpdateData update_data;
	    WorldPacket packet;   
	    i_creature.BuildCreateUpdateBlockForPlayer(&update_data, iter->second);
	    update_data.BuildPacket(&packet);
	    iter->second->GetSession()->SendPacket(&packet);
	}
    }
}


//=========================================//
//  CreatureNotVisibleMovementNotifier
void
CreatureNotVisibleMovementNotifier::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	if( iter->second->isAlive() )
	{
	    UpdateData update_data;
	    WorldPacket packet;   
	    i_creature.BuildOutOfRangeUpdateBlock(&update_data);
	    update_data.BuildPacket(&packet);
	    iter->second->GetSession()->SendPacket(&packet);
	}
    }
}


//===================================//
//        ObjectUpdater
template<class T> void 
ObjectUpdater::Visit(std::map<OBJECT_HANDLE, T *> &m) 
{
    for(typename std::map<OBJECT_HANDLE, T*>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	iter->second->Update(i_timeDiff);
    }
}

// specialization....
template void VisibleNotifier::Visit<GameObject>(std::map<OBJECT_HANDLE, GameObject *> &);
template void VisibleNotifier::Visit<Corpse>(std::map<OBJECT_HANDLE, Corpse *> &);
template void VisibleNotifier::Visit<DynamicObject>(std::map<OBJECT_HANDLE, DynamicObject *> &);

template void NotVisibleNotifier::Visit<GameObject>(std::map<OBJECT_HANDLE, GameObject *> &);
template void NotVisibleNotifier::Visit<Corpse>(std::map<OBJECT_HANDLE, Corpse *> &);
template void NotVisibleNotifier::Visit<DynamicObject>(std::map<OBJECT_HANDLE, DynamicObject *> &);

template void ObjectUpdater::Visit<GameObject>(std::map<OBJECT_HANDLE, GameObject *> &);
template void ObjectUpdater::Visit<Corpse>(std::map<OBJECT_HANDLE, Corpse *> &);
template void ObjectUpdater::Visit<DynamicObject>(std::map<OBJECT_HANDLE, DynamicObject *> &);


