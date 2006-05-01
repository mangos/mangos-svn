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

#include "Common.h"
#include "Bag.h"
#include "ObjectMgr.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "WorldPacket.h"
#include "UpdateData.h"
#include "WorldSession.h"

Bag::Bag( ): Item()
{
    m_objectType |= TYPE_CONTAINER;
    m_objectTypeId = TYPEID_CONTAINER;

    m_valuesCount = CONTAINER_END;

    memset(m_bagslot, 0, sizeof(Item *) * (20));            // Maximum 20 Slots
}

Bag::~Bag()
{
    for(int i = 0; i<20; i++)
    {
        if(m_bagslot[i])    delete m_bagslot[i];
    }
}

void Bag::Create(uint32 guidlow, uint32 itemid, Player* owner)
{
    ItemPrototype *m_itemProto = objmgr.GetItemPrototype(itemid);

    ASSERT(m_itemProto);
    ASSERT(m_itemProto->ContainerSlots <= 20);

    Object::_Create( guidlow, HIGHGUID_CONTAINER );

    SetUInt32Value(OBJECT_FIELD_ENTRY, itemid);
    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);

    SetUInt64Value(ITEM_FIELD_OWNER, owner->GetGUID());
    SetUInt64Value(ITEM_FIELD_CONTAINED, owner->GetGUID());

    SetUInt32Value(ITEM_FIELD_MAXDURABILITY, m_itemProto->MaxDurability);
    SetUInt32Value(ITEM_FIELD_DURABILITY, m_itemProto->MaxDurability);
    SetUInt32Value(ITEM_FIELD_FLAGS, m_itemProto->Flags);
    SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);

    // Setting the number of Slots the Container has
    SetUInt32Value(CONTAINER_FIELD_NUM_SLOTS, m_itemProto->ContainerSlots);

    // Cleanning 20 slots
    for (uint8 i = 0; i < 20; i++)
    {
        SetUInt64Value(CONTAINER_FIELD_SLOT_1 + (i*2), 0);
        m_bagslot[i] = NULL;
    }

    m_owner = owner;
}

void Bag::SaveToDB()
{
	Item::SaveToDB();
    sDatabase.PExecute("DELETE FROM `character_inventory` WHERE `guid` = '%u' AND `bag` = '%u';", m_owner->GetGUIDLow(), GetSlot());
    for (uint8 i = 0; i < 20; i++)
    {
        if (m_bagslot[i])
        {
            sDatabase.PExecute("INSERT INTO `character_inventory`  (`guid`,`bag`,`slot`,`item`,`item_template`) VALUES ('%u', '%u', '%u', '%u', '%u');", m_owner->GetGUIDLow(), GetSlot(), i, m_bagslot[i]->GetGUIDLow(), m_bagslot[i]->GetEntry());
            m_bagslot[i]->SaveToDB();
        }
    }
}

bool Bag::LoadFromDB(uint32 guid, uint32 auctioncheck)
{
	if(!Item::LoadFromDB(guid, auctioncheck))
		return false;
    for (uint8 i = 0; i < 20; i++)
    {
        SetUInt64Value(CONTAINER_FIELD_SLOT_1 + (i*2), 0);
        if (m_bagslot[i])
        {
            delete m_bagslot[i];
            m_bagslot[i] = NULL;
        }
    }
    QueryResult *result = sDatabase.PQuery("SELECT * FROM `character_inventory` WHERE `guid` = '%u' AND `bag` = '%u';", m_owner->GetGUIDLow(), GetSlot());

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint8  slot      = fields[2].GetUInt8();
            uint32 item_guid = fields[3].GetUInt32();
            uint32 item_id   = fields[4].GetUInt32();

            ItemPrototype* proto = objmgr.GetItemPrototype(item_id);

            Item *item = NewItemOrBag(proto);
            item->SetOwner(m_owner);
            item->SetSlot(slot);
            if(!item->LoadFromDB(item_guid, 1))
				continue;
            AddItemToBag(slot, item);
        } while (result->NextRow());

        delete result;
    }
	return true;
}

void Bag::DeleteFromDB()
{
    for (int i = 0; i < 20; i++)
    {
        if (m_bagslot[i])
        {
            m_bagslot[i]->DeleteFromDB();
        }
    }
}

uint8 Bag::FindFreeBagSlot()
{
    uint32 ContainerSlots=GetProto()->ContainerSlots;
    for (uint8 i=0; i <ContainerSlots; i++)
        if (!m_bagslot[i])
            return i;

    return NULL_SLOT;
}

Item* Bag::RemoveItemFromBag(uint8 slot)
{
    Item *pItem = m_bagslot[slot];
    if(m_bagslot[slot])
        m_bagslot[slot] = NULL;

    SetUInt64Value( CONTAINER_FIELD_SLOT_1 + (slot * 2), 0 );
    pItem->SetSlot( 0 );
    return pItem;
}

uint32 Bag::RemoveItemFromBag(uint8 slot,uint32 count)
{
    Item *pItem = m_bagslot[slot];
    int oldcnt=0,removed=0;
    if(m_bagslot[slot])
    {
        if((oldcnt=pItem->GetCount())>count)
        {
            m_bagslot[slot]->SetCount(oldcnt-count);
            removed=count;
        }
        else
        {
            m_bagslot[slot] = NULL;
            removed=oldcnt;
        }
    }
    if(removed>=oldcnt)
        SetUInt64Value( CONTAINER_FIELD_SLOT_1 + (slot * 2), 0 );

    return removed;
}

void Bag::BuildCreateUpdateBlockForPlayer( UpdateData *data, Player *target ) const
{
    Item::BuildCreateUpdateBlockForPlayer( data, target );

    for (int i = 0; i < 20; i++)
    {
        if(m_bagslot[i])
            m_bagslot[i]->BuildCreateUpdateBlockForPlayer( data, target );
    }
}

// If the bag is empty returns true
bool Bag::IsEmpty()
{
    uint32 ContainerSlots=GetProto()->ContainerSlots;
    for(uint32 i=0; i < ContainerSlots; i++)
        if (m_bagslot[i]) return false;

    return true;
}

uint8 Bag::GetSlotByItemGUID(uint64 guid)
{
    uint32 ContainerSlots=GetProto()->ContainerSlots;

    for(uint32 i=0;i<ContainerSlots;i++)
    {
        if(m_bagslot[i] != 0)
            if(m_bagslot[i]->GetGUID() == guid)
                return i;
    }
    return NULL_SLOT;
}

// Adds an item to a bag slot
// - slot can be NULL_SLOT, in that case function searchs for a free slot
// - Return values: 0 - item not added
//                  1 - item added to a free slot (and perhaps to a stack)
//                  2 - item added to a stack (item should be deleted)
uint8 Bag::AddItemToBag(uint8 slot, Item *item)
{
    if (!item) { return false; }

    UpdateData upd;
    WorldPacket packet;
    Item *pItem = 0;
    int stack = (item->GetProto()->MaxCount)?(item->GetProto()->MaxCount):1;
    int count = item->GetCount();
    uint8 addtoslot = NULL_SLOT;
    int freespace = 0;
    int freeslots = 0;

    if (slot != NULL_SLOT)
    {
        if (slot >= GetProto()->ContainerSlots)
        {
            return 0;
        }
        pItem = m_bagslot[slot];
        if (pItem)
        {
            sLog.outString("item exists");
            if (pItem->GetEntry() != item->GetEntry()) { return 0; }
            else
            {
                if ((stack - pItem->GetCount()) >= count)
                {
                    pItem->SetCount(((pItem->GetCount() + count) > stack)?stack:(pItem->GetCount() + count));
                    //pItem->SaveToDB();

                    pItem->SendUpdateToPlayer(m_owner);
                    return 2;
                }
                else
                {
                    return 0;
                }
            }
        }
        else
        {
            addtoslot = slot;
        }
    }
    else
    {
        // If slot is not specified, check for free slots (and stacks, if allowstack = true)
        // cycle 0 - searching for stack space
        // cycle 1 - searching for free slots
        // cycle 2 - adding to stacks
        // cycle 3 - adding to free slots
        for (int cycle=0; cycle <= 3; cycle++)
        {
            if ((cycle > 1) && (freespace < count) && (!freeslots)) { return 0; }
            for (uint8 slot2=0; slot2 < GetProto()->ContainerSlots; slot2++)
            {
                pItem = GetItemFromBag(slot2);
                if (!pItem)
                {
                    if (cycle == 1)
                    {
                        freeslots++;
                    }
                    else if (cycle == 3)
                    {
                        if (addtoslot == NULL_SLOT) { addtoslot = slot2; }
                    }
                }
                else
                {
                    if ((pItem->GetEntry() == item->GetEntry()) && ((pItem->GetCount() < stack) && (count)))
                    {
                        if (cycle == 0)
                        {
                            freespace += (stack - pItem->GetCount());
                        }
                        else if (cycle == 2)
                        {
                            int plus = count;
                            if ((pItem->GetCount() + count) > stack) { plus = stack - pItem->GetCount(); }
                            count -= plus;
                            pItem->SetCount(pItem->GetCount() + plus);
                            //pItem->SaveToDB();
                            pItem->SendUpdateToPlayer(m_owner);
                            if (!count) { return 2; }
                        }
                    }
                }
            }
        }
    }

    item->SetCount(count);
    item->SetSlot( addtoslot );

    m_bagslot[addtoslot] = item;
    SetUInt64Value(CONTAINER_FIELD_SLOT_1 + (addtoslot * 2), item->GetGUID());
    item->SetUInt64Value(ITEM_FIELD_CONTAINED, GetGUID());
    item->SendUpdateToPlayer(m_owner);

    return 1;
}
