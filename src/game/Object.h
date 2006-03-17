/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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

#include <set>


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





class Object
{
    public:
        virtual ~Object ( );

        virtual void Update ( float time ) { }

        const bool& IsInWorld() const { return m_inWorld; }
        virtual void AddToWorld() { m_inWorld = true; }
        virtual void RemoveFromWorld() { m_inWorld = false; }

        
        const uint64& GetGUID() const { return *((uint64*)m_uint32Values); }
        const uint32& GetGUIDLow() const { return m_uint32Values[0]; }
        const uint32& GetGUIDHigh() const { return m_uint32Values[1]; }
		
		inline
		uint32 GetEntry(){return m_uint32Values[OBJECT_FIELD_ENTRY];}
        
	        
        const uint8& GetTypeId() const { return m_objectTypeId; }
        bool isType(uint8 mask) const 
		{ 
			
			if (mask & m_objectType)
				return true;

			return false;
		}

        

        
        virtual void BuildCreateUpdateBlockForPlayer( UpdateData *data, Player *target ) const;
        void BuildValuesUpdateBlockForPlayer( UpdateData *data, Player *target ) const;
        void BuildOutOfRangeUpdateBlock( UpdateData *data ) const;
        void BuildMovementUpdateBlock( UpdateData * data, uint32 flags = 0 ) const;

        virtual void DestroyForPlayer( Player *target ) const;

        void BuildHeartBeatMsg( WorldPacket *data ) const;
        void BuildTeleportAckMsg( WorldPacket *data, float x, float y, float z, float ang) const;
        bool IsBeingTeleported() { return mSemaphoreTeleport; }
        void SetSemaphoreTeleport(bool semphsetting) { mSemaphoreTeleport = semphsetting; }

		void Relocate(const float &x, const float &y, const float &z, const float &orientation)
		{
		m_positionX = x;
		m_positionY = y;
		m_positionZ = z;
		m_orientation = orientation;
		}

        const float& GetPositionX( ) const { return m_positionX; }
        const float& GetPositionY( ) const { return m_positionY; }
        const float& GetPositionZ( ) const { return m_positionZ; }
        const float& GetOrientation( ) const { return m_orientation; }

        const uint32& GetTaximask( uint8 index ) const { return m_taximask[index]; }
        void SetTaximask( uint8 index, uint32 value ) { m_taximask[index] = value; }

        void SetMapId(uint32 newMap) { m_mapId = newMap; }

        const uint32& GetMapId( ) const { return m_mapId; }

        uint32 GetZoneId( );
		
        const uint32& GetUInt32Value( const uint16 &index ) const
        {
            ASSERT( index < m_valuesCount );
            return m_uint32Values[ index ];
        }

        
        const uint64& GetUInt64Value( const uint16 &index ) const
        {
            ASSERT( index + 1 < m_valuesCount );
            return *((uint64*)&(m_uint32Values[ index ]));
        }

        
        const float& GetFloatValue( const uint16 &index ) const
        {
            ASSERT( index < m_valuesCount );
            return m_floatValues[ index ];
        }

        
        void SetUInt32Value( const uint16 &index, const uint32 &value );

        
        void SetUInt64Value( const uint16 &index, const uint64 &value );

        
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

        float GetDistanceSq(const Object* obj) const
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

		float GetFacing(Object* obj) const
        {
            if(!obj) return 0;

			float VictimX = obj->GetPositionX();
			float VictimY = obj->GetPositionY();
			float PlayerX = GetPositionX();
			float PlayerY = GetPositionY();
						
			float dr1 = atan((VictimY - PlayerY) / (VictimX - PlayerX));
			
			if (VictimX >= PlayerX) 
			{
                dr1 += 1.57079633;	//rads (1/4)*2*PI
			} 
			else 
			{
				dr1 += 4.71238898; //rads (3/4)*2*PI
			}
            return (dr1 - GetOrientation());  //default return if in front of Victim 1.57079633 
        }

        void SendMessageToSet(WorldPacket *data, bool self);

        
        void LoadValues(const char* data);
        void LoadTaxiMask(const char* data);

        uint16 GetValuesCount() const { return m_valuesCount; }

		void InitValues() { _InitValues(); }

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
        void _Create (uint32 guidlow, uint32 guidhigh, uint32 mapid, float x, float y, float z, float ang, uint32 nameId);

        
        virtual void _SetUpdateBits(UpdateMask *updateMask, Player *target) const;
        
        virtual void _SetCreateBits(UpdateMask *updateMask, Player *target) const;
        void _BuildMovementUpdate(ByteBuffer * data, uint8 flags, uint32 flags2 ) const;
        void _BuildValuesUpdate( ByteBuffer *data, UpdateMask *updateMask  ) const;

        
        uint16 m_objectType;
        
        uint8 m_objectTypeId;

     
        
        
        uint32 m_mapId;

        
        float m_positionX;
        float m_positionY;
        float m_positionZ;
        float m_orientation;
        uint32 m_taximask[8];

    
        float m_walkSpeed;
        float m_runSpeed;
        float m_backWalkSpeed;
        float m_swimSpeed;
        float m_backSwimSpeed;
        float m_turnRate;

        
        bool mSemaphoreTeleport;

        
        float m_minZ;

        
        union
        {
            uint32 *m_uint32Values;
            float *m_floatValues;
        };

        
        uint16 m_valuesCount;

        
        UpdateMask m_updateMask;

        
        bool m_inWorld;

        
        bool m_objectUpdated;
};
#endif
