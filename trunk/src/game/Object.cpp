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
#include "Creature.h"
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
#include "WaypointMovementGenerator.h"
#include "TargetedMovementGenerator.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

#include "ObjectPosSelector.h"

#include "TemporarySummon.h"

uint32 GuidHigh2TypeId(uint32 guid_hi)
{
    switch(guid_hi)
    {
        case HIGHGUID_ITEM:         return TYPEID_ITEM;
        //case HIGHGUID_CONTAINER:    return TYPEID_CONTAINER; HIGHGUID_CONTAINER==HIGHGUID_ITEM currently
        case HIGHGUID_UNIT:         return TYPEID_UNIT;
        case HIGHGUID_PLAYER:       return TYPEID_PLAYER;
        case HIGHGUID_GAMEOBJECT:   return TYPEID_GAMEOBJECT;
        case HIGHGUID_DYNAMICOBJECT:return TYPEID_DYNAMICOBJECT;
        case HIGHGUID_CORPSE:       return TYPEID_CORPSE;
        case HIGHGUID_PLAYER_CORPSE:return 10;              // unknown
        case HIGHGUID_MO_TRANSPORT: return TYPEID_GAMEOBJECT;
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
    if(target == this)                                      // building packet for oneself
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
    _BuildValuesUpdate(updatetype, &buf, &updateMask, target );
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
    _BuildValuesUpdate(UPDATETYPE_VALUES, &buf, &updateMask, target );

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
    *data << (uint8)flags;                                  // update flags

    // 0x20
    if (flags & UPDATEFLAG_LIVING)
    {
        switch(GetTypeId())
        {
            case TYPEID_UNIT:
            {
                switch(GetEntry())
                {
                    case 6491:                              // Spirit Healer
                    case 13116:                             // Alliance Spirit Guide
                    case 13117:                             // Horde Spirit Guide
                        flags2 |= MOVEMENTFLAG_WATERWALKING;// waterwalking movement flag?
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
                flags2 &= ~MOVEMENTFLAG_SPLINE2;            // will be set manually

                if(((Player*)this)->isInFlight())
                {
                    WPAssert(!((Player*)this)->GetMotionMaster()->empty() && ((Player*)this)->GetMotionMaster()->top()->GetMovementGeneratorType() == FLIGHT_MOTION_TYPE);
                    flags2 = (MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_SPLINE2);
                }
            }
            break;
        }

        *data << uint32(flags2);                            // movement flags
        *data << uint8(0);                                  // unk 2.3.0
        *data << uint32(getMSTime());                       // time (in milliseconds)
    }

    // 0x40
    if (flags & UPDATEFLAG_HASPOSITION)
    {
        // 0x02
        if(flags & UPDATEFLAG_TRANSPORT)
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

    // 0x20
    if(flags & UPDATEFLAG_LIVING)
    {
        // 0x00000200
        if(flags2 & MOVEMENTFLAG_ONTRANSPORT)
        {
            *data << (uint64)((Player*)this)->GetTransport()->GetGUID();
            *data << (float)((Player*)this)->GetTransOffsetX();
            *data << (float)((Player*)this)->GetTransOffsetY();
            *data << (float)((Player*)this)->GetTransOffsetZ();
            *data << (float)((Player*)this)->GetTransOffsetO();
            *data << (uint32)((Player*)this)->GetTransTime();
        }

        // 0x02200000
        if(flags2 & (MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_UNK5))
        {
            if(GetTypeId() == TYPEID_PLAYER)
                *data << (float)((Player*)this)->m_movementInfo.s_angle;
            else
                *data << (float)0;                          // is't part of movement packet, we must store and send it...
        }

        if(GetTypeId() == TYPEID_PLAYER)
            *data << (uint32)((Player*)this)->m_movementInfo.fallTime;
        else
            *data << (uint32)0;                             // last fall time

        // 0x00001000
        if(flags2 & MOVEMENTFLAG_JUMPING)
        {
            if(GetTypeId() == TYPEID_PLAYER)
            {
                *data << (float)((Player*)this)->m_movementInfo.j_unk;
                *data << (float)((Player*)this)->m_movementInfo.j_sinAngle;
                *data << (float)((Player*)this)->m_movementInfo.j_cosAngle;
                *data << (float)((Player*)this)->m_movementInfo.j_xyspeed;
            }
            else
            {
                *data << (float)0;
                *data << (float)0;
                *data << (float)0;
                *data << (float)0;
            }
        }

        // 0x04000000
        if(flags2 & MOVEMENTFLAG_SPLINE)
        {
            if(GetTypeId() == TYPEID_PLAYER)
                *data << (float)((Player*)this)->m_movementInfo.u_unk1;
            else
                *data << (float)0;
        }

        *data << ((Unit*)this)->GetSpeed( MOVE_WALK );
        *data << ( ((Unit*)this)->IsMounted() ? ((Unit*)this)->GetSpeed( MOVE_MOUNTED ) : ((Unit*)this)->GetSpeed( MOVE_RUN ) );
        *data << ((Unit*)this)->GetSpeed( MOVE_SWIMBACK );
        *data << ((Unit*)this)->GetSpeed( MOVE_SWIM );
        *data << ((Unit*)this)->GetSpeed( MOVE_WALKBACK );
        *data << ((Unit*)this)->GetSpeed( MOVE_FLY );
        *data << ((Unit*)this)->GetSpeed( MOVE_FLYBACK );
        *data << ((Unit*)this)->GetSpeed( MOVE_TURN );

        // 0x08000000
        if(flags2 & MOVEMENTFLAG_SPLINE2)
        {
            if(!((Player*)this)->isInFlight())
            {
                sLog.outDebug("_BuildMovementUpdate: MOVEMENTFLAG_SPLINE2 but not in flight");
                return;
            }

            WPAssert(!((Player*)this)->GetMotionMaster()->empty() && ((Player*)this)->GetMotionMaster()->top()->GetMovementGeneratorType() == FLIGHT_MOTION_TYPE);

            FlightPathMovementGenerator *fmg = (FlightPathMovementGenerator*)(((Player*)this)->GetMotionMaster()->top());

            uint32 flags3 = 0x00000300;

            *data << uint32(flags3);                        // splines flag?

            if(flags3 & 0x10000)                            // probably x,y,z coords there
            {
                *data << (float)0;
                *data << (float)0;
                *data << (float)0;
            }

            if(flags3 & 0x20000)                            // probably guid there
            {
                *data << uint64(0);
            }

            if(flags3 & 0x40000)                            // may be orientation
            {
                *data << (float)0;
            }

            Path &path = fmg->GetPath();

            float x, y, z;
            ((Player*)this)->GetPosition(x, y, z);

            uint32 inflighttime = uint32(path.GetPassedLength(fmg->GetCurrentNode(), x, y, z) * 32);
            uint32 traveltime = uint32(path.GetTotalLength() * 32);

            *data << uint32(inflighttime);                  // passed move time?
            *data << uint32(traveltime);                    // full move time?
            *data << uint32(0);                             // ticks count?

            uint32 poscount = uint32(path.Size());

            *data << uint32(poscount);                      // points count

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

    // 0x8
    if(flags & UPDATEFLAG_LOWGUID)
    {
        switch(GetTypeId())
        {
            case TYPEID_OBJECT:
            case TYPEID_ITEM:
            case TYPEID_CONTAINER:
            case TYPEID_GAMEOBJECT:
            case TYPEID_DYNAMICOBJECT:
            case TYPEID_CORPSE:
                *data << uint32(GetGUIDLow());              // GetGUIDLow()
                break;
            case TYPEID_UNIT:
                *data << uint32(0x0000000B);                // unk, can be 0xB or 0xC
                break;
            case TYPEID_PLAYER:
                if(flags & UPDATEFLAG_SELF)
                    *data << uint32(0x00000015);            // unk, can be 0x15 or 0x22
                else
                    *data << uint32(0x00000008);            // unk, can be 0x7 or 0x8
                break;
            default:
                *data << uint32(0x00000000);                // unk
                break;
        }
    }

    // 0x10
    if(flags & UPDATEFLAG_HIGHGUID)
    {
        switch(GetTypeId())
        {
            case TYPEID_OBJECT:
            case TYPEID_ITEM:
            case TYPEID_CONTAINER:
            case TYPEID_GAMEOBJECT:
            case TYPEID_DYNAMICOBJECT:
            case TYPEID_CORPSE:
                *data << uint32(GetGUIDHigh());             // GetGUIDHigh()
                break;
            default:
                *data << uint32(0x00000000);                // unk
                break;
        }
    }

    // 0x4
    if(flags & UPDATEFLAG_FULLGUID)
    {
        *data << uint8(0);                                  // packed guid (probably target guid)
    }

    // 0x2
    if(flags & UPDATEFLAG_TRANSPORT)
    {
        *data << uint32(getMSTime());                       // ms time
    }
}

void Object::_BuildValuesUpdate(uint8 updatetype, ByteBuffer * data, UpdateMask *updateMask, Player *target) const
{
    if(!target)
        return;
    
    bool IsActivateToQuest = false;
    if (updatetype == UPDATETYPE_CREATE_OBJECT || updatetype == UPDATETYPE_CREATE_OBJECT2)
    {
        if (isType(TYPE_GAMEOBJECT) && !((GameObject*)this)->IsTransport())
        {   
            if ( ((GameObject*)this)->ActivateToQuest(target))
            {
                IsActivateToQuest = true;
                updateMask->SetBit(GAMEOBJECT_DYN_FLAGS);            
            }
        }
    }
    else            //case UPDATETYPE_VALUES
    {
        if (isType(TYPE_GAMEOBJECT) && !((GameObject*)this)->IsTransport())
        {   
            if ( ((GameObject*)this)->ActivateToQuest(target))
            {
                IsActivateToQuest = true;
            }
            updateMask->SetBit(GAMEOBJECT_DYN_FLAGS);            
            updateMask->SetBit(GAMEOBJECT_ANIMPROGRESS);
        }
    }

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
    else if(isType(TYPE_GAMEOBJECT))                        // gameobject case
    {
        for( uint16 index = 0; index < m_valuesCount; index ++ )
        {
            if( updateMask->GetBit( index ) )
            {
                // send in current format (float as float, uint32 as uint32)
                if ( index == GAMEOBJECT_DYN_FLAGS )
                {
                    if(IsActivateToQuest )
                        *data << uint32(9);                 // enable quest object. Represent 9, but 1 for client before 2.3.0
                    else
                        *data << uint32(0);                 // disable quest object
                }
                else
                    *data << m_uint32Values[ index ];       // other cases
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

void Object::SetInt32Value( uint16 index, int32 value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(m_int32Values[ index ] != value)
    {
        m_int32Values[ index ] = value;

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
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

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
    int32 cur = GetInt32Value(index);
    cur += (apply ? val : -val);
    SetInt32Value(index,cur);
}

void Object::ApplyModSignedFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    SetFloatValue(index,cur);
}

void Object::ApplyModPositiveFloatValue(uint16 index, float  val, bool apply)
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
    return MapManager::Instance().GetBaseMap(m_mapId)->GetZoneId(m_positionX,m_positionY);
}

uint32 WorldObject::GetAreaId() const
{
    return MapManager::Instance().GetBaseMap(m_mapId)->GetAreaId(m_positionX,m_positionY);
}

InstanceData* WorldObject::GetInstanceData()
{
    return MapManager::Instance().GetMap(m_mapId, this)->GetInstanceData();
}

                                                            //slow
float WorldObject::GetDistance(const WorldObject* obj) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistance2d(float x, float y) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float sizefactor = GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistance(const float x, const float y, const float z) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float sizefactor = GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistance2d(const WorldObject* obj) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return ( dist > 0 ? dist : 0);
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
    rand_z = z;

    UpdateGroundPositionZ(rand_x,rand_y,rand_z);            // update to LOS height if available
}

void WorldObject::UpdateGroundPositionZ(float x, float y, float &z) const
{
    if(VMAP::VMapFactory::createOrGetVMapManager()->isHeightCalcEnabled())
    {
        float new_z = MapManager::Instance().GetBaseMap(GetMapId())->GetHeight(x,y,z);
        if(new_z > VMAP_INVALID_HEIGHT)
            z = new_z+ 0.05f;                               // just to be sure that we are not a few pixel under the surface
    }
}

bool WorldObject::IsPositionValid() const
{
    return MaNGOS::IsValidMapCoord(m_positionX,m_positionY);
}

void WorldObject::MonsterSay(const char* text, const uint32 language, const uint64 TargetGuid)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    data << (uint8)CHAT_MSG_MONSTER_SAY;
    data << (uint32)language;
    data << (uint64)GetGUID();
    data << (uint32)0;                                      //2.1.0
    data << (uint32)(strlen(GetName())+1);
    data << GetName();
    data << (uint64)TargetGuid;                             //Unit Target
    data << (uint32)(strlen(text)+1);
    data << text;
    data << (uint8)0;                                       // ChatTag

    SendMessageToSet(&data, true);
}

void WorldObject::MonsterYell(const char* text, const uint32 language, const uint64 TargetGuid)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    data << (uint8)CHAT_MSG_MONSTER_YELL;
    data << (uint32)language;
    data << (uint64)GetGUID();
    data << (uint32)0;                                      //2.1.0
    data << (uint32)(strlen(GetName())+1);
    data << GetName();
    data << (uint64)TargetGuid;                             //Unit Target
    data << (uint32)(strlen(text)+1);
    data << text;
    data << (uint8)0;                                       // ChatTag

    SendMessageToSet(&data, true);
}

void WorldObject::MonsterTextEmote(const char* text, const uint64 TargetGuid)
{
    std::string rightText = "%s ";
    rightText.append(text);

    WorldPacket data(SMSG_MESSAGECHAT, 200);
    data << (uint8)CHAT_MSG_MONSTER_EMOTE;
    data << (uint32)LANG_UNIVERSAL;
    data << (uint64)GetGUID();                              // 2.1.0
    data << (uint32)0;                                      // 2.1.0
    data << (uint32)(strlen(GetName())+1);
    data << GetName();
    data << (uint64)TargetGuid;                             //Unit Target
    data << (uint32)(rightText.length()+1);
    data << rightText;
    data << (uint8)0;                                       // ChatTag

    SendMessageToSet(&data, true);                          // SendMessageToOwnTeamSet()?
}

void WorldObject::MonsterWhisper(const uint64 receiver, const char* text)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    data << (uint8)CHAT_MSG_MONSTER_WHISPER;
    data << (uint32)LANG_UNIVERSAL;
    data << (uint64)GetGUID();
    data << (uint32)0;                                      //unk1
    data << (uint32)(strlen(GetName())+1);
    data << GetName();
    data << (uint64)receiver;                               // Player's guid
    data << (uint32)(strlen(text)+1);
    data << text;
    data << (uint8)0;                                       // ChatTag

    Player *player = objmgr.GetPlayer(receiver);
    if(player && player->GetSession())
        player->GetSession()->SendPacket(&data);
}

void WorldObject::BuildHeartBeatMsg(WorldPacket *data) const
{
    data->Initialize(MSG_MOVE_HEARTBEAT, 32);

    data->append(GetPackGUID());
    *data << uint32(0);                                     // movement flags?
    *data << uint8(0);                                      // 2.3.0
    *data << getMSTime();                                   // time
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
    *data << uint32(0);                                     // this value increments every time
    *data << uint32(0);                                     // movement flags?
    *data << uint8(0);                                      // 2.3.0
    *data << getMSTime();                                   // time
    *data << x;
    *data << y;
    *data << z;
    *data << ang;
    *data << uint32(0);
}

void WorldObject::SendMessageToSet(WorldPacket *data, bool bToSelf)
{
    MapManager::Instance().GetMap(m_mapId, this)->MessageBroadcast(this, data);
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

Map* WorldObject::GetMap() const
{
    return MapManager::Instance().GetMap(GetMapId(), this);
}

Map const* WorldObject::GetBaseMap() const
{
    return MapManager::Instance().GetBaseMap(GetMapId());
}

Creature* WorldObject::SummonCreature(uint32 id, float x, float y, float z, float ang,TempSummonType spwtype,uint32 despwtime)
{
    TemporarySummon* pCreature = new TemporarySummon(this, GetGUID());

    pCreature->SetInstanceId(GetInstanceId());
    uint32 team = 0;
    if (GetTypeId()==TYPEID_PLAYER)
        team = ((Player*)this)->GetTeam();

    if (!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), GetMapId(), x, y, z, ang, id, team))
    {
        delete pCreature;
        return NULL;
    }

    pCreature->Summon(spwtype, despwtime);

    //return the creature therewith the summoner has access to it
    return pCreature;
}

namespace MaNGOS
{

    class NearUsedPosDo
    {
        public:
            NearUsedPosDo(WorldObject const& obj, WorldObject const* searcher, float angle, ObjectPosSelector& selector)
                : i_object(obj), i_searcher(searcher), i_angle(angle), i_selector(selector) {}

            void operator()(Corpse* u) const {}
            void operator()(DynamicObject* u) const {}

            void operator()(Creature* c) const
            {
                // skip self or target
                if(c==i_searcher || c==&i_object)
                    return;

                float x,y,z;

                if( !c->isAlive() || c->hasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNDED) || c->GetMotionMaster()->top()->GetMovementGeneratorType() != TARGETED_MOTION_TYPE ||
                    !((TargetedMovementGenerator<Creature>*)(c->GetMotionMaster()->top()))->GetDestination(x,y,z) )
                {
                    x = c->GetPositionX();
                    y = c->GetPositionY();
                }

                add(c,x,y);
            }

            template<class T>
                void operator()(T* u) const
            {
                // skip self or target
                if(u==i_searcher || u==&i_object)
                    return;

                float x,y;

                x = u->GetPositionX();
                y = u->GetPositionY();

                add(u,x,y);
            }

            // we must add used pos that can fill places around center
            void add(WorldObject* u, float x, float y) const
            {
                // dist include size of u
                float dist2d = i_object.GetDistance2d(x,y);

                // u is too nearest to i_object
                if(dist2d + i_object.GetObjectSize() + u->GetObjectSize() < i_selector.m_dist - i_selector.m_size)
                    return;

                // u is too far away from i_object
                if(dist2d + i_object.GetObjectSize() - u->GetObjectSize() > i_selector.m_dist + i_selector.m_size)
                    return;

                float angle = i_object.GetAngle(u)-i_angle;

                // move angle to range -pi ... +pi
                while( angle > M_PI)
                    angle -= 2.0f * M_PI;
                while(angle < -M_PI)
                    angle += 2.0f * M_PI;

                i_selector.AddUsedPos(u->GetObjectSize(),angle,dist2d + i_object.GetObjectSize());
            }
        private:
            WorldObject const& i_object;
            WorldObject const* i_searcher;
            float              i_angle;
            ObjectPosSelector& i_selector;
    };

}                                                           // namespace MaNGOS

//===================================================================================================

void WorldObject::GetNearPoint2D(float &x, float &y, float distance2d, float absAngle ) const
{
    x = GetPositionX() + (GetObjectSize() + distance2d) * cos(absAngle);
    y = GetPositionY() + (GetObjectSize() + distance2d) * sin(absAngle);
}

void WorldObject::GetNearPoint(WorldObject const* searcher, float &x, float &y, float &z, float distance2d, float absAngle ) const
{
    float size = searcher ? searcher->GetObjectSize() : 0;

    GetNearPoint2D(x,y,distance2d+size,absAngle);
    z = GetPositionZ();

    // if detection disabled, return first point
    if(!sWorld.getConfig(CONFIG_DETECT_POS_COLLISION))
    {
        UpdateGroundPositionZ(x,y,z);                       // update to LOS height if available
        return;
    }

    // or remember first point
    float first_x = x;
    float first_y = y;
    bool first_los_conflict = false;                        // first point LOS problems

    // prepare selector for for work
    ObjectPosSelector selector(GetPositionX(),GetPositionY(),GetObjectSize(),distance2d+size);

    // adding used positions around object
    {
        CellPair p(MaNGOS::ComputeCellPair(GetPositionX(), GetPositionY()));
        Cell cell(p);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();

        MaNGOS::NearUsedPosDo u_do(*this,searcher,absAngle,selector);
        MaNGOS::WorldObjectWorker<MaNGOS::NearUsedPosDo> worker(u_do);

        TypeContainerVisitor<MaNGOS::WorldObjectWorker<MaNGOS::NearUsedPosDo>, GridTypeMapContainer  > grid_obj_worker(worker);
        TypeContainerVisitor<MaNGOS::WorldObjectWorker<MaNGOS::NearUsedPosDo>, WorldTypeMapContainer > world_obj_worker(worker);

        CellLock<GridReadGuard> cell_lock(cell, p);
        cell_lock->Visit(cell_lock, grid_obj_worker,  *MapManager::Instance().GetMap(GetMapId(), this));
        cell_lock->Visit(cell_lock, world_obj_worker, *MapManager::Instance().GetMap(GetMapId(), this));
    }

    // maybe can just place in primary position
    if( selector.CheckOriginal() )
    {
        UpdateGroundPositionZ(x,y,z);                       // update to LOS height if available

        if(IsWithinLOS(x,y,z))
            return;

        first_los_conflict = true;                          // first point have LOS problems
    }

    float angle;                                            // candidate of angle for free pos

    // special case when one from list empty and then empty side preferred
    if(selector.FirstAngle(angle))
    {
        GetNearPoint2D(x,y,distance2d,absAngle+angle);
        z = GetPositionZ();
        UpdateGroundPositionZ(x,y,z);                       // update to LOS height if available

        if(IsWithinLOS(x,y,z))
            return;
    }

    // set first used pos in lists
    selector.InitializeAngle();

    // select in positions after current nodes (selection one by one)
    while(selector.NextAngle(angle))                        // angle for free pos
    {
        GetNearPoint2D(x,y,distance2d,absAngle+angle);
        z = GetPositionZ();
        UpdateGroundPositionZ(x,y,z);                       // update to LOS height if available

        if(IsWithinLOS(x,y,z))
            return;
    }

    // BAD NEWS: not free pos (or used or have LOS problems)
    // Attempt find _used_ pos without LOS problem

    if(!first_los_conflict)
    {
        x = first_x;
        y = first_y;

        UpdateGroundPositionZ(x,y,z);                       // update to LOS height if available
        return;
    }

    // special case when one from list empty and then empty side preferred
    if( selector.IsNonBalanced() )
    {
        if(!selector.FirstAngle(angle))                     // _used_ pos
        {
            GetNearPoint2D(x,y,distance2d,absAngle+angle);
            z = GetPositionZ();
            UpdateGroundPositionZ(x,y,z);                   // update to LOS height if available

            if(IsWithinLOS(x,y,z))
                return;
        }
    }

    // set first used pos in lists
    selector.InitializeAngle();

    // select in positions after current nodes (selection one by one)
    while(selector.NextUsedAngle(angle))                    // angle for used pos but maybe without LOS problem
    {
        GetNearPoint2D(x,y,distance2d,absAngle+angle);
        z = GetPositionZ();
        UpdateGroundPositionZ(x,y,z);                       // update to LOS height if available

        if(IsWithinLOS(x,y,z))
            return;
    }

    // BAD BAD NEWS: all found pos (free and used) have LOS problem :(
    x = first_x;
    y = first_y;

    UpdateGroundPositionZ(x,y,z);                           // update to LOS height if available
}
