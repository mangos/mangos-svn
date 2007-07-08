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
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "Log.h"
#include "Unit.h"
#include "ObjectAccessor.h"
#include "GossipDef.h"
#include "Language.h"

bool ChatHandler::HandleDebugInArcCommand(const char* args)
{
    Object *obj = getSelectedUnit();

    if(!obj)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    SendSysMessage(LANG_NOT_IMPLEMENTED);

    return true;
}

bool ChatHandler::HandleDebugSpellFailCommand(const char* args)
{
    if(!args)
        return false;

    char* px = strtok((char*)args, " ");
    if(!px)
        return false;

    uint8 failnum = (uint8)atoi(px);

    WorldPacket data(SMSG_CAST_RESULT, 6);
    data << (uint32)133;
    data << uint8(2);
    data << failnum;
    m_session->SendPacket(&data);

    return true;
}

bool ChatHandler::HandleSetPoiCommand(const char* args)
{
    Player *pPlayer = m_session->GetPlayer();
    Unit* target = getSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    if(!args)
        return false;

    char* icon_text = strtok((char*)args, " ");
    char* flags_text = strtok(NULL, " ");
    if(!icon_text || !flags_text)
        return false;

    uint32 icon = atol(icon_text);
    if ( icon < 0 )
        icon = 0;

    uint32 flags = atol(flags_text);

    sLog.outDetail("Command : POI, NPC = %u, icon = %u flags = %u", target->GetGUIDLow(), icon,flags);
    pPlayer->PlayerTalkClass->SendPointOfInterest(target->GetPositionX(), target->GetPositionY(), Poi_Icon(icon), flags, 30, "Test POI");
    return true;
}

bool ChatHandler::HandleSendItemErrorMsg(const char* args)
{
    uint8 error_msg = atol((char*)args);
    if(error_msg > 0)
        m_session->GetPlayer()->SendEquipError(error_msg, 0, 0);
    return true;
}

bool ChatHandler::HandleSendQuestPartyMsgCommand(const char* args)
{
    uint32 msg = atol((char*)args);
    if (msg >= 0)
        m_session->GetPlayer()->SendPushToPartyResponse(m_session->GetPlayer(), msg);
    return true;
}

bool ChatHandler::HandleSendQuestInvalidMsgCommand(const char* args)
{
    uint32 msg = atol((char*)args);
    if (msg >= 0)
        m_session->GetPlayer()->SendCanTakeQuestResponse(msg);
    return true;
}

bool ChatHandler::HandleGetItemState(const char* args)
{
    if (!args)
        return false;

    std::string state_str = args;

    ItemUpdateState state;
    bool list_queue = false, check_all = false;
    if (state_str == "unchanged") state = ITEM_UNCHANGED;
    else if (state_str == "changed") state = ITEM_CHANGED;
    else if (state_str == "new") state = ITEM_NEW;
    else if (state_str == "removed") state = ITEM_REMOVED;
    else if (state_str == "queue") list_queue = true;
    else if (state_str == "check_all") check_all = true;
    else return false;

    Player* player = getSelectedPlayer();
    if (!player) player = m_session->GetPlayer();

    if (!list_queue && !check_all)
    {
        state_str = "The player has the following " + state_str + " items: ";
        SendSysMessage(state_str.c_str());
        for (uint8 i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; i++)
        {
            if(i >= BUYBACK_SLOT_START && i < BUYBACK_SLOT_END)
                continue;

            Item *item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (!item) continue;
            if (!item->IsBag())
            {
                if (item->GetState() == state)
                    PSendSysMessage("bag: 255 slot: %d guid: %d owner: %d", item->GetSlot(), item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()));
            }
            else
            {
                Bag *bag = (Bag*)item;
                const ItemPrototype *proto = bag->GetProto();
                for (uint8 i = 0; i < proto->ContainerSlots; i++)
                {
                    Item* item = bag->GetItemByPos(i);
                    if (item && item->GetState() == state)
                        PSendSysMessage("bag: 255 slot: %d guid: %d owner: %d", item->GetSlot(), item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()));
                }
            }
        }
    }

    if (list_queue)
    {
        std::vector<Item *> &updateQueue = player->GetItemUpdateQueue();
        for(size_t i = 0; i < updateQueue.size(); i++)
        {
            Item *item = updateQueue[i];
            if(!item) continue;

            Bag *container = item->GetContainer();
            uint8 bag_slot = container ? container->GetSlot() : INVENTORY_SLOT_BAG_0;

            std::string st;
            switch(item->GetState())
            {
                case ITEM_UNCHANGED: st = "unchanged"; break;
                case ITEM_CHANGED: st = "changed"; break;
                case ITEM_NEW: st = "new"; break;
                case ITEM_REMOVED: st = "removed"; break;
            }

            PSendSysMessage("bag: %d slot: %d guid: %d - state: %s", bag_slot, item->GetSlot(), item->GetGUIDLow(), st.c_str());
        }
        if (!updateQueue.size())
            PSendSysMessage("updatequeue empty");
    }

    if (check_all)
    {
        bool error = false;
        std::vector<Item *> &updateQueue = player->GetItemUpdateQueue();
        for (uint8 i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; i++)
        {
            if(i >= BUYBACK_SLOT_START && i < BUYBACK_SLOT_END)
                continue;

            Item *item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (!item) continue;

            if (item->GetSlot() != i)
            {
                PSendSysMessage("item at slot %d, guid %d has an incorrect slot value: %d", i, item->GetGUIDLow(), item->GetSlot());
                error = true; continue;
            }

            if (item->GetOwnerGUID() != player->GetGUID())
            {
                PSendSysMessage("for the item at slot %d and itemguid %d, owner's guid (%d) and player's guid (%d) don't match!", item->GetSlot(), item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()), player->GetGUIDLow());
                error = true; continue;
            }

            Bag *container = item->GetContainer();
            if (container)
            {
                PSendSysMessage("item at slot: %d guid: %d has a container (slot: %d, guid: %d) but shouldnt!", item->GetSlot(), item->GetGUIDLow(), container->GetSlot(), container->GetGUIDLow());
                error = true; continue;
            }

            if (item->IsInUpdateQueue())
            {
                uint16 qp = item->GetQueuePos();
                if (qp > updateQueue.size())
                {
                    PSendSysMessage("item at slot: %d guid: %d has a queuepos (%d) larger than the update queue size! ", item->GetSlot(), item->GetGUIDLow(), qp);
                    error = true; continue;
                }

                if (updateQueue[qp] == NULL)
                {
                    PSendSysMessage("item at slot: %d guid: %d has a queuepos (%d) that points to NULL in the queue!", item->GetSlot(), item->GetGUIDLow(), qp);
                    error = true; continue;
                }

                if (updateQueue[qp] != item)
                {
                    PSendSysMessage("item at slot: %d guid: %d has has a queuepos (%d) that points to another item in the queue (bag: %d, slot: %d, guid: %d)", item->GetSlot(), item->GetGUIDLow(), qp, updateQueue[qp]->GetBagSlot(), updateQueue[qp]->GetSlot(), updateQueue[qp]->GetGUIDLow());
                    error = true; continue;
                }
            }
            else if (item->GetState() != ITEM_UNCHANGED)
            {
                PSendSysMessage("item at slot: %d guid: %d is not in queue but should be (state: %d)!", item->GetSlot(), item->GetGUIDLow(), item->GetState());
                error = true; continue;
            }

            if(item->IsBag())
            {
                Bag *bag = (Bag*)item;
                const ItemPrototype *proto = bag->GetProto();
                for (uint8 i = 0; i < proto->ContainerSlots; i++)
                {
                    Item* item = bag->GetItemByPos(i);
                    if (!item) continue;

                    if (item->GetSlot() != i)
                    {
                        PSendSysMessage("the item in bag %d slot %d, guid %d has an incorrect slot value: %d", bag->GetSlot(), i, item->GetGUIDLow(), item->GetSlot());
                        error = true; continue;
                    }

                    if (item->GetOwnerGUID() != player->GetGUID())
                    {
                        PSendSysMessage("for the item in bag %d at slot %d and itemguid %d, owner's guid (%d) and player's guid (%d) don't match!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()), player->GetGUIDLow());
                        error = true; continue;
                    }

                    Bag *container = item->GetContainer();
                    if (!container)
                    {
                        PSendSysMessage("the item in bag %d at slot %d with guid %d has no container!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow());
                        error = true; continue;
                    }

                    if (container != bag)
                    {
                        PSendSysMessage("the item in bag %d at slot %d with guid %d has a different container(slot %d guid %d)!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), container->GetSlot(), container->GetGUIDLow());
                        error = true; continue;
                    }

                    if (item->IsInUpdateQueue())
                    {
                        uint16 qp = item->GetQueuePos();
                        if (qp > updateQueue.size())
                        {
                            PSendSysMessage("item in bag: %d at slot: %d guid: %d has a queuepos (%d) larger than the update queue size! ", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), qp);
                            error = true; continue;
                        }

                        if (updateQueue[qp] == NULL)
                        {
                            PSendSysMessage("item in bag: %d at slot: %d guid: %d has a queuepos (%d) that points to NULL in the queue!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), qp);
                            error = true; continue;
                        }

                        if (updateQueue[qp] != item)
                        {
                            PSendSysMessage("item in bag: %d at slot: %d guid: %d has has a queuepos (%d) that points to another item in the queue (bag: %d, slot: %d, guid: %d)", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), qp, updateQueue[qp]->GetBagSlot(), updateQueue[qp]->GetSlot(), updateQueue[qp]->GetGUIDLow());
                            error = true; continue;
                        }
                    }
                    else if (item->GetState() != ITEM_UNCHANGED)
                    {
                        PSendSysMessage("item in bag: %d at slot: %d guid: %d is not in queue but should be (state: %d)!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), item->GetState());
                        error = true; continue;
                    }
                }
            }
        }

        for(size_t i = 0; i < updateQueue.size(); i++)
        {
            Item *item = updateQueue[i];
            if(!item) continue;

            if (item->GetOwnerGUID() != player->GetGUID())
            {
                PSendSysMessage("queue(%d): for the an item (guid %d), the owner's guid (%d) and player's guid (%d) don't match!", i, item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()), player->GetGUIDLow());
                error = true; continue;
            }

            if (item->GetQueuePos() != i)
            {
                PSendSysMessage("queue(%d): for the an item (guid %d), the queuepos doesn't match it's position in the queue!", i, item->GetGUIDLow());
                error = true; continue;
            }

            if (item->GetState() == ITEM_REMOVED) continue;
            Item *test = player->GetItemByPos( item->GetBagSlot(), item->GetSlot());

            if (test == NULL)
            {
                PSendSysMessage("queue(%d): the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the player doesn't have an item at that position!", i, item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow());
                error = true; continue;
            }

            if (test != item)
            {
                PSendSysMessage("queue(%d): the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the item with guid %d is there instead!", i, item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow(), test->GetGUIDLow());
                error = true; continue;
            }
        }
        if (!error)
            SendSysMessage("All OK!");
    }

    return true;
}
