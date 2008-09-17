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

#include "Database/DatabaseEnv.h"
#include "GridDefines.h"
#include "Policies/SingletonImp.h"
#include "WaypointManager.h"
#include "ProgressBar.h"

INSTANTIATE_SINGLETON_1(WaypointManager);

/// - Insert after the last point
void WaypointManager::AddLastNode(uint32 id, float x, float y, float z, float o, uint32 delay, uint32 wpGuid)
{
    _addNode(id, GetLastPoint(id, 0) + 1, x, y, z, o, delay, wpGuid);
}

/// - Insert after a certain point
void WaypointManager::AddAfterNode(uint32 id, uint32 point, float x, float y, float z, float o, uint32 delay, uint32 wpGuid)
{
    for(uint32 i = GetLastPoint(id, 0); i > point; i--)
        WorldDatabase.PExecuteLog("UPDATE creature_movement SET point=point+1 WHERE id='%u' AND point='%u'", id, i);

    _addNode(id, point + 1, x, y, z, o, delay, wpGuid);
}

/// - Insert without checking for collision
void WaypointManager::_addNode(uint32 id, uint32 point, float x, float y, float z, float o, uint32 delay, uint32 wpGuid)
{
    WorldDatabase.PExecuteLog("INSERT INTO creature_movement (id,point,position_x,position_y,position_z,orientation,wpguid,waittime) VALUES ('%u','%u','%f', '%f', '%f', '%f', '%d')", id, point, x, y, z, o, wpGuid, delay);
}

uint32 WaypointManager::GetLastPoint(uint32 id, uint32 default_notfound)
{
    uint32 point = default_notfound;
    QueryResult *result = WorldDatabase.PQuery( "SELECT MAX(point) FROM creature_movement WHERE id = '%u'", id);
    if( result )
    {
        point = (*result)[0].GetUInt32()+1;
        delete result;
    }
    return point;
}

void WaypointManager::DeleteNode(uint32 id, uint32 point)
{
    WorldDatabase.PExecuteLog("DELETE FROM creature_movement WHERE id='%u' AND point='%u'", id, point);
    WorldDatabase.PExecuteLog("UPDATE creature_movement SET point=point-1 WHERE id='%u' AND point>'%u'", id, point);
}

void WaypointManager::DeletePath(uint32 id)
{
    WorldDatabase.PExecuteLog("DELETE FROM creature_movement WHERE id='%u'", id);
}

void WaypointManager::SetNodePosition(uint32 id, uint32 point, float x, float y, float z)
{
    WorldDatabase.PExecuteLog("UPDATE creature_movement SET position_x = '%f',position_y = '%f',position_z = '%f' where id = '%u' AND point='%u'", x, y, z, id, point);
}

void WaypointManager::SetNodeText(uint32 id, uint32 point, const char *text_field, const char *text)
{
    if(!text_field) return;
    std::string field = text_field;
    WorldDatabase.escape_string(field);

    if(!text)
    {
        WorldDatabase.PExecuteLog("UPDATE creature_movement SET %s=NULL WHERE id='%u' AND point='%u'", field.c_str(), id, point);
    }
    else
    {
        std::string text2 = text;
        WorldDatabase.escape_string(text2);
        WorldDatabase.PExecuteLog("UPDATE creature_movement SET %s='%s' WHERE id='%u' AND point='%u'", field.c_str(), text2.c_str(), id, point);
    }
}
