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
#include "Language.h"
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

bool ChatHandler::load_command_table = true;

LanguageDesc lang_description[LANGUAGES_COUNT] =
{
    { LANG_ADDON,           0, 0                       },
    { LANG_GLOBAL,          0, 0                       },
    { LANG_UNIVERSAL,       0, 0                       },
    { LANG_ORCISH,        669, SKILL_LANG_ORCISH       },
    { LANG_DARNASSIAN,    671, SKILL_LANG_DARNASSIAN   },
    { LANG_TAURAHE,       670, SKILL_LANG_TAURAHE      },
    { LANG_DWARVISH,      672, SKILL_LANG_DWARVEN      },
    { LANG_COMMON,        668, SKILL_LANG_COMMON       },
    { LANG_DEMONIC,       815, SKILL_LANG_DEMON_TONGUE },
    { LANG_TITAN,         816, SKILL_LANG_TITAN        },
    { LANG_THALASSIAN,    813, SKILL_LANG_THALASSIAN   },
    { LANG_DRACONIC,      814, SKILL_LANG_DRACONIC     },
    { LANG_KALIMAG,       817, SKILL_LANG_OLD_TONGUE   },
    { LANG_GNOMISH,      7340, SKILL_LANG_GNOMISH      },
    { LANG_TROLL,        7341, SKILL_LANG_TROLL        },
    { LANG_GUTTERSPEAK, 17737, SKILL_LANG_GUTTERSPEAK  },
    { LANG_DRAENEI,     29932, SKILL_LANG_DRAENEI      }
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
    static ChatCommand modifyCommandTable[] =
    {
        { "hp",          SEC_MODERATOR,     &ChatHandler::HandleModifyHPCommand,         "",   NULL },
        { "mana",        SEC_MODERATOR,     &ChatHandler::HandleModifyManaCommand,       "",   NULL },
        { "rage",        SEC_MODERATOR,     &ChatHandler::HandleModifyRageCommand,       "",   NULL },
        { "energy",      SEC_MODERATOR,     &ChatHandler::HandleModifyEnergyCommand,     "",   NULL },
        { "money",       SEC_MODERATOR,     &ChatHandler::HandleModifyMoneyCommand,      "",   NULL },
        { "speed",       SEC_MODERATOR,     &ChatHandler::HandleModifySpeedCommand,      "",   NULL },
        { "swim",        SEC_MODERATOR,     &ChatHandler::HandleModifySwimCommand,       "",   NULL },
        { "scale",       SEC_MODERATOR,     &ChatHandler::HandleModifyScaleCommand,      "",   NULL },
        { "bit",         SEC_MODERATOR,     &ChatHandler::HandleModifyBitCommand,        "",   NULL },
        { "bwalk",       SEC_MODERATOR,     &ChatHandler::HandleModifyBWalkCommand,      "",   NULL },
        { "fly",         SEC_MODERATOR,     &ChatHandler::HandleModifyFlyCommand,        "",   NULL },
        { "aspeed",      SEC_MODERATOR,     &ChatHandler::HandleModifyASpeedCommand,     "",   NULL },
        { "faction",     SEC_MODERATOR,     &ChatHandler::HandleModifyFactionCommand,    "",   NULL },
        { "spell",       SEC_MODERATOR,     &ChatHandler::HandleModifySpellCommand,      "",   NULL },
        { "tp",          SEC_MODERATOR,     &ChatHandler::HandleModifyTalentCommand,     "",   NULL },
        { "titles",      SEC_MODERATOR,     &ChatHandler::HandleModifyKnownTitlesCommand, "",  NULL },
        { "mount",       SEC_MODERATOR,     &ChatHandler::HandleModifyMountCommand,       "",  NULL },
        { "honor",       SEC_MODERATOR,     &ChatHandler::HandleModifyHonorCommand,       "",  NULL },

        { NULL,          0, NULL,                                        "",   NULL }
    };

    static ChatCommand wpCommandTable[] =
    {
        { "show",        SEC_GAMEMASTER,   &ChatHandler::HandleWpShowCommand,           "",   NULL },
        { "add",         SEC_GAMEMASTER,   &ChatHandler::HandleWpAddCommand,            "",   NULL },
        { "modify",      SEC_GAMEMASTER,   &ChatHandler::HandleWpModifyCommand,         "",   NULL },

        { NULL,          0, NULL,                                       "",   NULL }
    };

    static ChatCommand debugCommandTable[] =
    {
        { "inarc",       SEC_ADMINISTRATOR, &ChatHandler::HandleDebugInArcCommand,         "",   NULL },
        { "spellfail",   SEC_ADMINISTRATOR, &ChatHandler::HandleDebugSpellFailCommand,     "",   NULL },
        { "setpoi",      SEC_ADMINISTRATOR, &ChatHandler::HandleSetPoiCommand,             "",   NULL },
        { "qpartymsg",   SEC_ADMINISTRATOR, &ChatHandler::HandleSendQuestPartyMsgCommand,  "",   NULL },
        { "qinvalidmsg", SEC_ADMINISTRATOR, &ChatHandler::HandleSendQuestInvalidMsgCommand,"",   NULL },
        { "itemmsg",     SEC_ADMINISTRATOR, &ChatHandler::HandleSendItemErrorMsg,          "",   NULL },
        { "getitemstate",SEC_ADMINISTRATOR, &ChatHandler::HandleGetItemState,              "",   NULL },
        { NULL,          0, NULL,                                                          "",   NULL }
    };

    static ChatCommand learnCommandTable[] =
    {
        { "all",         SEC_ADMINISTRATOR, &ChatHandler::HandleLearnAllCommand,         "",   NULL },
        { "all_gm",      SEC_GAMEMASTER,    &ChatHandler::HandleLearnAllGMCommand,       "",   NULL },
        { "all_crafts",  SEC_GAMEMASTER,    &ChatHandler::HandleLearnAllCraftsCommand,   "",   NULL },
        { "all_lang",    SEC_MODERATOR,     &ChatHandler::HandleLearnAllLangCommand,     "",   NULL },
        { "all_myclass", SEC_ADMINISTRATOR, &ChatHandler::HandleLearnAllMyClassCommand,  "",   NULL },
        { "all_myspells",SEC_ADMINISTRATOR, &ChatHandler::HandleLearnAllMySpellsCommand, "",   NULL },
        { "all_mytalents",SEC_ADMINISTRATOR,&ChatHandler::HandleLearnAllMyTalentsCommand,"",   NULL },
        { "",            SEC_ADMINISTRATOR, &ChatHandler::HandleLearnCommand,            "",   NULL },
        { NULL,          0, NULL,                                                        "",   NULL }
    };

    static ChatCommand reloadCommandTable[] =
    {
        { "all",                SEC_ADMINISTRATOR, &ChatHandler::HandleReloadAllCommand,                 "", NULL },
        { "all_quest",          SEC_ADMINISTRATOR, &ChatHandler::HandleReloadAllQuestCommand,            "", NULL },
        { "all_loot",           SEC_ADMINISTRATOR, &ChatHandler::HandleReloadAllLootCommand,             "", NULL },
        { "all_scripts",        SEC_ADMINISTRATOR, &ChatHandler::HandleReloadAllSpellCommand,            "", NULL },
        { "all_spell",          SEC_ADMINISTRATOR, &ChatHandler::HandleReloadAllSpellCommand,            "", NULL },

        { "areatrigger_tavern",          SEC_ADMINISTRATOR, &ChatHandler::HandleReloadAreaTriggerTavernCommand,       "", NULL },
        { "areatrigger_teleport",        SEC_ADMINISTRATOR, &ChatHandler::HandleReloadAreaTriggerTeleportCommand,     "", NULL },
        { "areatrigger_involvedrelation",SEC_ADMINISTRATOR, &ChatHandler::HandleReloadQuestAreaTriggersCommand,       "", NULL },
        { "button_scripts",              SEC_ADMINISTRATOR, &ChatHandler::HandleReloadButtonScriptsCommand,           "", NULL },
        { "command",                     SEC_ADMINISTRATOR, &ChatHandler::HandleReloadCommandCommand,                 "", NULL },
        { "creature_involvedrelation",   SEC_ADMINISTRATOR, &ChatHandler::HandleReloadCreatureQuestInvRelationsCommand,"",NULL },
        { "creature_loot_template",      SEC_ADMINISTRATOR, &ChatHandler::HandleReloadLootTemplatesCreatureCommand,   "", NULL },
        { "creature_questrelation",      SEC_ADMINISTRATOR, &ChatHandler::HandleReloadCreatureQuestRelationsCommand,  "", NULL },
        { "disenchant_loot_template",    SEC_ADMINISTRATOR, &ChatHandler::HandleReloadLootTemplatesDisenchantCommand, "", NULL },
        { "fishing_loot_template",       SEC_ADMINISTRATOR, &ChatHandler::HandleReloadLootTemplatesFishingCommand,    "", NULL },
        { "game_graveyard_zone",         SEC_ADMINISTRATOR, &ChatHandler::HandleReloadGameGraveyardZoneCommand,       "", NULL },
        { "gameobject_involvedrelation", SEC_ADMINISTRATOR, &ChatHandler::HandleReloadGOQuestInvRelationsCommand,     "", NULL },
        { "gameobject_questrelation",    SEC_ADMINISTRATOR, &ChatHandler::HandleReloadGOQuestRelationsCommand,        "", NULL },
        { "gameobject_loot_template",    SEC_ADMINISTRATOR, &ChatHandler::HandleReloadLootTemplatesGameobjectCommand, "", NULL },
        { "item_loot_template",          SEC_ADMINISTRATOR, &ChatHandler::HandleReloadLootTemplatesItemCommand,       "", NULL },
        { "pickpocketing_loot_template", SEC_ADMINISTRATOR, &ChatHandler::HandleReloadLootTemplatesPickpocketingCommand,"",NULL},
        { "prospecting_loot_template",   SEC_ADMINISTRATOR, &ChatHandler::HandleReloadLootTemplatesProspectingCommand,"", NULL },
        { "skinning_loot_template",      SEC_ADMINISTRATOR, &ChatHandler::HandleReloadLootTemplatesSkinningCommand,   "", NULL },
        { "quest_end_scripts",           SEC_ADMINISTRATOR, &ChatHandler::HandleReloadQuestEndScriptsCommand,         "", NULL },
        { "quest_start_scripts",         SEC_ADMINISTRATOR, &ChatHandler::HandleReloadQuestStartScriptsCommand,       "", NULL },
        { "quest_template",              SEC_ADMINISTRATOR, &ChatHandler::HandleReloadQuestTemplateCommand,           "", NULL },
        { "reserved_name",               SEC_ADMINISTRATOR, &ChatHandler::HandleReloadReservedNameCommand,            "", NULL },
        { "spell_affect",                SEC_ADMINISTRATOR, &ChatHandler::HandleReloadSpellAffectCommand,             "", NULL },
        { "spell_chain",                 SEC_ADMINISTRATOR, &ChatHandler::HandleReloadSpellChainCommand,              "", NULL },
        { "spell_learn_skill",           SEC_ADMINISTRATOR, &ChatHandler::HandleReloadSpellLearnSkillCommand,         "", NULL },
        { "spell_learn_spell",           SEC_ADMINISTRATOR, &ChatHandler::HandleReloadSpellLearnSpellCommand,         "", NULL },
        { "spell_proc_event",            SEC_ADMINISTRATOR, &ChatHandler::HandleReloadSpellProcEventCommand,          "", NULL },
        { "spell_script_target",         SEC_ADMINISTRATOR, &ChatHandler::HandleReloadSpellScriptTargetCommand,       "", NULL },
        { "spell_scripts",               SEC_ADMINISTRATOR, &ChatHandler::HandleReloadSpellScriptsCommand,            "", NULL },
        { "spell_teleport",              SEC_ADMINISTRATOR, &ChatHandler::HandleReloadSpellTeleportCommand,           "", NULL },

        { "",                            SEC_ADMINISTRATOR, &ChatHandler::HandleReloadCommand,                        "", NULL },
        { NULL,                          0,                 NULL,                                                     "", NULL }
    };

    static ChatCommand honorCommandTable[] =
    {
        { "add",            SEC_GAMEMASTER,     &ChatHandler::HandleAddHonorCommand,            "", NULL },
        { "addkill",        SEC_GAMEMASTER,     &ChatHandler::HandleHonorAddKillCommand,        "", NULL },
        { "flushkills",     SEC_GAMEMASTER,     &ChatHandler::HandleHonorFlushKillsCommand,     "", NULL },
        { "update",         SEC_GAMEMASTER,     &ChatHandler::HandleUpdateHonorFieldsCommand,   "", NULL },

        { NULL,             0,                  NULL,                                           "",   NULL }
    };

    static ChatCommand resetCommandTable[] =
    {
        { "honor",          SEC_ADMINISTRATOR,  &ChatHandler::HandleResetHonorCommand,          "", NULL },
        { "level",          SEC_ADMINISTRATOR,  &ChatHandler::HandleResetLevelCommand,          "", NULL },
        { "spells",         SEC_ADMINISTRATOR,  &ChatHandler::HandleResetSpellsCommand,         "", NULL },
        { "stats",          SEC_ADMINISTRATOR,  &ChatHandler::HandleResetStatsCommand,          "", NULL },
        { "talents",        SEC_ADMINISTRATOR,  &ChatHandler::HandleResetTalentsCommand,        "", NULL },

        { NULL,             0,                  NULL,                                           "",   NULL }
    };

    static ChatCommand commandTable[] =
    {
        { "acct",        SEC_PLAYER,        &ChatHandler::HandleAcctCommand,             "",   NULL },
        { "addmove",     SEC_GAMEMASTER,    &ChatHandler::HandleAddMoveCommand,          "",   NULL },
        { "setmovetype", SEC_GAMEMASTER,    &ChatHandler::HandleSetMoveTypeCommand,      "",   NULL },
        { "anim",        SEC_GAMEMASTER,    &ChatHandler::HandleAnimCommand,             "",   NULL },
        { "announce",    SEC_MODERATOR,     &ChatHandler::HandleAnnounceCommand,         "",   NULL },
        { "notify",      SEC_MODERATOR,     &ChatHandler::HandleNotifyCommand,           "",   NULL },
        { "go",          SEC_MODERATOR,     &ChatHandler::HandleGoXYZCommand,            "",   NULL },
        { "goxy",        SEC_MODERATOR,     &ChatHandler::HandleGoXYCommand,             "",   NULL },
        { "goxyz",       SEC_MODERATOR,     &ChatHandler::HandleGoXYZCommand,            "",   NULL },
        { "goname",      SEC_MODERATOR,     &ChatHandler::HandleGonameCommand,           "",   NULL },
        { "namego",      SEC_MODERATOR,     &ChatHandler::HandleNamegoCommand,           "",   NULL },
        { "groupgo",     SEC_MODERATOR,     &ChatHandler::HandleGroupgoCommand,          "",   NULL },
        { "aura",        SEC_ADMINISTRATOR, &ChatHandler::HandleAuraCommand,             "",   NULL },
        { "unaura",      SEC_ADMINISTRATOR, &ChatHandler::HandleUnAuraCommand,           "",   NULL },
        { "changelevel", SEC_GAMEMASTER,    &ChatHandler::HandleChangeLevelCommand,      "",   NULL },
        { "commands",    SEC_PLAYER,        &ChatHandler::HandleCommandsCommand,         "",   NULL },
        { "delete",      SEC_GAMEMASTER,    &ChatHandler::HandleDeleteCommand,           "",   NULL },
        { "demorph",     SEC_GAMEMASTER,    &ChatHandler::HandleDeMorphCommand,          "",   NULL },
        { "die",         SEC_ADMINISTRATOR, &ChatHandler::HandleDieCommand,              "",   NULL },
        { "revive",      SEC_ADMINISTRATOR, &ChatHandler::HandleReviveCommand,           "",   NULL },
        { "dismount",    SEC_PLAYER,        &ChatHandler::HandleDismountCommand,         "",   NULL },
        { "setmodel",    SEC_GAMEMASTER,    &ChatHandler::HandleSetModelCommand,         "",   NULL },
        { "factionid",   SEC_GAMEMASTER,    &ChatHandler::HandleFactionIdCommand,        "",   NULL },
        { "listgm",      SEC_PLAYER,        &ChatHandler::HandleGMListCommand,           "",   NULL },
        { "gm",          SEC_MODERATOR,     &ChatHandler::HandleGMmodeCommand,           "",   NULL },
        { "gps",         SEC_MODERATOR,     &ChatHandler::HandleGPSCommand,              "",   NULL },
        { "guid",        SEC_GAMEMASTER,    &ChatHandler::HandleGUIDCommand,             "",   NULL },
        { "help",        SEC_PLAYER,        &ChatHandler::HandleHelpCommand,             "",   NULL },
        { "info",        SEC_PLAYER,        &ChatHandler::HandleInfoCommand,             "",   NULL },
        { "npcinfo",     SEC_ADMINISTRATOR, &ChatHandler::HandleNpcInfoCommand,          "",   NULL },
        { "npcinfoset",  SEC_ADMINISTRATOR, &ChatHandler::HandleNpcInfoSetCommand,       "",   NULL },
        { "addvendoritem",SEC_GAMEMASTER,   &ChatHandler::HandleAddVendorItemCommand,    "",   NULL },
        { "delvendoritem",SEC_GAMEMASTER,   &ChatHandler::HandleDelVendorItemCommand,    "",   NULL },
        { "itemmove",    SEC_GAMEMASTER,    &ChatHandler::HandleItemMoveCommand,         "",   NULL },
        { "kick",        SEC_GAMEMASTER,    &ChatHandler::HandleKickPlayerCommand,       "",   NULL },
        { "learn",       SEC_MODERATOR,     NULL,                                        "",   learnCommandTable },
        { "cooldown",    SEC_ADMINISTRATOR, &ChatHandler::HandleCooldownCommand,         "",   NULL },
        { "unlearn",     SEC_ADMINISTRATOR, &ChatHandler::HandleUnLearnCommand,          "",   NULL },
        { "modify",      SEC_MODERATOR,     NULL,                                        "",   modifyCommandTable },
        { "debug",       SEC_MODERATOR,     NULL,                                        "",   debugCommandTable },
        { "morph",       SEC_GAMEMASTER,    &ChatHandler::HandleMorphCommand,            "",   NULL },
        { "name",        SEC_GAMEMASTER,    &ChatHandler::HandleNameCommand,             "",   NULL },
        { "subname",     SEC_GAMEMASTER,    &ChatHandler::HandleSubNameCommand,          "",   NULL },
        { "npcflag",     SEC_GAMEMASTER,    &ChatHandler::HandleNPCFlagCommand,          "",   NULL },
        { "distance",    SEC_ADMINISTRATOR, &ChatHandler::HandleGetDistanceCommand,      "",   NULL },
        { "addgo",       SEC_GAMEMASTER,    &ChatHandler::HandleGameObjectCommand,       "",   NULL },
        { "recall",      SEC_MODERATOR,     &ChatHandler::HandleRecallCommand,           "",   NULL },
        { "save",        SEC_PLAYER,        &ChatHandler::HandleSaveCommand,             "",   NULL },
        { "saveall",     SEC_MODERATOR,     &ChatHandler::HandleSaveAllCommand,          "",   NULL },
        { "security",    SEC_ADMINISTRATOR, &ChatHandler::HandleSecurityCommand,         "",   NULL },
        { "ban",         SEC_ADMINISTRATOR, &ChatHandler::HandleBanCommand,              "",   NULL },
        { "unban",       SEC_ADMINISTRATOR, &ChatHandler::HandleUnBanCommand,            "",   NULL },
        { "baninfo",     SEC_ADMINISTRATOR, &ChatHandler::HandleBanInfoCommand,          "",   NULL },
        { "banlist",     SEC_ADMINISTRATOR, &ChatHandler::HandleBanListCommand,          "",   NULL },
        { "standstate",  SEC_GAMEMASTER,    &ChatHandler::HandleStandStateCommand,       "",   NULL },
        { "start",       SEC_PLAYER,        &ChatHandler::HandleStartCommand,            "",   NULL },
        { "taxicheat",   SEC_MODERATOR,     &ChatHandler::HandleTaxiCheatCommand,        "",   NULL },
        { "gogrid",      SEC_MODERATOR,     &ChatHandler::HandleGoGridCommand,           "",   NULL },
        { "addweapon",   SEC_ADMINISTRATOR, &ChatHandler::HandleAddWeaponCommand,        "",   NULL },
        { "allowmove",   SEC_ADMINISTRATOR, &ChatHandler::HandleAllowMovementCommand,    "",   NULL },
        { "linkgrave",   SEC_ADMINISTRATOR, &ChatHandler::HandleLinkGraveCommand,        "",   NULL },
        { "neargrave",   SEC_ADMINISTRATOR, &ChatHandler::HandleNearGraveCommand,        "",   NULL },
        { "transport",   SEC_ADMINISTRATOR, &ChatHandler::HandleSpawnTransportCommand,   "",   NULL },
        { "explorecheat",SEC_ADMINISTRATOR, &ChatHandler::HandleExploreCheatCommand,     "",   NULL },
        { "hover",       SEC_ADMINISTRATOR, &ChatHandler::HandleHoverCommand,            "",   NULL },
        { "levelup",     SEC_ADMINISTRATOR, &ChatHandler::HandleLevelUpCommand,          "",   NULL },
        { "playemote",   SEC_ADMINISTRATOR, &ChatHandler::HandlePlayEmoteCommand,        "",   NULL },
        { "showarea",    SEC_ADMINISTRATOR, &ChatHandler::HandleShowAreaCommand,         "",   NULL },
        { "hidearea",    SEC_ADMINISTRATOR, &ChatHandler::HandleHideAreaCommand,         "",   NULL },
        { "addspw",      SEC_GAMEMASTER,    &ChatHandler::HandleAddSpwCommand,           "",   NULL },
        { "spawndist",   SEC_GAMEMASTER,    &ChatHandler::HandleSpawnDistCommand,        "",   NULL },
        { "spawntime",   SEC_GAMEMASTER,    &ChatHandler::HandleSpawnTimeCommand,        "",   NULL },
        { "additem",     SEC_ADMINISTRATOR, &ChatHandler::HandleAddItemCommand,          "",   NULL },
        { "additemset",  SEC_ADMINISTRATOR, &ChatHandler::HandleAddItemSetCommand,       "",   NULL },
        { "createguild", SEC_ADMINISTRATOR, &ChatHandler::HandleCreateGuildCommand,      "",   NULL },
        { "update",      SEC_ADMINISTRATOR, &ChatHandler::HandleUpdate,                  "",   NULL },
        { "bank",        SEC_ADMINISTRATOR, &ChatHandler::HandleBankCommand,             "",   NULL },
        { "wchange",     SEC_ADMINISTRATOR, &ChatHandler::HandleChangeWeather,           "",   NULL },
        { "reload",      SEC_ADMINISTRATOR, NULL,                                        "",   reloadCommandTable },
        { "loadscripts", SEC_ADMINISTRATOR, &ChatHandler::HandleLoadScriptsCommand,      "",   NULL },
        { "tele",        SEC_MODERATOR,     &ChatHandler::HandleTeleCommand,             "",   NULL },
        { "lookuptele",  SEC_MODERATOR,     &ChatHandler::HandleLookupTeleCommand,       "",   NULL },
        { "addtele",     SEC_ADMINISTRATOR, &ChatHandler::HandleAddTeleCommand,          "",   NULL },
        { "deltele",     SEC_ADMINISTRATOR, &ChatHandler::HandleDelTeleCommand,          "",   NULL },
        { "listauras",   SEC_ADMINISTRATOR, &ChatHandler::HandleListAurasCommand,        "",   NULL },
        { "reset",       SEC_ADMINISTRATOR, NULL,                                        "",   resetCommandTable },
        { "resetall",    SEC_ADMINISTRATOR, &ChatHandler::HandleResetAllCommand,         "",   NULL },
        { "ticket",      SEC_GAMEMASTER,    &ChatHandler::HandleTicketCommand,           "",   NULL },
        { "delticket",   SEC_GAMEMASTER,    &ChatHandler::HandleDelTicketCommand,        "",   NULL },
        { "maxskill",    SEC_ADMINISTRATOR, &ChatHandler::HandleMaxSkillCommand,         "",   NULL },
        { "setskill",    SEC_ADMINISTRATOR, &ChatHandler::HandleSetSkillCommand,         "",   NULL },
        { "whispers",    SEC_MODERATOR,     &ChatHandler::HandleWhispersCommand,         "",   NULL },
        { "say",         SEC_MODERATOR,     &ChatHandler::HandleSayCommand,              "",   NULL },
        { "npcwhisper",  SEC_MODERATOR,     &ChatHandler::HandleNpcWhisperCommand,       "",   NULL },
        { "yell",        SEC_MODERATOR,     &ChatHandler::HandleYellCommand,             "",   NULL },
        { "textemote",   SEC_MODERATOR,     &ChatHandler::HandleTextEmoteCommand,        "",   NULL },
        { "gocreature",  SEC_GAMEMASTER,    &ChatHandler::HandleGoCreatureCommand,       "",   NULL },
        { "goobject",    SEC_GAMEMASTER,    &ChatHandler::HandleGoObjectCommand,         "",   NULL },
        { "gotrigger",   SEC_GAMEMASTER,    &ChatHandler::HandleGoTriggerCommand,        "",   NULL },
        { "targetobject",SEC_GAMEMASTER,    &ChatHandler::HandleTargetObjectCommand,     "",   NULL },
        { "delobject",   SEC_GAMEMASTER,    &ChatHandler::HandleDelObjectCommand,        "",   NULL },
        { "turnobject",  SEC_GAMEMASTER,    &ChatHandler::HandleTurnObjectCommand,       "",   NULL },
        { "movecreature",SEC_GAMEMASTER,    &ChatHandler::HandleMoveCreatureCommand,     "",   NULL },
        { "moveobject",  SEC_GAMEMASTER,    &ChatHandler::HandleMoveObjectCommand,       "",   NULL },
        { "idleshutdown",SEC_ADMINISTRATOR, &ChatHandler::HandleIdleShutDownCommand,     "",   NULL },
        { "shutdown",    SEC_ADMINISTRATOR, &ChatHandler::HandleShutDownCommand,         "",   NULL },
        { "pinfo",       SEC_GAMEMASTER,    &ChatHandler::HandlePInfoCommand,            "",   NULL },
        { "plimit",      SEC_ADMINISTRATOR, &ChatHandler::HandlePLimitCommand,           "",   NULL },
        { "visible",     SEC_MODERATOR,     &ChatHandler::HandleVisibleCommand,          "",   NULL },
        { "playsound",   SEC_MODERATOR,     &ChatHandler::HandlePlaySoundCommand,        "",   NULL },
        { "listcreature",SEC_ADMINISTRATOR, &ChatHandler::HandleListCreatureCommand,     "",   NULL },
        { "listitem",    SEC_ADMINISTRATOR, &ChatHandler::HandleListItemCommand,         "",   NULL },
        { "listobject",  SEC_ADMINISTRATOR, &ChatHandler::HandleListObjectCommand,       "",   NULL },
        { "lookupitem",  SEC_ADMINISTRATOR, &ChatHandler::HandleLookupItemCommand,       "",   NULL },
        { "lookupitemset",SEC_ADMINISTRATOR,&ChatHandler::HandleLookupItemSetCommand,    "",   NULL },
        { "lookupskill", SEC_ADMINISTRATOR, &ChatHandler::HandleLookupSkillCommand,      "",   NULL },
        { "lookupspell", SEC_ADMINISTRATOR, &ChatHandler::HandleLookupSpellCommand,      "",   NULL },
        { "lookupquest", SEC_ADMINISTRATOR, &ChatHandler::HandleLookupQuestCommand,      "",   NULL },
        { "lookupcreature",SEC_ADMINISTRATOR,&ChatHandler::HandleLookupCreatureCommand,  "",   NULL },
        { "lookupobject",SEC_ADMINISTRATOR, &ChatHandler::HandleLookupObjectCommand,     "",   NULL },
        { "money",       SEC_MODERATOR,     &ChatHandler::HandleModifyMoneyCommand,      "",   NULL },
        { "titles",      SEC_MODERATOR,     &ChatHandler::HandleModifyKnownTitlesCommand, "",   NULL },
        { "speed",       SEC_MODERATOR,     &ChatHandler::HandleModifySpeedCommand,      "",   NULL },
        { "addquest",    SEC_ADMINISTRATOR, &ChatHandler::HandleAddQuest,                "",   NULL },
        { "removequest", SEC_ADMINISTRATOR, &ChatHandler::HandleRemoveQuest,             "",   NULL },
        { "password",    SEC_PLAYER,        &ChatHandler::HandlePasswordCommand,         "",   NULL },
        { "lockaccount", SEC_PLAYER,        &ChatHandler::HandleLockAccountCommand,      "",   NULL },
        { "respawn",     SEC_ADMINISTRATOR, &ChatHandler::HandleRespawnCommand,          "",   NULL },
        { "wp",          SEC_GAMEMASTER,    NULL,                                        "",   wpCommandTable },
        { "flymode",     SEC_ADMINISTRATOR, &ChatHandler::HandleFlyModeCommand,          "",   NULL },
        { "sendopcode",  SEC_ADMINISTRATOR, &ChatHandler::HandleSendOpcodeCommand,       "",   NULL },
        { "sellerr",     SEC_ADMINISTRATOR, &ChatHandler::HandleSellErrorCommand,        "",   NULL },
        { "buyerr",      SEC_ADMINISTRATOR, &ChatHandler::HandleBuyErrorCommand,         "",   NULL },
        { "uws",         SEC_ADMINISTRATOR, &ChatHandler::HandleUpdateWorldStateCommand, "",   NULL },
        { "ps",          SEC_ADMINISTRATOR, &ChatHandler::HandlePlaySound2Command,       "",   NULL },
        { "scn",         SEC_MODERATOR,     &ChatHandler::HandleSendChannelNotifyCommand,"",   NULL },
        { "scm",         SEC_MODERATOR,     &ChatHandler::HandleSendChatMsgCommand,      "",   NULL },
        { "sendmail",    SEC_MODERATOR,     &ChatHandler::HandleSendMailCommand,         "",   NULL },
        { "rename",      SEC_GAMEMASTER,    &ChatHandler::HandleRenameCommand,           "",   NULL },
        { "nametele",    SEC_MODERATOR,     &ChatHandler::HandleNameTeleCommand,         "",   NULL },
        { "grouptele",   SEC_MODERATOR,     &ChatHandler::HandleGroupTeleCommand,        "",   NULL },
        { "loadpdump",   SEC_ADMINISTRATOR, &ChatHandler::HandleLoadPDumpCommand,        "",   NULL },
        { "writepdump",  SEC_ADMINISTRATOR, &ChatHandler::HandleWritePDumpCommand,       "",   NULL },
        { "mute",        SEC_GAMEMASTER,    &ChatHandler::HandleMuteCommand,             "",   NULL },
        { "unmute",      SEC_GAMEMASTER,    &ChatHandler::HandleUnmuteCommand,           "",   NULL },
        { "honor",       SEC_GAMEMASTER,    NULL,                                        "",   honorCommandTable },
        { "movegens",    SEC_ADMINISTRATOR, &ChatHandler::HandleMovegensCommand,         "",   NULL },
        { "cast",        SEC_ADMINISTRATOR, &ChatHandler::HandleCastCommand,             "",   NULL },
        { "castback",    SEC_ADMINISTRATOR, &ChatHandler::HandleCastBackCommand,         "",   NULL },

        //! Development Commands
        { "setvalue",    SEC_ADMINISTRATOR, &ChatHandler::HandleSetValue,                "",   NULL },
        { "getvalue",    SEC_ADMINISTRATOR, &ChatHandler::HandleGetValue,                "",   NULL },
        { "Mod32Value",  SEC_ADMINISTRATOR, &ChatHandler::HandleMod32Value,              "",   NULL },

        { NULL,          0, NULL,                                        "",   NULL }
    };

    if(load_command_table)
    {
        load_command_table = false;

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
                            // first case for "" named subcommand
                            if (ptable[j].Name == "" && name == commandTable[i].Name ||
                                name == fmtstring("%s %s", commandTable[i].Name, ptable[j].Name) )
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
    const char* pos = strchr(line, '\n');
    while(pos != NULL)
    {
        if (pos-line > sizeof(buf)-1)
        {
            strncpy(buf, line, sizeof(buf)-1);
            buf[sizeof(buf)-1]=0;
            line += sizeof(buf) - 1;
        }
        else
        {
            strncpy(buf, line, pos-line);
            buf[pos-line]=0;
            line = pos+1;
            pos = strchr(line, '\n');
        }

        FillSystemMessageData(&data, session, buf);
        session->SendPacket(&data);
    }

    FillSystemMessageData(&data, session, line);
    session->SendPacket(&data);
}

void ChatHandler::SendGlobalSysMessage(const char *str)
{
    WorldPacket data;
    FillSystemMessageData(&data, m_session, str);
    sWorld.SendGlobalMessage(&data);
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
    char const* oldtext = text;
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
        // allow pass "" command name in table
        if(strlen(table[i].Name) && !hasStringAbbr(table[i].Name, cmd.c_str()))
            continue;

        // select subcommand from child commands list
        if(table[i].ChildCommands != NULL)
        {
            if(!ExecuteCommandInTable(table[i].ChildCommands, text))
            {
                if(table[i].Help != "")
                    SendSysMultilineMessage(table[i].Help.c_str());
                else if(table[i].Name != "")
                    SendSysMessage(LANG_NO_SUBCMD);
                else
                    SendSysMessage(LANG_CMD_SYNTAX);
            }

            return true;
        }

        // check security level only for simple  command (without child commands)
        if(m_session->GetSecurity() < table[i].SecurityLevel)
            continue;

        // table[i].Name == "" is special case: send original command to handler
        if((this->*(table[i].Handler))(strlen(table[i].Name)!=0 ? text : oldtext))
        {
            if(table[i].SecurityLevel > SEC_PLAYER)
            {
                Player* p = m_session->GetPlayer();
                uint64 sel_guid = p->GetSelection();
                sLog.outCommand("Command: %s [Player: %s (Account: %u) X: %f Y: %f Z: %f Map: %u Selected: %s (GUID: %u)]",
                    fullcmd.c_str(),p->GetName(),m_session->GetAccountId(),p->GetPositionX(),p->GetPositionY(),p->GetPositionZ(),p->GetMapId(),
                    (GUID_HIPART(sel_guid)==HIGHGUID_UNIT ? "creature" : (sel_guid !=0 ? "player" : "none")),
                    GUID_LOPART(sel_guid));
            }
        }
        else
        {
            if(table[i].Help != "")
                SendSysMultilineMessage(table[i].Help.c_str());
            else
                SendSysMessage(LANG_CMD_SYNTAX);
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
    if ((type != CHAT_MSG_CHANNEL && type != CHAT_MSG_WHISPER) || language == LANG_ADDON)
        *data << (uint32)language;
    else
        *data << (uint32)LANG_UNIVERSAL;

    switch(type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_PARTY:
        case CHAT_MSG_RAID:
        case CHAT_MSG_GUILD:
        case CHAT_MSG_OFFICER:
        case CHAT_MSG_YELL:
        case CHAT_MSG_WHISPER:
        case CHAT_MSG_CHANNEL:
        case CHAT_MSG_RAID_LEADER:
        case CHAT_MSG_RAID_WARNING:
        case CHAT_MSG_BG_SYSTEM_NEUTRAL:
        case CHAT_MSG_BG_SYSTEM_ALLIANCE:
        case CHAT_MSG_BG_SYSTEM_HORDE:
        case CHAT_MSG_BATTLEGROUND:
        case CHAT_MSG_BATTLEGROUND_LEADER:
            target_guid = session ? session->GetPlayer()->GetGUID() : 0;
            break;
        case CHAT_MSG_MONSTER_SAY:
            *data << (uint64)speaker->GetGUID();
            *data << uint32(0);                             // 2.1.0
            *data << (uint32)(strlen(((Creature *)speaker)->GetCreatureInfo()->Name) + 1);
            *data << ((Creature *)speaker)->GetCreatureInfo()->Name;
            break;
        default:
            if (type != CHAT_MSG_WHISPER_INFORM && type != CHAT_MSG_IGNORED && type != CHAT_MSG_DND && type != CHAT_MSG_AFK)
                target_guid = 0;                            // only for CHAT_MSG_WHISPER_INFORM used original value target_guid
            break;
    }
    /*// in CHAT_MSG_WHISPER_INFORM mode used original target_guid
    if (type == CHAT_MSG_SAY || type == CHAT_MSG_CHANNEL || type == CHAT_MSG_WHISPER ||
        type == CHAT_MSG_YELL || type == CHAT_MSG_PARTY || type == CHAT_MSG_RAID ||
        type == CHAT_MSG_RAID_LEADER || type == CHAT_MSG_RAID_WARN ||
        type == CHAT_MSG_GUILD || type == CHAT_MSG_OFFICER ||
        type == CHAT_MSG_BATTLEGROUND_ALLIANCE || type == CHAT_MSG_BATTLEGROUND_HORDE ||
        type == CHAT_MSG_BATTLEGROUND_CHAT || type == CHAT_MSG_BATTLEGROUND_LEADER)
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
    }*/

    *data << target_guid;                                   // there 0 for BG messages
    *data << uint32(0);                                     //2.1.0 unk

    if (type == CHAT_MSG_CHANNEL)
    {
        ASSERT(channelName);
        *data << channelName;
        //*data << target_guid;
    }
    /*if (type == CHAT_MSG_SYSTEM)
    {
        *data << (uint64)0;                                 // 2.1.0 unk
    }
    if (type == CHAT_MSG_SAY || type == CHAT_MSG_YELL || type == CHAT_MSG_PARTY)
        *data << target_guid;*/

    *data << target_guid;                                   // WTF?, for type=3

    *data << messageLength;
    *data << message;
    if(session != 0 && type != CHAT_MSG_WHISPER_INFORM && type != CHAT_MSG_DND && type != CHAT_MSG_AFK)
        *data << session->GetPlayer()->chatTag();
    else
        *data << uint8(0);
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
    return ObjectAccessor::Instance().GetCreatureOrPet(*m_session->GetPlayer(),m_session->GetPlayer()->GetSelection());
}

char*     ChatHandler::extractKeyFromLink(char* text, char const* linkType)
{
    // skip empty
    if(!text && !*text)
        return NULL;

    // return non link case
    if(text[0]!='|')
        return strtok(text, " ");

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r

    char* check = strtok(text, "|");                        // skip color
    if(!check)
        return NULL;                                        // end of data

    char* cLinkType = strtok(NULL, ":");                    // linktype
    if(!cLinkType)
        return NULL;                                        // end of data

    if(strcmp(cLinkType,linkType) != 0)
    {
        strtok(NULL, " ");                                  // skip link tail (to allow continue strtok(NULL,s) use after retturn from function
        SendSysMessage(LANG_WRONG_LINK_TYPE);
        return NULL;
    }

    char* cKey = strtok(NULL, ":|");                        // extract key
    strtok(NULL, "]");                                      // skip name with possible spalces
    strtok(NULL, " ");                                      // skip link tail (to allow continue strtok(NULL,s) use after retturn from function
    return cKey;
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
