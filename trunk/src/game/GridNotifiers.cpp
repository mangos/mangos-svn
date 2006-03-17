/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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





void PlayerNotifier::Visit(PlayerMapType &m)
{
    
    BuildForMySelf();

    UpdateData my_data;
    Player *player = &i_player;
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	if( iter->second == player )
	    continue; 

	WorldPacket packet;
	UpdateData update_data;

	
	
	i_player.BuildCreateUpdateBlockForPlayer(&update_data, iter->second);
	update_data.BuildPacket(&packet);
	iter->second->GetSession()->SendPacket(&packet);
	
	
	iter->second->BuildCreateUpdateBlockForPlayer(&my_data, &i_player);
    }

    if( my_data.HasData() )
    {
	WorldPacket my_packet;
	my_data.BuildPacket(&my_packet);
	i_player.GetSession()->SendPacket(&my_packet);
    }
    
    
}

void
PlayerNotifier::BuildForMySelf()
{
    WorldPacket packet;
    UpdateData data;

	if( !i_player.IsInWorld() )  
	{
        sLog.outDetail("Creating player data for himself %d", i_player.GetGUID());
        i_player.BuildCreateUpdateBlockForPlayer(&data, &i_player);
        data.BuildPacket(&packet);
        i_player.GetSession()->SendPacket(&packet);
        i_player.AddToWorld();
	}
}





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



void
ObjectMessageDeliverer::Visit(PlayerMapType &m)
{
    uint64 guid = i_object.GetGUID();
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	iter->second->GetSession()->SendPacket(i_message);
    }
}



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




template<class T> void 
ObjectUpdater::Visit(std::map<OBJECT_HANDLE, T *> &m) 
{
    for(typename std::map<OBJECT_HANDLE, T*>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
	iter->second->Update(i_timeDiff);
    }
}


template void VisibleNotifier::Visit<GameObject>(std::map<OBJECT_HANDLE, GameObject *> &);
template void VisibleNotifier::Visit<Corpse>(std::map<OBJECT_HANDLE, Corpse *> &);
template void VisibleNotifier::Visit<DynamicObject>(std::map<OBJECT_HANDLE, DynamicObject *> &);

template void NotVisibleNotifier::Visit<GameObject>(std::map<OBJECT_HANDLE, GameObject *> &);
template void NotVisibleNotifier::Visit<Corpse>(std::map<OBJECT_HANDLE, Corpse *> &);
template void NotVisibleNotifier::Visit<DynamicObject>(std::map<OBJECT_HANDLE, DynamicObject *> &);

template void ObjectUpdater::Visit<GameObject>(std::map<OBJECT_HANDLE, GameObject *> &);
template void ObjectUpdater::Visit<Corpse>(std::map<OBJECT_HANDLE, Corpse *> &);
template void ObjectUpdater::Visit<DynamicObject>(std::map<OBJECT_HANDLE, DynamicObject *> &);


