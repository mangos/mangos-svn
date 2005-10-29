/* Object.cpp
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
#include "MapMgr.h"

#include "Util.h"

#ifdef ENABLE_GRID_SYSTEM
#include "MapManager.h"
#include "ObjectAccessor.h"
#endif

using namespace std;

Object::Object( )
{
    m_objectTypeId = TYPEID_OBJECT;
    m_objectType = TYPE_OBJECT;

    m_positionX = 0.0f;
    m_positionY = 0.0f;
    m_positionZ = 0.0f;
    m_orientation = 0.0f;

    m_mapId = 0;
    m_zoneId = 0;

    m_uint32Values = 0;

    m_inWorld = false;

    m_minZ = -500;

    m_valuesCount = 0;

    m_walkSpeed = 2.5f;
    m_runSpeed = 7.0f;
    m_backWalkSpeed = 2.5;
    (*((uint32*)&m_swimSpeed)) = 0x40971c72;
    m_backSwimSpeed = 4.5;
    (*((uint32*)&m_turnRate)) = 0x40490FDF;
#ifndef ENABLE_GRID_SYSTEM
    m_mapMgr = 0;
#endif
    mSemaphoreTeleport = false;
}


Object::~Object( )
{
    if(m_uint32Values)
        delete [] m_uint32Values;
}


void Object::_Create( uint32 guidlow, uint32 guidhigh )
{
    if(!m_uint32Values) _InitValues();

    SetUInt32Value( OBJECT_FIELD_GUID, guidlow );
    SetUInt32Value( OBJECT_FIELD_GUID+1, guidhigh );
    SetUInt32Value( OBJECT_FIELD_TYPE, m_objectType );
}


void Object::_Create( uint32 guidlow, uint32 guidhigh, uint32 mapid, float x, float y, float z, float ang )
{
    if(!m_uint32Values) _InitValues();

    SetUInt32Value( OBJECT_FIELD_GUID, guidlow );
    SetUInt32Value( OBJECT_FIELD_GUID+1, guidhigh );
    SetUInt32Value( OBJECT_FIELD_TYPE, m_objectType );

    m_mapId = mapid;
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
    m_orientation = ang;
}


void Object::BuildMovementUpdateBlock(UpdateData * data, uint32 flags ) const
{
    ByteBuffer buf(500);

    buf << uint8( UPDATETYPE_MOVEMENT );          // update type
    buf << GetGUID();                             // object GUID

    _BuildMovementUpdate(&buf, flags, 0x00000000);

    data->AddUpdateBlock(buf);
}


void Object::BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    //they can see ANYONE or ANYTHING in a 30f radius from their corpse//FIX ME
#ifndef ENABLE_GRID_SYSTEM
    Creature *creat = objmgr.GetObject<Creature>(GetGUID());
#else    
    const Creature *creat = dynamic_cast<const Creature *>(this);
#endif
    if(target->isAlive())
    {
        //if creature exists and its spirit healer return
        if (creat && creat->GetUInt32Value(UNIT_FIELD_DISPLAYID) == 5233)
        {
            return;
        }
        if (creat)                                //if creature exists and isnt spririt healer update
        {
            ByteBuffer buf(500);

            // update type == creation
            buf << uint8( UPDATETYPE_CREATE_OBJECT );
            buf << GetGUID() ;                    // object GUID
            buf << GetTypeId();                   // object type

            // build and add the movement update portion of the packet
            _BuildMovementUpdate( &buf, 0x00000000, 0x00000000 );

            // 4 byte flags, 1 == active player
            buf << uint32( target == this ? 1 : 0 );
            buf << uint32( 0 );                   // uint32 attack cycle
            buf << uint32( 0 );                   // uint32 timer id
            buf << uint64( 0 );                   // GUID victim

            UpdateMask updateMask;
            updateMask.SetCount( m_valuesCount );
             _SetCreateBits( &updateMask, target );
            _BuildValuesUpdate( &buf, &updateMask );

            data->AddUpdateBlock(buf);
        }
        else if(!creat)                           //if there isnt any creature
        {
#ifndef ENABLE_GRID_SYSTEM
            Player *plyr = objmgr.GetObject<Player>(GetGUID());
#else
        Player *plyr = ObjectAccessor::Instance().FindPlayer(GetGUID());
#endif
            // if player exists and player and target is in group and player is dead
            if(plyr && plyr!=target && plyr->IsInGroup() && target->IsInGroup() && plyr->isDead())
            {
                if(plyr->IsGroupMember(target))   //if its group member of target update
                {
                    ByteBuffer buf(500);

                    // update type == creation
                    buf << uint8( UPDATETYPE_CREATE_OBJECT );
                    buf << GetGUID() ;            // object GUID
                    buf << GetTypeId();           // object type

                    // build and add the movement update portion of the packet
                    _BuildMovementUpdate( &buf, 0x00000000, 0x00000000 );

                    // 4 byte flags, 1 == active player
                    buf << uint32( target == this ? 1 : 0 );
                    buf << uint32( 0 );           // uint32 attack cycle
                    buf << uint32( 0 );           // uint32 timer id
                    buf << uint64( 0 );           // GUID victim

                    UpdateMask updateMask;
                    updateMask.SetCount( m_valuesCount );
                    _SetCreateBits( &updateMask, target );
                    _BuildValuesUpdate( &buf, &updateMask );

                    data->AddUpdateBlock(buf);
                }
                else                              //if isnt party member and dead return
                {
                    return;
                }
            }
            else if(plyr && plyr->isDead())       //if player isnt in a group and dead return
            {
                return;
            }
            // player end
            else                                  //self update
            {
                ByteBuffer buf(500);

                // update type == creation
                buf << uint8( UPDATETYPE_CREATE_OBJECT );
                buf << GetGUID() ;                // object GUID
                buf << GetTypeId();               // object type

                // build and add the movement update portion of the packet
                _BuildMovementUpdate( &buf, 0x00000000, 0x00000000 );

                // 4 byte flags, 1 == active player
                buf << uint32( target == this ? 1 : 0 );
                buf << uint32( 0 );               // uint32 attack cycle
                buf << uint32( 0 );               // uint32 timer id
                buf << uint64( 0 );               // GUID victim

                UpdateMask updateMask;
                updateMask.SetCount( m_valuesCount );
                _SetCreateBits( &updateMask, target );
                _BuildValuesUpdate( &buf, &updateMask );

                data->AddUpdateBlock(buf);
            }
        }
/*
        else                                      //is it needed
        {
            ByteBuffer buf(500);

            buf << uint8( UPDATETYPE_CREATE_OBJECT );// update type == creation
            buf << GetGUID() ;                       // object GUID
            buf << GetTypeId();                      // object type

            // build and add the movement update portion of the packet
            _BuildMovementUpdate( &buf, 0x00000000, 0x00000000 );

            buf << uint32( target == this ? 1 : 0 ); // 4 byte flags, 1 == active player
            buf << uint32( 0 );                      // uint32 attack cycle
            buf << uint32( 0 );                      // uint32 timer id
            buf << uint64( 0 );                      // GUID victim

            UpdateMask updateMask;
            updateMask.SetCount( m_valuesCount );
            _SetCreateBits( &updateMask, target );
            _BuildValuesUpdate( &buf, &updateMask );

            data->AddUpdateBlock(buf);
        }
*/
    }
    if(target->isDead())
    {
        if(!creat)
        {
#ifndef ENABLE_GRID_SYSTEM
            Player *plyr = objmgr.GetObject<Player>(GetGUID());
#else
        Player *plyr = ObjectAccessor::Instance().FindPlayer(GetGUID());
#endif
            // if player and player is in group of target update
            if(plyr && plyr->IsGroupMember(target))
            {
                ByteBuffer buf(500);

                // update type == creation
                buf << uint8( UPDATETYPE_CREATE_OBJECT );
                buf << GetGUID() ;                // object GUID
                buf << GetTypeId();               // object type

                // build and add the movement update portion of the packet
                _BuildMovementUpdate( &buf, 0x00000000, 0x00000000 );

                // 4 byte flags, 1 == active player
                buf << uint32( target == this ? 1 : 0 );
                buf << uint32( 0 );               // uint32 attack cycle
                buf << uint32( 0 );               // uint32 timer id
                buf << uint64( 0 );               // GUID victim

                UpdateMask updateMask;
                updateMask.SetCount( m_valuesCount );
                _SetCreateBits( &updateMask, target );
                _BuildValuesUpdate( &buf, &updateMask );

                data->AddUpdateBlock(buf);
            }
            else if(plyr && plyr->isAlive())      //if player is alive and they are in different groups return
            {
                return;
            }
            else                                  //if player and target is dead update or self update
            {
                ByteBuffer buf(500);

                // update type == creation
                buf << uint8( UPDATETYPE_CREATE_OBJECT );
                buf << GetGUID() ;                // object GUID
                buf << GetTypeId();               // object type

                // build and add the movement update portion of the packet
                _BuildMovementUpdate( &buf, 0x00000000, 0x00000000 );

                // 4 byte flags, 1 == active player
                buf << uint32( target == this ? 1 : 0 );
                buf << uint32( 0 );               // uint32 attack cycle
                buf << uint32( 0 );               // uint32 timer id
                buf << uint64( 0 );               // GUID victim

                UpdateMask updateMask;
                updateMask.SetCount( m_valuesCount );
                _SetCreateBits( &updateMask, target );
                _BuildValuesUpdate( &buf, &updateMask );

                data->AddUpdateBlock(buf);
            }
        }
        // if creature exists and its spirit healer update
        else if(creat && creat->GetUInt32Value(UNIT_FIELD_DISPLAYID) == 5233)
        {
            ByteBuffer buf(500);

            // update type == creation
            buf << uint8( UPDATETYPE_CREATE_OBJECT );
            buf << GetGUID() ;                    // object GUID
            buf << GetTypeId();                   // object type

            // build and add the movement update portion of the packet
            _BuildMovementUpdate( &buf, 0x00000000, 0x00000000 );

            // 4 byte flags, 1 == active player
            buf << uint32( target == this ? 1 : 0 );
            buf << uint32( 0 );                   // uint32 attack cycle
            buf << uint32( 0 );                   // uint32 timer id
            buf << uint64( 0 );                   // GUID victim

            UpdateMask updateMask;
            updateMask.SetCount( m_valuesCount );
            _SetCreateBits( &updateMask, target );
            _BuildValuesUpdate( &buf, &updateMask );

            data->AddUpdateBlock(buf);
        }
        else if(creat)                            //if creature exists and its not spirit healer reaturn
        {
            return;
        }
        else                                      //Does it needed?Want to know test it then.
        {
            ByteBuffer buf(500);

            // update type == creation
            buf << uint8( UPDATETYPE_CREATE_OBJECT );
            buf << GetGUID() ;                    // object GUID
            buf << GetTypeId();                   // object type

            // build and add the movement update portion of the packet
            _BuildMovementUpdate( &buf, 0x00000000, 0x00000000 );

            // 4 byte flags, 1 == active player
            buf << uint32( target == this ? 1 : 0 );
            buf << uint32( 0 );                   // uint32 attack cycle
            buf << uint32( 0 );                   // uint32 timer id
            buf << uint64( 0 );                   // GUID victim

            UpdateMask updateMask;
            updateMask.SetCount( m_valuesCount );
            _SetCreateBits( &updateMask, target );
            _BuildValuesUpdate( &buf, &updateMask );

            data->AddUpdateBlock(buf);
        }
    }
}


void Object::BuildValuesUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    ByteBuffer buf(500);

    buf << (uint8) UPDATETYPE_VALUES;             // update type == creation
    buf << GetGUID() ;                            // object GUID

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

    WorldPacket data;
    data.Initialize( SMSG_DESTROY_OBJECT );
    data << GetGUID();

    target->GetSession()->SendPacket( &data );
}


///////////////////////////////////////////////////////////////
/// Build the Movement Data portion of the update packet
/// Fills the data with this object's movement/speed info
/// TODO: rewrite this stuff, document unknown fields and flags
void Object::_BuildMovementUpdate(ByteBuffer * data, uint32 flags, uint32 flags2 ) const
{
    int spline_count = 0;

    *data << (uint32)flags;
    *data << (uint32)0;

    *data << (float)m_positionX;
    *data << (float)m_positionY;
    *data << (float)m_positionZ;
    *data << (float)m_orientation;
    *data << (float)0;

    if (flags & 0x20000000)
    {
        *data << (uint32)0;                       // uint64 Transport GUID
        *data << (uint32)0;
        *data << (float)0;                        // Float32 TransportX
        *data << (float)0;                        // Float32 TransportY
        *data << (float)0;                        // Float32 TransportZ
        *data << (float)0;                        // Float32 Transport Facing
    }

    if (flags & 0x1000000)
    {
        *data << (float)0;                        // Float32
    }

    if (flags & 0x4000)
    {
        *data << (uint16)0;                       // uint16
        *data << (float)0;                        // Float32 X
        *data << (float)0;                        // Float32 Y
        *data << (float)0;                        // Float32 Z
        *data << (float)0;                        // Float32 Facing
    }

    *data << m_walkSpeed;                         // walk speed
    *data << m_runSpeed;                          // run speed
    *data << m_backWalkSpeed;                     // backwards walk speed
    *data << m_swimSpeed;                         // swim speed
    *data << m_backSwimSpeed;                     // backwards swim speed
    *data << m_turnRate;                          // turn rate

    if ((flags & 0x00200000) != 0)
    {
        *data << (uint32)flags2;

        if (flags2 & 0x10000)
        {
            *data << (float)0;                    // Float32
        }

        if (flags2 & 0x20000)
        {
            *data << (uint32)0;                   // uint64
            *data << (uint32)0;
        }

        if (flags2 & 0x40000)
        {
            *data << (float)0;                    // Float32
        }

        *data << (uint16)0;                       // int16?
        *data << (uint32)0;                       // int32

        *data << (uint16)spline_count;            // uint16 Spline Count!?

        if (spline_count > 0)
        {
            for (int i = 0; i < spline_count; i++)
            {
                // 6 (wtf it's 8 :\) bytes per spline point
                *data << uint32(0);               // uint64
                *data << uint32(0);
            }
        }
    }                                             // end if flags2
}


//=======================================================================================
//  Creates an update block with the values of this object as
//  determined by the updateMask.
//=======================================================================================
void Object::_BuildValuesUpdate(ByteBuffer * data, UpdateMask *updateMask) const
{
    WPAssert(updateMask && updateMask->GetCount() == m_valuesCount);

    *data << (uint8)updateMask->GetBlockCount();
    data->append( updateMask->GetMask(), updateMask->GetLength() );

    for( uint16 index = 0; index < m_valuesCount; index ++ )
    {
        if( updateMask->GetBit( index ) )
            *data << m_uint32Values[ index ];
    }
}


void Object::BuildHeartBeatMsg(WorldPacket *data) const
{
    data->Initialize(MSG_MOVE_HEARTBEAT);

    *data << GetGUID();

    *data << uint32(0);                           // flags
    *data << uint32(0);                           // mysterious value #1

    *data << m_positionX;
    *data << m_positionY;
    *data << m_positionZ;

    *data << m_orientation;
}


void Object::BuildTeleportAckMsg(WorldPacket *data, float x, float y, float z, float ang) const
{
    ///////////////////////////////////////
    //Update player on the client with TELEPORT_ACK
    data->Initialize(MSG_MOVE_TELEPORT_ACK);

    *data << GetGUID();

    //First 4 bytes = no idea what it is
    *data << uint32(0);                           // flags
    *data << uint32(0);                           // mysterious value #1

    *data << x;
    *data << y;
    *data << z;
    *data << ang;
}

#ifndef ENABLE_GRID_SYSTEM
bool Object::SetPosition( float newX, float newY, float newZ, float newOrientation, bool allowPorting )
{
    m_orientation = newOrientation;
    bool updateMap = false, result = true;

    if (m_positionX != newX || m_positionY != newY)
        updateMap = true;

    m_positionX = newX;
    m_positionY = newY;
    m_positionZ = newZ;

    if (!allowPorting && newZ < m_minZ)
    {
        m_positionZ = 500;
        sLog.outError( "setPosition: fell through map; height ported" );

        result = false;
    }

    if (IsInWorld() && updateMap)
        m_mapMgr->ChangeObjectLocation(this);

    return result;
}
#endif

void Object::SendMessageToSet(WorldPacket *data, bool bToSelf)
{
#ifndef ENABLE_GRID_SYSTEM
    if (bToSelf && GetTypeId() == TYPEID_PLAYER)
    {
        // has to be a player to send to self
        ((Player*)this)->GetSession()->SendPacket(data);
    }

    std::set<Object*>::iterator itr;
    for (itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
    {
        ASSERT(*itr);

        if ((*itr)->GetTypeId() == TYPEID_PLAYER)
        {
            WorldSession *session = ((Player*)(*itr))->GetSession();
            WPWarning( session, "Null client in message set!" );
            session->SendPacket(data);
        }
    }
#else
    MapManager::Instance().GetMap(m_mapId)->MessageBoardcast(this, data);
#endif
}


/*
void Object::SendPacketListToSet(WorldSession::MessageList & msglist, bool bToSelf)
{
    if (bToSelf && GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->GetSession()->SendPacketList( msglist ); // has to be a player to send to self

    std::set<Object*>::iterator itr;
    for (itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
    {
        WPWarning((*itr), "Warning:  NULL Iterator in Set, skipping.");
        if (!(*itr))
            continue;

        if ((*itr)->GetTypeId() == TYPEID_PLAYER)
        {
            WorldSession *session = ((Player*)(*itr))->GetSession();
            WPWarning( session, "Null client in message set!" );
            if (session && session->IsInWorld() && session->GetPlayer())
                session->SendPacketList( msglist );
        }
    }
}
*/

////////////////////////////////////////////////////////////////////////////
/// Fill the object's Update Values from a space deliminated list of values.
void Object::LoadValues(const char* data)
{
    if(!m_uint32Values) _InitValues();

    vector<string> tokens = StrSplit(data, " ");

    vector<string>::iterator iter;
    int index;

    for (iter = tokens.begin(), index = 0;
        index < m_valuesCount && iter != tokens.end(); ++iter, ++index)
    {
        m_uint32Values[index] = atol((*iter).c_str());
    }
}


void Object::LoadTaxiMask(const char* data)
{
    vector<string> tokens = StrSplit(data, " ");

    int index;
    vector<string>::iterator iter;

    for (iter = tokens.begin(), index = 0;
        (index < 8) && (iter != tokens.end()); ++iter, ++index)
    {
        m_taximask[index] = atol((*iter).c_str());
    }
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

#ifndef ENABLE_GRID_SYSTEM
void Object::PlaceOnMap()
{
    ASSERT(!IsInWorld() && !m_mapMgr);

    MapMgr* mapMgr = sWorld.GetMap(m_mapId);
    ASSERT(mapMgr);

    mapMgr->AddObject(this);
    m_mapMgr = mapMgr;
    mSemaphoreTeleport = false;
}


void Object::RemoveFromMap()
{
    ASSERT(IsInWorld());
    mSemaphoreTeleport = true;
    m_mapMgr->RemoveObject(this);
    m_mapMgr = 0;
}
#endif

//! Set uint32 property
void Object::SetUInt32Value( const uint16 &index, const uint32 &value )
{
    ASSERT( index < m_valuesCount );
    m_uint32Values[ index ] = value;

    if(m_inWorld)
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
#ifndef ENABLE_GRID_SYSTEM
            m_mapMgr->ObjectUpdated(this);
#else
        ObjectAccessor::Instance().AddUpdateObject(this);
#endif
            m_objectUpdated = true;
        }
    }
}


//! Set uint64 property
void Object::SetUInt64Value( const uint16 &index, const uint64 &value )
{
    ASSERT( index + 1 < m_valuesCount );
    m_uint32Values[ index ] = *((uint32*)&value);
    m_uint32Values[ index + 1 ] = *(((uint32*)&value) + 1);

    if(m_inWorld)
    {
        m_updateMask.SetBit( index );
        m_updateMask.SetBit( index + 1 );

        if(!m_objectUpdated)
        {
#ifndef ENABLE_GRID_SYSTEM
            m_mapMgr->ObjectUpdated(this);
#else
        ObjectAccessor::Instance().AddUpdateObject(this);
#endif
            m_objectUpdated = true;
        }
    }
}


//! Set float property
void Object::SetFloatValue( const uint16 &index, const float &value )
{
    ASSERT( index < m_valuesCount );
    m_floatValues[ index ] = value;

    if(m_inWorld)
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
#ifndef ENABLE_GRID_SYSTEM
            m_mapMgr->ObjectUpdated(this);
#else
        ObjectAccessor::Instance().AddUpdateObject(this);
#endif
            m_objectUpdated = true;
        }
    }
}


void Object::SetFlag( const uint16 &index, uint32 newFlag )
{
    ASSERT( index < m_valuesCount );
    m_uint32Values[ index ] |= newFlag;

    if(m_inWorld)
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
#ifndef ENABLE_GRID_SYSTEM
            m_mapMgr->ObjectUpdated(this);
#else
        ObjectAccessor::Instance().AddUpdateObject(this);
#endif
            m_objectUpdated = true;
        }
    }
}


void Object::RemoveFlag( const uint16 &index, uint32 oldFlag )
{
    ASSERT( index < m_valuesCount );
    m_uint32Values[ index ] &= ~oldFlag;

    if(m_inWorld)
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
#ifndef ENABLE_GRID_SYSTEM
            m_mapMgr->ObjectUpdated(this);
#else
        ObjectAccessor::Instance().AddUpdateObject(this);
#endif
            m_objectUpdated = true;
        }
    }
}
