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

#ifndef MANGOS_MAP_INSTANCED_H
#define MANGOS_MAP_INSTANCED_H

#include "Map.h"

class MANGOS_DLL_DECL MapInstanced : public Map
{
    public:

        MapInstanced(uint32 id, time_t, uint32 aInstanceId);

        virtual void Update(const uint32&);
        virtual void MoveAllCreaturesInMoveList();
        virtual bool RemoveBones(uint64 guid, float x, float y);
        virtual void UnloadAll();

        Map* GetInstance(const WorldObject* obj);
        bool IsValidInstance(uint32 InstanceId);

    private:

        void CreateInstance(uint32 InstanceId, Map* &map);
    
        HM_NAMESPACE::hash_map< uint32, Map* > InstancedMaps;

        Map* _FindMap(uint32 InstanceId)
        {
            HM_NAMESPACE::hash_map< uint32, Map* >::iterator i = InstancedMaps.find(InstanceId);

            return(i == InstancedMaps.end() ? NULL : i->second);
        }
};

#endif
