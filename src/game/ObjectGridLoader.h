/* ObjectGridLoader.h
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

#ifndef MANGOS_OBJECTGRIDLOADER_H
#define MANGOS_OBJECTGRIDLOADER_H

#include "Utilities/TypeList.h"
#include "Platform/Define.h"
#include "GameSystem/Grid.h"
#include "GameSystem/GridLoader.h"
#include "Player.h"
#include "GameObject.h"
#include "Creature.h"
#include "DynamicObject.h"
#include "Corpse.h"


typedef TYPELIST_4(GameObject, Creature, DynamicObject, Corpse)    AllObjectTypes;

/*
 * @class ObjectGridLoader class implements a visitor pattern for the ContainerMapList
 * because that's the container used for storing both GameObjects and Creatures
 * which is in the grid.
 */

typedef Grid<Player, AllObjectTypes> GridType;

class MANGOS_DLL_DECL ObjectGridLoader
{
public:
    ObjectGridLoader(GridType &grid, Player &pl) : i_grid(grid), i_player(pl) {}

    void Load(GridType &grid);
    void Visit(std::map<OBJECT_HANDLE, GameObject *> &m);
    void Visit(std::map<OBJECT_HANDLE, Creature *> &m);


    void Visit(std::map<OBJECT_HANDLE, Corpse *> &m)
    {
	/* we don't load in corpses */
    }

    void Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m)
    {
	/* we don't load in dynamic objects.. we add it in dynamically */
    }

private:
    GridType &i_grid;
    Player &i_player;
};

/*
 * @class ObjectGridUnloader also implements the visitor pattern
 * for unloading the grid.
 */
class MANGOS_DLL_DECL ObjectGridUnloader
{
public:
    ObjectGridUnloader(GridType &grid) : i_grid(grid) {}

    void Unload(GridType &grid);
    void Visit(std::map<OBJECT_HANDLE, GameObject *> &m);
    void Visit(std::map<OBJECT_HANDLE, Creature *> &m);
    void Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m);
    void Visit(std::map<OBJECT_HANDLE, Corpse *> &m);

private:
    GridType &i_grid;
};

typedef GridLoader<Player, AllObjectTypes> GridLoaderType;
typedef std::map<OBJECT_HANDLE, Player* > PlayerMapType;
typedef std::map<OBJECT_HANDLE, Creature* > CreatureMapType;
typedef std::map<OBJECT_HANDLE, GameObject* > GameObjectMapType;
typedef std::map<OBJECT_HANDLE, DynamicObject* > DynamicObjectMapType;
typedef std::map<OBJECT_HANDLE, Corpse* > CorpseMapType;

#endif
