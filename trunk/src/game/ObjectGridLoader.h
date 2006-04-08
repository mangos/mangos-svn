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

#ifndef MANGOS_OBJECTGRIDLOADER_H
#define MANGOS_OBJECTGRIDLOADER_H

#include "Utilities/TypeList.h"
#include "Platform/Define.h"
#include "GameSystem/GridLoader.h"
#include "GridDefines.h"
#include "Cell.h"

class MANGOS_DLL_DECL ObjectGridLoader
{
    public:
        ObjectGridLoader(NGridType &grid, uint32 id, const Cell &cell)
            : i_cell(cell), i_grid(grid), i_mapId(id), i_gameObjects(0), i_creatures (0)
            {}

        void Load(GridType &grid);
        void Visit(std::map<OBJECT_HANDLE, GameObject *> &m);
        void Visit(std::map<OBJECT_HANDLE, Creature *> &m);

        void Visit(std::map<OBJECT_HANDLE, Corpse *> &m)
        {
        }

        void Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m)
        {

        }

        void LoadN(void)
        {
            i_gameObjects = 0; i_creatures = 0;
            i_cell.data.Part.cell_y = 0;
            for(unsigned int x=0; x < MAX_NUMBER_OF_CELLS; ++x)
            {
                i_cell.data.Part.cell_x = x;
                for(unsigned int y=0; y < MAX_NUMBER_OF_CELLS; ++y)
                {
                    i_cell.data.Part.cell_y = y;
                    GridLoader<Player, AllObjectTypes> loader;
                    loader.Load(i_grid(x, y), *this);
                }
            }
            sLog.outDebug("%d GameObjects and %d Creatures loaded for grid %d on map %d", i_gameObjects, i_creatures, i_grid.GetGridId(), i_mapId);
        }

    private:
        Cell i_cell;
        NGridType &i_grid;
        uint32 i_mapId;
        uint32 i_gameObjects;
        uint32 i_creatures;
};

class MANGOS_DLL_DECL ObjectGridUnloader
{
    public:
        ObjectGridUnloader(NGridType &grid) : i_grid(grid) {}

        void UnloadN(void)
        {
            for(unsigned int x=0; x < MAX_NUMBER_OF_CELLS; ++x)
            {
                for(unsigned int y=0; y < MAX_NUMBER_OF_CELLS; ++y)
                {
                    GridLoader<Player, AllObjectTypes> loader;
                    loader.Unload(i_grid(x, y), *this);
                }
            }
        }

        void Unload(GridType &grid);
        template<class T> void Visit(std::map<OBJECT_HANDLE, T *> &m);
    private:
        NGridType &i_grid;
};

typedef GridLoader<Player, AllObjectTypes> GridLoaderType;
#endif
