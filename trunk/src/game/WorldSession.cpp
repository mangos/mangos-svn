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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "Opcodes.h"
#include "WorldSocket.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "AuctionHouseObject.h"
#include "Group.h"
#include "Guild.h"
#include "World.h"
#include "NameTables.h"
#include "MapManager.h"
#include "ObjectAccessor.h"

WorldSession::WorldSession(uint32 id, WorldSocket *sock) : _player(0), _socket(sock),
_security(0), _accountId(id), _logoutTime(0)
{

}

WorldSession::~WorldSession()
{
    WorldPacket *packet;

    while(!_recvQueue.empty())
    {
        packet = _recvQueue.next();
        delete packet;
    }
}

void WorldSession::SetSocket(WorldSocket *sock)
{
    _socket = sock;
}

void WorldSession::SendPacket(WorldPacket* packet)
{
    if (_socket)
        _socket->SendPacket(packet);
}

void WorldSession::QueuePacket(WorldPacket& packet)
{
    WorldPacket *pck = new WorldPacket(packet);
    ASSERT(pck);

    _recvQueue.add(pck);
}

bool WorldSession::Update(uint32 diff)
{
    WorldPacket *packet;
    OpcodeHandler *table = _GetOpcodeHandlerTable();
    uint32 i;

    while (!_recvQueue.empty())
    {
        packet = _recvQueue.next();

        if(packet==NULL)
            continue;

        for (i = 0; table[i].handler != NULL; i++)
        {
            if (table[i].opcode == packet->GetOpcode())
            {
                if (table[i].status == STATUS_AUTHED ||
                    (table[i].status == STATUS_LOGGEDIN && _player))
                {
                    (this->*table[i].handler)(*packet);
                }
                else
                {
                    sLog.outError( "SESSION: received unexpected opcode %s (0x%.4X)",
                        LookupName(packet->GetOpcode(), g_worldOpcodeNames),
                        packet->GetOpcode());
                }

                break;
            }
        }

        if (table[i].handler == NULL)
            sLog.outError( "SESSION: received unhandled opcode %s (0x%.4X)",
                LookupName(packet->GetOpcode(), g_worldOpcodeNames),
                packet->GetOpcode());

        delete packet;
    }

    time_t currTime = time(NULL);
    if (!_socket || ShouldLogOut(currTime))
        LogoutPlayer(true);

    if (!_socket)
        return false;

    return true;
}

void WorldSession::LogoutPlayer(bool Save)
{
    if (_player)
    {
        // logging out just after died
        if (_player->GetDeathTimer())
        {
            _player->KillPlayer();
            _player->BuildPlayerRepop();
        }

        sDatabase.PExecute("UPDATE `character` SET `online` = 0 WHERE `guid` = '%u'", _player->GetGUIDLow());
        loginDatabase.PExecute("UPDATE `account` SET `online` = 0 WHERE `id` = '%u'", GetAccountId());

        if (_player->IsInGroup())
        {
            Group *group;
            group = objmgr.GetGroupByLeader(_player->GetGroupLeader());
            if(group!=NULL)
            {
                if (group->RemoveMember(_player->GetGUID()) > 1)
                    group->SendUpdate();
                else
                {
                    group->Disband();
                    objmgr.RemoveGroup(group);

                    delete group;
                }
            }
            _player->UnSetInGroup();
        }
        Guild *guild;
        guild = objmgr.GetGuildById(_player->GetGuildId());
        if(guild)
        {
            guild->LoadPlayerStatsByGuid(_player->GetGUID());

            WorldPacket data;
            data.Initialize(SMSG_GUILD_EVENT);
            data<<(uint8)GE_SIGNED_OFF;
            data<<(uint8)1;
            data<<_player->GetName();
            data<<(uint8)0<<(uint8)0<<(uint8)0;
            guild->BroadcastPacket(&data);
        }
        _player->UnsummonPet();
        _player->Uncharm();
        _player->UnsummonTotem();
        _player->InvisiblePjsNear.clear();

        ObjectAccessor::Instance().RemovePlayer(_player);
        MapManager::Instance().GetMap(_player->GetMapId())->Remove(_player, false);

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

        WorldPacket data;
        data.Initialize(SMSG_FRIEND_STATUS);
        data<<uint8(FRIEND_OFFLINE);
        data<<_player->GetGUID();
        _player->BroadcastPacketToFriendListers(&data);

        delete _player;
        _player = 0;

        data.Initialize( SMSG_LOGOUT_COMPLETE );
        SendPacket( &data );

        sLog.outDebug( "SESSION: Sent SMSG_LOGOUT_COMPLETE Message" );
    }

    LogoutRequest(0);
}

void WorldSession::KickPlayer()
{
    if(!_socket)
        return;

    LogoutPlayer(true);

    // session will removed in next update tick
    _socket->SetCloseAndDelete(true);
}

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

        // new
        { CMSG_FORCE_MOVE_UNROOT_ACK,       STATUS_LOGGEDIN, &WorldSession::HandleMoveUnRootAck                 },
        { CMSG_FORCE_MOVE_ROOT_ACK,         STATUS_LOGGEDIN, &WorldSession::HandleMoveRootAck                   },
        { CMSG_MOVE_WATER_WALK_ACK,         STATUS_LOGGEDIN, &WorldSession::HandleMoveWaterWalkAck              },

        // repair item
        { CMSG_REPAIR_ITEM,                 STATUS_LOGGEDIN, &WorldSession::HandleRepairItemOpcode              },

        { CMSG_SET_ACTIVE_MOVER,            STATUS_LOGGEDIN, &WorldSession::HandleSetActiveMoverOpcode          },
        { MSG_LOOKING_FOR_GROUP,            STATUS_LOGGEDIN, &WorldSession::HandleLookingForGroup               },
        { MSG_MOVE_TELEPORT_ACK,            STATUS_LOGGEDIN, &WorldSession::HandleMoveTeleportAck               },
        { CMSG_FORCE_RUN_SPEED_CHANGE_ACK,  STATUS_LOGGEDIN, &WorldSession::HandleForceRunSpeedChangeAck        },
        { CMSG_FORCE_SWIM_SPEED_CHANGE_ACK, STATUS_LOGGEDIN, &WorldSession::HandleForceSwimSpeedChangeAck       },
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

        { MSG_MOVE_WORLDPORT_ACK,           STATUS_LOGGEDIN, &WorldSession::HandleMoveWorldportAckOpcode        },
        { MSG_MOVE_HEARTBEAT,               STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_JUMP,                    STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_FORWARD,           STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_BACKWARD,          STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_SET_FACING,              STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_STOP,                    STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_STRAFE_LEFT,       STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_STRAFE_RIGHT,      STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_STOP_STRAFE,             STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_TURN_LEFT,         STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_TURN_RIGHT,        STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_STOP_TURN,               STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_PITCH_UP,          STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_PITCH_DOWN,        STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_STOP_PITCH,              STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_SET_RUN_MODE,            STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_SET_WALK_MODE,           STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_SET_PITCH,               STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_START_SWIM,              STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_STOP_SWIM,               STATUS_LOGGEDIN, &WorldSession::HandleMovementOpcodes               },
        { MSG_MOVE_FALL_LAND,               STATUS_LOGGEDIN, &WorldSession::HandleFallOpcode                    },
        { CMSG_MOUNTSPECIAL_ANIM,           STATUS_LOGGEDIN, &WorldSession::HandleMountSpecialAnimOpcode        },

        { CMSG_GROUP_INVITE,                STATUS_LOGGEDIN, &WorldSession::HandleGroupInviteOpcode             },
        { CMSG_GROUP_CANCEL,                STATUS_LOGGEDIN, &WorldSession::HandleGroupCancelOpcode             },
        { CMSG_GROUP_ACCEPT,                STATUS_LOGGEDIN, &WorldSession::HandleGroupAcceptOpcode             },
        { CMSG_GROUP_DECLINE,               STATUS_LOGGEDIN, &WorldSession::HandleGroupDeclineOpcode            },
        { CMSG_GROUP_UNINVITE,              STATUS_LOGGEDIN, &WorldSession::HandleGroupUninviteOpcode           },
        { CMSG_GROUP_UNINVITE_GUID,         STATUS_LOGGEDIN, &WorldSession::HandleGroupUninviteGuildOpcode      },
        { CMSG_GROUP_SET_LEADER,            STATUS_LOGGEDIN, &WorldSession::HandleGroupSetLeaderOpcode          },
        { CMSG_GROUP_DISBAND,               STATUS_LOGGEDIN, &WorldSession::HandleGroupDisbandOpcode            },
        { CMSG_LOOT_METHOD,                 STATUS_LOGGEDIN, &WorldSession::HandleLootMethodOpcode              },
        { CMSG_LOOT_ROLL,                   STATUS_LOGGEDIN, &WorldSession::HandleLootRoll                      },
        { CMSG_REQUEST_PARTY_MEMBER_STATS,  STATUS_LOGGEDIN, &WorldSession::HandleRequestPartyMemberStatsOpcode },
        { CMSG_REQUEST_RAID_INFO,           STATUS_LOGGEDIN, &WorldSession::HandleRequestRaidInfoOpcode         },
        { MSG_RAID_ICON_TARGET,             STATUS_LOGGEDIN, &WorldSession::HandleRaidIconTargetOpcode          },

        { CMSG_PETITION_SHOWLIST,           STATUS_LOGGEDIN, &WorldSession::HandlePetitionShowListOpcode        },
        { CMSG_PETITION_BUY,                STATUS_LOGGEDIN, &WorldSession::HandlePetitionBuyOpcode             },
        { CMSG_PETITION_SHOW_SIGNATURES,    STATUS_LOGGEDIN, &WorldSession::HandlePetitionShowSignOpcode        },
        { CMSG_PETITION_QUERY,              STATUS_LOGGEDIN, &WorldSession::HandlePetitionQueryOpcode           },
        { MSG_PETITION_RENAME,              STATUS_LOGGEDIN, &WorldSession::HandlePetitionRenameOpcode          },
        { CMSG_PETITION_SIGN,               STATUS_LOGGEDIN, &WorldSession::HandlePetitionSignOpcode            },
        { MSG_PETITION_DECLINE,             STATUS_LOGGEDIN, &WorldSession::HandlePetitionDeclineOpcode         },
        { CMSG_OFFER_PETITION,              STATUS_LOGGEDIN, &WorldSession::HandleOfferPetitionOpcode           },
        { CMSG_TURN_IN_PETITION,            STATUS_LOGGEDIN, &WorldSession::HandleTurnInPetitionOpcode          },
        { CMSG_MOVE_FALL_RESET,             STATUS_LOGGEDIN, &WorldSession::HandleMoveFallResetOpcode           },

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
        { CMSG_TAXIQUERYAVAILABLENODES,     STATUS_LOGGEDIN, &WorldSession::HandleTaxiQueryAviableNodesOpcode   },
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
        { MSG_LIST_STABLED_PETS ,           STATUS_LOGGEDIN, &WorldSession::HandleListStabledPetsOpcode         },

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

        { CMSG_ATTACKSWING,                 STATUS_LOGGEDIN, &WorldSession::HandleAttackSwingOpcode             },
        { CMSG_ATTACKSTOP,                  STATUS_LOGGEDIN, &WorldSession::HandleAttackStopOpcode              },
        { CMSG_SETSHEATHED,                 STATUS_LOGGEDIN, &WorldSession::HandleSetSheathedOpcode             },

        { CMSG_USE_ITEM,                    STATUS_LOGGEDIN, &WorldSession::HandleUseItemOpcode                 },
        { CMSG_OPEN_ITEM,                   STATUS_LOGGEDIN, &WorldSession::HandleOpenItemOpcode                },
        { CMSG_CAST_SPELL,                  STATUS_LOGGEDIN, &WorldSession::HandleCastSpellOpcode               },
        { CMSG_CANCEL_CAST,                 STATUS_LOGGEDIN, &WorldSession::HandleCancelCastOpcode              },
        { CMSG_CANCEL_AURA,                 STATUS_LOGGEDIN, &WorldSession::HandleCancelAuraOpcode              },
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
        //{ CMSG_BATTLEMASTER_HELLO,          STATUS_LOGGEDIN, &WorldSession::HandleBattleMasterHelloOpcode     },
        { CMSG_BATTLEMASTER_HELLO,          STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundHelloOpcode       },
        { CMSG_BATTLEMASTER_JOIN,           STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundJoinOpcode        },
        { MSG_BATTLEGROUND_PLAYER_POSITIONS,STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundPlayerPositionsOpcode },
        { MSG_PVP_LOG_DATA,                 STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundPVPlogdataOpcode  },
        { CMSG_BATTLEFIELD_PORT,            STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundPlayerPortOpcode  },
        { CMSG_BATTLEFIELD_LIST,            STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundListOpcode        },
        { CMSG_LEAVE_BATTLEFIELD,           STATUS_LOGGEDIN, &WorldSession::HandleBattleGroundLeaveOpcode       },

        { CMSG_SET_ACTIONBAR_TOGGLES,       STATUS_LOGGEDIN, &WorldSession::HandleSetActionBar                  },
        { CMSG_FIELD_WATCHED_FACTION_SHOW_BAR, STATUS_LOGGEDIN, &WorldSession::HandleSetWatchedFactionIndexOpcode },

        { CMSG_LOOT_ROLL,                   STATUS_LOGGEDIN, &WorldSession::HandleLootRoll                      },
        { CMSG_WARDEN_DATA,                 STATUS_LOGGEDIN, &WorldSession::HandleWardenDataOpcode              },
        { CMSG_WORLD_TELEPORT,              STATUS_LOGGEDIN, &WorldSession::HandleWorldTeleportOpcode           },
        { MSG_MINIMAP_PING,                 STATUS_LOGGEDIN, &WorldSession::HandleMinimapPingOpcode             },
        { MSG_RANDOM_ROLL,                  STATUS_LOGGEDIN, &WorldSession::HandleRandomRollOpcode              },

        { 0,                                0,               NULL                                               }
    };

    return table;
}

void WorldSession::HandleCancelChanneling( WorldPacket & recv_data )
{
    uint32 spellid;
    recv_data >> spellid;
}
