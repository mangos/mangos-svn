/* ZoneMapper.h
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

#ifndef __ZONEMAPPER_H
#define __ZONEMAPPER_H

#ifndef ENABLE_GRID_SYSTEM
#include "Common.h"
#include <bitset>

#define MAXZONES 3500

class ZoneMapper
{
public:

    ZoneMapper() : m_ZoneIDmap(0) {}
    inline void SetZoneBitOn(uint32 zoneid) { m_ZoneIDmap.set(zoneid, 1); }
    inline void SetZoneBitOff(uint32 zoneid) { m_ZoneIDmap.set(zoneid, 0); }
    inline bool GetZoneBit(uint32 zoneid) { return m_ZoneIDmap.test(zoneid); }

protected:
    std::bitset<MAXZONES> m_ZoneIDmap;
};

extern ZoneMapper ZoneIDMap;
// This file will be remove when grid system is enabled
#endif

#endif
