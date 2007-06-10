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
#include "Log.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "LootMgr.h"
#include "Object.h"
#include "Group.h"

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
            ObjectAccessor::Instance().GetGameObject(*player, lguid);

        // not check distance for GO in case owned GO (fishing bobber case, for example)
        if (!go || go->GetOwnerGUID() != _player->GetGUID() && !go->IsWithinDistInMap(_player,OBJECT_ITERACTION_DISTANCE))
            return;

        loot = &go->loot;
    }
    else if (IS_ITEM_GUID(lguid))
    {
        Item *pItem = player->GetItemByPos( player->GetPosByGuid( lguid ));

        if (!pItem)
            return;

        loot = &pItem->loot;
    }
    else
    {
        Creature* pCreature =
            ObjectAccessor::Instance().GetCreature(*player, lguid);

        bool ok_loot = pCreature && pCreature->isAlive() == (player->getClass()==CLASS_ROGUE && pCreature->lootForPickPocketed);

        if( !ok_loot || !pCreature->IsWithinDistInMap(_player,OBJECT_ITERACTION_DISTANCE) )
            return;

        loot = &pCreature->loot;
    }

    LootItem *item = NULL;
    QuestItem *qitem = NULL;
    bool is_looted = true, is_qitem = false;
    if (lootSlot >= loot->items.size())
    {
        lootSlot -= loot->items.size();
        QuestItemMap::iterator itr = loot->PlayerQuestItems.find(player);
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
    }

    if ((item == NULL) || is_looted)
    {
        player->SendEquipError( EQUIP_ERR_ALREADY_LOOTED, NULL, NULL );
        return;
    }

    // questitems use the blocked field for other purposes
    if (!qitem && item->is_blocked)
        return;

    uint16 dest;
    uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, item->itemid, item->count, false );
    if ( msg == EQUIP_ERR_OK )
    {
        Item * newitem = player->StoreNewItem( dest, item->itemid, item->count, true, item->randomPropertyId);

        if (qitem)
        {
            qitem->is_looted = true;
            if (!item->is_ffa || loot->PlayerQuestItems.size() == 1)
                player->SendNotifyLootItemRemoved(loot->items.size() + lootSlot);
            else
                loot->NotifyQuestItemRemoved(qitem->index);
        }
        else
            loot->NotifyItemRemoved(lootSlot);

        //if (item->is_ffa) item->is_looted = true;
        item->is_looted = true;
        loot->unlootedCount--;

        player->SendNewItem(newitem, uint32(item->count), false, false, true);
    }
    else
        player->SendEquipError( msg, NULL, NULL );
}

void WorldSession::HandleLootMoneyOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_LOOT_MONEY");

    Player *player = GetPlayer();
    uint64 guid = player->GetLootGUID();
    Loot *pLoot = NULL;

    if( IS_GAMEOBJECT_GUID( guid ) )
    {
        GameObject *pGameObject = ObjectAccessor::Instance().GetGameObject(*GetPlayer(), guid);

        // not check distance for GO in case owned GO (fishing bobber case, for example)
        if( pGameObject && (pGameObject->GetOwnerGUID()==_player->GetGUID() || pGameObject->IsWithinDistInMap(_player,OBJECT_ITERACTION_DISTANCE)) )
            pLoot = &pGameObject->loot;
    }
    else
    {
        Creature* pCreature = ObjectAccessor::Instance().GetCreature(*GetPlayer(), guid);

        bool ok_loot = pCreature && pCreature->isAlive() == (player->getClass()==CLASS_ROGUE && pCreature->lootForPickPocketed);

        if ( ok_loot && pCreature->IsWithinDistInMap(_player,OBJECT_ITERACTION_DISTANCE) )
            pLoot = &pCreature->loot ;
    }

    if( pLoot )
    {
        if (player->groupInfo.group)
        {
            Group *group = player->groupInfo.group;

            std::vector<Player*> playersNear;
            Group::MemberList const& members = group->GetMembers();
            for(Group::member_citerator itr = members.begin(); itr != members.end(); ++itr)
            {
                Player* playerGroup = objmgr.GetPlayer(itr->guid);
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

    WorldPacket data( SMSG_LOOT_RELEASE_RESPONSE, (8+1) );
    data << lguid << uint8(1);
    SendPacket( &data );

    if (IS_GAMEOBJECT_GUID(lguid))
    {
        GameObject *go =
            ObjectAccessor::Instance().GetGameObject(*player, lguid);

        // not check distance for GO in case owned GO (fishing bobber case, for example)
        if (!go || go->GetOwnerGUID() != _player->GetGUID() && !go->IsWithinDistInMap(_player,OBJECT_ITERACTION_DISTANCE))
            return;

        loot = &go->loot;

        if (go->GetGoType() == GAMEOBJECT_TYPE_DOOR)
        {
            // locked doors are opened with spelleffect openlock, prevent remove its as looted
            go->SetUInt32Value(GAMEOBJECT_FLAGS,33);
            go->SetUInt32Value(GAMEOBJECT_STATE,0);        //open
            go->SetLootState(GO_CLOSED);
            go->SetRespawnTime(5);                         //close door in 5 seconds                
        }
        else if (loot->isLooted() || go->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE)
        {
            // GO is mineral vein? so it is not removed after its looted
            if(go->GetGoType() == GAMEOBJECT_TYPE_CHEST) 
            { 
                uint32 go_min = go->GetGOInfo()->sound4;
                uint32 go_max = go->GetGOInfo()->sound5;

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
                            LockEntry const *lockInfo = sLockStore.LookupEntry(go->GetGOInfo()->sound0);
                            if(lockInfo)
                                ReqValue = lockInfo->requiredskill;    
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
    else if (IS_ITEM_GUID(lguid))
    {
        uint16 pos = player->GetPosByGuid( lguid );
        player->DestroyItem( (pos >> 8),(pos & 255), true);
        return;                                             // item can be looted only single player
    }
    else
    {
        Creature* pCreature =
            ObjectAccessor::Instance().GetCreature(*player, lguid);

        bool ok_loot = pCreature && pCreature->isAlive() == (player->getClass()==CLASS_ROGUE && pCreature->lootForPickPocketed);
        if ( !ok_loot || !pCreature->IsWithinDistInMap(_player,OBJECT_ITERACTION_DISTANCE) )
            return;

        loot = &pCreature->loot;

        Player *recipient = pCreature->GetLootRecipient();
        if (recipient && recipient->groupInfo.group)
        {
            if (recipient->groupInfo.group->GetLooterGuid() == player->GetGUID())
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
