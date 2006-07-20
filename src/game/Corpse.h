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

#ifndef MANGOSSERVER_CORPSE_H
#define MANGOSSERVER_CORPSE_H

#include "Object.h"

class Corpse : public Object
{
    public:
        Corpse( );

        bool Create( uint32 guidlow );
        bool Create( uint32 guidlow, Player *owner, uint32 mapid, float x, float y, float z, float ang );

        void SaveToDB(bool bones = false);
};
#endif
