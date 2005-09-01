/* GameObject.h
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

#ifndef WOWSERVER_GAMEOBJECT_H
#define WOWSERVER_GAMEOBJECT_H

#include "Object.h"

class GameObject : public Object
{
    public:
        GameObject( );

        void Create ( uint32 guidlow, uint32 display_id, uint8 state, uint32 obj_field_entry,
            float scale, uint16 type, uint16 faction, uint32 mapid,
            float x, float y, float z, float ang );

        void Update(uint32 p_time);

        void Despawn(uint32 time);
        bool FillLoot(WorldPacket *data);
        void FillItemList();

        int getItemAmount(int slot) { return m_ItemAmount[slot]; }
        void setItemAmount(int slot, uint32 value) { m_ItemAmount[slot] = value; }
        uint32 getItemId(int slot) { return m_ItemList[slot]; }
        bool hasLoot()
        {
            bool hasLoot = false;
            for(int i=0;i<10;i++)
                if(m_ItemAmount[i] && m_ItemAmount[i] != 0)
                    hasLoot = true;
            return hasLoot;

        }

// Serialization
        void SaveToDB();
        void LoadFromDB(uint32 guid);
        void DeleteFromDB();

    protected:

        uint32 m_RespawnTimer;
        uint32 m_gold;
        uint32 m_ItemList[10],m_ItemAmount[10];
        uint8 m_ItemCount;
};
#endif
