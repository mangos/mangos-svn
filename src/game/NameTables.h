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

/// \addtogroup u2w
/// @{
/// \file

#ifndef __NAMETABLES_H
#define __NAMETABLES_H

#include "Common.h"

struct NameTableEntry
{
    uint32 id;
    const char *name;
};

/// Lookup opcode name for human understandable logging
inline const char* LookupName(uint32 id, NameTableEntry *table)
{
    for(uint32 i = 0; table[i].name != 0; i++)
    {
        if (table[i].id == id)
            return table[i].name;
    }

    return "UNKNOWN";
}

extern NameTableEntry g_worldOpcodeNames[];
#endif
/// @}
