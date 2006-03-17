/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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
#include "MapManager.h"


GameObject::GameObject() : Object()
{
    m_objectType |= TYPE_GAMEOBJECT;
    m_objectTypeId = TYPEID_GAMEOBJECT;

    m_valuesCount = GAMEOBJECT_END;
    m_RespawnTimer = 0;
}

void GameObject::Create(uint32 guidlow, uint32 name_id, uint32 mapid, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3)
{
    const GameObjectInfo *info = objmgr.GetGameObjectInfo(name_id);
    Object::_Create(guidlow, HIGHGUID_GAMEOBJECT, mapid, x, y, z, ang, name_id);
    SetUInt32Value(GAMEOBJECT_TIMESTAMP, (uint32)time(NULL));
    SetFloatValue(GAMEOBJECT_POS_X, x);
    SetFloatValue(GAMEOBJECT_POS_Y, y);
    SetFloatValue(GAMEOBJECT_POS_Z, z);
    SetFloatValue(GAMEOBJECT_FACING, ang);
    SetFloatValue(OBJECT_FIELD_SCALE_X, info->size);
    
    SetUInt32Value(GAMEOBJECT_FACTION, info->faction);
    SetUInt32Value(GAMEOBJECT_FLAGS, info->flags);

	SetUInt32Value (OBJECT_FIELD_ENTRY, info->id);
	
	

	SetUInt32Value (GAMEOBJECT_DISPLAYID, info->displayId);

    
	SetUInt32Value (GAMEOBJECT_STATE, 1);
	SetUInt32Value (GAMEOBJECT_TYPE_ID, info->type);
    
    
	SetFloatValue (GAMEOBJECT_ROTATION, rotation0);
	SetFloatValue (GAMEOBJECT_ROTATION+1, rotation1);
	SetFloatValue (GAMEOBJECT_ROTATION+2, rotation2);
	SetFloatValue (GAMEOBJECT_ROTATION+3, rotation3);

	SetFloatValue (GAMEOBJECT_LEVEL, float(0));
	

	const LootMgr::LootList &loot_list(LootManager.getGameObjectsLootList(info->id));
    
	if (loot_list.size() > 0)
	    SetUInt32Value (GAMEOBJECT_DYN_FLAGS, UNIT_DYNFLAG_LOOTABLE);

	

}


void GameObject::Update(uint32 p_time)
{


    WorldPacket data;
    
    if(m_RespawnTimer > 0)
    {
        if(m_RespawnTimer > p_time)
            m_RespawnTimer -= p_time;
        else
        {
            data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
            data << GetGUID();
            SendMessageToSet(&data,true);
            SetUInt32Value(GAMEOBJECT_STATE,1);
            m_RespawnTimer = 0;
        }
    }
}



void GameObject::_generateLoot(Player &player, std::vector<uint32> &item_id, std::vector<uint32> &item_count, std::vector<uint32> &display_ids, uint32 &gold) const
{

    
    
    gold = urand(6,20);
    
    
    const LootMgr::LootList &loot_list(LootManager.getGameObjectsLootList(GetUInt32Value(OBJECT_FIELD_ENTRY)));
    bool not_done = (loot_list.size());
    std::vector<short> indexes(loot_list.size());
    std::generate(indexes.begin(), indexes.end(), SequenceGen());
    sLog.outDebug("Object %u has %d items.", GetUInt32Value(OBJECT_FIELD_ENTRY), loot_list.size());
    
    while (not_done)
    {
    
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
    uint32 gold = urand(6,20);
	uint32 last_id = 0;

    if( GetUInt32Value(GAMEOBJECT_FACTION) == 94
		|| GetUInt32Value(GAMEOBJECT_TYPE_ID) == 2
		|| GetUInt32Value(GAMEOBJECT_TYPE_ID) == 3)
    {
		_generateLoot(player, item_id, item_count, display_ids, gold);
		
		if (item_id.size() > 0)
		{
			Log::getSingleton().outDebug (">> Loot Generator: Sending data ...");
			Log::getSingleton().outDebug (">> Loot Generator: Items to send: %d, Guid: %d", item_id.size(), GetGUID());

			*data << GetGUID();
			*data << uint8(0x01);
			*data << uint32(gold);                  
			*data << (uint8)item_id.size();        
    
			for(uint8 i = 0; i < item_id.size(); i++)
			{
				if (item_id[i] != last_id) 
				{
					*data << uint8(i+1);  
					*data << uint32(item_id[i]);    
					last_id = item_id[i]; 
					*data << uint32(item_count[i]); 
					*data << uint32(display_ids[i]); 
					*data << uint32(0) << uint32(0) << uint8(0);

					
				}
			}
		}
		else
		{
			uint32 level = player.GetUInt32Value(UNIT_FIELD_LEVEL);
			int number_of_items = irand(0, 12);
			int tries = 0;
			uint32 loot_items_list[16];
			uint32 num_loot_items = 0;

			if (number_of_items > 4 && level < 5)
				number_of_items = irand(1, 4);

			if (number_of_items > 6 && level < 10)
				number_of_items = irand(1, 6);

			if (number_of_items > 8 && level < 20)
				number_of_items = irand(1, 8);

			if (number_of_items > 10 && level < 40)
				number_of_items = irand(1, 10);

			for(uint32 i = 0; i<number_of_items ; i++)
			{
				uint32 loot_item;

				if (num_item_prototypes > 32768)
					loot_item = irand(0, 32768) + irand(0, (num_item_prototypes-32768));
				else
					loot_item = irand(0, num_item_prototypes);

				ItemPrototype *tmpLootItem = objmgr.GetItemPrototype(item_proto_ids[loot_item]);

				while (!(tmpLootItem && tmpLootItem->DisplayInfoID) 
					|| tmpLootItem->ItemLevel > level*1.5 
					|| tmpLootItem->Field107 == -1) 
				{
	                if (num_item_prototypes > 32768)
		               loot_item = irand(0, 32768) + irand(0, (num_item_prototypes-32768));
			        else
				        loot_item = irand(0, (num_item_prototypes-32768));

					tmpLootItem = objmgr.GetItemPrototype(item_proto_ids[loot_item]);
					tries++;

					if (tries >= 50)
						break;
				}

				if (tries >= 50)
					break;

				loot_items_list[num_loot_items] = item_proto_ids[loot_item];

				num_loot_items++;
			}

			num_loot_items--;

			Log::getSingleton().outDebug (">> Loot Generator: Sending data ...");
			Log::getSingleton().outDebug (">> Loot Generator: Generated %u random loots.", num_loot_items);

			*data << GetGUID();
			*data << uint8(0x01);
			*data << uint32(gold);                  
			*data << uint8(num_loot_items);        

			for(uint8 i = 0; i < num_loot_items; i++)
			{
				if (loot_items_list[i] != last_id) 
				{
					ItemPrototype *tmpLootItem = objmgr.GetItemPrototype(loot_items_list[i]);

					*data << uint8(i+1);  
					*data << uint32(tmpLootItem->ItemId);    
					last_id = loot_items_list[i]; 
					*data << uint32(1); 
					*data << uint32(tmpLootItem->DisplayInfoID); 
					*data << uint32(0) << uint32(0) << uint8(0);

					
				}
			}
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
	float rotation0, rotation1, rotation2, rotation3;
    ss << "SELECT id,positionX,positionY,positionZ,orientation,zoneId,mapId,data,name_id,rotation0,rotation1,rotation2,rotation3 FROM gameobjects WHERE id=" << guid;

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

    rotation0 = fields[9].GetFloat();
    rotation1 = fields[10].GetFloat();
    rotation2 = fields[11].GetFloat();
    rotation3 = fields[12].GetFloat();

    LoadValues(fields[7].GetString());
    Create(id, name_id, map_id, x, y, z, ang, rotation0, rotation1, rotation2, rotation3);  
}


void GameObject::DeleteFromDB()
{
    char sql[256];

    sprintf(sql, "DELETE FROM gameobjects WHERE id=%u", GetGUIDLow());
    sDatabase.Execute(sql);
}
