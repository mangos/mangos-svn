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

#include "GameEvent.h"
#include "ObjectMgr.h"
#include "ProgressBar.h"
#include "Log.h"
#include "MapManager.h"

INSTANTIATE_SINGLETON_1(GameEvent);

bool GameEvent::CheckOneGameEvent(uint16 entry)
{
    // Get the event information
    time_t currenttime = time(NULL);
    if ((mGameEvent[entry].start < currenttime) && (currenttime < mGameEvent[entry].end) && (((currenttime - mGameEvent[entry].start) % (mGameEvent[entry].occurence * 3600)) < (mGameEvent[entry].length * 3600)))
        return true;
    else
        return false;
}

uint32 GameEvent::NextCheck(uint16 entry)
{
    time_t currenttime = time(NULL);

    // outdated event: we return max
    if (currenttime > mGameEvent[entry].end) 
        return max_ge_check_delay;

    // never started event, we return delay before start
    if (mGameEvent[entry].start > currenttime)
        return (mGameEvent[entry].start - currenttime);
   
    uint32 delay;
    // in event, we return the end of it
    if ((((currenttime - mGameEvent[entry].start) % (mGameEvent[entry].occurence * 3600)) < (mGameEvent[entry].length * 3600)))
        // we return the delay before it ends
        delay = (mGameEvent[entry].length * 3600) - ((currenttime - mGameEvent[entry].start) % (mGameEvent[entry].occurence * 3600));
    else // not in window, we return the delay before next start
        delay = (mGameEvent[entry].occurence * 3600) - ((currenttime - mGameEvent[entry].start) % (mGameEvent[entry].occurence * 3600));
    // In case the end is before next check
    if ((mGameEvent[entry].end - currenttime) < delay)
        return (mGameEvent[entry].end - currenttime);
    else
        return delay;
}

void GameEvent::LoadFromDB()
{
    QueryResult *result = sDatabase.Query("SELECT MAX(`entry`) FROM `game_event`");
    if( result )
    {
		Field *fields = result->Fetch();
		max_event_id = fields[0].GetUInt16();
        delete result;
	}

    if( !result || max_event_id == 0)
    {
        // NOTE:
        // on some platforms for an empty table this query will return NULL
        // on others the table will have a max entry of 0 and the query will never return NULL
		sLog.outErrorDb(">> Table game_event is empty:");
        sLog.outString("");
		return;
	}

	mGameEvent.resize(max_event_id + 1);

	result = sDatabase.Query("SELECT `entry`,UNIX_TIMESTAMP(`start`),UNIX_TIMESTAMP(`end`),`occurence`,`length`,`description` FROM `game_event`");
    if( !result )
    {
		sLog.outErrorDb(">> Table game_event is empty:");
        sLog.outString("");
		return;
    }

    uint32 count = 0; 

    barGoLink bar( result->GetRowCount() );
    do
    {
        count++;
        Field *fields = result->Fetch();

        bar.step();

        uint16 event_id = fields[0].GetUInt16();

        GameEventData& pGameEvent = mGameEvent[event_id];
        uint64 starttime        = fields[1].GetUInt64();
        pGameEvent.start        = time_t(starttime);
        uint64 endtime          = fields[2].GetUInt64();
        pGameEvent.end          = time_t(endtime);
        pGameEvent.occurence    = fields[3].GetUInt32();
        pGameEvent.length       = fields[4].GetUInt32();
        pGameEvent.description  = fields[5].GetCppString();
        
        if (CheckOneGameEvent(event_id))
            AddActiveEvent(event_id);

    } while( result->NextRow() );

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u game events", count );
    delete result;

    mGameEventCreatureGuids.resize((max_event_id * 2) + 1);
    //                               1                 2
    result = sDatabase.Query("SELECT `creature`.`guid`,`game_event_creature`.`event` "
        "FROM `creature` JOIN `game_event_creature` ON `creature`.`guid` = `game_event_creature`.`guid`");

    count = 0; 
    if( !result )
    {
        barGoLink bar2(1);
        bar2.step();

        sLog.outString("");
        sLog.outErrorDb(">> Loaded %u creatures in game events", count );
    } else {

        barGoLink bar2( result->GetRowCount() );
        do
        {
            count++;
            Field *fields = result->Fetch();
        
            bar2.step();
        
            uint32 guid = fields[0].GetUInt32();

            GuidList& crelist = mGameEventCreatureGuids[max_event_id + fields[1].GetInt16()];
            crelist.push_back(guid);

        } while( result->NextRow() );
        sLog.outString("");
        sLog.outString( ">> Loaded %u creatures in game events", count );
        delete result;
    }

    mGameEventGameobjectGuids.resize((max_event_id * 2) + 1);
    //                               1                   2
    result = sDatabase.Query("SELECT `gameobject`.`guid`,`game_event_gameobject`.`event` "
        "FROM `gameobject` JOIN `game_event_gameobject` ON `gameobject`.`guid`=`game_event_gameobject`.`guid`");

    count = 0; 
    if( !result )
    {
        barGoLink bar3(1);
        bar3.step();

        sLog.outString("");
        sLog.outErrorDb(">> Loaded %u gameobjects in game events", count );
    } else {

        barGoLink bar3( result->GetRowCount() );
        do
        {
            count++;
            Field *fields = result->Fetch();
            
            bar3.step();

            uint32 guid = fields[0].GetUInt32();

            GuidList& golist = mGameEventGameobjectGuids[max_event_id + fields[1].GetInt16()];
            golist.push_back(guid);

        } while( result->NextRow() );
        sLog.outString("");
        sLog.outString( ">> Loaded %u gameobjects in game events", count );
    
        delete result;
    }
}

uint32 GameEvent::Initialize() // return the next event delay in ms
{
    m_ActiveEvents.clear();
    uint32 delay = Update();
    sLog.outBasic("Game Event system initialized." );
    isSystemInit = true;
    return delay;
}

uint32 GameEvent::Update() // return the next event delay in ms
{   
    uint32 nextEventDelay = max_ge_check_delay; // 1 day
    uint32 calcDelay;
    for (uint16 itr = 1; itr <=	max_event_id; itr++)
    {
        //sLog.outErrorDb("Checking event %u",itr);
        if (CheckOneGameEvent(itr))
        {
            //sLog.outDebug("GameEvent %u is active",itr->first);
            if (!IsActiveEvent(itr))
            {
                AddActiveEvent(itr);
                ApplyNewEvent(itr);
            }
        } else {
            //sLog.outDebug("GameEvent %u is not active",itr->first);
            if (IsActiveEvent(itr))
            {
                RemoveActiveEvent(itr);
                UnApplyEvent(itr);
            } else {
                if (!isSystemInit)
                {
                    int16 event_nid = (-1) * (itr);
                    // spawn all negative ones for this event
                    GameEventSpawn(event_nid);
                }
            }
        }
        calcDelay = NextCheck(itr);
        if (calcDelay < nextEventDelay)
            nextEventDelay = calcDelay;
    }
    sLog.outBasic("Next game event check in %u secondes.", nextEventDelay + 1);
    return (nextEventDelay + 1) * 1000; // Add 1 seconde to be sure event has started/stopped at next call
}

void GameEvent::UnApplyEvent(uint16 event_id)
{
    sLog.outString("GameEvent %u \"%s\" removed.", event_id, mGameEvent[event_id].description.c_str());
    // un-spawn positive event tagged objects
    GameEventUnspawn(event_id);
    // spawn negative event tagget objects
    int16 event_nid = (-1) * event_id;
    GameEventSpawn(event_nid);
}

void GameEvent::ApplyNewEvent(uint16 event_id)
{
    sLog.outString("GameEvent %u \"%s\" started.", event_id, mGameEvent[event_id].description.c_str());
    // spawn positive event tagget objects
    GameEventSpawn(event_id);
    // un-spawn negative event tagged objects
    int16 event_nid = (-1) * event_id;
    GameEventUnspawn(event_nid);
}

void GameEvent::GameEventSpawn(int16 event_id)
{
    GuidList::iterator itr;
    for (itr = mGameEventCreatureGuids[max_event_id + event_id].begin();itr != mGameEventCreatureGuids[max_event_id + event_id].end();++itr)
    {
        // Add to correct cell
        CreatureData const* data = objmgr.GetCreatureData(*itr);
        if (data)
        {
            objmgr.AddCreatureToGrid(*itr, data);

            // Spawn if necessary (loaded grids only)
            Map* map = MapManager::Instance().GetBaseMap(data->mapid);
            // We use spawn coords to spawn
            if(!map->Instanceable() && !map->IsRemovalGrid(data->spawn_posX,data->spawn_posY))
            {
                Creature* pCreature = new Creature((WorldObject*)NULL);
                //sLog.outDebug("Spawning creature %u",*itr);
                if (!pCreature->LoadFromDB(*itr, map->GetInstanceId()))
                {
                    delete pCreature;
                } else {
                    map->Add(pCreature);
                }
            }
        }
    }
    for (itr = mGameEventGameobjectGuids[max_event_id + event_id].begin();itr != mGameEventGameobjectGuids[max_event_id + event_id].end();++itr)
    {
        // Add to correct cell
        GameObjectData const* data = objmgr.GetGOData(*itr);
        if (data)
        {
            objmgr.AddGameobjectToGrid(*itr, data);
            // Spawn if necessary (loaded grids only)
            Map* map = MapManager::Instance().GetBaseMap(data->mapid);
            // We use current coords to unspawn, not spawn coords since creature can have changed grid
            if(!map->Instanceable() && !map->IsRemovalGrid(data->posX, data->posY))
            {
                GameObject* pGameobject = new GameObject((WorldObject*)NULL);
                //sLog.outDebug("Spawning gameobject %u", *itr);
                if (!pGameobject->LoadFromDB(*itr, map->GetInstanceId()))
                {
                    delete pGameobject;
                } else {
                    map->Add(pGameobject);
                }
            }
        }
    }
}

void GameEvent::GameEventUnspawn(int16 event_id)
{
    GuidList::iterator itr;
    for (itr = mGameEventCreatureGuids[max_event_id + event_id].begin();itr != mGameEventCreatureGuids[event_id + max_event_id].end();++itr)
    {
        // Remove the creature from grid
        CreatureData const* data = objmgr.GetCreatureData(*itr);
        objmgr.RemoveCreatureFromGrid(*itr, data);

        Creature* pCreature;
        pCreature = ObjectAccessor::Instance().GetObjectInWorld(MAKE_GUID(*itr, HIGHGUID_UNIT), (Creature*)NULL);
        //sLog.outDebug("Un-spawning creature %u",*itr);
        if (pCreature)
        {
            pCreature->CombatStop(true);
            ObjectAccessor::Instance().AddObjectToRemoveList(pCreature);
        }
    }
    for (itr = mGameEventGameobjectGuids[max_event_id + event_id].begin();itr != mGameEventGameobjectGuids[max_event_id + event_id].end();++itr)
    {
        // Remove the gameobject from grid
        GameObjectData const* data = objmgr.GetGOData(*itr);
        objmgr.RemoveGameobjectFromGrid(*itr, data);

        GameObject* pGameobject;
        pGameobject = ObjectAccessor::Instance().GetObjectInWorld(MAKE_GUID(*itr, HIGHGUID_GAMEOBJECT), (GameObject*)NULL);
        //sLog.outDebug("Un-spawning gameobject %u",*itr);
        if (pGameobject)
        {
            ObjectAccessor::Instance().AddObjectToRemoveList(pGameobject);
        }
    }
}

GameEvent::GameEvent()
{
    isSystemInit = false;
    max_event_id = 0;
}
