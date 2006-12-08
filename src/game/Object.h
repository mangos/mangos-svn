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

#ifndef _OBJECT_H
#define _OBJECT_H

#include "Common.h"
#include "ByteBuffer.h"
#include "UpdateMask.h"
#include "World.h"
#include "QuestDef.h"

#include <set>

#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

#define SPEED_STOP      0.0f
#define SPEED_WALK      2.5f
#define SPEED_RUN       7.0f
#define SPEED_WALKBACK  1.25f
#define SPEED_SWIM      4.722222f
#define SPEED_SWIMBACK  4.5f
#define SPEED_TURN      3.141594f

#define OBJECT_CONTACT_DISTANCE 0.5
#define OBJECT_ITERACTION_DISTANCE 5

enum ObjMoveType
{
    MOVE_STOP       =0,
    MOVE_WALK       =1,
    MOVE_RUN        =2,
    MOVE_WALKBACK   =3,
    MOVE_SWIM       =4,
    MOVE_SWIMBACK   =5,
    MOVE_TURN       =6
};

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

class MANGOS_DLL_SPEC Object
{
    public:
        virtual ~Object ( );

        virtual void Update ( float time ) { }

        const bool& IsInWorld() const { return m_inWorld; }
        virtual void AddToWorld() { m_inWorld = true; }
        virtual void RemoveFromWorld() { m_inWorld = false; }

        const uint64& GetGUID() const { return GetUInt64Value(0); }
        const uint32& GetGUIDLow() const { return GetUInt32Value(0); }
        const uint32& GetGUIDHigh() const { return GetUInt32Value(1); }
        const ByteBuffer& GetPackGUID() const { return m_PackGUID; }
        uint32 GetEntry() const { return GetUInt32Value(OBJECT_FIELD_ENTRY); }

        const uint8& GetTypeId() const { return m_objectTypeId; }
        bool isType(uint8 mask) const
        {

            if (mask & m_objectType)
                return true;

            return false;
        }

        virtual void BuildCreateUpdateBlockForPlayer( UpdateData *data, Player *target ) const;
        void SendUpdateToPlayer(Player* player) const;

        void BuildValuesUpdateBlockForPlayer( UpdateData *data, Player *target ) const;
        void BuildOutOfRangeUpdateBlock( UpdateData *data ) const;
        void BuildMovementUpdateBlock( UpdateData * data, uint32 flags = 0 ) const;

        virtual void DestroyForPlayer( Player *target ) const;

        void BuildHeartBeatMsg( WorldPacket *data ) const;
        void BuildTeleportAckMsg( WorldPacket *data, float x, float y, float z, float ang) const;
        bool IsBeingTeleported() { return mSemaphoreTeleport; }
        void SetSemaphoreTeleport(bool semphsetting) { mSemaphoreTeleport = semphsetting; }

        void Relocate(float x, float y, float z, float orientation)
        {
            m_positionX = x;
            m_positionY = y;
            m_positionZ = z;
            m_orientation = orientation;
        }

        float GetPositionX( ) const { return m_positionX; }
        float GetPositionY( ) const { return m_positionY; }
        float GetPositionZ( ) const { return m_positionZ; }
        void GetPosition( float &x, float &y, float &z ) const
            { x = m_positionX; y = m_positionY; z = m_positionZ; }
        float GetOrientation( ) const { return m_orientation; }
        void GetClosePoint( const Object* victim, float &x, float &y, float &z ) const;
        void GetClosePoint( const float ox, const float oy, const float oz, float &x, float &y, float &z ) const;
        const float GetObjectSize() const
        {
            return ( m_valuesCount > UNIT_FIELD_BOUNDINGRADIUS ) ? m_floatValues[UNIT_FIELD_BOUNDINGRADIUS] : 0.39f;
        }
        bool IsPositionValid() const;

        void SetMapId(uint32 newMap) { m_mapId = newMap; }

        uint32 GetMapId() const { return m_mapId; }

        uint32 GetZoneId() const;
        uint32 GetAreaId() const;

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

        void SetUInt32Value( uint16 index,       uint32  value );
        void SetUInt64Value( uint16 index, const uint64 &value );
        void SetFloatValue(  uint16 index,       float   value );

        void ApplyModUInt32Value(uint16 index, int32 val, bool apply);
        void ApplyModUInt64Value(uint16 index, int32 val, bool apply);
        void ApplyModFloatValue( uint16 index, float val, bool apply);

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

        void ClearUpdateMask( )
        {
            m_updateMask.Clear();
            m_objectUpdated = false;
        }

        float GetDistanceSq( const Object* obj ) const;
        float GetDistance2dSq( const Object* obj ) const;
        float GetDistanceSq(const float x, const float y, const float z) const;
        float GetDistanceZ(const Object* obj) const;
        bool IsWithinDistInMap(const Object* obj, const float dist2compare) const;
        bool IsWithinDist(const Object* obj, const float dist2compare) const;
        float GetAngle( const Object* obj ) const;
        float GetAngle( const float x, const float y ) const;
        bool HasInArc( const float arcangle, const Object* obj ) const;
        void GetContactPoint( const Object* obj, float &x, float &y, float &z ) const;

        virtual void SendMessageToSet(WorldPacket *data, bool self);

        bool LoadValues(const char* data);

        uint16 GetValuesCount() const { return m_valuesCount; }

        void InitValues() { _InitValues(); }

        float GetSpeed() const { return m_speed; }
        float GetSpeed( ObjMoveType mtype ) const
        {
            float DefaultSpeed;                             //[7] = { 0.0f, 2.5f, 7.0f, 2.5f, 4.722222f, 4.5f, 3.141594f };
            switch(mtype)
            {
                case MOVE_WALK:
                    DefaultSpeed=SPEED_WALK;
                    break;
                case MOVE_RUN:
                    DefaultSpeed=SPEED_RUN;
                    break;
                case MOVE_WALKBACK:
                    DefaultSpeed=SPEED_WALKBACK;
                    break;
                case MOVE_SWIM:
                    DefaultSpeed=SPEED_SWIM;
                    break;
                case MOVE_SWIMBACK:
                    DefaultSpeed=SPEED_SWIMBACK;
                    break;
                case MOVE_TURN:
                    DefaultSpeed=SPEED_TURN;
                    break;
                default:
                    DefaultSpeed=SPEED_STOP;
                    break;
            }
            return m_speed * DefaultSpeed;
        }
        void SetSpeed(float newspd) { m_speed = newspd; }

        void addQuest(uint32 questid) { mQuests.push_back(questid); }
        void addInvolvedQuest(uint32 questid) { mInvolvedQuests.push_back(questid); }
        bool hasQuest(uint32 quest_id);
        bool hasInvolvedQuest(uint32 quest_id);

        std::list<uint32> mQuests;
        std::list<uint32> mInvolvedQuests;

    protected:

        Object ( );

        void _InitValues()
        {
            m_uint32Values = new uint32[ m_valuesCount ];
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
        void _SetPackGUID(ByteBuffer *buffer, const uint64 &guid64) const;

        uint16 m_objectType;

        uint8 m_objectTypeId;

        uint32 m_mapId;

        float m_positionX;
        float m_positionY;
        float m_positionZ;
        float m_orientation;

        float m_speed;
        uint8 m_moveType;

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

    private:
        ByteBuffer m_PackGUID;

        // for output helpfull error messages from asserts
        bool PrintIndexError(uint32 index, bool set) const;
        Object(const Object&);                              // prevent generation copy constructor
        Object& operator=(Object const&);                   // prevent generation assigment operator
};
#endif
