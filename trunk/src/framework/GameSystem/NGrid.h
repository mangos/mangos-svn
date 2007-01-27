/* 
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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

#ifndef MANGOS_NGRID_H
#define MANGOS_NGRID_H

/** NGrid is nothing more than a wrapper of the Grid with an NxN cells
 */

#include "GameSystem/Grid.h"

typedef enum
{
    GRID_STATE_INVALID = 0,
    GRID_STATE_ACTIVE = 1,
    GRID_STATE_IDLE = 2,
    GRID_STATE_REMOVAL= 3,
    MAX_GRID_STATE = 4
} grid_state_t;

template
<
unsigned int N,
class OBJECT,
class OBJECT_TYPES,
class ThreadModel = MaNGOS::SingleThreaded<OBJECT>
>
class MANGOS_DLL_DECL NGrid
{
    public:

        typedef Grid<OBJECT, OBJECT_TYPES, ThreadModel> GridType;
        NGrid(uint32 id) : i_gridId(id), i_cellstate(GRID_STATE_INVALID) {}

        const GridType& operator()(unsigned short x, unsigned short y) const { return i_cells[x][y]; }
        GridType& operator()(unsigned short x, unsigned short y) { return i_cells[x][y]; }

        inline const uint32& GetGridId(void) const { return i_gridId; }
        inline void SetGridId(const uint32 id) const { i_gridId = id; }
        inline grid_state_t GetGridState(void) const { return i_cellstate; }
        inline void SetGridState(grid_state_t s) { i_cellstate = s; }

        void AddObject(const uint32 x, const uint32 y, OBJECT *obj, OBJECT_HANDLE hdl)
        {
            i_cells[x][y].AddObject(obj, hdl);
        }

        void RemoveObject(const uint32 x, const uint32 y, OBJECT *obj, OBJECT_HANDLE hdl)
        {
            i_cells[x][y].RemoveObject(obj, hdl);
        }

        template<class T> void Visit(TypeContainerVisitor<T, TypeMapContainer<OBJECT_TYPES> > &visitor)
        {
            for(unsigned int x=0; x < N; ++x)
                for(unsigned int y=0; y < N; ++y)
                    i_cells[x][y].VisitGridObjects(visitor);
        }

        template<class T> void Visit(const uint32 &x, const uint32 &y, TypeContainerVisitor<T, TypeMapContainer<OBJECT_TYPES> > &visitor)
        {
            i_cells[x][y].VisitGridObjects(visitor);
        }

        template<class T> void Visit(TypeContainerVisitor<T, ContainerMapList<OBJECT> > &visitor)
        {
            for(unsigned int x=0; x < N; ++x)
                for(unsigned int y=0; y < N; ++y)
                    i_cells[x][y].VisitObjects(visitor);
        }

        template<class T> void Visit(const uint32 &x, const uint32 &y, TypeContainerVisitor<T, ContainerMapList<OBJECT> > &visitor)
        {
            i_cells[x][y].VisitObjects(visitor);
        }

        unsigned int ObjectsInGrid(void) const
        {
            unsigned int count=0;
            for(unsigned int x=0; x < N; ++x)
                for(unsigned int y=0; y < N; ++y)
                    count += i_cells[x][y].ObjectsInGrid();
            return count;
        }

        template<class SPECIFIC_OBJECT> const SPECIFIC_OBJECT* GetGridObject(const uint32 x, const uint32 y, OBJECT_HANDLE hdl) const
        {
            return i_cells[x][y].template GetGridObject<SPECIFIC_OBJECT>(hdl);
        }

        template<class SPECIFIC_OBJECT> SPECIFIC_OBJECT* GetGridObject(const uint32 x, const uint32 y, OBJECT_HANDLE hdl)
        {
            return i_cells[x][y].template GetGridObject<SPECIFIC_OBJECT>(hdl);
        }

        template<class SPECIFIC_OBJECT> bool AddGridObject(const uint32 x, const uint32 y, SPECIFIC_OBJECT *obj, OBJECT_HANDLE hdl)
        {
            return i_cells[x][y].template insert<SPECIFIC_OBJECT>(hdl, obj);
        }

        template<class SPECIFIC_OBJECT> bool RemoveGridObject(const uint32 x, const uint32 y, SPECIFIC_OBJECT *obj, OBJECT_HANDLE hdl)
        {
            return i_cells[x][y].template remove<SPECIFIC_OBJECT>(obj, hdl);
        }

    private:

        uint32 i_gridId;
        grid_state_t i_cellstate;
        GridType i_cells[N][N];
};
#endif
