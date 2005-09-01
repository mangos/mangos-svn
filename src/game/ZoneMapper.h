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

#include "Common.h"

#define MAXZONES 2777

class ZoneMapper
{
    public:
        ZoneMapper();
        ~ZoneMapper();
        void SetZoneBitOn(uint32 zoneid);
        void SetZoneBitOff(uint32 zoneid);
        bool GetZoneBit(uint32 zoneid);
    protected:
        bool m_ZoneIDmap[MAXZONES];
};
extern ZoneMapper ZoneIDMap;
#endif
