
#include "MapManager.h"

MapManager::MapManager() : i_zones(MAX_ZONES)
{
  std::fill(i_zones.begin(), i_zones.end(), NULL);
}

MapManager::~MapManager()
{
  for(ZonesType::iterater iter=i_zones.begin(); iter != i_zones.end(); ++iter)
    delete *iter;
  i_zones.clear();
}

void
MapManager::PlayerEnterLocation(Player *player, const uint32 &zone_id, const float &x, const float &y)
{
  if( i_zones[zone_id] == NULL )
    {    
      Guard guard;
      if( i_zones[zone_id] == NULL )
	{
	  Zone<MAX_GRIDS_PER_ZONE> zone = new Zone<MAX_GRIDS_PER_ZONE>;
	}
    }
}
