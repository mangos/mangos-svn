/* 
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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
#include "Map.h"
#include "MapManager.h"
#include "Transports.h"
#include "ObjectAccessor.h"

using namespace MaNGOS;

void
MaNGOS::PlayerNotifier::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        if( iter->second == &i_player )
            continue;

        iter->second->UpdateVisibilityOf(&i_player);
        i_player.UpdateVisibilityOf(iter->second);
    }
}

void
VisibleChangesNotifier::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        if(iter->second == &i_object)
            continue;

        iter->second->UpdateVisibilityOf(&i_object);
    }
}

void
VisibleNotifier::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        if( iter->second == &i_player )
            continue;

        iter->second->UpdateVisibilityOf(&i_player);
        i_player.UpdateVisibilityOf(iter->second,i_data,i_data_updates);
        i_clientGUIDs.erase(iter->second->GetGUID());
    }
}

void
VisibleNotifier::Notify()
{
    // generate outOfRange for not iterate objects
    i_data.AddOutOfRangeGUID(i_clientGUIDs);
    for(Player::ClientGUIDs::iterator itr = i_clientGUIDs.begin();itr!=i_clientGUIDs.end();++itr)
    {
        i_player.m_clientGUIDs.erase(*itr);

        #ifdef MANGOS_DEBUG
        if((sLog.getLogFilter() & LOG_FILTER_VISIBILITY_CHANGES)==0)
            sLog.outDebug("Object %u (Type: %u) is out of range (no in active cells set) now for player %u",GUID_LOPART(*itr),GuidHigh2TypeId(GUID_HIPART(*itr)),i_player.GetGUIDLow());
        #endif
    }

    // send update to other players (except player updates that already sent using SendUpdateToPlayer)
    for(UpdateDataMapType::iterator iter = i_data_updates.begin(); iter != i_data_updates.end(); ++iter)
    {
        if(iter->first==&i_player)
            continue;

        WorldPacket packet;
        iter->second.BuildPacket(&packet);
        iter->first->GetSession()->SendPacket(&packet);
    }

    if( i_data.HasData() )
    {
        // send create/outofrange packet to player (except player create updates that already sent using SendUpdateToPlayer)
        WorldPacket packet;
        i_data.BuildPacket(&packet);
        i_player.GetSession()->SendPacket(&packet);

        // send out of range to other players if need
        std::set<uint64> const& oor = i_data.GetOutOfRangeGUIDs();
        for(std::set<uint64>::const_iterator iter = oor.begin(); iter != oor.end(); ++iter)
        {
            if(GUID_HIPART(*iter)!=HIGHGUID_PLAYER)
                continue;

            Player* plr = ObjectAccessor::Instance().GetPlayer(i_player,*iter);
            if(plr)
                plr->UpdateVisibilityOf(&i_player);
        }
    }
}

void
MessageDeliverer::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        if( (iter->second != &i_player || i_toSelf)
            && (!i_ownTeamOnly || iter->second->GetTeam() == i_player.GetTeam()) )
        {
            if(WorldSession* session = iter->second->GetSession())
                session->SendPacket(i_message);
        }
    }
}

void
ObjectMessageDeliverer::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        if(WorldSession* session = iter->second->GetSession())
            session->SendPacket(i_message);
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

template<class T> void
ObjectUpdater::Visit(std::map<OBJECT_HANDLE, CountedPtr<T> > &m)
{
    for(typename std::map<OBJECT_HANDLE, CountedPtr<T> >::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        iter->second->Update(i_timeDiff);
    }
}

template void ObjectUpdater::Visit<GameObject>(GameObjectMapType &);
template void ObjectUpdater::Visit<DynamicObject>(DynamicObjectMapType &);
