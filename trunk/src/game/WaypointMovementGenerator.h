/* 
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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

#ifndef MANGOS_WAYPOINTMOVEMENTGENERATOR_H
#define MANGOS_WAYPOINTMOVEMENTGENERATOR_H

/** @page PathMovementGenerator is used to generate movements
 * of waypoints and flight paths.  Each serves the purpose
 * of generate activities so that it generates updated
 * packets for the players.
 */

#include "MovementGenerator.h"
#include "DestinationHolder.h"
#include "Path.h"
#include "Traveller.h"

#include "Player.h"

#include <vector>
#include <set>

#define FLIGHT_TRAVEL_UPDATE  100

#define VISUAL_WAYPOINT 1

struct WaypointBehavior
{
    uint32 emote;
    uint32 spell;
    std::string text[5];
    float orientation;
    uint32 model1;
    uint32 model2;
    std::string aiscript;
    bool HasDone;
};

template<class T>
class MANGOS_DLL_DECL PathMovementBase
{
    public:
        PathMovementBase() : i_currentNode(0) {}
        virtual ~PathMovementBase() {};

        inline bool MovementInProgress(void) const { return i_currentNode < i_path.Size(); }

        // template pattern, not defined .. override required
        void LoadPath(T &);
        void ReloadPath(T &);
        uint32 GetCurrentNode()
        {
            return i_currentNode;
        }

    protected:
        uint32 i_currentNode;
        DestinationHolder< Traveller<T> > i_destinationHolder;
        Path i_path;
};

/** WaypointMovementGenerator loads a series of way points
 * from the DB and apply it to the creature's movement generator.
 * Hence, the creature will move according to its predefined way points.
 */

template<class T>
class MANGOS_DLL_DECL WaypointMovementGenerator;

template<>
class MANGOS_DLL_DECL WaypointMovementGenerator<Creature>
: public MovementGeneratorMedium< Creature, WaypointMovementGenerator<Creature> >,
public PathMovementBase<Creature>
{
    TimeTracker i_nextMoveTime;
    std::vector<uint32> i_delays;
    std::vector<WaypointBehavior *> i_wpBehaviour;
    public:
        WaypointMovementGenerator(Creature &) : i_nextMoveTime(0) {}
        ~WaypointMovementGenerator() { ClearWaypoints(); }
        void WPAIScript(Creature &pCreature, std::string pAiscript);
        void Initialize(Creature &u)
        {
            i_nextMoveTime.Reset(0);                        // TODO: check the lower bound (0 is probably too small)
            u.StopMoving();
            LoadPath(u);
        }

        void Reset(Creature &u) { ReloadPath(u); }
        bool Update(Creature &u, const uint32 &diff);
        MovementGeneratorType GetMovementGeneratorType() { return WAYPOINT_MOTION_TYPE; }

        // now path movement implmementation
        void LoadPath(Creature &c);
        void ReloadPath(Creature &c) { ClearWaypoints(); LoadPath(c); }

        // statics
        static void Initialize(void);
    private:
        void ClearWaypoints();
        static std::set<uint32> si_waypointHolders;
};

/** FlightPathMovementGenerator generates movement of the player for the paths
 * and hence generates ground and activities for the player.
 */
class MANGOS_DLL_DECL FlightPathMovementGenerator
: public MovementGeneratorMedium< Player, FlightPathMovementGenerator >,
public PathMovementBase<Player>
{
    uint32 i_pathId;
    public:
        FlightPathMovementGenerator(uint32 id) : i_pathId(id) {}
        void Initialize(Player &);
        void Reset(Player &) {}
        bool Update(Player &, const uint32 &);
        MovementGeneratorType GetMovementGeneratorType() { return FLIGHT_MOTION_TYPE; }

        void EndFlight(Player &);

        void LoadPath(Player &);
        void ReloadPath(Player &) { /* don't reload flight path */ }

        Path& GetPath(void) { return i_path; }
        inline bool HasArrived(void) const { return (i_currentNode >= i_path.Size()); }
};
#endif
