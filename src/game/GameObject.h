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

#ifndef MANGOSSERVER_GAMEOBJECT_H
#define MANGOSSERVER_GAMEOBJECT_H

#include "Object.h"

struct GameObjectInfo
{
    GameObjectInfo(uint32 i=0, uint32 t=0, uint32 dis_id=0, uint32 f = 0, uint32 fl = 0, uint32 s0=0, uint32 s1=0, uint32 s2=0, uint32 s3 =0, uint32 s4=0,
           uint32 s5 = 0, uint32 s6 = 0, uint32 s7 = 0, 
           uint32 s8 = 0, uint32 s9 = 0, float sz = 1.0, const char *n = NULL) : id(i), type(t),displayId(dis_id), faction(f), flags(fl), sound0(s0), sound1(s1), sound2(s2), sound3(s3), sound4(s4), sound5(s5), sound6(s6), sound7(s7), sound8(s8), sound9(s9), size(sz), name(n == NULL ? "Unknown Object" : n) {}
    uint32 id;
    uint32 type;
    uint32 displayId;
    uint32 faction;
    uint32 flags;
    uint32 sound0;
    uint32 sound1;
    uint32 sound2;
    uint32 sound3;
    uint32 sound4;
    uint32 sound5;
    uint32 sound6;
    uint32 sound7;
    uint32 sound8;
    uint32 sound9;
    float size;
    std::string name;
};

class GameObject : public Object
{
public:
    GameObject( );
    
    void Create(uint32 guidlow, uint32 name_id, uint32 mapid, float x, float y, float z, float ang);
    void Update(uint32 p_time);    
#ifndef ENABLE_GRID_SYSTEM
    void Despawn(uint32 time);
#endif
    bool FillLoot(Player &, WorldPacket *data);
        
    // Serialization
    void SaveToDB();
    void LoadFromDB(uint32 guid);
    void DeleteFromDB();
    
protected:
    void _generateLoot(Player &, std::vector<uint32> &, std::vector<uint32>&, std::vector<uint32> &, uint32 &) const;    
    uint32 m_RespawnTimer;

    time_t m_nextThinkTime;
};

#endif
