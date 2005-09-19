
#include "MapManager.h"

void
MapManager::PlayerEnterLocation(Player *player, const uint32 &zone_id, const float &x, const float &y)
{
    if( !i_zoneStatus.test(zone_id) )
    {    
      Guard guard;
      if( !i_zoneStatus.test(zone_id) )
	{
	  Zone = new Zone;
	}
    }
}
