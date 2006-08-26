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
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Item.h"
#include "GameObject.h"
#include "Opcodes.h"
#include "Chat.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "Language.h"
#include "World.h"

bool ChatHandler::HandleGoObjectCommand(const char* args)
{
    if(!*args)
        return false;

    int32 guid = atoi((char*)args);
    if(!guid)
        return false;

    QueryResult *result = sDatabase.PQuery("SELECT `position_x`,`position_y`,`position_z`,`orientation`,`map` FROM `gameobject` WHERE `guid` = '%i'",guid);
    if (!result)
    {
        SendSysMessage("Object not found!");
        return true;
    }

    Field *fields = result->Fetch();
    float x = fields[0].GetFloat();
    float y = fields[1].GetFloat();
    float z = fields[2].GetFloat();
    float ort = fields[3].GetFloat();
    int mapid = fields[4].GetUInt16();
    delete result;

    if(!MapManager::ExistMAP(mapid,x,y))
    {
        PSendSysMessage("target map not exist (X: %f Y: %f MapId:%u)",x,y,mapid);
        return true;
    }

    m_session->GetPlayer()->TeleportTo(mapid, x, y, z, ort);
    return true;
}

bool ChatHandler::HandleGoCreatureCommand(const char* args)
{
    if(!*args)
        return false;

    int32 guid = atoi((char*)args);
    if(!guid)
        return false;

    QueryResult *result = sDatabase.PQuery("SELECT `position_x`,`position_y`,`position_z`,`orientation`,`map` FROM `creature` WHERE `guid` = '%i'",guid);
    if (!result)
    {
        SendSysMessage("Creature not found!");
        return true;
    }

    Field *fields = result->Fetch();
    float x = fields[0].GetFloat();
    float y = fields[1].GetFloat();
    float z = fields[2].GetFloat();
    float ort = fields[3].GetFloat();
    int mapid = fields[4].GetUInt16();
    
    delete result;

    if(!MapManager::ExistMAP(mapid,x,y))
    {
        PSendSysMessage("target map not exist (X: %f Y: %f MapId:%u)",x,y,mapid);
        return true;
    }

    m_session->GetPlayer()->TeleportTo(mapid, x, y, z, ort);
    return true;
}


bool ChatHandler::HandleGUIDCommand(const char* args)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();

    if (guid == 0)
    {
        SendSysMessage(LANG_NO_SELECTION);
        return true;
    }

    PSendSysMessage(LANG_OBJECT_GUID, GUID_LOPART(guid), GUID_HIPART(guid));
    return true;
}

bool ChatHandler::HandleNameCommand(const char* args)
{
    /* Temp. disabled
        if(!*args)
            return false;

        if(strlen((char*)args)>75)
        {
            PSendSysMessage(LANG_TOO_LONG_NAME, strlen((char*)args)-75);
            return true;
        }

        for (uint8 i = 0; i < strlen(args); i++)
        {
            if(!isalpha(args[i]) && args[i]!=' ')
            {
                SendSysMessage(LANG_CHARS_ONLY);
                return false;
            }
        }

        uint64 guid;
        guid = m_session->GetPlayer()->GetSelection();
        if (guid == 0)
        {
            SendSysMessage(LANG_NO_SELECTION);
            return true;
        }

        Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

        if(!pCreature)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        pCreature->SetName(args);
        uint32 idname = objmgr.AddCreatureTemplate(pCreature->GetName());
        pCreature->SetUInt32Value(OBJECT_FIELD_ENTRY, idname);

        pCreature->SaveToDB();
    */

    return true;
}

bool ChatHandler::HandleSubNameCommand(const char* args)
{
    /* Temp. disabled

    if(!*args)
        args = "";

    if(strlen((char*)args)>75)
    {

        PSendSysMessage(LANG_TOO_LONG_SUBNAME, strlen((char*)args)-75);
        return true;
    }

    for (uint8 i = 0; i < strlen(args); i++)
    {
        if(!isalpha(args[i]) && args[i]!=' ')
        {
            SendSysMessage(LANG_CHARS_ONLY);
            return false;
        }
    }
    uint64 guid;
    guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SendSysMessage(LANG_NO_SELECTION);
        return true;
    }

    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    uint32 idname = objmgr.AddCreatureSubName(pCreature->GetName(),args,pCreature->GetUInt32Value(UNIT_FIELD_DISPLAYID));
    pCreature->SetUInt32Value(OBJECT_FIELD_ENTRY, idname);

    pCreature->SaveToDB();
    */
    return true;
}

bool ChatHandler::HandleNYICommand(const char* args)
{
    SendSysMessage(LANG_NOT_IMPLEMENTED);
    return true;
}

bool ChatHandler::HandleProgCommand(const char* args)
{
    m_session->GetPlayer()->TeleportTo(451, 16391.80f, 16341.20f, 69.44f,0.0f);

    return true;
}

bool ChatHandler::HandleItemMoveCommand(const char* args)
{
    uint8 srcslot, dstslot;

    char* pParam1 = strtok((char*)args, " ");
    if (!pParam1)
        return false;

    char* pParam2 = strtok(NULL, " ");
    if (!pParam2)
        return false;

    srcslot = (uint8)atoi(pParam1);
    dstslot = (uint8)atoi(pParam2);

    uint16 src = ((INVENTORY_SLOT_BAG_0 << 8) | srcslot);
    uint16 dst = ((INVENTORY_SLOT_BAG_0 << 8) | dstslot);

    m_session->GetPlayer()->SwapItem( src, dst );

    return true;
}

bool ChatHandler::HandleSpawnCommand(const char* args)
{
    char* pEntry = strtok((char*)args, " ");
    if (!pEntry)
        return false;

    char* pFlags = strtok(NULL, " ");
    if (!pFlags)
        return false;

    char* pFaction = strtok(NULL, " ");
    if (!pFaction)
        return false;

    char* pLevel = strtok(NULL, " ");
    if (!pLevel)
        return false;

    char* pName = strtok(NULL, "%");
    if (!pName)
        return false;

    uint32 npcFlags  = atoi(pFlags);
    uint32 faction_id  = atoi(pFaction);
    uint32 level  = atoi(pLevel);
    uint32 display_id = atoi(pEntry);

    if (display_id==0)
        return false;

    for (uint8 i = 0; i < strlen(pName); i++)
    {
        if(!isalpha(pName[i]) && pName[i]!=' ')
        {
            SendSysMessage(LANG_CHARS_ONLY);
            return false;
        }
    }
    SpawnCreature(m_session, pName, display_id, npcFlags, faction_id, level);

    return true;
}

bool ChatHandler::HandleAddSpwCommand(const char* args)
{
    char* charID = strtok((char*)args, " ");
    if (!charID)
        return false;

    uint32 id  = atoi(charID);

    //QueryResult *result = sDatabase.PQuery("SELECT `modelid`,`flags`,`faction`,`level`,`name` FROM `creature_template` WHERE `entry` = '%u'", id);

    //if(result)
    //{
    //Field *fields = result->Fetch();

    //WorldPacket data;

    Player *chr = m_session->GetPlayer();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();

    Creature* pCreature = new Creature;
    if (!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), chr->GetMapId(), x, y, z, o, id))
    {
        delete pCreature;
        return false;
    }

    pCreature->AIM_Initialize();
    //pCreature->SetUInt32Value(UNIT_FIELD_HEALTH , 1); // temp set on 1 HP needs to be MAX HP (strange error)

    sLog.outDebug(LANG_ADD_OBJ);

    MapManager::Instance().GetMap(pCreature->GetMapId())->Add(pCreature);
    pCreature->SaveToDB();

    //delete result;
    return true;
    //}
    //else
    //    delete result;
    //return false;
}

bool ChatHandler::HandleDeleteCommand(const char* args)
{
    Creature *unit = getSelectedCreature();
    if(!unit)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    unit->AttackStop();
    unit->RemoveAllAttackers();

    unit->DeleteFromDB();

    ObjectAccessor::Instance().RemoveCreatureFromPlayerView(m_session->GetPlayer(),unit);
    ObjectAccessor::Instance().AddObjectToRemoveList(unit);

    SendSysMessage("Creature Removed");

    return true;
}

bool ChatHandler::HandleDeMorphCommand(const char* args)
{
    sLog.outError(LANG_DEMORPHED,m_session->GetPlayer()->GetName());
    m_session->GetPlayer()->DeMorph();
    return true;
}

bool ChatHandler::HandleItemCommand(const char* args)
{
    /*
        char* pitem = strtok((char*)args, " ");
        if (!pitem)
            return false;

        uint64 guid = m_session->GetPlayer()->GetSelection();
        if (guid == 0)
        {
            SendSysMessage(LANG_NO_SELECTION);
            return true;
        }

        Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

        if(!pCreature)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        uint32 item = atoi(pitem);
        int amount = -1;

        char* pamount = strtok(NULL, " ");
        if (pamount)
            amount = atoi(pamount);

        ItemPrototype* tmpItem = objmgr.GetItemPrototype(item);

        if(tmpItem)
        {
            QueryResult *result = sDatabase.PQuery("INSERT INTO `npc_vendor` (`entry`,`itemguid`,`amount`) VALUES('%u','%u','%d')",pCreature->GetEntry(), item, amount);

            uint8 itemscount = pCreature->GetItemCount();
            pCreature->setItemId(itemscount , item);
            pCreature->setItemAmount(itemscount , amount);
            pCreature->IncrItemCount();
            PSendSysMessage(LANG_ITEM_ADDED_TO_LIST,item,tmpItem->Name1);
            delete result;
        }
        else
        {
            PSendSysMessage(LANG_ITEM_NOT_FOUND,item);
        }
    */
    return true;
}

bool ChatHandler::HandleItemRemoveCommand(const char* args)
{
    /*
        char* iguid = strtok((char*)args, " ");
        if (!iguid)
            return false;

        uint64 guid = m_session->GetPlayer()->GetSelection();
        if (guid == 0)
        {
            SendSysMessage(LANG_NO_SELECTION);
            return true;
        }

        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

        if(!pCreature)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        uint32 itemguid = atoi(iguid);
        int slot = pCreature->GetItemSlot(itemguid);

        if(slot != -1)
        {
            uint32 guidlow = GUID_LOPART(guid);

            sDatabase.PExecute("DELETE FROM `npc_vendor` WHERE `entry` = '%u' AND `itemguid` = '%u'",pCreature->GetEntry(),itemguid);

            pCreature->setItemId(slot , 0);
            pCreature->setItemAmount(slot , 0);
            ItemPrototype* tmpItem = objmgr.GetItemPrototype(itemguid);
            if(tmpItem)
            {
                PSendSysMessage(LANG_ITEM_DELETED_FROM_LIST,itemguid,tmpItem->Name1);
            }
            else
            {
                PSendSysMessage(LANG_ITEM_DELETED_FROM_LIST,itemguid,"<unknonwn>");
            }

        }
        else
        {
            PSendSysMessage(LANG_ITEM_NOT_IN_LIST,itemguid);
        }
    */
    return true;
}

bool ChatHandler::HandleAddMoveCommand(const char* args)
{
    Creature* pCreature = getSelectedCreature();

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    // changed 'creatureId' to lowercase
    // changed 'X', 'y', 'Z' to 'positionx', 'positiony', 'positionz'
    sDatabase.PExecute("INSERT INTO `creature_movement` (`id`,`position_x`,`position_y`,`position_z`) VALUES ('%u', '%f', '%f', '%f')", pCreature->GetGUIDLow(), m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ());

    SendSysMessage(LANG_WAYPOINT_ADDED);

    return true;
}

bool ChatHandler::HandleRandomCommand(const char* args)
{
    if(!*args)
        return false;

    int option = atoi((char*)args);

    if (option != 0 && option != 1)
    {
        //m_session->GetPlayer( )->SendMessageToSet( &data, true );
        SendSysMessage(LANG_USE_BOL);
        return true;
    }

    Creature* pCreature = getSelectedCreature();

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    // fix me : 'moverandom' doesn't exist in https://svn.mangosproject.org/trac/MaNGOS/wiki/Database/creature ?
    // perhaps it should be 'state'?
    sDatabase.PExecute("UPDATE `creature` SET `moverandom` = '%i' WHERE `guid` = '%u'", option, pCreature->GetGUIDLow());

    pCreature->setMoveRandomFlag(option > 0);

    SendSysMessage(LANG_VALUE_SAVED);

    return true;
}

bool ChatHandler::HandleRunCommand(const char* args)
{
    if(!*args)
        return false;

    int option = atoi((char*)args);

    if(option != 0 && option != 1)
    {
        SendSysMessage(LANG_USE_BOL);
        return true;
    }

    Creature* pCreature = getSelectedCreature();

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    // fix me : 'running' doesn't exist in https://svn.mangosproject.org/trac/MaNGOS/wiki/Database/creatures ?
    // perhaps it should be 'state'?
    sDatabase.PExecute("UPDATE `creature` SET `running` = '%i' WHERE `guid` = '%u'", option, pCreature->GetGUIDLow());

    pCreature->setMoveRunFlag(option > 0);

    SendSysMessage(LANG_VALUE_SAVED);
    return true;
}

bool ChatHandler::HandleChangeLevelCommand(const char* args)
{
    if (!*args)
        return false;

    uint8 lvl = (uint8) atoi((char*)args);
    if ( lvl < 1 || lvl > sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL) + 3)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Creature* pCreature = getSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    pCreature->SetHealth( 100 + 30*lvl);
    pCreature->SetMaxHealth( 100 + 30*lvl);
    pCreature->SetLevel( lvl);

    pCreature->SaveToDB();

    return true;
}

bool ChatHandler::HandleNPCFlagCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 npcFlags = (uint32) atoi((char*)args);

    Creature* pCreature = getSelectedCreature();

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    pCreature->SetUInt32Value(UNIT_NPC_FLAGS, npcFlags);

    pCreature->SaveToDB();

    SendSysMessage(LANG_VALUE_SAVED_REJOIN);

    uint32 entry = pCreature->GetUInt32Value( OBJECT_FIELD_ENTRY );
    m_session->SendCreatureQuery( entry, pCreature->GetGUID() );

    return true;
}

bool ChatHandler::HandleDisplayIdCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 displayId = (uint32) atoi((char*)args);

    Creature *pCreature = getSelectedCreature();

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    pCreature->SetUInt32Value(UNIT_FIELD_DISPLAYID, displayId);

    pCreature->SaveToDB();

    return true;
}

bool ChatHandler::HandleFactionIdCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 factionId = (uint32) atoi((char*)args);

    Creature* pCreature = getSelectedCreature();

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    pCreature->setFaction(factionId);

    pCreature->SaveToDB();

    return true;
}

bool ChatHandler::HandleKickPlayerCommand(const char *args)
{
    char *kickName;

    char* px = strtok((char*)args, " ");
    if (!px)
        return false;

    int x=0;
    while(px[x]==' ')
        x++;
    kickName=&px[x];

    if (strlen(kickName) == 0)
    {
        return false;
    }

    sWorld.KickPlayer(kickName);
    return true;
}

void ChatHandler::ShowTicket(uint64 guid, uint32 category, char const* text)
{
    std::string name;
    objmgr.GetPlayerNameByGUID(guid,name);

    if(name=="") name = " <unknown> ";

    PSendSysMessage("Ticket of %s (Category: %i):\n%s\n", name.c_str(),category,text);
}

bool ChatHandler::HandleTicketCommand(const char* args)
{
    char* px = strtok((char*)args, " ");

    // ticket<end>
    if (!px)
    {
        QueryResult *result = sDatabase.Query("SELECT `ticket_id` FROM `character_ticket`");
        size_t count = result ? result->GetRowCount() : 0;

        PSendSysMessage("Tickets count: %i show new tickets: %s\n", count,m_session->GetPlayer()->isAcceptTickets() ?  "on" : "off");
        delete result;
        return true;
    }

    // ticket on
    if(strncmp(px,"on",3) == 0)
    {
        m_session->GetPlayer()->SetAcceptTicket(true);
        SendSysMessage("New ticket show: on");
        return true;
    }

    // ticket off
    if(strncmp(px,"off",4) == 0)
    {
        m_session->GetPlayer()->SetAcceptTicket(false);
        SendSysMessage("New ticket show: off");
        return true;
    }

    // ticket #num
    int num = atoi(px);
    if(num > 0)
    {
        QueryResult *result = sDatabase.Query("SELECT `guid`,`ticket_category`,`ticket_text` FROM `character_ticket`");

        if(!result || num > result->GetRowCount())
        {
            PSendSysMessage("Ticket %i doesn't exist", num);
            delete result;
            return true;
        }

        for(int i = 1; i < num; ++i)
            result->NextRow();

        Field* fields = result->Fetch();

        uint64 guid = fields[0].GetUInt64();
        uint32 category = fields[1].GetUInt32();
        char const* text = fields[2].GetString();

        ShowTicket(guid,category,text);
        delete result;
        return true;
    }

    uint64 guid = objmgr.GetPlayerGUIDByName(px);

    if(!guid)
        return false;

    // ticket $char_name
    QueryResult *result = sDatabase.PQuery("SELECT `guid`,`ticket_category`,`ticket_text` FROM `character_ticket` WHERE `guid` = '%u'",GUID_LOPART(guid));

    if(!result)
        return false;

    Field* fields = result->Fetch();

    uint32 category = fields[1].GetUInt32();
    char const* text = fields[2].GetString();

    ShowTicket(guid,category,text);
    delete result;

    return true;
}

uint32 ChatHandler::GetTicketIDByNum(uint32 num)
{
    QueryResult *result = sDatabase.Query("SELECT `ticket_id` FROM `character_ticket`");

    if(!result || num > result->GetRowCount())
    {
        PSendSysMessage("Ticket %i doesn't exist", num);
        delete result;
        return 0;
    }

    for(int i = 1; i < num; ++i)
        result->NextRow();

    Field* fields = result->Fetch();

    uint32 id = fields[0].GetUInt32();
    delete result;
    return id;
}

bool ChatHandler::HandleDelTicketCommand(const char *args)
{
    char* px = strtok((char*)args, " ");
    if (!px)
        return false;

    // delticket all
    if(strncmp(px,"all",4) == 0)
    {
        QueryResult *result = sDatabase.Query("SELECT `guid` FROM `character_ticket`");

        if(!result)
            return true;

        // notify players about ticket deleting
        do
        {
            Field* fields = result->Fetch();

            uint64 guid = fields[0].GetUInt64();

            if(Player* sender = objmgr.GetPlayer(guid))
                sender->GetSession()->SendGMTicketGetTicket(1,0);

        }while(result->NextRow());

        delete result;

        sDatabase.PExecute("DELETE FROM `character_ticket`");
        SendSysMessage("All tickets deleted.");
        return true;
    }

    int num = (uint32)atoi(px);

    // delticket #num
    if(num > 0)
    {
        QueryResult *result = sDatabase.PQuery("SELECT `ticket_id`,`guid` FROM `character_ticket` LIMIT '%i'",num);

        if(!result || num > result->GetRowCount())
        {
            PSendSysMessage("Ticket %i doesn't exist", num);
            delete result;
            return true;
        }

        for(int i = 1; i < num; ++i)
            result->NextRow();

        Field* fields = result->Fetch();

        uint32 id   = fields[0].GetUInt32();
        uint64 guid = fields[1].GetUInt64();
        delete result;

        sDatabase.PExecute("DELETE FROM `character_ticket` WHERE `ticket_id` = '%u'", id);

        // notify players about ticket deleting
        if(Player* sender = objmgr.GetPlayer(guid))
        {
            sender->GetSession()->SendGMTicketGetTicket(1,0);
            PSendSysMessage("Character %s ticket deleted.",sender->GetName());
        }
        else
            SendSysMessage("Ticket deleted.");

        return true;
    }

    uint64 guid = objmgr.GetPlayerGUIDByName(px);

    if(!guid)
        return false;

    // delticket $char_name
    sDatabase.PExecute("DELETE FROM `character_ticket` WHERE `guid` = '%u'",GUID_LOPART(guid));

    // notify players about ticket deleting
    if(Player* sender = objmgr.GetPlayer(guid))
        sender->GetSession()->SendGMTicketGetTicket(1,0);

    PSendSysMessage("Character %s ticket deleted.",px);
    return true;
}
