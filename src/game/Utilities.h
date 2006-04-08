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

#ifndef MANGOS_UTILITIES_H
#define MANGOS_UTILITIES_H

#include "Object.h"
#include "Map.h"

#define VISIBILITY_RANGE    10000
#define SPIRIT_HEALER       5233

#define IN_LINE_OF_SIGHT         900

namespace MaNGOS
{
    namespace Utilities
    {

        inline float calculate_distance_square(const float &x1, const float &y1, const float &h1, const float &x2, const float &y2, const float &h2)
        {
            float x_p = (x2 - x1);
            float y_p = (y2 - y1);
            float h_p = (h2 - h1);
            x_p *= x_p;
            y_p *= y_p;
            h_p *= h_p;
            return (x_p + y_p + h_p);
        }

        inline bool is_in_range(Player *obj1, Object *obj2)
        {
            assert(obj1->GetMapId() == obj2->GetMapId());
            return (calculate_distance_square(obj1->GetPositionX(), obj1->GetPositionY(), obj1->GetPositionZ(), obj2->GetPositionX(), obj2->GetPositionY(), obj2->GetPositionZ()) < VISIBILITY_RANGE);
        }

        inline bool is_in_line_of_sight(const float &x1, const float &y1, const float &z1, const float &x2, const float &y2, const float &z2, const float off_set)
        {
            return( (calculate_distance_square(x1, y1, z1, x2, y2, z2)*off_set) <= IN_LINE_OF_SIGHT );
        }

        template<class T> inline bool IsSpiritHealer(T *obj)
        {
            return false;
        }

        template<> inline bool IsSpiritHealer(Creature *obj)
        {
            return ( obj->GetUInt32Value(UNIT_FIELD_DISPLAYID) == SPIRIT_HEALER );
        }
    }
}
#endif
