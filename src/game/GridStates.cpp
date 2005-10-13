

#include "GridStates.h"
#include "GridNotifiers.h"
#include "ObjectAccessor.h"
#include "GameSystem/Grid.h"
#include "Log.h"

void
InvalidState::Update(Map &, GridType &, GridInfo &, const uint32 &x, const uint32 &y, const uint32 &) const
{
}

void
ActiveState::Update(Map &m, GridType &grid, GridInfo & info, const uint32 &x, const uint32 &y, const uint32 &t_diff) const
{
    if( grid.ObjectsInGrid() == 0 )
	grid.SetGridState(GRID_STATE_IDLE);
    else
    {
	GridReadGuard guard(info.i_lock);
	MaNGOS::GridUpdater updater(grid, t_diff);
	TypeContainerVisitor<MaNGOS::GridUpdater, ContainerMapList<Player> > player_notifier(updater);
	grid.VisitObjects(player_notifier);
	TypeContainerVisitor<MaNGOS::GridUpdater, TypeMapContainer<AllObjectTypes> > object_notifier(updater);
	grid.VisitGridObjects(object_notifier);	
    }
}

void
IdleState::Update(Map &m, GridType &grid, GridInfo &info, const uint32 &x, const uint32 &y, const uint32 &) const
{
    m.ResetGridExpiry(info);
    grid.SetGridState(GRID_STATE_REMOVAL);
    sLog.outDebug("Grid[%d,%d] on map %d moved to IDLE state", x, y, m.GetId());
}

void
RemovalState::Update(Map &m, GridType &grid, GridInfo &info, const uint32 &x, const uint32 &y, const uint32 &t_diff) const
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
