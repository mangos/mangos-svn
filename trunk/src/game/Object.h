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

#ifndef _OBJECT_H
#define _OBJECT_H

#include "Common.h"
#include "ByteBuffer.h"
#include "UpdateFields.h"
#include "UpdateData.h"
#include "GameSystem/GridReference.h"

#include <set>
#include <string>

#define CONTACT_DISTANCE            0.5f
#define INTERACTION_DISTANCE        5
#define ATTACK_DISTANCE                 5
#define DETECT_DISTANCE             20                      // max distance to successful detect stealthed unit
#define MAX_VISIBILITY_DISTANCE     (5*SIZE_OF_GRID_CELL/2) // max distance for visible object show, limited by active zone for player based at cell size (active zone = 5x5 cells)
#define DEFAULT_VISIBILITY_DISTANCE (SIZE_OF_GRID_CELL)     // default visible distance

enum TYPE
{
    TYPE_OBJECT         = 1,
    TYPE_ITEM           = 2,
    TYPE_CONTAINER      = 6,
    TYPE_UNIT           = 8,
    TYPE_PLAYER         = 16,
    TYPE_GAMEOBJECT     = 32,
    TYPE_DYNAMICOBJECT  = 64,
    TYPE_CORPSE         = 128,
    TYPE_AIGROUP        = 256,
    TYPE_AREATRIGGER    = 512
};

enum TYPEID
{
    TYPEID_OBJECT        = 0,
    TYPEID_ITEM          = 1,
    TYPEID_CONTAINER     = 2,
    TYPEID_UNIT          = 3,
    TYPEID_PLAYER        = 4,
    TYPEID_GAMEOBJECT    = 5,
    TYPEID_DYNAMICOBJECT = 6,
    TYPEID_CORPSE        = 7,
    TYPEID_AIGROUP       = 8,
    TYPEID_AREATRIGGER   = 9
};

uint32 GuidHigh2TypeId(uint32 guid_hi);

enum TempSummonType
{
    TEMPSUMMON_TIMED_OR_DEAD_DESPAWN       = 1,             // despawns after a specified time OR when the creature disappears
    TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN     = 2,             // despawns after a specified time OR when the creature dies
    TEMPSUMMON_TIMED_DESPAWN               = 3,             // despawns after a specified time
    TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT = 4,             // despawns after a specified time after the creature is out of combat
    TEMPSUMMON_CORPSE_DESPAWN              = 5,             // despawns instantly after death
    TEMPSUMMON_CORPSE_TIMED_DESPAWN        = 6,             // despawns after a specified time after death
    TEMPSUMMON_DEAD_DESPAWN                = 7,             // despawns when the creature disappears
    TEMPSUMMON_MANUAL_DESPAWN              = 8              // despawns when UnSummon() is called
};

class WorldPacket;
class UpdateData;
class ByteBuffer;
class WorldSession;
class Creature;
class Player;
class Map;
class MapCell;
class UpdateMask;
class InstanceData;

typedef HM_NAMESPACE::hash_map<Player*, UpdateData> UpdateDataMapType;

class MANGOS_DLL_SPEC Object
{
    public:
        virtual ~Object ( );

        const bool& IsInWorld() const { return m_inWorld; }
        virtual void AddToWorld()
        {
            if(m_inWorld)
                return;

            m_inWorld = true;

            // synchronize values mirror with values array (changes will send in updatecreate opcode any way
            ClearUpdateMask(true);
        }
        virtual void RemoveFromWorld() { m_inWorld = false; }

        const uint64& GetGUID() const { return GetUInt64Value(0); }
        const uint32& GetGUIDLow() const { return GetUInt32Value(0); }
        const uint32& GetGUIDHigh() const { return GetUInt32Value(1); }
        const ByteBuffer& GetPackGUID() const { return m_PackGUID; }
        uint32 GetEntry() const { return GetUInt32Value(OBJECT_FIELD_ENTRY); }

        const uint8& GetTypeId() const { return m_objectTypeId; }
        bool isType(uint8 mask) const { return (mask & m_objectType); }

        virtual void BuildCreateUpdateBlockForPlayer( UpdateData *data, Player *target ) const;
        void SendUpdateToPlayer(Player* player);

        void BuildValuesUpdateBlockForPlayer( UpdateData *data, Player *target ) const;
        void BuildOutOfRangeUpdateBlock( UpdateData *data ) const;
        void BuildMovementUpdateBlock( UpdateData * data, uint32 flags = 0 ) const;
        void BuildUpdate(UpdateDataMapType &);

        virtual void DestroyForPlayer( Player *target ) const;

        const int32& GetInt32Value( uint16 index ) const
        {
            ASSERT( index < m_valuesCount || PrintIndexError( index , false) );
            return m_int32Values[ index ];
        }

        const uint32& GetUInt32Value( uint16 index ) const
        {
            ASSERT( index < m_valuesCount || PrintIndexError( index , false) );
            return m_uint32Values[ index ];
        }

        const uint64& GetUInt64Value( uint16 index ) const
        {
            ASSERT( index + 1 < m_valuesCount || PrintIndexError( index , false) );
            return *((uint64*)&(m_uint32Values[ index ]));
        }

        const float& GetFloatValue( uint16 index ) const
        {
            ASSERT( index < m_valuesCount || PrintIndexError( index , false ) );
            return m_floatValues[ index ];
        }

        void SetInt32Value(  uint16 index,        int32  value );
        void SetUInt32Value( uint16 index,       uint32  value );
        void SetUInt64Value( uint16 index, const uint64 &value );
        void SetFloatValue(  uint16 index,       float   value );
        void SetStatFloatValue( uint16 index, float value);
        void SetStatInt32Value( uint16 index, int32 value);

        void ApplyModUInt32Value(uint16 index, int32 val, bool apply);
        void ApplyModInt32Value(uint16 index, int32 val, bool apply);
        void ApplyModUInt64Value(uint16 index, int32 val, bool apply);
        void ApplyModPositiveFloatValue( uint16 index, float val, bool apply);
        void ApplyModSignedFloatValue( uint16 index, float val, bool apply);

        void ApplyPercentModFloatValue(uint16 index, float val, bool apply)
        {
            val = val != -100.0f ? val : -99.9f ;
            SetFloatValue(index, GetFloatValue(index) * (apply?(100.0f+val)/100.0f : 100.0f / (100.0f+val)) );
        }

        void SetFlag( uint16 index, uint32 newFlag );

        void RemoveFlag( uint16 index, uint32 oldFlag );

        void ToggleFlag( uint16 index, uint32 flag)
        {
            if(HasFlag(index, flag))
                RemoveFlag(index, flag);
            else
                SetFlag(index, flag);
        }

        bool HasFlag( uint16 index, uint32 flag ) const
        {
            ASSERT( index < m_valuesCount || PrintIndexError( index , false ) );
            return (m_uint32Values[ index ] & flag) != 0;
        }

        void ApplyModFlag( uint16 index, uint32 flag, bool apply)
        {
            if(apply) SetFlag(index,flag); else RemoveFlag(index,flag);
        }

        void ClearUpdateMask(bool remove);
        void SendUpdateObjectToAllExcept(Player* exceptPlayer);

        bool LoadValues(const char* data);

        uint16 GetValuesCount() const { return m_valuesCount; }

        void InitValues() { _InitValues(); }

        virtual bool hasQuest(uint32 /* quest_id */) const { return false; }
        virtual bool hasInvolvedQuest(uint32 /* quest_id */) const { return false; }
    protected:

        Object ( );

        void _InitValues();
        void _Create (uint32 guidlow, uint32 guidhigh);

        virtual void _SetUpdateBits(UpdateMask *updateMask, Player *target) const;

        virtual void _SetCreateBits(UpdateMask *updateMask, Player *target) const;
        void _BuildMovementUpdate(ByteBuffer * data, uint8 flags, uint32 flags2 ) const;
        void _BuildValuesUpdate(uint8 updatetype, ByteBuffer *data, UpdateMask *updateMask, Player *target ) const;

        uint16 m_objectType;

        uint8 m_objectTypeId;
        uint8 m_updateFlag;

        union
        {
            int32  *m_int32Values;
            uint32 *m_uint32Values;
            float *m_floatValues;
        };

        uint32 *m_uint32Values_mirror;

        uint16 m_valuesCount;

        bool m_inWorld;

        bool m_objectUpdated;

    private:
        ByteBuffer m_PackGUID;

        // for output helpfull error messages from asserts
        bool PrintIndexError(uint32 index, bool set) const;
        Object(const Object&);                              // prevent generation copy constructor
        Object& operator=(Object const&);                   // prevent generation assigment operator
};

class MANGOS_DLL_SPEC WorldObject : public Object
{
    public:
        virtual ~WorldObject ( ) {}

        virtual void Update ( uint32 /*time_diff*/ ) { }

        void _Create (uint32 guidlow, uint32 guidhigh, uint32 mapid, float x, float y, float z, float ang, uint32 nameId);

        void Relocate(float x, float y, float z, float orientation)
        {
            m_positionX = x;
            m_positionY = y;
            m_positionZ = z;
            m_orientation = orientation;
        }

        void Relocate(float x, float y, float z)
        {
            m_positionX = x;
            m_positionY = y;
            m_positionZ = z;
        }

        void SetOrientation(float orientation) { m_orientation = orientation; }

        float GetPositionX( ) const { return m_positionX; }
        float GetPositionY( ) const { return m_positionY; }
        float GetPositionZ( ) const { return m_positionZ; }
        void GetPosition( float &x, float &y, float &z ) const
            { x = m_positionX; y = m_positionY; z = m_positionZ; }
        float GetOrientation( ) const { return m_orientation; }
        void GetNearPoint2D( float &x, float &y, float distance, float absAngle) const;
        void GetNearPoint( WorldObject const* searcher, float &x, float &y, float &z, float distance2d,float absAngle) const;
        void GetClosePoint(float &x, float &y, float &z, float distance2d = 0, float angle = 0) const
        {
            // angle calculated from current orientation
            GetNearPoint(NULL,x,y,z,distance2d,GetOrientation() + angle);
        }
        void GetContactPoint( const WorldObject* obj, float &x, float &y, float &z, float distance2d = CONTACT_DISTANCE) const
        {
            // angle to face `obj` to `this` using distance includes size of `obj`
            GetNearPoint(obj,x,y,z,distance2d,GetAngle( obj ));
        }
        const float GetObjectSize() const
        {
            return ( m_valuesCount > UNIT_FIELD_BOUNDINGRADIUS ) ? m_floatValues[UNIT_FIELD_BOUNDINGRADIUS] : 0.39f;
        }
        bool IsPositionValid() const;
        void UpdateGroundPositionZ(float x, float y, float &z) const;

        void GetRandomPoint( float x, float y, float z, float distance, float &rand_x, float &rand_y, float &rand_z ) const;

        void SetMapId(uint32 newMap) { m_mapId = newMap; }

        uint32 GetMapId() const { return m_mapId; }

        uint32 GetZoneId() const;
        uint32 GetAreaId() const;

        InstanceData* GetInstanceData();

        const char* GetName() const { return m_name.c_str(); }
        void SetName(std::string newname) { m_name=newname; }

        float GetDistanceSq( const WorldObject* obj ) const { float d = GetDistance(obj); return d*d; }
        float GetDistanceSq(const float x, const float y, const float z) const { float d = GetDistance(x,y,z); return d*d; }
        float GetDistance( const WorldObject* obj ) const;
        float GetDistance(const float x, const float y, const float z) const;
        float GetDistance2dSq( const WorldObject* obj ) const { float d = GetDistance2d(obj); return d*d; }
        float GetDistance2d(const WorldObject* obj) const;
        float GetDistance2d(const float x, const float y) const;
        float GetDistanceZ(const WorldObject* obj) const;
        bool IsInMap(const WorldObject* obj) const { return GetMapId()==obj->GetMapId() && GetInstanceId()==obj->GetInstanceId(); }
        bool IsWithinDistInMap(const WorldObject* obj, const float dist2compare) const;
        bool IsWithinLOS(const float x, const float y, const float z ) const;
        bool IsWithinLOSInMap(const WorldObject* obj) const;

        float GetAngle( const WorldObject* obj ) const;
        float GetAngle( const float x, const float y ) const;
        bool HasInArc( const float arcangle, const WorldObject* obj ) const;

        virtual void SendMessageToSet(WorldPacket *data, bool self);
        void BuildHeartBeatMsg( WorldPacket *data ) const;
        void BuildTeleportAckMsg( WorldPacket *data, float x, float y, float z, float ang) const;
        bool IsBeingTeleported() { return mSemaphoreTeleport; }
        void SetSemaphoreTeleport(bool semphsetting) { mSemaphoreTeleport = semphsetting; }

        void MonsterSay(const char* text, const uint32 language, const uint64 TargetGuid);
        void MonsterYell(const char* text, const uint32 language, const uint64 TargetGuid);
        void MonsterTextEmote(const char* text, const uint64 TargetGuid);
        void MonsterWhisper(const uint64 receiver, const char* text);

        void SendDestroyObject(uint64 guid);
        void SendObjectDeSpawnAnim(uint64 guid);

        virtual void SaveRespawnTime() {}

        uint32 GetInstanceId() const { return m_InstanceId; }
        void SetInstanceId(uint32 val) { m_InstanceId = val; }

        // main visibility check function in normal case (ignore grey zone distance check)
        bool isVisibleFor(Player const* u) const { return isVisibleForInState(u,false); }

        // low level function for visibility change code, must be define in all main world object subclasses
        virtual bool isVisibleForInState(Player const* u, bool inVisibleList) const = 0;

        Map      * GetMap() const;
        Map const* GetBaseMap() const;
        Creature* SummonCreature(uint32 id, float x, float y, float z, float ang,TempSummonType spwtype,uint32 despwtime);
    protected:
        explicit WorldObject( WorldObject *instantiator );
        std::string m_name;

    private:
        uint32 m_mapId;

        float m_positionX;
        float m_positionY;
        float m_positionZ;
        float m_orientation;

        bool mSemaphoreTeleport;

        uint32 m_InstanceId;
};
#endif
