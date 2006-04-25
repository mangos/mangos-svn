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

bool ChatHandler::HandleGUIDCommand(const char* args)
{
    WorldPacket data;

    uint64 guid;
    guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];
    sprintf((char*)buf,"Object guid is: lowpart %u highpart %X", GUID_LOPART(guid), GUID_HIPART(guid));
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    return true;
}

bool ChatHandler::HandleNameCommand(const char* args)
{
    /*    WorldPacket data;
    Temp. disabled
        if(!*args)
            return false;

        if(strlen((char*)args)>75)
        {

            char buf[256];
            sprintf((char*)buf,"The name was too long by %i", strlen((char*)args)-75);
            FillSystemMessageData(&data, m_session, buf);
            m_session->SendPacket( &data );
            return true;
        }

        for (uint8 i = 0; i < strlen(args); i++)
        {
            if(!isalpha(args[i]) && args[i]!=' ')
            {
                FillSystemMessageData(&data, m_session, "Error, name can only contain chars A-Z and a-z.");
                m_session->SendPacket( &data );
                return false;
            }
        }

        uint64 guid;
        guid = m_session->GetPlayer()->GetSelection();
        if (guid == 0)
        {
            FillSystemMessageData(&data, m_session, "No selection.");
            m_session->SendPacket( &data );
            return true;
        }

        Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

        if(!pCreature)
        {
            FillSystemMessageData(&data, m_session, "You should select a creature.");
            m_session->SendPacket( &data );
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
    WorldPacket data;

    if(!*args)
        args = "";

    if(strlen((char*)args)>75)
    {

        char buf[256];
        sprintf((char*)buf,"The subname was too long by %i", strlen((char*)args)-75);
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );
        return true;
    }

    for (uint8 i = 0; i < strlen(args); i++)
    {
        if(!isalpha(args[i]) && args[i]!=' ')
        {
            FillSystemMessageData(&data, m_session, "Error, name can only contain chars A-Z and a-z.");
            m_session->SendPacket( &data );
            return false;
        }
    }
    uint64 guid;
    guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
        m_session->SendPacket( &data );
        return true;
    }

    uint32 idname = objmgr.AddCreatureSubName(pCreature->GetName(),args,pCreature->GetUInt32Value(UNIT_FIELD_DISPLAYID));
    pCreature->SetUInt32Value(OBJECT_FIELD_ENTRY, idname);

    pCreature->SaveToDB();
    */
    return true;
}

bool ChatHandler::HandleProgCommand(const char* args)
{
    m_session->GetPlayer()->smsg_NewWorld(451, 16391.80f, 16341.20f, 69.44f,0.0f);

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

    Item * dstitem = m_session->GetPlayer()->GetItemBySlot(dstslot);
    Item * srcitem = m_session->GetPlayer()->GetItemBySlot(srcslot);

    //    m_session->GetPlayer()->SwapItemSlots(srcslot, dstslot);

    return true;
}

bool ChatHandler::HandleSpawnCommand(const char* args)
{
    WorldPacket data;

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
            FillSystemMessageData(&data, m_session, "Error, name can only contain chars A-Z and a-z.");
            m_session->SendPacket( &data );
            return false;
        }
    }
    SpawnCreature(m_session, pName, display_id, npcFlags, faction_id, level);

    return true;
}

bool ChatHandler::HandleAddSpwCommand(const char* args)
{

    WorldPacket data;
    char* charID = strtok((char*)args, " ");
    if (!charID)
        return false;

    uint32 id  = atoi(charID);

    QueryResult *result = sDatabase.PQuery("SELECT `modelid`,`flags`,`faction`,`level`,`name` FROM `creature_template` WHERE `entry` = '%d';", id);

    if(result)
    {
        Field *fields = result->Fetch();

        SpawnCreature(m_session, fields[4].GetString(), fields[0].GetUInt32(), fields[1].GetUInt32(), fields[2].GetUInt32(), fields[3].GetUInt32());
        delete result;
        return true;
    }
    else
        delete result;
    return false;
}

bool ChatHandler::HandleDeleteCommand(const char* args)
{
    WorldPacket data;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    Creature *unit = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!unit)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
        m_session->SendPacket( &data );
        return true;
    }

    unit->DeleteFromDB();

    MapManager::Instance().GetMap(unit->GetMapId())->Remove(unit, true);
    return true;
}

bool ChatHandler::HandleDeMorphCommand(const char* args)
{
    sLog.outError("Demorphed %s",m_session->GetPlayer()->GetName());
    m_session->GetPlayer()->DeMorph();
    return true;
}

bool ChatHandler::HandleItemCommand(const char* args)
{
    WorldPacket data;

    char* pitem = strtok((char*)args, " ");
    if (!pitem)
        return false;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
        m_session->SendPacket( &data );
        return true;
    }

    uint32 item = atoi(pitem);
    int amount = -1;

    char* pamount = strtok(NULL, " ");
    if (pamount)
        amount = atoi(pamount);

    ItemPrototype* tmpItem = objmgr.GetItemPrototype(item);

    std::stringstream sstext;
    if(tmpItem)
    {
        QueryResult *result = sDatabase.PQuery("INSERT INTO `npc_vendor` (`entry`,`itemguid`,`amount`) VALUES('%u','%u','%d');",pCreature->GetEntry(), item, amount);

        uint8 itemscount = (uint8)pCreature->getItemCount();
        pCreature->setItemId(itemscount , item);
        pCreature->setItemAmount(itemscount , amount);
        pCreature->increaseItemCount();

        sstext << "Item '" << item << "' '" << tmpItem->Name1 << "' Added to list" << '\0';
        delete result;
    }
    else
    {
        sstext << "Item '" << item << "' Not Found in Database." << '\0';
    }

    FillSystemMessageData(&data, m_session, sstext.str().c_str());
    m_session->SendPacket( &data );
    return true;
}

bool ChatHandler::HandleItemRemoveCommand(const char* args)
{
    WorldPacket data;

    char* iguid = strtok((char*)args, " ");
    if (!iguid)
        return false;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
        m_session->SendPacket( &data );
        return true;
    }

    uint32 itemguid = atoi(iguid);
    int slot = pCreature->getItemSlotById(itemguid);

    std::stringstream sstext;
    if(slot != -1)
    {
        uint32 guidlow = GUID_LOPART(guid);

        sDatabase.PExecute("DELETE FROM `npc_vendor` WHERE `entry` = '%u' AND `itemguid` = '%u'",pCreature->GetEntry(),itemguid);

        pCreature->setItemId(slot , 0);
        pCreature->setItemAmount(slot , 0);
        ItemPrototype* tmpItem = objmgr.GetItemPrototype(itemguid);
        if(tmpItem)
        {
            sstext << "Item '" << itemguid << "' '" << tmpItem->Name1 << "' Deleted from list" << '\0';
        }
        else
        {
            sstext << "Item '" << itemguid << "' Deleted from list" << '\0';
        }

    }
    else
    {
        sstext << "Item '" << itemguid << "' Not Found in List." << '\0';
    }

    FillSystemMessageData(&data, m_session, sstext.str().c_str());
    m_session->SendPacket( &data );

    return true;
}

bool ChatHandler::HandleAddMoveCommand(const char* args)
{
    WorldPacket data;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
        m_session->SendPacket( &data );
        return true;
    }

    // changed 'creatureId' to lowercase
    // changed 'X', 'y', 'Z' to 'positionx', 'positiony', 'positionz'
    sDatabase.PExecute("INSERT INTO `creature_movement` (`id`,`position_x`,`position_y`,`position_z`) VALUES ('%u', '%f', '%f', '%f');", GUID_LOPART(guid), m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ());

    FillSystemMessageData(&data, m_session, "Waypoint added.");
    m_session->SendPacket( &data );

    return true;
}

bool ChatHandler::HandleRandomCommand(const char* args)
{
    WorldPacket data;

    if(!*args)
        return false;

    int option = atoi((char*)args);

    if (option != 0 && option != 1)
    {
        m_session->GetPlayer( )->SendMessageToSet( &data, true );
        FillSystemMessageData(&data, m_session, "Incorrect value, use 0 or 1");
        m_session->SendPacket( &data );
        return true;
    }

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
        m_session->SendPacket( &data );
        return true;
    }

    // fix me : 'moverandom' doesn't exist in https://svn.mangosproject.org/trac/MaNGOS/wiki/Database/creatures ?
    // perhaps it should be 'state'?
    sDatabase.PExecute("UPDATE `creature` SET `moverandom` = '%i' WHERE `guid` = '%u';", option, GUID_LOPART(guid));

    pCreature->setMoveRandomFlag(option > 0);

    FillSystemMessageData(&data, m_session, "Value saved.");
    m_session->SendPacket( &data );

    return true;
}

bool ChatHandler::HandleRunCommand(const char* args)
{
    WorldPacket data;

    if(!*args)
        return false;

    int option = atoi((char*)args);

    if(option != 0 && option != 1)
    {
        m_session->GetPlayer( )->SendMessageToSet( &data, true );
        FillSystemMessageData(&data, m_session, "Incorrect value, use 0 or 1");
        m_session->SendPacket( &data );
        return true;
    }

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
        m_session->SendPacket( &data );
        return true;
    }

    // fix me : 'running' doesn't exist in https://svn.mangosproject.org/trac/MaNGOS/wiki/Database/creatures ?
    // perhaps it should be 'state'?
    sDatabase.PExecute("UPDATE `creature` SET `running` = '%i' WHERE `guid` = '%u';", option, GUID_LOPART(guid));

    pCreature->setMoveRunFlag(option > 0);

    FillSystemMessageData(&data, m_session, "Value saved.");
    m_session->SendPacket( &data );

    return true;
}

bool ChatHandler::HandleChangeLevelCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    uint8 lvl = (uint8) atoi((char*)args);
    if ( lvl < 1 || lvl > 63)
    {
        FillSystemMessageData(&data, m_session, "Incorrect value.");
        m_session->SendPacket( &data );
        return true;
    }

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
        m_session->SendPacket( &data );
        return true;
    }

    pCreature->SetUInt32Value(UNIT_FIELD_HEALTH, 100 + 30*lvl);
    pCreature->SetUInt32Value(UNIT_FIELD_MAXHEALTH, 100 + 30*lvl);
    pCreature->SetUInt32Value(UNIT_FIELD_LEVEL , lvl);

    pCreature->SaveToDB();

    return true;
}

bool ChatHandler::HandleNPCFlagCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    uint32 npcFlags = (uint32) atoi((char*)args);

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
        m_session->SendPacket( &data );
        return true;
    }

    pCreature->SetUInt32Value(UNIT_NPC_FLAGS, npcFlags);

    pCreature->SaveToDB();

    FillSystemMessageData(&data, m_session, "Value saved, you may need to rejoin or clean your client cache.");
    m_session->SendPacket( &data );

    uint32 entry = pCreature->GetUInt32Value( OBJECT_FIELD_ENTRY );
    m_session->SendCreatureQuery( entry, guid );

    return true;
}

bool ChatHandler::HandleDisplayIdCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    uint32 displayId = (uint32) atoi((char*)args);

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
        m_session->SendPacket( &data );
        return true;
    }

    pCreature->SetUInt32Value(UNIT_FIELD_DISPLAYID, displayId);

    pCreature->SaveToDB();

    return true;
}

bool ChatHandler::HandleFactionIdCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    uint32 factionId = (uint32) atoi((char*)args);

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
        m_session->SendPacket( &data );
        return true;
    }

    pCreature->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE , factionId);

    pCreature->SaveToDB();

    return true;
}
