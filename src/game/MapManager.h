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

#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "zthread/Mutex.h"
#include "Zone.h"
#include "Common.h"

#include <bitset>

// forward declaration..
class Player;

#define MAX_ZONES_ID 3500

class MANGOS_DLL_DECL MapManager : public MaNGOS::Singleton<MapManager, MaNGOS::ClassLevelLockable<MapManager, ZThread::Mutex> >
{
  /** Only allow the creation policy creates a singleton
   */
  friend class MaNGOS::OperatorNew<MapManager>;
  typedef HM_NAMESPACE::hash_map<uint32, Zone*> ZoneMapType;
  typedef std::pair<HM_NAMESPACE::hash_map<uint32, Zone*>::iterator, bool>  ZoneMapPair;

public:

  /// player enters the map.
  void EnterMap(Player *pl);

  /// Player exit a location in the map
  void ExitMap(Player *pl);

  /** Updates the stored map information given the time difference
   * from last update
   */
  void Update(uint32);

private:
  MapManager();
  ~MapManager();

  // prevent copy constructor and assignemnt operator on a singleton
  MapManager(const MapManager &);
  MapManager& operator=(const MapManager &);

  void PlayerEnterLocation(Player *, const uint32 &zone_id, const float &x, const float &y);

  typedef MaNGOS::ClassLevelLockable<MapManager, ZThread::Mutex>::Lock Guard;
  std::bitset<MAX_ZONES_ID> i_zoneState;
  ZoneMapType i_zones;

};


#endif
