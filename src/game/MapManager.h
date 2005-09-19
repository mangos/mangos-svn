/* MapManager.h
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

#ifndef MANGOS_MAPMANAGER_H
#define MANGOS_MAPMANAGER_H

/*
 * @class MapManager
 * MapMaanger manages the area in the game.  The world in MaNGOS is devided into
 * zones and each zone further devided into Grids. Each zone and hences grids
 * are all managed by the MapManager.
 */ 

#include "TypeList.h"
#include "Grid.h"
#include "GridLoader.h"
#include "Creature.h"
#include "GameObject.h"
#include "Player.h"

typedef TYPE_LIST_2(Creature, GameObject) AllObjectTypes;
typedef TYPE_LIST_2(CreatureLoader, GameObjectLoader) AllLoaders;

template<unsigned int T>
class Zone
{
public:
  Zone();
  ~Zone();

private:
  std::bitset<T> i_gridStatus;
  Grid<Player, AllObjectTypes> **i_Grids;
  ContainerList<AllLoaders> i_loaders;
};

class MapManager : public Singleton<MapManager>
{
public:
  friend class MaNGOS::OperatorNew<MapManager>;

  /// player enters the map.
  inline void EnterMap(Player *pl) { PlayerEnterLocation(pl, pl.GetZoneId(), pl.GetPositionX(), pl.GetPositionY()); }

  /// Player exit a location in the map
  void ExitMap(Player *pl);

private:
  MapManager();
  void PlayerEnterLocation(Player *, const unit32 &zone_id, const float &x, const float &y);

  typedef std::vector<Zone<MAX_GRIDS_PER_ZONE> > ZonesType;
  ZonesType i_zones;
};

// zone stuff
template<unsigned int T>
Zone<T>::Zone()
{
}


#endif
