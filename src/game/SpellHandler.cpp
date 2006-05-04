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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"

void WorldSession::HandleUseItemOpcode(WorldPacket& recvPacket)
{
    sLog.outDetail("WORLD: got use Item packet, data length = %i",recvPacket.size());

    Player* pUser = _player;
    uint8 bagIndex, slot, tmp3;
    uint32 spellId;

    recvPacket >> bagIndex >> slot >> tmp3;

    Item *pItem = pUser->GetItemBySlot(bagIndex, slot);
    ItemPrototype *proto = pItem->GetProto();
    spellId = proto->Spells[0].SpellId;

    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i", spellId);
        return;
    }

    if (!pUser->CanUseItem(proto)) return;

    if (pUser->inCombat)
    {
        if (proto->Class == ITEM_CLASS_CONSUMABLE || proto->Class == ITEM_CLASS_TRADE_GOODS ||
            proto->Class == ITEM_CLASS_KEY || proto->Class == ITEM_CLASS_JUNK)
        {

            WorldPacket data;
            data.Initialize (SMSG_INVENTORY_CHANGE_FAILURE);
            data << uint8(EQUIP_ERR_CANT_DO_IN_COMBAT);
            data << (pItem ? pItem->GetGUID() : uint64(0));
            //			data << (pItem ? pItem->GetGUID() : uint64(0));
            data << uint64(0);
            data << uint8(0);
            SendPacket(&data);
            return;
        }
    }

    Spell *spell = new Spell(pUser, spellInfo, false, 0);
    WPAssert(spell);

    SpellCastTargets targets;
    targets.read(&recvPacket, pUser);
    spell->m_CastItem = pItem;
    spell->prepare(&targets);

    uint32 ItemCount = pItem->GetCount();
    uint32 ItemClass = proto->Class;
    uint32 ItemId = proto->ItemId;

    if (ItemClass == ITEM_CLASS_CONSUMABLE)
    {
        if (ItemCount > 1)
        {
            pItem->SetCount(ItemCount-1);
        }
        else
        {
            pUser->RemoveItemFromSlot(bagIndex , slot);
            //pItem->DeleteFromDB();
            delete pItem;
        }
    }
}

#define OPEN_CHEST 11437
#define OPEN_SAFE 11535
#define OPEN_CAGE 11792
#define OPEN_BOOTY_CHEST 5107
#define OPEN_STRONGBOX 8517

void WorldSession::HandleGameObjectUseOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;
    uint32 spellId = OPEN_CHEST;
    const GameObjectInfo *info;

    recv_data >> guid;

    sLog.outDebug( "WORLD: Recvd CMSG_GAMEOBJ_USE Message [guid=%u]", guid);
    GameObject *obj = ObjectAccessor::Instance().GetGameObject(*_player, guid);

    if(!obj) return;
    uint32 t=obj->GetUInt32Value(GAMEOBJECT_TYPE_ID);
    obj->SetUInt32Value(GAMEOBJECT_FLAGS,2);
    obj->SetUInt32Value(GAMEOBJECT_FLAGS,2);
    switch(obj->GetUInt32Value(GAMEOBJECT_TYPE_ID))
    {
        case 22:
            info = obj->GetGOInfo();
            if(info)
            {
                spellId = info->sound0;
                guid=_player->GetGUID();
            }
            break;
        default:
            sLog.outDebug( "Unkonw Object Type %u\n", obj->GetUInt32Value(GAMEOBJECT_TYPE_ID));
            break;
    }

    SpellEntry *spellInfo = sSpellStore.LookupEntry( spellId );

    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    Spell *spell = new Spell(_player, spellInfo, false, 0);
    WPAssert(spell);

    SpellCastTargets targets;
    targets.setUnitTarget( _player );
    targets.m_GOTarget = obj;
    spell->prepare(&targets);

}

void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    uint32 spellId;

    recvPacket >> spellId;

    sLog.outDetail("WORLD: got cast spell packet, spellId - %i, data length = %i",
        spellId, recvPacket.size());

    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId );

    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    Spell *spell = new Spell(_player, spellInfo, false, 0);
    WPAssert(spell);

    SpellCastTargets targets;
    targets.read(&recvPacket,_player);

    spell->prepare(&targets);

}

void WorldSession::HandleCancelCastOpcode(WorldPacket& recvPacket)
{
    if(_player->m_currentSpell)
        _player->m_currentSpell->cancel();
}

void WorldSession::HandleCancelAuraOpcode( WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;
    _player->RemoveAura(spellId);
}

void WorldSession::HandleCancelAutoRepeatSpellOpcode( WorldPacket& recvPacket)
{
    if(_player->m_currentSpell)
        _player->m_currentSpell->cancel();
}
