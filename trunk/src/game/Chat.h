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

#ifndef MANGOSSERVER_CHAT_H
#define MANGOSSERVER_CHAT_H

#include "SharedDefines.h"
#include "Policies/Singleton.h"

class ChatHandler;
class WorldSession;
class Creature;
class Player;
class Unit;

struct LanguageDesc
{
    Language lang_id;
    uint32   spell_id;
    uint32   skill_id;
};

extern LanguageDesc lang_description[LANGUAGES_COUNT];

LanguageDesc const* GetLanguageDescByID(uint32 lang);
LanguageDesc const* GetLanguageDescBySpell(uint32 spell_id);
LanguageDesc const* GetLanguageDescBySkill(uint32 skill_id);

enum ChatMsg
{
    CHAT_MSG_SAY                                  = 0x00,
    CHAT_MSG_PARTY                                = 0x01,
    CHAT_MSG_RAID                                 = 0x02,
    CHAT_MSG_GUILD                                = 0x03,
    CHAT_MSG_OFFICER                              = 0x04,
    CHAT_MSG_YELL                                 = 0x05,
    CHAT_MSG_WHISPER                              = 0x06,   // whisper received
    CHAT_MSG_WHISPER_INFORM                       = 0x07,   // whisper sent
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
    CHAT_MSG_IGNORED                              = 0x16,
    CHAT_MSG_SKILL                                = 0x17,
    CHAT_MSG_LOOT                                 = 0x18,
    CHAT_MSG_UNK1                                 = 0x19,   // unk
    CHAT_MSG_MONSTER_WHISPER                      = 0x1A,
    CHAT_MSG_BATTLEGROUND                         = 0x52,
    CHAT_MSG_BATTLEGROUND_HORDE                   = 0x53,
    CHAT_MSG_BATTLEGROUND_ALLIANCE                = 0x54,
    CHAT_MSG_UNK2                                 = 0x55,   // unk, blue
    CHAT_MSG_UNK3                                 = 0x56,   // unk, yellow
    CHAT_MSG_RAID_LEADER                          = 0x57,
    CHAT_MSG_RAID_WARN                            = 0x58,
    CHAT_MSG_UNK4                                 = 0x59,
    CHAT_MSG_UNK5                                 = 0x5A,   // unk
    CHAT_MSG_UNK6                                 = 0x5B,   // unk
    CHAT_MSG_BATTLEGROUND_CHAT                    = 0x5C,
    CHAT_MSG_BATTLEGROUND_LEADER                  = 0x5D,
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

        static void FillMessageData( WorldPacket *data, WorldSession* session, uint8 type, uint32 language, const char* channelName, uint64 target_guid, const char* message, Unit *speaker = NULL);
        static void FillSystemMessageData( WorldPacket *data, WorldSession* session, const char* message )
        {
            FillMessageData( data, session, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, 0, message );
        }

        static void SendSysMessage(         WorldSession* session, const char *str);
        static void SendSysMultilineMessage(WorldSession* session, const char *str);
        static void PSendSysMessage(         WorldSession* session, const char *format, ...);
        static void PSendSysMultilineMessage(WorldSession* session, const char *format, ...);

        int ParseCommands(const char* text, WorldSession *session);

    protected:
        void SpawnCreature(WorldSession *session, const char* pName, uint32 level);

        bool hasStringAbbr(const char* s1, const char* s2);
        void SendSysMessage(const char *str) { SendSysMessage(m_session,str); }
        void SendSysMultilineMessage(const char *str) { SendSysMultilineMessage(m_session,str); }
        void PSendSysMessage(const char *format, ...);
        void PSendSysMultilineMessage(const char *format, ...);

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
        bool HandleNotifyCommand(const char* args);
        bool HandleGMOnCommand(const char* args);
        bool HandleGMOffCommand(const char* args);
        bool HandleVisibleCommand(const char* args);
        bool HandleGPSCommand(const char* args);
        bool HandleTaxiCheatCommand(const char* args);
        bool HandleWhispersCommand(const char* args);
        bool HandleSayCommand(const char* args);
        bool HandleYellCommand(const char* args);
        bool HandleEmoteCommand(const char* args);
        bool HandleWhisperCommand(const char* args);
        bool HandleSendMailCommand(const char* args);
        bool HandleNameTeleCommand(const char* args);

        bool HandleModifyKnownTitlesCommand(const char* args);
        bool HandleModifyHPCommand(const char* args);
        bool HandleModifyManaCommand(const char* args);
        bool HandleModifyRageCommand(const char* args);
        bool HandleModifyEnergyCommand(const char* args);
        bool HandleModifyMoneyCommand(const char* args);
        bool HandleModifyASpeedCommand(const char* args);
        bool HandleModifySpeedCommand(const char* args);
        bool HandleModifyBWalkCommand(const char* args);
        bool HandleModifyFlyCommand(const char* args);
        bool HandleModifySwimCommand(const char* args);
        bool HandleModifyScaleCommand(const char* args);
        bool HandleModifyMountCommand(const char* args);
        bool HandleModifyBitCommand(const char* args);
        bool HandleModifyFactionCommand(const char* args);
        bool HandleModifySpellCommand(const char* args);
        bool HandleModifyTalentCommand (const char* args);
        bool HandleReloadCommand(const char* args);
        bool HandleLoadScriptsCommand(const char* args);
        bool HandleSendQuestPartyMsgCommand(const char* args);
        bool HandleSendQuestInvalidMsgCommand(const char* args);

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
        bool HandleAddVendorItemCommand(const char* args);
        bool HandleDelVendorItemCommand(const char* args);
        bool HandleAddMoveCommand(const char* args);
        bool HandleSetMoveTypeCommand(const char* args);
        bool HandleRunCommand(const char* args);
        bool HandleChangeLevelCommand(const char* args);
        bool HandleSetPoiCommand(const char* args);
        bool HandleSendItemErrorMsg(const char* args);
        bool HandleNPCFlagCommand(const char* args);
        bool HandleDisplayIdCommand(const char* args);
        bool HandleFactionIdCommand(const char* args);
        bool HandleAddSpwCommand(const char* args);
        bool HandleSpawnDistCommand(const char* args);
        bool HandleSpawnTimeCommand(const char* args);
        bool HandleGoCreatureCommand(const char* args);
        bool HandleGoObjectCommand(const char* args);
        bool HandleTargetObjectCommand(const char* args);
        bool HandleDelObjectCommand(const char* args);
        bool HandleTurnObjectCommand(const char* args);
        bool HandleMoveObjectCommand(const char* args);
        bool HandlePInfoCommand(const char* args);

        bool HandleBanCommand(const char* args);
        bool HandleUnBanCommand(const char* args);
        bool HandleBanInfoCommand(const char* args);
        bool HandleBanListCommand(const char* args);
        bool HandleIdleShutDownCommand(const char* args);
        bool HandleShutDownCommand(const char* args);
        bool HandleSecurityCommand(const char* args);
        bool HandleGoXYCommand(const char* args);
        bool HandleWorldPortCommand(const char* args);
        bool HandleAddWeaponCommand(const char* args);
        bool HandleAllowMovementCommand(const char* args);
        bool HandleGoCommand(const char* args);
        bool HandleLearnCommand(const char* args);
        bool HandleCooldownCommand(const char* args);
        bool HandleUnLearnCommand(const char* args);
        bool HandleObjectCommand(const char* args);
        bool HandleGetDistanceCommand(const char* args);
        bool HandleGameObjectCommand(const char* args);
        bool HandleAnimCommand(const char* args);
        bool HandlePlaySoundCommand(const char* args);
        bool HandleStandStateCommand(const char* args);
        bool HandleDieCommand(const char* args);
        bool HandleReviveCommand(const char* args);
        bool HandleMorphCommand(const char* args);
        bool HandleAuraCommand(const char* args);
        bool HandleUnAuraCommand(const char* args);
        bool HandleLinkGraveCommand(const char* args);
        bool HandleNearGraveCommand(const char* args);
        bool HandleSpawnTransportCommand(const char* args);
        bool HandleExploreCheatCommand(const char* args);
        bool HandleEmote2Command(const char* args);
        bool HandleNpcInfoCommand(const char* args);
        bool HandleNpcInfoSetCommand(const char* args);
        bool HandleHoverCommand(const char* args);
        bool HandleLevelUpCommand(const char* args);
        bool HandleShowAreaCommand(const char* args);
        bool HandleHideAreaCommand(const char* args);
        bool HandleAddItemCommand(const char* args);
        bool HandleAddItemSetCommand(const char* args);
        bool HandleLookupItemCommand(const char * args);
        bool HandleLookupItemSetCommand(const char * args);
        bool HandleCreateGuildCommand(const char* args);
        bool HandleShowHonor(const char* args);
        bool HandleUpdate(const char* args);
        bool HandleBankCommand(const char* args);
        bool HandleChangeWeather(const char* args);
        bool HandleKickPlayerCommand(const char * args);
        bool HandleTeleCommand(const char * args);
        bool HandleAddTeleCommand(const char * args);
        bool HandleDelTeleCommand(const char * args);
        bool HandleListAurasCommand (const char * args);
        bool HandleLookupTeleCommand(const char * args);
        bool HandleResetCommand (const char * args);
        bool HandleTicketCommand(const char* args);
        bool HandleDelTicketCommand(const char* args);
        bool HandleMaxSkillCommand(const char* args);
        bool HandleSetSkillCommand(const char* args);
        bool HandleListCreatureCommand(const char* args);
        bool HandleListItemCommand(const char* args);
        bool HandleListObjectCommand(const char* args);
        bool HandleLookupSkillCommand(const char* args);
        bool HandleLookupSpellCommand(const char* args);
        bool HandleLookupQuestCommand(const char* args);
        bool HandleLookupCreatureCommand(const char* args);
        bool HandleLookupObjectCommand(const char* args);
        bool HandlePasswordCommand(const char* args);
        bool HandleLockAccountCommand(const char* args);
        bool HandleRespawnCommand(const char* args);
        bool HandleWpAddCommand(const char* args);
        bool HandleWpModifyCommand(const char* args);
        bool HandleWpShowCommand(const char* args);
        bool HandleFlyModeCommand(const char* args);
        bool HandleSendOpcodeCommand(const char* args);
        bool HandleSellErrorCommand(const char* args);
        bool HandleBuyErrorCommand(const char* args);
        bool HandleUpdateWorldStateCommand(const char* args);
        bool HandlePlaySound2Command(const char* args);
        bool HandleSendChannelNotifyCommand(const char* args);
        bool HandleSendChatMsgCommand(const char* args);
        bool HandleRenameCommand(const char * args);
        bool HandleLoadPDumpCommand(const char *args);
        bool HandleWritePDumpCommand(const char *args);

        //! Development Commands
        bool HandleSetValue(const char* args);
        bool HandleGetValue(const char* args);
        bool HandleSet32Bit(const char* args);
        bool HandleMod32Value(const char* args);
        bool HandleOutOfRange(const char * args);
        bool HandleAddQuest(const char * args);
        bool HandleRemoveQuest(const char * args);
        bool HandleSaveAllCommand(const char* args);
        bool HandleGetItemState(const char * args);

        Player*   getSelectedPlayer();
        Creature* getSelectedCreature();
        Unit*     getSelectedUnit();

        WorldSession *m_session;

        // Utility methods for commands
        void ShowTicket(uint64 guid, uint32 category, char const* text);
        uint32 GetTicketIDByNum(uint32 num);
};

#define sChatHandler MaNGOS::Singleton<ChatHandler>::Instance()
#endif

char const *fmtstring( char const *format, ... );
