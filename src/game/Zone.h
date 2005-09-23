/* Zone.h
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

#ifndef MANGOS_ZONE_H
#define MANGOS_ZONE_H

/** Zone represents an area in the world of MaNGOS.  There are numerous zones
    and all zones are further separated into Grids.  A grid can be either local
    or remotely managed.  
 */

#include "Platform/Define.h"
#include "Policies/ThreadingModel.h"
#include "zthread/Mutex.h"
#include "ObjectGridLoader.h"

class MANGOS_DLL_DECL Zone : public MaNGOS::ObjectLevelLockable<Zone, ZThread::Mutex>
{
    /** NotifyPlayer class is responsible to inform all players in the grid
	that's within range the existence of the player.  As well, inform
	the player himself the existence of other players
     */
    struct NotifyPlayer
    {
	NotifyPlayer(GridType &grid, Player &pl) : i_grid(grid), i_player(pl) {}
	void Visit(std::map<OBJECT_HANDLE, Player *> &);
	GridType &i_grid;
	Player &i_player;
    };

    /** NotifyObject class is responsible to inform all objects (that's creatures as well)
	in the grid that's within range the existence of the player.
     */
    struct NotifyObject
    {
	NotifyObject(GridType &grid, Player &pl) : i_grid(grid), i_player(pl) {}
	void buildObjectData(UpdateData &, Object *);
	void Visit(std::map<OBJECT_HANDLE, GameObject *> &);
	void Visit(std::map<OBJECT_HANDLE, Creature *> &);
	GridType &i_grid;
	Player &i_player;
    };

    /** PlayerUpdater class is responsible for updating all player and the objects
	that's within range of the player (nothing else)
     */
    struct PlayerUpdater
    {
	GridType &i_grid;
	uint32 i_timeDiff;
	PlayerUpdater(GridType &grid, uint32 diff) : i_grid(grid), i_timeDiff(diff) {}
	void Visit(std::map<OBJECT_HANDLE, Player *> &);
    };

public:
    Zone(const float y2, const float y1, const float x2, const float x1) : i_coord1(x1,y1), i_coord2(x2,y2), i_grid(NULL)	
    {
    }
    
    /** Player enters the zone
     */
    void AddPlayer(Player *);

    /** Player exits the zone
     */
    void RemovePlayer(Player *);    

    /** Updates the zone with the time difference from the last update
     */
    void Update(uint32);

private:

    typedef MaNGOS::ObjectLevelLockable<Zone, ZThread::Mutex>::Lock Guard;
    Coordinate i_coord1;
    Coordinate i_coord2;
    
    // for now one grid...
    GridType *i_grid;
};



#endif
