/* Object.h
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

#ifndef _OBJECT_H
#define _OBJECT_H

#include "UpdateMask.h"
#include "World.h"

// TODO: fix that type mess

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

class WorldPacket;
class UpdateData;
class ByteBuffer;
class WorldSession;
class Player;
class MapCell;

//====================================================================
//  Object
//  Base object for every item, unit, player, corpse, container, etc
//====================================================================
class Object
{
    public:
        typedef std::set<Object*> InRangeSet;

        virtual ~Object ( );

        virtual void Update ( float time ) { }

        const bool& IsInWorld() const { return m_inWorld; }
        virtual void AddToWorld() { m_inWorld = true; }
        virtual void RemoveFromWorld() { m_inWorld = false; }

// guid always comes first
        const uint64& GetGUID() const { return *((uint64*)m_uint32Values); }

// should be removed later
        const uint32& GetGUIDLow() const { return m_uint32Values[0]; }
        const uint32& GetGUIDHigh() const { return m_uint32Values[1]; }

// type
        const uint8& GetTypeId() const { return m_objectTypeId; }

//    void BuildUpdateMsgHeader( WorldPacket *data ) const;

//! This includes any nested objects we have, inventory for example.
        virtual void BuildCreateUpdateBlockForPlayer( UpdateData *data, Player *target ) const;
        void BuildValuesUpdateBlockForPlayer( UpdateData *data, Player *target ) const;
        void BuildOutOfRangeUpdateBlock( UpdateData *data ) const;
        void BuildMovementUpdateBlock( UpdateData * data, uint32 flags = 0 ) const;

        virtual void DestroyForPlayer( Player *target ) const;

        void BuildHeartBeatMsg( WorldPacket *data ) const;
        void BuildTeleportAckMsg( WorldPacket *data, float x, float y, float z, float ang) const;
        bool IsBeingTeleported() { return mSemaphoreTeleport; }
        void SetSemaphoreTeleport(bool semphsetting) { mSemaphoreTeleport = semphsetting; }

        bool SetPosition( float newX, float newY, float newZ, float newOrientation, bool allowPorting = false );

        const float& GetPositionX( ) const { return m_positionX; }
        const float& GetPositionY( ) const { return m_positionY; }
        const float& GetPositionZ( ) const { return m_positionZ; }
        const float& GetOrientation( ) const { return m_orientation; }

//! Only for MapMgr use
        MapCell* GetMapCell() const { return m_mapCell; }
//! Only for MapMgr use
        void SetMapCell(MapCell* cell) { m_mapCell = cell; }

        const uint32& GetTaximask( uint8 index ) const { return m_taximask[index]; }
        void SetTaximask( uint8 index, uint32 value ) { m_taximask[index] = value; }

        void SetMapId(uint32 newMap) { m_mapId = newMap; }
        void SetZoneId(uint32 newZone) { m_zoneId = newZone; }

        const uint32& GetMapId( ) const { return m_mapId; }
        const uint32& GetZoneId( ) const { return m_zoneId; }

//! Get uint32 property
        const uint32& GetUInt32Value( const uint16 &index ) const
        {
            ASSERT( index < m_valuesCount );
            return m_uint32Values[ index ];
        }

//! Get uint64 property
        const uint64& GetUInt64Value( const uint16 &index ) const
        {
            ASSERT( index + 1 < m_valuesCount );
            return *((uint64*)&(m_uint32Values[ index ]));
        }

//! Get float property
        const float& GetFloatValue( const uint16 &index ) const
        {
            ASSERT( index < m_valuesCount );
            return m_floatValues[ index ];
        }

//! Set uint32 property
        void SetUInt32Value( const uint16 &index, const uint32 &value );

//! Set uint64 property
        void SetUInt64Value( const uint16 &index, const uint64 &value );

//! Set float property
        void SetFloatValue( const uint16 &index, const float &value );

        void SetFlag( const uint16 &index, uint32 newFlag );

        void RemoveFlag( const uint16 &index, uint32 oldFlag );

        bool HasFlag( const uint16 &index, uint32 flag ) const
        {
            ASSERT( index < m_valuesCount );
            return (m_uint32Values[ index ] & flag) != 0;
        }

        void ClearUpdateMask( )
        {
            m_updateMask.Clear();
            m_objectUpdated = false;
        }

        float GetDistanceSq(Object* obj) const
        {
            ASSERT(obj->GetMapId() == m_mapId);

            float dx  = obj->GetPositionX() - GetPositionX();
            float dy  = obj->GetPositionY() - GetPositionY();
            float dz  = obj->GetPositionZ() - GetPositionZ();

            return ((dx*dx) + (dy*dy) + (dz*dz));
        }

        float GetDistance2dSq(Object* obj) const
        {
            ASSERT(obj->GetMapId() == m_mapId);

            float dx  = obj->GetPositionX() - GetPositionX();
            float dy  = obj->GetPositionY() - GetPositionY();

            return (dx*dx) + (dy*dy);
        }

// In-range object management, not sure if we need it
        bool IsInRangeSet(Object* pObj) { return !(m_objectsInRange.find(pObj) == m_objectsInRange.end()); }
        virtual void AddInRangeObject(Object* pObj) { m_objectsInRange.insert(pObj); }
        virtual void RemoveInRangeObject(Object* pObj) { m_objectsInRange.erase(pObj); }
        void ClearInRangeSet() { m_objectsInRange.clear(); }

        inline InRangeSet::iterator GetInRangeSetBegin() { return m_objectsInRange.begin(); }
        inline InRangeSet::iterator GetInRangeSetEnd() { return m_objectsInRange.end(); }

        void SendMessageToSet(WorldPacket *data, bool self);

//! Fill values with data from a space seperated string of uint32s.
        void LoadValues(const char* data);
        void LoadTaxiMask(const char* data);

        uint16 GetValuesCount() const { return m_valuesCount; }

//! Add object to map
        void PlaceOnMap();
//! Remove object from map
        void RemoveFromMap();

    protected:
        Object ( );

        void _InitValues()
        {
            m_uint32Values = new uint32[ m_valuesCount ];

            WPAssert(m_uint32Values);
            memset(m_uint32Values, 0, m_valuesCount*sizeof(uint32));

            m_updateMask.SetCount(m_valuesCount);
            ClearUpdateMask();
        }

        void _Create (uint32 guidlow, uint32 guidhigh);
        void _Create (uint32 guidlow, uint32 guidhigh, uint32 mapid, float x, float y, float z, float ang);

//! Mark values that need updating for specified player.
        virtual void _SetUpdateBits(UpdateMask *updateMask, Player *target) const;
//! Mark values that player should get when he/she/it sees object for first time.
        virtual void _SetCreateBits(UpdateMask *updateMask, Player *target) const;

        void _BuildMovementUpdate( ByteBuffer *data, uint32 flags, uint32 flags2 ) const;
        void _BuildValuesUpdate( ByteBuffer *data, UpdateMask *updateMask  ) const;

//! Types. Bitmasked together by subclasses.
        uint16 m_objectType;
//! Type id.
        uint8 m_objectTypeId;

//! Zone id.
        uint32 m_zoneId;
//! Continent/map id.
        uint32 m_mapId;
//! Map manager
        MapMgr *m_mapMgr;
//! Current map cell
        MapCell *m_mapCell;

// TODO: use vectors here
        float m_positionX;
        float m_positionY;
        float m_positionZ;
        float m_orientation;
        uint32 m_taximask[8];

//! Blizzard seem to send those for all object types. weird.
        float m_walkSpeed;
        float m_runSpeed;
        float m_backWalkSpeed;
        float m_swimSpeed;
        float m_backSwimSpeed;
        float m_turnRate;

// Semaphores - needed to forbid two operations on the same object at the same very time (may cause crashing\lack of data)
        bool mSemaphoreTeleport;

//! TODO: Should be removed later.
        float m_minZ;

//! Object properties.
        union
        {
            uint32 *m_uint32Values;
            float *m_floatValues;
        };

//! Number of properties
        uint16 m_valuesCount;

//! List of object properties that need updating.
        UpdateMask m_updateMask;

//! True if object exists in world
        bool m_inWorld;

//! True if object was updated
        bool m_objectUpdated;

//! Set of Objects in range.
//! TODO: that functionality should be moved into WorldServer.
        std::set<Object*> m_objectsInRange;
};
#endif
