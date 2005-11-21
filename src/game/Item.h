/* Item.h
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

#ifndef MANGOSSERVER_ITEM_H
#define MANGOSSERVER_ITEM_H

#include "Object.h"
#include "ItemPrototype.h"

enum InventoryChangeFailure 
{
	EQUIP_ERR_OK = 0,
	EQUIP_ERR_YOU_MUST_REACH_LEVEL_N,
	EQUIP_ERR_SKILL_ISNT_HIGH_ENOUGH,
	EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT,
	EQUIP_ERR_BAG_FULL,
	EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG,
	EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE,
	EQUIP_ERR_NO_REQUIRED_PROFICIENCY,
	EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE,
	EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM,
	EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM2,
	EQUIP_ERR_NO_EQUIPMENT_SLOTS_IS_AVAILABLE,
	EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED,
	EQUIP_ERR_CANT_DUAL_WIELD_YET,
	EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG,
	EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG2,
	EQUIP_ERR_CANT_CARRY_MORE_OF_THIS,
	EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE2,
	EQUIP_ERR_ITEM_CANT_STACK,
	EQUIP_ERR_ITEM_CANT_BE_EQUIPPED,
	EQUIP_ERR_ITEMS_CANT_BE_SWAPPED,
	EQUIP_ERR_SLOT_IS_EMPTY,
	EQUIP_ERR_ITEM_NOT_FOUND,
	EQUIP_ERR_CANT_DROP_SOULBOUND,
	EQUIP_ERR_OUT_OF_RANGE,
	EQUIP_ERR_TRIED_TO_SPLIT_MORE_THAN_COUNT,
	EQUIP_ERR_COULDNT_SPLIT_ITEMS,
	EQUIP_ERR_BAG_FULL2,
	EQUIP_ERR_NOT_ENOUGH_MONEY,
	EQUIP_ERR_NOT_A_BAG,
	EQUIP_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS,
	EQUIP_ERR_DONT_OWN_THAT_ITEM,
	EQUIP_ERR_CAN_EQUIP_ONLY1_QUIVER,
	EQUIP_ERR_MUST_PURCHASE_THAT_BAG_SLOT,
	EQUIP_ERR_TOO_FAR_AWAY_FROM_BANK,
	EQUIP_ERR_ITEM_LOCKED,
	EQUIP_ERR_YOU_ARE_STUNNED,
	EQUIP_ERR_YOU_ARE_DEAD,
	EQUIP_ERR_CANT_DO_RIGHT_NOW,
	EQUIP_ERR_BAG_FULL3,
	EQUIP_ERR_CAN_EQUIP_ONLY1_QUIVER2,
	EQUIP_ERR_CAN_EQUIP_ONLY1_AMMOPOUCH,
	EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED,
	EQUIP_ERR_EQUIPPED_CANT_BE_WRAPPED,
	EQUIP_ERR_WRAPPED_CANT_BE_WRAPPED,
	EQUIP_ERR_BOUND_CANT_BE_WRAPPED,
	EQUIP_ERR_UNIQUE_CANT_BE_WRAPPED,
	EQUIP_ERR_BAGS_CANT_BE_WRAPPED,
	EQUIP_ERR_ALREADY_LOOTED,
	EQUIP_ERR_INVENTORY_FULL,
	EQUIP_ERR_BANK_FULL,
	EQUIP_ERR_ITEM_IS_CURRENTLY_SOLD_OUT,
	EQUIP_ERR_BAG_FULL4,
	EQUIP_ERR_ITEM_NOT_FOUND2,
	EQUIP_ERR_ITEM_CANT_STACK2,
	EQUIP_ERR_BAG_FULL5,
	EQUIP_ERR_ITEM_SOLD_OUT,
	EQUIP_ERR_OBJECT_IS_BUSY,
	EQUIP_ERR_NONE,					// DOES NOT EXIST
	EQUIP_ERR_CANT_DO_IN_COMBAT,
	EQUIP_CANT_DO_WHILE_DISARMED,
	EQUIP_ERR_NONE2,				// DOES NOT EXIST
	EQUIP_ITEM_RANK_NOT_ENOUGH,
	EQUIP_ITEM_REPUTATION_NOT_ENOUGH
};

class Item : public Object
{
    public:
        Item ( );

        void Create( uint32 guidlow, uint32 itemid, Player* owner );

        ItemPrototype* GetProto() const { return m_itemProto; }

        Player* GetOwner() const { return m_owner; }
        void SetOwner(Player *owner) { m_owner = owner; }

        //! DB Serialization
        void SaveToDB();
        void LoadFromDB(uint32 guid, uint32 auctioncheck);
        void DeleteFromDB();

        //! Item Properties
        void SetDurability(uint32 Value);
        void SetDurabilityToMax();

		// UQ1: Added for real item info....
		void UpdateStats();

        ItemPrototype *GetItemProto() { return m_itemProto; }

		uint32 GetCount() { return GetUInt32Value (ITEM_FIELD_STACK_COUNT); }
		void SetCount(uint32 value) { SetUInt32Value (ITEM_FIELD_STACK_COUNT, value); }

    protected:
        ItemPrototype *m_itemProto;
        Player *m_owner;                          // let's not bother the manager with unneeded requests
};
#endif
