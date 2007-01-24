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

bool ChatHandler::HandleTargetObjectCommand(const char* args)
{

    Player* pl = m_session->GetPlayer();
    QueryResult *result;

    if(*args)
    {
        int32 id = atoi((char*)args);
        if(id)
            result = sDatabase.PQuery("SELECT `guid`, `id`, `position_x`, `position_y`, `position_z`, `orientation`, `map`, (POW(`position_x` - '%f', 2) + POW(`position_y` - '%f', 2) + POW(`position_z` - '%f', 2)) as `order` FROM `gameobject` WHERE `map` = '%i' AND `id` = '%u' ORDER BY `order` ASC LIMIT 1",
                pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), pl->GetMapId(),id);
        else
        {
            std::string name = args;
            sDatabase.escape_string(name);
            result = sDatabase.PQuery(
                "SELECT `guid`, `id`, `position_x`, `position_y`, `position_z`, `orientation`, `map`, (POW(`position_x` - %f, 2) + POW(`position_y` - %f, 2) + POW(`position_z` - %f, 2)) as `order` "
                "FROM `gameobject`,`gameobject_template` WHERE `gameobject_template`.`entry` = `gameobject`.`id` AND `map` = %i AND `name` LIKE '%%%s%%' ORDER BY `order` ASC LIMIT 1",
                pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), pl->GetMapId(),name.c_str());
        }
    }
    else
        result = sDatabase.PQuery("SELECT `guid`, `id`, `position_x`, `position_y`, `position_z`, `orientation`, `map`, (POW(`position_x` - %f, 2) + POW(`position_y` - %f, 2) + POW(`position_z` - %f, 2)) as `order` FROM `gameobject` WHERE `map` = %i ORDER BY `order` ASC LIMIT 1", m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ(), m_session->GetPlayer()->GetMapId());

    if (!result)
    {
        SendSysMessage("Nothing found!");
        return true;
    }

    Field *fields = result->Fetch();
    uint32 guid = fields[0].GetUInt32();
    uint32 id = fields[1].GetUInt32();
    float x = fields[2].GetFloat();
    float y = fields[3].GetFloat();
    float z = fields[4].GetFloat();
    float o = fields[5].GetFloat();
    int mapid = fields[6].GetUInt16();
    delete result;

    const GameObjectInfo *goI = objmgr.GetGameObjectInfo(id);

    if (!goI)
    {
        PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST,id);
        return false;
    }

    PSendSysMessage("Selected object:\n%s\nGUID: %u ID: %u\nX: %f Y: %f Z: %f MapId: %u\nOrientation: %f", goI->name, guid, id, x, y, z, mapid, o);

    return true;
}

bool ChatHandler::HandleGoObjectCommand(const char* args)
{
    if(m_session->GetPlayer()->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        return true;
    }

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
    if(m_session->GetPlayer()->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        return true;
    }

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
    if(m_session->GetPlayer()->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        return true;
    }

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

    char* pLevel = strtok(NULL, " ");
    if (!pLevel)
        return false;

    char* pName = strtok(NULL, "%");
    if (!pName)
        return false;

    uint32 level  = atoi(pLevel);

    for (uint8 i = 0; i < strlen(pName); i++)
    {
        if(!isalpha(pName[i]) && pName[i]!=' ')
        {
            SendSysMessage(LANG_CHARS_ONLY);
            return false;
        }
    }
    SpawnCreature(m_session, pName, level);

    return true;
}

bool ChatHandler::HandleAddSpwCommand(const char* args)
{
    char* charID = strtok((char*)args, " ");
    if (!charID)
        return false;

    uint32 id  = atoi(charID);

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

    pCreature->SaveToDB();
    pCreature->LoadFromDB(pCreature->GetGUIDLow());         // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells();
    pCreature->AddToWorld();
    MapManager::Instance().GetMap(pCreature->GetMapId())->Add(pCreature);

    sLog.outDebug(LANG_ADD_OBJ);

    return true;
}

bool ChatHandler::HandleDeleteCommand(const char* args)
{
    Creature *unit = getSelectedCreature();
    if(!unit)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    unit->CombatStop();

    unit->DeleteFromDB();

    ObjectAccessor::Instance().AddObjectToRemoveList(unit);

    SendSysMessage("Creature Removed");

    return true;
}

bool ChatHandler::HandleDelObjectCommand(const char* args)
{
    if(!*args)
        return false;

    uint32 lowguid = atoi((char*)args);
    if(!lowguid)
        return false;

    GameObject* obj = ObjectAccessor::Instance().GetGameObject(*m_session->GetPlayer(), MAKE_GUID(lowguid, HIGHGUID_GAMEOBJECT));

    if(!obj)
    {
        PSendSysMessage("Game Object (GUID: %u) not found", lowguid);
        return true;
    }

    uint64 owner_guid = obj->GetOwnerGUID();
    if(owner_guid)
    {
        Unit* owner = ObjectAccessor::Instance().GetUnit(*m_session->GetPlayer(),owner_guid);
        if(!owner && GUID_HIPART(owner_guid)!=HIGHGUID_PLAYER)
        {
            PSendSysMessage("Game Object (GUID: %u) have references in not found creature %u GO list, can't be deleted.", GUID_LOPART(owner_guid), obj->GetGUIDLow());
            return true;
        }

        owner->RemoveGameObject(obj,false);
    }

    obj->SetRespawnTime(0);                                 // not save respawn time
    obj->Delete();
    obj->DeleteFromDB();

    PSendSysMessage("Game Object (GUID: %u) removed", obj->GetGUIDLow());

    return true;
}

bool ChatHandler::HandleTurnObjectCommand(const char* args)
{
    if(!*args)
        return false;

    char* plowguid = strtok((char*)args, " ");

    if(!plowguid)
        return false;

    uint32 lowguid = (uint32)atoi(plowguid);

    GameObject* obj = ObjectAccessor::Instance().GetGameObject(*m_session->GetPlayer(), MAKE_GUID(lowguid, HIGHGUID_GAMEOBJECT));

    if(!obj)
    {
        PSendSysMessage("Game Object (GUID: %u) not found", lowguid);
        return true;
    }

    char* po = strtok(NULL, " ");
    float o;

    if (po)
    {
        o = (float)atof(po);
    }
    else
    {
        Player *chr = m_session->GetPlayer();
        o = chr->GetOrientation();
    }

    float rot2 = sin(o/2);
    float rot3 = cos(o/2);

    obj->Relocate(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), o);

    obj->SetFloatValue(GAMEOBJECT_FACING, o);
    obj->SetFloatValue(GAMEOBJECT_ROTATION+2, rot2);
    obj->SetFloatValue(GAMEOBJECT_ROTATION+3, rot3);

    obj->SaveToDB();
    obj->Refresh();

    PSendSysMessage("Game Object (GUID: %u) turned", obj->GetGUIDLow(), o);

    return true;
}

bool ChatHandler::HandleMoveObjectCommand(const char* args)
{
    if(!*args)
        return false;

    char* plowguid = strtok((char*)args, " ");

    if(!plowguid)
        return false;

    uint32 lowguid = (uint32)atoi(plowguid);

    GameObject* obj = ObjectAccessor::Instance().GetGameObject(*m_session->GetPlayer(), MAKE_GUID(lowguid, HIGHGUID_GAMEOBJECT));

    if(!obj)
    {
        PSendSysMessage("Game Object (GUID: %u) not found", lowguid);
        return true;
    }

    char* px = strtok(NULL, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");

    if (!px)
    {
        Player *chr = m_session->GetPlayer();

        obj->Relocate(chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ(), obj->GetOrientation());

        obj->SetFloatValue(GAMEOBJECT_POS_X, chr->GetPositionX());
        obj->SetFloatValue(GAMEOBJECT_POS_Y, chr->GetPositionY());
        obj->SetFloatValue(GAMEOBJECT_POS_Z, chr->GetPositionZ());
    }
    else
    {
        if(!py || !pz)
            return false;

        float x = (float)atof(px);
        float y = (float)atof(py);
        float z = (float)atof(pz);

        if(!MapManager::ExistMAP(obj->GetMapId(),x,y))
        {
            PSendSysMessage(".move target map not exist (X: %f Y: %f MapId:%u)",x,y,obj->GetMapId());
            return true;
        }

        obj->Relocate(x, y, z, obj->GetOrientation());

        obj->SetFloatValue(GAMEOBJECT_POS_X, x);
        obj->SetFloatValue(GAMEOBJECT_POS_Y, y);
        obj->SetFloatValue(GAMEOBJECT_POS_Z, z);
    }

    obj->SaveToDB();
    obj->Refresh();

    PSendSysMessage("Game Object (GUID: %u) moved", obj->GetGUIDLow());

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
    if(!*args)
        return false;

    char* guid_str = strtok((char*)args, " ");
    char* wait_str = strtok((char*)NULL, " ");

    uint32 lowguid = atoi((char*)guid_str);

    Creature* pCreature = NULL;

    if(lowguid)
        pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(lowguid,HIGHGUID_UNIT));

    if(!pCreature)
    {
        PSendSysMessage("Creature (GUID: %u) not found", lowguid);
        return true;
    }

    int wait = wait_str ? atoi(wait_str) : 0;

    if(wait < 0)
        wait = 0;

    uint32 point;

    QueryResult *result = sDatabase.PQuery( "SELECT MAX(`point`) FROM `creature_movement` WHERE `id` = '%u'",pCreature->GetGUIDLow());
    if( result )
    {
        point = (*result)[0].GetUInt32()+1;

        delete result;
    }
    else
        point = 0;

    Player* player = m_session->GetPlayer();

    sDatabase.PExecute("INSERT INTO `creature_movement` (`id`,`point`,`position_x`,`position_y`,`position_z`,`waittime`) VALUES ('%u','%u','%f', '%f', '%f','%u')",
        pCreature->GetGUIDLow(), point, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), wait);

    // update movement type
    if(pCreature->GetDefaultMovementType()!=WAYPOINT_MOTION_TYPE)
    {
        pCreature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
        sDatabase.PExecute("UPDATE `creature` SET `MovementType` = '%u' WHERE `guid` = '%u'", pCreature->GetDefaultMovementType(),pCreature->GetGUIDLow());
    }

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

    sDatabase.PExecute("UPDATE `creature_template` SET `npcflag` = '%u' WHERE `entry` = '%u'", npcFlags, pCreature->GetEntry());

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
    char* kickName = strtok((char*)args, " ");
    if (!kickName)
    {
        Player* player = getSelectedPlayer();
        if(player==m_session->GetPlayer())
        {
            SendSysMessage("You can't kick self by selecting, use .kick name ;)");
            return true;
        }

        player->GetSession()->KickPlayer();
    }
    else
    {
        std::string name = kickName;
        normalizePlayerName(name);
        if(sWorld.KickPlayer(name))
            PSendSysMessage("Player %s kicked.",name.c_str());
        else
            PSendSysMessage("Player %s not found.",name.c_str());
    }

    return true;
}

bool ChatHandler::HandlePInfoCommand(const char* args)
{
    Player* target = NULL;

    char* px = strtok((char*)args, " ");
    char* py = NULL;

    if (px)
    {
        std::string name = px;
        normalizePlayerName(name);
        target = objmgr.GetPlayer(name.c_str());
        if (target)
            py = strtok(NULL, " ");
        else
            py = px;
    }
    if(!target)
    {
        target = getSelectedPlayer();
    }

    if(!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        return true;
    }

    std::string username = "<error>";
    std::string last_ip = "<error>";

    QueryResult* result = loginDatabase.PQuery("SELECT `username`, `last_ip` FROM `account` WHERE `id` = '%u'",target->GetSession()->GetAccountId());
    if(result)
    {
        Field* fields = result->Fetch();
        username = fields[0].GetCppString();
        if(m_session->GetSecurity() >= target->GetSession()->GetSecurity())
            last_ip = fields[1].GetCppString();
        else
            last_ip = "-";

        delete result;
    }

    PSendSysMessage(LANG_PINFO_ACCOUNT,  target->GetName(), target->GetGUIDLow(), username.c_str(), target->GetSession()->GetAccountId(), target->GetSession()->GetSecurity(), last_ip.c_str());

    uint32 days = target->GetTotalPlayedTime() / (60*60*24);
    uint32 hours = (target->GetTotalPlayedTime() % (60*60*24)) / (60*60);
    uint32 gold = target->GetMoney() /(100*100);
    uint32 silv = (target->GetMoney() % (100*100)) / 100;
    uint32 copp = (target->GetMoney() % (100*100)) % 100;
    PSendSysMessage(LANG_PINFO_LEVEL,  days, hours, target->getLevel(), gold,silv,copp );

    if ( py && strncmp(py, "rep", 3) == 0 )
    {
        static const char* ReputationRankStr[MAX_REPUTATION_RANK] = {"Hated", "Hostile", "Unfriendly", "Neutral", "Friendly", "Honored", "Reverted", "Exalted"};
        std::list<struct Factions>::const_iterator itr;
        char* FactionName;
        for(itr = target->factions.begin(); itr != target->factions.end(); ++itr)
        {
            FactionEntry const *factionEntry = sFactionStore.LookupEntry(itr->ID);
            if (factionEntry)
                FactionName = factionEntry->name;
            else
                FactionName = "#Not found#";
            ReputationRank Rank = target->GetReputationRank(factionEntry);

            PSendSysMessage("Id:%4d %s %s %5d %1x", itr->ID, FactionName, ReputationRankStr[Rank], target->GetReputation(factionEntry), itr->Flags);
        }
    }
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

        if(!result || uint64(num) > result->GetRowCount())
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

    std::string name = px;
    normalizePlayerName(name);
    sDatabase.escape_string(name);                          // prevent SQL injection - normal name don't must changed by this call

    uint64 guid = objmgr.GetPlayerGUIDByName(name.c_str());

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

    for(uint32 i = 1; i < num; ++i)
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

        if(!result || uint64(num) > result->GetRowCount())
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

    std::string name = px;
    normalizePlayerName(name);
    sDatabase.escape_string(name);                          // prevent SQL injection - normal name don't must changed by this call

    uint64 guid = objmgr.GetPlayerGUIDByName(name.c_str());

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

bool ChatHandler::HandleSpawnDistCommand(const char* args)
{
    if(!*args)
        return false;

    int option = atoi((char*)args);
    if (option < 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return false;
    }

    int mtype=0;                                            // MovementType
    if (option > 0)
        mtype=1;

    Creature *pCreature = getSelectedCreature();
    uint64 u_guid = 0;

    if (pCreature)
        u_guid = pCreature->GetGUID();
    else
        return false;

    sDatabase.PQuery("UPDATE `creature` SET `spawndist`=%i, `MovementType`=%i WHERE `guid`=%u",option,mtype,u_guid);
    PSendSysMessage("Spawn distance changed to: %i",option);
    return true;
}

bool ChatHandler::HandleSpawnTimeCommand(const char* args)
{
    if(!*args)
        return false;

    char* stime = strtok((char*)args, " ");

    if (!stime)
        return false;

    int i_stime = atoi((char*)stime);

    if (i_stime < 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Creature *pCreature = getSelectedCreature();
    uint64 u_guid = 0;

    if (pCreature)
        u_guid = pCreature->GetGUID();
    else
        return false;

    sDatabase.PQuery("UPDATE `creature` SET `spawntimesecs`=%i WHERE `guid`=%u",i_stime,u_guid);
    PSendSysMessage("Spawn time changed to: %i",i_stime);

    return true;
}
