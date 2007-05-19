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

/// WorldSession constructor
WorldSession::WorldSession(uint32 id, WorldSocket *sock, uint32 sec) : _player(NULL), _socket(sock),
_security(sec), _accountId(id), _logoutTime(0), m_playerLoading(false), m_playerRecentlyLogout(false)
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

        // charater view
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
    objmgr.opcodeTable[ MSG_MOVE_WORLDPORT_ACK ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMoveWorldportAckOpcode        );
    objmgr.opcodeTable[ CMSG_FORCE_RUN_SPEED_CHANGE_ACK ]       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           );
    objmgr.opcodeTable[ CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK ]  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck       );
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
    objmgr.opcodeTable[ CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK ] = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck      );
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

    objmgr.opcodeTable[ CMSG_PET_ABANDON ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetAbandon                    );
    objmgr.opcodeTable[ CMSG_PET_SET_ACTION ]                   = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetSetAction                  );
    objmgr.opcodeTable[ CMSG_PET_RENAME ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandlePetRename                     );
    objmgr.opcodeTable[ CMSG_STABLE_PET ]                       = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleStablePet                     );
    objmgr.opcodeTable[ CMSG_UNSTABLE_PET ]                     = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleUnstablePet                   );
    objmgr.opcodeTable[ CMSG_BUY_STABLE_SLOT ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleBuyStableSlot                 );
    objmgr.opcodeTable[ CMSG_STABLE_REVIVE_PET ]                = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleStableRevivePet               );
    objmgr.opcodeTable[ CMSG_STABLE_SWAP_PET ]                  = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleStableSwapPet                 );

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
    objmgr.opcodeTable[ CMSG_ARENA_TEAM_DISBAND ]               = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamDisbandOpcode        );
    objmgr.opcodeTable[ CMSG_AREA_SPIRIT_HEALER_QUERY ]         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleAreaSpiritHealerQueryOpcode   );
    objmgr.opcodeTable[ CMSG_MOVE_SHIP_909 ]                    = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMoveShipOpcode                );
    objmgr.opcodeTable[ CMSG_MOVE_FLY_STATE_CHANGE ]            = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleMoveFlyStateChangeOpcode      );
    objmgr.opcodeTable[ CMSG_DISMOUNT ]                         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleDismountOpcode                );
    objmgr.opcodeTable[ CMSG_SELF_RES ]                         = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSelfResOpcode                 );
    objmgr.opcodeTable[ CMSG_SOCKET_ITEM ]                      = OpcodeHandler( STATUS_LOGGEDIN, &WorldSession::HandleSocketOpcode                  );
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
    if (_socket)
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
    /// answer : there is a way, but this is better, because it would use redundand RAM
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
    if (_player)
    {
        ///- If the player just died before logging out, make him appear as a ghost
        //FIXME: logout must be delayed in case lost connection with client in time of combat
        if (_player->GetDeathTimer() || _player->isAttacked())
        {
            _player->CombatStop(true);
            _player->DeleteInHateListOf();
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
            // give honor to all attackers from set
            for(std::set<Player*>::const_iterator itr = aset.begin(); itr != aset.end(); ++itr)
                (*itr)->CalculateHonor(_player);
        }

        ///- Remove player from battleground (teleport to entrance)
        if(_player->InBattleGround())
        {
            BattleGround* bg = sBattleGroundMgr.GetBattleGround(_player->GetBattleGroundId());
            if(bg)
                bg->RemovePlayer(_player->GetGUID(), true, true);
        }
        if(_player->InBattleGroundQueue())
        {
            BattleGround* bg = sBattleGroundMgr.GetBattleGround(_player->GetBattleGroundQueueId());
            if(bg)
                bg->RemovePlayerFromQueue(_player->GetGUID());
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
        _player->UnsummonTotem();
        _player->InvisiblePjsNear.clear();

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

        ///- Remove the player's pet from the world
        _player->RemovePet(NULL,PET_SAVE_AS_CURRENT);

        ///- If the player is in a group (or invited), remove him. If the group if then only 1 person, disband the group.
        _player->UninviteFromGroup();

        // remove player from the group if he is:
        // a) in group; b) not in raid group; c) logging out normally (not being kicked or disconnected)
        if(_player->groupInfo.group && !_player->groupInfo.group->isRaidGroup() && _socket)
            _player->RemoveFromGroup();

        ///- Remove the player from the world
        ObjectAccessor::Instance().RemovePlayer(_player);
        MapManager::Instance().GetMap(_player->GetMapId(), _player)->Remove(_player, false);

        ///- Send update to group
        if(_player->groupInfo.group)
            _player->groupInfo.group->SendUpdate();

        ///- Broadcast a logout message to the player's friends
        WorldPacket data(SMSG_FRIEND_STATUS, 9);
        data<<uint8(FRIEND_OFFLINE);
        data<<_player->GetGUID();
        _player->BroadcastPacketToFriendListers(&data);

        ///- Delete the player object
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

    m_playerRecentlyLogout = true;
    LogoutRequest(0);
}

/// Kick a player out of the World
void WorldSession::KickPlayer()
{
    if(!_socket)
        return;

    // player will be  logout and session will removed in next update tick
    _socket->CloseSocket();
    _socket = NULL;
}
/*
/// Return the Opcode handler table
OpcodeHandler* WorldSession::_GetOpcodeHandlerTable() const
{
    static OpcodeHandler table[] =
    {

        { CMSG_CHAR_ENUM,                   STATUS_AUTHED,   &WorldSession::HandleCharEnumOpcode                },
        { CMSG_CHAR_CREATE,                 STATUS_AUTHED,   &WorldSession::HandleCharCreateOpcode              },
        { CMSG_CHAR_DELETE,                 STATUS_AUTHED,   &WorldSession::HandleCharDeleteOpcode              },
        { CMSG_PLAYER_LOGIN,                STATUS_AUTHED,   &WorldSession::HandlePlayerLoginOpcode             },
        { CMSG_CHAR_RENAME,                 STATUS_AUTHED,   &WorldSession::HandleChangePlayerNameOpcode        },

        { CMSG_SET_ACTION_BUTTON,           STATUS_LOGGEDIN, &WorldSession::HandleSetActionButtonOpcode         },
        { CMSG_REPOP_REQUEST,               STATUS_LOGGEDIN, &WorldSession::HandleRepopRequestOpcode            },
        { CMSG_AUTOSTORE_LOOT_ITEM,         STATUS_LOGGEDIN, &WorldSession::HandleAutostoreLootItemOpcode       },
        { CMSG_LOOT_MONEY,                  STATUS_LOGGEDIN, &WorldSession::HandleLootMoneyOpcode               },
        { CMSG_LOOT,                        STATUS_LOGGEDIN, &WorldSession::HandleLootOpcode                    },
        { CMSG_LOOT_RELEASE,                STATUS_LOGGEDIN, &WorldSession::HandleLootReleaseOpcode             },
        { CMSG_WHO,                         STATUS_LOGGEDIN, &WorldSession::HandleWhoOpcode                     },
        { CMSG_LOGOUT_REQUEST,              STATUS_LOGGEDIN, &WorldSession::HandleLogoutRequestOpcode           },
        { CMSG_PLAYER_LOGOUT,               STATUS_LOGGEDIN, &WorldSession::HandlePlayerLogoutOpcode            },
        { CMSG_LOGOUT_CANCEL,               STATUS_LOGGEDIN, &WorldSession::HandleLogoutCancelOpcode            },
        { CMSG_GMTICKET_GETTICKET,          STATUS_LOGGEDIN, &WorldSession::HandleGMTicketGetTicketOpcode       },
        { CMSG_GMTICKET_CREATE,             STATUS_LOGGEDIN, &WorldSession::HandleGMTicketCreateOpcode          },
        { CMSG_GMTICKET_SYSTEMSTATUS,       STATUS_LOGGEDIN, &WorldSession::HandleGMTicketSystemStatusOpcode    },
        { CMSG_GMTICKET_DELETETICKET,       STATUS_LOGGEDIN, &WorldSession::HandleGMTicketDeleteOpcode          },
        { CMSG_GMTICKET_UPDATETEXT,         STATUS_LOGGEDIN, &WorldSession::HandleGMTicketUpdateTextOpcode      },
        { CMSG_TOGGLE_PVP,                  STATUS_LOGGEDIN, &WorldSession::HandleTogglePvP                     },

        // played time
        { CMSG_PLAYED_TIME,                 STATUS_LOGGEDIN, &WorldSession::HandlePlayedTime                    },

        // new inspect
        { CMSG_INSPECT,                     STATUS_LOGGEDIN, &WorldSession::HandleInspectOpcode                 },

        // new inspect stats
        { MSG_INSPECT_HONOR_STATS,          STATUS_LOGGEDIN, &WorldSession::HandleInspectHonorStatsOpcode       },

        // charater view
        { CMSG_TOGGLE_HELM,                 STATUS_LOGGEDIN, &WorldSession::HandleToggleHelmOpcode              },
        { CMSG_TOGGLE_CLOAK,                STATUS_LOGGEDIN, &WorldSession::HandleToggleCloakOpcode             },

        // repair item
        { CMSG_REPAIR_ITEM,                 STATUS_LOGGEDIN, &WorldSession::HandleRepairItemOpcode              },

        { MSG_LOOKING_FOR_GROUP,            STATUS_LOGGEDIN, &WorldSession::HandleLookingForGroup               },
        { CMSG_SET_FACTION_ATWAR,           STATUS_LOGGEDIN, &WorldSession::HandleSetFactionAtWar               },
        { CMSG_SET_FACTION_CHEAT,           STATUS_LOGGEDIN, &WorldSession::HandleSetFactionCheat               },
        { CMSG_ZONEUPDATE,                  STATUS_LOGGEDIN, &WorldSession::HandleZoneUpdateOpcode              },
        { CMSG_SET_TARGET_OBSOLETE,         STATUS_LOGGEDIN, &WorldSession::HandleSetTargetOpcode               },
        { CMSG_SET_SELECTION,               STATUS_LOGGEDIN, &WorldSession::HandleSetSelectionOpcode            },
        { CMSG_STANDSTATECHANGE,            STATUS_LOGGEDIN, &WorldSession::HandleStandStateChangeOpcode        },
        { CMSG_FRIEND_LIST,                 STATUS_LOGGEDIN, &WorldSession::HandleFriendListOpcode              },
        { CMSG_ADD_FRIEND,                  STATUS_LOGGEDIN, &WorldSession::HandleAddFriendOpcode               },
        { CMSG_DEL_FRIEND,                  STATUS_LOGGEDIN, &WorldSession::HandleDelFriendOpcode               },
        { CMSG_ADD_IGNORE,                  STATUS_LOGGEDIN, &WorldSession::HandleAddIgnoreOpcode               },
        { CMSG_DEL_IGNORE,                  STATUS_LOGGEDIN, &WorldSession::HandleDelIgnoreOpcode               },
        { CMSG_BUG,                         STATUS_LOGGEDIN, &WorldSession::HandleBugOpcode                     },
        { CMSG_SET_AMMO,                    STATUS_LOGGEDIN, &WorldSession::HandleSetAmmoOpcode                 },
        { CMSG_AREATRIGGER,                 STATUS_LOGGEDIN, &WorldSession::HandleAreaTriggerOpcode             },
        { CMSG_UPDATE_ACCOUNT_DATA,         STATUS_LOGGEDIN, &WorldSession::HandleUpdateAccountData             },
        { CMSG_REQUEST_ACCOUNT_DATA,        STATUS_LOGGEDIN, &WorldSession::HandleRequestAccountData            },
        { CMSG_MEETINGSTONE_INFO,           STATUS_LOGGEDIN, &WorldSession::HandleMeetingStoneInfo              },
        { CMSG_GAMEOBJ_USE,                 STATUS_LOGGEDIN, &WorldSession::HandleGameObjectUseOpcode           },
        { MSG_CORPSE_QUERY,                 STATUS_LOGGEDIN, &WorldSession::HandleCorpseQueryOpcode             },
        { CMSG_NAME_QUERY,                  STATUS_LOGGEDIN, &WorldSession::HandleNameQueryOpcode               },
        { CMSG_QUERY_TIME,                  STATUS_LOGGEDIN, &WorldSession::HandleQueryTimeOpcode               },
        { CMSG_CREATURE_QUERY,              STATUS_LOGGEDIN, &WorldSession::HandleCreatureQueryOpcode           },
        { CMSG_GAMEOBJECT_QUERY,            STATUS_LOGGEDIN, &WorldSession::HandleGameObjectQueryOpcode         },

        // movements
        { MSG_MOVE_START_FORWARD,           STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_BACKWARD,          STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_STOP,                    STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_STRAFE_LEFT,       STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_STRAFE_RIGHT,      STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_STOP_STRAFE,             STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_JUMP,                    STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_TURN_LEFT,         STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_TURN_RIGHT,        STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_STOP_TURN,               STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_PITCH_UP,          STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_PITCH_DOWN,        STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_STOP_PITCH,              STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_SET_RUN_MODE,            STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_SET_WALK_MODE,           STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_TELEPORT_ACK,            STATUS_LOGGEDIN, &WorldSession::HandleMoveTeleportAck               },
        { MSG_MOVE_FALL_LAND,               STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_SWIM,              STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_STOP_SWIM,               STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_SET_FACING,              STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_SET_PITCH,               STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_WORLDPORT_ACK,           STATUS_LOGGEDIN, &WorldSession::HandleMoveWorldportAckOpcode        },
        { CMSG_FORCE_RUN_SPEED_CHANGE_ACK,  STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           },
        { CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK, STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck       },
        { CMSG_FORCE_SWIM_SPEED_CHANGE_ACK, STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           },
        { CMSG_FORCE_MOVE_ROOT_ACK,         STATUS_LOGGEDIN, &WorldSession::HandleMoveRootAck                   },
        { CMSG_FORCE_MOVE_UNROOT_ACK,       STATUS_LOGGEDIN, &WorldSession::HandleMoveUnRootAck                 },
        { MSG_MOVE_HEARTBEAT,               STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { CMSG_MOVE_KNOCK_BACK_ACK,         STATUS_LOGGEDIN, &WorldSession::HandleMoveKnockBackAck              },
        { CMSG_MOVE_HOVER_ACK,              STATUS_LOGGEDIN, &WorldSession::HandleMoveHoverAck                  },
        { CMSG_SET_ACTIVE_MOVER,            STATUS_LOGGEDIN, &WorldSession::HandleSetActiveMoverOpcode          },
        { CMSG_MOVE_FALL_RESET,             STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { CMSG_MOVE_FEATHER_FALL_ACK,       STATUS_LOGGEDIN, &WorldSession::HandleFeatherFallAck                },
        { CMSG_MOVE_WATER_WALK_ACK,         STATUS_LOGGEDIN, &WorldSession::HandleMoveWaterWalkAck              },
        { CMSG_FORCE_WALK_SPEED_CHANGE_ACK, STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           },
        { CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK, STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck      },
        { CMSG_FORCE_TURN_RATE_CHANGE_ACK,  STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           },
        { CMSG_FORCE_FLY_SPEED_CHANGE_ACK,  STATUS_LOGGEDIN, &WorldSession::HandleForceSpeedChangeAck           },

        { CMSG_MOUNTSPECIAL_ANIM,           STATUS_LOGGEDIN, &WorldSession::HandleMountSpecialAnimOpcode        },

        { CMSG_GROUP_INVITE,                STATUS_LOGGEDIN, &WorldSession::HandleGroupInviteOpcode             },
        //{ CMSG_GROUP_CANCEL,                STATUS_LOGGEDIN, &WorldSession::HandleGroupCancelOpcode             },
        { CMSG_GROUP_ACCEPT,                STATUS_LOGGEDIN, &WorldSession::HandleGroupAcceptOpcode             },
        { CMSG_GROUP_DECLINE,               STATUS_LOGGEDIN, &WorldSession::HandleGroupDeclineOpcode            },
        { CMSG_GROUP_UNINVITE,              STATUS_LOGGEDIN, &WorldSession::HandleGroupUninviteNameOpcode       },
        { CMSG_GROUP_UNINVITE_GUID,         STATUS_LOGGEDIN, &WorldSession::HandleGroupUninviteGuidOpcode       },
        { CMSG_GROUP_SET_LEADER,            STATUS_LOGGEDIN, &WorldSession::HandleGroupSetLeaderOpcode          },
        { CMSG_GROUP_DISBAND,               STATUS_LOGGEDIN, &WorldSession::HandleGroupDisbandOpcode            },
        { CMSG_LOOT_METHOD,                 STATUS_LOGGEDIN, &WorldSession::HandleLootMethodOpcode              },
        { CMSG_LOOT_ROLL,                   STATUS_LOGGEDIN, &WorldSession::HandleLootRoll                      },
        { CMSG_REQUEST_PARTY_MEMBER_STATS,  STATUS_LOGGEDIN, &WorldSession::HandleRequestPartyMemberStatsOpcode },
        { CMSG_REQUEST_RAID_INFO,           STATUS_LOGGEDIN, &WorldSession::HandleRequestRaidInfoOpcode         },
        { MSG_RAID_ICON_TARGET,             STATUS_LOGGEDIN, &WorldSession::HandleRaidIconTargetOpcode          },
        { CMSG_GROUP_RAID_CONVERT,          STATUS_LOGGEDIN, &WorldSession::HandleRaidConvertOpcode             },
        { MSG_RAID_READY_CHECK,             STATUS_LOGGEDIN, &WorldSession::HandleRaidReadyCheckOpcode          },
        { CMSG_GROUP_CHANGE_SUB_GROUP,      STATUS_LOGGEDIN, &WorldSession::HandleGroupChangeSubGroupOpcode     },
        { CMSG_GROUP_ASSISTANT,             STATUS_LOGGEDIN, &WorldSession::HandleGroupAssistantOpcode          },
        { CMSG_GROUP_PROMOTE,               STATUS_LOGGEDIN, &WorldSession::HandleGroupPromoteOpcode            },

        { CMSG_PETITION_SHOWLIST,           STATUS_LOGGEDIN, &WorldSession::HandlePetitionShowListOpcode        },
        { CMSG_PETITION_BUY,                STATUS_LOGGEDIN, &WorldSession::HandlePetitionBuyOpcode             },
        { CMSG_PETITION_SHOW_SIGNATURES,    STATUS_LOGGEDIN, &WorldSession::HandlePetitionShowSignOpcode        },
        { CMSG_PETITION_QUERY,              STATUS_LOGGEDIN, &WorldSession::HandlePetitionQueryOpcode           },
        { MSG_PETITION_RENAME,              STATUS_LOGGEDIN, &WorldSession::HandlePetitionRenameOpcode          },
        { CMSG_PETITION_SIGN,               STATUS_LOGGEDIN, &WorldSession::HandlePetitionSignOpcode            },
        { MSG_PETITION_DECLINE,             STATUS_LOGGEDIN, &WorldSession::HandlePetitionDeclineOpcode         },
        { CMSG_OFFER_PETITION,              STATUS_LOGGEDIN, &WorldSession::HandleOfferPetitionOpcode           },
        { CMSG_TURN_IN_PETITION,            STATUS_LOGGEDIN, &WorldSession::HandleTurnInPetitionOpcode          },

        { CMSG_GUILD_QUERY,                 STATUS_AUTHED,   &WorldSession::HandleGuildQueryOpcode              },
        { CMSG_GUILD_CREATE,                STATUS_LOGGEDIN, &WorldSession::HandleGuildCreateOpcode             },
        { CMSG_GUILD_INVITE,                STATUS_LOGGEDIN, &WorldSession::HandleGuildInviteOpcode             },
        { CMSG_GUILD_REMOVE,                STATUS_LOGGEDIN, &WorldSession::HandleGuildRemoveOpcode             },
        { CMSG_GUILD_ACCEPT,                STATUS_LOGGEDIN, &WorldSession::HandleGuildAcceptOpcode             },
        { CMSG_GUILD_DECLINE,               STATUS_LOGGEDIN, &WorldSession::HandleGuildDeclineOpcode            },
        { CMSG_GUILD_INFO,                  STATUS_LOGGEDIN, &WorldSession::HandleGuildInfoOpcode               },
        { CMSG_GUILD_ROSTER,                STATUS_LOGGEDIN, &WorldSession::HandleGuildRosterOpcode             },
        { CMSG_GUILD_PROMOTE,               STATUS_LOGGEDIN, &WorldSession::HandleGuildPromoteOpcode            },
        { CMSG_GUILD_DEMOTE,                STATUS_LOGGEDIN, &WorldSession::HandleGuildDemoteOpcode             },
        { CMSG_GUILD_LEAVE,                 STATUS_LOGGEDIN, &WorldSession::HandleGuildLeaveOpcode              },
        { CMSG_GUILD_DISBAND,               STATUS_LOGGEDIN, &WorldSession::HandleGuildDisbandOpcode            },
        { CMSG_GUILD_LEADER,                STATUS_LOGGEDIN, &WorldSession::HandleGuildLeaderOpcode             },
        { CMSG_GUILD_MOTD,                  STATUS_LOGGEDIN, &WorldSession::HandleGuildMOTDOpcode               },
        { CMSG_GUILD_SET_PUBLIC_NOTE,       STATUS_LOGGEDIN, &WorldSession::HandleGuildSetPublicNoteOpcode      },
        { CMSG_GUILD_SET_OFFICER_NOTE,      STATUS_LOGGEDIN, &WorldSession::HandleGuildSetOfficerNoteOpcode     },
        { CMSG_GUILD_RANK,                  STATUS_LOGGEDIN, &WorldSession::HandleGuildRankOpcode               },
        { CMSG_GUILD_ADD_RANK,              STATUS_LOGGEDIN, &WorldSession::HandleGuildAddRankOpcode            },
        { CMSG_GUILD_DEL_RANK,              STATUS_LOGGEDIN, &WorldSession::HandleGuildDelRankOpcode            },
        { CMSG_GUILD_CHANGEINFO,            STATUS_LOGGEDIN, &WorldSession::HandleGuildChangeInfoOpcode         },
        { MSG_SAVE_GUILD_EMBLEM,            STATUS_LOGGEDIN, &WorldSession::HandleGuildSaveEmblemOpcode         },

        { CMSG_TAXINODE_STATUS_QUERY,       STATUS_LOGGEDIN, &WorldSession::HandleTaxiNodeStatusQueryOpcode     },
        { CMSG_TAXIQUERYAVAILABLENODES,     STATUS_LOGGEDIN, &WorldSession::HandleTaxiQueryAvailableNodesOpcode },
        { CMSG_ACTIVATETAXI,                STATUS_LOGGEDIN, &WorldSession::HandleActivateTaxiOpcode            },
        { CMSG_ACTIVATETAXI_FAR,            STATUS_LOGGEDIN, &WorldSession::HandleActivateTaxiFarOpcode         },
        { CMSG_MOVE_SPLINE_DONE,            STATUS_LOGGEDIN, &WorldSession::HandleTaxiNextDestinationOpcode     },

        { MSG_TABARDVENDOR_ACTIVATE,        STATUS_LOGGEDIN, &WorldSession::HandleTabardVendorActivateOpcode    },
        { CMSG_BANKER_ACTIVATE,             STATUS_LOGGEDIN, &WorldSession::HandleBankerActivateOpcode          },
        { CMSG_BUY_BANK_SLOT,               STATUS_LOGGEDIN, &WorldSession::HandleBuyBankSlotOpcode             },
        { CMSG_TRAINER_LIST,                STATUS_LOGGEDIN, &WorldSession::HandleTrainerListOpcode             },
        { CMSG_TRAINER_BUY_SPELL,           STATUS_LOGGEDIN, &WorldSession::HandleTrainerBuySpellOpcode         },
        { MSG_AUCTION_HELLO,                STATUS_LOGGEDIN, &WorldSession::HandleAuctionHelloOpcode            },
        { CMSG_GOSSIP_HELLO,                STATUS_LOGGEDIN, &WorldSession::HandleGossipHelloOpcode             },
        { CMSG_GOSSIP_SELECT_OPTION,        STATUS_LOGGEDIN, &WorldSession::HandleGossipSelectOptionOpcode      },
        { CMSG_SPIRIT_HEALER_ACTIVATE,      STATUS_LOGGEDIN, &WorldSession::HandleSpiritHealerActivateOpcode    },
        { CMSG_NPC_TEXT_QUERY,              STATUS_LOGGEDIN, &WorldSession::HandleNpcTextQueryOpcode            },
        { CMSG_BINDER_ACTIVATE,             STATUS_LOGGEDIN, &WorldSession::HandleBinderActivateOpcode          },
        { MSG_LIST_STABLED_PETS,            STATUS_LOGGEDIN, &WorldSession::HandleListStabledPetsOpcode         },

        { CMSG_DUEL_ACCEPTED,               STATUS_LOGGEDIN, &WorldSession::HandleDuelAcceptedOpcode            },
        { CMSG_DUEL_CANCELLED,              STATUS_LOGGEDIN, &WorldSession::HandleDuelCancelledOpcode           },

        { CMSG_ACCEPT_TRADE,                STATUS_LOGGEDIN, &WorldSession::HandleAcceptTradeOpcode             },
        { CMSG_BEGIN_TRADE,                 STATUS_LOGGEDIN, &WorldSession::HandleBeginTradeOpcode              },
        { CMSG_BUSY_TRADE,                  STATUS_LOGGEDIN, &WorldSession::HandleBusyTradeOpcode               },
                                                            // sended after loguot complete
        { CMSG_CANCEL_TRADE,                STATUS_AUTHED,   &WorldSession::HandleCancelTradeOpcode             },
        { CMSG_CLEAR_TRADE_ITEM,            STATUS_LOGGEDIN, &WorldSession::HandleClearTradeItemOpcode          },
        { CMSG_IGNORE_TRADE,                STATUS_LOGGEDIN, &WorldSession::HandleIgnoreTradeOpcode             },
        { CMSG_INITIATE_TRADE,              STATUS_LOGGEDIN, &WorldSession::HandleInitiateTradeOpcode           },
        { CMSG_SET_TRADE_GOLD,              STATUS_LOGGEDIN, &WorldSession::HandleSetTradeGoldOpcode            },
        { CMSG_SET_TRADE_ITEM,              STATUS_LOGGEDIN, &WorldSession::HandleSetTradeItemOpcode            },
        { CMSG_UNACCEPT_TRADE,              STATUS_LOGGEDIN, &WorldSession::HandleUnacceptTradeOpcode           },

        { CMSG_SPLIT_ITEM,                  STATUS_LOGGEDIN, &WorldSession::HandleSplitItemOpcode               },
        { CMSG_SWAP_INV_ITEM,               STATUS_LOGGEDIN, &WorldSession::HandleSwapInvItemOpcode             },
        { CMSG_DESTROYITEM,                 STATUS_LOGGEDIN, &WorldSession::HandleDestroyItemOpcode             },
        { CMSG_AUTOEQUIP_ITEM,              STATUS_LOGGEDIN, &WorldSession::HandleAutoEquipItemOpcode           },
        { CMSG_ITEM_QUERY_SINGLE,           STATUS_LOGGEDIN, &WorldSession::HandleItemQuerySingleOpcode         },
        { CMSG_SELL_ITEM,                   STATUS_LOGGEDIN, &WorldSession::HandleSellItemOpcode                },
        { CMSG_BUY_ITEM_IN_SLOT,            STATUS_LOGGEDIN, &WorldSession::HandleBuyItemInSlotOpcode           },
        { CMSG_BUY_ITEM,                    STATUS_LOGGEDIN, &WorldSession::HandleBuyItemOpcode                 },
        { CMSG_LIST_INVENTORY,              STATUS_LOGGEDIN, &WorldSession::HandleListInventoryOpcode           },
        { CMSG_SWAP_ITEM,                   STATUS_LOGGEDIN, &WorldSession::HandleSwapItem                      },
        { CMSG_BUYBACK_ITEM,                STATUS_LOGGEDIN, &WorldSession::HandleBuybackItem                   },
        { CMSG_AUTOSTORE_BAG_ITEM,          STATUS_LOGGEDIN, &WorldSession::HandleAutoStoreBagItemOpcode        },
        { CMSG_AUTOBANK_ITEM,               STATUS_LOGGEDIN, &WorldSession::HandleAutoBankItemOpcode            },
        { CMSG_AUTOSTORE_BANK_ITEM,         STATUS_LOGGEDIN, &WorldSession::HandleAutoStoreBankItemOpcode       },
        { CMSG_ITEM_NAME_QUERY,             STATUS_LOGGEDIN, &WorldSession::HandleItemNameQueryOpcode           },
        { CMSG_WRAP_ITEM,                   STATUS_LOGGEDIN, &WorldSession::HandleWrapItemOpcode                },

        { CMSG_ATTACKSWING,                 STATUS_LOGGEDIN, &WorldSession::HandleAttackSwingOpcode             },
        { CMSG_ATTACKSTOP,                  STATUS_LOGGEDIN, &WorldSession::HandleAttackStopOpcode              },
        { CMSG_SETSHEATHED,                 STATUS_LOGGEDIN, &WorldSession::HandleSetSheathedOpcode             },

        { CMSG_USE_ITEM,                    STATUS_LOGGEDIN, &WorldSession::HandleUseItemOpcode                 },
        { CMSG_OPEN_ITEM,                   STATUS_LOGGEDIN, &WorldSession::HandleOpenItemOpcode                },
        { CMSG_CAST_SPELL,                  STATUS_LOGGEDIN, &WorldSession::HandleCastSpellOpcode               },
        { CMSG_CANCEL_CAST,                 STATUS_LOGGEDIN, &WorldSession::HandleCancelCastOpcode              },
        { CMSG_CANCEL_AURA,                 STATUS_LOGGEDIN, &WorldSession::HandleCancelAuraOpcode              },
        { CMSG_CANCEL_GROWTH_AURA,          STATUS_LOGGEDIN, &WorldSession::HandleCancelGrowthAuraOpcode        },
        { CMSG_CANCEL_AUTO_REPEAT_SPELL,    STATUS_LOGGEDIN, &WorldSession::HandleCancelAutoRepeatSpellOpcode   },

        { CMSG_LEARN_TALENT,                STATUS_LOGGEDIN, &WorldSession::HandleLearnTalentOpcode             },
        { MSG_TALENT_WIPE_CONFIRM,          STATUS_LOGGEDIN, &WorldSession::HandleTalentWipeOpcode              },
        { CMSG_UNLEARN_SKILL,               STATUS_LOGGEDIN, &WorldSession::HandleUnlearnSkillOpcode            },

        { CMSG_QUESTGIVER_STATUS_QUERY,     STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverStatusQueryOpcode   },
        { CMSG_QUESTGIVER_HELLO,            STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverHelloOpcode         },
        { CMSG_QUESTGIVER_ACCEPT_QUEST,     STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverAcceptQuestOpcode   },
        { CMSG_QUESTGIVER_CHOOSE_REWARD,    STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverChooseRewardOpcode  },
        { CMSG_QUESTGIVER_REQUEST_REWARD,   STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverRequestRewardOpcode },
        { CMSG_QUESTGIVER_QUERY_QUEST,      STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverQuestQueryOpcode    },
        { CMSG_QUEST_QUERY,                 STATUS_LOGGEDIN, &WorldSession::HandleQuestQueryOpcode              },

        { CMSG_QUEST_CONFIRM_ACCEPT,        STATUS_LOGGEDIN, &WorldSession::HandleQuestConfirmAccept            },
        { CMSG_QUESTLOG_REMOVE_QUEST,       STATUS_LOGGEDIN, &WorldSession::HandleQuestLogRemoveQuest           },
        { CMSG_QUESTLOG_SWAP_QUEST,         STATUS_LOGGEDIN, &WorldSession::HandleQuestLogSwapQuest             },
        { CMSG_QUESTGIVER_CANCEL,           STATUS_LOGGEDIN, &WorldSession::HandleQuestgiverCancel              },
        { CMSG_QUESTGIVER_COMPLETE_QUEST,   STATUS_LOGGEDIN, &WorldSession::HandleQuestComplete                 },
        { CMSG_QUESTGIVER_QUEST_AUTOLAUNCH, STATUS_LOGGEDIN, &WorldSession::HandleQuestAutoLaunch               },
        { CMSG_PUSHQUESTTOPARTY,            STATUS_LOGGEDIN, &WorldSession::HandleQuestPushToParty              },
        { MSG_QUEST_PUSH_RESULT,            STATUS_LOGGEDIN, &WorldSession::HandleQuestPushResult               },

        { CMSG_TUTORIAL_FLAG,               STATUS_LOGGEDIN, &WorldSession::HandleTutorialFlag                  },
        { CMSG_TUTORIAL_CLEAR,              STATUS_LOGGEDIN, &WorldSession::HandleTutorialClear                 },
        { CMSG_TUTORIAL_RESET,              STATUS_LOGGEDIN, &WorldSession::HandleTutorialReset                 },

        { CMSG_MESSAGECHAT,                 STATUS_LOGGEDIN, &WorldSession::HandleMessagechatOpcode             },
        { CMSG_TEXT_EMOTE,                  STATUS_LOGGEDIN, &WorldSession::HandleTextEmoteOpcode               },
        { CMSG_CHAT_IGNORED,                STATUS_LOGGEDIN, &WorldSession::HandleChatIgnoredOpcode             },

        { CMSG_RECLAIM_CORPSE,              STATUS_LOGGEDIN, &WorldSession::HandleCorpseReclaimOpcode           },
        { CMSG_RESURRECT_RESPONSE,          STATUS_LOGGEDIN, &WorldSession::HandleResurrectResponseOpcode       },
        { CMSG_AUCTION_LIST_ITEMS,          STATUS_LOGGEDIN, &WorldSession::HandleAuctionListItems              },
        { CMSG_AUCTION_LIST_BIDDER_ITEMS,   STATUS_LOGGEDIN, &WorldSession::HandleAuctionListBidderItems        },
        { CMSG_AUCTION_SELL_ITEM,           STATUS_LOGGEDIN, &WorldSession::HandleAuctionSellItem               },
        { CMSG_AUCTION_REMOVE_ITEM,         STATUS_LOGGEDIN, &WorldSession::HandleAuctionRemoveItem             },
        { CMSG_AUCTION_LIST_OWNER_ITEMS,    STATUS_LOGGEDIN, &WorldSession::HandleAuctionListOwnerItems         },
        { CMSG_AUCTION_PLACE_BID,           STATUS_LOGGEDIN, &WorldSession::HandleAuctionPlaceBid               },

        { CMSG_JOIN_CHANNEL,                STATUS_LOGGEDIN, &WorldSession::HandleChannelJoin                   },
        { CMSG_LEAVE_CHANNEL,               STATUS_LOGGEDIN, &WorldSession::HandleChannelLeave                  },
        { CMSG_CHANNEL_LIST,                STATUS_LOGGEDIN, &WorldSession::HandleChannelList                   },
        { CMSG_CHANNEL_PASSWORD,            STATUS_LOGGEDIN, &WorldSession::HandleChannelPassword               },
        { CMSG_CHANNEL_SET_OWNER,           STATUS_LOGGEDIN, &WorldSession::HandleChannelSetOwner               },
        { CMSG_CHANNEL_OWNER,               STATUS_LOGGEDIN, &WorldSession::HandleChannelOwner                  },
        { CMSG_CHANNEL_MODERATOR,           STATUS_LOGGEDIN, &WorldSession::HandleChannelModerator              },
        { CMSG_CHANNEL_UNMODERATOR,         STATUS_LOGGEDIN, &WorldSession::HandleChannelUnmoderator            },
        { CMSG_CHANNEL_MUTE,                STATUS_LOGGEDIN, &WorldSession::HandleChannelMute                   },
        { CMSG_CHANNEL_UNMUTE,              STATUS_LOGGEDIN, &WorldSession::HandleChannelUnmute                 },
        { CMSG_CHANNEL_INVITE,              STATUS_LOGGEDIN, &WorldSession::HandleChannelInvite                 },
        { CMSG_CHANNEL_KICK,                STATUS_LOGGEDIN, &WorldSession::HandleChannelKick                   },
        { CMSG_CHANNEL_BAN,                 STATUS_LOGGEDIN, &WorldSession::HandleChannelBan                    },
        { CMSG_CHANNEL_UNBAN,               STATUS_LOGGEDIN, &WorldSession::HandleChannelUnban                  },
        { CMSG_CHANNEL_ANNOUNCEMENTS,       STATUS_LOGGEDIN, &WorldSession::HandleChannelAnnounce               },
        { CMSG_CHANNEL_MODERATE,            STATUS_LOGGEDIN, &WorldSession::HandleChannelModerate               },

        { CMSG_GET_MAIL_LIST,               STATUS_LOGGEDIN, &WorldSession::HandleGetMail                       },
        { CMSG_ITEM_TEXT_QUERY,             STATUS_LOGGEDIN, &WorldSession::HandleItemTextQuery                 },
        { CMSG_SEND_MAIL,                   STATUS_LOGGEDIN, &WorldSession::HandleSendMail                      },
        { CMSG_MAIL_TAKE_MONEY,             STATUS_LOGGEDIN, &WorldSession::HandleTakeMoney                     },
        { CMSG_MAIL_TAKE_ITEM,              STATUS_LOGGEDIN, &WorldSession::HandleTakeItem                      },
        { CMSG_MAIL_MARK_AS_READ,           STATUS_LOGGEDIN, &WorldSession::HandleMarkAsRead                    },
        { CMSG_MAIL_RETURN_TO_SENDER,       STATUS_LOGGEDIN, &WorldSession::HandleReturnToSender                },
        { CMSG_MAIL_DELETE,                 STATUS_LOGGEDIN, &WorldSession::HandleMailDelete                    },
        { CMSG_MAIL_CREATE_TEXT_ITEM,       STATUS_LOGGEDIN, &WorldSession::HandleMailCreateTextItem            },
        { MSG_QUERY_NEXT_MAIL_TIME,         STATUS_LOGGEDIN, &WorldSession::HandleMsgQueryNextMailtime          },

        { CMSG_COMPLETE_CINEMATIC,          STATUS_LOGGEDIN, &WorldSession::HandleCompleteCinema                },
        { CMSG_NEXT_CINEMATIC_CAMERA,       STATUS_LOGGEDIN, &WorldSession::HandleNextCinematicCamera           },

        { CMSG_MOVE_TIME_SKIPPED,           STATUS_LOGGEDIN, &WorldSession::HandleMoveTimeSkippedOpcode         },

        { CMSG_PAGE_TEXT_QUERY,             STATUS_LOGGEDIN, &WorldSession::HandlePageQueryOpcode               },
        { CMSG_READ_ITEM,                   STATUS_LOGGEDIN, &WorldSession::HandleReadItem                      },

        { CMSG_PET_ACTION,                  STATUS_LOGGEDIN, &WorldSession::HandlePetAction                     },
        { CMSG_PET_NAME_QUERY,              STATUS_LOGGEDIN, &WorldSession::HandlePetNameQuery                  },

        { CMSG_PET_ABANDON,                 STATUS_LOGGEDIN, &WorldSession::HandlePetAbandon                    },
        { CMSG_PET_SET_ACTION,              STATUS_LOGGEDIN, &WorldSession::HandlePetSetAction                  },
        { CMSG_PET_RENAME,                  STATUS_LOGGEDIN, &WorldSession::HandlePetRename                     },
        { CMSG_STABLE_PET,                  STATUS_LOGGEDIN, &WorldSession::HandleStablePet                     },
        { CMSG_UNSTABLE_PET,                STATUS_LOGGEDIN, &WorldSession::HandleUnstablePet                   },
        { CMSG_BUY_STABLE_SLOT,             STATUS_LOGGEDIN, &WorldSession::HandleBuyStableSlot                 },
        { CMSG_STABLE_REVIVE_PET,           STATUS_LOGGEDIN, &WorldSession::HandleStableRevivePet               },
        { CMSG_STABLE_SWAP_PET,             STATUS_LOGGEDIN, &WorldSession::HandleStableSwapPet                 },

        { CMSG_CANCEL_CHANNELLING ,         STATUS_LOGGEDIN, &WorldSession::HandleCancelChanneling              },

        //BattleGround

        { CMSG_BATTLEFIELD_STATUS,          STATUS_LOGGEDIN, &WorldSession::HandleBattlefieldStatusOpcode       },
        { CMSG_BATTLEMASTER_HELLO,          STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundHelloOpcode       },
        { CMSG_BATTLEMASTER_JOIN,           STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundJoinOpcode        },
        { MSG_BATTLEGROUND_PLAYER_POSITIONS,STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundPlayerPositionsOpcode },
        { MSG_PVP_LOG_DATA,                 STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundPVPlogdataOpcode  },
        { CMSG_BATTLEFIELD_PORT,            STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundPlayerPortOpcode  },
        { CMSG_BATTLEFIELD_LIST,            STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundListOpcode        },
        { CMSG_LEAVE_BATTLEFIELD,           STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundLeaveOpcode       },

        { CMSG_SET_ACTIONBAR_TOGGLES,       STATUS_LOGGEDIN, &WorldSession::HandleSetActionBar                  },
        { CMSG_FIELD_WATCHED_FACTION_SHOW_BAR, STATUS_LOGGEDIN, &WorldSession::HandleSetWatchedFactionIndexOpcode },

        { CMSG_LOOT_ROLL,                   STATUS_LOGGEDIN, &WorldSession::HandleLootRoll                      }, // WTF, why it 2 times there?
        { CMSG_WARDEN_DATA,                 STATUS_LOGGEDIN, &WorldSession::HandleWardenDataOpcode              },
        { CMSG_WORLD_TELEPORT,              STATUS_LOGGEDIN, &WorldSession::HandleWorldTeleportOpcode           },
        { MSG_MINIMAP_PING,                 STATUS_LOGGEDIN, &WorldSession::HandleMinimapPingOpcode             },
        { MSG_RANDOM_ROLL,                  STATUS_LOGGEDIN, &WorldSession::HandleRandomRollOpcode              },
        { CMSG_FAR_SIGHT,                   STATUS_LOGGEDIN, &WorldSession::HandleFarSightOpcode                },

        { MSG_LOOKING_FOR_GROUP,            STATUS_LOGGEDIN, &WorldSession::HandleLookingForGroup               },
        { CMSG_SET_LOOKING_FOR_GROUP,       STATUS_LOGGEDIN, &WorldSession::HandleSetLfgOpcode                  },
        { MSG_SET_DUNGEON_DIFFICULTY,       STATUS_LOGGEDIN, &WorldSession::HandleDungeonDifficultyOpcode       },
        { CMSG_MOVE_FLY_MODE_CHANGE_ACK,    STATUS_LOGGEDIN, &WorldSession::HandleMoveFlyModeChangeAckOpcode    },
        { CMSG_ARENA_TEAM_QUERY,            STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamQueryOpcode          },
        { MSG_MOVE_START_FLY_UP,            STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_STOP_FLY_UP,             STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { CMSG_LFG_SET_AUTOJOIN,            STATUS_AUTHED,   &WorldSession::HandleLfgAutoJoinOpcode             },
        { CMSG_LFG_UNSET_AUTOJOIN,          STATUS_LOGGEDIN, &WorldSession::HandleLfgCancelAutoJoinOpcode       },
        { CMSG_LFM_SET_AUTOADD,             STATUS_AUTHED,   &WorldSession::HandleLfmAutoAddMembersOpcode       },
        { CMSG_LFM_UNSET_AUTOADD,           STATUS_LOGGEDIN, &WorldSession::HandleLfmCancelAutoAddmembersOpcode },
        { CMSG_LOOKING_FOR_GROUP_CLEAR,     STATUS_LOGGEDIN, &WorldSession::HandleLfgClearOpcode                },
        { CMSG_SET_LOOKING_FOR_NONE,        STATUS_LOGGEDIN, &WorldSession::HandleLfmSetNoneOpcode              },
        { CMSG_SET_LOOKING_FOR_MORE,        STATUS_LOGGEDIN, &WorldSession::HandleLfmSetOpcode                  },
        { CMSG_SET_COMMENTARY,              STATUS_LOGGEDIN, &WorldSession::HandleLfgSetCommentOpcode           },
        { CMSG_CHOOSE_TITLE,                STATUS_LOGGEDIN, &WorldSession::HandleChooseTitleOpcode             },
        { MSG_INSPECT_ARENA_STATS,          STATUS_LOGGEDIN, &WorldSession::HandleInspectArenaStatsOpcode       },
        { CMSG_REALM_STATE_REQUEST,         STATUS_AUTHED,   &WorldSession::HandleRealmStateRequestOpcode       },
        { CMSG_ALLOW_MOVE_ACK,              STATUS_LOGGEDIN, &WorldSession::HandleAllowMoveAckOpcode            },
        { CMSG_WHOIS,                       STATUS_LOGGEDIN, &WorldSession::HandleWhoisOpcode                   },
        { CMSG_RESET_INSTANCES,             STATUS_LOGGEDIN, &WorldSession::HandleResetInstancesOpcode          },
        { CMSG_ARENA_TEAM_ROSTER,           STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamRosterOpcode         },
        { CMSG_ARENA_TEAM_ADD_MEMBER,       STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamAddMemberOpcode      },
        { CMSG_ARENA_TEAM_INVITE_ACCEPT,    STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamInviteAcceptOpcode   },
        { CMSG_ARENA_TEAM_INVITE_DECLINE,   STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamInviteDeclineOpcode  },
        { CMSG_ARENA_TEAM_LEAVE,            STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamLeaveOpcode          },
        { CMSG_ARENA_TEAM_DISBAND,          STATUS_LOGGEDIN, &WorldSession::HandleArenaTeamDisbandOpcode        },
        { CMSG_AREA_SPIRIT_HEALER_QUERY,    STATUS_LOGGEDIN, &WorldSession::HandleAreaSpiritHealerQueryOpcode   },
        { CMSG_MOVE_SHIP_909,               STATUS_LOGGEDIN, &WorldSession::HandleMoveShipOpcode                },
        { CMSG_MOVE_FLY_STATE_CHANGE,       STATUS_LOGGEDIN, &WorldSession::HandleMoveFlyStateChangeOpcode      },
        { CMSG_DISMOUNT,                    STATUS_LOGGEDIN, &WorldSession::HandleDismountOpcode                },

        // Socket gem
        { CMSG_SOCKET_ITEM,                 STATUS_LOGGEDIN, &WorldSession::HandleSocketOpcode                  },

        { 0,                                0,               NULL                                               }
    };

    return table;
}*/

/// Cancel channeling handler
/// \todo Complete HandleCancelChanneling function
void WorldSession::HandleCancelChanneling( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4);

    uint32 spellid;
    recv_data >> spellid;
}

void WorldSession::HandleFarSightOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,1);

    sLog.outDebug("WORLD: CMSG_FAR_SIGHT");
    recv_data.hexlike();

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
    uint32 lenght = strlen(Text)+1;
    WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 4+lenght);
    data << lenght;
    data << Text;
    SendPacket(&data);
}

void WorldSession::HandleDungeonDifficultyOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("MSG_SET_DUNGEON_DIFFICULTY");

    uint32 difficulty;
    recv_data >> difficulty;

    GetPlayer()->SetDungeonDifficulty(difficulty);
    GetPlayer()->SendDungeonDifficulty();
}

void WorldSession::HandleChooseTitleOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_CHOOSE_TITLE");

    uint32 title;
    recv_data >> title;    

    uint32 available = GetPlayer()->GetUInt32Value(PLAYER_FIELD_KNOWN_TITLES);
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
    sLog.outDebug("CMSG_REALM_STATE_REQUEST");

    uint32 unk;
    std::string split_date = "01/01/01";
    recv_data >> unk;

    WorldPacket data(SMSG_REALM_STATE_RESPONSE, 17);
    data << unk;
    data << uint32(0x00000000); // realm split state
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
    sLog.outDebug("CMSG_ALLOW_MOVE_ACK");

    uint32 unk, time;
    recv_data >> unk >> time;
    sLog.outDebug("response sent %u %u", unk, time/1000);
}

void WorldSession::HandleWhoisOpcode(WorldPacket& recv_data)
{
    sLog.outDebug("Received opcode CMSG_WHOIS");
    std::string charname, acc, email, lastip, msg;
    WorldPacket data;
    recv_data >> charname;

    if (GetSecurity() < 3)
    {
        SendNotification("You do not have permission to perform that function");
        return;
    }

    if(charname.size())
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
        if(!acc.size())
            acc = "Unknown";
        email = fields[1].GetCppString();
        if(!email.size())
            email = "Unknown";
        lastip = fields[2].GetCppString();
        if(!lastip.size())
            lastip = "Unknown";
        msg = charname + "'s " + "account is " + acc + ", e-mail " + email + ", last ip: " + lastip;
        data.Initialize(SMSG_WHOIS);
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
    WorldPacket data(SMSG_RESET_INSTANCES_RESULT, 4);
    data << mapid;
    _player->GetSession()->SendPacket(&data);
*/
}

void WorldSession::HandleMoveShipOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_MOVE_SHIP_909");
    recv_data.hexlike();
/*
WORLD: CMSG_MOVE_SHIP_909
STORAGE_SIZE: 56
01 02 80 00 7A 48 DF 00 | BA 8A 05 46 19 9D 7E 44
42 10 BA 40 5C 90 77 40 | 74 B0 02 00 00 00 00 80
42 D4 0B C0 65 FD 50 C1 | 42 10 BA 40 20 7F 13 40
5E 49 DF 00 C0 00 00 00

WORLD: CMSG_MOVE_SHIP_909
STORAGE_SIZE: 60
01 02 80 04 F0 D6 E0 00 | 3A B2 CD 45 C3 5B 3F 44
E4 4F B8 40 58 E7 4B 40 | 00 00 00 00 00 00 00 00
3A B2 CD 45 C3 5B 3F 44 | E4 4F B8 40 58 E7 4B 40
D4 D7 E0 00 CF 00 00 00 | E4 4F B8 40
*/
}

void WorldSession::HandleAreaSpiritHealerQueryOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_AREA_SPIRIT_HEALER_QUERY");
    recv_data.hexlike();
}

void WorldSession::HandleDismountOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_DISMOUNT");
    recv_data.hexlike();

    //If player is not mounted, so go out :)
    if (!_player->IsMounted()) // not blizz like; no any messages on blizz
    {
        sChatHandler.SendSysMessage(this, LANG_CHAR_NON_MOUNTED);
        return;
    }

    if(_player->isInFlight()) // not blizz like; no any messages on blizz
    {
        sChatHandler.SendSysMessage(this, LANG_YOU_IN_FLIGHT);
        return;
    }

    _player->Unmount();
    _player->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
}

void WorldSession::HandleMoveFlyModeChangeAckOpcode( WorldPacket & recv_data )
{
    // fly mode on/off
    sLog.outDebug("WORLD: CMSG_MOVE_FLY_MODE_CHANGE_ACK");
    recv_data.hexlike();

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

void WorldSession::HandleMoveFlyStateChangeOpcode( WorldPacket & recv_data )
{
    /*
    00 08 A0 01 4C AE DC 01 | F8 18 56 45 03 97 BD 44
    A2 8D 33 43 1C B3 29 40 | 00 00 00 00 45 00 00 00
    00 00 80 3F

    01 20 80 00 5F D3 DC 01 | 8B B3 55 45 4C 77 BD 44
    F 65 33 43 DC AB 53 40 | 00 00 00 00 00 00 00 00
    96 7D 7C BF 54 F9 28 BE | 00 00 60 41 00 00 00 00
    */

    // fly state: start flying/landing
    sLog.outDebug("WORLD: CMSG_MOVE_FLY_STATE_CHANGE");
    recv_data.hexlike();
    CHECK_PACKET_SIZE(recv_data,4+4+4+4+4+4);

    /* extract packet */
    uint32 flags, time, fallTime;
    float x, y, z, orientation;

    uint32 t_GUIDl, t_GUIDh;
    float  t_x, t_y, t_z, t_o;
    float  s_angle;
    float  j_unk1, j_sinAngle, j_cosAngle, j_xyspeed;
    float f_speed;

    recv_data >> flags >> time;
    recv_data >> x >> y >> z >> orientation;

    _player->SetMovementFlags(flags);

    if (flags & MOVEMENTFLAG_ONTRANSPORT) // and if opcode 909?
    {
        // recheck
        CHECK_PACKET_SIZE(recv_data,recv_data.rpos()+4+4+4+4+4+4);

        recv_data >> t_GUIDl >> t_GUIDh;
        recv_data >> t_x >> t_y >> t_z >> t_o;
    }
    if (flags & MOVEMENTFLAG_SWIMMING)
    {
        // recheck
        CHECK_PACKET_SIZE(recv_data,recv_data.rpos()+4);

        recv_data >> s_angle;                               // kind of angle, -1.55=looking down, 0=looking straight forward, +1.55=looking up
    }

    // recheck
    CHECK_PACKET_SIZE(recv_data,recv_data.rpos()+4);

    recv_data >> fallTime;                                  // duration of last jump (when in jump duration from jump begin to now)

    if ( (flags & MOVEMENTFLAG_JUMPING) || (flags & MOVEMENTFLAG_FALLING) )
    {
        // recheck
        CHECK_PACKET_SIZE(recv_data,recv_data.rpos()+4+4+4+4);

        recv_data >> j_unk1;                                // constant, but different when jumping in water and on land?
        recv_data >> j_sinAngle >> j_cosAngle;              // sin + cos of angle between orientation0 and players orientation
        recv_data >> j_xyspeed;                             // speed of xy movement
    }

    // recheck
    CHECK_PACKET_SIZE(recv_data,recv_data.rpos()+4);

    recv_data >> f_speed; // this is difference from standart movement opcodes...
    sLog.outDebug("f_speed %f", f_speed);
    /*----------------*/
}

void WorldSession::HandleSelfResOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("WORLD: CMSG_SELF_RES");  // empty opcode

    if(_player->GetUInt32Value(PLAYER_SELF_RES_SPELL))
    {
        SpellEntry const *spellInfo = sSpellStore.LookupEntry(_player->GetUInt32Value(PLAYER_SELF_RES_SPELL));
        if(spellInfo)
        _player->CastSpell(_player,spellInfo,false,0);

        _player->SetUInt32Value(PLAYER_SELF_RES_SPELL, 0);
    }
}
