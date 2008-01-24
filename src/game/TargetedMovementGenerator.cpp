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

#include "ByteBuffer.h"
#include "TargetedMovementGenerator.h"
#include "Errors.h"
#include "Creature.h"
#include "MapManager.h"
#include "DestinationHolderImp.h"

#define SMALL_ALPHA 0.05f

#include <cmath>
/*
struct StackCleaner
{
    Creature &i_creature;
    StackCleaner(Creature &creature) : i_creature(creature) {}
    void Done(void) { i_creature.StopMoving(); }
    ~StackCleaner()
    {
        i_creature->Clear();
    }
};
*/

template<class T>
void
TargetedMovementGenerator<T>::_setTargetLocation(T &owner)
{
    if( !&i_target || !&owner )
        return;

    if( owner.hasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNDED) )
        return;

    // prevent redundant micro-movement for pets, other followers.
    if(i_offset && i_target->IsWithinDistInMap(&owner,2*i_offset))
        return;

    float x, y, z;
    if(!i_offset)
    {
        // to nearest contact position
        i_target->GetContactPoint( &owner, x, y, z );
    }
    else
    {
        // to at i_offset distance from target and i_angle from target facing
        i_target->GetClosePoint(x,y,z,owner.GetObjectSize() + i_offset,i_angle);
    }

    //We don't update Mob Movement, if the difference between New destination and last destination is < BothObjectSize
    float  bothObjectSize = i_target->GetObjectSize() + owner.GetObjectSize() + CONTACT_DISTANCE;
    if( i_destinationHolder.HasDestination() && i_destinationHolder.GetDestinationDiff(x,y,z) < bothObjectSize )
        return;
    Traveller<T> traveller(owner);
    i_destinationHolder.SetDestination(traveller, x, y, z);
    owner.addUnitState(UNIT_STAT_CHASE);
}

template<class T>
void
TargetedMovementGenerator<T>::Initialize(T &owner)
{
    if(!&owner)
        return;
    owner.setMoveRunFlag(true);
    _setTargetLocation(owner);
}

template<class T>
void
TargetedMovementGenerator<T>::Finalize(T &owner)
{
    owner.clearUnitState(UNIT_STAT_CHASE);
}

template<class T>
void
TargetedMovementGenerator<T>::Reset(T &owner)
{
    Initialize(owner);
}

template<class T>
bool
TargetedMovementGenerator<T>::Update(T &owner, const uint32 & time_diff)
{
    if(!i_target.isValid())
    {
        return false;
    }
    if( !&owner || !owner.isAlive() || !&i_target )
        return true;
    if( owner.hasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNDED | UNIT_STAT_FLEEING) )
        return true;
    if( !owner.isInCombat() && !owner.hasUnitState(UNIT_STAT_FOLLOW) )
    {
        //owner.AIM_Initialize();   This case must be the one, when a creature aggroed you. By Initalized a new AI, we prevented to Ai::_stopAttack() to be executed properly.
        return false;
    }

    // prevent movement while casting spells with cast time or channel time
    if ( owner.IsNonMeleeSpellCasted(false, false,  true))
    {
        if (!owner.IsStopped())
            owner.StopMoving();
        return true;
    }

    // prevent crash after creature killed pet
    if (!owner.hasUnitState(UNIT_STAT_FOLLOW) && owner.getVictim() != i_target.getTarget())
        return true;

    Traveller<T> traveller(owner);

    if( !i_destinationHolder.HasDestination() )
        _setTargetLocation(owner);
    if( owner.IsStopped() && !i_destinationHolder.HasArrived() )
    {
        owner.addUnitState(UNIT_STAT_CHASE);
        i_destinationHolder.StartTravel(traveller);
        return true;
    }

    if (i_destinationHolder.UpdateTraveller(traveller, time_diff, false))
    {
        // put targeted movement generators on a higher priority
        if (owner.GetObjectSize())
            i_destinationHolder.ResetUpdate(50);

        float dist = i_target->GetObjectSize() + owner.GetObjectSize() + CONTACT_DISTANCE;

        // try to counter precision differences
        if( i_destinationHolder.GetDistanceFromDestSq(*i_target.getTarget()) > dist * dist + 0.8)
        {
            owner.SetInFront(i_target.getTarget());         // Set new Angle For Map::
            _setTargetLocation(owner);                      //Calculate New Dest and Send data To Player
        }
        // Update the Angle of the target only for Map::, no need to send packet for player
        else if ( !i_angle && !owner.HasInArc( 0.01f, i_target.getTarget() ) )
            owner.SetInFront(i_target.getTarget());

        if(( owner.IsStopped() && !i_destinationHolder.HasArrived() ) || i_recalculateTravel )
        {
            i_recalculateTravel = false;
            //Angle update will take place into owner.StopMoving()
            owner.SetInFront(i_target.getTarget());

            owner.StopMoving();
            if(owner.canReachWithAttack(i_target.getTarget()) && !owner.hasUnitState(UNIT_STAT_FOLLOW))
                owner.Attack(i_target.getTarget());
        }
    }
    return true;
}

template<class T>
Unit*
TargetedMovementGenerator<T>::GetTarget() const
{
    return i_target.getTarget();
}

template void TargetedMovementGenerator<Player>::_setTargetLocation(Player &);
template void TargetedMovementGenerator<Creature>::_setTargetLocation(Creature &);
template void TargetedMovementGenerator<Player>::Initialize(Player &);
template void TargetedMovementGenerator<Creature>::Initialize(Creature &);
template void TargetedMovementGenerator<Player>::Finalize(Player &);
template void TargetedMovementGenerator<Creature>::Finalize(Creature &);
template void TargetedMovementGenerator<Player>::Reset(Player &);
template void TargetedMovementGenerator<Creature>::Reset(Creature &);
template bool TargetedMovementGenerator<Player>::Update(Player &, const uint32 &);
template bool TargetedMovementGenerator<Creature>::Update(Creature &, const uint32 &);
template Unit* TargetedMovementGenerator<Player>::GetTarget() const;
template Unit* TargetedMovementGenerator<Creature>::GetTarget() const;
