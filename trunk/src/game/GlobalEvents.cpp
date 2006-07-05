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

Corpse *m_pCorpse;

void HandleCorpsesErase(void*)
{
    sLog.outBasic("Global Event (corpses/bones removal)");

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `game_corpse` WHERE UNIX_TIMESTAMP()-UNIX_TIMESTAMP(`time`) > 1200 AND `bones_flag` = 1;");

    if(result)
    {
        Field *fields = result->Fetch();
        m_pCorpse = new Corpse();

        uint64 guid = fields[0].GetUInt64();
        float positionX = fields[2].GetFloat();
        float positionY = fields[3].GetFloat();
        float positionZ = fields[4].GetFloat();
        float ort       = fields[5].GetFloat();
        uint32 mapid    = fields[7].GetUInt32();

        m_pCorpse->Relocate(positionX,positionY,positionZ,ort);
        m_pCorpse->SetMapId(mapid);
        m_pCorpse->LoadValues( fields[8].GetString() );

        ObjectAccessor::Instance().RemoveBonesFromPlayerView(m_pCorpse);
        MapManager::Instance().GetMap(m_pCorpse->GetMapId())->Remove(m_pCorpse,true);

        sDatabase.PExecute("DELETE FROM `game_corpse` WHERE guid = '%ul';",(unsigned long)guid);

        m_pCorpse=NULL;
        delete result;
    }

    result = sDatabase.PQuery("SELECT * FROM `game_corpse` WHERE UNIX_TIMESTAMP()-UNIX_TIMESTAMP(`time`) > 259200 AND `bones_flag` = 0;");

    if(result)
    {

        Field *fields = result->Fetch();
        m_pCorpse = new Corpse();

        uint64 guid = fields[0].GetUInt64();
        float positionX = fields[2].GetFloat();
        float positionY = fields[3].GetFloat();
        float positionZ = fields[4].GetFloat();
        float ort       = fields[5].GetFloat();
        uint32 mapid    = fields[7].GetUInt32();

        m_pCorpse->Relocate(positionX,positionY,positionZ,ort);
        m_pCorpse->SetMapId(mapid);
        m_pCorpse->LoadValues( fields[8].GetString() );

        ObjectAccessor::Instance().RemoveBonesFromPlayerView(m_pCorpse);
        MapManager::Instance().GetMap(m_pCorpse->GetMapId())->Remove(m_pCorpse,true);

        sDatabase.PExecute("DELETE FROM `game_corpse` WHERE `guid` = '%ul';",(unsigned long)guid);

        m_pCorpse=NULL;
        delete result;
    }
}
