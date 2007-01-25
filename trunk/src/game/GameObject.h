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
#include "Database/DatabaseEnv.h"

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

// For containers: [GO_NOT_READY] -> GO_CLOSED -> GO_OPEN -> GO_LOOTED -> GO_CLOSED -> ...
// For bobber:     GO_NOT_READY   -> GO_CLOSED -> GO_OPEN -> GO_LOOTED -> <deleted>
enum LootState
{
    GO_NOT_READY = 0,
    GO_CLOSED,
    GO_OPEN,
    GO_LOOTED
};

class Unit;

// 5 sec for bobber catch
#define FISHING_BOBBER_READY_TIME 5

class MANGOS_DLL_SPEC GameObject : public WorldObject
{
    public:
        GameObject();
        ~GameObject();

        bool Create(uint32 guidlow, uint32 name_id, uint32 mapid, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 animprogress, uint32 dynflags);
        void Update(uint32 p_time);
        GameObjectInfo const* GetGOInfo() const;

        bool IsTransport() const;

        void SetOwnerGUID(uint64 owner) { SetUInt64Value(OBJECT_FIELD_CREATED_BY, owner); }
        uint64 GetOwnerGUID() const { return GetUInt64Value(OBJECT_FIELD_CREATED_BY); }
        Unit* GetOwner() const;

        void SaveToDB();
        bool LoadFromDB(uint32 guid, QueryResult *result = NULL);
        void DeleteFromDB();
        void SetLootState(LootState s) { m_lootState = s; }
        void SetRespawnTime(uint32 respawn) { m_respawnTime = respawn ? time(NULL) + respawn : 0; }
        void Respawn() { if(m_respawnTime > 0) m_respawnTime = time(NULL); }
        bool isFinished() { return m_respawnTime == 0;}
        void Refresh();
        void Delete();
        void SetSpellId(uint32 id) { m_spellId = id;}
        uint32 GetSpellId() { return m_spellId;}
        void getFishLoot(Loot *loot);
        uint32 GetGoType() { return GetUInt32Value(GAMEOBJECT_TYPE_ID); }

        LootState getLootState() { return m_lootState; }

        void AddToSkillupList(uint32 PlayerGuidLow) { m_SkillupList.push_back(PlayerGuidLow); }
        bool IsInSkillupList(uint32 PlayerGuidLow)
        {
            for (std::list<uint32>::iterator i = m_SkillupList.begin(); i != m_SkillupList.end(); ++i)
                if (*i == PlayerGuidLow) return true;
            return false;
        }

        void CountUseTimes();

        void SaveRespawnTime();

        Loot        loot;
        uint32      lootid;

        uint32      m_usetimes;

    protected:

        void _LoadQuests();

        uint32      m_spellId;
        time_t      m_respawnTime;                          // (secs) time of next respawn
        uint32      m_respawnDelayTime;                     // (secs)
        uint32      m_flags;
        LootState   m_lootState;
        std::list<uint32> m_SkillupList;
};
#endif
