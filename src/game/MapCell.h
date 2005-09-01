/* MapCell.h
 *
 * Copyright (C) 2004 Wow Daemon
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

#ifndef __MAP_CELL_H
#define __MAP_CELL_H

class MapCell
{
    public:
        typedef std::set<Object*> ObjectSet;

        void AddObject(Object *obj) { _objects.insert(obj); }
        void RemoveObject(Object *obj) { _objects.erase(obj); }
        bool HasObject(Object *obj) { return !(_objects.find(obj) == _objects.end()); }

        ObjectSet::iterator Begin() { return _objects.begin(); }
        ObjectSet::iterator End() { return _objects.end(); }

    private:
        ObjectSet _objects;
};
#endif
