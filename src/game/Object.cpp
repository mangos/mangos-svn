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

#include "Common.h"
#include "SharedDefines.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "Object.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "UpdateMask.h"
#include "Util.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Log.h"
#include "Transports.h"
#include "VMapFactory.h"
#include "FlightMaster.h"

uint32 GuidHigh2TypeId(uint32 guid_hi)
{
    switch(guid_hi)
    {
        case HIGHGUID_ITEM:         return TYPEID_ITEM;
        case HIGHGUID_UNIT:         return TYPEID_UNIT;
        case HIGHGUID_PLAYER:       return TYPEID_PLAYER;
        case HIGHGUID_GAMEOBJECT:   return TYPEID_GAMEOBJECT;
        case HIGHGUID_DYNAMICOBJECT:return TYPEID_DYNAMICOBJECT;
        case HIGHGUID_CORPSE:       return TYPEID_CORPSE;
        case HIGHGUID_PLAYER_CORPSE:return 10;              // unknown
        case HIGHGUID_TRANSPORT:    return TYPEID_GAMEOBJECT;
    }
    return 10;                                              // unknown
}

Object::Object( )
{
    m_objectTypeId      = TYPEID_OBJECT;
    m_objectType        = TYPE_OBJECT;

    m_uint32Values      = 0;
    m_uint32Values_mirror = 0;
    m_valuesCount       = 0;

    m_inWorld           = false;
    m_objectUpdated     = false;

    m_PackGUID.clear();
    m_PackGUID.appendPackGUID(0);
}

Object::~Object( )
{
    if(m_objectUpdated)
        ObjectAccessor::Instance().RemoveUpdateObject(this);

    if(m_uint32Values)
    {
        if(IsInWorld())
        {
            ///- Do NOT call RemoveFromWorld here, if the object is a player it will crash
            sLog.outError("Object::~Object - guid="I64FMTD", typeid=%d deleted but still in world!!", GetGUID(), GetTypeId());
            //assert(0);
        }

        //DEBUG_LOG("Object desctr 1 check (%p)",(void*)this);
        delete [] m_uint32Values;
        delete [] m_uint32Values_mirror;
        //DEBUG_LOG("Object desctr 2 check (%p)",(void*)this);
    }
}

void Object::_InitValues()
{
    m_uint32Values = new uint32[ m_valuesCount ];
    memset(m_uint32Values, 0, m_valuesCount*sizeof(uint32));

    m_uint32Values_mirror = new uint32[ m_valuesCount ];
    memset(m_uint32Values_mirror, 0, m_valuesCount*sizeof(uint32));

    m_objectUpdated = false;
}

void Object::_Create( uint32 guidlow, uint32 guidhigh )
{
    if(!m_uint32Values) _InitValues();

    SetUInt32Value( OBJECT_FIELD_GUID, guidlow );
    SetUInt32Value( OBJECT_FIELD_GUID+1, guidhigh );
    SetUInt32Value( OBJECT_FIELD_TYPE, m_objectType );
    m_PackGUID.clear();
    m_PackGUID.appendPackGUID(GetGUID());
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
    if(!target)
    {
        return;
    }

    uint8  updatetype = UPDATETYPE_CREATE_OBJECT;
    uint8  flags      = m_updateFlag;
    uint32 flags2     = 0;

    /** lower flag1 **/
    if(target == this) // building packet for oneself
    {
        flags |= UPDATEFLAG_SELF;

        /*** temporary reverted - until real source of stack corruption will not found
        updatetype = UPDATETYPE_CREATE_OBJECT2;
        ****/
    }

    if(flags & UPDATEFLAG_HASPOSITION)
    {
        // UPDATETYPE_CREATE_OBJECT2 dynamic objects, corpses...
        if(isType(TYPE_DYNAMICOBJECT) || isType(TYPE_CORPSE) || isType(TYPE_PLAYER))
        /*** temporary reverted - until real source of stack corruption will not found
        if(isType(TYPE_DYNAMICOBJECT) || isType(TYPE_CORPSE))
        ***/
            updatetype = UPDATETYPE_CREATE_OBJECT2;

        // UPDATETYPE_CREATE_OBJECT2 for pets...
        if(target->GetPetGUID() == GetGUID())
            updatetype = UPDATETYPE_CREATE_OBJECT2;

        // UPDATETYPE_CREATE_OBJECT2 for some gameobject types...
        if(isType(TYPE_GAMEOBJECT))
        {
            switch(m_uint32Values[GAMEOBJECT_TYPE_ID])
            {
                case GAMEOBJECT_TYPE_TRAP:
                case GAMEOBJECT_TYPE_DUEL_ARBITER:
                case GAMEOBJECT_TYPE_FLAGSTAND:
                case GAMEOBJECT_TYPE_FLAGDROP:
                    updatetype = UPDATETYPE_CREATE_OBJECT2;
                    break;
            }
        }
    }

    // flags2 only used at LIVING objects
    /*if(flags & UPDATEFLAG_LIVING)
    {
        if(m_objectTypeId == TYPEID_PLAYER && ((Player*)this)->GetTransport() != 0)
            flags2 |= MOVEMENTFLAG_ONTRANSPORT;

        if(m_objectTypeId == TYPEID_PLAYER)
        {
            updatetype = UPDATETYPE_CREATE_OBJECT2; // dunno when exactly this's used
            //flags2 |= 0x00002000;
        }
    }*/

    //sLog.outDebug("BuildCreateUpdate: update-type: %u, object-type: %u got flags: %X, flags2: %X", updatetype, m_objectTypeId, flags, flags2);

    ByteBuffer buf(500);
    buf << (uint8)updatetype;
    //buf.append(GetPackGUID());    //client crashes when using this
    buf << (uint8)0xFF << GetGUID();
    buf << (uint8)m_objectTypeId;

    _BuildMovementUpdate(&buf, flags, flags2);

    UpdateMask updateMask;
    updateMask.SetCount( m_valuesCount );
    _SetCreateBits( &updateMask, target );
    _BuildValuesUpdate( &buf, &updateMask, target );
    data->AddUpdateBlock(buf);
}

void Object::BuildUpdate(UpdateDataMapType &update_players)
{
    ObjectAccessor::_buildUpdateObject(this,update_players);
    ClearUpdateMask(true);
}

void Object::SendUpdateToPlayer(Player* player)
{
    // send update to another players
    SendUpdateObjectToAllExcept(player);

    // send create update to player
    UpdateData upd;
    WorldPacket packet;

    upd.Clear();
    BuildCreateUpdateBlockForPlayer(&upd, player);
    upd.BuildPacket(&packet);
    player->GetSession()->SendPacket(&packet);

    // now object updated/(create updated)
}

void Object::BuildValuesUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    ByteBuffer buf(500);

    buf << (uint8) UPDATETYPE_VALUES;
    //buf.append(GetPackGUID());    //client crashes when using this. but not have crash in debug mode
    buf << (uint8)0xFF;
    buf << GetGUID();

    UpdateMask updateMask;
    updateMask.SetCount( m_valuesCount );

    _SetUpdateBits( &updateMask, target );
    _BuildValuesUpdate( &buf, &updateMask, target );

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

    if (flags & UPDATEFLAG_LIVING)          // 0x20
    {
        switch(GetTypeId())
        {
            case TYPEID_UNIT:
                {
                    switch(GetEntry())
                    {
                        case 6491:          // Spirit Healer
                        case 13116:         // Alliance Spirit Guide
                        case 13117:         // Horde Spirit Guide
                            flags2 |= MOVEMENTFLAG_WATERWALKING;   // waterwalking movement flag?
                            break;
                    }
                }
                break;
            case TYPEID_PLAYER:
                {
                    flags2 = ((Player*)this)->GetMovementFlags();

                    if(((Player*)this)->GetTransport())
                        flags2 |= MOVEMENTFLAG_ONTRANSPORT;
                    else
                        flags2 &= ~MOVEMENTFLAG_ONTRANSPORT;

                    // remove unknown, unused etc flags for now
                    flags2 &= ~MOVEMENTFLAG_SPLINE;
                    flags2 &= ~MOVEMENTFLAG_SPLINE2;
                    flags2 &= ~MOVEMENTFLAG_JUMPING;
                    flags2 &= ~MOVEMENTFLAG_FALLING;
                    flags2 &= ~MOVEMENTFLAG_SWIMMING;

                    if(((Player*)this)->isInFlight())
                        if(FlightMaster::Instance().GetFlightPathMovementGenerator((Player*)this))
                            flags2 = (MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_SPLINE2);
                }
                break;
        }

        *data << flags2;                    // movement flags
        *data << getMSTime();               // this appears to be time in ms but can be any thing (mask, flags)
    }

    if (flags & UPDATEFLAG_HASPOSITION)     // 0x40
    {
        if(flags & UPDATEFLAG_TRANSPORT)    // 0x2
        {
            *data << (float)0;
            *data << (float)0;
            *data << (float)0;
            *data << ((WorldObject *)this)->GetOrientation();
        }
        else
        {
            *data << ((WorldObject *)this)->GetPositionX();
            *data << ((WorldObject *)this)->GetPositionY();
            *data << ((WorldObject *)this)->GetPositionZ();
            *data << ((WorldObject *)this)->GetOrientation();
        }
    }

    if (flags & UPDATEFLAG_LIVING)          // 0x20
    {
        if(flags2 & MOVEMENTFLAG_ONTRANSPORT)   // 0x200
        {
            *data << (uint64)((Player*)this)->GetTransport()->GetGUID();
            *data << (float)((Player*)this)->GetTransOffsetX();
            *data << (float)((Player*)this)->GetTransOffsetY();
            *data << (float)((Player*)this)->GetTransOffsetZ();
            *data << (float)((Player*)this)->GetTransOffsetO();
            *data << (uint32)((Player*)this)->GetTransTime();
        }

        /*if(flags2 & MOVEMENTFLAG_SWIMMING)  // 0x200000
        {
            // is't part of movement packet, we must store and send it...
            *data << (float)0;              // we can get this value from movement packets?
        }*/

        // fall time according movement packet structure...
        *data << (uint32)0;                 // unknown

        // 0x2000 or 0x4000
        /*if((flags2 & MOVEMENTFLAG_JUMPING) || (flags2 & MOVEMENTFLAG_FALLING))
        {
            // is't part of movement packet, we must store and send it...
            *data << (float)0;
            *data << (float)0;
            *data << (float)0;
            *data << (float)0;
        }*/

        /*if(flags2 & MOVEMENTFLAG_SPLINE)    // 0x4000000
        {
            *data << uint32(0);
        }*/

        *data << ((Unit*)this)->GetSpeed( MOVE_WALK );
        *data << ((Unit*)this)->GetSpeed( MOVE_RUN );
        *data << ((Unit*)this)->GetSpeed( MOVE_SWIMBACK );
        *data << ((Unit*)this)->GetSpeed( MOVE_SWIM );
        *data << ((Unit*)this)->GetSpeed( MOVE_WALKBACK );
        *data << ((Unit*)this)->GetSpeed( MOVE_FLY );
        *data << ((Unit*)this)->GetSpeed( MOVE_FLYBACK );
        *data << ((Unit*)this)->GetSpeed( MOVE_TURN );

        if(flags2 & MOVEMENTFLAG_SPLINE2)   // 0x8000000
        {
            FlightPathMovementGenerator *fmg = FlightMaster::Instance().GetFlightPathMovementGenerator((Player*)this);
            if (!fmg)
            {
                // how we can get there?
                sLog.outError("Bad thing happens :(");
                return;
            }

            uint32 flags3 = 0x00000300;

            *data << flags3;                // splines flag?

            if(flags3 & 0x10000)            // probably x,y,z coords there
            {
                *data << (float)0;
                *data << (float)0;
                *data << (float)0;
            }

            if(flags3 & 0x20000)            // probably guid there
            {
                *data << uint64(0);
            }

            if(flags3 & 0x40000)            // may be orientation
            {
                *data << (float)0;
            }

            Path &path = fmg->GetPath();

            float x, y, z;
            ((Player*)this)->GetPosition(x, y, z);

            uint32 inflighttime = uint32(path.GetPassedLength(fmg->GetCurrentNode(), x, y, z) * 32);
            uint32 traveltime = uint32(path.GetTotalLength() * 32);

            *data << uint32(inflighttime);  // passed move time?
            *data << uint32(traveltime);    // full move time?
            *data << uint32(0);             // ticks count?

            uint32 poscount = uint32(path.Size());

            *data << uint32(poscount);      // points count

            for(uint32 i = 0; i < poscount; ++i)
            {
                *data << path.GetNodes()[i].x;
                *data << path.GetNodes()[i].y;
                *data << path.GetNodes()[i].z;
            }

            /*for(uint32 i = 0; i < poscount; i++)
            {
                // path points
                *data << (float)0;
                *data << (float)0;
                *data << (float)0;
            }*/

            *data << path.GetNodes()[poscount-1].x;
            *data << path.GetNodes()[poscount-1].y;
            *data << path.GetNodes()[poscount-1].z;

            // target position (path end)
            /**data << ((Unit*)this)->GetPositionX();
            *data << ((Unit*)this)->GetPositionY();
            *data << ((Unit*)this)->GetPositionZ();*/
        }
    }

    if(flags & UPDATEFLAG_ALL)              // 0x10
    {
        *data << uint32(0);                 // unk, probably timestamp
    }

    if(flags & UPDATEFLAG_HIGHGUID)         // 0x8
    {
        *data << uint32(0);                 // unk, probably timestamp
    }

    /*if(flags & UPDATEFLAG_FULLGUID)       // 0x4
    {
        packed guid (probably target guid)  // unk
    }*/

    if(flags & UPDATEFLAG_TRANSPORT)        // 0x2
    {
        *data << getMSTime();               // ms time
    }
}

void Object::_BuildValuesUpdate(ByteBuffer * data, UpdateMask *updateMask, Player *target) const
{
    WPAssert(updateMask && updateMask->GetCount() == m_valuesCount);

    *data << (uint8)updateMask->GetBlockCount();
    data->append( updateMask->GetMask(), updateMask->GetLength() );

    // 2 specialized loops for speed optimization in non-unit case
    if(isType(TYPE_UNIT))                                   // unit (creature/player) case
    {
        for( uint16 index = 0; index < m_valuesCount; index ++ )
        {
            if( updateMask->GetBit( index ) )
            {
                // remove custom flag before send
                if( index == UNIT_NPC_FLAGS )
                    *data << uint32(m_uint32Values[ index ] & ~UNIT_NPC_FLAG_GUARD);
                // FIXME: Some values at server stored in float format but must be sent to client in uint32 format
                else if(
                    index >= UNIT_FIELD_BASEATTACKTIME && index <= UNIT_FIELD_RANGEDATTACKTIME ||

                    index >= UNIT_FIELD_POSSTAT0   && index <= UNIT_FIELD_POSSTAT4 ||
                    index >= UNIT_FIELD_NEGSTAT0   && index <= UNIT_FIELD_NEGSTAT4 ||

                    index >= UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE  && index <= (UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + 6) ||
                    index >= UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE  && index <= (UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + 6) )
                {
                    // convert from float to uint32 and send
                    *data << uint32(m_floatValues[ index ] < 0 ? 0 : m_floatValues[ index ]);
                }
                else
                {
                    // send in current format (float as float, uint32 as uint32)
                    *data << m_uint32Values[ index ];
                }
            }
        }
    }
    else                                                    // other objects case (no special index checks)
    {
        for( uint16 index = 0; index < m_valuesCount; index ++ )
        {
            if( updateMask->GetBit( index ) )
            {
                // send in current format (float as float, uint32 as uint32)
                *data << m_uint32Values[ index ];
            }
        }
    }
}

void Object::ClearUpdateMask(bool remove)
{
    for( uint16 index = 0; index < m_valuesCount; index ++ )
    {
        if(m_uint32Values_mirror[index]!= m_uint32Values[index])
            m_uint32Values_mirror[index] = m_uint32Values[index];
    }
    if(m_objectUpdated)
    {
        if(remove)
            ObjectAccessor::Instance().RemoveUpdateObject(this);
        m_objectUpdated = false;
    }
}

// Send current value fields changes to all viewers
void Object::SendUpdateObjectToAllExcept(Player* exceptPlayer)
{
    // changes will be send in create packet
    if(!IsInWorld())
        return;

    // nothing do
    if(!m_objectUpdated)
        return;

    ObjectAccessor::UpdateObject(this,exceptPlayer);
}

bool Object::LoadValues(const char* data)
{
    if(!m_uint32Values) _InitValues();

    Tokens tokens = StrSplit(data, " ");

    if(tokens.size() != m_valuesCount)
        return false;

    Tokens::iterator iter;
    int index;
    for (iter = tokens.begin(), index = 0; index < m_valuesCount; ++iter, ++index)
    {
        m_uint32Values[index] = atol((*iter).c_str());
    }

    return true;
}

void Object::_SetUpdateBits(UpdateMask *updateMask, Player *target) const
{
    for( uint16 index = 0; index < m_valuesCount; index ++ )
    {
        if(m_uint32Values_mirror[index]!= m_uint32Values[index])
            updateMask->SetBit(index);
    }
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
            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetStatFloatValue( uint16 index, float value)
{
    if(value < 0)   
        value = 0.0f;

    SetFloatValue(index, value);
}

void Object::SetStatInt32Value( uint16 index, int32 value)
{
    if(value < 0)   
        value = 0;

    SetUInt32Value(index, uint32(value));
}

void Object::ApplyModUInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetUInt32Value(index);
    cur += (apply ? val : -val);
    if(cur < 0)
        cur = 0;
    SetUInt32Value(index,cur);
}

void Object::ApplyModInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetUInt32Value(index);
    cur += (apply ? val : -val);
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
            if(!m_objectUpdated)
            {
                ObjectAccessor::Instance().AddUpdateObject(this);
                m_objectUpdated = true;
            }
        }
    }
}

bool Object::PrintIndexError(uint32 index, bool set) const
{
    sLog.outError("ERROR: Attempt %s non-existed value field: %u (count: %u) for object typeid: %u type mask: %u",(set ? "set value to" : "get value from"),index,m_valuesCount,GetTypeId(),m_objectType);

    // assert must fail after function call
    return false;
}

WorldObject::WorldObject( WorldObject *instantiator )
{
    m_positionX         = 0.0f;
    m_positionY         = 0.0f;
    m_positionZ         = 0.0f;
    m_orientation       = 0.0f;

    m_mapId             = 0;
    m_InstanceId        = 0;

    m_name = "";

    mSemaphoreTeleport  = false;

    if (instantiator)
    {
        m_InstanceId = instantiator->GetInstanceId();
    }
}

void WorldObject::_Create( uint32 guidlow, uint32 guidhigh, uint32 mapid, float x, float y, float z, float ang, uint32 nameId )
{
    Object::_Create(guidlow, guidhigh);

    // nameId not required, it set in other place...
    //SetUInt32Value( OBJECT_FIELD_ENTRY,nameId);

    m_mapId = mapid;
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
    m_orientation = ang;
}

uint32 WorldObject::GetZoneId() const
{
    return MapManager::Instance().GetMap(m_mapId, this)->GetZoneId(m_positionX,m_positionY);
}

uint32 WorldObject::GetAreaId() const
{
    return MapManager::Instance().GetMap(m_mapId, this)->GetAreaId(m_positionX,m_positionY);
}

                                                            //slow
float WorldObject::GetDistanceSq(const WorldObject* obj) const
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

                                                            //slow
float WorldObject::GetDistance2dSq(const WorldObject* obj) const
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
    if (!obj || !IsInMap(obj)) return false;

    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float distsq = dx*dx + dy*dy + dz*dz;
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}
bool WorldObject::IsWithinLOSInMap(const WorldObject* obj) const
{
    if (!IsInMap(obj)) return false;
    float ox,oy,oz;
    obj->GetPosition(ox,oy,oz);
    return(IsWithinLOS(ox, oy, oz ));
}

bool WorldObject::IsWithinLOS(const float ox, const float oy, const float oz ) const
{
    float x,y,z;
    GetPosition(x,y,z);
    VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
    return vMapManager->isInLineOfSight(GetMapId(), x, y, z+2.0f, ox, oy, oz+2.0f);
}

float WorldObject::GetAngle(const WorldObject* obj) const
{
    if(!obj) return 0;
    return GetAngle( obj->GetPositionX(), obj->GetPositionY() );
}

// Return angle in range 0..2*pi
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

void WorldObject::GetContactPoint( const WorldObject* obj, float &x, float &y, float &z, float distance ) const
{
    // angle to face `obj` to `this`
    float angle = GetAngle( obj );
    x = GetPositionX() + (GetObjectSize() + obj->GetObjectSize() + distance ) * cos(angle);
    y = GetPositionY() + (GetObjectSize() + obj->GetObjectSize() + distance ) * sin(angle);

    if(VMAP::VMapFactory::createOrGetVMapManager()->isHeightCalcEnabled())
    {
        z = MapManager::Instance().GetMap(GetMapId(), this)->GetVMapHeight(x,y,GetPositionZ());
        if(z != VMAP_INVALID_HEIGHT)
            z += 0.2f; // just to be sure that we are not a few pixel under the surface
        else 
            z = GetPositionZ();
    }
    else
        z = GetPositionZ();                                 // hack required in case LOS height disabled
}

void WorldObject::GetRandomPoint( float x, float y, float z, float distance, float &rand_x, float &rand_y, float &rand_z) const
{
    if(distance==0)
    {
        rand_x = x;
        rand_y = y;
        rand_z = z;
        return;
    }

    // angle to face `obj` to `this`
    float angle = rand_norm()*2*M_PI;
    float new_dist = rand_norm()*distance;

    rand_x = x + new_dist * cos(angle);
    rand_y = y + new_dist * sin(angle);

    if(VMAP::VMapFactory::createOrGetVMapManager()->isHeightCalcEnabled())
    {
        rand_z = MapManager::Instance().GetMap(GetMapId(), this)->GetHeight(x,y,z);
        if(rand_z != VMAP_INVALID_HEIGHT)
            rand_z += 0.2f; // just to be sure that we are not a few pixel under the surface
        else 
            rand_z = GetPositionZ();
    }
    else
        rand_z = z;                                         // hack required in case LOS height disabled
}

void WorldObject::Say(const char* text, const uint32 language, const uint64 TargetGuid)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);    
    data << (uint8)CHAT_MSG_MONSTER_SAY;
    data << (uint32)language;
    data << (uint64)GetGUID();
    data << (uint32)0;                      //2.1.0
    data << (uint32)(strlen(GetName())+1);
    data << GetName();
    data << (uint64)TargetGuid;             //Unit Target
    data << (uint32)(strlen(text)+1);
    data << text;
    data << (uint8)0;                       // ChatTag

    SendMessageToSet(&data, true);
}

void WorldObject::Yell(const char* text, const uint32 language, const uint64 TargetGuid)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);    
    data << (uint8)CHAT_MSG_MONSTER_YELL;
    data << (uint32)language;
    data << (uint64)GetGUID();
    data << (uint32)0;                      //2.1.0
    data << (uint32)(strlen(GetName())+1);
    data << GetName();
    data << (uint64)TargetGuid;             //Unit Target
    data << (uint32)(strlen(text)+1);
    data << text;
    data << (uint8)0;                       // ChatTag

    SendMessageToSet(&data, true);
}

void WorldObject::TextEmote(const char* text, const uint64 TargetGuid)
{
    std::string rightText = "%s ";
    rightText.append(text);

    WorldPacket data(SMSG_MESSAGECHAT, 200);    
    data << (uint8)CHAT_MSG_MONSTER_EMOTE;
    data << (uint32)LANG_UNIVERSAL;
    data << (uint64)GetGUID();              // 2.1.0
    data << (uint32)0;                      // 2.1.0
    data << (uint32)(strlen(GetName())+1);
    data << GetName();
    data << (uint64)TargetGuid;             //Unit Target
    data << (uint32)(rightText.length()+1);
    data << rightText;
    data << (uint8)0;                       // ChatTag

    SendMessageToSet(&data, true);          // SendMessageToOwnTeamSet()?
}

void WorldObject::Whisper(const uint64 receiver, const char* text)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    data << (uint8)CHAT_MSG_MONSTER_WHISPER;
    data << (uint32)LANG_UNIVERSAL;
    data << (uint32)1;
    data << GetName();
    data << (uint64)receiver;               //Also the Unit Target
    data << (uint32)(strlen(text)+1);
    data << text;
    data << (uint8)0;                       // ChatTag

    Player *player = objmgr.GetPlayer(receiver);
    if(player && player->GetSession())
        player->GetSession()->SendPacket(&data);
}

void WorldObject::GetClosePoint( const WorldObject* victim, float &x, float &y, float &z, float distance, float angle ) const
{
    angle += victim ? GetAngle( victim ) : GetOrientation();

    x = GetPositionX() + (GetObjectSize() + distance) * cos(angle);
    y = GetPositionY() + (GetObjectSize() + distance) * sin(angle);

    if(VMAP::VMapFactory::createOrGetVMapManager()->isHeightCalcEnabled())
    {
        z = MapManager::Instance().GetMap(GetMapId(), this)->GetHeight(x,y,GetPositionZ());
        if(z != VMAP_INVALID_HEIGHT)
            z += 0.2f; // just to be sure that we are not a few pixel under the surface
        else 
            z = GetPositionZ();
    }
    else
        z = GetPositionZ();                                 // hack required in case LOS height disabled
}

bool WorldObject::IsPositionValid() const
{
    return MaNGOS::IsValidMapCoord(m_positionX,m_positionY);
}

void WorldObject::BuildHeartBeatMsg(WorldPacket *data) const
{
    data->Initialize(MSG_MOVE_HEARTBEAT, 32);

    data->append(GetPackGUID());
    *data << uint32(0);             // movement flags?
    *data << getMSTime();           // time
    *data << m_positionX;
    *data << m_positionY;
    *data << m_positionZ;
    *data << m_orientation;
    *data << uint32(0);
}

void WorldObject::BuildTeleportAckMsg(WorldPacket *data, float x, float y, float z, float ang) const
{
    data->Initialize(MSG_MOVE_TELEPORT_ACK, 41);
    data->append(GetPackGUID());
    *data << uint32(0);             // this value increments every time
    *data << uint32(0);             // movement flags?
    *data << getMSTime();           // time
    *data << x;
    *data << y;
    *data << z;
    *data << ang;
    *data << uint32(0);
}

void WorldObject::SendMessageToSet(WorldPacket *data, bool bToSelf)
{
    MapManager::Instance().GetMap(m_mapId, this)->MessageBoardcast(this, data);
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
