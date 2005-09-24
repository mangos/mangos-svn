
#include "MapManager.h"
#include "ZoneSearcher.h"

MapManager::MapManager() : i_zoneState(0)
{
}

MapManager::~MapManager()
{
  i_zoneState.reset();

  for(ZoneMapType::iterator iter=i_zones.begin(); iter != i_zones.end(); ++iter)
      delete iter->second;

  i_zones.clear();
}

void
MapManager::EnterMap(Player *pl)
{
  PlayerEnterLocation(pl, pl->GetZoneId(), pl->GetPositionX(), pl->GetPositionY());
}

void
MapManager::PlayerEnterLocation(Player *player, const uint32 &zone_id, const float &x, const float &y)
{
  ZoneMapType::iterator iter(i_zones.end());
  if( !i_zoneState.test(zone_id) )
    {    
      Guard guard(*this); // <== inefficient... need to fix this (should be zone specific lock
      if( !i_zoneState.test(zone_id) )
	{
	  // finds the zone coordinates and creates the corresponding zone.
	  ZoneMapPair p = i_zones.insert( ZoneMapType::value_type(zone_id, ZoneSearcher<AllZoneEnums>::Create(x, y)) );
	  i_zoneState.set(zone_id, 1);
	  iter = p.first;
	}
    }
  else
    {
      iter = i_zones.find(zone_id);
    }

  assert( iter != i_zones.end() );
  iter->second->AddPlayer(player);
}

void
MapManager::Update(uint32 diff)
{
  for(ZoneMapType::iterator iter=i_zones.begin(); iter != i_zones.end(); ++iter)
    iter->second->Update(diff);
}
