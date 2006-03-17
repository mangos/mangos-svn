/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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
#include "Chat.h"
#include "ScriptCalls.h"
#include <zlib/zlib.h>
#include "MapManager.h"
#include "ObjectAccessor.h"



void my_esc( char * r, const char * s )
{
          int n = strlen( s ), i, j;
          for ( i = 0, j = 0; s[i]; i++ )
          {
            if ( s[i] == '"' || s[i] == '\\' || s[i] == '\'' ) r[j++] = '\\';
            r[j++] = s[i];
          }
          r[j] = 0;
}

void WorldSession::HandleRepopRequestOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_REPOP_REQUEST Message" );

    GetPlayer()->BuildPlayerRepop();
    GetPlayer()->RepopAtGraveyard();
}


void WorldSession::HandleAutostoreLootItemOpcode( WorldPacket & recv_data )
{
    uint8 slot = 0;
    uint32 itemid = 0;
    uint8 lootSlot = 0;
    WorldPacket data;
    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*_player, _player->GetLootGUID());

    if (!pCreature)
        return;

    recv_data >> lootSlot;
    lootSlot -=1;                                 


    slot = GetPlayer()->FindFreeItemSlot(INVTYPE_SLOT_ITEM);

    if (slot == INVENTORY_SLOT_ITEM_END)
    {
        
        data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
        data << uint8(48);                        
        data << uint64(0);
        data << uint64(0);
        data << uint8(0);
        SendPacket( &data );
        return;
    }

    if (pCreature->getItemAmount(lootSlot) == 0)  
        return;

    itemid = pCreature->getItemId(lootSlot);
    pCreature->setItemAmount(lootSlot, 0);

    Item *item = new Item();
    ASSERT(item);

    item->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), itemid, GetPlayer());
    GetPlayer()->AddItemToSlot(slot, item);
	item->UpdateStats();

    data.Initialize( SMSG_LOOT_REMOVED );
    data << uint8(lootSlot+1);
    SendPacket( &data );

}

void WorldSession::HandleLootMoneyOpcode( WorldPacket & recv_data )
{

    WorldPacket data;

    uint32 newcoinage = 0;
    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*_player, _player->GetLootGUID());

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

    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);

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
    
    data << uint8(1);                             
    data << uint32(pCreature->getLootMoney());

    if (pCreature->getItemCount() > 0)
    {
        data << uint8(tmpItemsCount);
        
        for(i = 0; i<pCreature->getItemCount() ; i++)
        {
            if (pCreature->getItemAmount((int)i) > 0)
            {
                data << uint8(i+1);                   
                tmpLootItem = objmgr.GetItemPrototype(pCreature->getItemId((int)i));
                
                data << uint32(pCreature->getItemId((int)i));
                
                data << uint32(pCreature->getItemAmount((int)i));
                
                data << uint32(tmpLootItem->DisplayInfoID);
                data << uint8(0) << uint32(0) << uint32(0);
            }
        }
    }
    else
    {
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
                || tmpLootItem->Field107 == -1) 
            {
                

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

            
            pCreature->setItemId(pCreature->getItemCount(), item_proto_ids[loot_item]);
            pCreature->setItemAmount(pCreature->getItemCount(), 1);
            pCreature->increaseItemCount();

            num_loot_items++;
            tmpItemsCount++;
        }

        
        Log::getSingleton( ).outDebug( "Randomly generated %i loot items (from %i prototypes).", num_loot_items, num_item_prototypes);   

        data << uint8(tmpItemsCount);

        for(i = 0; i<num_loot_items ; i++)
        {
            tmpLootItem = objmgr.GetItemPrototype(loot_items_list[i]);

            data << uint8(i+1);                   
            
            data << uint32(loot_items_list[i]);
            
            data << uint32(1);
            
            data << uint32(tmpLootItem->DisplayInfoID);
            data << uint8(0) << uint32(0) << uint32(0);
        }

        
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

    ObjectAccessor::PlayersMapType &m(ObjectAccessor::Instance().GetPlayers());
    for(ObjectAccessor::PlayersMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if ( itr->second->GetName() )
        {
            clientcount++;

            datalen = datalen + strlen(itr->second->GetName()) + 1 + 21;
        }
    }

    data.Initialize( SMSG_WHO );
    data << uint64( clientcount );

    for(ObjectAccessor::PlayersMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
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

            data << uint32( 0x00000000 );         
        }
    }

    WPAssert(data.size() == datalen);
    SendPacket(&data);
}


void WorldSession::HandleLogoutRequestOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_LOGOUT_REQUEST Message" );

    if( !(GetPlayer()->inCombat) ){ 
      data.Initialize( SMSG_LOGOUT_RESPONSE );
      data << uint32(0); 
      data << uint8(0); 
      SendPacket( &data );
      
      
      
      LogoutRequest(time(NULL));

      
	  GetPlayer()->SetUInt32Value(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT_DOWN_IN_GROUND); 
    }
    else {
      data.Initialize( SMSG_LOGOUT_RESPONSE );
      data << (uint8)0xC; 
      data << uint32(0);  
      data << uint8(0);
      SendPacket( &data );

      LogoutRequest(0);
    }
}


void WorldSession::HandlePlayerLogoutOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_PLAYER_LOGOUT Message" );


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
    data.Initialize( SMSG_QUERY_TIME_RESPONSE );
    data << (uint32)20;
    SendPacket( &data );

    uint64 guid;
    std::stringstream query,query1;
    Field *fields;
    guid = GetPlayer()->GetGUID();
    query << "SELECT COUNT(*) FROM `gmtickets` where guid='" << guid << "'";
    QueryResult *result = sDatabase.Query( query.str().c_str() );

        if (result)
        {
            int cnt;
            fields = result->Fetch();
            cnt = fields[0].GetUInt32();

            if ( cnt > 0 )
            {
                data.Initialize( SMSG_GMTICKET_GETTICKET );
                query1 << "SELECT * FROM `gmtickets` where guid='" << guid << "'";
                QueryResult *result = sDatabase.Query( query1.str().c_str() );
                fields = result->Fetch();

                DEBUG_LOG( "query=%s\n", query1.str().c_str() );
                char tickettext[255];
                strcpy( tickettext,fields[2].GetString() );
                data << uint32(6); 
                data.append((uint8 *)tickettext,strlen(tickettext)+1);
                SendPacket( &data );
            }
            else
            {
                data << uint32(1); 
                data << uint32(0);
                SendPacket( &data );
            }

        }
    delete result;
}


void WorldSession::HandleGMTicketUpdateTextOpcode( WorldPacket & recv_data )
{
        WorldPacket data;
        uint64 guid = GetPlayer()->GetGUID();
        std::string ticketText = "";
        char * p, p1[512];
        uint8 buf[516];
        memcpy( buf, recv_data.contents(), sizeof buf <recv_data.size() ? sizeof buf : recv_data.size() );
        p = (char *)buf + 1;
        my_esc( p1, (const char *)buf + 1 );
        std::stringstream ss;
        ticketText = p1;
        ss << "UPDATE `gmtickets` set ticket_text = '" << ticketText << "' WHERE guid = '" << guid << "'";
        sDatabase.Execute( ss.str( ).c_str( ) );
}

void WorldSession::HandleGMTicketDeleteOpcode( WorldPacket & recv_data )
{
        WorldPacket data;
        uint64 guid = GetPlayer()->GetGUID();
        std::stringstream ss;
        ss << "DELETE FROM `gmtickets` where guid='" << guid << "' LIMIT 1";
        sDatabase.Execute( ss.str( ).c_str( ) );
        data.Initialize( SMSG_GMTICKET_GETTICKET );
        data << uint32(1); 
        data << uint32(0);
        SendPacket( &data );
}


void WorldSession::HandleGMTicketCreateOpcode( WorldPacket & recv_data )
{
 
 
 
 
 
 
 
 
 
 

        WorldPacket data;
        uint64 guid;
        guid = GetPlayer()->GetGUID();
        std::string ticketText = "";
    	std::stringstream query;
	Field *fields;
        char * p, p1[512];
        uint8 buf[516];
        int   cat[] = { 0,5,1,2,0,6,4,7,0,8,3 };
        memcpy( buf, recv_data.contents(), sizeof buf < recv_data.size() ? sizeof buf : recv_data.size() );
        buf[272] = 0;
        p = (char *)buf + 17;
        my_esc( p1, (const char *)buf + 17 );
        std::stringstream ss;
        ticketText = p1;

    query << "SELECT COUNT(*) FROM `gmtickets` where guid='" << guid << "'";
    QueryResult *result = sDatabase.Query( query.str().c_str() );

        if (result)
        {
            int cnt;
            fields = result->Fetch();
            cnt = fields[0].GetUInt32();


            if ( cnt > 0 )
            {
        data.Initialize( SMSG_GMTICKET_CREATE );
        data << uint32(1);
        SendPacket( &data );
	    }
	else {
        ss << "INSERT INTO `gmtickets` VALUES ('','" << guid << "', '" << ticketText << "', '" << cat[buf[0]] << "')";
        sDatabase.Execute( ss.str( ).c_str( ) );

	data.Initialize( SMSG_QUERY_TIME_RESPONSE );
	data << (uint32)20;
	SendPacket( &data );

        data.Initialize( SMSG_GMTICKET_CREATE );
        data << uint32(2);
        SendPacket( &data );
	DEBUG_LOG("update the ticket\n");
	     }
	}





	delete result;
}

void WorldSession::HandleGMTicketSystemStatusOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    
    data.Initialize( SMSG_GMTICKET_SYSTEMSTATUS );
    data << uint32(1);

    SendPacket( &data );
}

#include "Object.h"

void WorldSession::HandleEnablePvP(WorldPacket& recvPacket)
{
    WorldPacket data;

    if ( (!GetPlayer()->isAlive()) || GetPlayer()->inCombat ) 
    {
        data.Initialize(SMSG_CAST_RESULT);
        data << uint32(0);
        data << uint8(2);
        data << uint8(97);  
        SendPacket(&data);
        return;
    }

    if( GetPlayer()->HasFlag(UNIT_FIELD_FLAGS , 0x08) )
    {
        GetPlayer()->RemoveFlag(UNIT_FIELD_FLAGS , 0x08);
        GetPlayer()->SetPvP(false);
    }
    else
    {
        GetPlayer()->SetFlag(UNIT_FIELD_FLAGS , 0x08);
        GetPlayer()->SetPvP(true);
    }
}

void WorldSession::HandleZoneUpdateOpcode( WorldPacket & recv_data )
{
    uint32 newZone,oldZone;
    WPAssert(GetPlayer());

    recv_data >> newZone;
    Log::getSingleton( ).outDetail("WORLD: Recvd ZONE_UPDATE: %u", newZone);

    if (GetPlayer()->GetZoneId() == newZone)
        return;

    oldZone = GetPlayer( )->GetZoneId();

    
    GetPlayer()->SetZoneId((uint32)newZone); 
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
        
        uint32 bytes1 = GetPlayer( )->GetUInt32Value( UNIT_FIELD_BYTES_1 );
        uint8 bytes[4];

        
        

        bytes[0] = uint8(bytes1 & 0xff);
        bytes[1] = uint8((bytes1>>8) & 0xff);
        bytes[2] = uint8((bytes1>>16) & 0xff);
        bytes[3] = uint8((bytes1>>24) & 0xff);

        
        uint8 animstate;
        recv_data >> animstate;

        
        bytes[0] = animstate;

        uint32 newbytes = (bytes[0]) + (bytes[1]<<8) + (bytes[2]<<16) + (bytes[3]<<24);
        GetPlayer( )->SetUInt32Value(UNIT_FIELD_BYTES_1 , newbytes);
    }
}


void WorldSession::HandleFriendListOpcode( WorldPacket & recv_data )
{
    WorldPacket data, dataI;

    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_FRIEND_LIST"  );

    unsigned char Counter=0, nrignore=0;
        int i=0;
    uint64 guid;
    std::stringstream query,query2,query3,query4;
        Field *fields;
    Player* pObj;
    FriendStr friendstr[255];

    guid=GetPlayer()->GetGUID();

    query << "SELECT COUNT(*) FROM `social` where flags = 'FRIEND' AND guid='" << guid << "'";
    QueryResult *result = sDatabase.Query( query.str().c_str() );

    if(result)
    {
        fields = result->Fetch();
        Counter=fields[0].GetUInt32();

        query2 << "SELECT * FROM `social` where flags = 'FRIEND' AND guid='" << guid << "'";
        result = sDatabase.Query( query2.str().c_str() );
        
        if(result)
        {
            fields = result->Fetch();
            friendstr[i].PlayerGUID = fields[2].GetUInt64();
	    pObj = ObjectAccessor::Instance().FindPlayer( friendstr[i].PlayerGUID );

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
		pObj = ObjectAccessor::Instance().FindPlayer(friendstr[i].PlayerGUID);
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

    

    data.Initialize( SMSG_FRIEND_LIST );
    data << Counter;

    for (int j=0; j<Counter; j++)
    {

        Log::getSingleton( ).outDetail( "WORLD: Adding Friend - Guid:%ld, Status:%d, Area:%d, Level:%d Class:%d",friendstr[j].PlayerGUID, friendstr[j].Status, friendstr[j].Area,friendstr[j].Level,friendstr[j].Class  );

        data << friendstr[j].PlayerGUID << friendstr[j].Status ;
        if (friendstr[j].Status != 0)
            data << friendstr[j].Area << friendstr[j].Level << friendstr[j].Class;
    }

    SendPacket( &data );
    Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_FRIEND_LIST)" );

    

    query3 << "SELECT COUNT(*) FROM `social` where flags = 'IGNORE' AND guid='" << guid << "'";
  result = sDatabase.Query( query3.str().c_str() );
    
    if(!result) return;
    
    fields = result->Fetch();
  nrignore=fields[0].GetUInt32();


    dataI.Initialize( SMSG_IGNORE_LIST );
  dataI << nrignore;

    query4 << "SELECT * FROM `social` where flags = 'IGNORE' AND guid='" << guid << "'";
  result = sDatabase.Query( query4.str().c_str() );
  
    if(!result) return;
    
  do
    {
        
                fields = result->Fetch();
        dataI << fields[2].GetUInt64();

       }while( result->NextRow() );


  SendPacket( &dataI );
    Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_IGNORE_LIST)" );
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
    pfriend = ObjectAccessor::Instance().FindPlayer(friendGuid);

    fquery << "SELECT * FROM social WHERE flags = 'FRIEND' AND friendid = " << friendGuid;

    if(sDatabase.Query( fquery.str().c_str() )) friendResult = FRIEND_ALREADY;
    if (!strcmp(GetPlayer()->GetName(),friendName.c_str())) friendResult = FRIEND_SELF;
    
    
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
        Log::getSingleton( ).outDetail( "WORLD: %s Guid Already a Friend. ", friendName.c_str() );
    }
    else if(friendResult==FRIEND_SELF)
    {
        data << (uint8)friendResult << (uint64)friendGuid;
        Log::getSingleton( ).outDetail( "WORLD: %s Guid can't add himself. ", friendName.c_str() );
    }
    else
    {
        data << (uint8)friendResult << (uint64)friendGuid;
        Log::getSingleton( ).outDetail( "WORLD: %s Guid not found. ", friendName.c_str() );
    }

    
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

    

    
    data.Initialize( SMSG_FRIEND_STATUS );

    data << (uint8)FriendResult << (uint64)FriendGUID;

    
    std::stringstream query;
    uint64 guid;
    guid=GetPlayer()->GetGUID();

    query << "DELETE FROM `social` WHERE flags = 'FRIEND' AND `guid`=" << guid << " AND `friendid`=" << FriendGUID;
    sDatabase.Query( query.str().c_str() );
    

    SendPacket( &data );

    Log::getSingleton( ).outDebug( "WORLD: Sent motd (SMSG_FRIEND_STATUS)" );
}

void WorldSession::HandleAddIgnoreOpcode( WorldPacket & recv_data )
{
    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_ADD_IGNORE"  );

    std::string IgnoreName = "UNKNOWN";
    std::stringstream iquery;
    unsigned char ignoreResult = FRIEND_IGNORE_NOT_FOUND;
    Player *pIgnore=NULL;
    uint64 IgnoreGuid = 0;

    WorldPacket data;

    recv_data >> IgnoreName;

    Log::getSingleton( ).outDetail( "WORLD: %s asked to Ignore: '%s'",
        GetPlayer()->GetName(), IgnoreName.c_str() );
    
    
   
    IgnoreGuid = objmgr.GetPlayerGUIDByName(IgnoreName.c_str());
    pIgnore = ObjectAccessor::Instance().FindPlayer(IgnoreGuid);

    iquery << "SELECT * FROM social WHERE flags = 'IGNORE' AND friendid = " << IgnoreGuid;

    if(sDatabase.Query( iquery.str().c_str() )) ignoreResult = FRIEND_IGNORE_ALREADY;
    if (!strcmp(GetPlayer()->GetName(),IgnoreName.c_str())) ignoreResult = FRIEND_IGNORE_SELF;
    
    
    data.Initialize( SMSG_FRIEND_STATUS );

    if (pIgnore && ignoreResult!=FRIEND_IGNORE_ALREADY && ignoreResult!=FRIEND_IGNORE_SELF)
    {
        ignoreResult = FRIEND_IGNORE_ADDED;
                
        
        std::stringstream query;
        uint64 guid;
        guid=GetPlayer()->GetGUID();

        data << (uint8)ignoreResult << (uint64)IgnoreGuid;

        query << "INSERT INTO `social` VALUES ('" << IgnoreName << "', " << guid << ", " << IgnoreGuid << ", 'IGNORE')" ;
        sDatabase.Query( query.str().c_str() );    
    }
    else if(ignoreResult==FRIEND_IGNORE_ALREADY)
    {
        data << (uint8)ignoreResult << (uint64)IgnoreGuid;
        Log::getSingleton( ).outDetail( "WORLD: %s Guid Already Ignored. ", IgnoreName.c_str() );
    }
    else if(ignoreResult==FRIEND_IGNORE_SELF)
    {
        data << (uint8)ignoreResult << (uint64)IgnoreGuid;
        Log::getSingleton( ).outDetail( "WORLD: %s Guid can't add himself. ", IgnoreName.c_str() );
    }
    else
    {
        data << (uint8)ignoreResult << (uint64)IgnoreGuid;
        Log::getSingleton( ).outDetail( "WORLD: %s Guid not found. ", IgnoreName.c_str() );
    }

    
    SendPacket( &data );

    Log::getSingleton( ).outDebug( "WORLD: Sent (SMSG_FRIEND_STATUS)" );
}

void WorldSession::HandleDelIgnoreOpcode( WorldPacket & recv_data )
{
    uint64 IgnoreGUID;
    WorldPacket data;

    Log::getSingleton( ).outDebug( "WORLD: Recieved CMSG_DEL_IGNORE"  );
    recv_data >> IgnoreGUID;

    unsigned char IgnoreResult = FRIEND_IGNORE_REMOVED;

    
    data.Initialize( SMSG_FRIEND_STATUS );

    data << (uint8)IgnoreResult << (uint64)IgnoreGUID;


    std::stringstream query;
    uint64 guid;
    guid=GetPlayer()->GetGUID();

    query << "DELETE FROM `social` WHERE flags = 'IGNORE' AND `guid`=" << guid << " AND `friendid`=" << IgnoreGUID;
    sDatabase.Query( query.str().c_str() );


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

	AreaTriggerPoint *pArea = objmgr.GetAreaTriggerQuestPoint( id );
	Quest *pQuest;

	if (pArea) pQuest = objmgr.GetQuest( pArea->Quest_ID ); else
		pQuest = NULL;

	scriptCallAreaTrigger( GetPlayer(), pQuest, id );

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
            MapManager::Instance().GetMap(GetPlayer()->GetMapId())->Remove(GetPlayer(), false);

            data.Initialize(SMSG_NEW_WORLD);
            data << at->mapId << at->X << at->Y << at->Z << 0.0f;
            SendPacket( &data );

            GetPlayer()->SetMapId(at->mapId);
            GetPlayer()->Relocate(at->X, at->Y, at->Z, 0.0f);
	    
            MapManager::Instance().GetMap(GetPlayer()->GetMapId())->Add(GetPlayer());
        }

        delete at;
    }
}


void WorldSession::HandleUpdateAccountData(WorldPacket &recv_data)
{

}


void WorldSession::HandleRequestAccountData(WorldPacket& recv_data)
{
    
    
    
    
    
    Log::getSingleton().outDetail("WORLD: Received CMSG_REQUEST_ACCOUNT_DATA");


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


void WorldSession::HandleCompleteCinema( WorldPacket & recv_data )
{
    DEBUG_LOG( "WORLD: Player is watching cinema" );
}

void WorldSession::HandleNextCinematicCamera( WorldPacket & recv_data )
{
    DEBUG_LOG( "WORLD: Which movie to play" );
}


void WorldSession::HandleBattlefieldStatusOpcode( WorldPacket & recv_data )
{
    
    DEBUG_LOG( "WORLD: Battleground status - not yet" );
}




void WorldSession::HandleMoveTimeSkippedOpcode( WorldPacket & recv_data )
{
    
    DEBUG_LOG( "WORLD: Move time lag/synchronization fix - not yet" );
}



#define OPEN_CHEST 11437
#define OPEN_SAFE 11535
#define OPEN_CAGE 11792
#define OPEN_BOOTY_CHEST 5107
#define OPEN_STRONGBOX 8517

void WorldSession::HandleGameObjectUseOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;
	uint32 spellId = OPEN_CHEST;

    recv_data >> guid;

    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_GAMEOBJ_USE Message [guid=%d]", guid);   
    GameObject *obj = ObjectAccessor::Instance().GetGameObject(*_player, guid);

	GetPlayer()->SetLootGUID(guid);

    SpellEntry *spellInfo = sSpellStore.LookupEntry( spellId );

    if(!spellInfo)
    {
        Log::getSingleton( ).outError("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    Spell *spell = new Spell(GetPlayer(), spellInfo, false, 0);
    WPAssert(spell);

    SpellCastTargets targets;
    targets.m_unitTarget = guid;
    spell->prepare(&targets);
}


void WorldSession::HandleReadItem( WorldPacket & recv_data )
{
	uint8 slot, uslot;

	WorldPacket data;

    recv_data >> uslot >> slot;

	Log::getSingleton( ).outDebug( "WORLD: CMSG_READ_ITEM");
	Item *pItem = GetPlayer()->GetItemBySlot( slot );
	

}
