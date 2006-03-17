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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"
#include "UpdateData.h"
#include "Chat.h"
#include "MapManager.h"




void WorldSession::HandleDuelAcceptedOpcode(WorldPacket& recvPacket)
{

    sLog.outString( "HandleDuelAcceptedOpcode.\n" );

    
    
    uint64 guid;

    recvPacket >> guid;

    Player *pl;
    Player *plTarget;
    WorldPacket data;


    WorldPacket Msgdata;

    WorldPacket packet,packetR;
    UpdateData updata;

    pl     = GetPlayer();                         
    plTarget = objmgr.GetPlayer(pl->m_duelGUID);
    

    if(pl->m_duelSenderGUID == pl->GetGUID())
    {
   
        data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
        data << (uint64)guid;
        pl->GetSession()->SendPacket(&data);
        plTarget->GetSession()->SendPacket(&data);

    }
    else
    {
        data.Initialize(SMSG_PET_BROKEN | CMSG_LEARN_SPELL);
        data << (uint64)0xbb8;
        pl->GetSession()->SendPacket(&data);
        plTarget->GetSession()->SendPacket(&data);
        

        pl->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, 469 );
        plTarget->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, 67 );


    }

    pl->m_isInDuel = true;
    plTarget->m_isInDuel = true;

}


void WorldSession::HandleDuelCancelledOpcode(WorldPacket& recvPacket)
{

    sLog.outString( "HandleDuelCancelledOpcode.\n" );

    uint64 guid;
    Player *pl;
    Player *plTarget;
    WorldPacket data;

    recvPacket >> guid;

    pl       = GetPlayer();                       
    plTarget = objmgr.GetPlayer(pl->m_duelGUID);

    data.Initialize(SMSG_GAMEOBJECT_DESPAWN_ANIM);
    data << (uint64)guid;
    pl->GetSession()->SendPacket(&data);
    plTarget->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_DESTROY_OBJECT);
    data << (uint64)guid;
    pl->GetSession()->SendPacket(&data);
    plTarget->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_DUEL_COMPLETE);
    data << (uint8)0;                             
    pl->GetSession()->SendPacket(&data);
    plTarget->GetSession()->SendPacket(&data);

    pl->m_isInDuel = false;
    plTarget->m_isInDuel = false;

    GameObject* obj = NULL;
    if( pl )
       obj = ObjectAccessor::Instance().GetGameObject(*pl, guid);

    if(obj)
    {
	
	MapManager::Instance().GetMap(obj->GetMapId())->Remove(obj,true);
    }
}
