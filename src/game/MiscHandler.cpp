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
#include "Object.h"


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
    sLog.outDebug( "WORLD: Recvd CMSG_REPOP_REQUEST Message" );

    GetPlayer()->BuildPlayerRepop();
    GetPlayer()->RepopAtGraveyard();
}

void WorldSession::HandleAutostoreLootItemOpcode( WorldPacket & recv_data )
{
    uint8 lootSlot;
	Loot * loot;
	recv_data >> lootSlot;
	uint64 guid=_player->GetLootGUID();
	

	if(IS_GAMEOBJECT_GUID(guid))
	{
		GameObject *go=ObjectAccessor::Instance().GetGameObject(*_player, guid);
		if(!go)return;
		else loot=&go->loot ;
	}else
	{
		Creature* pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
		if (!pCreature)	return;
		else loot=&pCreature->loot ;
	}

	WorldPacket data;
	
		

	if (loot->items.at(lootSlot).isLooted)
	{  
		data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
		data << uint8(EQUIP_ERR_ALREADY_LOOTED);                        
		data << uint64(0);
		data << uint64(0);
		data << uint8(0);
		SendPacket( &data );
		return;
	}



	if (GetPlayer()->AddNewItem(0,NULL_SLOT,loot->items.at(lootSlot).item.itemid,1,false,false))
	{
		loot->items.at(lootSlot).isLooted=true;
		data.Initialize( SMSG_LOOT_REMOVED );
		data << uint8(lootSlot);
		SendPacket( &data );
			
			
		data.Initialize( SMSG_ITEM_PUSH_RESULT );
		data << _player->GetGUID();
		data << uint64(0x00000000);
		data << uint8(0x01);
		data << uint8(0x00);
		data << uint8(0x00);
		data << uint8(0x00);
		data << uint8(0xFF);
		data << uint32(loot->items.at(lootSlot).item.itemid);
		data << uint64(0);

		/*data << uint8(0x00);
		data << uint8(0x00);
		data << uint8(0x00);
		data << uint32(0x00000000);
		data << uint8(0x00);*/
		SendPacket( &data );
	}else
	{
		data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
		data << uint8(EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE);                        
		data << uint64(0);
		data << uint64(0);
		data << uint8(0);
		SendPacket( &data );
	}
	
}

void WorldSession::HandleLootMoneyOpcode( WorldPacket & recv_data )
{
	Loot * loot;
 	uint64 guid=_player->GetLootGUID();

	if(IS_GAMEOBJECT_GUID(guid))
	{
		GameObject *go=ObjectAccessor::Instance().GetGameObject(*_player, guid);
		if(!go)return;
		else loot=&go->loot ;
	}else
	{
		Creature* pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
		if (!pCreature)	return;
		else loot=&pCreature->loot ;
	}

    uint32 newcoinage = _player->GetUInt32Value(PLAYER_FIELD_COINAGE)+ loot->gold ;
	loot->gold =0;
    GetPlayer()->SetUInt32Value(PLAYER_FIELD_COINAGE , newcoinage);
}

extern int num_item_prototypes;


void WorldSession::HandleLootOpcode( WorldPacket & recv_data )
{
    uint64 guid;
	recv_data >> guid;
	GetPlayer()->SendLoot(guid,1);

}


void WorldSession::HandleLootReleaseOpcode( WorldPacket & recv_data )
{	
    uint64 guid;
    recv_data >> guid;
   
	GetPlayer()->SetLootGUID(0);

    WorldPacket data;
    data.Initialize( SMSG_LOOT_RELEASE_RESPONSE );
    data << guid << uint8( 1 );
    SendPacket( &data );

	if(IS_GAMEOBJECT_GUID(guid))
	{
		//FIXME: remove go after it's looted


	}else
	{
		Creature* pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
		if (!pCreature)	return;
		Loot * 	loot=&pCreature->loot ;
		if(!loot->gold)
		{
			bool Looted=true;
			for(std::vector<__LootItem>::iterator i=loot->items.begin();i!=loot->items.end();i++)
				if(!i->isLooted){Looted=false;break;}

			if(Looted)
			{
				pCreature->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);//this is probably wrong
				if(pCreature->GetCreatureInfo()->SkinLootId)
				pCreature->SetFlag (UNIT_FIELD_FLAGS, 0x4000000);// set skinnable
			}
		}
	}



}


void WorldSession::HandleWhoOpcode( WorldPacket & recv_data )
{
    uint32 clientcount = 0;
    int datalen = 8;
    int countcheck = 0;
    WorldPacket data;

    sLog.outDebug( "WORLD: Recvd CMSG_WHO Message" );

    ObjectAccessor::PlayersMapType &m(ObjectAccessor::Instance().GetPlayers());
    for(ObjectAccessor::PlayersMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if ( itr->second->GetName() )
        {
            clientcount++;

            datalen = datalen + strlen(itr->second->GetName()) + 1 + 17;
        }
    }

    data.Initialize( SMSG_WHO );
	data << uint32( clientcount );
	data << uint32( clientcount );

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
        }
    }

    WPAssert(data.size() == datalen);
	SendPacket(&data);
}


void WorldSession::HandleLogoutRequestOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    sLog.outDebug( "WORLD: Recvd CMSG_LOGOUT_REQUEST Message" );

    if( !(GetPlayer()->inCombat) )
	{ 
		data.Initialize( SMSG_LOGOUT_RESPONSE );
		data << uint32(0);
		data << uint8(0);
		SendPacket( &data );
     
		LogoutRequest(time(NULL));

		//! Set the flag so player sits
		GetPlayer()->SetFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT);

		//! DISABLE_ROTATE = 0x40000;
		GetPlayer()->SetFlag(UNIT_FIELD_FLAGS, 0x40000);

		// Can't move
		data.Initialize( SMSG_FORCE_MOVE_ROOT );
		data << (uint8)0xFF << GetPlayer()->GetGUID();
		SendPacket( &data );
    }
    else 
	{
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

    sLog.outDebug( "WORLD: Recvd CMSG_PLAYER_LOGOUT Message" );
    LogoutRequest(0);
    LogoutPlayer(1);
}


void WorldSession::HandleLogoutCancelOpcode( WorldPacket & recv_data )
{
    WorldPacket data;

    sLog.outDebug( "WORLD: Recvd CMSG_LOGOUT_CANCEL Message" );

    LogoutRequest(0);

    data.Initialize( SMSG_LOGOUT_CANCEL_ACK );
    SendPacket( &data );

    //!we can move again
    data.Initialize( SMSG_FORCE_MOVE_UNROOT );
    data << (uint8)0xFF << GetPlayer()->GetGUID();
    SendPacket( &data );

    //! Stand Up
	//! Removes the flag so player stands
	GetPlayer()->RemoveFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT);

	//! DISABLE_ROTATE
	GetPlayer()->RemoveFlag(UNIT_FIELD_FLAGS, 0x40000);

    sLog.outDebug( "WORLD: sent SMSG_LOGOUT_CANCEL_ACK Message" );
}


void WorldSession::HandleGMTicketGetTicketOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    data.Initialize( SMSG_QUERY_TIME_RESPONSE );
//    data << (uint32)20;
    data << (uint32)getMSTime();
    SendPacket( &data );

    uint64 guid;
    Field *fields;
    guid = GetPlayer()->GetGUID();

    QueryResult *result = sDatabase.PQuery("SELECT COUNT(*) FROM gmtickets where guid = '%d';", guid);

        if (result)
        {
            int cnt;
            fields = result->Fetch();
            cnt = fields[0].GetUInt32();

            if ( cnt > 0 )
            {
                data.Initialize( SMSG_GMTICKET_GETTICKET );

		QueryResult *result = sDatabase.PQuery("SELECT * FROM gmtickets WHERE guid = '%d';", guid);
                fields = result->Fetch();

                char tickettext[255];
                strcpy( tickettext,fields[2].GetString() );
                data << uint32(6); 
                data.append((uint8 *)tickettext,strlen(tickettext)+1);
                SendPacket( &data );
		delete result;
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
        ticketText = p1;
        sDatabase.PExecute("UPDATE `gmtickets` set ticket_text = '%s' WHERE guid = '%d';", ticketText.c_str(), guid);

}

void WorldSession::HandleGMTicketDeleteOpcode( WorldPacket & recv_data )
{
        WorldPacket data;
        uint64 guid = GetPlayer()->GetGUID();

	sDatabase.PExecute("DELETE FROM `gmtickets` where guid = '%d' LIMIT 1",guid);

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
	Field *fields;
        char * p, p1[512];
        uint8 buf[516];
        int   cat[] = { 0,5,1,2,0,6,4,7,0,8,3 };
        memcpy( buf, recv_data.contents(), sizeof buf < recv_data.size() ? sizeof buf : recv_data.size() );
        buf[272] = 0;
        p = (char *)buf + 17;
        my_esc( p1, (const char *)buf + 17 );
        ticketText = p1;

	QueryResult *result = sDatabase.PQuery("SELECT COUNT(*) FROM gmtickets where guid = '%d';",guid);

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

        sDatabase.PExecute("INSERT INTO `gmtickets` (guid,ticket_text, ticket_category) VALUES ('%ul', '%s', '%d');", (unsigned long)guid, ticketText.c_str(), cat[buf[0]]);

	data.Initialize( SMSG_QUERY_TIME_RESPONSE );
	//data << (uint32)20;
	data << (uint32)getMSTime();
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
    uint32 newZone;
    WPAssert(GetPlayer());

    // if player is resting stop resting
    if(GetPlayer()->HasFlag(PLAYER_FLAGS, 0x20))
	GetPlayer()->RemoveFlag(PLAYER_FLAGS, 0x20);

    recv_data >> newZone;
    sLog.outDetail("WORLD: Recvd ZONE_UPDATE: %u", newZone);

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
	sLog.outDebug( "WORLD: Received CMSG_STAND_STATE_CHANGE"  );
    if( GetPlayer() != 0 )
    {
        uint8 animstate;
        recv_data >> animstate;
        
        uint32 bytes1 = GetPlayer( )->GetUInt32Value( UNIT_FIELD_BYTES_1 );
		bytes1 &=0xFFFFFF00;
		bytes1 |=animstate;
		GetPlayer( )->SetUInt32Value(UNIT_FIELD_BYTES_1 , bytes1);
    }
}


void WorldSession::HandleFriendListOpcode( WorldPacket & recv_data )
{
    WorldPacket data, dataI;

    sLog.outDebug( "WORLD: Received CMSG_FRIEND_LIST"  );

    unsigned char Counter=0, nrignore=0;
        int i=0;
    uint64 guid;
        Field *fields;
    Player* pObj;
    FriendStr friendstr[255];

    guid=GetPlayer()->GetGUID();

    QueryResult *result = sDatabase.PQuery("SELECT COUNT(*) FROM `social` WHERE flags = 'FRIEND' AND guid = '%d';",guid);    

    if(result)
    {
        fields = result->Fetch();
        Counter=fields[0].GetUInt32();
				delete result;

				result = sDatabase.PQuery("SELECT * FROM `social` WHERE flags = 'FRIEND' AND guid = '%d';",guid);
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
            delete result;
        }
    }
		    
    

    data.Initialize( SMSG_FRIEND_LIST );
    data << Counter;

    for (int j=0; j<Counter; j++)
    {

        sLog.outDetail( "WORLD: Adding Friend - Guid:%ld, Status:%d, Area:%d, Level:%d Class:%d",friendstr[j].PlayerGUID, friendstr[j].Status, friendstr[j].Area,friendstr[j].Level,friendstr[j].Class  );

        data << friendstr[j].PlayerGUID << friendstr[j].Status ;
        if (friendstr[j].Status != 0)
            data << friendstr[j].Area << friendstr[j].Level << friendstr[j].Class;
    }

    SendPacket( &data );
    sLog.outDebug( "WORLD: Sent (SMSG_FRIEND_LIST)" );

    
    result = sDatabase.PQuery("SELECT COUNT(*) FROM `social` WHERE flags = 'IGNORE' AND guid = '%d';", guid);    

    if(!result) return;
    
    fields = result->Fetch();
    nrignore=fields[0].GetUInt32();
    delete result;
    
    dataI.Initialize( SMSG_IGNORE_LIST );
    dataI << nrignore;
    

    result = sDatabase.PQuery("SELECT * FROM `social` WHERE flags = 'IGNORE' AND guid = '%d';", guid);
  
    if(!result) return;
    
  do
    {
        
	fields = result->Fetch();
        dataI << fields[2].GetUInt64();

       }while( result->NextRow() );
	delete result;

	SendPacket( &dataI );
    sLog.outDebug( "WORLD: Sent (SMSG_IGNORE_LIST)" );
	
}


void WorldSession::HandleAddFriendOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_ADD_FRIEND"  );

    std::string friendName = "UNKNOWN";
    unsigned char friendResult = FRIEND_NOT_FOUND;
    Player *pfriend=NULL;
    uint32 friendGuid = 0;
    uint32 friendArea = 0, friendLevel = 0, friendClass = 0;
    WorldPacket data;

    recv_data >> friendName;

    sLog.outDetail( "WORLD: %s asked to add friend : '%s'",
        GetPlayer()->GetName(), friendName.c_str() );
    
    
   
    friendGuid = objmgr.GetPlayerGUIDByName(friendName.c_str());
    pfriend = ObjectAccessor::Instance().FindPlayer(friendGuid);

    QueryResult *result = sDatabase.PQuery("SELECT * FROM social WHERE flags = 'FRIEND' AND friendid = '%d';", friendGuid);

    if( result )
	friendResult = FRIEND_ALREADY;

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

			data << (uint8)friendResult << (uint64)friendGuid << (uint8)0;
            data << (uint32)friendArea << (uint32)friendLevel << (uint32)friendClass;


	    delete result;            
            
            uint64 guid;
            guid=GetPlayer()->GetGUID();
		   
			result = sDatabase.PQuery("INSERT INTO `social` VALUES ('%s', '%d', '%d', 'FRIEND');", friendName.c_str(), (uint32)guid, (uint32)friendGuid);
		
				
	    delete result;
        }
        else
            friendResult = FRIEND_ADDED_OFFLINE;

        sLog.outDetail( "WORLD: %s Guid found '%ld' area:%d Level:%d Class:%d. ",
            friendName.c_str(), friendGuid, friendArea, friendLevel, friendClass);

    }
    else if(friendResult==FRIEND_ALREADY)
    {
        data << (uint8)friendResult << (uint64)friendGuid;
        sLog.outDetail( "WORLD: %s Guid Already a Friend. ", friendName.c_str() );
    }
    else if(friendResult==FRIEND_SELF)
    {
        data << (uint8)friendResult << (uint64)friendGuid;
        sLog.outDetail( "WORLD: %s Guid can't add himself. ", friendName.c_str() );
    }
    else
    {
        data << (uint8)friendResult << (uint64)friendGuid;
        sLog.outDetail( "WORLD: %s Guid not found. ", friendName.c_str() );
    }

    
    SendPacket( &data );

    sLog.outDebug( "WORLD: Sent (SMSG_FRIEND_STATUS)" );
}

void WorldSession::HandleDelFriendOpcode( WorldPacket & recv_data )
{
    uint64 FriendGUID;
    WorldPacket data;

    sLog.outDebug( "WORLD: Received CMSG_DEL_FRIEND"  );
    recv_data >> FriendGUID;

    uint8 FriendResult = FRIEND_REMOVED;

    data.Initialize( SMSG_FRIEND_STATUS );

    data << (uint8)FriendResult << (uint64)FriendGUID;

    uint64 guid;
    guid=GetPlayer()->GetGUID();

    sDatabase.PExecute("DELETE FROM `social` WHERE flags = 'FRIEND' AND `guid` = '%d' AND `friendid` = '%d'",(uint32)guid,(uint32)FriendGUID);

    SendPacket( &data );

    sLog.outDebug( "WORLD: Sent motd (SMSG_FRIEND_STATUS)" );
}

void WorldSession::HandleAddIgnoreOpcode( WorldPacket & recv_data )
{
    sLog.outDebug( "WORLD: Received CMSG_ADD_IGNORE"  );

    std::string IgnoreName = "UNKNOWN";
    unsigned char ignoreResult = FRIEND_IGNORE_NOT_FOUND;
    Player *pIgnore=NULL;
    uint64 IgnoreGuid = 0;

    WorldPacket data;

    recv_data >> IgnoreName;

    sLog.outDetail( "WORLD: %s asked to Ignore: '%s'",
        GetPlayer()->GetName(), IgnoreName.c_str() );
    
    
   
    IgnoreGuid = objmgr.GetPlayerGUIDByName(IgnoreName.c_str());
    pIgnore = ObjectAccessor::Instance().FindPlayer(IgnoreGuid);


    QueryResult *result = sDatabase.PQuery("SELECT * FROM social WHERE flags = 'IGNORE' AND friendid = '%d';", (uint32)IgnoreGuid);

    if( result )
		ignoreResult = FRIEND_IGNORE_ALREADY;
	delete result;

    if (!strcmp(GetPlayer()->GetName(),IgnoreName.c_str())) ignoreResult = FRIEND_IGNORE_SELF;
    
    
    data.Initialize( SMSG_FRIEND_STATUS );
    

    if (pIgnore && ignoreResult!=FRIEND_IGNORE_ALREADY && ignoreResult!=FRIEND_IGNORE_SELF)
    {
        ignoreResult = FRIEND_IGNORE_ADDED;
                
        uint64 guid;
        guid=GetPlayer()->GetGUID();

        data << (uint8)ignoreResult << (uint64)IgnoreGuid;

		QueryResult *result = sDatabase.PQuery("INSERT INTO `social` VALUES ('%s', '%d', '%d', 'IGNORE');", IgnoreName.c_str(), (uint32)guid, (uint32)IgnoreGuid);

    }
    else if(ignoreResult==FRIEND_IGNORE_ALREADY)
    {
        data << (uint8)ignoreResult << (uint64)IgnoreGuid;
        sLog.outDetail( "WORLD: %s Guid Already Ignored. ", IgnoreName.c_str() );
    }
    else if(ignoreResult==FRIEND_IGNORE_SELF)
    {
        data << (uint8)ignoreResult << (uint64)IgnoreGuid;
        sLog.outDetail( "WORLD: %s Guid can't add himself. ", IgnoreName.c_str() );
    }
    else
    {
        data << (uint8)ignoreResult << (uint64)IgnoreGuid;
        sLog.outDetail( "WORLD: %s Guid not found. ", IgnoreName.c_str() );
    }

    
    SendPacket( &data );
    delete result;
    sLog.outDebug( "WORLD: Sent (SMSG_FRIEND_STATUS)" );
}

void WorldSession::HandleDelIgnoreOpcode( WorldPacket & recv_data )
{
    uint64 IgnoreGUID;
    WorldPacket data;

    sLog.outDebug( "WORLD: Received CMSG_DEL_IGNORE"  );
    recv_data >> IgnoreGUID;

    unsigned char IgnoreResult = FRIEND_IGNORE_REMOVED;

    
    data.Initialize( SMSG_FRIEND_STATUS );

    data << (uint8)IgnoreResult << (uint64)IgnoreGUID;


    uint64 guid;
    guid=GetPlayer()->GetGUID();

    sDatabase.PExecute("DELETE FROM `social` WHERE flags = 'IGNORE' AND `guid` = '%d' AND `friendid` = '%d'",(uint32)guid,(uint32)IgnoreGUID);

    SendPacket( &data );

    sLog.outDebug( "WORLD: Sent motd (SMSG_FRIEND_STATUS)" );

}

void WorldSession::HandleBugOpcode( WorldPacket & recv_data )
{
    uint32 suggestion, contentlen;
    std::string content;
    uint32 typelen;
    std::string type;

    recv_data >> suggestion >> contentlen >> content >> typelen >> type;

    if( suggestion == 0 )
        sLog.outDebug( "WORLD: Received CMSG_BUG [Bug Report]" );
    else
        sLog.outDebug( "WORLD: Received CMSG_BUG [Suggestion]" );

    sLog.outDebug( type.c_str( ) );
    sLog.outDebug( content.c_str( ) );

    sDatabase.PExecute ("INSERT INTO bugreport (rep_type, rep_content) VALUES('%s', '%s');", type.c_str( ), content.c_str( ));

}


void WorldSession::HandleCorpseReclaimOpcode(WorldPacket &recv_data)
{
    sLog.outDetail("WORLD: Received CMSG_RECLAIM_CORPSE");

    uint64 guid;
    recv_data >> guid;

    // resurrect
    GetPlayer()->ResurrectPlayer();

    // spawnbones
    GetPlayer()->SpawnCorpseBones();

    // set health
    GetPlayer()->SetUInt32Value(UNIT_FIELD_HEALTH,(uint32)(GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*0.50) );

    // update world right away
    MapManager::Instance().GetMap(GetPlayer()->GetMapId())->Add(GetPlayer());
}


void WorldSession::HandleResurrectResponseOpcode(WorldPacket & recv_data)
{
    sLog.outDetail("WORLD: Received CMSG_RESURRECT_RESPONSE");

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
	sLog.outDebug("WORLD: Received CMSG_AREATRIGGER");
	
    uint32 Trigger_ID;
    WorldPacket data;
    
    recv_data >> Trigger_ID;
    sLog.outDebug("Trigger ID:%d",Trigger_ID);
    AreaTrigger * at = objmgr.GetAreaTrigger(Trigger_ID);

	AreaTriggerPoint *pArea = objmgr.GetAreaTriggerQuestPoint( Trigger_ID );
	Quest *pQuest;

	if (pArea) pQuest = objmgr.GetQuest( pArea->Quest_ID ); else
		pQuest = NULL;

	Script->scriptAreaTrigger( GetPlayer(), pQuest, Trigger_ID );

    if(at)
    {
		if(at->mapId == GetPlayer()->GetMapId() )
		{
            WorldPacket movedata;
            GetPlayer( )->BuildTeleportAckMsg(&movedata, at->X,
                at->Y, at->Z, GetPlayer()->GetOrientation() );
            GetPlayer( )->SendMessageToSet(&movedata,true);
		}else{
		    GetPlayer()->smsg_NewWorld(at->mapId,at->X,at->Y,at->Z,GetPlayer()->GetOrientation());
		}
        delete at;
    }


	//set resting flag we are in the inn
	Field *fields;
	QueryResult *result = sDatabase.PQuery("SELECT COUNT(*) FROM tavern WHERE triggerid = '%d';", Trigger_ID);
        if(result)
        {
		int cnt;
		fields = result->Fetch();
		cnt = fields[0].GetUInt32();
	
			// player flag 0x20 - resting
			if ( cnt > 0 )
			{
	                if(!GetPlayer()->HasFlag(PLAYER_FLAGS, 0x20))
                        GetPlayer()->SetFlag(PLAYER_FLAGS, 0x20);
			}
        }
	delete result;

}


void WorldSession::HandleUpdateAccountData(WorldPacket &recv_data)
{
}


void WorldSession::HandleRequestAccountData(WorldPacket& recv_data)
{
    sLog.outDetail("WORLD: Received CMSG_REQUEST_ACCOUNT_DATA");
}


void WorldSession::HandleSetActionButtonOpcode(WorldPacket& recv_data)
{
    sLog.outString( "WORLD: Received CMSG_SET_ACTION_BUTTON" );
    uint8 button, misc, type;
    uint16 action;
    recv_data >> button >> action >> misc >> type;
    sLog.outString( "BUTTON: %u ACTION: %u TYPE: %u MISC: %u", button, action, type, misc );
    if(action==0)
    {
        sLog.outString( "MISC: Remove action from button %u", button );
        
        GetPlayer()->removeAction(button);
    }
    else
    {
        if(type==64)
        {
            sLog.outString( "MISC: Added Macro %u into button %u", action, button );
            GetPlayer()->addAction(button,action,misc,type);
        }
        else if(type==0)
        {
            sLog.outString( "MISC: Added Action %u into button %u", action, button );
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

void WorldSession::HandleMooveUnRootAck(WorldPacket& recv_data) {

    sLog.outDebug( "WORLD: CMSG_FORCE_MOVE_UNROOT_ACK" );
    WorldPacket data;
    uint64 guid;
    uint64 uknown1;
    float PositionX;
    float PositionY;
    float PositionZ;
    float Orientation;

    recv_data >> guid;
    recv_data >> uknown1;
    recv_data >> PositionX;
    recv_data >> PositionY;
    recv_data >> PositionZ;
    recv_data >> Orientation;

    DEBUG_LOG("Guid %d",guid);
    DEBUG_LOG("uknown1 %d",uknown1);
    DEBUG_LOG("X %f",PositionX);
    DEBUG_LOG("Y %f",PositionY);
    DEBUG_LOG("Z %f",PositionZ);
    DEBUG_LOG("O %f",Orientation);
}

void WorldSession::HandleLookingForGroup(WorldPacket& recv_data) {
	// TODO send groups need data
}

void WorldSession::HandleMooveRootAck(WorldPacket& recv_data) {

    sLog.outDebug( "WORLD: CMSG_FORCE_MOVE_ROOT_ACK" );
    WorldPacket data;
    uint64 guid;
    uint64 uknown1;
    float PositionX;
    float PositionY;
    float PositionZ;
    float Orientation;

    recv_data >> guid;
    recv_data >> uknown1;
    recv_data >> PositionX;
    recv_data >> PositionY;
    recv_data >> PositionZ;
    recv_data >> Orientation;

    DEBUG_LOG("Guid %d",guid);
    DEBUG_LOG("uknown1 %d",uknown1);
    DEBUG_LOG("X %f",PositionX);
    DEBUG_LOG("Y %f",PositionY);
    DEBUG_LOG("Z %f",PositionZ);
    DEBUG_LOG("O %f",Orientation);
}

void WorldSession::HandleMoveTeleportAck(WorldPacket& recv_data) {

    WorldPacket data;
    uint64 guid;
    uint32 value1;

    recv_data >> guid;
    recv_data >> value1; // ms time ?
    DEBUG_LOG("Guid %d",guid);
    DEBUG_LOG("Value 1 %d",value1);
}

void WorldSession::HandleForceRunSpeedChangeAck(WorldPacket& recv_data) {
        // set run speed ? received data is more
}

void WorldSession::HandleForceSwimSpeedChangeAck(WorldPacket& recv_data) {
        // set swim speed ? received data is more
}

void WorldSession::HandleSetActionBar(WorldPacket& recv_data)
{
	uint8 ActionBar;
	uint32 temp;
		
	recv_data >> ActionBar;

	temp = ((GetPlayer()->GetUInt32Value( PLAYER_FIELD_BYTES )) & 0xFFF0FFFF) + (ActionBar << 16);
	GetPlayer()->SetUInt32Value( PLAYER_FIELD_BYTES, temp);
}
void WorldSession::HandleMoveWaterWalkAck(WorldPacket& recv_data)
{

// TODO
// we receive guid,x,y,z

}

void WorldSession::HandleChangePlayerNameOpcode(WorldPacket& recv_data) 
{
	// TODO
	// need to be written
}
