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

/// \addtogroup u2w
/// @{
/// \file

#ifndef _OPCODES_H
#define _OPCODES_H

/// List of OpCodes
enum OpCodes
{
    MSG_NULL_ACTION                                 = 0,
    CMSG_BOOTME                                     = 1,
    CMSG_DBLOOKUP                                   = 2,
    SMSG_DBLOOKUP                                   = 3,
    CMSG_QUERY_OBJECT_POSITION                      = 4,
    SMSG_QUERY_OBJECT_POSITION                      = 5,
    CMSG_QUERY_OBJECT_ROTATION                      = 6,
    SMSG_QUERY_OBJECT_ROTATION                      = 7,
    CMSG_WORLD_TELEPORT                             = 8,
    CMSG_TELEPORT_TO_UNIT                           = 9,
    CMSG_ZONE_MAP                                   = 10,
    SMSG_ZONE_MAP                                   = 11,
    CMSG_DEBUG_CHANGECELLZONE                       = 12,
    CMSG_EMBLAZON_TABARD_OBSOLETE                   = 13,
    CMSG_UNEMBLAZON_TABARD_OBSOLETE                 = 14,
    CMSG_RECHARGE                                   = 15,
    CMSG_LEARN_SPELL                                = 16,
    CMSG_CREATEMONSTER                              = 17,
    CMSG_DESTROYMONSTER                             = 18,
    CMSG_CREATEITEM                                 = 19,
    CMSG_CREATEGAMEOBJECT                           = 20,

    //CMSG_MAKEMONSTERATTACKME                        = 21, //OBSOLETE
    CMSG_MAKEMONSTERATTACKME_OBSOLETE               = 21,
    CMSG_MAKEMONSTERATTACKGUID                      = 22,
    CMSG_ENABLEDEBUGCOMBATLOGGING_OBSOLETE          = 23,
    CMSG_FORCEACTION                                = 24,
    CMSG_FORCEACTIONONOTHER                         = 25,
    CMSG_FORCEACTIONSHOW                            = 26,
    SMSG_FORCEACTIONSHOW                            = 27,
    SMSG_ATTACKERSTATEUPDATEDEBUGINFO_OBSOLETE      = 28,
    SMSG_DEBUGINFOSPELL_OBSOLETE                    = 29,
    SMSG_DEBUGINFOSPELLMISS_OBSOLETE                = 30,
    SMSG_DEBUG_PLAYER_RANGE_OBSOLETE                = 31,
    CMSG_UNDRESSPLAYER                              = 32,
    CMSG_BEASTMASTER                                = 33,
    CMSG_GODMODE                                    = 34,
    SMSG_GODMODE                                    = 35,
    CMSG_CHEAT_SETMONEY                             = 36,
    CMSG_LEVEL_CHEAT                                = 37,
    CMSG_PET_LEVEL_CHEAT                            = 38,
    CMSG_LEVELUP_CHEAT_OBSOLETE                     = 39,
    CMSG_COOLDOWN_CHEAT                             = 40,
    CMSG_USE_SKILL_CHEAT                            = 41,
    CMSG_FLAG_QUEST                                 = 42,
    CMSG_FLAG_QUEST_FINISH                          = 43,
    CMSG_CLEAR_QUEST                                = 44,
    CMSG_SEND_EVENT                                 = 45,
    CMSG_DEBUG_AISTATE                              = 46,
    SMSG_DEBUG_AISTATE                              = 47,
    CMSG_DISABLE_PVP_CHEAT                          = 48,
    CMSG_ADVANCE_SPAWN_TIME                         = 49,
    CMSG_PVP_PORT_OBSOLETE                          = 50,
    CMSG_AUTH_SRP6_BEGIN                            = 51,
    CMSG_AUTH_SRP6_PROOF                            = 52,
    CMSG_AUTH_SRP6_RECODE                           = 53,
    CMSG_CHAR_CREATE                                = 54,
    CMSG_CHAR_ENUM                                  = 55,
    CMSG_CHAR_DELETE                                = 56,
    SMSG_AUTH_SRP6_RESPONSE                         = 57,
    SMSG_CHAR_CREATE                                = 58,
    SMSG_CHAR_ENUM                                  = 59,
    SMSG_CHAR_DELETE                                = 60,
    CMSG_PLAYER_LOGIN                               = 61,
    SMSG_NEW_WORLD                                  = 62,
    SMSG_TRANSFER_PENDING                           = 63,
    SMSG_TRANSFER_ABORTED                           = 64,
    SMSG_CHARACTER_LOGIN_FAILED                     = 65, // kick client to character select screen and show "World server is down".
    SMSG_LOGIN_SETTIMESPEED                         = 66,
    SMSG_GAMETIME_UPDATE                            = 67,
    CMSG_GAMETIME_SET                               = 68,
    SMSG_GAMETIME_SET                               = 69,
    CMSG_GAMESPEED_SET                              = 70,
    SMSG_GAMESPEED_SET                              = 71,
    CMSG_SERVERTIME                                 = 72,
    SMSG_SERVERTIME                                 = 73,
    CMSG_PLAYER_LOGOUT                              = 74,
    CMSG_LOGOUT_REQUEST                             = 75,
    SMSG_LOGOUT_RESPONSE                            = 76,
    SMSG_LOGOUT_COMPLETE                            = 77,
    CMSG_LOGOUT_CANCEL                              = 78,
    SMSG_LOGOUT_CANCEL_ACK                          = 79,
    CMSG_NAME_QUERY                                 = 80,
    SMSG_NAME_QUERY_RESPONSE                        = 81,
    CMSG_PET_NAME_QUERY                             = 82,
    SMSG_PET_NAME_QUERY_RESPONSE                    = 83,
    CMSG_GUILD_QUERY                                = 84,
    SMSG_GUILD_QUERY_RESPONSE                       = 85,
    CMSG_ITEM_QUERY_SINGLE                          = 86,
    CMSG_ITEM_QUERY_MULTIPLE                        = 87,
    SMSG_ITEM_QUERY_SINGLE_RESPONSE                 = 88,
    SMSG_ITEM_QUERY_MULTIPLE_RESPONSE               = 89,
    CMSG_PAGE_TEXT_QUERY                            = 90,
    SMSG_PAGE_TEXT_QUERY_RESPONSE                   = 91,
    CMSG_QUEST_QUERY                                = 92,
    SMSG_QUEST_QUERY_RESPONSE                       = 93,
    CMSG_GAMEOBJECT_QUERY                           = 94,
    SMSG_GAMEOBJECT_QUERY_RESPONSE                  = 95,
    CMSG_CREATURE_QUERY                             = 96,
    SMSG_CREATURE_QUERY_RESPONSE                    = 97,
    CMSG_WHO                                        = 98,
    SMSG_WHO                                        = 99,
    CMSG_WHOIS                                      = 100,
    SMSG_WHOIS                                      = 101,
    CMSG_FRIEND_LIST                                = 102,
    SMSG_FRIEND_LIST                                = 103,
    SMSG_FRIEND_STATUS                              = 104,
    CMSG_ADD_FRIEND                                 = 105,
    CMSG_DEL_FRIEND                                 = 106,
    SMSG_IGNORE_LIST                                = 107,
    CMSG_ADD_IGNORE                                 = 108,
    CMSG_DEL_IGNORE                                 = 109,
    CMSG_GROUP_INVITE                               = 110,
    SMSG_GROUP_INVITE                               = 111,
    CMSG_GROUP_CANCEL                               = 112,
    SMSG_GROUP_CANCEL                               = 113,
    CMSG_GROUP_ACCEPT                               = 114,
    CMSG_GROUP_DECLINE                              = 115,
    SMSG_GROUP_DECLINE                              = 116,
    CMSG_GROUP_UNINVITE                             = 117,
    CMSG_GROUP_UNINVITE_GUID                        = 118,
    SMSG_GROUP_UNINVITE                             = 119,
    CMSG_GROUP_SET_LEADER                           = 120,
    SMSG_GROUP_SET_LEADER                           = 121,
    CMSG_LOOT_METHOD                                = 122,
    CMSG_GROUP_DISBAND                              = 123,
    SMSG_GROUP_DESTROYED                            = 124,
    SMSG_GROUP_LIST                                 = 125,
    SMSG_PARTY_MEMBER_STATS                         = 126,
    SMSG_PARTY_COMMAND_RESULT                       = 127,
    UMSG_UPDATE_GROUP_MEMBERS                       = 128,
    CMSG_GUILD_CREATE                               = 129,
    CMSG_GUILD_INVITE                               = 130,
    SMSG_GUILD_INVITE                               = 131,
    CMSG_GUILD_ACCEPT                               = 132,
    CMSG_GUILD_DECLINE                              = 133,
    SMSG_GUILD_DECLINE                              = 134,
    CMSG_GUILD_INFO                                 = 135,
    SMSG_GUILD_INFO                                 = 136,
    CMSG_GUILD_ROSTER                               = 137,
    SMSG_GUILD_ROSTER                               = 138,
    CMSG_GUILD_PROMOTE                              = 139,
    CMSG_GUILD_DEMOTE                               = 140,
    CMSG_GUILD_LEAVE                                = 141,
    CMSG_GUILD_REMOVE                               = 142,
    CMSG_GUILD_DISBAND                              = 143,
    CMSG_GUILD_LEADER                               = 144,
    CMSG_GUILD_MOTD                                 = 145,
    SMSG_GUILD_EVENT                                = 146,
    SMSG_GUILD_COMMAND_RESULT                       = 147,
    UMSG_UPDATE_GUILD                               = 148,
    CMSG_MESSAGECHAT                                = 149,
    SMSG_MESSAGECHAT                                = 150,
    CMSG_JOIN_CHANNEL                               = 151,
    CMSG_LEAVE_CHANNEL                              = 152,
    SMSG_CHANNEL_NOTIFY                             = 153,
    CMSG_CHANNEL_LIST                               = 154,
    SMSG_CHANNEL_LIST                               = 155,
    CMSG_CHANNEL_PASSWORD                           = 156,
    CMSG_CHANNEL_SET_OWNER                          = 157,
    CMSG_CHANNEL_OWNER                              = 158,
    CMSG_CHANNEL_MODERATOR                          = 159,
    CMSG_CHANNEL_UNMODERATOR                        = 160,
    CMSG_CHANNEL_MUTE                               = 161,
    CMSG_CHANNEL_UNMUTE                             = 162,
    CMSG_CHANNEL_INVITE                             = 163,
    CMSG_CHANNEL_KICK                               = 164,
    CMSG_CHANNEL_BAN                                = 165,
    CMSG_CHANNEL_UNBAN                              = 166,
    CMSG_CHANNEL_ANNOUNCEMENTS                      = 167,
    CMSG_CHANNEL_MODERATE                           = 168,
    SMSG_UPDATE_OBJECT                              = 169,
    SMSG_DESTROY_OBJECT                             = 170,
    CMSG_USE_ITEM                                   = 171,
    CMSG_OPEN_ITEM                                  = 172,
    CMSG_READ_ITEM                                  = 173,
    SMSG_READ_ITEM_OK                               = 174,
    SMSG_READ_ITEM_FAILED                           = 175,
    SMSG_ITEM_COOLDOWN                              = 176,
    CMSG_GAMEOBJ_USE                                = 177,
    CMSG_GAMEOBJ_CHAIR_USE_OBSOLETE                 = 178,
    SMSG_GAMEOBJECT_CUSTOM_ANIM                     = 179,
    CMSG_AREATRIGGER                                = 180,
    MSG_MOVE_START_FORWARD                          = 181,
    MSG_MOVE_START_BACKWARD                         = 182,
    MSG_MOVE_STOP                                   = 183,
    MSG_MOVE_START_STRAFE_LEFT                      = 184,
    MSG_MOVE_START_STRAFE_RIGHT                     = 185,
    MSG_MOVE_STOP_STRAFE                            = 186,
    MSG_MOVE_JUMP                                   = 187,
    MSG_MOVE_START_TURN_LEFT                        = 188,
    MSG_MOVE_START_TURN_RIGHT                       = 189,
    MSG_MOVE_STOP_TURN                              = 190,
    MSG_MOVE_START_PITCH_UP                         = 191,
    MSG_MOVE_START_PITCH_DOWN                       = 192,
    MSG_MOVE_STOP_PITCH                             = 193,
    MSG_MOVE_SET_RUN_MODE                           = 194,
    MSG_MOVE_SET_WALK_MODE                          = 195,
    MSG_MOVE_TOGGLE_LOGGING                         = 196,
    MSG_MOVE_TELEPORT                               = 197,
    MSG_MOVE_TELEPORT_CHEAT                         = 198,
    MSG_MOVE_TELEPORT_ACK                           = 199,
    MSG_MOVE_TOGGLE_FALL_LOGGING                    = 200,
    MSG_MOVE_FALL_LAND                              = 201,
    MSG_MOVE_START_SWIM                             = 202,
    MSG_MOVE_STOP_SWIM                              = 203,
    MSG_MOVE_SET_RUN_SPEED_CHEAT                    = 204,
    MSG_MOVE_SET_RUN_SPEED                          = 205,
    MSG_MOVE_SET_RUN_BACK_SPEED_CHEAT               = 206,
    MSG_MOVE_SET_RUN_BACK_SPEED                     = 207,
    MSG_MOVE_SET_WALK_SPEED_CHEAT                   = 208,
    MSG_MOVE_SET_WALK_SPEED                         = 209,
    MSG_MOVE_SET_SWIM_SPEED_CHEAT                   = 210,
    MSG_MOVE_SET_SWIM_SPEED                         = 211,
    MSG_MOVE_SET_SWIM_BACK_SPEED_CHEAT              = 212,
    MSG_MOVE_SET_SWIM_BACK_SPEED                    = 213,
    MSG_MOVE_SET_ALL_SPEED_CHEAT                    = 214,
    MSG_MOVE_SET_TURN_RATE_CHEAT                    = 215,
    MSG_MOVE_SET_TURN_RATE                          = 216,
    MSG_MOVE_TOGGLE_COLLISION_CHEAT                 = 217,
    MSG_MOVE_SET_FACING                             = 218,
    MSG_MOVE_SET_PITCH                              = 219,
    MSG_MOVE_WORLDPORT_ACK                          = 220,
    SMSG_MONSTER_MOVE                               = 221,
    SMSG_MOVE_WATER_WALK                            = 222,
    SMSG_MOVE_LAND_WALK                             = 223,
    MSG_MOVE_SET_RAW_POSITION_ACK                   = 224,
    CMSG_MOVE_SET_RAW_POSITION                      = 225,
    SMSG_FORCE_RUN_SPEED_CHANGE                     = 226,
    CMSG_FORCE_RUN_SPEED_CHANGE_ACK                 = 227,
    SMSG_FORCE_RUN_BACK_SPEED_CHANGE                = 228,
    CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK            = 229,
    SMSG_FORCE_SWIM_SPEED_CHANGE                    = 230,
    CMSG_FORCE_SWIM_SPEED_CHANGE_ACK                = 231,
    SMSG_FORCE_MOVE_ROOT                            = 232,
    CMSG_FORCE_MOVE_ROOT_ACK                        = 233,
    SMSG_FORCE_MOVE_UNROOT                          = 234,
    CMSG_FORCE_MOVE_UNROOT_ACK                      = 235,
    MSG_MOVE_ROOT                                   = 236,
    MSG_MOVE_UNROOT                                 = 237,
    MSG_MOVE_HEARTBEAT                              = 238,
    SMSG_MOVE_KNOCK_BACK                            = 239,
    CMSG_MOVE_KNOCK_BACK_ACK                        = 240,
    MSG_MOVE_KNOCK_BACK                             = 241,
    SMSG_MOVE_FEATHER_FALL                          = 242,
    SMSG_MOVE_NORMAL_FALL                           = 243,
    SMSG_MOVE_SET_HOVER                             = 244,
    SMSG_MOVE_UNSET_HOVER                           = 245,
    CMSG_MOVE_HOVER_ACK                             = 246,
    MSG_MOVE_HOVER                                  = 247,
    CMSG_TRIGGER_CINEMATIC_CHEAT                    = 248,
    CMSG_OPENING_CINEMATIC                          = 249,
    SMSG_TRIGGER_CINEMATIC                          = 250,
    CMSG_NEXT_CINEMATIC_CAMERA                      = 251,
    CMSG_COMPLETE_CINEMATIC                         = 252,
    SMSG_TUTORIAL_FLAGS                             = 253,
    CMSG_TUTORIAL_FLAG                              = 254,
    CMSG_TUTORIAL_CLEAR                             = 255,
    CMSG_TUTORIAL_RESET                             = 256,
    CMSG_STANDSTATECHANGE                           = 257,
    CMSG_EMOTE                                      = 258,
    SMSG_EMOTE                                      = 259,
    CMSG_TEXT_EMOTE                                 = 260,
    SMSG_TEXT_EMOTE                                 = 261,
    CMSG_AUTOEQUIP_GROUND_ITEM                      = 262,
    CMSG_AUTOSTORE_GROUND_ITEM                      = 263,
    CMSG_AUTOSTORE_LOOT_ITEM                        = 264,
    CMSG_STORE_LOOT_IN_SLOT                         = 265,
    CMSG_AUTOEQUIP_ITEM                             = 266,
    CMSG_AUTOSTORE_BAG_ITEM                         = 267,
    CMSG_SWAP_ITEM                                  = 268,
    CMSG_SWAP_INV_ITEM                              = 269,
    CMSG_SPLIT_ITEM                                 = 270,
    CMSG_PICKUP_ITEM                                = 271,
    CMSG_DROP_ITEM                                  = 272,
    CMSG_DESTROYITEM                                = 273,
    SMSG_INVENTORY_CHANGE_FAILURE                   = 274,
    SMSG_OPEN_CONTAINER                             = 275,
    CMSG_INSPECT                                    = 276,
    SMSG_INSPECT                                    = 277,
    CMSG_INITIATE_TRADE                             = 278,
    CMSG_BEGIN_TRADE                                = 279,
    CMSG_BUSY_TRADE                                 = 280,
    CMSG_IGNORE_TRADE                               = 281,
    CMSG_ACCEPT_TRADE                               = 282,
    CMSG_UNACCEPT_TRADE                             = 283,
    CMSG_CANCEL_TRADE                               = 284,
    CMSG_SET_TRADE_ITEM                             = 285,
    CMSG_CLEAR_TRADE_ITEM                           = 286,
    CMSG_SET_TRADE_GOLD                             = 287,
    SMSG_TRADE_STATUS                               = 288,
    SMSG_TRADE_STATUS_EXTENDED                      = 289,
    SMSG_INITIALIZE_FACTIONS                        = 290,
    SMSG_SET_FACTION_VISIBLE                        = 291,
    SMSG_SET_FACTION_STANDING                       = 292,
    CMSG_SET_FACTION_ATWAR                          = 293,
    CMSG_SET_FACTION_CHEAT                          = 294,
    SMSG_SET_PROFICIENCY                            = 295,
    CMSG_SET_ACTION_BUTTON                          = 296,
    SMSG_ACTION_BUTTONS                             = 297,
    SMSG_INITIAL_SPELLS                             = 298,
    SMSG_LEARNED_SPELL                              = 299,
    SMSG_SUPERCEDED_SPELL                           = 300,
    CMSG_NEW_SPELL_SLOT                             = 301,
    CMSG_CAST_SPELL                                 = 302,
    CMSG_CANCEL_CAST                                = 303,
    SMSG_CAST_RESULT                                = 304,
    SMSG_SPELL_START                                = 305,
    SMSG_SPELL_GO                                   = 306,
    SMSG_SPELL_FAILURE                              = 307,
    SMSG_SPELL_COOLDOWN                             = 308,
    SMSG_COOLDOWN_EVENT                             = 309,
    CMSG_CANCEL_AURA                                = 310,
    SMSG_UPDATE_AURA_DURATION                       = 311,
    SMSG_PET_CAST_FAILED                            = 312, // Your pet is in combat
    MSG_CHANNEL_START                               = 313,
    MSG_CHANNEL_UPDATE                              = 314,
    CMSG_CANCEL_CHANNELLING                         = 315,
    SMSG_AI_REACTION                                = 316,
    CMSG_SET_SELECTION                              = 317,

    //CMSG_SET_TARGET                                = 318, //OBSOLETE
    CMSG_SET_TARGET_OBSOLETE                        = 318,
    CMSG_UNUSED                                     = 319,
    CMSG_UNUSED2                                    = 320,
    CMSG_ATTACKSWING                                = 321,
    CMSG_ATTACKSTOP                                 = 322,
    SMSG_ATTACKSTART                                = 323,
    SMSG_ATTACKSTOP                                 = 324,
    SMSG_ATTACKSWING_NOTINRANGE                     = 325,
    SMSG_ATTACKSWING_BADFACING                      = 326,
    SMSG_ATTACKSWING_NOTSTANDING                    = 327,
    SMSG_ATTACKSWING_DEADTARGET                     = 328,
    SMSG_ATTACKSWING_CANT_ATTACK                    = 329,
    SMSG_ATTACKERSTATEUPDATE                        = 330,
    SMSG_VICTIMSTATEUPDATE_OBSOLETE                 = 331,
    SMSG_DAMAGE_DONE_OBSOLETE                       = 332,
    SMSG_DAMAGE_TAKEN_OBSOLETE                      = 333,
    SMSG_CANCEL_COMBAT                              = 334,
    SMSG_PLAYER_COMBAT_XP_GAIN_OBSOLETE             = 335,
    SMSG_HEALSPELL_ON_PLAYER_OBSOLETE               = 336,
    SMSG_HEALSPELL_ON_PLAYERS_PET_OBSOLETE          = 337,
    CMSG_SHEATHE_OBSOLETE                           = 338,
    CMSG_SAVE_PLAYER                                = 339,
    CMSG_SETDEATHBINDPOINT                          = 340,
    SMSG_BINDPOINTUPDATE                            = 341,
    CMSG_GETDEATHBINDZONE                           = 342,
    SMSG_BINDZONEREPLY                              = 343,
    SMSG_PLAYERBOUND                                = 344,
    SMSG_DEATH_NOTIFY_OBSOLETE                      = 345,
    CMSG_REPOP_REQUEST                              = 346,
    SMSG_RESURRECT_REQUEST                          = 347,
    CMSG_RESURRECT_RESPONSE                         = 348,
    CMSG_LOOT                                       = 349,
    CMSG_LOOT_MONEY                                 = 350,
    CMSG_LOOT_RELEASE                               = 351,
    SMSG_LOOT_RESPONSE                              = 352,
    SMSG_LOOT_RELEASE_RESPONSE                      = 353,
    SMSG_LOOT_REMOVED                               = 354,
    SMSG_LOOT_MONEY_NOTIFY                          = 355,
    SMSG_LOOT_ITEM_NOTIFY                           = 356,
    SMSG_LOOT_CLEAR_MONEY                           = 357,
    SMSG_ITEM_PUSH_RESULT                           = 358,
    SMSG_DUEL_REQUESTED                             = 359,
    SMSG_DUEL_OUTOFBOUNDS                           = 360,
    SMSG_DUEL_INBOUNDS                              = 361,
    SMSG_DUEL_COMPLETE                              = 362,
    SMSG_DUEL_WINNER                                = 363,
    CMSG_DUEL_ACCEPTED                              = 364,
    CMSG_DUEL_CANCELLED                             = 365,
    SMSG_MOUNTRESULT                                = 366, // 0 - can't mount that unit, 1 - mount too far away, 2 - already mounted, 3 - that unit can't be mounted, 4 - that mount is not our pet, 5 - unknown mount error, 6 - can't mount while looting, 7 - cant mount because your race, 8 - shapeshifted, 9 - you dismount before continuing
    SMSG_DISMOUNTRESULT                             = 367, // 0 - int err, don't have pet to dismount, 1 - not mounted, 2 - int err, dismounting a non-pet
    SMSG_PUREMOUNT_CANCELLED_OBSOLETE               = 368,
    CMSG_MOUNTSPECIAL_ANIM                          = 369,
    SMSG_MOUNTSPECIAL_ANIM                          = 370,
    SMSG_PET_TAME_FAILURE                           = 371, // uint32: 0 - Unknown taming error, 1 - Creature not found, 2 - too many pets, 3 - creature already controlled, 4 - creature not tameable, 5 - have active summon, 6 - can't tame, 7 - don't have pet to summon, 8 - internal pet error, 9 - high level, 10 - your pet is dead, 11 - your pet is not dead
    CMSG_PET_SET_ACTION                             = 372,
    CMSG_PET_ACTION                                 = 373,
    CMSG_PET_ABANDON                                = 374,
    CMSG_PET_RENAME                                 = 375,
    SMSG_PET_NAME_INVALID                           = 376, // Error, invalid name entered.
    SMSG_PET_SPELLS                                 = 377,

    //CMSG_PET_CAST_SPELL_OBSOLETE                    = 378, //OBSOLETE
    SMSG_PET_MODE                                   = 378, // uint64 pet_guid + uint32 flags?
    CMSG_GOSSIP_HELLO                               = 379,
    CMSG_GOSSIP_SELECT_OPTION                       = 380,
    SMSG_GOSSIP_MESSAGE                             = 381,
    SMSG_GOSSIP_COMPLETE                            = 382,
    CMSG_NPC_TEXT_QUERY                             = 383,
    SMSG_NPC_TEXT_UPDATE                            = 384,
    SMSG_NPC_WONT_TALK                              = 385,
    CMSG_QUESTGIVER_STATUS_QUERY                    = 386,
    SMSG_QUESTGIVER_STATUS                          = 387,
    CMSG_QUESTGIVER_HELLO                           = 388,
    SMSG_QUESTGIVER_QUEST_LIST                      = 389,
    CMSG_QUESTGIVER_QUERY_QUEST                     = 390,
    CMSG_QUESTGIVER_QUEST_AUTOLAUNCH                = 391,
    SMSG_QUESTGIVER_QUEST_DETAILS                   = 392,
    CMSG_QUESTGIVER_ACCEPT_QUEST                    = 393,
    CMSG_QUESTGIVER_COMPLETE_QUEST                  = 394,
    SMSG_QUESTGIVER_REQUEST_ITEMS                   = 395,
    CMSG_QUESTGIVER_REQUEST_REWARD                  = 396,
    SMSG_QUESTGIVER_OFFER_REWARD                    = 397,
    CMSG_QUESTGIVER_CHOOSE_REWARD                   = 398,
    SMSG_QUESTGIVER_QUEST_INVALID                   = 399,
    CMSG_QUESTGIVER_CANCEL                          = 400,
    SMSG_QUESTGIVER_QUEST_COMPLETE                  = 401,
    SMSG_QUESTGIVER_QUEST_FAILED                    = 402,
    CMSG_QUESTLOG_SWAP_QUEST                        = 403,
    CMSG_QUESTLOG_REMOVE_QUEST                      = 404,
    SMSG_QUESTLOG_FULL                              = 405,
    SMSG_QUESTUPDATE_FAILED                         = 406,
    SMSG_QUESTUPDATE_FAILEDTIMER                    = 407,
    SMSG_QUESTUPDATE_COMPLETE                       = 408,
    SMSG_QUESTUPDATE_ADD_KILL                       = 409,
    SMSG_QUESTUPDATE_ADD_ITEM                       = 410,
    CMSG_QUEST_CONFIRM_ACCEPT                       = 411,
    SMSG_QUEST_CONFIRM_ACCEPT                       = 412,
    CMSG_PUSHQUESTTOPARTY                           = 413,
    CMSG_LIST_INVENTORY                             = 414,
    SMSG_LIST_INVENTORY                             = 415,
    CMSG_SELL_ITEM                                  = 416,
    SMSG_SELL_ITEM                                  = 417,
    CMSG_BUY_ITEM                                   = 418,
    CMSG_BUY_ITEM_IN_SLOT                           = 419,
    SMSG_BUY_ITEM                                   = 420,
    SMSG_BUY_FAILED                                 = 421,
    CMSG_TAXICLEARALLNODES                          = 422,
    CMSG_TAXIENABLEALLNODES                         = 423,
    CMSG_TAXISHOWNODES                              = 424,
    SMSG_SHOWTAXINODES                              = 425,
    CMSG_TAXINODE_STATUS_QUERY                      = 426,
    SMSG_TAXINODE_STATUS                            = 427,
    CMSG_TAXIQUERYAVAILABLENODES                    = 428,
    CMSG_ACTIVATETAXI                               = 429,
    SMSG_ACTIVATETAXIREPLY                          = 430,
    SMSG_NEW_TAXI_PATH                              = 431,
    CMSG_TRAINER_LIST                               = 432,
    SMSG_TRAINER_LIST                               = 433,
    CMSG_TRAINER_BUY_SPELL                          = 434,
    SMSG_TRAINER_BUY_SUCCEEDED                      = 435,
    SMSG_TRAINER_BUY_FAILED                         = 436,
    CMSG_BINDER_ACTIVATE                            = 437,
    SMSG_PLAYERBINDERROR                            = 438, // You already bound there.
    CMSG_BANKER_ACTIVATE                            = 439,
    SMSG_SHOW_BANK                                  = 440,
    CMSG_BUY_BANK_SLOT                              = 441,
    SMSG_BUY_BANK_SLOT_RESULT                       = 442,
    CMSG_PETITION_SHOWLIST                          = 443,
    SMSG_PETITION_SHOWLIST                          = 444,
    CMSG_PETITION_BUY                               = 445,
    CMSG_PETITION_SHOW_SIGNATURES                   = 446,
    SMSG_PETITION_SHOW_SIGNATURES                   = 447,
    CMSG_PETITION_SIGN                              = 448,
    SMSG_PETITION_SIGN_RESULTS                      = 449,
    MSG_PETITION_DECLINE                            = 450,
    CMSG_OFFER_PETITION                             = 451,
    CMSG_TURN_IN_PETITION                           = 452,
    SMSG_TURN_IN_PETITION_RESULTS                   = 453,
    CMSG_PETITION_QUERY                             = 454,
    SMSG_PETITION_QUERY_RESPONSE                    = 455,
    SMSG_FISH_NOT_HOOKED                            = 456,
    SMSG_FISH_ESCAPED                               = 457,
    CMSG_BUG                                        = 458,
    SMSG_NOTIFICATION                               = 459,
    CMSG_PLAYED_TIME                                = 460,
    SMSG_PLAYED_TIME                                = 461,
    CMSG_QUERY_TIME                                 = 462,
    SMSG_QUERY_TIME_RESPONSE                        = 463,
    SMSG_LOG_XPGAIN                                 = 464,
    MSG_SPLIT_MONEY                                 = 465,
    CMSG_RECLAIM_CORPSE                             = 466,
    CMSG_WRAP_ITEM                                  = 467,
    SMSG_LEVELUP_INFO                               = 468,
    MSG_MINIMAP_PING                                = 469,
    SMSG_RESISTLOG                                  = 470,
    SMSG_ENCHANTMENTLOG                             = 471,
    CMSG_SET_SKILL_CHEAT                            = 472,
    SMSG_START_MIRROR_TIMER                         = 473,
    SMSG_PAUSE_MIRROR_TIMER                         = 474,
    SMSG_STOP_MIRROR_TIMER                          = 475,
    CMSG_PING                                       = 476,
    SMSG_PONG                                       = 477,
    SMSG_CLEAR_COOLDOWN                             = 478,
    SMSG_GAMEOBJECT_PAGETEXT                        = 479, // uint64 guid
    CMSG_SETSHEATHED                                = 480,
    SMSG_COOLDOWN_CHEAT                             = 481,
    SMSG_SPELL_DELAYED                              = 482,
    CMSG_PLAYER_MACRO_OBSOLETE                      = 483,
    SMSG_PLAYER_MACRO_OBSOLETE                      = 484,
    CMSG_GHOST                                      = 485,
    CMSG_GM_INVIS                                   = 486,

    //CMSG_SCREENSHOT                                = 487, //OBSOLETE
    SMSG_INVALID_PROMOTION_CODE                     = 487, // Couldn't validate code, please try again.
    MSG_GM_BIND_OTHER                               = 488,
    MSG_GM_SUMMON                                   = 489,
    SMSG_ITEM_TIME_UPDATE                           = 490, // uint64 guid + uint32 time
    SMSG_ITEM_ENCHANT_TIME_UPDATE                   = 491,
    SMSG_AUTH_CHALLENGE                             = 492,
    CMSG_AUTH_SESSION                               = 493,
    SMSG_AUTH_RESPONSE                              = 494,
    MSG_GM_SHOWLABEL                                = 495,

    //MSG_ADD_DYNAMIC_TARGET                        = 496, //OBSOLETE
    MSG_ADD_DYNAMIC_TARGET_OBSOLETE                 = 496,
    MSG_SAVE_GUILD_EMBLEM                           = 497,
    MSG_TABARDVENDOR_ACTIVATE                       = 498,
    SMSG_PLAY_SPELL_VISUAL                          = 499,
    CMSG_ZONEUPDATE                                 = 500,
    SMSG_PARTYKILLLOG                               = 501,
    SMSG_COMPRESSED_UPDATE_OBJECT                   = 502,
    SMSG_OBSOLETE                                   = 503,
    SMSG_EXPLORATION_EXPERIENCE                     = 504,
    CMSG_GM_SET_SECURITY_GROUP                      = 505,
    CMSG_GM_NUKE                                    = 506,
    MSG_RANDOM_ROLL                                 = 507,
    SMSG_ENVIRONMENTALDAMAGELOG                     = 508,
    CMSG_RWHOIS                                     = 509,
    SMSG_RWHOIS                                     = 510,
    MSG_LOOKING_FOR_GROUP                           = 511,
    CMSG_SET_LOOKING_FOR_GROUP                      = 512,
    CMSG_UNLEARN_SPELL                              = 513,
    CMSG_UNLEARN_SKILL                              = 514,
    SMSG_REMOVED_SPELL                              = 515,
    CMSG_DECHARGE                                   = 516,
    CMSG_GMTICKET_CREATE                            = 517,
    SMSG_GMTICKET_CREATE                            = 518,
    CMSG_GMTICKET_UPDATETEXT                        = 519,
    SMSG_GMTICKET_UPDATETEXT                        = 520,
    SMSG_ACCOUNT_DATA_MD5                           = 521,
    CMSG_REQUEST_ACCOUNT_DATA                       = 522,
    CMSG_UPDATE_ACCOUNT_DATA                        = 523,
    SMSG_UPDATE_ACCOUNT_DATA                        = 524,
    SMSG_CLEAR_FAR_SIGHT_IMMEDIATE                  = 525,
    SMSG_POWERGAINLOG_OBSOLETE                      = 526,
    CMSG_GM_TEACH                                   = 527,
    CMSG_GM_CREATE_ITEM_TARGET                      = 528,
    CMSG_GMTICKET_GETTICKET                         = 529,
    SMSG_GMTICKET_GETTICKET                         = 530,
    CMSG_UNLEARN_TALENTS                            = 531,
    SMSG_GAMEOBJECT_SPAWN_ANIM                      = 532,
    SMSG_GAMEOBJECT_DESPAWN_ANIM                    = 533,
    MSG_CORPSE_QUERY                                = 534,
    CMSG_GMTICKET_DELETETICKET                      = 535,
    SMSG_GMTICKET_DELETETICKET                      = 536,
    SMSG_CHAT_WRONG_FACTION                         = 537, // You can only whisper to the members of your alliance.
    CMSG_GMTICKET_SYSTEMSTATUS                      = 538,
    SMSG_GMTICKET_SYSTEMSTATUS                      = 539,
    CMSG_SPIRIT_HEALER_ACTIVATE                     = 540,
    CMSG_SET_STAT_CHEAT                             = 541,
    SMSG_SET_REST_START                             = 542,
    CMSG_SKILL_BUY_STEP                             = 543,
    CMSG_SKILL_BUY_RANK                             = 544,
    CMSG_XP_CHEAT                                   = 545,
    SMSG_SPIRIT_HEALER_CONFIRM                      = 546,
    CMSG_CHARACTER_POINT_CHEAT                      = 547,
    SMSG_GOSSIP_POI                                 = 548,
    CMSG_CHAT_IGNORED                               = 549,
    CMSG_GM_VISION                                  = 550,
    CMSG_SERVER_COMMAND                             = 551,
    CMSG_GM_SILENCE                                 = 552,
    CMSG_GM_REVEALTO                                = 553,
    CMSG_GM_RESURRECT                               = 554,
    CMSG_GM_SUMMONMOB                               = 555,
    CMSG_GM_MOVECORPSE                              = 556,
    CMSG_GM_FREEZE                                  = 557,
    CMSG_GM_UBERINVIS                               = 558,
    CMSG_GM_REQUEST_PLAYER_INFO                     = 559,
    SMSG_GM_PLAYER_INFO                             = 560,
    CMSG_GUILD_RANK                                 = 561,
    CMSG_GUILD_ADD_RANK                             = 562,
    CMSG_GUILD_DEL_RANK                             = 563,
    CMSG_GUILD_SET_PUBLIC_NOTE                      = 564,
    CMSG_GUILD_SET_OFFICER_NOTE                     = 565,
    SMSG_LOGIN_VERIFY_WORLD                         = 566,
    CMSG_CLEAR_EXPLORATION                          = 567,
    CMSG_SEND_MAIL                                  = 568,
    SMSG_SEND_MAIL_RESULT                           = 569,
    CMSG_GET_MAIL_LIST                              = 570,
    SMSG_MAIL_LIST_RESULT                           = 571,
    CMSG_BATTLEFIELD_LIST                           = 572,
    SMSG_BATTLEFIELD_LIST                           = 573,
    CMSG_BATTLEFIELD_JOIN                           = 574,
    SMSG_BATTLEFIELD_WIN                            = 575,
    SMSG_BATTLEFIELD_LOSE                           = 576,
    CMSG_TAXICLEARNODE                              = 577,
    CMSG_TAXIENABLENODE                             = 578,
    CMSG_ITEM_TEXT_QUERY                            = 579,
    SMSG_ITEM_TEXT_QUERY_RESPONSE                   = 580,
    CMSG_MAIL_TAKE_MONEY                            = 581,
    CMSG_MAIL_TAKE_ITEM                             = 582,
    CMSG_MAIL_MARK_AS_READ                          = 583,
    CMSG_MAIL_RETURN_TO_SENDER                      = 584,
    CMSG_MAIL_DELETE                                = 585,
    CMSG_MAIL_CREATE_TEXT_ITEM                      = 586,
    SMSG_SPELLLOGMISS                               = 587,
    SMSG_SPELLLOGEXECUTE                            = 588,
    SMSG_DEBUGAURAPROC                              = 589,
    SMSG_PERIODICAURALOG                            = 590,
    SMSG_SPELLDAMAGESHIELD                          = 591,
    SMSG_SPELLNONMELEEDAMAGELOG                     = 592,
    CMSG_LEARN_TALENT                               = 593,
    SMSG_RESURRECT_FAILED                           = 594,

    //CMSG_ENABLE_PVP                                = 595, //OBSOLETE
    CMSG_TOGGLE_PVP                                 = 595,
    SMSG_ZONE_UNDER_ATTACK                          = 596,
    MSG_AUCTION_HELLO                               = 597,
    CMSG_AUCTION_SELL_ITEM                          = 598,
    CMSG_AUCTION_REMOVE_ITEM                        = 599,
    CMSG_AUCTION_LIST_ITEMS                         = 600,
    CMSG_AUCTION_LIST_OWNER_ITEMS                   = 601,
    CMSG_AUCTION_PLACE_BID                          = 602,
    SMSG_AUCTION_COMMAND_RESULT                     = 603,
    SMSG_AUCTION_LIST_RESULT                        = 604,
    SMSG_AUCTION_OWNER_LIST_RESULT                  = 605,
    SMSG_AUCTION_BIDDER_NOTIFICATION                = 606,
    SMSG_AUCTION_OWNER_NOTIFICATION                 = 607,
    SMSG_PROCRESIST                                 = 608,
    SMSG_STANDSTATE_CHANGE_FAILURE                  = 609,
    SMSG_DISPEL_FAILED                              = 610,
    SMSG_SPELLORDAMAGE_IMMUNE                       = 611,
    CMSG_AUCTION_LIST_BIDDER_ITEMS                  = 612,
    SMSG_AUCTION_BIDDER_LIST_RESULT                 = 613,
    SMSG_SET_FLAT_SPELL_MODIFIER                    = 614,
    SMSG_SET_PCT_SPELL_MODIFIER                     = 615,
    CMSG_SET_AMMO                                   = 616,
    SMSG_CORPSE_RECLAIM_DELAY                       = 617,
    CMSG_SET_ACTIVE_MOVER                           = 618,
    CMSG_PET_CANCEL_AURA                            = 619,
    CMSG_PLAYER_AI_CHEAT                            = 620,
    CMSG_CANCEL_AUTO_REPEAT_SPELL                   = 621,
    MSG_GM_ACCOUNT_ONLINE                           = 622,
    MSG_LIST_STABLED_PETS                           = 623,
    CMSG_STABLE_PET                                 = 624,
    CMSG_UNSTABLE_PET                               = 625,
    CMSG_BUY_STABLE_SLOT                            = 626,
    SMSG_STABLE_RESULT                              = 627,
    CMSG_STABLE_REVIVE_PET                          = 628,
    CMSG_STABLE_SWAP_PET                            = 629,
    MSG_QUEST_PUSH_RESULT                           = 630,
    SMSG_PLAY_MUSIC                                 = 631,
    SMSG_PLAY_OBJECT_SOUND                          = 632,
    CMSG_REQUEST_PET_INFO                           = 633,
    CMSG_FAR_SIGHT                                  = 634,
    SMSG_SPELLDISPELLOG                             = 635,
    SMSG_DAMAGE_CALC_LOG                            = 636,
    CMSG_ENABLE_DAMAGE_LOG                          = 637,
    CMSG_GROUP_CHANGE_SUB_GROUP                     = 638,

    //SMSG_RAID_MEMBER_STATS                        = 639, //OBSOLETE
    CMSG_REQUEST_PARTY_MEMBER_STATS                 = 639,
    CMSG_GROUP_SWAP_SUB_GROUP                       = 640,
    CMSG_RESET_FACTION_CHEAT                        = 641,
    CMSG_AUTOSTORE_BANK_ITEM                        = 642,
    CMSG_AUTOBANK_ITEM                              = 643,
    MSG_QUERY_NEXT_MAIL_TIME                        = 644,
    SMSG_RECEIVED_MAIL                              = 645,
    SMSG_RAID_GROUP_ONLY                            = 646, // You are not in this instance group. You will be teleported to %s in %u Minites. uint32 time(milliseconds)+uint32 unk
    CMSG_SET_DURABILITY_CHEAT                       = 647,
    CMSG_SET_PVP_RANK_CHEAT                         = 648,
    CMSG_ADD_PVP_MEDAL_CHEAT                        = 649,
    CMSG_DEL_PVP_MEDAL_CHEAT                        = 650,
    CMSG_SET_PVP_TITLE                              = 651,
    SMSG_PVP_CREDIT                                 = 652,
    SMSG_AUCTION_REMOVED_NOTIFICATION               = 653,
    CMSG_GROUP_RAID_CONVERT                         = 654,
    CMSG_GROUP_ASSISTANT                            = 655,
    CMSG_BUYBACK_ITEM                               = 656,
    SMSG_SERVER_MESSAGE                             = 657,
    CMSG_MEETINGSTONE_JOIN                          = 658,
    CMSG_MEETINGSTONE_LEAVE                         = 659,
    CMSG_MEETINGSTONE_CHEAT                         = 660,
    SMSG_MEETINGSTONE_SETQUEUE                      = 661, // You have left the queue to join a party for %s. 
    CMSG_MEETINGSTONE_INFO                          = 662,
    SMSG_MEETINGSTONE_COMPLETE                      = 663, // Your group is complete, you have left the LFG matchmaking system.
    SMSG_MEETINGSTONE_IN_PROGRESS                   = 664, // You still seeking more members through LFG matchmaking system., empty?
    SMSG_MEETINGSTONE_MEMBER_ADDED                  = 665, // %s has been added to the group by the LFG matchmaking system.
    CMSG_GMTICKETSYSTEM_TOGGLE                      = 666,
    CMSG_CANCEL_GROWTH_AURA                         = 667,
    SMSG_CANCEL_AUTO_REPEAT                         = 668,
    SMSG_STANDSTATE_CHANGE_ACK                      = 669,
    SMSG_LOOT_ALL_PASSED                            = 670,
    SMSG_LOOT_ROLL_WON                              = 671,
    CMSG_LOOT_ROLL                                  = 672,
    SMSG_LOOT_START_ROLL                            = 673,
    SMSG_LOOT_ROLL                                  = 674,
    CMSG_LOOT_MASTER_GIVE                           = 675,
    SMSG_LOOT_MASTER_LIST                           = 676,
    SMSG_SET_FORCED_REACTIONS                       = 677,
    SMSG_SPELL_FAILED_OTHER                         = 678,
    SMSG_GAMEOBJECT_RESET_STATE                     = 679, // uint64 guid
    CMSG_REPAIR_ITEM                                = 680,
    SMSG_CHAT_PLAYER_NOT_FOUND                      = 681,
    MSG_TALENT_WIPE_CONFIRM                         = 682,
    SMSG_SUMMON_REQUEST                             = 683, // uint64 guid + uint32 area/zoneid + uint32 time(milliseconds?)
    CMSG_SUMMON_RESPONSE                            = 684,
    MSG_MOVE_TOGGLE_GRAVITY_CHEAT                   = 685,
    SMSG_MONSTER_MOVE_TRANSPORT                     = 686,
    SMSG_PET_BROKEN                                 = 687, // Your pet has run away (:D)
    MSG_MOVE_FEATHER_FALL                           = 688,
    MSG_MOVE_WATER_WALK                             = 689,
    CMSG_SERVER_BROADCAST                           = 690,
    CMSG_SELF_RES                                   = 691, // only if PLAYER_SELF_RES_SPELL field != 0
    SMSG_FEIGN_DEATH_RESISTED                       = 692, // Resisted
    CMSG_RUN_SCRIPT                                 = 693,
    SMSG_SCRIPT_MESSAGE                             = 694,
    SMSG_DUEL_COUNTDOWN                             = 695,
    SMSG_AREA_TRIGGER_MESSAGE                       = 696,
    CMSG_TOGGLE_HELM                                = 697,
    CMSG_TOGGLE_CLOAK                               = 698,

    //SMSG_SPELL_REFLECTED                            = 699, //OBSOLETE
    SMSG_MEETINGSTONE_JOINFAILED                    = 699, // 1 - must be party leader, 3 - can't use while in raid
    SMSG_PLAYER_SKINNED                             = 700, // uint8 0x00 Insignia taken - You can only resurrect at the graveyard
    SMSG_DURABILITY_DAMAGE_DEATH                    = 701,
    CMSG_SET_EXPLORATION                            = 702,
    CMSG_SET_ACTIONBAR_TOGGLES                      = 703,
    UMSG_DELETE_GUILD_CHARTER                       = 704,
    MSG_PETITION_RENAME                             = 705,
    SMSG_INIT_WORLD_STATES                          = 706,
    SMSG_UPDATE_WORLD_STATE                         = 707,
    CMSG_ITEM_NAME_QUERY                            = 708,
    SMSG_ITEM_NAME_QUERY_RESPONSE                   = 709,
    SMSG_PET_ACTION_FEEDBACK                        = 710, // uint8 0x04 - unk, 0x01 - pet dead, 0x2 - no target, 0x3 can't attack
    CMSG_CHAR_RENAME                                = 711,
    SMSG_CHAR_RENAME                                = 712,
    CMSG_MOVE_SPLINE_DONE                           = 713,
    CMSG_MOVE_FALL_RESET                            = 714,
    SMSG_INSTANCE_SAVE_CREATED                      = 715, // You are now saved to this instance
    SMSG_RAID_INSTANCE_INFO                         = 716,
    CMSG_REQUEST_RAID_INFO                          = 717,
    CMSG_MOVE_TIME_SKIPPED                          = 718,
    CMSG_MOVE_FEATHER_FALL_ACK                      = 719,
    CMSG_MOVE_WATER_WALK_ACK                        = 720,
    CMSG_MOVE_NOT_ACTIVE_MOVER                      = 721,
    SMSG_PLAY_SOUND                                 = 722,
    CMSG_BATTLEFIELD_STATUS                         = 723,
    SMSG_BATTLEFIELD_STATUS                         = 724,
    CMSG_BATTLEFIELD_PORT                           = 725,
    MSG_INSPECT_HONOR_STATS                         = 726,
    CMSG_BATTLEMASTER_HELLO                         = 727,
    CMSG_MOVE_START_SWIM_CHEAT                      = 728,
    CMSG_MOVE_STOP_SWIM_CHEAT                       = 729,
    SMSG_FORCE_WALK_SPEED_CHANGE                    = 730,
    CMSG_FORCE_WALK_SPEED_CHANGE_ACK                = 731,
    SMSG_FORCE_SWIM_BACK_SPEED_CHANGE               = 732,
    CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK           = 733,
    SMSG_FORCE_TURN_RATE_CHANGE                     = 734,
    CMSG_FORCE_TURN_RATE_CHANGE_ACK                 = 735,
    MSG_PVP_LOG_DATA                                = 736,
    CMSG_LEAVE_BATTLEFIELD                          = 737,
    CMSG_AREA_SPIRIT_HEALER_QUERY                   = 738,
    CMSG_AREA_SPIRIT_HEALER_QUEUE                   = 739,
    SMSG_AREA_SPIRIT_HEALER_TIME                    = 740, // uint64 guid+uint32 time
    CMSG_GM_UNTEACH                                 = 741,
    SMSG_WARDEN_DATA                                = 742,
    CMSG_WARDEN_DATA                                = 743,
    SMSG_GROUP_JOINED_BATTLEGROUND                  = 744, // uint32 0xfffffffc, probably different error messages...
    MSG_BATTLEGROUND_PLAYER_POSITIONS               = 745,
    CMSG_PET_STOP_ATTACK                            = 746,
    SMSG_BINDER_CONFIRM                             = 747,
    SMSG_BATTLEGROUND_PLAYER_JOINED                 = 748,
    SMSG_BATTLEGROUND_PLAYER_LEFT                   = 749,
    CMSG_BATTLEMASTER_JOIN                          = 750,
    SMSG_ADDON_INFO                                 = 751,
    CMSG_PET_UNLEARN                                = 752,
    SMSG_PET_UNLEARN_CONFIRM                        = 753,
    SMSG_PARTY_MEMBER_STATS_FULL                    = 754,
    CMSG_PET_SPELL_AUTOCAST                         = 755,
    SMSG_WEATHER                                    = 756,
    SMSG_PLAY_TIME_WARNING                          = 757,
    SMSG_MINIGAME_SETUP                             = 758,
    SMSG_MINIGAME_STATE                             = 759,
    CMSG_MINIGAME_MOVE                              = 760,
    SMSG_MINIGAME_MOVE_FAILED                       = 761,
    SMSG_INSTANCE_RESET_SCHEDULED                   = 762, // WARNING! %s is scheduled to reset in %u minutes! and other messages...
    SMSG_COMPRESSED_MOVE                            = 763,
    CMSG_GUILD_CHANGEINFO                           = 764,
    SMSG_UNKNOWN_765                                = 765, // Trial accounts can not send unlimited tells, you must wait before you can send tells to more players.
    SMSG_SET_MOVE_SPEED                             = 766, // GUID + float speed, move speed, except swim/turn/fly
    SMSG_SET_RUN_BACK_SPEED                         = 767, // GUID + float speed, run back speed
    SMSG_SET_SWIM_SPEED                             = 768, // GUID + float speed, swim and swim back speed
    // 769
    SMSG_SET_SWIM_BACK_SPEED                        = 770, // swim back speed
    SMSG_SET_TURN_RATE                              = 771, // turn rate (note: client crashes if fly mode enabled, and turn rate = 0)
    SMSG_UNKNOWN_772                                = 772, // packed GUID
    SMSG_UNKNOWN_773                                = 773, // set movement flag 0x20000000
    SMSG_UNKNOWN_774                                = 774, // stop effect of 773 opcode
    SMSG_UNKNOWN_775                                = 775, // movement related, looks like hover, movement flag 0x40000000, we can't jump if we are lands
    SMSG_UNKNOWN_776                                = 776, // stop effect of 775 opcode
    SMSG_MOVE_SET_WATERWALK                         = 777, // packed GUID, set movement flag 0x10000000, waterwalking...
    SMSG_MOVE_STOP_WATERWALK                        = 778, // packed GUID, stop effect of 777 opcode
    SMSG_UNKNOWN_779                                = 779, // packed guid, change animation to swim/fly like
    SMSG_UNKNOWN_780                                = 780, // packed guid
    SMSG_MOVE_STOP_WALK                             = 781, // packed guid, remove 0x100 movement flag (walk)
    SMSG_MOVE_START_WALK                            = 782, // set 0x100 movement flag (walk)
    // 783
    // 784
    // 785
    CMSG_ACTIVATETAXI_FAR                           = 786,
    // 787 causes client crash
    // 788
    // 789
    // 790
    CMSG_FIELD_WATCHED_FACTION_INACTIVE             = 791,
    CMSG_FIELD_WATCHED_FACTION_SHOW_BAR             = 792,
    SMSG_UNKNOWN_793                                = 793, // packed guid + uint32
    SMSG_UNKNOWN_794                                = 794, // packed guid, movement related, set 0x1000 movement flag, all speed to 0, except turn rate
    // 795
    SMSG_UNKNOWN_796                                = 796, // uint64, guid?
    CMSG_RESET_INSTANCES                            = 797, // reset instances, empty
    SMSG_RESET_INSTANCES_RESULT                     = 798, // uint32 mapid, chat message: %s has been reset.
    // 799
    SMSG_UNKNOWN_800                                = 800, // uint32 mapid, instance related (save?)
    MSG_RAID_ICON_TARGET                            = 801, // uint8+uint8+uint64 guid, or only one uint8
    MSG_RAID_READY_CHECK                            = 802, // uint64+uint8
    // 803
    SMSG_AI_UNKNOWN                                 = 804, // GUID + uint32, looks like SMSG_AI_REACTION (pet action sound?)
    SMSG_UNKNOWN_805                                = 805, // uint32 unk + x, y, z (pet dismiss sound?)
    // 806
    // 807
    SMSG_GM_SURVEY_REQUEST                          = 808, // uint32, 1 - causes client get ticket request, 2 - hide, 3 - show
    MSG_SET_DUNGEON_DIFFICULTY                      = 809, // uint32+uint32+uint32
    CMSG_GM_SURVEY_RESULTS                          = 810, // script function named GMSurveySubmit()
    SMSG_UNKNOWN_811                                = 811, // uint32, 0x0, SMSG_INSTANCE_RESET_ACTIVATE ?
    // 812
    // 813
    // 814
    SMSG_UNKNOWN_815                                = 815, // spell related, uint64 guid + spellid
    SMSG_UNKNOWN_816                                = 816, // spell related, uint64 guid + spellid + uint32 unk + uint64 guid (target?)
    // 817
    SMSG_UNKNOWN_818                                = 818, // 2.0.8, received before server MOTD, strange regexp sequence, looks like anti spam filter for chat messages...
    // 819
    // 820
    // 821
    // 822
    // 823
    // 824
    // 825
    SMSG_OUTDOORPVP_NOTIFY                          = 826, // looks like chat packets
    SMSG_OUTDOORPVP_NOTIFY2                         = 827, // may be it's changed to 826?
    // 828
    SMSG_MOTD                                       = 829, // server MOTD message, uint32 + message
    // 830
    // 831
    // 832
    SMSG_UNKNOWN_833                                = 833, // teleport
    SMSG_UNKNOWN_834                                = 834, // teleport
    SMSG_FLY_MODE_START                             = 835, // packed guid + uint32, start fly
    SMSG_FLY_MODE_STOP                              = 836, // packed guid + uint32, stop fly
    CMSG_MOVE_FLY_MODE_CHANGE_ACK                   = 837, // movement related, fly on/off ack
    CMSG_MOVE_FLY_STATE_CHANGE                      = 838, // movement related, fly start/stop(land) ack, may be MSG
    CMSG_SOCKET_ITEM                                = 839, // click on "Socket Gems" button in Jewelcrafting UI, contains uint64 item guid + 3 x uint64 gems guid's
    // 840, CMSG?
    SMSG_ARENA_TEAM_COMMAND_RESULT                  = 841, // uint32 command, name1, name2, uint32 errorcode
    // 842
    CMSG_ARENA_TEAM_QUERY                           = 843,
    SMSG_ARENA_TEAM_QUERY_RESPONSE                  = 844,
    CMSG_ARENA_TEAM_ROSTER                          = 845,
    SMSG_ARENA_TEAM_ROSTER                          = 846,
    CMSG_ARENA_TEAM_ADD_MEMBER                      = 847,
    SMSG_ARENA_TEAM_INVITE                          = 848,
    CMSG_ARENA_TEAM_INVITE_ACCEPT                   = 849,
    CMSG_ARENA_TEAM_INVITE_DECLINE                  = 850,
    CMSG_ARENA_TEAM_LEAVE                           = 851,
    CMSG_ARENA_TEAM_REMOVE_FROM_TEAM                = 852,
    CMSG_ARENA_TEAM_DISBAND                         = 853,
    CMSG_ARENA_TEAM_PROMOTE_TO_CAPTAIN              = 854, // also must be demote opcode...
    SMSG_UNKNOWN_855                                = 855, // guild related...
    CMSG_ARENAMASTER_JOIN                           = 856,
    MSG_MOVE_START_FLY_UP                           = 857, // movement related, fly up, possible MSG
    MSG_MOVE_STOP_FLY_UP                            = 858, // movement related, stop fly up, possible MSG
    SMSG_ARENA_TEAM_STATS                           = 859,
    CMSG_LFG_SET_AUTOJOIN                           = 860,
    CMSG_LFG_UNSET_AUTOJOIN                         = 861,
    CMSG_LFM_SET_AUTOADD                            = 862,
    CMSG_LFM_UNSET_AUTOADD                          = 863,
    CMSG_LFG_INVITE_ACCEPT                          = 864,
    CMSG_LFG_INVITE_CANCEL                          = 865,
    // 866, CMSG?
    CMSG_LOOKING_FOR_GROUP_CLEAR                    = 867,
    CMSG_SET_LOOKING_FOR_NONE                       = 868,
    CMSG_SET_LOOKING_FOR_MORE                       = 869,
    CMSG_SET_COMMENTARY                             = 870,
    SMSG_LFG_871                                    = 871, // Matchmaking timed out.
    SMSG_LFG_872                                    = 872, // Matchmaking timed out waiting for other player.
    SMSG_LFG_873                                    = 873, // Group no longer available.
    SMSG_LFG_874                                    = 974, // Matched Player(s) have gone offline.
    // 875
    SMSG_LFG_876                                    = 876, // LFM eye, in progress, uint16+uint32+uint8?, 3 x uint8(0x0)
    SMSG_LFG_877                                    = 877, // cause client send CMSG_SET_LOOKING_FOR_GROUP, uint32+uint8?
    SMSG_LFG_878                                    = 878, // cause client send CMSG_SET_LOOKING_FOR_GROUP
    SMSG_LFG_879                                    = 879, // LFG eye, cause client send CMSG_SET_LOOKING_FOR_GROUP
    SMSG_LFG_INVITE                                 = 880, // show invite dialog: The LFG system has matched you to a group for %s. 
    SMSG_LFG_881                                    = 881, // The LFG system is waiting to complete match for %s.
    // 882
    SMSG_CHANGE_TITLE                               = 883, // uint32 title_id, causes message and visual effect
    CMSG_CHOOSE_TITLE                               = 884, // uint32 title_id
    CMSG_DISMOUNT                                   = 885, // /dismount command
    SMSG_ARENA_TEAM_UNK                             = 886,
    MSG_INSPECT_ARENA_STATS                         = 887,
    SMSG_SH_POSITION                                = 888, // spirit healer position, map/x/y/z, at player death...
    CMSG_CANCEL_TEMP_ITEM_ENCHANTMENT               = 889, // cancel temporary item enchantment
    // 890
    // 891
    // 892
    // 893
    SMSG_MOVE_SET_FLY_SPEED                         = 894, // all fly speed, packed guid, uint32 movement_flags, time, x,y,z,o,unk,speed...
    // 895
    SMSG_MOVE_SET_FLY_BACK_SPEED                    = 896,
    SMSG_FORCE_FLY_SPEED_CHANGE                     = 897, // packed guid, uint32, speed
    CMSG_FORCE_FLY_SPEED_CHANGE_ACK                 = 898,
    SMSG_FORCE_FLY_BACK_SPEED_CHANGE                = 899,
    CMSG_FORCE_FLY_BACK_SPEED_CHANGE_ACK            = 900,
    SMSG_MOVE_SET_FLY_SPEED2                        = 901, // same as 894, strange, packed guid+speed
    SMSG_MOVE_SET_FLY_BACK_SPEED2                   = 902, // same as 896, strange,
    // 903
    // 904 SMSG_FLIGHT_SPLINE_SYNC?
    // 905
    // 906
    SMSG_REALM_STATE_RESPONSE                       = 907, // response to 908 opcode, 4 x uint32 + uint8
    CMSG_REALM_STATE_REQUEST                        = 908, // realm related, uint32+uint32+3*(uint16+uint8), appears at select character screen, uint32 0xFFFFFFFF
    CMSG_MOVE_SHIP_909                              = 909, // movement related, transport related(ships)
    CMSG_GROUP_PROMOTE                              = 910, // make main-tank / main-assistant
    // 911
    SMSG_ALLOW_MOVE                                 = 912, // uint32, allow player movement, value increments every time and reset to 0 after far teleport, used for client-server synchronization
    CMSG_ALLOW_MOVE_ACK                             = 913, // client response to SMSG_ALLOW_MOVE
    // 914
    // 915
    // 916
    // 917
    SMSG_UNKNOWN_918                                = 918, // chat message: The party leader has attempted to reset the instance you are in. Please zone out to allow the instance to reset.
    SMSG_UNKNOWN_919                                = 919, // uint8(0)+uint32(0/1)+uint64(unk/guid)
    SMSG_UNKNOWN_920                                = 920, // notify message: This system is currently disabled.
};

//if you add new opcode .. Do NOT forget to change the following define MAX_OPCODE_ID and also add new opcode to table in opcodes.cpp
#define MAX_OPCODE_ID 920

/// Results of friend related commands
enum FriendsResult
{
    FRIEND_DB_ERROR                               = 0x00,
    FRIEND_LIST_FULL                              = 0x01,
    FRIEND_ONLINE                                 = 0x02,
    FRIEND_OFFLINE                                = 0x03,
    FRIEND_NOT_FOUND                              = 0x04,
    FRIEND_REMOVED                                = 0x05,
    FRIEND_ADDED_ONLINE                           = 0x06,
    FRIEND_ADDED_OFFLINE                          = 0x07,
    FRIEND_ALREADY                                = 0x08,
    FRIEND_SELF                                   = 0x09,
    FRIEND_ENEMY                                  = 0x0A,
    FRIEND_IGNORE_FULL                            = 0x0B,
    FRIEND_IGNORE_SELF                            = 0x0C,
    FRIEND_IGNORE_NOT_FOUND                       = 0x0D,
    FRIEND_IGNORE_ALREADY                         = 0x0E,
    FRIEND_IGNORE_ADDED                           = 0x0F,
    FRIEND_IGNORE_REMOVED                         = 0x10
};

/// Non Player Character flags
enum NPCFlags
{
    UNIT_NPC_FLAG_NONE              = 0,
    UNIT_NPC_FLAG_GOSSIP            = 1,
    UNIT_NPC_FLAG_QUESTGIVER        = 2,
    UNIT_NPC_FLAG_VENDOR            = 4,
    UNIT_NPC_FLAG_TAXIVENDOR        = 8,
    UNIT_NPC_FLAG_TRAINER           = 16,
    UNIT_NPC_FLAG_SPIRITHEALER      = 32,
    UNIT_NPC_FLAG_GUARD             = 64,                   //UQ1: ???  We can use as guard flag?, used by blizz for BattleGround spirit guides... (entry 13116 and 13117)
    UNIT_NPC_FLAG_INNKEEPER         = 128,
    UNIT_NPC_FLAG_BANKER            = 256,
    UNIT_NPC_FLAG_PETITIONER        = 512,                  // 1024+512 = guild petitions, 512 = arena team petitions
    UNIT_NPC_FLAG_TABARDVENDOR      = 1024,
    UNIT_NPC_FLAG_BATTLEFIELDPERSON = 2048,
    UNIT_NPC_FLAG_AUCTIONEER        = 4096,
    UNIT_NPC_FLAG_STABLE            = 8192,
    UNIT_NPC_FLAG_ARMORER           = 16384,
};
#endif
/// @}
