/* ZoneDefine.h
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
#include "ZoneDefine.h"
#include "ObjectGridLoader.h"

class MANGOS_DLL_DECL Zone : public MaNGOS::ObjectLevelLockable<Zone, ZThread::Mutex>
{
public:
    Zone(const float x1, const float x2, const float y1, const float y2) : i_coord1(x1,y1), i_coord2(x2,y2), i_grid(NULL)	
    {
    }
    
    /** Player enters the zone
     */
    void AddPlayer(Player *);

    /** Player exits the zone
     */
    void RemovePlayer(Player *);    

private:

    typedef MaNGOS::ObjectLevelLockable<Zone, ZThread::Mutex>::Lock Guard;
    Coordinate i_coord1;
    Coordinate i_coord2;
    
    // for now one grid...
    GridType *i_grid;
};

#endif
