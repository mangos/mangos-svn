/* Chat.cpp
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "Chat.h"

createFileSingleton( ChatHandler );

#ifdef ENABLE_GRID_SYSTEM
#include "MapManager.h"
#endif

ChatHandler::ChatHandler()
{

}


ChatHandler::~ChatHandler()
{

}


ChatCommand * ChatHandler::getCommandTable()
{
    // TODO: change default security levels?

    static bool first_call = true;

    static ChatCommand modifyCommandTable[] =
    {
        { "hp",          1, &ChatHandler::HandleModifyHPCommand,      "",   NULL },
        { "mana",        1, &ChatHandler::HandleModifyManaCommand,    "",   NULL },
        { "rage",        1, &ChatHandler::HandleModifyRageCommand,    "",   NULL },
        { "energy",      1, &ChatHandler::HandleModifyEnergyCommand,  "",   NULL },
        { "gold",        1, &ChatHandler::HandleModifyGoldCommand,    "",   NULL },
        { "level",       1, &ChatHandler::HandleModifyLevelCommand,   "",   NULL },
        { "speed",       1, &ChatHandler::HandleModifySpeedCommand,   "",   NULL },
        { "swim",        1, &ChatHandler::HandleModifySwimCommand,    "",   NULL },
        { "scale",       1, &ChatHandler::HandleModifyScaleCommand,   "",   NULL },
        { "mount",       1, &ChatHandler::HandleModifyMountCommand,   "",   NULL },
        { "bit",         1, &ChatHandler::HandleModifyBitCommand,     "",   NULL },
        { "bwalk",       1, &ChatHandler::HandleModifyBWalkCommand,   "",   NULL },
        { "aspeed",      1, &ChatHandler::HandleModifyASpedCommand,   "",   NULL },
        { "faction",     1, &ChatHandler::HandleModifyFactionCommand, "",   NULL },
        { NULL,          0, NULL,                                     "",   NULL }
    };

    static ChatCommand debugCommandTable[] =
    {
        { "inarc",       1, &ChatHandler::HandleDebugInArcCommand,    "",   NULL },
        { NULL,          0, NULL,                                     "",   NULL }
    };

    // in alphabetical order
    static ChatCommand commandTable[] =
    {
        { "acct",        0, &ChatHandler::HandleAcctCommand,          "",   NULL },
        { "addmove",     2, &ChatHandler::HandleAddMoveCommand,       "",   NULL },
        { "addspirit",   3, &ChatHandler::HandleAddSpiritCommand,     "",   NULL },
        { "anim",        3, &ChatHandler::HandleAnimCommand,          "",   NULL },
        { "announce",    1, &ChatHandler::HandleAnnounceCommand,      "",   NULL },
        { "appear",      1, &ChatHandler::HandleAppearCommand,        "",   NULL },
        { "aura",        3, &ChatHandler::HandleAuraCommand,          "",   NULL },
        { "changelevel", 2, &ChatHandler::HandleChangeLevelCommand,   "",   NULL },
        { "commands",    0, &ChatHandler::HandleCommandsCommand,      "",   NULL },
        { "delete",      2, &ChatHandler::HandleDeleteCommand,        "",   NULL },
        { "demorph",     2, &ChatHandler::HandleDeMorphCommand,       "",   NULL },
        { "die",         3, &ChatHandler::HandleDieCommand,           "",   NULL },
        { "revive",      3, &ChatHandler::HandleReviveCommand,        "",   NULL },
        { "dismount",    0, &ChatHandler::HandleDismountCommand,      "",   NULL },
        { "displayid",   2, &ChatHandler::HandleDisplayIdCommand,     "",   NULL },
        { "factionid",   2, &ChatHandler::HandleFactionIdCommand,     "",   NULL },
        { "gmlist",      0, &ChatHandler::HandleGMListCommand,        "",   NULL },
        { "gmoff",       1, &ChatHandler::HandleGMOffCommand,         "",   NULL },
        { "gmon",        1, &ChatHandler::HandleGMOnCommand,          "",   NULL },
        { "gps",         1, &ChatHandler::HandleGPSCommand,           "",   NULL },
        { "guid",        2, &ChatHandler::HandleGUIDCommand,          "",   NULL },
        { "help",        0, &ChatHandler::HandleHelpCommand,          "",   NULL },
        { "info",        0, &ChatHandler::HandleInfoCommand,          "",   NULL },
        { "npcinfo",     3, &ChatHandler::HandleNpcInfoCommand,       "",   NULL },
		{ "npcinfoset",  3, &ChatHandler::HandleNpcInfoSetCommand,    "",   NULL },
        { "item",        2, &ChatHandler::HandleItemCommand,          "",   NULL },
        { "itemrmv",     2, &ChatHandler::HandleItemRemoveCommand,    "",   NULL },
        { "itemmove",    2, &ChatHandler::HandleItemMoveCommand,      "",   NULL },
        { "kick",        1, &ChatHandler::HandleNYICommand,           "",   NULL },
        { "learn",       3, &ChatHandler::HandleLearnCommand,         "",   NULL },
        { "unlearn",     3, &ChatHandler::HandleUnLearnCommand,       "",   NULL },//add by vendy
        { "modify",      1, NULL,                                     "",   modifyCommandTable },
        { "debug",       1, NULL,                                     "",   debugCommandTable },
        { "morph",       3, &ChatHandler::HandleMorphCommand,         "",   NULL },
        { "mount",       0, &ChatHandler::HandleMountCommand,         "",   NULL },
        { "go",          3, &ChatHandler::HandleMoveCommand,          "",   NULL },
        { "name",        2, &ChatHandler::HandleNameCommand,          "",   NULL },
        { "subname",     2, &ChatHandler::HandleSubNameCommand,       "",   NULL },
        { "npcflag",     2, &ChatHandler::HandleNPCFlagCommand,       "",   NULL },
        { "cdist",        1, &ChatHandler::HandleCreatureDistanceCommand,       "",   NULL },
        { "object",      3, &ChatHandler::HandleObjectCommand,        "",   NULL },
        { "gameobject",  3, &ChatHandler::HandleGameObjectCommand,    "",   NULL },
        { "prog",        2, &ChatHandler::HandleProgCommand,          "",   NULL },
        { "random",      2, &ChatHandler::HandleRandomCommand,        "",   NULL },
        { "recall",      1, &ChatHandler::HandleRecallCommand,        "",   NULL },
        { "run",         2, &ChatHandler::HandleRunCommand,           "",   NULL },
        { "save",        0, &ChatHandler::HandleSaveCommand,          "",   NULL },
        { "security",    3, &ChatHandler::HandleSecurityCommand,      "",   NULL },
        { "AddSpawn",    2, &ChatHandler::HandleSpawnCommand,         "",   NULL },
        { "standstate",  3, &ChatHandler::HandleStandStateCommand,    "",   NULL },
        { "start",       0, &ChatHandler::HandleStartCommand,         "",   NULL },
        { "summon",      1, &ChatHandler::HandleSummonCommand,        "",   NULL },
        { "taxicheat",   1, &ChatHandler::HandleTaxiCheatCommand,     "",   NULL },
        { "worldport",   3, &ChatHandler::HandleWorldPortCommand,     "",   NULL },
        { "addweapon",   3, &ChatHandler::HandleAddWeaponCommand,     "",   NULL },
        { "allowmove",   3, &ChatHandler::HandleAllowMovementCommand, "",   NULL },
        { "addgrave",    3, &ChatHandler::HandleAddGraveCommand,      "",   NULL },
        { "addsh",       3, &ChatHandler::HandleAddSHCommand,         "",   NULL },
        { "transport",   3, &ChatHandler::HandleSpawnTransportCommand,"",   NULL },
        { "explorecheat",3, &ChatHandler::HandleExploreCheatCommand,  "",   NULL },
        { "hover",       3, &ChatHandler::HandleHoverCommand,         "",   NULL },
        { "levelup",     3, &ChatHandler::HandleLevelUpCommand,       "",   NULL },
        { "emote",       3, &ChatHandler::HandleEmoteCommand,         "",   NULL },
        { "showarea",    3, &ChatHandler::HandleShowAreaCommand,      "",   NULL },
        { "hidearea",    3, &ChatHandler::HandleHideAreaCommand,      "",   NULL },
        { "addspw",      2, &ChatHandler::HandleAddSpwCommand,        "",   NULL },
        { "additem",     3, &ChatHandler::HandleAddItemCommand,       "",   NULL }, //add by vendy
        { NULL,          0, NULL,                                     "",   NULL }
    };

    if(first_call)
    {
        std::stringstream s;
        for(uint32 i = 0; commandTable[i].Name != NULL; i++)
        {
            s.rdbuf()->str("");
            s << "SELECT security, help FROM commands WHERE name = '" << commandTable[i].Name  << "'";
            QueryResult* result = sDatabase.Query(s.str().c_str());
            if (result)
            {
                commandTable[i].SecurityLevel = (uint16)(*result)[1].GetUInt16();
                commandTable[i].Help = (*result)[1].GetString();
                delete result;
            }
            if(commandTable[i].ChildCommands != NULL)
            {
                ChatCommand *ptable = commandTable[i].ChildCommands;
                for(uint32 j = 0; ptable[j].Name != NULL; j++)
                {
                    s.rdbuf()->str("");
                    s << "SELECT security, help FROM commands WHERE name = '" << commandTable[i].Name << " "
                        << ptable[j].Name << "'";

                    QueryResult* result = sDatabase.Query(s.str().c_str());
                    if (result)
                    {
                        ptable[i].SecurityLevel = (uint16)(*result)[1].GetUInt16();
                        ptable[i].Help = (*result)[1].GetString();
                        delete result;
                    }
                }
            }
        }

        first_call = false;
    }

    return commandTable;
}


bool ChatHandler::hasStringAbbr(const char* s1, const char* s2)
{
    for(;;)
    {
        if( !*s2 )
            return true;
        else if( !*s1 )
            return false;
        else if( tolower( *s1 ) != tolower( *s2 ) )
            return false;
        s1++; s2++;
    }
}


void ChatHandler::SendMultilineMessage(const char *str)
{
    char buf[256];
    WorldPacket data;

    const char* line = str;
    const char* pos = line;
    while((pos = strchr(line, '\n')) != NULL)
    {
        strncpy(buf, line, pos-line);
        buf[pos-line]=0;

        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket(&data);

        line = pos+1;
    }

    FillSystemMessageData(&data, m_session, line);
    m_session->SendPacket(&data);
}


bool ChatHandler::ExecuteCommandInTable(ChatCommand *table, const char* text)
{
    std::string cmd = "";

    // skip command
    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        text++;
    }

    while (*text == ' ') text++;                  // skip whitespace

    if(!cmd.length())
        return false;

    for(uint32 i = 0; table[i].Name != NULL; i++)
    {
        if(!hasStringAbbr(table[i].Name, cmd.c_str()))
            continue;

        if(m_session->GetSecurity() < table[i].SecurityLevel)
            continue;

        if(table[i].ChildCommands != NULL)
        {
            if(!ExecuteCommandInTable(table[i].ChildCommands, text))
            {
                if(table[i].Help != "")
                    SendMultilineMessage(table[i].Help.c_str());
                else
                {
                    WorldPacket data;
                    FillSystemMessageData(&data, m_session, "There is no such subcommand.");
                    m_session->SendPacket(&data);
                }
            }

            return true;
        }

        if(!(this->*(table[i].Handler))(text))
        {
            if(table[i].Help != "")
                SendMultilineMessage(table[i].Help.c_str());
            else
            {
                WorldPacket data;
                FillSystemMessageData(&data, m_session, "Incorrect syntax.");
                m_session->SendPacket(&data);
            }
        }

        return true;
    }

    return false;
}


int ChatHandler::ParseCommands(const char* text, WorldSession *session)
{
    m_session = session;

    ASSERT(text);
    ASSERT(*text);

    if(m_session->GetSecurity() == 0)
        return 0;

    if(text[0] != '!' && text[0] != '.')          // let's not confuse users
        return 0;

    text++;

    if(!ExecuteCommandInTable(getCommandTable(), text))
    {
        WorldPacket data;
        FillSystemMessageData(&data, m_session, "There is no such command.");
        m_session->SendPacket(&data);
    }
    return 1;
}


void ChatHandler::FillMessageData( WorldPacket *data, WorldSession* session, uint32 type, uint32 language, const char *channelName, const char *message ) const
{
    //Packet structure
    //uint8      type;
    //uint32     language;
    //uint64     guid;
    //uint64     guid;
    //uint32     len_of_text;
    //char       text[];         // not sure ? i think is null terminated .. not null terminated
    //uint8      afk_state;

    uint32 messageLength = strlen((char*)message) + 1;
    uint8 afk = 0;

    data->Initialize(SMSG_MESSAGECHAT);
    *data << (uint8)type;
    *data << language;

    if (type == CHAT_MSG_CHANNEL)
    {
        ASSERT(channelName);
        *data << channelName;
    }

    uint64 guid = 0;
    if (type == CHAT_MSG_SAY || type == CHAT_MSG_CHANNEL || type == CHAT_MSG_WHISPER || type == CHAT_MSG_YELL || type == CHAT_MSG_PARTY)
    {
        guid = session ? session->GetPlayer()->GetGUID() : 0;
    }
    else if (type == CHAT_MSG_WHISPER_INFORM)
    {
        // Convert ChannelName back to the to Players GUID
        guid = uint32(channelName);               //session ? session->GetPlayer()->GetGUID() : 0; // FIXME: may be receiver?
    }

    *data << guid;

    // crashfix
    if (type == CHAT_MSG_SAY || type == CHAT_MSG_YELL || type == CHAT_MSG_PARTY)
        *data << guid;

    *data << messageLength;
    *data << message;

    if(type == CHAT_MSG_AFK && session != 0)
        afk = session->GetPlayer()->ToggleAFK();

    *data << afk;
}


void ChatHandler::SpawnCreature(WorldSession *session, const char* name, uint32 displayId, uint32 npcFlags, uint32 factionId, uint32 level)
{
    WorldPacket data;

    // Create the requested monster
    Player *chr = session->GetPlayer();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();

    Creature* pCreature = new Creature();

    pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), name, chr->GetMapId(), x, y, z, o, objmgr.AddCreatureName(pCreature->GetName(), displayId));
    pCreature->SetZoneId(chr->GetZoneId());
    pCreature->SetUInt32Value(OBJECT_FIELD_ENTRY, objmgr.AddCreatureName(pCreature->GetName(), displayId));
    pCreature->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
    pCreature->SetUInt32Value(UNIT_FIELD_DISPLAYID, displayId);
    pCreature->SetUInt32Value(UNIT_NPC_FLAGS , npcFlags);
    pCreature->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE , factionId);
    pCreature->SetUInt32Value(UNIT_FIELD_HEALTH, 28 + 30*level);
    pCreature->SetUInt32Value(UNIT_FIELD_MAXHEALTH, 28 + 30*level);
    pCreature->SetUInt32Value(UNIT_FIELD_LEVEL , level);

    pCreature->SetFloatValue(UNIT_FIELD_COMBATREACH , 1.5f);
    pCreature->SetFloatValue(UNIT_FIELD_MAXDAMAGE ,  5.0f);
    pCreature->SetFloatValue(UNIT_FIELD_MINDAMAGE , 8.0f);
    pCreature->SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, 1900);
    pCreature->SetUInt32Value(UNIT_FIELD_BASEATTACKTIME+1, 2000);
    pCreature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 2.0f);
    Log::getSingleton( ).outError("AddObject at Chat.cpp");
#ifndef ENABLE_GRID_SYSTEM
    objmgr.AddObject(pCreature);
    pCreature->PlaceOnMap();
#else
    MapManager::Instance().GetMap(pCreature->GetMapId())->Add(pCreature);
#endif

    pCreature->SaveToDB();
}


void ChatHandler::smsg_NewWorld(WorldSession *session, uint32 mapid, float x, float y, float z)
{
    WorldPacket data;
    data.Initialize(SMSG_TRANSFER_PENDING);
    data << uint32(0);

    session->SendPacket(&data);
    MapManager::Instance().GetMap(session->GetPlayer()->GetMapId())->Remove(session->GetPlayer(), false);

    // Build a NEW WORLD packet
    data.Initialize(SMSG_NEW_WORLD);
    data << (uint32)mapid << (float)x << (float)y << (float)z << (float)0.0f;
    session->SendPacket( &data );

    // TODO: clear attack list

    session->GetPlayer()->SetMapId(mapid);
    session->GetPlayer()->Relocate(x, y, z, 0); // sets the position without triggers any packet stuff
    MapManager::Instance().GetMap(session->GetPlayer()->GetMapId())->Add(session->GetPlayer()); // he enters the new world
}


void ChatHandler::MovePlayer(WorldSession *session, float x, float y, float z)
{
    WorldPacket data;

    // Output new position to the console
    Log::getSingleton( ).outDetail( "WORLD: Moved player to (%f, %f, %f)", x, y, z );

    ////////////////////////////////////////
    // Set the new position of the character
    Player *chr = session->GetPlayer();

    // Send new position to client via MSG_MOVE_TELEPORT_ACK
    chr->BuildTeleportAckMsg(&data, x, y, z, 0);
    session->SendPacket(&data);

    // Set actual position and update in-range lists
    chr->SetPosition(x, y, z, 0);

    //////////////////////////////////
    // Now send new position of this player to clients using MSG_MOVE_HEARTBEAT
    chr->BuildHeartBeatMsg(&data);
    chr->SendMessageToSet(&data, true);

    char txtBuffer[256];
    sprintf(txtBuffer,"You have been moved to (%f, %f, %f)",x,y,z );
    FillSystemMessageData(&data, session, txtBuffer);
    session->SendPacket( &data );
}


Player * ChatHandler::getSelectedChar(WorldSession *client)
{
    uint64 guid;
    Player *chr;

    guid = client->GetPlayer()->GetSelection();
    if (guid == 0)
        chr = client->GetPlayer();                // autoselect
    else
        chr = objmgr.GetPlayer(guid);
        // chr = objmgr.GetObject<Player>(guid);

    return chr;
}

//UQ1: Generic string formatting for output... CHECKME: May want this somewhere else???
char *fmtstring( char *format, ... ) 
{
    va_list        argptr;
    #define    MAX_FMT_STRING    32000
    static char        temp_buffer[MAX_FMT_STRING];
    static char        string[MAX_FMT_STRING];    // in case va is called by nested functions
    static int        index = 0;
    char    *buf;
    int len;

    va_start (argptr, format);
    vsprintf (temp_buffer, format, argptr);
    va_end (argptr);

    if ((len = strlen(temp_buffer)) >= MAX_FMT_STRING) {
        return "ERROR";
    }

    if (len + index >= MAX_FMT_STRING-1) {
        index = 0;
    }

    buf = &string[index];
    memcpy( buf, temp_buffer, len+1 );

    index += len + 1;

    return buf;
}

