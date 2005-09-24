/* ZoneSearch.h
 *
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

#ifndef MANGOS_ZONESEARCH_H
#define MANGOS_ZONESEARCH_H

#include "ZoneDefine.h"

template<class T> struct ZoneSearcher
{
  /* force not to work if T is not zone_t */
};

template<zone_t T> struct ZoneSearcher<ZoneHolder<T> >
{
  static Zone* Create(const float x, const float y)
  {
    if( ZoneDefinition<T>::InZone(x, y) )
      return ZoneDefinition<T>::Create();
    return NULL;
  }
};

template<> struct ZoneSearcher<TypeNull>
{
  /* creates an unknown zone to capture all coordinates that's unknown */
  static Zone* Create(const float x, const float y)
  {
    return (new Zone);
  }
};

// recursion
template<class H, class T> struct ZoneSearcher<TypeList<H, T> >
{
  static Zone* Create(const float x, const float y)
  {
    Zone *z = ZoneSearcher<H>::Create(x, y);
    return( z == NULL ? ZoneSearcher<T>::Create(x, y) : z);
  }
};

#endif
