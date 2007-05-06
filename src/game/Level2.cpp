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
#include "Item.h"
#include "GameObject.h"
#include "Opcodes.h"
#include "Chat.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "Language.h"
#include "World.h"
#include "WaypointMovementGenerator.h"

#include <iostream>
#include <fstream>
using namespace std;

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
        SendSysMessage(LANG_COMMAND_TARGETOBJNOTFOUND);
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
    Player* _player = m_session->GetPlayer();

    if(_player->isInFlight())
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
        SendSysMessage(LANG_COMMAND_GOOBJNOTFOUND);
        return true;
    }

    Field *fields = result->Fetch();
    float x = fields[0].GetFloat();
    float y = fields[1].GetFloat();
    float z = fields[2].GetFloat();
    float ort = fields[3].GetFloat();
    int mapid = fields[4].GetUInt16();
    delete result;

    if(!MapManager::IsValidMapCoord(mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        return true;
    }

    _player->SetRecallPosition(_player->GetMapId(),_player->GetPositionX(),_player->GetPositionY(),_player->GetPositionZ(),_player->GetOrientation());

    _player->TeleportTo(mapid, x, y, z, ort);
    return true;
}

bool ChatHandler::HandleGoCreatureCommand(const char* args)
{
    Player* _player = m_session->GetPlayer();

    if(_player->isInFlight())
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
        SendSysMessage(LANG_COMMAND_GOCREATNOTFOUND);
        return true;
    }

    Field *fields = result->Fetch();
    float x = fields[0].GetFloat();
    float y = fields[1].GetFloat();
    float z = fields[2].GetFloat();
    float ort = fields[3].GetFloat();
    int mapid = fields[4].GetUInt16();

    delete result;

    if(!MapManager::IsValidMapCoord(mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        return true;
    }

    _player->SetRecallPosition(_player->GetMapId(),_player->GetPositionX(),_player->GetPositionY(),_player->GetPositionZ(),_player->GetOrientation());

    _player->TeleportTo(mapid, x, y, z, ort);
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
    Player* _player = m_session->GetPlayer();

    if(_player->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        return true;
    }

    _player->SetRecallPosition(_player->GetMapId(),_player->GetPositionX(),_player->GetPositionY(),_player->GetPositionZ(),_player->GetOrientation());

    _player->TeleportTo(451, 16391.80f, 16341.20f, 69.44f,0.0f);

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

    if(srcslot==dstslot)
        return true;

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

    Creature* pCreature = new Creature(chr);
    if (!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), chr->GetMapId(), x, y, z, o, id))
    {
        delete pCreature;
        return false;
    }

    pCreature->SaveToDB();

    // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells();
    pCreature->LoadFromDB(pCreature->GetGUIDLow(), NULL, chr->GetInstanceId());

    pCreature->AddToWorld();
    MapManager::Instance().GetMap(pCreature->GetMapId(), pCreature)->Add(pCreature);

    sLog.outDebug(LANG_ADD_OBJ);

    return true;
}

bool ChatHandler::HandleDeleteCommand(const char* args)
{
    Creature *unit = getSelectedCreature();
    if(!unit || unit->isPet() || unit->isTotem())
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    unit->CombatStop(true);

    unit->DeleteFromDB();

    unit->CleanupCrossRefsBeforeDelete();
    ObjectAccessor::Instance().AddObjectToRemoveList(unit);

    SendSysMessage(LANG_COMMAND_DELCREATMESSAGE);

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
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
        return true;
    }

    uint64 owner_guid = obj->GetOwnerGUID();
    if(owner_guid)
    {
        Unit* owner = ObjectAccessor::Instance().GetUnit(*m_session->GetPlayer(),owner_guid);
        if(!owner && GUID_HIPART(owner_guid)!=HIGHGUID_PLAYER)
        {
            PSendSysMessage(LANG_COMMAND_DELOBJREFERCREATURE, GUID_LOPART(owner_guid), obj->GetGUIDLow());
            return true;
        }

        owner->RemoveGameObject(obj,false);
    }

    obj->SetRespawnTime(0);                                 // not save respawn time
    obj->Delete();
    obj->DeleteFromDB();

    PSendSysMessage(LANG_COMMAND_DELOBJMESSAGE, obj->GetGUIDLow());

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
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
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

    PSendSysMessage(LANG_COMMAND_TURNOBJMESSAGE, obj->GetGUIDLow(), o);

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
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
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

        if(!MapManager::IsValidMapCoord(obj->GetMapId(),x,y))
        {
            PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,obj->GetMapId());
            return true;
        }

        obj->Relocate(x, y, z, obj->GetOrientation());

        obj->SetFloatValue(GAMEOBJECT_POS_X, x);
        obj->SetFloatValue(GAMEOBJECT_POS_Y, y);
        obj->SetFloatValue(GAMEOBJECT_POS_Z, z);
    }

    obj->SaveToDB();
    obj->Refresh();

    PSendSysMessage(LANG_COMMAND_MOVEOBJMESSAGE, obj->GetGUIDLow());

    return true;
}

bool ChatHandler::HandleDeMorphCommand(const char* args)
{
    sLog.outError(LANG_DEMORPHED,m_session->GetPlayer()->GetName());
    m_session->GetPlayer()->DeMorph();
    return true;
}

bool ChatHandler::HandleAddVendorItemCommand(const char* args)
{
    if (!*args)
        return false;

    Creature* vendor = getSelectedCreature();
    if (!vendor || !vendor->isVendor())
    {
        SendSysMessage(LANG_COMMAND_VENDORSELECTION);
        return true;
    }

    char* pitem = strtok((char*)args, " ");
    uint32 itemId = atol(pitem);
    if (!pitem)
    {
        SendSysMessage(LANG_COMMAND_ADDVENDORITEMSEND);
        return true;
    }

    char* fmaxcount = strtok(NULL, " ");                    //add maxcount, default: 0
    uint32 maxcount = 0;
    if (fmaxcount)
        maxcount = atol(fmaxcount);

    char* fincrtime = strtok(NULL, " ");                    //add incrtime, default: 0
    uint32 incrtime = 0;
    if (fincrtime)
        incrtime = atol(fincrtime);

    ItemPrototype const *pProto = objmgr.GetItemPrototype(itemId);
    if(!pProto)
    {
        PSendSysMessage(LANG_ITEM_NOT_FOUND, itemId);
        return true;
    }

    if(vendor->FindItem(itemId))
    {
        PSendSysMessage(LANG_ITEM_ALREADY_IN_LIST,itemId);
        return true;
    }

    if (vendor->GetItemCount() >= MAX_VENDOR_ITEMS)
    {
        SendSysMessage(LANG_COMMAND_ADDVENDORITEMITEMS);
        return true;
    }

    // add to DB and to current ingame vendor
    sDatabase.PExecuteLog("INSERT INTO `npc_vendor` (`entry`,`item`,`maxcount`,`incrtime`) VALUES('%u','%u','%u','%u')",vendor->GetEntry(), itemId, maxcount,incrtime);
    vendor->AddItem(itemId,maxcount,incrtime);
    PSendSysMessage(LANG_ITEM_ADDED_TO_LIST,itemId,pProto->Name1,maxcount,incrtime);
    return false;
}

bool ChatHandler::HandleDelVendorItemCommand(const char* args)
{
    if (!*args)
        return false;

    Creature* vendor = getSelectedCreature();
    if (!vendor || !vendor->isVendor())
    {
        SendSysMessage(LANG_COMMAND_VENDORSELECTION);
        return true;
    }

    char* pitem = strtok((char*)args, " ");
    uint32 itemId = atol(pitem);

    ItemPrototype const *pProto = objmgr.GetItemPrototype(itemId);
    if(!pProto)
    {
        PSendSysMessage(LANG_ITEM_NOT_FOUND, itemId);
        return true;
    }

    if (!vendor->RemoveItem(itemId))
    {
        PSendSysMessage(LANG_ITEM_NOT_IN_LIST,itemId);
        return true;
    }

    sDatabase.PExecuteLog("DELETE FROM `npc_vendor` WHERE `entry`='%u' AND `item`='%u'",vendor->GetEntry(), itemId);
    PSendSysMessage(LANG_ITEM_DELETED_FROM_LIST,itemId,pProto->Name1);
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

    // attempt check creature existance by DB
    if(!pCreature)
    {
        QueryResult *result = sDatabase.PQuery( "SELECT `guid` FROM `creature` WHERE `guid` = '%u'",lowguid);
        if(!result)
        {
            PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
            return true;
        }
        delete result;
    }
    else
    {
        // obtain real GUID for DB operations
        lowguid = pCreature->GetDBTableGUIDLow();
    }

    int wait = wait_str ? atoi(wait_str) : 0;

    if(wait < 0)
        wait = 0;

    uint32 point;

    QueryResult *result = sDatabase.PQuery( "SELECT MAX(`point`) FROM `creature_movement` WHERE `id` = '%u'",lowguid);
    if( result )
    {
        point = (*result)[0].GetUInt32()+1;

        delete result;
    }
    else
        point = 0;

    Player* player = m_session->GetPlayer();

    sDatabase.PExecuteLog("INSERT INTO `creature_movement` (`id`,`point`,`position_x`,`position_y`,`position_z`,`waittime`) VALUES ('%u','%u','%f', '%f', '%f','%u')",
        lowguid, point, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), wait);

    // update movement type
    sDatabase.PExecuteLog("UPDATE `creature` SET `MovementType` = '%u' WHERE `guid` = '%u'", WAYPOINT_MOTION_TYPE,lowguid);
    if(pCreature)
    {
        pCreature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
        (*pCreature)->Initialize();
        if(pCreature->isAlive())                            // dead creature will reset movement generator at respawn
        {
            pCreature->setDeathState(JUST_DIED);
            pCreature->Respawn();
        }
    }

    SendSysMessage(LANG_WAYPOINT_ADDED);

    return true;
}

bool ChatHandler::HandleSetMoveTypeCommand(const char* args)
{
    if(!*args)
        return false;

    char* guid_str = strtok((char*)args, " ");
    char* type_str = strtok((char*)NULL, " ");

    if(!guid_str)
        return false;

    uint32 lowguid = 0;
    Creature* pCreature = NULL;

    if(!type_str)                                           // case .setmovetype $move_type (with selected creature)
    {
        type_str = guid_str;
        pCreature = getSelectedCreature();
        if(!pCreature)
            return false;
        lowguid = pCreature->GetDBTableGUIDLow();
    }
    else                                                    // case .setmovetype #creature_guid $move_type (with selected creature)
    {
        lowguid = atoi((char*)guid_str);
        if(lowguid)
            pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(lowguid,HIGHGUID_UNIT));

        // attempt check creature existance by DB
        if(!pCreature)
        {
            QueryResult *result = sDatabase.PQuery( "SELECT `guid` FROM `creature` WHERE `guid` = '%u'",lowguid);
            if(!result)
            {
                PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
                return true;
            }
            delete result;
        }
        else
        {
            lowguid = pCreature->GetDBTableGUIDLow();
        }
    }

    // now lowguid is low guid really existed creature
    // and pCreature point (maybe) to this creature or NULL

    MovementGeneratorType move_type;

    std::string type = type_str;

    if(type == "stay")
        move_type = IDLE_MOTION_TYPE;
    else if(type == "random")
        move_type = RANDOM_MOTION_TYPE;
    else if(type == "way")
        move_type = WAYPOINT_MOTION_TYPE;
    else
        return false;

    // update movement type
    sDatabase.BeginTransaction();
    sDatabase.PExecuteLog("UPDATE `creature` SET `MovementType` = '%u' WHERE `guid` = '%u'", move_type,lowguid);
    sDatabase.PExecuteLog("DELETE FROM `creature_movement` WHERE `id` = '%u'",lowguid);
    sDatabase.CommitTransaction();
    if(pCreature)
    {
        pCreature->SetDefaultMovementType(move_type);
        (*pCreature)->Initialize();
        if(pCreature->isAlive())                            // dead creature will reset movement generator at respawn
        {
            pCreature->setDeathState(JUST_DIED);
            pCreature->Respawn();
        }
    }

    PSendSysMessage(LANG_MOVE_TYPE_SET,type_str);

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
    //sDatabase.PExecuteLog("UPDATE `creature` SET `running` = '%i' WHERE `guid` = '%u'", option, pCreature->GetDBTableGUIDLow());

    pCreature->setMoveRunFlag(option > 0);

    //SendSysMessage(LANG_VALUE_SAVED);
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

    sDatabase.PExecuteLog("UPDATE `creature_template` SET `npcflag` = '%u' WHERE `entry` = '%u'", npcFlags, pCreature->GetEntry());

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

    if (!sFactionTemplateStore.LookupEntry(factionId))
    {
        PSendSysMessage(LANG_WRONG_FACTION, factionId);
        return true;
    }

    Creature* pCreature = getSelectedCreature();

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    pCreature->setFaction(factionId);
    
    // faction is set in creature_template - not inside creature
    //pCreature->SaveToDB(); -- obsolete
    sDatabase.PExecuteLog("UPDATE `creature_template` SET `faction` = '%u' WHERE `entry` = '%u'", factionId, pCreature->GetEntry());


    return true;
}

bool ChatHandler::HandleKickPlayerCommand(const char *args)
{
    char* kickName = strtok((char*)args, " ");
    if (!kickName)
    {
        Player* player = getSelectedPlayer();

        if(!player)
        {
            SendSysMessage(LANG_NO_CHAR_SELECTED);
            return true;
        }

        if(player==m_session->GetPlayer())
        {
            SendSysMessage(LANG_COMMAND_KICKSELF);
            return true;
        }

        player->GetSession()->KickPlayer();
    }
    else
    {
        std::string name = kickName;
        normalizePlayerName(name);

        if(name==m_session->GetPlayer()->GetName())
        {
            SendSysMessage(LANG_COMMAND_KICKSELF);
            return true;
        }

        if(sWorld.KickPlayer(name))
        {
            PSendSysMessage(LANG_COMMAND_KICKMESSAGE,name.c_str());
        }
        else
            PSendSysMessage(LANG_COMMAND_KICKNOTFOUNDPLAYER,name.c_str());
    }

    return true;
}

bool ChatHandler::HandlePInfoCommand(const char* args)
{
    Player* target = NULL;
    uint64 targetGUID = 0;

    char* px = strtok((char*)args, " ");
    char* py = NULL;

    std::string name;

    if (px)
    {
        name = px;
        normalizePlayerName(name);
        target = objmgr.GetPlayer(name.c_str());
        if (target)
            py = strtok(NULL, " ");
        else
        {
            targetGUID = objmgr.GetPlayerGUIDByName(name.c_str());
            if(targetGUID)
                py = strtok(NULL, " ");
            else
                py = px;
        }
    }

    if(!target && !targetGUID)
    {
        target = getSelectedPlayer();
    }

    if(!target && !targetGUID)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        return true;
    }

    uint32 accId = 0;
    uint32 money = 0;
    uint32 total_player_time = 0;
    uint32 level = 0;

    // get additional information from Player object
    if(target)
    {
        targetGUID = target->GetGUID();
        name = target->GetName();                           // re-read for case getSelectedPlayer() target
        accId = target->GetSession()->GetAccountId();
        money = target->GetMoney();
        total_player_time = target->GetTotalPlayedTime();
        level = target->getLevel();
    }
    // get additional information from DB
    else
    {
        accId = objmgr.GetPlayerAccountIdByGUID(targetGUID);
        Player plr(m_session);                              // use current session for temporary load
        plr.MinimalLoadFromDB(targetGUID);
        money = plr.GetMoney();
        total_player_time = plr.GetTotalPlayedTime();
        level = plr.getLevel();
    }

    std::string username = "<error>";
    std::string last_ip = "<error>";
    uint32 security = 0;

    QueryResult* result = loginDatabase.PQuery("SELECT `username`,`gmlevel`,`last_ip` FROM `account` WHERE `id` = '%u'",accId);
    if(result)
    {
        Field* fields = result->Fetch();
        username = fields[0].GetCppString();
        security = fields[1].GetUInt32();
        if(m_session->GetSecurity() >= security)
            last_ip = fields[2].GetCppString();
        else
            last_ip = "-";

        delete result;
    }

    PSendSysMessage(LANG_PINFO_ACCOUNT, (target?"":"(offline)"), name.c_str(), GUID_LOPART(targetGUID), username.c_str(), accId, security, last_ip.c_str());

    std::string timeStr = secsToTimeString(total_player_time,true,true);
    uint32 gold = money /GOLD;
    uint32 silv = (money % GOLD) / SILVER;
    uint32 copp = (money % GOLD) % SILVER;
    PSendSysMessage(LANG_PINFO_LEVEL,  timeStr.c_str(), level, gold,silv,copp );

    if ( py && strncmp(py, "rep", 3) == 0 )
    {
        if(!target)
        {
            // rep option not implemented for offline case
            SendSysMessage(LANG_PINFO_NO_REP);
            return true;
        }

        static const char* ReputationRankStr[MAX_REPUTATION_RANK] = {"Hated", "Hostile", "Unfriendly", "Neutral", "Friendly", "Honored", "Reverted", "Exalted"};
        char* FactionName;
        for(FactionsList::const_iterator itr = target->m_factions.begin(); itr != target->m_factions.end(); ++itr)
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

        PSendSysMessage(LANG_COMMAND_TICKETCOUNT, count,m_session->GetPlayer()->isAcceptTickets() ?  "on" : "off");
        delete result;
        return true;
    }

    // ticket on
    if(strncmp(px,"on",3) == 0)
    {
        m_session->GetPlayer()->SetAcceptTicket(true);
        SendSysMessage(LANG_COMMAND_TICKETON);
        return true;
    }

    // ticket off
    if(strncmp(px,"off",4) == 0)
    {
        m_session->GetPlayer()->SetAcceptTicket(false);
        SendSysMessage(LANG_COMMAND_TICKETOFF);
        return true;
    }

    // ticket #num
    int num = atoi(px);
    if(num > 0)
    {
        QueryResult *result = sDatabase.Query("SELECT `guid`,`ticket_category`,`ticket_text` FROM `character_ticket`");

        if(!result || uint64(num) > result->GetRowCount())
        {
            PSendSysMessage(LANG_COMMAND_TICKENOTEXIST, num);
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
        PSendSysMessage(LANG_COMMAND_TICKENOTEXIST, num);
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
                sender->GetSession()->SendGMTicketGetTicket(0x0A,0);

        }while(result->NextRow());

        delete result;

        sDatabase.PExecute("DELETE FROM `character_ticket`");
        SendSysMessage(LANG_COMMAND_ALLTICKETDELETED);
        return true;
    }

    int num = (uint32)atoi(px);

    // delticket #num
    if(num > 0)
    {
        QueryResult *result = sDatabase.PQuery("SELECT `ticket_id`,`guid` FROM `character_ticket` LIMIT '%i'",num);

        if(!result || uint64(num) > result->GetRowCount())
        {
            PSendSysMessage(LANG_COMMAND_TICKENOTEXIST, num);
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
            sender->GetSession()->SendGMTicketGetTicket(0x0A,0);
            PSendSysMessage(LANG_COMMAND_TICKETPLAYERDEL,sender->GetName());
        }
        else
            SendSysMessage(LANG_COMMAND_TICKETDEL);

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
        sender->GetSession()->SendGMTicketGetTicket(0x0A,0);

    PSendSysMessage(LANG_COMMAND_TICKETPLAYERDEL,px);
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
        u_guid = pCreature->GetDBTableGUIDLow();
    else
        return false;

    sDatabase.PExecuteLog("UPDATE `creature` SET `spawndist`=%i, `MovementType`=%i WHERE `guid`=%u",option,mtype,u_guid);
    PSendSysMessage(LANG_COMMAND_SPAWNDIST,option);
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
        u_guid = pCreature->GetDBTableGUIDLow();
    else
        return false;

    sDatabase.PExecuteLog("UPDATE `creature` SET `spawntimesecs`=%i WHERE `guid`=%u",i_stime,u_guid);
    PSendSysMessage(LANG_COMMAND_SPAWNTIME,i_stime);

    return true;
}

/**
 * Add a waypoint to a creature.
 *
 * The user can either select an npc or provide its GUID.
 *
 * The user can even select a visual waypoint - then the new waypoint
 * is placed *after* the selected one - this makes insertion of new
 * waypoints possible.
 *
 * eg:
 * .wp add 12345
 * -> adds a waypoint to the npc with the GUID 12345
 *
 * .wp add
 * -> adds a waypoint to the currently selected creature
 *
 *
 * @param args if the user did not provide a GUID, it is NULL
 *
 * @return true - command did succeed, false - something went wrong
 */
bool ChatHandler::HandleWpAddCommand(const char* args)
{
    sLog.outDebug("DEBUG: HandleWpAddCommand");

    // optional
    char* guid_str = NULL;

    if(*args)
    {
        guid_str = strtok((char*)args, " ");
    }

    uint32 lowguid = 0;
    uint32 point = 0;
    Unit* target = getSelectedCreature();
    // Did player provide a GUID?
    if (!guid_str)
    {
        sLog.outDebug("DEBUG: HandleWpAddCommand - No GUID provided");

        // No GUID provided
        // -> Player must have selected a creature

        if(!target)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }
        if (target->GetEntry() == VISUAL_WAYPOINT )
        {
            sLog.outDebug("DEBUG: HandleWpAddCommand - target->GetEntry() == VISUAL_WAYPOINT (1) ");

            QueryResult *result =
                sDatabase.PQuery( "SELECT `id`, `point` FROM `creature_movement` WHERE `wpguid` = %u",
                target->GetGUIDLow() );
            if(!result)
            {
                PSendSysMessage("Creature (GUID: %u) No waypoint found - used 'wpguid'. Now trying to find it by its position...", target->GetGUIDLow());
                // User selected a visual spawnpoint -> get the NPC
                // Select NPC GUID
                // Since we compare float values, we have to deal with
                // some difficulties.
                // Here we search for all waypoints that only differ in one from 1 thousand
                // (0.001) - There is no other way to compare C++ floats with mySQL floats
                // See also: http://dev.mysql.com/doc/refman/5.0/en/problems-with-float.html
                const char* maxDIFF = "0.01";
                result = sDatabase.PQuery( "SELECT `id`, `point` FROM `creature_movement` WHERE (abs(`position_x` - %f) <= %s ) and (abs(`position_y` - %f) <= %s ) and (abs(`position_z` - %f) <= %s )",
                    target->GetPositionX(), maxDIFF, target->GetPositionY(), maxDIFF, target->GetPositionZ(), maxDIFF);
                if(!result)
                {
                    PSendSysMessage("Creature (GUID: %u) No waypoints found - This is a MaNGOS db problem (single float).", target->GetGUIDLow());
                    return true;
                }
            }
            do
            {
                Field *fields = result->Fetch();
                lowguid = fields[0].GetUInt32();
                point   = fields[1].GetUInt32();
            }while( result->NextRow() );
            delete result;
        }
        else
        {
            lowguid = target->GetGUIDLow();
        }

    }
    else
    {
        sLog.outDebug("DEBUG: HandleWpAddCommand - GUID provided");

        // GUID provided
        // Warn if player also selected a creature
        // -> Creature selection is ignored <-
        if(target)
        {
            SendSysMessage("Selected creature is ignored - provided GUID is used");
        }
        lowguid = atoi((char*)guid_str);
    }
    // lowguid -> GUID of the NPC
    // point   -> number of the waypoint (if not 0)
    sLog.outDebug("DEBUG: HandleWpAddCommand - danach");

    Creature* pCreature = NULL;

    if(lowguid)
        pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(lowguid,HIGHGUID_UNIT));

    // attempt check creature existance by DB
    if(!pCreature)
    {
        QueryResult *result = sDatabase.PQuery( "SELECT `guid` FROM `creature` WHERE `guid` = '%u'",lowguid);
        if(!result)
        {
            PSendSysMessage("Creature (GUID: %u) not found", lowguid);
            return true;
        }
        delete result;
    }

    sLog.outDebug("DEBUG: HandleWpAddCommand - point == 0");

    QueryResult *result = sDatabase.PQuery( "SELECT MAX(`point`) FROM `creature_movement` WHERE `id` = '%u'",lowguid);
    if( result )
    {
        point = (*result)[0].GetUInt32()+1;
        delete result;
    }
    else
        point = 1;

    Player* player = m_session->GetPlayer();

    sDatabase.PExecuteLog("INSERT INTO `creature_movement` (`id`,`point`,`position_x`,`position_y`,`position_z`,`orientation`) VALUES ('%u','%u','%f', '%f', '%f', '%f')",
        lowguid, point, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation());

    // update movement type
    sDatabase.PExecuteLog("UPDATE `creature` SET `MovementType` = '%u' WHERE `guid` = '%u'", WAYPOINT_MOTION_TYPE,lowguid);
    if(pCreature)
    {
        pCreature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
        (*pCreature)->Initialize();
        if(pCreature->isAlive())                            // dead creature will reset movement generator at respawn
        {
            pCreature->setDeathState(JUST_DIED);
            pCreature->Respawn();
        }
    }

    PSendSysMessage("Waypoint %i added to GUID: %d", point, lowguid);

    return true;
}                                                           // HandleWpAddCommand

/**
 * .wp modify emote | spell | text | del | move | add
 *
 * add -> add a WP after the selected visual waypoint
 *        User must select a visual waypoint and then issue ".wp modify add"
 *
 * emote <emoteID>
 *   User has selected a visual waypoint before.
 *   <emoteID> is added to this waypoint. Everytime the
 *   NPC comes to this waypoint, the emote is called.
 *
 * emote <GUID> <WPNUM> <emoteID>
 *   User has not selected visual waypoint before.
 *   For the waypoint <WPNUM> for the NPC with <GUID>
 *   an emote <emoteID> is added.
 *   Everytime the NPC comes to this waypoint, the emote is called.
 *
 *
 * info <GUID> <WPNUM> -> User did not select a visual waypoint and
 */
bool ChatHandler::HandleWpModifyCommand(const char* args)
{
    sLog.outDebug("DEBUG: HandleWpModifyCommand");

    if(!*args)
        return false;

    // first arg: add del text emote spell waittime move
    char* show_str = strtok((char*)args, " ");
    if (!show_str)
    {
        return false;
    }

    string show = show_str;
    // Check
    // Remember: "show" must also be the name of a column!
    if( (show != "emote") && (show != "spell") && (show != "aiscript")
        && (show != "text1") && (show != "text2") && (show != "text3") && (show != "text4") && (show != "text5")
        && (show != "waittime") && (show != "del") && (show != "move") && (show != "add")
        && (show != "model1") && (show != "model2") && (show != "orientation")
        && (show != "import") && (show != "export") )
    {
        return false;
    }

    // Next arg is: <GUID> <WPNUM> <ARGUMENT>
    char* arg_str = NULL;

    // Did user provide a GUID
    // or did the user select a creature?
    // -> variable lowguid is filled with the GUID of the NPC
    uint32 lowguid = 0;
    uint32 point = 0;
    uint32 wpGuid = 0;
    Unit* target = getSelectedCreature();

    if(target)
    {
        sLog.outDebug("DEBUG: HandleWpModifyCommand - User did select an NPC");

        // The visual waypoint
        Creature* wpCreature = NULL;
        // The NPC
        Creature* npcCreature = NULL;

        wpGuid = target->GetGUIDLow();

        // Did the user select a visual spawnpoint?
        if(wpGuid)
            wpCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(wpGuid,HIGHGUID_UNIT));

        // attempt check creature existance by DB
        if(!wpCreature)
        {
            QueryResult *result = sDatabase.PQuery( "SELECT `guid` FROM `creature` WHERE `guid` = '%u'",wpGuid);
            if(!result)
            {
                PSendSysMessage("Creature (GUID: %u) not found", wpGuid);
                return true;
            }
            delete result;
        }
        // User did select a visual waypoint?
        // Check the creature
        if (wpCreature->GetEntry() != VISUAL_WAYPOINT )
        {
            if( (show != "export") && (show != "import") )
            {
                PSendSysMessage("You must select a visual waypoint.");
                return true;
            }
            lowguid = wpGuid;
            arg_str = strtok((char*)NULL, " ");
        }
        else
        {
            QueryResult *result =
                sDatabase.PQuery( "SELECT `id`, `point` FROM `creature_movement` WHERE `wpguid` = %u",
                target->GetGUIDLow() );
            if(!result)
            {
                sLog.outDebug("DEBUG: HandleWpModifyCommand - No waypoint found - used 'wpguid'");

                PSendSysMessage("Creature (GUID: %u) No waypoint found - used 'wpguid'. Now trying to find it by its position...", target->GetGUIDLow());
                // Select waypoint number from database
                // Since we compare float values, we have to deal with
                // some difficulties.
                // Here we search for all waypoints that only differ in one from 1 thousand
                // (0.001) - There is no other way to compare C++ floats with mySQL floats
                // See also: http://dev.mysql.com/doc/refman/5.0/en/problems-with-float.html
                const char* maxDIFF = "0.01";
                result = sDatabase.PQuery( "SELECT `id`, `point` FROM `creature_movement` WHERE (abs(`position_x` - %f) <= %s ) and (abs(`position_y` - %f) <= %s ) and (abs(`position_z` - %f) <= %s )",
                    wpCreature->GetPositionX(), maxDIFF, wpCreature->GetPositionY(), maxDIFF, wpCreature->GetPositionZ(), maxDIFF);
                if(!result)
                {
                    PSendSysMessage("Creature (GUID: %u) No waypoints found - This is a MaNGOS db problem (single float).", wpGuid);
                    return true;
                }
            }
            sLog.outDebug("DEBUG: HandleWpModifyCommand - After getting wpGuid");

            do
            {
                Field *fields = result->Fetch();
                lowguid = fields[0].GetUInt32();
                point   = fields[1].GetUInt32();
            }while( result->NextRow() );

            // Cleanup memory
            sLog.outDebug("DEBUG: HandleWpModifyCommand - Cleanup memory");
            delete result;
            // We have the waypoint number and the GUID of the "master npc"
            // Text is enclosed in "<>", all other arguments not
            if( show.find("text") != string::npos )
            {
                arg_str = strtok((char*)NULL, "<>");
            }
            else
            {
                arg_str = strtok((char*)NULL, " ");
            }
        }
    }
    else
    {
        // User did provide <GUID> <WPNUM>
        char* guid_str = strtok((char*)NULL, " ");
        char* point_str = strtok((char*)NULL, " ");

        // Text is enclosed in "<>", all other arguments not
        if( show.find("text") != string::npos )
        {
            arg_str = strtok((char*)NULL, "<>");
        }
        else
        {
            arg_str = strtok((char*)NULL, " ");
        }

        if( !guid_str )
        {
            SendSysMessage("No GUID provided.");
            return false;
        }
        if( !point_str )
        {
            SendSysMessage("No waypoint number provided.");
            return false;
        }
        if( (show != "del") && (show != "move") && (show != "add") )
        {
            if( !arg_str )
            {
                PSendSysMessage("Argument required for '%s'.", show.c_str());
                return false;
            }
        }
        lowguid = atoi((char*)guid_str);
        PSendSysMessage("DEBUG: GUID provided: %d", lowguid);
        point    = atoi((char*)point_str);
        PSendSysMessage("DEBUG: wpNumber provided: %d", point);

        // Now we need the GUID of the visual waypoint
        // -> "del", "move", "add" command

        QueryResult *result = sDatabase.PQuery( "SELECT `wpguid` FROM `creature_movement` WHERE `id` = '%u' AND `point` = '%u'", lowguid, point);
        if(result)
        {
            do
            {
                Field *fields = result->Fetch();
                wpGuid  = fields[0].GetUInt32();
            }while( result->NextRow() );

            // Free memory
            delete result;
        }
        else
        {
            PSendSysMessage("Creature (GUID: %u), visual waypoint number '%u' not found. Now trying to find it by its position...", lowguid, point);
            // Select waypoint number from database
            QueryResult *result = sDatabase.PQuery( "SELECT `position_x`,`position_y`,`position_z` FROM `creature_movement` WHERE `point`='%d' AND `id` = '%u'", point, lowguid);
            if(result)
            {
                PSendSysMessage("Creature (GUID: %u) No waypoint found.", lowguid);
                return true;
            }

            Field *fields = result->Fetch();
            float x         = fields[0].GetFloat();
            float y         = fields[1].GetFloat();
            float z         = fields[2].GetFloat();
            // Cleanup memory
            delete result;

            // Select waypoint number from database
            // Since we compare float values, we have to deal with
            // some difficulties.
            // Here we search for all waypoints that only differ in one from 1 thousand
            // (0.001) - There is no other way to compare C++ floats with mySQL floats
            // See also: http://dev.mysql.com/doc/refman/5.0/en/problems-with-float.html
            const char* maxDIFF = "0.01";
            QueryResult *result2 = sDatabase.PQuery( "SELECT `guid` FROM `creature` WHERE (abs(`position_x` - %f) <= %s ) and (abs(`position_y` - %f) <= %s ) and (abs(`position_z` - %f) <= %s ) and `id`=%d",
                x, maxDIFF, y, maxDIFF, z, maxDIFF, VISUAL_WAYPOINT);
            if(!result)
            {
                PSendSysMessage("Waypoint-Creature (GUID: %u) Not found", VISUAL_WAYPOINT);
                return true;
            }
            do
            {
                Field *fields = result2->Fetch();
                wpGuid  = fields[0].GetUInt32();
            }while( result->NextRow() );

            // Free memory
            delete result2;
        }

    }

    sLog.outDebug("DEBUG: HandleWpModifyCommand - Parameters parsed - now execute the command");

    // Check for argument
    if( (show.find("text") == string::npos ) && (show != "del") && (show != "move") && (show != "add") && (show != "aiscript"))
    {
        if( arg_str == NULL )
        {
            PSendSysMessage("You did not provide an argument for %s.", show_str);
            return false;
        }
    }
    // wpGuid  -> GUID of the waypoint creature
    // lowguid -> GUID of the NPC
    // point   -> waypoint number

    // Special functions:
    // add - move - del -> no args commands
    // Add a waypoint after the selected visual
    if(show == "add")
    {
        PSendSysMessage("DEBUG: wp modify add, GUID: %u", lowguid);

        // Get the creature for which we read the waypoint
        Creature* npcCreature = NULL;
        npcCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), MAKE_GUID(lowguid, HIGHGUID_UNIT));

        if( !npcCreature )
        {
            PSendSysMessage("Could not find NPC...");
            return true;
        }
        else
        {
            // obtain real GUID for DB operations
            lowguid = npcCreature->GetDBTableGUIDLow();
        }
        sLog.outDebug("DEBUG: HandleWpModifyCommand - add -- npcCreature");

        // What to do:
        // Add the visual spawnpoint (DB only)
        // Adjust the waypoints
        // Respawn the owner of the waypoints
        sLog.outDebug("DEBUG: HandleWpModifyCommand - add -- SELECT MAX(`point`)");
        QueryResult *result = sDatabase.PQuery( "SELECT MAX(`point`) FROM `creature_movement` WHERE `id` = '%u'",lowguid);
        uint32 maxPoint = 0;
        if( result )
        {
            maxPoint = (*result)[0].GetUInt32();
            delete result;
        }
        for( int i=maxPoint; i>point; i-- )
        {
            sDatabase.PExecuteLog("UPDATE `creature_movement` SET point=point+1 WHERE id='%u' AND point='%u'",
                lowguid, i);
        }

        Player* chr = m_session->GetPlayer();
        sDatabase.PExecuteLog("INSERT INTO `creature_movement` (`id`,`point`,`position_x`,`position_y`,`position_z`) VALUES ('%u','%u','%f', '%f', '%f')",
            lowguid, point+1, chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ());

        if(npcCreature)
        {
            (*npcCreature)->Initialize();
            if(npcCreature->isAlive())                      // dead creature will reset movement generator at respawn
            {
                npcCreature->setDeathState(JUST_DIED);
                npcCreature->Respawn();
            }
        }

        Creature* wpCreature = new Creature(chr);
        if (!wpCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), chr->GetMapId(),
            chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ(), chr->GetOrientation(), VISUAL_WAYPOINT))
        {
            PSendSysMessage("Could not create visual waypoint with creatureID: %d", VISUAL_WAYPOINT);
            delete wpCreature;
            return false;
        }
        sDatabase.PExecuteLog("UPDATE `creature_movement` SET `wpguid` = '%u' WHERE `id` = '%u' and `point` = '%u'", wpCreature->GetGUIDLow(), lowguid, point+1);

        wpCreature->SaveToDB();
        wpCreature->LoadFromDB(wpCreature->GetGUIDLow(), NULL, chr->GetInstanceId());
        // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells();
        wpCreature->AddToWorld();
        MapManager::Instance().GetMap(wpCreature->GetMapId(), wpCreature)->Add(wpCreature);

        PSendSysMessage("Waypoint %d added.", point+1);
        return true;
    }                                                       // add

    if(show == "del")
    {
        PSendSysMessage("DEBUG: wp modify del, GUID: %u", lowguid);

        // Get the creature for which we read the waypoint
        Creature* npcCreature = NULL;
        npcCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(lowguid, HIGHGUID_UNIT));

        // wpCreature
        Creature* wpCreature = NULL;
        if( wpGuid != 0 )
        {
            wpCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(wpGuid, HIGHGUID_UNIT));
            wpCreature->CombatStop(true);
            wpCreature->DeleteFromDB();
            ObjectAccessor::Instance().AddObjectToRemoveList(wpCreature);
        }

        // What to do:
        // Remove the visual spawnpoint
        // Adjust the waypoints
        // Respawn the owner of the waypoints

        sDatabase.PExecuteLog("DELETE FROM `creature_movement` WHERE id='%u' AND point='%u'",
            lowguid, point);
        sDatabase.PExecuteLog("UPDATE `creature_movement` SET point=point-1 WHERE id='%u' AND point>'%u'",
            lowguid, point);

        if(npcCreature)
        {
            // Any waypoints left?
            QueryResult *result2 = sDatabase.PQuery( "SELECT `point` FROM `creature_movement` WHERE `id` = '%u'",lowguid);
            if(!result2)
            {
                npcCreature->SetDefaultMovementType(RANDOM_MOTION_TYPE);
            }
            else
            {
                npcCreature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                delete result2;
            }
            (*npcCreature)->Initialize();
            if(npcCreature->isAlive())                      // dead creature will reset movement generator at respawn
            {
                npcCreature->setDeathState(JUST_DIED);
                npcCreature->Respawn();
            }
        }

        PSendSysMessage("Waypoint removed.");
        return true;
    }                                                       // del

    if(show == "move")
    {
        PSendSysMessage("DEBUG: wp move, GUID: %u", lowguid);

        Player *chr = m_session->GetPlayer();
        {
            // Get the creature for which we read the waypoint
            Creature* npcCreature = NULL;
            npcCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(lowguid,HIGHGUID_UNIT));

            // wpCreature
            Creature* wpCreature = NULL;
            // What to do:
            // Move the visual spawnpoint
            // Respawn the owner of the waypoints
            if( wpGuid != 0 )
            {
                wpCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(wpGuid, HIGHGUID_UNIT));
                wpCreature->CombatStop(true);
                wpCreature->DeleteFromDB();
                ObjectAccessor::Instance().AddObjectToRemoveList(wpCreature);
                // re-create
                Creature* wpCreature2 = new Creature(chr);
                if (!wpCreature2->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), chr->GetMapId(),
                    chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ(), chr->GetOrientation(), VISUAL_WAYPOINT))
                {
                    PSendSysMessage("Could not create visual waypoint with creatureID: %d", VISUAL_WAYPOINT);
                    delete wpCreature2;
                    return false;
                }
                wpCreature2->SaveToDB();
                                                            // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells();
                wpCreature2->LoadFromDB(wpCreature2->GetGUIDLow(), NULL, chr->GetInstanceId());
                wpCreature2->AddToWorld();
                MapManager::Instance().GetMap(npcCreature->GetMapId(), npcCreature)->Add(wpCreature2);
                //MapManager::Instance().GetMap(npcCreature->GetMapId())->Add(wpCreature2);
            }

            sDatabase.PExecuteLog("UPDATE `creature_movement` SET `position_x` = '%f',`position_y` = '%f',`position_z` = '%f' where `id` = '%u' AND point='%u'",
                chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ(), lowguid, point );

            if(npcCreature)
            {
                (*npcCreature)->Initialize();
                if(npcCreature->isAlive())                  // dead creature will reset movement generator at respawn
                {
                    npcCreature->setDeathState(JUST_DIED);
                    npcCreature->Respawn();
                }
            }
            PSendSysMessage("Waypoint changed.");
        }
        return true;
    }                                                       // move

    if(show == "export")
    {
        PSendSysMessage("DEBUG: wp export, GUID: %u", lowguid);

        ofstream outfile;
        outfile.open (arg_str);

        QueryResult *result = sDatabase.PQuery(
        //        0          1             2             3              4             5      6           7          8         9        10        11    12        13       14        15          16
            "SELECT `point`, `position_x`, `position_y`, `position_z`, `orientation`, `model1`, `model2`, `waittime`, `emote`, `spell`, `text1`, `text2`, `text3`, `text4`, `text5`, `aiscript`, `id` FROM `creature_movement` WHERE `id` = '%u' ORDER BY `point`", lowguid );

        if( result )
        {
            do
            {
                Field *fields = result->Fetch();

                outfile << "insert into creature_movement ";
                outfile << "( `id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `model1`, `model2`, `waittime`, `emote`, `spell`, `text1`, `text2`, `text3`, `text4`, `text5`, `aiscript` ) values ";

                //sLog.outDebug("( ");
                outfile << "( ";
                //sLog.outDebug("id");
                outfile << fields[16].GetUInt32();          // id
                outfile << ", ";
                //sLog.outDebug("point");
                outfile << fields[0].GetUInt32();           // point
                outfile << ", ";
                //sLog.outDebug("position_x");
                outfile << fields[1].GetFloat();            // position_x
                outfile << ", ";
                //sLog.outDebug("position_y");
                outfile << fields[2].GetFloat();            // position_y
                outfile << ", ";
                //sLog.outDebug("position_z");
                outfile << fields[3].GetUInt32();           // position_z
                outfile << ", ";
                //sLog.outDebug("orientation");
                outfile << fields[4].GetUInt32();           // orientation
                outfile << ", ";
                //sLog.outDebug("model1");
                outfile << fields[5].GetUInt32();           // model1
                outfile << ", ";
                //sLog.outDebug("model2");
                outfile << fields[6].GetUInt32();           // model2
                outfile << ", ";
                //sLog.outDebug("waittime");
                outfile << fields[7].GetUInt16();           // waittime
                outfile << ", ";
                //sLog.outDebug("emote");
                outfile << fields[8].GetUInt32();           // emote
                outfile << ", ";
                //sLog.outDebug("spell");
                outfile << fields[9].GetUInt32();           // spell
                outfile << ", ";
                //sLog.outDebug("text1");
                const char *tmpChar = fields[10].GetString();
                if( !tmpChar )
                {
                    outfile << "NULL";                      // text1
                }
                else
                {
                    outfile << "'";
                    outfile << tmpChar;                     // text1
                    outfile << "'";
                }
                outfile << ", ";
                //sLog.outDebug("text2");
                tmpChar = fields[11].GetString();
                if( !tmpChar )
                {
                    outfile << "NULL";                      // text2
                }
                else
                {
                    outfile << "'";
                    outfile << tmpChar;                     // text2
                    outfile << "'";
                }
                outfile << ", ";
                //sLog.outDebug("text3");
                tmpChar = fields[12].GetString();
                if( !tmpChar )
                {
                    outfile << "NULL";                      // text3
                }
                else
                {
                    outfile << "'";
                    outfile << tmpChar;                     // text3
                    outfile << "'";
                }
                outfile << ", ";
                //sLog.outDebug("text4");
                tmpChar = fields[13].GetString();
                if( !tmpChar )
                {
                    outfile << "NULL";                      // text4
                }
                else
                {
                    outfile << "'";
                    outfile << tmpChar;                     // text4
                    outfile << "'";
                }
                outfile << ", ";
                //sLog.outDebug("text5");
                tmpChar = fields[14].GetString();
                if( !tmpChar )
                {
                    outfile << "NULL";                      // text5
                }
                else
                {
                    outfile << "'";
                    outfile << tmpChar;                     // text5
                    outfile << "'";
                }
                outfile << ", ";
                //sLog.outDebug("aiscript");
                tmpChar = fields[15].GetString();
                if( !tmpChar )
                {
                    outfile << "NULL";                      // aiscript
                }
                else
                {
                    outfile << "'";
                    outfile << tmpChar;                     // aiscript
                    outfile << "'";
                }
                outfile << ");\n ";

            } while( result->NextRow() );
            delete result;

            PSendSysMessage("WP export successfull.");
        }
        else
        {
            PSendSysMessage("No waypoints found inside the database.");
            outfile << "No waypoints found inside the database.";
        }
        outfile.close();

        return true;
    }
    if(show == "import")
    {
        PSendSysMessage("DEBUG: wp import, GUID: %u", lowguid);

        string line;
        ifstream infile (arg_str);
        if (infile.is_open())
        {
            while (! infile.eof() )
            {
                getline (infile,line);
                //cout << line << endl;
                QueryResult *result = sDatabase.PQuery(line.c_str());
                delete result;
            }
            infile.close();
        }
        PSendSysMessage("File imported.");

        return true;
    }

    // Create creature - npc that has the waypoint
    Creature* npcCreature = NULL;

    if(lowguid)
        npcCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(lowguid,HIGHGUID_UNIT));

    // attempt check creature existance by DB
    if(!npcCreature)
    {
        QueryResult *result = sDatabase.PQuery( "SELECT `guid` FROM `creature` WHERE `guid` = '%u'",lowguid);
        if(!result)
        {
            PSendSysMessage("Creature (GUID: %u) not found", lowguid);
            return true;
        }
        delete result;
    }

    const char *text = arg_str;

    Player* player = m_session->GetPlayer();

    if( text == 0 )
    {
        sDatabase.PExecuteLog("UPDATE `creature_movement` SET %s=NULL WHERE id='%u' AND point='%u'",
            show_str, lowguid, point);
    }
    else
    {
        sDatabase.PExecuteLog("UPDATE `creature_movement` SET %s='%s' WHERE id='%u' AND point='%u'",
            show_str, text, lowguid, point);
    }

    if(npcCreature)
    {
        npcCreature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
        (*npcCreature)->Initialize();
        if(npcCreature->isAlive())                          // dead creature will reset movement generator at respawn
        {
            npcCreature->setDeathState(JUST_DIED);
            npcCreature->Respawn();
        }
    }
    PSendSysMessage("Waypoint %s modified.", show_str);

    return true;
}

/**
 * .wp show info | on | off
 *
 * info -> User has selected a visual waypoint before
 *
 * info <GUID> <WPNUM> -> User did not select a visual waypoint and
 *                        provided the GUID of the NPC and the number of
 *                        the waypoint.
 *
 * on -> User has selected an NPC; all visual waypoints for this
 *       NPC are added to the world
 *
 * on <GUID> -> User did not select an NPC - instead the GUID of the
 *              NPC is provided. All visual waypoints for this NPC
 *              are added from the world.
 *
 * off -> User has selected an NPC; all visual waypoints for this
 *        NPC are removed from the world.
 *
 * on <GUID> -> User did not select an NPC - instead the GUID of the
 *              NPC is provided. All visual waypoints for this NPC
 *              are removed from the world.
 *
 *
 */
bool ChatHandler::HandleWpShowCommand(const char* args)
{
    sLog.outDebug("DEBUG: HandleWpShowCommand");

    if(!*args)
        return false;

    sLog.outCommand("DEBUG: HandleWpShowCommand - nach args");

    // first arg: on, off, first, last
    char* show_str = strtok((char*)args, " ");
    if (!show_str)
    {
        return false;
    }
    // second arg: GUID (optional, if a creature is selected)
    char* guid_str = strtok((char*)NULL, " ");
    sLog.outDebug("DEBUG: HandleWpShowCommand: show_str: %s", show_str);
    sLog.outDebug("DEBUG: HandleWpShowCommand: guid_str: %s", guid_str);
    //if (!guid_str) {
    //	return false;
    //}

    // Did user provide a GUID
    // or did the user select a creature?
    // -> variable lowguid is filled with the GUID
    uint32 lowguid = 0;
    Unit* target = getSelectedCreature();
    // Did player provide a GUID?
    if (!guid_str)
    {
        sLog.outDebug("DEBUG: HandleWpShowCommand: !guid_str");
        // No GUID provided
        // -> Player must have selected a creature

        if(!target)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }
        lowguid = target->GetGUIDLow();
    }
    else
    {
        sLog.outDebug("DEBUG: HandleWpShowCommand: GUID provided");
        // GUID provided
        // Warn if player also selected a creature
        // -> Creature selection is ignored <-
        if(target)
        {
            SendSysMessage("Selected creature is ignored - provided GUID is used");
        }
        lowguid = atoi((char*)guid_str);
    }

    sLog.outDebug("DEBUG: HandleWpShowCommand: danach");

    string show = show_str;
    uint32 Maxpoint;

    Creature* pCreature = NULL;

    sLog.outDebug("DEBUG: HandleWpShowCommand: lowguid: %u", lowguid);
    if(lowguid)
        pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(lowguid,HIGHGUID_UNIT));

    sLog.outDebug("DEBUG: HandleWpShowCommand: Habe creature: %ld", pCreature );
    // attempt check creature existance by DB
    if(!pCreature)
    {
        QueryResult *result = sDatabase.PQuery( "SELECT `guid` FROM `creature` WHERE `guid` = '%u'",lowguid);
        if(!result)
        {
            PSendSysMessage("Creature (GUID: %u) not found", lowguid);
            return true;
        }
        delete result;
    }

    sLog.outDebug("DEBUG: HandleWpShowCommand: wpshow - show: %s", show_str);
    //PSendSysMessage("wpshow - show: %s", show);

    // Show info for the selected waypoint
    if(show == "info")
    {
        PSendSysMessage("DEBUG: wp info, GUID: %u", lowguid);

        // Check if the user did specify a visual waypoint
        if( pCreature->GetEntry() != VISUAL_WAYPOINT )
        {
            PSendSysMessage("No visual waypoint selected.");
            return true;
        }

        //PSendSysMessage("wp on, GUID: %u", lowguid);

        //pCreature->GetPositionX();

        QueryResult *result =
            sDatabase.PQuery( "SELECT `id`, `point`, `waittime`, `emote`, `spell`, `text1`, `text2`, `text3`, `text4`, `text5`, `model1`, `model2`, `aiscript` FROM `creature_movement` WHERE `wpguid` = %u",
            pCreature->GetGUID() );
        if(!result)
        {
            // Since we compare float values, we have to deal with
            // some difficulties.
            // Here we search for all waypoints that only differ in one from 1 thousand
            // (0.001) - There is no other way to compare C++ floats with mySQL floats
            // See also: http://dev.mysql.com/doc/refman/5.0/en/problems-with-float.html
            const char* maxDIFF = "0.01";
            PSendSysMessage("Creature (GUID: %u) No waypoint found - used 'wpguid'. Now trying to find it by its position...", pCreature->GetGUID());

            result = sDatabase.PQuery( "SELECT `id`, `point`, `waittime`, `emote`, `spell`, `text1`, `text2`, `text3`, `text4`, `text5`, `model1`, `model2`, `aiscript` FROM `creature_movement` WHERE (abs(`position_x` - %f) <= %s ) and (abs(`position_y` - %f) <= %s ) and (abs(`position_z` - %f) <= %s )",
                pCreature->GetPositionX(), maxDIFF, pCreature->GetPositionY(), maxDIFF, pCreature->GetPositionZ(), maxDIFF);
            if(!result)
            {
                PSendSysMessage("Creature (GUID: %u) No waypoints found - This is a MaNGOS db problem (single float).", lowguid);
                return true;
            }
        }
        do
        {
            Field *fields = result->Fetch();
            uint32 creGUID          = fields[0].GetUInt32();
            uint32 point            = fields[1].GetUInt32();
            int waittime            = fields[2].GetUInt32();
            uint32 emote            = fields[3].GetUInt32();
            uint32 spell            = fields[4].GetUInt32();
            const char * text1      = fields[5].GetString();
            const char * text2      = fields[6].GetString();
            const char * text3      = fields[7].GetString();
            const char * text4      = fields[8].GetString();
            const char * text5      = fields[9].GetString();
            uint32 model1           = fields[10].GetUInt32();
            uint32 model2           = fields[11].GetUInt32();
            const char * aiscript   = fields[12].GetString();
            // Get the creature for which we read the waypoint
            Creature* wpCreature = NULL;

            wpCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(creGUID,HIGHGUID_UNIT));

            PSendSysMessage("Waypoint %d: Info for creature: %s, GUID: %d", point, wpCreature->GetName(), creGUID);
            PSendSysMessage("Waittime: %d", waittime);
            PSendSysMessage("Model 1: %d", model1);
            PSendSysMessage("Model 2: %d", model2);
            PSendSysMessage("Emote: %d", emote);
            PSendSysMessage("Spell: %d", spell);
            PSendSysMessage("Text1: %s", text1);
            PSendSysMessage("Text2: %s", text2);
            PSendSysMessage("Text3: %s", text3);
            PSendSysMessage("Text4: %s", text4);
            PSendSysMessage("Text5: %s", text5);
            PSendSysMessage("AIScript: %s", aiscript);

        }while( result->NextRow() );
        // Cleanup memory
        delete result;
        return true;
    }

    if(show == "on")
    {
        PSendSysMessage("DEBUG: wp on, GUID: %u", lowguid);

        QueryResult *result = sDatabase.PQuery( "SELECT `point`, `position_x`,`position_y`,`position_z` FROM `creature_movement` WHERE `id` = '%u'",lowguid);
        if(!result)
        {
            PSendSysMessage("Creature (GUID: %u) No waypoints found", lowguid);
            return true;
        }
        // Delete all visuals for this NPC
        QueryResult *result2 = sDatabase.PQuery( "SELECT `wpguid` FROM `creature_movement` WHERE `id` = '%u' and wpguid <> 0", lowguid);
        if(result2)
        {
            bool hasError = false;
            do
            {
                Field *fields = result2->Fetch();
                uint32 wpguid = fields[0].GetUInt32();
                Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(wpguid,HIGHGUID_UNIT));

                if(!pCreature)
                {
                    PSendSysMessage("Warning: Could not delete WP from the world with ID: %d", wpguid);
                    hasError = true;
                    sDatabase.PExecuteLog("DELETE FROM `creature` WHERE `guid` = '%u'", wpguid);
                }
                else
                {
                    pCreature->CombatStop(true);
                    pCreature->DeleteFromDB();
                    ObjectAccessor::Instance().AddObjectToRemoveList(pCreature);
                }

            }while( result2->NextRow() );
            delete result2;
            if( hasError )
            {
                PSendSysMessage("This happens if the waypoint is too far away from your char.");
                PSendSysMessage("The WP is deleted from the database, but not from the world here.");
                PSendSysMessage("They will disappear after a server restart.");
            }
        }

        do
        {
            Field *fields = result->Fetch();
            uint32 point    = fields[0].GetUInt32();
            float x         = fields[1].GetFloat();
            float y         = fields[2].GetFloat();
            float z         = fields[3].GetFloat();

            uint32 id = VISUAL_WAYPOINT;

            Player *chr = m_session->GetPlayer();
            float o = chr->GetOrientation();

            Creature* wpCreature = new Creature(chr);
            if (!wpCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), chr->GetMapId(), x, y, z, o, id))
            {
                PSendSysMessage("Could not create visual waypoint with creatureID: %d", id);
                delete wpCreature;
                return false;
            }
            wpCreature->SetVisibility(VISIBILITY_OFF);
            sLog.outDebug("DEBUG: UPDATE `creature_movement` SET `wpguid` = '%u");
            // set "wpguid" column to the visual waypoint
            sDatabase.PExecuteLog("UPDATE `creature_movement` SET `wpguid` = '%u' WHERE `id` = '%u' and `point` = '%u'", wpCreature->GetGUIDLow(), lowguid, point);

            wpCreature->SaveToDB();
                                                            // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells();
            wpCreature->LoadFromDB(wpCreature->GetGUIDLow(), NULL, chr->GetInstanceId());
            wpCreature->AddToWorld();
            MapManager::Instance().GetMap(wpCreature->GetMapId(), wpCreature)->Add(wpCreature);
            //MapManager::Instance().GetMap(wpCreature->GetMapId())->Add(wpCreature);
        }while( result->NextRow() );

        // Cleanup memory
        delete result;
        return true;
    }

    if(show == "first")
    {
        PSendSysMessage("DEBUG: wp first, GUID: %u", lowguid);

        QueryResult *result = sDatabase.PQuery( "SELECT `position_x`,`position_y`,`position_z` FROM `creature_movement` WHERE `point`='1' AND `id` = '%u'",lowguid);
        if(!result)
        {
            PSendSysMessage("Creature (GUID: %u) No waypoints found.", lowguid);
            return true;
        }

        Field *fields = result->Fetch();
        float x         = fields[0].GetFloat();
        float y         = fields[1].GetFloat();
        float z         = fields[2].GetFloat();
        uint32 id = VISUAL_WAYPOINT;

        Player *chr = m_session->GetPlayer();
        float o = chr->GetOrientation();

        Creature* pCreature = new Creature(chr);
        if (!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), chr->GetMapId(), x, y, z, o, id))
        {
            PSendSysMessage("Could not create visual waypoint with creatureID: %d", id);
            delete pCreature;
            return false;
        }

        pCreature->SaveToDB();
        pCreature->LoadFromDB(pCreature->GetGUIDLow(), NULL, chr->GetInstanceId());
        pCreature->AddToWorld();
        MapManager::Instance().GetMap(pCreature->GetMapId(), pCreature)->Add(pCreature);
        //player->PlayerTalkClass->SendPointOfInterest(x, y, 6, 6, 0, "First Waypoint");

        // Cleanup memory
        delete result;
        return true;
    }

    if(show == "last")
    {
        PSendSysMessage("DEBUG: wp last, GUID: %u", lowguid);

        QueryResult *result = sDatabase.PQuery( "SELECT MAX(`point`) FROM `creature_movement` WHERE `id` = '%u'",lowguid);
        if( result )
        {
            Maxpoint = (*result)[0].GetUInt32();

            delete result;
        }
        else
            Maxpoint = 0;

        QueryResult *result1 = sDatabase.PQuery( "SELECT `position_x`,`position_y`,`position_z` FROM `creature_movement` WHERE `point` ='%u' AND `id` = '%u'",Maxpoint, lowguid);
        if(!result1)
        {
            PSendSysMessage("Creature (GUID: %u) Last waypoint not found.", lowguid);
            return true;
        }
        Field *fields = result1->Fetch();
        float x         = fields[0].GetFloat();
        float y         = fields[1].GetFloat();
        float z         = fields[2].GetFloat();
        uint32 id = VISUAL_WAYPOINT;

        Player *chr = m_session->GetPlayer();
        float o = chr->GetOrientation();

        Creature* pCreature = new Creature(chr);
        if (!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), chr->GetMapId(), x, y, z, o, id))
        {
            PSendSysMessage("Could not create waypoint-creature with ID: %d", id);
            delete pCreature;
            return false;
        }

        pCreature->SaveToDB();
        pCreature->LoadFromDB(pCreature->GetGUIDLow(), NULL, chr->GetInstanceId());
        pCreature->AddToWorld();
        MapManager::Instance().GetMap(pCreature->GetMapId(), pCreature)->Add(pCreature);
        //player->PlayerTalkClass->SendPointOfInterest(x, y, 6, 6, 0, "Last Waypoint");
        // Cleanup memory
        delete result1;
        return true;
    }

    if(show == "off")
    {
        QueryResult *result = sDatabase.PQuery("SELECT `guid` FROM `creature` WHERE `id` = '%d'", VISUAL_WAYPOINT);
        if(!result)
        {
            SendSysMessage("No visual waypoints found");
            return true;
        }
        bool hasError = false;
        do
        {
            Field *fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),MAKE_GUID(guid,HIGHGUID_UNIT));

            //Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

            if(!pCreature)
            {
                PSendSysMessage("Warning: Could not delete WP from the world with ID: %d", guid);
                hasError = true;
                sDatabase.PExecuteLog("DELETE FROM `creature` WHERE `guid` = '%u'", guid);
            }
            else
            {
                pCreature->CombatStop(true);

                pCreature->DeleteFromDB();

                ObjectAccessor::Instance().AddObjectToRemoveList(pCreature);
            }
        }while(result->NextRow());
        // set "wpguid" column to "empty" - no visual waypoint spawned
        sDatabase.PExecuteLog("UPDATE `creature_movement` SET `wpguid` = '0'");

        if( hasError )
        {
            PSendSysMessage("This happens if the waypoint is too far away from your char.");
            PSendSysMessage("The WP is deleted from the database, but not from the world here.");
            PSendSysMessage("They will disappear after a server restart.");
        }

        SendSysMessage("All visual waypoints removed");
        // Cleanup memory
        delete result;

        return true;
    }

    PSendSysMessage("DEBUG: wpshow - no valid command found");

    return true;
}                                                           // HandleWpShowCommand
