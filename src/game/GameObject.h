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

#ifndef MANGOSSERVER_GAMEOBJECT_H
#define MANGOSSERVER_GAMEOBJECT_H

#include "Common.h"
#include "SharedDefines.h"
#include "Object.h"
#include "LootMgr.h"
#include "Database/DatabaseEnv.h"

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

// from `gameobject_template`
struct GameObjectInfo
{
    uint32  id;
    uint32  type;
    uint32  displayId;
    char   *name;
    uint32  faction;
    uint32  flags;
    float   size;
    union                                                   // different GO types have different data field
    {
        //0 GAMEOBJECT_TYPE_DOOR
        struct
        {
            uint32 _data0;
            uint32 lockId;                                  //1
        } door;
        //1 GAMEOBJECT_TYPE_BUTTON
        struct
        {
            uint32 _data0;
            uint32 lockId;                                  //1
            uint32 _data2[2];
            uint32 isBattlegroundObject;                    //4
        } button;
        //3 GAMEOBJECT_TYPE_CHEST
        struct
        {
            uint32 lockId;                                  //0
            uint32 lootId;                                  //1
            uint32 _data2[2];
            uint32 minSuccessOpens;                         //4
            uint32 maxSuccessOpens;                         //5
            uint32 eventId;                                 //6
            uint32 _data7;
            uint32 questId;                                 //8 not used currently but store quest required for GO activation for player
            uint32 _data9[5];
            uint32 _data14;                                 //14 something == trap.data12 == goober.data14 ???
        } chest;
        //6 GAMEOBJECT_TYPE_TRAP
        struct
        {
            uint32 _data0;                                  //0 lockid???
            uint32 _data1;
            uint32 radius;                                  //2
            uint32 spellId;                                 //3
            uint32 isNeedDespawn;                           //4 (if >0)
            uint32 _data5[7];
            uint32 _data12;                                 //12 something == chest.data14 == goober.data14 ???
        } trap;
        //8 GAMEOBJECT_TYPE_SPELL_FOCUS
        struct
        {
            uint32 focusId;                                 //0
            uint32 dist;                                    //1
            uint32 linkedTrapId;                            //2
        } spellFocus;
        //10 GAMEOBJECT_TYPE_GOOBER
        struct
        {
            uint32 _data0;                                  //0 lockid ???
            uint32 questId;                                 //1
            uint32 eventId;                                 //2
            uint32 _data3[7];
            uint32 spellId;                                 //10
            uint32 _data11;
            uint32 linkedTrapId;                            //12
            uint32 _data13;
            uint32 _data14;                                 //14 something == trap.data12 == chest.data14 ???
            uint32 _data15;
            uint32 isBattlegroundObject;                    //16
        } goober;
        //13 GAMEOBJECT_TYPE_CAMERA
        struct
        {
            uint32 _data0;
            uint32 cinematicId;                             //1
        } camera;
        //15 GAMEOBJECT_TYPE_MO_TRANSPORT
        struct
        {
            uint32 taxiPathId;                              //0
        } moTransport;
        //17 GAMEOBJECT_TYPE_FISHINGNODE
        struct
        {
            uint32 _data0;
            uint32 lootId;                                  //1
        } fishnode;
        //18 GAMEOBJECT_TYPE_SUMMONING_RITUAL
        struct
        {
            uint32 reqParticipants;                         //0
            uint32 spellId;                                 //1
        } summoningRitual;
        //22 GAMEOBJECT_TYPE_SPELLCASTER
        struct
        {
            uint32 spellId;                                 //0
            uint32 charges;                                 //1
            uint32 partyOnly;                               //2
            uint32 spellId2;                                //3 (is it used in this way?)
        } spellcaster;
        //23 GAMEOBJECT_TYPE_MEETINGSTONE
        struct
        {
            uint32 minLevel;                                //0
            uint32 maxLevel;                                //1
        } meetingstone;
        //25 GAMEOBJECT_TYPE_FISHINGHOLE                    // not implemented yet
        struct
        {
            uint32 radius;                                  //0 how close bobber must land for sending loot
            uint32 lootId;                                  //1
            uint32 minSuccessOpens;                         //2
            uint32 maxSuccessOpens;                         //3
            uint32 lockId;                                  //4 possibly 1628 for all?
        } fishinghole;

        // not use for specific field access (only for output with loop by all filed), also this determinate max union size
        struct                                              // GAMEOBJECT_TYPE_SPELLCASTER
        {
            uint32 data[24];
        } raw;
    };
    char   *ScriptName;
};

struct GameObjectLocale
{
    std::vector<std::string> Name;
};

// from `ganeobject`
struct GameObjectData
{
    uint32 id;                                              // entry in gamobject_template
    uint32 mapid;
    float posX;
    float posY;
    float posZ;
    float orientation;
    float rotation0;
    float rotation1;
    float rotation2;
    float rotation3;
    int32  spawntimesecs;
    uint32 animprogress;
    uint32 go_state;
};

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined( __GNUC__ )
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
        explicit GameObject( WorldObject *instantiator );
        ~GameObject();

        void AddToWorld();
        void RemoveFromWorld();

        bool Create(uint32 guidlow, uint32 name_id, uint32 mapid, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 animprogress, uint32 go_state);
        void Update(uint32 p_time);
        GameObjectInfo const* GetGOInfo() const;

        bool IsTransport() const;

        void SetOwnerGUID(uint64 owner)
        {
            m_spawnedByDefault = false;                     // all object with owner is de-spawned after delay
            SetUInt64Value(OBJECT_FIELD_CREATED_BY, owner);
        }
        uint64 GetOwnerGUID() const { return GetUInt64Value(OBJECT_FIELD_CREATED_BY); }
        Unit* GetOwner() const;

        uint32 GetDBTableGUIDLow() const { return m_DBTableGuid; }

        void Say(const char* text, const uint32 language, const uint64 TargetGuid) { MonsterSay(text,language,TargetGuid); }
        void Yell(const char* text, const uint32 language, const uint64 TargetGuid) { MonsterYell(text,language,TargetGuid); }
        void TextEmote(const char* text, const uint64 TargetGuid) { MonsterTextEmote(text,TargetGuid); }
        void Whisper(const uint64 receiver, const char* text) { MonsterWhisper(receiver,text); }

        void SaveToDB();
        bool LoadFromDB(uint32 guid, uint32 InstanceId);
        void DeleteFromDB();
        void SetLootState(LootState s) { m_lootState = s; }
        static uint32 GetLootId(GameObjectInfo const* info);
        uint32 GetLootId() const { return GetLootId(GetGOInfo()); }
        uint32 GetLockId() const
        {
            switch(GetGoType())
            {
                case GAMEOBJECT_TYPE_DOOR: return GetGOInfo()->door.lockId;
                case GAMEOBJECT_TYPE_CHEST: return GetGOInfo()->chest.lockId;
                default: return 0;
            }
        }

        void SetRespawnTime(int32 respawn)
        {
            m_respawnTime = respawn > 0 ? time(NULL) + respawn : 0;
            m_respawnDelayTime = respawn > 0 ? respawn : 0;
        }
        void Respawn();
        bool isSpawned() const
        {
            return m_respawnDelayTime == 0 ||
                m_respawnTime > 0 && !m_spawnedByDefault ||
                m_respawnTime == 0 && m_spawnedByDefault;
        }
        bool isSpawnedByDefault() const { return m_spawnedByDefault; }
        void Refresh();
        void Delete();
        void SetSpellId(uint32 id) { m_spellId = id;}
        uint32 GetSpellId() const { return m_spellId;}
        void getFishLoot(Loot *loot);
        uint32 GetGoType() const { return GetUInt32Value(GAMEOBJECT_TYPE_ID); }

        LootState getLootState() const { return m_lootState; }

        void AddToSkillupList(uint32 PlayerGuidLow) { m_SkillupList.push_back(PlayerGuidLow); }
        bool IsInSkillupList(uint32 PlayerGuidLow) const
        {
            for (std::list<uint32>::const_iterator i = m_SkillupList.begin(); i != m_SkillupList.end(); ++i)
                if (*i == PlayerGuidLow) return true;
            return false;
        }
        void ClearSkillupList() { m_SkillupList.clear(); }

        void AddUniqueUse(Player* player);
        void AddUse() { ++m_usetimes; }
        
        uint32 GetUseCount() const { return m_usetimes; }
        uint32 GetUniqueUseCount() const { return m_unique_users.size(); }

        void SaveRespawnTime();

        Loot        loot;

        bool hasQuest(uint32 quest_id) const;
        bool hasInvolvedQuest(uint32 quest_id) const;
        bool ActivateToQuest(Player *pTarget) const;

        uint32 GetLinkedGameObjectEntry() const
        {
            switch(GetGoType())
            {
                case GAMEOBJECT_TYPE_SPELL_FOCUS: return GetGOInfo()->spellFocus.linkedTrapId;
                case GAMEOBJECT_TYPE_GOOBER:      return GetGOInfo()->goober.linkedTrapId;
                default: return 0;
            }
        }
        void TriggeringLinkedGameObject( uint32 trapEntry, Unit* target);

        bool isVisibleForInState(Player const* u, bool inVisibleList) const;

        GridReference<GameObject> &GetGridRef() { return m_gridRef; }
    protected:
        uint32      m_charges;                              // Spell charges for GAMEOBJECT_TYPE_SPELLCASTER (22)
        uint32      m_spellId;
        time_t      m_respawnTime;                          // (secs) time of next respawn (or despawn if GO have owner()),
        uint32      m_respawnDelayTime;                     // (secs) if 0 then current GO state no dependent from timer
        uint32      m_flags;
        LootState   m_lootState;
        bool        m_spawnedByDefault;
        time_t      m_environmentcastTime;
        std::list<uint32> m_SkillupList;

        std::set<uint32> m_unique_users;
        uint32 m_usetimes;

        uint32 m_DBTableGuid;
    private:
        GridReference<GameObject> m_gridRef;
};
#endif
