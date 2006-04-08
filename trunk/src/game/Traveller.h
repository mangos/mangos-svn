/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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

#ifndef MANGOS_TRAVELLER_H
#define MANGOS_TRAVELLER_H

#include "MapManager.h"
#include <cassert>

/** Traveller is a wrapper for units (creatures or players) that
 * travel from point A to point B using the destination holder.
 */
#define UNIT_WALK_SPEED            2.5f
#define UNIT_RUN_SPEED             7.0f
#define UNIT_SWIM_SPEED            7.0f
#define PLAYER_FLIGHT_SPEED        32.0f
template<class T>
struct MANGOS_DLL_DECL Traveller
{
    T &i_traveller;
    Traveller(T &t) : i_traveller(t) {}
    Traveller(const Traveller &obj) : i_traveller(obj) {}
    Traveller& operator=(const Traveller &obj)
    {
        this->~Traveller();
        new (this) Traveller(obj);
        return *this;
    }

    operator T&(void) { return i_traveller; }
    operator const T&(void) { return i_traveller; }
    inline const float &GetPositionX(void) const { return i_traveller.GetPositionX(); }
    inline const float &GetPositionY(void) const { return i_traveller.GetPositionY(); }
    inline const float &GetPositionZ(void) const { return i_traveller.GetPositionZ(); }
    inline T& GetTraveller(void) { return i_traveller; }

    float Speed(void) { assert(false); }
    void Relocation(const float &x, const float &y, const float &z, const float &orientation) {}
    void Relocation(const float &x, const float &y, const float &z) { Relocation(x, y, z, i_traveller.GetOrientation()); }
    void MoveTo(const float &x, const float &y, const float &z, const uint32 &t) {}
};

// specializetion for creatures
template<>
inline float Traveller<Creature>::Speed()
{
    return i_traveller.GetMobSpeed() * (i_traveller.getMoveRunFlag() ? UNIT_RUN_SPEED : UNIT_WALK_SPEED);
}

template<>
inline void Traveller<Creature>::Relocation(const float &x, const float &y, const float &z, const float &orientation)
{
    MapManager::Instance().GetMap(i_traveller.GetMapId())->CreatureRelocation(&i_traveller, x, y, z, orientation);
}

template<>
inline void Traveller<Creature>::MoveTo(const float &x, const float &y, const float &z, const uint32 &t)
{
    i_traveller.AI_SendMoveToPacket(x, y, z, t, i_traveller.getMoveRunFlag());
}

// specialization for players
template<>
inline float Traveller<Player>::Speed()
{
    return PLAYER_FLIGHT_SPEED;
}

template<>
inline void Traveller<Player>::Relocation(const float &x, const float &y, const float &z, const float &orientation)
{
    MapManager::Instance().GetMap(i_traveller.GetMapId())->PlayerRelocation(&i_traveller, x, y, z, orientation);
}

typedef Traveller<Creature> CreatureTraveller;
typedef Traveller<Player> PlayerTraveller;
#endif
