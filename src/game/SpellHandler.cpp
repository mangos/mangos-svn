/* SpellHandler.cpp
 *
 * Copyright (C) 2004 Wow Daemon
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

// check for spell id
    SpellEntry *spellInfo = sSpellStore.LookupEntry( spellId );

    if(!spellInfo)
    {
        Log::getSingleton( ).outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    Spell *spell = new Spell(GetPlayer(), spellInfo,false, 0);
    WPAssert(spell);

    SpellCastTargets targets;
    targets.read(&recvPacket,GetPlayer()->GetGUID());
    spell->m_CastItem = tmpItem;
    spell->prepare(&targets);
}


void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    uint32 spellId;

    recvPacket >> spellId;

    Log::getSingleton( ).outDetail("WORLD: got cast spell packet, spellId - %i, data length = %i\n",
        spellId, recvPacket.size());

// check for spell id
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
