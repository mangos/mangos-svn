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
#include "Log.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "LootMgr.h"
#include "Object.h"
#include "Group.h"

void WorldSession::HandleAutostoreLootItemOpcode( WorldPacket & recv_data )
{
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

        if (!go)
            return;

        loot = &go->loot;
    }
    else
    {
        Creature* pCreature =
            ObjectAccessor::Instance().GetCreature(*player, lguid);

        if (!pCreature)
            return;

        loot = &pCreature->loot;
    }

    WorldPacket data;
    LootItem *item = NULL;
    if (loot->items.size() > lootSlot)
    {
        item = &(loot->items[lootSlot]);
    }

    if ((item == NULL) || (item->is_looted))
    {
        player->SendEquipError( EQUIP_ERR_ALREADY_LOOTED, NULL, NULL );
        return;
    }

    if (item->is_blocked)
        return;

    uint16 dest;
    uint8 msg = player->CanStoreNewItem( 0, NULL_SLOT, dest, item->itemid, 1, false );
    if ( msg == EQUIP_ERR_OK )
    {
        player->StoreNewItem( dest, item->itemid, 1, true ,true);
        item->is_looted = true;

        loot->NotifyItemRemoved(lootSlot);

        data.Initialize( SMSG_ITEM_PUSH_RESULT );
        data << player->GetGUID();
        data << uint64(0x00000000);
        data << uint8(0x01);
        data << uint8(0x00);
        data << uint8(0x00);
        data << uint8(0x00);
        data << uint8(0xFF);
        data << uint32(item->itemid);
        data << uint64(0);

        /*data << uint8(0x00);
        data << uint8(0x00);
        data << uint8(0x00);
        data << uint32(0x00000000);
        data << uint8(0x00);*/
        SendPacket( &data );
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
        if( pGameObject )
            pLoot = &pGameObject->loot;
    }
    else
    {
        Creature* pCreature = ObjectAccessor::Instance().GetCreature(*GetPlayer(), guid);
        if ( pCreature )
            pLoot = &pCreature->loot ;
    }

    if( pLoot )
    {
        if (player->IsInGroup())
        {
            WorldPacket data;
            Group *group = objmgr.GetGroupByLeader(player->GetGroupLeader());
            uint32 iMembers = group->GetMembersCount();

            // it is probably more costly to call getplayer for each member
            // than temporarily storing them in a vector
            std::vector<Player*> playersNear;
            playersNear.reserve(iMembers);
            for (int i=0; i<iMembers; i++)
            {
                Player* playerGroup = objmgr.GetPlayer(group->GetMemberGUID(i));
                if (player->GetDistance2dSq(playerGroup) < sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE))
                    playersNear.push_back(playerGroup);
            }

            for (std::vector<Player*>::iterator i = playersNear.begin(); i != playersNear.end(); ++i)
            {
                (*i)->ModifyMoney( uint32((pLoot->gold)/(playersNear.size())) );
                //Offset surely incorrect, but works
                data.Initialize( SMSG_LOOT_MONEY_NOTIFY );
                data << uint32((pLoot->gold)/(playersNear.size()));
                (*i)->GetSession()->SendPacket( &data );
            }
        }
        else
        {
            player->ModifyMoney( pLoot->gold );
        }
        pLoot->gold = 0;
        pLoot->NotifyMoneyRemoved();
    }
}

void WorldSession::HandleLootOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_LOOT");
    uint64 guid;
    recv_data >> guid;
    GetPlayer()->SendLoot(guid, LOOT_CORPSE);
}

void WorldSession::HandleLootReleaseOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_LOOT_RELEASE");
    Player  *player = GetPlayer();
    Loot    *loot;
    uint64   lguid;

    recv_data >> lguid;

    player->SetLootGUID(0);

    WorldPacket data;
    data.Initialize( SMSG_LOOT_RELEASE_RESPONSE );
    data << lguid << uint8(1);
    SendPacket( &data );

    if (IS_GAMEOBJECT_GUID(lguid))
    {
        GameObject *go =
            ObjectAccessor::Instance().GetGameObject(*player, lguid);

        if (!go)
            return;

        loot = &go->loot;

        vector<LootItem>::iterator i;
        i = find_if(loot->items.begin(), loot->items.end(),
            LootItem::not_looted);

        if((i == loot->items.end()) && (loot->gold == 0))
        {
            go->SetLootState(GO_LOOTED);
            loot->clear();
        }
        else
            go->SetLootState(GO_OPEN);
    }
    else
    {
        Creature* pCreature =
            ObjectAccessor::Instance().GetCreature(*player, lguid);

        if (!pCreature)
            return;

        loot = &pCreature->loot;

        Player *recipient = pCreature->GetLootRecipient();
        if (recipient && recipient->IsInGroup())
        {
            Group *group = objmgr.GetGroupByLeader(recipient->GetGroupLeader());
            if (group->GetLooterGuid() == player->GetGUID())
                loot->released = true;
        }

        vector<LootItem>::iterator i;
        i = find_if(loot->items.begin(), loot->items.end(),
            LootItem::not_looted);

        if((i == loot->items.end()) && (loot->gold == 0))
        {
            //this is probably wrong
            pCreature->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
            if(pCreature->GetCreatureInfo()->SkinLootId)
                pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
            loot->clear();
        }
    }

    //Player is not looking at loot list, he doesn't need to see updates on the loot list
    loot->RemoveLooter(player);
}
