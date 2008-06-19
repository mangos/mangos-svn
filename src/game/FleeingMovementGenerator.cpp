/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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

#include "Creature.h"
#include "MapManager.h"
#include "FleeingMovementGenerator.h"
#include "DestinationHolderImp.h"

template<class T>
void
FleeingMovementGenerator<T>::_setTargetLocation(T &owner)
{
    if( !&owner )
        return;

    if( owner.hasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNDED) )
        return;

    float x, y, z;

    if(_getPoint(owner, x, y, z))
    {
        i_nextCheckTime.Reset(0);
        owner.addUnitState(UNIT_STAT_FLEEING);
        owner.SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);
        Traveller<T> traveller(owner);
        uint32 totalTravelTime = i_destinationHolder.SetDestination(traveller, x, y, z);
        if(totalTravelTime > 100)
            i_nextMoveTime.Reset(totalTravelTime-100);
        else
            i_nextMoveTime.Reset(50);
    }
    else
    {
        i_nextCheckTime.Reset(500);
    }
}

template<class T>
bool
FleeingMovementGenerator<T>::_getPoint(T &owner, float &x, float &y, float &z)
{
    if(!&owner)
        return false;

    x = owner.GetPositionX();
    y = owner.GetPositionY();
    z = owner.GetPositionZ();

    float cur_dist;
    float cur_angle;

    if(i_fright)                                            // fear just applied by i_fright, run away
    {
        i_center_x = owner.GetPositionX();
        i_center_y = owner.GetPositionY();

        cur_dist = rand_norm()*flee_distance;
        cur_angle = i_fright->GetAngle(&owner);
        i_fright = NULL;
    }
    else                                                    // random target point
    {
        cur_dist = rand_norm()*flee_distance;
        cur_angle = rand_norm()*2*M_PI;
    }

    float temp_x, temp_y, angle;
    const Map * _map = MapManager::Instance().GetBaseMap(owner.GetMapId());
    //primitive path-finding
    for(uint8 i = 0; i < 18; i++)
    {
        float distance = cur_dist;

        if(distance < 1.0f)
            return false;
        else if(distance > 5.0f)
            distance = 5.0f;

        switch(i)
        {
            case 0:
                angle = cur_angle;
                break;
            case 1:
                angle = cur_angle;
                distance /= 2;
                break;
            case 2:
                angle = cur_angle;
                distance /= 4;
                break;
            case 3:
                angle = cur_angle + M_PI/4.0f;
                break;
            case 4:
                angle = cur_angle - M_PI/4.0f;
                break;
            case 5:
                angle = cur_angle + M_PI/4.0f;
                distance /= 2;
                break;
            case 6:
                angle = cur_angle - M_PI/4.0f;
                distance /= 2;
                break;
            case 7:
                angle = cur_angle + M_PI/2.0f;
                break;
            case 8:
                angle = cur_angle - M_PI/2.0f;
                break;
            case 9:
                angle = cur_angle + M_PI/2.0f;
                distance /= 2;
                break;
            case 10:
                angle = cur_angle + M_PI/2.0f;
                distance /= 2;
                break;
            case 11:
                angle = cur_angle + M_PI/4.0f;
                distance /= 4;
                break;
            case 12:
                angle = cur_angle - M_PI/4.0f;
                distance /= 4;
                break;
            case 13:
                angle = cur_angle + M_PI/2.0f;
                distance /= 4;
                break;
            case 14:
                angle = cur_angle - M_PI/2.0f;
                distance /= 4;
                break;
            case 15:
                angle = cur_angle + M_PI*3/4.0f;
                distance /= 2;
                break;
            case 16:
                angle = cur_angle - M_PI*3/4.0f;
                distance /= 2;
                break;
            case 17:
                angle = cur_angle + M_PI;
                distance /= 2;
                break;
        }
        temp_x = x + distance * cos(angle);
        temp_y = y + distance * sin(angle);
        MaNGOS::NormalizeMapCoord(temp_x);
        MaNGOS::NormalizeMapCoord(temp_y);
        if( owner.IsWithinLOS(temp_x,temp_y,z) && 
            (i_center_x-temp_x)*(i_center_x-temp_x)+(i_center_y-temp_y)*(i_center_y-temp_y) <= flee_distance*flee_distance )
        {
            bool is_water_now = _map->IsInWater(x,y,z);

            if(is_water_now && _map->IsInWater(temp_x,temp_y,z))
            {
                x = temp_x;
                y = temp_y;
                return true;
            }
            float new_z = _map->GetHeight(temp_x,temp_y,z,true);

            if(new_z <= INVALID_HEIGHT)
                continue;

            bool is_water_next = _map->IsInWater(temp_x,temp_y,new_z);

            if((is_water_now && !is_water_next && !is_land_ok) || (!is_water_now && is_water_next && !is_water_ok))
                continue;

            if( !(new_z - z) || distance / fabs(new_z - z) > 1.0f)
            {
                float new_z_left = _map->GetHeight(temp_x + 1.0f*cos(angle+M_PI/2),temp_y + 1.0f*sin(angle+M_PI/2),z,true);
                float new_z_right = _map->GetHeight(temp_x + 1.0f*cos(angle-M_PI/2),temp_y + 1.0f*sin(angle-M_PI/2),z,true);
                if(fabs(new_z_left - new_z) < 1.2f && fabs(new_z_right - new_z) < 1.2f)
                {
                    x = temp_x;
                    y = temp_y;
                    z = new_z;
                    return true;
                }
            }
        }
    }
    return false;
}

template<>
void
FleeingMovementGenerator<Creature>::Initialize(Creature &owner)
{
    if(!&owner || !&i_fright)
        return;
    is_water_ok = owner.isCanSwimOrFly();
    is_land_ok  = owner.isCanWalkOrFly();
    owner.RemoveUnitMovementFlag(MOVEMENTFLAG_WALK_MODE);
    _setTargetLocation(owner);
}

template<>
void
FleeingMovementGenerator<Player>::Initialize(Player &owner)
{
    if(!&owner || !&i_fright)
        return;
    is_water_ok = true;
    is_land_ok  = true;
    owner.RemoveUnitMovementFlag(MOVEMENTFLAG_WALK_MODE);
    _setTargetLocation(owner);
}

template<class T>
void
FleeingMovementGenerator<T>::Finalize(T &owner)
{
    owner.clearUnitState(UNIT_STAT_FLEEING);
    owner.RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);
}

template<class T>
void
FleeingMovementGenerator<T>::Reset(T &owner)
{
    Initialize(owner);
}

template<class T>
bool
FleeingMovementGenerator<T>::Update(T &owner, const uint32 & time_diff)
{
    if( !&owner || !owner.isAlive() )
        return false;
    if( owner.hasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNDED) )
        return true;

    Traveller<T> traveller(owner);

    if( (owner.IsStopped() && !i_destinationHolder.HasArrived()) || !i_destinationHolder.HasDestination() )
    {
        _setTargetLocation(owner);
        return true;
    }

    i_nextMoveTime.Update(time_diff);
    i_nextCheckTime.Update(time_diff);
    if (i_destinationHolder.UpdateTraveller(traveller, time_diff, false))
    {
        i_destinationHolder.ResetUpdate(50);
        if(i_nextCheckTime.Passed() && (i_nextMoveTime.Passed() || i_destinationHolder.HasArrived()))
        {
            _setTargetLocation(owner);
            return true;
        }
    }
    return true;
}

template bool FleeingMovementGenerator<Player>::_getPoint(Player &, float &, float &, float &);
template bool FleeingMovementGenerator<Creature>::_getPoint(Creature &, float &, float &, float &);
template void FleeingMovementGenerator<Player>::_setTargetLocation(Player &);
template void FleeingMovementGenerator<Creature>::_setTargetLocation(Creature &);
template void FleeingMovementGenerator<Player>::Finalize(Player &);
template void FleeingMovementGenerator<Creature>::Finalize(Creature &);
template void FleeingMovementGenerator<Player>::Reset(Player &);
template void FleeingMovementGenerator<Creature>::Reset(Creature &);
template bool FleeingMovementGenerator<Player>::Update(Player &, const uint32 &);
template bool FleeingMovementGenerator<Creature>::Update(Creature &, const uint32 &);
