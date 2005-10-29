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

		ItemPrototype *GetItemProto() { return m_itemProto; }

    protected:
        ItemPrototype *m_itemProto;
        Player *m_owner;                          // let's not bother the manager with unneeded requests
};
#endif
