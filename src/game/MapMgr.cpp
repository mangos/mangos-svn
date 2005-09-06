/* MapMgr.cpp
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

#include "Common.h"
#include "Log.h"
#include "Object.h"
#include "Player.h"
#include "Item.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "MapCell.h"
#include "MapMgr.h"

MapMgr::MapMgr(uint32 mapId) : _mapId(mapId)
{
    _minX = -18000;
    _minY = -18000;
    _maxX = 18000;
    _maxY = 18000;

    _cellSize = 150;
    _sizeX = (abs(_minX) + _maxX)/_cellSize;
    _sizeY = (abs(_minY) + _maxY)/_cellSize;

    _cells = new MapCell*[_sizeX];
    ASSERT(_cells);
    for (uint32 i = 0; i < _sizeX; i++)
    {
        _cells[i] = new MapCell[_sizeY];
        ASSERT(_cells[i]);
    }

    _timers[MMUPDATE_FIELDS].SetInterval(100);
    _timers[MMUPDATE_OBJECTS].SetInterval(100);
}


MapMgr::~MapMgr()
{
    if(_cells)
    {
        for (uint32 i = 0; i < _sizeX; i++)
        {
            delete [] _cells[i];
        }

        delete _cells;
    }
}


void MapMgr::AddObject(Object *obj)
{
    ASSERT(obj);
    // make sure object is a virgin
    ASSERT(obj->GetInRangeSetBegin() == obj->GetInRangeSetEnd());
    ASSERT(obj->GetMapId() == _mapId);
    ASSERT(obj->GetPositionX() < _maxX && obj->GetPositionX() > _minX);
    ASSERT(obj->GetPositionY() < _maxY && obj->GetPositionY() > _minY);
    ASSERT(_cells);

    sLog.outDetail("Adding object "I64FMT" with type %i to the map %u.",
        obj->GetGUID(), obj->GetTypeId(), _mapId);

    // That object types are not map objects. TODO: add AI groups here?
    if(obj->GetTypeId() == TYPEID_ITEM || obj->GetTypeId() == TYPEID_CONTAINER)
    {
    // mark object as updatable and exit
        obj->AddToWorld();
        return;
    }

    uint32 x = (uint32)(obj->GetPositionX() > 0 ? abs(_minX) + obj->GetPositionX() :
    abs(_minX) - abs(obj->GetPositionX()));
    uint32 y = (uint32)(obj->GetPositionY() > 0 ? abs(_minY) + obj->GetPositionY() :
    abs(_minY) - abs(obj->GetPositionY()));
    x /= _sizeX;
    y /= _sizeY;

/*
    sLog.outDetail("Obj position: %f %f Cell position: %u %u",
    obj->GetPositionX(), obj->GetPositionY(), x, y);
*/

    MapCell *objCell = &(_cells[x][y]);

    uint32 endX = x < _sizeX ? x + 1 : _sizeX;
    uint32 endY = y < _sizeY ? y + 1 : _sizeY;
    uint32 startX = x > 0 ? x - 1 : 0;
    uint32 startY = y > 0 ? y - 1 : 0;
    uint32 posX, posY;
    MapCell *cell;
    MapCell::ObjectSet::iterator iter;

    WorldPacket packet;
    UpdateData data;
    UpdateData playerData;

    for (posX = startX; posX <= endX; posX++ )
    {
        for (posY = startY; posY <= endY; posY++ )
        {
            cell = &(_cells[posX][posY]);
            ASSERT(cell);

            for (iter = cell->Begin(); iter != cell->End(); iter++)
            {
                if ((*iter)->GetDistance2dSq(obj) <= UPDATE_DISTANCE*UPDATE_DISTANCE)
                {
                    // Object in range, add to set
                    if((*iter)->GetTypeId() == TYPEID_PLAYER)
                    {
                        sLog.outDetail("Creating object "I64FMT" for player "I64FMT".",
                            obj->GetGUID(), (*iter)->GetGUID());

                        data.Clear();
                        obj->BuildCreateUpdateBlockForPlayer( &data, (Player*)*iter );
                        data.BuildPacket(&packet);

                        ((Player*)*iter)->GetSession()->SendPacket( &packet );
                    }

                    (*iter)->AddInRangeObject(obj);

                    if(obj->GetTypeId() == TYPEID_PLAYER)
                    {
                        sLog.outDetail("Creating object "I64FMT" for player "I64FMT".",
                            (*iter)->GetGUID(), obj->GetGUID());

                        (*iter)->BuildCreateUpdateBlockForPlayer( &playerData, (Player*)obj );
                    }

                    obj->AddInRangeObject(*iter);
                }
            }
        }
    }

    if(obj->GetTypeId() == TYPEID_PLAYER)
    {
        sLog.outDetail("Creating player "I64FMT" for himself.", obj->GetGUID());
        obj->BuildCreateUpdateBlockForPlayer( &playerData, (Player*)obj );

        playerData.BuildPacket( &packet );
        ((Player*)obj)->GetSession()->SendPacket( &packet );
    }

    objCell->AddObject(obj);

    _objects[obj->GetGUID()] = obj;

    obj->SetMapCell(objCell);
    obj->AddToWorld();
}


void MapMgr::RemoveObject(Object *obj)
{
    ASSERT(obj);
    ASSERT(obj->GetMapId() == _mapId);
    ASSERT(obj->GetPositionX() > _minX && obj->GetPositionX() < _maxX);
    ASSERT(obj->GetPositionY() > _minY && obj->GetPositionY() < _maxY);
    ASSERT(_cells);

    sLog.outDetail("Removing object "I64FMT" with type %i from the world.",
        obj->GetGUID(), obj->GetTypeId());

    // That object types are not map objects. TODO: add AI groups here?
    if(obj->GetTypeId() == TYPEID_ITEM || obj->GetTypeId() == TYPEID_CONTAINER)
    {
    // remove updatable flag and exit
        obj->RemoveFromWorld();
        return;
    }

    obj->RemoveFromWorld();

    ObjectMap::iterator itr = _objects.find(obj->GetGUID());
    _objects.erase(itr);

    // remove us from updated objects list
    ObjectSet::iterator updi = _updatedObjects.find(obj);
    if(updi != _updatedObjects.end())
        _updatedObjects.erase(updi);

    MapCell *objCell = obj->GetMapCell();

    obj->SetMapCell(0);
    objCell->RemoveObject(obj);

    for (Object::InRangeSet::iterator iter = obj->GetInRangeSetBegin();
        iter != obj->GetInRangeSetEnd(); iter++)
    {
        (*iter)->RemoveInRangeObject(obj);

        if((*iter)->GetTypeId() == TYPEID_PLAYER)
            obj->DestroyForPlayer( (Player*)*iter );
    }

    obj->ClearInRangeSet();
}


void MapMgr::ChangeObjectLocation(Object *obj)
{
    ASSERT(obj);
    ASSERT(obj->GetMapId() == _mapId);
    ASSERT(_cells);

    if(obj->GetTypeId() == TYPEID_ITEM || obj->GetTypeId() == TYPEID_CONTAINER)
        return;

    WorldPacket packet;
    UpdateData data;
    UpdateData playerData;

    Object* curObj;

    for (Object::InRangeSet::iterator iter = obj->GetInRangeSetBegin();
        iter != obj->GetInRangeSetEnd();)
    {
        curObj = *iter;
        iter++;

        if (curObj->GetDistance2dSq(obj) > UPDATE_DISTANCE*UPDATE_DISTANCE)
        {
            sLog.outDetail("Object "I64FMT" no longer in field of view of object "I64FMT".",
                obj->GetGUID(), (curObj)->GetGUID());

            if( obj->GetTypeId() == TYPEID_PLAYER )
                curObj->BuildOutOfRangeUpdateBlock( &playerData );

            obj->RemoveInRangeObject(curObj);

            if( curObj->GetTypeId() == TYPEID_PLAYER )
            {
                data.Clear();
                obj->BuildOutOfRangeUpdateBlock( &data );
                data.BuildPacket(&packet);
                ((Player*)curObj)->GetSession()->SendPacket( &packet );
            }

            curObj->RemoveInRangeObject(obj);
        }
    }

    uint32 cellX = (uint32)(obj->GetPositionX() > 0 ? abs(_minX) + obj->GetPositionX() :
    abs(_minX) - abs(obj->GetPositionX()));
    uint32 cellY = (uint32)(obj->GetPositionY() > 0 ? abs(_minY) + obj->GetPositionY() :
    abs(_minY) - abs(obj->GetPositionY()));
    cellX /= _sizeX;
    cellY /= _sizeY;

/*
    sLog.outDetail("Obj position: %f %f Cell position: %u %u",
    obj->GetPositionX(), obj->GetPositionY(), cellX, cellY);
*/

    MapCell *objCell = &(_cells[cellX][cellY]);

    if (objCell != obj->GetMapCell())
    {
        obj->GetMapCell()->RemoveObject(obj);
        objCell->AddObject(obj);
        obj->SetMapCell(objCell);
    }

    uint32 endX = cellX < _sizeX ? cellX + 1 : _sizeX;
    uint32 endY = cellY < _sizeY ? cellY + 1 : _sizeY;
    uint32 startX = cellX > 0 ? cellX - 1 : 0;
    uint32 startY = cellY > 0 ? cellY - 1 : 0;
    uint32 posX, posY;
    MapCell *cell;
    MapCell::ObjectSet::iterator iter;

    for (posX = startX; posX <= endX; posX++ )
    {
        for (posY = startY; posY <= endY; posY++ )
        {
            cell = &(_cells[posX][posY]);
            ASSERT(cell);

            for (iter = cell->Begin(); iter != cell->End(); iter++)
            {
                curObj = *iter;
                if (curObj != obj &&
                    (curObj)->GetDistance2dSq(obj) <= UPDATE_DISTANCE*UPDATE_DISTANCE &&
                    !obj->IsInRangeSet(curObj))
                {
                    // Object in range, add to set
                    if((curObj)->GetTypeId() == TYPEID_PLAYER)
                    {
                        sLog.outDetail("Creating object "I64FMT" for player "I64FMT".",
                            obj->GetGUID(), (curObj)->GetGUID());

                        data.Clear();
                        obj->BuildCreateUpdateBlockForPlayer( &data, (Player*)curObj );
                        data.BuildPacket(&packet);

                        ((Player*)curObj)->GetSession()->SendPacket( &packet );
                    }

                    (curObj)->AddInRangeObject(obj);

                    if(obj->GetTypeId() == TYPEID_PLAYER)
                    {
                        sLog.outDetail("Creating object "I64FMT" for player "I64FMT".",
                            (curObj)->GetGUID(), obj->GetGUID());

                        (curObj)->BuildCreateUpdateBlockForPlayer( &playerData, (Player*)obj );
                    }

                    obj->AddInRangeObject(curObj);
                }
            }
        }
    }

    if (obj->GetTypeId() == TYPEID_PLAYER)
    {
        playerData.BuildPacket(&packet);
        ((Player*)obj)->GetSession()->SendPacket( &packet );
    }
}


void MapMgr::_UpdateObjects()
{
    UpdateData *data;
    WorldPacket packet;
    HM_NAMESPACE::hash_map<Player*, UpdateData*> updates;
    HM_NAMESPACE::hash_map<Player*, UpdateData*>::iterator i;

    ObjectSet::iterator iobj, iobjend;

    for ( iobj = _updatedObjects.begin(), iobjend = _updatedObjects.end();
        iobj != iobjend; iobj++ )
    {
        if( (*iobj)->GetTypeId() == TYPEID_PLAYER )
        {
            i = updates.find( (Player*)*iobj );
            if(i == updates.end())
            {
                data = new UpdateData;
                ASSERT(data);

                updates[(Player*)*iobj] = data;
            }
            else
                data = i->second;

            (*iobj)->BuildValuesUpdateBlockForPlayer( data, (Player*)*iobj );
        }

        if( (*iobj)->GetTypeId() == TYPEID_ITEM ||  (*iobj)->GetTypeId() == TYPEID_CONTAINER )
        {
            i = updates.find( ((Item*)*iobj)->GetOwner() );
            if(i == updates.end())
            {
                data = new UpdateData;
                ASSERT(data);

                updates[((Item*)*iobj)->GetOwner()] = data;
            }
            else
                data = i->second;

            (*iobj)->BuildValuesUpdateBlockForPlayer( data, ((Item*)*iobj)->GetOwner());
        }

        for( Object::InRangeSet::iterator iplr = (*iobj)->GetInRangeSetBegin();
            iplr != (*iobj)->GetInRangeSetEnd(); iplr++)
        {
            if( (*iplr)->GetTypeId() == TYPEID_PLAYER )
            {
                Log::getSingleton( ).outDetail("Sending updater to player %u",
                    (*iplr)->GetGUIDLow());

                // TODO: take ranges into accounts as suggested by quetzal

                i = updates.find( (Player*)*iplr );
                if(i == updates.end())
                {
                    data = new UpdateData;
                    ASSERT(data);

                    updates[(Player*)*iplr] = data;
                }
                else
                    data = i->second;

                (*iobj)->BuildValuesUpdateBlockForPlayer( data, (Player*)*iplr );
            }
        }

        (*iobj)->ClearUpdateMask();
    }

    for ( i = updates.begin(); i != updates.end(); i++ )
    {
        i->second->BuildPacket( &packet );
        ((Player*)(i->first))->GetSession()->SendPacket( &packet );

        delete i->second;
    }

    updates.clear();
    _updatedObjects.clear();
}


void MapMgr::Update(time_t diff)
{
    for(int i = 0; i < MMUPDATE_COUNT; i++)
        _timers[i].Update(diff);

/*
    if (_timers[MMUPDATE_OBJECTS].Passed())
    {
        _timers[MMUPDATE_OBJECTS].Reset();

        ObjectMgr::PlayerMap::iterator chriter;
        ObjectMgr::CreatureMap::iterator iter;

        for( chriter = objmgr.Begin<Player>(); chriter != objmgr.End<Player>( ); ++ chriter )
            chriter->second->Update( fTime );

        for( iter = objmgr.Begin<Creature>(); iter != objmgr.End<Creature>(); ++ iter )
            iter->second->Update( fTime );
    }
*/

    if (_timers[MMUPDATE_FIELDS].Passed())
    {
        _timers[MMUPDATE_FIELDS].Reset();

        _UpdateObjects();
    }
}
