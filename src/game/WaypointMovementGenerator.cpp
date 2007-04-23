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

/*
creature_movement Table

alter table creature_movement add `text1` varchar(255) default NULL;
alter table creature_movement add `text2` varchar(255) default NULL;
alter table creature_movement add `text3` varchar(255) default NULL;
alter table creature_movement add `text4` varchar(255) default NULL;
alter table creature_movement add `text5` varchar(255) default NULL;
alter table creature_movement add `aiscript` varchar(255) default NULL;
alter table creature_movement add `emote` int(10) unsigned default '0';
alter table creature_movement add `spell` int(5) unsigned default '0';
alter table creature_movement add `wpguid` int(11) default '0';

*/

#include <ctime>

#include "WaypointMovementGenerator.h"
#include "ObjectMgr.h"
#include "Creature.h"
#include "FlightMaster.h"

#include <cassert>

//-----------------------------------------------//
void
WaypointMovementGenerator::_load(Creature &c)
{
    i_path.Clear();
    i_wpBehaviour.clear();

    QueryResult *result = NULL;
    sLog.outDebug("DEBUG: WaypointMovementGenerator::_load: GUID - %d", c.GetGUIDLow());
    // Identify by GUID
    result = sDatabase.PQuery("SELECT `position_x`, `position_y`, `position_z`, `orientation`, `model1`, `model2`, `waittime`, `emote`, `spell`, `text1`, `text2`, `text3`, `text4`, `text5`, `aiscript` FROM `creature_movement` WHERE `id` = '%u' ORDER BY `point`", c.GetDBTableGUIDLow());
    /*
    if( result ) {
    sLog.outDebug("DEBUG: Number of hits: %d", result->GetRowCount());
    } else {
    sLog.outDebug("DEBUG: Nothing found");
    }
    */

    if( result )
    {
        unsigned int count = 0;
        const unsigned int sz = result->GetRowCount();
        i_path.Resize( sz );
        i_delays.resize( sz );
        i_wpBehaviour.resize( sz );

        do
        {
            //sLog.outDebug("DEBUG: _load");
            Field *fields = result->Fetch();
            i_path[count].x         = fields[0].GetFloat();
            i_path[count].y         = fields[1].GetFloat();
            i_path[count].z         = fields[2].GetFloat();
            float orientation       = fields[3].GetFloat();
            uint32 model1           = fields[4].GetUInt32();
            uint32 model2           = fields[5].GetUInt32();
            i_delays[count]         = fields[6].GetUInt16();
            uint32 emote            = fields[7].GetUInt32();
            uint32 spell            = fields[8].GetUInt32();
            std::string text1       = fields[9].GetCppString();
            std::string text2       = fields[10].GetCppString();
            std::string text3       = fields[11].GetCppString();
            std::string text4       = fields[12].GetCppString();
            std::string text5       = fields[13].GetCppString();
            std::string aiscript    = fields[14].GetCppString();

            if( (emote != 0) || (spell != 0)
                || (text1 != "") || (text2 != "") || (text3 != "") || (text4 != "") || (text5 != "")
                || (aiscript != "")
                || (model1 != 0)  || (model2 != 0) || (orientation != 100))
            {
                WaypointBehavior *tmpWPB = new WaypointBehavior;

                sLog.outDebug("DEBUG: _load  ---  Adding WaypointBehavior");

                tmpWPB->text[0] = text1;
                tmpWPB->text[1] = text2;
                tmpWPB->text[2] = text3;
                tmpWPB->text[3] = text4;
                tmpWPB->text[4] = text5;
                tmpWPB->aiscript = aiscript;
                tmpWPB->orientation = orientation;
                tmpWPB->emote = emote;
                tmpWPB->spell = spell;
                tmpWPB->model1 = model1;
                tmpWPB->model2 = model2;
                tmpWPB->HasDone = false;
                i_wpBehaviour[count] = tmpWPB;
            }
            else
            {
                i_wpBehaviour[count] = NULL;
            }

            if(!MaNGOS::IsValidMapCoord(i_path[count].x,i_path[count].y))
            {
                sLog.outErrorDb("ERROR: Creature (guidlow %d,entry %d) have invalid coordinates in his waypoint %d (X: %d, Y: %d).",
                    c.GetGUIDLow(),c.GetEntry(),count,i_path[count].x,i_path[count].y
                    );

                // prevent invalid coordinates using
                MaNGOS::NormalizeMapCoord(i_path[count].x);
                MaNGOS::NormalizeMapCoord(i_path[count].y);
                i_path[count].z = MapManager::Instance ().GetMap(c.GetMapId(), &c)->GetHeight(i_path[count].x,i_path[count].y);
            }
            // to prevent a misbehaviour inside "update"
            // update is alway called with the next wp - but the wpSys needs the current
            // so when the routine is called the first time, wpSys gets the last waypoint
            // and this prevents the system from performing text/emote, etc
            if( count == (sz-1) )
            {
                if( i_wpBehaviour[count] != NULL )
                {
                    i_wpBehaviour[count]->HasDone = true;
                }
            }
            //if( i_delays[count] < 30 /* millisecond */ )
            //	i_delays[count] = (rand() % 5000);
            ++count;

        } while( result->NextRow() );

        delete result;

        assert( sz == count );
    }
}

void
WaypointMovementGenerator::Initialize()
{
    QueryResult *result = sDatabase.Query("SELECT distinct(`id`) as uniqueid FROM `creature_movement`");

    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            si_waypointHolders.insert( fields[0].GetUInt32() );
        }
        while( result->NextRow() );

        delete result;
    }
}

int
WaypointMovementGenerator::Permissible(const Creature *c)
{
    if (si_waypointHolders.find(c->GetGUIDLow()) != si_waypointHolders.end())
    {
        DEBUG_LOG("Creature [guid=%u] returns waypoint movement permit.", c->GetGUIDLow());
        return WAYPOINT_MOTION_TYPE;
    }

    return CANNOT_HANDLE_TYPE;
}

bool
WaypointMovementGenerator::Update(Creature &creature, const uint32 &diff)
{
    if(!&creature)
        return true;
    if(i_creature.hasUnitState(UNIT_STAT_ROOT) || i_creature.hasUnitState(UNIT_STAT_STUNDED))
        return true;

    // prevent crash at empty waypoint path.
    if(i_path.Size()==0)
    {
        return true;
    }

    CreatureTraveller traveller(creature);

    /*
    if( npcIsStopped[creature.GetGUID()] )
    {
        i_nextMoveTime.Update(40000);
        i_destinationHolder.UpdateTraveller(traveller, ((diff)-40000), false);
        npcIsStopped[creature.GetGUID()] = false;
        return true;
    }
    */
    i_nextMoveTime.Update(diff);
    i_destinationHolder.UpdateTraveller(traveller, diff, false);

    if( i_creature.IsStopped() )
    {
        uint32 wpB = i_currentNode > 0 ? i_currentNode-1 : i_wpBehaviour.size()-1;

        if( i_wpBehaviour[wpB] != NULL )
        {
            struct WaypointBehavior *tmpBehavior = i_wpBehaviour[wpB];

            if (!tmpBehavior->HasDone)
            {
                if(tmpBehavior->emote != 0)
                {
                    creature.SetUInt32Value(UNIT_NPC_EMOTESTATE,tmpBehavior->emote);
                }
                if(tmpBehavior->aiscript != "")
                {
                    WPAIScript(creature, tmpBehavior->aiscript);
                }
                //sLog.outDebug("DEBUG: tmpBehavior->text[0] TEST");
                if(tmpBehavior->text[0] != "")
                {
                    //sLog.outDebug("DEBUG: tmpBehavior->text[0] != \"\"");
                    // Only one text is set
                    if( tmpBehavior->text[1] == "" )
                    {
                        //sLog.outDebug("DEBUG: tmpBehavior->text[1] == NULL");
                        creature.Say(tmpBehavior->text[0].c_str(), 0, 0);
                    }
                    else
                    {
                        // Select one from max 5 texts
                        int maxText = 4;
                        for( int i=0; i<4; i++ )
                        {
                            if( tmpBehavior->text[i] == "" )
                            {
                                //sLog.outDebug("DEBUG: tmpBehavior->text[i] == \"\": %d", i);
                                //sLog.outDebug("DEBUG: rand() % (i): %d", rand() % (i));

                                creature.Say(tmpBehavior->text[rand() % i].c_str(), 0, 0);
                                break;
                            }
                        }
                    }
                }
                if(tmpBehavior->spell != 0)
                {
                    //sLog.outDebug("DEBUG: wpSys - spell");
                    creature.CastSpell(&creature,tmpBehavior->spell, false);
                }
                if (tmpBehavior->orientation !=100)
                {
                    //sLog.outDebug("DEBUG: wpSys - orientation");
                    creature.SetOrientation(tmpBehavior->orientation);
                }
                if(tmpBehavior->model1 != 0)
                {
                    //sLog.outDebug("DEBUG: wpSys - model1");
                    creature.SetUInt32Value(UNIT_FIELD_DISPLAYID, tmpBehavior->model1);
                }
                tmpBehavior->HasDone = true;
            }                                               // HasDone == false
        }                                                   // wpBehaviour found
    }                                                       // i_creature.IsStopped()

    if( i_nextMoveTime.Passed() )
    {
        if( i_creature.IsStopped() )
        {
            assert( i_currentNode < i_path.Size() );
            creature.addUnitState(UNIT_STAT_ROAMING);
            const Path::PathNode &node(i_path(i_currentNode));
            i_destinationHolder.SetDestination(traveller, node.x, node.y, node.z);
            i_nextMoveTime.Reset(i_destinationHolder.GetTotalTravelTime());
            uint32 wpB = i_currentNode > 0 ? i_currentNode-1 : i_wpBehaviour.size()-1;

            if( i_wpBehaviour[wpB] != NULL )
            {
                struct WaypointBehavior *tmpBehavior = i_wpBehaviour[wpB];
                tmpBehavior->HasDone = false;
                if(tmpBehavior->model2 != 0)
                {
                    creature.SetUInt32Value(UNIT_FIELD_DISPLAYID, tmpBehavior->model2);
                }
                if (tmpBehavior->orientation !=100)
                {
                    creature.SetOrientation(tmpBehavior->orientation);
                }
                creature.SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
            }
        }
        else
        {
            creature.StopMoving();
            i_nextMoveTime.Reset(i_delays[i_currentNode]);
            ++i_currentNode;
            if( i_currentNode >= i_path.Size() )
                i_currentNode = 0;
        }
    }
    return true;
}

void
WaypointMovementGenerator::WPAIScript(Creature &pCreature, std::string pAiscript)
{
    time_t curr;
    tm local;
    time(&curr);                                            // get current time_t value
    local=*(localtime(&curr));                              //
    int cT = ((local.tm_hour*100)+local.tm_min);

    sLog.outDebug("WPAIScript: %s", pAiscript.c_str());

    if( pAiscript == "guard-sw")                            //demo script for WP-AI System
    {
        if(pCreature.GetEntry() == 68 || 1423)
        {
            if(!( (cT < 1800) && (cT > 800) ))              //If time not smaler than 1800 and not bigger than 800 (24 hour format)
            {                                               //try to set model of Off-hand (shield) to 0 (imo it doesn't work...)
                pCreature.SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, 0);
                pCreature.SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 2, 234948100);
                pCreature.SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 2 + 1, 4);

                //set new Off-Hand Item as lamp
                pCreature.SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, 7557);
                pCreature.SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 2, 385941508);
                pCreature.SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 2 + 1, 7);
            }                                               //else do it in other direction...
            else
            {
                pCreature.SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, 0);
                pCreature.SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 2, 385941508);
                pCreature.SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 2 + 1, 7);

                pCreature.SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, 2080);
                pCreature.SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 2, 234948100);
                pCreature.SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 2 + 1, 4);
            }
        }
        sLog.outDebug("guard-sw");
    }
}

std::set<uint32> WaypointMovementGenerator::si_waypointHolders;

//----------------------------------------------------//
FlightPathMovementGenerator::FlightPathMovementGenerator(Player &pl, uint32 id) : i_pathId(id), i_player(pl)
{
    Initialize();
    FlightMaster::Instance().ReportFlight(&i_player, this);
}

void
FlightPathMovementGenerator::LoadPath(Player &pl)
{
    objmgr.GetTaxiPathNodes(i_pathId, i_path);
}

void
FlightPathMovementGenerator::Initialize()
{
    i_player.MoveToHateOfflineList();
    i_player.addUnitState(UNIT_STAT_IN_FLIGHT);
    LoadPath(i_player);
    i_currentNode = 0;
    Traveller<Player> traveller(i_player);
    i_destinationHolder.SetDestination(traveller, i_path[i_currentNode].x, i_path[i_currentNode].y, i_path[i_currentNode].z);
}

//
// Unique1's ASTAR Pathfinding Code... For future use & reference...
//

#ifdef __PATHFINDING__

int GetFCost(int to, int num, int parentNum, float *gcost); // Below...

int ShortenASTARRoute(short int *pathlist, int number)
{                                                           // Wrote this to make the routes a little smarter (shorter)... No point looping back to the same places... Unique1
    short int temppathlist[MAX_PATHLIST_NODES];
    int count = 0;
    //    int count2 = 0;
    int temp, temp2;
    int link;
    int upto = 0;

    for (temp = number; temp >= 0; temp--)
    {
        qboolean shortened = qfalse;

        for (temp2 = 0; temp2 < temp; temp2++)
        {
            for (link = 0; link < nodes[pathlist[temp]].enodenum; link++)
            {
                if (nodes[pathlist[temp]].links[link].flags & PATH_BLOCKED)
                    continue;

                //if ((bot->client->ps.eFlags & EF_TANK) && nodes[bot->current_node].links[link].flags & PATH_NOTANKS)    //if this path is blocked, skip it
                //    continue;

                //if (nodes[nodes[pathlist[temp]].links[link].targetNode].origin[2] > nodes[pathlist[temp]].origin[2] + 32)
                //    continue;

                if (nodes[pathlist[temp]].links[link].targetNode == pathlist[temp2])
                {                                           // Found a shorter route...
                    //if (OrgVisible(nodes[pathlist[temp2]].origin, nodes[pathlist[temp]].origin, -1))
                    {
                        temppathlist[count] = pathlist[temp2];
                        temp = temp2;
                        count++;
                        shortened = qtrue;
                    }
                }
            }
        }

        if (!shortened)
        {
            temppathlist[count] = pathlist[temp];
            count++;
        }
    }

    upto = count;

    for (temp = 0; temp < count; temp++)
    {
        pathlist[temp] = temppathlist[upto];
        upto--;
    }

    G_Printf("ShortenASTARRoute: Path size reduced from %i to %i nodes...n", number, count);
    return count;
}

/*
===========================================================================
CreatePathAStar
This function uses the A* pathfinding algorithm to determine the
shortest path between any two nodes.
It's fairly complex, so I'm not really going to explain it much.
Look up A* and binary heaps for more info.
pathlist stores the ideal path between the nodes, in reverse order,
and the return value is the number of nodes in that path
===========================================================================
*/
int CreatePathAStar(gentity_t *bot, int from, int to, short int *pathlist)
{
    //all the data we have to hold...since we can't do dynamic allocation, has to be MAX_NODES
    //we can probably lower this later - eg, the open list should never have more than at most a few dozen items on it
    short int openlist[MAX_NODES+1];                        //add 1 because it's a binary heap, and they don't use 0 - 1 is the first used index
    float gcost[MAX_NODES];
    int fcost[MAX_NODES];
    char list[MAX_NODES];                                   //0 is neither, 1 is open, 2 is closed - char because it's the smallest data type
    short int parent[MAX_NODES];

    short int numOpen = 0;
    short int atNode, temp, newnode=-1;
    qboolean found = qfalse;
    int count = -1;
    float gc;
    int i, u, v, m;
    vec3_t vec;

    //clear out all the arrays
    memset(openlist, 0, sizeof(short int)*(MAX_NODES+1));
    memset(fcost, 0, sizeof(int)*MAX_NODES);
    memset(list, 0, sizeof(char)*MAX_NODES);
    memset(parent, 0, sizeof(short int)*MAX_NODES);
    memset(gcost, -1, sizeof(float)*MAX_NODES);

    //make sure we have valid data before calculating everything
    if ((from == NODE_INVALID) || (to == NODE_INVALID) || (from >= MAX_NODES) || (to >= MAX_NODES) || (from == to))
        return -1;

    openlist[1] = from;                                     //add the starting node to the open list
    numOpen++;
    gcost[from] = 0;                                        //its f and g costs are obviously 0
    fcost[from] = 0;

    while (1)
    {
        if (numOpen != 0)                                   //if there are still items in the open list
        {
            //pop the top item off of the list
            atNode = openlist[1];
            list[atNode] = 2;                               //put the node on the closed list so we don't check it again
            numOpen--;

            openlist[1] = openlist[numOpen+1];              //move the last item in the list to the top position
            v = 1;

            //this while loop reorders the list so that the new lowest fcost is at the top again
            while (1)
            {
                u = v;
                if ((2*u+1) < numOpen)                      //if both children exist
                {
                    if (fcost[openlist[u]] >= fcost[openlist[2*u]])
                        v = 2*u;
                    if (fcost[openlist[v]] >= fcost[openlist[2*u+1]])
                        v = 2*u+1;
                }
                else
                {
                    if ((2*u) < numOpen)                    //if only one child exists
                    {
                        if (fcost[openlist[u]] >= fcost[openlist[2*u]])
                            v = 2*u;
                    }
                }

                if (u != v)                                 //if they're out of order, swap this item with its parent
                {
                    temp = openlist[u];
                    openlist[u] = openlist[v];
                    openlist[v] = temp;
                }
                else
                    break;
            }

            for (i = 0; i < nodes[atNode].enodenum; i++)    //loop through all the links for this node
            {
                newnode = nodes[atNode].links[i].targetNode;

                //if this path is blocked, skip it
                if (nodes[atNode].links[i].flags & PATH_BLOCKED)
                    continue;
                //if this path is blocked, skip it
                if (bot->client && (bot->client->ps.eFlags & EF_TANK) && nodes[atNode].links[i].flags & PATH_NOTANKS)
                    continue;
                //skip any unreachable nodes
                if (bot->client && (nodes[newnode].type & NODE_ALLY_UNREACHABLE) && (bot->client->sess.sessionTeam == TEAM_ALLIES))
                    continue;
                if (bot->client && (nodes[newnode].type & NODE_AXIS_UNREACHABLE) && (bot->client->sess.sessionTeam == TEAM_AXIS))
                    continue;

                if (list[newnode] == 2)                     //if this node is on the closed list, skip it
                    continue;

                if (list[newnode] != 1)                     //if this node is not already on the open list
                {
                    openlist[++numOpen] = newnode;          //add the new node to the open list
                    list[newnode] = 1;
                    parent[newnode] = atNode;               //record the node's parent

                    if (newnode == to)                      //if we've found the goal, don't keep computing paths!
                        break;                              //this will break the 'for' and go all the way to 'if (list[to] == 1)'

                    //store it's f cost value
                    fcost[newnode] = GetFCost(to, newnode, parent[newnode], gcost);

                    //this loop re-orders the heap so that the lowest fcost is at the top
                    m = numOpen;
                    while (m != 1)                          //while this item isn't at the top of the heap already
                    {
                        //if it has a lower fcost than its parent
                        if (fcost[openlist[m]] <= fcost[openlist[m/2]])
                        {
                            temp = openlist[m/2];
                            openlist[m/2] = openlist[m];
                            openlist[m] = temp;             //swap them
                            m /= 2;
                        }
                        else
                            break;
                    }
                }
                else                                        //if this node is already on the open list
                {
                    gc = gcost[atNode];
                    VectorSubtract(nodes[newnode].origin, nodes[atNode].origin, vec);
                    gc += VectorLength(vec);                //calculate what the gcost would be if we reached this node along the current path

                    if (gc < gcost[newnode])                //if the new gcost is less (ie, this path is shorter than what we had before)
                    {
                        parent[newnode] = atNode;           //set the new parent for this node
                        gcost[newnode] = gc;                //and the new g cost

                        for (i = 1; i < numOpen; i++)       //loop through all the items on the open list
                        {
                            if (openlist[i] == newnode)     //find this node in the list
                            {
                                //calculate the new fcost and store it
                                fcost[newnode] = GetFCost(to, newnode, parent[newnode], gcost);

                                //reorder the list again, with the lowest fcost item on top
                                m = i;
                                while (m != 1)
                                {
                                    //if the item has a lower fcost than it's parent
                                    if (fcost[openlist[m]] < fcost[openlist[m/2]])
                                    {
                                        temp = openlist[m/2];
                                        openlist[m/2] = openlist[m];
                                        openlist[m] = temp; //swap them
                                        m /= 2;
                                    }
                                    else
                                        break;
                                }
                                break;                      //exit the 'for' loop because we already changed this node
                            }                               //if
                        }                                   //for
                    }                                       //if (gc < gcost[newnode])
                }                                           //if (list[newnode] != 1) --> else
            }                                               //for (loop through links)
        }                                                   //if (numOpen != 0)
        else
        {
            found = qfalse;                                 //there is no path between these nodes
            break;
        }

        if (list[to] == 1)                                  //if the destination node is on the open list, we're done
        {
            found = qtrue;
            break;
        }
    }                                                       //while (1)

    if (found == qtrue)                                     //if we found a path
    {
        //G_Printf("%s - path found!n", bot->client->pers.netname);
        count = 0;

        temp = to;                                          //start at the end point
        while (temp != from)                                //travel along the path (backwards) until we reach the starting point
        {
            pathlist[count++] = temp;                       //add the node to the pathlist and increment the count
            temp = parent[temp];                            //move to the parent of this node to continue the path
        }

        pathlist[count++] = from;                           //add the beginning node to the end of the pathlist

        #ifdef __BOT_SHORTEN_ROUTING__
        count = ShortenASTARRoute(pathlist, count);         // This isn't working... Dunno why.. Unique1
        #endif                                              //__BOT_SHORTEN_ROUTING__
    }
    else
    {
        //G_Printf("^1*** ^4BOT DEBUG^5: (CreatePathAStar) There is no route between node ^7%i^5 and node ^7%i^5.n", from, to);
        count = CreateDumbRoute(from, to, pathlist);

        if (count > 0)
        {
            #ifdef __BOT_SHORTEN_ROUTING__
            count = ShortenASTARRoute(pathlist, count);     // This isn't working... Dunno why.. Unique1
            #endif                                          //__BOT_SHORTEN_ROUTING__
            return count;
        }
    }

    return count;                                           //return the number of nodes in the path, -1 if not found
}

/*
===========================================================================
GetFCost
Utility function used by A* pathfinding to calculate the
cost to move between nodes towards a goal.  Using the A*
algorithm F = G + H, G here is the distance along the node
paths the bot must travel, and H is the straight-line distance
to the goal node.
Returned as an int because more precision is unnecessary and it
will slightly speed up heap access
===========================================================================
*/
int GetFCost(int to, int num, int parentNum, float *gcost)
{
    float gc = 0;
    float hc = 0;
    vec3_t v;

    if (gcost[num] == -1)
    {
        if (parentNum != -1)
        {
            gc = gcost[parentNum];
            VectorSubtract(nodes[num].origin, nodes[parentNum].origin, v);
            gc += VectorLength(v);
        }
        gcost[num] = gc;
    }
    else
        gc = gcost[num];

    VectorSubtract(nodes[to].origin, nodes[num].origin, v);
    hc = VectorLength(v);

    return (int)(gc + hc);
}
#endif                                                      //__PATHFINDING__
