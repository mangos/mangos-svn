/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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
#include "Log.h"
#include "Corpse.h"
#include "Gameobject.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "WorldSession.h"
#include "LootMgr.h"
#include "Object.h"
#include "Group.h"
#include "World.h"

void WorldSession::HandleAutostoreLootItemOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,1);

    sLog.outDebug("WORLD: CMSG_AUTOSTORE_LOOT_ITEM");
    Player  *player =   GetPlayer();
    uint64   lguid =    player->GetLootGUID();
    Loot    *loot;
    uint8    lootSlot;

    recv_data >> lootSlot;

    if (IS_GAMEOBJECT_GUID(lguid))
    {
        GameObject *go =
            ObjectAccessor::GetGameObject(*player, lguid);

        // not check distance for GO in case owned GO (fishing bobber case, for example)
        if (!go || go->GetOwnerGUID() != _player->GetGUID() && !go->IsWithinDistInMap(_player,INTERACTION_DISTANCE))
        {
            player->SendLootRelease(lguid);
            return;
        }

        loot = &go->loot;
    }
    else if (IS_ITEM_GUID(lguid))
    {
        Item *pItem = player->GetItemByGuid( lguid );

        if (!pItem)
        {
            player->SendLootRelease(lguid);
            return;
        }

        loot = &pItem->loot;
    }
    else
    {
        Creature* pCreature =
            ObjectAccessor::GetCreature(*player, lguid);

        bool ok_loot = pCreature && pCreature->isAlive() == (player->getClass()==CLASS_ROGUE && pCreature->lootForPickPocketed);

        if( !ok_loot || !pCreature->IsWithinDistInMap(_player,INTERACTION_DISTANCE) )
        {
            player->SendLootRelease(lguid);
            return;
        }

        loot = &pCreature->loot;
    }

    LootItem *item = NULL;
    QuestItem *qitem = NULL;
    QuestItem *ffaitem = NULL;
    QuestItem *conditem = NULL;
    bool is_looted = true;
    if (lootSlot >= loot->items.size())
    {
        lootSlot -= loot->items.size();
        QuestItemMap::iterator itr = loot->PlayerQuestItems.find(player->GetGUIDLow());
        if (itr != loot->PlayerQuestItems.end() && lootSlot < itr->second->size())
        {
            qitem = &itr->second->at(lootSlot);
            item = &loot->quest_items[qitem->index];
            is_looted = qitem->is_looted;
        }
    }
    else
    {
        item = &loot->items[lootSlot];
        is_looted = item->is_looted;
        if(item->freeforall)
        {
            QuestItemMap::iterator itr = loot->PlayerFFAItems.find(player->GetGUIDLow());
            if (itr != loot->PlayerFFAItems.end())
            {
                for(QuestItemList::iterator iter=itr->second->begin(); iter!= itr->second->end(); ++iter)
                if(iter->index==lootSlot)
                {
                    ffaitem = (QuestItem*)&(*iter);
                    is_looted = ffaitem->is_looted;
                    break;
                }
            }
        }
        else if(item->condition)
        {
            QuestItemMap::iterator itr = loot->PlayerNonQuestNonFFAConditionalItems.find(player->GetGUIDLow());
            if (itr != loot->PlayerNonQuestNonFFAConditionalItems.end())
            {
                for(QuestItemList::iterator iter=itr->second->begin(); iter!= itr->second->end(); ++iter)
                if(iter->index==lootSlot)
                {
                    conditem = (QuestItem*)&(*iter);
                    is_looted = conditem->is_looted;
                    break;
                }
            }
        }
    }

    if ((item == NULL) || is_looted)
    {
        player->SendEquipError( EQUIP_ERR_ALREADY_LOOTED, NULL, NULL );
        return;
    }

    // questitems use the blocked field for other purposes
    if (!qitem && item->is_blocked)
    {
        player->SendLootRelease(lguid);
        return;
    }

    uint16 dest;
    uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, item->itemid, item->count, false );
    if ( msg == EQUIP_ERR_OK )
    {
        Item * newitem = player->StoreNewItem( dest, item->itemid, item->count, true, item->randomPropertyId);

        if (qitem)
        {
            qitem->is_looted = true;
            if (item->freeforall || loot->PlayerQuestItems.size() == 1) //freeforall is 1 if everyone's supposed to get the quest item.
                player->SendNotifyLootItemRemoved(loot->items.size() + lootSlot);
            else
                loot->NotifyQuestItemRemoved(qitem->index);
        }
        else
        {
            if (ffaitem)
            {
                //freeforall case, notify only one player of the removal
                ffaitem->is_looted=true;
                player->SendNotifyLootItemRemoved(lootSlot);
            }
            else
            {
                //not freeforall, notify everyone
                if(conditem)
                    conditem->is_looted=true;
                loot->NotifyItemRemoved(lootSlot);
            }
        }

        //if only one person is supposed to loot the item, then set it to looted
        if (!item->freeforall)
            item->is_looted = true;

        loot->unlootedCount--;

        player->SendNewItem(newitem, uint32(item->count), false, false, true);
    }
    else
        player->SendEquipError( msg, NULL, NULL );
}

void WorldSession::HandleLootMoneyOpcode( WorldPacket & /*recv_data*/ )
{
    sLog.outDebug("WORLD: CMSG_LOOT_MONEY");

    Player *player = GetPlayer();
    uint64 guid = player->GetLootGUID();
    Loot *pLoot = NULL;

    if( IS_GAMEOBJECT_GUID( guid ) )
    {
        GameObject *pGameObject = ObjectAccessor::GetGameObject(*GetPlayer(), guid);

        // not check distance for GO in case owned GO (fishing bobber case, for example)
        if( pGameObject && (pGameObject->GetOwnerGUID()==_player->GetGUID() || pGameObject->IsWithinDistInMap(_player,INTERACTION_DISTANCE)) )
            pLoot = &pGameObject->loot;
    }
    else if( IS_CORPSE_GUID( guid ) )    // remove insignia ONLY in BG
    {
        Corpse *bones = ObjectAccessor::GetCorpse(*GetPlayer(), guid);

        if (bones && bones->IsWithinDistInMap(_player,INTERACTION_DISTANCE) )
            pLoot = &bones->loot;
    }
    else
    {
        Creature* pCreature = ObjectAccessor::GetCreature(*GetPlayer(), guid);

        bool ok_loot = pCreature && pCreature->isAlive() == (player->getClass()==CLASS_ROGUE && pCreature->lootForPickPocketed);

        if ( ok_loot && pCreature->IsWithinDistInMap(_player,INTERACTION_DISTANCE) )
            pLoot = &pCreature->loot ;
    }

    if( pLoot )
    {
        if (player->GetGroup())
        {
            Group *group = player->GetGroup();

            std::vector<Player*> playersNear;
            for(GroupReference *itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* playerGroup = itr->getSource();
                if(!playerGroup)
                    continue;
                if (player->GetDistance2dSq(playerGroup) < sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE))
                    playersNear.push_back(playerGroup);
            }

            for (std::vector<Player*>::iterator i = playersNear.begin(); i != playersNear.end(); ++i)
            {
                (*i)->ModifyMoney( uint32((pLoot->gold)/(playersNear.size())) );
                //Offset surely incorrect, but works
                WorldPacket data( SMSG_LOOT_MONEY_NOTIFY, 4 );
                data << uint32((pLoot->gold)/(playersNear.size()));
                (*i)->GetSession()->SendPacket( &data );
            }
        }
        else
            player->ModifyMoney( pLoot->gold );
        pLoot->gold = 0;
        pLoot->NotifyMoneyRemoved();
    }
}

void WorldSession::HandleLootOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDebug("WORLD: CMSG_LOOT");

    uint64 guid;
    recv_data >> guid;

    GetPlayer()->SendLoot(guid, LOOT_CORPSE);
}

void WorldSession::HandleLootReleaseOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDebug("WORLD: CMSG_LOOT_RELEASE");
    Player  *player = GetPlayer();
    Loot    *loot;
    uint64   lguid;

    recv_data >> lguid;

    player->SetLootGUID(0);
    player->SendLootRelease(lguid);

    player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);

    if (IS_GAMEOBJECT_GUID(lguid))
    {
        GameObject *go =
            ObjectAccessor::GetGameObject(*player, lguid);

        // not check distance for GO in case owned GO (fishing bobber case, for example)
        if (!go || go->GetOwnerGUID() != _player->GetGUID() && !go->IsWithinDistInMap(_player,INTERACTION_DISTANCE))
            return;

        loot = &go->loot;

        if (go->GetGoType() == GAMEOBJECT_TYPE_DOOR)
        {
            // locked doors are opened with spelleffect openlock, prevent remove its as looted
            go->SetUInt32Value(GAMEOBJECT_FLAGS,33);
            go->SetUInt32Value(GAMEOBJECT_STATE,0);         //open
            go->SetLootState(GO_CLOSED);
            go->SetRespawnTime(5);                          //close door in 5 seconds
        }
        else if (loot->isLooted() || go->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE)
        {
            // GO is mineral vein? so it is not removed after its looted
            if(go->GetGoType() == GAMEOBJECT_TYPE_CHEST)
            {
                uint32 go_min = go->GetGOInfo()->data4;
                uint32 go_max = go->GetGOInfo()->data5;

                // only vein pass this check
                if(go_min != 0 && go_max > go_min)
                {
                    float amount_rate = sWorld.getRate(RATE_MINING_AMOUNT);
                    float min_amount = go_min*amount_rate;
                    float max_amount = go_max*amount_rate;

                    go->AddUse(player);
                    float uses = float(go->GetUseCount());

                    if(uses < max_amount)
                    {
                        if(uses >= min_amount)
                        {
                            float chance_rate = sWorld.getRate(RATE_MINING_NEXT);

                            int32 ReqValue = 175;
                            LockEntry const *lockInfo = sLockStore.LookupEntry(go->GetGOInfo()->data0);
                            if(lockInfo)
                                ReqValue = lockInfo->requiredminingskill;
                            float skill = float(player->GetSkillValue(SKILL_MINING))/(ReqValue+25);
                            double chance = pow(0.8*chance_rate,4*(1/double(max_amount))*double(uses));
                            if(roll_chance_f(100*chance+skill))
                            {
                                go->SetLootState(GO_CLOSED);
                            }
                            else                            // not have more uses
                                go->SetLootState(GO_LOOTED);
                        }
                        else                                // 100% chance until min uses
                            go->SetLootState(GO_CLOSED);
                    }
                    else                                    // max uses already
                        go->SetLootState(GO_LOOTED);
                }
                else                                        // not vein
                    go->SetLootState(GO_LOOTED);
            }
            else                                            // not chest (or vein/herb/etc)
                go->SetLootState(GO_LOOTED);

            loot->clear();
        }
        else
            // not fully looted object
            go->SetLootState(GO_OPEN);
    }
    else if (IS_CORPSE_GUID(lguid))        // ONLY remove insignia at BG
    {
        Corpse *corpse = ObjectAccessor::GetCorpse(*player, lguid);
        if (!corpse)
            return;

        loot = &corpse->loot;

        if (loot->isLooted())
        {
            loot->clear();
            corpse->RemoveFlag(CORPSE_FIELD_DYNAMIC_FLAGS, CORPSE_DYNFLAG_LOOTABLE);
        }
    }
    else if (IS_ITEM_GUID(lguid))
    {
        uint16 pos = player->GetPosByGuid( lguid );
        Item *pItem = player->GetItemByPos(pos);
        if(!pItem)
            return;
        if( (pItem->GetProto()->BagFamily & BAG_FAMILY_MASK_MINING_SUPP) &&
            pItem->GetProto()->Class == ITEM_CLASS_TRADE_GOODS &&
            pItem->GetCount() >= 5)
        {
            pItem->m_lootGenerated = false;
            pItem->loot.clear();

            uint32 count = 5;
            player->DestroyItemCount(pItem, count, true);
        }
        else
            // FIXME: item don't must be deleted in case not fully looted state. But this pre-request implement loot saving in DB at item save. Or checting possible.
            player->DestroyItem( (pos >> 8),(pos & 255), true);
        return;                                             // item can be looted only single player
    }
    else
    {
        Creature* pCreature = ObjectAccessor::GetCreature(*player, lguid);

        bool ok_loot = pCreature && pCreature->isAlive() == (player->getClass()==CLASS_ROGUE && pCreature->lootForPickPocketed);
        if ( !ok_loot || !pCreature->IsWithinDistInMap(_player,INTERACTION_DISTANCE) )
            return;

        loot = &pCreature->loot;

        Player *recipient = pCreature->GetLootRecipient();
        if (recipient && recipient->GetGroup())
        {
            if (recipient->GetGroup()->GetLooterGuid() == player->GetGUID())
                loot->released = true;
        }

        if (loot->isLooted())
        {
            //this is probably wrong
            pCreature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            loot->clear();
        }
    }

    //Player is not looking at loot list, he doesn't need to see updates on the loot list
    loot->RemoveLooter(player->GetGUID());
}
