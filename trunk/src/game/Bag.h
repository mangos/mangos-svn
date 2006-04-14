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

#ifndef MANGOS_BAG_H
#define MANGOS_BAG_H

#include "Object.h"
#include "ItemPrototype.h"
#include "Unit.h"
#include "Creature.h"
#include "Item.h"

class Bag : public Item
{
    public:

        Bag();
        ~Bag();

        void Create(uint32 guidlow, uint32 itemid, Player* owner);

        uint8 AddItemToBag(uint8 slot, Item *item);
        Item* RemoveItemFromBag(uint8 slot);
		uint32 RemoveItemFromBag(uint8 slot,uint32 count);

        Item* GetItemFromBag(uint8 slot) { return m_bagslot[slot]; }

        int8 FindFreeBagSlot();
        int8 GetSlotByItemGUID(uint64 guid);
        bool IsEmpty();

        // DB operations
        void SaveToDB();
        void LoadFromDB(uint32 guid, uint32 auctioncheck);
        void DeleteFromDB();

        void BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const;

    protected:

        // Bag Storage space
        Item* m_bagslot[20];
};
#endif
