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

#include "Log.h"
#include "Database/DatabaseEnv.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "EventSystem.h"
#include "GlobalEvents.h"

static void CorpsesErase(CorpseType type,uint32 delay)
{
    QueryResult *result = sDatabase.PQuery("SELECT * FROM `corpse` WHERE UNIX_TIMESTAMP()-UNIX_TIMESTAMP(`time`) > '%u' AND `bones_flag` = '%u';",delay,type );

    if(result)
    {
        do
        {

            Field *fields = result->Fetch();
            uint64 guid = fields[0].GetUInt64();
            float positionX = fields[2].GetFloat();
            float positionY = fields[3].GetFloat();
            //float positionZ = fields[4].GetFloat();
            //float ort       = fields[5].GetFloat();
            uint32 mapid    = fields[7].GetUInt32();

            sLog.outDebug("[Global event] Removing %s %u (X:%f Y:%f Map:%u).",(type==CORPSE_BONES?"bones":"corpse"),GUID_LOPART(guid),positionX,positionY,mapid);

            Corpse *corpse = ObjectAccessor::Instance().GetCorpse(positionX,positionY,mapid,guid);
            if(corpse)
                corpse->DeleteFromWorld(true);
            else
                sLog.outDebug("%s %u not found in world. Delete from DB also.",(type==CORPSE_BONES?"Bones":"Corpse"),GUID_LOPART(guid));

            sDatabase.PExecute("DELETE FROM `corpse` WHERE `guid` = '%u';",GUID_LOPART(guid));
            sDatabase.PExecute("DELETE FROM `corpse_grid` WHERE `guid` = '%u';",GUID_LOPART(guid));
        } while (result->NextRow());

        delete result;
    }
}


void HandleCorpsesErase(void*)
{
    mysql_thread_init();                                    // let thread do safe mySQL requests

    //sLog.outBasic("Global Event (corpses/bones removal)");

    CorpsesErase(CORPSE_BONES,             20*60); // 20 mins
    CorpsesErase(CORPSE_RESURRECTABLE,3*24*60*60); // 3 days

    mysql_thread_end();                                     // free mySQL thread resources
}
