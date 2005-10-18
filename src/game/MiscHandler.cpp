/* MiscHandler.cpp
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
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "Auth/BigNumber.h"
#include "Auth/Sha1.h"
#include "UpdateData.h"
#include "LootMgr.h"
#include <zlib/zlib.h>

#ifdef ENABLE_GRID_SYSTEM
#include "MapManager.h"
#include "ObjectAccessor.h"
#endif

void WorldSession::HandleRepopRequestOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_REPOP_REQUEST Message" );

    GetPlayer()->BuildPlayerRepop();
    GetPlayer()->RepopAtGraveyard();
}


void WorldSession::HandleAutostoreLootItemOpcode( WorldPacket & recv_data )
{
    uint8 i, slot = 0;
    uint32 itemid = 0;
    uint8 lootSlot = 0;
    WorldPacket data;
#ifndef ENABLE_GRID_SYSTEM
    Creature* pCreature = objmgr.GetObject<Creature>(GetPlayer()->GetLootGUID());
#else
    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*_player, _player->GetLootGUID());
#endif
    if (!pCreature)
        return;

    recv_data >> lootSlot;
    lootSlot -=1;                                 //to prevent Slot 0 from been used "Still Rolling for item fix"

    for(i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        if (GetPlayer()->GetItemBySlot(i) == NULL)
        {
            slot = i;
            break;
        }
    }

    if (slot == 0)
    {
        // Our User doesn't have a free Slot in there bag
        data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
        data << uint8(48);                        // Inventory Full
        data << uint64(0);
        data << uint64(0);
        data << uint8(0);
        SendPacket( &data );
        return;
    }

    if (pCreature->getItemAmount(lootSlot) == 0)  //Can't sell the item for cash
        return;

    itemid = pCreature->getItemId(lootSlot);
    pCreature->setItemAmount(lootSlot, 0);

    Item *item = new Item();
    ASSERT(item);

    item->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), itemid, GetPlayer());
    GetPlayer()->AddItemToSlot(slot, item);

    data.Initialize( SMSG_LOOT_REMOVED );
    data << uint8(lootSlot+1);
    SendPacket( &data );

}

void WorldSession::HandleLootMoneyOpcode( WorldPacket & recv_data )
{

    WorldPacket data;

    uint32 newcoinage = 0;
#ifndef ENABLE_GRID_SYSTEM
    Creature* pCreature = objmgr.GetObject<Creature>(GetPlayer()->GetLootGUID());
#else
    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*_player, _player->GetLootGUID());
#endif
    if (!pCreature)
        return;

    newcoinage = GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE) + pCreature->getLootMoney();
    pCreature->setLootMoney(0);
    GetPlayer()->SetUInt32Value( PLAYER_FIELD_COINAGE , newcoinage);
}

extern int num_item_prototypes;
extern uint32 item_proto_ids[64550];

void WorldSession::HandleLootOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    uint16 tmpDataLen;
    uint8 i, tmpItemsCount = 0;
    ItemPrototype *tmpLootItem;
    WorldPacket data;
	uint16 num_loot_items = 0;
	uint32 loot_items_list[16];

    recv_data >> guid;

#ifndef ENABLE_GRID_SYSTEM
    Creature* pCreature = objmgr.GetObject<Creature>(guid);
#else
    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
#endif
    if (!pCreature)
        return;

    GetPlayer()->SetLootGUID(guid);

    for(i = 0; i < pCreature->getItemCount() ; i++)
    {
        if (pCreature->getItemAmount((int)i) > 0)
            tmpItemsCount++;
    }

    tmpDataLen = 14 + tmpItemsCount*22;

    data.Initialize( SMSG_LOOT_RESPONSE );

    data << guid;
    // 0 =  Premission Denied | 1 = 4 = 5 = 2 = Death | 3 = Fishing
    data << uint8(1);                             //loot Type
    data << uint32(pCreature->getLootMoney());

	if (pCreature->getItemCount() > 0)
	{
	    data << uint8(tmpItemsCount);
	    // for(i = 0; i < tmpItemsCount ; i++)
		for(i = 0; i<pCreature->getItemCount() ; i++)
		{
			if (pCreature->getItemAmount((int)i) > 0)
			{
				data << uint8(i+1);                   //Item Slot, must be > 0
				tmpLootItem = objmgr.GetItemPrototype(pCreature->getItemId((int)i));
				//item ID
				data << uint32(pCreature->getItemId((int)i));
				//quantity
				data << uint32(pCreature->getItemAmount((int)i));
				//Display IconID
				data << uint32(tmpLootItem->DisplayInfoID);
				data << uint8(0) << uint32(0) << uint32(0);
			}
		}
	}
	else
	{// UQ1: Generate some random loot items...
		uint32 level = pCreature->getLevel();
		int number_of_items = irand(0, 12);
		int tries = 0;

		num_loot_items = 0;

		if (number_of_items > 4 && _player->getLevel() < 5)
			number_of_items = irand(1, 4);

		if (number_of_items > 6 && _player->getLevel() < 10)
			number_of_items = irand(1, 6);

		if (number_of_items > 8 && _player->getLevel() < 20)
			number_of_items = irand(1, 8);

		if (number_of_items > 10 && _player->getLevel() < 40)
			number_of_items = irand(1, 10);

		for(i = 0; i<number_of_items ; i++)
		{
			uint32 loot_item;


			if (num_item_prototypes > 32768)
				loot_item = irand(0, 32768) + irand(0, (num_item_prototypes-32768));
			else
				loot_item = irand(0, num_item_prototypes);

			tmpLootItem = objmgr.GetItemPrototype(item_proto_ids[loot_item]);

			while (!(tmpLootItem && tmpLootItem->DisplayInfoID) 
				|| tmpLootItem->ItemLevel > _player->getLevel()*1.5 
				|| tmpLootItem->Field107 == -1) // Quest Item ???
			{
				/*if (!tmpLootItem)
				{
					Log::getSingleton( ).outDebug( "Loot item ID: %u does not exist.", item_proto_ids[loot_item]);   
				}
				else if (!tmpLootItem->DisplayInfoID)
				{
					Log::getSingleton( ).outDebug( "Loot item ID: %u has no display ID.", item_proto_ids[loot_item]);   
				}
				else if (tmpLootItem->ItemLevel > _player->getLevel()*1.5)
				{
					Log::getSingleton( ).outDebug( "Loot item ID: %u is too higher level for player (%u > %u).", item_proto_ids[loot_item], tmpLootItem->ItemLevel, _player->getLevel()*1.5);   
				}*/

				if (num_item_prototypes > 32768)
					loot_item = irand(0, 32768) + irand(0, (num_item_prototypes-32768));
				else
					loot_item = irand(0, (num_item_prototypes-32768));

				tmpLootItem = objmgr.GetItemPrototype(item_proto_ids[loot_item]);
				tries++;

				if (tries >= 50)
					break;
			}

			if (tries >= 50)
				break;

			loot_items_list[num_loot_items] = item_proto_ids[loot_item];

			// Add item to the creature for the next loot...
			pCreature->setItemId(pCreature->getItemCount(), item_proto_ids[loot_item]);
            pCreature->setItemAmount(pCreature->getItemCount(), 1);
			pCreature->increaseItemCount();

			num_loot_items++;
			tmpItemsCount++;
        }

		//tmpItemsCount--;
		Log::getSingleton( ).outDebug( "Randomly generated %i loot items (from %i prototypes).", num_loot_items, num_item_prototypes);   

		data << uint8(tmpItemsCount);

		for(i = 0; i<num_loot_items ; i++)
		{
			tmpLootItem = objmgr.GetItemPrototype(loot_items_list[i]);

			data << uint8(i+1);                   //Item Slot, must be > 0
			//item ID
			data << uint32(loot_items_list[i]);
			//quantity
			data << uint32(1);
			//Display IconID
			data << uint32(tmpLootItem->DisplayInfoID);
			data << uint8(0) << uint32(0) << uint32(0);
		}

		// For assertion below...
		tmpDataLen = data.size();
	}

    WPAssert(data.size() == tmpDataLen);
    SendPacket( &data );
}


void WorldSession::HandleLootReleaseOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    WorldPacket data;

    recv_data >> guid;

    GetPlayer()->SetLootGUID(0);

    data.Initialize( SMSG_LOOT_RELEASE_RESPONSE );
    data << guid << uint8( 1 );
    SendPacket( &data );
}


void WorldSession::HandleWhoOpcode( WorldPacket & recv_data )
{
    uint64 clientcount = 0;
    int datalen = 8;
    int countcheck = 0;
    WorldPacket data;

    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_WHO Message" );
#ifndef ENABLE_GRID_SYSTEM
    ObjectMgr::PlayerMap::const_iterator itr;
    for (itr = objmgr.Begin<Player>(); itr != objmgr.End<Player>(); itr++)
#else
	ObjectAccessor::PlayerMapType &m(ObjectAccessor::Instance().GetPlayers());
    for(ObjectAccessor::PlayerMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
#endif
    {
        if ( itr->second->GetName() )
        {
            clientcount++;

            datalen = datalen + strlen(itr->second->GetName()) + 1 + 21;
        }
    }

    data.Initialize( SMSG_WHO );
    data << uint64( clientcount );

#ifndef ENABLE_GRID_SYSTEM
    for (itr = objmgr.Begin<Player>(); itr != objmgr.End<Player>(); itr++)
#else
	for(ObjectAccessor::PlayerMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
#endif
    {
        if ( itr->second->GetName() && (countcheck  < clientcount))
        {
            countcheck++;

            data.append(itr->second->GetName() , strlen(itr->second->GetName()) + 1);
            data << uint8( 0x00 );
            data << uint32( itr->second->getLevel() );
            data << uint32( itr->second->getClass() );
            data << uint32( itr->second->getRace() );
            data << uint32( itr->second->GetZoneId() );

            data << uint32( 0x00000000 );         // this is the guild id...once we have guilds working it'll go here.
        }
    }

    WPAssert(data.size() == datalen);
    SendPacket(&data);
}


void WorldSession::HandleLogoutRequestOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_LOGOUT_REQUEST Message" );

    if( !(GetPlayer()->inCombat) ){ //Nao esta em combate
      data.Initialize( SMSG_LOGOUT_RESPONSE );
      data << uint32(0); //Filler
      data << uint8(0); //Logout accepted
      SendPacket( &data );
      //Essa funcao coloca o tempo atual numa variavel chamada _logouttime (WorldSession.h)
	  //Essa variavel eh verificada a cada WorldSession::Update atraves da funcao ShouldLogout()
	  //Caso essa funcao retorne TRUE entao o logout sera efetuado!
      LogoutRequest(time(NULL));
    }
    else {
	  data.Initialize( SMSG_LOGOUT_RESPONSE );
	  data << (uint8)0xC; //Logout not accepted
	  data << uint32(0);  //Filler
	  data << uint8(0);
	  SendPacket( &data );

	  LogoutRequest(0);
    }
}


void WorldSession::HandlePlayerLogoutOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_PLAYER_LOGOUT Message" );

//    LogoutRequest(1);
}


void WorldSession::HandleLogoutCancelOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_LOGOUT_CANCEL Message" );

    LogoutRequest(0);

    data.Initialize( SMSG_LOGOUT_CANCEL_ACK );
    SendPacket( &data );

    Log::getSingleton( ).outDebug( "WORLD: sent SMSG_LOGOUT_CANCEL_ACK Message" );
}


void WorldSession::HandleGMTicketGetTicketOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    data.Initialize( SMSG_GMTICKET_GETTICKET );
    data << uint32(6);
    data << uint32(0xffbfbfbf);
    data << uint8(0);
    data << uint8(3);

    SendPacket( &data );
}


void WorldSession::HandleGMTicketCreateOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    // TODO: Receive message sent and relay it to an online GM
    data.Initialize( SMSG_GMTICKET_CREATE );
    data << uint32(2);

    SendPacket( &data );
}


void WorldSession::HandleGMTicketSystemStatusOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    // TODO: Receive message sent and relay it to an online GM
    data.Initialize( SMSG_GMTICKET_SYSTEMSTATUS );
    data << uint32(1);

    SendPacket( &data );
}

#include "Object.h"

void WorldSession::HandleZoneUpdateOpcode( WorldPacket & recv_data )
{
    uint32 newZone,oldZone;
    WPAssert(GetPlayer());

    recv_data >> newZone;
    Log::getSingleton( ).outDetail("WORLD: Recvd ZONE_UPDATE: %u", newZone);

    if (GetPlayer()->GetZoneId() == newZone)
        return;

    oldZone = GetPlayer( )->GetZoneId();

    // Setting new zone
    GetPlayer()->SetZoneId((uint16)newZone);
}


void WorldSession::HandleSetTargetOpcode( WorldPacket & recv_data )
{
    uint64 guid ;
    recv_data >> guid;

    if( GetPlayer( ) != 0 )
    {
        GetPlayer( )->SetTarget(guid);
    }
}


void WorldSession::HandleSetSelectionOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    recv_data >> guid;

    if( GetPlayer( ) != 0 )
        GetPlayer( )->SetSelection(guid);
    else
        return;

    // if its a new Target set Combo Points Target to 0
    if(GetPlayer( )->GetUInt64Value(PLAYER__FIELD_COMBO_TARGET) != guid)
    {
        GetPlayer( )->SetUInt64Value(PLAYER__FIELD_COMBO_TARGET,0);
        GetPlayer( )->SetUInt32Value(PLAYER_FIELD_BYTES,((GetPlayer( )->GetUInt32Value(PLAYER_FIELD_BYTES) & ~(0xFF << 8)) | (0x00 << 8)));
    }
}


void WorldSession::HandleStandStateChangeOpcode( WorldPacket & recv_data )
{
    if( GetPlayer( ) != 0 )
    {
        // retrieve current BYTES
        uint32 bytes1 = GetPlayer( )->GetUInt32Value( UNIT_FIELD_BYTES_1 );
        uint8 bytes[4];

        // uint64 guid; no need for it in 0.12
        // recv_data >> guid;

        bytes[0] = uint8(bytes1 & 0xff);
        bytes[1] = uint8((bytes1>>8) & 0xff);
        bytes[2] = uint8((bytes1>>16) & 0xff);
        bytes[3] = uint8((bytes1>>24) & 0xff);

        // retrieve new stand state
        uint8 animstate;
        recv_data >> animstate;

        // if (bytes[0] == animstate) break;
        bytes[0] = animstate;

        uint32 newbytes = (bytes[0]) + (bytes[1]<<8) + (bytes[2]<<16) + (bytes[3]<<24);
        GetPlayer( )->SetUInt32Value(UNIT_FIELD_BYTES_1 , newbytes);
    }
}


void WorldSession::HandleFriendListOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_FRIEND_LIST"  );
    // TODO: send SMSG_FRIEND_LIST with a list of friend->fixed
    // <nothin>void Player::LoadFromDB( uint32 guid )
    // <nothin>add a new function in Player
    // <nothin>_LoadFriendList();
    // <nothin>and _SaveFriendList();
    // <Deadknight>ok
    // <Deadknight> after that one goes to logout other one?
    // <nothin>call _SaveFriendList(); in void Player::SaveToDB()
    // <Deadknight>ha
    // Deadknight
    unsigned char Counter=0;
    uint64 guid;
    std::stringstream query;
    std::stringstream query2;
    int i=0;
    Player* pObj;
    FriendStr friendstr[255];

    guid=GetPlayer()->GetGUID();

    query << "SELECT COUNT(*) FROM `social` where guid='" << guid << "'";
    QueryResult *result = sDatabase.Query( query.str().c_str() );

    if(result)
    {
        Field *fields = result->Fetch();
        Counter=fields[0].GetUInt32();

        query2 << "SELECT * FROM `social` where guid='" << guid << "'";
        result = sDatabase.Query( query2.str().c_str() );
        if(result)
        {
            fields = result->Fetch();
            friendstr[i].PlayerGUID = fields[2].GetUInt64();
#ifndef ENABLE_GRID_SYSTEM
            pObj=objmgr.GetObject<Player>(friendstr[i].PlayerGUID);
#else
	    pObj = ObjectAccessor::Instance().FindPlayer( friendstr[i].PlayerGUID );
#endif
            if(pObj && pObj->IsInWorld())
            {
                friendstr[i].Status = 1;
                friendstr[i].Area = pObj->GetZoneId();
                friendstr[i].Level = pObj->getLevel();
                friendstr[i].Class = pObj->getClass();
                i++;
            }
            else
            {
                friendstr[i].Status = 0;
                friendstr[i].Area = 0;
                friendstr[i].Level = 0;
                friendstr[i].Class = 0;
                i++;
            }

            while( result->NextRow() )
            {
                friendstr[i].PlayerGUID = fields[2].GetUInt64();
#ifndef ENABLE_GRID_SYSTEM
                pObj = objmgr.GetObject<Player>(friendstr[i].PlayerGUID);
#else
		pObj = ObjectAccessor::Instance().FindPlayer(friendstr[i].PlayerGUID);
#endif
                if(pObj)
                {
                    friendstr[i].Status = 1;
                    friendstr[i].Area = pObj->GetZoneId();
                    friendstr[Counter].Level = pObj->getLevel();
                    friendstr[Counter].Class = pObj->getClass();
                    i++;
                }
                else
                {
                    friendstr[i].Status = 0;
                    friendstr[i].Area = 0;
                    friendstr[Counter].Level = 0;
                    friendstr[Counter].Class = 0;
                    i++;
                }
            }
        }
    }
    else
    {
        sLog.outError("There is no social table");
    }
    //Finish

    data.Initialize( SMSG_FRIEND_LIST );
    data << Counter;

    for (int j=0; j<Counter; j++)
    {
        // adding friend
        Log::getSingleton( ).outDetail( "WORLD: Adding Friend - Guid:%ld, Status:%d, Area:%d, Level:%d Class:%d",friendstr[j].PlayerGUID, friendstr[j].Status, friendstr[j].Area,friendstr[j].Level,friendstr[j].Class  );

        data << friendstr[j].PlayerGUID << friendstr[j].Status ;
        if (friendstr[j].Status != 0)
            data << friendstr[j].Area << friendstr[j].Level << friendstr[j].Class;
    }

    SendPacket( &data );

    Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_FRIEND_LIST)" );
}


void WorldSession::HandleAddFriendOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_ADD_FRIEND"  );

    std::string friendName = "UNKNOWN";
	std::stringstream fquery;
	unsigned char friendResult = FRIEND_NOT_FOUND;
	Player *pfriend=NULL;
	uint64 friendGuid = 0;
    uint32 friendArea = 0, friendLevel = 0, friendClass = 0;
	WorldPacket data;

    recv_data >> friendName;

    Log::getSingleton( ).outDetail( "WORLD: %s asked to add friend : '%s'",
        GetPlayer()->GetName(), friendName.c_str() );
	
    
   
    friendGuid = objmgr.GetPlayerGUIDByName(friendName.c_str());
#ifndef ENABLE_GRID_SYSTEM
	pfriend = objmgr.GetObject<Player>(friendGuid);
#else
	pfriend = ObjectAccessor::Instance().FindPlayer(friendGuid);
#endif
	fquery << "SELECT * FROM social WHERE friendid = " << friendGuid;

	if(sDatabase.Query( fquery.str().c_str() )) friendResult = FRIEND_ALREADY;
	if (!strcmp(GetPlayer()->GetName(),friendName.c_str())) friendResult = FRIEND_SELF;
	
	// Send response.
    data.Initialize( SMSG_FRIEND_STATUS );

    if (friendGuid > 0 && friendResult!=FRIEND_ALREADY && friendResult!=FRIEND_SELF)
    {
        if( pfriend != NULL && pfriend->IsInWorld())
		{
            friendResult = FRIEND_ADDED_ONLINE;
			friendArea = pfriend->GetZoneId();
			friendLevel = pfriend->getLevel();
			friendClass = pfriend->getClass();
			
			data << (uint8)friendResult << (uint64)friendGuid;
            data << (uint32)friendArea << (uint32)friendLevel << (uint32)friendClass;
			
			// Deadknight addon start add buffer to social table
			std::stringstream query;
			uint64 guid;
			guid=GetPlayer()->GetGUID();

			query << "INSERT INTO `social` VALUES ('" << friendName << "', " << guid << ", " << friendGuid << ", 'FRIEND')" ;
			sDatabase.Query( query.str().c_str() );	

		}
        else
            friendResult = FRIEND_ADDED_OFFLINE;

        Log::getSingleton( ).outDetail( "WORLD: %s Guid found '%ld' area:%d Level:%d Class:%d. ",
            friendName.c_str(), friendGuid, friendArea, friendLevel, friendClass);

    }
	else if(friendResult==FRIEND_ALREADY)
	{
		data << (uint8)friendResult << (uint64)friendGuid;
		Log::getSingleton( ).outDetail( "WORLD: %s Guid Already a Friend ", friendName.c_str() );
	}
	else if(friendResult==FRIEND_SELF)
	{
		data << (uint8)friendResult << (uint64)friendGuid;
		Log::getSingleton( ).outDetail( "WORLD: %s Guid Is you! ", friendName.c_str() );
	}
	else
	{
		data << (uint8)friendResult << (uint64)friendGuid;
        Log::getSingleton( ).outDetail( "WORLD: %s Guid not found ", friendName.c_str() );
	}

    // Finish
    SendPacket( &data );

    Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_FRIEND_STATUS)" );
}


void WorldSession::HandleDelFriendOpcode( WorldPacket & recv_data )
{
    uint64 FriendGUID;
    WorldPacket data;

    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_DEL_FRIEND"  );
    recv_data >> FriendGUID;

    unsigned char FriendResult = FRIEND_REMOVED;

    int FriendArea = 0;
    int FriendLevel = 0;
    int FriendClass = 0;

    // TODO: Delete Friend from list, and fill in FriendResult.//finished

    // Send response.
    data.Initialize( SMSG_FRIEND_STATUS );

    data << (uint8)FriendResult << (uint64)FriendGUID;

    // Deadknight Addon remove from social table
    std::stringstream query;
    uint64 guid;
    guid=GetPlayer()->GetGUID();

    query << "DELETE FROM `social` WHERE `guid`=" << guid << " AND `friendid`=" << FriendGUID;
    sDatabase.Query( query.str().c_str() );
    // Finish

    SendPacket( &data );

    Log::getSingleton( ).outDebug( "WORLD: Sent motd (SMSG_FRIEND_STATUS)" );
}


void WorldSession::HandleBugOpcode( WorldPacket & recv_data )
{
    uint32 suggestion, contentlen;
    std::string content;
    uint32 typelen;
    std::string type;

    recv_data >> suggestion >> contentlen >> content >> typelen >> type;

    if( suggestion == 0 )
        Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_BUG [Bug Report]" );
    else
        Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_BUG [Suggestion]" );

    Log::getSingleton( ).outDebug( type.c_str( ) );
    Log::getSingleton( ).outDebug( content.c_str( ) );
}


/*void WorldSession::HandleJoinChannelOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    std::string channelName;
    recv_data >> channelName;
    data.Initialize(SMSG_CHANNEL_NOTIFY);
    data << uint8(2);
    data << channelName;
    data << uint32(1);
    SendPacket( &data );
}


void WorldSession::HandleLeaveChannelOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    std::string channelName;
    recv_data >> channelName;
    data.Initialize(SMSG_CHANNEL_NOTIFY);
    data << uint8(3);
    data << channelName;
    SendPacket( &data );
}
*/


void WorldSession::HandleCorpseReclaimOpcode(WorldPacket &recv_data)
{
    Log::getSingleton().outDetail("WORLD: Received CMSG_RECLAIM_CORPSE");

    uint64 guid;
    recv_data >> guid;

    GetPlayer()->SetMovement(MOVE_LAND_WALK);
    GetPlayer()->SetMovement(MOVE_UNROOT);

    GetPlayer( )->SetPlayerSpeed(RUN, (float)7.5, true);
    GetPlayer( )->SetPlayerSpeed(SWIM, (float)4.9, true);

    GetPlayer( )->SetUInt32Value(CONTAINER_FIELD_SLOT_1+29, 0);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURA+32, 0);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURALEVELS+8, 0xeeeeeeee);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+8, 0xeeeeeeee);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURAFLAGS+4, 0);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURASTATE, 0);

    GetPlayer()->ResurrectPlayer();
    GetPlayer()->SetUInt32Value(UNIT_FIELD_HEALTH, (uint32)(GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*0.50) );
    GetPlayer()->SpawnCorpseBones();
}


void WorldSession::HandleResurrectResponseOpcode(WorldPacket & recv_data)
{
    Log::getSingleton().outDetail("WORLD: Received CMSG_RESURRECT_RESPONSE");

    if(GetPlayer()->isAlive())
        return;

    WorldPacket data;
    uint64 guid;
    uint8 status;
    recv_data >> guid;
    recv_data >> status;

    if(status != 0)
        return;

    if(GetPlayer()->m_resurrectGUID == 0)
        return;

    GetPlayer( )->SetMovement(MOVE_LAND_WALK);
    GetPlayer( )->SetMovement(MOVE_UNROOT);
    GetPlayer( )->SetPlayerSpeed(RUN, (float)7.5, true);
    GetPlayer( )->SetPlayerSpeed(SWIM, (float)4.9, true);

    GetPlayer( )->SetUInt32Value(CONTAINER_FIELD_SLOT_1+29, 0);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURA+32, 0);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURALEVELS+8, 0xeeeeeeee);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+8, 0xeeeeeeee);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURAFLAGS+4, 0);
    GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURASTATE, 0);

    GetPlayer()->ResurrectPlayer();
    GetPlayer()->GetUInt32Value(UNIT_FIELD_HEALTH) > GetPlayer()->m_resurrectHealth ? GetPlayer()->SetUInt32Value(UNIT_FIELD_HEALTH, GetPlayer()->m_resurrectHealth )
        : GetPlayer()->SetUInt32Value(UNIT_FIELD_HEALTH, GetPlayer()->GetUInt32Value(UNIT_FIELD_HEALTH) );
    GetPlayer()->GetUInt32Value(UNIT_FIELD_POWER1) > GetPlayer()->m_resurrectMana ? GetPlayer()->SetUInt32Value(UNIT_FIELD_POWER1, GetPlayer()->m_resurrectMana )
        : GetPlayer()->SetUInt32Value(UNIT_FIELD_POWER1, GetPlayer()->GetUInt32Value(UNIT_FIELD_POWER1) );
    GetPlayer()->SpawnCorpseBones();

    GetPlayer()->BuildTeleportAckMsg(&data, GetPlayer()->m_resurrectX, GetPlayer()->m_resurrectY, GetPlayer()->m_resurrectZ, GetPlayer()->GetOrientation());
    GetPlayer()->GetSession()->SendPacket(&data);
    GetPlayer()->SetPosition(GetPlayer()->m_resurrectX ,GetPlayer()->m_resurrectY ,GetPlayer()->m_resurrectZ,GetPlayer()->GetOrientation());
    GetPlayer()->m_resurrectGUID = 0;
    GetPlayer()->m_resurrectHealth = GetPlayer()->m_resurrectHealth = 0;
    GetPlayer()->m_resurrectX = GetPlayer()->m_resurrectY = GetPlayer()->m_resurrectZ = 0;
}


void WorldSession::HandleSetAmmoOpcode(WorldPacket & recv_data)
{
    uint32 ammoId;
    recv_data >> ammoId;
    GetPlayer()->SetUInt32Value(PLAYER_AMMO_ID,ammoId);

    return;
}


void WorldSession::HandleAreaTriggerOpcode(WorldPacket & recv_data)
{
    uint32 id;
    WorldPacket data;
    recv_data >> id;
	AreaTrigger * at = objmgr.GetAreaTrigger(id);

	if(at)
	{
		if(at->mapId = GetPlayer()->GetMapId())
		{
			GetPlayer()->BuildTeleportAckMsg(&data, at->X, at->Y, at->Z, 0.0f);
			SendPacket(&data);
			GetPlayer()->SetPosition(at->X, at->Y, at->Z, 0.0f);
			GetPlayer()->BuildHeartBeatMsg(&data);
			GetPlayer()->SendMessageToSet(&data, true);
		}
		else
		{
			data.Initialize(SMSG_TRANSFER_PENDING);
			data << uint32(0);

			SendPacket(&data);
#ifndef ENABLE_GRID_SYSTEM
			GetPlayer()->RemoveFromMap();
#else
			MapManager::Instance().GetMap(GetPlayer()->GetMapId())->RemoveFromMap(GetPlayer());
#endif
			data.Initialize(SMSG_NEW_WORLD);
			data << at->mapId << at->X << at->Y << at->Z << 0.0f;
			SendPacket( &data );

			GetPlayer()->SetMapId(at->mapId);
			GetPlayer()->SetPosition(at->X, at->Y, at->Z, 0.0f);
		}

		delete at;
	}
}


void WorldSession::HandleUpdateAccountData(WorldPacket &recv_data)
{
/*
    Log::getSingleton().outDetail("WORLD: Received CMSG_UPDATE_ACCOUNT_DATA");

    uint32 uiID, uiDecompressedSize;
    recv_data >> uiID;
    recv_data >> uiDecompressedSize;

    ByteBuffer buff(uiDecompressedSize);

    int err;
    if ( (err = uncompress(const_cast<uint8*>(buff.contents()), &uiDecompressedSize, const_cast<uint8*>(recv_data.contents()) + 8, (recv_data.size() - 8))) == Z_OK)
    {
        std::stringstream ss;
        ss << "UPDATE accounts SET uiconfig" << uiID << "=\"" << buff.contents() << "\" WHERE acct=" << GetAccountId() << " LIMIT 1";
        sDatabase.Execute(ss.str().c_str());
    }
*/
}


void WorldSession::HandleRequestAccountData(WorldPacket& recv_data)
{
    // this function should get uiconfigX from mysql table
    // and send it to the client.
    // it works, but it crashes server, dunno why,
    // here seems all to be fine .. but on exit from funct
    // exception is thrown
    Log::getSingleton().outDetail("WORLD: Received CMSG_REQUEST_ACCOUNT_DATA");

/*
    WorldPacket data;

    uint32 id;
    recv_data >> id;
    std::stringstream ss;

    ss << "SELECT uiconfig" << id << " FROM accounts WHERE acct=" << GetAccountId();
    QueryResult *result = sDatabase.Query(ss.str().c_str());
    if (result)
    {
        data.Initialize(SMSG_UPDATE_ACCOUNT_DATA);
        std::string res = result->Fetch()->GetString();
        ByteBuffer buf(res.length());
        buf.append(res.c_str(), res.length());
        uint32 destsize = (uint32)res.length();
        data << destsize;
        int err;
        if ( (err = compress(const_cast<uint8*>(data.contents()) + sizeof(uint32), &destsize, buf.contents(), buf.size())) != Z_OK)
        {
            Log::getSingleton().outDetail("Error while compressing ACCOUNT_DATA");
        }
        else SendPacket(&data);
    }
*/
}


void WorldSession::HandleSetActionButtonOpcode(WorldPacket& recv_data)
{
    Log::getSingleton( ).outString( "WORLD: Recieved CMSG_SET_ACTION_BUTTON" );
    uint8 button, misc, type;
    uint16 action;
    recv_data >> button >> action >> misc >> type;
    Log::getSingleton( ).outString( "BUTTON: %u ACTION: %u TYPE: %u MISC: %u", button, action, type, misc );
    if(action==0)
    {
        Log::getSingleton( ).outString( "MISC: Remove action from button %u", button );
        //remove the action button from the db
        GetPlayer()->removeAction(button);
    }
    else
    {
        if(type==64)
        {
            Log::getSingleton( ).outString( "MISC: Added Macro %u into button %u", action, button );
            GetPlayer()->addAction(button,action,misc,type);
        }
        else if(type==0)
        {
            Log::getSingleton( ).outString( "MISC: Added Action %u into button %u", action, button );
            GetPlayer()->addAction(button,action,type,misc);
        }
    }
}

/// cinema
void WorldSession::HandleCompleteCinema( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Player is watching cinema" );
}


void 
WorldSession::HandleGameObjectUseOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;
    recv_data >> guid;

    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_GAMEOBJ_USE Message [guid=%d]", guid);   
#ifndef ENABLE_GRID_SYSTEM
    GameObject *obj = objmgr.GetObject<GameObject>(guid);
#else
    GameObject *obj = ObjectAccessor::Instance().GetGameObject(*_player, guid);
#endif
    
    if( obj != NULL )
    {
	data.Initialize( SMSG_LOOT_RESPONSE );
	if( obj->FillLoot(*_player, &data) )
	{
	    _player->SetLootGUID(guid);
	    SendPacket(&data);
	}
    }
}
