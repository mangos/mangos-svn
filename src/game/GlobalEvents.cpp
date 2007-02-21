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

/** \file
    \ingroup u2w
*/

#include "Log.h"
#include "Database/DatabaseEnv.h"
#include "Common.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "EventSystem.h"
#include "GlobalEvents.h"
#include "ObjectDefines.h"
#include "Corpse.h"

/// Handle periodic erase of corpses and bones
static void CorpsesErase(CorpseType type,uint32 delay)
{
    ///- Get the list of eligible corpses/bones to be removed
    //No SQL injection (uint32 and enum)
    QueryResult *result = sDatabase.PQuery("SELECT `guid`,`position_x`,`position_y`,`map`,`player` FROM `corpse` WHERE UNIX_TIMESTAMP()-UNIX_TIMESTAMP(`time`) > '%u' AND `bones_flag` = '%u'",delay,type );

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guidlow = fields[0].GetUInt32();
            float positionX = fields[1].GetFloat();
            float positionY = fields[2].GetFloat();
            uint32 mapid    = fields[3].GetUInt32();
            uint64 player_guid = MAKE_GUID(fields[4].GetUInt32(),HIGHGUID_PLAYER);

            uint64 guid = MAKE_GUID(guidlow,HIGHGUID_CORPSE);

            sLog.outDebug("[Global event] Removing %s %u (X:%f Y:%f Map:%u).",(type==CORPSE_BONES?"bones":"corpse"),guidlow,positionX,positionY,mapid);

            /// Resurrectable - convert corpses to bones
            if(type==CORPSE_RESURRECTABLE)
            {
                // must be in world object list
                Corpse *corpse = ObjectAccessor::Instance().GetCorpseForPlayerGUID(player_guid);
                if(corpse)
                    corpse->ConvertCorpseToBones();
                else
                {
                    sLog.outDebug("Corpse %u not found in world. Delete from DB.",guidlow);
                    sDatabase.BeginTransaction();
                    sDatabase.PExecute("DELETE FROM `corpse` WHERE `guid` = '%u'",guidlow);
                    sDatabase.PExecute("DELETE FROM `corpse_grid` WHERE `guid` = '%u'",guidlow);
                    sDatabase.CommitTransaction();
                }
            }
            else
            ///- or delete bones
            {
                ///- If the map where the bones is loaded
                if(!MapManager::Instance().GetMap(mapid)->IsRemovalGrid(positionX,positionY))
                {
                    ///- delete bones from world
                    Corpse *corpse = MapManager::Instance().GetMap(mapid)->GetObjectNear<Corpse>(positionX,positionY,guid);
                    if(corpse)
                        corpse->DeleteBonnesFromWorld();
                    else
                        sLog.outDebug("Bones %u not found in world. Delete from DB also.",guidlow);

                }

                ///- remove bones from the database
                sDatabase.BeginTransaction();
                sDatabase.PExecute("DELETE FROM `corpse` WHERE `guid` = '%u'",guidlow);
                sDatabase.PExecute("DELETE FROM `corpse_grid` WHERE `guid` = '%u'",guidlow);
                sDatabase.CommitTransaction();
            }
        } while (result->NextRow());

        delete result;
    }
}

/// not thread guarded variant for call from other thread
void CorpsesErase()
{
    CorpsesErase(CORPSE_BONES, 20*MINUTE);
    CorpsesErase(CORPSE_RESURRECTABLE,3*DAY);
}

/// thread guarded variant for call from event system
void HandleCorpsesErase(void*)
{
    sDatabase.ThreadStart();                                // let thread do safe mySQL requests

    CorpsesErase();

    sDatabase.ThreadEnd();                                  // free mySQL thread resources
}
