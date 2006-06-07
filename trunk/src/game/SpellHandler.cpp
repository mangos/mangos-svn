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

    Item *pItem = pUser->GetItemByPos(bagIndex, slot);
    ItemPrototype *proto = pItem->GetProto();
    spellId = proto->Spells[0].SpellId;

    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i", spellId);
        return;
    }

    if (!pUser->CanUseItem(pItem, true)) return;

    if (pUser->isInCombat())
    {
        if (proto->Class == ITEM_CLASS_CONSUMABLE || proto->Class == ITEM_CLASS_TRADE_GOODS ||
            proto->Class == ITEM_CLASS_KEY || proto->Class == ITEM_CLASS_JUNK)
        {

            WorldPacket data;
            data.Initialize (SMSG_INVENTORY_CHANGE_FAILURE);
            data << uint8(EQUIP_ERR_CANT_DO_IN_COMBAT);
            data << (pItem ? pItem->GetGUID() : uint64(0));
            //            data << (pItem ? pItem->GetGUID() : uint64(0));
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

    uint32 t = obj->GetUInt32Value(GAMEOBJECT_TYPE_ID);

    switch(t)
    {
        //door
        case GAMEOBJECT_TYPE_DOOR:                          //0
            obj->SetUInt32Value(GAMEOBJECT_FLAGS,33);
            obj->SetUInt32Value(GAMEOBJECT_STATE,0);        //open
            //obj->SetUInt32Value(GAMEOBJECT_TIMESTAMP,0x465EE6D2); //load timestamp

            obj->SetLootState((LootState)0);
            obj->SetSespawnTimer(5000);                     //close door in 5 seconds

            return;
            break;
        case GAMEOBJECT_TYPE_QUESTGIVER:                    // 2
            _player->PrepareQuestMenu( guid );
            _player->SendPreparedQuest( guid );
            return;
            break;

            //Sitting: Wooden bench, chairs enzz
        case GAMEOBJECT_TYPE_CHAIR:                         //7

            info = obj->GetGOInfo();
            if(info)
            {
                spellId = info->sound0;

                _player->BuildTeleportAckMsg(&data, obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());
                _player->GetSession()->SendPacket(&data);

                _player->SetPosition( obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());
                _player->BuildHeartBeatMsg(&data);
                _player->SendMessageToSet(&data, true);

                                                            //offset 3 is related to the DB
                _player->SetUInt32Value(UNIT_FIELD_BYTES_1, _player->GetUInt32Value(UNIT_FIELD_BYTES_1) | (3 + spellId) );

                return;
            }
            break;

            //big gun, its a spell/aura
        case GAMEOBJECT_TYPE_GOOBER:                        //10

            //chest locked
        case GAMEOBJECT_TYPE_SPELLCASTER:                   //22

            obj->SetUInt32Value(GAMEOBJECT_FLAGS,2);

            info = obj->GetGOInfo();
            if(info)
            {
                spellId = info->sound0;
                if (spellId == 0)
                    spellId = info->sound3;

                guid=_player->GetGUID();
            }
            break;

        case GAMEOBJECT_TYPE_FLAGSTAND:                     //24
            //GB flag
            info = obj->GetGOInfo();
            if(info)
            {
                spellId = info->sound0;
                guid=_player->GetGUID();
            }
            break;

        case GAMEOBJECT_TYPE_FLAGDROP:                      //26
            //GB flag dropped
            info = obj->GetGOInfo();
            if(info)
            {
                spellId = info->sound0;
                guid=_player->GetGUID();
            }
            break;

        default:
            sLog.outDebug( "Unknown Object Type %u\n", obj->GetUInt32Value(GAMEOBJECT_TYPE_ID));
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

    Spell *spell ;
    spell = new Spell(_player, spellInfo, false, 0);

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
    _player->RemoveAurasDueToSpell(spellId);
}

void WorldSession::HandleCancelAutoRepeatSpellOpcode( WorldPacket& recvPacket)
{
    if(_player->m_currentSpell)
        _player->m_currentSpell->cancel();
}
