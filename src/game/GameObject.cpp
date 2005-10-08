/* GameObject.cpp
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
#include "GameObject.h"
#include "ObjectMgr.h"
#include "UpdateMask.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "LootMgr.h"
#include "Database/DatabaseEnv.h"

#ifdef ENABLE_GRID_SYSTEM
#include "MapManager.h"
#endif

GameObject::GameObject() : Object()
{
    m_objectType |= TYPE_GAMEOBJECT;
    m_objectTypeId = TYPEID_GAMEOBJECT;

    m_valuesCount = GAMEOBJECT_END;
    m_RespawnTimer = 0;
}

void GameObject::Create(uint32 guidlow, uint32 name_id, uint32 mapid, float x, float y, float z, float ang)
{
    const GameObjectInfo &info(*(objmgr.GetGameObjectInfo(name_id)));
    Object::_Create(guidlow, HIGHGUID_GAMEOBJECT, mapid, x, y, z, ang);
    SetUInt32Value(GAMEOBJECT_TIMESTAMP, (uint32)time(NULL));
    SetFloatValue(GAMEOBJECT_POS_X, x);
    SetFloatValue(GAMEOBJECT_POS_Y, y);
    SetFloatValue(GAMEOBJECT_POS_Z, z);
    SetFloatValue(GAMEOBJECT_FACING, ang);
    SetFloatValue(OBJECT_FIELD_SCALE_X, info.size);
    SetUInt32Value(GAMEOBJECT_STATE, 1);
    SetUInt32Value(GAMEOBJECT_FACTION, info.faction);
    SetUInt32Value(GAMEOBJECT_FLAGS, info.flags);
}


void GameObject::Update(uint32 p_time)
{
	if (m_nextThinkTime > time(NULL))
		return; // Think once every 5 secs only for GameObject updates...

	m_nextThinkTime = time(NULL) + 5;

    WorldPacket data;
    // Respawn Timer
    if(m_RespawnTimer > 0)
    {
        if(m_RespawnTimer > p_time)
            m_RespawnTimer -= p_time;
        else
        {
            if(!this->IsInWorld())
            {
#ifndef ENABLE_GRID_SYSTEM
                PlaceOnMap();
#endif
            }

            data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
            data << GetGUID();
            SendMessageToSet(&data,true);
            SetUInt32Value(GAMEOBJECT_STATE,1);
            m_RespawnTimer = 0;
        }
    }
}

#ifndef ENABLE_GRID_SYSTEM
void GameObject::Despawn(uint32 time)
{
    WorldPacket data;
    RemoveFromMap();

    data.Initialize(SMSG_GAMEOBJECT_DESPAWN_ANIM);
    data << GetGUID();
    SendMessageToSet(&data,true);

    m_RespawnTimer = time;
}
#endif

void
GameObject::_generateLoot(Player &player, std::vector<uint32> &item_id, std::vector<uint32> &item_count, std::vector<uint32> &display_ids, uint32 &gold) const
{

    // this is not ready yet.. still need to solve the data first
    return;
    gold = 0;
    
    // Generate max value
    const LootMgr::LootList &loot_list(LootMgr::getSingleton().getGameObjectsLootList(GetUInt32Value(OBJECT_FIELD_ENTRY)));
    bool not_done = (loot_list.size());
    std::vector<short> indexes(loot_list.size());
    std::generate(indexes.begin(), indexes.end(), SequenceGen());
    sLog.outDebug("Number of items to get %d.", loot_list.size());
    
    while (not_done)
    {
	// generate the item you need to pick
	int idx = rand()%indexes.size();
	const LootItem &item(loot_list[indexes[idx]]);
	indexes.erase(indexes.begin()+idx);
	ItemPrototype *pCurItem = objmgr.GetItemPrototype(item.itemid);
	
	if( pCurItem != NULL && item.chance >= (rand()%100) )
	{
	    item_id.push_back(item.itemid);
	    item_count.push_back(1);
	    display_ids.push_back(pCurItem->DisplayInfoID);
	}
	
	not_done = indexes.size();
    }
}

bool GameObject::FillLoot(Player &player, WorldPacket *data)
{
    std::vector<uint32> item_id, item_count, display_ids;
    uint32 gold;

    if( GetUInt32Value(GAMEOBJECT_FACTION) == 94 )
    {
	_generateLoot(player, item_id, item_count, display_ids, gold);
	*data << GUID_LOPART(this->GetGUID());
	*data << uint8(0x01);
	*data << uint32(gold);                  // Loot Money
	*data << (uint8)item_id.size();        // item Count
	
	for(uint8 i = 0; i < item_id.size(); i++)
	{
	    *data << uint8(i+1);  // slot, must be greater than zero
	    *data << (uint32)item_id[i];    // item id
	    *data << uint32(item_count[i]); // quantity
	    *data << uint32(display_ids[i]); // iconid
	    *data << uint32(0) << uint32(0) << uint8(0);
	}
	
	return true;
    }

    return false;
}

void GameObject::SaveToDB()
{
    std::stringstream ss;
    ss << "DELETE FROM gameobjects WHERE id=" << GetGUIDLow();
    sDatabase.Execute(ss.str().c_str());

    ss.rdbuf()->str("");
    ss << "INSERT INTO gameobjects (id, zoneid, mapId, positionX, positionY, positionZ, orientation, data) VALUES ( "
        << GetGUIDLow() << ", "
        << GetZoneId() << ", "
        << GetMapId() << ", "
        << m_positionX << ", "
        << m_positionY << ", "
        << m_positionZ << ", "
        << m_orientation << ", '";

    for( uint16 index = 0; index < m_valuesCount; index ++ )
        ss << GetUInt32Value(index) << " ";

    ss << "' )";

    sDatabase.Execute( ss.str( ).c_str( ) );
}


void GameObject::LoadFromDB(uint32 guid)
{
    std::stringstream ss;
    ss << "SELECT id,positionX,positionY,positionZ,orientation,zoneId,mapId,data,name_id FROM gameobjects WHERE id=" << guid;

    std::auto_ptr<QueryResult> result(sDatabase.Query( ss.str().c_str() ));

    if( result.get() ==  NULL)
	return;

    Field *fields = result->Fetch();
    uint32 id= fields[0].GetUInt32();
    float x = fields[1].GetFloat();
    float y = fields[2].GetFloat();
    float z = fields[3].GetFloat();
    float ang = fields[4].GetFloat();
    m_zoneId = fields[5].GetUInt32();
    uint32 map_id = fields[6].GetUInt32();
    uint32 name_id = fields[8].GetUInt32();

    LoadValues(fields[7].GetString());
    Create(id, name_id, map_id, x, y, z, ang);       
}


void GameObject::DeleteFromDB()
{
    char sql[256];

    sprintf(sql, "DELETE FROM gameobjects WHERE id=%u", GetGUIDLow());
    sDatabase.Execute(sql);
}
