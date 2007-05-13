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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "BattleGroundMgr.h"
#include "MapManager.h"
#include "ScriptCalls.h"

void WorldSession::HandleUseItemOpcode(WorldPacket& recvPacket)
{
    // TODO: add targets.read() check
    CHECK_PACKET_SIZE(recvPacket,1+1+1);

    sLog.outDetail("WORLD: CMSG_USE_ITEM packet, data length = %i",recvPacket.size());

    Player* pUser = _player;
    uint8 bagIndex, slot, tmp3;

    recvPacket >> bagIndex >> slot >> tmp3;

    Item *pItem = pUser->GetItemByPos(bagIndex, slot);
    if(!pItem)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
        return;
    }

    ItemPrototype const *proto = pItem->GetProto();
    if(!proto)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, NULL );
        return;
    }

    uint8 msg = pUser->CanUseItem(pItem);
    if( msg != EQUIP_ERR_OK )
    {
        pUser->SendEquipError( msg, pItem, NULL );
        return;
    }

    if (pUser->isInCombat())
    {
        uint8 consumable = proto->Class == ITEM_CLASS_CONSUMABLE &&
            proto->SubClass != ITEM_SUBCLASS_POTION &&
            proto->SubClass != ITEM_SUBCLASS_SCROLL &&
            proto->SubClass != ITEM_SUBCLASS_BANDAGE &&
            proto->SubClass != ITEM_SUBCLASS_HEALTHSTONE &&
            proto->SubClass != ITEM_SUBCLASS_COMBAT_EFFECT;

        uint8 trade_goods = proto->Class == ITEM_CLASS_TRADE_GOODS && proto->SubClass != ITEM_SUBCLASS_EXPLOSIVES;
        if (consumable || trade_goods ||
            proto->Class == ITEM_CLASS_KEY || proto->Class == ITEM_CLASS_MISC)
        {
            pUser->SendEquipError(EQUIP_ERR_CANT_DO_IN_COMBAT,pItem,NULL);
            return;
        }
    }

    // check also  BIND_WHEN_PICKED_UP for .additem or .additemset case by GM (not binded at adding to inventory)
    if( pItem->GetProto()->Bonding == BIND_WHEN_USE || pItem->GetProto()->Bonding == BIND_WHEN_PICKED_UP)
    {
        if (!pItem->IsSoulBound())
        {
            pItem->SetState(ITEM_CHANGED, pUser);
            pItem->SetBinding( true );
        }
    }

    SpellCastTargets targets;
    targets.read(&recvPacket, pUser);

    // use trigerred flag only for items with many spell casts and for not first cast
    int count = 0;

    for(int i = 0; i <5; ++i)
    {
        if( proto->Spells[i].SpellTrigger != USE )
            continue;

        uint32 spellId = proto->Spells[i].SpellId;

        if(!spellId)
            continue;

        SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
        if(!spellInfo)
        {
            sLog.outError("Item (Entry: %u) in have wrong spell id %u, ignoring ",proto->ItemId, spellId);
            continue;
        }

        Spell *spell = new Spell(pUser, spellInfo, (count > 0) , 0);
        spell->m_CastItem = pItem;
        spell->prepare(&targets);

        // delete triggered spell
        if(count > 0)
            delete spell;

        ++count;
    }

    Script->ItemUse(pUser,pItem);
}

#define OPEN_CHEST 11437
#define OPEN_SAFE 11535
#define OPEN_CAGE 11792
#define OPEN_BOOTY_CHEST 5107
#define OPEN_STRONGBOX 8517

void WorldSession::HandleOpenItemOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket,1+1);

    sLog.outDetail("WORLD: CMSG_OPEN_ITEM packet, data length = %i",recvPacket.size());

    Player* pUser = _player;
    uint8 bagIndex, slot;

    recvPacket >> bagIndex >> slot;

    sLog.outDetail("bagIndex: %u, slot: %u",bagIndex,slot);

    Item *pItem = pUser->GetItemByPos(bagIndex, slot);
    if(!pItem)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
        return;
    }

    ItemPrototype const *proto = pItem->GetProto();
    if(!proto)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, NULL );
        return;
    }

    // locked item
    uint32 lockId = proto->LockID;
    if(lockId)
    {
        LockEntry const *lockInfo = sLockStore.LookupEntry(lockId);

        if (!lockInfo)
        {
            pUser->SendEquipError(EQUIP_ERR_ITEM_LOCKED, pItem, NULL );
            sLog.outError( "WORLD::OpenItem: item [guid = %u] has an unknown lockId: %u!", pItem->GetGUIDLow() , lockId);
            return;
        }

        // required picklocking
        if(lockInfo->requiredlockskill || lockInfo->requiredskill)
        {
            pUser->SendEquipError(EQUIP_ERR_ITEM_LOCKED, pItem, NULL );
            return;
        }
    }

    if(pItem->HasFlag(ITEM_FIELD_FLAGS, 8))                 // wrapped?
    {
        QueryResult *result = sDatabase.PQuery("SELECT `entry`, `flags` FROM `character_gifts` WHERE `item_guid` = '%u'", pItem->GetGUIDLow());
        if (result)
        {
            Field *fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            uint32 flags = fields[1].GetUInt32();

            pItem->SetUInt64Value(ITEM_FIELD_GIFTCREATOR, 0);
            pItem->SetUInt32Value(OBJECT_FIELD_ENTRY, entry);
            pItem->SetUInt32Value(ITEM_FIELD_FLAGS, flags);
            pItem->SetState(ITEM_CHANGED, pUser);
            delete result;
        }
        else
        {
            sLog.outError("Wrapped item %u don't have record in character_gifts table and will deleted", pItem->GetGUIDLow());
            pUser->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
            return;
        }
        sDatabase.PExecute("DELETE FROM `character_gifts` WHERE `item_guid` = '%u'", pItem->GetGUIDLow());
    }
    else
        pUser->SendLoot(pItem->GetGUID(),LOOT_CORPSE);
}

void WorldSession::HandleGameObjectUseOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    uint32 spellId = OPEN_CHEST;
    const GameObjectInfo *info;

    recv_data >> guid;

    sLog.outDebug( "WORLD: Recvd CMSG_GAMEOBJ_USE Message [guid=%u]", guid);
    GameObject *obj = ObjectAccessor::Instance().GetGameObject(*_player, guid);

    if(!obj) return;
    // uint32 t = obj->GetUInt32Value(GAMEOBJECT_TYPE_ID);
    //obj->SetUInt32Value(GAMEOBJECT_FLAGS,2);
    //obj->SetUInt32Value(GAMEOBJECT_FLAGS,2);

    // default spell caster is player that use GO
    Unit* spellCaster = GetPlayer();
    // default spell target is player that use GO
    Unit* spellTarget = GetPlayer();

    uint32 t = obj->GetUInt32Value(GAMEOBJECT_TYPE_ID);
    switch(t)
    {
        //door
        case GAMEOBJECT_TYPE_DOOR:                          //0
            obj->SetUInt32Value(GAMEOBJECT_FLAGS,33);
            obj->SetUInt32Value(GAMEOBJECT_STATE,0);        //open
            //obj->SetUInt32Value(GAMEOBJECT_TIMESTAMP,0x465EE6D2); //load timestamp

            obj->SetLootState(GO_CLOSED);
            obj->SetRespawnTime(5);                         //close door in 5 seconds

            return;
        case GAMEOBJECT_TYPE_QUESTGIVER:                    // 2
            _player->PrepareQuestMenu( guid );
            _player->SendPreparedQuest( guid );
            return;
            //Sitting: Wooden bench, chairs enzz
        case GAMEOBJECT_TYPE_CHAIR:                         //7

            info = obj->GetGOInfo();
            if(info)
            {
                spellId = info->sound0;
                //guid=GetPlayer()->GetGUID();

                _player->TeleportTo(obj->GetMapId(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation(),false,false);
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

                //guid=_player->GetGUID();
            }

            break;
        case GAMEOBJECT_TYPE_CAMERA:                        //13
            info = obj->GetGOInfo();
            if(info)
            {
                uint32 cinematic_id = info->sound1;
                if(cinematic_id)
                {
                    WorldPacket data(SMSG_TRIGGER_CINEMATIC, 4);
                    data << cinematic_id;
                    _player->GetSession()->SendPacket(&data);
                }
                return;
            }
            break;
            //fishing bobber
        case GAMEOBJECT_TYPE_FISHINGNODE:                   //17
        {
            if(_player->GetGUID() != obj->GetOwnerGUID())
                return;

            switch(obj->getLootState())
            {
                case GO_CLOSED:                             // ready for loot
                {
                    // 1) skill must beb >= base_zone_skill
                    // 2) if skill == base_zone_skill => 5% chance
                    // 3) chance is liniar dependence from (base_zone_skill-skill)

                    int32 skill = _player->GetSkillValue(SKILL_FISHING);
                    int32 zone_skill = _player->FishingMinSkillForCurrentZone();
                    int32 chance = skill - zone_skill + 5;
                    int32 roll = irand(1,100);

                    DEBUG_LOG("Fishing check (skill: %i zone min skill: %i chance %i roll: %i",skill,zone_skill,chance,roll);

                    if(skill >= zone_skill && chance >= roll)
                    {
                        // prevent removing GO at spell cancel
                        _player->RemoveGameObject(obj,false);
                        obj->SetOwnerGUID(_player->GetGUID());

                        //fish catched
                        _player->UpdateFishingSkill();
                        _player->SendLoot(obj->GetGUID(),LOOT_FISHING);
                    }
                    else
                    {
                        // fish escaped
                        obj->SetLootState(GO_LOOTED);       // can be deleted now

                        WorldPacket data(SMSG_FISH_ESCAPED, 0);
                        SendPacket(&data);
                    }
                    break;
                }
                case GO_LOOTED:                             // nothing to do, will be deleted at next update
                    break;
                default:
                {
                    obj->SetLootState(GO_LOOTED);

                    WorldPacket data(SMSG_FISH_NOT_HOOKED, 0);
                    SendPacket(&data);
                    break;
                }
            }

            if(_player->m_currentSpell)
            {
                _player->m_currentSpell->SendChannelUpdate(0);
                _player->m_currentSpell->finish();
            }
            return;
        }

        case GAMEOBJECT_TYPE_SUMMONING_RITUAL:
        {
            Unit* caster = obj->GetOwner();

            if( !caster || caster->GetTypeId()!=TYPEID_PLAYER )
                return;

            // accept only use by player from same group for caster except caster itself
            if(((Player*)caster)==GetPlayer() || !((Player*)caster)->IsInSameGroupWith(GetPlayer()))
                return;

            obj->AddUse(GetPlayer());

            // must 2 group members use GO
            if(obj->GetUniqueUseCount() < 2)
                return;
            
            // in case summoning ritual caster is GO creator
            spellCaster = caster;

            if(!caster->m_currentSpell)
                return;

            // update target pointer by guid

            // in case summoning ritual target is caster current selection
            //TODO: maybe GO creating spell must set target different from caster
            Player* targetPlayer = ObjectAccessor::Instance().FindPlayer(((Player*)caster)->GetSelection());

            // recheck that target is group member and can enter to GO map
            if( !targetPlayer || !targetPlayer->IsInSameGroupWith((Player*)caster) ||
                !MapManager::Instance().GetMap(obj->GetMapId(),obj)->CanEnter(targetPlayer) )
            {
                caster->m_currentSpell->cancel();
                return;
            }

            // target is correct
            spellTarget = targetPlayer;

            // prepare data for final summoning (before current spell finish to prevent access to deleted GO)
            info = obj->GetGOInfo();
            spellId = info->sound1;

            // finish spell
            caster->m_currentSpell->SendChannelUpdate(0);
            caster->m_currentSpell->finish();

            // can be deleted now
            obj->SetLootState(GO_LOOTED);

            // go to end function to spell casting
            break;
        }
        case GAMEOBJECT_TYPE_FLAGSTAND:                     // 24
            if(_player->InBattleGround() &&                 // in battleground
                !_player->IsMounted() &&                    // not mounted
                !_player->HasStealthAura() &&               // not stealthed
                !_player->HasInvisibilityAura())            // not invisible
            {
                //BG flag click
                info = obj->GetGOInfo();
                if(info)
                    spellId = info->sound1;
            }
            break;
        case GAMEOBJECT_TYPE_FLAGDROP:                      // 26
            if(_player->InBattleGround() &&                 // in battleground
                !_player->IsMounted() &&                    // not mounted
                !_player->HasStealthAura() &&               // not stealthed
                !_player->HasInvisibilityAura())            // not invisible
            {
                //BG flag dropped
                info = obj->GetGOInfo();
                if(info->id == 179785)                      // Silverwing Flag
                {
                    if(_player->GetTeam() == ALLIANCE)
                        spellId = 23385;                    // Alliance Flag Returns
                    if(_player->GetTeam() == HORDE)
                        spellId = 23335;                    // Silverwing Flag
                }
                if(info->id == 179786)                      // Warsong Flag
                {
                    if(_player->GetTeam() == HORDE)
                        spellId = 23386;                    // Horde Flag Returns
                    if(_player->GetTeam() == ALLIANCE)
                        spellId = 23333;                    // Warsong Flag
                }
                obj->Delete();
            }
            break;
        default:
            sLog.outDebug( "Unknown Object Type %u\n", obj->GetUInt32Value(GAMEOBJECT_TYPE_ID));
            break;
    }

    SpellEntry const *spellInfo = sSpellStore.LookupEntry( spellId );

    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    Spell *spell = new Spell(spellCaster, spellInfo, false, 0);

    SpellCastTargets targets;
    targets.setUnitTarget( spellTarget );
    targets.setGOTarget( obj );
    spell->prepare(&targets);

}

void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket,4);

    uint32 spellId;

    recvPacket >> spellId;

    sLog.outDetail("WORLD: got cast spell packet, spellId - %i, data length = %i",
        spellId, recvPacket.size());

    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId );

    if(!spellInfo)
    {
        sLog.outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    if ( !_player->HasSpell (spellId) )
    {
        //cheater? kick? ban?
        return;
    }

    Spell *spell ;
    spell = new Spell(_player, spellInfo, false, 0);

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
    CHECK_PACKET_SIZE(recvPacket,4);

    uint32 spellId;
    recvPacket >> spellId;
    _player->RemoveAurasDueToSpell(spellId);
}

void WorldSession::HandleCancelGrowthAuraOpcode( WorldPacket& recvPacket)
{
    // nothing do
}

void WorldSession::HandleCancelAutoRepeatSpellOpcode( WorldPacket& recvPacket)
{
    // may be better send SMSG_CANCEL_AUTO_REPEAT?
    // cancel and prepare for deleting
    _player->castSpell(NULL);
}
