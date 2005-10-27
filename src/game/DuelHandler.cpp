/* DuelHandler.cpp
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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"
#include "UpdateData.h"

void WorldSession::HandleDuelAcceptedOpcode(WorldPacket& recvPacket)
{
    sLog.outString( "HandleDuelAcceptedOpcode.\n" );
    //if you want to  get this handle , learn spell 7266 first, 
	//I don't how to do ,Please FIX ME
	uint64 guid;
    
	recvPacket >> guid;

	Player *pl;
	WorldPacket data;

	WorldPacket packet,packetR;
    UpdateData updata;

	pl     = GetPlayer(); // get duel sender
	
	pl->DuelVsPlayer->BuildCreateUpdateBlockForPlayer( &updata, pl );
    updata.BuildPacket(&packet);
    pl->GetSession()->SendPacket( &packet );
	data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
    data << (uint64)guid; 
	pl->GetSession()->SendPacket(&data);  

	updata.Clear();

    pl->BuildCreateUpdateBlockForPlayer( &updata, pl->DuelVsPlayer );
    updata.BuildPacket(&packet);
    pl->DuelVsPlayer->GetSession()->SendPacket( &packet );

	data.Initialize(SMSG_GAMEOBJECT_SPAWN_ANIM);
    data << (uint64)guid; 
	pl->DuelVsPlayer->GetSession()->SendPacket(&data);			

	data.Initialize(SMSG_PET_BROKEN | CMSG_LEARN_SPELL);
    data << (uint64)0xbb8; 
	pl->GetSession()->SendPacket(&data);	
	pl->DuelVsPlayer->GetSession()->SendPacket(&data);		

}
void WorldSession::HandleDuelCancelledOpcode(WorldPacket& recvPacket)
{
	sLog.outString( "HandleDuelCancelledOpcode.\n" );
    //First time is work ,but can't send duel again , I don't know why
	//Please FIX ME
    uint64 guid;
	Player *pl;
	WorldPacket data;

    recvPacket >> guid;

	pl     = GetPlayer(); //get player
	WPAssert(pl->DuelVsPlayer );
	
	pl->SetUInt32Value(PLAYER_DUEL_ARBITER,0);
	pl->SetUInt32Value(PLAYER_DUEL_ARBITER_01,0);
	pl->SetUInt32Value(PLAYER_DUEL_TEAM,0);

	pl->DuelVsPlayer->SetUInt32Value(PLAYER_DUEL_ARBITER,0);
	pl->DuelVsPlayer->SetUInt32Value(PLAYER_DUEL_ARBITER_01,0);
	pl->DuelVsPlayer->SetUInt32Value(PLAYER_DUEL_TEAM,0);

	data.Initialize(SMSG_DUEL_COMPLETE);
    data << (uint8)0; // Duel Cancel
	pl->GetSession()->SendPacket(&data);
	pl->DuelVsPlayer->GetSession()->SendPacket(&data);

	//pl->DuelVsPlayer = NULL;
	//pl->DuelSendPlayer = NULL;
}





