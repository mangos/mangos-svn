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

#ifdef ENABLE_GRID_SYSTEM
#include "ObjectAccessor.h"
#endif

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

#ifndef ENABLE_GRID_SYSTEM
    Player *pChar = objmgr.GetObject<Player>(guid);
#else
    Player *pChar = ObjectAccessor::Instance().FindPlayer(guid);
#endif
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
{// UQ1: !!! FIXME !!! This seems to do absolutely nothing!!! We have this really wrong!
    WorldPacket data;
    uint32 entry;
    uint64 guid;
    CreatureInfo *ci;

    recv_data >> entry;
    recv_data >> guid;

    ci = objmgr.GetCreatureName(entry);
    Log::getSingleton( ).outDetail("WORLD: CMSG_CREATURE_QUERY '%s' - entry: %u guid: %u", ci->Name.c_str());

    data.Initialize( SMSG_CREATURE_QUERY_RESPONSE );
    data << (uint32)entry;
    
    //if (stricmp(ci->Name.c_str(), ""))
    data << ci->Name.c_str();

    data << uint8(0) << uint8(0) << uint8(0);
    
    //if (stricmp(ci->SubName.c_str(), ""))
    data << ci->SubName.c_str();                  // Subname

    data << ci->unknown1;                         // unknown 1
    /*
    if ((ci->Type & 2) > 0)
    {
        data << uint32(7);
    }
    else
    {
        data << uint32(0);
    }
    */
    data << ci->Type;                             // Creature Type
    data << ci->unknown2;                         // unknown 3
    data << ci->unknown3;                         // unknown 4
    data << ci->unknown4;                         // unknown 5
    data << ci->DisplayID;                        // DisplayID

    //UQ1: WowwoW Style...
/*
    data << ci->Name.c_str();
    data << uint8(0);
    if (stricmp(ci->SubName.c_str(), ""))
        data << ci->SubName.c_str();                  // Subname

    data << uint8(0);
    data << uint8(0);
    data << uint8(0);

    //if (mobile1.Guild != null)
    //    data << uint32(0);

    data << uint8(0);

    data << uint8(0); //flags

    if ((ci->Type & 2) > 0)
    {
        data << uint8(7);
    }
    else
    {
        data << uint8(0);
    }
    data << uint8(ci->Type);

    data << ci->unknown4;                         // unknown 5
    data << uint8(0);
    data << uint8(0);

    data << ci->DisplayID;                        // DisplayID*/

    SendPacket( &data );
}


//////////////////////////////////////////////////////////////
/// This function handles CMSG_GAMEOBJECT_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleGameObjectQueryOpcode( WorldPacket & recv_data )
{

    WorldPacket data;
    uint32 entryID;
    uint64 guid;

    recv_data >> entryID;
    recv_data >> guid;    

    Log::getSingleton( ).outDetail("WORLD: CMSG_GAMEOBJECT_QUERY '%u'", guid);
    const GameObjectInfo *info = objmgr.GetGameObjectInfo(entryID);
    if( info == NULL )
    {
    Log::getSingleton( ).outDebug( "Missing game object info for entry %d", entryID);    
    return; // no luck..
    }

    data << (uint32)(entryID);
    data << (uint32)info->type;
    data << (uint32)info->displayId;
    data << info->name.c_str();
    data << uint8(0) << uint8(0) << uint8(0);
    data << uint32(info->sound0);
    data << uint32(info->sound1);
    data << uint32(info->sound2);
    data << uint32(info->sound3);
    data << uint32(info->sound4);
    data << uint32(info->sound5);
    data << uint32(info->sound6);
    data << uint32(info->sound7);
    data << uint32(info->sound8);
    data << uint32(info->sound9);
    data << uint16(0);
    data << uint8(0);
    SendPacket( &data );
}


//////////////////////////////////////////////////////////////
/// This function handles MSG_CORPSE_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleCorpseQueryOpcode(WorldPacket &recv_data)
{
    Log::getSingleton().outDetail("WORLD: Received MSG_CORPSE_QUERY");

    Corpse *pCorpse = NULL;
    WorldPacket data;
#ifndef ENABLE_GRID_SYSTEM
    pCorpse = objmgr.GetCorpseByOwner(GetPlayer());
#else
    pCorpse = ObjectAccessor::Instance().GetCorpse(*GetPlayer(), GetPlayer()->GetGUID());
#endif
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
