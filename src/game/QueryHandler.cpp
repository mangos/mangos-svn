/* QueryHandler.cpp
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
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"

//////////////////////////////////////////////////////////////
/// This function handles CMSG_NAME_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleNameQueryOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;

    recv_data >> guid;

    uint32 race = 0, gender = 0, cl = 0;
    std::string name = "ERROR_NO_NAME_FOR_GUID";

    Player *pChar = objmgr.GetObject<Player>(guid);
    if (pChar == NULL)
    {
        if (!objmgr.GetPlayerNameByGUID(guid, name))
            Log::getSingleton( ).outError( "No player name found for this guid" );

// TODO: load race, class, sex, etc from db
    }
    else
    {
        race = pChar->getRace();
        gender = pChar->getGender();
        cl = pChar->getClass();
        name = pChar->GetName();
    }

    Log::getSingleton( ).outDebug( "Recieved CMSG_NAME_QUERY for: %s", name.c_str() );

    data.Initialize( SMSG_NAME_QUERY_RESPONSE );

    data << guid;
    data << name;
    data << race << gender << cl;

    SendPacket( &data );
}


//////////////////////////////////////////////////////////////
/// This function handles CMSG_QUERY_TIME:
//////////////////////////////////////////////////////////////
void WorldSession::HandleQueryTimeOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    data.Initialize( SMSG_QUERY_TIME_RESPONSE );

    data << (uint32)time(NULL);
    SendPacket( &data );
}


//////////////////////////////////////////////////////////////
/// This function handles CMSG_CREATURE_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleCreatureQueryOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 entry;
    uint64 guid;
    CreatureInfo *ci;

    recv_data >> entry;
    recv_data >> guid;

    ci = objmgr.GetCreatureName(entry);
    Log::getSingleton( ).outDetail("WORLD: CMSG_CREATURE_QUERY '%s'", ci->Name.c_str());

    data.Initialize( SMSG_CREATURE_QUERY_RESPONSE );
    data << (uint32)entry;
    data << ci->Name.c_str();
    data << uint8(0) << uint8(0) << uint8(0);
    data << ci->SubName.c_str();                  // Subname
    data << ci->unknown1;                         // unknown 1
    data << ci->Type;                             // Creature Type
    data << ci->unknown2;                         // unknown 3
    data << ci->unknown3;                         // unknown 4
    data << ci->unknown4;                         // unknown 5
    data << ci->DisplayID;                        // DisplayID

    SendPacket( &data );
}


//////////////////////////////////////////////////////////////
/// This function handles CMSG_GAMEOBJECT_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleGameObjectQueryOpcode( WorldPacket & recv_data )
{

    WorldPacket data;
    data.Initialize( SMSG_GAMEOBJECT_QUERY_RESPONSE );
    uint32 entryID;
    uint64 guid;

    recv_data >> entryID;
    recv_data >> guid;

    Log::getSingleton( ).outDetail("WORLD: CMSG_GAMEOBJECT_QUERY '%u'", guid);

    GameObject* gameObj = objmgr.GetObject<GameObject>(guid);
    if(!gameObj)
        return;

    std::string name = "";

    data << gameObj->GetUInt32Value(OBJECT_FIELD_ENTRY);
    data << gameObj->GetUInt32Value(GAMEOBJECT_TYPE_ID);
    data << gameObj->GetUInt32Value(GAMEOBJECT_DISPLAYID);
    data << name.c_str();
    data << uint8(0) << uint8(0) << uint8(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint16(0);
    data << uint8(0);

/*
00 00 07 87 00 00 00 08 00 00 00 C0 54 F9 EC 01
04 00 00 00 0A 00 00 00 12 00 00 00 00 00 00 08
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 4F 52 44 45 52
*/

/* Mailbox
22 33 02 00 // ENTRY
13 00 00 00 // Unknown
8e 08 00 00 // Display_id
4d 61 69 6c | 62 6f 78 00 // Mailbox (Null terminated)
00          // name2
00          // name3
00          // name4
00 00 00 00 // 1
00 00 00 00 // 2
00 00 00 00 // 3
00 00 00 00 // 4
00 00 00 00 // 5
00 00 00 00 // 6
00 00 00 00 // 7
00 00 00 00 // 8
00 00 00 00 // 9
00 00 00 00 // 10
00 00 00    // 11
*/

//WPAssert( data.size() == 64 );
    SendPacket( &data );
}


//////////////////////////////////////////////////////////////
/// This function handles MSG_CORPSE_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleCorpseQueryOpcode(WorldPacket &recv_data)
{
    Log::getSingleton().outDetail("WORLD: Received MSG_CORPSE_QUERY");

    Corpse *pCorpse;
    WorldPacket data;

    pCorpse = objmgr.GetCorpseByOwner(GetPlayer());

    if(pCorpse)
    {
        data.Initialize(MSG_CORPSE_QUERY);
        data << uint8(0x01);
        data << uint32(0x00000001);
        data << pCorpse->GetPositionX();
        data << pCorpse->GetPositionY();
        data << pCorpse->GetPositionZ();
        data << uint32(0x00000001);
        SendPacket(&data);
    }
}
