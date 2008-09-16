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

#ifndef MANGOS_WAYPOINTMANAGER_H
#define MANGOS_WAYPOINTMANAGER_H

class WaypointManager
{
    public:
        WaypointManager() {}
        ~WaypointManager() {}

        void AddLastNode(uint32 id, float x, float y, float z, float o, uint32 delay, uint32 wpGuid);
        void AddAfterNode(uint32 id, uint32 point, float x, float y, float z, float o, uint32 delay, uint32 wpGuid);
        uint32 GetLastPoint(uint32 id, uint32 default_notfound);
        void DeleteNode(uint32 id, uint32 point);
        void DeletePath(uint32 id);
        void SetNodePosition(uint32 id, uint32 point, float x, float y, float z);
        void SetNodeText(uint32 id, uint32 point, const char *text_field, const char *text);

    private:
        void _addNode(uint32 id, uint32 point, float x, float y, float z, float o, uint32 delay, uint32 wpGuid);
};

#define WaypointMgr MaNGOS::Singleton<WaypointManager>::Instance()

#endif
