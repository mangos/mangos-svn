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

#ifndef MANGOSSERVER_CHAT_H
#define MANGOSSERVER_CHAT_H

#include "SharedDefines.h"
#include "Policies/Singleton.h"

class ChatHandler;
class WorldSession;
class Player;
class Unit;

enum ChatMsg
{
    CHAT_MSG_SAY                                  = 0x00,
    CHAT_MSG_PARTY                                = 0x01,

    CHAT_MSG_GUILD                                = 0x03,
    CHAT_MSG_OFFICER                              = 0x04,
    CHAT_MSG_YELL                                 = 0x05,
    CHAT_MSG_WHISPER                              = 0x06,
    CHAT_MSG_WHISPER_INFORM                       = 0x07,
    CHAT_MSG_EMOTE                                = 0x08,
    CHAT_MSG_TEXT_EMOTE                           = 0x09,
    CHAT_MSG_SYSTEM                               = 0x0A,
    CHAT_MSG_MONSTER_SAY                          = 0x0B,
    CHAT_MSG_MONSTER_YELL                         = 0x0C,
    CHAT_MSG_MONSTER_EMOTE                        = 0x0D,
    CHAT_MSG_CHANNEL                              = 0x0E,
    CHAT_MSG_CHANNEL_JOIN                         = 0x0F,
    CHAT_MSG_CHANNEL_LEAVE                        = 0x10,
    CHAT_MSG_CHANNEL_LIST                         = 0x11,
    CHAT_MSG_CHANNEL_NOTICE                       = 0x12,
    CHAT_MSG_CHANNEL_NOTICE_USER                  = 0x13,
    CHAT_MSG_AFK                                  = 0x14,
    CHAT_MSG_DND                                  = 0x15,
    CHAT_MSG_COMBAT_LOG                           = 0x16,
    CHAT_MSG_IGNORED                              = 0x17,
    CHAT_MSG_SKILL                                = 0x18,
    CHAT_MSG_LOOT                                 = 0x19,
};

class ChatCommand
{
    public:
        const char *       Name;
        uint16             SecurityLevel;
        bool (ChatHandler::*Handler)(const char* args) ;
        std::string        Help;
        ChatCommand *      ChildCommands;
};

class ChatHandler
{
    public:
        ChatHandler();
        ~ChatHandler();

        void FillMessageData( WorldPacket *data, WorldSession* session, uint32 type, uint32 language, const char* channelName, const char* message ) const;
        void FillSystemMessageData( WorldPacket *data, WorldSession* session, const char* message ) const
        {
            FillMessageData( data, session, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, message );
        }

        int ParseCommands(const char* text, WorldSession *session);

    protected:
        void SpawnCreature(WorldSession *session, const char* pName, uint32 displayId, uint32 npcFlags, uint32 factionId, uint32 level);

        bool hasStringAbbr(const char* s1, const char* s2);
        void SendMultilineMessage(const char *str);

        bool ExecuteCommandInTable(ChatCommand *table, const char* text);
        bool ShowHelpForCommand(ChatCommand *table, const char* cmd);

        ChatCommand* getCommandTable();

        bool HandleHelpCommand(const char* args);
        bool HandleCommandsCommand(const char* args);
        bool HandleNYICommand(const char* args);
        bool HandleAcctCommand(const char* args);
        bool HandleStartCommand(const char* args);
        bool HandleInfoCommand(const char* args);
        bool HandleDismountCommand(const char* args);
        bool HandleSaveCommand(const char* args);
        bool HandleGMListCommand(const char* args);

        bool HandleNamegoCommand(const char* args);
        bool HandleGonameCommand(const char* args);
        bool HandleRecallCommand(const char* args);
        bool HandleAnnounceCommand(const char* args);
        bool HandleGMOnCommand(const char* args);
        bool HandleGMOffCommand(const char* args);
        bool HandleGPSCommand(const char* args);
        bool HandleTaxiCheatCommand(const char* args);

        bool HandleModifyHPCommand(const char* args);
        bool HandleModifyManaCommand(const char* args);
        bool HandleModifyRageCommand(const char* args);
        bool HandleModifyEnergyCommand(const char* args);
        bool HandleModifyLevelCommand(const char* args);
        bool HandleModifyGoldCommand(const char* args);
        bool HandleModifyASpedCommand(const char* args);
        bool HandleModifySpeedCommand(const char* args);
        bool HandleModifyBWalkCommand(const char* args);
        bool HandleModifySwimCommand(const char* args);
        bool HandleModifyScaleCommand(const char* args);
        bool HandleModifyMountCommand(const char* args);
        bool HandleModifyBitCommand(const char* args);
        bool HandleModifyFactionCommand(const char* args);
        bool HandleModifySpellCommand(const char* args);
        bool HandleReloadCommand(const char* args);
        bool HandleLoadScriptsCommand(const char* args);

        bool HandleDebugInArcCommand(const char* args);
        bool HandleDebugSpellFailCommand(const char* args);

        bool HandleGUIDCommand(const char* args);
        bool HandleNameCommand(const char* args);
        bool HandleSubNameCommand(const char* args);
        bool HandleProgCommand(const char* args);
        bool HandleItemMoveCommand(const char* args);
        bool HandleSpawnCommand(const char* args);
        bool HandleDeleteCommand(const char* args);
        bool HandleDeMorphCommand(const char* args);
        bool HandleItemCommand(const char* args);
        bool HandleItemRemoveCommand(const char* args);
        bool HandleAddMoveCommand(const char* args);
        bool HandleRandomCommand(const char* args);
        bool HandleRunCommand(const char* args);
        bool HandleChangeLevelCommand(const char* args);
        bool HandleSetPoiCommand(const char* args);
        bool HandleSendItemErrorMsg(const char* args);
        bool HandleNPCFlagCommand(const char* args);
        bool HandleDisplayIdCommand(const char* args);
        bool HandleFactionIdCommand(const char* args);
        bool HandleAddSpwCommand(const char* args);

        bool HandleSecurityCommand(const char* args);
        bool HandleWorldPortCommand(const char* args);
        bool HandleAddWeaponCommand(const char* args);
        bool HandleAllowMovementCommand(const char* args);
        bool HandleAddSpiritCommand(const char* args);
        bool HandleGoCommand(const char* args);
        bool HandleLearnCommand(const char* args);
        bool HandleUnLearnCommand(const char* args);
        bool HandleLearnSkillCommand(const char* args);
        bool HandleUnLearnSkillCommand(const char* args);
        bool HandleObjectCommand(const char* args);
        bool HandleGetDistanceCommand(const char* args);
        bool HandleGameObjectCommand(const char* args);
        bool HandleAnimCommand(const char* args);
        bool HandleStandStateCommand(const char* args);
        bool HandleDieCommand(const char* args);
        bool HandleReviveCommand(const char* args);
        bool HandleMorphCommand(const char* args);
        bool HandleAuraCommand(const char* args);
        bool HandleAddGraveCommand(const char* args);
        bool HandleAddSHCommand(const char* args);
        bool HandleSpawnTransportCommand(const char* args);
        bool HandleExploreCheatCommand(const char* args);
        bool HandleEmoteCommand(const char* args);
        bool HandleNpcInfoCommand(const char* args);
        bool HandleNpcInfoSetCommand(const char* args);
        bool HandleHoverCommand(const char* args);
        bool HandleLevelUpCommand(const char* args);
        bool HandleShowAreaCommand(const char* args);
        bool HandleHideAreaCommand(const char* args);
        bool HandleAddItemCommand(const char* args);
        bool HandleCreateGuildCommand(const char* args);
        bool HandleShowHonor(const char* args);
        bool HandleUpdate(const char* args);
        bool HandleBankCommand(const char* args);
        bool HandleChangeWeather(const char* args);

        //! Development Commands
        bool HandleSetValue(const char* args);
        bool HandleGetValue(const char* args);
        bool HandleSet32Bit(const char* args);
        bool HandleMod32Value(const char* args);

        Player* getSelectedChar(WorldSession *client);

        WorldSession *m_session;
};

#define sChatHandler MaNGOS::Singleton<ChatHandler>::Instance()
#endif

char const *fmtstring( char const *format, ... );
