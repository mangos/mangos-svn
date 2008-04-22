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

#ifndef MANGOS_OBJECTDEFINES_H
#define MANGOS_OBJECTDEFINES_H

#include "Platform/Define.h"

// used for creating values for respawn for example
#define MAKE_PAIR(l, h)  uint64( uint32(l) | ( uint64(h) << 32 ) )
#define PAIR_HIPART(x)   (uint32)((uint64(x) >> 48) & 0x0000FFFF)
#define PAIR_LOPART(x)   (uint32)(uint64(x) & 0x00FFFFFF)

enum HighGuid
{
    HIGHGUID_ITEM           = 0x4000,                       // blizz 4000
    HIGHGUID_CONTAINER      = 0x4000,                       // blizz 4000
    HIGHGUID_PLAYER         = 0x0000,                       // blizz 0000
    HIGHGUID_GAMEOBJECT     = 0xF110,                       // blizz F110
    HIGHGUID_TRANSPORT      = 0xF120,                       // blizz F120 (for GAMEOBJECT_TYPE_TRANSPORT)
    HIGHGUID_UNIT           = 0xF130,                       // blizz F130
    HIGHGUID_PET            = 0xF140,                       // blizz F140
    HIGHGUID_DYNAMICOBJECT  = 0xF100,                       // blizz F100
    HIGHGUID_CORPSE         = 0xF101,                       // blizz F100
    HIGHGUID_MO_TRANSPORT   = 0x1FC0,                       // blizz 1FC0 (for GAMEOBJECT_TYPE_MO_TRANSPORT)
};

#define IS_EMPTY_GUID(Guid)          ( Guid == 0 )

#define IS_CREATURE_GUID(Guid)       ( GUID_HIPART(Guid) == HIGHGUID_UNIT )
#define IS_PLAYER_GUID(Guid)         ( GUID_HIPART(Guid) == HIGHGUID_PLAYER && Guid!=0 )
                                                            // special case for empty guid need check
#define IS_ITEM_GUID(Guid)           ( GUID_HIPART(Guid) == HIGHGUID_ITEM )
#define IS_GAMEOBJECT_GUID(Guid)     ( GUID_HIPART(Guid) == HIGHGUID_GAMEOBJECT )
#define IS_DYNAMICOBJECT_GUID(Guid)  ( GUID_HIPART(Guid) == HIGHGUID_DYNAMICOBJECT )
#define IS_CORPSE_GUID(Guid)         ( GUID_HIPART(Guid) == HIGHGUID_CORPSE )
#define IS_TRANSPORT(Guid)           ( GUID_HIPART(Guid) == HIGHGUID_TRANSPORT )
#define IS_MO_TRANSPORT(Guid)        ( GUID_HIPART(Guid) == HIGHGUID_MO_TRANSPORT )

// l - OBJECT_FIELD_GUID
// e - OBJECT_FIELD_ENTRY for GO (except GAMEOBJECT_TYPE_MO_TRANSPORT) and creatures or UNIT_FIELD_PETNUMBER for pets
// h - OBJECT_FIELD_GUID + 1
#define MAKE_NEW_GUID(l, e, h)   uint64( uint64(l) | ( uint64(e) << 24 ) | ( uint64(h) << 48 ) )

#define GUID_HIPART(x)   (uint32)((uint64(x) >> 48) & 0x0000FFFF)

// We have different low and middle part size for different guid types
#define _GUID_ENPART_2(x) 0
#define _GUID_ENPART_3(x) (uint32)((uint64(x) >> 24) & 0x00FFFFFF)
#define _GUID_LOPART_2(x) (uint32)(uint64(x) & 0xFFFFFFFF)
#define _GUID_LOPART_3(x) (uint32)(uint64(x) & 0x00FFFFFF)

inline bool IsGuidHaveEnPart(uint64 const& guid)
{
    switch(GUID_HIPART(guid))
    {
        case HIGHGUID_ITEM:
        case HIGHGUID_PLAYER:
            return false; 
        case HIGHGUID_GAMEOBJECT:
        case HIGHGUID_TRANSPORT:
        case HIGHGUID_UNIT:
        case HIGHGUID_PET:
        case HIGHGUID_DYNAMICOBJECT:
        case HIGHGUID_CORPSE:
        case HIGHGUID_MO_TRANSPORT:
        default:
            return true; 
    }
}

#define GUID_ENPART(x) (IsGuidHaveEnPart(x) ? _GUID_ENPART_3(x) : _GUID_ENPART_2(x))
#define GUID_LOPART(x) (IsGuidHaveEnPart(x) ? _GUID_LOPART_3(x) : _GUID_LOPART_2(x))

#endif
