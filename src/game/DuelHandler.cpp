/* DuelHandler.cpp
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
 *
 * This	program	is free	software; you can redistribute it and/or modify
 * it under	the	terms of the GNU General Public	License	as published by
 * the Free	Software Foundation; either	version	2 of the License, or
 * (at your	option)	any	later version.
 *
 * This	program	is distributed in the hope that	it will	be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A	PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received	a copy of the GNU General Public License
 * along with this program;	if not,	write to the Free Software
 * Foundation, Inc., 59	Temple Place, Suite	330, Boston, MA	 02111-1307	 USA
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
/*
void BuildDuelPacket(uint64 ObjID,uint64 Cast, uint64 Target)
{

}
*/

void WorldSession::HandleDuelAcceptedOpcode(WorldPacket& recvPacket)
{

    sLog.outString( "HandleDuelAcceptedOpcode.\n" );

    //if you want to  get this handle ,	learn spell	7266 first,
    //I	don't how to do	,Please	FIX	ME
    uint64 guid;

    recvPacket >> guid;

    Player *pl;
    Player *plTarget;
    WorldPacket data;

    WorldPacket packet,packetR;
    UpdateData updata;

    pl     = GetPlayer();                         // get duel sender
    plTarget = objmgr.GetPlayer(pl->m_duelGUID);
    //pl->build

    if(pl->m_duelSenderGUID == pl->GetGUID())
    {
        pl->BuildCreateUpdateBlockForPlayer( &updata, plTarget );
        updata.BuildPacket(&packet);
        plTarget->GetSession()->SendPacket( &packet );

        updata.Clear();

        plTarget->BuildCreateUpdateBlockForPlayer( &updata, pl );
        updata.BuildPacket(&packet);
        pl->GetSession()->SendPacket( &packet );

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

    pl       = GetPlayer();                       //get player
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
    data << (uint8)0;                             // Duel	Cancel
    pl->GetSession()->SendPacket(&data);
    plTarget->GetSession()->SendPacket(&data);

    pl->m_isInDuel = false;
    plTarget->m_isInDuel = false;

}
