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

/** \file
    \ingroup u2w
*/

#include "Opcodes.h"
#include "NameTables.h"

/// Correspondance between opcodes and their names
// this is an string array, for more opcode comments look to opcodes.h
// do not change order of lines!
// when renaming opcodes, replace old name by the new one
const char* g_worldOpcodeNames[] =
{

    "MSG_NULL_ACTION",                                     //MSG_NULL_ACTION
    "CMSG_BOOTME",                                         //CMSG_BOOTME
    "CMSG_DBLOOKUP",                                       //CMSG_DBLOOKUP
    "SMSG_DBLOOKUP",                                       //SMSG_DBLOOKUP
    "CMSG_QUERY_OBJECT_POSITION",                          //CMSG_QUERY_OBJECT_POSITION
    "SMSG_QUERY_OBJECT_POSITION",                          //SMSG_QUERY_OBJECT_POSITION
    "CMSG_QUERY_OBJECT_ROTATION",                          //CMSG_QUERY_OBJECT_ROTATION
    "SMSG_QUERY_OBJECT_ROTATION",                          //SMSG_QUERY_OBJECT_ROTATION
    "CMSG_WORLD_TELEPORT",                                 //CMSG_WORLD_TELEPORT
    "CMSG_TELEPORT_TO_UNIT",                               //CMSG_TELEPORT_TO_UNIT
    "CMSG_ZONE_MAP",                                       //CMSG_ZONE_MAP
    "SMSG_ZONE_MAP",                                       //SMSG_ZONE_MAP
    "CMSG_DEBUG_CHANGECELLZONE",                           //CMSG_DEBUG_CHANGECELLZONE
    "CMSG_EMBLAZON_TABARD_OBSOLETE",                       //CMSG_EMBLAZON_TABARD_OBSOLETE
    "CMSG_UNEMBLAZON_TABARD_OBSOLETE",                     //CMSG_UNEMBLAZON_TABARD_OBSOLETE
    "CMSG_RECHARGE",                                       //CMSG_RECHARGE
    "CMSG_LEARN_SPELL",                                    //CMSG_LEARN_SPELL
    "CMSG_CREATEMONSTER",                                  //CMSG_CREATEMONSTER
    "CMSG_DESTROYMONSTER",                                 //CMSG_DESTROYMONSTER
    "CMSG_CREATEITEM",                                     //CMSG_CREATEITEM
    "CMSG_CREATEGAMEOBJECT",                               //CMSG_CREATEGAMEOBJECT
    "CMSG_MAKEMONSTERATTACKME_OBSOLETE",                   //CMSG_MAKEMONSTERATTACKME_OBSOLETE
    "CMSG_MAKEMONSTERATTACKGUID",                          //CMSG_MAKEMONSTERATTACKGUID
    "CMSG_ENABLEDEBUGCOMBATLOGGING_OBSOLETE",              //CMSG_ENABLEDEBUGCOMBATLOGGING_OBSOLETE
    "CMSG_FORCEACTION",                                    //CMSG_FORCEACTION
    "CMSG_FORCEACTIONONOTHER",                             //CMSG_FORCEACTIONONOTHER
    "CMSG_FORCEACTIONSHOW",                                //CMSG_FORCEACTIONSHOW
    "SMSG_FORCEACTIONSHOW",                                //SMSG_FORCEACTIONSHOW
    "SMSG_ATTACKERSTATEUPDATEDEBUGINFO_OBSOLETE",          //SMSG_ATTACKERSTATEUPDATEDEBUGINFO_OBSOLETE
    "SMSG_DEBUGINFOSPELL_OBSOLETE",                        //SMSG_DEBUGINFOSPELL_OBSOLETE
    "SMSG_DEBUGINFOSPELLMISS_OBSOLETE",                    //SMSG_DEBUGINFOSPELLMISS_OBSOLETE
    "SMSG_DEBUG_PLAYER_RANGE_OBSOLETE",                    //SMSG_DEBUG_PLAYER_RANGE_OBSOLETE
    "CMSG_UNDRESSPLAYER",                                  //CMSG_UNDRESSPLAYER
    "CMSG_BEASTMASTER",                                    //CMSG_BEASTMASTER
    "CMSG_GODMODE",                                        //CMSG_GODMODE
    "SMSG_GODMODE",                                        //SMSG_GODMODE
    "CMSG_CHEAT_SETMONEY",                                 //CMSG_CHEAT_SETMONEY
    "CMSG_LEVEL_CHEAT",                                    //CMSG_LEVEL_CHEAT
    "CMSG_PET_LEVEL_CHEAT",                                //CMSG_PET_LEVEL_CHEAT
    "CMSG_LEVELUP_CHEAT_OBSOLETE",                         //CMSG_LEVELUP_CHEAT_OBSOLETE
    "CMSG_COOLDOWN_CHEAT",                                 //CMSG_COOLDOWN_CHEAT
    "CMSG_USE_SKILL_CHEAT",                                //CMSG_USE_SKILL_CHEAT
    "CMSG_FLAG_QUEST",                                     //CMSG_FLAG_QUEST
    "CMSG_FLAG_QUEST_FINISH",                              //CMSG_FLAG_QUEST_FINISH
    "CMSG_CLEAR_QUEST",                                    //CMSG_CLEAR_QUEST
    "CMSG_SEND_EVENT",                                     //CMSG_SEND_EVENT
    "CMSG_DEBUG_AISTATE",                                  //CMSG_DEBUG_AISTATE
    "SMSG_DEBUG_AISTATE",                                  //SMSG_DEBUG_AISTATE
    "CMSG_DISABLE_PVP_CHEAT",                              //CMSG_DISABLE_PVP_CHEAT
    "CMSG_ADVANCE_SPAWN_TIME",                             //CMSG_ADVANCE_SPAWN_TIME
    "CMSG_PVP_PORT_OBSOLETE",                              //CMSG_PVP_PORT_OBSOLETE
    "CMSG_AUTH_SRP6_BEGIN",                                //CMSG_AUTH_SRP6_BEGIN
    "CMSG_AUTH_SRP6_PROOF",                                //CMSG_AUTH_SRP6_PROOF
    "CMSG_AUTH_SRP6_RECODE",                               //CMSG_AUTH_SRP6_RECODE
    "CMSG_CHAR_CREATE",                                    //CMSG_CHAR_CREATE
    "CMSG_CHAR_ENUM",                                      //CMSG_CHAR_ENUM
    "CMSG_CHAR_DELETE",                                    //CMSG_CHAR_DELETE
    "SMSG_AUTH_SRP6_RESPONSE",                             //SMSG_AUTH_SRP6_RESPONSE
    "SMSG_CHAR_CREATE",                                    //SMSG_CHAR_CREATE
    "SMSG_CHAR_ENUM",                                      //SMSG_CHAR_ENUM
    "SMSG_CHAR_DELETE",                                    //SMSG_CHAR_DELETE
    "CMSG_PLAYER_LOGIN",                                   //CMSG_PLAYER_LOGIN
    "SMSG_NEW_WORLD",                                      //SMSG_NEW_WORLD
    "SMSG_TRANSFER_PENDING",                               //SMSG_TRANSFER_PENDING
    "SMSG_TRANSFER_ABORTED",                               //SMSG_TRANSFER_ABORTED
    "SMSG_CHARACTER_LOGIN_FAILED",                         //SMSG_CHARACTER_LOGIN_FAILED
    "SMSG_LOGIN_SETTIMESPEED",                             //SMSG_LOGIN_SETTIMESPEED
    "SMSG_GAMETIME_UPDATE",                                //SMSG_GAMETIME_UPDATE
    "CMSG_GAMETIME_SET",                                   //CMSG_GAMETIME_SET
    "SMSG_GAMETIME_SET",                                   //SMSG_GAMETIME_SET
    "CMSG_GAMESPEED_SET",                                  //CMSG_GAMESPEED_SET
    "SMSG_GAMESPEED_SET",                                  //SMSG_GAMESPEED_SET
    "CMSG_SERVERTIME",                                     //CMSG_SERVERTIME
    "SMSG_SERVERTIME",                                     //SMSG_SERVERTIME
    "CMSG_PLAYER_LOGOUT",                                  //CMSG_PLAYER_LOGOUT
    "CMSG_LOGOUT_REQUEST",                                 //CMSG_LOGOUT_REQUEST
    "SMSG_LOGOUT_RESPONSE",                                //SMSG_LOGOUT_RESPONSE
    "SMSG_LOGOUT_COMPLETE",                                //SMSG_LOGOUT_COMPLETE
    "CMSG_LOGOUT_CANCEL",                                  //CMSG_LOGOUT_CANCEL
    "SMSG_LOGOUT_CANCEL_ACK",                              //SMSG_LOGOUT_CANCEL_ACK
    "CMSG_NAME_QUERY",                                     //CMSG_NAME_QUERY
    "SMSG_NAME_QUERY_RESPONSE",                            //SMSG_NAME_QUERY_RESPONSE
    "CMSG_PET_NAME_QUERY",                                 //CMSG_PET_NAME_QUERY
    "SMSG_PET_NAME_QUERY_RESPONSE",                        //SMSG_PET_NAME_QUERY_RESPONSE
    "CMSG_GUILD_QUERY",                                    //CMSG_GUILD_QUERY
    "SMSG_GUILD_QUERY_RESPONSE",                           //SMSG_GUILD_QUERY_RESPONSE
    "CMSG_ITEM_QUERY_SINGLE",                              //CMSG_ITEM_QUERY_SINGLE
    "CMSG_ITEM_QUERY_MULTIPLE",                            //CMSG_ITEM_QUERY_MULTIPLE
    "SMSG_ITEM_QUERY_SINGLE_RESPONSE",                     //SMSG_ITEM_QUERY_SINGLE_RESPONSE
    "SMSG_ITEM_QUERY_MULTIPLE_RESPONSE",                   //SMSG_ITEM_QUERY_MULTIPLE_RESPONSE
    "CMSG_PAGE_TEXT_QUERY",                                //CMSG_PAGE_TEXT_QUERY
    "SMSG_PAGE_TEXT_QUERY_RESPONSE",                       //SMSG_PAGE_TEXT_QUERY_RESPONSE
    "CMSG_QUEST_QUERY",                                    //CMSG_QUEST_QUERY
    "SMSG_QUEST_QUERY_RESPONSE",                           //SMSG_QUEST_QUERY_RESPONSE
    "CMSG_GAMEOBJECT_QUERY",                               //CMSG_GAMEOBJECT_QUERY
    "SMSG_GAMEOBJECT_QUERY_RESPONSE",                      //SMSG_GAMEOBJECT_QUERY_RESPONSE
    "CMSG_CREATURE_QUERY",                                 //CMSG_CREATURE_QUERY
    "SMSG_CREATURE_QUERY_RESPONSE",                        //SMSG_CREATURE_QUERY_RESPONSE
    "CMSG_WHO",                                            //CMSG_WHO
    "SMSG_WHO",                                            //SMSG_WHO
    "CMSG_WHOIS",                                          //CMSG_WHOIS
    "SMSG_WHOIS",                                          //SMSG_WHOIS
    "CMSG_FRIEND_LIST",                                    //CMSG_FRIEND_LIST
    "SMSG_FRIEND_LIST",                                    //SMSG_FRIEND_LIST
    "SMSG_FRIEND_STATUS",                                  //SMSG_FRIEND_STATUS
    "CMSG_ADD_FRIEND",                                     //CMSG_ADD_FRIEND
    "CMSG_DEL_FRIEND",                                     //CMSG_DEL_FRIEND
    "SMSG_IGNORE_LIST",                                    //SMSG_IGNORE_LIST
    "CMSG_ADD_IGNORE",                                     //CMSG_ADD_IGNORE
    "CMSG_DEL_IGNORE",                                     //CMSG_DEL_IGNORE
    "CMSG_GROUP_INVITE",                                   //CMSG_GROUP_INVITE
    "SMSG_GROUP_INVITE",                                   //SMSG_GROUP_INVITE
    "CMSG_GROUP_CANCEL",                                   //CMSG_GROUP_CANCEL
    "SMSG_GROUP_CANCEL",                                   //SMSG_GROUP_CANCEL
    "CMSG_GROUP_ACCEPT",                                   //CMSG_GROUP_ACCEPT
    "CMSG_GROUP_DECLINE",                                  //CMSG_GROUP_DECLINE
    "SMSG_GROUP_DECLINE",                                  //SMSG_GROUP_DECLINE
    "CMSG_GROUP_UNINVITE",                                 //CMSG_GROUP_UNINVITE
    "CMSG_GROUP_UNINVITE_GUID",                            //CMSG_GROUP_UNINVITE_GUID
    "SMSG_GROUP_UNINVITE",                                 //SMSG_GROUP_UNINVITE
    "CMSG_GROUP_SET_LEADER",                               //CMSG_GROUP_SET_LEADER
    "SMSG_GROUP_SET_LEADER",                               //SMSG_GROUP_SET_LEADER
    "CMSG_LOOT_METHOD",                                    //CMSG_LOOT_METHOD
    "CMSG_GROUP_DISBAND",                                  //CMSG_GROUP_DISBAND
    "SMSG_GROUP_DESTROYED",                                //SMSG_GROUP_DESTROYED
    "SMSG_GROUP_LIST",                                     //SMSG_GROUP_LIST
    "SMSG_PARTY_MEMBER_STATS",                             //SMSG_PARTY_MEMBER_STATS
    "SMSG_PARTY_COMMAND_RESULT",                           //SMSG_PARTY_COMMAND_RESULT
    "UMSG_UPDATE_GROUP_MEMBERS",                           //UMSG_UPDATE_GROUP_MEMBERS
    "CMSG_GUILD_CREATE",                                   //CMSG_GUILD_CREATE
    "CMSG_GUILD_INVITE",                                   //CMSG_GUILD_INVITE
    "SMSG_GUILD_INVITE",                                   //SMSG_GUILD_INVITE
    "CMSG_GUILD_ACCEPT",                                   //CMSG_GUILD_ACCEPT
    "CMSG_GUILD_DECLINE",                                  //CMSG_GUILD_DECLINE
    "SMSG_GUILD_DECLINE",                                  //SMSG_GUILD_DECLINE
    "CMSG_GUILD_INFO",                                     //CMSG_GUILD_INFO
    "SMSG_GUILD_INFO",                                     //SMSG_GUILD_INFO
    "CMSG_GUILD_ROSTER",                                   //CMSG_GUILD_ROSTER
    "SMSG_GUILD_ROSTER",                                   //SMSG_GUILD_ROSTER
    "CMSG_GUILD_PROMOTE",                                  //CMSG_GUILD_PROMOTE
    "CMSG_GUILD_DEMOTE",                                   //CMSG_GUILD_DEMOTE
    "CMSG_GUILD_LEAVE",                                    //CMSG_GUILD_LEAVE
    "CMSG_GUILD_REMOVE",                                   //CMSG_GUILD_REMOVE
    "CMSG_GUILD_DISBAND",                                  //CMSG_GUILD_DISBAND
    "CMSG_GUILD_LEADER",                                   //CMSG_GUILD_LEADER
    "CMSG_GUILD_MOTD",                                     //CMSG_GUILD_MOTD
    "SMSG_GUILD_EVENT",                                    //SMSG_GUILD_EVENT
    "SMSG_GUILD_COMMAND_RESULT",                           //SMSG_GUILD_COMMAND_RESULT
    "UMSG_UPDATE_GUILD",                                   //UMSG_UPDATE_GUILD
    "CMSG_MESSAGECHAT",                                    //CMSG_MESSAGECHAT
    "SMSG_MESSAGECHAT",                                    //SMSG_MESSAGECHAT
    "CMSG_JOIN_CHANNEL",                                   //CMSG_JOIN_CHANNEL
    "CMSG_LEAVE_CHANNEL",                                  //CMSG_LEAVE_CHANNEL
    "SMSG_CHANNEL_NOTIFY",                                 //SMSG_CHANNEL_NOTIFY
    "CMSG_CHANNEL_LIST",                                   //CMSG_CHANNEL_LIST
    "SMSG_CHANNEL_LIST",                                   //SMSG_CHANNEL_LIST
    "CMSG_CHANNEL_PASSWORD",                               //CMSG_CHANNEL_PASSWORD
    "CMSG_CHANNEL_SET_OWNER",                              //CMSG_CHANNEL_SET_OWNER
    "CMSG_CHANNEL_OWNER",                                  //CMSG_CHANNEL_OWNER
    "CMSG_CHANNEL_MODERATOR",                              //CMSG_CHANNEL_MODERATOR
    "CMSG_CHANNEL_UNMODERATOR",                            //CMSG_CHANNEL_UNMODERATOR
    "CMSG_CHANNEL_MUTE",                                   //CMSG_CHANNEL_MUTE
    "CMSG_CHANNEL_UNMUTE",                                 //CMSG_CHANNEL_UNMUTE
    "CMSG_CHANNEL_INVITE",                                 //CMSG_CHANNEL_INVITE
    "CMSG_CHANNEL_KICK",                                   //CMSG_CHANNEL_KICK
    "CMSG_CHANNEL_BAN",                                    //CMSG_CHANNEL_BAN
    "CMSG_CHANNEL_UNBAN",                                  //CMSG_CHANNEL_UNBAN
    "CMSG_CHANNEL_ANNOUNCEMENTS",                          //CMSG_CHANNEL_ANNOUNCEMENTS
    "CMSG_CHANNEL_MODERATE",                               //CMSG_CHANNEL_MODERATE
    "SMSG_UPDATE_OBJECT",                                  //SMSG_UPDATE_OBJECT
    "SMSG_DESTROY_OBJECT",                                 //SMSG_DESTROY_OBJECT
    "CMSG_USE_ITEM",                                       //CMSG_USE_ITEM
    "CMSG_OPEN_ITEM",                                      //CMSG_OPEN_ITEM
    "CMSG_READ_ITEM",                                      //CMSG_READ_ITEM
    "SMSG_READ_ITEM_OK",                                   //SMSG_READ_ITEM_OK
    "SMSG_READ_ITEM_FAILED",                               //SMSG_READ_ITEM_FAILED
    "SMSG_ITEM_COOLDOWN",                                  //SMSG_ITEM_COOLDOWN
    "CMSG_GAMEOBJ_USE",                                    //CMSG_GAMEOBJ_USE
    "CMSG_GAMEOBJ_CHAIR_USE_OBSOLETE",                     //CMSG_GAMEOBJ_CHAIR_USE_OBSOLETE
    "SMSG_GAMEOBJECT_CUSTOM_ANIM",                         //SMSG_GAMEOBJECT_CUSTOM_ANIM
    "CMSG_AREATRIGGER",                                    //CMSG_AREATRIGGER
    "MSG_MOVE_START_FORWARD",                              //MSG_MOVE_START_FORWARD
    "MSG_MOVE_START_BACKWARD",                             //MSG_MOVE_START_BACKWARD
    "MSG_MOVE_STOP",                                       //MSG_MOVE_STOP
    "MSG_MOVE_START_STRAFE_LEFT",                          //MSG_MOVE_START_STRAFE_LEFT
    "MSG_MOVE_START_STRAFE_RIGHT",                         //MSG_MOVE_START_STRAFE_RIGHT
    "MSG_MOVE_STOP_STRAFE",                                //MSG_MOVE_STOP_STRAFE
    "MSG_MOVE_JUMP",                                       //MSG_MOVE_JUMP
    "MSG_MOVE_START_TURN_LEFT",                            //MSG_MOVE_START_TURN_LEFT
    "MSG_MOVE_START_TURN_RIGHT",                           //MSG_MOVE_START_TURN_RIGHT
    "MSG_MOVE_STOP_TURN",                                  //MSG_MOVE_STOP_TURN
    "MSG_MOVE_START_PITCH_UP",                             //MSG_MOVE_START_PITCH_UP
    "MSG_MOVE_START_PITCH_DOWN",                           //MSG_MOVE_START_PITCH_DOWN
    "MSG_MOVE_STOP_PITCH",                                 //MSG_MOVE_STOP_PITCH
    "MSG_MOVE_SET_RUN_MODE",                               //MSG_MOVE_SET_RUN_MODE
    "MSG_MOVE_SET_WALK_MODE",                              //MSG_MOVE_SET_WALK_MODE
    "MSG_MOVE_TOGGLE_LOGGING",                             //MSG_MOVE_TOGGLE_LOGGING
    "MSG_MOVE_TELEPORT",                                   //MSG_MOVE_TELEPORT
    "MSG_MOVE_TELEPORT_CHEAT",                             //MSG_MOVE_TELEPORT_CHEAT
    "MSG_MOVE_TELEPORT_ACK",                               //MSG_MOVE_TELEPORT_ACK
    "MSG_MOVE_TOGGLE_FALL_LOGGING",                        //MSG_MOVE_TOGGLE_FALL_LOGGING
    "MSG_MOVE_FALL_LAND",                                  //MSG_MOVE_FALL_LAND
    "MSG_MOVE_START_SWIM",                                 //MSG_MOVE_START_SWIM
    "MSG_MOVE_STOP_SWIM",                                  //MSG_MOVE_STOP_SWIM
    "MSG_MOVE_SET_RUN_SPEED_CHEAT",                        //MSG_MOVE_SET_RUN_SPEED_CHEAT
    "MSG_MOVE_SET_RUN_SPEED",                              //MSG_MOVE_SET_RUN_SPEED
    "MSG_MOVE_SET_RUN_BACK_SPEED_CHEAT",                   //MSG_MOVE_SET_RUN_BACK_SPEED_CHEAT
    "MSG_MOVE_SET_RUN_BACK_SPEED",                         //MSG_MOVE_SET_RUN_BACK_SPEED
    "MSG_MOVE_SET_WALK_SPEED_CHEAT",                       //MSG_MOVE_SET_WALK_SPEED_CHEAT
    "MSG_MOVE_SET_WALK_SPEED",                             //MSG_MOVE_SET_WALK_SPEED
    "MSG_MOVE_SET_SWIM_SPEED_CHEAT",                       //MSG_MOVE_SET_SWIM_SPEED_CHEAT
    "MSG_MOVE_SET_SWIM_SPEED",                             //MSG_MOVE_SET_SWIM_SPEED
    "MSG_MOVE_SET_SWIM_BACK_SPEED_CHEAT",                  //MSG_MOVE_SET_SWIM_BACK_SPEED_CHEAT
    "MSG_MOVE_SET_SWIM_BACK_SPEED",                        //MSG_MOVE_SET_SWIM_BACK_SPEED
    "MSG_MOVE_SET_ALL_SPEED_CHEAT",                        //MSG_MOVE_SET_ALL_SPEED_CHEAT
    "MSG_MOVE_SET_TURN_RATE_CHEAT",                        //MSG_MOVE_SET_TURN_RATE_CHEAT
    "MSG_MOVE_SET_TURN_RATE",                              //MSG_MOVE_SET_TURN_RATE
    "MSG_MOVE_TOGGLE_COLLISION_CHEAT",                     //MSG_MOVE_TOGGLE_COLLISION_CHEAT
    "MSG_MOVE_SET_FACING",                                 //MSG_MOVE_SET_FACING
    "MSG_MOVE_SET_PITCH",                                  //MSG_MOVE_SET_PITCH
    "MSG_MOVE_WORLDPORT_ACK",                              //MSG_MOVE_WORLDPORT_ACK
    "SMSG_MONSTER_MOVE",                                   //SMSG_MONSTER_MOVE
    "SMSG_MOVE_WATER_WALK",                                //SMSG_MOVE_WATER_WALK
    "SMSG_MOVE_LAND_WALK",                                 //SMSG_MOVE_LAND_WALK
    "MSG_MOVE_SET_RAW_POSITION_ACK",                       //MSG_MOVE_SET_RAW_POSITION_ACK
    "CMSG_MOVE_SET_RAW_POSITION",                          //CMSG_MOVE_SET_RAW_POSITION
    "SMSG_FORCE_RUN_SPEED_CHANGE",                         //SMSG_FORCE_RUN_SPEED_CHANGE
    "CMSG_FORCE_RUN_SPEED_CHANGE_ACK",                     //CMSG_FORCE_RUN_SPEED_CHANGE_ACK
    "SMSG_FORCE_RUN_BACK_SPEED_CHANGE",                    //SMSG_FORCE_RUN_BACK_SPEED_CHANGE
    "CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK",                //CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK
    "SMSG_FORCE_SWIM_SPEED_CHANGE",                        //SMSG_FORCE_SWIM_SPEED_CHANGE
    "CMSG_FORCE_SWIM_SPEED_CHANGE_ACK",                    //CMSG_FORCE_SWIM_SPEED_CHANGE_ACK
    "SMSG_FORCE_MOVE_ROOT",                                //SMSG_FORCE_MOVE_ROOT
    "CMSG_FORCE_MOVE_ROOT_ACK",                            //CMSG_FORCE_MOVE_ROOT_ACK
    "SMSG_FORCE_MOVE_UNROOT",                              //SMSG_FORCE_MOVE_UNROOT
    "CMSG_FORCE_MOVE_UNROOT_ACK",                          //CMSG_FORCE_MOVE_UNROOT_ACK
    "MSG_MOVE_ROOT",                                       //MSG_MOVE_ROOT
    "MSG_MOVE_UNROOT",                                     //MSG_MOVE_UNROOT
    "MSG_MOVE_HEARTBEAT",                                  //MSG_MOVE_HEARTBEAT
    "SMSG_MOVE_KNOCK_BACK",                                //SMSG_MOVE_KNOCK_BACK
    "CMSG_MOVE_KNOCK_BACK_ACK",                            //CMSG_MOVE_KNOCK_BACK_ACK
    "MSG_MOVE_KNOCK_BACK",                                 //MSG_MOVE_KNOCK_BACK
    "SMSG_MOVE_FEATHER_FALL",                              //SMSG_MOVE_FEATHER_FALL
    "SMSG_MOVE_NORMAL_FALL",                               //SMSG_MOVE_NORMAL_FALL
    "SMSG_MOVE_SET_HOVER",                                 //SMSG_MOVE_SET_HOVER
    "SMSG_MOVE_UNSET_HOVER",                               //SMSG_MOVE_UNSET_HOVER
    "CMSG_MOVE_HOVER_ACK",                                 //CMSG_MOVE_HOVER_ACK
    "MSG_MOVE_HOVER",                                      //MSG_MOVE_HOVER
    "CMSG_TRIGGER_CINEMATIC_CHEAT",                        //CMSG_TRIGGER_CINEMATIC_CHEAT
    "CMSG_OPENING_CINEMATIC",                              //CMSG_OPENING_CINEMATIC
    "SMSG_TRIGGER_CINEMATIC",                              //SMSG_TRIGGER_CINEMATIC
    "CMSG_NEXT_CINEMATIC_CAMERA",                          //CMSG_NEXT_CINEMATIC_CAMERA
    "CMSG_COMPLETE_CINEMATIC",                             //CMSG_COMPLETE_CINEMATIC
    "SMSG_TUTORIAL_FLAGS",                                 //SMSG_TUTORIAL_FLAGS
    "CMSG_TUTORIAL_FLAG",                                  //CMSG_TUTORIAL_FLAG
    "CMSG_TUTORIAL_CLEAR",                                 //CMSG_TUTORIAL_CLEAR
    "CMSG_TUTORIAL_RESET",                                 //CMSG_TUTORIAL_RESET
    "CMSG_STANDSTATECHANGE",                               //CMSG_STANDSTATECHANGE
    "CMSG_EMOTE",                                          //CMSG_EMOTE
    "SMSG_EMOTE",                                          //SMSG_EMOTE
    "CMSG_TEXT_EMOTE",                                     //CMSG_TEXT_EMOTE
    "SMSG_TEXT_EMOTE",                                     //SMSG_TEXT_EMOTE
    "CMSG_AUTOEQUIP_GROUND_ITEM",                          //CMSG_AUTOEQUIP_GROUND_ITEM
    "CMSG_AUTOSTORE_GROUND_ITEM",                          //CMSG_AUTOSTORE_GROUND_ITEM
    "CMSG_AUTOSTORE_LOOT_ITEM",                            //CMSG_AUTOSTORE_LOOT_ITEM
    "CMSG_STORE_LOOT_IN_SLOT",                             //CMSG_STORE_LOOT_IN_SLOT
    "CMSG_AUTOEQUIP_ITEM",                                 //CMSG_AUTOEQUIP_ITEM
    "CMSG_AUTOSTORE_BAG_ITEM",                             //CMSG_AUTOSTORE_BAG_ITEM
    "CMSG_SWAP_ITEM",                                      //CMSG_SWAP_ITEM
    "CMSG_SWAP_INV_ITEM",                                  //CMSG_SWAP_INV_ITEM
    "CMSG_SPLIT_ITEM",                                     //CMSG_SPLIT_ITEM
    "CMSG_PICKUP_ITEM",                                    //CMSG_PICKUP_ITEM
    "CMSG_DROP_ITEM",                                      //CMSG_DROP_ITEM
    "CMSG_DESTROYITEM",                                    //CMSG_DESTROYITEM
    "SMSG_INVENTORY_CHANGE_FAILURE",                       //SMSG_INVENTORY_CHANGE_FAILURE
    "SMSG_OPEN_CONTAINER",                                 //SMSG_OPEN_CONTAINER
    "CMSG_INSPECT",                                        //CMSG_INSPECT
    "SMSG_INSPECT",                                        //SMSG_INSPECT
    "CMSG_INITIATE_TRADE",                                 //CMSG_INITIATE_TRADE
    "CMSG_BEGIN_TRADE",                                    //CMSG_BEGIN_TRADE
    "CMSG_BUSY_TRADE",                                     //CMSG_BUSY_TRADE
    "CMSG_IGNORE_TRADE",                                   //CMSG_IGNORE_TRADE
    "CMSG_ACCEPT_TRADE",                                   //CMSG_ACCEPT_TRADE
    "CMSG_UNACCEPT_TRADE",                                 //CMSG_UNACCEPT_TRADE
    "CMSG_CANCEL_TRADE",                                   //CMSG_CANCEL_TRADE
    "CMSG_SET_TRADE_ITEM",                                 //CMSG_SET_TRADE_ITEM
    "CMSG_CLEAR_TRADE_ITEM",                               //CMSG_CLEAR_TRADE_ITEM
    "CMSG_SET_TRADE_GOLD",                                 //CMSG_SET_TRADE_GOLD
    "SMSG_TRADE_STATUS",                                   //SMSG_TRADE_STATUS
    "SMSG_TRADE_STATUS_EXTENDED",                          //SMSG_TRADE_STATUS_EXTENDED
    "SMSG_INITIALIZE_FACTIONS",                            //SMSG_INITIALIZE_FACTIONS
    "SMSG_SET_FACTION_VISIBLE",                            //SMSG_SET_FACTION_VISIBLE
    "SMSG_SET_FACTION_STANDING",                           //SMSG_SET_FACTION_STANDING
    "CMSG_SET_FACTION_ATWAR",                              //CMSG_SET_FACTION_ATWAR
    "CMSG_SET_FACTION_CHEAT",                              //CMSG_SET_FACTION_CHEAT
    "SMSG_SET_PROFICIENCY",                                //SMSG_SET_PROFICIENCY
    "CMSG_SET_ACTION_BUTTON",                              //CMSG_SET_ACTION_BUTTON
    "SMSG_ACTION_BUTTONS",                                 //SMSG_ACTION_BUTTONS
    "SMSG_INITIAL_SPELLS",                                 //SMSG_INITIAL_SPELLS
    "SMSG_LEARNED_SPELL",                                  //SMSG_LEARNED_SPELL
    "SMSG_SUPERCEDED_SPELL",                               //SMSG_SUPERCEDED_SPELL
    "CMSG_NEW_SPELL_SLOT",                                 //CMSG_NEW_SPELL_SLOT
    "CMSG_CAST_SPELL",                                     //CMSG_CAST_SPELL
    "CMSG_CANCEL_CAST",                                    //CMSG_CANCEL_CAST
    "SMSG_CAST_RESULT",                                    //SMSG_CAST_RESULT
    "SMSG_SPELL_START",                                    //SMSG_SPELL_START
    "SMSG_SPELL_GO",                                       //SMSG_SPELL_GO
    "SMSG_SPELL_FAILURE",                                  //SMSG_SPELL_FAILURE
    "SMSG_SPELL_COOLDOWN",                                 //SMSG_SPELL_COOLDOWN
    "SMSG_COOLDOWN_EVENT",                                 //SMSG_COOLDOWN_EVENT
    "CMSG_CANCEL_AURA",                                    //CMSG_CANCEL_AURA
    "SMSG_UPDATE_AURA_DURATION",                           //SMSG_UPDATE_AURA_DURATION
    "SMSG_PET_CAST_FAILED",                                //SMSG_PET_CAST_FAILED
    "MSG_CHANNEL_START",                                   //MSG_CHANNEL_START
    "MSG_CHANNEL_UPDATE",                                  //MSG_CHANNEL_UPDATE
    "CMSG_CANCEL_CHANNELLING",                             //CMSG_CANCEL_CHANNELLING
    "SMSG_AI_REACTION",                                    //SMSG_AI_REACTION
    "CMSG_SET_SELECTION",                                  //CMSG_SET_SELECTION
    "CMSG_SET_TARGET_OBSOLETE",                            //CMSG_SET_TARGET_OBSOLETE
    "CMSG_UNUSED",                                         //CMSG_UNUSED
    "CMSG_UNUSED2",                                        //CMSG_UNUSED2
    "CMSG_ATTACKSWING",                                    //CMSG_ATTACKSWING
    "CMSG_ATTACKSTOP",                                     //CMSG_ATTACKSTOP
    "SMSG_ATTACKSTART",                                    //SMSG_ATTACKSTART
    "SMSG_ATTACKSTOP",                                     //SMSG_ATTACKSTOP
    "SMSG_ATTACKSWING_NOTINRANGE",                         //SMSG_ATTACKSWING_NOTINRANGE
    "SMSG_ATTACKSWING_BADFACING",                          //SMSG_ATTACKSWING_BADFACING
    "SMSG_ATTACKSWING_NOTSTANDING",                        //SMSG_ATTACKSWING_NOTSTANDING
    "SMSG_ATTACKSWING_DEADTARGET",                         //SMSG_ATTACKSWING_DEADTARGET
    "SMSG_ATTACKSWING_CANT_ATTACK",                        //SMSG_ATTACKSWING_CANT_ATTACK
    "SMSG_ATTACKERSTATEUPDATE",                            //SMSG_ATTACKERSTATEUPDATE
    "SMSG_VICTIMSTATEUPDATE_OBSOLETE",                     //SMSG_VICTIMSTATEUPDATE_OBSOLETE
    "SMSG_DAMAGE_DONE_OBSOLETE",                           //SMSG_DAMAGE_DONE_OBSOLETE
    "SMSG_DAMAGE_TAKEN_OBSOLETE",                          //SMSG_DAMAGE_TAKEN_OBSOLETE
    "SMSG_CANCEL_COMBAT",                                  //SMSG_CANCEL_COMBAT
    "SMSG_PLAYER_COMBAT_XP_GAIN_OBSOLETE",                 //SMSG_PLAYER_COMBAT_XP_GAIN_OBSOLETE
    "SMSG_HEALSPELL_ON_PLAYER_OBSOLETE",                   //SMSG_HEALSPELL_ON_PLAYER_OBSOLETE
    "SMSG_HEALSPELL_ON_PLAYERS_PET_OBSOLETE",              //SMSG_HEALSPELL_ON_PLAYERS_PET_OBSOLETE
    "CMSG_SHEATHE_OBSOLETE",                               //CMSG_SHEATHE_OBSOLETE
    "CMSG_SAVE_PLAYER",                                    //CMSG_SAVE_PLAYER
    "CMSG_SETDEATHBINDPOINT",                              //CMSG_SETDEATHBINDPOINT
    "SMSG_BINDPOINTUPDATE",                                //SMSG_BINDPOINTUPDATE
    "CMSG_GETDEATHBINDZONE",                               //CMSG_GETDEATHBINDZONE
    "SMSG_BINDZONEREPLY",                                  //SMSG_BINDZONEREPLY
    "SMSG_PLAYERBOUND",                                    //SMSG_PLAYERBOUND
    "SMSG_DEATH_NOTIFY_OBSOLETE",                          //SMSG_DEATH_NOTIFY_OBSOLETE
    "CMSG_REPOP_REQUEST",                                  //CMSG_REPOP_REQUEST
    "SMSG_RESURRECT_REQUEST",                              //SMSG_RESURRECT_REQUEST
    "CMSG_RESURRECT_RESPONSE",                             //CMSG_RESURRECT_RESPONSE
    "CMSG_LOOT",                                           //CMSG_LOOT
    "CMSG_LOOT_MONEY",                                     //CMSG_LOOT_MONEY
    "CMSG_LOOT_RELEASE",                                   //CMSG_LOOT_RELEASE
    "SMSG_LOOT_RESPONSE",                                  //SMSG_LOOT_RESPONSE
    "SMSG_LOOT_RELEASE_RESPONSE",                          //SMSG_LOOT_RELEASE_RESPONSE
    "SMSG_LOOT_REMOVED",                                   //SMSG_LOOT_REMOVED
    "SMSG_LOOT_MONEY_NOTIFY",                              //SMSG_LOOT_MONEY_NOTIFY
    "SMSG_LOOT_ITEM_NOTIFY",                               //SMSG_LOOT_ITEM_NOTIFY
    "SMSG_LOOT_CLEAR_MONEY",                               //SMSG_LOOT_CLEAR_MONEY
    "SMSG_ITEM_PUSH_RESULT",                               //SMSG_ITEM_PUSH_RESULT
    "SMSG_DUEL_REQUESTED",                                 //SMSG_DUEL_REQUESTED
    "SMSG_DUEL_OUTOFBOUNDS",                               //SMSG_DUEL_OUTOFBOUNDS
    "SMSG_DUEL_INBOUNDS",                                  //SMSG_DUEL_INBOUNDS
    "SMSG_DUEL_COMPLETE",                                  //SMSG_DUEL_COMPLETE
    "SMSG_DUEL_WINNER",                                    //SMSG_DUEL_WINNER
    "CMSG_DUEL_ACCEPTED",                                  //CMSG_DUEL_ACCEPTED
    "CMSG_DUEL_CANCELLED",                                 //CMSG_DUEL_CANCELLED
    "SMSG_MOUNTRESULT",                                    //SMSG_MOUNTRESULT
    "SMSG_DISMOUNTRESULT",                                 //SMSG_DISMOUNTRESULT
    "SMSG_PUREMOUNT_CANCELLED_OBSOLETE",                   //SMSG_PUREMOUNT_CANCELLED_OBSOLETE
    "CMSG_MOUNTSPECIAL_ANIM",                              //CMSG_MOUNTSPECIAL_ANIM
    "SMSG_MOUNTSPECIAL_ANIM",                              //SMSG_MOUNTSPECIAL_ANIM
    "SMSG_PET_TAME_FAILURE",                               //SMSG_PET_TAME_FAILURE
    "CMSG_PET_SET_ACTION",                                 //CMSG_PET_SET_ACTION
    "CMSG_PET_ACTION",                                     //CMSG_PET_ACTION
    "CMSG_PET_ABANDON",                                    //CMSG_PET_ABANDON
    "CMSG_PET_RENAME",                                     //CMSG_PET_RENAME
    "SMSG_PET_NAME_INVALID",                               //SMSG_PET_NAME_INVALID
    "SMSG_PET_SPELLS",                                     //SMSG_PET_SPELLS
    "SMSG_PET_MODE",                                       //SMSG_PET_MODE
    "CMSG_GOSSIP_HELLO",                                   //CMSG_GOSSIP_HELLO
    "CMSG_GOSSIP_SELECT_OPTION",                           //CMSG_GOSSIP_SELECT_OPTION
    "SMSG_GOSSIP_MESSAGE",                                 //SMSG_GOSSIP_MESSAGE
    "SMSG_GOSSIP_COMPLETE",                                //SMSG_GOSSIP_COMPLETE
    "CMSG_NPC_TEXT_QUERY",                                 //CMSG_NPC_TEXT_QUERY
    "SMSG_NPC_TEXT_UPDATE",                                //SMSG_NPC_TEXT_UPDATE
    "SMSG_NPC_WONT_TALK",                                  //SMSG_NPC_WONT_TALK
    "CMSG_QUESTGIVER_STATUS_QUERY",                        //CMSG_QUESTGIVER_STATUS_QUERY
    "SMSG_QUESTGIVER_STATUS",                              //SMSG_QUESTGIVER_STATUS
    "CMSG_QUESTGIVER_HELLO",                               //CMSG_QUESTGIVER_HELLO
    "SMSG_QUESTGIVER_QUEST_LIST",                          //SMSG_QUESTGIVER_QUEST_LIST
    "CMSG_QUESTGIVER_QUERY_QUEST",                         //CMSG_QUESTGIVER_QUERY_QUEST
    "CMSG_QUESTGIVER_QUEST_AUTOLAUNCH",                    //CMSG_QUESTGIVER_QUEST_AUTOLAUNCH
    "SMSG_QUESTGIVER_QUEST_DETAILS",                       //SMSG_QUESTGIVER_QUEST_DETAILS
    "CMSG_QUESTGIVER_ACCEPT_QUEST",                        //CMSG_QUESTGIVER_ACCEPT_QUEST
    "CMSG_QUESTGIVER_COMPLETE_QUEST",                      //CMSG_QUESTGIVER_COMPLETE_QUEST
    "SMSG_QUESTGIVER_REQUEST_ITEMS",                       //SMSG_QUESTGIVER_REQUEST_ITEMS
    "CMSG_QUESTGIVER_REQUEST_REWARD",                      //CMSG_QUESTGIVER_REQUEST_REWARD
    "SMSG_QUESTGIVER_OFFER_REWARD",                        //SMSG_QUESTGIVER_OFFER_REWARD
    "CMSG_QUESTGIVER_CHOOSE_REWARD",                       //CMSG_QUESTGIVER_CHOOSE_REWARD
    "SMSG_QUESTGIVER_QUEST_INVALID",                       //SMSG_QUESTGIVER_QUEST_INVALID
    "CMSG_QUESTGIVER_CANCEL",                              //CMSG_QUESTGIVER_CANCEL
    "SMSG_QUESTGIVER_QUEST_COMPLETE",                      //SMSG_QUESTGIVER_QUEST_COMPLETE
    "SMSG_QUESTGIVER_QUEST_FAILED",                        //SMSG_QUESTGIVER_QUEST_FAILED
    "CMSG_QUESTLOG_SWAP_QUEST",                            //CMSG_QUESTLOG_SWAP_QUEST
    "CMSG_QUESTLOG_REMOVE_QUEST",                          //CMSG_QUESTLOG_REMOVE_QUEST
    "SMSG_QUESTLOG_FULL",                                  //SMSG_QUESTLOG_FULL
    "SMSG_QUESTUPDATE_FAILED",                             //SMSG_QUESTUPDATE_FAILED
    "SMSG_QUESTUPDATE_FAILEDTIMER",                        //SMSG_QUESTUPDATE_FAILEDTIMER
    "SMSG_QUESTUPDATE_COMPLETE",                           //SMSG_QUESTUPDATE_COMPLETE
    "SMSG_QUESTUPDATE_ADD_KILL",                           //SMSG_QUESTUPDATE_ADD_KILL
    "SMSG_QUESTUPDATE_ADD_ITEM",                           //SMSG_QUESTUPDATE_ADD_ITEM
    "CMSG_QUEST_CONFIRM_ACCEPT",                           //CMSG_QUEST_CONFIRM_ACCEPT
    "SMSG_QUEST_CONFIRM_ACCEPT",                           //SMSG_QUEST_CONFIRM_ACCEPT
    "CMSG_PUSHQUESTTOPARTY",                               //CMSG_PUSHQUESTTOPARTY
    "CMSG_LIST_INVENTORY",                                 //CMSG_LIST_INVENTORY
    "SMSG_LIST_INVENTORY",                                 //SMSG_LIST_INVENTORY
    "CMSG_SELL_ITEM",                                      //CMSG_SELL_ITEM
    "SMSG_SELL_ITEM",                                      //SMSG_SELL_ITEM
    "CMSG_BUY_ITEM",                                       //CMSG_BUY_ITEM
    "CMSG_BUY_ITEM_IN_SLOT",                               //CMSG_BUY_ITEM_IN_SLOT
    "SMSG_BUY_ITEM",                                       //SMSG_BUY_ITEM
    "SMSG_BUY_FAILED",                                     //SMSG_BUY_FAILED
    "CMSG_TAXICLEARALLNODES",                              //CMSG_TAXICLEARALLNODES
    "CMSG_TAXIENABLEALLNODES",                             //CMSG_TAXIENABLEALLNODES
    "CMSG_TAXISHOWNODES",                                  //CMSG_TAXISHOWNODES
    "SMSG_SHOWTAXINODES",                                  //SMSG_SHOWTAXINODES
    "CMSG_TAXINODE_STATUS_QUERY",                          //CMSG_TAXINODE_STATUS_QUERY
    "SMSG_TAXINODE_STATUS",                                //SMSG_TAXINODE_STATUS
    "CMSG_TAXIQUERYAVAILABLENODES",                        //CMSG_TAXIQUERYAVAILABLENODES
    "CMSG_ACTIVATETAXI",                                   //CMSG_ACTIVATETAXI
    "SMSG_ACTIVATETAXIREPLY",                              //SMSG_ACTIVATETAXIREPLY
    "SMSG_NEW_TAXI_PATH",                                  //SMSG_NEW_TAXI_PATH
    "CMSG_TRAINER_LIST",                                   //CMSG_TRAINER_LIST
    "SMSG_TRAINER_LIST",                                   //SMSG_TRAINER_LIST
    "CMSG_TRAINER_BUY_SPELL",                              //CMSG_TRAINER_BUY_SPELL
    "SMSG_TRAINER_BUY_SUCCEEDED",                          //SMSG_TRAINER_BUY_SUCCEEDED
    "SMSG_TRAINER_BUY_FAILED",                             //SMSG_TRAINER_BUY_FAILED
    "CMSG_BINDER_ACTIVATE",                                //CMSG_BINDER_ACTIVATE
    "SMSG_PLAYERBINDERROR",                                //SMSG_PLAYERBINDERROR
    "CMSG_BANKER_ACTIVATE",                                //CMSG_BANKER_ACTIVATE
    "SMSG_SHOW_BANK",                                      //SMSG_SHOW_BANK
    "CMSG_BUY_BANK_SLOT",                                  //CMSG_BUY_BANK_SLOT
    "SMSG_BUY_BANK_SLOT_RESULT",                           //SMSG_BUY_BANK_SLOT_RESULT
    "CMSG_PETITION_SHOWLIST",                              //CMSG_PETITION_SHOWLIST
    "SMSG_PETITION_SHOWLIST",                              //SMSG_PETITION_SHOWLIST
    "CMSG_PETITION_BUY",                                   //CMSG_PETITION_BUY
    "CMSG_PETITION_SHOW_SIGNATURES",                       //CMSG_PETITION_SHOW_SIGNATURES
    "SMSG_PETITION_SHOW_SIGNATURES",                       //SMSG_PETITION_SHOW_SIGNATURES
    "CMSG_PETITION_SIGN",                                  //CMSG_PETITION_SIGN
    "SMSG_PETITION_SIGN_RESULTS",                          //SMSG_PETITION_SIGN_RESULTS
    "MSG_PETITION_DECLINE",                                //MSG_PETITION_DECLINE
    "CMSG_OFFER_PETITION",                                 //CMSG_OFFER_PETITION
    "CMSG_TURN_IN_PETITION",                               //CMSG_TURN_IN_PETITION
    "SMSG_TURN_IN_PETITION_RESULTS",                       //SMSG_TURN_IN_PETITION_RESULTS
    "CMSG_PETITION_QUERY",                                 //CMSG_PETITION_QUERY
    "SMSG_PETITION_QUERY_RESPONSE",                        //SMSG_PETITION_QUERY_RESPONSE
    "SMSG_FISH_NOT_HOOKED",                                //SMSG_FISH_NOT_HOOKED
    "SMSG_FISH_ESCAPED",                                   //SMSG_FISH_ESCAPED
    "CMSG_BUG",                                            //CMSG_BUG
    "SMSG_NOTIFICATION",                                   //SMSG_NOTIFICATION
    "CMSG_PLAYED_TIME",                                    //CMSG_PLAYED_TIME
    "SMSG_PLAYED_TIME",                                    //SMSG_PLAYED_TIME
    "CMSG_QUERY_TIME",                                     //CMSG_QUERY_TIME
    "SMSG_QUERY_TIME_RESPONSE",                            //SMSG_QUERY_TIME_RESPONSE
    "SMSG_LOG_XPGAIN",                                     //SMSG_LOG_XPGAIN
    "MSG_SPLIT_MONEY",                                     //MSG_SPLIT_MONEY
    "CMSG_RECLAIM_CORPSE",                                 //CMSG_RECLAIM_CORPSE
    "CMSG_WRAP_ITEM",                                      //CMSG_WRAP_ITEM
    "SMSG_LEVELUP_INFO",                                   //SMSG_LEVELUP_INFO
    "MSG_MINIMAP_PING",                                    //MSG_MINIMAP_PING
    "SMSG_RESISTLOG",                                      //SMSG_RESISTLOG
    "SMSG_ENCHANTMENTLOG",                                 //SMSG_ENCHANTMENTLOG
    "CMSG_SET_SKILL_CHEAT",                                //CMSG_SET_SKILL_CHEAT
    "SMSG_START_MIRROR_TIMER",                             //SMSG_START_MIRROR_TIMER
    "SMSG_PAUSE_MIRROR_TIMER",                             //SMSG_PAUSE_MIRROR_TIMER
    "SMSG_STOP_MIRROR_TIMER",                              //SMSG_STOP_MIRROR_TIMER
    "CMSG_PING",                                           //CMSG_PING
    "SMSG_PONG",                                           //SMSG_PONG
    "SMSG_CLEAR_COOLDOWN",                                 //SMSG_CLEAR_COOLDOWN
    "SMSG_GAMEOBJECT_PAGETEXT",                            //SMSG_GAMEOBJECT_PAGETEXT
    "CMSG_SETSHEATHED",                                    //CMSG_SETSHEATHED
    "SMSG_COOLDOWN_CHEAT",                                 //SMSG_COOLDOWN_CHEAT
    "SMSG_SPELL_DELAYED",                                  //SMSG_SPELL_DELAYED
    "CMSG_PLAYER_MACRO_OBSOLETE",                          //CMSG_PLAYER_MACRO_OBSOLETE
    "SMSG_PLAYER_MACRO_OBSOLETE",                          //SMSG_PLAYER_MACRO_OBSOLETE
    "CMSG_GHOST",                                          //CMSG_GHOST
    "CMSG_GM_INVIS",                                       //CMSG_GM_INVIS
    "SMSG_INVALID_PROMOTION_CODE",                         //SMSG_INVALID_PROMOTION_CODE
    "MSG_GM_BIND_OTHER",                                   //MSG_GM_BIND_OTHER
    "MSG_GM_SUMMON",                                       //MSG_GM_SUMMON
    "SMSG_ITEM_TIME_UPDATE",                               //SMSG_ITEM_TIME_UPDATE
    "SMSG_ITEM_ENCHANT_TIME_UPDATE",                       //SMSG_ITEM_ENCHANT_TIME_UPDATE
    "SMSG_AUTH_CHALLENGE",                                 //SMSG_AUTH_CHALLENGE
    "CMSG_AUTH_SESSION",                                   //CMSG_AUTH_SESSION
    "SMSG_AUTH_RESPONSE",                                  //SMSG_AUTH_RESPONSE
    "MSG_GM_SHOWLABEL",                                    //MSG_GM_SHOWLABEL
    "MSG_ADD_DYNAMIC_TARGET_OBSOLETE",                     //MSG_ADD_DYNAMIC_TARGET_OBSOLETE
    "MSG_SAVE_GUILD_EMBLEM",                               //MSG_SAVE_GUILD_EMBLEM
    "MSG_TABARDVENDOR_ACTIVATE",                           //MSG_TABARDVENDOR_ACTIVATE
    "SMSG_PLAY_SPELL_VISUAL",                              //SMSG_PLAY_SPELL_VISUAL
    "CMSG_ZONEUPDATE",                                     //CMSG_ZONEUPDATE
    "SMSG_PARTYKILLLOG",                                   //SMSG_PARTYKILLLOG
    "SMSG_COMPRESSED_UPDATE_OBJECT",                       //SMSG_COMPRESSED_UPDATE_OBJECT
    "SMSG_OBSOLETE",                                       //SMSG_OBSOLETE
    "SMSG_EXPLORATION_EXPERIENCE",                         //SMSG_EXPLORATION_EXPERIENCE
    "CMSG_GM_SET_SECURITY_GROUP",                          //CMSG_GM_SET_SECURITY_GROUP
    "CMSG_GM_NUKE",                                        //CMSG_GM_NUKE
    "MSG_RANDOM_ROLL",                                     //MSG_RANDOM_ROLL
    "SMSG_ENVIRONMENTALDAMAGELOG",                         //SMSG_ENVIRONMENTALDAMAGELOG
    "CMSG_RWHOIS",                                         //CMSG_RWHOIS
    "SMSG_RWHOIS",                                         //SMSG_RWHOIS
    "MSG_LOOKING_FOR_GROUP",                               //MSG_LOOKING_FOR_GROUP
    "CMSG_SET_LOOKING_FOR_GROUP",                          //CMSG_SET_LOOKING_FOR_GROUP
    "CMSG_UNLEARN_SPELL",                                  //CMSG_UNLEARN_SPELL
    "CMSG_UNLEARN_SKILL",                                  //CMSG_UNLEARN_SKILL
    "SMSG_REMOVED_SPELL",                                  //SMSG_REMOVED_SPELL
    "CMSG_DECHARGE",                                       //CMSG_DECHARGE
    "CMSG_GMTICKET_CREATE",                                //CMSG_GMTICKET_CREATE
    "SMSG_GMTICKET_CREATE",                                //SMSG_GMTICKET_CREATE
    "CMSG_GMTICKET_UPDATETEXT",                            //CMSG_GMTICKET_UPDATETEXT
    "SMSG_GMTICKET_UPDATETEXT",                            //SMSG_GMTICKET_UPDATETEXT
    "SMSG_ACCOUNT_DATA_MD5",                               //SMSG_ACCOUNT_DATA_MD5
    "CMSG_REQUEST_ACCOUNT_DATA",                           //CMSG_REQUEST_ACCOUNT_DATA
    "CMSG_UPDATE_ACCOUNT_DATA",                            //CMSG_UPDATE_ACCOUNT_DATA
    "SMSG_UPDATE_ACCOUNT_DATA",                            //SMSG_UPDATE_ACCOUNT_DATA
    "SMSG_CLEAR_FAR_SIGHT_IMMEDIATE",                      //SMSG_CLEAR_FAR_SIGHT_IMMEDIATE
    "SMSG_POWERGAINLOG_OBSOLETE",                          //SMSG_POWERGAINLOG_OBSOLETE
    "CMSG_GM_TEACH",                                       //CMSG_GM_TEACH
    "CMSG_GM_CREATE_ITEM_TARGET",                          //CMSG_GM_CREATE_ITEM_TARGET
    "CMSG_GMTICKET_GETTICKET",                             //CMSG_GMTICKET_GETTICKET
    "SMSG_GMTICKET_GETTICKET",                             //SMSG_GMTICKET_GETTICKET
    "CMSG_UNLEARN_TALENTS",                                //CMSG_UNLEARN_TALENTS
    "SMSG_GAMEOBJECT_SPAWN_ANIM",                          //SMSG_GAMEOBJECT_SPAWN_ANIM
    "SMSG_GAMEOBJECT_DESPAWN_ANIM",                        //SMSG_GAMEOBJECT_DESPAWN_ANIM
    "MSG_CORPSE_QUERY",                                    //MSG_CORPSE_QUERY
    "CMSG_GMTICKET_DELETETICKET",                          //CMSG_GMTICKET_DELETETICKET
    "SMSG_GMTICKET_DELETETICKET",                          //SMSG_GMTICKET_DELETETICKET
    "SMSG_CHAT_WRONG_FACTION",                             //SMSG_CHAT_WRONG_FACTION
    "CMSG_GMTICKET_SYSTEMSTATUS",                          //CMSG_GMTICKET_SYSTEMSTATUS
    "SMSG_GMTICKET_SYSTEMSTATUS",                          //SMSG_GMTICKET_SYSTEMSTATUS
    "CMSG_SPIRIT_HEALER_ACTIVATE",                         //CMSG_SPIRIT_HEALER_ACTIVATE
    "CMSG_SET_STAT_CHEAT",                                 //CMSG_SET_STAT_CHEAT
    "SMSG_SET_REST_START",                                 //SMSG_SET_REST_START
    "CMSG_SKILL_BUY_STEP",                                 //CMSG_SKILL_BUY_STEP
    "CMSG_SKILL_BUY_RANK",                                 //CMSG_SKILL_BUY_RANK
    "CMSG_XP_CHEAT",                                       //CMSG_XP_CHEAT
    "SMSG_SPIRIT_HEALER_CONFIRM",                          //SMSG_SPIRIT_HEALER_CONFIRM
    "CMSG_CHARACTER_POINT_CHEAT",                          //CMSG_CHARACTER_POINT_CHEAT
    "SMSG_GOSSIP_POI",                                     //SMSG_GOSSIP_POI
    "CMSG_CHAT_IGNORED",                                   //CMSG_CHAT_IGNORED
    "CMSG_GM_VISION",                                      //CMSG_GM_VISION
    "CMSG_SERVER_COMMAND",                                 //CMSG_SERVER_COMMAND
    "CMSG_GM_SILENCE",                                     //CMSG_GM_SILENCE
    "CMSG_GM_REVEALTO",                                    //CMSG_GM_REVEALTO
    "CMSG_GM_RESURRECT",                                   //CMSG_GM_RESURRECT
    "CMSG_GM_SUMMONMOB",                                   //CMSG_GM_SUMMONMOB
    "CMSG_GM_MOVECORPSE",                                  //CMSG_GM_MOVECORPSE
    "CMSG_GM_FREEZE",                                      //CMSG_GM_FREEZE
    "CMSG_GM_UBERINVIS",                                   //CMSG_GM_UBERINVIS
    "CMSG_GM_REQUEST_PLAYER_INFO",                         //CMSG_GM_REQUEST_PLAYER_INFO
    "SMSG_GM_PLAYER_INFO",                                 //SMSG_GM_PLAYER_INFO
    "CMSG_GUILD_RANK",                                     //CMSG_GUILD_RANK
    "CMSG_GUILD_ADD_RANK",                                 //CMSG_GUILD_ADD_RANK
    "CMSG_GUILD_DEL_RANK",                                 //CMSG_GUILD_DEL_RANK
    "CMSG_GUILD_SET_PUBLIC_NOTE",                          //CMSG_GUILD_SET_PUBLIC_NOTE
    "CMSG_GUILD_SET_OFFICER_NOTE",                         //CMSG_GUILD_SET_OFFICER_NOTE
    "SMSG_LOGIN_VERIFY_WORLD",                             //SMSG_LOGIN_VERIFY_WORLD
    "CMSG_CLEAR_EXPLORATION",                              //CMSG_CLEAR_EXPLORATION
    "CMSG_SEND_MAIL",                                      //CMSG_SEND_MAIL
    "SMSG_SEND_MAIL_RESULT",                               //SMSG_SEND_MAIL_RESULT
    "CMSG_GET_MAIL_LIST",                                  //CMSG_GET_MAIL_LIST
    "SMSG_MAIL_LIST_RESULT",                               //SMSG_MAIL_LIST_RESULT
    "CMSG_BATTLEFIELD_LIST",                               //CMSG_BATTLEFIELD_LIST
    "SMSG_BATTLEFIELD_LIST",                               //SMSG_BATTLEFIELD_LIST
    "CMSG_BATTLEFIELD_JOIN",                               //CMSG_BATTLEFIELD_JOIN
    "SMSG_BATTLEFIELD_WIN",                                //SMSG_BATTLEFIELD_WIN
    "SMSG_BATTLEFIELD_LOSE",                               //SMSG_BATTLEFIELD_LOSE
    "CMSG_TAXICLEARNODE",                                  //CMSG_TAXICLEARNODE
    "CMSG_TAXIENABLENODE",                                 //CMSG_TAXIENABLENODE
    "CMSG_ITEM_TEXT_QUERY",                                //CMSG_ITEM_TEXT_QUERY
    "SMSG_ITEM_TEXT_QUERY_RESPONSE",                       //SMSG_ITEM_TEXT_QUERY_RESPONSE
    "CMSG_MAIL_TAKE_MONEY",                                //CMSG_MAIL_TAKE_MONEY
    "CMSG_MAIL_TAKE_ITEM",                                 //CMSG_MAIL_TAKE_ITEM
    "CMSG_MAIL_MARK_AS_READ",                              //CMSG_MAIL_MARK_AS_READ
    "CMSG_MAIL_RETURN_TO_SENDER",                          //CMSG_MAIL_RETURN_TO_SENDER
    "CMSG_MAIL_DELETE",                                    //CMSG_MAIL_DELETE
    "CMSG_MAIL_CREATE_TEXT_ITEM",                          //CMSG_MAIL_CREATE_TEXT_ITEM
    "SMSG_SPELLLOGMISS",                                   //SMSG_SPELLLOGMISS
    "SMSG_SPELLLOGEXECUTE",                                //SMSG_SPELLLOGEXECUTE
    "SMSG_DEBUGAURAPROC",                                  //SMSG_DEBUGAURAPROC
    "SMSG_PERIODICAURALOG",                                //SMSG_PERIODICAURALOG
    "SMSG_SPELLDAMAGESHIELD",                              //SMSG_SPELLDAMAGESHIELD
    "SMSG_SPELLNONMELEEDAMAGELOG",                         //SMSG_SPELLNONMELEEDAMAGELOG
    "CMSG_LEARN_TALENT",                                   //CMSG_LEARN_TALENT
    "SMSG_RESURRECT_FAILED",                               //SMSG_RESURRECT_FAILED
    "CMSG_TOGGLE_PVP",                                     //CMSG_TOGGLE_PVP
    "SMSG_ZONE_UNDER_ATTACK",                              //SMSG_ZONE_UNDER_ATTACK
    "MSG_AUCTION_HELLO",                                   //MSG_AUCTION_HELLO
    "CMSG_AUCTION_SELL_ITEM",                              //CMSG_AUCTION_SELL_ITEM
    "CMSG_AUCTION_REMOVE_ITEM",                            //CMSG_AUCTION_REMOVE_ITEM
    "CMSG_AUCTION_LIST_ITEMS",                             //CMSG_AUCTION_LIST_ITEMS
    "CMSG_AUCTION_LIST_OWNER_ITEMS",                       //CMSG_AUCTION_LIST_OWNER_ITEMS
    "CMSG_AUCTION_PLACE_BID",                              //CMSG_AUCTION_PLACE_BID
    "SMSG_AUCTION_COMMAND_RESULT",                         //SMSG_AUCTION_COMMAND_RESULT
    "SMSG_AUCTION_LIST_RESULT",                            //SMSG_AUCTION_LIST_RESULT
    "SMSG_AUCTION_OWNER_LIST_RESULT",                      //SMSG_AUCTION_OWNER_LIST_RESULT
    "SMSG_AUCTION_BIDDER_NOTIFICATION",                    //SMSG_AUCTION_BIDDER_NOTIFICATION
    "SMSG_AUCTION_OWNER_NOTIFICATION",                     //SMSG_AUCTION_OWNER_NOTIFICATION
    "SMSG_PROCRESIST",                                     //SMSG_PROCRESIST
    "SMSG_STANDSTATE_CHANGE_FAILURE",                      //SMSG_STANDSTATE_CHANGE_FAILURE
    "SMSG_DISPEL_FAILED",                                  //SMSG_DISPEL_FAILED
    "SMSG_SPELLORDAMAGE_IMMUNE",                           //SMSG_SPELLORDAMAGE_IMMUNE
    "CMSG_AUCTION_LIST_BIDDER_ITEMS",                      //CMSG_AUCTION_LIST_BIDDER_ITEMS
    "SMSG_AUCTION_BIDDER_LIST_RESULT",                     //SMSG_AUCTION_BIDDER_LIST_RESULT
    "SMSG_SET_FLAT_SPELL_MODIFIER",                        //SMSG_SET_FLAT_SPELL_MODIFIER
    "SMSG_SET_PCT_SPELL_MODIFIER",                         //SMSG_SET_PCT_SPELL_MODIFIER
    "CMSG_SET_AMMO",                                       //CMSG_SET_AMMO
    "SMSG_CORPSE_RECLAIM_DELAY",                           //SMSG_CORPSE_RECLAIM_DELAY
    "CMSG_SET_ACTIVE_MOVER",                               //CMSG_SET_ACTIVE_MOVER
    "CMSG_PET_CANCEL_AURA",                                //CMSG_PET_CANCEL_AURA
    "CMSG_PLAYER_AI_CHEAT",                                //CMSG_PLAYER_AI_CHEAT
    "CMSG_CANCEL_AUTO_REPEAT_SPELL",                       //CMSG_CANCEL_AUTO_REPEAT_SPELL
    "MSG_GM_ACCOUNT_ONLINE",                               //MSG_GM_ACCOUNT_ONLINE
    "MSG_LIST_STABLED_PETS",                               //MSG_LIST_STABLED_PETS
    "CMSG_STABLE_PET",                                     //CMSG_STABLE_PET
    "CMSG_UNSTABLE_PET",                                   //CMSG_UNSTABLE_PET
    "CMSG_BUY_STABLE_SLOT",                                //CMSG_BUY_STABLE_SLOT
    "SMSG_STABLE_RESULT",                                  //SMSG_STABLE_RESULT
    "CMSG_STABLE_REVIVE_PET",                              //CMSG_STABLE_REVIVE_PET
    "CMSG_STABLE_SWAP_PET",                                //CMSG_STABLE_SWAP_PET
    "MSG_QUEST_PUSH_RESULT",                               //MSG_QUEST_PUSH_RESULT
    "SMSG_PLAY_MUSIC",                                     //SMSG_PLAY_MUSIC
    "SMSG_PLAY_OBJECT_SOUND",                              //SMSG_PLAY_OBJECT_SOUND
    "CMSG_REQUEST_PET_INFO",                               //CMSG_REQUEST_PET_INFO
    "CMSG_FAR_SIGHT",                                      //CMSG_FAR_SIGHT
    "SMSG_SPELLDISPELLOG",                                 //SMSG_SPELLDISPELLOG
    "SMSG_DAMAGE_CALC_LOG",                                //SMSG_DAMAGE_CALC_LOG
    "CMSG_ENABLE_DAMAGE_LOG",                              //CMSG_ENABLE_DAMAGE_LOG
    "CMSG_GROUP_CHANGE_SUB_GROUP",                         //CMSG_GROUP_CHANGE_SUB_GROUP
    "CMSG_REQUEST_PARTY_MEMBER_STATS",                     //CMSG_REQUEST_PARTY_MEMBER_STATS
    "CMSG_GROUP_SWAP_SUB_GROUP",                           //CMSG_GROUP_SWAP_SUB_GROUP
    "CMSG_RESET_FACTION_CHEAT",                            //CMSG_RESET_FACTION_CHEAT
    "CMSG_AUTOSTORE_BANK_ITEM",                            //CMSG_AUTOSTORE_BANK_ITEM
    "CMSG_AUTOBANK_ITEM",                                  //CMSG_AUTOBANK_ITEM
    "MSG_QUERY_NEXT_MAIL_TIME",                            //MSG_QUERY_NEXT_MAIL_TIME
    "SMSG_RECEIVED_MAIL",                                  //SMSG_RECEIVED_MAIL
    "SMSG_RAID_GROUP_ONLY",                                //SMSG_RAID_GROUP_ONLY
    "CMSG_SET_DURABILITY_CHEAT",                           //CMSG_SET_DURABILITY_CHEAT
    "CMSG_SET_PVP_RANK_CHEAT",                             //CMSG_SET_PVP_RANK_CHEAT
    "CMSG_ADD_PVP_MEDAL_CHEAT",                            //CMSG_ADD_PVP_MEDAL_CHEAT
    "CMSG_DEL_PVP_MEDAL_CHEAT",                            //CMSG_DEL_PVP_MEDAL_CHEAT
    "CMSG_SET_PVP_TITLE",                                  //CMSG_SET_PVP_TITLE
    "SMSG_PVP_CREDIT",                                     //SMSG_PVP_CREDIT
    "SMSG_AUCTION_REMOVED_NOTIFICATION",                   //SMSG_AUCTION_REMOVED_NOTIFICATION
    "CMSG_GROUP_RAID_CONVERT",                             //CMSG_GROUP_RAID_CONVERT
    "CMSG_GROUP_ASSISTANT",                                //CMSG_GROUP_ASSISTANT
    "CMSG_BUYBACK_ITEM",                                   //CMSG_BUYBACK_ITEM
    "SMSG_SERVER_MESSAGE",                                 //SMSG_SERVER_MESSAGE
    "CMSG_MEETINGSTONE_JOIN",                              //CMSG_MEETINGSTONE_JOIN
    "CMSG_MEETINGSTONE_LEAVE",                             //CMSG_MEETINGSTONE_LEAVE
    "CMSG_MEETINGSTONE_CHEAT",                             //CMSG_MEETINGSTONE_CHEAT
    "SMSG_MEETINGSTONE_SETQUEUE",                          //SMSG_MEETINGSTONE_SETQUEUE
    "CMSG_MEETINGSTONE_INFO",                              //CMSG_MEETINGSTONE_INFO
    "SMSG_MEETINGSTONE_COMPLETE",                          //SMSG_MEETINGSTONE_COMPLETE
    "SMSG_MEETINGSTONE_IN_PROGRESS",                       //SMSG_MEETINGSTONE_IN_PROGRESS
    "SMSG_MEETINGSTONE_MEMBER_ADDED",                      //SMSG_MEETINGSTONE_MEMBER_ADDED
    "CMSG_GMTICKETSYSTEM_TOGGLE",                          //CMSG_GMTICKETSYSTEM_TOGGLE
    "CMSG_CANCEL_GROWTH_AURA",                             //CMSG_CANCEL_GROWTH_AURA
    "SMSG_CANCEL_AUTO_REPEAT",                             //SMSG_CANCEL_AUTO_REPEAT
    "SMSG_STANDSTATE_CHANGE_ACK",                          //SMSG_STANDSTATE_CHANGE_ACK
    "SMSG_LOOT_ALL_PASSED",                                //SMSG_LOOT_ALL_PASSED
    "SMSG_LOOT_ROLL_WON",                                  //SMSG_LOOT_ROLL_WON
    "CMSG_LOOT_ROLL",                                      //CMSG_LOOT_ROLL
    "SMSG_LOOT_START_ROLL",                                //SMSG_LOOT_START_ROLL
    "SMSG_LOOT_ROLL",                                      //SMSG_LOOT_ROLL
    "CMSG_LOOT_MASTER_GIVE",                               //CMSG_LOOT_MASTER_GIVE
    "SMSG_LOOT_MASTER_LIST",                               //SMSG_LOOT_MASTER_LIST
    "SMSG_SET_FORCED_REACTIONS",                           //SMSG_SET_FORCED_REACTIONS
    "SMSG_SPELL_FAILED_OTHER",                             //SMSG_SPELL_FAILED_OTHER
    "SMSG_GAMEOBJECT_RESET_STATE",                         //SMSG_GAMEOBJECT_RESET_STATE
    "CMSG_REPAIR_ITEM",                                    //CMSG_REPAIR_ITEM
    "SMSG_CHAT_PLAYER_NOT_FOUND",                          //SMSG_CHAT_PLAYER_NOT_FOUND
    "MSG_TALENT_WIPE_CONFIRM",                             //MSG_TALENT_WIPE_CONFIRM
    "SMSG_SUMMON_REQUEST",                                 //SMSG_SUMMON_REQUEST
    "CMSG_SUMMON_RESPONSE",                                //CMSG_SUMMON_RESPONSE
    "MSG_MOVE_TOGGLE_GRAVITY_CHEAT",                       //MSG_MOVE_TOGGLE_GRAVITY_CHEAT
    "SMSG_MONSTER_MOVE_TRANSPORT",                         //SMSG_MONSTER_MOVE_TRANSPORT
    "SMSG_PET_BROKEN",                                     //SMSG_PET_BROKEN
    "MSG_MOVE_FEATHER_FALL",                               //MSG_MOVE_FEATHER_FALL
    "MSG_MOVE_WATER_WALK",                                 //MSG_MOVE_WATER_WALK
    "CMSG_SERVER_BROADCAST",                               //CMSG_SERVER_BROADCAST
    "CMSG_SELF_RES",                                       //CMSG_SELF_RES
    "SMSG_FEIGN_DEATH_RESISTED",                           //SMSG_FEIGN_DEATH_RESISTED
    "CMSG_RUN_SCRIPT",                                     //CMSG_RUN_SCRIPT
    "SMSG_SCRIPT_MESSAGE",                                 //SMSG_SCRIPT_MESSAGE
    "SMSG_DUEL_COUNTDOWN",                                 //SMSG_DUEL_COUNTDOWN
    "SMSG_AREA_TRIGGER_MESSAGE",                           //SMSG_AREA_TRIGGER_MESSAGE
    "CMSG_TOGGLE_HELM",                                    //CMSG_TOGGLE_HELM
    "CMSG_TOGGLE_CLOAK",                                   //CMSG_TOGGLE_CLOAK
    "SMSG_MEETINGSTONE_JOINFAILED",                        //SMSG_MEETINGSTONE_JOINFAILED
    "SMSG_PLAYER_SKINNED",                                 //SMSG_PLAYER_SKINNED
    "SMSG_DURABILITY_DAMAGE_DEATH",                        //SMSG_DURABILITY_DAMAGE_DEATH
    "CMSG_SET_EXPLORATION",                                //CMSG_SET_EXPLORATION
    "CMSG_SET_ACTIONBAR_TOGGLES",                          //CMSG_SET_ACTIONBAR_TOGGLES
    "UMSG_DELETE_GUILD_CHARTER",                           //UMSG_DELETE_GUILD_CHARTER
    "MSG_PETITION_RENAME",                                 //MSG_PETITION_RENAME
    "SMSG_INIT_WORLD_STATES",                              //SMSG_INIT_WORLD_STATES
    "SMSG_UPDATE_WORLD_STATE",                             //SMSG_UPDATE_WORLD_STATE
    "CMSG_ITEM_NAME_QUERY",                                //CMSG_ITEM_NAME_QUERY
    "SMSG_ITEM_NAME_QUERY_RESPONSE",                       //SMSG_ITEM_NAME_QUERY_RESPONSE
    "SMSG_PET_ACTION_FEEDBACK",                            //SMSG_PET_ACTION_FEEDBACK
    "CMSG_CHAR_RENAME",                                    //CMSG_CHAR_RENAME
    "SMSG_CHAR_RENAME",                                    //SMSG_CHAR_RENAME
    "CMSG_MOVE_SPLINE_DONE",                               //CMSG_MOVE_SPLINE_DONE
    "CMSG_MOVE_FALL_RESET",                                //CMSG_MOVE_FALL_RESET
    "SMSG_INSTANCE_SAVE_CREATED",                          //SMSG_INSTANCE_SAVE_CREATED
    "SMSG_RAID_INSTANCE_INFO",                             //SMSG_RAID_INSTANCE_INFO
    "CMSG_REQUEST_RAID_INFO",                              //CMSG_REQUEST_RAID_INFO
    "CMSG_MOVE_TIME_SKIPPED",                              //CMSG_MOVE_TIME_SKIPPED
    "CMSG_MOVE_FEATHER_FALL_ACK",                          //CMSG_MOVE_FEATHER_FALL_ACK
    "CMSG_MOVE_WATER_WALK_ACK",                            //CMSG_MOVE_WATER_WALK_ACK
    "CMSG_MOVE_NOT_ACTIVE_MOVER",                          //CMSG_MOVE_NOT_ACTIVE_MOVER
    "SMSG_PLAY_SOUND",                                     //SMSG_PLAY_SOUND
    "CMSG_BATTLEFIELD_STATUS",                             //CMSG_BATTLEFIELD_STATUS
    "SMSG_BATTLEFIELD_STATUS",                             //SMSG_BATTLEFIELD_STATUS
    "CMSG_BATTLEFIELD_PORT",                               //CMSG_BATTLEFIELD_PORT
    "MSG_INSPECT_HONOR_STATS",                             //MSG_INSPECT_HONOR_STATS
    "CMSG_BATTLEMASTER_HELLO",                             //CMSG_BATTLEMASTER_HELLO
    "CMSG_MOVE_START_SWIM_CHEAT",                          //CMSG_MOVE_START_SWIM_CHEAT
    "CMSG_MOVE_STOP_SWIM_CHEAT",                           //CMSG_MOVE_STOP_SWIM_CHEAT
    "SMSG_FORCE_WALK_SPEED_CHANGE",                        //SMSG_FORCE_WALK_SPEED_CHANGE
    "CMSG_FORCE_WALK_SPEED_CHANGE_ACK",                    //CMSG_FORCE_WALK_SPEED_CHANGE_ACK
    "SMSG_FORCE_SWIM_BACK_SPEED_CHANGE",                   //SMSG_FORCE_SWIM_BACK_SPEED_CHANGE
    "CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK",               //CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK
    "SMSG_FORCE_TURN_RATE_CHANGE",                         //SMSG_FORCE_TURN_RATE_CHANGE
    "CMSG_FORCE_TURN_RATE_CHANGE_ACK",                     //CMSG_FORCE_TURN_RATE_CHANGE_ACK
    "MSG_PVP_LOG_DATA",                                    //MSG_PVP_LOG_DATA
    "CMSG_LEAVE_BATTLEFIELD",                              //CMSG_LEAVE_BATTLEFIELD
    "CMSG_AREA_SPIRIT_HEALER_QUERY",                       //CMSG_AREA_SPIRIT_HEALER_QUERY
    "CMSG_AREA_SPIRIT_HEALER_QUEUE",                       //CMSG_AREA_SPIRIT_HEALER_QUEUE
    "SMSG_AREA_SPIRIT_HEALER_TIME",                        //SMSG_AREA_SPIRIT_HEALER_TIME
    "CMSG_HARDWARE_SURVEY_RESULTS",                        //CMSG_HARDWARE_SURVEY_RESULTS
    "SMSG_WARDEN_DATA",                                    //SMSG_WARDEN_DATA
    "CMSG_WARDEN_DATA",                                    //CMSG_WARDEN_DATA
    "SMSG_GROUP_JOINED_BATTLEGROUND",                      //SMSG_GROUP_JOINED_BATTLEGROUND
    "MSG_BATTLEGROUND_PLAYER_POSITIONS",                   //MSG_BATTLEGROUND_PLAYER_POSITIONS
    "CMSG_PET_STOP_ATTACK",                                //CMSG_PET_STOP_ATTACK
    "SMSG_BINDER_CONFIRM",                                 //SMSG_BINDER_CONFIRM
    "SMSG_BATTLEGROUND_PLAYER_JOINED",                     //SMSG_BATTLEGROUND_PLAYER_JOINED
    "SMSG_BATTLEGROUND_PLAYER_LEFT",                       //SMSG_BATTLEGROUND_PLAYER_LEFT
    "CMSG_BATTLEMASTER_JOIN",                              //CMSG_BATTLEMASTER_JOIN
    "SMSG_ADDON_INFO",                                     //SMSG_ADDON_INFO
    "CMSG_PET_UNLEARN",                                    //CMSG_PET_UNLEARN
    "SMSG_PET_UNLEARN_CONFIRM",                            //SMSG_PET_UNLEARN_CONFIRM
    "SMSG_PARTY_MEMBER_STATS_FULL",                        //SMSG_PARTY_MEMBER_STATS_FULL
    "CMSG_PET_SPELL_AUTOCAST",                             //CMSG_PET_SPELL_AUTOCAST
    "SMSG_WEATHER",                                        //SMSG_WEATHER
    "SMSG_PLAY_TIME_WARNING",                              //SMSG_PLAY_TIME_WARNING
    "SMSG_MINIGAME_SETUP",                                 //SMSG_MINIGAME_SETUP
    "SMSG_MINIGAME_STATE",                                 //SMSG_MINIGAME_STATE
    "CMSG_MINIGAME_MOVE",                                  //CMSG_MINIGAME_MOVE
    "SMSG_MINIGAME_MOVE_FAILED",                           //SMSG_MINIGAME_MOVE_FAILED
    "UNKNOWN",                                             //762
    "SMSG_COMPRESSED_MOVE",                                //SMSG_COMPRESSED_MOVE= 763,
    "CMSG_GUILD_CHANGEINFO",                               //CMSG_GUILD_CHANGEINFO
    "SMSG_UNKNOWN_765",                                    //SMSG_UNKNOWN_765 = 765
    "SMSG_SET_MOVE_SPEED",                                 //SMSG_SET_MOVE_SPEED = 766
    "SMSG_SET_RUN_BACK_SPEED",                             // 767
    "SMSG_SET_SWIM_SPEED",                                 // 768
    "SMSG_UNKNOWN_769",                                    // 769
    "SMSG_SET_SWIM_BACK_SPEED",                            // 770"MSG_SET_TURN_RATE",  
    "UNKNOWN",                                             // 771
    "SMSG_UNKNOWN_772",                                    // 772
    "SMSG_UNKNOWN_773",                                    // 773
    "SMSG_UNKNOWN_774",                                    // 774
    "SMSG_UNKNOWN_775",                                    // 775
    "SMSG_UNKNOWN_776",                                    // 776
    "SMSG_MOVE_SET_WATERWALK",                             // 777
    "SMSG_MOVE_STOP_WATERWALK",                            // 778
    "SMSG_UNKNOWN_779",                                    // 779
    "SMSG_UNKNOWN_780",                                    // 780
    "SMSG_MOVE_STOP_WALK",                                 // 781
    "SMSG_MOVE_START_WALK",                                // 782
    "UNKNOWN",                                             // 783
    "UNKNOWN",                                             // 784
    "UNKNOWN",                                             // 785
    "CMSG_ACTIVATETAXI_FAR",                               // 786
    "UNKNOWN",                                             // 787 causes client crash
    "UNKNOWN",                                             // 788
    "UNKNOWN",                                             // 789
    "UNKNOWN",                                             // 790
    "CMSG_FIELD_WATCHED_FACTION_INACTIVE",                 // 791
    "CMSG_FIELD_WATCHED_FACTION_SHOW_BAR",                 // 792
    "SMSG_UNKNOWN_793",                                    // 793
    "SMSG_UNKNOWN_794",                                    // 794
    "UNKNOWN",                                             // 795
    "SMSG_UNKNOWN_796",                                    // 796
    "CMSG_RESET_INSTANCES",                                // 797
    "SMSG_RESET_INSTANCES_RESULT",                         // 798
    "UNKNOWN",                                             // 799
    "SMSG_UNKNOWN_800",                                    // 800
    "MSG_RAID_ICON_TARGET",                                // 801
    "MSG_RAID_READY_CHECK",                                // 802
    "UNKNOWN",                                             // 803
    "SMSG_AI_UNKNOWN",                                     // 804
    "SMSG_UNKNOWN_805",                                    // 805
    "UNKNOWN",                                             // 806
    "UNKNOWN",                                             // 807
    "SMSG_UNKNOWN_808",                                    // 808
    "MSG_SET_DUNGEON_DIFFICULTY",                          // 809
    "UNKNOWN",                                             // 810
    "SMSG_UNKNOWN_811",                                    // 811
    "UNKNOWN",                                             // 812
    "UNKNOWN",                                             // 813
    "UNKNOWN",                                             // 814
    "SMSG_UNKNOWN_815",                                    // 815
    "SMSG_UNKNOWN_816",                                    // 816
    "UNKNOWN",                                             // 817
    "SMSG_UNKNOWN_818",                                    // 818
    "UNKNOWN",                                             // 819
    "UNKNOWN",                                             // 820
    "UNKNOWN",                                             // 821
    "UNKNOWN",                                             // 822
    "UNKNOWN",                                             // 823
    "UNKNOWN",                                             // 824
    "UNKNOWN",                                             // 825
    "SMSG_OUTDOORPVP_NOTIFY",                              //SMSG_OUTDOORPVP_NOTIFY 826
    "SMSG_OUTDOORPVP_NOTIFY2",                             // 827
    "UNKNOWN",                                             // 828
    "SMSG_MOTD",                                           // 829
    "UNKNOWN",                                             // 830
    "UNKNOWN",                                             // 831
    "UNKNOWN",                                             // 832
    "SMSG_UNKNOWN_833",                                    // 833
    "SMSG_UNKNOWN_834",                                    // 834
    "SMSG_FLY_MODE_START",                                 // 835
    "SMSG_FLY_MODE_STOP",                                  // 836
    "CMSG_MOVE_FLY_MODE_CHANGE_ACK",                       // 837
    "CMSG_MOVE_FLY_STATE_CHANGE",                          // 838
    "CMSG_SOCKET_ITEM",                                    //CMSG_SOCKET_ITEM 839
    "UNKNOWN",                                             // 840
    "SMSG_ARENA_TEAM_COMMAND_RESULT",                      // 841
    "UNKNOWN",                                             // 842
    "CMSG_ARENA_TEAM_QUERY",                               // 843
    "SMSG_ARENA_TEAM_QUERY_RESPONSE",                      // 844
    "CMSG_ARENA_TEAM_ROSTER",                              // 845
    "SMSG_ARENA_TEAM_ROSTER",                              // 846
    "CMSG_ARENA_TEAM_ADD_MEMBER",                          // 847
    "SMSG_ARENA_TEAM_INVITE",                              // 848
    "CMSG_ARENA_TEAM_INVITE_ACCEPT",                       // 849
    "CMSG_ARENA_TEAM_INVITE_DECLINE",                      // 850
    "CMSG_ARENA_TEAM_LEAVE",                               // 851
    "CMSG_ARENA_TEAM_REMOVE_FROM_TEAM",                    // 852
    "CMSG_ARENA_TEAM_DISBAND",                             // 853
    "CMSG_ARENA_TEAM_PROMOTE_TO_CAPTAIN",                  // 854
    "SMSG_UNKNOWN_855",                                    // 855
    "UNKNOWN",                                             // 856
    "MSG_MOVE_START_FLY_UP",                               // 857
    "MSG_MOVE_STOP_FLY_UP",                                // 858
    "SMSG_ARENA_TEAM_STATS",                               // 859
    "CMSG_LFG_SET_AUTOJOIN",                               // 860
    "CMSG_LFG_UNSET_AUTOJOIN",                             // 861
    "CMSG_LFM_SET_AUTOADD",                                // 862
    "CMSG_LFM_UNSET_AUTOADD",                              // 863
    "CMSG_LFG_INVITE_ACCEPT",                              // 864
    "CMSG_LFG_INVITE_CANCEL",                              // 865
    "UNKNOWN",                                             // 866
    "CMSG_LOOKING_FOR_GROUP_CLEAR",                        // 867
    "CMSG_SET_LOOKING_FOR_NONE",                           // 868
    "CMSG_SET_LOOKING_FOR_MORE",                           // 869
    "CMSG_SET_COMMENTARY",                                 // 870
    "SMSG_LFG_871",                                        // 871
    "SMSG_LFG_872",                                        // 872
    "SMSG_LFG_873",                                        // 873
    "SMSG_LFG_874",                                        // 974
    "UNKNOWN",                                             // 875
    "SMSG_LFG_876",                                        // 876
    "SMSG_LFG_877",                                        // 877
    "SMSG_LFG_878",                                        // 878
    "SMSG_LFG_879",                                        // 879
    "SMSG_LFG_INVITE",                                     // 880
    "SMSG_LFG_881",                                        // 881
    "UNKNOWN",                                             // 882
    "UNKNOWN",                                             // 883
    "CMSG_CHOOSE_TITLE",                                   // 884
    "CMSG_DISMOUNT",                                       // 885
    "UNKNOWN",                                             // 886
    "MSG_INSPECT_ARENA_STATS",                             // 887
    "SMSG_SH_POSITION",                                    // 888
    "CMSG_CANCEL_TEMP_ITEM_ENCHANTMENT",                   // 889
    "UNKNOWN",                                             // 890
    "UNKNOWN",                                             // 891
    "UNKNOWN",                                             // 892
    "UNKNOWN",                                             // 893
    "SMSG_MOVE_SET_FLY_SPEED",                             // 894
    "UNKNOWN",                                             // 895
    "SMSG_MOVE_SET_FLY_BACK_SPEED",                        // 896
    "SMSG_FORCE_FLY_SPEED_CHANGE",                         // 897
    "CMSG_FORCE_FLY_SPEED_CHANGE_ACK",                     // 898
    "SMSG_FORCE_FLY_BACK_SPEED_CHANGE",                    // 899
    "CMSG_FORCE_FLY_BACK_SPEED_CHANGE_ACK",                // 900
    "SMSG_MOVE_SET_FLY_SPEED2",                            // 901
    "SMSG_MOVE_SET_FLY_BACK_SPEED2",                       // 902
    "UNKNOWN",                                             // 903
    "UNKNOWN",                                             // 904 SMSG_FLIGHT_SPLINE_SYNC?
    "UNKNOWN",                                             // 905
    "UNKNOWN",                                             // 906
    "SMSG_REALM_STATE_RESPONSE",                           // 907
    "CMSG_REALM_STATE_REQUEST",                            // 908
    "CMSG_MOVE_SHIP_909",                                  // 909
    "CMSG_GROUP_PROMOTE",                                  // 910
    "UNKNOWN",                                             // 911
    "SMSG_ALLOW_MOVE",                                     // 912
    "CMSG_ALLOW_MOVE_ACK",                                 // 913
    "UNKNOWN",                                             // 914
    "UNKNOWN",                                             // 915
    "UNKNOWN",                                             // 916
    "UNKNOWN",                                             // 917
    "SMSG_UNKNOWN_918",                                    // 918
    "SMSG_UNKNOWN_919",                                    // 919
    "SMSG_UNKNOWN_920",                                    // 920 
    //there would be declared new opcode, id 921
    //do not forget to change MAX_OPCODE_ID, if you add a line here!
};
