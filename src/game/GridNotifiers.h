/* GridNotifiers.h
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

#ifndef MANGOS_GRIDNOTIFIERS_H
#define MANGOS_GRIDNOTIFIERS_H

/** @page Grid System Overview
    The Grid System (TGS) in MaNGOS uses mainly the visitor mixture with
    template pattern.  It allows object of interested to keep track of nearly
    objects (not necessary of the same types).  An object moving into or
    out of the grid has to be notify the grid (the grid itself does
    not keep track of the object movements.. only the objects themself do.
 */

#include "ObjectGridLoader.h"
#include "ByteBuffer.h"
#include "UpdateData.h"

class Player;

namespace MaNGOS
{
    /** PlayerNotifier class is responsible to inform all players in the grid
	that's within range the existence of the player.  As well, inform
	the player himself the existence of other players. It also grap
	the objects within its visibility.
     */
    struct MANGOS_DLL_DECL PlayerNotifier
    {
	PlayerNotifier(GridType &grid, Player &pl) : i_grid(grid), i_player(pl) {}
	void Visit(PlayerMapType &);
	void Visit(GameObjectMapType &);
	void Visit(CreatureMapType &);
	void Visit(DynamicObjectMapType &);
	void Visit(CorpseMapType &);
	GridType &i_grid;
	Player &i_player;
    };

    /** ExitNotifier notifies player's exit status...
     */
    struct MANGOS_DLL_DECL ExitNotifier
    {
	ExitNotifier(GridType &grid, Player &pl) : i_grid(grid), i_player(pl) {}
	void Visit(PlayerMapType &);
	GridType& i_grid;
	Player &i_player;
    };

    /** InRangeRemove removes the objects in range of a player due to the
     * object's lifetime is up
     */
    struct MANGOS_DLL_DECL InRangeRemover
    {
	InRangeRemover(GridType& grid, Object &obj) : i_grid(grid), i_object(obj) {}
	void Visit(PlayerMapType &);	
	GridType &i_grid;
	Object &i_object;
    };


    /** GridUpdater updates all object status only	
     */
    struct MANGOS_DLL_DECL GridUpdater
    {
	GridType &i_grid;
	uint32 i_timeDiff;
	GridUpdater(GridType &grid, uint32 diff) : i_grid(grid), i_timeDiff(diff) {}

	template<class T> void updateObjects(std::map<OBJECT_HANDLE, T *> &m)
	{
	    for(typename std::map<OBJECT_HANDLE, T*>::iterator iter=m.begin(); iter != m.end(); ++iter)
		iter->second->Update(i_timeDiff);
	}

	void Visit(PlayerMapType &m) { updateObjects<Player>(m); }
	void Visit(CreatureMapType &m){ updateObjects<Creature>(m); }
	void Visit(GameObjectMapType &m) { updateObjects<GameObject>(m); }
	void Visit(DynamicObjectMapType &m) { updateObjects<DynamicObject>(m); }
	void Visit(CorpseMapType &m) { updateObjects<Corpse>(m); }
    };

    /** PlayerUpdatePacket pretty much do what PlayerRelocation except for all players
     */
    struct MANGOS_DLL_DECL PlayerUpdatePacket
    {
	GridType &i_grid;
	PlayerUpdatePacket(GridType &grid) : i_grid(grid) {}
	void Visit(PlayerMapType &);
    };

    /** ObjectRemoverNotifer is responsible for update the grid status when an
     * object is removed from the 
     */
    struct MANGOS_DLL_DECL ObjectRemoverNotifier
    {
	Player &i_player;
	ObjectRemoverNotifier(Player &pl) : i_player(pl) {}
	void Visit(std::map<OBJECT_HANDLE, GameObject *> &m);
	void Visit(std::map<OBJECT_HANDLE, Creature *> &m);
	void Visit(std::map<OBJECT_HANDLE, Player *> &m);
	void Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m);
	void Visit(std::map<OBJECT_HANDLE, Corpse *> &m);	
    };

    /** MessageDeliverer delivers a certain incomming message to all players
     */
    struct MANGOS_DLL_DECL MessageDeliverer
    {
	Player &i_player;
	WorldPacket *i_message;
	MessageDeliverer(Player &pl, WorldPacket *msg) : i_player(pl), i_message(msg) {}
	void Visit(PlayerMapType &m);
    };

    /** ObjectMessageDeliverer is a message from unit or object (a non player to his in range 
     * players
     */
    struct MANGOS_DLL_DECL ObjectMessageDeliverer
    {
	Object &i_object;
	WorldPacket *i_message;
	ObjectMessageDeliverer(Object &obj, WorldPacket *msg) : i_object(obj), i_message(msg) {}
	void Visit(PlayerMapType &m);
    };
    
    /** PositionRelocationNotifier informs the map manager that certain object
     * moved it position. Its up to the MapManager to ensure that the
     * object is still within its zone.  Its up to the zone that the object is within its
     * grid. etc..
     */
    struct MANGOS_DLL_DECL PlayerRelocationNotifier
    {
	Player &i_player;
	UpdateData i_data;
	PlayerRelocationNotifier(Player& player) : i_player(player) {}
	~PlayerRelocationNotifier();
	void Visit(PlayerMapType &);
	template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &);	
    };

    /** ObjectEnterNotifer notifies the players an brand new object has arrived.
     * put them in arange to the player if it is.  This likely due to spawn.
     */
    template<class T>
    struct MANGOS_DLL_DECL ObjectEnterNotifier
    {
	T& i_object;
	GridType &i_grid;
	ObjectEnterNotifier(GridType &grid, T& obj) : i_grid(grid), i_object(obj) {}
	void Visit(PlayerMapType &);
    };

}

#endif
