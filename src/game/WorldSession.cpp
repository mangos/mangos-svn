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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "Opcodes.h"
#include "WorldSocket.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "Group.h"
#include "Guild.h"
#include "World.h"
#include "NameTables.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "BattleGroundMgr.h"
#include "Language.h"                                       // for CMSG_DISMOUNT handler
#include "Chat.h"

/// WorldSession constructor
WorldSession::WorldSession(uint32 id, WorldSocket *sock, uint32 sec, bool tbc, time_t mute_time, uint8 locale) :
LookingForGroup_auto_join(false), LookingForGroup_auto_add(false), m_muteTime(mute_time),
_player(NULL), _socket(sock),_security(sec), _accountId(id), m_isTBC(tbc), m_sessionLanguage(locale),
_logoutTime(0), m_playerLoading(false), m_playerLogout(false), m_playerRecentlyLogout(false)
{
    FillOpcodeHandlerHashTable();
}

/// WorldSession destructor
WorldSession::~WorldSession()
{
    ///- unload player if not unloaded
    if(_player)
        LogoutPlayer(true);

    /// - If have unclosed socket, close it
    if(_socket)
        _socket->CloseSocket();

    _socket = NULL;

    ///- empty incoming packet queue
    while(!_recvQueue.empty())
    {
        WorldPacket *packet = _recvQueue.next();
        delete packet;
    }
}

void WorldSession::FillOpcodeHandlerHashTable()
{
    //if table is already filled
    if (!objmgr.opcodeTable.empty())
        return;

    //fill table if empty
    objmgr.opcodeTable[ CMSG_CHAR_ENUM ]                        = OpcodeHandler( STATUS_AUTHED,   &WorldSession::HandleCharEnumOpcode                );
    objmgr.opcodeTable[ CMSG_CHAR_CREATE ]                      = OpcodeHandler( STATUS_AUTHED,   &WorldSession::HandleCharCreateOpcode              );
    objmgr.opcodeTable[ CMSG_CHAR_DELETE ]                      = OpcodeHandler( STATUS_AUTHED,   &WorldSession::HandleCharDeleteOpcode              );
    objmgr.opcodeTable[ CMSG_PLAYER_LOGIN ]                     = OpcodeHandler( STATUS_AUTHED,   &WorldSession::HandlePlayerLoginOpcode             );
    objmgr.opcodeTable[ CMSG_CHAR_RENAME ]                      = OpcodeHandler( STATUS_AUTHED,   &WorldSession::HandleChangePlayerNameOpcode        );

    objmgr.opcodeTable[ CMSG_SET_ACTION_BUTTON ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetActionButtonOpcode         );
    objmgr.opcodeTable[ CMSG_REPOP_REQUEST ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleRepopRequestOpcode            );
    objmgr.opcodeTable[ CMSG_AUTOSTORE_LOOT_ITEM ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAutostoreLootItemOpcode       );
    objmgr.opcodeTable[ CMSG_LOOT_MONEY ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLootMoneyOpcode               );
    objmgr.opcodeTable[ CMSG_LOOT ]                             = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLootOpcode                    );
    objmgr.opcodeTable[ CMSG_LOOT_RELEASE ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLootReleaseOpcode             );
    objmgr.opcodeTable[ CMSG_WHO ]                              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleWhoOpcode                     );
    objmgr.opcodeTable[ CMSG_LOGOUT_REQUEST ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLogoutRequestOpcode           );
    objmgr.opcodeTable[ CMSG_PLAYER_LOGOUT ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePlayerLogoutOpcode            );
    objmgr.opcodeTable[ CMSG_LOGOUT_CANCEL ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLogoutCancelOpcode            );
    objmgr.opcodeTable[ CMSG_GMTICKET_GETTICKET ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGMTicketGetTicketOpcode       );
    objmgr.opcodeTable[ CMSG_GMTICKET_CREATE ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGMTicketCreateOpcode          );
    objmgr.opcodeTable[ CMSG_GMTICKET_SYSTEMSTATUS ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGMTicketSystemStatusOpcode    );
    objmgr.opcodeTable[ CMSG_GMTICKET_DELETETICKET ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGMTicketDeleteOpcode          );
    objmgr.opcodeTable[ CMSG_GMTICKET_UPDATETEXT ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGMTicketUpdateTextOpcode      );
    objmgr.opcodeTable[ CMSG_TOGGLE_PVP ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTogglePvP                     );

    // played time
    objmgr.opcodeTable[ CMSG_PLAYED_TIME ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePlayedTime                    );

    // new inspect
    objmgr.opcodeTable[ CMSG_INSPECT ]                          = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleInspectOpcode                 );

    // new inspect stats
    objmgr.opcodeTable[ MSG_INSPECT_HONOR_STATS ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleInspectHonorStatsOpcode       );

    // character view
    objmgr.opcodeTable[ CMSG_TOGGLE_HELM ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleToggleHelmOpcode              );
    objmgr.opcodeTable[ CMSG_TOGGLE_CLOAK ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleToggleCloakOpcode             );

    // repair item
    objmgr.opcodeTable[ CMSG_REPAIR_ITEM ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleRepairItemOpcode              );

    objmgr.opcodeTable[ MSG_LOOKING_FOR_GROUP ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLookingForGroup               );
    objmgr.opcodeTable[ CMSG_SET_FACTION_ATWAR ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetFactionAtWar               );
    objmgr.opcodeTable[ CMSG_SET_FACTION_CHEAT ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetFactionCheat               );
    objmgr.opcodeTable[ CMSG_ZONEUPDATE ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleZoneUpdateOpcode              );
    objmgr.opcodeTable[ CMSG_SET_TARGET_OBSOLETE ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetTargetOpcode               );
    objmgr.opcodeTable[ CMSG_SET_SELECTION ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetSelectionOpcode            );
    objmgr.opcodeTable[ CMSG_STANDSTATECHANGE ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleStandStateChangeOpcode        );
    objmgr.opcodeTable[ CMSG_FRIEND_LIST ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleFriendListOpcode              );
    objmgr.opcodeTable[ CMSG_ADD_FRIEND ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAddFriendOpcode               );
    objmgr.opcodeTable[ CMSG_DEL_FRIEND ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleDelFriendOpcode               );
    objmgr.opcodeTable[ CMSG_ADD_IGNORE ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAddIgnoreOpcode               );
    objmgr.opcodeTable[ CMSG_DEL_IGNORE ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleDelIgnoreOpcode               );
    objmgr.opcodeTable[ CMSG_BUG ]                              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBugOpcode                     );
    objmgr.opcodeTable[ CMSG_SET_AMMO ]                         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetAmmoOpcode                 );
    objmgr.opcodeTable[ CMSG_AREATRIGGER ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAreaTriggerOpcode             );
    objmgr.opcodeTable[ CMSG_UPDATE_ACCOUNT_DATA ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleUpdateAccountData             );
    objmgr.opcodeTable[ CMSG_REQUEST_ACCOUNT_DATA ]             = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleRequestAccountData            );
    objmgr.opcodeTable[ CMSG_MEETINGSTONE_INFO ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMeetingStoneInfo              );
    objmgr.opcodeTable[ CMSG_GAMEOBJ_USE ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGameObjectUseOpcode           );
    objmgr.opcodeTable[ MSG_CORPSE_QUERY ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleCorpseQueryOpcode             );
    objmgr.opcodeTable[ CMSG_NAME_QUERY ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleNameQueryOpcode               );
    objmgr.opcodeTable[ CMSG_QUERY_TIME ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQueryTimeOpcode               );
    objmgr.opcodeTable[ CMSG_CREATURE_QUERY ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleCreatureQueryOpcode           );
    objmgr.opcodeTable[ CMSG_GAMEOBJECT_QUERY ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGameObjectQueryOpcode         );

    // movements
    objmgr.opcodeTable[ MSG_MOVE_START_FORWARD ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_START_BACKWARD ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_STOP ]                         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_START_STRAFE_LEFT ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_START_STRAFE_RIGHT ]           = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_STOP_STRAFE ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_JUMP ]                         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_START_TURN_LEFT ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_START_TURN_RIGHT ]             = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_STOP_TURN ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_START_PITCH_UP ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_START_PITCH_DOWN ]             = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_STOP_PITCH ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_SET_RUN_MODE ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_SET_WALK_MODE ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_TELEPORT_ACK ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMoveTeleportAck               );
    objmgr.opcodeTable[ MSG_MOVE_FALL_LAND ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_START_SWIM ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_STOP_SWIM ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_SET_FACING ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_SET_PITCH ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_SHIP_909 ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_FLY_STATE_CHANGE ]             = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_UNKNOWN_935]                        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );        
    objmgr.opcodeTable[ MSG_MOVE_WORLDPORT_ACK ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMoveWorldportAckOpcode        );
    objmgr.opcodeTable[ CMSG_FORCE_RUN_SPEED_CHANGE_ACK ]       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           );
    objmgr.opcodeTable[ CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK ]  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           );
    objmgr.opcodeTable[ CMSG_FORCE_SWIM_SPEED_CHANGE_ACK ]      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           );
    objmgr.opcodeTable[ CMSG_FORCE_MOVE_ROOT_ACK ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMoveRootAck                   );
    objmgr.opcodeTable[ CMSG_FORCE_MOVE_UNROOT_ACK ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMoveUnRootAck                 );
    objmgr.opcodeTable[ MSG_MOVE_HEARTBEAT ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ CMSG_MOVE_KNOCK_BACK_ACK ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMoveKnockBackAck              );
    objmgr.opcodeTable[ CMSG_MOVE_HOVER_ACK ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMoveHoverAck                  );
    objmgr.opcodeTable[ CMSG_SET_ACTIVE_MOVER ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetActiveMoverOpcode          );
    objmgr.opcodeTable[ CMSG_MOVE_FALL_RESET ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ CMSG_MOVE_FEATHER_FALL_ACK ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleFeatherFallAck                );
    objmgr.opcodeTable[ CMSG_MOVE_WATER_WALK_ACK ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMoveWaterWalkAck              );
    objmgr.opcodeTable[ CMSG_FORCE_WALK_SPEED_CHANGE_ACK ]      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           );
    objmgr.opcodeTable[ CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK ] = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           );
    objmgr.opcodeTable[ CMSG_FORCE_TURN_RATE_CHANGE_ACK ]       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           );
    objmgr.opcodeTable[ CMSG_FORCE_FLY_SPEED_CHANGE_ACK ]       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           );

    objmgr.opcodeTable[ CMSG_MOUNTSPECIAL_ANIM ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMountSpecialAnimOpcode        );

    objmgr.opcodeTable[ CMSG_GROUP_INVITE ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGroupInviteOpcode             );
    //objmgr._opcodeTable[ CMSG_GROUP_CANCEL ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGroupCancelOpcode             );
    objmgr.opcodeTable[ CMSG_GROUP_ACCEPT ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGroupAcceptOpcode             );
    objmgr.opcodeTable[ CMSG_GROUP_DECLINE ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGroupDeclineOpcode            );
    objmgr.opcodeTable[ CMSG_GROUP_UNINVITE ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGroupUninviteNameOpcode       );
    objmgr.opcodeTable[ CMSG_GROUP_UNINVITE_GUID ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGroupUninviteGuidOpcode       );
    objmgr.opcodeTable[ CMSG_GROUP_SET_LEADER ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGroupSetLeaderOpcode          );
    objmgr.opcodeTable[ CMSG_GROUP_DISBAND ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGroupDisbandOpcode            );
    objmgr.opcodeTable[ CMSG_LOOT_METHOD ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLootMethodOpcode              );
    objmgr.opcodeTable[ CMSG_LOOT_ROLL ]                        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLootRoll                      );
    objmgr.opcodeTable[ CMSG_REQUEST_PARTY_MEMBER_STATS ]       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleRequestPartyMemberStatsOpcode );
    objmgr.opcodeTable[ CMSG_REQUEST_RAID_INFO ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleRequestRaidInfoOpcode         );
    objmgr.opcodeTable[ MSG_RAID_ICON_TARGET ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleRaidIconTargetOpcode          );
    objmgr.opcodeTable[ CMSG_GROUP_RAID_CONVERT ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleRaidConvertOpcode             );
    objmgr.opcodeTable[ MSG_RAID_READY_CHECK ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleRaidReadyCheckOpcode          );
    objmgr.opcodeTable[ CMSG_GROUP_CHANGE_SUB_GROUP ]           = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGroupChangeSubGroupOpcode     );
    objmgr.opcodeTable[ CMSG_GROUP_ASSISTANT ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGroupAssistantOpcode          );
    objmgr.opcodeTable[ CMSG_GROUP_PROMOTE ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGroupPromoteOpcode            );

    objmgr.opcodeTable[ CMSG_PETITION_SHOWLIST ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetitionShowListOpcode        );
    objmgr.opcodeTable[ CMSG_PETITION_BUY ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetitionBuyOpcode             );
    objmgr.opcodeTable[ CMSG_PETITION_SHOW_SIGNATURES ]         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetitionShowSignOpcode        );
    objmgr.opcodeTable[ CMSG_PETITION_QUERY ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetitionQueryOpcode           );
    objmgr.opcodeTable[ MSG_PETITION_RENAME ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetitionRenameOpcode          );
    objmgr.opcodeTable[ CMSG_PETITION_SIGN ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetitionSignOpcode            );
    objmgr.opcodeTable[ MSG_PETITION_DECLINE ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetitionDeclineOpcode         );
    objmgr.opcodeTable[ CMSG_OFFER_PETITION ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleOfferPetitionOpcode           );
    objmgr.opcodeTable[ CMSG_TURN_IN_PETITION ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTurnInPetitionOpcode          );

    objmgr.opcodeTable[ CMSG_GUILD_QUERY ]                      = OpcodeHandler( STATUS_AUTHED,   &WorldSession::HandleGuildQueryOpcode              );
    objmgr.opcodeTable[ CMSG_GUILD_CREATE ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildCreateOpcode             );
    objmgr.opcodeTable[ CMSG_GUILD_INVITE ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildInviteOpcode             );
    objmgr.opcodeTable[ CMSG_GUILD_REMOVE ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildRemoveOpcode             );
    objmgr.opcodeTable[ CMSG_GUILD_ACCEPT ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildAcceptOpcode             );
    objmgr.opcodeTable[ CMSG_GUILD_DECLINE ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildDeclineOpcode            );
    objmgr.opcodeTable[ CMSG_GUILD_INFO ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildInfoOpcode               );
    objmgr.opcodeTable[ CMSG_GUILD_ROSTER ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildRosterOpcode             );
    objmgr.opcodeTable[ CMSG_GUILD_PROMOTE ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildPromoteOpcode            );
    objmgr.opcodeTable[ CMSG_GUILD_DEMOTE ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildDemoteOpcode             );
    objmgr.opcodeTable[ CMSG_GUILD_LEAVE ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildLeaveOpcode              );
    objmgr.opcodeTable[ CMSG_GUILD_DISBAND ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildDisbandOpcode            );
    objmgr.opcodeTable[ CMSG_GUILD_LEADER ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildLeaderOpcode             );
    objmgr.opcodeTable[ CMSG_GUILD_MOTD ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildMOTDOpcode               );
    objmgr.opcodeTable[ CMSG_GUILD_SET_PUBLIC_NOTE ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildSetPublicNoteOpcode      );
    objmgr.opcodeTable[ CMSG_GUILD_SET_OFFICER_NOTE ]           = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildSetOfficerNoteOpcode     );
    objmgr.opcodeTable[ CMSG_GUILD_RANK ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildRankOpcode               );
    objmgr.opcodeTable[ CMSG_GUILD_ADD_RANK ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildAddRankOpcode            );
    objmgr.opcodeTable[ CMSG_GUILD_DEL_RANK ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildDelRankOpcode            );
    objmgr.opcodeTable[ CMSG_GUILD_CHANGEINFO ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildChangeInfoOpcode         );
    objmgr.opcodeTable[ MSG_SAVE_GUILD_EMBLEM ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGuildSaveEmblemOpcode         );

    objmgr.opcodeTable[ CMSG_TAXINODE_STATUS_QUERY ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTaxiNodeStatusQueryOpcode     );
    objmgr.opcodeTable[ CMSG_TAXIQUERYAVAILABLENODES ]          = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTaxiQueryAvailableNodesOpcode );
    objmgr.opcodeTable[ CMSG_ACTIVATETAXI ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleActivateTaxiOpcode            );
    objmgr.opcodeTable[ CMSG_ACTIVATETAXI_FAR ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleActivateTaxiFarOpcode         );
    objmgr.opcodeTable[ CMSG_MOVE_SPLINE_DONE ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTaxiNextDestinationOpcode     );

    objmgr.opcodeTable[ MSG_TABARDVENDOR_ACTIVATE ]             = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTabardVendorActivateOpcode    );
    objmgr.opcodeTable[ CMSG_BANKER_ACTIVATE ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBankerActivateOpcode          );
    objmgr.opcodeTable[ CMSG_BUY_BANK_SLOT ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBuyBankSlotOpcode             );
    objmgr.opcodeTable[ CMSG_TRAINER_LIST ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTrainerListOpcode             );
    objmgr.opcodeTable[ CMSG_TRAINER_BUY_SPELL ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTrainerBuySpellOpcode         );
    objmgr.opcodeTable[ MSG_AUCTION_HELLO ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAuctionHelloOpcode            );
    objmgr.opcodeTable[ CMSG_GOSSIP_HELLO ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGossipHelloOpcode             );
    objmgr.opcodeTable[ CMSG_GOSSIP_SELECT_OPTION ]             = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGossipSelectOptionOpcode      );
    objmgr.opcodeTable[ CMSG_SPIRIT_HEALER_ACTIVATE ]           = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSpiritHealerActivateOpcode    );
    objmgr.opcodeTable[ CMSG_NPC_TEXT_QUERY ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleNpcTextQueryOpcode            );
    objmgr.opcodeTable[ CMSG_BINDER_ACTIVATE ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBinderActivateOpcode          );
    objmgr.opcodeTable[ MSG_LIST_STABLED_PETS ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleListStabledPetsOpcode         );

    objmgr.opcodeTable[ CMSG_DUEL_ACCEPTED ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleDuelAcceptedOpcode            );
    objmgr.opcodeTable[ CMSG_DUEL_CANCELLED ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleDuelCancelledOpcode           );

    objmgr.opcodeTable[ CMSG_ACCEPT_TRADE ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAcceptTradeOpcode             );
    objmgr.opcodeTable[ CMSG_BEGIN_TRADE ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBeginTradeOpcode              );
    objmgr.opcodeTable[ CMSG_BUSY_TRADE ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBusyTradeOpcode               );
                                                            // sended after logout complete
    objmgr.opcodeTable[ CMSG_CANCEL_TRADE ]                     = OpcodeHandler( STATUS_AUTHED,   &WorldSession::HandleCancelTradeOpcode             );
    objmgr.opcodeTable[ CMSG_CLEAR_TRADE_ITEM ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleClearTradeItemOpcode          );
    objmgr.opcodeTable[ CMSG_IGNORE_TRADE ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleIgnoreTradeOpcode             );
    objmgr.opcodeTable[ CMSG_INITIATE_TRADE ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleInitiateTradeOpcode           );
    objmgr.opcodeTable[ CMSG_SET_TRADE_GOLD ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetTradeGoldOpcode            );
    objmgr.opcodeTable[ CMSG_SET_TRADE_ITEM ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetTradeItemOpcode            );
    objmgr.opcodeTable[ CMSG_UNACCEPT_TRADE ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleUnacceptTradeOpcode           );

    objmgr.opcodeTable[ CMSG_SPLIT_ITEM ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSplitItemOpcode               );
    objmgr.opcodeTable[ CMSG_SWAP_INV_ITEM ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSwapInvItemOpcode             );
    objmgr.opcodeTable[ CMSG_DESTROYITEM ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleDestroyItemOpcode             );
    objmgr.opcodeTable[ CMSG_AUTOEQUIP_ITEM ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAutoEquipItemOpcode           );
    objmgr.opcodeTable[ CMSG_ITEM_QUERY_SINGLE ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleItemQuerySingleOpcode         );
    objmgr.opcodeTable[ CMSG_SELL_ITEM ]                        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSellItemOpcode                );
    objmgr.opcodeTable[ CMSG_BUY_ITEM_IN_SLOT ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBuyItemInSlotOpcode           );
    objmgr.opcodeTable[ CMSG_BUY_ITEM ]                         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBuyItemOpcode                 );
    objmgr.opcodeTable[ CMSG_LIST_INVENTORY ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleListInventoryOpcode           );
    objmgr.opcodeTable[ CMSG_SWAP_ITEM ]                        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSwapItem                      );
    objmgr.opcodeTable[ CMSG_BUYBACK_ITEM ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBuybackItem                   );
    objmgr.opcodeTable[ CMSG_AUTOSTORE_BAG_ITEM ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAutoStoreBagItemOpcode        );
    objmgr.opcodeTable[ CMSG_AUTOBANK_ITEM ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAutoBankItemOpcode            );
    objmgr.opcodeTable[ CMSG_AUTOSTORE_BANK_ITEM ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAutoStoreBankItemOpcode       );
    objmgr.opcodeTable[ CMSG_ITEM_NAME_QUERY ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleItemNameQueryOpcode           );
    objmgr.opcodeTable[ CMSG_WRAP_ITEM ]                        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleWrapItemOpcode                );

    objmgr.opcodeTable[ CMSG_ATTACKSWING ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAttackSwingOpcode             );
    objmgr.opcodeTable[ CMSG_ATTACKSTOP ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAttackStopOpcode              );
    objmgr.opcodeTable[ CMSG_SETSHEATHED ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetSheathedOpcode             );

    objmgr.opcodeTable[ CMSG_USE_ITEM ]                         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleUseItemOpcode                 );
    objmgr.opcodeTable[ CMSG_OPEN_ITEM ]                        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleOpenItemOpcode                );
    objmgr.opcodeTable[ CMSG_CAST_SPELL ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleCastSpellOpcode               );
    objmgr.opcodeTable[ CMSG_CANCEL_CAST ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleCancelCastOpcode              );
    objmgr.opcodeTable[ CMSG_CANCEL_AURA ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleCancelAuraOpcode              );
    objmgr.opcodeTable[ CMSG_CANCEL_GROWTH_AURA ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleCancelGrowthAuraOpcode        );
    objmgr.opcodeTable[ CMSG_CANCEL_AUTO_REPEAT_SPELL ]         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleCancelAutoRepeatSpellOpcode   );

    objmgr.opcodeTable[ CMSG_LEARN_TALENT ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLearnTalentOpcode             );
    objmgr.opcodeTable[ MSG_TALENT_WIPE_CONFIRM ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTalentWipeOpcode              );
    objmgr.opcodeTable[ CMSG_UNLEARN_SKILL ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleUnlearnSkillOpcode            );

    objmgr.opcodeTable[ CMSG_QUESTGIVER_STATUS_QUERY ]          = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverStatusQueryOpcode   );
    objmgr.opcodeTable[ CMSG_QUESTGIVER_HELLO ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverHelloOpcode         );
    objmgr.opcodeTable[ CMSG_QUESTGIVER_ACCEPT_QUEST ]          = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverAcceptQuestOpcode   );
    objmgr.opcodeTable[ CMSG_QUESTGIVER_CHOOSE_REWARD ]         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverChooseRewardOpcode  );
    objmgr.opcodeTable[ CMSG_QUESTGIVER_REQUEST_REWARD ]        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverRequestRewardOpcode );
    objmgr.opcodeTable[ CMSG_QUESTGIVER_QUERY_QUEST ]           = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverQuestQueryOpcode    );
    objmgr.opcodeTable[ CMSG_QUEST_QUERY ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestQueryOpcode              );

    objmgr.opcodeTable[ CMSG_QUEST_CONFIRM_ACCEPT ]             = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestConfirmAccept            );
    objmgr.opcodeTable[ CMSG_QUESTLOG_REMOVE_QUEST ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestLogRemoveQuest           );
    objmgr.opcodeTable[ CMSG_QUESTLOG_SWAP_QUEST ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestLogSwapQuest             );
    objmgr.opcodeTable[ CMSG_QUESTGIVER_CANCEL ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverCancel              );
    objmgr.opcodeTable[ CMSG_QUESTGIVER_COMPLETE_QUEST ]        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestComplete                 );
    objmgr.opcodeTable[ CMSG_QUESTGIVER_QUEST_AUTOLAUNCH ]      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestAutoLaunch               );
    objmgr.opcodeTable[ CMSG_PUSHQUESTTOPARTY ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestPushToParty              );
    objmgr.opcodeTable[ MSG_QUEST_PUSH_RESULT ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleQuestPushResult               );

    objmgr.opcodeTable[ CMSG_TUTORIAL_FLAG ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTutorialFlag                  );
    objmgr.opcodeTable[ CMSG_TUTORIAL_CLEAR ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTutorialClear                 );
    objmgr.opcodeTable[ CMSG_TUTORIAL_RESET ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTutorialReset                 );

    objmgr.opcodeTable[ CMSG_MESSAGECHAT ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMessagechatOpcode             );
    objmgr.opcodeTable[ CMSG_TEXT_EMOTE ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTextEmoteOpcode               );
    objmgr.opcodeTable[ CMSG_CHAT_IGNORED ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChatIgnoredOpcode             );

    objmgr.opcodeTable[ CMSG_RECLAIM_CORPSE ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleCorpseReclaimOpcode           );
    objmgr.opcodeTable[ CMSG_RESURRECT_RESPONSE ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleResurrectResponseOpcode       );
    objmgr.opcodeTable[ CMSG_AUCTION_LIST_ITEMS ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAuctionListItems              );
    objmgr.opcodeTable[ CMSG_AUCTION_LIST_BIDDER_ITEMS ]        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAuctionListBidderItems        );
    objmgr.opcodeTable[ CMSG_AUCTION_SELL_ITEM ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAuctionSellItem               );
    objmgr.opcodeTable[ CMSG_AUCTION_REMOVE_ITEM ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAuctionRemoveItem             );
    objmgr.opcodeTable[ CMSG_AUCTION_LIST_OWNER_ITEMS ]         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAuctionListOwnerItems         );
    objmgr.opcodeTable[ CMSG_AUCTION_PLACE_BID ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAuctionPlaceBid               );

    objmgr.opcodeTable[ CMSG_JOIN_CHANNEL ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelJoin                   );
    objmgr.opcodeTable[ CMSG_LEAVE_CHANNEL ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelLeave                  );
    objmgr.opcodeTable[ CMSG_CHANNEL_LIST ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelList                   );
    objmgr.opcodeTable[ CMSG_CHANNEL_PASSWORD ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelPassword               );
    objmgr.opcodeTable[ CMSG_CHANNEL_SET_OWNER ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelSetOwner               );
    objmgr.opcodeTable[ CMSG_CHANNEL_OWNER ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelOwner                  );
    objmgr.opcodeTable[ CMSG_CHANNEL_MODERATOR ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelModerator              );
    objmgr.opcodeTable[ CMSG_CHANNEL_UNMODERATOR ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelUnmoderator            );
    objmgr.opcodeTable[ CMSG_CHANNEL_MUTE ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelMute                   );
    objmgr.opcodeTable[ CMSG_CHANNEL_UNMUTE ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelUnmute                 );
    objmgr.opcodeTable[ CMSG_CHANNEL_INVITE ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelInvite                 );
    objmgr.opcodeTable[ CMSG_CHANNEL_KICK ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelKick                   );
    objmgr.opcodeTable[ CMSG_CHANNEL_BAN ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelBan                    );
    objmgr.opcodeTable[ CMSG_CHANNEL_UNBAN ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelUnban                  );
    objmgr.opcodeTable[ CMSG_CHANNEL_ANNOUNCEMENTS ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelAnnounce               );
    objmgr.opcodeTable[ CMSG_CHANNEL_MODERATE ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelModerate               );
    objmgr.opcodeTable[ CMSG_CHANNEL_ROSTER_QUERY ]             = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelRosterQuery            );
    objmgr.opcodeTable[ CMSG_CHANNEL_INFO_QUERY ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelInfoQuery              );

    objmgr.opcodeTable[ CMSG_GET_MAIL_LIST ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleGetMail                       );
    objmgr.opcodeTable[ CMSG_ITEM_TEXT_QUERY ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleItemTextQuery                 );
    objmgr.opcodeTable[ CMSG_SEND_MAIL ]                        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSendMail                      );
    objmgr.opcodeTable[ CMSG_MAIL_TAKE_MONEY ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTakeMoney                     );
    objmgr.opcodeTable[ CMSG_MAIL_TAKE_ITEM ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleTakeItem                      );
    objmgr.opcodeTable[ CMSG_MAIL_MARK_AS_READ ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMarkAsRead                    );
    objmgr.opcodeTable[ CMSG_MAIL_RETURN_TO_SENDER ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleReturnToSender                );
    objmgr.opcodeTable[ CMSG_MAIL_DELETE ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMailDelete                    );
    objmgr.opcodeTable[ CMSG_MAIL_CREATE_TEXT_ITEM ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMailCreateTextItem            );
    objmgr.opcodeTable[ MSG_QUERY_NEXT_MAIL_TIME ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMsgQueryNextMailtime          );

    objmgr.opcodeTable[ CMSG_COMPLETE_CINEMATIC ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleCompleteCinema                );
    objmgr.opcodeTable[ CMSG_NEXT_CINEMATIC_CAMERA ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleNextCinematicCamera           );

    objmgr.opcodeTable[ CMSG_MOVE_TIME_SKIPPED ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMoveTimeSkippedOpcode         );

    objmgr.opcodeTable[ CMSG_PAGE_TEXT_QUERY ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePageQueryOpcode               );
    objmgr.opcodeTable[ CMSG_READ_ITEM ]                        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleReadItem                      );

    objmgr.opcodeTable[ CMSG_PET_ACTION ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetAction                     );
    objmgr.opcodeTable[ CMSG_PET_NAME_QUERY ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetNameQuery                  );
    objmgr.opcodeTable[ CMSG_REQUEST_PET_INFO ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleRequestPetInfoOpcode          );

    objmgr.opcodeTable[ CMSG_PET_ABANDON ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetAbandon                    );
    objmgr.opcodeTable[ CMSG_PET_SET_ACTION ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetSetAction                  );
    objmgr.opcodeTable[ CMSG_PET_RENAME ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetRename                     );
    objmgr.opcodeTable[ CMSG_STABLE_PET ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleStablePet                     );
    objmgr.opcodeTable[ CMSG_UNSTABLE_PET ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleUnstablePet                   );
    objmgr.opcodeTable[ CMSG_BUY_STABLE_SLOT ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBuyStableSlot                 );
    objmgr.opcodeTable[ CMSG_STABLE_REVIVE_PET ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleStableRevivePet               );
    objmgr.opcodeTable[ CMSG_STABLE_SWAP_PET ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleStableSwapPet                 );
    objmgr.opcodeTable[ CMSG_PET_CANCEL_AURA ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetCancelAuraOpcode           );
    objmgr.opcodeTable[ CMSG_PET_UNLEARN ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetUnlearnOpcode              );
    objmgr.opcodeTable[ CMSG_PET_SPELL_AUTOCAST ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetSpellAutocastOpcode        );
    objmgr.opcodeTable[ MSG_ADD_DYNAMIC_TARGET_OBSOLETE ]       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAddDynamicTargetObsoleteOpcode);

    objmgr.opcodeTable[ CMSG_CANCEL_CHANNELLING  ]              = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleCancelChanneling              );

    //BattleGround
    objmgr.opcodeTable[ CMSG_BATTLEFIELD_STATUS ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBattlefieldStatusOpcode       );
    objmgr.opcodeTable[ CMSG_BATTLEMASTER_HELLO ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundHelloOpcode       );
    objmgr.opcodeTable[ CMSG_BATTLEMASTER_JOIN ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundJoinOpcode        );
    objmgr.opcodeTable[ MSG_BATTLEGROUND_PLAYER_POSITIONS ]     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundPlayerPositionsOpcode );
    objmgr.opcodeTable[ MSG_PVP_LOG_DATA ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundPVPlogdataOpcode  );
    objmgr.opcodeTable[ CMSG_BATTLEFIELD_PORT ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundPlayerPortOpcode  );
    objmgr.opcodeTable[ CMSG_BATTLEFIELD_LIST ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundListOpcode        );
    objmgr.opcodeTable[ CMSG_LEAVE_BATTLEFIELD ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundLeaveOpcode       );
    objmgr.opcodeTable[ CMSG_ARENAMASTER_JOIN ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundArenaJoin         );

    objmgr.opcodeTable[ CMSG_SET_ACTIONBAR_TOGGLES ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetActionBar                  );
    objmgr.opcodeTable[ CMSG_FIELD_WATCHED_FACTION_SHOW_BAR ]   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetWatchedFactionIndexOpcode  );
    objmgr.opcodeTable[ CMSG_FIELD_WATCHED_FACTION_INACTIVE ]   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetWatchedFactionInactiveOpcode );

    objmgr.opcodeTable[ CMSG_LOOT_ROLL ]                        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLootRoll                      );
    objmgr.opcodeTable[ CMSG_WARDEN_DATA ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleWardenDataOpcode              );
    objmgr.opcodeTable[ CMSG_WORLD_TELEPORT ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleWorldTeleportOpcode           );
    objmgr.opcodeTable[ MSG_MINIMAP_PING ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMinimapPingOpcode             );
    objmgr.opcodeTable[ MSG_RANDOM_ROLL ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleRandomRollOpcode              );
    objmgr.opcodeTable[ CMSG_FAR_SIGHT ]                        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleFarSightOpcode                );

    objmgr.opcodeTable[ MSG_LOOKING_FOR_GROUP ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLookingForGroup               );
    objmgr.opcodeTable[ CMSG_SET_LOOKING_FOR_GROUP ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSetLfgOpcode                  );
    objmgr.opcodeTable[ MSG_SET_DUNGEON_DIFFICULTY ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleDungeonDifficultyOpcode       );
    objmgr.opcodeTable[ CMSG_MOVE_FLY_MODE_CHANGE_ACK ]         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMoveFlyModeChangeAckOpcode    );
    objmgr.opcodeTable[ CMSG_ARENA_TEAM_QUERY ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamQueryOpcode          );
    objmgr.opcodeTable[ MSG_MOVE_START_FLY_UP ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ MSG_MOVE_STOP_FLY_UP ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               );
    objmgr.opcodeTable[ CMSG_LFG_SET_AUTOJOIN ]                 = OpcodeHandler( STATUS_AUTHED,   &WorldSession::HandleLfgAutoJoinOpcode             );
    objmgr.opcodeTable[ CMSG_LFG_UNSET_AUTOJOIN ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLfgCancelAutoJoinOpcode       );
    objmgr.opcodeTable[ CMSG_LFM_SET_AUTOADD ]                  = OpcodeHandler( STATUS_AUTHED,   &WorldSession::HandleLfmAutoAddMembersOpcode       );
    objmgr.opcodeTable[ CMSG_LFM_UNSET_AUTOADD ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLfmCancelAutoAddmembersOpcode );
    objmgr.opcodeTable[ CMSG_LOOKING_FOR_GROUP_CLEAR ]          = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLfgClearOpcode                );
    objmgr.opcodeTable[ CMSG_SET_LOOKING_FOR_NONE ]             = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLfmSetNoneOpcode              );
    objmgr.opcodeTable[ CMSG_SET_LOOKING_FOR_MORE ]             = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLfmSetOpcode                  );
    objmgr.opcodeTable[ CMSG_SET_COMMENTARY ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleLfgSetCommentOpcode           );
    objmgr.opcodeTable[ CMSG_CHOOSE_TITLE ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChooseTitleOpcode             );
    objmgr.opcodeTable[ MSG_INSPECT_ARENA_STATS ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleInspectArenaStatsOpcode       );
    objmgr.opcodeTable[ CMSG_REALM_STATE_REQUEST ]              = OpcodeHandler( STATUS_AUTHED,   &WorldSession::HandleRealmStateRequestOpcode       );
    objmgr.opcodeTable[ CMSG_ALLOW_MOVE_ACK ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAllowMoveAckOpcode            );
    objmgr.opcodeTable[ CMSG_WHOIS ]                            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleWhoisOpcode                   );
    objmgr.opcodeTable[ CMSG_RESET_INSTANCES ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleResetInstancesOpcode          );
    objmgr.opcodeTable[ CMSG_ARENA_TEAM_ROSTER ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamRosterOpcode         );
    objmgr.opcodeTable[ CMSG_ARENA_TEAM_ADD_MEMBER ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamAddMemberOpcode      );
    objmgr.opcodeTable[ CMSG_ARENA_TEAM_INVITE_ACCEPT ]         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamInviteAcceptOpcode   );
    objmgr.opcodeTable[ CMSG_ARENA_TEAM_INVITE_DECLINE ]        = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamInviteDeclineOpcode  );
    objmgr.opcodeTable[ CMSG_ARENA_TEAM_LEAVE ]                 = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamLeaveOpcode          );
    objmgr.opcodeTable[ CMSG_ARENA_TEAM_REMOVE_FROM_TEAM ]      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamRemoveFromTeamOpcode );
    objmgr.opcodeTable[ CMSG_ARENA_TEAM_DISBAND ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamDisbandOpcode        );
    objmgr.opcodeTable[ CMSG_ARENA_TEAM_PROMOTE_TO_CAPTAIN ]    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamPromoteToCaptainOpcode );
    objmgr.opcodeTable[ CMSG_AREA_SPIRIT_HEALER_QUERY ]         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAreaSpiritHealerQueryOpcode   );
    objmgr.opcodeTable[ CMSG_AREA_SPIRIT_HEALER_QUEUE ]         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAreaSpiritHealerQueueOpcode   );
    objmgr.opcodeTable[ CMSG_DISMOUNT ]                         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleDismountOpcode                );
    objmgr.opcodeTable[ CMSG_SELF_RES ]                         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSelfResOpcode                 );
    objmgr.opcodeTable[ CMSG_SOCKET_ITEM ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSocketOpcode                  );
    objmgr.opcodeTable[ CMSG_CANCEL_TEMP_ITEM_ENCHANTMENT]      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleCancelTempItemEnchantmentOpcode);
    objmgr.opcodeTable[ CMSG_REPORT_SPAM ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleReportSpamOpcode              );

    objmgr.opcodeTable[ CMSG_CHANNEL_ENABLE_VOICE ]             = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleChannelEnableVoiceOpcode      );
    objmgr.opcodeTable[ CMSG_CHANNEL_VOICE_CHAT_QUERY ]         = OpcodeHandler( STATUS_AUTHED,   &WorldSession::HandleChannelVoiceChatQuery         );
    objmgr.opcodeTable[ CMSG_VOICE_SETTINGS ]                   = OpcodeHandler( STATUS_AUTHED,   &WorldSession::HandleVoiceSettingsOpcode           );
}

void WorldSession::SizeError(WorldPacket const& packet, uint32 size) const
{
    sLog.outError("Client (account %u) send packet %s (%u) with size %u but expected %u (attempt crash server?), skipped",
        GetAccountId(),LookupName(packet.GetOpcode(),g_worldOpcodeNames),packet.GetOpcode(),packet.size(),size);
}

/// Get the player name
char const* WorldSession::GetPlayerName() const
{
    return GetPlayer() ? GetPlayer()->GetName() : "<none>";
}

/// Set the WorldSocket associated with this session
void WorldSession::SetSocket(WorldSocket *sock)
{
    _socket = sock;
}

/// Send a packet to the client
void WorldSession::SendPacket(WorldPacket* packet)
{
    if (!_socket)
        return;
    #ifdef MANGOS_DEBUG
    // Code for network use statistic
    static uint64 sendPacketCount = 0;
    static uint64 sendPacketBytes = 0;

    static time_t firstTime = time(NULL);
    static time_t lastTime = firstTime;                     // next 60 secs start time

    static uint64 sendLastPacketCount = 0;
    static uint64 sendLastPacketBytes = 0;

    time_t cur_time = time(NULL);

    if((cur_time - lastTime) < 60)
    {
        sendPacketCount+=1;
        sendPacketBytes+=packet->size();

        sendLastPacketCount+=1;
        sendLastPacketBytes+=packet->size();
    }
    else
    {
        uint64 minTime = uint64(cur_time - lastTime);
        uint64 fullTime = uint64(lastTime - firstTime);
        sLog.outDetail("Send all time packets count: " I64FMTD " bytes: " I64FMTD " avr.count/sec: %f avr.bytes/sec: %f time: %u",sendPacketCount,sendPacketBytes,float(sendPacketCount)/fullTime,float(sendPacketBytes)/fullTime,uint32(fullTime));
        sLog.outDetail("Send last min packets count: " I64FMTD " bytes: " I64FMTD " avr.count/sec: %f avr.bytes/sec: %f",sendLastPacketCount,sendLastPacketBytes,float(sendLastPacketCount)/minTime,float(sendLastPacketBytes)/minTime);

        lastTime = cur_time;
        sendLastPacketCount = 1;
        sendLastPacketBytes = packet->wpos();               // wpos is real written size
    }
    #endif                                                  // !MANGOS_DEBUG

    _socket->SendPacket(packet);
}

/// Add an incoming packet to the queue
void WorldSession::QueuePacket(WorldPacket& packet)
{
    WorldPacket *pck = new WorldPacket(packet);
    _recvQueue.add(pck);
}

/// Update the WorldSession (triggered by World update)
bool WorldSession::Update(uint32 diff)
{
    WorldPacket *packet;

    ///- Retrieve packets from the receive queue and call the appropriate handlers
    /// \todo Is there a way to consolidate the OpcondeHandlerTable and the g_worldOpcodeNames to only maintain 1 list?
    /// answer : there is a way, but this is better, because it would use redundant RAM
    while (!_recvQueue.empty())
    {
        packet = _recvQueue.next();

        /*#if 1
        sLog.outError( "MOEP: %s (0x%.4X)",
                        LookupName(packet->GetOpcode(), g_worldOpcodeNames),
                        packet->GetOpcode());
        #endif*/
        OpcodeTableMap::const_iterator iter = objmgr.opcodeTable.find( packet->GetOpcode() );

        if (iter == objmgr.opcodeTable.end())
        {
            sLog.outError( "SESSION: received unhandled opcode %s (0x%.4X)",
                LookupName(packet->GetOpcode(), g_worldOpcodeNames),
                packet->GetOpcode());
        }
        else
        {
            if (iter->second.status == STATUS_LOGGEDIN && _player)
            {
                (this->*iter->second.handler)(*packet);
            }
            else if (iter->second.status == STATUS_AUTHED)
            {
                m_playerRecentlyLogout = false;
                (this->*iter->second.handler)(*packet);
            }
            else
                // skip STATUS_LOGGEDIN opcode unexpected errors if player logout sometime ago - this can be network lag delayed packets
            if(!m_playerRecentlyLogout)
            {
                sLog.outError( "SESSION: received unexpected opcode %s (0x%.4X)",
                    LookupName(packet->GetOpcode(), g_worldOpcodeNames),
                    packet->GetOpcode());
            }
        }

        delete packet;
    }

    ///- If necessary, log the player out
    time_t currTime = time(NULL);
    if (!_socket || (ShouldLogOut(currTime) && !m_playerLoading))
        LogoutPlayer(true);

    if (!_socket)
        return false;                                       //Will remove this session from the world session map

    return true;
}

/// %Log the player out
void WorldSession::LogoutPlayer(bool Save)
{
    m_playerLogout = true;

    if (_player)
    {
        ///- If the player just died before logging out, make him appear as a ghost
        //FIXME: logout must be delayed in case lost connection with client in time of combat
        if (_player->GetDeathTimer())
        {
            _player->getHostilRefManager().deleteReferences();
            _player->BuildPlayerRepop();
        }
        else if (_player->isAttacked())
        {
            _player->CombatStop(true);
            _player->getHostilRefManager().setOnlineOfflineState(false);
            _player->KillPlayer();
            _player->BuildPlayerRepop();

            // build set of player who attack _player or who have pet attacking of _player
            std::set<Player*> aset;
            for(Unit::AttackerSet::const_iterator itr = _player->getAttackers().begin(); itr != _player->getAttackers().end(); ++itr)
            {
                Unit* owner = (*itr)->GetOwner();           // including player controlled case
                if(owner)
                {
                    if(owner->GetTypeId()==TYPEID_PLAYER)
                        aset.insert((Player*)owner);
                }
                else
                if((*itr)->GetTypeId()==TYPEID_PLAYER)
                    aset.insert((Player*)(*itr));
            }
            // give honor to all attackers from set like group case
            for(std::set<Player*>::const_iterator itr = aset.begin(); itr != aset.end(); ++itr)
                (*itr)->RewardHonor(_player,aset.size());
        }

        ///- Remove player from battleground (teleport to entrance)
        if(_player->InBattleGround())
            _player->LeaveBattleground();

        if(_player->InBattleGroundQueue())
        {
            for (int i=0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
            {
                int32 bgTypeId = _player->GetBattleGroundQueueId(i);
                if (bgTypeId != 0)
                {
                    _player->RemoveBattleGroundQueueId(bgTypeId);
                    sBattleGroundMgr.m_BattleGroundQueues[ bgTypeId ].RemovePlayer(_player->GetGUID(), true);
                }
            }
        }

        ///- Reset the online field in the account table
        // no point resetting online in character table here as Player::SaveToDB() will set it to 1 since player has not been removed from world at this stage
        //No SQL injection as AccountID is uint32
        loginDatabase.PExecute("UPDATE `account` SET `online` = 0 WHERE `id` = '%u'", GetAccountId());

        ///- If the player is in a guild, update the guild roster and broadcast a logout message to other guild members
        Guild *guild = objmgr.GetGuildById(_player->GetGuildId());
        if(guild)
        {
            guild->LoadPlayerStatsByGuid(_player->GetGUID());

            WorldPacket data(SMSG_GUILD_EVENT, (5+12));     // name limited to 12 in character table.
            data<<(uint8)GE_SIGNED_OFF;
            data<<(uint8)1;
            data<<_player->GetName();
            data<<(uint8)0<<(uint8)0<<(uint8)0;
            guild->BroadcastPacket(&data);
        }

        ///- Release charmed creatures and unsummon totems
        _player->Uncharm();
        _player->UnsummonAllTotems();
        _player->RemovePet(NULL,PET_SAVE_AS_CURRENT, true);

        ///- empty buyback items and save the player in the database
        // some save parts only correctly work in case player present in map/player_lists (pets, etc)
        if(Save)
        {
            uint32 eslot;
            for(int j = BUYBACK_SLOT_START; j < BUYBACK_SLOT_END; j++)
            {
                eslot = j - BUYBACK_SLOT_START;
                _player->SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1+eslot*2,0);
                _player->SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1+eslot,0);
                _player->SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1+eslot,0);
            }
            _player->SaveToDB();
        }

        ///- Leave all channels before player delete...
        _player->CleanupChannels();

        ///- If the player is in a group (or invited), remove him. If the group if then only 1 person, disband the group.
        _player->UninviteFromGroup();

        // remove player from the group if he is:
        // a) in group; b) not in raid group; c) logging out normally (not being kicked or disconnected)
        if(_player->GetGroup() && !_player->GetGroup()->isRaidGroup() && _socket)
            _player->RemoveFromGroup();

        ///- Remove the player from the world
        ObjectAccessor::Instance().RemoveObject(_player);
        MapManager::Instance().GetMap(_player->GetMapId(), _player)->Remove(_player, false);

        ///- Send update to group
        if(_player->GetGroup())
            _player->GetGroup()->SendUpdate();

        ///- Broadcast a logout message to the player's friends
        WorldPacket data(SMSG_FRIEND_STATUS, 9);
        data<<uint8(FRIEND_OFFLINE);
        data<<_player->GetGUID();
        _player->BroadcastPacketToFriendListers(&data);

        ///- Delete the player object
        _player->CleanupsBeforeDelete();                    // do some cleanup before deleting to prevent crash at crossreferences to already deleted data
        _player->TradeCancel(false);

        delete _player;
        _player = 0;

        ///- Send the 'logout complete' packet to the client
        data.Initialize( SMSG_LOGOUT_COMPLETE, 0 );
        SendPacket( &data );

        ///- Since each account can only have one online character at any given time, ensure all characters for active account are marked as offline
        //No SQL injection as AccountId is uint32
        sDatabase.PExecute("UPDATE `character` SET `online` = 0 WHERE `account` = '%u'", GetAccountId());
        sLog.outDebug( "SESSION: Sent SMSG_LOGOUT_COMPLETE Message" );
    }

    m_playerLogout = false;
    m_playerRecentlyLogout = true;
    LogoutRequest(0);
}

/// Kick a player out of the World
void WorldSession::KickPlayer()
{
    if(!_socket)
        return;

    // player will be logout and session will removed in next update tick
    _socket->CloseSocket();
    _socket = NULL;
}

/// Cancel channeling handler
/// \todo Complete HandleCancelChanneling function
void WorldSession::HandleCancelChanneling( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 4);

    uint32 spellid;
    recv_data >> spellid;
}

void WorldSession::HandleFarSightOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 1);

    sLog.outDebug("WORLD: CMSG_FAR_SIGHT");
    //recv_data.hexlike();

    uint8 unk;
    recv_data >> unk;

    switch(unk)
    {
        case 0:
            //WorldPacket data(SMSG_CLEAR_FAR_SIGHT_IMMEDIATE, 0)
            //SendPacket(&data);
            //_player->SetUInt64Value(PLAYER_FARSIGHT, 0);
            sLog.outDebug("Removed FarSight from player %u", _player->GetGUIDLow());
            break;
        case 1:
            sLog.outDebug("Added FarSight " I64FMTD " to player %u", _player->GetUInt64Value(PLAYER_FARSIGHT), _player->GetGUIDLow());
            break;
    }
}

void WorldSession::SendAreaTriggerMessage(const char* Text, ...)
{
    va_list ap;
    char szStr [1024];
    szStr[0] = '\0';

    va_start(ap, Text);
    vsnprintf( szStr, 1024, Text, ap );
    va_end(ap);

    uint32 length = strlen(szStr)+1;
    WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 4+length);
    data << length;
    data << szStr;
    SendPacket(&data);
}

void WorldSession::HandleDungeonDifficultyOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 4);

    sLog.outDebug("MSG_SET_DUNGEON_DIFFICULTY");

    uint32 difficulty;
    recv_data >> difficulty;

    GetPlayer()->SetDungeonDifficulty(difficulty);
    GetPlayer()->SendDungeonDifficulty();
}

void WorldSession::HandleChooseTitleOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 4);

    sLog.outDebug("CMSG_CHOOSE_TITLE");

    uint32 title;
    recv_data >> title;

    uint32 available = GetPlayer()->GetUInt32Value(PLAYER__FIELD_KNOWN_TITLES);
    if(!(available & (1<<title)))
        return;

    GetPlayer()->SetUInt32Value(PLAYER_CHOSEN_TITLE, title);
    /*
    PLAYER_FIELD_KNOWN_TITLES:
    1 << title# (0=none)
    */
}

void WorldSession::HandleNewUnknownOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("New Unknown Opcode %u", recv_data.GetOpcode());
    recv_data.hexlike();
    /*
    New Unknown Opcode 837
    STORAGE_SIZE: 60
    02 00 00 00 00 00 00 00 | 00 00 00 00 01 20 00 00
    89 EB 33 01 71 5C 24 C4 | 15 03 35 45 74 47 8B 42
    BA B8 1B 40 00 00 00 00 | 00 00 00 00 77 66 42 BF
    23 91 26 3F 00 00 60 41 | 00 00 00 00

    New Unknown Opcode 837
    STORAGE_SIZE: 44
    02 00 00 00 00 00 00 00 | 00 00 00 00 00 00 80 00
    7B 80 34 01 84 EA 2B C4 | 5F A1 36 45 C9 39 1C 42
    BA B8 1B 40 CE 06 00 00 | 00 00 80 3F
    */
}

void WorldSession::HandleRealmStateRequestOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 4);

    sLog.outDebug("CMSG_REALM_STATE_REQUEST");

    uint32 unk;
    std::string split_date = "01/01/01";
    recv_data >> unk;

    WorldPacket data(SMSG_REALM_STATE_RESPONSE, 4+4+split_date.size()+1);
    data << unk;
    data << uint32(0x00000000);                             // realm split state
    // split states:
    // 0x0 realm normal
    // 0x1 realm split
    // 0x2 realm split pending
    data << split_date;
    SendPacket(&data);
    sLog.outDebug("response sent %u", unk);
}

void WorldSession::HandleAllowMoveAckOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 4+4);

    sLog.outDebug("CMSG_ALLOW_MOVE_ACK");

    uint32 counter, time_;
    recv_data >> counter >> time_;

    // time_ seems always more than getMSTime()
    uint32 diff = time_ - getMSTime();

    sLog.outDebug("response sent: counter %u, time %u (HEX: %X), ms. time %u, diff %u", counter, time_, time_, getMSTime(), diff);
}

void WorldSession::HandleWhoisOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 1);

    sLog.outDebug("Received opcode CMSG_WHOIS");
    std::string charname, acc, email, lastip, msg;
    recv_data >> charname;

    if (GetSecurity() < SEC_ADMINISTRATOR)
    {
        SendNotification("You do not have permission to perform that function");
        return;
    }

    if(charname.empty())
    {
        SendNotification("Please provide character name");
        return;
    }

    uint32 accid;
    Field *fields;

    Player *plr = objmgr.GetPlayer(charname.c_str());

    if(plr)
        accid = plr->GetSession()->GetAccountId();
    else
    {
        SendNotification("Player %s not found or offline", charname.c_str());
        return;
    }

    if(!accid)
    {
        SendNotification("Account for character %s not found", charname.c_str());
        return;
    }

    QueryResult *result = loginDatabase.PQuery("SELECT `username`,`email`,`last_ip` FROM `account` WHERE `id`=%u", accid);
    if(result)
    {
        fields = result->Fetch();
        acc = fields[0].GetCppString();
        if(acc.empty())
            acc = "Unknown";
        email = fields[1].GetCppString();
        if(email.empty())
            email = "Unknown";
        lastip = fields[2].GetCppString();
        if(lastip.empty())
            lastip = "Unknown";
        msg = charname + "'s " + "account is " + acc + ", e-mail: " + email + ", last ip: " + lastip;

        WorldPacket data(SMSG_WHOIS, msg.size()+1);
        data << msg;
        _player->GetSession()->SendPacket(&data);
    }
    else
        SendNotification("Account for character %s not found", charname.c_str());

    delete result;
    sLog.outDebug("Received whois command from player %s for character %s", GetPlayer()->GetName(), charname.c_str());
}

void WorldSession::HandleResetInstancesOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_RESET_INSTANCES");
    /*
        uint32 mapid = 0;
        WorldPacket data(SMSG_RESET_INSTANCES_SUCCESS, 4);
        data << mapid;
        _player->GetSession()->SendPacket(&data);
    */
}

void WorldSession::HandleDismountOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_DISMOUNT");
    //recv_data.hexlike();

    //If player is not mounted, so go out :)
    if (!_player->IsMounted())                              // not blizz like; no any messages on blizz
    {
        sChatHandler.SendSysMessage(this, LANG_CHAR_NON_MOUNTED);
        return;
    }

    if(_player->isInFlight())                               // not blizz like; no any messages on blizz
    {
        sChatHandler.SendSysMessage(this, LANG_YOU_IN_FLIGHT);
        return;
    }

    _player->Unmount();
    _player->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
}

void WorldSession::HandleMoveFlyModeChangeAckOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 8+4+4);

    // fly mode on/off
    sLog.outDebug("WORLD: CMSG_MOVE_FLY_MODE_CHANGE_ACK");
    //recv_data.hexlike();

    uint64 guid;
    uint32 unk;
    uint32 flags;

    recv_data >> guid >> unk >> flags;

    _player->SetMovementFlags(flags);
    /*
    on:
    25 00 00 00 00 00 00 00 | 00 00 00 00 00 00 80 00
    85 4E A9 01 19 BA 7A C3 | 42 0D 70 44 44 B0 A8 42
    78 15 94 40 39 03 00 00 | 00 00 80 3F
    off:
    25 00 00 00 00 00 00 00 | 00 00 00 00 00 00 00 00
    10 FD A9 01 19 BA 7A C3 | 42 0D 70 44 44 B0 A8 42
    78 15 94 40 39 03 00 00 | 00 00 00 00
    */
}

void WorldSession::HandleSelfResOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_SELF_RES");                  // empty opcode

    if(_player->GetUInt32Value(PLAYER_SELF_RES_SPELL))
    {
        SpellEntry const *spellInfo = sSpellStore.LookupEntry(_player->GetUInt32Value(PLAYER_SELF_RES_SPELL));
        if(spellInfo)
            _player->CastSpell(_player,spellInfo,false,0);

        _player->SetUInt32Value(PLAYER_SELF_RES_SPELL, 0);
    }
}

void WorldSession::HandleReportSpamOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 1+8);
    sLog.outDebug("WORLD: CMSG_REPORT_SPAM");
    recv_data.hexlike();

    uint8 spam_type;                                        // 0 - mail, 1 - chat
    uint64 spammer_guid;
    uint32 unk1, unk2, unk3, unk4 = 0;
    std::string description = "";
    recv_data >> spam_type;                                 // unk 0x01 const, may be spam type (mail/chat)
    recv_data >> spammer_guid;                              // player guid
    switch(spam_type)
    {
        case 0:
            CHECK_PACKET_SIZE(recv_data, recv_data.rpos()+4+4+4);
            recv_data >> unk1;                              // unk
            recv_data >> unk2;                              // probably mail id
            recv_data >> unk3;                              // unk
            break;
        case 1:
            CHECK_PACKET_SIZE(recv_data, recv_data.rpos()+4+4+4+4+1);
            recv_data >> unk1;                              // probably language
            recv_data >> unk2;                              // message type?
            recv_data >> unk3;                              // probably channel id
            recv_data >> unk4;                              // unk random value
            recv_data >> description;                       // spam description string (messagetype, channel name, player name, message)
            break;
    }

    // NOTE: all chat messages from this spammer automatically ignored by spam reporter until logout in case chat spam.
    // if it's mail spam - ALL mails from this spammer automatically removed by client

    // Complaint Received message
    WorldPacket data(SMSG_REPORT_SPAM_RESPONSE, 1);
    data << uint8(0);
    SendPacket(&data);

    sLog.outDebug("REPORT SPAM: type %u, guid %u, unk2 %u, unk3, %u, unk4 %u, unk5 %u, message %s", spam_type, GUID_LOPART(spammer_guid), unk1, unk2, unk3, unk4, description.c_str());
}

void WorldSession::HandleRequestPetInfoOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_REQUEST_PET_INFO");
    recv_data.hexlike();
}
