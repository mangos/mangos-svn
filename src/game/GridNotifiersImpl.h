/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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

#ifndef MANGOS_GRIDNOTIFIERSIMPL_H
#define MANGOS_GRIDNOTIFIERSIMPL_H

#include "GridNotifiers.h"
#include "WorldPacket.h"
#include "Player.h"
#include "UpdateData.h"
#include "CreatureAI.h"
#include "Utilities.h"

template<>
inline void
MaNGOS::NotVisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
	for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
		if( ( i_player.isAlive() && iter->second->isAlive()) ||
		(i_player.isDead() && iter->second->isDead()) )
			iter->second->BuildOutOfRangeUpdateBlock(&i_data);
}

template<>
inline void
MaNGOS::NotVisibleNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        if( iter->second == &i_player )
            continue;
        if( (i_player.isAlive() && iter->second->isAlive()) ||
            (i_player.isDead() && iter->second->isDead()) )
        {
            iter->second->BuildOutOfRangeUpdateBlock(&i_data);

			UpdateData his_data;
            WorldPacket his_pk;
            i_player.BuildOutOfRangeUpdateBlock(&his_data);
            his_data.BuildPacket(&his_pk);
            iter->second->GetSession()->SendPacket(&his_pk);
        }
    }
}

template<>
inline void
MaNGOS::VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
	for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	{
		if( (i_player.isAlive() && iter->second->isAlive() ) ||
		(i_player.isDead() && iter->second->isDead()))
		{
			iter->second->BuildCreateUpdateBlockForPlayer(&i_data, &i_player);
		}
		else
		{
			ObjectAccessor::Instance().RemoveCreatureFromPlayerView(&i_player, iter->second);
		}
	}
}

template<>
inline void
MaNGOS::VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
	for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
	{
		if( iter->second == &i_player )
			continue;

		if( (i_player.isAlive() && iter->second->isAlive()) ||
			(i_player.isDead() && iter->second->isDead()) )
		{
			iter->second->SendUpdateToPlayer(&i_player);
            i_player.SendUpdateToPlayer(iter->second);

		}
		else
		{
			ObjectAccessor::Instance().RemovePlayerFromPlayerView(&i_player, iter->second);
		}
	}
}

template<>
inline void
MaNGOS::ObjectUpdater::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    std::map<OBJECT_HANDLE, Creature *> tmp(m);
    for(std::map<OBJECT_HANDLE, Creature*>::iterator iter=tmp.begin(); iter != tmp.end(); ++iter)
		if(!MaNGOS::Utilities::IsSpiritHealer(iter->second))
        	iter->second->Update(i_timeDiff);
}

template<>
inline void
MaNGOS::PlayerConfrontationNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( iter->second->isAlive() && !iter->second->testStateFlag(UNIT_STAT_IN_FLIGHT) && iter->second->AI().IsVisible(&i_player) )
            iter->second->AI().MoveInLineOfSight(&i_player);
}
#endif
