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
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "Chat.h"
#include "MapManager.h"
#include "Policies/SingletonImp.h"

LanguageDesc lang_description[LANGUAGES_COUNT] =
{
    {                                                       // LANG_GLOBAL/LANG_UNIVERSAL = 0
        LANG_UNIVERSAL,       0, 0
    },
    { LANG_ORCISH,        669, SKILL_LANG_ORCISH       },
    { LANG_DARNASSIAN,    671, SKILL_LANG_DARNASSIAN   },
    { LANG_TAURAHE,       670, SKILL_LANG_TAURAHE      },
    { LANG_DWARVISH,      672, SKILL_LANG_DWARVEN      },
    { LANG_COMMON,        668, SKILL_LANG_COMMON       },
    { LANG_DEMONIC,       815, SKILL_LANG_DEMON_TONGUE },
    { LANG_TITAN,         816, SKILL_LANG_TITAN        },
    { LANG_THELASSIAN,    813, SKILL_LANG_THALASSIAN   },
    { LANG_DRACONIC,      814, SKILL_LANG_DRACONIC     },
    { LANG_KALIMAG,       817, SKILL_LANG_OLD_TONGUE   },
    { LANG_GNOMISH,      7340, SKILL_LANG_GNOMISH      },
    { LANG_TROLL,        7341, SKILL_LANG_TROLL        },
    { LANG_GUTTERSPEAK, 17737, SKILL_LANG_GUTTERSPEAK  }
};

LanguageDesc const* GetLanguageDescByID(uint32 lang)
{
    for(int i = 0; i < LANGUAGES_COUNT; ++i)
    {
        if(uint32(lang_description[i].lang_id) == lang)
            return &lang_description[i];
    }

    return NULL;
}

LanguageDesc const* GetLanguageDescBySpell(uint32 spell_id)
{
    for(int i = 0; i < LANGUAGES_COUNT; ++i)
    {
        if(lang_description[i].spell_id == spell_id)
            return &lang_description[i];
    }

    return NULL;
}

LanguageDesc const* GetLanguageDescBySkill(uint32 skill_id)
{
    for(int i = 0; i < LANGUAGES_COUNT; ++i)
    {
        if(lang_description[i].skill_id == skill_id)
            return &lang_description[i];
    }

    return NULL;
}

INSTANTIATE_SINGLETON_1( ChatHandler );

ChatHandler::ChatHandler()
{

}

ChatHandler::~ChatHandler()
{

}

ChatCommand * ChatHandler::getCommandTable()
{

    static bool first_call = true;

    static ChatCommand modifyCommandTable[] =
    {
        { "hp",          1, &ChatHandler::HandleModifyHPCommand,         "",   NULL },
        { "mana",        1, &ChatHandler::HandleModifyManaCommand,       "",   NULL },
        { "rage",        1, &ChatHandler::HandleModifyRageCommand,       "",   NULL },
        { "energy",      1, &ChatHandler::HandleModifyEnergyCommand,     "",   NULL },
        { "money",       1, &ChatHandler::HandleModifyMoneyCommand,      "",   NULL },
        { "speed",       1, &ChatHandler::HandleModifySpeedCommand,      "",   NULL },
        { "swim",        1, &ChatHandler::HandleModifySwimCommand,       "",   NULL },
        { "scale",       1, &ChatHandler::HandleModifyScaleCommand,      "",   NULL },
        { "bit",         1, &ChatHandler::HandleModifyBitCommand,        "",   NULL },
        { "bwalk",       1, &ChatHandler::HandleModifyBWalkCommand,      "",   NULL },
        { "aspeed",      1, &ChatHandler::HandleModifyASpeedCommand,     "",   NULL },
        { "faction",     1, &ChatHandler::HandleModifyFactionCommand,    "",   NULL },
        { "spell",       1, &ChatHandler::HandleModifySpellCommand,      "",   NULL },
        { "tp",          1, &ChatHandler::HandleModifyTalentCommand,     "",   NULL },

        { NULL,          0, NULL,                                        "",   NULL }
    };

    static ChatCommand debugCommandTable[] =
    {
        { "inarc",       3, &ChatHandler::HandleDebugInArcCommand,         "",   NULL },
        { "spellfail",   3, &ChatHandler::HandleDebugSpellFailCommand,     "",   NULL },
        { "setpoi",      3, &ChatHandler::HandleSetPoiCommand,             "",   NULL },
        { "qpartymsg",   3, &ChatHandler::HandleSendQuestPartyMsgCommand,  "",   NULL },
        { "qinvalidmsg", 3, &ChatHandler::HandleSendQuestInvalidMsgCommand,"",   NULL },
        { "itemmsg",     3, &ChatHandler::HandleSendItemErrorMsg,          "",   NULL },
        { "getitemstate",3, &ChatHandler::HandleGetItemState,              "",   NULL },
        { NULL,          0, NULL,                                          "",   NULL }
    };

    static ChatCommand commandTable[] =
    {
        { "acct",        0, &ChatHandler::HandleAcctCommand,             "",   NULL },
        { "addmove",     2, &ChatHandler::HandleAddMoveCommand,          "",   NULL },
        { "anim",        3, &ChatHandler::HandleAnimCommand,             "",   NULL },
        { "announce",    1, &ChatHandler::HandleAnnounceCommand,         "",   NULL },
        { "go",          3, &ChatHandler::HandleGoCommand,               "",   NULL },
        { "goname",      1, &ChatHandler::HandleGonameCommand,           "",   NULL },
        { "namego",      1, &ChatHandler::HandleNamegoCommand,           "",   NULL },
        { "aura",        3, &ChatHandler::HandleAuraCommand,             "",   NULL },
        { "unaura",      3, &ChatHandler::HandleUnAuraCommand,           "",   NULL },
        { "changelevel", 2, &ChatHandler::HandleChangeLevelCommand,      "",   NULL },
        { "commands",    0, &ChatHandler::HandleCommandsCommand,         "",   NULL },
        { "delete",      2, &ChatHandler::HandleDeleteCommand,           "",   NULL },
        { "demorph",     2, &ChatHandler::HandleDeMorphCommand,          "",   NULL },
        { "die",         3, &ChatHandler::HandleDieCommand,              "",   NULL },
        { "revive",      3, &ChatHandler::HandleReviveCommand,           "",   NULL },
        { "dismount",    0, &ChatHandler::HandleDismountCommand,         "",   NULL },
        { "displayid",   2, &ChatHandler::HandleDisplayIdCommand,        "",   NULL },
        { "factionid",   2, &ChatHandler::HandleFactionIdCommand,        "",   NULL },
        { "gmlist",      0, &ChatHandler::HandleGMListCommand,           "",   NULL },
        { "gmoff",       1, &ChatHandler::HandleGMOffCommand,            "",   NULL },
        { "gmon",        1, &ChatHandler::HandleGMOnCommand,             "",   NULL },
        { "gps",         1, &ChatHandler::HandleGPSCommand,              "",   NULL },
        { "guid",        2, &ChatHandler::HandleGUIDCommand,             "",   NULL },
        { "help",        0, &ChatHandler::HandleHelpCommand,             "",   NULL },
        { "info",        0, &ChatHandler::HandleInfoCommand,             "",   NULL },
        { "npcinfo",     3, &ChatHandler::HandleNpcInfoCommand,          "",   NULL },
        { "npcinfoset",  3, &ChatHandler::HandleNpcInfoSetCommand,       "",   NULL },
        { "item",        2, &ChatHandler::HandleItemCommand,             "",   NULL },
        { "itemrmv",     2, &ChatHandler::HandleItemRemoveCommand,       "",   NULL },
        { "itemmove",    2, &ChatHandler::HandleItemMoveCommand,         "",   NULL },
        { "kick",        2, &ChatHandler::HandleKickPlayerCommand,       "",   NULL },
        { "learn",       3, &ChatHandler::HandleLearnCommand,            "",   NULL },
        { "cooldown",    3, &ChatHandler::HandleCooldownCommand,         "",   NULL },
        { "unlearn",     3, &ChatHandler::HandleUnLearnCommand,          "",   NULL },
        { "learnskill",  3, &ChatHandler::HandleLearnSkillCommand,       "",   NULL },
        { "unlearnskill",3, &ChatHandler::HandleUnLearnSkillCommand,     "",   NULL },
        { "modify",      1, NULL,                                        "",   modifyCommandTable },
        { "debug",       1, NULL,                                        "",   debugCommandTable },
        { "morph",       3, &ChatHandler::HandleMorphCommand,            "",   NULL },
        { "name",        2, &ChatHandler::HandleNameCommand,             "",   NULL },
        { "subname",     2, &ChatHandler::HandleSubNameCommand,          "",   NULL },
        { "npcflag",     2, &ChatHandler::HandleNPCFlagCommand,          "",   NULL },
        { "distance",    3, &ChatHandler::HandleGetDistanceCommand,      "",   NULL },
        { "object",      3, &ChatHandler::HandleObjectCommand,           "",   NULL },
        { "gameobject",  3, &ChatHandler::HandleGameObjectCommand,       "",   NULL },
        { "addgo",       3, &ChatHandler::HandleGameObjectCommand,       "",   NULL },
        { "prog",        2, &ChatHandler::HandleProgCommand,             "",   NULL },
        { "random",      2, &ChatHandler::HandleRandomCommand,           "",   NULL },
        { "recall",      1, &ChatHandler::HandleRecallCommand,           "",   NULL },
        { "run",         2, &ChatHandler::HandleRunCommand,              "",   NULL },
        { "save",        0, &ChatHandler::HandleSaveCommand,             "",   NULL },
        { "saveall",     1, &ChatHandler::HandleSaveAllCommand,          "",   NULL },
        { "security",    3, &ChatHandler::HandleSecurityCommand,         "",   NULL },
        { "banip",       3, &ChatHandler::HandleBanIPCommand,            "",   NULL },
        { "banaccount",  3, &ChatHandler::HandleBanAccountCommand,       "",   NULL },
        { "unbanip",     3, &ChatHandler::HandleUnBanIPCommand,          "",   NULL },
        { "unbanaccount",3, &ChatHandler::HandleUnBanAccountCommand,     "",   NULL },
        { "AddSpawn",    2, &ChatHandler::HandleSpawnCommand,            "",   NULL },
        { "standstate",  3, &ChatHandler::HandleStandStateCommand,       "",   NULL },
        { "start",       0, &ChatHandler::HandleStartCommand,            "",   NULL },
        { "taxicheat",   1, &ChatHandler::HandleTaxiCheatCommand,        "",   NULL },
        { "goxy",        3, &ChatHandler::HandleGoXYCommand     ,        "",   NULL },
        { "worldport",   3, &ChatHandler::HandleWorldPortCommand,        "",   NULL },
        { "addweapon",   3, &ChatHandler::HandleAddWeaponCommand,        "",   NULL },
        { "allowmove",   3, &ChatHandler::HandleAllowMovementCommand,    "",   NULL },
        { "linkgrave",   3, &ChatHandler::HandleLinkGraveCommand,        "",   NULL },
        { "neargrave",   3, &ChatHandler::HandleNearGraveCommand,        "",   NULL },
        { "transport",   3, &ChatHandler::HandleSpawnTransportCommand,   "",   NULL },
        { "explorecheat",3, &ChatHandler::HandleExploreCheatCommand,     "",   NULL },
        { "hover",       3, &ChatHandler::HandleHoverCommand,            "",   NULL },
        { "levelup",     3, &ChatHandler::HandleLevelUpCommand,          "",   NULL },
        { "emote",       3, &ChatHandler::HandleEmoteCommand,            "",   NULL },
        { "showarea",    3, &ChatHandler::HandleShowAreaCommand,         "",   NULL },
        { "hidearea",    3, &ChatHandler::HandleHideAreaCommand,         "",   NULL },
        { "addspw",      2, &ChatHandler::HandleAddSpwCommand,           "",   NULL },
        { "spawndist",   2, &ChatHandler::HandleSpawnDistCommand,        "",   NULL },
        { "spawntime",   2, &ChatHandler::HandleSpawnTimeCommand,        "",   NULL },
        { "additem",     3, &ChatHandler::HandleAddItemCommand,          "",   NULL },
        { "additemset",  3, &ChatHandler::HandleAddItemSetCommand,       "",   NULL },
        { "createguild", 3, &ChatHandler::HandleCreateGuildCommand,      "",   NULL },
        { "showhonor",   0, &ChatHandler::HandleShowHonor,               "",   NULL },
        { "update",      3, &ChatHandler::HandleUpdate,                  "",   NULL },
        { "bank",        3, &ChatHandler::HandleBankCommand,             "",   NULL },
        { "wchange",     3, &ChatHandler::HandleChangeWeather,           "",   NULL },
        { "reload",      3, &ChatHandler::HandleReloadCommand,           "",   NULL },
        { "loadscripts", 3, &ChatHandler::HandleLoadScriptsCommand,      "",   NULL },
        { "tele",        1, &ChatHandler::HandleTeleCommand,             "",   NULL },
        { "lookuptele",  1, &ChatHandler::HandleLookupTeleCommand,       "",   NULL },
        { "addtele",     3, &ChatHandler::HandleAddTeleCommand,          "",   NULL },
        { "deltele",     3, &ChatHandler::HandleDelTeleCommand,          "",   NULL },
        { "listauras",   3, &ChatHandler::HandleListAurasCommand,        "",   NULL },
        { "reset",       3, &ChatHandler::HandleResetCommand,            "",   NULL },
        { "fixunlearn",  3, &ChatHandler::HandleFixUnlearnCommand,       "",   NULL },
        { "ticket",      2, &ChatHandler::HandleTicketCommand,           "",   NULL },
        { "delticket",   2, &ChatHandler::HandleDelTicketCommand,        "",   NULL },
        { "maxskill",    3, &ChatHandler::HandleMaxSkillCommand,         "",   NULL },
        { "setskill",    3, &ChatHandler::HandleSetSkillCommand,         "",   NULL },
        { "whispers",    1, &ChatHandler::HandleWhispersCommand,         "",   NULL },
        { "gocreature",  2, &ChatHandler::HandleGoCreatureCommand,       "",   NULL },
        { "goobject",    2, &ChatHandler::HandleGoObjectCommand,         "",   NULL },
        { "targetobject",2, &ChatHandler::HandleTargetObjectCommand,     "",   NULL },
        { "delobject",   2, &ChatHandler::HandleDelObjectCommand,        "",   NULL },
        { "turnobject",  2, &ChatHandler::HandleTurnObjectCommand,       "",   NULL },
        { "moveobject",  2, &ChatHandler::HandleMoveObjectCommand,       "",   NULL },
        { "idleshutdown",3, &ChatHandler::HandleIdleShutDownCommand,     "",   NULL },
        { "shutdown",    3, &ChatHandler::HandleShutDownCommand,         "",   NULL },
        { "pinfo",       2, &ChatHandler::HandlePInfoCommand,            "",   NULL },
        { "visible",     1, &ChatHandler::HandleVisibleCommand,          "",   NULL },
        { "playsound",   1, &ChatHandler::HandlePlaySoundCommand,        "",   NULL },
        { "lookupitem",  3, &ChatHandler::HandleLookupItemCommand,       "",   NULL },
        { "lookupskill", 3, &ChatHandler::HandleLookupSkillCommand,      "",   NULL },
        { "lookupspell", 3, &ChatHandler::HandleLookupSpellCommand,      "",   NULL },
        { "lookupquest", 3, &ChatHandler::HandleLookupQuestCommand,      "",   NULL },
        { "lookupcreature",3, &ChatHandler::HandleLookupCreatureCommand, "",   NULL },
        { "money",       1, &ChatHandler::HandleModifyMoneyCommand,      "",   NULL },
        { "speed",       1, &ChatHandler::HandleModifySpeedCommand,      "",   NULL },
        { "addquest",    3, &ChatHandler::HandleAddQuest,                "",   NULL },
        { "password",    0, &ChatHandler::HandlePasswordCommand,         "",   NULL },
        { "lockaccount", 0, &ChatHandler::HandleLockAccountCommand,      "",   NULL },
        { "respawn",     3, &ChatHandler::HandleRespawnCommand,          "",   NULL },
        //! Development Commands
        { "setvalue",    3, &ChatHandler::HandleSetValue,                "",   NULL },
        { "getvalue",    3, &ChatHandler::HandleGetValue,                "",   NULL },
        { "Mod32Value",  3, &ChatHandler::HandleMod32Value,              "",   NULL },
        { "NewMail",     3, &ChatHandler::HandleSendMailNotice,          "",   NULL },
        { "QNM",         3, &ChatHandler::HandleQueryNextMailTime,       "",   NULL },
        { "oor",         3, &ChatHandler::HandleOutOfRange,              "",   NULL },

        { NULL,          0, NULL,                                        "",   NULL }
    };

    if(first_call)
    {
        QueryResult *result = sDatabase.Query("SELECT `name`,`security`,`help` FROM `command`");
        if (result)
        {
            do
            {
                Field *fields = result->Fetch();
                std::string name = fields[0].GetCppString();
                for(uint32 i = 0; commandTable[i].Name != NULL; i++)
                {
                    if (name == commandTable[i].Name)
                    {
                        commandTable[i].SecurityLevel = (uint16)fields[1].GetUInt16();
                        commandTable[i].Help = fields[2].GetCppString();
                    }
                    if(commandTable[i].ChildCommands != NULL)
                    {
                        ChatCommand *ptable = commandTable[i].ChildCommands;
                        for(uint32 j = 0; ptable[j].Name != NULL; j++)
                        {
                            if (name == fmtstring("%s %s", commandTable[i].Name, ptable[j].Name))
                            {
                                ptable[j].SecurityLevel = (uint16)fields[1].GetUInt16();
                                ptable[j].Help = fields[2].GetCppString();
                            }
                        }
                    }
                }
            } while(result->NextRow());
            delete result;
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

void ChatHandler::SendSysMultilineMessage(WorldSession* session, const char *str)
{
    char buf[256];
    WorldPacket data;

    const char* line = str;
    const char* pos = line;
    while((pos = strchr(line, '\n')) != NULL)
    {
        strncpy(buf, line, pos-line);
        buf[pos-line]=0;

        FillSystemMessageData(&data, session, buf);
        session->SendPacket(&data);

        line = pos+1;
    }

    FillSystemMessageData(&data, session, line);
    session->SendPacket(&data);
}

void ChatHandler::SendSysMessage(WorldSession* session, const char *str)
{
    WorldPacket data;
    FillSystemMessageData(&data, session, str);
    session->SendPacket(&data);
}

void ChatHandler::PSendSysMultilineMessage(WorldSession* session, const char *format, ...)
{
    va_list ap;
    char str [1024];
    va_start(ap, format);
    vsnprintf(str,1024,format, ap );
    va_end(ap);
    SendSysMultilineMessage(session,str);
}

void ChatHandler::PSendSysMessage(WorldSession* session, const char *format, ...)
{
    va_list ap;
    char str [1024];
    va_start(ap, format);
    vsnprintf(str,1024,format, ap );
    va_end(ap);
    SendSysMessage(session,str);
}

void ChatHandler::PSendSysMultilineMessage(const char *format, ...)
{
    va_list ap;
    char str [1024];
    va_start(ap, format);
    vsnprintf(str,1024,format, ap );
    va_end(ap);
    SendSysMultilineMessage(m_session,str);
}

void ChatHandler::PSendSysMessage(const char *format, ...)
{
    va_list ap;
    char str [1024];
    va_start(ap, format);
    vsnprintf(str,1024,format, ap );
    va_end(ap);
    SendSysMessage(m_session,str);
}

bool ChatHandler::ExecuteCommandInTable(ChatCommand *table, const char* text)
{
    std::string fullcmd = text;                             // original `text` can't be used. It content destroyed in command code processing.
    std::string cmd = "";

    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        text++;
    }

    while (*text == ' ') text++;

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
                    SendSysMultilineMessage(table[i].Help.c_str());
                else
                    SendSysMessage("There is no such subcommand.");
            }

            return true;
        }

        if((this->*(table[i].Handler))(text))
        {
            if(table[i].SecurityLevel > 0)
            {
                Player* p = m_session->GetPlayer();
                sLog.outCommand("Command: %s [Player: %s X: %f Y: %f Z: %f Map: %u Selected: %s %u]",
                    fullcmd.c_str(),p->GetName(),p->GetPositionX(),p->GetPositionY(),p->GetPositionZ(),p->GetMapId(),
                    (GUID_HIPART(p->GetSelection())==HIGHGUID_UNIT ? "creature" : (GUID_HIPART(p->GetSelection())==HIGHGUID_PLAYER ? "player" : "none")),GUID_LOPART(p->GetSelection()));
            }
        }
        else
        {
            if(table[i].Help != "")
                SendSysMultilineMessage(table[i].Help.c_str());
            else
                SendSysMessage("Incorrect syntax.");
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

    //if(m_session->GetSecurity() == 0)
    //    return 0;

    if(text[0] != '!' && text[0] != '.')
        return 0;

    // ignore single . and ! in line
    if(strlen(text) < 2)
        return 0;

    // ignore messages staring from many dots.
    if(text[0] == '.' && text[1] == '.' || text[0] == '!' && text[1] == '!')
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

//Note: target_guid used only in CHAT_MSG_WHISPER_INFORM mode (in this case channelName ignored)
void ChatHandler::FillMessageData( WorldPacket *data, WorldSession* session, uint8 type, uint32 language, const char *channelName, uint64 target_guid, const char *message, Unit *speaker)
{
    uint32 messageLength = (message ? strlen(message) : 0) + 1;

    data->Initialize(SMSG_MESSAGECHAT, 100);                // guess size
    *data << (uint8)type;
    *data << language;

    if (type == CHAT_MSG_CHANNEL)
    {
        ASSERT(channelName);
        *data << channelName;
    }

    // in CHAT_MSG_WHISPER_INFORM mode used original target_guid
    if (type == CHAT_MSG_SAY  || type == CHAT_MSG_CHANNEL || type == CHAT_MSG_WHISPER ||
        type == CHAT_MSG_YELL || type == CHAT_MSG_PARTY  || type == CHAT_MSG_RAID  || type == CHAT_MSG_RAID_LEADER || type == CHAT_MSG_RAID_WARN ||
        type == CHAT_MSG_GUILD || type == CHAT_MSG_OFFICER)
    {
        target_guid = session ? session->GetPlayer()->GetGUID() : 0;
    }
    else if (type == CHAT_MSG_MONSTER_SAY)
    {
        *data << (uint64)(((Creature *)speaker)->GetGUID());
        *data << (uint32)(strlen(((Creature *)speaker)->GetCreatureInfo()->Name) + 1);
        *data << ((Creature *)speaker)->GetCreatureInfo()->Name;
    }
    else if (type != CHAT_MSG_WHISPER_INFORM && type != CHAT_MSG_IGNORED && type != CHAT_MSG_DND && type != CHAT_MSG_AFK)
    {
        target_guid = 0;                                    // only for CHAT_MSG_WHISPER_INFORM used original value target_guid
    }

    *data << target_guid;

    if (type == CHAT_MSG_SAY || type == CHAT_MSG_YELL || type == CHAT_MSG_PARTY)
        *data << target_guid;

    *data << messageLength;
    *data << message;
    if(session != 0 && type != CHAT_MSG_WHISPER_INFORM && type != CHAT_MSG_DND && type != CHAT_MSG_AFK)
        *data << session->GetPlayer()->chatTag();
    else
        *data << uint8(0);
}

void ChatHandler::SpawnCreature(WorldSession *session, const char* name, uint32 level)
{
    /*
    //SpawnCreature is invallid, remains for educatial reasons
    Temp. disabled (c) Phantomas
        WorldPacket data;

        Player *chr = session->GetPlayer();
        float x = chr->GetPositionX();
        float y = chr->GetPositionY();
        float z = chr->GetPositionZ();
        float o = chr->GetOrientation();

        Creature* pCreature = new Creature();

        if(!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), name, chr->GetMapId(), x, y, z, o, objmgr.AddCreatureTemplate(pCreature->GetName(), displayId)))
        {
            delete pCreature;
            return false;
        }
        pCreature->SetZoneId(chr->GetZoneId());
        pCreature->SetUInt32Value(OBJECT_FIELD_ENTRY, objmgr.AddCreatureTemplate(pCreature->GetName(), displayId));
        pCreature->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
        pCreature->SetUInt32Value(UNIT_FIELD_DISPLAYID, displayId);
        pCreature->SetHealth(28 + 30*level);
        pCreature->SetMaxHealth(28 + 30*level);
        pCreature->SetLevel(level);

        pCreature->SetFloatValue(UNIT_FIELD_COMBATREACH , 1.5f);
        pCreature->SetFloatValue(UNIT_FIELD_MAXDAMAGE ,  5.0f);
        pCreature->SetFloatValue(UNIT_FIELD_MINDAMAGE , 8.0f);
        pCreature->SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, 1900);
        pCreature->SetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME, 2000);
        pCreature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 2.0f);
        pCreature->AIM_Initialize();
        sLog.outError("AddObject at Chat.cpp");

        MapManager::Instance().GetMap(pCreature->GetMapId())->Add(pCreature);
        pCreature->SaveToDB();
    */
}

Player * ChatHandler::getSelectedPlayer()
{
    uint64 guid  = m_session->GetPlayer()->GetSelection();

    if (guid == 0)
        return m_session->GetPlayer();

    return objmgr.GetPlayer(guid);
}

Unit* ChatHandler::getSelectedUnit()
{
    uint64 guid = m_session->GetPlayer()->GetSelection();

    if (guid == 0)
        return m_session->GetPlayer();

    return ObjectAccessor::Instance().GetUnit(*m_session->GetPlayer(),guid);
}

Creature* ChatHandler::getSelectedCreature()
{
    return ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),m_session->GetPlayer()->GetSelection());
}

char const *fmtstring( char const *format, ... )
{
    va_list        argptr;
    #define    MAX_FMT_STRING    32000
    static char        temp_buffer[MAX_FMT_STRING];
    static char        string[MAX_FMT_STRING];
    static int        index = 0;
    char    *buf;
    int len;

    va_start(argptr, format);
    vsnprintf(temp_buffer,MAX_FMT_STRING, format, argptr);
    va_end(argptr);

    len = strlen(temp_buffer);

    if( len >= MAX_FMT_STRING )
        return "ERROR";

    if (len + index >= MAX_FMT_STRING-1)
    {
        index = 0;
    }

    buf = &string[index];
    memcpy( buf, temp_buffer, len+1 );

    index += len + 1;

    return buf;
}
