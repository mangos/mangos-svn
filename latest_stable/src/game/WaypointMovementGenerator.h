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
class MANGOS_DLL_DECL PathMovementGenerator
{
    public:
        PathMovementGenerator() : i_currentNode(0) {}

        inline bool MovementInProgress(void) const { return i_currentNode < i_path.Size(); }

        // template patern.. overrride required
        virtual void LoadPath(T &) = 0;
        virtual void ReloadPath(T &) = 0;

    protected:
        uint32 i_currentNode;
        DestinationHolder<Traveller<T> > i_destinationHolder;
        Path i_path;
};

/** WaypointMovementGenerator loads a series of way points
 * from the DB and apply it to the creature's movement generator.
 * Hence, the creature will move according to its predefined way points.
 */
class MANGOS_DLL_DECL WaypointMovementGenerator : public MovementGenerator, public PathMovementGenerator<Creature>
{
    Creature &i_creature;
    TimeTracker i_nextMoveTime;
    std::vector<uint32> i_delays;
    std::vector<WaypointBehavior *> i_wpBehaviour;
    public:
        WaypointMovementGenerator(Creature &c) : i_creature(c), i_nextMoveTime(0) {}
        ~WaypointMovementGenerator() {}
        void WPAIScript(Creature &pCreature, std::string pAiscript);
        void Initialize(Creature &c)
        {
            i_nextMoveTime.Reset(0);                        // TODO: check the lower bound (0 is probably too small)
            i_creature.StopMoving();
            LoadPath(c);
        }

        void Reset(Creature &c) { ReloadPath(c); }
        bool Update(Creature &creature, const uint32 &diff);
        MovementGeneratorType GetMovementGeneratorType() { return WAYPOINT_MOTION_TYPE; }

        // now path movement implmementation
        void LoadPath(Creature &c) { _load(c); }
        void ReloadPath(Creature &c) { _load(c); }

        // statics
        static void Initialize(void);
        static int Permissible(const Creature *);
    private:
        void _load(Creature &);
        static std::set<uint32> si_waypointHolders;
};

/** FlightPathMovementGenerator generates movement of the player for the paths
 * and hence generates ground and activities for the player.
 */
class MANGOS_DLL_DECL FlightPathMovementGenerator : public PathMovementGenerator<Player>
{
    uint32 i_pathId;
    Player &i_player;
    public:
        FlightPathMovementGenerator(Player &pl, uint32 id);
        void Initialize(void);

        void LoadPath(Player &);
        void ReloadPath(Player &) { /* don't reload flight path */ }
        void UpdatePath(Player &pl, const uint32 &);

        Path& GetPath(void) { return i_path; }
        Player& GetPassenger(void) const { return i_player; }
        inline bool HasArrived(void) const { return (i_currentNode >= i_path.Size()); }

        bool CheckFlight(const uint32 diff)
        {
            UpdatePath(i_player, diff);
            if( HasArrived() )
            {
                float x, y, z;
                i_destinationHolder.GetLocationNow(x, y, z);
                i_player.SetPosition(x, y, z, true);
                i_player.FlightComplete();
                return true;
            }
            return false;
        }
};
#endif
