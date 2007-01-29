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

#include "Common.h"
#include "ObjectMgr.h"
#include "Database/DatabaseEnv.h"
#include "Container.h"

Container::Container( )
{
    m_objectType |= TYPE_CONTAINER;
    m_objectTypeId = TYPEID_CONTAINER;

    m_valuesCount = CONTAINER_END;
}

void Container::Create( uint32 guidlow, uint32 itemid, Player *owner )
{
    Object::_Create( guidlow, HIGHGUID_CONTAINER );

    ItemPrototype const *m_itemProto = objmgr.GetItemPrototype( itemid );
    ASSERT(m_itemProto);

    uint32 ContainerSlots =m_itemProto->ContainerSlots;

    SetUInt32Value( OBJECT_FIELD_ENTRY, itemid );
    SetFloatValue( OBJECT_FIELD_SCALE_X, 1.0f );
    SetUInt64Value( ITEM_FIELD_OWNER, owner->GetGUID() );
    SetUInt64Value( ITEM_FIELD_CONTAINED, owner->GetGUID() );
    SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
    SetUInt32Value( CONTAINER_FIELD_NUM_SLOTS,ContainerSlots);

    m_Slot = new Item*[ContainerSlots];
    memset(m_Slot, 0, sizeof(Item*)*ContainerSlots);

    m_owner = owner;
}

uint8 Container::FindFreeSlot()
{
    uint32 TotalSlots = GetUInt32Value( CONTAINER_FIELD_NUM_SLOTS );
    for (uint8 i=0; i < TotalSlots; i++)
    {
        if(!m_Slot[i])
        {
            return i;
        }
        else
        {
        }
    }
    return (uint8)-1;
}

void Container::AddItem(uint8 slot, Item *item)
{
    ASSERT(m_Slot[slot] == NULL);

    m_Slot[slot] = item;
    SetUInt64Value( (uint16)(CONTAINER_FIELD_SLOT_1  + (slot*2)), item->GetGUID() );
}

ItemPrototype const * Container:: GetProto()
{
    return  objmgr.GetItemPrototype(GetUInt32Value( OBJECT_FIELD_ENTRY ));
}
