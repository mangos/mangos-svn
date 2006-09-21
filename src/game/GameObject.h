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

#ifndef MANGOSSERVER_GAMEOBJECT_H
#define MANGOSSERVER_GAMEOBJECT_H

#include "Common.h"
#include "Object.h"
#include "LootMgr.h"

// Only GCC 4.1.0 and later support #pragma pack(push,1) syntax
#if defined( __GNUC__ ) && (GCC_MAJOR < 4 || GCC_MAJOR == 4 && GCC_MINOR < 1)
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

struct GameObjectInfo
{
    uint32  id;
    uint32  type;
    uint32  displayId;
    char   *name;
    uint32  faction;
    uint32  flags;
    float   size;
    uint32  sound0;
    uint32  sound1;
    uint32  sound2;
    uint32  sound3;
    uint32  sound4;
    uint32  sound5;
    uint32  sound6;
    uint32  sound7;
    uint32  sound8;
    uint32  sound9;
    char   *ScriptName;
};

#if defined( __GNUC__ ) && (GCC_MAJOR < 4 || GCC_MAJOR == 4 && GCC_MINOR < 1)
#pragma pack()
#else
#pragma pack(pop)
#endif

enum LootState
{
    GO_CLOSED = 0,
    GO_OPEN,
    GO_LOOTED
};

class MANGOS_DLL_SPEC GameObject : public Object
{
    public:
        GameObject();
        ~GameObject();

        bool Create(uint32 guidlow, uint32 name_id, uint32 mapid, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3);
        void Update(uint32 p_time);
        GameObjectInfo const* GetGOInfo() const;

        bool IsTransport() const;

        // GO links stored in Unit::m_gameObj
        bool isReferenced() const { return m_refs !=0; }
        void AddRef()                                       // for used in Unit::m_gameObj operations
        {
            ++m_refs;
        }
        void RemoveRef()                                    // for used in Unit::m_gameObj operations
        {
            if(m_refs) --m_refs;
        }

        void SaveToDB();
        bool LoadFromDB(uint32 guid);
        void DeleteFromDB();
        void SetLootState(LootState s) { m_lootState = s; }
        void SetRespawnTimer(uint32 respawn) { m_respawnTimer = respawn; }
        bool isFinished() { return m_respawnTimer == 0;}
        void Delete();
        void SetSpellId(uint32 id) { m_spellId = id;}
        uint32 GetSpellId() { return m_spellId;}
        void getFishLoot(Loot *loot);

        LootState getLootState() { return m_lootState; }
        bool HaveLootSkill() const { return m_lootskill; }
        void SetLootSkill(bool skill) { m_lootskill = skill; }

        Loot        loot;
        uint32      lootid;

    protected:

        void _LoadQuests();

        uint32      m_spellId;
        uint32      m_respawnTimer;
        uint32      m_respawnDelayTime;
        uint32      m_flags;
        LootState   m_lootState;
        bool        m_lootskill;
        uint32      m_refs;
};
#endif
