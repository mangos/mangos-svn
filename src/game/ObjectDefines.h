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

enum HIGHGUID
{
    HIGHGUID_ITEM           = 0x40000000,                   // blizz 40000000
    HIGHGUID_CONTAINER      = 0x40000000,                   // blizz 40000000
    HIGHGUID_UNIT           = 0xF0070000,                   // blizz F009????, where "????" is unit entry
    HIGHGUID_PLAYER         = 0x00000000,                   // blizz 00000000
    HIGHGUID_GAMEOBJECT     = 0xF0060000,                   // blizz F0090000
    HIGHGUID_DYNAMICOBJECT  = 0xF00A0000,                   // blizz F0090000, F0320000
    HIGHGUID_CORPSE         = 0xF0090000,                   // blizz F0090000, F0320000
    HIGHGUID_PLAYER_CORPSE  = 0xF0080000,                   // blizz F0090000, F0320000
    HIGHGUID_MO_TRANSPORT   = 0x1FC00000,                   // blizz 1FC00000 (type 15)
    HIGHGUID_TRANSPORT      = 0x1FA70000                    // blizz 1FA70000 (type 11)
};

#define IS_CREATURE_GUID(Guid)       ( GUID_HIPART(Guid) == HIGHGUID_UNIT )
#define IS_PLAYER_GUID(Guid)         ( GUID_HIPART(Guid) == HIGHGUID_PLAYER )
#define IS_ITEM_GUID(Guid)           ( GUID_HIPART(Guid) == HIGHGUID_ITEM )
#define IS_GAMEOBJECT_GUID(Guid)     ( GUID_HIPART(Guid) == HIGHGUID_GAMEOBJECT )
#define IS_DYNAMICOBJECT_GUID(Guid)  ( GUID_HIPART(Guid) == HIGHGUID_DYNAMICOBJECT )
#define IS_CORPSE_GUID(Guid)         ( GUID_HIPART(Guid) == HIGHGUID_CORPSE )
#define IS_PLAYER_CORPSE_GUID(Guid)  ( GUID_HIPART(Guid) == HIGHGUID_PLAYER_CORPSE )
#define IS_TRANSPORT(Guid)           ( GUID_HIPART(Guid) == HIGHGUID_TRANSPORT )
#define IS_MO_TRANSPORT(Guid)        ( GUID_HIPART(Guid) == HIGHGUID_MO_TRANSPORT )
#endif
