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

#include "Common.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "Object.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "Util.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Log.h"
#include "Transports.h"

using namespace std;

Object::Object( )
{
    m_objectTypeId      = TYPEID_OBJECT;
    m_objectType        = TYPE_OBJECT;

    m_uint32Values      = 0;
    m_valuesCount       = 0;

    m_inWorld           = false;
    m_objectUpdated     = false;
}

Object::~Object( )
{

    if(m_objectUpdated)
        ObjectAccessor::Instance().RemoveUpdateObject(this);

    if(m_uint32Values)
    {
        //DEBUG_LOG("Object desctr 1 check (%p)",(void*)this);
        delete [] m_uint32Values;
        //DEBUG_LOG("Object desctr 2 check (%p)",(void*)this);
    }
}

void Object::_Create( uint32 guidlow, uint32 guidhigh )
{
    if(!m_uint32Values) _InitValues();

    SetUInt32Value( OBJECT_FIELD_GUID, guidlow );
    SetUInt32Value( OBJECT_FIELD_GUID+1, guidhigh );
    SetUInt32Value( OBJECT_FIELD_TYPE, m_objectType );
    m_PackGUID.clear();
    _SetPackGUID(&m_PackGUID,GetGUID());
}

void Object::BuildMovementUpdateBlock(UpdateData * data, uint32 flags ) const
{
    ByteBuffer buf(500);

    buf << uint8( UPDATETYPE_MOVEMENT );
    buf << GetGUID();

    _BuildMovementUpdate(&buf, flags, 0x00000000);

    data->AddUpdateBlock(buf);
}

void Object::BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    if(!target) return;

    ByteBuffer buf(500);
    buf << uint8( UPDATETYPE_CREATE_OBJECT );
    buf << uint8( 0xFF );
    buf << GetGUID() ;
    buf << m_objectTypeId;

    switch(m_objectTypeId)
    {
        case TYPEID_OBJECT:                                 //do nothing
            break;
        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
            _BuildMovementUpdate( &buf, 0x10, 0x0 );
            break;
        case TYPEID_UNIT:
            _BuildMovementUpdate( &buf, 0x70, 0x800000 );
            break;
        case TYPEID_PLAYER:
        {
            if( target == this )                            //build for self
            {
                buf.clear();
                buf << uint8( UPDATETYPE_CREATE_OBJECT2 );
                buf << uint8( 0xFF );                       // must be packet GUID ?
                buf << GetGUID() ;
                buf << m_objectTypeId;
                _BuildMovementUpdate( &buf, 0x71, 0x2000 );
            }
            //build for other player
            else
            {
                _BuildMovementUpdate( &buf, 0x70, 0x0 );
            }
        }break;
        case TYPEID_CORPSE:
        case TYPEID_GAMEOBJECT:
        case TYPEID_DYNAMICOBJECT:
        {
            if ((GUID_HIPART(GetGUID())==HIGHGUID_PLAYER_CORPSE) || (GUID_HIPART(GetGUID()) == HIGHGUID_TRANSPORT))
                _BuildMovementUpdate( &buf, 0x52, 0x0 );
            else
                _BuildMovementUpdate( &buf, 0x50, 0x0 );
        }break;
        //case TYPEID_AIGROUP:
        //case TYPEID_AREATRIGGER:
        //break;
        default:                                            //know type
            sLog.outDetail("Unknown Object Type %u Create Update Block.\n", m_objectTypeId);
            break;
    }

    UpdateMask updateMask;
    updateMask.SetCount( m_valuesCount );
    _SetCreateBits( &updateMask, target );
    _BuildValuesUpdate( &buf, &updateMask );
    data->AddUpdateBlock(buf);

}

void Object::SendUpdateToPlayer(Player* player) const
{
    //if (!player->IsInWorld()) return;

    UpdateData upd;
    WorldPacket packet;

    upd.Clear();
    BuildCreateUpdateBlockForPlayer(&upd, player);
    upd.BuildPacket(&packet);
    player->GetSession()->SendPacket(&packet);
}

void Object::BuildValuesUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    ByteBuffer buf(500);

    buf << (uint8) UPDATETYPE_VALUES;
    buf << (uint8) 0xFF;                                    // must be packed GUID  ?
    buf << GetGUID();

    UpdateMask updateMask;
    updateMask.SetCount( m_valuesCount );
    _SetUpdateBits( &updateMask, target );
    _BuildValuesUpdate( &buf, &updateMask );

    data->AddUpdateBlock(buf);
}

void Object::BuildOutOfRangeUpdateBlock(UpdateData * data) const
{
    data->AddOutOfRangeGUID(GetGUID());
}

void Object::DestroyForPlayer(Player *target) const
{
    ASSERT(target);

    WorldPacket data(SMSG_DESTROY_OBJECT, 8);
    data << GetGUID();

    target->GetSession()->SendPacket( &data );
}

void Object::_BuildMovementUpdate(ByteBuffer * data, uint8 flags, uint32 flags2 ) const
{
    *data << (uint8)flags;
    if( m_objectTypeId==TYPEID_PLAYER )
    {
        if(((Player*)this)->GetTransport())
        {
            flags2 |= 0x02000000;
        }
        *data << (uint32)flags2;

        *data << (uint32)getMSTime();

        if (!((Player *)this)->GetTransport())
        {
            *data << ((Player *)this)->GetPositionX();
            *data << ((Player *)this)->GetPositionY();
            *data << ((Player *)this)->GetPositionZ();
            *data << ((Player *)this)->GetOrientation();
        }
        else
        {
            //*data << ((Player *)this)->m_transport->GetPositionX() + (float)((Player *)this)->m_transX;
            //*data << ((Player *)this)->m_transport->GetPositionY() + (float)((Player *)this)->m_transY;
            //*data << ((Player *)this)->m_transport->GetPositionZ() + (float)((Player *)this)->m_transZ;

            *data << ((Player *)this)->GetTransport()->GetPositionX();
            *data << ((Player *)this)->GetTransport()->GetPositionY();
            *data << ((Player *)this)->GetTransport()->GetPositionZ();
            *data << ((Player *)this)->GetTransport()->GetOrientation();

            *data << (uint64)(((Player *)this)->GetTransport()->GetGUID());
            *data << ((Player *)this)->GetTransOffsetX();
            *data << ((Player *)this)->GetTransOffsetY();
            *data << ((Player *)this)->GetTransOffsetZ();
            *data << ((Player *)this)->GetTransOffsetO();
        }

        *data << (float)0;

        if(flags2 & 0x2000)                                 //update self
        {
            *data << (float)0;
            *data << (float)1.0;
            *data << (float)0;
            *data << (float)0;
        }
        *data << ((Player*)this)->GetSpeed( MOVE_WALK );
        *data << ((Player*)this)->GetSpeed( MOVE_RUN );
        *data << ((Player*)this)->GetSpeed( MOVE_SWIMBACK );
        *data << ((Player*)this)->GetSpeed( MOVE_SWIM );
        *data << ((Player*)this)->GetSpeed( MOVE_WALKBACK );
        *data << ((Player*)this)->GetSpeed( MOVE_TURN );
    }
    if( m_objectTypeId==TYPEID_UNIT )
    {
        *data << (uint32)flags2;
        *data << (uint32)0xB5771D7F;
        *data << ((Unit *)this)->GetPositionX();
        *data << ((Unit *)this)->GetPositionY();
        *data << ((Unit *)this)->GetPositionZ();
        *data << ((Unit *)this)->GetOrientation();
        *data << (float)0;
        *data << ((Creature*)this)->GetSpeed( MOVE_WALK );
        *data << ((Creature*)this)->GetSpeed( MOVE_RUN );
        *data << ((Creature*)this)->GetSpeed( MOVE_SWIMBACK );
        *data << ((Creature*)this)->GetSpeed( MOVE_SWIM );
        *data << ((Creature*)this)->GetSpeed( MOVE_WALKBACK );
        *data << ((Creature*)this)->GetSpeed( MOVE_TURN );
        uint8 PosCount=0;
        if(flags2 & 0x400000)
        {
            *data << (uint32)0x0;
            *data << (uint32)0x659;
            *data << (uint32)0xB7B;
            *data << (uint32)0xFDA0B4;
            *data << (uint32)PosCount;
            for(int i=0;i<PosCount+1;i++)
            {
                *data << (float)0;                          //x
                *data << (float)0;                          //y
                *data << (float)0;                          //z
            }
        }
    }
    if( (m_objectTypeId==TYPEID_CORPSE) || (m_objectTypeId==TYPEID_GAMEOBJECT) || (m_objectTypeId==TYPEID_DYNAMICOBJECT))
    {
        if(GUID_HIPART(GetGUID()) != HIGHGUID_TRANSPORT)
        {
            *data << ((WorldObject *)this)->GetPositionX();
            *data << ((WorldObject *)this)->GetPositionY();
            *data << ((WorldObject *)this)->GetPositionZ();
        }
        else
        {
            *data << (uint32)0;
            *data << (uint32)0;
            *data << (uint32)0;
        }
        *data << ((WorldObject *)this)->GetOrientation();
    }

    *data << (uint32)0x1;

    if ((GUID_HIPART(GetGUID()) == HIGHGUID_TRANSPORT))
    {
        uint32 updT = (uint32)getMSTime();
        *data << (uint32)updT;
    }

    if(  GUID_HIPART(GetGUID()) == HIGHGUID_PLAYER_CORPSE)
        *data << (uint32)0xBD38BA14;                        //fix me
}

void Object::_BuildValuesUpdate(ByteBuffer * data, UpdateMask *updateMask) const
{
    WPAssert(updateMask && updateMask->GetCount() == m_valuesCount);

    *data << (uint8)updateMask->GetBlockCount();
    data->append( updateMask->GetMask(), updateMask->GetLength() );

    for( uint16 index = 0; index < m_valuesCount; index ++ )
    {
        if( updateMask->GetBit( index ) )
        {
            // Some values at server stored in float format but must be sended to client in uint32 format
            if( isType(TYPE_UNIT) && (
                index >= UNIT_FIELD_POWER1         && index <= UNIT_FIELD_MAXPOWER5 ||
                index >= UNIT_FIELD_BASEATTACKTIME && index <= UNIT_FIELD_RANGEDATTACKTIME ||
                index >= UNIT_FIELD_STR            && index <= UNIT_FIELD_RESISTANCES + 6 )
                || isType(TYPE_PLAYER) &&
                index >= PLAYER_FIELD_POSSTAT0 && index <= PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE + 6 )
            {
                // convert from float to uint32 and send
                *data << uint32(m_floatValues[ index ]);
            }
            else
            {
                // send in current format (float as float, uint32 as uint32)
                *data << m_uint32Values[ index ];
            }
        }
    }
}

bool Object::LoadValues(const char* data)
{
    if(!m_uint32Values) _InitValues();

    vector<string> tokens = StrSplit(data, " ");

    if(tokens.size() != m_valuesCount)
        return false;

    vector<string>::iterator iter;
    int index;

    for (iter = tokens.begin(), index = 0; index < m_valuesCount; ++iter, ++index)
    {
        m_uint32Values[index] = atol((*iter).c_str());
    }
    return true;
}

void Object::_SetUpdateBits(UpdateMask *updateMask, Player *target) const
{
    *updateMask = m_updateMask;
}

void Object::_SetCreateBits(UpdateMask *updateMask, Player *target) const
{
    for( uint16 index = 0; index < m_valuesCount; index++ )
    {
        if(GetUInt32Value(index) != 0)
            updateMask->SetBit(index);
    }
}

void Object::SetUInt32Value( uint16 index, uint32 value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );
    if(m_uint32Values[ index ] != value)
    {
        m_uint32Values[ index ] = value;

        if(m_inWorld)
        {
            m_updateMask.SetBit( index );

            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetUInt64Value( uint16 index, const uint64 &value )
{
    ASSERT( index + 1 < m_valuesCount || PrintIndexError( index , true ) );
    if(*((uint64*)&(m_uint32Values[ index ])) != value)
    {
        m_uint32Values[ index ] = *((uint32*)&value);
        m_uint32Values[ index + 1 ] = *(((uint32*)&value) + 1);

        if(m_inWorld)
        {
            m_updateMask.SetBit( index );
            m_updateMask.SetBit( index + 1 );

            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetFloatValue( uint16 index, float value )
{
    ASSERT( index + 1 < m_valuesCount || PrintIndexError( index , true ) );
    if(m_floatValues[ index ] != value)
    {
        m_floatValues[ index ] = value;

        if(m_inWorld)
        {
            m_updateMask.SetBit( index );

            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::ApplyModUInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetUInt32Value(index);
    cur += (apply ? val : -val);
    if(cur < 0)
        cur = 0;
    SetUInt32Value(index,cur);
}

void Object::ApplyModFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    if(cur < 0)
        cur = 0;
    SetFloatValue(index,cur);
}

void Object::SetFlag( uint16 index, uint32 newFlag )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );
    uint32 oldval = m_uint32Values[ index ];
    uint32 newval = oldval | newFlag;

    if(oldval != newval)
    {
        m_uint32Values[ index ] = newval;

        if(m_inWorld)
        {
            m_updateMask.SetBit( index );

            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::RemoveFlag( uint16 index, uint32 oldFlag )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );
    uint32 oldval = m_uint32Values[ index ];
    uint32 newval = oldval & ~oldFlag;

    if(oldval != newval)
    {
        m_uint32Values[ index ] = newval;

        if(m_inWorld)
        {
            m_updateMask.SetBit( index );

            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

bool Object::hasQuest(uint32 quest_id)
{
    return (find(mQuests.begin(), mQuests.end(), quest_id) != mQuests.end());
}

bool Object::hasInvolvedQuest(uint32 quest_id)
{
    return (find(mInvolvedQuests.begin(), mInvolvedQuests.end(), quest_id) != mInvolvedQuests.end());
}

bool Object::PrintIndexError(uint32 index, bool set) const
{
    sLog.outError("ERROR: Attempt %s non-existed value field: %u (count: %u) for object typeid: %u type mask: %u",(set ? "set value to" : "get value from"),index,m_valuesCount,GetTypeId(),m_objectType);

    // assert must fail after function call
    return false;
}

void Object::_SetPackGUID(ByteBuffer *buffer, const uint64 &guid64) const
{
    size_t mask_position = buffer->wpos();
    *buffer << uint8(0);
    for(uint8 i = 0; i < 8; i++)
    {
        if(((uint8*)&guid64)[i])
        {
            const_cast<uint8*>(buffer->contents())[mask_position] |= (1<<i);
            *buffer << ((uint8*)&guid64)[i];
        }
    }
}

WorldObject::WorldObject( )
{
    m_positionX         = 0.0f;
    m_positionY         = 0.0f;
    m_positionZ         = 0.0f;
    m_orientation       = 0.0f;

    m_mapId             = 0;

    mSemaphoreTeleport  = false;
}

void WorldObject::_Create( uint32 guidlow, uint32 guidhigh, uint32 mapid, float x, float y, float z, float ang, uint32 nameId )
{
    Object::_Create(guidlow, guidhigh);

    SetUInt32Value( OBJECT_FIELD_ENTRY,nameId);

    m_mapId = mapid;
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
    m_orientation = ang;
}

uint32 WorldObject::GetZoneId() const
{
    return MapManager::Instance().GetMap(m_mapId)->GetZoneId(m_positionX,m_positionY);
}

uint32 WorldObject::GetAreaId() const
{
    return MapManager::Instance().GetMap(m_mapId)->GetAreaId(m_positionX,m_positionY);
}

float WorldObject::GetDistanceSq(const WorldObject* obj) const        //slow
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return ( dist > 0 ? dist * dist : 0);
}

float WorldObject::GetDistanceSq(const float x, const float y, const float z) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float sizefactor = GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return ( dist > 0 ? dist * dist : 0);
}

float WorldObject::GetDistance2dSq(const WorldObject* obj) const      //slow
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return ( dist > 0 ? dist * dist : 0);
}

float WorldObject::GetDistanceZ(const WorldObject* obj) const
{
    float dz = fabs(GetPositionZ() - obj->GetPositionZ());
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = dz - sizefactor;
    return ( dist > 0 ? dist : 0);
}

bool WorldObject::IsWithinDistInMap(const WorldObject* obj, const float dist2compare) const
{
    if (GetMapId()!=obj->GetMapId()) return false;
    return IsWithinDist(obj, dist2compare);
}

bool WorldObject::IsWithinDist(const WorldObject* obj, const float dist2compare) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float distsq = dx*dx + dy*dy + dz*dz;
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float maxdist = dist2compare + sizefactor;
    return distsq < maxdist * maxdist;
}

float WorldObject::GetAngle(const WorldObject* obj) const
{
    if(!obj) return 0;
    return GetAngle( obj->GetPositionX(), obj->GetPositionY() );
}

// Retirn angle in range 0..2*pi
float WorldObject::GetAngle( const float x, const float y ) const
{
    float dx = x - GetPositionX();
    float dy = y - GetPositionY();

    float ang = atan2(dy, dx);
    ang = (ang >= 0) ? ang : 2 * M_PI + ang;
    return ang;
}

bool WorldObject::HasInArc(const float arcangle, const WorldObject* obj) const
{
    float arc = arcangle;

    // move arc to range 0.. 2*pi
    while( arc >= 2.0f * M_PI )
        arc -=  2.0f * M_PI;
    while( arc < 0 )
        arc +=  2.0f * M_PI;

    float angle = GetAngle( obj );
    angle -= m_orientation;

    // move angle to range -pi ... +pi
    while( angle > M_PI)
        angle -= 2.0f * M_PI;
    while(angle < -M_PI)
        angle += 2.0f * M_PI;

    float lborder =  -1 * (arc/2.0f);                       // in range -pi..0
    float rborder = (arc/2.0f);                             // in range 0..pi
    return (( angle >= lborder ) && ( angle <= rborder ));
}

void WorldObject::GetContactPoint( const WorldObject* obj, float &x, float &y, float &z ) const
{
    float angle = GetAngle( obj );
    x = GetPositionX() + (GetObjectSize() + obj->GetObjectSize() + OBJECT_CONTACT_DISTANCE) * cos(angle);
    y = GetPositionY() + (GetObjectSize() + obj->GetObjectSize() + OBJECT_CONTACT_DISTANCE) * sin(angle);
    z = GetPositionZ();
}

void WorldObject::GetClosePoint( const WorldObject* victim, float &x, float &y, float &z ) const
{
    if( victim )
        GetClosePoint( victim->GetPositionX(), victim->GetPositionY(), victim->GetPositionZ(), x, y, z);
    else
        GetClosePoint( 0, 0, 0, x, y, z);
}

void WorldObject::GetClosePoint( const float ox, const float oy, const float oz, float &x, float &y, float &z ) const
{
    float angle;
    if( ox == 0 && oy == 0 )
        angle = GetOrientation();
    else
        angle = GetAngle( ox, oy );
    x = GetPositionX() + GetObjectSize() * cos(angle);
    y = GetPositionY() + GetObjectSize() * sin(angle);
    z = GetPositionZ();

}

bool WorldObject::IsPositionValid() const
{
    return MaNGOS::IsValidMapCoord(m_positionX) && MaNGOS::IsValidMapCoord(m_positionY);
}

void WorldObject::BuildHeartBeatMsg(WorldPacket *data) const
{
    data->Initialize(MSG_MOVE_HEARTBEAT, 32);               //2

    *data << GetGUID();                                     //8
    *data << uint32(0);                                     //4
    *data << uint32(0);                                     //4

    *data << m_positionX;                                   //4
    *data << m_positionY;                                   //4
    *data << m_positionZ;                                   //4

    *data << m_orientation;                                 //4
}

void WorldObject::BuildTeleportAckMsg(WorldPacket *data, float x, float y, float z, float ang) const
{
    data->Initialize(MSG_MOVE_TELEPORT_ACK, 41);
    *data << uint8(0xFF);
    *data << GetGUID();
    *data << uint32(0x800000);
    *data << uint16(0x67EE);
    *data << uint16(0xD1EB);
    *data << m_orientation;                                 // instead of *data << z;
    *data << x;
    *data << y;
    *data << z;                                             // instead of *data << ang;
    *data << ang;
    *data << uint32(0x0);
}

void WorldObject::SendMessageToSet(WorldPacket *data, bool bToSelf)
{
    MapManager::Instance().GetMap(m_mapId)->MessageBoardcast(this, data);
}

void WorldObject::SendDestroyObject(uint64 guid)
{
    WorldPacket data(SMSG_DESTROY_OBJECT, 8);
    data << guid;
    SendMessageToSet(&data, true);
}

void WorldObject::SendObjectDeSpawnAnim(uint64 guid)
{
    WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
    data << guid;
    SendMessageToSet(&data, true);
}

