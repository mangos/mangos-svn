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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"

void WorldSession::HandleUseItemOpcode(WorldPacket& recvPacket)
{


	Player* p_User = GetPlayer();
    Log::getSingleton( ).outDetail("WORLD: got use Item packet, data length = %i\n",recvPacket.size());
    uint8 tmp1,slot,tmp3;
    uint32 spellId;

    recvPacket >> tmp1 >> slot >> tmp3;

    Item* tmpItem = new Item;
    tmpItem = p_User->GetItemBySlot(slot);
    ItemPrototype *itemProto = tmpItem->GetProto();
    spellId = itemProto->SpellId[0];

    
    SpellEntry *spellInfo = sSpellStore.LookupEntry( spellId );
    if(!spellInfo)
    {
        Log::getSingleton( ).outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }
	
	



	

	
	if (!p_User->CanUseItem(itemProto)) return;
	

	
	if (p_User->inCombat) {
		
		if (itemProto->Class == ITEM_CLASS_CONSUMABLE	|| 
			itemProto->Class == ITEM_CLASS_TRADE_GOODS	||
			
			itemProto->Class == ITEM_CLASS_KEY			||
			itemProto->Class == ITEM_CLASS_JUNK			){

			WorldPacket data;

			data.Initialize (SMSG_INVENTORY_CHANGE_FAILURE);

			data << uint32(EQUIP_ERR_CANT_DO_IN_COMBAT);
			data << (tmpItem ? tmpItem->GetGUID() : uint64(0));
			data << (tmpItem ? tmpItem->GetGUID() : uint64(0));
			data << uint8(0);

			SendPacket( &data );
			return;
		}
	}
	

	
    Spell *spell = new Spell(GetPlayer(), spellInfo,false, 0);
    WPAssert(spell);

	SpellCastTargets targets;
    targets.read(&recvPacket,GetPlayer()->GetGUID());
    spell->m_CastItem = tmpItem;
    spell->prepare(&targets);
	

	
	uint32 ItemCount = tmpItem->GetCount();
	uint32 ItemClass = itemProto->Class;
	uint32 ItemId    = itemProto->ItemId;

	if (ItemClass == ITEM_CLASS_CONSUMABLE) {
		if (ItemCount > 1) {
			tmpItem->SetCount(ItemCount-1);
		}
		else {
			p_User->RemoveItemFromSlot(slot);
			
			
			
			
		}
	}
	

}

void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    uint32 spellId;

    recvPacket >> spellId;

#ifdef __REVIVE_CHEAT__ 
    if (GetPlayer()->isDead())
    {
        GetPlayer()->SetMovement(MOVE_LAND_WALK);
        GetPlayer()->SetMovement(MOVE_UNROOT);
        GetPlayer()->SetPlayerSpeed(RUN, (float)7.5, true);
        GetPlayer()->SetPlayerSpeed(SWIM, (float)4.9, true);

        GetPlayer()->SetUInt32Value(CONTAINER_FIELD_SLOT_1+29, 0);
        GetPlayer()->SetUInt32Value(UNIT_FIELD_AURA+32, 0);
        GetPlayer()->SetUInt32Value(UNIT_FIELD_AURALEVELS+8, 0xeeeeeeee);
        GetPlayer()->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+8, 0xeeeeeeee);
        GetPlayer()->SetUInt32Value(UNIT_FIELD_AURAFLAGS+4, 0);
        GetPlayer()->SetUInt32Value(UNIT_FIELD_AURASTATE, 0);

        GetPlayer()->ResurrectPlayer();
        GetPlayer()->SetUInt32Value(UNIT_FIELD_HEALTH, (uint32)(GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*0.50) );
        GetPlayer()->SpawnCorpseBones();
        
    }
#endif 


    Log::getSingleton( ).outDetail("WORLD: got cast spell packet, spellId - %i, data length = %i",
        spellId, recvPacket.size());

    
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId );

    if(!spellInfo)
    {
        Log::getSingleton( ).outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

        Spell *spell = new Spell(GetPlayer(), spellInfo, false, 0);
        WPAssert(spell);

        SpellCastTargets targets;
        targets.read(&recvPacket,GetPlayer()->GetGUID());

        spell->prepare(&targets);

}


void WorldSession::HandleCancelCastOpcode(WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;

    if(GetPlayer()->m_currentSpell)
        GetPlayer()->m_currentSpell->cancel();
}


void WorldSession::HandleCancelAuraOpcode( WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;
    GetPlayer()->RemoveAffectById(spellId);
}
