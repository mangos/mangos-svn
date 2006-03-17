/* GridStates.cpp
 *
 * Copyright (C) 2004 Wow Daemon
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

#include "GridStates.h"
#include "GridNotifiers.h"
#include "ObjectAccessor.h"
#include "GameSystem/Grid.h"
#include "Log.h"

void InvalidState::Update(Map &, GridType &, GridInfo &, const uint32 &x, const uint32 &y, const uint32 &) const
{
}


void ActiveState::Update(Map &m, GridType &grid, GridInfo & info, const uint32 &x, const uint32 &y, const uint32 &t_diff) const
{
    if( grid.ObjectsInGrid() == 0 )
        grid.SetGridState(GRID_STATE_IDLE);
    else
    {
        MaNGOS::GridUpdater updater(grid, t_diff);
        TypeContainerVisitor<MaNGOS::GridUpdater, ContainerMapList<Player> > player_notifier(updater);
        grid.VisitObjects(player_notifier);
        TypeContainerVisitor<MaNGOS::GridUpdater, TypeMapContainer<AllObjectTypes> > object_notifier(updater);
        grid.VisitGridObjects(object_notifier);
    }
}


void IdleState::Update(Map &m, GridType &grid, GridInfo &info, const uint32 &x, const uint32 &y, const uint32 &) const
{
    m.ResetGridExpiry(info);
    grid.SetGridState(GRID_STATE_REMOVAL);
    sLog.outDebug("Grid[%d,%d] on map %d moved to IDLE state", x, y, m.GetId());
}


void RemovalState::Update(Map &m, GridType &grid, GridInfo &info, const uint32 &x, const uint32 &y, const uint32 &t_diff) const
{
    info.i_timer.Update(t_diff);
    if( info.i_timer.Passed() )
    {
        if( !m.UnloadGrid(x, y) )
        {
            sLog.outDebug("Grid[%d,%d] for map %d differed unloading due to players nearby", x, y, m.GetId());
            m.ResetGridExpiry(info);
        }
    }
}
